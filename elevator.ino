#include <Arduino.h>
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#include "Seeed_BME280.h"
 
BME280 bme280; //initialize barometer

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); //iniitalize LCD display


float distTrav() {

}

//All variables:

unsigned long lastToggle = 0;

int distTraveled = 0;

int init_alt, altitude, buttonState1;

const int buttonPin1 = 1;

float init_pressure, pressure;
int deltaAlt = 0;

void setup(void) {
  if(!bme280.init()){
    Serial.println("Device error!");
  }
  delay(5000); //startup period

  u8g2.begin(); //start LCD display
  pinMode(buttonPin1, INPUT); //for the reset button
  init_pressure = bme280.getPressure();
  //Figure out initial altitude
  altitude = bme280.calcAltitude(init_pressure);
  init_alt = altitude; 
}
 

void loop(void) {
//when button is clicked, finds difference in altitude
//adds to counter, makes beginning = current altitude, thereby zeroing everything
  
  if(millis()-lastToggle > 5000) {
    pressure = bme280.getPressure();
    int altitude = bme280.calcAltitude(pressure);
    lastToggle = millis();
    int delta = abs(altitude - init_alt);
    if(delta > 1) {
      distTraveled += abs(altitude - init_alt);
      init_alt = altitude;
    }
  }

  //button 2 resets the distTraveled to 0 and updates the current altitude as begining

  pressure = bme280.getPressure();

  altitude = bme280.calcAltitude(pressure);

  buttonState1 = analogRead(buttonPin1);

  if (buttonState1 >= 700) {
    distTraveled = 0;
    init_alt = altitude;
  }
  //displaying everything to the LCD

  u8g2.firstPage();

  float pressure = bme280.getPressure();

  int altitude = bme280.calcAltitude(pressure);

 
 do {

    u8g2.setFont(u8g2_font_ncenB10_tr);

    //Current altitude:

    u8g2.setCursor(0,35);

    u8g2.print("Altitude: ");

    int zeroed = altitude - init_alt;

    u8g2.print(zeroed);

    u8g2.print("m");

    //Cumulative:

    u8g2.setCursor(0,50);

    u8g2.print("Cumulative: ");

    u8g2.print(distTraveled);

    u8g2.print("m");

   } while ( u8g2.nextPage() );

  delay(10);

}
