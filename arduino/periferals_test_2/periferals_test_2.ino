/*
João Camacho, 2025

Code adapted from the WPSH203 LCD and
WPI300 Keypad Manuals. AND AFM PAPER REFERENCE

*/

/* ///////////////////
        INCLUDES
*/ ///////////////////

#include <LCD_I2C.h>
#include <Keypad.h>
#include <Wire.h>

/* ////////////////////
        LCD SCREEN
*/ ////////////////////

LCD_I2C lcd(0x27); // Default address of most PCF8574 modules, change according

/* ////////////////
        KEYPAD
*/ ////////////////

const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
 {'1','2','3'},
 {'4','5','6'},
 {'7','8','9'},
 {'*','0','#'}
};
byte rowPins[ROWS] = {A5, 4, 5, 6}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {0, 1, A4}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

/* /////////////////////////
        GLOBAL VARIABLES
*/ /////////////////////////

// Analog Sensor Data
int micAnalog = -1;     // Microphone reading (buzzer).
int rotaryAnalog1 = -1; // First Rotary Encoder.
int rotaryAnalog2 = -1; // Second Rotary Encoder.
int micPin = A0;        // Microphone data pin.
int rotary1Pin = A1;    // Rotary Encoder 1 pin.
int rotary2Pin = A2;    // Rotary Encoder 2 pin.

// UI Variables
const unsigned int lcdWidth = 16;  // LCD Specs
const unsigned int lcdHeight = 2;  // LCD Specs

int menu = 0;                      // Which menu the user is selecting.

unsigned int startLoopTime = 0;    // Keeps track of the times in which the main loop starts. ms.
unsigned int endLoopTime = 0;      // Keeps track of the times in which the main loop ends. ms.
unsigned int loopDt = 0;           // Difference between the two previous variables. ms.

const int lcdHorizontalScrollMax = 350;  // Controlls how often the line display scrolls. ms.
const int lcdHorizontalScrollMin = -1110;// Reset tolerance in beginning and end of scroll. ms.
int lcdHorizontalScrollCounter1 = 0;     // Keeps track of how much time has passed since a scroll - Line 1. ms.
int lcdHorizontalScrollCounter2 = 0;     // Keeps track of how much time has passed since a scroll - Line 2. ms.
byte lcdHorizontalScrollPivot1 = 0;      // Keeps track of where the scroll is currently - Line 1.
byte lcdHorizontalScrollPivot2 = 0;      // Keeps track of where the scroll is currently - Line 2.

char* lcdLine1 = " ";     // Line 1 of the LCD
char* lcdLine2 = " ";     // Line 2 of the LCD

String msg_operation = "";
char * msg_scrollInstructions = "Press [*] to scroll left and [#] to scroll right.";
char * msg_credits = "CREDITS - This project was developed by Joao Camacho [106224], 2025, under the supervision of Prof. Dr. Luis Viseu Melo, in the context of the PIC-I project.";
char * msg_splashScreen = "WELCOME! - AFM High-Range Automation @ IST";
char * msg_obfuscated = "################";

/* ///////////////////////
        UI FUNCTIONS
*/ ///////////////////////

// Prints a message to a given row of the LCD screen and allows for auto-scrolling.
void lcdPrint(int row, char* line, byte* pivot, int* clock){

  // Msg size and se cursor in the correct position
  int msgSize = strlen(line);
  lcd.setCursor(0,row);
  
  // Text bigger than LCD display - autoscroll
  if (msgSize > lcdWidth) {

    // Select appropriate lcdWidth character long substring.
    char msg[lcdWidth + 1];
    strncpy(msg, line + *pivot, lcdWidth);
    msg[lcdWidth] = '\0';

    // Autoscroll
    if (*clock >= lcdHorizontalScrollMax){
      lcd.print(msg);
      *clock = 0;
      if (*pivot == 0) *clock = lcdHorizontalScrollMin;
      *pivot += 1;
    }
    
    // Reached end
    if (msgSize - *pivot - lcdWidth + 1 <= 0){
      *clock = lcdHorizontalScrollMin;
      *pivot = 0;
      lcd.print(msg);
    }

  // Text fits - no autoscroll
  } else {

    // Make strings fit the LCD
    char msg[lcdWidth + 1];
    memset(msg, ' ', 16);
    strncpy(msg, line, strlen(line));
    msg[lcdWidth] = '\0';

    // Print and reset autoscroll quantities.
    lcd.print(msg);
    *pivot = 0;
    *clock = 0;

  }
}

void switchMenu(int m){
  menu = m;
  lcdHorizontalScrollPivot1 = 0;
  lcdHorizontalScrollPivot2 = 0;
  lcdHorizontalScrollCounter1 = lcdHorizontalScrollMin;
  lcdHorizontalScrollCounter2 = lcdHorizontalScrollMin;
  lcdLine1 = msg_obfuscated;
  lcdLine2 = msg_obfuscated;
}

/* ////////////////
        SETUP
*/ ////////////////

void setup(){
  // Setup LCD
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  // Serial Setup
  Serial.begin(9600);
}

/* ////////////////
        LOOP
*/ ////////////////

void loop() {

  // Time keeping
  startLoopTime = millis();

  // Update counters
  lcdHorizontalScrollCounter1 += loopDt;
  lcdHorizontalScrollCounter2 += loopDt;

  // Analog Sensors Input Acquire
  micAnalog = analogRead(micPin);
  rotaryAnalog1 = analogRead(rotary1Pin);
  rotaryAnalog2 = analogRead(rotary2Pin);

  // Keypad Digital Buttons Acquire
  char key = keypad.getKey();

  // Menu System
  switch (menu) {
    
    case 2:
      lcdLine1 = "ACQUISITION - Acquire AFM images.";
      lcdLine2 = "[0]: Configure, [1]: Begin Acquisition, [*]: Return";
      if (key == '*') switchMenu(menu-1);
      break;

    case 1:
      lcdLine1 = "MANUAL - Manually control the stepper motors.";
      lcdLine2 = "[0]: Configure, [1]: Move X, [2]: Move Y, [#/*]: Scroll";
      if (key == '*') switchMenu(menu-1);
      if (key == '#') switchMenu(menu+1);
      break;

    case 0:
      lcdLine1 = msg_splashScreen;
      lcdLine2 = msg_scrollInstructions;
      if (key == '*') switchMenu(menu-1);
      if (key == '#') switchMenu(menu+1);
      break;
    
    case -1:
      lcdLine1 = msg_credits;
      lcdLine2 = msg_scrollInstructions;
      if (key == '*') switchMenu(menu-1);
      if (key == '#') switchMenu(menu+1);
      break;
    
    case -2:
      lcdLine1 = "MOTOR DEBUGGING - Activate both motors to see if they work.";
      lcdLine2 = "[0]: Start, [#/*]: Scroll";
      if (key == '#') switchMenu(menu+1);
      if (key == '*') switchMenu(menu-1);
      if (key == '0') switchMenu(menu*10);
      break;
    
    case -3:
      lcdLine1 = "SENSOR DATA - M: Microphone; X: Rotary Encoder 1; Y: Rotary Encoder 2; [#] to Return.";
      msg_operation = "M"+String(micAnalog)+" X"+String(rotaryAnalog1)+" Y"+String(rotaryAnalog2);
      lcdLine2 = msg_operation.c_str();
      if (key == '#') switchMenu(menu+1);
      break;
    
    case -20:
      lcdLine1 = "Powering...";
      lcdLine2 = "Press 0 to return.";
      if (key == '0') switchMenu(-2);
      break;

    default:
      lcdLine1 = "ERR: MENU UNSET!";
      lcdLine2 = ":(";
      if (key) switchMenu(0);
      break;
  }

  // Update LCD Display
  lcdPrint(0, lcdLine1, &lcdHorizontalScrollPivot1, &lcdHorizontalScrollCounter1);
  lcdPrint(1, lcdLine2, &lcdHorizontalScrollPivot2, &lcdHorizontalScrollCounter2);

  // Time keeping
  endLoopTime = millis();
  loopDt = endLoopTime - startLoopTime;

} 