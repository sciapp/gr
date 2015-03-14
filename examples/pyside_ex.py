#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
GR / PySide interoperability example
"""

import sys
import os
from PySide import QtCore, QtGui
try:
    from PySide import shiboken
except ImportError:
    import shiboken
from gr.pygr import *

class GrWidget(QtGui.QWidget) :
    def __init__(self, *args) :
        QtGui.QWidget.__init__(self)
        
        self.setupUi(self)

        os.environ["GKS_WSTYPE"] = "381"
        os.environ["GKS_DOUBLE_BUF"] = "True"

        self.connect(self.DrawButton, QtCore.SIGNAL("clicked()"), self.draw)
        self.connect(self.QuitButton, QtCore.SIGNAL("clicked()"), self.quit)
        self.w = 500
        self.h = 500
        self.sizex = 1
        self.sizey = 1
 
    def setupUi(self, Form) :

        Form.setWindowTitle("GrWidget")
        Form.resize(QtCore.QSize(500, 500).expandedTo(Form.minimumSizeHint()))

        self.DrawButton = QtGui.QPushButton(Form)
        self.DrawButton.setText("Draw")
        self.DrawButton.setGeometry(QtCore.QRect(290, 5, 100, 25))
        self.DrawButton.setObjectName("draw")

        self.QuitButton = QtGui.QPushButton(Form)
        self.QuitButton.setText("Quit")
        self.QuitButton.setGeometry(QtCore.QRect(395, 5, 100, 25))
        self.QuitButton.setObjectName("quit")

        QtCore.QMetaObject.connectSlotsByName(Form)

    def quit(self) :
        gr.emergencyclosegks()
        self.close()

    def draw(self) :
        self.setStyleSheet("background-color:white;");

        x = range(0, 128)
        y = range(0, 128)
        z = readfile(os.path.join(os.path.dirname(os.path.realpath(__file__)),
                                  "kws.dat"), separator='$')
        zrange = max(z) - min(z)
        h = [min(z) + i * 0.025 * zrange for i in range(0, 40)]

        gr.clearws()
        mwidth  = self.w * 2.54 / self.logicalDpiX() / 100
        mheight = self.h * 2.54 / self.logicalDpiY() / 100
        gr.setwsviewport(0, mwidth, 0, mheight)
        gr.setwswindow(0, self.sizex, 0, self.sizey)
        gr.setviewport(0.075 * self.sizex, 0.95 * self.sizex, 0.075 * self.sizey, 0.95 * self.sizey)
        gr.setwindow(1, 128, 1, 128)
        gr.setspace(min(z), max(z), 0, 90)
        gr.setcharheight(0.018)
        gr.setcolormap(-39)
        gr.surface(x, y, z, 5)
        gr.contour(x, y, h, z, -1)
        gr.axes(5, 5, 1, 1, 2, 2, 0.0075)
        self.update()

    def resizeEvent(self, event):
        self.w = event.size().width()
        self.h = event.size().height()
        if self.w > self.h:
          self.sizex = 1
          self.sizey = float(self.h)/self.w
        else:
          self.sizex = float(self.w)/self.h
          self.sizey = 1
        self.draw()

    def paintEvent(self, ev) :
        self.painter = QtGui.QPainter()
        self.painter.begin(self)
        self.painter.drawText(15, 15, "Contour Example using PySide ...")
        os.environ['GKSconid'] = "%x!%x" % (int(shiboken.getCppPointer(self)[0]), int(shiboken.getCppPointer(self.painter)[0]))
        gr.updatews()
        self.painter.end()

app = QtGui.QApplication(sys.argv)
if not sys.platform == "linux2" :
    app.setStyle('Windows')

w = GrWidget()
w.show()

sys.exit(app.exec_())
