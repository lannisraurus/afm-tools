"""
Duarte Tavares, João Camacho, Jorge Costa, Margarida Saraiva
IST, 2025 - IAD

This file contains the commandWindow class, which displays text only.

"""
##################### Imports
from PyQt5.QtCore import *      # Basic Qt functionalities.
from PyQt5.QtWidgets import *   # GUI windows
from PyQt5.QtCore import *      # Qt threads, ...
from PyQt5.QtGui import *       # GUI Elements

##################### Commands Window Class
class commandWindow(QWidget):
    # Constructor
    def __init__(self, int_commands, ext_commands, mix_commands):
        super().__init__()
        self.setWindowTitle("Commands")

        # Text sections
        self.intCommands = int_commands
        self.extCommands = ext_commands
        self.mixCommands = mix_commands

        self.commandOutputLine = QTextEdit()
        self.commandOutputLine.setReadOnly(True)
        
        self.setFixedSize(600,500)

        self.logTextSplashScreen = ""

        self.setCommandText()

        self.mainLayout = QVBoxLayout()
        self.setLayout(self.mainLayout)
        self.mainLayout.addWidget(self.commandOutputLine)

    # Set the command text that is displayed
    def setCommandText(self):
        self.commandOutputLine.setPlainText(self.logTextSplashScreen + \
                "* Internal Commands - Processed by RaspberryPi:\n" + self.intCommands + "\n" + \
                "* External Commands - Processed by Arduino:\n" + self.extCommands + "\n" +\
                "* Mixed Commands - Processed by both RaspberryPi and Arduino:\n" +self.mixCommands )

    # Update external commands, provided by the main Window class.
    def updateExternalCommands(self,desc):
        self.extCommands = desc
        self.setCommandText()