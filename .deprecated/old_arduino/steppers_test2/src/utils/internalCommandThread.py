"""
Duarte Tavares, João Camacho, Jorge Costa, Margarida Saraiva
IST, 2025 - IAD

This file contains the internalCommandThread, an utility class which
can call generic functions from the mainWindow, and communicate using
pyqtsignal.

"""
##################### Imports
from PyQt5.QtCore import *  # Qt threads, ...

##################### Internal Command Thread Class
class internalCommandThread(QThread):
    
    # Signals
    finished = pyqtSignal()
    send_data = pyqtSignal(list)
    
    # Constructor
    def __init__(self,obj,func,params):
        super().__init__()
        self.obj = obj       # main window
        self.func = func     # routine
        self.params = params # parameters

    # Run the routine
    def run(self):
        # Run the routine function with the given params in the initializer
        getattr(self.obj, self.func)(self.params,self.send_data)
        # Send a finished signal, if need be.
        self.finished.emit()