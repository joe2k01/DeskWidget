#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include "config.h"

SoftwareSerial mSerial(0, 2);
StaticJsonDocument<JSON_OBJECT_SIZE(3)> doc;
char url[200];

void setup() {
  sprintf(url, "http://api.openweathermap.org/data/2.5/weather?q=Cagliari&APPID=%s", APPID);
  WiFi.begin(APNAME, PWD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  mSerial.begin(2400);
  Serial.begin(9600);
  HTTPClient http;
    http.begin(url);
    if (http.GET() == 200) {
      DynamicJsonDocument weather(1200);
      deserializeJson(weather, http.getString());
      
      int temp = weather["main"]["temp"].as<int>() / 10;
      int humidity = weather["main"]["humidity"].as<int>();
      auto location = weather["name"].as<const char*>();

      doc["temp"] = temp;
      doc["humidity"] = humidity;
      doc["city"] = location;
    }
}

void loop() {
  serializeJson(doc, mSerial);
  serializeJson(doc, Serial);
  delay(1000);
}
