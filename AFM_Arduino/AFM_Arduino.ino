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
const int micPin = A0;        // Microphone data pin. Change accordingly.
const int rotaryXPin = A1;    // Rotary Encoder 1 pin. Change accordingly.
const int rotaryYPin = A2;    // Rotary Encoder 2 pin. Change accordingly.

// Motor Wiring Variables
const int dirPinX = 8;
const int stepPinX = 9;
const int enablePinX = 10;
const int dirPinY = 11;
const int stepPinY = 12;
const int enablePinY = 13;

/* ///////////////////////
        UI VARIABLES
*/ ///////////////////////

// Keypad input
char key = ' ';

// LCD Specs
const unsigned int lcdWidth = 16;  // LCD Specs
const unsigned int lcdHeight = 2;  // LCD Specs

// Menu variables
int menu = 0;                      // Which menu the user is selecting.

// Timekeeping variables
unsigned long startLoopTime = 0;    // Keeps track of the times in which the main loop starts. ms.
unsigned long endLoopTime = 0;      // Keeps track of the times in which the main loop ends. ms.
unsigned long loopDt = 0;           // Difference between the two previous variables. ms.

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
char* lcdLine1 = " ";       // Line 1 of the LCD. Set variable to write.
char* lcdLine2 = " ";       // Line 2 of the LCD. Set variable to write.
String msg_operation = "";  // For intermediate conversions of Strings to c_str (char*) and/or user input.
const char * msg_scrollInstructions = "[*/#]: Scroll";
const char * msg_obfuscated = "################";
const char * msg_menu162[] = {"[0]: Micrometres", "[1]: Nanometres"};
const int msg_menu162_size = 2;
const char * msg_menu17[] = {"[*]: Left", "[#]: Right", "[0]: Return", "[1]: Delete"};
const int msg_menu17_size = 4;
const char * msg_menu16[] = {"[#]: Proceed", "[*]: Return"};
const int msg_menu16_size = 2;
const char * msg_menu15[] = {"[0]: Steps", "[1]: Delay", "[*/#]: Return"};
const int msg_menu15_size = 3;
const char * msg_menu2[] = {"[0]: Settings", "[1]: Acquire", "[*]: Return"};
const int msg_menu2_size = 3;
const char * msg_menu1[] = {"[0]: Settings", "[1]: Callibrate", "[3]: Edit Cal.", "[5]: View steps", "[4]: Move X-", "[6]: Move X+", "[8]: Move Y-", "[2]: Move Y+", "[#/*]: Scroll"};
const int msg_menu1_size = 9;
const char * msg_menu02[] = {"[0]: Proceed", msg_scrollInstructions};
const int msg_menu02_size = 2;

// Callibration point scroller
int calPointMenuPivot = 0

/* /////////////////////////////////////////////
       !!! AFM VARIABLES - NOT IN EEPROM !!!
*/ /////////////////////////////////////////////

// EEPROM verification ID. Change for hard reset.
const byte eepromID = 139;

// Step counters
long steps_X = 0;
long steps_Y = 0;

// Callibration
const int callibrationPointsMax = 10;
int callibrationStep = 0; // Internal variable that tracks where in the callibration procedure the user is.
                          // 0: add point; 1: insert displacement.
int xCallibrationStepsStart = 0;
int yCallibrationStepsStart= 0;
int xCallibrationStepsStop = 0;
int yCallibrationStepsStop = 0;
float xCallibrationMeasure = 0; // micrometres
float yCallibrationMeasure = 0; // micrometres

/* ///////////////////////////////////////
       !!! AFM VARIABLES - EEPROM !!!
*/ ///////////////////////////////////////

// Manual movement variables
long manual_delay;
long manual_steps;

// Callibration
float xStepsToReal[callibrationPointsMax];
float yStepsToReal[callibrationPointsMax];
int stepsToRealPivot;

// Acquisition variables
float Hz;
int lines;
char* setpoint;

/* ////////////////////////////
       EEPROM AND RESETTING
*/ ////////////////////////////
// NOTE: VARIABLES MUST BE IN THE SAME ORDER IN ALL THE FUNCTIONS!!!

// Set all AFM settings to default.
void resetSettings(){
  manual_delay = 900000;
  manual_steps = 128;
  stepsToRealPivot = 0;
  for (int i = 0; i < callibrationPointsMax; i++) xStepsToReal[i] = 0;
  for (int i = 0; i < callibrationPointsMax; i++) yStepsToReal[i] = 0;
  Hz = 1;
  lines = 512;
  setpoint = "0.2";
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
}

/* /////////////////////////////////
        INSTRUMENTATION ROUTINES
*/ /////////////////////////////////

void lcdPrint(int row, char* line, byte* pivot, int* clock); // Declaration so that the compiler doesn't scream at me.
void updateDisplay();

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

    case 166:
      lcdLine1 = "ERROR - The point could not be added, you have exceeded the maximum ammount of callibration points!";
      lcdLine2 = "Press any key";
      if (key) switchMenu(1);
      break;

    case 165:
      lcdLine1 = "COMPLETE - Saving point ...";
      lcdLine2 = "Press any key";

      if (key) {
        if (stepsToRealPivot < callibrationPointsMax) {

          if (xCallibrationStepsStop - xCallibrationStepsStart != 0){
            xStepsToReal[stepsToRealPivot] = float(xCallibrationMeasure)/float(xCallibrationStepsStop - xCallibrationStepsStart);
          } else xStepsToReal[stepsToRealPivot] = 0;

          if (yCallibrationStepsStop - yCallibrationStepsStart != 0){
            yStepsToReal[stepsToRealPivot] = float(yCallibrationMeasure)/float(yCallibrationStepsStop - yCallibrationStepsStart);
          } else xStepsToReal[stepsToRealPivot] = 0;
          

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
      if (key == '0') { yCallibrationMeasure = float(msg_operation.toInt()); switchMenu(165); }
      if (key == '1') { yCallibrationMeasure = 0.001*float(msg_operation.toInt()); switchMenu(165); }
      break;

    case 163:
      lcdLine1 = "INSERT Y Displacement. Insert 0 for no measure. Units chosen afterwards (micrometres or nanometres).";
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
      if (key == '0') { xCallibrationMeasure = float(msg_operation.toInt()); switchMenu(163); msg_operation = ""; }
      if (key == '1') { xCallibrationMeasure = 0.001*float(msg_operation.toInt()); switchMenu(163); msg_operation = ""; }
      break;

    case 161:
      lcdLine1 = "INSERT X Displacement. Insert 0 for no measure. Units chosen afterwards (micrometres or nanometres).";
      if (key == '#') {
        switchMenu(162);
      } else if (key == '*' && msg_operation.length() > 0) {
        msg_operation = msg_operation.substring(0, msg_operation.length() - 1);
      } else if (key && key != '*' && key != '#') msg_operation += key;
      lcdLine2 = msg_operation.c_str();
      break;

    case 160:
      lcdLine1 = "ADD CALLIBRATION POINT - Move your AFM using manual movement by a known X and Y ammount, and select the callibrate option again.";
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
      lcdLine1 = "INSERT DELAY (MICROSECONDS) ([#] to exit, [*] to erase, numbers to write).";
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
          calPointMenuPivot -= 1;
          if (calPointMenuPivot < 0) calPointMenuPivot = 0;
        }
        if (key == '#') {
          calPointMenuPivot += 1;
          if (calPointMenuPivot >= stepsToRealPivot) calPointMenuPivot = stepsToRealPivot - 1;
        }

        msg_operation = "X:"+String(xStepsToReal[calPointMenuPivot], 10)+" Y:"+String(yStepsToReal[calPointMenuPivot], 10);
        lcdLine1 = msg_operation.c_str();

        if (key == '1') {
          for (int j = calPointMenuPivot; j < calPointMenuPivot; j++) {
            xStepsToReal[j] = xStepsToReal[j + 1];
            yStepsToReal[j] = yStepsToReal[j + 1];
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
    
    // Acquisition routines for the AFM.
    case 2:
      lcdLine1 = "ACQUISITION - Acquire AFM images.";
      if (lcdVerticalScrollPivot >= msg_menu2_size) lcdVerticalScrollPivot = 0;
      lcdLine2 = msg_menu2[lcdVerticalScrollPivot];
      if (key == '*') switchMenu(menu-1);
      if (key == '0') switchMenu(20);  // Acquisition Settings
      if (key == '1') switchMenu(21);  // Begin Acquisition
      break;

    // Manual movement of stepper motors.
    case 1:
      lcdLine1 = "MANUAL/CALLIBRATE - Manually control the stepper motors. [Callibration] adds callibration points, and [Edit Cal.] allows you to remove added callibration points.";
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
      lcdLine1 = "WELCOME! - AFM High-Range Automation @ IST";;
      lcdLine2 = msg_scrollInstructions;
      if (key == '*') switchMenu(menu-1);
      if (key == '#') switchMenu(menu+1);
      break;
    
    // Credits screen
    case -1:
      lcdLine1 = "CREDITS - This project was developed by Joao Camacho [106224], 2025, under the supervision of Prof. Dr. Luis Viseu Melo, in the context of the PIC-I project. All the code and information regarding this project can be found at [github.com/lannisraurus/REPONAMEINSERTHERE].";
      lcdLine2 = msg_scrollInstructions;
      if (key == '*') switchMenu(menu-1);
      if (key == '#') switchMenu(menu+1);
      break;
    
    // Reset
    case -2:
      lcdLine1 = "RESET SETTINGS - Reset all manual movement and acquisition settings.";
      if (lcdVerticalScrollPivot >= msg_menu02_size) lcdVerticalScrollPivot = 0;
      lcdLine2 = msg_menu02[lcdVerticalScrollPivot];
      if (key == '*') switchMenu(menu-1);
      if (key == '#') switchMenu(menu+1);
      if (key == '0') {
        resetSettings();
        switchMenu(-99);
      }
      break;
    
    // Sensor data - debugging
    case -3:
      lcdLine1 = "SENSOR DATA - M: Microphone; X: Rotary Encoder X; Y: Rotary Encoder Y; [#] to Return.";
      msg_operation = "M"+String(micAnalog)+" X"+String(rotaryAnalogX)+" Y"+String(rotaryAnalogY);
      lcdLine2 = msg_operation.c_str();
      if (key == '#') switchMenu(menu+1);
      break;

    // Factory reset
    case -99:
      lcdLine1 = "SETTINGS RESET - Either you reset your settings, or no data/corrupted data was found on the EEPROM.";
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