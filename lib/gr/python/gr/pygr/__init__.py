# -*- coding: utf-8 -*-
"""Python GR module

Exported Classes:

"""
# standard library
import math
import time
# local library
import gr
from gr.pygr.base import GRMeta

__author__  = """Christian Felder <c.felder@fz-juelich.de>,
Josef Heinen <j.heinen@fz-juelich.de>"""
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

#__all__ = ['delay', 'gr', 'plot', 'plot3d', 'readfile']

class Point(object):
    
    def __init__(self, x, y):
        self._x, self._y = x, y
        
    @property
    def x(self):
        """Get the current x value."""
        return self._x
    
    @x.setter
    def x(self, value):
        self._x = value
    
    @property
    def y(self):
        """Get the current y value."""
        return self._y
    
    @y.setter
    def y(self, value):
        self._y = value
    
    def __str__(self):
        return "(%s, %s)" %(self._x, self._y)
    
    def __eq__(self, other):
        return (self.x == other.x and self.y == other.y)
    
    def __ne__(self, other):
        return not self.__eq__(other)
    
    def __add__(self, other):
        return Point(self.x+other.x, self.y+other.y)
    
    def __sub__(self, other):
        return Point(self.x-other.x, self.y-other.y)
    
    def __mul__(self, other):
        """Calculate scalar product."""
        return self.x*other.x + self.y*other.y
    
    def __div__(self, other):
        """Calculate component-by-component division."""
        return Point(self.x/other.x, self.y/other.y)
    
    def __neg__(self):
        """Calculate negation."""
        return Point(-self.x, -self.y)
    
    def __pos__(self):
        return self
    
    def __abs__(self):
        return Point(abs(self.x), abs(self.y))
    
    def norm(self):
        """Calculate euclidean norm."""
        return math.sqrt(self*self)
    
class RegionOfInterest(object):
    
    LEGEND = 0x1
    
    def __init__(self, *args, **kwargs):
        self._poly, self._x, self._y = None, None, None
        self._ref = None
        self._regionType = None
        for p in args:
            self.append(p)
        if "reference" in kwargs:
            self._ref = kwargs["reference"]
        if "regionType" in kwargs:
            self._regionType = kwargs["regionType"]
        
    @property
    def x(self):
        """Get the current x values."""
        return self._x
    
    @property
    def y(self):
        """Get the current y values."""
        return self._y
    
    @property
    def reference(self):
        """Get the current reference."""
        return self._ref
    
    @reference.setter
    def reference(self, ref):
        self._ref = ref
        
    @property
    def regionType(self):
        """Get the current regionType."""
        return self._regionType
    
    @regionType.setter
    def regionType(self, rtype):
        self._regionType = rtype
        
    def append(self, *args):
        if self._poly is None:
            self._poly = []
            self._x = []
            self._y = []
        for p0 in args:
            self._poly.append(p0)
            self._x.append(p0.x)
            self._y.append(p0.y)
            
    def isPointInside(self, p0):
        """
        
        """
        # Even-odd rule (Point in Polygon Test)
        # assume a ray from testpoint to infinity along positive x axis
        # count intersections along this ray.
        # If odd inside - outside otherwise.
        res = False
        if self._poly:
            n = len(self._poly)
            j = n-1
            for i in range(n):
                pi, pj = self._poly[i], self._poly[j]
                # if testpoint y component between pi, pj
                # => intersection possible
                if ( ((pi.y > p0.y) != (pj.y > p0.y)) and
                     # if x components are both greater than testpoint's x
                     # => intersection (with polyline)
                     ( ((pi.x > p0.x) and (pj.x > p0.x)) or
                    # if testpoint left from intersection (with polyline)
                    # along x axis with same y component
                    # => intersection with polyline
                       (p0.x < ((p0.y-pi.y) * (pi.x-pj.x)/(pi.y-pj.y) + pi.x))
                     )
                    ):
                    # flip on intersection
                    res = not res
                j = i
        return res

class CoordConverter(object):
    
    def __init__(self, width, height, window=None):
        self._width, self._height, self._window = width, height, window
        self._p = None
    
    def _checkRaiseXY(self):
        if self._p.x is None or self._p.y is None:
            raise AttributeError("x or y has not been initialized.")
        return True
    
    @property
    def window(self):
        return self._window
        
    def setWindow(self, xmin, xmax, ymin, ymax):
        self._window = [xmin, xmax, ymin, ymax]
    
    def getWindow(self):
        if self._window:
            return list(self._window)
        else:
            return None
    
    def setDC(self, x, y):
        self._p = Point(x, y)
        return self
        
    def setNDC(self, x, y):
        self._p = Point(x * self._width, (1.-y) * self._height)
        return self
        
    def setWC(self, x, y, window=None):
        if window:
            self.setWindow(*window)
        if self.getWindow():
            window = gr.inqwindow()
            gr.setwindow(*self.getWindow())
            tNC = gr.wctondc(x, y) # ndc tuple
            gr.setwindow(*window)
        else:
            tNC = gr.wctondc(x, y) # ndc tuple
        self.setNDC(tNC[0], tNC[1])
        return self
    
    def getDC(self):
        self._checkRaiseXY()
        return self._p
        
    def getNDC(self):
        self._checkRaiseXY()
        return Point(float(self._p.x)/self._width,
                     1. - float(self._p.y)/self._height)
        
    def getWC(self):
        self._checkRaiseXY()
        ndcPoint = self.getNDC()
        if self.getWindow():
            window = gr.inqwindow()
            gr.setwindow(*self.getWindow())
            tWC = gr.ndctowc(ndcPoint.x, ndcPoint.y) # wc tuple
            gr.setwindow(*window)
        else:
            tWC = gr.ndctowc(ndcPoint.x, ndcPoint.y) # wc tuple
        return Point(tWC[0], tWC[1])

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
    
class ColorIndexGenerator(object):
    
    _distinct_colors = range(980, 1000)
    _n = len(_distinct_colors)
    _curIdx = 0
    
    @staticmethod
    def nextColorIndex():
        idx = ColorIndexGenerator._distinct_colors[ColorIndexGenerator._curIdx]
        ColorIndexGenerator._curIdx = ((ColorIndexGenerator._curIdx + 1)
                                       % ColorIndexGenerator._n)
        return idx

class ErrorBar(GRMeta):
    
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
        
class Plot(GRMeta):
    
    DEFAULT_VIEWPORT = (.1, .95, .1, .95)
    
    def __init__(self, viewport=list(DEFAULT_VIEWPORT)):
        self._viewport = viewport
        self._viewMaxYForText = viewport[3] + .05
        self._lstAxes = []
        self._title = None
        self._subTitle = None
        self._lblX = None
        self._lblY = None
        self._legend = False
        self._legendROI = None
        
    @property
    def xlabel(self):
        """get label for x axis"""
        return self._lblX
        
    @xlabel.setter
    def xlabel(self, xlabel):
        self._lblX = xlabel
        
    @property
    def ylabel(self):
        """get label for y axis"""
        return self._lblY
        
    @ylabel.setter
    def ylabel(self, ylabel):
        self._lblY = ylabel
        
    @property
    def subTitle(self):
        """get plot subtitle"""
        return self._subTitle
    
    @subTitle.setter
    def subTitle(self, subTitle):
        self._subTitle = subTitle
        
    @property
    def title(self):
        """get plot title"""
        return self._title
    
    @title.setter
    def title(self, title):
        self._title = title
        
    @property
    def viewport(self):
        """get current viewport"""
        return self._viewport
    
    @viewport.setter
    def viewport(self, viewport):
        self._viewport = viewport
        self._viewMaxYForText = viewport[3] + .05
        for axes in self._lstAxes:
            axes.viewport = viewport
        
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
                
    def setGrid(self, bool):
        for axes in self._lstAxes:
            axes.setGrid(bool)
            
    def setLegend(self, bool):
        self._legend = bool
                
    def reset(self):
        for axes in self._lstAxes:
            axes.reset()
        
    def logXinDomain(self):
        logXinDomain = True
        for axes in self._lstAxes:
            logXinDomain = (logXinDomain & axes.isXLogDomain())
        return logXinDomain
            
    def logYinDomain(self):
        logYinDomain = True
        for axes in self._lstAxes:
            logYinDomain = (logYinDomain & axes.isYLogDomain())
        return logYinDomain
    
    def pick(self, p0, width, height):
        coord = None
        window = gr.inqwindow()
        if self._lstAxes:
            coord = CoordConverter(width, height)
            points = []
            lstAxes = []
            for axes in self._lstAxes:
                gr.setwindow(*axes.getWindow())
                coord.setNDC(p0.x, p0.y)
                wcPick = coord.getWC()
                curves = filter(lambda c: c.visible, axes.getCurves())
                for curve in curves:
                    for idx, x in enumerate(curve.x):
                        if x >= wcPick.x:
                            break
                    coord.setWC(x, curve.y[idx])
                    points.append(coord.getNDC())
                    lstAxes.append(axes)
            if points: 
                # calculate distance between p0 and point on curve
                norms = map(lambda p: (p0-p).norm(), points)
                # nearest point
                idx = norms.index(min(norms))
                p = points[idx]
                axes = lstAxes[idx]
                coord.setNDC(p.x, p.y)
                coord.setWindow(*axes.getWindow())
            else:
                coord = None
        gr.setwindow(*window)
        return coord
    
    def select(self, p0, p1, width, height):
        window = gr.inqwindow()
        coord = CoordConverter(width, height)
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
        
    def pan(self, dp, width, height):
        window = gr.inqwindow()
        coord = CoordConverter(width, height)
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
        
    def zoom(self, dpercent):
        window = gr.inqwindow()
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
            
    def getROI(self, p0):
        res = None
        if self._legendROI:
            for roi in self._legendROI:
                if roi.isPointInside(p0):
                    res = roi
        return res
        
    def addAxes(self, *args, **kwargs):
        for plotAxes in args:
            if plotAxes and plotAxes not in self._lstAxes:
                plotAxes.viewport = self.viewport
                self._lstAxes.append(plotAxes)
        return self

    def drawGR(self):
        [xmin, xmax, ymin, ymax] = self.viewport
        # draw title and subtitle
        if self.title or self.subTitle:
            dyTitle = 0
            dySubTitle = 0
            charHeight = .027 * (self._viewMaxYForText-ymin)
            gr.settextalign(gr.TEXT_HALIGN_CENTER, gr.TEXT_VALIGN_TOP)
            gr.setcharup(0., 1.)
            gr.setcharheight(charHeight)
            x = xmin + (xmax-xmin)/2
            if self.title:
                tby = gr.inqtextext(0, 0, self.title)[1]
                tby = map(lambda y: gr.wctondc(0, y)[1], tby)
                dyTitle = max(tby)-min(tby)
            y = self._viewMaxYForText - (dyTitle+dySubTitle)/2
            if self.title and self.subTitle:
                tby = gr.inqtextext(0, 0, self.subTitle)[1]
                tby = map(lambda y: gr.wctondc(0, y)[1], tby)
                dySubTitle = max(tby)-min(tby)
            if (ymax+dyTitle+dySubTitle) > self._viewMaxYForText:
                # preserve self._viewMaxYForText value
                # to avoid changing by self.viewport setter
                maxYForText = self._viewMaxYForText
                self.viewport = [xmin, xmax, ymin, ymax-dyTitle-dySubTitle]
                self._viewMaxYForText = maxYForText
                gr.clearws()
            if self.title:
                gr.text(x, y, self.title)
                y -= dyTitle
            if self.subTitle:
                gr.text(x, y, self.subTitle)
        # draw axes and curves
        if self._lstAxes:
            for axes in self._lstAxes:
                axes.drawGR()
        # current values, viewport maybe changed above
        [xmin, xmax, ymin, ymax] = self.viewport
        dyXLabel = 0
        dxYLabel = 0
        # draw x- and y label
        if self.xlabel:
            gr.settextalign(gr.TEXT_HALIGN_CENTER, gr.TEXT_VALIGN_TOP)
            gr.setcharup(0., 1.)
            tby = gr.inqtextext(0, 0, self.xlabel)[1]
            tby = map(lambda y: gr.wctondc(0, y)[1], tby)
            dyXLabel = max(tby)-min(tby)
            gr.text(xmin+(xmax-xmin)/2., ymin-dyXLabel/2.-.05, self.xlabel)
        if self.ylabel:
            gr.settextalign(gr.TEXT_HALIGN_CENTER, gr.TEXT_VALIGN_TOP)
            gr.setcharup(-1., 0.)
            tbx = gr.inqtextext(0, 0, self.ylabel)[0]
            tbx = map(lambda y: gr.wctondc(0, y)[0], tbx)
            dxYLabel = max(tbx)-min(tbx)
            gr.text(xmin-dxYLabel/2.-.075, ymin+(ymax-ymin)/2., self.ylabel)
            gr.setcharup(0., 1.)
        # draw legend and calculate ROIs for legend items 
        if self._legend:
            x, y = .1, ymin-dyXLabel-.075
            if y < 0:
                self.viewport[2] += dyXLabel
                gr.clearws()
                self.drawGR()
            else:
                window = gr.inqwindow()
                gr.setviewport(0, 1, 0, 1)
                gr.setwindow(0, 1, 0, 1)
                # preserve old values
                ltype = gr.inqlinetype()
                mtype = gr.inqmarkertype()
                lcolor = gr.inqlinecolorind()
                mcolor = gr.inqmarkercolorind()
                self._legendROI = []
                for axes in self._lstAxes:
                    for curve in axes.getCurves():
                        if curve.legend:
                            gr.setlinetype(curve.getLineType())
                            gr.setmarkertype(curve.getMarkerType())
                            gr.setlinecolorind(curve.getLineColor())
                            gr.setmarkercolorind(curve.getMarkerColor())
                            if curve.getLineType() is not None:
                                gr.setlinecolorind(curve.getLineColor())
                                gr.setmarkercolorind(curve.getMarkerColor())
                                gr.setlinetype(curve.getLineType())
                                gr.polyline(2, [x, x+.1], [y, y])
                                if (curve.getMarkerType() != gr.MARKERTYPE_DOT
                                    and curve.getMarkerType() is not None):
                                    gr.setmarkertype(curve.getMarkerType())
                                    gr.polymarker(1, [x+.1/2.], [y])
                            elif curve.getMarkerType() is not None:
                                gr.setmarkertype(curve.getMarkerType)
                                gr.polymarker(1, [x+.1/2.], [y])
                            tbx, tby = gr.inqtextext(0, 0, curve.legend)
                            ybase = y-(max(tby)-min(tby))/2
                            ytop = y+(max(tby)-min(tby))/2
                            roi = RegionOfInterest(Point(x, ybase),
                                                   Point(x, ytop),
                                                   reference=curve,
                                                   regionType=RegionOfInterest.LEGEND)
                            x += .11
                            if curve.visible:
                                gr.settextcolorind(1)
                            else:
                                gr.settextcolorind(83)
                            gr.settextalign(gr.TEXT_HALIGN_LEFT,
                                            gr.TEXT_VALIGN_HALF)
                            gr.text(x, y, curve.legend)
                            gr.settextcolorind(1)
                            x += max(tbx)-min(tbx)
                            roi.append(Point(x, ytop), Point(x, ybase))
                            self._legendROI.append(roi)
                if not self._legendROI:
                    self._legendROI = None
                # restore old values
                gr.setlinecolorind(lcolor)
                gr.setmarkercolorind(mcolor)
                gr.setlinetype(ltype)
                gr.setmarkertype(mtype)
                # restore viewport and window
                gr.setviewport(*self.viewport)
                gr.setwindow(*window)

class PlotCurve(GRMeta):
    
    COUNT = 0
    
    def __init__(self, x, y, errBar1=None, errBar2=None,
                 linetype=gr.LINETYPE_SOLID, markertype=gr.MARKERTYPE_DOT,
                 linecolor=None, markercolor=1, legend=None):
        self._x, self._y = x, y
        self._e1, self._e2 = errBar1, errBar2
        self._linetype, self._markertype = linetype, markertype
        self._markercolor = markercolor
        self._legend = legend
        if linecolor is None:
            self._linecolor = ColorIndexGenerator.nextColorIndex()
        else:
            self._linecolor = linecolor
        PlotCurve.COUNT += 1
        self._id = PlotCurve.COUNT
        if legend is None:
            self._legend = "curve %d" %self._id
        self._n = len(self._x)
        self._visible = True
        
    def setLineType(self, linetype):
        self._linetype = linetype
        
    def getLineType(self):
        return self._linetype
        
    def setMarkerType(self, markertype):
        self._markertype = markertype
        
    def getMarkerType(self):
        return self._markertype
    
    def setLineColor(self, color):
        self._linecolor = color
        
    def getLineColor(self):
        return self._linecolor
    
    def setMarkerColor(self, color):
        self._markercolor = color
        
    def getMarkerColor(self):
        return self._markercolor

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
        
    @property
    def legend(self):
        """Get current text for a legend."""
        return self._legend
        
    @legend.setter
    def legend(self, s):
        self._legend = s
        
    @property
    def visible(self):
        return self._visible
    
    @visible.setter
    def visible(self, bool):
        self._visible = bool
    
    def drawGR(self):
        if self.visible:
            # preserve old values
            ltype = gr.inqlinetype()
            mtype = gr.inqmarkertype()
            lcolor = gr.inqlinecolorind()
            mcolor = gr.inqmarkercolorind()
    
            if self.getLineType() is not None:
                gr.setlinecolorind(self.getLineColor())
                gr.setmarkercolorind(self.getMarkerColor())
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
            # restore old values
            gr.setlinecolorind(lcolor)
            gr.setmarkercolorind(mcolor)
            gr.setlinetype(ltype)
            gr.setmarkertype(mtype)
            
class PlotAxes(GRMeta):

    COUNT = 0

    def __init__(self, xtick=None, ytick=None, majorx=None, majory=None,
                 viewport=list(Plot.DEFAULT_VIEWPORT), drawX=True, drawY=True):
        self._xtick, self._ytick = xtick, ytick
        self.majorx, self.majory = majorx, majory
        self._viewport, self._drawX, self._drawY = viewport, drawX, drawY
        self._lstPlotCurve = []
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
    def xtick(self):
        """get current user-defined interval between minor x ticks"""
        return self._xtick
    
    @xtick.setter
    def xtick(self, tickInterval):
        self._xtick = tickInterval
        
    @property
    def ytick(self):
        """get current user-defined interval between minor y ticks"""
        return self._ytick
    
    @ytick.setter
    def ytick(self, tickInterval):
        self._ytick = tickInterval
        
    @property
    def majorx(self):
        """get current user-defined number of minor tick intervals between
        major x tick marks"""
        return self._majorx

    @majorx.setter    
    def majorx(self, minorCount):
        self._majorx = minorCount if minorCount > 0 or minorCount is None else 1
        
    @property
    def majory(self):
        """get current user-defined number of minor tick intervals between
        major y tick marks"""
        return self._majory 

    @majory.setter    
    def majory(self, minorCount):
        self._majory = minorCount if minorCount > 0 or minorCount is None else 1
    
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
        if self._window:
            return list(self._window)
        else:
            return None
    
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
                    majorx = self._majorx if self._majorx is not None else 5
                    if self._xtick is not None:
                        xtick = self._xtick
                    else:
                        xtick = gr.tick(xmin, xmax) / majorx
                if self.scale & gr.OPTION_Y_LOG:
                    ytick = majory = 1
                else:
                    majory = self._majory if self._majory is not None else 5
                    if self._ytick is not None:
                        ytick = self._ytick
                    else:
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
                if self.isGridEnabled() and self.getId() == 1:
                    gr.grid(xtick, ytick, xmax, ymax, majorx, majory)
                if self.getId() == 1:
                    # first x, y axis
                    if not self.isXDrawingEnabled():
                        majorx = -majorx
                    if not self.isYDrawingEnabled():
                        majory = -majory
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
        return self
    
def ndctowc(x, y):
    try:
        iter(x)
        iter(y)
        resX = []
        resY = []
        for xi, yi in map(lambda x, y: gr.ndctowc(x, y), x, y): 
            resX.append(xi)
            resY.append(yi)
        return resX, resY
    except TypeError:
        return gr.ndctowc(x, y)
    
def wctondc(x, y):
    try:
        iter(x)
        iter(y)
        resX = []
        resY = []
        for xi, yi in map(lambda x, y: gr.wctondc(x, y), x, y): 
            resX.append(xi)
            resY.append(yi)
        return resX, resY
    except TypeError:
        return gr.wctondc(x, y)
    
def _guessdimension(len):
    x = int(math.sqrt(len))
    d = []
    while x >= 1:
        y = len / x
        if x * y == len:
            d.append((x, y))
        x -= 1
    return sorted(d, key=lambda t: t[1] - t[0])

def readfile(path, separator = ''):
    fp = open(path, "r")
    if separator != '':
        while True:
            line = fp.readline()
            if not line:
                break
            if line[0] == separator:
                break
    data = []
    for line in fp.readlines():
        for item in line.split():
            data.append(float(item))
    fp.close()
    return data
 
def plot(x, y,
         bgcolor = 0,
         viewport = (0.1, 0.95, 0.1, 0.95),
         window = None,
         scale = 0,
         grid = True,
         linetype = gr.LINETYPE_SOLID,
         markertype = gr.MARKERTYPE_DOT,
         clear = True,
         update = True):
    if clear:
        gr.clearws()
    n = len(x)
    if window == None:
        if scale & gr.OPTION_X_LOG == 0:
            xmin, xmax = gr.adjustrange(min(x), max(x))
        else:
            xmin, xmax = (min(x), max(x))
        if scale & gr.OPTION_Y_LOG == 0:
            ymin, ymax = gr.adjustrange(min(y), max(y))
        else:
            ymin, ymax = (min(y), max(y))
    else:
        xmin, xmax, ymin, ymax = window
    if scale & gr.OPTION_X_LOG == 0:
        majorx = 5
        xtick = gr.tick(xmin, xmax) / majorx
    else:
        xtick = majorx = 1
    if scale & gr.OPTION_Y_LOG == 0:
        majory = 5
        ytick = gr.tick(ymin, ymax) /majory 
    else:
        ytick = majory = 1
    gr.setviewport(viewport[0], viewport[1], viewport[2], viewport[3])
    gr.setwindow(xmin, xmax, ymin, ymax)
    gr.setscale(scale)
    if bgcolor != 0:
        gr.setfillintstyle(1)
        gr.setfillcolorind(bgcolor)
        gr.fillrect(xmin, xmax, ymin, ymax)
    charheight = 0.024 * (viewport[3] - viewport[2])
    gr.setcharheight(charheight)
    if grid:
         gr.grid(xtick, ytick, xmax, ymax, majorx, majory)
    gr.axes(xtick, ytick, xmin, ymin,  majorx,  majory,  0.01)
    gr.axes(xtick, ytick, xmax, ymax, -majorx, -majory, -0.01)
    gr.setlinetype(linetype)
    gr.polyline(n, x, y)
    if markertype != gr.MARKERTYPE_DOT:
        gr.setmarkertype(markertype)
        gr.polymarker(n, x, y)
    if update:
        gr.updatews()

def plot3d(z,
           viewport = (0.1, 0.9, 0.1, 0.9),
           rotation = 30,
           tilt = 70,
           colormap = 1,
           option = 4,
           contours = True,
           xtitle = '',
           ytitle = '',
           ztitle = ''):
    gr.clearws()
    xmin, ymin = (1, 1)
    xmax, ymax = _guessdimension(len(z))[0]
    xtick = gr.tick(xmin, xmax) / 5
    ytick = gr.tick(ymin, ymax) / 5
    x = range(1, xmax + 1)
    y = range(1, ymax + 1)
    zmin = min(z)
    zmax = max(z)
    zmin, zmax = gr.adjustrange(zmin, zmax)
    ztick = gr.tick(zmin, zmax) / 5
    gr.setviewport(viewport[0], viewport[1], viewport[2], viewport[3])
    gr.setwindow(xmin, xmax, ymin, ymax)
    gr.setspace(zmin, zmax, rotation, tilt)
    charheight = 0.024 * (viewport[3] - viewport[2])
    gr.setcharheight(charheight)
    if rotation != 0 or tilt != 90:
        gr.axes3d(xtick, 0, ztick, xmin, ymin, zmin, 5, 0, 5, -0.01)
        gr.axes3d(0, ytick,  0, xmax, ymin, zmin, 0, 5, 0,  0.01)
    gr.setcolormap(colormap)
    gr.surface(xmax, ymax, x, y, z, option)
    if contours:
        gr.contour(xmax, ymax, 0, x, y, range(1), z, 0)
    if rotation == 0 and tilt == 90:
        gr.axes(xtick, ytick, xmin, ymin, 5, 5, -0.01)
    if xtitle != '' or ytitle != '' or ztitle != '':
        gr.titles3d(xtitle, ytitle, ztitle)
    gr.updatews()

def delay(seconds):
    time.sleep(seconds)
