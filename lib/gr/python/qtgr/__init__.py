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
import gr
import pygr
# local library
import qtgr.events
from qtgr.events import GUIConnector, MouseEvent, PickEvent, CoordConverter

__author__  = "Christian Felder <c.felder@fz-juelich.de>"
__date__    = "2013-04-10"
__version__ = "0.1.0"
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
    
    # jpeg, tif
    GR_PRINT_TYPES = { "ps" : "PostScript (*.ps)",
                      "eps" : "Encapsulated PostScript (*.eps)",
                      "pdf" : "Portable Document Format (*.pdf)",
                      "bmp" : "Windows Bitmap (*.bmp)",
                      "jpg" : "JPEG (*.jpg)",
                      "png" : "Portable Network Graphics (*.png)",
                      "tiff" : "Tagged Image File Format (*.tiff)",
                      "fig" : "Figure (*.fig)",
                      "svg" : "Scalable Vector Graphics (*.svg)",
                      "wmf" : "Windows Metafile (*.wmf)"
    }
    
    GR_GRAPHICS_TYPES = { "grx" : "Graphics Format (*.grx)"}
    
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
        
    def __del__(self):
        if gr:
            gr.emergencyclosegks()
        # super destructor not available
#        super(GRWidget, self).__del__()

class InteractiveGRWidget(GRWidget):
    
    GRX_SUFFIX = ".grx"
    
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
        self._option_scale = 0
        self._grid = True
        self._bgColor = 163
        self._x = None
        self._y = None
        self._resetWindow = True
        self._pickMode = False
        self._plotTitle = None
        self._plotSubTitle = None
        self._lblX = None
        self._lblY = None
#        self._printPath = None
        self.setviewport(0.1, 0.95, 0.1, 0.95)
        
    def _drawTitleAndSubTitle(self):
        title =  self.getTitle()
        subTitle = self.getSubTitle()
        if title or subTitle:
            gr.settextalign(gr.TEXT_HALIGN_CENTER, gr.TEXT_VALIGN_TOP)
            gr.setcharup(0., 1.)
            [xmin, xmax, ymin, ymax] = self.getviewport()
            x = xmin + (xmax-xmin)/2
            dy = .05
            if title and subTitle:
                dy = .1
            y = ymax + dy
            if y > 1.:
                y = ymax
                self.setviewport(xmin, xmax, ymin, ymax-dy)
                self._draw(True)
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
            
    def _draw(self, clear=False, update=True):
        if self._x and self._y:
            self.plot(self._x, self._y, clear)
        self._drawTitleAndSubTitle()
        self._drawXYLabel()
        if update:
            self.update()
        
#    def _save(self, path):
    def save(self, path):
        (p, ext) = os.path.splitext(path)
        if ext.lower() == InteractiveGRWidget.GRX_SUFFIX:
            gr.begingraphics(path)
            self._draw(update=False)
            gr.endgraphics()
        else:
            gr.beginprint(path)
            self._draw(update=False)
            gr.endprint()
        
#    def save(self, path):
#        self._printPath = path
    
    def plot(self, x, y, clear=False):
        if self._resetWindow:
            window = None
            self._resetWindow = False
        else:
            window = gr.inqwindow()
        self._x = x
        self._y = y
        pygr.plot(x, y, bgcolor=self._bgColor, viewport=self._viewport,
                  window=window, scale=self._option_scale, grid=self._grid,
                  clear=clear, update=False)
    
    def paintEvent(self, event):
        super(InteractiveGRWidget, self).paintEvent(event)
        self._painter.begin(self)
#        if self._printPath:
#            self._save(self._printPath)
#            self._printPath = None
#        else:
        if self._mouseLeft:
            startDC = self._startPoint.getDC()
            endDC = self._curPoint.getDC()
            rect = QtCore.QRect(QtCore.QPoint(startDC.x(), startDC.y()),
                                QtCore.QPoint(endDC.x(),
                                              endDC.y())).normalized()
            self._painter.setOpacity(.75)
            self._painter.drawRect(rect)
            self._painter.setOpacity(1.)

        self._painter.end()
        
    def getTitle(self):
        return self._plotTitle
    
    def setTitle(self, title):
        self._plotTitle = title
        self._draw(True)
        self.update()
                
    def getSubTitle(self):
        return self._plotSubTitle
    
    def setSubTitle(self, subtitle):
        self._plotSubTitle = subtitle
        self._draw(True)
        self.update()
    
    def getXLabel(self):
        return self._lblX
    
    def setXLabel(self, xlabel):
        self._lblX = xlabel
        self._draw(True)
        self.update()
        
    def getYLabel(self):
        return self._lblY
    
    def setYLabel(self, ylabel):
        self._lblY = ylabel
        self._draw(True)
        self.update()
    
    def setviewport(self, xmin, xmax, ymin, ymax):
        self._viewport = [xmin, xmax, ymin, ymax]
        
    def getviewport(self):
        return self._viewport
    
    def getPickMode(self):
        return self._pickMode
    
    def setPickMode(self, bool):
        self._pickMode = bool
        self.emit(QtCore.SIGNAL("modePick(bool)"), self._pickMode)
        
    def setLogX(self, bool):
        if bool:
            self._option_scale |= gr.OPTION_X_LOG
        else:
            self._option_scale &= ~gr.OPTION_X_LOG
        self._draw(clear=True)
            
    def setLogY(self, bool):
        if bool:
            self._option_scale |= gr.OPTION_Y_LOG
        else:
            self._option_scale &= ~gr.OPTION_Y_LOG
        self._draw(clear=True)
            
    def setGrid(self, bool):
        self._grid = bool
        self._draw(clear=True)
        
    def reset(self):
#        gr.clearws()
        self._resetWindow = True
        self._draw(clear=True)
        
    def _check_window(self, xmin, xmax, ymin, ymax):
        res = True
        if (math.isnan(xmin) or math.isinf(xmin) or math.isnan(xmax)
            or math.isinf(xmax) or math.isnan(ymin) or math.isinf(ymin) or
            math.isnan(ymax) or math.isinf(ymax) or
            xmin > max or ymin > ymax):
            res = False
        return res
    
    def _pick(self, p0, type):
        if self._x and self._y:
            p0x = p0.x()
            for idx, x in enumerate(self._x):
                if x >= p0x:
                    break
            coord = CoordConverter(self.width(), self.height())
            coord.setWC(self._x[idx], self._y[idx])
            dcPoint = coord.getDC()
            
            QtGui.QApplication.sendEvent(self, PickEvent(type,
                                                         self.width(),
                                                         self.height(),
                                                         dcPoint.x(),
                                                         dcPoint.y()))
        
    def _select(self, p0, p1):
        gr.clearws()
        if p0.x() > p1.x():
            xmin = p1.x()
            xmax = p0.x()
        else:
            xmin = p0.x()
            xmax = p1.x()
        if p0.y() > p1.y():
            ymin = p1.y()
            ymax = p0.y()
        else:
            ymin = p0.y()
            ymax = p1.y()
        gr.setwindow(xmin, xmax, ymin, ymax)
        self._draw()
        
    def _pan(self, dp):
        gr.clearws()
        window = gr.inqwindow()
        window[0] -= dp.x()
        window[1] -= dp.x()
        window[2] -= dp.y()
        window[3] -= dp.y()
        gr.setwindow(*window)
        self._draw()
                
    def _zoom(self, point, dpercent):
        gr.clearws()
        window = gr.inqwindow()
        winWidth_2 = (window[1]-window[0])/2
        winHeight_2 = (window[3]-window[2])/2
        dx_2 = winWidth_2 - (1+dpercent)*winWidth_2
        dy_2 = winHeight_2 - (1+dpercent)*winHeight_2
        
        window[0] -= dx_2
        window[1] += dx_2
        window[2] -= dy_2
        window[3] += dy_2
        
        if not self._check_window(*window):
            self._resetWindow = True
            self._draw(True)
        else:
            gr.setwindow(*window)
            self._draw()
        
    def mousePress(self, event):
        if event.getButtons() & MouseEvent.LEFT_BUTTON:
            if self.getPickMode():
                self._pick(event.getWC(), PickEvent.PICK_PRESS)
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
            p0 = self._startPoint.getWC()
            p1 = self._curPoint.getWC()
            if p0 != p1:
                self._select(p0, p1)
        elif event.getButtons() & MouseEvent.RIGHT_BUTTON:
            self._mouseRight = False
        self._curPoint = event
            
    def mouseMove(self, event):
        if self.getPickMode():
            self._pick(event.getWC(), PickEvent.PICK_MOVE)
        if event.getButtons() & MouseEvent.LEFT_BUTTON:
            self._curPoint = event
            self.update()
        elif event.getButtons() & MouseEvent.RIGHT_BUTTON:
            p0 = self._curPoint.getWC() # point before now
            p1 = event.getWC()
            dp = p1-p0
            self._curPoint = event
            self._pan(dp)
            
    def wheelMove(self, event):
        # delta percent
        dpercent = event.getDegree()/16.
        self._zoom(event.getWC(), dpercent)
        
    def pickMove(self, event):
        wcPoint = event.getWC()
        self._draw(True)
        gr.setmarkertype(gr.MARKERTYPE_PLUS)
        gr.polymarker(1, [wcPoint.x()], [wcPoint.y()])
        self.update()
        
if __name__ == "__main__":
    import sys
    import pygr
    app = QtGui.QApplication(sys.argv)
    grw = InteractiveGRWidget()
    grw.setviewport(0.1, 0.95, 0.1, 0.9)
    grw.show()
#    gr.begingraphics("test.grx")
#    gr.beginprint("test.pdf")
#    gr.openws(5, "WISS", 5)
#    gr.createseg(0)
    x = [-3.3 + t*.1 for t in range(66)]
    y = [t**5 - 13*t**3 + 36*t for t in x]
#    y = [math.exp(t) for t in x]
    grw.plot(x, y)
#    pygr.plot(x, y, bgcolor=163, clear=False, update=False)
#    gr.clearws()
#    gks set seg_xform 0.5 0.5 0 0 pi/2 1 1
#    gr.setsegtran(0, .5, .5, 0, 0, 1.55, 1, 1)
    
#    gr.closews(5)
#    gr.endprint()
#    gr.endgraphics()
    sys.exit(app.exec_())
