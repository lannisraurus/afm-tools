#include <math.h>
#include "Keyboard.h"
#include <W5IOKeypad.h>
W5IOKeypad Keypad(7,6,5,4,3,2);
#include <LCD_I2C.h>
LCD_I2C lcd(0x27); // Default address of most PCF8574 modules, change according

int i,j,setpoint=4;
unsigned long int lines = 512;    //Parameters from AFM
float Hz = 1, acquisitionTime;
char amplitudeSetPoint[] = "0.2", setpoint_char=setpoint+'0';

int IOKey = 0;  //Initiate Keypad at 0

const int analogPin0 = A0; //Listening to beep

//Motors
int motor; //Motor identifier, 1 or 2
int dir; //Motor direction, 0 or 1
const int dirPin1 = 8;
const int stepPin1 = 9;
const int enablePin1 = 10;
const int dirPin2 = 11;
const int stepPin2 = 12;
const int enablePin2 = 13;
const int full = 200*16;  //200 steps * 16 microstepping

double v=1;

int columns = 1;
int sep_columns; //microns
int steps_column;
double factor = 2400/(200*16.0*45/11);  //2400 microns per round, 200 steps, 16 microstep, 45/11 gear reduction 

int steps_pointer=2;
long int Pos_x=0,Pos_y=0;
//long int steps[]={5455,2728,1364,699,350,176,88,44,22,11,6}; //Prior to experimental correction
long int steps[]={4895,2447,1224,627,313,157,78,39,20,10,5}; //After experimental correction 17.83 µm/ 16 µm;
long int steps_um[]={1000,500,250,128,64,32,16,8,4,2,1};

long int stepDelay = 250; //By default 250 ms between steps, fastest we try.

unsigned long StartTime, CurrentTime, ElapsedTime, sec_timer, secs;
 
void setup() {
  Serial.begin(9600);
  Keyboard.begin();
  lcd.begin();
  lcd.backlight();

  pinMode(dirPin1, OUTPUT);
  pinMode(stepPin1, OUTPUT);
  pinMode(enablePin1, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  pinMode(stepPin2, OUTPUT);
  pinMode(enablePin2, OUTPUT);
  digitalWrite(enablePin1, HIGH); //Put the motor off by default
  digitalWrite(enablePin2, HIGH); //Put the motor off by default
  lcd.setCursor(0, 0);
  lcd.print(F("mafernandez LSIP"));
  print_v_step(v,steps_um[steps_pointer]);     
}
 
void loop() {

   //We read keypad and proceed accordingly
   IOKey = 0;
   IOKey=Keypad.ReadButtons();
   if(IOKey==1){
     lcd.clear();
     lcd.setCursor(4, 0);
     lcd.print("Speed /2");
     if (stepDelay<256000) {
       v=v/2;
       stepDelay = stepDelay*2;        //The higher delay, the slower speed
     }
     else {
           lcd.setCursor(4, 0);
           lcd.print("Min speed");
          }     
     print_v_step(v,steps_um[steps_pointer]);     
   }
   
   if(IOKey==2){
     lcd.clear();
     lcd.setCursor(4, 0);
     lcd.print("Speed x2");
     if (stepDelay>250) {
            stepDelay = stepDelay/2;
            v=v*2;
     }
     else {
           lcd.setCursor(4, 0);
           lcd.print("Max speed");
          }     
      print_v_step(v,steps_um[steps_pointer]);     
   }

   if(IOKey==3){              //For illustrative purposes only
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pos_x ");
    lcd.setCursor(6, 0);
    lcd.print((long int) Pos_x);
    lcd.setCursor(0, 1);
    lcd.print("Pos_y ");
    lcd.setCursor(6, 1);
    lcd.print((long int) Pos_y);
    delay(1500);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("                ");     
    print_v_step(v,steps_um[steps_pointer]);     
   }

   
/*   if(IOKey==4){              //For illustrative purposes only
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pressing Alt+Tab");
    lcd.setCursor(0, 1);
    lcd.print("Clear LCD");
    Keyboard.press(KEY_LEFT_ALT);
    Keyboard.press(KEY_TAB);
    Keyboard.releaseAll();
    delay(1500);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("                ");     
    print_v_step(v,steps_um[steps_pointer]);     
   }*/


   if(IOKey==5){              //Reestablish default values
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Reset 250um-step");     
      v=1;
      steps_pointer=2;
      stepDelay = 250;
      print_v_step(v,steps_um[steps_pointer]);     
   }

   if(IOKey==6){
     lcd.clear();
     lcd.setCursor(4, 0);
     lcd.print("Steps /2:");
     if (steps_pointer<10) {
      steps_pointer += 1;
      print_v_step(v,steps_um[steps_pointer]);     
     }
     else {
           lcd.clear();
           lcd.setCursor(4, 0);
           lcd.print("Min step");
           print_v_step(v,steps_um[steps_pointer]);     
     }
   }
   
   if(IOKey==7){
     lcd.clear();
     lcd.setCursor(4, 0);
     lcd.print("Steps x2:");
     if (steps_pointer>0) {
           steps_pointer -= 1; //5464 is 1 mm-step, defined as max step
           print_v_step(v,steps_um[steps_pointer]);     
     }
     else{
           lcd.clear();
           lcd.setCursor(1, 0);
           lcd.print("Max step! 1mm");     
           print_v_step(v,steps_um[steps_pointer]);     
     }
   }
   
   if(IOKey==8){
      columns=10;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("1+2- 6x7% Done:5");
      lcd.setCursor(0, 1);
      lcd.print("Columns: ");    
      while(IOKey!=5){
        IOKey=0;
        IOKey=Keypad.ReadButtons();
        if(IOKey==1) columns+=1;
        if(IOKey==2) columns-=1;
        if(IOKey==6) columns*=2;
        if(IOKey==7) columns/=2;
        if (columns<3) columns=2;
        if (columns>500) columns=500;
        lcd.setCursor(11, 1);
        lcd.print(columns);
        delay(100);
        lcd.setCursor(11, 1);
        lcd.print("   ");
      }

      delay(500);
      IOKey=0;
      steps_pointer=2;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("1- 2+ step Done: 5");
      print_v_step(v,steps_um[steps_pointer]);     
      while(IOKey!=5){
        IOKey=0;
        IOKey=Keypad.ReadButtons();
        if((IOKey==1)&&(steps_pointer<10)) {
          steps_pointer=steps_pointer+1;
          print_v_step(v,steps_um[steps_pointer]);     
        }
        if((IOKey==2)&&(steps_pointer>0)) {
          steps_pointer=steps_pointer-1;
          print_v_step(v,steps_um[steps_pointer]);     
        }
      }

      delay(500);
      IOKey=0;
      setpoint=4;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("1+ 2- Done: 5");
      lcd.setCursor(0, 1);
      lcd.print("Setpoint: 0.");    
      while(IOKey!=5){
        IOKey=0;
        IOKey=Keypad.ReadButtons();
        if(IOKey==1) setpoint+=1;
        if(IOKey==2) setpoint-=1;
        if (setpoint<1) setpoint=1;
        if (setpoint>9) setpoint=9;
        lcd.setCursor(12, 1);
        lcd.print(setpoint);
      }
      setpoint_char = setpoint +'0';

      delay(500);
      IOKey=0;
      lines = 512;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("1x 2% Done: 5");
      lcd.setCursor(0, 1);
      lcd.print("Time(s): ");    
      while(IOKey!=5){
        IOKey=0;
        IOKey=Keypad.ReadButtons();
        if(IOKey==1) lines*=2;
        if(IOKey==2) lines/=2;
        if (lines<256) lines=256;
        if (lines<1024) {
          lcd.setCursor(12, 1);
          lcd.print(" ");
        }
        if (lines>4096) lines=4096;
        lcd.setCursor(9, 1);
        lcd.print(int(lines));
      }
    
      stepDelay = 1000;   //Move it slow
      for (i=0;i<columns-1;i++){
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Img: ");
          lcd.setCursor(11, 0);
          lcd.print(i+1);
          AFMing(setpoint_char,lines,Hz);          
          lcd.setCursor(11, 0);
          lcd.print("   ");
          if(i<columns-2){
            MotorMove(motor=1, dir=0, steps[steps_pointer], stepDelay);
            Pos_x=Pos_x+steps_um[steps_pointer];
          }
      }
      lcd.clear();
    }
   
   if(IOKey==9){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Acquiring 1 img");
    AFMing(setpoint_char,lines,Hz);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Done, Up!");
   }


   if(IOKey==11){
     lcd.clear();
     lcd.setCursor(0, 0);
     lcd.print("Moving");    
     Pos_x=Pos_x-steps_um[steps_pointer];
     lcd.clear();
     lcd.setCursor(0, 0);
     lcd.print("Pos_x ");
     lcd.setCursor(6, 0);
     lcd.print((long int) Pos_x);
     lcd.setCursor(0, 1);
     lcd.print("Pos_y ");
     lcd.setCursor(6, 1);
     lcd.print((long int) Pos_y);
     MotorMove(motor=1, dir=1, steps[steps_pointer], stepDelay);
     lcd.clear();
     lcd.setCursor(0, 0);
     lcd.print("                ");     
     print_v_step(v,steps_um[steps_pointer]);  
     //Serial.print((long int) Pos_x);
     //Serial.print("\n");     
   }
   if(IOKey==12){
     lcd.clear();
     lcd.setCursor(0, 0);
     lcd.print("Moving");    
     Pos_y=Pos_y+steps_um[steps_pointer];
     lcd.clear();
     lcd.setCursor(0, 0);
     lcd.print("Pos_x ");
     lcd.setCursor(6, 0);
     lcd.print((long int) Pos_x);
     lcd.setCursor(0, 1);
     lcd.print("Pos_y ");
     lcd.setCursor(6, 1);
     lcd.print((long int) Pos_y);
     MotorMove(motor=2, dir=1, steps[steps_pointer], stepDelay);
     lcd.clear();
     lcd.setCursor(0, 0);
     lcd.print("                ");     
     print_v_step(v,steps_um[steps_pointer]);  
     //Serial.print((long int) Pos_y);
     //Serial.print("\n");
   }
   if(IOKey==14){
     lcd.clear();
     lcd.setCursor(0, 0);
     lcd.print("Moving");    
     Pos_y=Pos_y-steps_um[steps_pointer];
     lcd.clear();
     lcd.setCursor(0, 0);
     lcd.print("Pos_x ");
     lcd.setCursor(6, 0);
     lcd.print((long int) Pos_x);
     lcd.setCursor(0, 1);
     lcd.print("Pos_y ");
     lcd.setCursor(6, 1);
     lcd.print((long int) Pos_y);
     MotorMove(motor=2, dir=0, steps[steps_pointer], stepDelay);
     lcd.clear();
     lcd.setCursor(0, 0);
     lcd.print("                ");
     print_v_step(v,steps_um[steps_pointer]);     
     //Serial.print((long int) Pos_y);
     //Serial.print("\n");
   }
   if(IOKey==15){
     lcd.clear();
     lcd.setCursor(0, 0);
     lcd.print("Moving");    
     Pos_x=Pos_x+steps_um[steps_pointer];
     lcd.clear();
     lcd.setCursor(0, 0);
     lcd.print("Pos_x ");
     lcd.setCursor(6, 0);
     lcd.print((long int) Pos_x);
     lcd.setCursor(0, 1);
     lcd.print("Pos_y ");
     lcd.setCursor(6, 1);
     lcd.print((long int) Pos_y);
     MotorMove(motor=1, dir=0, steps[steps_pointer], stepDelay);
     lcd.clear();
     lcd.setCursor(0, 0);
     lcd.print("                ");
     print_v_step(v,steps_um[steps_pointer]);     
     //Serial.print((long int) Pos_x);
     //Serial.print("\n");
   }

}

int print_v_step(double v, long int steps){
     //lcd.setCursor(0, 1);
     //lcd.print("                ");
     char v_str[5];
     dtostrf(v,5,3, v_str);
     lcd.setCursor(0, 1);
     lcd.print(v_str);
     lcd.setCursor(5, 1);
     lcd.print("v Dx:");
     
     lcd.setCursor(11, 1);
     lcd.print((long int) steps);  
    return 0;    
}


int MotorMove(int motor, int dir, int steps, int stepDelay){
    if (motor==1){
     digitalWrite(enablePin1, LOW); //LOW active, HIGH inactive
     digitalWrite(dirPin1, dir);   //HIGH right, LOW left
     for (int x = 0; x < steps; x++) {
       digitalWrite(stepPin1, HIGH);
       delayMicroseconds(stepDelay);
       digitalWrite(stepPin1, LOW);
       delayMicroseconds(stepDelay);
     }
     digitalWrite(enablePin1, HIGH);
    }
    else if (motor==2){
       //Comented until we put second motor and remove blinking to beep. 
     digitalWrite(enablePin2, LOW); //LOW active, HIGH inactive
     digitalWrite(dirPin2, dir);   //HIGH right, LOW left
     for (int x = 0; x < steps; x++) {
       digitalWrite(stepPin2, HIGH);
       delayMicroseconds(stepDelay);
       digitalWrite(stepPin2, LOW);
       delayMicroseconds(stepDelay);
     }
     digitalWrite(enablePin2, HIGH);      
    }     
    return 0;    
}

int AFMing(char setpoint_char, int lines, float Hz){
    //Autotune
    Keyboard.press(KEY_LEFT_ALT);Keyboard.press('v');delay(200);Keyboard.releaseAll();
    Keyboard.press(KEY_LEFT_ALT);Keyboard.press('w');delay(200);Keyboard.releaseAll();
    Keyboard.press(KEY_LEFT_ALT);Keyboard.press('c');delay(200);Keyboard.releaseAll();
    delay(500);
    Keyboard.press(KEY_LEFT_ALT);Keyboard.press('p');delay(200);Keyboard.releaseAll();
    Keyboard.press(KEY_LEFT_ALT);Keyboard.press('a');delay(200);Keyboard.releaseAll();
    Keyboard.press(KEY_LEFT_ALT);Keyboard.press('t');delay(200);Keyboard.releaseAll();
    delay(15000); //Check how much it takes to tune, now set to 15s
    Keyboard.press(KEY_LEFT_ALT);Keyboard.press('b');delay(200);Keyboard.releaseAll();
    delay(1000);
    //Engage
    Keyboard.press(KEY_LEFT_CTRL);Keyboard.press('e');delay(200);Keyboard.releaseAll();
    //Wait for beep
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("Waiting beep");
    //Keyboard.press('0');delay(200);Keyboard.press('.');delay(200);Keyboard.press(setpoint_char);delay(200);Keyboard.press(KEY_RETURN);Keyboard.releaseAll();delay(200); //0.2 modify for your value
    while(analogRead(A0)<800){}
    delay(1500);
    //Adjust amplitude setpoint
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
        
    acquisitionTime=((float) lines)*1000;
    acquisitionTime=acquisitionTime/Hz;
    acquisitionTime=acquisitionTime+20000; //Wait 512 lines/1 Hz = 512 s = 8min53s. We add 20s just in case.
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("T (s): ");
    sec_timer=0;
    StartTime = millis();
    secs = millis();
    CurrentTime = millis();
    while((CurrentTime - StartTime) < ((unsigned long) acquisitionTime)){
      secs=millis();
      CurrentTime = millis();
      while((CurrentTime-secs) < 1000) {
        CurrentTime=millis();
      }
      sec_timer=sec_timer+1;
      lcd.setCursor(7, 1);
      lcd.print((int)sec_timer);     
    }
    
    return 0;  
}
