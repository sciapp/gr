#!/usr/bin/env python
# -*- coding: utf-8 -*-
# -*- no-plot -*-
"""
Plot contour lines using QtGR.
"""

# standard library
import sys
# local library
from gr.pygr import Plot, PlotAxes, PlotContour
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

    xd = [3, 3, 10, 18, 18, 10, 10, 5, 1, 15, 20, 5, 15, 10, 7, 13, 16]
    yd = [3, 18, 18, 3, 18, 10, 1, 5, 10, 5, 10, 15, 15, 15, 20, 20, 8]
    zd = [25, 25, 25, 25, 25, -5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 25]

    axes = ContourAxes(viewport)
    axes.addCurves(PlotContour(xd, yd, zd, nx=40, ny=40))
    axes.setGrid(False)
    plot = Plot(viewport).addAxes(axes)
    plot.title = "Plot contour lines using QtGR"

    grw.addPlot(plot)
    grw.resize(QtCore.QSize(500, 500))
    grw.show()
    win = axes.getWindow()
    grw.update()

    sys.exit(app.exec_())

if __name__ == '__main__':
    main(sys.argv)

