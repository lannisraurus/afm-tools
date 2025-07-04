/*
João Camacho, 2025, PIC-I @ IST

Code adapted from the WPSH203 LCD and
WPI300 Keypad Manuals. Stepper control and AFM Nanoscope code adapted from
https://www.hardware-x.com/article/S2468-0672(23)00054-8/fulltext

Depending on your Nanoscope version the keyboard shortcuts might be different, in which case
it is recommended to alter the acquireAFMImage function. It is also important to check all the
variables and make sure that they are well suited to your setup (some acquisition variables
cannot be changed in the user GUI!).

It is important to note that originally it was planned to receive direct input from the AFM
acquisition system utilizing a microphone, and taking in input with rotary encoders to cross-check
motor movement (missing steps for some reason). These features were not implemented due to time
constraints. You may upgrade this code to allow for these features in your build.
*/

/* ///////////////////
        INCLUDES
*/ ///////////////////

#include <Wire.h>       // For I2C.
#include <LCD_I2C.h>    // I2C LCD.
#include <Keypad.h>     // For taking in user input.
#include "Keyboard.h"   // For sending commands to AFM computer.
#include <EEPROM.h>     // Saving variables between shutdowns.

/* ///////////////////////////
        PRE-DECLARATIONS
*/ ///////////////////////////

void lcdPrint(int, char*, byte*, int*);
void updateDisplay();

/* ////////////////////
        LCD SCREEN
*/ ////////////////////

const byte lcdAddress = 0x27;   // LCD I2C Address - Change according to your module.
LCD_I2C lcd(lcdAddress);        // Default address of most PCF8574 modules.

/* ///////////////////////
        KEYPAD PINS
*/ ///////////////////////

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

const byte resetButtonPin = A3;

/* ///////////////////////
        SENSOR PINS
*/ ///////////////////////

// Sensor Data. NOTE: NOT USED IN THIS BUILD! MAY BE USED IN FUTURE RENDITIONS!
int micRead = -1;              // Microphone reading (buzzer).
int rotaryAnalogX = -1;        // First Rotary Encoder X.
int rotaryAnalogY = -1;        // Second Rotary Encoder Y.
const byte micPin = A0;        // Microphone data pin.
const byte rotaryXPin = A1;    // Rotary Encoder 1 pin.
const byte rotaryYPin = A2;    // Rotary Encoder 2 pin.

// Motor Wiring Variables
const byte dirPinY = 8;
const byte stepPinY = 9;
const byte enablePinY = 10;
const byte dirPinX = 11;
const byte stepPinX = 12;
const byte enablePinX = 13;

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
const byte lcdHorizontalScrollStep = 4;  // How many characters are skipped per autoscroll step.
const int lcdHorizontalScrollMax = 750;  // Controlls how often the line display scrolls. ms.
const int lcdHorizontalScrollMin = -1310;// Reset tolerance in beginning and end of scroll. ms.
int lcdHorizontalScrollCounter1 = 0;     // Keeps track of how much time has passed since a scroll - Line 1. ms.
int lcdHorizontalScrollCounter2 = 0;     // Keeps track of how much time has passed since a scroll - Line 2. ms.
byte lcdHorizontalScrollPivot1 = 0;      // Keeps track of where the scroll is currently - Line 1.
byte lcdHorizontalScrollPivot2 = 0;      // Keeps track of where the scroll is currently - Line 2.
int lcdVerticalScrollCounter = 0;        // Allows for vertical scrolling by line, as per menu command.
byte lcdVerticalScrollPivot = 0;         // Which element is being selected
const int lcdVerticalScrollMax = 1720;   // Vertical scroll amount of time on screen. ms.

// Display LCD data
char* lcdLine1 = " ";        // Line 1 of the LCD. Set variable to write.
char* lcdLine2 = " ";        // Line 2 of the LCD. Set variable to write.
String msg_operation = "";   // For intermediate conversions of Strings to c_str (char*) and/or user input.
String msg_operation2 = "";  // For intermediate conversions of Strings to c_str (char*) and/or user input.

// Vertical scrolling arrays
const char * msg_menu17[] = {"[*]: <-", "[#]: ->", "[0]: Back", "[1]: Erase"};
const byte msg_menu17_size = 4;
const char * msg_menu16[] = {"[#]: Ok", "[*]: Back"};
const byte msg_menu16_size = 2;
const char * msg_menu15[] = {"[0]: Steps", "[1]: Delay", "[*/#]: Back"};
const byte msg_menu15_size = 3;
const char * msg_menu2[] = {"[0]: Settings", "[1]: Acquire", "[*]: Back"};
const byte msg_menu2_size = 3;
const char * msg_menu1[] = {"[0]: Settings", "[1]: Calib", "[3]: Edit Cal.", "[5]: See Steps", "[4]: X-", "[6]: X+", "[8]: Y-", "[2]: Y+", "[#/*]: Scroll"};
const byte msg_menu1_size = 9;
const char * msg_menu02[] = {"[0]: Proceed", "[#]: Back"};
const byte msg_menu02_size = 2;

// Calibration
byte calPointMenuPivot = 0; // Calibration point see menu

/* /////////////////////////////////////////////
       !!! AFM VARIABLES - NOT IN EEPROM !!!
*/ /////////////////////////////////////////////
// Adjust these variables if needed!

// EEPROM verification ID. Change for hard reset of all settings.
const byte eepromID = 43;

// Step counters
long steps_X = 0; // steps
long steps_Y = 0; // steps

// Calibration
const byte calibrationPointsMax = 6;
int xCalibrationSteps = 0;         // Steps
int yCalibrationSteps = 0;         // Steps
int xCalibrationMeasure = 0;       // nm
int yCalibrationMeasure = 0;       // nm

// AFM Keyboard pressing / Displaying
const long timeBetweenKeys = 250; // ms  time that a key is pressed for
const long timeDisplay = 2000;    // ms  time for a display during acquisition routine
const long timeOperation = 500;   // ms  time between generic keyboard operations
const long timeEngage = 2000;     // ms  time to wait after engaging (menu disappearing)
const long timeEngaging = 300000; // ms  time it takes to engage. ALTERNATIVE TO ORIGINAL MICROPHONE IDEA!!!

// AFM Conversion variables
float xStepsToRealMean = 0;
float yStepsToRealMean = 0;
byte xStepsToRealN = 0;
byte yStepsToRealN = 0;

/* ///////////////////////////////////////
       !!! AFM VARIABLES - EEPROM !!!
*/ ///////////////////////////////////////

// ALL VARIABLES DEFINED HERE ARE SET IN resetSettings!

// Manual movement variables
long manual_delay;  // microseconds
long manual_steps;  // steps

// Calibration
float xStepsToReal[calibrationPointsMax];  // nm/step
float yStepsToReal[calibrationPointsMax];  // nm/step
byte stepsToRealPivot;                      // How many points exist + 1

// Acquisition variables
int Hz;         // Acquisition Frequency in !!mHz!!
int lines;      // Lines to acquire.
byte rows;      // How many images to acquire in a row.
byte cols;      // How many images to acquire in a collumn.
int imgStep;    // How many nanometres to displace between each image.

/* ////////////////////////////
       EEPROM AND RESETTING
*/ ////////////////////////////

// NOTE: VARIABLES MUST BE IN THE SAME ORDER IN ALL THE FUNCTIONS!!!

// Set all AFM settings to default.
void resetSettings(){
  manual_delay = 10;
  manual_steps = 128;
  stepsToRealPivot = 0;
  for (int i = 0; i < calibrationPointsMax; i++) xStepsToReal[i] = 0;
  for (int i = 0; i < calibrationPointsMax; i++) yStepsToReal[i] = 0;
  Hz = 1000;
  lines = 128;
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
  EEPROM.put(eepromPivot, rows); eepromPivot += sizeof(rows);
  EEPROM.put(eepromPivot, cols); eepromPivot += sizeof(cols);
  EEPROM.put(eepromPivot, imgStep); eepromPivot += sizeof(imgStep);
}

/* /////////////////////////////////
        INSTRUMENTATION ROUTINES
*/ /////////////////////////////////

// Move one of the motors, given their pins.
void moveMotor(byte stepPin, byte dirPin, byte enablePin, long steps, long stepDelay, boolean direction, long* stepCounter){
  digitalWrite(enablePin, LOW);
  digitalWrite(dirPin, direction);
  delay(2*stepDelay);
  for (int x = 0; x < steps; x++) {
    digitalWrite(stepPin, HIGH);
    delay(stepDelay/2);
    digitalWrite(stepPin, LOW);
    delay(stepDelay/2);
  }
  digitalWrite(enablePin, HIGH);
  if (direction) *stepCounter += steps; else *stepCounter -= steps;
  
}

// Acquire an AFM image.
void acquireAFMImage(int lines, int Hz){

    // ENGAGE INPUT
    lcdLine1 = "Engaging...";
    lcdLine2 = "key = force skip";
    updateDisplay();
    Keyboard.press(KEY_LEFT_ALT); Keyboard.press('r'); delay(timeBetweenKeys); Keyboard.releaseAll();
    Keyboard.press('e'); delay(timeBetweenKeys); Keyboard.releaseAll();

    // Engage delay
    delay(timeEngage);

    // Wait for engaging. IF YOU INCLUDE MICROPHONE, YOU WOULD PLACE CODE HERE!!!
    // Something like -> read microphone; if read > threshold, break out of loop.
    keypad.getKey();
    unsigned long StartTimeEngage = millis();
    unsigned long CurrentTimeEngage = millis();
    msg_operation2 = "Total="+String(long(round(timeEngaging/1000.)))+"s";
    lcdLine2 = msg_operation2.c_str();
    boolean engaging = true;
    while(engaging) {
      CurrentTimeEngage = millis();
      msg_operation = "Time="+ String(round((CurrentTimeEngage - StartTimeEngage)/1000.))+"s";
      lcdLine1 = msg_operation.c_str();
      updateDisplay();
      if ( (CurrentTimeEngage - StartTimeEngage) > timeEngaging ) engaging = false;
      if (keypad.getKey()) engaging = false;
      ////////// RESET BUTTON !!!!
      if (!digitalRead(resetButtonPin)) {return;}
    }
    
    // Standard Delay
    delay(timeBetweenKeys);

    // Engage delay
    delay(timeEngage);

    // Close annoying pop-up windows in our case :)
    Keyboard.press(KEY_ESC); delay(timeBetweenKeys); Keyboard.releaseAll();

    // Standard delay
    delay(timeOperation);
    
    // START FRAME DOWN
    lcdLine1 = "Setting...";
    lcdLine2 = "Frame Down";
    updateDisplay();
    Keyboard.press(KEY_LEFT_ALT); Keyboard.press('r'); delay(timeBetweenKeys); Keyboard.releaseAll();
    Keyboard.press('d'); delay(timeBetweenKeys); Keyboard.releaseAll();

    // Standard delay
    delay(timeOperation);

    // REVERSE TO PREVENT DRIFT - Wasn't working properly in our case; you may reimplement!
    /*lcdLine1 = "Setting...";
    lcdLine2 = "Reversing";
    updateDisplay();
    Keyboard.press(KEY_LEFT_ALT); Keyboard.press('r'); delay(timeBetweenKeys); Keyboard.releaseAll();
    Keyboard.press('r'); delay(timeBetweenKeys); Keyboard.releaseAll();

    // Standard delay
    delay(timeOperation);*/

    // BEGIN CAPTURING AND WITHDRAW
    lcdLine1 = "Setting...";
    lcdLine2 = "Capture+Withdraw";
    updateDisplay();
    Keyboard.press(KEY_LEFT_ALT); Keyboard.press('r'); delay(timeBetweenKeys); Keyboard.releaseAll();
    Keyboard.press('w'); delay(timeBetweenKeys); Keyboard.releaseAll();

    // Engage delay
    delay(timeOperation);

    // Close annoying pop-up windows in our case :)
    Keyboard.press(KEY_ESC); delay(timeBetweenKeys); Keyboard.releaseAll();

    // Standard delay
    delay(timeOperation);
    
    // TIME KEEPING VARIABLES
    float acquisitionTime = ((float) lines)*1000.*1000. / float(Hz); // *1000 mHz->Hz ; *1000 s->ms
    acquisitionTime += 15000.;                                       // Safety threshold
    unsigned long StartTime = millis();
    unsigned long CurrentTime = millis();

    // Total time
    msg_operation2 = "Total="+String(long(round(acquisitionTime/1000.)))+"s";
    lcdLine2 = msg_operation2.c_str();

    // Wait until image is done.
    while((CurrentTime - StartTime) < ((unsigned long) round(acquisitionTime) )){

      CurrentTime = millis();
      
      msg_operation = "Time="+ String(round((CurrentTime - StartTime)/1000.))+"s";
      lcdLine1 = msg_operation.c_str();
      updateDisplay();

      if (keypad.getKey()) break;

      ////////// RESET BUTTON !!!!
      if (!digitalRead(resetButtonPin)) {return;}

    }
    
    return 0;  
}

void acquisitionRoutine(){
      // Calculate scaling factors
      lcdLine1 = "Calculating";
      lcdLine2 = "scaling factors";
      updateDisplay();
      delay(timeDisplay);
      xStepsToRealMean = 0;
      yStepsToRealMean = 0;
      xStepsToRealN = 0;
      yStepsToRealN = 0;
      for (int factor = 0; factor < calibrationPointsMax; factor++){
        if (xStepsToReal[factor] > 0){
          xStepsToRealN += 1;
          xStepsToRealMean += xStepsToReal[factor];
        }
        if (yStepsToReal[factor] > 0){
          yStepsToRealN += 1;
          yStepsToRealMean += yStepsToReal[factor];
        }
      }
      if (xStepsToRealN > 0) xStepsToRealMean /= float(xStepsToRealN);
      if (yStepsToRealN > 0) yStepsToRealMean /= float(yStepsToRealN);
      if (xStepsToRealN == 0 || yStepsToRealN == 0) {
        lcdLine1 = "ERROR! Need";
        lcdLine2 = "Calibration!";
        updateDisplay();
        delay(timeDisplay);
        switchMenu(2);
      } else {

      // Scaling Factors Display
      msg_operation = "X:"+String(xStepsToRealMean, 3);
      msg_operation2 = "Y:"+String(yStepsToRealMean, 3);
      lcdLine1 = msg_operation.c_str();
      lcdLine2 = msg_operation2.c_str();
      updateDisplay();
      delay(timeDisplay);
      
      // Confirm Autotuning
      lcdLine1 = "Confirm (#):";
      lcdLine2 = "Autotuned?";
      boolean autotuning = true;
      while (autotuning) {
        updateDisplay();
        if (keypad.getKey() == '#') autotuning = false;
        ////////// RESET BUTTON !!!!
        if (!digitalRead(resetButtonPin)) {switchMenu(0); return;}
      }

      // Confirm Lines+Hz
      lcdLine1 = "Confirm (#):";
      msg_operation = "mHz:"+String(Hz)+", Lines:"+String(lines);
      lcdLine2 = msg_operation.c_str();
      boolean lineshz = true;
      while (lineshz) {
        startLoopTime = millis();
        lcdHorizontalScrollCounter1 += loopDt;
        lcdHorizontalScrollCounter2 += loopDt;
        lcdVerticalScrollCounter += loopDt;
        updateDisplay();
        if (keypad.getKey() == '#') lineshz = false;
        endLoopTime = millis();
        loopDt = endLoopTime - startLoopTime;
        ////////// RESET BUTTON !!!!
        if (!digitalRead(resetButtonPin)) {switchMenu(0); return;}
      }
      
      // Message
      lcdLine1 = "----STARTING----";
      lcdLine2 = "--ACQUISITION---";
      updateDisplay();
      delay(timeDisplay);
  
      
      // Move the steppers and acquire images
      for (int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
          // Acquire image ij
          lcdLine1 = "Acquiring image:";
          msg_operation = "i:"+String(i)+" j:"+String(j);
          lcdLine2 = msg_operation.c_str();
          updateDisplay();
          delay(timeDisplay);
          acquireAFMImage(lines, Hz);
          if (!digitalRead(resetButtonPin)) {switchMenu(0); return;}
          // Move motor along collumns
          lcdLine1 = "Powering X+ ...";
          lcdLine2 = " ";
          updateDisplay();
          moveMotor(stepPinX, dirPinX, enablePinX, long(round(float(imgStep)/xStepsToRealMean)), 10, 1, &steps_X);
        }
        // Go back to the beginning of the row
        lcdLine1 = "Powering X- ...";
        lcdLine2 = " ";
        updateDisplay();
        moveMotor(stepPinX, dirPinX, enablePinX, cols*long(round(float(imgStep)/xStepsToRealMean)), 10, 0, &steps_X);
        // Go down one row
        lcdLine1 = "Powering Y- ...";
        lcdLine2 = " ";
        updateDisplay();
        moveMotor(stepPinY, dirPinY, enablePinY, long(round(float(imgStep)/yStepsToRealMean)), 10, 0, &steps_Y);
      }

      // Homing
      lcdLine1 = "FINISHED";
      lcdLine2 = "Homing ...";
      updateDisplay();
      delay(timeDisplay);
      // moveMotor(stepPinX, dirPinX, enablePinX, cols*long(round(float(imgStep)/xStepsToRealMean)), 10, 0, &steps_X);
      moveMotor(stepPinY, dirPinY, enablePinY, (rows+1)*long(round(float(imgStep)/yStepsToRealMean)), 10, 1, &steps_Y);

      // End routine
      switchMenu(2);
      
      }
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
      lcdLine1 = "Image Step (nm) ([#] to proceed, [*] to erase).";
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

    /*
    case 204:
      lcdLine1 = "Amp. Setpoint ([#] to proceed).";
      if (key == '#') {
        saveToEEPROM();
        msg_operation = String(imgStep);
        switchMenu(205);
      } else if (key && key != '*' && key != '#') {msg_operation = "0."+String(key); setpoint = key;}
      lcdLine2 = msg_operation.c_str();
      break;
    */

    case 203:
      lcdLine1 = "mHz ([#] to proceed, [*] to erase).";
      if (key == '#') {
        Hz = msg_operation.toInt();
        saveToEEPROM();
        msg_operation = String(imgStep);
        switchMenu(205);
      } else if (key == '*' && msg_operation.length() > 0) {
        msg_operation = msg_operation.substring(0, msg_operation.length() - 1);
      } else if (key && key != '*' && key != '#') msg_operation += key;
      lcdLine2 = msg_operation.c_str();
      break;

    case 202:
      lcdLine1 = "Lines ([#] to proceed, [*] to erase).";
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
      lcdLine1 = "Cols ([#] to proceed, [*] to erase).";
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
      lcdLine1 = "Rows ([#] to proceed, [*] to erase).";
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
      lcdLine1 = "ERROR - max points reached!";
      lcdLine2 = "Press any key";
      if (key) switchMenu(1);
      break;

    case 165:
      lcdLine1 = "COMPLETE - Saving point";
      lcdLine2 = "Press any key";

      if (key) {
        if (stepsToRealPivot < calibrationPointsMax) {

          if (xCalibrationSteps != 0){
            xStepsToReal[stepsToRealPivot] = float(xCalibrationMeasure)/float(xCalibrationSteps);
          } else xStepsToReal[stepsToRealPivot] = 0;

          if (yCalibrationSteps != 0){
            yStepsToReal[stepsToRealPivot] = float(yCalibrationMeasure)/float(yCalibrationSteps);
          } else yStepsToReal[stepsToRealPivot] = 0;
        
          stepsToRealPivot += 1;
          saveToEEPROM();

          switchMenu(1);

        } else {
          switchMenu(166);
        }
        
      }
      break;

    /*
    case 164:
      lcdLine1 = "Units?";
      if (lcdVerticalScrollPivot >= msg_menu162_size) lcdVerticalScrollPivot = 0;
      lcdLine2 = msg_menu162[lcdVerticalScrollPivot];
      if (key == '0') { yCalibrationMeasure = 1000.f*float(msg_operation.toInt()); switchMenu(165); }
      if (key == '1') { yCalibrationMeasure = float(msg_operation.toInt()); switchMenu(165); }
      break;
    */

    case 164:
      lcdLine1 = "dY (steps). 0 to ignore. [#] to proceed. [*] to erase.";
      if (key == '#') {
        yCalibrationSteps = float(msg_operation.toInt()); switchMenu(165); msg_operation = "";
      } else if (key == '*' && msg_operation.length() > 0) {
        msg_operation = msg_operation.substring(0, msg_operation.length() - 1);
      } else if (key && key != '*' && key != '#') msg_operation += key;
      lcdLine2 = msg_operation.c_str();
      break;

    case 163:
      lcdLine1 = "dY (nm). 0 to ignore. [#] to proceed. [*] to erase.";
      if (key == '#') {
        yCalibrationMeasure = float(msg_operation.toInt()); switchMenu(164); msg_operation = "";
      } else if (key == '*' && msg_operation.length() > 0) {
        msg_operation = msg_operation.substring(0, msg_operation.length() - 1);
      } else if (key && key != '*' && key != '#') msg_operation += key;
      lcdLine2 = msg_operation.c_str();
      break;

    /*
    case 162:
      lcdLine1 = "Units?";
      if (lcdVerticalScrollPivot >= msg_menu162_size) lcdVerticalScrollPivot = 0;
      lcdLine2 = msg_menu162[lcdVerticalScrollPivot];
      if (key == '0') { xCalibrationMeasure = 1000.f*float(msg_operation.toInt()); switchMenu(163); msg_operation = ""; }
      if (key == '1') { xCalibrationMeasure = float(msg_operation.toInt()); switchMenu(163); msg_operation = ""; }
      break;
    */

    case 162:
      lcdLine1 = "dX (steps). 0 to ignore. [#] to proceed. [*] to erase.";
      if (key == '#') {
        xCalibrationSteps = msg_operation.toInt(); switchMenu(163); msg_operation = "";
      } else if (key == '*' && msg_operation.length() > 0) {
        msg_operation = msg_operation.substring(0, msg_operation.length() - 1);
      } else if (key && key != '*' && key != '#') msg_operation += key;
      lcdLine2 = msg_operation.c_str();
      break;

    case 161:
      lcdLine1 = "dX (nm). 0 to ignore. [#] to proceed. [*] to erase.";
      if (key == '#') {
        xCalibrationMeasure = msg_operation.toInt(); switchMenu(162); msg_operation = "";
      } else if (key == '*' && msg_operation.length() > 0) {
        msg_operation = msg_operation.substring(0, msg_operation.length() - 1);
      } else if (key && key != '*' && key != '#') msg_operation += key;
      lcdLine2 = msg_operation.c_str();
      break;

    /*
    case 160:
      lcdLine1 = "Add Point - Manually move AFM by XY, then select calibration option.";
      lcdLine2 = "Press any key";
      if (key) switchMenu(1);
      break;
    */

    /*
    // Manual movement of stepper motors - Settings delay - save or discard delay.
    case 153:
      lcdLine1 = "Save? [#]";
      lcdLine2 = "Discard? [*]";
      if (key == '#') { manual_delay = msg_operation.toInt(); saveToEEPROM(); switchMenu(15); }
      if (key == '*') { switchMenu(15); }
      break;
    */

    // Manual movement of stepper motors - Settings delay.
    case 152:
      lcdLine1 = "Delay (ms) ([#] to exit, [*] to erase).";
      if (key == '#') {
        manual_delay = msg_operation.toInt(); saveToEEPROM(); switchMenu(15);
      } else if (key == '*' && msg_operation.length() > 0) {
        msg_operation = msg_operation.substring(0, msg_operation.length() - 1);
      } else if (key && key != '*' && key != '#') msg_operation += key;
      lcdLine2 = msg_operation.c_str();
      break;

    /*
    // Manual movement of stepper motors - Settings steps - save or discard steps.
    case 151:
      lcdLine1 = "Save? [#]";
      lcdLine2 = "Discard? [*]";
      if (key == '#') { manual_steps = msg_operation.toInt(); saveToEEPROM(); switchMenu(15); }
      if (key == '*') { switchMenu(15); }
      break;
    */

    // Manual movement of stepper motors - Settings steps.
    case 150:
      lcdLine1 = "Steps ([#] to save, [*] to erase).";
      if (key == '#') {
        manual_steps = msg_operation.toInt(); saveToEEPROM(); switchMenu(15);
      } else if (key == '*' && msg_operation.length() > 0) {
        msg_operation = msg_operation.substring(0, msg_operation.length() - 1);
      } else if (key && key != '*' && key != '#') msg_operation += key;
      lcdLine2 = msg_operation.c_str();
      break;

    // MANAGE CALIBRATION POINTS
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

    // CALIBRATION
    case 16:
      lcdLine1 = "Add cal. point?";
      if (lcdVerticalScrollPivot >= msg_menu16_size) lcdVerticalScrollPivot = 0;
      lcdLine2 = msg_menu16[lcdVerticalScrollPivot];
      if (key == '#') {
        msg_operation = "";
        switchMenu(161);
      }
      if (key == '*') switchMenu(1);  // Return
      break;

    // Manual movement of stepper motors - Settings.
    case 15:
      lcdLine1 = "Manual Settings";
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
    
    /*
    case 22:
      if (micRead) {
        lcdLine1 = "SIGNAL!";
      } else {
        lcdLine1 = "NO SIGNAL";
      }
      if (key == '#') {
        micThreshold = msg_operation.toInt(); saveToEEPROM();
        micReadMin = 1024;
        micReadMax = 0;
        switchMenu(2);
      } else if (key == '*' && msg_operation.length() > 0) {
        msg_operation = msg_operation.substring(0, msg_operation.length() - 1);
      } else if (key && key != '*' && key != '#') msg_operation += key;
      lcdLine2 = msg_operation.c_str();
      break;
    */
    
    case 21:

      acquisitionRoutine();
      break;
      
    

    // Acquisition routines for the AFM.
    case 2:
      lcdLine1 = "Acquisition";
      if (lcdVerticalScrollPivot >= msg_menu2_size) lcdVerticalScrollPivot = 0;
      lcdLine2 = msg_menu2[lcdVerticalScrollPivot];
      if (key == '*') switchMenu(menu-1);
      if (key == '0') { msg_operation = String(rows); switchMenu(200); }  // Acquisition Settings
      if (key == '1') switchMenu(21);  // Begin Acquisition
      // if (key == '2') { msg_operation = String(micThreshold); switchMenu(22); }  // Adjust Mic Threshold - DEPRECATED!! YOU MAY ADD IF YOU WANT :)
      break;

    // Manual movement of stepper motors.
    case 1:
      lcdLine1 = "Manual/Calibration";
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
      if (key == '1') switchMenu(16); // Calibrate
      if (key == '3') {switchMenu(17); calPointMenuPivot = 0;} // Manage Calibration points
      break;

    // Welcome splash screen
    case 0:
      lcdLine1 = "AFM High-Range Automation @ IST";;
      lcdLine2 = "[*/#]: Scroll";
      if (key == '*') switchMenu(menu-1);
      if (key == '#') switchMenu(menu+1);
      break;
    
    // Credits screen
    case -1:
      lcdLine1 = "Guide - github.com/lannisraurus/afm-tools";
      lcdLine2 = "[*/#]: Scroll";
      if (key == '*') switchMenu(menu-1);
      if (key == '#') switchMenu(menu+1);
      break;
    
    // Reset
    case -2:
      lcdLine1 = "Reset Settings";
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
  // Reset button setup
  pinMode(resetButtonPin, INPUT_PULLUP);
  // Motor setup
  pinMode(dirPinX, OUTPUT);
  pinMode(stepPinX, OUTPUT);
  pinMode(enablePinX, OUTPUT);
  pinMode(dirPinY, OUTPUT);
  pinMode(stepPinY, OUTPUT);
  pinMode(enablePinY, OUTPUT);
  digitalWrite(enablePinX, HIGH);
  digitalWrite(enablePinY, HIGH);
  // Mic setup
  // pinMode(micPin, INPUT);
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

  // Resetting
  if (!digitalRead(resetButtonPin) && menu != 0) switchMenu(0);


  // Update counters
  lcdHorizontalScrollCounter1 += loopDt;
  lcdHorizontalScrollCounter2 += loopDt;
  lcdVerticalScrollCounter += loopDt;

  // Keypad Digital Buttons Acquire
  key = keypad.getKey();

  // Menu System
  menuLoop();

  // Vertical Scrolling
  if (lcdVerticalScrollCounter >= lcdVerticalScrollMax) { lcdVerticalScrollCounter = 0; lcdVerticalScrollPivot += 1; }
  // Update LCD Display (+Horizontal Scrolling)
  updateDisplay();

  // Time keeping
  endLoopTime = millis();
  loopDt = endLoopTime - startLoopTime;

} 