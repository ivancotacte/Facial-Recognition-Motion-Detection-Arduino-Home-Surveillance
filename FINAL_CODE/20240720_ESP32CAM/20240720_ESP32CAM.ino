#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h>

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

char ssid[32];
char password[32];

String serverName = "esp-32-cam-server.replit.app";
String uploadServerPath = "/upload";
String emailServerPath = "/sendalerts";

const int serverPort = 443;
bool usingSendPhotoToServer = false;

const int txPin = 13;
const int rxPin = 12;

bool matchFaceActive = false;
bool matchFace = false;
unsigned long matchFaceprevMillis = 0;
const int matchFaceinterval = 2000;

bool intruderFaceActive = false;
bool intruderFace = false;
unsigned long intruderPrevMillis = 0;
const int intruderInterval = 2000;

WiFiClientSecure client;
WiFiManager wifiManager;

void startCameraServer();

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, rxPin, txPin);
  Serial.setDebugOutput(true);
  Serial.println();

  wifiManager.autoConnect("Group1 - AutoConnectAP");

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

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);
    s->set_brightness(s, 1);
    s->set_saturation(s, -2);
  }
  s->set_framesize(s, FRAMESIZE_HQVGA);
  s->set_vflip(s, 1);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");

  sendTextAlert("ESP32CAM: Your IP Address: " + WiFi.localIP().toString(), "http://" +WiFi.localIP().toString());
}

void loop() {
  if (matchFace && !matchFaceActive) {
    matchFaceActive = true;
    Serial2.println("MATCHFACE");

    matchFaceprevMillis = millis();
  }

  if (matchFaceActive && millis() - matchFaceprevMillis > matchFaceinterval) {
    matchFaceActive = false; 
    matchFace = false;
  }

  if (!usingSendPhotoToServer) {
    if (intruderFace && !intruderFaceActive) {
      intruderFaceActive = true;
      takePicture("IntruderAlert");
      intruderPrevMillis = millis();
    }

    if (intruderFaceActive && millis() - intruderPrevMillis > intruderInterval) {
      intruderFaceActive = false;
      intruderFace = false;
    }
  }
}

String takePicture(String fileName) {
  String responseHeaders;
  String responseBody;

  bool usingSendPhotoToServer = true;

  camera_fb_t * fb = esp_camera_fb_get();
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }
  
  Serial.println("Connecting to server: " + serverName);
  client.setCACert(nullptr);
  if (client.connect(serverName.c_str(), serverPort)) {
    Serial.println("Connection successful!");    
    String boundary = "--MK--";
    String head = "--" + boundary + "\r\nContent-Disposition: form-data; name=\"image\"; filename=\"" + fileName + ".jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--" + boundary + "--\r\n";

    uint32_t imageLen = fb->len;
    uint32_t totalLen = imageLen + head.length() + tail.length();
  
    Serial.println("Sending Image...");

    client.println("POST " + uploadServerPath + " HTTP/1.1");
    client.println("Host: " + serverName);
    client.println("Content-Length: " + String(totalLen));
    client.println("Content-Type: multipart/form-data; boundary=" + boundary);
    client.println();
    client.print(head);
  
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    size_t bufferSize = 1024;
    for (size_t n = 0; n < fbLen; n += bufferSize) {
      size_t remaining = fbLen - n;
      size_t chunkSize = remaining < bufferSize ? remaining : bufferSize;
      client.write(fbBuf + n, chunkSize);
    }   
    client.print(tail);
    
    esp_camera_fb_return(fb);
    
    int timoutTimer = 10000;
    long startTimer = millis();
    boolean state = false;
    
    while ((startTimer + timoutTimer) > millis()) {
      Serial.print(".");
      delay(100);      
      while (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (responseHeaders.length() == 0) { state = true; }
          responseHeaders = "";
        }
        else if (c != '\r') { responseHeaders += String(c); }
        if (state == true) { responseBody += String(c); }
        startTimer = millis();
      }
      if (responseBody.length() > 0) { break; }
    }
    Serial.println();
    client.stop();
    Serial.println(responseBody);

    bool usingSendPhotoToServer = false;
  } else {
    responseBody = "Connection to " + serverName +  " failed.";
    Serial.println(responseBody);

    bool usingSendPhotoToServer = false;
  }
  return responseBody;
}

void sendTextAlert(String subject, String message) {
  String responseHeaders;
  String responseBody;

  Serial.println("Connecting to server: " + serverName);
  client.setCACert(nullptr);
  if (client.connect(serverName.c_str(), serverPort)) {
    Serial.println("Connection successful!");

    String postData = "subject=" + subject + "&message=" + message;

    client.println("POST " + emailServerPath + " HTTP/1.1");
    client.println("Host: " + serverName);
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Content-Length: " + String(postData.length()));
    client.println();
    client.print(postData);

    int timoutTimer = 10000;
    long startTimer = millis();
    boolean state = false;

    while ((startTimer + timoutTimer) > millis()) {
      Serial.print(".");
      delay(100);      
      while (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (responseHeaders.length() == 0) { state = true; }
          responseHeaders = "";
        }
        else if (c != '\r') { responseHeaders += String(c); }
        if (state == true) { responseBody += String(c); }
        startTimer = millis();
      }
      if (responseBody.length() > 0) { break; }
    }
    Serial.println();
    client.stop();
    Serial.println(responseBody);
  } else {
    responseBody = "Connection to " + serverName +  " failed.";
    Serial.println(responseBody);
  }
}