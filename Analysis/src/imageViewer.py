##################### Imports
# Qt
from PyQt5.QtCore import *      # Basic Qt functionalities.
from PyQt5.QtWidgets import *   # GUI windows
from PyQt5.QtCore import *      # Qt threads, ...
from PyQt5.QtGui import *       # GUI Elements
# Nanoscope
from nanoscope import files
# matplotlib
import matplotlib.pyplot as plt
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure
# other
import sys
import numpy as np

class imageViewer(QWidget):
    def __init__(self, image):
        super().__init__()

        self.image = image
        self.label = QLabel("[Click to get coords]")
        self.figure = Figure()
        self.canvas = FigureCanvas(self.figure)

        layout = QVBoxLayout()
        layout.addWidget(self.canvas)
        layout.addWidget(self.label)
        self.setLayout(layout)

        self.ax = self.figure.add_subplot(111)
        self.im = self.ax.imshow(self.image, cmap='gray')
        self.figure.colorbar(self.im, ax=self.ax)

        self.canvas.mpl_connect("button_press_event", self.on_click)

        self.x = 0
        self.y = 0

    def on_click(self, event):
        if event.inaxes == self.ax:
            self.x = int(event.xdata)
            self.y = int(event.ydata)
            value = self.image[self.y, self.x]
            self.label.setText(f"Clicked at x={self.x}, y={self.y}, value={value:.3f}")