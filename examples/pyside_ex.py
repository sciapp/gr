#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
from PySide import QtCore, QtGui, shiboken
from gr.pygr import *

class GrWidget(QtGui.QWidget) :
    def __init__(self, *args) :
        QtGui.QWidget.__init__(self)
        
        self.setupUi(self)

        os.environ["GKS_WSTYPE"] = "381"
        os.environ["GKS_DOUBLE_BUF"] = "True"

        self.connect(self.DrawButton, QtCore.SIGNAL("clicked()"), self.draw)
        self.connect(self.QuitButton, QtCore.SIGNAL("clicked()"), self.quit)
 
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

        gr.setviewport(0.075, 0.95, 0.075, 0.95)
        gr.setwindow(1, 128, 1, 128)
        gr.setspace(min(z), max(z), 0, 90)
        gr.setcharheight(0.018)
        gr.setcolormap(-3)
        gr.surface(128, 128, x, y, z, 5)
        gr.contour(128, 128, 20, x, y, h, z, -1)
        gr.axes(5, 5, 1, 1, 2, 2, -0.0075)

        self.update()

    def paintEvent(self, ev) :
        self.painter = QtGui.QPainter()
        self.painter.begin(self)
        self.painter.drawText(15, 15, "Contour Example using PySide ...")
        os.environ['GKSconid'] = "%x!%x" % (long(shiboken.getCppPointer(self)[0]), long(shiboken.getCppPointer(self.painter)[0]))
        gr.updatews()
        self.painter.end()

app = QtGui.QApplication(sys.argv)
if not sys.platform == "linux2" :
    app.setStyle('Windows')

w = GrWidget()
w.show()

sys.exit(app.exec_())
