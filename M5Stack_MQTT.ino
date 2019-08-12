#include <WiFiClient.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <M5Stack.h>
#include <ArduinoJson.h>
 
char *ssid = "";
char *password = "";
const char *endpoint = "";
const char *mqtt_username = "";
const char *mqtt_password = "";
const int port = 1883;
char *deviceID = "M5Stack";
char *pubTopic = "m5stack";
char *subTopic = "color";
 
////////////////////////////////////////////////////////////////////////////////
   
WiFiClient httpsClient;
PubSubClient mqttClient(httpsClient);
   
void setup() {
    Serial.begin(115200);
     
    // Initialize the M5Stack object
    M5.begin();
 
    // START
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(3);
    M5.Lcd.printf("START");
     
    // Start WiFi
    Serial.println("Connecting to ");
    Serial.print(ssid);
    WiFi.begin(ssid, password);
   
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
 
    // WiFi Connected
    Serial.println("\nWiFi Connected.");
    M5.Lcd.setCursor(10, 40);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(3);
    M5.Lcd.printf("WiFi Connected.");
     
    mqttClient.setServer(endpoint, port);
    mqttClient.setCallback(mqttCallback);
   
    connectMQTT();
}
   
void connectMQTT() {
    while (!mqttClient.connected()) {
        if (mqttClient.connect(deviceID, mqtt_username, mqtt_password)) {
            Serial.println("Connected.");
            int qos = 0;
            mqttClient.subscribe(subTopic, qos);
            Serial.println("Subscribed.");
        } else {
            Serial.print("Failed. Error state=");
            Serial.println(mqttClient.state());
            delay(5000);
        }
    }
}
   
long messageSentAt = 0;
int count = 0;
char pubMessage[128];
int led,red,green,blue;
   
void mqttCallback (char* topic, byte* payload, unsigned int length) {
 
    String str = "";
    Serial.print("Received. topic=");
    Serial.println(topic);
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
        str += (char)payload[i];
    }
    Serial.print("\n");
 
    StaticJsonBuffer<200> jsonBuffer;
     
    JsonObject& root = jsonBuffer.parseObject(str);
   
    if (!root.success()) {
      Serial.println("parseObject() failed");
      return;
    }
    const char* message = root["message"];
    led = root["led"];
    red = root["r"];
    green = root["g"];
    blue = root["b"];
 
    Serial.print("red = ");
    Serial.print(red);
    Serial.print(" green = ");
    Serial.println(green);
    Serial.print(" blue = ");
    Serial.println(blue);
 
    if( led == 1 ){
      uint16_t RGB = ((red>>3)<<11) | ((green>>2)<<5) | (blue>>3);
      M5.Lcd.fillRect(0, 0, 320, 240, RGB);
      M5.Lcd.setCursor(10, 120);
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setTextSize(5);
      M5.Lcd.printf(message);
    } else {
      M5.Lcd.fillScreen(BLACK);
    }
 
    delay(300);
     
}
  
void mqttLoop() {
    if (!mqttClient.connected()) {
        connectMQTT();
    }
    mqttClient.loop();
}
 
void loop() {
   mqttLoop();
 
  long now = millis();
  if (now - messageSentAt > 5000) {
      messageSentAt = now;
      sprintf(pubMessage, "{\"count\": %d}", count++);
      Serial.print("Publishing message to topic ");
      Serial.println(pubTopic);
      Serial.println(pubMessage);
      mqttClient.publish(pubTopic, pubMessage);
      Serial.println("Published.");
  }
}
