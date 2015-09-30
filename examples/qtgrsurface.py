#!/usr/bin/env python
# -*- coding: utf-8 -*-
# -*- no-plot -*-
"""
Plot surface and contour lines using QtGR.
"""

# standard library
import sys
import os
import numpy as np
# local library
import gr
from gr.pygr import Plot, PlotAxes, PlotSurface, PlotContour, readfile
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


class ContourAxes(PlotAxes):

    def setWindow(self, xmin, xmax, ymin, ymax):
        if xmin < 1:
            xmin = 1
        if ymin < 1:
            ymin = 1
        return PlotAxes.setWindow(self, xmin, xmax, ymin, ymax)


def main(*args):
    app = QtGui.QApplication(*args)
    grw = InteractiveGRWidget()
    viewport = [0.1, 0.88, 0.1, 0.88]

    z = readfile(os.path.join(os.path.dirname(os.path.realpath(__file__)),
                              "fecr.dat"))
    zd = np.asarray(z)
    xd = yd = np.arange(1, 201)

    axes = ContourAxes(viewport)
    axes.addCurves(PlotSurface(xd, yd, zd, option=gr.OPTION_CELL_ARRAY))
    axes.addCurves(PlotContour(xd, yd, zd))
    axes.setGrid(False)
    plot = Plot(viewport).addAxes(axes)
    plot.title = "Plot surface and contour lines using QtGR"

    grw.addPlot(plot)
    grw.resize(QtCore.QSize(500, 500))
    grw.show()
    win = axes.getWindow()
    grw.update()

    sys.exit(app.exec_())

if __name__ == '__main__':
    main(sys.argv)

