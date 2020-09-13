/*
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

String getPower() {
  return String(power);
}

String setPower(String value) {
  power = value.toInt();
  power = power == 0 ? 0 : 1;
  return String(power);
}

String getLeadingZero() {
  return String(leadingZeroEnabled);
}

String setLeadingZero(String value) {
  leadingZeroEnabled = value.toInt();
  leadingZeroEnabled = leadingZeroEnabled == 0 ? 0 : 1;
  return String(leadingZeroEnabled);
}

String getBlinkingColon() {
  return String(blinkingColonEnabled);
}

String setBlinkingColon(String value) {
  blinkingColonEnabled = value.toInt();
  blinkingColonEnabled = blinkingColonEnabled == 0 ? 0 : 1;
  return String(blinkingColonEnabled);
}

String getTime24() {
  return String(time24);
}

String setTime24(String value) {
  time24 = value.toInt();
  time24 = time24 == 0 ? 0 : 1;
  return String(time24);
}

String getTemp() {
  return String(temp);
}

String setTemp(String value) {
  return String(temp);
}

String getHumid() {
  return String(rh);
}

String setHumid(String value) {
  return String(rh);
}

String getBrightness() {
  return String(brightness);
}

String setBrightness(String value) {
  brightness = value.toInt();
  FastLED.setBrightness(brightness);
  return String(brightness);
}

//String getTimezoneOffset() {
//  return String(timezoneOffset);
//}
//
//String setTimezoneOffset(String value) {
//  timezoneOffset = value.toInt();
//  if (timezoneOffset < -12) timezoneOffset = -12;
//  else if (timezoneOffset > 14) timezoneOffset = 14;
//  
//  gmtOffset_sec = timezoneOffset * 3600;
//  tzChanged=true;
//  return String(timezoneOffset);
//}

String getTemperatureOffset() {
  return String(temperatureOffset);
}

String setTemperatureOffset(String value) {
  temperatureOffset = value.toInt();
  return String(temperatureOffset);
}

String getPattern() {
  return String(currentPatternIndex);
}

void setPattern(uint8_t value)
{
  if (value >= patternCount)
    value = patternCount - 1;

  currentPatternIndex = value;
}

String setPattern(String value) {
  setPattern(value.toInt());
  return String(currentPatternIndex);
}

String getPatterns() {
  String json = "";

  for (uint8_t i = 0; i < patternCount; i++) {
    json += "\"" + patterns[i].name + "\"";
    if (i < patternCount - 1)
      json += ",";
  }

  return json;
}



String getPalette() {
  return String(currentPaletteIndex);
}

String setPalette(String value) {
  currentPaletteIndex = value.toInt();
  if (currentPaletteIndex < 0) currentPaletteIndex = 0;
  else if (currentPaletteIndex >= paletteCount) currentPaletteIndex = paletteCount - 1;
  targetPalette = palettes[currentPaletteIndex];
  return String(currentPaletteIndex);
}

String getPalettes() {
  String json = "";

  for (uint8_t i = 0; i < paletteCount; i++) {
    json += "\"" + paletteNames[i] + "\"";
    if (i < paletteCount - 1)
      json += ",";
  }

  return json;
}

String getSpeed() {
  return String(speed);
}

String setSpeed(String value) {
  speed = value.toInt();
  return String(speed);
}

String getAutoplay() {
  return String(autoplay);
}

String setAutoplay(String value) {
  autoplay = value.toInt();
  autoplay = autoplay == 0 ? 0 : 1;
  autoPlayTimeout = millis() + (autoplayDuration * 1000);
  return String(autoplay);
}

String getAutoplayDuration() {
  return String(autoplayDuration);
}

String setAutoplayDuration(String value) {
  autoplayDuration = value.toInt();
  if (autoplayDuration < 1) autoplayDuration = 1;
  else if (autoplayDuration > 255) autoplayDuration = 255;
  autoPlayTimeout = millis() + (autoplayDuration * 1000);
  return String(autoplayDuration);
}

String getCyclePalettes() {
  return String(cyclePalettes);
}

String setCyclePalettes(String value) {
  cyclePalettes = value.toInt();
  cyclePalettes = cyclePalettes == 0 ? 0 : 1;
  paletteTimeout = millis() + (paletteDuration * 1000);
  return String(cyclePalettes);
}

String getPaletteDuration() {
  return String(paletteDuration);
}

String setPaletteDuration(String value) {
  paletteDuration = value.toInt();
  if (paletteDuration < 1) paletteDuration = 1;
  else if (paletteDuration > 255) paletteDuration = 255;
  paletteTimeout = millis() + (paletteDuration * 1000);
  return String(paletteDuration);
}

String getSolidColor() {
  return String(solidColor.r) + "," + String(solidColor.g) + "," + String(solidColor.b);
}

String setSolidColor(uint8_t r, uint8_t g, uint8_t b)
{
  solidColor = CRGB(r, g, b);

  return "\"" + String(solidColor.r) + "," + String(solidColor.g) + "," + String(solidColor.b) + "\"";
}

String setSolidColor(CRGB color) {
  return setSolidColor(color.r, color.g, color.b);
}

String setSolidColor(String value) {
  CRGB color = parseColor(value);
  
  return setSolidColor(color);
}

String getCooling() {
  return String(cooling);
}

String setCooling(String value) {
  cooling = value.toInt();
  return String(cooling);
}

String getSparking() {
  return String(sparking);
}

String setSparking(String value) {
  sparking = value.toInt();
  return String(sparking);
}

String getTwinkleSpeed() {
  return String(twinkleSpeed);
}

String setTwinkleSpeed(String value) {
  twinkleSpeed = value.toInt();
  if (twinkleSpeed < 0) twinkleSpeed = 0;
  else if (twinkleSpeed > 8) twinkleSpeed = 8;
  return String(twinkleSpeed);
}

String getTwinkleDensity() {
  return String(twinkleDensity);
}

String setTwinkleDensity(String value) {
  twinkleDensity = value.toInt();
  if (twinkleDensity < 0) twinkleDensity = 0;
  else if (twinkleDensity > 8) twinkleDensity = 8;
  return String(twinkleDensity);
}

FieldList fields = {
  { "power", "Power", BooleanFieldType, 0, 1, getPower, NULL, setPower },
  { "time24", "24 hour time", BooleanFieldType, 0, 1, getTime24, NULL, setTime24 },
  { "leadingZeroEnabled", "Show Leading Zero", BooleanFieldType, 0, 1, getLeadingZero, NULL, setLeadingZero },
  { "blinkingColonEnabled", "Blink colon", BooleanFieldType, 0, 1, getBlinkingColon, NULL, setBlinkingColon },
//  { "timezoneOffset", "Timezone Offset Hours", NumberFieldType, -12, 14, getTimezoneOffset, NULL, setTimezoneOffset },
  { "temperatureOffset", "Temperature offset", NumberFieldType, -20, 20, getTemperatureOffset, NULL, setTemperatureOffset},
  { "temperature", "Temperature", NumberFieldType, temp-1, temp+1, getTemp, NULL, setTemp},
  { "humidity", "Humidity", NumberFieldType, rh-1, rh+1, getHumid, NULL, setHumid},

  { "patternSection", "Pattern", SectionFieldType },
  { "brightness", "Brightness", NumberFieldType, 1, 255, getBrightness, NULL, setBrightness },
  { "speed", "Speed", NumberFieldType, 1, 255, getSpeed, NULL, setSpeed },  
  { "pattern", "Pattern", SelectFieldType, 0, patternCount, getPattern, getPatterns, setPattern },
  { "autoplay", "Cycle Patterns", BooleanFieldType, 0, 1, getAutoplay, NULL, setAutoplay },
  { "autoplayDuration", "Pattern Duration", NumberFieldType, 1, 255, getAutoplayDuration, NULL, setAutoplayDuration },
  
  { "paletteSection", "Palette", SectionFieldType },
  { "palette", "Palette", SelectFieldType, 0, paletteCount, getPalette, getPalettes, setPalette },
  { "cyclePalettes", "Cycle Palettes", BooleanFieldType, 0, 1, getCyclePalettes, NULL, setCyclePalettes },
  { "paletteDuration", "Palette Duration", NumberFieldType, 1, 255, getPaletteDuration, NULL, setPaletteDuration },
  
  { "solidColorSection", "Solid Color", SectionFieldType },
  { "solidColor", "Color", ColorFieldType, 0, 255, getSolidColor, NULL, setSolidColor },
  
  { "fire", "Fire & Water", SectionFieldType },
  { "cooling", "Cooling", NumberFieldType, 0, 255, getCooling, NULL, setCooling },
  { "sparking", "Sparking", NumberFieldType, 0, 255, getSparking, NULL, setSparking },
  
  { "twinklesSection", "Twinkles", SectionFieldType },
  { "twinkleSpeed", "Twinkle Speed", NumberFieldType, 0, 8, getTwinkleSpeed, NULL, setTwinkleSpeed },
  { "twinkleDensity", "Twinkle Density", NumberFieldType, 0, 8, getTwinkleDensity, NULL, setTwinkleDensity },
};

uint8_t fieldCount = ARRAY_SIZE(fields);
