# -*- coding: utf-8 -*-
"""PyQt, PySide GR module

Exported Classes:

"""
# standard library
import os
import math
import logging
# local library
import gr
import qtgr.events
from qtgr.backend import QtCore, QtGui, getGKSConnectionId
from gr.pygr import Plot, PlotAxes, RegionOfInterest, DeviceCoordConverter
from qtgr.events import GUIConnector, MouseEvent, PickEvent, ROIEvent, \
    LegendEvent
from gr._version import __version__, __revision__

__author__ = "Christian Felder <c.felder@fz-juelich.de>"
__copyright__ = """Copyright (c) 2012-2015: Josef Heinen, Florian Rhiem, Christian Felder,
and other contributors:

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
"""

_log = logging.getLogger(__name__)


class GRWidget(QtGui.QWidget):

    def __init__(self, *args, **kwargs):
        super(GRWidget, self).__init__(*args, **kwargs)
        self._clear, self._update = False, False
        self._sizex, self._sizey = 1., 1.
        self._dwidth, self._dheight = self.width(), self.height()
        self._mwidth = self.width() * .0254 / self.logicalDpiX()
        self._mheight = self.height() * .0254 / self.logicalDpiY()
        self._keepRatio = False
        self._bgColor = QtCore.Qt.white
        os.environ["GKS_WSTYPE"] = "381" # GKS Qt Plugin
        os.environ["GKS_DOUBLE_BUF"] = "True"

    def paintEvent(self, event):
        self._painter = QtGui.QPainter()
        self._painter.begin(self)
        self._painter.fillRect(0, 0, self.width(), self.height(), self._bgColor)
        os.environ["GKSconid"] = getGKSConnectionId(self, self._painter)
        self.draw(self._clear, self._update)
        gr.updatews()
        self._painter.end()

    def resizeEvent(self, event):
        self._dwidth, self._dheight = self.width(), self.height()
        self._mwidth = self.width() * .0254 / self.logicalDpiX()
        self._mheight = self.height() * .0254 / self.logicalDpiY()
        if self._mwidth > self._mheight:
            self._sizex = 1.
            if self.keepRatio:
                self._sizey = 1.
                self._mwidth = self._mheight
                self._dwidth = self._dheight
            else:
                self._sizey = self._mheight / self._mwidth
        else:
            if self.keepRatio:
                self._sizex = 1.
                self._mheight = self._mwidth
                self._dheight = self._dwidth
            else:
                self._sizex = self._mwidth / self._mheight
            self._sizey = 1.

    def setBackground(self, qcolor):
        self._bgColor = qcolor

    @property
    def mwidth(self):
        """Get metric width of the widget excluding any window frame."""
        return self._mwidth

    @property
    def mheight(self):
        """Get metric height of the widget excluding any window frame."""
        return self._mheight

    @property
    def dwidth(self):
        """Get device width in consideration of ratio (keepRatio)."""
        return self._dwidth

    @property
    def dheight(self):
        """Get device height in consideration of ratio (keepRatio)."""
        return self._dheight

    @property
    def sizex(self):
        """..."""
        return self._sizex

    @property
    def sizey(self):
        """..."""
        return self._sizey

    @property
    def keepRatio(self):
        return self._keepRatio

    @keepRatio.setter
    def keepRatio(self, bool):
        self._keepRatio = bool
        self.resizeEvent(None)
        self.update()

    def _draw(self, clear=False, update=True):
        self._clear, self._update = clear, update

    def draw(self, clear=False, update=True):
        # put gr commands in here
        pass

    def save(self, path):
        (p, ext) = os.path.splitext(path)
        if ext.lower()[1:] == gr.GRAPHIC_GRX:
            gr.begingraphics(path)
            self.draw()
            gr.endgraphics()
        else:
            gr.beginprint(path)
            self.draw()
            gr.endprint()
        self.repaint()

    def printDialog(self, documentName="qtgr-untitled"):
        printer = QtGui.QPrinter(QtGui.QPrinter.HighResolution)
        printer.setDocName(documentName)
        painter = QtGui.QPainter()
        dlg = QtGui.QPrintDialog(printer)
        if dlg.exec_() == QtGui.QPrintDialog.Accepted:
            painter.begin(printer)
            os.environ["GKSconid"] = getGKSConnectionId(self, painter)

            # upscaling to paper size and
            # alignment (horizontal and vertical centering)
            xscale = printer.pageRect().width() / float(self.width())
            yscale = printer.pageRect().height() / float(self.height())
            scale = min(xscale, yscale)
            painter.translate(printer.paperRect().x() +
                              printer.pageRect().width() / 2,
                              printer.paperRect().y() +
                              printer.pageRect().height() / 2)
            painter.scale(scale, scale)
            painter.translate(-self.width() / 2, -self.height() / 2)

            self.draw(True)
            gr.updatews()
            painter.end()

    def __del__(self):
        if gr:
            gr.emergencyclosegks()
        # super destructor not available
#        super(GRWidget, self).__del__()


class InteractiveGRWidget(GRWidget):

    logXinDomain = QtCore.Signal(bool)
    logYinDomain = QtCore.Signal(bool)
    modePick = QtCore.Signal(bool)

    def __init__(self, *args, **kwargs):
        super(InteractiveGRWidget, self).__init__(*args, **kwargs)
        guiConn = GUIConnector(self)
        guiConn.connect(qtgr.events.MouseEvent.MOUSE_MOVE, self.mouseMove)
        guiConn.connect(qtgr.events.MouseEvent.MOUSE_PRESS, self.mousePress)
        guiConn.connect(qtgr.events.MouseEvent.MOUSE_RELEASE, self.mouseRelease)
        guiConn.connect(qtgr.events.WheelEvent.WHEEL_MOVE, self.wheelMove)
        guiConn.connect(qtgr.events.PickEvent.PICK_MOVE, self.pickMove)
        self.setMouseTracking(True)
        self._mouseLeft = False
        self._mouseRight = False
        self._startPoint = None
        self._curPoint = None
        self._logXinDomain = None
        self._logYinDomain = None
        self._pickMode = False
        self._pickEvent = None
        self._selectEnabled, self._panEnabled = True, True
        self._zoomEnabled = True
        self._lstPlot = []

    def draw(self, clear=False, update=True):
        if clear:
            gr.clearws()
        gr.setwsviewport(0, self.mwidth, 0, self.mheight)
        gr.setwswindow(0, self.sizex, 0, self.sizey)

        for plot in self._lstPlot:
            plot.sizex, plot.sizey = self.sizex, self.sizey
            plot.drawGR()
            # logDomainCheck
            logXinDomain = plot.logXinDomain()
            logYinDomain = plot.logYinDomain()
            if logXinDomain != self._logXinDomain:
                self._logXinDomain = logXinDomain
                self.logXinDomain.emit(self._logXinDomain)
            if logYinDomain != self._logYinDomain:
                self._logYinDomain = logYinDomain
                self.logYinDomain.emit(self._logYinDomain)

        if self._pickEvent:
            event = self._pickEvent
            gr.setviewport(*event.viewport)
            wcPoint = event.getWC(event.viewport)
            window = gr.inqwindow()
            gr.setwindow(*event.getWindow())
            gr.setmarkertype(gr.MARKERTYPE_PLUS)
            gr.polymarker([wcPoint.x], [wcPoint.y])
            gr.setwindow(*window)

    def addPlot(self, *args, **kwargs):
        for plot in args:
            if plot and plot not in self._lstPlot:
                self._lstPlot.append(plot)
        self._draw(clear=True, update=True)
        return self._lstPlot

    def plot(self, *args, **kwargs):
        plot = Plot()
        axes = PlotAxes(plot.viewport)
        axes.plot(*args, **kwargs)
        plot.addAxes(axes)
        return self.addPlot(plot)

    def adjustSelectRect(self, p0, p1):
        # can be used to restrict select rectangle, e.g. to given aspect ratio
        return p0, p1

    def paintEvent(self, event):
        super(InteractiveGRWidget, self).paintEvent(event)
        self._painter.begin(self)
        if self._mouseLeft and self.getMouseSelectionEnabled():
            p0 = self._startPoint.getNDC()
            p1 = self._curPoint.getNDC()
            p0, p1 = self.adjustSelectRect(p0, p1)
            coords = DeviceCoordConverter(self.dwidth, self.dheight)
            coords.setNDC(p0.x, p0.y)
            p0dc = coords.getDC()
            coords.setNDC(p1.x, p1.y)
            p1dc = coords.getDC()
            if self._getPlotsForPoint(p0):
                rect = QtCore.QRect(QtCore.QPoint(p0dc.x, p0dc.y),
                                    QtCore.QPoint(p1dc.x, p1dc.y)).normalized()
                self._painter.setOpacity(.75)
                self._painter.drawRect(rect)
                self._painter.setOpacity(1.)

        self._painter.end()

    def setAutoScale(self, mask):
        for plot in self._lstPlot:
            plot.autoscale = mask

    def getPickMode(self):
        return self._pickMode

    def setPickMode(self, bool):
        self._pickMode = bool
        self.modePick.emit(self._pickMode)

    def getMouseSelectionEnabled(self):
        return self._selectEnabled

    def setMouseSelectionEnabled(self, flag):
        self._selectEnabled = flag

    def getMousePanEnabled(self):
        return self._panEnabled

    def setMousePanEnabled(self, flag):
        self._panEnabled = flag

    def getMouseZoomEnabled(self):
        return self._zoomEnabled

    def setMouseZoomEnabled(self, flag):
        self._zoomEnabled = flag

    def _getPlotsForPoint(self, p0):
        res = []
        for plot in self._lstPlot:
            xmin, xmax, ymin, ymax = plot.viewportscaled
            if p0.x >= xmin and p0.x <= xmax and p0.y >= ymin and p0.y <= ymax:
                res.append(plot)
        return res

    def _pick(self, p0, type):
        for plot in self._getPlotsForPoint(p0):
            (coord, _axes, _curve) = plot.pick(p0, self.dwidth, self.dheight)
            if coord:
                dcPoint = coord.getDC()
                QtGui.QApplication.sendEvent(self, PickEvent(type,
                                                             self.dwidth,
                                                             self.dheight,
                                                             dcPoint.x,
                                                             dcPoint.y,
                                                             plot.viewport,
                                                             coord.getWindow()))

    def _select(self, p0, p1):
        self._pickEvent = None
        change = False
        for plot in self._getPlotsForPoint(p0):
            plot.select(p0, p1, self.dwidth, self.dheight)
            change = True
        if change:
            self._draw(True)
            self.update()

    def _pan(self, p0, dp):
        self._pickEvent = None
        change = False
        for plot in self._getPlotsForPoint(p0):
            plot.pan(dp, self.dwidth, self.dheight)
            change = True
        if change:
            self._draw(True)
            self.update()

    def _zoom(self, dpercent, p0):
        self._pickEvent = None
        change = False
        for plot in self._getPlotsForPoint(p0):
            plot.zoom(dpercent, p0, self.dwidth, self.dheight)
            change = True
        if change:
            self._draw(True)
            self.update()

    def _roi(self, p0, type, buttons, modifiers):
        for plot in self._lstPlot:
            roi = plot.getROI(p0)
            if roi:
                if roi.regionType == RegionOfInterest.LEGEND:
                    eventObj = LegendEvent
                else:
                    eventObj = ROIEvent
                coords = DeviceCoordConverter(self.dwidth, self.dheight)
                coords.setNDC(p0.x, p0.y)
                p0dc = coords.getDC()
                QtGui.QApplication.sendEvent(self,
                                             eventObj(type,
                                                      self.dwidth,
                                                      self.dheight,
                                                      p0dc.x, p0dc.y,
                                                      buttons, modifiers,
                                                      roi))

    def mousePress(self, event):
        if event.getButtons() & MouseEvent.LEFT_BUTTON:
            if self.getPickMode():
                self.setPickMode(False)
                self._pick(event.getNDC(), PickEvent.PICK_PRESS)
            else:
                if event.getModifiers() & MouseEvent.CONTROL_MODIFIER:
                    self.setPickMode(True)
                else:
                    self._mouseLeft = True
        elif event.getButtons() & MouseEvent.RIGHT_BUTTON:
            self._mouseRight = True
        self._curPoint = event
        self._startPoint = event

    def mouseRelease(self, event):
        if event.getButtons() & MouseEvent.LEFT_BUTTON and self._mouseLeft:
            self._mouseLeft = False
            self._curPoint = event
            p0 = self._startPoint.getNDC()
            p1 = self._curPoint.getNDC()
            if p0 != p1:
                if (self.getMouseSelectionEnabled()
                    and self._getPlotsForPoint(p0)):
                    p0, p1 = self.adjustSelectRect(p0, p1)
                    self._select(p0, p1)
            else:
                self._roi(p0, ROIEvent.ROI_CLICKED, event.getButtons(),
                          event.getModifiers())
        elif event.getButtons() & MouseEvent.RIGHT_BUTTON and self._mouseRight:
            self._mouseRight = False
            self._roi(event.getNDC(), ROIEvent.ROI_CLICKED, event.getButtons(),
                      event.getModifiers())
        self._curPoint = event

    def mouseMove(self, event):
        if self.getPickMode():
            self._pick(event.getNDC(), PickEvent.PICK_MOVE)
        if event.getButtons() & MouseEvent.LEFT_BUTTON:
            self._curPoint = event
            super(InteractiveGRWidget, self).update()
        elif event.getButtons() & MouseEvent.RIGHT_BUTTON:
            p0 = self._curPoint.getNDC() # point before now
            p1 = event.getNDC()
            dp = p1 - p0
            self._curPoint = event
            if self.getMousePanEnabled():
                self._pan(self._startPoint.getNDC(), dp)
                self._mouseRight = False
        self._roi(event.getNDC(), ROIEvent.ROI_OVER, event.getButtons(),
                  event.getModifiers())

    def wheelMove(self, event):
        if self.getMouseZoomEnabled():
            # delta percent
            dpercent = event.getSteps() * .1
            self._zoom(dpercent, event.getNDC())

    def pickMove(self, event):
        self._pickEvent = event
        self.update()


if __name__ == "__main__":
    import sys
    from gr import pygr
    logging.basicConfig(level=logging.CRITICAL)
    for name in [__name__, pygr.base.__name__, pygr.__name__]:
        logging.getLogger(name).setLevel(logging.DEBUG)
    app = QtGui.QApplication(sys.argv)
    grw = InteractiveGRWidget()
    grw.resize(QtCore.QSize(500, 500))
    viewport = [0.1, 0.45, 0.1, 0.88]
    vp2 = [.6, .95, .1, .88]

    x = [-3.3 + t * .1 for t in range(66)]
    y = [t ** 5 - 13 * t ** 3 + 36 * t for t in x]

    n = 100
    pi2_n = 2.*math.pi / n
    x2 = [i * pi2_n for i in range(0, n + 1)]
    y2 = map(lambda xi: math.sin(xi), x2)

    plot = Plot(viewport).addAxes(PlotAxes(viewport).plot(x, y),
                                  PlotAxes(viewport).plot(x2, y2))
    plot.title, plot.subTitle = "foo", "bar"
    plot.xlabel, plot.ylabel = "x", "f(x)"

    plot2 = Plot(vp2).addAxes(PlotAxes(vp2).plot(x2, y2))
    plot2.title, plot2.subTitle = "Title", "Subtitle"
    plot2.xlabel = "x"

    grw.addPlot(plot)
    grw.addPlot(plot2)
    grw.show()
    grw.update()

    sys.exit(app.exec_())
