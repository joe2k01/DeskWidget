#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include "config.h"

SoftwareSerial mSerial(0, 2);
StaticJsonDocument<JSON_OBJECT_SIZE(3)> doc;
ESP8266WebServer server(80);
HTTPClient http;
char url[200];

void fetchWeather(String location) {
  char locationChar[100];
  location.toCharArray(locationChar, 100);
  sprintf(url, "http://api.openweathermap.org/data/2.5/weather?q=%s&APPID=%s", locationChar, APPID);
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
  http.end();
  serializeJson(doc, mSerial);
  serializeJson(doc, Serial);
}

void handleRoot() {
  if (server.hasArg("location")) {
    String location = server.arg("location");
    location.replace(" ", "+");
    fetchWeather(location);
  }
  server.send(200, "text/html", HTML);
}

void setup() {
  WiFi.begin(APNAME, PWD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  mSerial.begin(2400);
  Serial.begin(9600);
  Serial.println(WiFi.localIP());
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
  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();
}
