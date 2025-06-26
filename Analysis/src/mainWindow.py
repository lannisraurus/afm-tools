########################## Imports
# Qt
from PyQt5.QtCore import *
from PyQt5.QtGui import *
# from PyQt5.QtOpenGL import *
from PyQt5.QtWidgets import *
# OpenGL
# import OpenGL.GL as gl
# from OpenGL import GLU
# from OpenGL.arrays import vbo
# Numpy
import numpy as np
# System
import sys
# Other windows
from src.calibWindow import calibWindow

########################## Main Window Class
class MainWindow(QMainWindow):

    ######################### CONSTRUCTOR

    def __init__(self, *args, **kwargs):
        
        # Intialize super
        super().__init__(*args, **kwargs)
        
        # Window dimensions/aesthetics
        self.setWindowTitle('AFM Tools')
        
        # Central Widget
        self.mainWidget = QWidget(self)
        self.setCentralWidget(self.mainWidget)
        
        # OpenGL Widgets
        # self.glWidget = GLWidget(self)
        # self.glWidget.setFixedSize(720,600)

        # Other Windows
        self.calibWindow = calibWindow()

        # Main Window Buttons
        self.highRangeCalibButton = QPushButton("High-Range Acquisition: Calibration")
        self.highRangeImageCollage = QPushButton("High-Range Acquisition: Collage")

        self.highRangeCalibButton.pressed.connect(self.calibWindow.show)

        # Labels
        self.mainBanner = QLabel()
        pixmap = QPixmap("AFMToolsBanner.png")
        self.mainBanner.setPixmap(pixmap)
        
        # Layout create
        self.mainLayout = QVBoxLayout()
        self.topLayout = QHBoxLayout()
        self.leftTopLayout = QVBoxLayout()
        self.rightTopLayout = QVBoxLayout()
        self.bannerLayout = QVBoxLayout()

        # Layout config
        self.rightTopLayout.setAlignment(Qt.AlignRight)
        self.leftTopLayout.setAlignment(Qt.AlignTop)
        self.bannerLayout.setAlignment(Qt.AlignCenter)

        # Layout add layouts
        self.mainLayout.addLayout(self.topLayout)
        self.topLayout.addLayout(self.leftTopLayout)
        self.topLayout.addLayout(self.rightTopLayout)
        self.leftTopLayout.addLayout(self.bannerLayout)

        # Layout add widgets
        # self.rightTopLayout.addWidget(self.glWidget)
        self.bannerLayout.addWidget(self.mainBanner)
        self.leftTopLayout.addWidget(self.highRangeCalibButton)
        self.leftTopLayout.addWidget(self.highRangeImageCollage)
        
        # Central layout
        self.mainWidget.setLayout(self.mainLayout)

        # OpenGL display loop
        # self.timer = QTimer()
        # self.timer.setInterval(10)
        # self.timer.timeout.connect(self.glWidget.updateGL)
        # self.timer.start()
    
    ######################### EVENTS
    
    def closeEvent(self,event):
        self.calibWindow.close()
        event.accept()
