"""
Duarte Tavares, João Camacho, Jorge Costa, Margarida Saraiva
IST, 2025 - IAD

This programme consists of a GUI application with the purpose of
communicating with an Arduino via USB.

The Arduino should be equipped with a command parsing software to fulfill
the requests given by this software.

These commands may consist of a key-word (example: acquire), followed by
various parameters, depending on desired functionalities.

Additionally, a 'request_commands' command should be implemented in the
Arduino software, returning a description of the implemented commands,
so that users can perform various operations without needing to check the
code developed there.

Enjoy!

"""

##################### Python Libraries
from PyQt5.QtWidgets import QApplication # Qt App

##################### User defined classes/functions
from src.mainWindow import mainWindow

##################### Stop __pycache__ from generating
import sys
sys.dont_write_bytecode = True

##################### Main Programme Function

if __name__ == '__main__':

    # Start QApplication
    mainApp = QApplication([])
    
    # Create main window
    mainWin = mainWindow()
    mainWin.installEventFilter(mainWin)
    
    # Start event loop
    mainApp.exec()