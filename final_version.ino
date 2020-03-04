//DEFAULT UNITS: pressure (mbar = hPa), distance (m), time (ms), temperature (degrees C)
#include <Arduino.h>
#include <U8g2lib.h>
#include <U8x8lib.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Wire.h>

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); //constructor for screen
Adafruit_BME280 bme280; //constructor for bme280 sensor

unsigned long initTime, currentTime, lastClick = 0;
float a0, af, p0, pf, dx, distTraveled;
int delayTime, val1, val2; //button 1 = scroll, button 2 = select
bool isMoving, goingUp, goingDown, b1push, b2push;

int current_screen = 0;
int cursor_ = 1;
int iconXpos = 117;
int iconYpos = 16;


void setup() {
  Serial.begin(9600);
  Wire.begin();
  bme280.begin(0x76); //hex address for GYBMEP sensor, 0x76 for breakout board
  //configure BME280 settings for indoor navigation profile
  bme280.setSampling(Adafruit_BME280::MODE_NORMAL,
                     Adafruit_BME280::SAMPLING_X2,  // temperature oversampling 2x
                     Adafruit_BME280::SAMPLING_X16, // pressure oversampling 16x
                     Adafruit_BME280::SAMPLING_X1,  // humidity oversampling 1x
                     Adafruit_BME280::FILTER_X16,
                     Adafruit_BME280::STANDBY_MS_0_5 );
  delayTime = 40;

  pinMode(2, INPUT_PULLUP); //scroll
  pinMode(3, INPUT_PULLUP); //select

  u8g2.begin();
  u8g2.setFont(u8g_font_unifont);
  display(current_screen);
  delay(5000);
  current_screen = 1;
  display(current_screen);
}


void loop() {
  checkButtons();
  delay(1000);
  initTime = millis();
  p0 = bme280.readPressure();
  a0 = bme280.readAltitude(p0);
  accumulateDist(distTraveled);
  display(current_screen);
  delay(delayTime);
}


/*              ODOMETER FUNCTIONS
*/

//function calculates distance traveled, returns as float & modifies motion state bool vars
float accumulateDist(float distance_traveled) {
  float temp_dist = 0; //local variable
  while (temp_dist <= 1) {
    pf = bme280.readPressure();
    af = bme280.readAltitude(pf);
    dx = af - a0;
    motionState(dx);
    temp_dist = abs(dx);
    currentTime = millis();
    if ((currentTime - initTime >= 5000) && (temp_dist < 1)) {
      distance_traveled += temp_dist;
      break;
    }
    else if (temp_dist == 1) {
      distance_traveled += 1;
      temp_dist = 0;
    }
  }
  return distance_traveled;
}

void motionState(float dx) {
  if (dx < 0) {
    isMoving = true;
    goingUp = false;
    goingDown = true;
  }
  else if (dx > 0) {
    isMoving = true;
    goingUp = true;
    goingDown = false;
  }
  else {
    isMoving = false;
    goingUp = false;
    goingDown = false;
  }
}

/*              DISPLAY FUNCTIONS
*/

void display(int screen) {
  u8g2.firstPage();
  do {
    drawScreen(screen, cursor_);
  } while (u8g2.nextPage());
}

void goBack() {
  if (current_screen != 1) {
    u8g2.setCursor(0, 27);
    u8g2.print("Hit select to go back");
    if (val2 == LOW) {
      current_screen = 1;
      display(current_screen);
    }
  }
}

void checkButtons() {
  val1 = digitalRead(2);
  if (val1 == LOW) {
    b1push = true;
  }
  val2 = digitalRead(3);
  if (val2 == LOW) {
    b2push = true;
  }

  

  
  if (b1push) {
    //u8g2.drawStr(56, 120, "B1");
    if (millis() - lastClick > 1000) {
      if (current_screen == 1) {
        if (cursor_ < 5) {
          cursor_++;
        }
        else if (cursor_ == 5) {
          cursor_ = 1;
        }
        lastClick = millis();
        display(current_screen);
      }
    }
    b1push = false;
  }
  if (b2push) {
    //u8g2.drawStr(56, 120, "B2");
    if (current_screen == 1) {
      current_screen = cursor_;
      display(current_screen);
    }
    else {
      goBack();
    }
    b2push = false;
  }
}

void drawScreen(int screen1, int cursor1) {
  switch (screen1) {
    case 0: {
        u8g2.setFont(u8g2_font_ncenB10_tr);
        u8g2.setCursor(0, 27);
        u8g2.print("Loading...");
      } break;
    case 1: {
        u8g2.setFont(u8g2_font_7x14_tf);
        static const uint8_t NUM_MENU_ITEMS = 5;
        const char *menu_items[NUM_MENU_ITEMS] = {
          "1. Display Mode",
          "2. Odometer",
          "3. Speedometer",
          "4. Settings",
          "5. Raw Data",
        };
        uint8_t i, h;
        u8g2.setFontRefHeightText();
        u8g2.setFontPosTop();
        u8g2.setCursor(0, 0);
        h = u8g2.getFontAscent() - u8g2.getFontDescent();
        for (i = 0; i < 5; i++ ) {
          u8g2.drawStr(16, (i * h) + 4, menu_items[i]);
        }
        //determine position of cursor
        u8g2.drawGlyph(0, (cursor1 - 1)*h + 4, 0x00bb);
      } break;
    case 2: {
        u8g2.setFont(u8g2_font_ncenB10_tr);
        u8g2.setCursor(0, 0);
        u8g2.print("Display Mode");
        if (!isMoving) {
          u8g2.drawGlyph(iconXpos, iconYpos, 0x23f8);
        }
        else if (isMoving && goingUp) {
          u8g2.drawGlyph(iconXpos, iconYpos, 0x23f6);
        }
        else if (isMoving && goingDown) {
          u8g2.drawGlyph(iconXpos, iconYpos, 0x23f7);
        }
        u8g2.setFont(u8g2_font_7x14_tf);
        u8g2.setCursor(0, 18);
        u8g2.print("Dist. Trav: ");
        u8g2.print(distTraveled);
        u8g2.print("m");
        u8g2.setCursor(0, 40);
        u8g2.print("Max Speed: ");
        u8g2.print("-1");
        u8g2.print("m/s");
        u8g2.setCursor(0, 64);
        u8g2.print("Avg Speed: ");
        u8g2.print("-1");
        u8g2.print("m/s");
      } break;
    case 3: {
        u8g2.setFont(u8g2_font_ncenB10_tr);
        u8g2.setCursor(0, 0);
        u8g2.print("Odometer");
        if (!isMoving) {
          u8g2.drawGlyph(iconXpos, iconYpos, 0x23f8);
        }
        else if (isMoving && goingUp) {
          u8g2.drawGlyph(iconXpos, iconYpos, 0x23f6);
        }
        else if (isMoving && goingDown) {
          u8g2.drawGlyph(iconXpos, iconYpos, 0x23f7);
        }
        u8g2.setFont(u8g2_font_7x14_tf);
        u8g2.setCursor(0, 18);
        u8g2.print("Rel. Alt: ");
        u8g2.setCursor(0, 36);
        u8g2.print(af);
        u8g2.print("m");
        u8g2.setCursor(0, 48);
        u8g2.print("Dist. Trav: ");
        u8g2.print(-1);
        u8g2.print("m");
      } break;
    case 4: {
        u8g2.setFont(u8g2_font_ncenB10_tr);
        u8g2.setCursor(0, 0);
        u8g2.print("Speedometer");
        if (!isMoving) {
          u8g2.drawGlyph(iconXpos, iconYpos, 0x23f8);
        }
        else if (isMoving && goingUp) {
          u8g2.drawGlyph(iconXpos, iconYpos, 0x23f6);
        }
        else if (isMoving && goingDown) {
          u8g2.drawGlyph(iconXpos, iconYpos, 0x23f7);
        }
        u8g2.setFont(u8g2_font_7x14_tf);
        u8g2.setCursor(0, 27);
        u8g2.print("Max Speed: ");
        u8g2.print("-1");
        u8g2.print("m/s");
        u8g2.setCursor(0, 51);
        u8g2.print("Avg. Speed: ");
        u8g2.print(-1);
        u8g2.print("m/s");
      } break;
    case 5: {
        u8g2.setFont(u8g2_font_ncenB10_tr);
        u8g2.setCursor(0, 0);
        u8g2.print("Raw Data");
        u8g2.setFont(u8g2_font_7x14_tf);
        u8g2.setCursor(0, 15);
        u8g2.print("Pressure: ");
        u8g2.setCursor(0, 31);
        u8g2.print(bme280.readPressure());
        u8g2.print("mbar");
        u8g2.setCursor(0, 48);
        u8g2.print("Temp: ");
        u8g2.print(bme280.readTemperature());
        u8g2.print(" deg C");
        u8g2.setCursor(0, 64);
        u8g2.print("Hum: ");
        u8g2.print(bme280.readHumidity());
        u8g2.print("%");
      } break;
    case 6: {
        u8g2.setFont(u8g2_font_ncenB10_tr);
        u8g2.setCursor(0, 0);
        u8g2.print("Settings");
        u8g2.setFont(u8g2_font_7x14_tf);
        u8g2.setCursor(0, 20);
        u8g2.print("Change units");
        //figure out how to change units for dist, pressure and temp
      } break;
      /*case 7: {
          //RESET SENSOR
          //ask before clearing
          //clear all memory
        } break;
      */
      break;
  }
}
