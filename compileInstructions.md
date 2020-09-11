This document is intended to describe the steps needed to compile and upload modified firmware to the ESPixelClock
Note that it is currently a work in progress, and may not be comprehensive, but should provide a good starting point at least.


Follow these instructions to install the ESP32 boards into the arduino IDE, https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md

Set the board to "ESP32 Dev module" in the Boards Menu,

install the required libraries, including (but not necessarily limited to)
- FastLED.h
- AsyncTCP.h
- ESPAsyncWebServer.h
- AsyncElegantOTA.h
- DS3231M.h
- SHT21.h

Compile code to verify changes,

Then Go to the _Sketch_ menu > and click _Export Compiled Binary_
Upload the .bin file created using the web UI

