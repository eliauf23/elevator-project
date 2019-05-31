//DEFAULT UNITS: pressure (mbar = hPa), distance (m), time (ms), temperature (degrees C)

#include <Arduino.h>
#include <U8g2lib.h>
#include <U8x8lib.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Wire.h>

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); //constructor for screen
Adafruit_BME280 bme280; //constructor for bme280 sensor

float a0, af, p0, pf, dx, distTraveled; //initial & final altitude, initial & final pressure
//dx differential change in vert. dist
bool isMoving, goingUp, goingDown = false;
unsigned long initTime, currentTime; //uses millis() fn
int delayTime;
//v1 = scroll, v2 = select
int val1, val2;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

uint8_t current_screen = 0;
uint8_t cursor_ = 1;
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
    delayTime = 41;
  
  pinMode(A0, INPUT); //scroll
  pinMode(A1, INPUT); //select
  
  u8g2.begin();
  u8g2.setFont(u8g_font_unifont);
  display();
  delay(5000);
  current_screen = 1;
  display();
}

void loop() {
  
  val1 = analogRead(A0);
  val2 = analogRead(A1);



  if ((millis() - lastDebounceTime) > debounceDelay) {
 
    if (val1 > 500) {
      if (current_screen == 0) {
          current_screen = 1;
      }
        if(current_screen == 1) {
          if(cursor_ < 6) {
          cursor_++;
        }
        else if(cursor_ == 6) {
          cursor_ = 1;
        }
        display();
        lastDebounceTime = millis(); //set the current time
      }
    }
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
 
    if ((val2 > 500) && (current_screen = 1) ) {
      current_screen = cursor_;
      display();
    }
    else {
    goBack();
    }
      display();
      lastDebounceTime = millis(); //set the current time
    }
  
  initTime = millis();
  p0 = bme280.readPressure();
  a0 = bme280.readAltitude(p0);
  accumulateDist(distTraveled);
 
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
  return 12;
}

void goBack() {

  if ((millis() - lastDebounceTime) > debounceDelay) {

    if ((val2 > 500) && (current_screen != 1) ) {
      current_screen = 1;
      display();
      lastDebounceTime = millis(); //set the current time
    }
  }
}
    
void drawScreen() {

  switch(current_screen) {
    
    case 0: {
      u8g2.setFont(u8g2_font_ncenB10_tr);
      u8g2.setCursor(0, 27);
      u8g2.print("Loading...");
    } break;

    case 1: {
      static const uint8_t NUM_MENU_ITEMS = 6;
      const char *menu_items[NUM_MENU_ITEMS] = {
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

      u8g2.setCursor(0, 0);
      u8g2.drawGlyph(0, 10, &#187)

      //u8g2.print("          MENU");
      h = u8g2.getFontAscent()-u8g2.getFontDescent();
      w = 128;
      
        for(i = 0; i < 6; i++ ) {
            d = (w-u8g2.getStrWidth(menu_items[i]))/2;
            if (i == cursor_ ) {
              //u8g2.drawBox(0, i*h+1, w, h); //draw line
            }
        u8g2.drawStr(d, (i*h) + 4, menu_items[i]);
        }
     } break;

    case 2: {
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.setCursor(0, 0);
    u8g2.print("Display Mode");

    if(!isMoving) {
      u8g2.drawGlyph(iconXpos, iconYpos, 0x23f8);
    }
    else if(isMoving && goingUp) {
      u8g2.drawGlyph(iconXpos, iconYpos, 0x23f6);
    }
    else if(isMoving && goingDown) {
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
    
    if(!isMoving) {
      u8g2.drawGlyph(iconXpos, iconYpos, 0x23f8);
    }
    else if(isMoving && goingUp) {
      u8g2.drawGlyph(iconXpos, iconYpos, 0x23f6);
    }
    else if(isMoving && goingDown) {
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

    if(!isMoving) {
      u8g2.drawGlyph(iconXpos, iconYpos, 0x23f8);
    }
    else if(isMoving && goingUp) {
      u8g2.drawGlyph(iconXpos, iconYpos, 0x23f6);
    }
    else if(isMoving && goingDown) {
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

    case 7: {
      //RESET SENSOR
      //ask before clearing
      //clear all memory
      
    } break;
    
    break;
  } 
}
