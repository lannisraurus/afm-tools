##################### Imports
# Qt
from PyQt5.QtCore import *      # Basic Qt functionalities.
from PyQt5.QtWidgets import *   # GUI windows
from PyQt5.QtCore import *      # Qt threads, ...
from PyQt5.QtGui import *       # GUI Elements
# Nanoscope
import nanoscope
import matplotlib.pyplot as plt
# mine
from src.imageViewer import imageViewer

##################### Commands Window Class
class calibWindow(QWidget):

    ############################### Constructor
    def __init__(self, *args, **kwargs):

        # Intialize super
        super().__init__(*args, **kwargs)

        # Layouts
        self.mainLayout = QVBoxLayout()
        self.topLayout = QHBoxLayout()
        self.leftTopLayout = QVBoxLayout()
        self.rightTopLayout = QVBoxLayout()

        # Layout config
        self.rightTopLayout.setAlignment(Qt.AlignRight)
        self.leftTopLayout.setAlignment(Qt.AlignTop)

        # Layout add layouts
        self.mainLayout.addLayout(self.topLayout)
        self.topLayout.addLayout(self.leftTopLayout)
        self.topLayout.addLayout(self.rightTopLayout)

        # WIDGETS


        self.viewer = imageViewer(image_data)

        # WIDGETS ADD TO LAYOUTS
        self.leftTopLayout.addWidget(self.viewer)
        
        # Central layout
        self.setLayout(self.mainLayout)

        # Intializing general stuff
        self.setWindowTitle('High-Range Acquisition: Calibration')