#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2561_U.h"

#include <DHT.h>
#include <DHT_U.h>

#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

#include <time.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>


const int MQ135Pin = 35;
const int MQ7Pin = 34; 
const char WIFI_SSID[] = "IoT-WiFi";         
const char WIFI_PASSWORD[] = "HEhVy77356"; 
const String HOST_NAME   = "http://10.0.10.56:8080"; 
const char* ntpServer = "pool.ntp.org";
const long utcOffsetInSeconds = 7200; // UTC +2
const int interval = 900000; // 15 minutes
unsigned long previousMillis = 0;
unsigned long currentMillis;
const int buttonPin = 26;
int lastButtonState = HIGH;
#define DHTPIN 25     
#define DHTTYPE DHT22     
DHT_Unified dht(DHTPIN, DHTTYPE);
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT);
WebServer server(80);
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
    Serial.println("WiFi not connected!");
    delay(1000);
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; i++) {
      Serial.println(WiFi.SSID(i));
    }
  }
  const String ipaddr = WiFi.localIP().toString().c_str();
  Serial.println("WiFi setup finished");
  
  //setup dht22 sensor
  Serial.println("dht22 setup");
  sensor_t sensor;
  dht.begin();
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);
  Serial.println("dht22 setup finished");
  // NTP-Server synchronisieren
  configTime(utcOffsetInSeconds, 0, ntpServer);
  Serial.println("time config finished");
  
  // setup light sensor tsl251
  tsl.setGain(TSL2561_GAIN_1X);
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);
  Serial.println("TSL2561 setup finished");

  pinMode(MQ7Pin,INPUT);
  Serial.println("mq7 setup finished");
  pinMode(MQ135Pin,INPUT);
  Serial.println("mq135 setup finished");
  pinMode(buttonPin,INPUT);
  Serial.println("touch button setup finished");
  
  server.on("/read", serverTriggerRead);
  server.on("/",hardwareInfoOutput);
  server.begin();
  Serial.println("server setup finished");

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  const String firstrow = "IP:" + ipaddr + ":80";
  lcd.print(firstrow);  
  lcd.setCursor(0, 1);
  lcd.print("Next in:");
  Serial.println("lcd setup finished");
    
}

void loop() {
  server.handleClient();
  // 15 minute interval reading
  currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    serverTriggerRead();
    previousMillis = currentMillis;
  }
  // remainingTime in seconds
  unsigned int remainingTime = (interval/1000) - (currentMillis-previousMillis)/1000;
  printRemainingTime(remainingTime);
 
  // button press triggered reading
  int buttonState = digitalRead(buttonPin);
  if (buttonState == LOW && lastButtonState == HIGH) {
    Serial.println("Button pressed");
    serverTriggerRead();
  }
  lastButtonState = buttonState;
  delay(50);
}

void serverTriggerRead(){
  Serial.println("http server triggered, reading and processing data");
  readAndSendData();
  previousMillis=currentMillis;
  server.send(200, "text/plain", "OK");
}

void readAndSendData(){
  if ((WiFi.status() != WL_CONNECTED)) {
  Serial.print(millis());
  Serial.println("Reconnecting to WiFi...");
  WiFi.disconnect();
  WiFi.reconnect();
  }
  //get current timestamp from server
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  char buffer[20]; 
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &timeinfo);
  Serial.println("current timestamp : " + String(buffer));
  //read temperature data from dht22 sensor
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    sendHttpRequest("/temperature", String(event.temperature), String(buffer));
  }
  // read humidity data from dht22 sensor
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    Serial.print(event.relative_humidity);
    sendHttpRequest("/humidity", String(event.relative_humidity), String(buffer));
  }
  // read light data from TSL2561 sensor
  sensors_event_t lightEvent;
  tsl.getEvent(&lightEvent);
  if (event.light) {
   sendHttpRequest("/light", String(lightEvent.light), String(buffer));
  } else {
    Serial.print("Sensor overload sending 0 Lux instead");
    sendHttpRequest("/light", "0.0", String(buffer));
 }
  // Air quality sensor MQ135
  sendHttpRequest("/airquality",String(analogRead(MQ135Pin)),String(buffer));
  // CO sensor MQ7
  sendHttpRequest("/co",String(analogRead(MQ7Pin)),String(buffer));
}


void sendHttpRequest(String URL, String value,String timestamp){
  HTTPClient http;
  http.begin(HOST_NAME + URL);
  http.addHeader("Content-Type", "application/json");
  String payload = "{";
  payload += "\"value\":\"" + value + "\",";
  payload += "\"timestamp\":\"" + timestamp + "\"";
  payload += "}";
  int httpCode = http.POST(payload);
  http.end();
  Serial.print("HTTP Return Code of request to : ");
  Serial.print (" " + URL);
  Serial.print(" with value: " + value + " and timestamp " + timestamp);
  Serial.println(httpCode);
}

String formatNumber(int number, int width) {
  String result = String(number);
  while (result.length() < width) {
    result = "0" + result;
  }
  return result;
}

void printRemainingTime(int seconds){
  lcd.setCursor(9, 1);
  lcd.print(formatNumber(seconds/60,2));
  lcd.setCursor(11,1);
  lcd.print("m ");
  lcd.setCursor(12,1);
  lcd.print(formatNumber(seconds%60,2));
  lcd.setCursor(14,1);
  lcd.print("s");
}

void hardwareInfoOutput(){
    String html = "<!DOCTYPE HTML>";
    html += "<html>";
    html += "<head>";
    html += "<link rel=\"icon\" href=\"data:,\">";
    html += "</head>";
    html += "<h4> Hardware Info </h4> ";
    html += "Host-Name: " + String(WiFi.getHostname())+ " <br>";
    html += "Connected WiFi-SSID: " + String(WiFi.SSID()) + "<br>";
    html += "IP-Adress: " + String(WiFi.localIP().toString().c_str()) + " <br>";
    html += "MAC-Adress: " + String(WiFi.macAddress()) + " <br";
    html += "</p>";
    html += "<h4> Sensor Info </h4> ";
    html += "DHT22: Temperature and Humidity Sensor <br>";
    html += "TSL2561: Luminosity / Lux Calculation Sensor <br>";
    html += "MQ7: Carbon Monoxide Sensor <br>";
    html += "MQ35: Gas Sensor for Air Quality <br>";
    html += "</html>";
    server.send(200, "text/html", html);
}
