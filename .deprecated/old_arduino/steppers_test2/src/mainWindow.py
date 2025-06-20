"""
Duarte Tavares, João Camacho, Jorge Costa, Margarida Saraiva
IST, 2025 - IAD

This file contains the main window class, containing all the UI
and programme methods.

"""
##################### Python Library Imports
from PyQt5.QtCore import *      # Basic Qt functionalities.
from PyQt5.QtWidgets import *   # GUI windows
from PyQt5.QtCore import *      # Qt threads, ...
from PyQt5.QtGui import *       # GUI Elements
import re
import time                     # For routines
import pyqtgraph.exporters
import pyqtgraph

##################### User defined functions (imports)
from src.comms.arduinoComms import arduinoComms
from src.ui.commandWindow import commandWindow
from src.ui.graphWindow import graphWindow
from src.ui.inputConsole import inputConsole
from src.utils.internalCommandThread import internalCommandThread

##################### Main Programme Class
class mainWindow(QWidget):

    ############ Constructor
    def __init__(self, *args, **kwargs):

        ##### UI
        super().__init__(*args, **kwargs)   # Initialize parent class

        # UI - General
        self.setWindowTitle('Raspberry Pi - Arduino Interface')
        self.setGeometry(500, 500, 420, 220)

        # UI Elements - Buttons
        self.startButton = QPushButton('Run')
        self.startButton.clicked.connect(self.startCommand)
        
        self.stopButton = QPushButton('Interrupt')
        self.stopButton.clicked.connect(self.stopCommand)
        
        self.commandInfoButton = QPushButton('Commands')
        self.commandInfoButton.clicked.connect(self.infoCommand)
        self.commandInfoButton.setIcon(self.style().standardIcon(QStyle.SP_MessageBoxInformation))
        
        self.graphButton = QPushButton('Graph')
        self.graphButton.clicked.connect(self.graphShow)
        self.graphButton.setIcon(self.style().standardIcon(QStyle.SP_FileDialogContentsView))

        # UI Elements - Pixmaps
        self.groupLogoPixmap = QPixmap('assets/logo.png')
        self.groupLogoPixmap = self.groupLogoPixmap.scaled(250, 250, Qt.KeepAspectRatio)

        # UI Elements - Labels
        self.commandInputLabel = QLabel('Command:')

        self.groupLogoLabel = QLabel()
        self.groupLogoLabel.setPixmap(self.groupLogoPixmap)

        # UI Elements - Line/Text Edits
        self.commandInputLine = inputConsole('assets/input_log', self)
        self.commandInputLine.returnPressed.connect(self.startCommand)
        
        self.commandOutputLine = QTextEdit()
        self.commandOutputLine.setReadOnly(True)
        self.commandOutputLine.setMinimumSize(500,250)
        
        self.logTextSplashScreen = "******************************\n* RPi - Arduino Interface (Log) *\n******************************\n\n"
        self.logText(self.logTextSplashScreen)  # Function defined further

        # Create a layouts
        self.mainLayout = QVBoxLayout()
        self.setLayout(self.mainLayout)
        
        self.topLayout = QHBoxLayout()
        self.midLayout = QHBoxLayout()
        self.bottomLayout = QVBoxLayout()
        
        self.mainLayout.addLayout(self.topLayout)
        self.mainLayout.addLayout(self.midLayout)
        self.mainLayout.addLayout(self.bottomLayout)

        # UI Elements - Top Layout 
        self.topLayout.addWidget(self.groupLogoLabel)
        self.topLayout.addWidget(self.commandOutputLine)
        
        # UI Elements - Mid Layout
        self.midLayout.addWidget(self.commandInputLabel)
        self.midLayout.addWidget(self.commandInputLine)
        self.midLayout.addWidget(self.commandInfoButton)
        self.midLayout.addWidget(self.graphButton)

        # UI Elements - Bottom Layout
        self.bottomLayout.addWidget(self.startButton)
        self.bottomLayout.addWidget(self.stopButton)

        # Show window
        self.show()


        ##### COMMS - Serial Communication Object
        self.arduinoCommsObject = arduinoComms()
        self.logText(self.arduinoCommsObject.initialize())

        ##### INTERNAL COMMANDS AND MIXED COMMANDS - function dictionary and descriptions
        self.intCommands = {
            "test_port": self.testPort,
            "change_port": self.changePort,
            "clear": self.logClear,
            "list_ports": self.listPorts,
            "set_titles": self.setTitles,
            "clear_graph": self.clearGraph,
            "save_graph": self.saveGraph,
        }
        self.intCommandsDescription = "test_port: Tests communications through the selected port.\n" + \
            "change_port PORT_NAME: Changes the port to whatever the user provides.\n" + \
            "clear: Clears the console log.\n" + \
            "list_ports: Re-checks available ports and prints them on screen.\n" + \
            "set_titles: Set the X axis title (specified after -x), Y axis title (specified after -y) and/or graph title (specified after -g).\n" + \
            "clear_graph: Clears all data in the graph.\n" + \
            "save_graph: Saves graph displayed in Graph window.\n"
        
        self.mixCommands = {

        }

        self.mixCommandsDescription = "NONE.\n"

        ##### External commands
        self.extCommandsDescription = ""
        self.hasRequestedExt =  False

        ##### Additional Windows
        self.infoWindow = commandWindow(self.intCommandsDescription, self.extCommandsDescription, self.mixCommandsDescription)
        self.graphWindow = graphWindow()

        ##### Threaded processes booleans
        self.interrupt = False  # Interrupt a thread



    ############# Events
    def closeEvent(self,event):
        self.interrupt = True
        time.sleep(0.1)
        self.arduinoCommsObject.closePort()
        self.graphWindow.close()
        self.infoWindow.close()
        self.commandInputLine.saveLog()
        event.accept()



    ############ Button Methods
    # 'Run' Button; used to parse and send commands
    def startCommand(self):
        
        # Uninterrupt for potential new thread routines.
        self.interrupt = False

        # Extract the command from the input line
        cmd = self.commandInputLine.text()

        # Clear command line input
        self.commandInputLine.clear()
        self.commandInputLine.setFinal(cmd)

        # Reset previous command log index
        self.commandInputLine.resetIndex()

        # Add new empty line to previous command log
        if cmd != "" and not cmd.isspace():
            self.commandInputLine.addLine()

        # Split the command into substrings
        # This regex certifies that data between quotes is not separated by spaces, allowing for string data with spaces.
        expression = r'(?:[^\s"]|"(?:\\.|[^"\\])*")+'       # Regex for data between quotes
        cmdPartitions = re.findall(expression, cmd.strip())
        
        # Remove quotes from the quoted strings
        cmdPartitions = [part.strip('"') if part.startswith('"') and part.endswith('"') else part for part in cmdPartitions]

        # Extract Command Tag (first word)
        cmdTag = ""
        if cmdPartitions:
            cmdTag = cmdPartitions[0]
        
        # Command Extraction Variables
        cmdArgs = []
        cmdKwargs = {}
        keyKwarg = ""

        # Extract arguments from the command - place in cmgArgs and cmdKwargs
        if cmdPartitions:
            for string in cmdPartitions[1:]:
                if string.startswith('-'):
                    if keyKwarg:
                        cmdKwargs[keyKwarg] = True
                    keyKwarg = string[1:]
                else:
                    if keyKwarg:
                        cmdKwargs[keyKwarg] = string
                        keyKwarg = ""
                    else:
                        cmdArgs.append(string)
            if keyKwarg:
                cmdKwargs[keyKwarg] = True
        
        # Parsing complete; Send command to its' rightful place.
        if cmdTag in self.intCommands.keys():
            # Run internal commands - processed by RPi
            self.logText("* Running internal command \'"+cmd+"\'\n")
            self.intCommands[cmdTag](*cmdArgs,**cmdKwargs)
        elif cmdTag in self.mixCommands.keys():
            # Run mixed commands - processed by RPi, but sends external commands
            self.logText("* Running mixed command \'"+cmd+"\'\n")
            self.mixCommands[cmdTag](*cmdArgs,**cmdKwargs)
        elif cmd:
            # Run external commands - processed by arduino (NON EMPTY ONLY)
            self.logText("* Running external command \'"+cmd+"\'\n")
            result = self.arduinoCommsObject.sendExternalCommand(cmd)
            self.logText(result[0])
            self.logText(">>> "+result[1])


    # 'Interrupt' Button; used to interrupt on-going processes in the RPi/Arduino
    def stopCommand(self):
        self.logText("* Interrupting...\n")
        self.interrupt = True
    
    # Info icon button. Displays information on implemented commands in separate window.
    def infoCommand(self):
        # Retrieve descriptions from Arduino
        self.requestExternalCommands()
        # Open info window with the descriptions
        self.infoWindow.updateExternalCommands(self.extCommandsDescription)
        self.infoWindow.show()
        self.infoWindow.activateWindow()

    # Investigate icon button. Opens up a pyqtgraph window.
    def graphShow(self):
        self.graphWindow.show()
        self.graphWindow.activateWindow()



    ############ Utility
    # Logs text onto the programme log window.
    def logText(self,msg): 
        self.commandOutputLine.setPlainText(self.commandOutputLine.toPlainText() + msg)
        self.commandOutputLine.moveCursor(self.commandOutputLine.textCursor().End)

    # Adds a point to the pyqtgraph implemented in graphWindow.
    def addDataPoint(self,point):
        # If list is in the required format
        if len(point) == 2:
            self.graphWindow.addDataPoint(point[0],point[1])
        # Otherwise - error
        else:
            self.logText("* ERROR: data point is not a data pair...\n")
            errMsg = ""
            for p in point:
                if type(p) == str:
                    errMsg += p+' '
            if len(errMsg) > 0:
                self.logText("* ...the data received was: "+errMsg+'\n')

    # Request external commands
    def requestExternalCommands(self):
        self.extCommandsDescription = self.arduinoCommsObject.sendExternalCommand("request_commands")[1].replace("|","\n")
        self.extCommandsKeys = [tag.partition(":")[0].partition(" ")[0] for tag in self.extCommandsDescription.split("\n")][:-1]



    ############ Internal Commands - Routines processed by this programme.
    # Clears programme log
    def logClear(self,*args,**kwargs):
        # Command takes no arguments; error checking if there are arguments
        if args:
            self.logText("* ERROR: Unknown args in the clear function.\n")
        if kwargs:
            self.logText("* ERROR: Unknown kwargs in the clear function.\n")
        # Clear all text and move cursor to beginning
        if (not args) and (not kwargs):
            self.commandOutputLine.setPlainText(self.logTextSplashScreen)
            self.commandOutputLine.moveCursor(self.commandOutputLine.textCursor().End)
    
    # Tests the communication with a port
    def testPort(self,*args,**kwargs):
        # Command takes no arguments; error checking if there are arguments
        if args:
            self.logText("* ERROR: Unknown args in the test_port function.\n")
        if kwargs:
            self.logText("* ERROR: Unknown kwargs in the test_port function.\n")
        # Try opening with arduinoCommsObject, convert status int into string using object.
        if not args and not kwargs:
            self.logText(self.arduinoCommsObject.tryOpeningIntToStr(self.arduinoCommsObject.tryOpening()))

    # List all the available devices in the computer
    def listPorts(self,*args,**kwargs):
        # Command takes no arguments; error checking if there are arguments
        if args:
            self.logText("* ERROR: Unknown args in the list_ports function.\n")
        if kwargs:
            self.logText("* ERROR: Unknown kwargs in the list_ports function.\n")
        # Use arduinoCommsObject to list all the ports in the computer.
        if not args and not kwargs:
            self.logText(self.arduinoCommsObject.listPorts())

    # Change selected device for communication
    def changePort(self,*args,**kwargs):
        # Command takes no kwargs, only one arg; error checking.
        if kwargs:
            self.logText("* ERROR: Unknown kwargs in the change_port function.\n")
        elif len(args) != 1:
            self.logText("* ERROR: change_port functions expects 1 argument!\n")
        # Change the port with arduinoCommsObject
        else:
            self.logText(self.arduinoCommsObject.changePort(args[0]))

    
    # Set the titles of the graph
    def setTitles(self, *args, **kwargs):
        # This function takes no args, only kwargs; error checking ahead:
        if len(args) > 0:
            return self.logText("* ERROR: set_titles does not take regular arguments, only kwargs.\n")
        if len(kwargs) == 0:
            return self.logText("* ERROR: Parameters missing in set_titles function\n")
        # Read kwargs and set graph titles. x - x label; y - y label; g - graph title
        for tag in kwargs.keys():
            if tag not in ["x", "y", "g"]:
                return self.logText("* ERROR: Unrecognised parameter in set_titles function\n")
            if tag == "x":
                self.graphWindow.graphPlot.setLabel("bottom", kwargs["x"])
            if tag == "y":
                self.graphWindow.graphPlot.setLabel("left", kwargs["y"])
            if tag == "g":
                self.graphWindow.graphPlot.setTitle(kwargs["g"])

    # Clear current graph
    def clearGraph(self, *args, **kwargs):
        # Function takes no arguments or kwargs.
        if args:
            self.logText("* ERROR: Unknown args in the clear_graph function.\n")
        if kwargs:
            self.logText("* ERROR: Unknown kwargs in the clear_graph function.\n")
        # If no arguments, clear graph.
        if not args and not kwargs:
            self.graphWindow.clearGraph()

    # Save current graph
    def saveGraph(self):
        self.exporter = pyqtgraph.exporters.ImageExporter(self.graphWindow.graphPlot.scene())
        self.exporter.export("data.png")


    ############ Mixed Commands - Routines processed by this programme, using external commands as well

    ############ Mixed Command Threading
