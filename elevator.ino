//Elavator Buttons: Reset & Trigger

//Included Libraries:
#include <Arduino.h>
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
#include "Seeed_BME280.h"

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)
//Set up Borometer:
BME280 bme280;
Adafruit_BME280 bme;
//Set up LCD display:
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

//All variables:
int state;
unsigned long lastToggle;
float distTraveled = 0;
float beginning;
const int buttonPin1 = 1;
float altitude;
float altitudeLive;
float deltaAlt = 0;
int upOrDown = 0;
int reset = 0;
float zeroed;
float minimum = 1;
int iconX;
int iconY;

void setup(void) {

  setUp();
  //Delay so that chips have enough time to turn on
  delay(5000);
  //button for reset
  pinMode(buttonPin1, INPUT);
  //lastToggle = millis();
  //Figure out initial altitude
  altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  beginning = altitude;
}

void loop(void) {
  reset = 0;
  //tracks the first button and makes is that whenever this button is clicked, it will compare the altitude to the previous one,
  //add it to the counter, and set the currect altitude as the new begining
  counter();
  //button 2 resets the distTraveled to 0 and updates the current altitude as begining
  resetButton();
  //displaying everything to the LCD
  u8g2.firstPage();
  do {
    imageloop();
  } while ( u8g2.nextPage() );
  delay(10);
  //if it is resetting, let it sit for 1 second
  if (reset == 1) {
    delay(1000);
  }
}

void imageloop(void) {
  altitudeLive = bme.readAltitude(SEALEVELPRESSURE_HPA);
  beginning = altitude;
  iconX= 117;
  iconY = 16;
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.setCursor(0, iconY-2);

  u8g2.setFont(u8g2_font_unifont_t_symbols);
  if (reset == 1) {
    u8g2.print("Resetting...");
    u8g2.drawGlyph(90, iconY, 0x23f3);
  } else {
    switch (upOrDown) {
      case 0: {
          u8g2.drawGlyph(iconX, iconY, 0x23f8);
        } break;
      case 1: {
          u8g2.drawGlyph(iconX, iconY, 0x23f6);
        } break;
      case -1: {
          u8g2.drawGlyph(iconX, iconY, 0x23f7);
        } break;
    }
  }
  //Current altitude:
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.setCursor(0, 27);
  u8g2.print("Altitude: ");
  zeroed = altitudeLive - deltaAlt;
  u8g2.print(zeroed);
  u8g2.print("m");
  //Cumulative:
  u8g2.setCursor(0, 50);
  u8g2.print("Total: ");
  u8g2.print(distTraveled);
  u8g2.print("m");

}
void resetButton(void) {

  int buttonState1 = analogRead(buttonPin1);
  if (buttonState1 >= 700) {
    distTraveled = 0;
    beginning = altitude;
    deltaAlt = altitude;
    reset = 1;
    //delay(1000);
  }
}
void counter(void) {
  bme.takeForcedMeasurement(); 
  
  if (millis() - lastToggle > 5000) {
    altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
    lastToggle = millis();
    int delta = abs(altitude - beginning);
    if (delta > minimum) {
      distTraveled += abs(altitude - beginning);
      if ((altitude - beginning) > 0) {
        upOrDown = 1;
      } else {
        upOrDown = -1;
      }
      beginning = altitude;
    } else upOrDown = 0;
  }
}
void setUp(void) {
  Serial.begin(9600);

  if (! bme.begin(&Wire)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  if (!bme280.init()) {
    Serial.println("Device error!");
  }
  Serial.println();
  //Necessary for LCD display
  u8g2.begin();
}
