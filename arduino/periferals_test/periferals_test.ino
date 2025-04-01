/*
João Camacho, 2025

Code adapted from the WPSH203 LCD and
WPI300 Keypad Manuals.
*/

/* //////////////////
        INCLUDES
*/ //////////////////

#include <LiquidCrystal.h>
#include <Keypad.h>

/* ////////////////////
        LCD SCREEN
*/ ////////////////////

// Create Liquid Crystal Object
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
int lcd_key = 0;
int adc_key_in = 0;
unsigned char message_count = 0;
unsigned long prev_trigger = 0;

// Analog buttons
#define btnRIGHT 0
#define btnUP 1
#define btnDOWN 2
#define btnLEFT 3
#define btnSELECT 4
#define btnNONE 5

// Read LCD Analog Buttons
int read_LCD_buttons() {
  adc_key_in = analogRead(0);
  if (adc_key_in < 50) return btnRIGHT;
  if (adc_key_in < 195) return btnUP;
  if (adc_key_in < 380) return btnDOWN;
  if (adc_key_in < 555) return btnLEFT;
  if (adc_key_in < 790) return btnSELECT;
  return btnNONE;
}

/* ////////////////
        KEYPAD
*/ ////////////////

const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
 {'1','2','3'},
 {'4','5','6'},
 {'7','8','9'},
 {'*','0','#'}
};
byte rowPins[ROWS] = {13, 12, 11, 3}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {2, 1, 0}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

/* ////////////////
        SETUP
*/ ////////////////

void setup()
{
  // Setup LCD Library
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  lcd.print("Msg0 ");
  // Serial Setup
  Serial.begin(9600);
}

/* ////////////////
        LOOP
*/ ////////////////

void loop() {

  // Debug time on LCD
  lcd.setCursor(9,1); // move cursor to second line "1" and 9 spaces over
  lcd.print(millis()/1000); // display seconds elapsed since power-up
  lcd.setCursor(0,1); // move to the begining of the second line
  
  // Read Analog Buttons
  lcd_key = read_LCD_buttons(); // read the buttons
  
  // Parse Analog Button instructions
  switch (lcd_key) {

    case btnRIGHT:
      lcd.print("RIGHT "); // Print RIGHT on LCD screen
      if((millis() - prev_trigger) > 500) { 
        message_count++;
        if(message_count > 3) message_count = 0;
        prev_trigger = millis();
      }
    break;

    case btnLEFT:
      lcd.print(adc_key_in);
      lcd.print(" v"); // ends with v(olt)
      if((millis() - prev_trigger) > 500) {
        message_count--;
        if(message_count == 255) message_count = 3;
        prev_trigger = millis();
      }
    break;
 
    case btnUP:
      lcd.print("UP    ");
      break;
 
    case btnDOWN:
      lcd.print("DOWN  ");
      break;
 
    case btnSELECT:
      lcd.print("SELECT");
      break;
 
    case btnNONE:
      lcd.print("TEST  ");
      break;

  }

  // If a button was pressed, check if a different message needs to be displayed
  if(lcd_key != btnNONE) {
    
    lcd.setCursor(0,0);
    switch(message_count) {
      
      case 0:
        lcd.print("Msg1 ");
        break;

      case 1:
        lcd.print("Msg2 ");
        break;

      case 2:
        lcd.print("Msg3 ");
        break;

      case 3:
        lcd.print("Msg4 ");
        break;
 
 
    lcd.setCursor(0,1);
    }
  }

  // Keypad Digital Buttons
  char key = keypad.getKey();
  lcd.setCursor(9,0);
  if (key) lcd.print(key);

} 