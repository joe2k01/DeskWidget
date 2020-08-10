#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include "config.h"

#define MINUTE 60 * 1000L

ESP8266WebServer server(80);
HTTPClient http;
char url[200];
char mLoc[100];
unsigned long elapsed = 0;

void fetchWeather(String location) {
  char locationChar[100];
  location.toCharArray(locationChar, 100);

  memset(mLoc, '\0', 100);
  strcpy(mLoc, locationChar);

  sprintf(url, "http://api.openweathermap.org/data/2.5/weather?q=%s&APPID=%s", locationChar, APPID);
  http.begin(url);
  if (http.GET() == 200) {
    DynamicJsonDocument weather(1200);
    deserializeJson(weather, http.getString());

    int temp = weather["main"]["temp"].as<int>() / 10;
    int humidity = weather["main"]["humidity"];
    auto locationName = weather["name"].as<const char*>();
    long epoch = weather["dt"].as<long>() + weather["timezone"].as<long>();

    char icon[4];
    JsonArray weatherArray = weather["weather"].as<JsonArray>();
    for (JsonVariant v : weatherArray) {
      JsonObject o = v.as<JsonObject>();
      auto tmpIcon = o["icon"].as<const char*>();
      strcpy(icon, tmpIcon);
      icon[3] = '\0';
    }

    int messageSize = location.length() + 6;
    char message[messageSize];
    /*
     * message is a char array of this kind: 1199{1596796182}10d#City@
     * 11: are temperature digits
     * 99: are humidity digits
     * {: marks the start of the unix time
     * 1596796182: unix time
     * }: marks the end of the unix time
     * 10d: icon identifier
     * #: marks the start of the location name
     * City: is the location name 
     * @: marks the end of the location name
     */
    sprintf(message, "%d%d{%ld}%s#%s@", temp, humidity, epoch, icon, locationName);
    Serial.write(message);
  }
  http.end();
}

void handleRoot() {
  if (server.hasArg("location")) {
    String location = server.arg("location");
    location.replace(" ", "+");
    char locationBuffer[100];
    location.toCharArray(locationBuffer, 100);
    int addresses = location.length() + 1;
    EEPROM.write(0, addresses);
    for (int i = 1; i < addresses; i++) {
      EEPROM.write(i, locationBuffer[i - 1]);
    }
    EEPROM.commit();
    fetchWeather(location);
  }
  server.send(200, "text/html", HTML);
}

void setup() {
  WiFi.begin(APNAME, PWD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.begin(9600);
  // Serial.println(WiFi.localIP());

  delay(2000);

  EEPROM.begin(512);
  int addresses = EEPROM.read(0);
  if (addresses > 0) {
    for (int i = 1; i < addresses; i++) {
      mLoc[i - 1] = EEPROM.read(i);
    }
    mLoc[addresses - 1] = '\0';
    fetchWeather(String(mLoc));
  }

  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();

  if ((millis() - elapsed) >= MINUTE) {
    elapsed += MINUTE;
    fetchWeather(mLoc);
  } else if (millis() < elapsed) {
    elapsed = 0L;
    fetchWeather(mLoc);
  }
}
