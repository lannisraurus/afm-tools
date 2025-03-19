########################## Imports
# Qt
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtOpenGL import *
from PyQt5.QtWidgets import *
# OpenGL
import OpenGL.GL as gl
from OpenGL import GLU
from OpenGL.arrays import vbo
# Numpy
import numpy as np
# System
import sys


########################## OpenGL Widget
class GLWidget(QGLWidget):
    def __init__(self, parent=None):
        self.parent = parent
        QGLWidget.__init__(self, parent)
    
    def initializeGL(self):
        self.qglClearColor(QColor(0, 0, 250))
        gl.glEnable(gl.GL_DEPTH_TEST)

        self.initGeometry()

        self.rotX = 0.0
        self.rotY = 0.0
        self.rotZ = 0.0

    def resizeGL(self, width, height):
        gl.glViewport(0, 0, width, height)
        gl.glMatrixMode(gl.GL_PROJECTION)
        gl.glLoadIdentity()
        aspect = width / float(height)

        GLU.gluPerspective(45.0, aspect, 1.0, 100.0)
        gl.glMatrixMode(gl.GL_MODELVIEW)
    
    def paintGL(self):
        
        # Clear canvas
        gl.glClear(gl.GL_COLOR_BUFFER_BIT | gl.GL_DEPTH_BUFFER_BIT)
        
        # Render
        gl.glPushMatrix()    # push the current matrix to the current stack

        gl.glTranslate(0.0, 0.0, -50.0)    # third, translate cube to specified depth
        gl.glScale(20.0, 20.0, 20.0)       # second, scale cube
        gl.glRotate(self.rotX, 1.0, 0.0, 0.0)
        gl.glRotate(self.rotY, 0.0, 1.0, 0.0)
        gl.glRotate(self.rotZ, 0.0, 0.0, 1.0)
        gl.glTranslate(-0.5, -0.5, -0.5)   # first, translate cube center to origin

        gl.glEnableClientState(gl.GL_VERTEX_ARRAY)
        gl.glEnableClientState(gl.GL_COLOR_ARRAY)

        gl.glVertexPointer(3, gl.GL_FLOAT, 0, self.vertVBO)
        gl.glColorPointer(3, gl.GL_FLOAT, 0, self.colorVBO)

        gl.glDrawElements(gl.GL_QUADS, len(self.cubeIdxArray), gl.GL_UNSIGNED_INT, self.cubeIdxArray)

        gl.glDisableClientState(gl.GL_VERTEX_ARRAY)
        gl.glDisableClientState(gl.GL_COLOR_ARRAY)

        gl.glPopMatrix()    # restore the previous modelview matrix

    def setRotX(self, val):
        self.rotX = val

    def setRotY(self, val):
        self.rotY = val

    def setRotZ(self, val):
        self.rotZ = val
    
    def initGeometry(self):
        self.cubeVtxArray = np.array(
            [[0.0, 0.0, 0.0],
            [1.0, 0.0, 0.0],
            [1.0, 1.0, 0.0],
            [0.0, 1.0, 0.0],
            [0.0, 0.0, 1.0],
            [1.0, 0.0, 1.0],
            [1.0, 1.0, 1.0],
            [0.0, 1.0, 1.0]])
        
        self.vertVBO = vbo.VBO(np.reshape(self.cubeVtxArray,(1, -1)).astype(np.float32))
        self.vertVBO.bind()

        self.cubeClrArray = np.array(
            [[0.0, 0.0, 0.0],
            [1.0, 0.0, 0.0],
            [1.0, 1.0, 0.0],
            [0.0, 1.0, 0.0],
            [0.0, 0.0, 1.0],
            [1.0, 0.0, 1.0],
            [1.0, 1.0, 1.0],
            [0.0, 1.0, 1.0 ]])
        
        self.colorVBO = vbo.VBO(np.reshape(self.cubeClrArray,(1, -1)).astype(np.float32))
        self.colorVBO.bind()

        self.cubeIdxArray = np.array(
            [0, 1, 2, 3,
            3, 2, 6, 7,
            1, 0, 4, 5,
            2, 1, 5, 6,
            0, 3, 7, 4,
            7, 6, 5, 4 ])


########################## Main Window Class
class MainWindow(QMainWindow):

    # Constructor
    def __init__(self, *args, **kwargs):
        
        # Intialize super
        super().__init__(*args, **kwargs)
        
        # Window dimensions/aesthetics
        self.resize(1080, 480)
        self.setWindowTitle('AFM-Tools')
        
        # Central Widget
        self.mainWidget = QWidget(self)
        self.setCentralWidget(self.mainWidget)
        
        # OpenGL Widgets
        self.glWidget = GLWidget(self)
        self.glWidget.setFixedSize(720,600)

        # Labels
        self.mainBanner = QLabel()
        pixmap = QPixmap("AFMToolsBanner.png")
        self.mainBanner.setPixmap(pixmap)

        # Sliders
        self.sliderX = QSlider(Qt.Horizontal)
        self.sliderX.valueChanged.connect(lambda val: self.glWidget.setRotX(val*4))

        self.sliderY = QSlider(Qt.Horizontal)
        self.sliderY.valueChanged.connect(lambda val: self.glWidget.setRotY(val*4))

        self.sliderZ = QSlider(Qt.Horizontal)
        self.sliderZ.valueChanged.connect(lambda val: self.glWidget.setRotZ(val*4))
        
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
        self.rightTopLayout.addWidget(self.glWidget)
        self.bannerLayout.addWidget(self.mainBanner)
        self.leftTopLayout.addWidget(self.sliderX)
        self.leftTopLayout.addWidget(self.sliderY)
        self.leftTopLayout.addWidget(self.sliderZ)
        
        # Central layout
        self.mainWidget.setLayout(self.mainLayout)

        # OpenGL display loop
        self.timer = QTimer()
        self.timer.setInterval(10)
        self.timer.timeout.connect(self.glWidget.updateGL)
        self.timer.start()


########################## Main Function
if __name__ == '__main__':
    app = QApplication(sys.argv)

    win = MainWindow()
    win.show()

    sys.exit(app.exec_())