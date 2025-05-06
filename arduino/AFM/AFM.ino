/*
João Camacho, 2025

Code adapted from the WPSH203 LCD and
WPI300 Keypad Manuals. AND AFM PAPER REFERENCE

*/

/* ///////////////////
        INCLUDES
*/ ///////////////////

#include <LCD_I2C.h>    // I2C LCD.
#include <Keypad.h>     // For taking in user input.
#include "Keyboard.h"   // For sending commands to AFM computer.
#include <Wire.h>       // For I2C.

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
const int lcdHorizontalScrollStep = 4;   // How many characters are skipped per autoscroll step.
const int lcdHorizontalScrollMax = 750;  // Controlls how often the line display scrolls. ms.
const int lcdHorizontalScrollMin = -1310;// Reset tolerance in beginning and end of scroll. ms.
int lcdHorizontalScrollCounter1 = 0;     // Keeps track of how much time has passed since a scroll - Line 1. ms.
int lcdHorizontalScrollCounter2 = 0;     // Keeps track of how much time has passed since a scroll - Line 2. ms.
byte lcdHorizontalScrollPivot1 = 0;      // Keeps track of where the scroll is currently - Line 1.
byte lcdHorizontalScrollPivot2 = 0;      // Keeps track of where the scroll is currently - Line 2.
int lcdVerticalScrollCounter = 0;        // Allows for vertical scrolling by line, as per menu command.
int lcdVerticalScrollPivot = 0;          // Which element is being selected
const int lcdVerticalScrollMax = 1720;   // Vertical scroll amount of time on screen. ms.

// Display LCD data
char* lcdLine1 = " ";       // Line 1 of the LCD
char* lcdLine2 = " ";       // Line 2 of the LCD
String msg_operation = "";  // For intermediate conversions of Strings to c_str (char*)

const char * msg_scrollInstructions = "[*/#]: Scroll";
const char * msg_obfuscated = "################";

const char * msg_menu3[] = {"[0]: Settings", "[1]: Acquire", "[*]: Return"};
const int msg_menu3_size = 3;

const char * msg_menu2[] = {"[0]: Callibrate", "[#/*]: Scroll"};
const int msg_menu2_size = 2;

const char * msg_menu1[] = {"[0]: Settings", "[5]: View steps", "[4]: Move X-", "[6]: Move X+", "[8]: Move Y-", "[2]: Move Y+", "[#/*]: Scroll"};
const int msg_menu1_size = 7;

const char * msg_menu02[] = {"[0]: Motor X", "[1]: Motor Y", "[#/*]: Scroll"};
const int msg_menu02_size = 3;

// Motor Wiring Variables
const int dirPinX = 8;
const int stepPinX = 9;
const int enablePinX = 10;
const int dirPinY = 11;
const int stepPinY = 12;
const int enablePinY = 13;

// Debug Motor Variables
int debugMotorSteps = 1024;
long debugMotorDelay = 900000;

////////////////////////////// !!! AFM VARIABLES !!!

// Manual movement variables
long manual_delay = 900000;
int manual_steps = 128;

// Step counters
int steps_X = 0;
int steps_Y = 0;

// Acquisition variables
float Hz = 1;
int lines = 512;
char* setpoint = "0.2";


/* ///////////////////
        ROUTINES
*/ ///////////////////

void lcdPrint(int row, char* line, byte* pivot, int* clock); // Declaration so that the compiler doesn't scream at me.
void updateDisplay();

// Move one of the motors, given their pins. NOTE: ADD UPDATE TO POS_X AND POS_Y!!!
void moveMotor(int stepPin, int dirPin, int enablePin, int steps, long stepDelay, int direction, int* stepCounter){
  digitalWrite(enablePin, LOW);
  digitalWrite(dirPin, direction);
  for (int x = 0; x < steps; x++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay/2);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay/2);
  }
  digitalWrite(enablePin, HIGH);
  *stepCounter += steps;
}

// Acquire an AFM image.
void AFMing(char setpoint_char, int lines, float Hz){
    
    // Autotune - CHANGE DELAYS, WHAT SHOULD THEY BE? HOW COUKD WE DETECT? SHOULD WE AUTOTUNE ALL THE TIME?
    lcdLine1 = "Autotuning...";
    lcdLine2 = " ";
    updateDisplay();
    Keyboard.press(KEY_LEFT_ALT);Keyboard.press('v');delay(200);Keyboard.releaseAll();
    Keyboard.press(KEY_LEFT_ALT);Keyboard.press('w');delay(200);Keyboard.releaseAll();
    Keyboard.press(KEY_LEFT_ALT);Keyboard.press('c');delay(200);Keyboard.releaseAll();
    delay(500);
    Keyboard.press(KEY_LEFT_ALT);Keyboard.press('p');delay(200);Keyboard.releaseAll();
    Keyboard.press(KEY_LEFT_ALT);Keyboard.press('a');delay(200);Keyboard.releaseAll();
    Keyboard.press(KEY_LEFT_ALT);Keyboard.press('t');delay(200);Keyboard.releaseAll();
    delay(15000);
    Keyboard.press(KEY_LEFT_ALT);Keyboard.press('b');delay(200);Keyboard.releaseAll();
    delay(1000);
    
    // Engage
    Keyboard.press(KEY_LEFT_CTRL);Keyboard.press('e');delay(200);Keyboard.releaseAll();
    
    // Wait for completion of engage. NOTE: WHAT SHOULD THE THRESHOLD BE?
    lcdLine1 = "Engaging...";
    lcdLine2 = " ";
    updateDisplay();
    while( analogRead(micPin) < 800 ) {}
    delay(1500);
    
    // Amplitude Setpoint
    Keyboard.press(KEY_LEFT_ALT);Keyboard.press('p');delay(200);Keyboard.releaseAll();
    Keyboard.press(KEY_LEFT_ALT);Keyboard.press('f');delay(200);Keyboard.releaseAll();
    delay(500);
    Keyboard.press(KEY_DOWN_ARROW);delay(100);Keyboard.releaseAll();
    delay(200);
    Keyboard.press(KEY_DOWN_ARROW);delay(100);Keyboard.releaseAll();
    delay(200);
    Keyboard.press(KEY_DOWN_ARROW);delay(100);Keyboard.releaseAll();
    delay(200);
    Keyboard.press('0');delay(200);Keyboard.press('.');delay(200);Keyboard.press(setpoint_char);delay(200);Keyboard.press(KEY_RETURN);Keyboard.releaseAll();delay(200); //0.2 modify for your value
    
    //Start Frame Down
    Keyboard.press(KEY_LEFT_ALT);Keyboard.press('f');delay(200);Keyboard.releaseAll();
    Keyboard.press(KEY_LEFT_ALT);Keyboard.press('d');delay(200);Keyboard.releaseAll();    
    
    //Reverse to avoid distortion
    delay(12000);
    Keyboard.press(KEY_LEFT_ALT);Keyboard.press('f');delay(200);Keyboard.releaseAll();
    Keyboard.press(KEY_LEFT_ALT);Keyboard.press('r');delay(200);Keyboard.releaseAll();
    
    //Start Capture and withdraw when finished
    Keyboard.press(KEY_LEFT_ALT);Keyboard.press('c');delay(200);Keyboard.releaseAll();
    Keyboard.press(KEY_LEFT_ALT);Keyboard.press('w');delay(200);Keyboard.releaseAll();
    
    // Define Acquisition time-keeping varibles
    float acquisitionTime=((float) lines)*1000;
    acquisitionTime=acquisitionTime/Hz;
    acquisitionTime=acquisitionTime+20000; //Wait 512 lines/1 Hz = 512 s = 8min53s. We add 20s just in case.
    unsigned long sec_timer=0;
    unsigned StartTime = millis();
    unsigned secs = millis();
    unsigned CurrentTime = millis();

    // Wait until image is done.
    while((CurrentTime - StartTime) < ((unsigned long) acquisitionTime)){
      secs=millis();
      CurrentTime = millis();
      while((CurrentTime-secs) < 1000) {
        CurrentTime=millis();
      }
      sec_timer=sec_timer+1;
      
      msg_operation = "Time (s): "+String((int)sec_timer);
      lcdLine1 = msg_operation.c_str();
      lcdLine2 = " ";
      updateDisplay();   
    }
    
    return 0;  
}

// AFM Acquire image and wait for beep

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
  lcdVerticalScrollPivot = 0;
  lcdVerticalScrollCounter = 0;
}

// Update LCD Display
void updateDisplay(){
  lcdPrint(0, lcdLine1, &lcdHorizontalScrollPivot1, &lcdHorizontalScrollCounter1);
  lcdPrint(1, lcdLine2, &lcdHorizontalScrollPivot2, &lcdHorizontalScrollCounter2);
}

// Menu loop
void menuLoop(){
  switch (menu) {

    case 10:
      msg_operation = "X="+String(steps_X)+" Y="+String(steps_Y);
      lcdLine1 = msg_operation.c_str();
      lcdLine2 = "Any key - Return";
      if (key) switchMenu(menu/10);
      break;
    
    case 3:
      lcdLine1 = "ACQUISITION - Acquire AFM images.";
      if (lcdVerticalScrollPivot >= msg_menu3_size) lcdVerticalScrollPivot = 0;
      lcdLine2 = msg_menu3[lcdVerticalScrollPivot];
      if (key == '*') switchMenu(menu-1);
      break;

    case 2:
      lcdLine1 = "CALLIBRATE - Callibrate AFM.";
      if (lcdVerticalScrollPivot >= msg_menu2_size) lcdVerticalScrollPivot = 0;
      lcdLine2 = msg_menu2[lcdVerticalScrollPivot];
      if (key == '*') switchMenu(menu-1);
      if (key == '#') switchMenu(menu+1);
      break;

    case 1:
      lcdLine1 = "MANUAL - Manually control the stepper motors.";
      if (lcdVerticalScrollPivot >= msg_menu1_size) lcdVerticalScrollPivot = 0;
      lcdLine2 = msg_menu1[lcdVerticalScrollPivot];
      if (key == '*') switchMenu(menu-1);
      if (key == '#') switchMenu(menu+1);
      if (key == '5') switchMenu(menu*10);
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
      if (lcdVerticalScrollPivot >= msg_menu02_size) lcdVerticalScrollPivot = 0;
      lcdLine2 = msg_menu02[lcdVerticalScrollPivot];
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
      moveMotor(stepPinX, dirPinX, enablePinX, debugMotorSteps, debugMotorDelay, 0, &steps_X);
      switchMenu(menu/10);
      break;
    
    case -21:
      lcdLine1 = "Powering Y Motor";
      lcdLine2 = "";
      updateDisplay();
      moveMotor(stepPinY, dirPinY, enablePinY, debugMotorSteps, debugMotorDelay, 0, &steps_Y);
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
  lcdVerticalScrollCounter += loopDt;

  // Analog Sensors Input Acquire
  micAnalog = analogRead(micPin);
  rotaryAnalogX = analogRead(rotaryXPin);
  rotaryAnalogY = analogRead(rotaryYPin);

  // Keypad Digital Buttons Acquire
  key = keypad.getKey();

  // Menu System
  menuLoop();

  // Vertical Scrolling
  if (lcdVerticalScrollCounter >= lcdVerticalScrollMax) { lcdVerticalScrollCounter = 0; lcdVerticalScrollPivot += 1; }

  // Update LCD Display
  updateDisplay();

  // Time keeping
  endLoopTime = millis();
  loopDt = endLoopTime - startLoopTime;

} 