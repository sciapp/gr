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

