#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "config.h"

#define MINUTE 60 * 1000L

ESP8266WebServer server(80);
HTTPClient http;
WiFiUDP ntpUDP;

char url[200];
char mLoc[100] = "";
unsigned long elapsed = 0;
int addrOffset = 0;

void configurationSetUp();

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

    NTPClient timeClient(ntpUDP, weather["timezone"].as<long>());
    timeClient.begin();
    timeClient.update();

    long epoch = timeClient.getEpochTime();

    timeClient.end();

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
       message is a char array of this kind: 1199{1596796182}10d#City@
       11: are temperature digits
       99: are humidity digits
       {: marks the start of the unix time
       1596796182: unix time
       }: marks the end of the unix time
       10d: icon identifier
       #: marks the start of the location name
       City: is the location name
       @: marks the end of the location name
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
    int addresses = location.length();
    EEPROM.write(addrOffset, addresses);
    int j = addrOffset;
    for (int i = 0; i < addresses; i++) {
      j++;
      EEPROM.write(j, locationBuffer[i]);
    }
    EEPROM.commit();
    fetchWeather(location);
  }
  server.send(200, "text/html", HTML);
}

void handleConfigurationRoot() {
  if (server.hasArg("ssid") && server.hasArg("pwd")) {
    for (int i = 0; i < 513; i++) { // Wipe the EEPROM
      EEPROM.write(i, -1);
    }
    EEPROM.commit();
    memset(mLoc, '\0', 100); // Clear the location variable

    String ssid = server.arg("ssid");
    char ssidBuffer[100];
    ssid.toCharArray(ssidBuffer, 100);
    int ssidAddresses = ssid.length();

    String pwd = server.arg("pwd");
    char pwdBuffer[100];
    pwd.toCharArray(pwdBuffer, 100);
    int pwdAddresses = pwd.length();

    EEPROM.write(0, ssidAddresses);
    for (int i = 0; i < ssidAddresses; i++) {
      EEPROM.write(i + 1, ssidBuffer[i]);
    }
    int j = ssidAddresses + 1;
    EEPROM.write(j, pwdAddresses);
    for (int k = 0; k < pwdAddresses; k++) {
      j++;
      EEPROM.write(j, pwdBuffer[k]);
    }
    EEPROM.commit();
    server.send(200, "text/plain", "Your DeskWidget will now connect to the network");
    addrOffset = j + 1;
    delay(200);

    WiFi.softAPdisconnect(true);
    WiFi.disconnect(true);

    int n = WiFi.scanNetworks();
    boolean networkAvailable = false;
    for (int i = 0; i < n; i++) {
      if (WiFi.SSID(i) == ssid)
        networkAvailable = true;
      if (networkAvailable)
        break;
    }

    if (networkAvailable) {
      WiFi.begin(ssid, pwd);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
      }

      server.on("/", handleRoot);
      server.begin();

      char ip[16];
      WiFi.localIP().toString().toCharArray(ip, 16);
      strcat(ip, "@");
      Serial.write(ip);
    } else {
      configurationSetUp();
    }
  }
  server.send(200, "text/html", CHTML);
}

int readEEPROM(int a, char buff[]) {
  int addresses = EEPROM.read(a);
  if (addresses > 0 && addresses <= 100) {
    a++;
    for (int i = 0; i < addresses; i++) {
      buff[i] = EEPROM.read(a + i);
    }
    buff[addresses] = '\0';
    return addresses;
  }
  return -1;
}

void configurationSetUp() {
  char softSSID[22] = "DeskWidgetSetUp-";
  char randomN[5];

  sprintf(randomN, "%d", random(1000, 9999));
  strcat(softSSID, randomN);

  WiFi.softAP(softSSID, PWD, 1, false, 1); // Create a network to which the user will connect to configure the DeskWidget

  server.on("/c", handleConfigurationRoot);
  server.begin();

  strcat(softSSID, "@");
  Serial.write(softSSID);
}

void setup() {
  WiFi.softAPdisconnect(true); // Turn off soft AP mode in case configuration wa run before
  EEPROM.begin(512);
  Serial.begin(9600);

  char ssid[100];
  int a = readEEPROM(0, ssid); // Read the AccessPoint's name saved in EEPROM

  char pwd[100]; // Read the AccessPoint's password saved in EEPROM
  int p = readEEPROM(a + 1, pwd);
  if (a > 0 && p > 0) {
    addrOffset = p + a + 2; // Set an offset not to overwrite AccessPoint credentials in EEPROM

    int n = WiFi.scanNetworks();
    boolean networkAvailable = false;
    for (int i = 0; i < n; i++) {
      char buff[100];
      String mSSID = WiFi.SSID(i);
      mSSID.toCharArray(buff, 100);
      networkAvailable = (strcmp(buff, ssid) == 0);
      if (networkAvailable)
        break;
    }

    if (networkAvailable) {
      WiFi.begin(ssid, pwd);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
      }

      delay(2000);

      server.on("/", handleRoot);
      server.on("/c", handleConfigurationRoot);
      server.begin();

      int f = readEEPROM(addrOffset, mLoc);
      if (f > 0)
        fetchWeather(String(mLoc));
      else {
        char ip[16];
        WiFi.localIP().toString().toCharArray(ip, 16);
        strcat(ip, "@");
        Serial.write(ip);
      }
    } else {
      configurationSetUp();
    }
  } else {
    configurationSetUp();
  }
}

void loop() {
  server.handleClient();

  if (addrOffset > 0 && mLoc[0] != '\0') {
    if ((millis() - elapsed) >= MINUTE) {
      elapsed += MINUTE;
      fetchWeather(mLoc);
    } else if (millis() < elapsed) {
      elapsed = 0L;
      fetchWeather(mLoc);
    }
  }
}
