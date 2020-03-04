//Elavator Buttons: Reset & Trigger

//Included Libraries:
#include <Arduino.h>
#include <U8g2lib.h>
#include <U8x8lib.h>

//include <SPI.h>

#include <Wire.h>
#include "Seeed_BME280.h"

//include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)
//Set up Borometer:
BME280 bme280;
Adafruit_BME280 bme;
//Set up LCD display:
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

//All variables:
int state;
unsigned long lastToggle, lastToggleB;
float distTraveled = 0;
float beginning;

//const int buttonPin1 = 10;
//const int buttonPin2 = 12;

const int buttonPin1 = A1;
const int buttonPin2 = 5;

float altitude;
float altitudeLive;
float deltaAlt = 0;
int upOrDown = 0;
int reset = 0;
int zeroed;
float minimum = 1.5;
int iconX;
int iconY;
int deltaTime;
float velocity;
float deltadist;
uint8_t current_screen = 0;
uint8_t cursor_ = 0;
int iconXpos = 117;
int iconYpos = 16;
int val1, val2;
float maxV = 0;
float temp;
int timeM;

void setup(void) {
  setUp();
}

void loop(void) {
  reset = 0;
  val1 = digitalRead(buttonPin1);
  val2 = digitalRead(buttonPin2);
  counter();
  //button 2 resets the distTraveled to 0 and updates the current altitude as begining
  //resetButton();
  //displaying everything to the LCD
  buttonCheck1();
  buttonCheck2();
  /*u8g2.firstPage();
  do {
    imageloop();
  } while ( u8g2.nextPage() );*/
  display();
  delay(10);
  
  //if it is resetting, let it sit for 1 second
  /*
  if (reset == 1) {
    delay(1000);
  }*/
  
}

void imageloop(void) {
  altitudeLive = bme.readAltitude(SEALEVELPRESSURE_HPA);
  beginning = altitude;
  iconX= 117;
  iconY = 16;
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.setCursor(0, iconY-2);

  u8g2.setFont(u8g2_font_7x14_tf);
  if (reset == 1) {
    u8g2.print("Resetting...");
    u8g2.drawGlyph(90, iconY, 0x23f3);
  } else {
    switch (upOrDown) {
      case 0: {
          u8g2.drawGlyph(iconX, iconY, 0x23f8);
        } break;
      case 1: {
          u8g2.drawGlyph(iconX - 5, iconY, 0x23f6);
        } break;
      case -1: {
          u8g2.drawGlyph(iconX - 5, iconY, 0x23f7);
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
  u8g2.setCursor(0, 42);
  u8g2.print("Total: ");
  u8g2.print(distTraveled);
  u8g2.print("m");
  u8g2.setCursor(0, 57);
  u8g2.print("Velocity: ");
  u8g2.print(velocity);
  u8g2.print("m/s");
}

void counter(void) {
  bme.takeForcedMeasurement(); 
  
  if (millis() - lastToggle > 2500) {
    deltaTime = millis() - lastToggle;
    altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
    lastToggle = millis();
    deltadist = abs(altitude - beginning);
    Serial.println(deltadist);
    if (deltadist > minimum) {
      distTraveled += abs(altitude - beginning);
      Serial.println(distTraveled);
      Serial.println();
      if ((altitude - beginning) > 0) {
        upOrDown = 1;
      } else {
        upOrDown = -1;
      }
      beginning = altitude;
    } else upOrDown = 0;
    calcV(deltadist);
    calcMaxV();
  }
}
void setUp(void) {
  pinMode(buttonPin2, INPUT); //scroll
  pinMode(buttonPin1, INPUT); //select
  Serial.begin(9600);
  Wire.begin();
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
  u8g2.setFont(u8g2_font_ncenB10_tr);
  display();
  delay(5000);
  altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  beginning = altitude;
  current_screen = 1;
  display();
}

void display() {
  u8g2.firstPage();
  do {
    drawScreen();
  } while (u8g2.nextPage());
}

void drawScreen() {
  altitudeLive = bme.readAltitude(SEALEVELPRESSURE_HPA);
  switch (current_screen) {

    case 0: {
        u8g2.setFont(u8g2_font_ncenB10_tr);
        u8g2.setCursor(0, 27);
        u8g2.print("Loading...");
      } break;

    case 1: {
        u8g2.setFont(u8g2_font_7x14_tf);
        static const uint8_t NUM_MENU_ITEMS = 6;
        const char *menu_items[NUM_MENU_ITEMS] = {
          "Dashboard",
          "Distance Data",
          "Speed Data",
          "Sensor Output",
          "Settings",
        };
        uint8_t i, h;
        int w, d;

        u8g2.setFontRefHeightText();
        u8g2.setFontPosTop();

        u8g2.setCursor(0, 0);
        h = u8g2.getFontAscent() - u8g2.getFontDescent();
        w = 128;

        for (i = 0; i < 5; i++ ) {
          d = (w - u8g2.getStrWidth(menu_items[i])) / 2;
          if (i == cursor_) {
            u8g2.drawGlyph(0, (i * h) + 3, 0x00BB);
          }
          u8g2.drawStr(d, (i * h) + 3, menu_items[i]);
        }
      } break;

    case 2: {
        u8g2.setFont(u8g2_font_ncenB10_tr);
        u8g2.setCursor(0, 0);
        u8g2.print("Dashboard");

        if (upOrDown == 0) {
          u8g2.drawGlyph(iconXpos, iconYpos, 0x23f8);
        }
        else if (upOrDown == 1) {
          u8g2.drawGlyph(iconXpos, iconYpos, 0x23f6);
        }
        else if (upOrDown == -1) {
          u8g2.drawGlyph(iconXpos, iconYpos, 0x23f7);
        }
        u8g2.setFont(u8g2_font_7x14_tf);
        u8g2.setCursor(0, 18);
        u8g2.print("Current Alt.:");
        zeroed = altitudeLive - deltaAlt;
        u8g2.print(zeroed);
        u8g2.print("m");
        u8g2.setCursor(0, 40);
        u8g2.print("Speed:");
        u8g2.print(velocity);
        u8g2.print("m/s");
      } break;

    case 3: {
        u8g2.setFont(u8g2_font_ncenB10_tr);
        u8g2.setCursor(0, 0);
        u8g2.print("Distance Data");

        if (upOrDown == 0) {
          u8g2.drawGlyph(iconXpos, iconYpos, 0x23f8);
        }
        else if (upOrDown == 1) {
          u8g2.drawGlyph(iconXpos, iconYpos, 0x23f6);
        }
        else if (upOrDown == -1) {
          u8g2.drawGlyph(iconXpos, iconYpos, 0x23f7);
        }

        u8g2.setFont(u8g2_font_7x14_tf);
        u8g2.setCursor(0, 18);
        u8g2.print("Dist. Traveled: ");
        
        u8g2.setCursor(0, 34);
        u8g2.print(distTraveled);
        u8g2.print("m");
        /*
        u8g2.print("Time Since Reset: ");
        u8g2.setCursor(0, 50);
        
        timeM = millis();
        timeM/1000;
        if (timeM < 60) {
          u8g2.print(timeM);
          u8g2.print("s");
        } else if (timeM >= 60) {
          u8g2.print(timeM / 60);
          u8g2.print("m");
        }*/

      } break;

    case 4: {
        u8g2.setFont(u8g2_font_ncenB10_tr);
        u8g2.setCursor(0, 0);
        u8g2.print("Speed Data");

        if (upOrDown == 0) {
          u8g2.drawGlyph(iconXpos, iconYpos, 0x23f8);
        }
        else if (upOrDown == 1) {
          u8g2.drawGlyph(iconXpos, iconYpos, 0x23f6);
        }
        else if (upOrDown == -1) {
          u8g2.drawGlyph(iconXpos, iconYpos, 0x23f7);
        }
        u8g2.setFont(u8g2_font_7x14_tf);
        u8g2.setCursor(0, 17);
        u8g2.print("Running Speed: ");
        u8g2.setCursor(0, 32);
        u8g2.print(maxV);
        u8g2.print("m/s");
        u8g2.setCursor(0, 47);
        u8g2.print("Number of stops: ");
        u8g2.print("...");
        u8g2.print("m/s");

      } break;

    case 5: {
      
        u8g2.setFont(u8g2_font_ncenB10_tr);
        u8g2.setCursor(0, 0);
        u8g2.print("Sensor Output");
        
        u8g2.setFont(u8g2_font_7x14_tf);
        u8g2.setCursor(0, 15);
        u8g2.print("Pressure: ");
        u8g2.setCursor(0, 31);
        u8g2.print(bme280.getHumidity());
        u8g2.print("mbar");
        u8g2.setCursor(0, 48);
        u8g2.print("Temp: ");
        u8g2.print(bme280.getTemperature());
        u8g2.print(" deg C");
        u8g2.setCursor(0, 64);
        u8g2.print("Hum: ");
        u8g2.print(bme280.getHumidity());
        u8g2.print("%");

      } break;

    case 6: {
        u8g2.setFont(u8g2_font_ncenB10_tr);
        u8g2.setCursor(0, 0);
        u8g2.print("Settings");

        u8g2.setFont(u8g2_font_7x14_tf);
        u8g2.setCursor(0, 20);
        u8g2.print("Click To Reset");
        //figure out how to change units for dist, pressure and temp

      } break;
  }
}


void calcV (float dist) {
  deltaTime = deltaTime/1000;
  velocity = dist/deltaTime;
    if(velocity <= 0.5) {
      velocity = 0;
    }
}

void buttonCheck1(void)
{
  if (val1 == HIGH && millis() - lastToggleB > 200) {
    Serial.print("Hello!");
    //why is this necessary?
    if (current_screen == 0) {
      current_screen = 1;
    }
    if (current_screen == 1) {
      if (cursor_ < 4) {
        cursor_++;
      }
      else if (cursor_ == 4) {
        cursor_ = 0;
      }
    }
    if (current_screen == 6) {
      resetButton();
    }
    lastToggleB = millis();
    display();
  } 
}

void goBack() {
      current_screen = 1; 
}

void buttonCheck2(void)
{
  if (val2 == HIGH && millis() - lastToggleB > 200) {
    Serial.print(cursor_);
    if (current_screen == 1) {
      current_screen = (cursor_ + 2); 
    }
    else {
      goBack();
    }
    Serial.print(current_screen);
    lastToggleB = millis();
    display(); 
  }
}

void resetButton(void) {
    altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
    distTraveled = 0;
    beginning = altitude;
    deltaAlt = altitude;
    velocity = 0;
    reset = 1;
    maxV = 0;
}

void getTemp(void) {
  temp = bme280.getTemperature();
}
void calcMaxV(void){
  if(velocity > maxV) {
    maxV = velocity;
  }
}