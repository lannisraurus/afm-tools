"""
Duarte Tavares, João Camacho, Jorge Costa, Margarida Saraiva
IST, 2025 - IAD

This file contains the graphWindow class, which plots data.

"""
##################### Imports
from PyQt5.QtCore import *      # Basic Qt functionalities.
from PyQt5.QtWidgets import *   # GUI windows
from PyQt5.QtCore import *      # Qt threads, ...
from PyQt5.QtGui import *       # GUI Elements
import pyqtgraph
import pyqtgraph.exporters

##################### Graph Window Class
class graphWindow(QMainWindow):
    def __init__(self):
        super().__init__() 
         # Set title
        self.setWindowTitle('Graph')

        # Set Size
        self.setGeometry(500, 500, 720, 420)

        # pyqt stuff
        self.graphPlot = pyqtgraph.PlotWidget()
        self.setCentralWidget(self.graphPlot)
        self.graphPlot.setBackground("w")
        self.xs = []
        self.ys = []
        self.graphPlot.setTitle("Title")
        self.graphPlot.setLabel("left", "Y")
        self.graphPlot.setLabel("bottom", "X")
        self.line = self.graphPlot.plot(self.xs, self.ys)

    # Add a point to the graph
    def addDataPoint(self,x,y):
        self.xs.append(x)
        self.ys.append(y)
        self.line.setData(self.xs, self.ys)
    
    # clear all data in the graph
    def clearGraph(self):
        self.xs = []
        self.ys = []
        self.line.setData(self.xs, self.ys)