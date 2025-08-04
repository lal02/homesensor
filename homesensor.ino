#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include "Adafruit_TSL2561_U.h"
#include <Wire.h>

//light sensor
//Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT);

const int MQ135Pin = 35;
const int MQ7Pin = 34;
const char WIFI_SSID[] = "IoT-WiFi";         
const char WIFI_PASSWORD[] = "HEhVy77356"; 
const String HOST_NAME   = "http://10.0.10.56:8080"; 

// humidity and temperature sensor data
#define DHTPIN 25     
#define DHTTYPE DHT22     
DHT_Unified dht(DHTPIN, DHTTYPE);

// time related constants
const char* ntpServer = "pool.ntp.org";
const long utcOffsetInSeconds = 7200; // UTC +2

// sensor reading interval
const int delayMS = 900000; // 15 minutes


void setup() {
  Serial.begin(9600);
  //setup wifi data
  Serial.println("wifi setup");
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
  //setup dht22 sensor
  Serial.println("dht22 setup");
  sensor_t sensor;
  dht.begin();
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);
  // NTP-Server synchronisieren
  configTime(utcOffsetInSeconds, 0, ntpServer);
  //setup light sensor tsl251
  //tsl.setGain(TSL2561_GAIN_1X);  // Niedrigerer Bereich, wenn der Sensor überlastet ist
  //tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  // Setze die Integrationszeit
  //Serial.println("TSL2561 initialized.");
  Serial.println("Setup finished, moving to loop");

  //mq7 setup
  pinMode(MQ7Pin,INPUT);
  pinMode(MQ135Pin,INPUT);
}

void loop() {
  //check if connection is still available
  if ((WiFi.status() != WL_CONNECTED)) {
  Serial.print(millis());
  Serial.println("Reconnecting to WiFi...");
  WiFi.disconnect();
  WiFi.reconnect();
  }
  //get current timestamp from time-server
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
  //sensors_event_t lightEvent;
  //tsl.getEvent(&lightEvent);
  //if (event.light) {
  // sendHttpRequest("/light", String(lightEvent.light), String(buffer));
  //} else {
  //  Serial.print("Sensor overload sending 0 Lux instead");
  //  sendHttpRequest("/light", "0.0", String(buffer));
 //}

  int val = analogRead(MQ135Pin);
  Serial.println(val);
  // Air quality sensor MQ135
  sendHttpRequest("/airquality",String(analogRead(MQ135Pin)),String(buffer));
  // CO sensor MQ7
  sendHttpRequest("/co",String(analogRead(MQ7Pin)),String(buffer));

  Serial.print("Waiting for ");
  Serial.print(delayMS/1000);
  Serial.println("seconds until next interval");
  delay(delayMS);
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
