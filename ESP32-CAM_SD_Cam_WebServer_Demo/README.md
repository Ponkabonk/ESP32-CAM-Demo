Working code showing the camera, SD card and Web server all working on the ESP32-CAM board

/capture - will grab an image<br>
/read - reads state of GPIO16<br>
/write?state=ON - pulls GPIO12 up<br>
/write?state=OFF - pulls GPIO12 low<br>

This is a "no-frills" demo used only to demonstrate that it is possible to get everything working at the same time.

When first run, the code will initialize the SD card and create a file called "/config.txt" if missing.  The only option available in this demo is "deviceName".  Once the config.txt file has been created, you can pop the card into your PC and edit "deviceName".  Subsequent reboots will use the deviceName option from the card instead of the temporary, hard-coded device name.

FYI, using the SD card will tie up GPIOs 2, 13, 14 and 15 making them unavailable for other uses.  These connections are made internal to the ESP32-CAM so no external connections are required.  Keep in mind that with the serial connector in use you really only have three GPIOs available: 4 (the on-board LED), 12 and 16
