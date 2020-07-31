#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);
SoftwareSerial mSerial(4, 7);
StaticJsonDocument<200> doc;

int temp, humidity;
String city;

void setup() {
  Serial.begin(9600);
  mSerial.begin(2400);
  lcd.init();
  lcd.backlight();
}

void loop() {
  String json;
   while (mSerial.available()) {
   if (mSerial.available() > 0 ) {
    json = mSerial.readString();
    Serial.println(json);
   }
   DeserializationError error = deserializeJson(doc, json);
   if (error) {
    Serial.println(error.c_str());
   } else {
    Serial.println("Succes");
   }

   if (temp != doc["temp"].as<int>() || humidity != doc["humidity"] || city != doc["city"]) {
    temp = doc["temp"].as<int>();
    humidity = doc["humidity"].as<int>();
    city = doc["city"].as<String>();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temperature: ");
    lcd.print(temp);
    lcd.setCursor(0, 1);
    lcd.print("Humidity: ");
    lcd.print(humidity);
    lcd.setCursor(0, 2);
    lcd.print("City: ");
    lcd.setCursor(0, 3);
    lcd.print(city);
   }
  }
}
