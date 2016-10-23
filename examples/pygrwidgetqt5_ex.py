#!/usr/bin/env python
# -*- coding: utf-8 -*-
# -*- no-plot -*-
"""
GR / PyQt5 interoperability example
"""

import os
import sys
from PyQt5 import QtCore, QtGui, QtWidgets
import gr
from gr.pygr import readfile
from qtgr import GRWidget


class GraphicsWidget(GRWidget):
    def __init__(self, *args, **kwargs):
        super(GraphicsWidget, self).__init__(*args, **kwargs)
        self._draw_graphics = False
        self.setupUi()

    def setupUi(self):
        self.setWindowTitle("GRWidget Demo")
        self.resize(500, 500)

        self.DrawButton = QtWidgets.QPushButton(self)
        self.DrawButton.setText("Draw")
        self.DrawButton.setGeometry(290, 5, 100, 25)

        self.QuitButton = QtWidgets.QPushButton(self)
        self.QuitButton.setText("Quit")
        self.QuitButton.setGeometry(395, 5, 100, 25)

        self.DrawButton.clicked.connect(self.set_draw_flag)
        self.QuitButton.clicked.connect(self.quit)

    def quit(self):
        gr.emergencyclosegks()
        self.close()

    def draw(self):
        if not self._draw_graphics:
            return

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
        gr.surface(x, y, z, 5)
        gr.contour(x, y, h, z, -1)
        gr.axes(5, 5, 1, 1, 2, 2, 0.0075)

    def set_draw_flag(self):
        self._draw_graphics = True
        self.update()

    def paintEvent(self, ev):
        super(GraphicsWidget, self).paintEvent(ev)
        painter = QtGui.QPainter(self)
        painter.drawText(15, 15, "Contour Example using PyQt5/GRWidget ...")


if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    w = GraphicsWidget()
    w.show()
    sys.exit(app.exec_())
