#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <SoftwareSerial.h>

#include "icons.h"

#define W 100 // Icon Width
#define H 100 // Icon Height

#define MAX_DISPLAY_BUFFER_SIZE 800
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))

GxEPD2_BW<GxEPD2_420, MAX_HEIGHT(GxEPD2_420)> display(GxEPD2_420(/*CS*/ 10, /*DC=*/ 9, /*RST=*/ 8, /*BUSY=*/ 7));
SoftwareSerial mSerial(4, 2);

int textHeight;

void displaySetUp(bool initial) {
  display.init();
  display.setRotation(0);
  display.setFont(&FreeMonoBold12pt7b);
  display.setTextColor(GxEPD_BLACK);

  if (initial) {
    display.setFullWindow();
    char retr[] = "Retrieving Data";
    int a, b, c;
    display.getTextBounds(retr, 0, 0, &a, &b, &c, &textHeight);
  
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
  mSerial.begin(2400);
  displaySetUp(true);
}

void loop() {
  char message[200];
  
  while (!mSerial.available()) {
    // Do nothing
  }

  if (mSerial.available() > 0) {
    /*
     * message is a char array of this kind: 1199#City@
     * 11: are temperature digits
     * 99: are humidity digits
     * #: marks the start of the location name
     * City: is the location name 
     * @: marks the end of the location name
     */
    int index = 0;
    char c = mSerial.read();
    while (c != '@') { // Read until @ and exclude it
      if (isAscii(c)) { // Make sure valid characters are coming in
        message[index] = c;
        index++;
      }
      c = mSerial.read();
    }
    message[index] = '\0'; // Terminate the string
  }
  Serial.println(message);
  int temp = (10 * (message[0] - '0')) + (message[1] - '0'); // char[n] - '0' converts char[n] to an integer; multiply by 10 and add char[n+1] - '0' to concatenate 2 digits into one int
  int humidity;
  if(message[4] == '#') {
    humidity = (10 * (message[2] - '0')) + (message[3] - '0');
  } else {
    humidity = 100;
  }
  int index = 0;
  for (int i = 0; i < strlen(message); i++) {
    if (message[i] == '#') // Define the offset at which the location name starts
      index = i + 1;
  }
  char location[50];
  int j = 0;
  for (index; index < strlen(message); index++) {
    location[j] = message[index];
    j++;
  }
  location[j] = '\0';
  display.firstPage();
    do {
      display.fillScreen(GxEPD_WHITE);
      display.setCursor(0, textHeight);
      display.print(temp);
      display.setCursor(0, 2 * textHeight);
      display.print(humidity);
      display.setCursor(0, 3 * textHeight);
      display.print(location);
    } while (display.nextPage());
}
