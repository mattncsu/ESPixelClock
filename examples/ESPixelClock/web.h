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

//#include "compressed_html.h"
//#include "combined_html.h"

String processor(const String& var){
  Serial.print(var+": ");
 char dateStr[13];
 char timeStr[10];
  sprintf(dateStr,"%04d-%02d-%02d", now.year(),now.month(), now.day());
  sprintf(timeStr,"%02d:%02d:%02d",now.hour(), now.minute(), now.second());
  
  if(var == "WIFIAP"){Serial.println(ssid);return String(ssid);}
  if(var == "PASSWORD"){Serial.println("*****");return String(password);} 
  if(var == "NTPSERVER1"){Serial.println(ntpServer1);return String(ntpServer1);}
  if(var == "NTPSERVER2"){Serial.println(ntpServer2);return String(ntpServer2);}
  if(var == "TEMPERATURE"){Serial.println(temp);return String(temp);}
  if(var == "HUMIDITY"){Serial.println(rh);return String(rh);}
  if(var == "DATE"){Serial.println(dateStr);return String(dateStr);}
  if(var == "TIME"){Serial.println(timeStr);return String(timeStr);}
  if(var == "TIMEZONE"){Serial.println(tzInfo);return String(tzInfo);}
  return String(); //Return empty string if no matches
}



void setupWeb() {
//ASYNC
  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  server.on("/all", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("GET: /all");
    String json = getFieldsJson(fields, fieldCount);
    request->send(200, "text/json", json);
  });

  server.on("/fieldValue", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("GET: /fieldValue");
    String name = request->getParam(0)->name();
    String value = getFieldValue(name, fields, fieldCount);
    request->send(200, "text/json", value);
  });

  server.on("/fonts/glyphicons-halflings-regular.woff2", [](AsyncWebServerRequest *request) {
    request->redirect("http://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/fonts/glyphicons-halflings-regular.woff2");
    request->send(301);
  });

  server.on("/fonts/glyphicons-halflings-regular.woff", [](AsyncWebServerRequest *request) {
    request->redirect("http://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/fonts/glyphicons-halflings-regular.woff");
    request->send(301);//, "text/html", "");
  });
  
  server.on("/fonts/glyphicons-halflings-regular.ttf", [](AsyncWebServerRequest *request) {
    request->redirect("http://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/fonts/glyphicons-halflings-regular.ttf");
    request->send(301);
  });

  server.on("/css/bootstrap.min.css.map", [](AsyncWebServerRequest *request) {
    request->redirect("http://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap.min.css.map");
    request->send(301);//, "text/html", "");
  });
  
  server.on("/css/jquery.minicolors.min.css.map", [](AsyncWebServerRequest *request) {
    request->redirect("https://cdnjs.cloudflare.com/ajax/libs/jquery-minicolors/2.2.4/jquery.minicolors.min.css.map");
    request->send(301);
  });
  

  server.on("/fieldValue", HTTP_POST, [](AsyncWebServerRequest *request) {
    Serial.println("ASYNC POST: /fieldValue");
    int paramsCount = request->params();
    for(int i = 0; i< paramsCount; i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isFile()){ //p->isPost() is also true
        Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      } else if(p->isPost()){
        Serial.printf("POST %d[%s]: %s\n", i,p->name().c_str(), p->value().c_str());
      } else {
        Serial.printf("GET %d [%s]: %s\n", i,p->name().c_str(), p->value().c_str());
      }
    }
    String name = request->getParam(0)->value();
    String params,newValue;
    
    if (name == "solidColor"){
      String r,g,b;
      r = request->getParam(2)->value();
      g = request->getParam(3)->value();
      b = request->getParam(4)->value();
      Serial.println("RGB:" + r +" "+ g +" "+ b);
      params=r+","+g+","+b; //sprintf("%s,%s,%s",r,g,b);
      newValue = setFieldValueAsync(name, params, fields, fieldCount);
    } 
    else {
      params = request->getParam(1)->value();
      newValue = setFieldValueAsync(name, params, fields, fieldCount);
    }

    request->send(200, "text/json", newValue);
  });

  server.on("/wifi.htm", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("/wifi.htm");
    request->send(SPIFFS, "/wifi.htm", String(), false, processor);
  });

  server.on("/WIFIsubmit", HTTP_POST, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Saved! Rebooting...");
    preferences.begin("ESPixelClock",false);
    String recvDate, recvTime;
    for (int i = 0; i < request->params(); i++) {
      String name = request->getParam(i)->name();
      String params = request->getParam(i)->value();
      Serial.println(name + " : " + params);
      if (name == "WIFI_AP") {
        Serial.println("Saving Wifi creds");
        ssid = params;
        preferences.putString("ssid",ssid);
        
      } else if (name == "WIFI_PASS") {
        password = params;
        preferences.putString("password",password);
      } else if (name == "NTP_SERVER1") {
        preferences.putString("ntpServer1",params);
      } else if (name == "NTP_SERVER2") {
        preferences.putString("ntpServer2",params);
      } else if (name == "TIMEZONE") {
        tzInfo = params;
        struct tm recvTime;
        preferences.putString("timezone",tzInfo);
        Serial.print("Tz selected: ");Serial.println(params);
      } else if (name == "DATE") {
        recvDate=params;
      } else if (name == "TIME") {
        recvTime=params;
      }
    }
    preferences.end();
    char buf[20];
    strcpy(buf,(recvDate + " " + recvTime).c_str());
    Serial.println(buf);
    struct tm timeinfo;

    if (strptime(buf, "%Y-%m-%d %T",&timeinfo) == NULL)
          Serial.printf("\nstrptime failed\n");
    else
    {
      Serial.println(&timeinfo, "Submitted time: %A, %B %d %Y %H:%M:%S");
      int hour = timeinfo.tm_hour;
      int minute  = timeinfo.tm_min;
      int second  = timeinfo.tm_sec;
      int day = timeinfo.tm_mday;
      int month = timeinfo.tm_mon + 1;
      int year = timeinfo.tm_year +1900;
      DS3231M.adjust(DateTime(year,month,day,hour,minute,second));      // Adjust the RTC date/time
      Serial.print(F("RTC time/date has been set.\n")); 
    }
    Serial.print(F("Preferences updated, rebooting...\n\n\n\n\n"));
    delay(1000);
    ESP.restart();
  });


//  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.htm").setTemplateProcessor(processor).setCacheControl("max-age=86400");
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.htm").setCacheControl("max-age=86400");

  server.onNotFound([](AsyncWebServerRequest *request){
    Serial.printf("NOT_FOUND: ");
    if(request->method() == HTTP_GET)
      Serial.printf("GET");
    else if(request->method() == HTTP_POST)
      Serial.printf("POST");
    else if(request->method() == HTTP_DELETE)
      Serial.printf("DELETE");
    else if(request->method() == HTTP_PUT)
      Serial.printf("PUT");
    else if(request->method() == HTTP_PATCH)
      Serial.printf("PATCH");
    else if(request->method() == HTTP_HEAD)
      Serial.printf("HEAD");
    else if(request->method() == HTTP_OPTIONS)
      Serial.printf("OPTIONS");
    else
      Serial.printf("UNKNOWN");
    Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

    if(request->contentLength()){
      Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
      Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
    }

    int headers = request->headers();
    int i;
    for(i=0;i<headers;i++){
      AsyncWebHeader* h = request->getHeader(i);
      Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    }

    int params = request->params();
    for(i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isFile()){
        Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      } else if(p->isPost()){
        Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      } else {
        Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }

    request->send(404);
  });

  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    if(!index)
      Serial.printf("BodyStart: %u\n", total);
    Serial.printf("%s", (const char*)data);
    if(index + len == total)
      Serial.printf("BodyEnd: %u\n", total);
  });
  server.begin();

}

//void handleWeb() {
//  static bool webServerStarted = false;
//  esp_wifi_ap_get_sta_list(&stationList);
//  // check for connection
//  if ( WiFi.status() == WL_CONNECTED || stationList.num > 0) {
//    if (!webServerStarted) {
//      // turn off the board's LED when connected to wifi
//
//      webServerStarted = true;
//      setupWeb();
//      
//      //init and get the time
////      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2); 
//    }
//    
//  } else {
//    // blink the board's LED while connecting to wifi
//
//  }
//}
