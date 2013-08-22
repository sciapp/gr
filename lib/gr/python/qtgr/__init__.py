#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""PyQt GR module

Exported Classes:

"""
# standard library
import os
import math
# third party
from PyQt4 import QtCore
from PyQt4 import QtGui
import sip
# local library
import gr
import qtgr.events
from gr.pygr import Plot, PlotAxes, RegionOfInterest
from qtgr.events import GUIConnector, MouseEvent, PickEvent, ROIEvent,\
    LegendEvent, TickEvent

__author__  = "Christian Felder <c.felder@fz-juelich.de>"
__date__    = "2013-06-27"
__version__ = "0.2.0"
__copyright__ = """Copyright 2012, 2013 Forschungszentrum Juelich GmbH

This file is part of GR, a universal framework for visualization applications.
Visit https://iffwww.iff.kfa-juelich.de/portal/doku.php?id=gr for the latest
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

class GRWidget(QtGui.QWidget):
    
    def __init__(self, *args, **kwargs):
        super(GRWidget, self).__init__(*args, **kwargs)
        self._clear, self._update, self._checkTicks = False, False, False
        os.environ["GKS_WSTYPE"] = "381" # GKS Qt Plugin
        os.environ["GKS_DOUBLE_BUF"] = "True"
        self.setPalette(QtGui.QPalette(QtGui.QColor.fromRgb(0xffffff)))
        self.setAttribute(QtCore.Qt.WA_PaintOnScreen)
    
    def paintEvent(self, event):
        self._painter = QtGui.QPainter()
        self._painter.begin(self) 
        os.environ["GKSconid"] = "%x!%x" %(sip.unwrapinstance(self),
                                           sip.unwrapinstance(self._painter))
        self.draw(self._clear, self._update, self._checkTicks)
        gr.updatews()
        self._painter.end()
        
    def _draw(self, clear=False, update=True, checkTicks=True):
        self._clear, self._update, self._checkTicks = clear, update, checkTicks
        
    def draw(self, clear=False, update=True, checkTicks=True):
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
            os.environ["GKSconid"] = "%x!%x" %(sip.unwrapinstance(self),
                                               sip.unwrapinstance(painter))
        
            # upscaling to paper size and
            # alignment (horizontal and vertical centering)
            xscale = printer.pageRect().width()/float(self.width());
            yscale = printer.pageRect().height()/float(self.height());
            scale = min(xscale, yscale);
            painter.translate(printer.paperRect().x() +
                              printer.pageRect().width()/2,
                              printer.paperRect().y() +
                              printer.pageRect().height()/2)
            painter.scale(scale, scale);
            painter.translate(-self.width()/2, -self.height()/2);
        
            self.draw(True)
            gr.updatews()
            painter.end()
        
    def __del__(self):
        if gr:
            gr.emergencyclosegks()
        # super destructor not available
#        super(GRWidget, self).__del__()

class InteractiveGRWidget(GRWidget):
    
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
        self._lstPlot = []
        self._dictAxesTicks = {}
        
    def update(self, checkTicks=True):
##        self._draw(clear=True, update=True, checkTicks=checkTicks)
        self._checkTicks = checkTicks
        super(InteractiveGRWidget, self).update()
        
    def updateTicks(self):
        self.update(checkTicks=False)
    
    def draw(self, clear=False, update=True, checkTicks=True):
        if clear:
            gr.clearws()
            
        for plot in self._lstPlot:
            plot.drawGR()
            # logDomainCheck
            logXinDomain = plot.logXinDomain()
            logYinDomain = plot.logYinDomain()
            if logXinDomain != self._logXinDomain:
                self._logXinDomain = logXinDomain
                self.emit(QtCore.SIGNAL("logXinDomain(bool)"),
                          self._logXinDomain)
            if logYinDomain != self._logYinDomain:
                self._logYinDomain = logYinDomain
                self.emit(QtCore.SIGNAL("logYinDomain(bool)"),
                          self._logYinDomain)
            # axes tick changed check
            if checkTicks:
                for axes in plot.getAxes():
                    self._axesTickValues(axes)

        if self._pickEvent:
            event = self._pickEvent
            wcPoint = event.getWC()
            window = gr.inqwindow()
            gr.setwindow(*event.getWindow())
            gr.setmarkertype(gr.MARKERTYPE_PLUS)
            gr.polymarker(1, [wcPoint.x], [wcPoint.y])
            gr.setwindow(*window)

    def addPlot(self, *args, **kwargs):
        for plot in args:
            if plot and plot not in self._lstPlot:
                self._lstPlot.append(plot)
        self._draw(clear=True, update=True)
        return self._lstPlot
        
    def plot(self, *args, **kwargs):
        plot = Plot()
        axes = PlotAxes(plot.viewport())
        axes.plot(*args, **kwargs)
        plot.addAxes(axes)
        return self.addPlot(plot)
    
    def paintEvent(self, event):
        super(InteractiveGRWidget, self).paintEvent(event)
        self._painter.begin(self)
        if self._mouseLeft:
            startDC = self._startPoint.getDC()
            endDC = self._curPoint.getDC()
            rect = QtCore.QRect(QtCore.QPoint(startDC.x, startDC.y),
                                QtCore.QPoint(endDC.x, endDC.y)).normalized()
            self._painter.setOpacity(.75)
            self._painter.drawRect(rect)
            self._painter.setOpacity(1.)

        self._painter.end()
        
    def getPickMode(self):
        return self._pickMode
    
    def setPickMode(self, bool):
        self._pickMode = bool
        self.emit(QtCore.SIGNAL("modePick(bool)"), self._pickMode)
        
    def _axesTickValues(self, axes):
        oldX = None
        oldY = None
        xtickValue = axes.getXtickValues()
        ytickValue = axes.getYtickValues()
        if axes in self._dictAxesTicks:
            oldX = self._dictAxesTicks[axes]['x']
            oldY = self._dictAxesTicks[axes]['y']
        if xtickValue and xtickValue != oldX:
            QtGui.QApplication.sendEvent(self,
                                         TickEvent(TickEvent.TICKS_CHANGED,
                                                   TickEvent.AXIS_X,
                                                   xtickValue, axes))
        if ytickValue and ytickValue != oldY:
            QtGui.QApplication.sendEvent(self,
                                         TickEvent(TickEvent.TICKS_CHANGED,
                                                   TickEvent.AXIS_Y,
                                                   ytickValue, axes))
        self._dictAxesTicks[axes] = { 'x': xtickValue, 'y': ytickValue }
        
    def _pick(self, p0, type):
        for plot in self._lstPlot:
            coord = plot.pick(p0, self.width(), self.height())
            if coord:
                dcPoint = coord.getDC()
                QtGui.QApplication.sendEvent(self, PickEvent(type,
                                                             self.width(),
                                                             self.height(),
                                                             dcPoint.x,
                                                             dcPoint.y,
                                                             coord.getWindow()))
        
    def _select(self, p0, p1):
        self._pickEvent = None
        for plot in self._lstPlot:
            plot.select(p0, p1, self.width(), self.height())
        self._draw(True)
        self.update()
        
    def _pan(self, dp):
        self._pickEvent = None
        for plot in self._lstPlot:
            plot.pan(dp, self.width(), self.height())
        self._draw(True)
        self.update()
                
    def _zoom(self, dpercent):
        self._pickEvent = None
        for plot in self._lstPlot:
            plot.zoom(dpercent)
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
                QtGui.QApplication.sendEvent(self,
                                             eventObj(type,
                                                      self.width(),
                                                      self.height(),
                                                      p0.x, p0.y,
                                                      buttons, modifiers,
                                                      roi))
        
    def mousePress(self, event):
        if event.getButtons() & MouseEvent.LEFT_BUTTON:
            if self.getPickMode():
                self._pick(event.getNDC(), PickEvent.PICK_PRESS)
                self.setPickMode(False)
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
                self._select(p0, p1)
            else:
                self._roi(p0, ROIEvent.ROI_CLICKED, event.getButtons(),
                          event.getModifiers())
        elif event.getButtons() & MouseEvent.RIGHT_BUTTON:
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
            dp = p1-p0
            self._curPoint = event
            self._pan(dp)
        self._roi(event.getNDC(), ROIEvent.ROI_OVER, event.getButtons(),
                  event.getModifiers())
            
    def wheelMove(self, event):
        # delta percent
        dpercent = event.getDegree()/16.
        self._zoom(dpercent)
        
    def pickMove(self, event):
        self._pickEvent = event
        self.update()
        
if __name__ == "__main__":
    import sys
    app = QtGui.QApplication(sys.argv)
    grw = InteractiveGRWidget()
    grw.viewport = [0.1, 0.95, 0.1, 0.9]
    grw.show()
    x = [-3.3 + t*.1 for t in range(66)]
    y = [t**5 - 13*t**3 + 36*t for t in x]
    
    n = 100
    pi2_n = 2.*math.pi/n
    x2 = [i * pi2_n for i in range(0, n+1)]
    y2 = map(lambda xi: math.sin(xi), x2)
    
    grw.addPlot(Plot().addAxes(PlotAxes().plot(x, y),
                               PlotAxes().plot(x2, y2)))
    grw.update()
    
    sys.exit(app.exec_())
