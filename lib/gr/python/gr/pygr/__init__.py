# -*- coding: utf-8 -*-
"""Python GR module

Exported Classes:

"""

# standard library
import math
import time
import logging
import collections
# third party
from numpy import ndarray, asarray
# local library
import gr
import gr3
from gr.pygr.base import GRDrawAttributes, GRMeta, GRViewPort, GRVisibility
from gr.pygr.helper import ColorIndexGenerator, DomainChecker
from gr.pygr.mlab import plot, oplot, scatter, histogram, imshow, stem, polar
from gr.pygr.mlab import contour, contourf, surface, wireframe, plot3, trisurf, tricont
from gr.pygr.mlab import heatmap, hexbin
from gr.pygr.mlab import legend, title, xlabel, ylabel, xlim, ylim
from gr.pygr.mlab import figure, hold, subplot
from gr._version import __version__, __revision__

__author__ = """Christian Felder <c.felder@fz-juelich.de>,
Josef Heinen <j.heinen@fz-juelich.de>,
Florian Rhiem <f.rhiem@fz-juelich.de>"""
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


class Coords2D(object):

    def __init__(self, x, y, updatex_callback=None, updatey_callback=None):
        self._x, self._y = x, y
        self._updatex_callback = updatex_callback
        self._updatey_callback = updatey_callback

    @property
    def x(self):
        """Get the current list/ndarray of x values."""
        return self._x

    @x.setter
    def x(self, lst):
        self._x = lst
        if self._updatex_callback:
            self._updatex_callback(self)

    @property
    def y(self):
        """Get the current list/ndarray of y values."""
        return self._y

    @y.setter
    def y(self, lst):
        self._y = lst
        if self._updatey_callback:
            self._updatey_callback(self)

    def setUpdateXCallback(self, fp):
        self._updatex_callback = fp

    def setUpdateYCallback(self, fp):
        self._updatey_callback = fp


class Coords3D(Coords2D):

    def __init__(self, x, y, z):
        Coords2D.__init__(self, x, y)
        self._z = z

    @property
    def z(self):
        """Get the current list/ndarray of z values."""
        return self._z

    @z.setter
    def z(self, lst):
        self._z = lst


class GridCoords3D(Coords3D):

    def __init__(self, x, y, z, nx=None, ny=None):
        if nx and ny:
            Coords3D.__init__(self, *gr.gridit(x, y, z, nx, ny))
        else:
            Coords3D.__init__(self, x, y, z)


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
        return "(%s, %s)" % (self._x, self._y)

    def __eq__(self, other):
        return (self.x == other.x and self.y == other.y)

    def __ne__(self, other):
        return not self.__eq__(other)

    def __add__(self, other):
        return Point(self.x + other.x, self.y + other.y)

    def __sub__(self, other):
        return Point(self.x - other.x, self.y - other.y)

    def __mul__(self, other):
        """Calculate scalar product."""
        return self.x * other.x + self.y * other.y

    def __div__(self, other):
        """Calculate component-by-component division."""
        return Point(self.x / other.x, self.y / other.y)

    def __neg__(self):
        """Calculate negation."""
        return Point(-self.x, -self.y)

    def __pos__(self):
        return self

    def __abs__(self):
        return Point(abs(self.x), abs(self.y))

    def norm(self):
        """Calculate euclidean norm."""
        return math.sqrt(self * self)


class RegionOfInterest(object):

    LEGEND = 0x1
    TEXT = 0x2

    def __init__(self, *args, **kwargs):
        self._poly, self._x, self._y = None, None, None
        self._ref = None
        self._regionType = None
        self._axes = None
        for p in args:
            self.append(p)
        if "reference" in kwargs:
            self._ref = kwargs["reference"]
        if "regionType" in kwargs:
            self._regionType = kwargs["regionType"]
        if "axes" in kwargs:
            self._axes = kwargs["axes"]

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

    def _getNDCPoly(self):
        poly = []
        if self._poly:
            if self._axes:
                coord = CoordConverter(self._axes.sizex, self._axes.sizey,
                                       self._axes.getWindow())
                gr.setscale(self._axes.scale)
                for pWC in self._poly:
                    coord.setWCforPlotAxes(pWC.x, pWC.y, self._axes)
                    poly.append(coord.getNDC())
            else:
                poly = self._poly
        return poly

    def isPointInside(self, p0):
        """
        
        """
        # Even-odd rule (Point in Polygon Test)
        # assume a ray from testpoint to infinity along positive x axis
        # count intersections along this ray.
        # If odd inside - outside otherwise.
        res = False
        poly = self._getNDCPoly()
        if poly:
            n = len(poly)
            j = n - 1
            for i in range(n):
                pi, pj = poly[i], poly[j]
                # if testpoint y component between pi, pj
                # => intersection possible
                if (((pi.y > p0.y) != (pj.y > p0.y)) and
                     # if x components are both greater than testpoint's x
                     # => intersection (with polyline)
                     (((pi.x > p0.x) and (pj.x > p0.x)) or
                    # if testpoint left from intersection (with polyline)
                    # along x axis with same y component
                    # => intersection with polyline
                       (p0.x < ((p0.y - pi.y) * (pi.x - pj.x) / (pi.y - pj.y) + pi.x))
                     )
                    ):
                    # flip on intersection
                    res = not res
                j = i
        return res

class CoordConverter(object):

    def __init__(self, sizex, sizey, window=None, scale=None):
        self._sizex, self._sizey, self._window = sizex, sizey, window
        self._scale = scale
        self._p = None  # always stored in NDC

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

    def setScale(self, scale):
        self._scale = scale

    def getScale(self):
        return self._scale

    def setNDC(self, x, y):
        self._p = Point(x, y)
        return self

    def setWC(self, x, y, viewport, window=None):
        if self.getScale() is not None:
            scale = self.getScale()
        else:
            scale = gr.inqscale()
        if window:
            self.setWindow(*window)
        if self.getWindow():
            xmin, xmax, ymin, ymax = self.getWindow()
        else:
            xmin, xmax, ymin, ymax = gr.inqwindow()
        vp = viewport
        if scale & gr.OPTION_Y_LOG:
            ymin, ymax = math.log10(ymin), math.log10(ymax)
            y = math.log10(y)
        if scale & gr.OPTION_X_LOG:
            xmin, xmax = math.log10(xmin), math.log10(xmax)
            x = math.log10(x)
        ndcX = (vp[0] * self._sizex + (x - xmin) / (xmax - xmin)
                * (vp[1] - vp[0]) * self._sizex)
        ndcY = (vp[2] * self._sizey + (y - ymin) / (ymax - ymin)
                * (vp[3] - vp[2]) * self._sizey)
        self.setNDC(ndcX, ndcY)
        return self

    def setWCforPlotAxes(self, x, y, axes):
        self.setWC(x, y, axes.viewport, axes.getWindow())
        return self

    def getNDC(self):
        self._checkRaiseXY()
        return self._p

    def getWC(self, viewport):
        if self.getScale() is not None:
            scale = self.getScale()
        else:
            scale = gr.inqscale()
        self._checkRaiseXY()
        ndcPoint = self.getNDC()
        ndcPoint = Point(ndcPoint.x, ndcPoint.y)
        if self.getWindow():
            xmin, xmax, ymin, ymax = self.getWindow()
        else:
            xmin, xmax, ymin, ymax = gr.inqwindow()
        vp = viewport
        if scale & gr.OPTION_Y_LOG:
            ymin, ymax = math.log10(ymin), math.log10(ymax)
        if scale & gr.OPTION_X_LOG:
            xmin, xmax = math.log10(xmin), math.log10(xmax)
        wcY = (ymin + (ndcPoint.y / self._sizey - vp[2])
               * (ymax - ymin) / (vp[3] - vp[2]))
        wcX = (xmin + (ndcPoint.x / self._sizex - vp[0])
               * (xmax - xmin) / (vp[1] - vp[0]))
        if scale & gr.OPTION_Y_LOG:
            wcY = 10 ** wcY
        if scale & gr.OPTION_X_LOG:
            wcX = 10 ** wcX
        return Point(wcX, wcY)


class DeviceCoordConverter(CoordConverter):

    def __init__(self, width, height, sizex=None, sizey=None, window=None,
                 scale=None):
        self._width, self._height = width, height
        if sizex is None and sizey is None:
            if self._width > self._height:
                sizex = 1.
                sizey = float(self._height) / self._width
            else:
                sizex = float(self._width) / self._height
                sizey = 1.
        CoordConverter.__init__(self, sizex, sizey, window, scale)

    def setDC(self, x, y):
        self._p = Point(float(x) / self._width * self._sizex,
                        (1. - float(y) / self._height) * self._sizey)
        return self

    def getDC(self):
        self._checkRaiseXY()
        return Point(self._p.x / self._sizex * self._width,
                     (1. - self._p.y / self._sizey) * self._height)


class Text(GRVisibility, Point, GRMeta):

    DEFAULT_CHARHEIGHT_FACTOR = .027

    def __init__(self, x, y, text, axes=None,
                 charheight=DEFAULT_CHARHEIGHT_FACTOR, charup=(0, 1),
                 halign=gr.TEXT_HALIGN_LEFT, valign=gr.TEXT_VALIGN_TOP,
                 textcolor=1, font=None, precision=None,
                 textpath=gr.TEXT_PATH_RIGHT, charexpan=1.,
                 hideviewport=True):
        GRVisibility.__init__(self, True)
        Point.__init__(self, x, y)
        self._font, self._precision = None, None
        self._x, self._y, self._text, self._axes = x, y, text, axes
        self._charheight = charheight
        self._charup, self._halign, self._valign = charup, halign, valign
        self._textcolor = textcolor
        self._textpath, self._charexpan = textpath, charexpan
        self._hideviewport = hideviewport
        if font is not None and precision is not None:
            self._font, self._precision = font, precision
        self._textlines = text.split('\n')

    @property
    def text(self):
        """Return the current text string."""
        return self._text

    @text.setter
    def text(self, text):
        self._text = text
        self._textlines = text.split('\n')

    @property
    def charheight(self):
        """Return the current charheight factor."""
        return self._charheight

    @charheight.setter
    def charheight(self, height):
        self._charheight = height

    @property
    def charup(self):
        """Return the current charup vector (ux, uy)."""
        return self._charup

    @charup.setter
    def charup(self, vector):
        self._charup = vector

    @property
    def halign(self):
        """Return the horizontal alignment."""
        return self._halign

    @halign.setter
    def halign(self, horizontal):
        self._halign = horizontal

    @property
    def valign(self):
        """Return the vertical alignment."""
        return self._valign

    @valign.setter
    def valign(self, vertical):
        self._valign = vertical

    @property
    def textcolor(self):
        """Return the color index for the text color."""
        return self._textcolor

    @textcolor.setter
    def textcolor(self, colorind):
        self._textcolor = colorind

    @property
    def textpath(self):
        """Return the current text path (direction in which text will be drawn)."""
        return self._textpath

    @textpath.setter
    def textpath(self, direction):
        self._textpath = direction

    @property
    def charexpan(self):
        """Return the current character expansion factor (width to height ratio)."""
        return self._charexpan

    @charexpan.setter
    def charexpan(self, factor):
        self._charexpan = factor

    def setFont(self, font, precision=gr.TEXT_PRECISION_STRING):
        """Set the font and the precision of the text representation."""
        self._font, self._precision = font, precision

    @property
    def hideviewport(self):
        """Return the current hide outside viewport state."""
        return self._hideviewport

    @hideviewport.setter
    def hideviewport(self, flag):
        self._hideviewport = flag

    def getBoundingBox(self):
        if self._axes:
            gr.setscale(self._axes.scale)
            coord = CoordConverter(self._axes.sizex, self._axes.sizey,
                                   self._axes.getWindow())
            coord.setWCforPlotAxes(self.x, self.y, self._axes)
            p0 = coord.getNDC()
            x, y = p0.x, p0.y

            charHeight = self.charheight * self._axes.sizey
            window = gr.inqwindow()
            # set viewport and window to NDC to allow 'line-drawing'
            # in all regions and in NDC coordinates
            # if hideviewport is False.
            gr.setviewport(0, self._axes.sizex, 0, self._axes.sizey)
            gr.setwindow(0, self._axes.sizex, 0, self._axes.sizey)
        else:
            x, y = self.x, self.y
            charHeight = self.charheight

        gr.setcharheight(charHeight)
        gr.setcharup(*self.charup)
        gr.settextalign(self.halign, self.valign)
        gr.settextpath(self.textpath)
        gr.setcharexpan(self.charexpan)
        if self._font is not None and self._precision is not None:
            gr.settextfontprec(self._font, self._precision)

        tbx, tby = gr.inqtext(x, y, self.text)

        if self._axes:
            gr.setviewport(*self._axes.viewportscaled)
            gr.setwindow(*window)
        return tbx, tby

    def drawGR(self):
        if self.visible:
            isInViewport = True
            if self._axes:
                coord = CoordConverter(self._axes.sizex, self._axes.sizey,
                                       self._axes.getWindow())
                coord.setWCforPlotAxes(self.x, self.y, self._axes)
                p0 = coord.getNDC()
                x, y = p0.x, p0.y
                charHeight = self.charheight * self._axes.sizey
                window = gr.inqwindow()
                # set viewport and window to NDC to allow 'line-drawing'
                # in all regions and in NDC coordinates
                # if hideviewport is False.
                gr.setviewport(0, self._axes.sizex, 0, self._axes.sizey)
                gr.setwindow(0, self._axes.sizex, 0, self._axes.sizey)
            else:
                x, y = self.x, self.y
                charHeight = self.charheight

            gr.setcharheight(charHeight)
            gr.setcharup(*self.charup)
            gr.settextalign(self.halign, self.valign)
            gr.settextpath(self.textpath)
            gr.setcharexpan(self.charexpan)
            if self._font is not None and self._precision is not None:
                gr.settextfontprec(self._font, self._precision)
            gr.settextcolorind(self.textcolor)

            if self._axes:
                tbx, tby = gr.inqtext(x, y, self.text)
                tbxmin, tbxmax = min(tbx), max(tbx)
                tbymin, tbymax = min(tby), max(tby)
                xmin, xmax, ymin, ymax = self._axes.viewportscaled
                if (tbxmin < xmin or tbxmax > xmax
                    or tbymin < ymin or tbymax > ymax):
                    isInViewport = False

            if not self.hideviewport or isInViewport:
                gr.text(x, y, self.text)

            if self._axes:
                gr.setviewport(*self._axes.viewportscaled)
                gr.setwindow(*window)
            gr.settextcolorind(1)


class ErrorBar(GRDrawAttributes, Coords2D, GRMeta):

    HORIZONTAL = 0
    VERTICAL = 1

    def __init__(self, x, y, dneg, dpos=None, direction=VERTICAL,
                 linetype=gr.LINETYPE_SOLID, markertype=gr.MARKERTYPE_OMARK,
                 linecolor=1, markercolor=1, linewidth=1):
        GRDrawAttributes.__init__(self, linetype, markertype, linecolor,
                                  markercolor, linewidth)
        Coords2D.__init__(self, asarray(x), asarray(y))
        self._direction = direction
        self._grerror = None
        if direction == ErrorBar.VERTICAL:
            self._grerror = gr.verrorbars
        elif direction == ErrorBar.HORIZONTAL:
            self._grerror = gr.herrorbars
        else:
            raise AttributeError("unsupported value for direction.")
        self.dneg, self.dpos = dneg, dpos if dpos is not None else dneg

    def _updateErrors(self):
        if self.direction == ErrorBar.VERTICAL:
            self._eneg = self.y - self._dneg
            self._epos = self.y + self._dpos
        else: # HORIZONTAL
            self._eneg = self.x - self._dneg
            self._epos = self.x + self._dpos

    @property
    def direction(self):
        """Get ErrorBars direction."""
        return self._direction

    @Coords2D.x.setter
    def x(self, seq):
        Coords2D.x.__set__(self, asarray(seq))
        self._updateErrors()

    @Coords2D.y.setter
    def y(self, seq):
        Coords2D.y.__set__(self, asarray(seq))
        self._updateErrors()

    @property
    def dneg(self):
        """Get the current ndarray of absolute deviation in negative direction."""
        return self._dneg

    @dneg.setter
    def dneg(self, seq):
        self._dneg = asarray(seq)
        if self.direction == ErrorBar.VERTICAL:
            self._eneg = self.y - self._dneg
        else: # HORIZONTAL
            self._eneg = self.x - self._dneg

    @property
    def dpos(self):
        """Get the current ndarray of absolute deviation in positive direction."""
        return self._dpos

    @dpos.setter
    def dpos(self, seq):
        self._dpos = asarray(seq)
        if self.direction == ErrorBar.VERTICAL:
            self._epos = self.y + self._dpos
        else: # HORIZONTAL
            self._epos = self.x + self._dpos

    def drawGR(self):
        # preserve old values
        ltype = gr.inqlinetype()
        mtype = gr.inqmarkertype()
        lcolor = gr.inqlinecolorind()
        mcolor = gr.inqmarkercolorind()
        lwidth = gr.inqlinewidth()

        if self.linetype is not None:
            gr.setlinecolorind(self.linecolor)
            gr.setmarkercolorind(self.markercolor)
            gr.setlinetype(self.linetype)
            gr.setlinewidth(self.linewidth)
        if self.markertype is not None:
            gr.setmarkertype(self.markertype)
        else:
            gr.setmarkertype(gr.MARKERTYPE_DOT)

        self._grerror(self._x, self._y, self._eneg, self._epos)

        # restore old values
        gr.setlinecolorind(lcolor)
        gr.setmarkercolorind(mcolor)
        gr.setlinetype(ltype)
        gr.setmarkertype(mtype)
        gr.setlinewidth(lwidth)

class Plot(GRViewPort, GRMeta):

    def __init__(self, viewport=GRViewPort.DEFAULT_VIEWPORT):
        super(Plot, self).__init__(viewport)
        self._lstAxes = []
        self._title, self._subTitle = None, None
        self._lblX, self._lblY = None, None
        self._offsetLblX, self._offsetLblY = 0., 0.
        self._legend = False
        self._legendWidth = 0.1
        self._legendROI = []
        self._rois = []
        self._autoscale = 0x0
        self._countAxes = 0

    def getAxes(self, idx=None):
        ret = None
        if idx is None:
            ret = self._lstAxes
        elif idx < len(self._lstAxes):
            ret = self._lstAxes[idx]
        return ret

    @property
    def offsetXLabel(self):
        """get NDC Y offset for xlabel"""
        return self._offsetLblX

    @offsetXLabel.setter
    def offsetXLabel(self, yoff):
        self._offsetLblX = yoff

    @property
    def offsetYLabel(self):
        """get NDC X offset for ylabel"""
        return self._offsetLblY

    @offsetYLabel.setter
    def offsetYLabel(self, xoff):
        self._offsetLblY = xoff

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

    @GRViewPort.viewport.setter
    def viewport(self, viewport):
        GRViewPort.viewport.__set__(self, viewport)
        for axes in self._lstAxes:
            axes.viewport = viewport

    @GRViewPort.sizex.setter
    def sizex(self, value):
        GRViewPort.sizex.__set__(self, value)
        for axes in self._lstAxes:
            axes.sizex = value

    @GRViewPort.sizey.setter
    def sizey(self, value):
        GRViewPort.sizey.__set__(self, value)
        for axes in self._lstAxes:
            axes.sizey = value

    def setLogX(self, bool, rescale=False):
        if bool:
            for axes in self._lstAxes:
                if axes.isXLogDomain():
                    axes.setLogX(bool)
                else:
                    if rescale:
                        lstPlotCurves = axes.getCurves()
                        if lstPlotCurves:
                            visibleCurves = [c for c in lstPlotCurves
                                             if c.visible]
                            if not visibleCurves:
                                visibleCurves = lstPlotCurves
                            window = axes.getWindow()
                            xmax = window[1]
                            xmin = min(min(c.x) for c in visibleCurves)
                            if xmin <= 0:
                                # 3 constant
                                xmin = 1. / 10 ** max(0, 3 - math.log10(xmax))

                            window[0] = xmin
                            axes.setWindow(*window)
                            axes.setLogX(bool)
                    else:
                        win = axes.getWindow()
                        if win is not None:
                            raise Exception("AXES[%d]: (%d..%d) "
                                            % (axes.getId(), win[0], win[1])
                                            + "not in log(x) domain.")
                        else:
                            raise Exception("AXES[%d] not in log(x) domain."
                                            % axes.getId())
        else:
            for axes in self._lstAxes:
                axes.setLogX(bool)

    def setLogY(self, bool, rescale=False):
        if bool:
            for axes in self._lstAxes:
                if axes.isYLogDomain():
                    axes.setLogY(bool)
                else:
                    if rescale:
                        lstPlotCurves = axes.getCurves()
                        if lstPlotCurves:
                            visibleCurves = [c for c in lstPlotCurves
                                             if c.visible]
                            if not visibleCurves:
                                visibleCurves = lstPlotCurves
                            window = axes.getWindow()
                            ymax = window[3]
                            ymin = min(min(c.y) for c in visibleCurves)
                            if ymin <= 0:
                                # 3 constant
                                ymin = 1. / 10 ** max(0, 3 - math.log10(ymax))

                            window[2] = ymin
                            axes.setWindow(*window)
                            axes.setLogY(bool)
                    else:
                        win = axes.getWindow()
                        if win is not None:
                            raise Exception("AXES[%d]: (%d..%d) "
                                            % (axes.getId(), win[2], win[3])
                                            + "not in log(y) domain.")
                        else:
                            raise Exception("AXES[%d] not in log(y) domain."
                                            % axes.getId())
        else:
            for axes in self._lstAxes:
                axes.setLogY(bool)

    def setGrid(self, bool):
        for axes in self._lstAxes:
            axes.setGrid(bool)

    def isLegendEnabled(self):
        return self._legend

    def setLegend(self, bool):
        self._legend = bool

    def setLegendWidth(self, width):
        self._legendWidth = width

    @property
    def autoscale(self):
        return self._autoscale

    @autoscale.setter
    def autoscale(self, mask):
        self._autoscale = mask
        for axes in self._lstAxes:
            axes.autoscale = mask

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
        coord, axes, curve = None, None, None
        window = gr.inqwindow()
        if self._lstAxes:
            coord = DeviceCoordConverter(width, height, self._sizex,
                                         self._sizey)
            points = []
            lstAxes = []
            lstCurves = []
            for axes in self._lstAxes:
                gr.setwindow(*axes.getWindow())
                coord.setWindow(*axes.getWindow())
                coord.setScale(axes.scale)
                coord.setNDC(p0.x, p0.y)
                wcPick = coord.getWC(axes.viewport)
                curves = [c for c in axes.getCurves() if c.visible]
                for curve in curves:
                    for idx, x in enumerate(curve.x):
                        if x >= wcPick.x:
                            break
                    coord.setWC(x, curve.y[idx], axes.viewport)
                    points.append(coord.getNDC())
                    lstAxes.append(axes)
                    lstCurves.append(curve)
            if points:
                # calculate distance between p0 and point on curve
                norms = [(p0 - p).norm() for p in points]
                # nearest point
                idx = norms.index(min(norms))
                p = points[idx]
                axes = lstAxes[idx]
                curve = lstCurves[idx]
                coord.setNDC(p.x, p.y)
                coord.setWindow(*axes.getWindow())
                coord.setScale(axes.scale)
            else:
                coord, axes, curve = None, None, None
        gr.setwindow(*window)
        return (coord, axes, curve)

    def select(self, p0, p1, width, height):
        window = gr.inqwindow()
        coord = CoordConverter(self._sizex, self._sizey)
        for axes in self._lstAxes:
            win = axes.getWindow()
            coord.setWindow(*win)
            coord.setScale(axes.scale)
            gr.setwindow(*win)
            gr.setscale(axes.scale)
            p0World = coord.setNDC(p0.x, p0.y).getWC(self.viewport)
            p1World = coord.setNDC(p1.x, p1.y).getWC(self.viewport)
            xmin = min(p0World.x, p1World.x)
            xmax = max(p0World.x, p1World.x)
            ymin = min(p0World.y, p1World.y)
            ymax = max(p0World.y, p1World.y)
            axes.setWindow(xmin, xmax, ymin, ymax)
        gr.setwindow(*window)

    def pan(self, dp, width, height):
        window = gr.inqwindow()
        coord = CoordConverter(self._sizex, self._sizey)
        for axes in self._lstAxes:
            win = axes.getWindow()
            coord.setWindow(*win)
            coord.setScale(axes.scale)
            xmin, xmax, ymin, ymax = win
            gr.setwindow(*win)
            gr.setscale(axes.scale)
            pmin = coord.setWC(xmin, ymin, axes.viewport, win).getNDC()
            pmax = coord.setWC(xmax, ymax, axes.viewport, win).getNDC()
            ndcWin = [pmin.x - dp.x, pmax.x - dp.x,
                      pmin.y - dp.y, pmax.y - dp.y]
            wmin = coord.setNDC(ndcWin[0], ndcWin[2]).getWC(axes.viewport)
            wmax = coord.setNDC(ndcWin[1], ndcWin[3]).getWC(axes.viewport)
            win = [wmin.x, wmax.x, wmin.y, wmax.y]
            axes.setWindow(*win)
        gr.setwindow(*window)

    def zoom(self, dpercent, p0=None, width=None, height=None):
        window = gr.inqwindow()
        coord = CoordConverter(self._sizex, self._sizey)
        for axes in self._lstAxes:
            win = axes.getWindow()
            xmin, xmax, ymin, ymax = win
            coord.setWindow(*win)
            coord.setScale(axes.scale)
            gr.setwindow(*win)
            gr.setscale(axes.scale)
            # zoom from center
            pmin = coord.setWC(xmin, ymin, axes.viewport, win).getNDC()
            pmax = coord.setWC(xmax, ymax, axes.viewport, win).getNDC()
            winWidth = pmax.x - pmin.x
            winWidth_2 = winWidth / 2
            winHeight = pmax.y - pmin.y
            winHeight_2 = winHeight / 2
            dx_2 = winWidth_2 - (1 + dpercent) * winWidth_2
            dy_2 = winHeight_2 - (1 + dpercent) * winHeight_2
            ndcWin = [pmin.x - dx_2, pmax.x + dx_2,
                      pmin.y - dy_2, pmax.y + dy_2]
            # calculate ratio and adjust to ratio / zoom at origin p0
            if p0 is not None and width is not None and height is not None:
                ratio_xleft = (p0.x - pmin.x) / winWidth
                ratio_ydown = (p0.y - pmin.y) / winHeight
                # calculate new xmin, xmax, ymin, ymax with same ratio
                xmin_n = p0.x - ratio_xleft * (ndcWin[1] - ndcWin[0])
                xmax_n = xmin_n + (ndcWin[1] - ndcWin[0])
                ymin_n = p0.y - ratio_ydown * (ndcWin[3] - ndcWin[2])
                ymax_n = ymin_n + (ndcWin[3] - ndcWin[2])
                ndcWin = [xmin_n, xmax_n, ymin_n, ymax_n]
            wmin = coord.setNDC(ndcWin[0], ndcWin[2]).getWC(self.viewport)
            wmax = coord.setNDC(ndcWin[1], ndcWin[3]).getWC(self.viewport)
            win = [wmin.x, wmax.x, wmin.y, wmax.y]
            gr.setwindow(*window)
            if not axes.setWindow(*win):
                axes.setWindow(*window)
                self.reset()
                break

    def addROI(self, roi):
        if roi not in self._rois:
            self._rois.append(roi)
            _log.debug("#rois: %d", len(self._rois))
        else:
            _log.debug("found existing roi")

    def getROI(self, p0):
        res = None
        for roi in self._legendROI + self._rois:
            if roi.isPointInside(p0):
                res = roi
        return res

    def addAxes(self, *args, **kwargs):
        for plotAxes in args:
            if plotAxes and plotAxes not in self._lstAxes:
                plotAxes.viewport = self.viewport
                self._lstAxes.append(plotAxes)
                # overwrite ids (1: first axis, 2: second axis)
                self._countAxes += 1
                plotAxes._id = self._countAxes
        return self

    def drawGR(self):
        [xmin, xmax, ymin, ymax] = self.viewportscaled
         # redraw flag will be set if viewport has been adjusted in order to
         # display all legend items.
        redraw = False
        # -- draw title and subtitle -----------------------------------------
        charHeight = .027 * (ymax - ymin)
        charHeightUnscaled = .027 * (self.viewport[3] - self.viewport[2])
        if self.title or self.subTitle:
            dyTitle = 0
            dySubTitle = 0
            gr.settextalign(gr.TEXT_HALIGN_CENTER, gr.TEXT_VALIGN_BOTTOM)
            gr.setcharup(0., 1.)
            gr.setcharheight(charHeight)
            x = xmin + (xmax - xmin) / 2
            if self.title:
                dyTitle = charHeightUnscaled
            if self.title and self.subTitle:
                dySubTitle = charHeightUnscaled
            y = (self.viewport[3] + dyTitle + dySubTitle + charHeightUnscaled
                 + 0.01)

            if self.title:
                gr.text(x, y * self.sizey, self.title)
                y -= dyTitle + 0.01
            if self.subTitle:
                gr.text(x, y * self.sizey, self.subTitle)
        # -- draw axes and curves --------------------------------------------
        if self._lstAxes:
            for axes in self._lstAxes:
                axes.drawGR()
        dyXLabel = 0
        dxYLabel = 0
        # -- draw x- and y label ---------------------------------------------
        y = self.offsetXLabel + self.viewport[2]
        if self.xlabel:
            gr.settextalign(gr.TEXT_HALIGN_CENTER, gr.TEXT_VALIGN_TOP)
            gr.setcharup(0., 1.)
#            tby = gr.inqtext(0, 0, self.xlabel)[1]
#            tby = map(lambda y: gr.wctondc(0, y)[1], tby)
#            dyXLabel = max(tby) - min(tby) # already scaled
            dyXLabel = charHeightUnscaled
            y -= dyXLabel + charHeightUnscaled
            gr.text(xmin + (xmax - xmin) / 2., y * self.sizey, self.xlabel)
        if self.ylabel:
            gr.settextalign(gr.TEXT_HALIGN_CENTER, gr.TEXT_VALIGN_TOP)
            gr.setcharup(-1., 0.)
            tbx = gr.inqtext(0, 0, self.ylabel)[0]
            tbx = [gr.wctondc(0, yi)[0] for yi in tbx]
            dxYLabel = max(tbx) - min(tbx)
            gr.text(self.offsetYLabel + xmin - dxYLabel / 2. - .075,
                    ymin + (ymax - ymin) / 2., self.ylabel)
            gr.setcharup(0., 1.)
        # -- draw legend and calculate ROIs for legend items -----------------
        if self._legend:
            x, y = xmin, y - dyXLabel - charHeightUnscaled
            if y < 0:
                vp = self.viewport
                vp[2] += dyXLabel
                self.viewport = vp # propagate update (calls setter)
                gr.clearws()
                self.drawGR()
            else:
                # preserve old values
                ltype = gr.inqlinetype()
                mtype = gr.inqmarkertype()
                lcolor = gr.inqlinecolorind()
                mcolor = gr.inqmarkercolorind()
                lwidth = gr.inqlinewidth()
                window = gr.inqwindow()
                scale = gr.inqscale()
                gr.setviewport(0, self.sizex, 0, self.sizey)
                gr.setwindow(0, self.sizex, 0, self.sizey)
                self._legendROI = []
                for axes in self._lstAxes:
                    if redraw:
                        break
                    for curve in axes.getCurves():
                        if curve.legend:
                            tbx, tby = gr.inqtext(0, 0, curve.legend)
                            textWidth = max(tbx) - min(tbx)
                            lineWidth = self._legendWidth
                            if x + lineWidth + textWidth > self.sizex:
                                x = xmin
                                y -= 2 * charHeightUnscaled
                                if y < 0:
                                    vp = self.viewport
                                    vp[2] += 3 * charHeightUnscaled
                                    # propagate update (calls setter)
                                    self.viewport = vp
                                    redraw = True
                                    break
                            gr.setmarkertype(curve.markertype)
                            gr.setmarkercolorind(curve.markercolor)
                            ys = y * self.sizey # scaled y value
                            if curve.linetype is not None:
                                gr.setlinecolorind(curve.linecolor)
                                gr.setmarkercolorind(curve.markercolor)
                                gr.setlinetype(curve.linetype)
                                gr.setlinewidth(curve.linewidth)
                                gr.polyline([x, x + lineWidth], [ys, ys])
                                if (curve.markertype != gr.MARKERTYPE_DOT
                                    and curve.markertype is not None):
                                    gr.setmarkertype(curve.markertype)
                                    gr.polymarker([x + lineWidth / 2.], [ys])
                            elif curve.markertype is not None:
                                gr.setmarkertype(curve.markertype)
                                gr.polymarker([x + lineWidth / 2.], [ys])

                            ybase = (y - charHeightUnscaled / 2) * self.sizey
                            ytop = (y + charHeightUnscaled / 2) * self.sizey
                            roi = RegionOfInterest(Point(x, ybase),
                                                   Point(x, ytop),
                                                   reference=curve,
                                           regionType=RegionOfInterest.LEGEND)
                            x += lineWidth + .01
                            if curve.visible:
                                gr.settextcolorind(1)
                            else:
                                gr.settextcolorind(83)
                            gr.settextalign(gr.TEXT_HALIGN_LEFT,
                                            gr.TEXT_VALIGN_HALF)
                            gr.text(x, ys, curve.legend)
                            gr.settextcolorind(1)
                            x += textWidth
                            roi.append(Point(x, ytop), Point(x, ybase))
                            self._legendROI.append(roi)
#                            gr.polyline(roi.x, roi.y)
                            tbx = gr.inqtext(0, 0, "X")[0]
                            charWidth = max(tbx) - min(tbx)
                            x += charWidth
                # restore old values
                gr.setlinecolorind(lcolor)
                gr.setmarkercolorind(mcolor)
                gr.setlinetype(ltype)
                gr.setmarkertype(mtype)
                gr.setlinewidth(lwidth)
                # restore viewport and window
                gr.setviewport(*self.viewportscaled)
                gr.setwindow(*window)
                # restore scale
                gr.setscale(scale)
        if redraw:
            gr.clearws()
            self.drawGR()


def isinstanceof(other, cls):
    if not isinstance(other, cls):
        raise ValueError("%s is not an instance of %s." % (type(other),
                                                           cls.__name__))


def islistof(otheriter, cls):
    if isinstance(otheriter, collections.Iterable):
        for other in otheriter:
            if not isinstance(other, cls):
                raise ValueError("Iterable contains %s " % type(other) +
                                 "which is not an instance of %s."
                                 % cls.__name__)


class Coords2DList(list):

    TYPE = Coords2D

    def __init__(self, *args, **kwargs):
        list.__init__(self, *args, **kwargs)
        islistof(self, self.TYPE)
        self.xmin, self.xmax, self.ymin, self.ymax = None, None, None, None
        self.updateMinMax(reset=True)

    def _calcMinMax(self, coords2d, *args):
        coords = (coords2d, ) + args
        xmin = min(min(coord.x) for coord in coords)
        xmax = max(max(coord.x) for coord in coords)
        ymin = min(min(coord.y) for coord in coords)
        ymax = max(max(coord.y) for coord in coords)
        return xmin, xmax, ymin, ymax

    def updateMinMax(self, *coords, **kwargs):
        reset = kwargs.pop("reset", False)
        if not coords:
            coords = self
        if coords:
            xmin, xmax, ymin, ymax = self._calcMinMax(*coords)
            if reset:
                self.xmin, self.xmax = xmin, xmax
                self.ymin, self.ymax = ymin, ymax
            else:
                self.xmin = (min(self.xmin, xmin) if self.xmin is not None
                             else xmin)
                self.xmax = (max(self.xmax, xmax) if self.xmax is not None
                             else xmax)
                self.ymin = (min(self.ymin, ymin) if self.ymin is not None
                             else ymin)
                self.ymax = (max(self.ymax, ymax) if self.ymax is not None
                             else ymax)
        else:
            self.xmin, self.xmax, self.ymin, self.ymax = None, None, None, None

    def __iadd__(self, otheriter):
        islistof(otheriter, self.TYPE)
        res = list.__iadd__(self, otheriter)
        self.updateMinMax(*otheriter)
        return res

    def append(self, other):
        isinstanceof(other, self.TYPE)
        res = list.append(self, other)
        self.updateMinMax(other)
        return res

    def extend(self, otheriter):
        islistof(otheriter, self.TYPE)
        res = list.extend(self, otheriter)
        self.updateMinMax(*otheriter)
        return res

    def insert(self, index, other):
        isinstanceof(other, self.TYPE)
        res = list.insert(self, index, other)
        self.updateMinMax(other)
        return res

    def pop(self, *args, **kwargs):
        other = list.pop(self, *args, **kwargs)
        self.updateMinMax(reset=True)
        return other

    def remove(self, other):
        res = list.remove(self, other)
        self.updateMinMax(reset=True)
        return res


class Coords3DList(Coords2DList):

    TYPE = Coords3D

    def __init__(self, *args, **kwargs):
        self.zmin, self.zmax = None, None
        Coords2DList.__init__(self, *args, **kwargs)

    def updateMinMax(self, *coords, **kwargs):
        Coords2DList.updateMinMax(self, *coords, **kwargs)
        reset = kwargs.pop("reset", False)
        if not coords:
            coords = self
        if coords:
            zmin = min(min(coord.z) for coord in coords)
            zmax = max(max(coord.z) for coord in coords)
            if reset:
                self.zmin, self.zmax = zmin, zmax
            else:
                self.zmin = (min(self.zmin, zmin) if self.zmin is not None
                             else zmin)
                self.zmax = (max(self.zmax, zmax) if self.zmax is not None
                             else zmax)
        else:
            self.zmin, self.zmax, = None, None


class PlotCurve(GRDrawAttributes, GRVisibility, Coords2D, GRMeta):

    COUNT = 0

    def __init__(self, x, y, errBar1=None, errBar2=None,
                 linetype=gr.LINETYPE_SOLID, markertype=gr.MARKERTYPE_DOT,
                 linecolor=None, markercolor=1, legend=None, linewidth=1):
        GRDrawAttributes.__init__(self, linetype, markertype, linecolor,
                                  markercolor, linewidth)
        GRVisibility.__init__(self, True)
        Coords2D.__init__(self, x, y)
        self._e1, self._e2 = errBar1, errBar2
        self._legend = legend
        PlotCurve.COUNT += 1
        self._id = PlotCurve.COUNT
        if legend is None:
            self._legend = "curve %d" % self._id

    @property
    def errorBar1(self):
        """Get current ErrorBar #1 instance."""
        return self._e1

    @errorBar1.setter
    def errorBar1(self, value):
        self._e1 = value

    @property
    def errorBar2(self):
        """Get current ErrorBar #2 instance."""
        return self._e2

    @errorBar2.setter
    def errorBar2(self, value):
        self._e2 = value

    @property
    def legend(self):
        """Get current text for a legend."""
        return self._legend

    @legend.setter
    def legend(self, s):
        self._legend = s

    def drawGR(self):
        if self.visible:
            # preserve old values
            ltype = gr.inqlinetype()
            mtype = gr.inqmarkertype()
            lcolor = gr.inqlinecolorind()
            mcolor = gr.inqmarkercolorind()
            lwidth = gr.inqlinewidth()

            if self.linetype is not None and len(self.x) > 1:
                gr.setlinecolorind(self.linecolor)
                gr.setmarkercolorind(self.markercolor)
                gr.setlinetype(self.linetype)
                gr.setlinewidth(self.linewidth)
                gr.polyline(self.x, self.y)
                if (self.markertype != gr.MARKERTYPE_DOT and
                    self.markertype is not None):
                    gr.setmarkertype(self.markertype)
                    gr.polymarker(self.x, self.y)
            elif self.markertype is not None:
                gr.setmarkercolorind(self.markercolor)
                gr.setmarkertype(self.markertype)
                gr.polymarker(self.x, self.y)
            if self.errorBar1:
                self.errorBar1.drawGR()
            if self.errorBar2:
                self.errorBar2.drawGR()
            # restore old values
            gr.setlinecolorind(lcolor)
            gr.setmarkercolorind(mcolor)
            gr.setlinetype(ltype)
            gr.setmarkertype(mtype)
            gr.setlinewidth(lwidth)


class PlotSurface(GRVisibility, GridCoords3D, GRMeta):

    def __init__(self, x, y, z, colormap=gr.COLORMAP_TEMPERATURE,
                 option=gr.OPTION_CELL_ARRAY, nx=None, ny=None):
        GRVisibility.__init__(self, True)
        GridCoords3D.__init__(self, x, y, z, nx, ny)
        self._colormap, self._option = colormap, option

    @property
    def colormap(self):
        """Get the current colormap."""
        return self._colormap

    @colormap.setter
    def colormap(self, cmap):
        self._colormap = cmap

    @property
    def option(self):
        """Get current surface display option."""
        return self._option

    @option.setter
    def option(self, opt):
        self._option = opt

    def drawGR(self):
        if self.visible:
            gr.setspace(min(self.z), max(self.z), 0, 90)
            gr.setcolormap(self.colormap)
            gr3.surface(self.x, self.y, self.z, self.option)


class PlotContour(GRVisibility, GridCoords3D, GRMeta):

    def __init__(self, x, y, z, h=None, majorh=0, nx=None, ny=None):
        GRVisibility.__init__(self, True)
        GridCoords3D.__init__(self, x, y, z, nx, ny)
        self._h, self._majorh = h or [], majorh

    @property
    def h(self):
        """Get the current list/ndarray of z values for the height."""
        return self._h

    @h.setter
    def h(self, lst):
        self._h = lst

    @property
    def majorh(self):
        """Get the interval of labeled contour lines majorh. A value of 1 will
        label every line. A value of 2 every second line and so on. A value of
        0 disables labeling"""
        return self._majorh

    @majorh.setter
    def majorh(self, mh):
        self._majorh = mh

    def drawGR(self):
        if self.visible:
            gr.setspace(min(self.z), max(self.z), 0, 90)
            gr.contour(self.x, self.y, self.h, self.z, self.majorh)


class PlotAxes(GRViewPort, GRMeta):

    COUNT = 0
    SCALE_X = 0x1
    SCALE_Y = 0x2
    OPTION  = {
        SCALE_X: gr.OPTION_X_LOG,
        SCALE_Y: gr.OPTION_Y_LOG,
    }
    COORDLIST_CLASS = Coords2DList

    def __init__(self, viewport, xtick=None, ytick=None, majorx=None,
                 majory=None, drawX=True, drawY=True):
        super(PlotAxes, self).__init__(viewport)
        self._xtick, self._ytick = xtick, ytick
        self._majorx, self._majory = None, None
        self.majorx, self.majory = majorx, majory
        self._drawX, self._drawY = drawX, drawY
        self._curves = self.COORDLIST_CLASS()
        self._visibleCurves = self.COORDLIST_CLASS()
        self._backgroundColor = 0
        self._window = None
        self._scale = 0
        self._grid = True
        self._autoscale = 0x0
        PlotAxes.COUNT += 1
        self._id = PlotAxes.COUNT
        self._xtick_callback, self._ytick_callback = None, None

    def getVisibleCurves(self):
        return self._visibleCurves

    def getCurves(self):
        return self._curves

    def isXLogDomain(self):
        window = self.getWindow()
        if window:
            return DomainChecker.isInLogDomain(window[0], window[1])
        else:
            return False

    def isYLogDomain(self):
        window = self.getWindow()
        if window:
            return DomainChecker.isInLogDomain(window[2], window[3])
        else:
            return False

    @property
    def xtick(self):
        """get current user-defined interval between minor x ticks"""
        if self.scale & gr.OPTION_X_LOG:
            xtick = 1
        elif self._xtick is not None:
            xtick = self._xtick
        else:
            win = self.getWindow()
            if win:
                xmin, xmax = win[:2]
                xtick = gr.tick(xmin, xmax) / self.majorx
            else:
                xtick = None
        return xtick

    @xtick.setter
    def xtick(self, tickInterval):
        self._xtick = tickInterval

    @property
    def ytick(self):
        """get current user-defined interval between minor y ticks"""
        if self.scale & gr.OPTION_Y_LOG:
            ytick = 1
        elif self._ytick is not None:
            ytick = self._ytick
        else:
            win = self.getWindow()
            if win:
                ymin, ymax = win[2:]
                ytick = gr.tick(ymin, ymax) / self.majory
            else:
                ytick = None
        return ytick

    @ytick.setter
    def ytick(self, tickInterval):
        self._ytick = tickInterval

    @property
    def majorx(self):
        """get current user-defined number of minor tick intervals between
        major x tick marks"""
        if self.scale & gr.OPTION_X_LOG:
            mx = 1
        elif self._majorx is not None:
            mx = self._majorx
        else:
            mx = 5
        return mx

    @majorx.setter
    def majorx(self, minorCount):
        self._majorx = minorCount if minorCount is None or minorCount > 0 else 1

    @property
    def majory(self):
        """get current user-defined number of minor tick intervals between
        major y tick marks"""
        if self.scale & gr.OPTION_Y_LOG:
            my = 1
        elif self._majory is not None:
            my = self._majory
        else:
            my = 5
        return my

    @majory.setter
    def majory(self, minorCount):
        self._majory = minorCount if minorCount is None or minorCount > 0 else 1

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

    def _adjustLogRange(self, amin, amax, scale):
        # minimum window borders for log scale
        wmin, wmax = gr.precision * 10, gr.precision * 100
        if scale in PlotAxes.OPTION:
            if self.scale & PlotAxes.OPTION[scale]:
                if amax < wmax:
                    amax = wmax
                if amin < wmin:
                    if self.autoscale & scale:
                        amin = 1. / 10 ** max(0, 3 - math.log10(amax))
                    else:
                        amin = wmin
            if gr.validaterange(amin, amax):
                return amin, amax
            raise ValueError("amin, amax not in window domain.")
        raise ValueError("Unknown value '%s' for argument scale." % scale)

    def _adjustWindow(self, xmin, xmax, ymin, ymax):
        xmin, xmax = self._adjustLogRange(xmin, xmax, PlotAxes.SCALE_X)
        ymin, ymax = self._adjustLogRange(ymin, ymax, PlotAxes.SCALE_Y)
        return xmin, xmax, ymin, ymax

    def setWindow(self, xmin, xmax, ymin, ymax):
        res = True
        try:
            self._window = self._adjustWindow(xmin, xmax, ymin, ymax)
        except ValueError:
            res = False
            _log.debug("xmin, xmax, ymin, ymax not in window domain.")
        return res

    def getWindow(self):
        if self._window:
            return list(self._window)
        else:
            return None

    def setXtickCallback(self, fp):
        self._xtick_callback = fp

    def setYtickCallback(self, fp):
        self._ytick_callback = fp

    def _calcWindowForCurves(self, curves, xmin, xmax, ymin, ymax):
        # take error bars into account
        # if error bar's center is in current window
        # adjust window margins to contain error bars.
        for curve in curves:
            if isinstance(curve, PlotCurve):
                for bar in [curve.errorBar1, curve.errorBar2]:
                    if bar:
                        if bar.direction == ErrorBar.HORIZONTAL:
                            for i, x in enumerate(bar.x):
                                if x >= xmin and x <= xmax:
                                    bneg = x - bar.dneg[i]
                                    bpos = x + bar.dpos[i]
                                    if bneg < xmin:
                                        xmin = bneg
                                    if bpos > xmax:
                                        xmax = bpos
                        elif bar.direction == ErrorBar.VERTICAL:
                            for i, y in enumerate(bar.y):
                                if y >= ymin and y <= ymax:
                                    bneg = y - bar.dneg[i]
                                    bpos = y + bar.dpos[i]
                                    if bneg < ymin:
                                        ymin = bneg
                                    if bpos > ymax:
                                        ymax = bpos
        return xmin, xmax, ymin, ymax

    def getBoundingBox(self):
        res = None
        vcurves = self.getVisibleCurves() or self.getCurves()
        if vcurves:
            res = self._calcWindowForCurves(vcurves, vcurves.xmin, vcurves.xmax,
                                            vcurves.ymin, vcurves.ymax)
        return res

    def reset(self):
        bbox = self.getBoundingBox()
        if bbox:
            xmin, xmax, ymin, ymax = bbox

            if math.fabs(xmax - xmin) < gr.precision:
                if not gr.validaterange(xmin, xmax):
                    xmin, xmax = gr.adjustrange(xmin, xmax)
            elif not self.autoscale and self.scale & gr.OPTION_X_LOG == 0:
                xmin, xmax = gr.adjustrange(xmin, xmax)
            if math.fabs(ymax - ymin) < gr.precision:
                if not gr.validaterange(ymin, ymax):
                    ymin, ymax = gr.adjustrange(ymin, ymax)
            elif not self.autoscale and self.scale & gr.OPTION_Y_LOG == 0:
                ymin, ymax = gr.adjustrange(ymin, ymax)

            self.setWindow(xmin, xmax, ymin, ymax)
            if self.autoscale:
                self.doAutoScale()

    def doAutoScale(self, curvechanged=None):
        win = self.getWindow()
        # global xmin, xmax, ymin, ymax
        vc = self.getVisibleCurves()
        if not vc:
            vc = self.getCurves()
        if win:
            # calculate previous xmin, xmax
            xpmin, xpmax = vc.xmin, vc.xmax
            # update vc min max / calculate current xmin, xmax[, ymin, ymax]
            vc.updateMinMax(reset=True)
            xmin, xmax, ymin, ymax = vc.xmin, vc.xmax, vc.ymin, vc.ymax
            yl = curvechanged.y[-1] if curvechanged else vc.ymax

            # scaled values for xmin, xmax and last added y point
            sxmin, sxmax, symin, symax = self._scaleWindow(xmin, xmax,
                                                           self.xtick, yl,
                                                           yl, self.ytick)
            if self.autoscale & PlotAxes.SCALE_X and xmin != xmax:
                if xmin < xpmin:
                    # growing in negative direction
                    # hold max value
                    if xmax >= win[1]:
                        # win[1]: user max value
                        xmax = win[1]
                    xmin = sxmin
                elif xmax > xpmax:
                    # growing in positive direction
                    # hold min value
                    if xmin <= win[0]:
                        # win[0]: user min value
                        xmin = win[0]
                    xmax = sxmax
                else:
                    # no auto scaling in x keep previous values
                    xmin, xmax = win[0], win[1]

                if xmin > win[0]:
                    xmin = win[0]
                if xmax < win[1]:
                    xmax = win[1]
            else:
                xmin, xmax = win[0], win[1]

            if self.autoscale & PlotAxes.SCALE_Y and ymin != ymax:
                # compare y components of current window with last added y point
                ymin = symin if symin < win[2] else win[2]
                ymax = symax if symax > win[3] else win[3]
            else:
                # no auto scaling in x keep previous values
                ymin, ymax = win[2], win[3]

            # add border to calculated global curves min max
            # taking error bars into account.
            vxmin, vxmax, vymin, vymax = self._calcWindowForCurves(vc, vc.xmin,
                                                                   vc.xmax,
                                                                   vc.ymin,
                                                                   vc.ymax)
            bxmin, bxmax, bymin, bymax = self._scaleWindow(vxmin, vxmax,
                                                           self.xtick, vymin,
                                                           vymax, self.ytick)
            # check whether the calculated window is in the border area
            # and ensure minimal border widths and heights.
            if xmin <= vc.xmin and xmin > bxmin:
                xmin = bxmin
            if xmax >= vc.xmax and xmax < bxmax:
                xmax = bxmax
            if ymin <= vc.ymin and ymin > bymin:
                ymin = bymin
            if ymax >= vc.ymax and ymax < bymax:
                ymax = bymax

            xmin, xmax, ymin, ymax = self._calcWindowForCurves(vc, xmin, xmax,
                                                                ymin, ymax)
            self.setWindow(xmin, xmax, ymin, ymax)
        else:
            # update vc min max / calculate current xmin, xmax, ymin, ymax
            vc.updateMinMax(reset=True)
            self.reset()
        return self.getWindow()

    def isReset(self):
        # obsolete / deprecated
        # reset() now performs the reset immediately.
        return False

    @property
    def autoscale(self):
        return self._autoscale

    @autoscale.setter
    def autoscale(self, mask):
        self._autoscale = mask

    def _scaleWindow(self, xmin, xmax, xtick, ymin, ymax, ytick):
        try:
            return self._adjustWindow(*self.scaleWindow(xmin, xmax, xtick,
                                                        ymin, ymax, ytick))
        except ValueError as e:
            _log.debug("got error in _adjustWindow: %s" % e)
            _log.debug("falling back to input window")
            return xmin, xmax, ymin, ymax

    def scaleWindow(self, xmin, xmax, xtick, ymin, ymax, ytick):
        # can be used to rescale the current window when in autoscale mode,
        # e.g. adding a delta to the bounding box
        return xmin, xmax, ymin, ymax

    def getId(self):
        return self._id

    def drawGR(self):
        curves = self.getCurves()
        viewport = self.viewportscaled
        if curves:
            window = self.getWindow()
            if not window:
                self.reset()
                window = self.getWindow()
            xmin, xmax, ymin, ymax = window

            gr.setviewport(*self.viewportscaled)
            gr.setwindow(*window)
            gr.setscale(self.scale)
            if self.backgroundColor and self.getId() == 1:
                gr.setfillintstyle(1)
                gr.setfillcolorind(self.backgroundColor)
                gr.fillrect(*window)
            charHeight = .024 * (viewport[3] - viewport[2])
            gr.setcharheight(charHeight)
            if self.isGridEnabled() and self.getId() == 1:
                # delta majorx, delta majory
                dmx = self.xtick * self.majorx
                dmy = self.ytick * self.majory
                # xorg, yorg
                i = int(xmin / dmx) if xmin < 0 else int(xmin / dmx + 1)
                j = int(ymin / dmy) if ymin < 0 else int(ymin / dmy + 1)
                xorg = i * dmx
                yorg = j * dmy
                gr.grid(self.xtick, self.ytick, xorg, yorg, self.majorx,
                        self.majory)
            if self.getId() == 1:
                # first x, y axis
                majorx, majory = self.majorx, self.majory
                if not self.isXDrawingEnabled():
                    majorx = -majorx
                if not self.isYDrawingEnabled():
                    majory = -majory
                gr.axeslbl(self.xtick, self.ytick, xmin, ymin, majorx, majory,
                           0.01, self._xtick_callback, self._ytick_callback)
            elif self.getId() == 2:
                # second x, y axis
                majorx, majory = self.majorx, self.majory
                if not self.isXDrawingEnabled():
                    majorx = -majorx
                if not self.isYDrawingEnabled():
                    majory = -majory
                gr.axeslbl(self.xtick, self.ytick, xmax, ymax, majorx, majory,
                           - 0.01, self._xtick_callback,
                           self._ytick_callback)
            for curve in curves:
                curve.drawGR()

    def plot(self, *args, **kwargs):
        if len(args) > 0 and len(args) % 2 == 0:
            self._curves = Coords2DList()
            self._visibleCurves = Coords2DList()
            for i in range(0, len(args) // 2):
                x = args[2 * i]
                y = args[2 * i + 1]
                self.addCurves(PlotCurve(x, y))
        return self

    def addCurves(self, *args, **kwargs):
        for plotCurve in args:
            if plotCurve and plotCurve not in self._curves:
                self._curves.append(plotCurve)
            if plotCurve.visible and plotCurve not in self._visibleCurves:
                self._visibleCurves.append(plotCurve)
            plotCurve.setVisibleCallback(self.curveVisibilityChanged)
            plotCurve.setUpdateXCallback(self.curveDataChanged)
            plotCurve.setUpdateYCallback(self.curveDataChanged)
        return self

    def curveVisibilityChanged(self, curve, flag):
        if curve in self._visibleCurves:
            if not flag:
                self._visibleCurves.remove(curve)
        elif flag:
            self._visibleCurves.append(curve)

    def curveDataChanged(self, curve):
        if len(curve.x) and len(curve.y):
            curves = self.getCurves()
            curves.updateMinMax(reset=True)
            if curve.visible:
                if self.autoscale:
                    self.doAutoScale(curve)
                else:
                    vc = self.getVisibleCurves()
                    vc.updateMinMax(reset=True)


def ndctowc(x, y):
    try:
        iter(x)
        iter(y)
    except TypeError:
        return gr.ndctowc(x, y)
    else:
        resX = []
        resY = []
        for xi, yi in map(gr.ndctowc, x, y): # pylint: disable=bad-builtin
            resX.append(xi)
            resY.append(yi)
        return resX, resY

def wctondc(x, y):
    try:
        iter(x)
        iter(y)
    except TypeError:
        return gr.wctondc(x, y)
    else:
        resX = []
        resY = []
        for xi, yi in map(gr.wctondc, x, y): # pylint: disable=bad-builtin
            resX.append(xi)
            resY.append(yi)
        return resX, resY

def _guessdimension(len):
    x = int(math.sqrt(len))
    d = []
    while x >= 1:
        y = len // x
        if x * y == len:
            d.append((x, y))
        x -= 1
    return sorted(d, key=lambda t: t[1] - t[0])

def readfile(path, separator=''):
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


def delay(seconds):
    time.sleep(seconds)
