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
#include <EEPROM.h>     // For keep variables from shutdown.

/* /////////////////////////
        PRE-DECLARATIONS
*/ /////////////////////////

void lcdPrint(int, char*, byte*, int*);
void updateDisplay();

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

/* //////////////
        PINS
*/ //////////////

// Analog Sensor Data
int micAnalog = -1;     // Microphone reading (buzzer).
int rotaryAnalogX = -1; // First Rotary Encoder X.
int rotaryAnalogY = -1; // Second Rotary Encoder Y.
const byte micPin = A0;        // Microphone data pin. Change accordingly.
const byte rotaryXPin = A1;    // Rotary Encoder 1 pin. Change accordingly.
const byte rotaryYPin = A2;    // Rotary Encoder 2 pin. Change accordingly.

// Motor Wiring Variables
const byte dirPinX = 8;
const byte stepPinX = 9;
const byte enablePinX = 10;
const byte dirPinY = 11;
const byte stepPinY = 12;
const byte enablePinY = 13;

/* ///////////////////////
        UI VARIABLES
*/ ///////////////////////

// Keypad input
char key = ' ';

// LCD Specs
const byte lcdWidth = 16;  // LCD Specs
const byte lcdHeight = 2;  // LCD Specs

// Menu variables
int menu = 0;                      // Which menu the user is selecting.

// Timekeeping variables
unsigned int startLoopTime = 0;    // Keeps track of the times in which the main loop starts. ms.
unsigned int endLoopTime = 0;      // Keeps track of the times in which the main loop ends. ms.
unsigned int loopDt = 0;           // Difference between the two previous variables. ms.

// Autoscrolling variables
const byte lcdHorizontalScrollStep = 4;   // How many characters are skipped per autoscroll step.
const int lcdHorizontalScrollMax = 750;  // Controlls how often the line display scrolls. ms.
const int lcdHorizontalScrollMin = -1310;// Reset tolerance in beginning and end of scroll. ms.
int lcdHorizontalScrollCounter1 = 0;     // Keeps track of how much time has passed since a scroll - Line 1. ms.
int lcdHorizontalScrollCounter2 = 0;     // Keeps track of how much time has passed since a scroll - Line 2. ms.
byte lcdHorizontalScrollPivot1 = 0;      // Keeps track of where the scroll is currently - Line 1.
byte lcdHorizontalScrollPivot2 = 0;      // Keeps track of where the scroll is currently - Line 2.
int lcdVerticalScrollCounter = 0;        // Allows for vertical scrolling by line, as per menu command.
byte lcdVerticalScrollPivot = 0;          // Which element is being selected
const int lcdVerticalScrollMax = 1720;   // Vertical scroll amount of time on screen. ms.

// Display LCD data
char* lcdLine1 = " ";       // Line 1 of the LCD. Set variable to write.
char* lcdLine2 = " ";       // Line 2 of the LCD. Set variable to write.
String msg_operation = "";  // For intermediate conversions of Strings to c_str (char*) and/or user input.
const char * msg_menu162[] = {"[0]: \xB5m", "[1]: nm"};
const byte msg_menu162_size = 2;
const char * msg_menu17[] = {"[*]: Left", "[#]: Right", "[0]: Return", "[1]: Delete"};
const byte msg_menu17_size = 4;
const char * msg_menu16[] = {"[#]: Proceed", "[*]: Return"};
const byte msg_menu16_size = 2;
const char * msg_menu15[] = {"[0]: Steps", "[1]: Delay", "[*/#]: Return"};
const byte msg_menu15_size = 3;
const char * msg_menu2[] = {"[0]: Settings", "[1]: Acquire", "[*]: Return"};
const byte msg_menu2_size = 3;
const char * msg_menu1[] = {"[0]: Settings", "[1]: Callibrate", "[3]: Edit Cal.", "[5]: View steps", "[4]: Move X-", "[6]: Move X+", "[8]: Move Y-", "[2]: Move Y+", "[#/*]: Scroll"};
const byte msg_menu1_size = 9;
const char * msg_menu02[] = {"[0]: Proceed", "[#]: Return"};
const byte msg_menu02_size = 2;

// Callibration point scroller
byte calPointMenuPivot = 0;

/* /////////////////////////////////////////////
       !!! AFM VARIABLES - NOT IN EEPROM !!!
*/ /////////////////////////////////////////////

// EEPROM verification ID. Change for hard reset.
const byte eepromID = 67;

// Step counters
long steps_X = 0; // steps
long steps_Y = 0; // steps

// Callibration
const byte callibrationPointsMax = 6;
byte callibrationStep = 0; // Internal variable that tracks where in the callibration procedure the user is.
                          // 0: add point; 1: insert displacement.
long xCallibrationStepsStart = 0; // steps
long yCallibrationStepsStart= 0;  // steps
long xCallibrationStepsStop = 0;  // steps
long yCallibrationStepsStop = 0;  // steps
float xCallibrationMeasure = 0;   // nm
float yCallibrationMeasure = 0;   // nm

// AFM Keyboard pressing
const long timeBetweenKeys = 250; // ms
const long timeAutotune = 20000;  // ms

// AFM Mic Threshold
const int micThreshold = 800; // out of 1024, sensor units

// AFM Conversion variables
float xStepsToRealMean = 0;
float yStepsToRealMean = 0;
byte xStepsToRealN = 0;
byte yStepsToRealN = 0;

/* ///////////////////////////////////////
       !!! AFM VARIABLES - EEPROM !!!
*/ ///////////////////////////////////////

// Manual movement variables
long manual_delay;  // microseconds
long manual_steps;  // steps

// Callibration
float xStepsToReal[callibrationPointsMax];  // nm/step
float yStepsToReal[callibrationPointsMax];  // nm/step
byte stepsToRealPivot;                      // How many points exist + 1

// Acquisition variables
int Hz;       // Acquisition Frequency in MILLIHERTZ.
int lines;      // Lines.
char setpoint; // Amplitude setpoint character.
byte rows;      // How many images to acquire in a row.
byte cols;      // How many images to acquire in a collumn.
int imgStep;    // How many nanometres to displace between each image.

/* ////////////////////////////
       EEPROM AND RESETTING
*/ ////////////////////////////
// NOTE: VARIABLES MUST BE IN THE SAME ORDER IN ALL THE FUNCTIONS!!!

// Set all AFM settings to default.
void resetSettings(){
  manual_delay = 1500000;
  manual_steps = 128;
  stepsToRealPivot = 0;
  for (int i = 0; i < callibrationPointsMax; i++) xStepsToReal[i] = 0;
  for (int i = 0; i < callibrationPointsMax; i++) yStepsToReal[i] = 0;
  Hz = 1;
  lines = 512;
  setpoint = '2';
  rows = 3;
  cols = 3;
  imgStep = 1000;
}

// Load data from the EEPROM and report on success.
int loadFromEEPROM(){
  // Verify if eepromID is the same
  byte eepromIDverify = 0;
  EEPROM.get(0, eepromIDverify);
  if (eepromIDverify == eepromID) {
    // ID was correct. Load all data
    int eepromPivot = 1;
    EEPROM.get(eepromPivot, manual_delay); eepromPivot += sizeof(manual_delay);
    EEPROM.get(eepromPivot, manual_steps); eepromPivot += sizeof(manual_steps);
    EEPROM.get(eepromPivot, stepsToRealPivot); eepromPivot += sizeof(stepsToRealPivot);
    EEPROM.get(eepromPivot, xStepsToReal); eepromPivot += sizeof(xStepsToReal);
    EEPROM.get(eepromPivot, yStepsToReal); eepromPivot += sizeof(yStepsToReal);
    EEPROM.get(eepromPivot, Hz); eepromPivot += sizeof(Hz);
    EEPROM.get(eepromPivot, lines); eepromPivot += sizeof(lines);
    EEPROM.get(eepromPivot, setpoint); eepromPivot += sizeof(setpoint);
    EEPROM.get(eepromPivot, rows); eepromPivot += sizeof(rows);
    EEPROM.get(eepromPivot, cols); eepromPivot += sizeof(cols);
    EEPROM.get(eepromPivot, imgStep); eepromPivot += sizeof(imgStep);
    return 0;
  }
  return -99;
}

// Save data to EEPROM.
void saveToEEPROM(){
  int eepromPivot = 0;
  EEPROM.put(eepromPivot, eepromID); eepromPivot += sizeof(eepromID);
  EEPROM.put(eepromPivot, manual_delay); eepromPivot += sizeof(manual_delay);
  EEPROM.put(eepromPivot, manual_steps); eepromPivot += sizeof(manual_steps);
  EEPROM.put(eepromPivot, stepsToRealPivot); eepromPivot += sizeof(stepsToRealPivot);
  EEPROM.put(eepromPivot, xStepsToReal); eepromPivot += sizeof(xStepsToReal);
  EEPROM.put(eepromPivot, yStepsToReal); eepromPivot += sizeof(yStepsToReal);
  EEPROM.put(eepromPivot, Hz); eepromPivot += sizeof(Hz);
  EEPROM.put(eepromPivot, lines); eepromPivot += sizeof(lines);
  EEPROM.put(eepromPivot, setpoint); eepromPivot += sizeof(setpoint);
  EEPROM.put(eepromPivot, rows); eepromPivot += sizeof(rows);
  EEPROM.put(eepromPivot, cols); eepromPivot += sizeof(cols);
  EEPROM.put(eepromPivot, imgStep); eepromPivot += sizeof(imgStep);
}

/* /////////////////////////////////
        INSTRUMENTATION ROUTINES
*/ /////////////////////////////////

// Move one of the motors, given their pins. NOTE: ADD UPDATE TO POS_X AND POS_Y!!!
void moveMotor(int stepPin, int dirPin, int enablePin, long steps, long stepDelay, int direction, long* stepCounter){
  digitalWrite(enablePin, LOW);
  digitalWrite(dirPin, direction);
  for (int x = 0; x < steps; x++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay/2);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay/2);
  }
  digitalWrite(enablePin, HIGH);
  if (direction) *stepCounter += steps; else *stepCounter -= steps;
  
}

// Acquire an AFM image.
void acquireAFMImage(char setpoint_char, int lines, float Hz){
    
    // AUTOTUNE
    lcdLine1 = "Autotuning...";
    lcdLine2 = " ";
    updateDisplay();
    Keyboard.press(KEY_LEFT_ALT); Keyboard.press('v'); delay(timeBetweenKeys); Keyboard.releaseAll();
    Keyboard.press(KEY_LEFT_ALT); Keyboard.press('w'); delay(timeBetweenKeys); Keyboard.releaseAll();
    Keyboard.press(KEY_LEFT_ALT); Keyboard.press('c'); delay(timeBetweenKeys); Keyboard.releaseAll();
    delay(2*timeBetweenKeys);
    Keyboard.press(KEY_LEFT_ALT); Keyboard.press('p'); delay(timeBetweenKeys); Keyboard.releaseAll();
    Keyboard.press(KEY_LEFT_ALT); Keyboard.press('a'); delay(timeBetweenKeys); Keyboard.releaseAll();
    Keyboard.press(KEY_LEFT_ALT); Keyboard.press('t'); delay(timeBetweenKeys); Keyboard.releaseAll();
    delay(timeAutotune);
    Keyboard.press(KEY_LEFT_ALT); Keyboard.press('b'); delay(timeBetweenKeys); Keyboard.releaseAll();
    delay(4*timeBetweenKeys);
    
    // ENGAGE
    lcdLine1 = "Engaging...";
    lcdLine2 = "Waiting for beep";
    updateDisplay();
    Keyboard.press(KEY_LEFT_CTRL); Keyboard.press('e'); delay(timeBetweenKeys); Keyboard.releaseAll();
    while( analogRead(micPin) < micThreshold ) {}
    delay(6*timeBetweenKeys);
    
    // AMPLITUDE SETPOINT
    lcdLine1 = "Setting...";
    lcdLine2 = "Amp. Setpoint";
    updateDisplay();
    Keyboard.press(KEY_LEFT_ALT); Keyboard.press('p'); delay(timeBetweenKeys); Keyboard.releaseAll();
    Keyboard.press(KEY_LEFT_ALT); Keyboard.press('f'); delay(timeBetweenKeys); Keyboard.releaseAll();
    delay(2*timeBetweenKeys);
    Keyboard.press(KEY_DOWN_ARROW); delay(timeBetweenKeys); Keyboard.releaseAll();
    delay(2*timeBetweenKeys);
    Keyboard.press(KEY_DOWN_ARROW); delay(timeBetweenKeys); Keyboard.releaseAll();
    delay(2*timeBetweenKeys);
    Keyboard.press(KEY_DOWN_ARROW); delay(timeBetweenKeys); Keyboard.releaseAll();
    delay(2*timeBetweenKeys);
    Keyboard.press('0'); delay(timeBetweenKeys);
    Keyboard.press('.'); delay(timeBetweenKeys);
    Keyboard.press(setpoint_char); delay(timeBetweenKeys);
    Keyboard.press(KEY_RETURN); delay(timeBetweenKeys); Keyboard.releaseAll();
    delay(2*timeBetweenKeys);

    // MAYBE SET OTHER VARIABLES (HZ AND LINES) ALSO ???
    
    // START FRAME DOWN
    lcdLine1 = "Setting...";
    lcdLine2 = "Capturing";
    updateDisplay();
    Keyboard.press(KEY_LEFT_ALT); Keyboard.press('f'); delay(timeBetweenKeys); Keyboard.releaseAll();
    Keyboard.press(KEY_LEFT_ALT); Keyboard.press('d'); delay(timeBetweenKeys); Keyboard.releaseAll();    
    
    // REVERSE TO PREVENT DRIFT
    delay(48*timeBetweenKeys);
    Keyboard.press(KEY_LEFT_ALT); Keyboard.press('f'); delay(timeBetweenKeys); Keyboard.releaseAll();
    Keyboard.press(KEY_LEFT_ALT); Keyboard.press('r'); delay(timeBetweenKeys); Keyboard.releaseAll();
    
    // BEGIN CAPTURING
    Keyboard.press(KEY_LEFT_ALT); Keyboard.press('c'); delay(timeBetweenKeys); Keyboard.releaseAll();
    Keyboard.press(KEY_LEFT_ALT); Keyboard.press('w'); delay(timeBetweenKeys); Keyboard.releaseAll();
    
    // TIME KEEPING VARIABLES
    float acquisitionTime=((float) lines)*1000; 
    acquisitionTime /= float(Hz);   // 512 lines/1 Hz = 512 s = 8min53s.
    acquisitionTime += 20000;       // +20s for safety
    unsigned long sec_timer = 0;
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
      
      msg_operation = "Time (s): "+ String((int)sec_timer);
      lcdLine1 = msg_operation.c_str();
      lcdLine2 = " ";
      updateDisplay();   
    }
    
    return 0;  
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
  lcdLine1 = "################";
  lcdLine2 = "################";
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

    case 205:
      lcdLine1 = "INSERT IMAGE STEP IN NANOMETRES ([#] to proceed, [*] to erase, numbers to write).";
      if (key == '#') {
        imgStep = msg_operation.toInt();
        saveToEEPROM();
        msg_operation = " ";
        switchMenu(2);
      } else if (key == '*' && msg_operation.length() > 0) {
        msg_operation = msg_operation.substring(0, msg_operation.length() - 1);
      } else if (key && key != '*' && key != '#') msg_operation += key;
      lcdLine2 = msg_operation.c_str();
      break;

    case 204:
      lcdLine1 = "INSERT AMP. SETPOINT ([#] to proceed).";
      if (key == '#') {
        saveToEEPROM();
        msg_operation = String(imgStep);
        switchMenu(205);
      } else if (key && key != '*' && key != '#') {msg_operation = "0."+String(key); setpoint = key;}
      lcdLine2 = msg_operation.c_str();
      break;

    case 203:
      lcdLine1 = "INSERT HZ ([#] to proceed, [*] to erase, numbers to write).";
      if (key == '#') {
        Hz = msg_operation.toInt();
        saveToEEPROM();
        msg_operation = "0."+String(setpoint);
        switchMenu(204);
      } else if (key == '*' && msg_operation.length() > 0) {
        msg_operation = msg_operation.substring(0, msg_operation.length() - 1);
      } else if (key && key != '*' && key != '#') msg_operation += key;
      lcdLine2 = msg_operation.c_str();
      break;

    case 202:
      lcdLine1 = "INSERT LINES ([#] to proceed, [*] to erase, numbers to write).";
      if (key == '#') {
        lines = msg_operation.toInt();
        saveToEEPROM();
        msg_operation = String(Hz);
        switchMenu(203);
      } else if (key == '*' && msg_operation.length() > 0) {
        msg_operation = msg_operation.substring(0, msg_operation.length() - 1);
      } else if (key && key != '*' && key != '#') msg_operation += key;
      lcdLine2 = msg_operation.c_str();
      break;

    case 201:
      lcdLine1 = "INSERT COLS ([#] to proceed, [*] to erase, numbers to write).";
      if (key == '#') {
        cols = msg_operation.toInt();
        saveToEEPROM();
        msg_operation = String(lines);
        switchMenu(202);
      } else if (key == '*' && msg_operation.length() > 0) {
        msg_operation = msg_operation.substring(0, msg_operation.length() - 1);
      } else if (key && key != '*' && key != '#') msg_operation += key;
      lcdLine2 = msg_operation.c_str();
      break;

    case 200:
      lcdLine1 = "INSERT ROWS ([#] to proceed, [*] to erase, numbers to write).";
      if (key == '#') {
        rows = msg_operation.toInt();
        saveToEEPROM();
        msg_operation = String(cols);
        switchMenu(201);
      } else if (key == '*' && msg_operation.length() > 0) {
        msg_operation = msg_operation.substring(0, msg_operation.length() - 1);
      } else if (key && key != '*' && key != '#') msg_operation += key;
      lcdLine2 = msg_operation.c_str();
      break;

    case 166:
      lcdLine1 = "ERROR - Could not add point, max reached!";
      lcdLine2 = "Press any key";
      if (key) switchMenu(1);
      break;

    case 165:
      lcdLine1 = "COMPLETE - Saving point";
      lcdLine2 = "Press any key";

      if (key) {
        if (stepsToRealPivot < callibrationPointsMax) {

          if (xCallibrationStepsStop - xCallibrationStepsStart != 0){
            xStepsToReal[stepsToRealPivot] = float(xCallibrationMeasure)/float(abs(xCallibrationStepsStop - xCallibrationStepsStart));
          } else xStepsToReal[stepsToRealPivot] = 0;

          if (yCallibrationStepsStop - yCallibrationStepsStart != 0){
            yStepsToReal[stepsToRealPivot] = float(yCallibrationMeasure)/float(abs(yCallibrationStepsStop - yCallibrationStepsStart));
          } else yStepsToReal[stepsToRealPivot] = 0;
          

          stepsToRealPivot += 1;
          saveToEEPROM();

          switchMenu(1);

        } else {
          switchMenu(166);
        }
        
      }
      break;

    case 164:
      lcdLine1 = "Units?";
      if (lcdVerticalScrollPivot >= msg_menu162_size) lcdVerticalScrollPivot = 0;
      lcdLine2 = msg_menu162[lcdVerticalScrollPivot];
      if (key == '0') { yCallibrationMeasure = 1000.f*float(msg_operation.toInt()); switchMenu(165); }
      if (key == '1') { yCallibrationMeasure = float(msg_operation.toInt()); switchMenu(165); }
      break;

    case 163:
      lcdLine1 = "INSERT Y Displacement. 0 for no measure. Units chosen after.";
      if (key == '#') {
        switchMenu(164);
      } else if (key == '*' && msg_operation.length() > 0) {
        msg_operation = msg_operation.substring(0, msg_operation.length() - 1);
      } else if (key && key != '*' && key != '#') msg_operation += key;
      lcdLine2 = msg_operation.c_str();
      break;

    case 162:
      lcdLine1 = "Units?";
      if (lcdVerticalScrollPivot >= msg_menu162_size) lcdVerticalScrollPivot = 0;
      lcdLine2 = msg_menu162[lcdVerticalScrollPivot];
      if (key == '0') { xCallibrationMeasure = 1000.f*float(msg_operation.toInt()); switchMenu(163); msg_operation = ""; }
      if (key == '1') { xCallibrationMeasure = float(msg_operation.toInt()); switchMenu(163); msg_operation = ""; }
      break;

    case 161:
      lcdLine1 = "INSERT X Displacement. 0 for no measure. Units chosen after.";
      if (key == '#') {
        switchMenu(162);
      } else if (key == '*' && msg_operation.length() > 0) {
        msg_operation = msg_operation.substring(0, msg_operation.length() - 1);
      } else if (key && key != '*' && key != '#') msg_operation += key;
      lcdLine2 = msg_operation.c_str();
      break;

    case 160:
      lcdLine1 = "ADD CALLIBRATION POINT - Move AFM using manual movement by X and Y, then select callibration option.";
      lcdLine2 = "Press any key";
      if (key) switchMenu(1);
      break;

    
    // Manual movement of stepper motors - Settings delay - save or discard delay.
    case 153:
      lcdLine1 = "Save? [#]";
      lcdLine2 = "Discard? [*]";
      if (key == '#') { manual_delay = msg_operation.toInt(); saveToEEPROM(); switchMenu(15); }
      if (key == '*') { switchMenu(15); }
      break;

    // Manual movement of stepper motors - Settings delay.
    case 152:
      lcdLine1 = "INSERT DELAY (\xB5s) ([#] to exit, [*] to erase, numbers to write).";
      if (key == '#') {
        switchMenu(153);
      } else if (key == '*' && msg_operation.length() > 0) {
        msg_operation = msg_operation.substring(0, msg_operation.length() - 1);
      } else if (key && key != '*' && key != '#') msg_operation += key;
      lcdLine2 = msg_operation.c_str();
      break;

    // Manual movement of stepper motors - Settings steps - save or discard steps.
    case 151:
      lcdLine1 = "Save? [#]";
      lcdLine2 = "Discard? [*]";
      if (key == '#') { manual_steps = msg_operation.toInt(); saveToEEPROM(); switchMenu(15); }
      if (key == '*') { switchMenu(15); }
      break;

    // Manual movement of stepper motors - Settings steps.
    case 150:
      lcdLine1 = "INSERT STEPS ([#] to exit, [*] to erase, numbers to write).";
      if (key == '#') {
        switchMenu(151);
      } else if (key == '*' && msg_operation.length() > 0) {
        msg_operation = msg_operation.substring(0, msg_operation.length() - 1);
      } else if (key && key != '*' && key != '#') msg_operation += key;
      lcdLine2 = msg_operation.c_str();
      break;

    // MANAGE CALLIBRATION POINTS
    case 17:
      lcdLine1 = "";
      if (lcdVerticalScrollPivot >= msg_menu17_size) lcdVerticalScrollPivot = 0;
      lcdLine2 = msg_menu17[lcdVerticalScrollPivot];
      if (key == '0') { switchMenu(1); calPointMenuPivot = 0; }

      if (stepsToRealPivot > 0){

        if (key == '*') {
          if (calPointMenuPivot > 0) calPointMenuPivot -= 1;
        }
        if (key == '#') {
          calPointMenuPivot += 1;
          if (calPointMenuPivot >= stepsToRealPivot - 1) calPointMenuPivot = stepsToRealPivot - 1;
        }

        msg_operation = "X:"+String(xStepsToReal[calPointMenuPivot], 3)+"nm Y:"+String(yStepsToReal[calPointMenuPivot], 3)+"nm";
        lcdLine1 = msg_operation.c_str();

        if (key == '1') {
          if (calPointMenuPivot < stepsToRealPivot - 1){
            for (int j = calPointMenuPivot; j < stepsToRealPivot; j++) {
              xStepsToReal[j] = xStepsToReal[j + 1];
              yStepsToReal[j] = yStepsToReal[j + 1];
            }
          } else {
            xStepsToReal[calPointMenuPivot] = 0;
            yStepsToReal[calPointMenuPivot] = 0;
          }
          
          stepsToRealPivot -= 1;
          calPointMenuPivot = 0;
          saveToEEPROM();
        }

      } else {
        lcdLine1 = "No points.";
      }
      
      break;

    // CALLIBRATION
    case 16:
      lcdLine1 = "CALLIBRATE";
      if (lcdVerticalScrollPivot >= msg_menu16_size) lcdVerticalScrollPivot = 0;
      lcdLine2 = msg_menu16[lcdVerticalScrollPivot];
      if (key == '#') {
        if (callibrationStep == 0){
          xCallibrationStepsStart = steps_X;
          yCallibrationStepsStart = steps_Y;
          callibrationStep = 1;
          switchMenu(160);
        }else{
          msg_operation = "";
          xCallibrationStepsStop = steps_X;
          yCallibrationStepsStop = steps_Y;
          callibrationStep = 0;
          switchMenu(161);
        }
      }
      if (key == '*') switchMenu(1);  // Return
      break;

    // Manual movement of stepper motors - Settings.
    case 15:
      lcdLine1 = "MANUAL SETTINGS";
      if (lcdVerticalScrollPivot >= msg_menu15_size) lcdVerticalScrollPivot = 0;
      lcdLine2 = msg_menu15[lcdVerticalScrollPivot];
      if (key == '0') {msg_operation = String(manual_steps); switchMenu(150);}    // Steps
      if (key == '1') {msg_operation = String(manual_delay); switchMenu(152);}    // Delay
      if (key == '#' || key == '*') switchMenu(1);  // Return
      break;

    // Manual Movement (Y+)
    case 14:
      lcdLine1 = "Powering Y+ ...";
      msg_operation = String(manual_steps)+" "+String(manual_delay);
      lcdLine2 = msg_operation.c_str();
      updateDisplay();
      moveMotor(stepPinY, dirPinY, enablePinY, manual_steps, manual_delay, 1, &steps_Y);
      switchMenu(1);
      break;

    // Manual Movement (Y-)
    case 13:
      lcdLine1 = "Powering Y- ...";
      msg_operation = String(manual_steps)+" "+String(manual_delay);
      lcdLine2 = msg_operation.c_str();
      updateDisplay();
      moveMotor(stepPinY, dirPinY, enablePinY, manual_steps, manual_delay, 0, &steps_Y);
      switchMenu(1);
      break;

    // Manual Movement (X+)
    case 12:
      lcdLine1 = "Powering X+ ...";
      msg_operation = String(manual_steps)+" "+String(manual_delay);
      lcdLine2 = msg_operation.c_str();
      updateDisplay();
      moveMotor(stepPinX, dirPinX, enablePinX, manual_steps, manual_delay, 1, &steps_X);
      switchMenu(1);
      break;

    // Manual Movement (X-)
    case 11:
      lcdLine1 = "Powering X- ...";
      msg_operation = String(manual_steps)+" "+String(manual_delay);
      lcdLine2 = msg_operation.c_str();
      updateDisplay();
      moveMotor(stepPinX, dirPinX, enablePinX, manual_steps, manual_delay, 0, &steps_X);
      switchMenu(1);
      break;

    // Manual Movement - See steps.
    case 10:
      msg_operation = "X="+String(steps_X)+" Y="+String(steps_Y);
      lcdLine1 = msg_operation.c_str();
      lcdLine2 = "Any key - Return";
      if (key) switchMenu(1);
      break;
    
    
    case 21:
      /*
      // Calculate scaling factors
      xStepsToRealMean = 0;
      yStepsToRealMean = 0;
      xStepsToRealN = 0;
      yStepsToRealN = 0;
      for (int factor = 0; factor < callibrationPointsMax; factor++){
        if (xStepsToReal[factor] > 0){
          xStepsToRealN += 1;
          xStepsToRealMean += xStepsToReal[factor];
        }
        if (yStepsToReal[factor] > 0){
          yStepsToRealN += 1;
          yStepsToRealMean += yStepsToReal[factor];
        }
      }
      xStepsToRealMean /= float(xStepsToRealN);
      yStepsToRealMean /= float(yStepsToRealN);

      // Message
      lcdLine1 = "----STARTING----";
      lcdLine2 = "--ACQUISITION---";
      updateDisplay();
      delay(2500);

      // Move the steppers and acquire images
      for (int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
          // Acquire image ij
          lcdLine1 = "Acq. img.";
          msg_operation = "i:"+String(i)+" j:"+String(j);
          lcdLine2 = msg_operation.c_str();
          updateDisplay();
          delay(2000);
          acquireAFMImage(setpoint, lines, Hz);
          // Move motor along collumns
          lcdLine1 = "Powering X+ ...";
          lcdLine2 = " ";
          updateDisplay();
          moveMotor(stepPinX, dirPinX, enablePinX, long(round(float(imgStep)*xStepsToRealMean)), 900000, 1, &steps_X);
        }
        // Go back to the beginning of the row
        lcdLine1 = "Powering X- ...";
        lcdLine2 = " ";
        updateDisplay();
        moveMotor(stepPinX, dirPinX, enablePinX, cols*long(round(float(imgStep)*xStepsToRealMean)), 900000, 0, &steps_X);
        // Go down one row
        lcdLine1 = "Powering Y- ...";
        lcdLine2 = " ";
        updateDisplay();
        moveMotor(stepPinY, dirPinY, enablePinY, long(round(float(imgStep)*yStepsToRealMean)), 900000, 0, &steps_Y);
      }

      // Homing
      lcdLine1 = "FINISHED";
      lcdLine2 = "Homing ...";
      updateDisplay();
      moveMotor(stepPinX, dirPinX, enablePinX, cols*long(round(float(imgStep)*xStepsToRealMean)), 900000, 0, &steps_X);
      moveMotor(stepPinY, dirPinY, enablePinY, rows*long(round(float(imgStep)*yStepsToRealMean)), 900000, 1, &steps_Y);

      // End routine
      switchMenu(2);
      */
      break;
      
    
    

    // Acquisition routines for the AFM.
    case 2:
      lcdLine1 = "ACQUISITION";
      if (lcdVerticalScrollPivot >= msg_menu2_size) lcdVerticalScrollPivot = 0;
      lcdLine2 = msg_menu2[lcdVerticalScrollPivot];
      if (key == '*') switchMenu(menu-1);
      if (key == '0') { msg_operation = String(rows); switchMenu(200); }  // Acquisition Settings
      if (key == '1') switchMenu(21);  // Begin Acquisition
      break;

    // Manual movement of stepper motors.
    case 1:
      lcdLine1 = "MANUAL/CALLIBRATE";
      if (lcdVerticalScrollPivot >= msg_menu1_size) lcdVerticalScrollPivot = 0;
      lcdLine2 = msg_menu1[lcdVerticalScrollPivot];
      if (key == '*') switchMenu(menu-1);
      if (key == '#') switchMenu(menu+1);
      if (key == '5') switchMenu(10); // View Steps (done)
      if (key == '4') switchMenu(11); // X- Move (done)
      if (key == '6') switchMenu(12); // X+ Move (done)
      if (key == '8') switchMenu(13); // Y- Move (done)
      if (key == '2') switchMenu(14); // Y+ Move (done)
      if (key == '0') switchMenu(15); // Movement Settings (done)
      if (key == '1') switchMenu(16); // Callibrate
      if (key == '3') {switchMenu(17); calPointMenuPivot = 0;} // Manage Callibration points
      break;

    // Welcome splash screen
    case 0:
      lcdLine1 = "WELCOME - AFM High-Range Automation @ IST";;
      lcdLine2 = "[*/#]: Scroll";
      if (key == '*') switchMenu(menu-1);
      if (key == '#') switchMenu(menu+1);
      break;
    
    // Credits screen
    case -1:
      lcdLine1 = "CREDITS - https://github.com/lannisraurus/afm-tools";
      lcdLine2 = "[*/#]: Scroll";
      if (key == '*') switchMenu(menu-1);
      if (key == '#') switchMenu(menu+1);
      break;
    
    // Reset
    case -2:
      lcdLine1 = "RESET SETTINGS";
      if (lcdVerticalScrollPivot >= msg_menu02_size) lcdVerticalScrollPivot = 0;
      lcdLine2 = msg_menu02[lcdVerticalScrollPivot];
      if (key == '#') switchMenu(menu+1);
      if (key == '0') {
        resetSettings();
        saveToEEPROM();
        switchMenu(-99);
      }
      break;

    // RESET
    case -99:
      lcdLine1 = "SETTINGS RESET!";
      lcdLine2 = "Press any key";
      if (key) switchMenu(0);
      break;

    // Safeguard for undefined behaviour
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
  // EEPROM and loading settings
  resetSettings();
  menu = loadFromEEPROM();
  saveToEEPROM();
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