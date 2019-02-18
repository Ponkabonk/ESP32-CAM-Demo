Working code showing the camera, SD card and Web server all working on the ESP32-CAM board

/capture - will grab an image<br>
/read - reads state of GPIO16<br>
/write?state=ON - pulls GPIO12 up<br>
/write?state=OFF - pulls GPIO12 low<br>

This is a "no-frills" demo used only to demonstrate that it is possible to get everything working at the same time.

When first run, the code will initialize the SD card and create a file called "/config.txt" if missing.  The only option available in this demo is "deviceName".  Once the config.txt file has been created, you can pop the card into your PC and edit "deviceName".  Subsequent reboots will use the deviceName option from the card instead of the temporary, hard-coded device name.

