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

#define HOSTNAME "ESPixelClock " ///< Hostname. The setup function adds the Chip ID at the end.

const bool apMode = true;

// AP mode password
const char WiFiAPPSK[] = "espixelclock";

void setupWifi() {
    // Set Hostname.
    String hostname(HOSTNAME);
  
    uint64_t chipid = ESP.getEfuseMac();
    uint8_t mac = (uint8_t)(chipid>>40);
    String hex = String(mac, HEX); // six octets
    hostname += hex;
  
    char hostnameChar[hostname.length() + 1];
    memset(hostnameChar, 0, hostname.length() + 1);
  
    for (uint8_t i = 0; i < hostname.length(); i++)
      hostnameChar[i] = hostname.charAt(i);
  
    WiFi.setHostname(hostnameChar); 
    // Print hostname.
    Serial.println("Hostname: " + hostname);
    IPAddress Ip(192, 168, 1, 1);
    IPAddress NMask(255, 255, 255, 0);
//    WiFi.softAPConfig(Ip, Ip, NMask);  


    Serial.println ("Trying SSID " + ssid + ": *******");// + password);
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    uint32_t timer=millis();
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi..");
      if(millis()-timer>8000){
        Serial.println("Giving up on " + String(ssid) + " after 8 seconds of trying.");
        WiFi.mode(WIFI_OFF);
        delay(100);
        WiFi.mode(WIFI_AP);
        int a=WiFi.softAP(hostnameChar, WiFiAPPSK);
        WiFi.softAPConfig(Ip, Ip, NMask);  
        Serial.print("softap returned: ");Serial.println(a);
        Serial.printf("Connect to Wi-Fi access point: %s\n", hostnameChar);
        Serial.print("and open http://");
        Serial.print(WiFi.softAPIP());
        Serial.println("/ in your browser");
        break;
      }
    }
    if (WiFi.status() == WL_CONNECTED){
      Serial.print(" CONNECTED TO: " + String(ssid) + ", IP: ");
      Serial.println(WiFi.localIP());
      
    }
    

}
