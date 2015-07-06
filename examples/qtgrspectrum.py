#!/usr/bin/env python
# -*- coding: utf-8 -*-
# -*- no-plot -*-
"""
Sample microphone input and display power spectrum in QtGR.
"""

# standard library
import sys
# third party
import pyaudio
import numpy
from numpy import arange, sin, pi
from scipy import signal
# local library
import gr
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

FS = 44100  # Sampling frequency
SAMPLES = 4096
F = [FS / float(SAMPLES) * t for t in range(0, SAMPLES // 2)]

mic = None
def get_spectrum():
    global mic
    if mic is None:
        pa = pyaudio.PyAudio()
        mic = pa.open(format=pyaudio.paInt16, channels=1, rate=FS,
                      input=True, frames_per_buffer=SAMPLES)
    amplitudes = numpy.fromstring(mic.read(SAMPLES), dtype=numpy.short)
    return abs(numpy.fft.fft(amplitudes / 32768.0))[:SAMPLES/2]


def parabolic(x, f, i):
    xe = 1/2. * (f[i-1] - f[i+1]) / (f[i-1] - 2 * f[i] + f[i+1]) + x
    ye = f[i] - 1/4. * (f[i-1] - f[i+1]) * (xe - x)
    return xe, ye


class PeakBars(PlotCurve):

    def drawGR(self):
        if self.linetype is not None and self.x:
            # preserve old values
            lcolor = gr.inqlinecolorind()
            gr.setlinewidth(2)
            gr.setlinecolorind(self.linecolor)

            for xi, yi in zip(self.x, self.y):
                gr.polyline([xi, xi], [0, yi])

            # restore old values
            gr.setlinecolorind(lcolor)
            gr.setlinewidth(1)


class DependentPlotCurve(PlotCurve):

    def __init__(self, *args, **kwargs):
        PlotCurve.__init__(self, *args, **kwargs)
        self._dependent = []

    @property
    def dependent(self):
        """Return dependent objects which implement the GRMeta interface."""
        return self._dependent

    @dependent.setter
    def dependent(self, value):
        self._dependent = value

    # pylint: disable=W0221
    @PlotCurve.visible.setter
    def visible(self, flag):
        PlotCurve.visible.__set__(self, flag)
        for dep in self.dependent:
            dep.visible = flag

    def drawGR(self):
        PlotCurve.drawGR(self)
        for dep in self.dependent:
            if dep.visible:
                dep.drawGR()


class ClampedPlotAxes(PlotAxes):

    def setWindow(self, xmin, xmax, ymin, ymax):
        if ymin < 0:
            ymin = 0
        PlotAxes.setWindow(self, xmin, xmax, ymin, ymax)


class MainWindow(QtGui.QMainWindow):

    def __init__(self, *args, **kwargs):
        QtGui.QMainWindow.__init__(self, *args, **kwargs)
        self._grw = InteractiveGRWidget()
        self.setCentralWidget(self._grw)

        viewport = [0.1, 0.88, 0.1, 0.88]

        self.curve = DependentPlotCurve([0], [0], linecolor=4)
        self.peakbars = PeakBars([], [], linecolor=2)
        self.curve.dependent = [self.peakbars]
        axes = ClampedPlotAxes(viewport, 50, 5, 1, 2)
        axes.setWindow(50, 25000, 0, 100)
        axes.addCurves(self.curve)
        plot = Plot(viewport).addAxes(axes)
        plot.title = "QtGR Power Spectrum"
        plot.subTitle = "Capturing Microphone"
        axes.scale = gr.OPTION_X_LOG
        self._grw.addPlot(plot)

        timer = QtCore.QTimer(self)
        timer.timeout.connect(self.updateData)
        timer.start(40)

    def updateData(self):
        power, peakind = None, None
        try:
            power = get_spectrum()
            peakind = signal.find_peaks_cwt(power, numpy.array([5]))
        except IOError:
            return
        else:
            self.peakbars.visible = False
            self.curve.x = F[1:]
            self.curve.y = power[1:]
            self.peakbars.x, self.peakbars.y = [], []
            for p in peakind:
                if power[p] > 10:
                    self.peakbars.visible = True
                    xi, yi = parabolic(F[p], power, p)
                    self.peakbars.x.append(xi)
                    self.peakbars.y.append(yi)
            self.update()


def main(*args):
    app = QtGui.QApplication(sys.argv)
    mainWin = MainWindow()
    mainWin.resize(QtCore.QSize(500, 500))
    mainWin.show()
    sys.exit(app.exec_())


if __name__ == "__main__":
    main(sys.argv)

