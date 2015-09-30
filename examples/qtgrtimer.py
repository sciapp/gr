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
__copyright__ = """Copyright (c) 2012-2015: Josef Heinen, Florian Rhiem,
Christian Felder and other contributors:

http://gr-framework.org/credits.html

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

The GR framework can be built with plugins that use code from the
following projects, which have their own licenses:

- MuPDF - a lightweight PDF and XPS viewer (AFPL)
- Ghostscript - an interpreter for the PostScript language and for PDF (AFPL)
- FFmpeg - a multimedia framework (LGPL / GPLv2)

"""


class ScaledPlotAxes(PlotAxes):

    def doAutoScale(self, curvechanged=None):
        xmin, xmax, ymin, ymax = PlotAxes.doAutoScale(self, curvechanged)
        bxmin, _bxmax, _bymin, _bymax = self.getBoundingBox()
        if xmin < bxmin:
            xmin = bxmin
        self.setWindow(xmin, xmax, ymin, ymax)
        return self.getWindow()


class MainWindow(QtGui.QMainWindow):

    def __init__(self, *args, **kwargs):
        QtGui.QMainWindow.__init__(self, *args, **kwargs)
        self._grw = InteractiveGRWidget()
        self.setCentralWidget(self._grw)

        viewport = [0.1, 0.88, 0.1, 0.88]
        x = arange(0, 2 * pi, 0.01)
        y = sin(x)

        self.curve = PlotCurve(x, y)
        axes = ScaledPlotAxes(viewport)
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

