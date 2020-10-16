/*
   ESPixelClock
   https://github.com/mattncsu/ESPixelClock

   



   Timezone codes from: https://github.com/jdlambert/micro_tz_db
   Based on:
   ESP32 FastLED WebServer: https://github.com/jasoncoon/esp32-fastled-webserver
   Copyright (C) 2017 Jason Coon

   Built upon the amazing FastLED work of Daniel Garcia and Mark Kriegsman:
   https://github.com/FastLED/FastLED

   ESP32 support provided by the hard work of Sam Guyer:
   https://github.com/samguyer/FastLED

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <FastLED.h>
#include <WiFi.h>
#include "esp_wifi.h"
#include <FS.h>
#include <SPIFFS.h>
#include <EEPROM.h>
#include <Preferences.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

#include <time.h>
#include <DS3231M.h> //https://github.com/SV-Zanshin/DS3231M
#include <SHT21.h> //https://github.com/markbeee/SHT21
DS3231M_Class DS3231M;
SHT21 SHT21;
wifi_sta_list_t stationList; //Used to determine if anyone connected to softAP
Preferences preferences;
AsyncWebServer server(80); //for the update server

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001008)
#warning "Requires FastLED 3.1.8 or later; check github for latest code."
#endif

String ssid,password,tzInfo,ntpServer1,ntpServer2,clockName; 

int8_t timezoneOffset = -6; // Central Time
bool tzChanged=false;
int8_t temperatureOffset = 0;
long gmtOffset_sec = timezoneOffset * 3600;
const int daylightOffset_sec = 3600;

int16_t timeInt = 8888;  // keeps track of what number to display
bool colon = false;
bool timerMode,countdownMode;
int16_t countdown;
long startTime;
float temp,rh;
DateTime now;

#define LED_PIN     27 //LED from lighted switch
#define BTN_PIN     32 //Push button to start stop watch mode
#define IDLE_TIME   5*60*1000 //terminate stop watch mode after 5 minutes

uint8_t autoplay = 0;
uint8_t autoplayDuration = 10;
unsigned long autoPlayTimeout = 0;

uint8_t currentPatternIndex = 0; // Index number of which pattern is current

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

uint8_t power = 1;
uint8_t leadingZeroEnabled = 1;
uint8_t blinkingColonEnabled = 1;
uint8_t time24 = 1;
uint8_t brightness = 8;

uint8_t speed = 30;

// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100
uint8_t cooling = 50;

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
uint8_t sparking = 120;

CRGB solidColor = CRGB::Blue;

uint8_t cyclePalettes = 0;
uint8_t paletteDuration = 10;
uint8_t currentPaletteIndex = 3;
unsigned long paletteTimeout = 0;

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#define DATA_PIN    18
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    86
CRGBArray<NUM_LEDS> leds;

#define MILLI_AMPS         2000 // IMPORTANT: set the max milli-Amps of your power supply (4A = 4000mA)
#define FRAMES_PER_SECOND  60

#include "patterns.h"
#include "field.h"
#include "fields.h"


#include "wifi.h"
#include "web.h"

// wifi ssid and password should be added to a file in the sketch named secrets.h
// the secrets.h file should be added to the .gitignore file and never committed or
// pushed to public source control (GitHub).
// const char* ssid = "........";
// const char* password = "........";

struct Button {
    const uint8_t PIN;
    bool pressed;
};
Button button1 = {BTN_PIN, false};

void IRAM_ATTR isr(void* arg) {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200)
  {
    Button* s = static_cast<Button*>(arg);
    s->pressed = true;
  }
  last_interrupt_time = interrupt_time;
    
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void printLocalTime()
{
  
  setenv("TZ", tzInfo.c_str(), 1);
  tzset();
  now = DS3231M.now();

  //struct tm timeinfo;

  Serial.printf("%04d-%02d-%02d %02d:%02d:%02d Temp: %.1fC RH:%.1f%%  %s\n", now.year(),          // Use sprintf() to pretty print    //
          now.month(), now.day(), now.hour(), now.minute(), now.second(),
          temp, rh, tzInfo.c_str());  
  if (time24 == 0){
    if (now.hour() > 12){
      timeInt = (now.hour()-12)*100+now.minute();
    }
    else{
      timeInt = now.hour()*100+now.minute();
    }
  }
  else{
    timeInt = now.hour()*100+now.minute();
  }
}

void getNTPTime(){
  if ( WiFi.status() == WL_CONNECTED) {
    Serial.println("Getting NTP Time...");
    configTzTime(tzInfo.c_str(), ntpServer1.c_str(), ntpServer2.c_str()); //get fresh NTP time
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
      Serial.println("Failed to obtain NTP time");
      return;
    }
    Serial.println(&timeinfo, "NTP Time: %A, %B %d %Y %H:%M:%S");
    int hour = timeinfo.tm_hour;
    int minute  = timeinfo.tm_min;
    int second  = timeinfo.tm_sec;
    int day = timeinfo.tm_mday;
    int month = timeinfo.tm_mon + 1;
    int year = timeinfo.tm_year +1900;
    DS3231M.adjust(DateTime(year,month,day,hour,minute,second));      // Adjust the RTC date/time
    Serial.print(F("RTC time/date has been set.\n")); 
  
  } else {
    Serial.println("Can't get NTP time without WiFi...");
  }
}


void setup() {
  pinMode(button1.PIN,INPUT_PULLUP);
  attachInterruptArg(button1.PIN, isr, &button1, FALLING);
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(115200);
  Serial.println("Reading data from EEPROM...");
 
  preferences.begin("ESPixelClock", true);
    ssid =  preferences.getString("ssid", "none");           //NVS key ssid,default="none"
    Serial.printf("     SSID: %s\n",ssid.c_str());
    password =  preferences.getString("password", "none");   //NVS key password,default="none"
    Serial.printf("     WiFi pass: %s\n","*****");
    tzInfo = preferences.getString("timezone", "EST5EDT,M3.2.0,M11.1.0"); //load eastern time if no tz stored
    Serial.printf("     Timezone: %s\n",tzInfo.c_str());
    ntpServer1  = preferences.getString("ntpServer1", "time.google.com");
    Serial.printf("     ntpServer1: %s\n",ntpServer1.c_str());
    ntpServer2 = preferences.getString("ntpServer2", "pool.ntp.org");
    Serial.printf("     ntpServer2: %s\n",ntpServer2.c_str());
    clockName = preferences.getString("clockName", "ESPixelClock");
  preferences.end();
  Serial.println("Done loading data from EEPROM");

  if (DS3231M.begin()){
    Serial.print(F("DS3231M found!\n"));
    printLocalTime();
  }
   else {
    Serial.print(F("DS3231M not found, moving on...\n"));
  }
  SHT21.begin();

  SPIFFS.begin();
  listDir(SPIFFS, "/", 1);

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setCorrection(TypicalSMD5050);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MILLI_AMPS);
 
  // set master brightness control
  FastLED.setBrightness(brightness);
  fill_rainbow( leds, NUM_LEDS, gHue, speed);
  FastLED.show();
  FastLED.delay(250);
  autoPlayTimeout = millis() + (autoplayDuration * 1000);
  
  loadFieldsFromEEPROM(fields, fieldCount);
  setupWifi();
  setupWeb();
  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started on port 80");
  getNTPTime(); //get initial time
  temp = SHT21.getTemperature()+temperatureOffset; //get inital temp reading
  rh = SHT21.getHumidity();
}

void maskTime(int timeInt){
  // Based on the current timeInt set number segments on or off
  uint8_t c1 = 0;  // Variable to store 1s digit
  uint8_t c10 = 0;  // Variable to store 10s digit
  uint8_t c100 = 0;  // Variable to store 100s digit
  uint8_t c1000 = 0;  // Variable to store 100s digit
  int c;

  c1 = timeInt % 10;
  c10 = (timeInt / 10) % 10;
  c100 = (timeInt / 100) % 10;
  c1000 = (timeInt / 1000) % 10;
  
  CRGB color = CRGB::Black; //unused segment color
  
  //next block of if statements sets segments to black to form digits
  if (c1000 == 0) { 
    if (leadingZeroEnabled==1){  //set leading value to black if zero
      seg1G = color; 
    } else {
      seg1A = seg1B = seg1C = seg1D = seg1E = seg1F = seg1G = color;
    }
  }
  if (c1000 == 1) { seg1A = seg1D = seg1E = seg1F = seg1G = color; } 
  if (c1000 == 2) { seg1C = seg1F = color; } 
  if (c1000 == 3) { seg1E = seg1F = color; } 
  if (c1000 == 4) { seg1A = seg1D = seg1E = color; } 
  if (c1000 == 5) { seg1B = seg1E = color; } 
  if (c1000 == 6) { seg1B = color; } //B
  if (c1000 == 7) { seg1D = seg1E = seg1F = seg1G = color; } 
  if (c1000 == 8) {  }
  if (c1000 == 9) { seg1D = seg1E = color; } 

  if (c100 == 0) { seg2G = color; }
  if (c100 == 1) { seg2A = seg2D = seg2E = seg2F = seg2G = color; } 
  if (c100 == 2) { seg2C = seg2F = color; } 
  if (c100 == 3) { seg2E = seg2F = color; } 
  if (c100 == 4) { seg2A = seg2D = seg2E = color; } 
  if (c100 == 5) { seg2B = seg2E = color; } 
  if (c100 == 6) { seg2B = color; } //B
  if (c100 == 7) { seg2D = seg2E = seg2F = seg2G = color; } 
  if (c100 == 8) {  }
  if (c100 == 9) { seg2D = seg2E = color; } 

  if (c10 == 0) { seg3G = color; }
  if (c10 == 1) { seg3A = seg3D = seg3E = seg3F = seg3G = color; } 
  if (c10 == 2) { seg3C = seg3F = color; } 
  if (c10 == 3) { seg3E = seg3F = color; } 
  if (c10 == 4) { seg3A = seg3D = seg3E = color; } 
  if (c10 == 5) { seg3B = seg3E = color; } 
  if (c10 == 6) { seg3B = color; } //B
  if (c10 == 7) { seg3D = seg3E = seg3F = seg3G = color; } 
  if (c10 == 8) {  }
  if (c10 == 9) { seg3D = seg3E = color; } 

  if (c1 == 0) { seg4G = color; }
  if (c1 == 1) { seg4A = seg4D = seg4E = seg4F = seg4G = color; } 
  if (c1 == 2) { seg4C = seg4F = color; } 
  if (c1 == 3) { seg4E = seg4F = color; } 
  if (c1 == 4) { seg4A = seg4D = seg4E = color; } 
  if (c1 == 5) { seg4B = seg4E = color; } 
  if (c1 == 6) { seg4B = color; } //B
  if (c1 == 7) { seg4D = seg4E = seg4F = seg4G = color; } 
  if (c1 == 8) {  }
  if (c1 == 9) { seg4D = seg4E = color; } 

  if (!colon && blinkingColonEnabled == 1){
    col = CRGB::Black; //turns off colon to make it blink
  }
  if (timerMode){
    if (millis() - startTime <99000){
      leds[84]=CRGB::Black; //convert colon to decimal if timer < 99 seconds
    }
  }
}

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  currentPatternIndex = (currentPatternIndex + 1) % patternCount;
}

void nextPalette()
{
  currentPaletteIndex = (currentPaletteIndex + 1) % paletteCount;
  targetPalette = palettes[currentPaletteIndex];
}

void loop()
{
  //handleWeb();
  AsyncElegantOTA.loop();
  if (button1.pressed) {
    Serial.printf("Button 1 has been pressed\n");
    button1.pressed = false;
    if (!timerMode){
      countdownMode=true;
      countdown=10;
      digitalWrite(LED_PIN,HIGH);
    } else {
      digitalWrite(LED_PIN,LOW);
      FastLED.delay(15000);
      timerMode=false;
    }
  }

  if (power == 0) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
  }
  else {
    // Call the current pattern function once, updating the 'leds' array
    patterns[currentPatternIndex].pattern();
    maskTime(timeInt);

    EVERY_N_MILLISECONDS(40) {
      // slowly blend the current palette to the next
      nblendPaletteTowardPalette(currentPalette, targetPalette, 8);
      gHue++;  // slowly cycle the "base color" through the rainbow
    }

    if (autoplay == 1 && (millis() > autoPlayTimeout)) {
      nextPattern();
      autoPlayTimeout = millis() + (autoplayDuration * 1000);
    }

    if (cyclePalettes == 1 && (millis() > paletteTimeout)) {
      nextPalette();
      paletteTimeout = millis() + (paletteDuration * 1000);
    }
  }

  // send the 'leds' array out to the actual LED strip
  FastLED.show();

  // insert a delay to keep the framerate modest
  // delay(1000 / FRAMES_PER_SECOND);
  FastLED.delay(1000 / FRAMES_PER_SECOND);

  // time stuff
  EVERY_N_SECONDS(1){
    colon = !colon; // flash the colon
  }

//  EVERY_N_SECONDS(120){ //UNCOMMENT TO TEST THE PUSH BUTTON ISR THAT STARTS TIMER MODE AFTER 2 MINUTES
//  static unsigned long last_interrupt_time = 0;
//  unsigned long interrupt_time = millis();
//  // If interrupts come faster than 200ms, assume it's a bounce and ignore
//  if (interrupt_time - last_interrupt_time > 200)
//  {
//    Button* s = static_cast<Button*>(&button1);
//    s->pressed = true;
//  }
//  last_interrupt_time = interrupt_time;
//  }

  EVERY_N_MILLISECONDS(1000){

    if(countdownMode && countdown==0){ //check if countdown is over, if so, start stopwatch
      countdownMode=false;
      timerMode=true;
      startTime=millis();
      timeInt=0;
    }
    if(countdownMode){
      countdown--;
    }
  }

  EVERY_N_MILLISECONDS(10){
    int x = millis() - startTime;
    if (countdownMode){
      timeInt=countdown;
    }
    if (timerMode){    //show stopwatch on clock as xx.xx up to 99 seconds
      if(x < 99000){
        timeInt=(x)/10;
        colon=true;
        
      }
      else{
        timeInt = (x/1000/60)*100 + (x/1000) % 60; //show stopwatch in mm:ss until IDLE_TIME reached
      }
      if (millis()-startTime>IDLE_TIME){ //revert back to a clock mode
        timerMode=false;
      }
    }
  }

  EVERY_N_MILLISECONDS(250){ //flash LED in stopwatch mode
    if (timerMode){
      digitalWrite(LED_PIN,(!digitalRead(LED_PIN)));
    }
  }
  
  EVERY_N_SECONDS(5){ //refesh display every 5 seconds
    if (!timerMode && !countdownMode){
      printLocalTime();
    }
  } 
  
  EVERY_N_HOURS(12){ //get NTP time every 12 hours, clock relies on RTC with <5ppm drift to keep time between intervals (<15 sec/month at normal temperatures)
      getNTPTime(); //get fresh NTP time
  }
  if (tzChanged){
    getNTPTime();
    Serial.println("tz changed");
    tzChanged=false;
  }

  EVERY_N_MINUTES(5){ //I2C reading causes studder in pattern
    temp = SHT21.getTemperature()+temperatureOffset;
    rh = SHT21.getHumidity();
  }

}
