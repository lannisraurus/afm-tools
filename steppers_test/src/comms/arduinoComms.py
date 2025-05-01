"""
Duarte Tavares, João Camacho, Jorge Costa, Margarida Saraiva
IST, 2025 - IAD

This file contains all arduino communication capabilities.

"""

##################### Python Library Imports
import serial
import serial.tools.list_ports
import time

##################### Arduino Communication Class
class arduinoComms:

    # Initialize class - return status string
    def initialize(self):
        
        msg = "* Setting up arduino communication...\n"
        
        # Signal that communications are occupied with a thread
        self.occupied = False

        # Communication specifications
        self.serialObject = serial.Serial()
        self.serialObject.baudrate = 115200
        self.serialObject.timeout = None
        
        # Timeout if reading goes on for too long
        self.timeoutSeconds = 5

        # List ports in the status msg
        msg += self.listPorts()
        # Find valid ports; 2 arduino alternatives are specified, these can be set manually anyways.
        self.validPorts = [port for port in self.systemDevices if port in ['/dev/ttyACM0','/dev/ttyUSB0']]
        try:
            self.serialObject.port = self.validPorts[0]
        except:
            msg += "* WARNING: No valid ports found!\n"
        if len(self.validPorts) > 1:
            msg += "* WARNING: Multiple valid ports found! Defaulted to first port.\n"
        msg += "* Selected Device: "+self.selectedPortStr()+'.\n'
        msg += self.tryOpeningIntToStr(self.tryOpening())
        return msg+"* Finished arduino communication setup.\n"

    # Lists available ports in message format.
    def listPorts(self):
       self.systemDevices = [port.device for port in serial.tools.list_ports.comports()]
       return "* Available Devices: "+self.systemPortsStr()+'.\n'

    # Make a list of the system available ports, string.
    def systemPortsStr(self):
        msg = ""
        for port in self.systemDevices:
            msg += "["+str(port)+'] '
        if msg == "":
            return "None"
        return msg

    # Returns the string of the selected port
    def selectedPortStr(self):
        return str(self.serialObject.port)

    # Checks if serial is open for communication
    def isOpen(self):
        return self.serialObject.isOpen()

    # Try opening, return status error int code depending.
    def tryOpening(self):
        try:
            if not self.isOpen():
                self.serialObject.open()
        except:
            return 1
        return 0
    
    # Turn tryOpening status into a string explaining the error.
    def tryOpeningIntToStr(self,error):
        if error == 0:
            return "* Successfully opened port \'"+self.selectedPortStr()+".\'\n"
        elif error == 1:
            return "* ERROR: Could not open serial port! Port may be busy/invalid!\n"

    # Changes port for communication, returns status message of the operation.
    def changePort(self, port):
        if port in self.systemDevices:
            self.serialObject.port = port
            return "* Changed port to \'"+self.selectedPortStr()+"\'\n"
        else:
            return "* ERROR: Port is not in the list of permitted ports! Permitted ports are: "+self.systemPortsStr()+".\n"

    # Closes the communication port.
    def closePort(self):
        self.serialObject.close()

    # Write message onto the arduino.
    def writeMessage(self,msg):
        # Check COnnection
        tryOpen = self.tryOpening()
        if tryOpen != 0:
            return self.tryOpeningIntToStr(tryOpen)
        # Send
        self.serialObject.write((msg+'\n').encode('utf-8'))
        return "* Sent the message: \'"+msg+"\' to the Arduino Port.\n"

    # Read a message from the arduino
    def readMessage(self):
        # Error Checking
        tryOpen = self.tryOpening()
        if tryOpen != 0:
            return self.tryOpeningIntToStr(tryOpen)
        
        # Keep reading until there is something; checking for timeouts
        message = ""
        start_time = time.time()
        # \n signal the end of an instruction
        while (len(message) == 0 or message[-1] != '\n') and time.time()-start_time <= self.timeoutSeconds:
            while self.serialObject.inWaiting() > 0 :
                message += self.serialObject.read(1).decode('utf-8')
        
        if time.time()-start_time >= self.timeoutSeconds:
            return "TIMEOUT! ("+str(time.time()-start_time)+" seconds).\n"
        
        return message 

    # Send an external command; uses read and write. Waits until the comm channel is unoccupied.
    def sendExternalCommand(self, cmd):
            while(self.occupied):
                pass
            self.occupied = True 
            writeMsg = self.writeMessage(cmd)
            readMsg = self.readMessage()
            self.occupied = False
            # Return array, first element is write message, second is the read msg.
            return (writeMsg,readMsg)