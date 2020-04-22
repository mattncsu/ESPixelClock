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
#include <FastLED_NeoMatrix.h>
#include <FastLED.h>
#include "secrets.h" //defines ssid, password, and ntpServer as const char*
#include <WiFi.h>
#include <time.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <Hash.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>


AsyncWebServer server(80);

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

const long  gmtOffset_sec = -18000; //Eastern Time
const int   daylightOffset_sec = 3600;
bool colon,timerMode,countdownMode;
int countdown;

#define DATA_PIN    18
#define CLK_PIN     13
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS    86
#define BRIGHTNESS  10
#define FRAMES_PER_SECOND 100
#define LED_PIN     14
#define BTN_PIN     16
#define IDLE_TIME   5*60*1000

#define mw 25
#define mh 9
#define NUMMATRIX (mw*mh)
CRGB leds[NUMMATRIX];
// Define matrix width and height.
FastLED_NeoMatrix *matrix = new FastLED_NeoMatrix(leds, mw, mh, 
  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
    NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE);

//PCB Layout:
//    .0 .1 .2 .3 .4 .5 .6 .7 .8 .9 10 11.12 13 14 15 16 17 18 19 20 21 22 23 24
// 0  .. .0 .1 .2 .. .. .. 21 22 23 .. .. .. .. .. 42 43 44 .. .. .. 63 64 65 ..
// 1  17 .. .. .. .3 .. 38 .. .. .. 24 .. .. .. 59 .. .. .. 45 .. 80 .. .. .. 66
// 2  16 .. .. .. .4 .. 37 .. .. .. 25 .. 84 .. 58 .. .. .. 46 .. 79 .. .. .. 67
// 3  15 .. .. .. .5 .. 36 .. .. .. 26 .. .. .. 57 .. .. .. 47 .. 78 .. .. .. 68
// 4  .. 18 19 20 .. .. .. 39 40 41 .. .. .. .. .. 60 61 62 .. .. .. 81 82 83 ..
// 5  14 .. .. .. .6 .. 35 .. .. .. 27 .. .. .. 56 .. .. .. 48 .. 77 .. .. .. 69
// 6  13 .. .. .. .7 .. 34 .. .. .. 28 .. 85 .. 55 .. .. .. 49 .. 76 .. .. .. 70
// 7  12 .. .. .. .8 .. 33 .. .. .. 29 .. .. .. 54 .. .. .. 50 .. 75 .. .. .. 71
// 8  .. 11 10 .9 .. .. .. 32 31 30 .. .. .. .. .. 53 52 51 .. .. .. 74 73 72 ..    

uint16_t myRemapFn(uint16_t x, uint16_t y) {
    switch(y){
    case 0:
      switch(x){
          case 1: return 0;
          case 2: return 1;
          case 3: return 2;
          case 7: return 21;
          case 8: return 22;
          case 9: return 23;
          case 15: return 42;
          case 16: return 43;
          case 17: return 44;
          case 21: return 63;
          case 22: return 64;
          case 23: return 65;
          default: return 86;
      }
    
    case 1:
      switch(x){
        case 0: return 17;
        case 4: return 3;
        case 6: return 38;
        case 10: return 24;
        case 14: return 59;
        case 18: return 45;
        case 20: return 80;
        case 24: return 66;
        default: return 86;
      }
    case 2:
      switch(x){
        case 0: return 16;
        case 4: return 4;
        case 6: return 37;
        case 10: return 25;
        case 12: return 84;
        case 14: return 58;
        case 18: return 46;
        case 20: return 79;
        case 24: return 67;
        default: return 86;
      }   
    case 3:
      switch(x){
        case 0: return 15;
        case 4: return 5;
        case 6: return 36;
        case 10: return 26;
        case 14: return 57;
        case 18: return 47;
        case 20: return 78;
        case 24: return 68;
        default: return 86;
      }   
    case 4:
      switch(x){
          case 1: return 18;
          case 2: return 19;
          case 3: return 20;
          case 7: return 39;
          case 8: return 40;
          case 9: return 41;
          case 15: return 60;
          case 16: return 61;
          case 17: return 62;
          case 21: return 81;
          case 22: return 82;
          case 23: return 83;
          default: return 86;
      }   
    case 5:
      switch(x){
        case 0: return 14;
        case 4: return 6;
        case 6: return 35;
        case 10: return 27;
        case 14: return 56;
        case 18: return 48;
        case 20: return 77;
        case 24: return 69;
        default: return 86;
      }   
    case 6:
      switch(x){
        case 0: return 13;
        case 4: return 7;
        case 6: return 34;
        case 10: return 28;
        case 12: return 85;
        case 14: return 55;
        case 18: return 49;
        case 20: return 76;
        case 24: return 70;
        default: return 86;
      }   
    case 7:
      switch(x){
        case 0: return 12;
        case 4: return 8;
        case 6: return 33;
        case 10: return 29;
        case 14: return 54;
        case 18: return 50;
        case 20: return 75;
        case 24: return 71;
        default: return 86;
      }   
    case 8:
      switch(x){
          case 1: return 11;
          case 2: return 10;
          case 3: return 9;
          case 7: return 32;
          case 8: return 31;
          case 9: return 30;
          case 15: return 53;
          case 16: return 52;
          case 17: return 51;
          case 21: return 74;
          case 22: return 73;
          case 23: return 72;
          default: return 86;
      }   
  } 
}

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

uint8_t pps = 3;  // number of Pixels Per Segment
CHSV segBlack(0,0,0); //black

//CRGBArray<NUM_LEDS> leds;

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

int count = 8888;  // keeps track of what number to display


//---------------------------------------------------------------

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  count=(timeinfo.tm_hour*100+timeinfo.tm_min);
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


}//end setSegments

void setup() {
  Serial.begin(115200);  // Allows serial monitor output
  pinMode(button1.PIN,INPUT_PULLUP);
  attachInterruptArg(button1.PIN, isr, &button1, FALLING);
  pinMode(15, OUTPUT); //Led pin GND
  digitalWrite(15,LOW); //ground LED pin
  pinMode(LED_PIN, OUTPUT);

  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

 server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! I am ESP32.");
  });

  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");

 
  
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  FastLED.addLeds<NEOPIXEL,PIN>(  leds, NUMMATRIX  ).setCorrection(TypicalLEDStrip);
//  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setBrightness(BRIGHTNESS);
  matrix->begin();
  matrix->setTextWrap(false);
  matrix->setBrightness(BRIGHTNESS);
  matrix->setRemapFunction(myRemapFn);
  currentPalette = RainbowColors_p;
//  currentPalette = CloudColors_p;
//  currentPalette = PartyColors_p;
//  currentPalette = ForestColors_p;
//  currentPalette = OceanColors_p;
  currentBlending = LINEARBLEND;
  FastLED.clear();  // Initially clear all pixels
}

long startTime;

//---------------------------------------------------------------
void loop()
{
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
  static uint8_t startIndex = 0;
  EVERY_N_MILLISECONDS(50){ //motion speed: increase delay to slow down
    startIndex = startIndex + 1;
  }
    setSegments(count,startIndex);  // Determine which segments are ON or OFF

  EVERY_N_MILLISECONDS(1000){
    colon = !colon; //flash the colon
    if(countdownMode && countdown==0){
      countdownMode=false;
      timerMode=true;
      startTime=millis();
      count=0;
    }
    if(countdownMode){
      countdown--;
    }
  }

  EVERY_N_MILLISECONDS(10){
    int x = millis() - startTime;
    if (countdownMode){
      count=countdown;
    }
    if (timerMode){
      if(x < 90000){
        count=(x)/10;
        colon=true;
      }
      else{
        count = (x/1000/60)*100 + (x/1000) % 60;
      }
      if (millis()-startTime>IDLE_TIME){
        timerMode=false;
      }
    }
  }

  EVERY_N_MILLISECONDS(250){
    if (timerMode){
      digitalWrite(LED_PIN,(!digitalRead(LED_PIN)));
    }
  }
  
  EVERY_N_MILLISECONDS(5000){
    if (!timerMode && !countdownMode){
      printLocalTime();
    }
  } 
  EVERY_N_MINUTES(5){
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); //get fresh NTP time
  }
  FastLED.delay(1000/FRAMES_PER_SECOND); 

}
