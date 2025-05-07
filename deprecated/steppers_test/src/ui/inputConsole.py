"""
Duarte Tavares, João Camacho, Jorge Costa, Margarida Saraiva
IST, 2025 - IAD

This file contains the inputConsole, used in the main window.
the input console can detect inputs and take in written text from the
keyboard. It also has the capacity to save previous commands in a file, load
them up, and a capacity to autocomplete given a list of command keys from the
main window class.

"""
##################### Imports
from PyQt5.QtCore import *      # Basic Qt functionalities.
from PyQt5.QtWidgets import *   # GUI windows
from PyQt5.QtCore import *      # Qt threads, ...
from PyQt5.QtGui import *       # GUI Elements

##################### Input console class
class inputConsole(QLineEdit):

    # Constructor
    def __init__(self,logPath,mainWin,*args,**kwargs):
        super().__init__(*args,**kwargs)
        # Working vars
        self.currentText = ""
        self.index = 0
        self.lines = [""]
        self.logPath = logPath
        # load log from previous sessions
        self.loadLog()
        self.mainWin = mainWin

    # Reset index of the list; called after employing command.
    def resetIndex(self):
        self.index = 0

    # Add an empty line; called after employing command.
    def addLine(self):
        self.lines.insert(0,"")

    # Clear all lines.
    def clearLines(self):
        self.lines = [""]

    # Set most recent line to a cmd; called after employing command.
    def setFinal(self, cmd):
        self.lines[0] = cmd

    # key press events; used for detecting going up and down the list.
    def keyPressEvent(self,event):
        key = event.key()
        self.lines[self.index] = self.text()
        if key == Qt.Key_Up:
            if self.index < len(self.lines)-1:
                self.index += 1
                self.setText(self.lines[self.index])
        elif key == Qt.Key_Down:
            if self.index > 0:
                self.index -= 1
                self.setText(self.lines[self.index])
        super().keyPressEvent(event)
    
    # used for detecting tab clicking
    def event(self, event):
        if event.type() == QEvent.KeyPress and event.key() == Qt.Key_Tab:
            # Retrieve external commands if they have not been requisited yet
            if self.mainWin.hasRequestedExt == False:
                self.mainWin.requestExternalCommands()
                self.mainWin.hasRequestedExt = True
            # simple autocomplete functionality
            autocomplete = [cmd for cmd in self.mainWin.intCommands.keys() if cmd.startswith(self.text())]
            autocomplete += [cmd for cmd in self.mainWin.mixCommands.keys() if cmd.startswith(self.text())]
            autocomplete += [cmd for cmd in self.mainWin.extCommandsKeys if cmd.startswith(self.text())]
            if len(autocomplete) == 1:
                self.setText(autocomplete[0])
            else:
                minLenght = min([len(s) for s in autocomplete])
                currText = self.text()
                while(len(currText) < minLenght):
                    currText += autocomplete[0][len(currText)]
                    newautocomplete = [cmd for cmd in self.mainWin.intCommands.keys() if cmd.startswith(currText)]
                    newautocomplete += [cmd for cmd in self.mainWin.mixCommands.keys() if cmd.startswith(self.text())]
                    newautocomplete += [cmd for cmd in self.mainWin.extCommandsKeys if cmd.startswith(self.text())]
                    if(len(newautocomplete) == len(autocomplete)):
                        self.setText(currText)


            return True
        return QWidget.event(self, event)

    # Save a log to the defined path.
    def saveLog(self):
        with open(self.logPath,'w') as file:
            for elem in self.lines:
                if elem != "" and not elem.isspace():
                    file.write(elem+'\n')

    # Load log from the defined path.
    def loadLog(self):
        with open(self.logPath, 'r') as file:
            for line in file:
                self.lines.append(line.strip())