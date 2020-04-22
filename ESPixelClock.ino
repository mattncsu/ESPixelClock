//***************************************************************
// A clock using pixels arranged in a 4-digit 7-segment display
//
//  This example uses 3 Pixels Per Segment (pps).
//  3pps x 7segments x 4digits + 2 digit colon = 86 pixels total
//
//
//  Based on 7Segment code by Marc Miller, (https://github.com/marmilicious/FastLED_examples/)
//  and ESP32 Simpletime example
//
//
//***************************************************************

#include <FastLED.h>
#include "secrets.h" //defines ssid, password, and ntpServer as const char*
#include <WiFi.h>
#include <time.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <DS3231M.h> //https://github.com/SV-Zanshin/DS3231M
#include <Sodaq_SHT2x.h> //https://github.com/mattncsu/Sodaq_SHT2x

DS3231M_Class DS3231M;

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

const long  gmtOffset_sec = -18000; //Eastern Time
const int   daylightOffset_sec = 3600;
float temp,rh;

#define DATA_PIN    18
#define CLK_PIN     13
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS    86
#define BRIGHTNESS  10
#define FRAMES_PER_SECOND 100
#define LED_PIN     14
#define BTN_PIN     15

uint8_t pps = 3;  // number of Pixels Per Segment
CHSV segBlack(0,0,0); //black

CRGBArray<NUM_LEDS> leds;

// Name segments and define pixel ranges.
//     1....2....3....4
//    AAA
//   F   B
//    GGG   
//   E   C  
//    DDD

CRGBSet seg1A(  leds(pps*0,  pps-1+(pps*0)  ));
CRGBSet seg1B(  leds(pps*1,  pps-1+(pps*1)  ));
CRGBSet seg1C(  leds(pps*2,  pps-1+(pps*2)  ));
CRGBSet seg1D(  leds(pps*3,  pps-1+(pps*3)  ));
CRGBSet seg1E(  leds(pps*4,  pps-1+(pps*4)  ));
CRGBSet seg1F(  leds(pps*5,  pps-1+(pps*5)  ));
CRGBSet seg1G(  leds(pps*6,  pps-1+(pps*6)  ));

CRGBSet seg2A(  leds(pps*0+(1*7*pps),  pps-1+(pps*0)+(1*7*pps)  ));
CRGBSet seg2B(  leds(pps*1+(1*7*pps),  pps-1+(pps*1)+(1*7*pps)  ));
CRGBSet seg2C(  leds(pps*2+(1*7*pps),  pps-1+(pps*2)+(1*7*pps)  ));
CRGBSet seg2D(  leds(pps*3+(1*7*pps),  pps-1+(pps*3)+(1*7*pps)  ));
CRGBSet seg2E(  leds(pps*4+(1*7*pps),  pps-1+(pps*4)+(1*7*pps)  ));
CRGBSet seg2F(  leds(pps*5+(1*7*pps),  pps-1+(pps*5)+(1*7*pps)  ));
CRGBSet seg2G(  leds(pps*6+(1*7*pps),  pps-1+(pps*6)+(1*7*pps)  ));

CRGBSet seg3A(  leds(pps*0+(2*7*pps),  pps-1+(pps*0)+(2*7*pps)  ));
CRGBSet seg3B(  leds(pps*1+(2*7*pps),  pps-1+(pps*1)+(2*7*pps)  ));
CRGBSet seg3C(  leds(pps*2+(2*7*pps),  pps-1+(pps*2)+(2*7*pps)  ));
CRGBSet seg3D(  leds(pps*3+(2*7*pps),  pps-1+(pps*3)+(2*7*pps)  ));
CRGBSet seg3E(  leds(pps*4+(2*7*pps),  pps-1+(pps*4)+(2*7*pps)  ));
CRGBSet seg3F(  leds(pps*5+(2*7*pps),  pps-1+(pps*5)+(2*7*pps)  ));
CRGBSet seg3G(  leds(pps*6+(2*7*pps),  pps-1+(pps*6)+(2*7*pps)  ));

CRGBSet seg4A(  leds(pps*0+(3*7*pps),  pps-1+(pps*0)+(3*7*pps)  ));
CRGBSet seg4B(  leds(pps*1+(3*7*pps),  pps-1+(pps*1)+(3*7*pps)  ));
CRGBSet seg4C(  leds(pps*2+(3*7*pps),  pps-1+(pps*2)+(3*7*pps)  ));
CRGBSet seg4D(  leds(pps*3+(3*7*pps),  pps-1+(pps*3)+(3*7*pps)  ));
CRGBSet seg4E(  leds(pps*4+(3*7*pps),  pps-1+(pps*4)+(3*7*pps)  ));
CRGBSet seg4F(  leds(pps*5+(3*7*pps),  pps-1+(pps*5)+(3*7*pps)  ));
CRGBSet seg4G(  leds(pps*6+(3*7*pps),  pps-1+(pps*6)+(3*7*pps)  ));

CRGBSet col(leds(84,85)); //colon

int count = 8888;  // 4-digit number displayed on LEDs


//---------------------------------------------------------------

void printLocalTime()
{
 
  DateTime now = DS3231M.now();
  temp = SHT2x.GetTemperature();
  rh = SHT2x.GetHumidity();
//  Serial.print(DS3231M.temperature()/100.0);
//  Serial.println("\xC2\xB0""C");
  Serial.printf("%04d-%02d-%02d %02d:%02d:%02d Temp: %.1fC RH:%.1f%%\n", now.year(),          // Use sprintf() to pretty print    //
          now.month(), now.day(), now.hour(), now.minute(), now.second(),
          temp, rh);  

  count=now.hour()*100+now.minute();

//  Serial.print("Humidity(%RH): ");
//  Serial.print(SHT2x.GetHumidity());
//  Serial.print("    Temp: ");
//  Serial.print(SHT2x.GetTemperature());
//  Serial.print("C     Dewpoint(C): ");
//  Serial.println(SHT2x.GetDewPoint());
// For plotting temperatures in Arduino Serial Plotter:
//  Serial.printf("RTC_T:%.1f SHT_T:%.1f RHT_RH:%.1f\n",DS3231M.temperature()/100.0-2.3,SHT2x.GetTemperature(),SHT2x.GetHumidity());
}

void getNTPTime(){
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); //get fresh NTP time
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "NTP Time: %A, %B %d %Y %H:%M:%S");

  int hour = timeinfo.tm_hour;
  int minute  = timeinfo.tm_min;
  int second  = timeinfo.tm_sec;
  
  int day = timeinfo.tm_mday;
  int month = timeinfo.tm_mon + 1;
  int year = timeinfo.tm_year +1900;
  DS3231M.adjust(DateTime(year,month,day,hour,minute,second));      // Adjust the RTC date/time         //
  Serial.print(F("RTC time/date has been set.\n")); 
   
}

void setup() {
  Serial.begin(115200);  // Allows serial monitor output
  pinMode(BTN_PIN,INPUT_PULLUP);
  pinMode(LED_PIN,OUTPUT);
  //connect to WiFi
  if (DS3231M.begin()){
    Serial.print(F("DS3231M found!\n"));
  }
   else {
    Serial.print(F("DS3231M not found, moving on...\n"));
  }
  Serial.printf("Connecting to %s ", ssid);
  WiFi.mode(WIFI_STA);
  int timeout = millis()+10000;
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      if (millis() > timeout){
        Serial.println("Giving up on wifi..");
        break;
      }
  }
  Serial.println(" CONNECTED");

 
  
  //init and get the time
  getNTPTime();
  printLocalTime();

  
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setBrightness(BRIGHTNESS);
  currentPalette = RainbowColors_p;
//  currentPalette = CloudColors_p;
//  currentPalette = PartyColors_p;
//  currentPalette = ForestColors_p;
//  currentPalette = OceanColors_p;
  currentBlending = LINEARBLEND;
  FastLED.clear();  // Initially clear all pixels
}

bool colon;

//---------------------------------------------------------------
void loop()
{
  ArduinoOTA.handle();

  static uint8_t startIndex = 0;
  EVERY_N_MILLISECONDS(50){ //motion speed: increase delay to slow down
    startIndex = startIndex + 1;
  }
    setSegments(count,startIndex);  // Determine which segments are ON or OFF

  EVERY_N_MILLISECONDS(1000){
    colon = !colon; //flash the colon

  }
  EVERY_N_MILLISECONDS(5000){
    printLocalTime(); //Updates the time in the display to current time
  } 
  EVERY_N_MINUTES(60){
    getNTPTime();
  }
  FastLED.delay(1000/FRAMES_PER_SECOND); 
//  if(digitalRead(BTN_PIN)){
//    digitalWrite(LED_PIN,HIGH);
//     Serial.println("BTN PRESSED");
//    
//  } else {
//    digitalWrite(LED_PIN,LOW);
//   
//  }
}

//---------------------------------------------------------------
void setSegments(int count, uint8_t colorIndex){
  // Based on the current count set number segments on or off
  uint8_t c1 = 0;  // Variable to store 1s digit
  uint8_t c10 = 0;  // Variable to store 10s digit
  uint8_t c100 = 0;  // Variable to store 100s digit
  uint8_t c1000 = 0;  // Variable to store 100s digit
  int c;
  CHSV segCOLOR(0,0,0);

  c1 = count % 10;
  c10 = (count / 10) % 10;
  c100 = (count / 100) % 10;
  c1000 = (count / 1000) % 10;
    
//  Serial.print("count = "); Serial.print(count);  // Print to serial monitor current count
//  Serial.print("\t  1000s: "); Serial.print(c1000);  // Print 1000s digit
//  Serial.print("  100s: "); Serial.print(c100);  // Print 100s digit
//  Serial.print("   10s: "); Serial.print(c10);  // Print 10s digit
//  Serial.print("   1s: "); Serial.println(c1);  // Print 1s digit

     uint8_t brightness = 255;
     
     for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
    //next block of if statements sets segments to black to form digits
    segCOLOR = segBlack; //unused segment color
    if (c1000 == 0) { seg1G = segCOLOR; }
    if (c1000 == 1) { seg1A = seg1D = seg1E = seg1F = seg1G = segCOLOR; } 
    if (c1000 == 2) { seg1C = seg1F = segCOLOR; } 
    if (c1000 == 3) { seg1E = seg1F = segCOLOR; } 
    if (c1000 == 4) { seg1A = seg1D = seg1E = segCOLOR; } 
    if (c1000 == 5) { seg1B = seg1E = segCOLOR; } 
    if (c1000 == 6) { seg1B = segCOLOR; } //B
    if (c1000 == 7) { seg1D = seg1E = seg1F = seg1G = segCOLOR; } 
    if (c1000 == 8) {  }
    if (c1000 == 9) { seg1D = seg1E = segCOLOR; } 

    if (c100 == 0) { seg2G = segCOLOR; }
    if (c100 == 1) { seg2A = seg2D = seg2E = seg2F = seg2G = segCOLOR; } 
    if (c100 == 2) { seg2C = seg2F = segCOLOR; } 
    if (c100 == 3) { seg2E = seg2F = segCOLOR; } 
    if (c100 == 4) { seg2A = seg2D = seg2E = segCOLOR; } 
    if (c100 == 5) { seg2B = seg2E = segCOLOR; } 
    if (c100 == 6) { seg2B = segCOLOR; } //B
    if (c100 == 7) { seg2D = seg2E = seg2F = seg2G = segCOLOR; } 
    if (c100 == 8) {  }
    if (c100 == 9) { seg2D = seg2E = segCOLOR; } 

    if (c10 == 0) { seg3G = segCOLOR; }
    if (c10 == 1) { seg3A = seg3D = seg3E = seg3F = seg3G = segCOLOR; } 
    if (c10 == 2) { seg3C = seg3F = segCOLOR; } 
    if (c10 == 3) { seg3E = seg3F = segCOLOR; } 
    if (c10 == 4) { seg3A = seg3D = seg3E = segCOLOR; } 
    if (c10 == 5) { seg3B = seg3E = segCOLOR; } 
    if (c10 == 6) { seg3B = segCOLOR; } //B
    if (c10 == 7) { seg3D = seg3E = seg3F = seg3G = segCOLOR; } 
    if (c10 == 8) {  }
    if (c10 == 9) { seg3D = seg3E = segCOLOR; } 

    if (c1 == 0) { seg4G = segCOLOR; }
    if (c1 == 1) { seg4A = seg4D = seg4E = seg4F = seg4G = segCOLOR; } 
    if (c1 == 2) { seg4C = seg4F = segCOLOR; } 
    if (c1 == 3) { seg4E = seg4F = segCOLOR; } 
    if (c1 == 4) { seg4A = seg4D = seg4E = segCOLOR; } 
    if (c1 == 5) { seg4B = seg4E = segCOLOR; } 
    if (c1 == 6) { seg4B = segCOLOR; } //B
    if (c1 == 7) { seg4D = seg4E = seg4F = seg4G = segCOLOR; } 
    if (c1 == 8) {  }
    if (c1 == 9) { seg4D = seg4E = segCOLOR; } 

     if (colon){
//      col = colON; //comment out to use palette color
    } else {
      col = CRGB::Black; //turns off colon to make it blink
    }

  
//  for (uint8_t p=0; p < NUM_LEDS;p++) { //prints the status of all LEDs for troubleshooting
//  Serial.print(leds[p]);
//  if((p+1) % (7*pps) == 0){
//    Serial.print(" ");
//  }
//}
//Serial.println();
}//end setSegments
