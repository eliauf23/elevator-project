//code for buttons, modes, initial interface, loading/configuration screen
//start(unpause/wake up), pause, stop, reset
//mode button allows you to cycle through functionalitiy
//selector button allows you to select option

#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>

void setup(void) {
	u8g2.begin();
}

void loop(void) {
	u8g2.firstPage();
	


}






















int prev_switch_state, switch_state = 0;

void loop() {
	u8g2.clearBuffer(); //clear internal memory
	//partial clear
	u8g2.setFont(u8g2_font_ncenB10_tr);

	start_test();
	u8g2.setCursor(4,10);
	u8g2.print("To choose mode \n click button 1");
	u8g2.setCursor(64,10);
	u8g2.print("To select mode \n click button 2");



}
u8g2.firstPage();

//if(//condition) {

    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.setCursor(0,35);
    u8g2.print("Starting...");
    delay(5);


//mode 2
    u8g2.print("Starting ")




    u8g2.print("Altitude: ");

    int zeroed = altitudeb - deltaAlt;

    u8g2.print(zeroed);

    u8g2.print("m");

    //Cumulative:

    u8g2.setCursor(0,50);

    u8g2.print("Cumulative: ");

    u8g2.print(distTraveled);

    u8g2.print("m");

  // } while ( u8g2.nextPage() );

  delay(10);

//}
