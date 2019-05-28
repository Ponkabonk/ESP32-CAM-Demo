#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <WifiMulti.h>
#include "esp_camera.h"
#include "esp_timer.h"

// used by SDcard
#include <SD.h>
#include "FS.h"
#include "SPI.h"

// Include JSON stuff
#include <ArduinoJson.h>

WiFiMulti wifiMulti;
const char* ssid = "Ponkabonk";
const char* password = "9QX2GCY9D669MD4Y";

const int SERVER_PORT = 80;

String opt_deviceName = "DeviceName"; // just used so there is an option to save/load
const int chipSelect = 13; 
static bool hasSD = false;

#define DEBUG 1

hw_timer_t *timer = NULL;

WebServer server(SERVER_PORT);
String headerHTML = "";
String footerHTML = "";

static const size_t bufferSize = 2048;

// Camera Stuff
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
// End Camera Stuff

//static esp_err_t card_err;

void setup() {

  pinMode(16, INPUT_PULLUP);
  pinMode(12, OUTPUT);

  headerHTML = html_header();
  footerHTML = "</body></html>";   

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  SPI.begin(14, 2, 15, 13); //used by SDcard

  if (!SD.begin(chipSelect)) { 
    Serial.println("SD initializing failed!");
  } else
  {
    hasSD = true;
    Serial.println("SD initialized!");    
  }

//    uint8_t cardType = SD.cardType();
//    if (cardType == CARD_NONE) {
//      Serial.println("No SD card attached");
//      hasSD = false;      
//    }
//    Serial.print("SD Card Type: ");
//    if (cardType == CARD_MMC) {
//      Serial.println("MMC");
//    } else if (cardType == CARD_SD) {
//      Serial.println("SDSC");
//    } else if (cardType == CARD_SDHC) {
//      Serial.println("SDHC");
//    } else {
//      Serial.println("UNKNOWN");
//    }


  loadConfig();


  wifiMulti.addAP(ssid, password);  

  while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
    delay(250);
    Serial.print('.');
  }

  Serial.println("");
  Serial.print("WiFi Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  long rssi = WiFi.RSSI();
  Serial.print("RSSI:");
  Serial.println(rssi);
  
  server.on("/", HTTP_GET, handle_root);
  server.on("/write", HTTP_GET, handle_write);
  server.on("/read", HTTP_GET, handle_read);  
  server.on("/capture", HTTP_GET, handle_capture);  
  server.onNotFound(handle_NotFound);
  server.begin();
  
  Serial.print("Web Server Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");

// Camera Stuff
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
// End Camera Stuff
  
}

void loop() {
  server.handleClient();
}


void handle_root(){
    server.send(200, "text/html", headerHTML+"I wanna hang a map of the world in my house. Then I'm gonna put pins into all the locations that I've traveled to. But first, I'm gonna have to travel to the top two corners of the map so it won't fall down.<br><br>-Mitch Hedberg"+footerHTML);
}

void handle_NotFound() {
    server.send(200, "text/html", "notFound");  
}

void handle_read() {
  Serial.println("Reading GPIO 16 state.");
  String myState = getState(16);
  String reply = "GPIO: 16 = "+ myState;
  server.send(200, "text/html", reply);    
}
void handle_write(){

  Serial.println("Changing GPIO 12 state.");
  String mystate = server.arg("state");

  String reply = "Bad argument";
  if(mystate == "ON")
  {
    digitalWrite(12, HIGH);
    reply = "OK";
    
  } else if(mystate == "OFF")
  {
    digitalWrite(12, LOW);      
    reply = "OK";      
  }
  
  server.send(200, "text/html", reply);     

}


String getState(int gpio)
{
    int inputState = digitalRead(gpio);
    String state = "";

    if(inputState == 0)
    {
      state = "0";
    } else 
    {
      state = "1";      
    }

  return state;
  
}

esp_err_t handle_capture(){
  if(DEBUG == 1) {Serial.println("grabbing image");}
  String secret = server.arg("s");

    WiFiClient client = server.client();
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    int64_t fr_start = esp_timer_get_time();
    fb = esp_camera_fb_get();
    int64_t fr_end = esp_timer_get_time();

    if (!fb) {
        if(DEBUG == 1) {Serial.println("Camera capture failed");}
        String response = "HTTP/1.1 500 OK\r\n";
        server.sendContent(response);        
        return ESP_FAIL;
    }

    if(DEBUG == 1) {Serial.printf("Image Captured: %uB %ums\n", (uint32_t)(fb->len), (uint32_t)((fr_end - fr_start)/1000));}

    size_t fb_len = 0;
    fb_len = fb->len;
//    Serial.println("fb_len="+(String)fb_len);
    char buf[fb_len+1];

  static uint8_t buffer[bufferSize] = {0xFF};
  size_t len = fb_len;
  if (!client.connected()) {return ESP_ERR_INVALID_STATE;}
  
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: image/jpeg\r\n";
  response += "Content-Length: " + String(len) + "\r\n\r\n";
  server.sendContent(response);
    size_t copied = 0;
  while (len) {
    size_t will_copy = (len < bufferSize) ? len : bufferSize;
    memcpy(buffer, (fb->buf+copied), will_copy);
    if (!client.connected()) {
      return ESP_ERR_INVALID_STATE;
    }
    client.write(&buffer[0], will_copy);
    copied+=will_copy;
    len -= will_copy;
  }
    client.stop();  
  if(DEBUG == 1) {Serial.println("Image sent. "+String(copied)+" copied.");}

// uncomment following line to save image to SD card
  //save_image_to_SD(fb);
  esp_camera_fb_return(fb);   
  return ESP_OK;
}


bool save_image_to_SD(camera_fb_t* fb)
{
  if(!hasSD) {return 0;}
    
  File imageFile;
  imageFile = SD.open("/capture.jpg", FILE_WRITE);
  imageFile.write(fb->buf, fb->len);
  imageFile.close();
  
  return 1;
}

String html_header()
{
  String header = "<!DOCTYPE html><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><meta name=\"viewport\" content=\"width=device-width, maximum-scale=1, initial-scale=1\"><title>Hello!</title>";
  header += "</head>";
  return header;
}




bool loadConfig()
{
  if(!hasSD) return 0;
  Serial.println("Loading configuration.");
//  StaticJsonBuffer<1024> jsonBuffer;
 
  if (SD.exists("/config.txt")) {
    Serial.println("Config file found.");
    char buff[1024];
    File configFile = SD.open("/config.txt", FILE_READ);    
    int i =0;
    while(configFile.available())
    {
      buff[i]=configFile.read();
      i++;
    }
    buff[i] = 0;
    String str(buff);
    Serial.println(buff);
//    JsonObject& jObject = jsonBuffer.parseObject(buff);

    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, buff);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
    }

//    opt_deviceName = jObject["deviceName"].asString();    
    opt_deviceName = doc["deviceName"].as<String>();
    Serial.println("deviceName is: "+opt_deviceName);   
    return(1);    
  } else 
  {
    Serial.println("Making new config file!");   
    saveConfig(); 
    return(0);    
  }
 
}

void saveConfig()
{
  if(!hasSD) return;
  String json = "{";
  json += "\"deviceName\":\""+opt_deviceName+"\"";
  json += "}";
  
  Serial.println(json);

  SD.remove("/config.txt");
  File configFile = SD.open("/config.txt", FILE_WRITE);
  configFile.seek(0);
  configFile.println(json);
  configFile.close();
  Serial.println("Configuration saved.");      
  
  }
