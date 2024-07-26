
/*
* Kamilya Zhailaubayeva 201531425
* Superviser: Akhan Almagambetov
* Project title: Smart scale for determining the perfect ratio of grain-to-water
* Description: designing code for the scale with the I2C LCD in Arduino IDE.
*/

#include <HX711_ADC.h> // HX711 library
#include <LiquidCrystal_I2C.h> // I2C LCD library

const int tw = 5; // taring weight pin
const int dout = 6; // dout pin
const int sck = 7; // sck pin
HX711_ADC LoadCell(dout, sck); // setting HX711 pins
LiquidCrystal_I2C lcd(0x27, 16, 2); // setting I2C LCD

void setup() {
  
  pinMode (tw, INPUT_PULLUP);
  
  LoadCell.begin(); // connecting to HX711
  LoadCell.start(1000); // 1 second stabilization time of the scale
  LoadCell.setCalFactor(375); // calibarating the scale
  
  lcd.begin(); // connecting to the I2C LCD
  lcd.backlight(); // turning on the backlight
  lcd.setCursor(0, 0); // starting from 1st column, 1st row of the LCD
  lcd.print("Smart Scale");
  lcd.setCursor(0, 1); // starting from 1st column, 2nd row of the LCD
  lcd.print("10KG LOAD CELL");
  delay(4000);
  lcd.clear(); // After 4 seconds clearing the LCD
}

void loop() { 
  
  lcd.setCursor(0, 0); // starting from 1st column, 1st row of the LCD
  lcd.print("Weight on Scale:");
  LoadCell.update(); // aquiring data from the scale
  float data = LoadCell.getData(); // geting the resulting weight
  
  if (data < 0) { 
    // If the weight is negative number:
    data = (-1) * data;
    lcd.setCursor(0, 1); // starting from 1st column, 2nd row of the LCD
    lcd.print("-");
  }
  else { 
    // If the weight is positive number:
    lcd.setCursor(0, 1); // starting from 1st column, 2nd row of the LCD
    lcd.print(" ");
  }
  
  lcd.setCursor(1, 1); // starting from 2nd column, 2nd row
  lcd.print(data, 1); // printing the aquired weigth
  lcd.print(" grams   "); // printing the unit of the weight

  if (digitalRead (tw) == LOW) {
    lcd.setCursor(0, 1); // starting from 1st column, 2nd row of the LCD
    lcd.print(" Taring weigth ");
    LoadCell.start(1000); // restarting the scale
    lcd.clear();
    
  }
  
}
