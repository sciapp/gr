#!/usr/bin/env python
# -*- coding: utf-8 -*-
# -*- no-plot -*-
"""
Plotting two-dimensional live data using QtGR.
"""

# standard library
import sys
# third party
from numpy import arange, sin, pi
# local library
from gr.pygr import Plot, PlotAxes, PlotCurve
from qtgr.backend import QtCore, QtGui
from qtgr import InteractiveGRWidget

__author__ = "Christian Felder <c.felder@fz-juelich.de>"
__copyright__ = """Copyright 2012-2015 Forschungszentrum Juelich GmbH

This file is part of GR, a universal framework for visualization applications.
Visit http://gr-framework.org for the latest
version.

GR was developed by the Scientific IT-Systems group at the Peter Grünberg
Institute at Forschunsgzentrum Jülich. The main development has been done
by Josef Heinen who currently maintains the software.

GR is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This framework is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GR. If not, see <http://www.gnu.org/licenses/>.

"""


class MainWindow(QtGui.QMainWindow):

    def __init__(self, *args, **kwargs):
        QtGui.QMainWindow.__init__(self, *args, **kwargs)
        self._grw = InteractiveGRWidget()
        self.setCentralWidget(self._grw)

        viewport = [0.1, 0.88, 0.1, 0.88]
        x = arange(0, 2 * pi, 0.01)
        y = sin(x)

        self.curve = PlotCurve(x, y)
        axes = PlotAxes(viewport)
        axes.addCurves(self.curve)
        plot = Plot(viewport).addAxes(axes)
        plot.title = "QtGR Timer example"
        plot.subTitle = "Plotting Live-Data"
        plot.xlabel = "t"
        plot.ylabel = "sin(t)"
        plot.autoscale = PlotAxes.SCALE_X | PlotAxes.SCALE_Y
        self._grw.addPlot(plot)

        timer = QtCore.QTimer(self)
        timer.timeout.connect(self.updateData)
        timer.start(40)

    def updateData(self):
        self.curve.x += 0.05
        self.curve.y = sin(self.curve.x)
        self.update()


def main(*args):
    app = QtGui.QApplication(sys.argv)
    mainWin = MainWindow()
    mainWin.resize(QtCore.QSize(500, 500))
    mainWin.show()
    sys.exit(app.exec_())


if __name__ == "__main__":
    main(sys.argv)

