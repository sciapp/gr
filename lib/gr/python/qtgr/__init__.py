#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
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
import pygr
import qtgr.events
from qtgr.events import GUIConnector, MouseEvent, PickEvent
from qtgr.events import CoordConverter, Point
import qtgr.base

__author__  = "Christian Felder <c.felder@fz-juelich.de>"
__date__    = "2013-04-19"
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

class Helper(object):
    
    _ZERO = 1e-8
    _EPSILON = 1e-8
    
    @staticmethod
    def isInLogDomain(*args):
        res = True
        for value in args:
            if value <= Helper._ZERO:
                res = False
                break
        return res
    
    @staticmethod
    def isInWindowDomain(xmin, xmax, ymin, ymax):
        res = True
        if (math.isnan(xmin) or math.isinf(xmin) or math.isnan(xmax)
            or math.isinf(xmax) or math.isnan(ymin) or math.isinf(ymin) or
            math.isnan(ymax) or math.isinf(ymax) or
            xmin > max or ymin > ymax or abs(xmax-xmin) < Helper._EPSILON or
            abs(ymax-ymin) < Helper._EPSILON):
            res = False
#        print "isInWindowDomain( %s, %s, %s, %s )" %(xmin, xmax, ymin, ymax)
#        print "  %s %s" %(abs(xmax-xmin), abs(xmax-xmin) < Helper._EPSILON)
#        print "  %s %s" %(abs(ymax-ymin), abs(xmax-xmin) < Helper._EPSILON)
#        print "  %s" %res
        return res

class ErrorBar(qtgr.base.GRMeta):
    
    HORIZONTAL = 0
    VERTICAL = 1
    
    def __init__(self, x, y, dneg, dpos, direction=VERTICAL):
        self._grerror = None
        self._direction = direction
        self._x = x
        self._y = y
        self._n = len(self._x)
        self._dneg = dneg
        self._dpos = dpos
        if direction == ErrorBar.VERTICAL:
            self._grerror = gr.verrorbars
        elif direction == ErrorBar.HORIZONTAL:
            self._grerror = gr.herrorbars
        else:
            raise AttributeError("unsupported value for direction.")
        
    def drawGR(self):
        self._grerror(self._n, self._x, self._y, self._dneg, self._dpos)

class PlotCurve(qtgr.base.GRMeta):
    
    def __init__(self, x, y, errBar1=None, errBar2=None,
                 linetype=gr.LINETYPE_SOLID, markertype=gr.MARKERTYPE_DOT):
        self._x, self._y = x, y
        self._e1, self._e2 = errBar1, errBar2
        self._linetype, self._markertype = linetype, markertype
        self._n = len(self._x)
        
    def setLineType(self, linetype):
        self._linetype = linetype
        
    def getLineType(self):
        return self._linetype
        
    def setMarkerType(self, markertype):
        self._markertype = markertype
        
    def getMarkerType(self):
        return self._markertype

    @property
    def x(self):
        """Get the current list/ndarray of x values."""
        return self._x
    
    @x.setter
    def x(self, lst):
        self._x = lst
    
    @property
    def y(self):
        """Get the current list/ndarray of y values."""
        return self._y
    
    @y.setter
    def y(self, lst):
        self._y = lst
    
    def drawGR(self):
        if self.getLineType() is not None:
            gr.setlinetype(self._linetype)
            gr.polyline(self._n, self.x, self.y)
            if (self.getMarkerType() != gr.MARKERTYPE_DOT and
                self.getMarkerType() is not None):
                gr.setmarkertype(self._markertype)
                gr.polymarker(self._n, self.x, self.y)
        elif self.getMarkerType() is not None:
            gr.setmarkertype(self._markertype)
            gr.polymarker(self._n, self.x, self.y)
        if self._e1:
            self._e1.drawGR()
        if self._e2:
            self._e2.drawGR()
            
class PlotAxes(qtgr.base.GRMeta):

    COUNT = 0

    def __init__(self, viewport=[0.1, 0.95, 0.1, 0.95], drawX=True, drawY=True):
        self._viewport, self._drawX, self._drawY = viewport, drawX, drawY
        self._lstPlotCurve = None
        self._backgroundColor = 163
        self._window = None
        self._scale = 0
        self._grid = True
        self._resetWindow = True
        PlotAxes.COUNT += 1
        self._id = PlotAxes.COUNT
        
    def getCurves(self):
        return self._lstPlotCurve
    
    def isXLogDomain(self):
        window = self.getWindow()
        return Helper.isInLogDomain(window[0], window[1])
    
    def isYLogDomain(self):
        window = self.getWindow()
        return Helper.isInLogDomain(window[2], window[3])
    
    @property
    def viewport(self):
        """get current viewport"""
        return self._viewport
    
    @viewport.setter
    def viewport(self, viewport):
        self._viewport = viewport
    
    @property
    def scale(self):
        return self._scale
    
    @scale.setter
    def scale(self, options):
        self._scale = options
        
    @property
    def backgroundColor(self):
        return self._backgroundColor
    
    @backgroundColor.setter
    def backgroundColor(self, color):
        self._backgroundColor = color
        
    def isXDrawingEnabled(self):
        return self._drawX
        
    def setXDrawing(self, bool):
        self._drawX = bool
        
    def isYDrawingEnabled(self):
        return self._drawY
        
    def setYDrawing(self, bool):
        self._drawY = bool
        
    def setLogX(self, bool):
        if bool:
            self.scale |= gr.OPTION_X_LOG
        else:
            self.scale &= ~gr.OPTION_X_LOG
        
    def setLogY(self, bool):
        if bool:
            self.scale |= gr.OPTION_Y_LOG
        else:
            self.scale &= ~gr.OPTION_Y_LOG
        
    def setGrid(self, bool):
        self._grid = bool
        
    def isGridEnabled(self):
        return self._grid
    
    def setWindow(self, xmin, xmax, ymin, ymax):
        self._window = [xmin, xmax, ymin, ymax]
    
    def getWindow(self):
        return list(self._window)
    
    def reset(self):
        self._resetWindow = True
        
    def isReset(self):
        return self._resetWindow
    
    def getId(self):
        return self._id
    
    def drawGR(self):
        lstPlotCurve = self.getCurves()
        viewport = self.viewport
        if lstPlotCurve:
            if self.isReset():
                self._resetWindow = False
                # global xmin, xmax, ymin, ymax
                xmin = min(map(lambda curve: min(curve.x),
                               lstPlotCurve))
                xmax = max(map(lambda curve: max(curve.x),
                               lstPlotCurve))
                ymin = min(map(lambda curve: min(curve.y),
                               lstPlotCurve))
                ymax = max(map(lambda curve: max(curve.y),
                               lstPlotCurve))
                if self.scale & gr.OPTION_X_LOG == 0:
                    xmin, xmax = gr.adjustrange(xmin, xmax)
                if self.scale & gr.OPTION_Y_LOG == 0:
                    ymin, ymax = gr.adjustrange(ymin, ymax)
                window = [xmin, xmax, ymin, ymax]
                self.setWindow(*window)
            else:
                window = self.getWindow()
                xmin, xmax, ymin, ymax = window
                
            if (window[0] > xmin or xmin > window[1] or window[2] > ymin or
                ymin > window[3]):
                #GKS: Rectangle definition is invalid in routine SET_WINDOW
                #origin outside current window
                self.reset()
                self.drawGR()
            else:
                if self.scale & gr.OPTION_X_LOG:
                    xtick = majorx = 1
                else:
                    majorx = 5
                    xtick = gr.tick(xmin, xmax) / majorx
                if self.scale & gr.OPTION_Y_LOG:
                    ytick = majory = 1
                else:
                    majory = 5
                    ytick = gr.tick(ymin, ymax) / majory
                gr.setviewport(*viewport)
                gr.setwindow(*window)
                gr.setscale(self.scale)
                if self.backgroundColor and self.getId() == 1:
                    gr.setfillintstyle(1)
                    gr.setfillcolorind(self.backgroundColor)
                    gr.fillrect(*window)
                charHeight = .024 * (viewport[3] - viewport[2])
                gr.setcharheight(charHeight)
                if self.getId() == 1:
                    # first x, y axis
                    if not self.isXDrawingEnabled():
                        majorx = -majorx
                    if not self.isYDrawingEnabled():
                        majory = -majory
                    if self.isGridEnabled():
                        gr.grid(xtick, ytick, xmax, ymax, majorx, majory)
                    gr.axes(xtick, ytick, xmin, ymin,  majorx,  majory,  0.01)
                elif self.getId() == 2:
                    # second x, y axis
                    if not self.isXDrawingEnabled():
                        majorx = -majorx
                    if not self.isYDrawingEnabled():
                        majory = -majory
                    gr.axes(xtick, ytick, xmax, ymax, majorx, majory, -0.01)
                for curve in lstPlotCurve:
                    curve.drawGR()
    
    def plot(self, *args, **kwargs):
        if len(args) > 0 and len(args)%2 == 0:
            self._lstPlotCurve = []
            for i in range(0, len(args)/2):
                x = args[2*i]
                y = args[2*i + 1]
                self._lstPlotCurve.append(PlotCurve(x, y))
        return self
    
    def addCurves(self, *args, **kwargs):
        for plotCurve in args:
            if plotCurve and plotCurve not in self._lstPlotCurve:
                self._lstPlotCurve.append(plotCurve)
            
class GRWidget(QtGui.QWidget):
    
    def __init__(self, *args, **kwargs):
        super(GRWidget, self).__init__(*args, **kwargs)
        os.environ["GKS_WSTYPE"] = "381" # GKS Qt Plugin
        os.environ["GKS_DOUBLE_BUF"] = "True"
        self.setPalette(QtGui.QPalette(QtGui.QColor.fromRgb(0xffffff)))
        self.setAttribute(QtCore.Qt.WA_PaintOnScreen)
    
    def paintEvent(self, event):
        self._painter = QtGui.QPainter()
        self._painter.begin(self)
        os.environ["GKSconid"] = "%x!%x" %(sip.unwrapinstance(self),
                                            sip.unwrapinstance(self._painter))
        gr.updatews()
        self._painter.end()
        
    # put gr commands in here
    def draw(self, clear=False, update=True):
        pass
        
    def save(self, path):
        (p, ext) = os.path.splitext(path)
        if ext.lower()[1:] == gr.GRAPHIC_GRX:
            gr.begingraphics(path)
            self.draw(update=False)
            gr.endgraphics()
        else:
            gr.beginprint(path)
            self.draw(update=False)
            gr.endprint()
            
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
        self._plotTitle = None
        self._plotSubTitle = None
        self._lblX = None
        self._lblY = None
        self.viewport = [0.1, 0.95, 0.1, 0.95]
        self._lstAxes = []
        
    def _drawTitleAndSubTitle(self):
        title =  self.getTitle()
        subTitle = self.getSubTitle()
        if title or subTitle:
            gr.settextalign(gr.TEXT_HALIGN_CENTER, gr.TEXT_VALIGN_TOP)
            gr.setcharup(0., 1.)
            [xmin, xmax, ymin, ymax] = self.viewport
            x = xmin + (xmax-xmin)/2
            dy = .05
            if title and subTitle:
                dy = .1
            y = ymax + dy
            if y > 1.:
                y = ymax
                self.viewport = [xmin, xmax, ymin, ymax-dy]
                self.draw(True)
            if title:
                gr.text(x, y, title)
                y -= .05
            if subTitle:
                gr.text(x, y, subTitle)
            
    def _drawXYLabel(self):
        xlabel = self.getXLabel()
        ylabel = self.getYLabel()
        if xlabel:
            gr.settextalign(gr.TEXT_HALIGN_CENTER, gr.TEXT_VALIGN_TOP)
            gr.setcharup(0., 1.)
            gr.text(.5, 0.035, xlabel)
        if ylabel:
            gr.settextalign(gr.TEXT_HALIGN_CENTER, gr.TEXT_VALIGN_TOP)
            gr.setcharup(-1., 0.)
            gr.text(0., .5, ylabel)
            gr.setcharup(0., 1.)
            
    def _drawPlot(self, lstAxes):
        if lstAxes:
            for axes in lstAxes:
                axes.drawGR()
            # log x, log y domain check
            # this check has to be performed after plotting because of
            # possible window changes, e.g. on resetWindow.
            self._logDomainCheck()
        
    def draw(self, clear=False, update=True):
        if clear:
            gr.clearws()
            
        self._drawPlot(self._lstAxes)

        self._drawTitleAndSubTitle()
        self._drawXYLabel()
        if update:
            self.update()
            
    def _logDomainCheck(self):
        # log x, log y domain check
        # emit signals on change
        logXinDomain = True
        logYinDomain = True
        for axes in self._lstAxes:
            logXinDomain = (logXinDomain & axes.isXLogDomain())
            logYinDomain = (logYinDomain & axes.isYLogDomain())
        if logXinDomain != self._logXinDomain:
            self._logXinDomain = logXinDomain
            self.emit(QtCore.SIGNAL("logXinDomain(bool)"), self._logXinDomain)
        if logYinDomain != self._logYinDomain:
            self._logYinDomain = logYinDomain
            self.emit(QtCore.SIGNAL("logYinDomain(bool)"), self._logYinDomain)
            
    def plot(self, *args, **kwargs):
        axes = PlotAxes(self.viewport)
        axes.plot(*args, **kwargs)
        self.addAxes(axes)
        self.draw(clear=True, update=True)
        
    def addAxes(self, *args, **kwargs):
        for plotAxes in args:
            if plotAxes and plotAxes not in self._lstAxes:
                self._lstAxes.append(plotAxes)
        self.draw(clear=True)

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
        
    def getTitle(self):
        return self._plotTitle
    
    def setTitle(self, title):
        self._plotTitle = title
        self.draw(True)
        self.update()
                
    def getSubTitle(self):
        return self._plotSubTitle
    
    def setSubTitle(self, subtitle):
        self._plotSubTitle = subtitle
        self.draw(True)
        self.update()
    
    def getXLabel(self):
        return self._lblX
    
    def setXLabel(self, xlabel):
        self._lblX = xlabel
        self.draw(True)
        self.update()
        
    def getYLabel(self):
        return self._lblY
    
    def setYLabel(self, ylabel):
        self._lblY = ylabel
        self.draw(True)
        self.update()
    
    @property
    def viewport(self):
        """get current viewport"""
        return self._viewport
    
    @viewport.setter
    def viewport(self, viewport):
        self._viewport = viewport
    
    def setviewport(self, xmin, xmax, ymin, ymax):
        self.viewport = [xmin, xmax, ymin, ymax]
        
    def getPickMode(self):
        return self._pickMode
    
    def setPickMode(self, bool):
        self._pickMode = bool
        self.emit(QtCore.SIGNAL("modePick(bool)"), self._pickMode)
        
    def setLogX(self, bool):
        if bool:
            for axes in self._lstAxes:
                if axes.isXLogDomain():
                    axes.setLogX(bool)
                else:
                    win = axes.getWindow()
                    raise Exception("AXES[%d]: (%d..%d) not in log(x) domain."
                                    %(axes.getId(), win[0], win[1]))
        else:
            for axes in self._lstAxes:
                axes.setLogX(bool)
        self.draw(clear=True)
            
    def setLogY(self, bool):
        if bool:
            for axes in self._lstAxes:
                if axes.isYLogDomain():
                    axes.setLogY(bool)
                else:
                    win = axes.getWindow()
                    raise Exception("AXES[%d]: (%d..%d) not in log(y) domain."
                                    %(axes.getId(), win[2], win[3]))
        else:
            for axes in self._lstAxes:
                axes.setLogY(bool)
        self.draw(clear=True)
        
    def setGrid(self, bool):
        for axes in self._lstAxes:
            axes.setGrid(bool)
        self.draw(clear=True)
        
    def reset(self):
        for axes in self._lstAxes:
            axes.reset()
        self.draw(clear=True)
        
    def _pick(self, p0, type):
        window = gr.inqwindow()
        if self._lstAxes:
            coord = CoordConverter(self.width(), self.height())
            points = []
            lstAxes = []
            for axes in self._lstAxes:
                gr.setwindow(*axes.getWindow())
                coord.setNDC(p0.x, p0.y)
                wcPick = coord.getWC()
                for curve in axes.getCurves():
                    for idx, x in enumerate(curve.x):
                        if x >= wcPick.x:
                            break
                    coord.setWC(x, curve.y[idx])
                    points.append(coord.getNDC())
                    lstAxes.append(axes) 
            # calculate distance between p0 and point on curve
            norms = map(lambda p: (p0-p).norm(), points)
            # nearest point
            idx = norms.index(min(norms))
            p = points[idx]
            axes = lstAxes[idx]
            coord.setNDC(p.x, p.y)
            dcPoint = coord.getDC()
            QtGui.QApplication.sendEvent(self, PickEvent(type,
                                                         self.width(),
                                                         self.height(),
                                                         dcPoint.x,
                                                         dcPoint.y,
                                                         axes.getWindow()))
        gr.setwindow(*window)
        
    def _select(self, p0, p1):
        window = gr.inqwindow()
        coord = CoordConverter(self.width(), self.height())
        gr.clearws()
        for axes in self._lstAxes:
            win = axes.getWindow()
            gr.setwindow(*win)
            p0World = coord.setNDC(p0.x, p0.y).getWC()
            p1World = coord.setNDC(p1.x, p1.y).getWC()
            xmin = min(p0World.x, p1World.x)
            xmax = max(p0World.x, p1World.x)
            ymin = min(p0World.y, p1World.y)
            ymax = max(p0World.y, p1World.y)
            axes.setWindow(xmin, xmax, ymin, ymax)
        gr.setwindow(*window)
        self.draw()
        
    def _pan(self, dp):
        window = gr.inqwindow()
        coord = CoordConverter(self.width(), self.height())
        gr.clearws()
        for axes in self._lstAxes:
            win = axes.getWindow()
            gr.setwindow(*win)
            gr.setscale(0)
            coord.setWC(0, 0)
            ndcOrigin = coord.getNDC()
            coord.setNDC(ndcOrigin.x + dp.x, ndcOrigin.y + dp.y)
            dpWorld = coord.getWC()
            win[0] -= dpWorld.x
            win[1] -= dpWorld.x
            win[2] -= dpWorld.y
            win[3] -= dpWorld.y
            gr.setscale(axes.scale)
            axes.setWindow(*win)
        gr.setwindow(*window)
        self.draw()
                
    def _zoom(self, dpercent):
        window = gr.inqwindow()
        gr.clearws()
        for axes in self._lstAxes:
            win = axes.getWindow()
            winWidth_2 = (win[1]-win[0])/2
            winHeight_2 = (win[3]-win[2])/2
            dx_2 = winWidth_2 - (1+dpercent)*winWidth_2
            dy_2 = winHeight_2 - (1+dpercent)*winHeight_2
            win[0] -= dx_2
            win[1] += dx_2
            win[2] -= dy_2
            win[3] += dy_2
            if Helper.isInWindowDomain(*window):
                axes.setWindow(*win)
            else:
                axes.setWindow(*window)
                self.reset()
                break
        self.draw()
        
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
        elif event.getButtons() & MouseEvent.RIGHT_BUTTON:
            self._mouseRight = False
        self._curPoint = event
            
    def mouseMove(self, event):
        if self.getPickMode():
            self._pick(event.getNDC(), PickEvent.PICK_MOVE)
        if event.getButtons() & MouseEvent.LEFT_BUTTON:
            self._curPoint = event
            self.update()
        elif event.getButtons() & MouseEvent.RIGHT_BUTTON:
            p0 = self._curPoint.getNDC() # point before now
            p1 = event.getNDC()
            dp = p1-p0
            self._curPoint = event
            self._pan(dp)
            
    def wheelMove(self, event):
        # delta percent
        dpercent = event.getDegree()/16.
        self._zoom(dpercent)
        
    def pickMove(self, event):
        wcPoint = event.getWC()
        self.draw(True)
        window = gr.inqwindow()
        gr.setwindow(*event._window)
        gr.setmarkertype(gr.MARKERTYPE_PLUS)
        gr.polymarker(1, [wcPoint.x], [wcPoint.y])
        gr.setwindow(*window)
        self.update()
        
if __name__ == "__main__":
    import sys
    import pygr
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
    
    grw.addAxes(PlotAxes(grw.viewport).plot(x, y),
                PlotAxes(grw.viewport).plot(x2, y2))
#    grw.plot([0, 0], [1, 1], [2,2,2], [3,3,3])
    
#    pygr.plot(x, y, bgcolor=163, clear=False, update=False)
#    gr.clearws()
    sys.exit(app.exec_())
