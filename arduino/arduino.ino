#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <TimeLib.h>

#include "icons.h"
#include "degree.h"

#define W 100 // Icon Width
#define H 100 // Icon Height

#define MAX_DISPLAY_BUFFER_SIZE 800
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))

GxEPD2_BW<GxEPD2_420, MAX_HEIGHT(GxEPD2_420)> display(GxEPD2_420(/*CS*/ 10, /*DC=*/ 9, /*RST=*/ 8, /*BUSY=*/ 7));

uint16_t textHeight;

void displaySetUp(bool initial) {
  display.init();
  display.setRotation(0);
  display.setFont(&FreeMonoBold12pt7b);
  display.setTextColor(GxEPD_BLACK);

  if (initial) {
    display.setFullWindow();
    char retr[] = "Retrieving Data";
    int16_t a, b; uint16_t c;
    display.getTextBounds(retr, 0, 0, &a, &b, &c, &textHeight);
    textHeight += 7;

    display.firstPage();
    do {
      display.fillScreen(GxEPD_WHITE);
      display.setCursor(0, textHeight);
      display.print(retr);
    } while (display.nextPage());
  }
}

void setup() {
  Serial.begin(9600);
  displaySetUp(true);
}

void loop() {
  char message[200];
  int8_t index = 0;

  while (!Serial.available()) {
    // Do nothing
  }

  if (Serial.available() > 0) {
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
    char c = Serial.read();
    while (c != '@') { // Read until @ and exclude it
      if (isAscii(c) && c != '\n') { // Make sure valid characters are coming in. Ignore \n to allow IDE's serial monitor usage
        message[index] = c;
        index++;
      }
      c = Serial.read();
    }
    message[index] = '\0'; // Terminate the string
    index = 0;
  }

  if (message[0] == 'D' || message[0] == '1') {
    display.firstPage();
    do {
      display.setCursor(0, textHeight);
      display.print(message);
    } while (display.nextPage());
  } else if (strchr(message, '{') != NULL) { // Only proceed for valid messages
    int8_t temp = (10 * (message[0] - '0')) + (message[1] - '0'); // char[n] - '0' converts char[n] to an integer; multiply by 10 and add char[n+1] - '0' to concatenate 2 digits into one int
    int8_t humidity, icon;
    char dateAndTime[80];
    bool light = false;
    time_t epoch;
    if (message[4] == '{') {
      humidity = (10 * (message[2] - '0')) + (message[3] - '0');
      epoch = (message[5] - '0'); // Extract unix time from message START
      index = 6;
      char epochChar[11];
      while (message[index] != '}') {
        epoch = (epoch * 10) + (message[index] - '0');
        index++;
      } // Extract unix time from message END
    } else {
      humidity = 100;
      epoch = (message[6] - '0'); // Extract unix time from message START
      index = 7;
      char epochChar[11];
      while (message[index] != '}') {
        epoch = (epoch * 10) + (message[index] - '0');
        index++;
      } // Extract unix time from message END
    }

    index++;
    icon = (10 * (message[index] - '0')) + (message[index + 1] - '0');
    if (message[index + 2] == 'd')
      light = true;

    char days[7][4] = {
      "Sun",
      "Mon",
      "Tue",
      "Wed",
      "Thu",
      "Fri",
      "Sat"
    };
    sprintf(dateAndTime, "%s %d/%d/%d %02d:%02d", days[weekday(epoch) - 1], day(epoch), month(epoch), year(epoch), hour(epoch), minute(epoch));

    index = 0;
    while (message[index] != '#') { // Define the offset at which the location name starts
      index++;
    }
    index++;
    char location[50];
    int8_t j = 0;
    for (index; index < strlen(message); index++) {
      location[j] = message[index];
      j++;
    }
    location[j] = '\0';

    char tempInfo[18]; char humidityInfo[15];
    sprintf(tempInfo, "Temperature: %d", temp);
    sprintf(humidityInfo, "Humidity: %d%c", humidity, '%');
    int16_t a, b; uint16_t tempInfoWidth, c;
    display.getTextBounds(tempInfo, 0, 0, &a, &b, &tempInfoWidth, &c);
    display.firstPage();
    do {
      if (light) {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
        display.fillRect(0, 0, 400, 6, GxEPD_BLACK);
        display.fillRect(0, 0, 6, 300, GxEPD_BLACK);
        display.fillRect(0, 294, 400, 6, GxEPD_BLACK);
        display.fillRect(394, 0, 6, 300, GxEPD_BLACK);
      } else {
        display.fillScreen(GxEPD_BLACK);
        display.setTextColor(GxEPD_WHITE);
        display.fillRect(0, 0, 400, 6, GxEPD_WHITE);
        display.fillRect(0, 0, 6, 300, GxEPD_WHITE);
        display.fillRect(0, 294, 400, 6, GxEPD_WHITE);
        display.fillRect(394, 0, 6, 300, GxEPD_WHITE);
      }

      switch (icon) {
        case 1:
          display.drawInvertedBitmap(7, textHeight, light ? _01d : _01n, W, H, light ? GxEPD_BLACK : GxEPD_WHITE);
          break;
        case 2:
          display.drawInvertedBitmap(7, textHeight, clouds, W, H, light ? GxEPD_BLACK : GxEPD_WHITE);
          break;
        case 3:
          display.drawInvertedBitmap(7, textHeight, clouds, W, H, light ? GxEPD_BLACK : GxEPD_WHITE);
          break;
        case 4:
          display.drawInvertedBitmap(7, textHeight, clouds, W, H, light ? GxEPD_BLACK : GxEPD_WHITE);
          break;
        case 9:
          display.drawInvertedBitmap(7, textHeight, rain, W, H, light ? GxEPD_BLACK : GxEPD_WHITE);
          break;
        case 10:
          display.drawInvertedBitmap(7, textHeight, rain, W, H, light ? GxEPD_BLACK : GxEPD_WHITE);
          break;
        case 11:
          display.drawInvertedBitmap(7, textHeight, _11, W, H, light ? GxEPD_BLACK : GxEPD_WHITE);
          break;
        case 13:
          display.drawInvertedBitmap(7, textHeight, _13, W, H, light ? GxEPD_BLACK : GxEPD_WHITE);
          break;
        case 50:
          display.drawInvertedBitmap(7, textHeight, _50, W, H, light ? GxEPD_BLACK : GxEPD_WHITE);
          break;
      }

      display.setCursor(107, textHeight + 50);
      display.print(tempInfo);

      display.setFont(&Degree);
      display.setCursor(107 + tempInfoWidth, textHeight + 50);
      display.print(char(0));

      display.setFont(&FreeMonoBold12pt7b);
      display.setCursor(107 + tempInfoWidth + 17, textHeight + 50);
      display.print('C');
      display.setCursor(107, (2 * textHeight) + 50);
      display.print(humidityInfo);
      display.setCursor(7, (3 * textHeight) + 100);
      display.print(location);
      display.setCursor(7, (4 * textHeight) + 100);
      display.print(dateAndTime);
    } while (display.nextPage());
  }
}
