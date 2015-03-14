#!/usr/bin/env python
# -*- coding: utf-8 -*-
# -*- no-plot -*-
"""
Simple MRI visualization example
    - drag the mouse to move the camera
    - use the mousewheel to change the threshold value for densities
"""

from os import environ
from sys import argv, exit
from glob import glob

have_pyside = True
try:
    from PySide import shiboken
except ImportError:
    try:
        import shiboken
    except ImportError:
        have_pyside = False
if have_pyside:
    from PySide import QtCore, QtGui
else:
    try:
        from PyQt4 import QtCore, QtGui
        from sip import unwrapinstance
    except ImportError:
        print('Unable to load Qt binding')
        exit(-1)

from numpy import fromfile, uint16, ones
from math import sin, cos, pi
from gr import clearws, setviewport, setwindow, updatews
from gr3 import setbackgroundcolor, triangulate, createmesh, cameralookat, \
                drawmesh, drawimage, clear, deletemesh, GR3_Drawable, export
from ctypes import c_int


data = fromfile("mri.raw", uint16)
data = data.reshape((64, 64, 93))

def spherical_to_cartesian(r, theta, phi):
    x = r * sin(theta) * cos(phi)
    y = r * sin(theta) * sin(phi)
    z = r * cos(theta)
    return (x, y, z)


class GrWidget(QtGui.QWidget):
    def __init__(self):
        QtGui.QWidget.__init__(self)

        self.setup_ui(self)

        environ["GKS_WSTYPE"] = "381"
        environ["GKS_DOUBLE_BUF"] = "True"

        self.w = 500
        self.h = 500
        self.beginx = self.x = 0
        self.beginy = self.y = 0
        self.painter = QtGui.QPainter()
        self.isolevel = (data.min() + data.max()) // 2
        self.export = False
        
    @staticmethod
    def setup_ui(form):
        form.setWindowTitle("GrWidget")
        form.resize(QtCore.QSize(500, 500).expandedTo(form.minimumSizeHint()))

    def draw_image(self):
        w, h = (self.w, self.h)
        clearws()
        setwindow(0, self.w, 0, self.h)
        setviewport(0, 1, 0, 1)

        setbackgroundcolor(1, 1, 1, 0)
        vertices, normals = triangulate(data, \
          (1.0/64, 1.0/64, 1.0/128), (-0.5, -0.5, -0.5), self.isolevel)
        mesh = createmesh(len(vertices)*3, vertices, normals, \
          ones(vertices.shape))
        drawmesh(mesh, 1, (0,0,0), (0,0,1), (0,1,0), (1,1,1), (1,1,1))
        center = spherical_to_cartesian(-2, pi*self.y/self.h+pi/2, pi*self.x/self.w)
        up = spherical_to_cartesian(1, pi*self.y/self.h+pi, pi*self.x/self.w)
        cameralookat(center[0], center[1], -0.25+center[2], 0, 0, -0.25, up[0], up[1], up[2])
        drawimage(0, self.w, 0, self.h, \
          self.w, self.h, GR3_Drawable.GR3_DRAWABLE_GKS)
        if self.export:
            export("mri.html", 800, 800)
            print("Saved current isosurface to mri.html")
            self.export = False
        clear()
        deletemesh(c_int(mesh.value))

    def mouseMoveEvent(self, event):
        self.x += event.pos().x() - self.beginx
        self.y -= event.pos().y() - self.beginy
        self.beginx = event.pos().x()
        self.beginy = event.pos().y()
        self.draw_image()
        self.update()

    def mousePressEvent(self, event):
        self.beginx = event.pos().x()
        self.beginy = event.pos().y()
        
    def keyPressEvent(self, event):
        if event.key() == QtCore.Qt.Key_Escape:
            self.close()
        elif event.text() == 'h':
            self.export = True
        self.draw_image()
        self.update()

    def wheelEvent(self, ev):
        if ev.delta() > 0:
            if self.isolevel * 1.01 < data.max():
                self.isolevel *= 1.01
        else:
            if self.isolevel * 0.99 > data.min():
                self.isolevel *= 0.99
        self.isolevel = int(self.isolevel + 0.5)
        self.draw_image()
        self.update()

    def resizeEvent(self, event):
        self.draw_image()
        self.update()

    def paintEvent(self, ev):
        self.painter.begin(self)
        if have_pyside:
            environ['GKSconid'] = "%x!%x" % (
                int(shiboken.getCppPointer(self)[0]),
                int(shiboken.getCppPointer(self.painter)[0]))
        else:
            environ["GKSconid"] = "%x!%x" % (unwrapinstance(self),
                                             unwrapinstance(self.painter))
        updatews()
        self.painter.end()


app = QtGui.QApplication(argv)
win = GrWidget()
win.show()

exit(app.exec_())
