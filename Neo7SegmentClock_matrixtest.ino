#include <Adafruit_GFX.h>
#include <FastLED_NeoMatrix.h>
#include <FastLED.h>

#define PIN 18
#define BRIGHTNESS 10

#define mw 25
#define mh 9
#define NUMMATRIX (mw*mh)
CRGB leds[NUMMATRIX];
// Define matrix width and height.
FastLED_NeoMatrix *matrix = new FastLED_NeoMatrix(leds, mw, mh, 
  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
    NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE);

// This could also be defined as matrix->color(255,0,0) but those defines
// are meant to work for adafruit_gfx backends that are lacking color()
#define LED_BLACK    0

#define LED_RED_VERYLOW   (3 <<  11)
#define LED_RED_LOW     (7 <<  11)
#define LED_RED_MEDIUM    (15 << 11)
#define LED_RED_HIGH    (31 << 11)

#define LED_GREEN_VERYLOW (1 <<  5)   
#define LED_GREEN_LOW     (15 << 5)  
#define LED_GREEN_MEDIUM  (31 << 5)  
#define LED_GREEN_HIGH    (63 << 5)  

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

void scrolling_line(){
 
  for (uint8_t i = 0;i<mw;i++){
    matrix->clear();
    matrix->drawLine(i,0,i,mh,LED_RED_HIGH);
    matrix->show();
    delay(100);
  }
  for (uint8_t i = 0;i<mh;i++){
    matrix->clear();
    matrix->drawLine(0,i,mw,i,LED_GREEN_HIGH);
    matrix->show();
    delay(100);
  }
}

void setup() {
  // put your setup code here, to run once:
// Time for serial port to work?
    delay(1000);
    Serial.begin(115200);
    Serial.print("Init on pin: ");
    Serial.println(PIN);
    Serial.print("Matrix Size: ");
    Serial.print(mw);
    Serial.print(" ");
    Serial.print(mh);
    Serial.print(" ");
    Serial.println(NUMMATRIX);
    FastLED.addLeds<NEOPIXEL,PIN>(  leds, NUMMATRIX  ).setCorrection(TypicalLEDStrip);
    Serial.print("Setup serial: ");
    Serial.println(NUMMATRIX);
    matrix->begin();
    matrix->setTextWrap(false);
    matrix->setBrightness(BRIGHTNESS);
    matrix->setRemapFunction(myRemapFn);
}

void loop() {
  // put your main code here, to run repeatedly:
scrolling_line();
}
