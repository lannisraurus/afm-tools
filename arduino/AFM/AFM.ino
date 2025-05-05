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

const byte lcdAddress = 0x27;   // LCD I2C Address - Change according to your module.
LCD_I2C lcd(lcdAddress);        // Default address of most PCF8574 modules.

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
byte rowPins[ROWS] = {A5, 4, 5, 6}; // Row pins. Change accordingly.
byte colPins[COLS] = {0, 1, A4};    // Collumn pins. Change accordingly.
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

/* /////////////////////////
        GLOBAL VARIABLES
*/ /////////////////////////

// Analog Sensor Data
int micAnalog = -1;     // Microphone reading (buzzer).
int rotaryAnalogX = -1; // First Rotary Encoder X.
int rotaryAnalogY = -1; // Second Rotary Encoder Y.
const int micPin = A0;        // Microphone data pin. Change accordingly.
const int rotaryXPin = A1;    // Rotary Encoder 1 pin. Change accordingly.
const int rotaryYPin = A2;    // Rotary Encoder 2 pin. Change accordingly.

// Keypad input
char key = ' ';

// LCD Specs
const unsigned int lcdWidth = 16;  // LCD Specs
const unsigned int lcdHeight = 2;  // LCD Specs

// Menu variables
int menu = 0;                      // Which menu the user is selecting.

// Timekeeping variables
unsigned int startLoopTime = 0;    // Keeps track of the times in which the main loop starts. ms.
unsigned int endLoopTime = 0;      // Keeps track of the times in which the main loop ends. ms.
unsigned int loopDt = 0;           // Difference between the two previous variables. ms.

// Autoscrolling variables
const int lcdHorizontalScrollStep = 5;   // How many characters are skipped per autoscroll step.
const int lcdHorizontalScrollMax = 750;  // Controlls how often the line display scrolls. ms.
const int lcdHorizontalScrollMin = -1110;// Reset tolerance in beginning and end of scroll. ms.
int lcdHorizontalScrollCounter1 = 0;     // Keeps track of how much time has passed since a scroll - Line 1. ms.
int lcdHorizontalScrollCounter2 = 0;     // Keeps track of how much time has passed since a scroll - Line 2. ms.
byte lcdHorizontalScrollPivot1 = 0;      // Keeps track of where the scroll is currently - Line 1.
byte lcdHorizontalScrollPivot2 = 0;      // Keeps track of where the scroll is currently - Line 2.

// Display LCD data
char* lcdLine1 = " ";       // Line 1 of the LCD
char* lcdLine2 = " ";       // Line 2 of the LCD
String msg_operation = "";  // For intermediate conversions of Strings to c_str (char*)
char * msg_scrollInstructions = "[*/#]: Scroll";
char * msg_obfuscated = "################";

// Motor Wiring Variables
const int dirPinX = 8;
const int stepPinX = 9;
const int enablePinX = 10;
const int dirPinY = 11;
const int stepPinY = 12;
const int enablePinY = 13;

// !!! AFM VARIABLES !!!

/* ///////////////////
        ROUTINES
*/ ///////////////////

// Move one of the motors, given their pins.
void moveMotor(int stepPin, int dirPin, int enablePin, int steps, int stepDelay, int direction){
  digitalWrite(enablePin, LOW);
  digitalWrite(dirPin, direction);
  for (int x = 0; x < steps; x++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay);
  }
  digitalWrite(enablePin, HIGH);
}

/* ///////////////////////
        UI FUNCTIONS
*/ ///////////////////////

// Prints a message to a given row of the LCD screen and allows for auto-scrolling.
void lcdPrint(int row, char* line, byte* pivot, int* clock){

  // Msg size and set cursor in the correct position
  int msgSize = strlen(line);
  lcd.setCursor(0,row);
  
  // Text bigger than LCD display - autoscroll
  if (msgSize > lcdWidth) {

    // Select appropriate lcdWidth character long substring.
    char msg[lcdWidth + 1];

    // Clamp pivot
    if (*pivot > msgSize - lcdWidth) {
      *pivot = msgSize - lcdWidth;
    }

    // Autoscroll Print
    if (*clock >= lcdHorizontalScrollMax) {
      strncpy(msg, line + *pivot, lcdWidth);
      msg[lcdWidth] = '\0';
      lcd.print(msg);
      *clock = 0;
      if (*pivot == 0) *clock = lcdHorizontalScrollMin;
      *pivot += lcdHorizontalScrollStep;
    }
    
    // Restart
    if (*pivot >= msgSize - lcdWidth + lcdHorizontalScrollStep){
      *clock = lcdHorizontalScrollMin;
      *pivot = 0;
    }

  // Text fits - no autoscroll
  } else {

    // Make strings fit the LCD
    char msg[lcdWidth + 1];
    memset(msg, ' ', lcdWidth);
    strncpy(msg, line, strlen(line));
    msg[lcdWidth] = '\0';

    // Print and reset autoscroll quantities.
    lcd.print(msg);
    *pivot = 0;
    *clock = 0;

  }
}

// Used for switching between menus
void switchMenu(int m){
  menu = m;
  lcdHorizontalScrollPivot1 = 0;
  lcdHorizontalScrollPivot2 = 0;
  lcdHorizontalScrollCounter1 = lcdHorizontalScrollMin;
  lcdHorizontalScrollCounter2 = lcdHorizontalScrollMin;
  lcdLine1 = msg_obfuscated;
  lcdLine2 = msg_obfuscated;
}

// Update LCD Display
void updateDisplay(){
  lcdPrint(0, lcdLine1, &lcdHorizontalScrollPivot1, &lcdHorizontalScrollCounter1);
  lcdPrint(1, lcdLine2, &lcdHorizontalScrollPivot2, &lcdHorizontalScrollCounter2);
}

// Menu loop
void menuLoop(){
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
      lcdLine1 = "WELCOME! - AFM High-Range Automation @ IST";;
      lcdLine2 = msg_scrollInstructions;
      if (key == '*') switchMenu(menu-1);
      if (key == '#') switchMenu(menu+1);
      break;
    
    case -1:
      lcdLine1 = "CREDITS - This project was developed by Joao Camacho [106224], 2025, under the supervision of Prof. Dr. Luis Viseu Melo, in the context of the PIC-I project.";
      lcdLine2 = msg_scrollInstructions;
      if (key == '*') switchMenu(menu-1);
      if (key == '#') switchMenu(menu+1);
      break;
    
    case -2:
      lcdLine1 = "MOTOR DEBUG - Activate both motors to check if the wiring is correct.";
      lcdLine2 = "[0]: Start Motor X, [1]: Start Motor Y, [#/*]: Scroll";
      if (key == '#') switchMenu(menu+1);
      if (key == '*') switchMenu(menu-1);
      if (key == '0') switchMenu(menu*10);
      if (key == '1') switchMenu(menu*10-1);
      break;
    
    case -3:
      lcdLine1 = "SENSOR DATA - M: Microphone; X: Rotary Encoder X; Y: Rotary Encoder Y; [#] to Return.";
      msg_operation = "M"+String(micAnalog)+" X"+String(rotaryAnalogX)+" Y"+String(rotaryAnalogY);
      lcdLine2 = msg_operation.c_str();
      if (key == '#') switchMenu(menu+1);
      break;
    
    case -20:
      lcdLine1 = "Powering X Motor";
      lcdLine2 = "";
      updateDisplay();
      moveMotor(stepPinX, dirPinX, enablePinX, 4000, 500, 0);
      switchMenu(menu/10);
      break;
    
    case -21:
      lcdLine1 = "Powering Y Motor";
      lcdLine2 = "";
      updateDisplay();
      moveMotor(stepPinY, dirPinY, enablePinY, 4000, 500, 0);
      switchMenu(menu/10);
      break;

    default:
      lcdLine1 = "ERR: MENU UNSET!";
      lcdLine2 = ":(";
      if (key) switchMenu(0);
      break;
  }
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
  // Motor setup
  pinMode(dirPinX, OUTPUT);
  pinMode(stepPinX, OUTPUT);
  pinMode(enablePinX, OUTPUT);
  pinMode(dirPinY, OUTPUT);
  pinMode(stepPinY, OUTPUT);
  pinMode(enablePinY, OUTPUT);
  digitalWrite(enablePinX, HIGH);
  digitalWrite(enablePinY, HIGH);
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
  rotaryAnalogX = analogRead(rotaryXPin);
  rotaryAnalogY = analogRead(rotaryYPin);

  // Keypad Digital Buttons Acquire
  key = keypad.getKey();

  // Menu System
  menuLoop();

  // Update LCD Display
  updateDisplay();

  // Time keeping
  endLoopTime = millis();
  loopDt = endLoopTime - startLoopTime;

} 