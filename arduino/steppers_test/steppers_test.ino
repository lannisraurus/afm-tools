/*
Duarte Tavares, João Camacho, Jorge Costa, Margarida Saraiva - Grupo 1 2024/2025

This file contains all Arduino functionalities, including data acquisition, external commands processing and other communication with the Raspberry Pi.

João Camacho - Code was updated for the sole purpose of the AFM experiment.
*/

// Needed variables
unsigned long pivot = 0;
String currCmd = "";

// MOTORS
int motor;  // Motor identifier, 1 or 2
int dir;    // Motor direction, 0 or 1
const int dirPin1 = 8;
const int stepPin1 = 9;
const int enablePin1 = 10;
const int dirPin2 = 11;
const int stepPin2 = 12;
const int enablePin2 = 13;

// Setup
void setup() {
	// Initialize serial communication at 115200 bits per second:
	Serial.begin(115200);
  // MOTORS
}

// Loop for reading and executing commands
void loop() {
  
	if(Serial.available()>0) { // Checks for new commands in serial communication
    
    // Read serial
    currCmd = Serial.readStringUntil('\n');
    currCmd.trim();
    
		// Run current command
		if(currCmd == "") 
    { 

    } else if (currCmd.indexOf("move_stepper") == 0) {

      String args = currCmd.substring(13);

      int space1 = args.indexOf(' ');
      int space2 = args.indexOf(' ', space1 + 1);
      int space3 = args.indexOf(' ', space2 + 1);

      int motor = args.substring(0, space1).toInt();
      int dir = args.substring(space1 + 1, space2).toInt();
      int steps = args.substring(space2 + 1, space3).toInt();
      int step_delay = args.substring(space3 + 1).toInt();

      Serial.print("Initializing movement with: motor=");
      Serial.print(motor);
      Serial.print(", dir=");
      Serial.print(dir);
      Serial.print(", steps=");
      Serial.print(steps);
      Serial.print(", step_delay=");
      Serial.println(step_delay);

      MotorMove(motor, dir, steps, step_delay);

    } else if (currCmd == "request_commands") { 
      // Print out about all external commands handled by Arduino, split by |
      Serial.print("request_commands: Gather commands from the arduino board.");
      Serial.println("|move_stepper [motor] [dir] [steps] [step_delay]: [motor] selects which of the two motors you are selecting (either 1 or 2); [dir] selects the direction of the rotation, either 0 or 1; [steps] dictates how many motor steps are to be taken; [step_delay] dictates how much time is taken between each step, in microseconds.");
      
		} else {
      // If command is not one of the above commands, sends error message for unknown instruction
			Serial.println("ARDUINO ERROR: Unknown Instruction!");
		}

   currCmd="";
	}
}

// FUNCTION STOLEN FROM AFM PAPER DONT FORGET TO REFERENCE HEHE
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
