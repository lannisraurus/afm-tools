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

### DEPRECATED FEATURE - 3D VIEWER... IMPLEMENT IT IF YOU WANT :)
"""########################## OpenGL Widget
class GLWidget(QGLWidget):
    def __init__(self, parent=None):
        self.parent = parent
        QGLWidget.__init__(self, parent)
    
    def initializeGL(self):
        self.qglClearColor(QColor(80, 80, 140))
        gl.glEnable(gl.GL_DEPTH_TEST)

        self.initGeometry()

        self.x_rot = 0
        self.y_rot = 0
        self.last_x = 0
        self.last_y = 0
        self.is_dragging = False
        self.cameraDist = 3.0
        self.scrollSensitivity = 0.005
        self.axesMult = 0.10
        self.axesAdd = 30
        self.axesCamera = 3.0

    def resizeGL(self, width, height):
        gl.glViewport(0, 0, width, height)
        gl.glMatrixMode(gl.GL_PROJECTION)
        gl.glLoadIdentity()
        aspect = width / float(height)

        GLU.gluPerspective(45.0, aspect, 1.0, 100.0)
        gl.glMatrixMode(gl.GL_MODELVIEW)

    def drawAxes(self):        
        # X
        gl.glColor3f(1, 0, 0)
        gl.glBegin(gl.GL_LINES)
        gl.glVertex3f(0, 0, 0)
        gl.glVertex3f(1, 0, 0)
        gl.glEnd()
        # Y
        gl.glColor3f(0, 1, 0)
        gl.glBegin(gl.GL_LINES)
        gl.glVertex3f(0, 0, 0)
        gl.glVertex3f(0, 1, 0)
        gl.glEnd()
        # Z
        gl.glColor3f(0, 0, 1)
        gl.glBegin(gl.GL_LINES)
        gl.glVertex3f(0, 0, 0)
        gl.glVertex3f(0, 0, 1)
        gl.glEnd()

    def paintGL(self):
        
        # CLEAR SETUP
        gl.glClear(gl.GL_COLOR_BUFFER_BIT | gl.GL_DEPTH_BUFFER_BIT) # Clear canvas

        # Allow for efficient memory access
        gl.glEnableClientState(gl.GL_VERTEX_ARRAY)
        gl.glEnableClientState(gl.GL_COLOR_ARRAY)

        # OBJECTS
        gl.glPushMatrix()
        gl.glTranslate(0.0, 0.0, -self.cameraDist)      # Translate (camera pos)
        gl.glScale(1.0, 1.0, 1.0)           # Scale coords
        gl.glRotatef(self.x_rot, 1, 0, 0)   # Rotate according to mouse input
        gl.glRotatef(self.y_rot, 0, 1, 0)   # Rotate according to mouse input
        gl.glTranslate(-0.5, -0.5, -0.5)    # Center object
        gl.glVertexPointer(3, gl.GL_FLOAT, 0, self.vertVBO)
        gl.glColorPointer(3, gl.GL_FLOAT, 0, self.colorVBO)
        gl.glDrawElements(gl.GL_QUADS, len(self.cubeIdxArray), gl.GL_UNSIGNED_INT, self.cubeIdxArray)
        gl.glPopMatrix()

        gl.glViewport(self.axesAdd, self.axesAdd, int(self.size().width()*self.axesMult), int(self.size().height()*self.axesMult))
        gl.glPushMatrix()
        gl.glTranslate(0.0, 0.0, -self.axesCamera)      # Translate (camera pos)
        gl.glScale(1.0, 1.0, 1.0)           # Scale coords
        gl.glRotatef(self.x_rot, 1, 0, 0)   # Rotate according to mouse input
        gl.glRotatef(self.y_rot, 0, 1, 0)   # Rotate according to mouse input
        gl.glTranslate(0, 0, 0)
        self.drawAxes()
        gl.glPopMatrix()
        gl.glViewport(0, 0, self.size().width(), self.size().height())

        # Disable the memory access
        gl.glDisableClientState(gl.GL_VERTEX_ARRAY)
        gl.glDisableClientState(gl.GL_COLOR_ARRAY)
        

    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton:
            self.last_x = event.x()
            self.last_y = event.y()
            self.is_dragging = True

    def wheelEvent(self, event):
        self.cameraDist -= event.angleDelta().y()*self.scrollSensitivity
        if self.cameraDist <= 0:
            self.cameraDist = 0


    def mouseMoveEvent(self, event):
        if self.is_dragging:
            dx = event.x() - self.last_x
            dy = event.y() - self.last_y

            # Update rotation based on mouse movement
            self.x_rot += dy
            self.y_rot += dx

            self.last_x = event.x()
            self.last_y = event.y()

            self.update()

    def mouseReleaseEvent(self, event):
        if event.button() == Qt.LeftButton:
            self.is_dragging = False

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
"""