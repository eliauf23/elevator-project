//DEFAULT UNITS: pressure (mbar = hPa), distance (m), time (ms), temperature (degrees C)

#include <Arduino.h>
#include <U8g2lib.h>
#include <U8x8lib.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Wire.h>
#include <JC_Button.h>
  #define PULLUP true     
  #define INVERT true    
  #define DEBOUNCE_MS 20 

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); //constructor for screen
Adafruit_BME280 bme280; //constructor for bme280 sensor

float a0, af, p0, pf, dx, distTraveled; //initial & final altitude, initial & final pressure
//dx differential change in vert. dist
bool isMoving, goingUp, goingDown;
unsigned long initTime, currentTime; //uses millis() fn
int delayTime;

Button scroll(2, PULLUP, INVERT, DEBOUNCE_MS);
Button select(3, PULLUP, INVERT, DEBOUNCE_MS);
uint8_t current_screen = 0;
uint8_t cursor = 1;
bool mainMenu = true;

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
    delayTime = 41;

  u8g2.begin();
  u8g2.setFont(u8g_font_unifont);
  display();
  delay(5000);
}

void loop() {
  initTime = millis();
  p0 = bme280.readPressure();
  a0 = bme280.readAltitude(p0);
  accumulateDist(distTraveled);

  //check if buttons are pressed and modify screen accordingly
  scroll.read();
  select.read();

  //control flow for graphics
  if(scroll.pressedFor(500) && mainMenu) {
    if(cursor < 6) {
      cursor++;
    }
    else if(cursor == 6) {
      cursor = 1;
    }
    drawScreen();
  }
  //else - nothing happens - can't scroll outside of main menu (yet)

  if(select.pressedFor(500)) {
    if(mainMenu) {
      current_screen = cursor;
      drawScreen();
    }
    else {
    goBack();
    }
  }

  
  //call display function
  delay(delayTime);
}

/*              ODOMETER FUNCTIONS
*/

//function calculates distance traveled, returns as float & modifies motion state bool vars
float accumulateDist(float distance_traveled) {
  float temp_dist = 0; //local variable
  while(temp_dist <= 1) {
    pf = bme280.readPressure();
    af = bme280.readAltitude(pf);
    dx = af-a0;
    motionState(dx);
    temp_dist = abs(dx);
    currentTime = millis();
    if((currentTime-initTime >= 5000) && (temp_dist < 1)) {
      distance_traveled += temp_dist;
      break;
    }
    else if(temp_dist == 1) {
      distance_traveled += 1;
      temp_dist = 0;
    }
  }
  return distance_traveled;
}

void motionState(float dx) {
  if(dx < 0) {
    isMoving = true;
    goingUp = false;
    goingDown = true;
    
  }
  else if(dx > 0) {
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

void display() {
  u8g2.firstPage();  
    do {
        drawScreen();
    } while( u8g2.nextPage() );
}

int lineSpacing() {
  return 24;
}

void goBack() {
  //display message: go back? if select immediately, continue
  current_screen = 0;
  cursor = 1;
  drawScreen();
}

void drawScreen() {

  switch(current_screen) {
    
    case 0: {
      u8g2.setCursor(0, 27);
      u8g2.print("Loading...");
    } break;

    case 1: {
      static const uint8_t NUM_MENU_ITEMS = 6;
      const char *menu_items[] = {
        "Display Mode", 
        "Odometer", 
        "Speedometer",
        "Settings",
        "Raw Data",
        "Setup Info"
      };
      uint8_t i, h;
      int w, d;

      u8g2.setFontRefHeightText();
      u8g2.setFontPosTop();

      h = u8g2.getFontAscent()-u8g2.getFontDescent();
      w = u8g2.getWidth();
        for(i = 0; i < 3; i++ ) {
            d = (w-u8g2.getStrWidth(menu_items[i]))/2;
            if (i == menu_current ) {
              u8g2.drawBox(0, i*h+1, w, h);
            }
        u8g2.drawStr(d, i*h, menu_strings[i]);
        }
     } break;

    case 2: {
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(0,getFontLineSpacing(),"Display Mode");
    iconXpos = 117;
    iconYpos = 16;
    u8g2.setCursor(0, iconYpos-2);

    if(!isMoving) {
      u8g2.drawGlyph(iconX, iconY, 0x23f8);
    }
    else if(isMoving && goingUp) {
      u8g2.drawGlyph(iconX, iconY, 0x23f6);
    }
    else if(isMoving && goingDown) {
      u8g2.drawGlyph(iconX, iconY, 0x23f7);
    }

    u8g2.setCursor(0, 27);
    u8g2.print("Dist. Traveled: ");
    u8g2.print(distTraveled);
    u8g2.print("m");
    u8g2.setCursor(0, 51);
    u8g2.print("Max Speed: ");
    u8g2.print(-1);
    u8g2.print("m/s");
    u8g2.setCursor(0, 75);
    u8g2.print("Avg Speed: ");
    u8g2.print(-1);
    u8g2.print("m/s");

  } break;

    case 3: {
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(0,getFontLineSpacing(),"Odometer");
    iconXpos = 117;
    iconYpos = 16;
    u8g2.setCursor(0, iconYpos-2);

    if(!isMoving) {
      u8g2.drawGlyph(iconX, iconY, 0x23f8);
    }
    else if(isMoving && goingUp) {
      u8g2.drawGlyph(iconX, iconY, 0x23f6);
    }
    else if(isMoving && goingDown) {
      u8g2.drawGlyph(iconX, iconY, 0x23f7);
    }

    u8g2.setCursor(0, 27);
    u8g2.print("Rel. Altitude: ");
    u8g2.print(af);
    u8g2.print("m");
    u8g2.setCursor(0, 51);
    u8g2.print("Dist. Traveled: ");
    u8g2.print(-1);
    u8g2.print("m");

  } break;

    case 4: {
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(0,getFontLineSpacing(),"Speedometer");
    iconXpos = 117;
    iconYpos = 16;
    u8g2.setCursor(0, iconYpos-2);

    if(!isMoving) {
      u8g2.drawGlyph(iconX, iconY, 0x23f8);
    }
    else if(isMoving && goingUp) {
      u8g2.drawGlyph(iconX, iconY, 0x23f6);
    }
    else if(isMoving && goingDown) {
      u8g2.drawGlyph(iconX, iconY, 0x23f7);
    }

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
    u8g2.drawStr(0,getFontLineSpacing(),"Raw Data");
    u8g2.setCursor(0, 27);
    u8g2.print("Pressure: ");
    u8g2.print(bme280.readPressure());
    u8g2.print("mbar");
    u8g2.setCursor(0, 51);
    u8g2.print("Temperature: ");
    u8g2.print(bme280.readTemperature());
    u8g2.print(" deg C");
    u8g2.setCursor(0, 75);
    u8g2.print("Humidity: ");
    u8g2.print(bme280.readHumidity());
    u8g2.print("%");
      
    } break;

    case 6: {
      u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(0,getFontLineSpacing(),"Settings");
    u8g2.setCursor(0, 27);
    u8g2.print("Change units");
    //figure out how to change units for dist, pressure and temp
    
    } break;

    case 7: {
      //RESET SENSOR
      //ask before clearing
      //clear all memory
      
    } break;
    
    break;
  } 
}
