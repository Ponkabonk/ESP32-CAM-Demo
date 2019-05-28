# ESP32-CAM-Demo

Files:
ESP32-CAM_SD_Cam_WebServer_Demo.ino - shows that it is possible to use the camera, SD card and a WebServer at the same time.

You CAN program the ESP32-CAM using the Arduino IDE.  Here's how:

Board: ESP32 Wrover Module

1. Configure TTL converter (see below)
2. Remove power.
3. Short pin IO0 (zero) to ground
4. Apply power
5. Upload to card.
6. When IDE tells you to, remove power, remove short on to ground on IO0 and re-apply power.

GND -> GND<br>
CTS -> NC<br>
VCC -> VCC (+5vDC) (I just get this from the USB connector.  No additional power is required.)<br>
TXD -> U0R<br>
RXD -> U0T<br>
DTR -> NC<br>

Please keep in mind that this is demo, proof of concept code only.  You'll need to harden it before using in production.
