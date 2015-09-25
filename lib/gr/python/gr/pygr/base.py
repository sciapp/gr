# -*- coding: utf-8 -*-
"""..."""
# standard library
import logging
# third party
# ...
# local library
import gr
from gr.pygr.helper import ColorIndexGenerator
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

class GRMeta(object):

    def drawGR(self):
        raise NotImplementedError

class GRDrawAttributes(object):

    def __init__(self, linetype=gr.LINETYPE_SOLID, markertype=gr.MARKERTYPE_DOT,
                 linecolor=None, markercolor=1, linewidth=1):
        self._linetype, self._markertype = linetype, markertype
        self._linecolor, self._markercolor = linecolor, markercolor
        self._linewidth = linewidth
        if linecolor is None:
            self._linecolor = ColorIndexGenerator.nextColorIndex()
        else:
            self._linecolor = linecolor

    @property
    def linetype(self):
        """Get current linetype."""
        return self._linetype

    @linetype.setter
    def linetype(self, value):
        self._linetype = value

    @property
    def linewidth(self):
        """Get current linewidth."""
        return self._linewidth

    @linewidth.setter
    def linewidth(self, value):
        self._linewidth = value

    @property
    def markertype(self):
        """Get current markertype."""
        return self._markertype

    @markertype.setter
    def markertype(self, value):
        self._markertype = value

    @property
    def linecolor(self):
        """Get current linecolor."""
        return self._linecolor

    @linecolor.setter
    def linecolor(self, value):
        self._linecolor = value

    @property
    def markercolor(self):
        """Get current markercolor."""
        return self._markercolor

    @markercolor.setter
    def markercolor(self, value):
        self._markercolor = value

class GRViewPort(object):

    DEFAULT_VIEWPORT = (.1, .95, .1, .95)

    def __init__(self, viewport=DEFAULT_VIEWPORT):
        self._viewport = list(viewport)
        self._sizex, self._sizey = 1., 1.

    @property
    def sizex(self):
        """..."""
        return self._sizex

    @sizex.setter
    def sizex(self, value):
        self._sizex = value

    @property
    def sizey(self):
        """..."""
        return self._sizey

    @sizey.setter
    def sizey(self, value):
        self._sizey = value

    @property
    def viewport(self):
        """get current viewport"""
        return self._viewport

    @viewport.setter
    def viewport(self, viewport):
        self._viewport = list(viewport)

    @property
    def viewportscaled(self):
        vp = list(self.viewport)
        vp[0] *= self.sizex
        vp[1] *= self.sizex
        vp[2] *= self.sizey
        vp[3] *= self.sizey
        return vp


class GRVisibility(object):

    def __init__(self, visible=True):
        self._visible = visible
        self._visible_callback = None

    @property
    def visible(self):
        """Get visibility flag for this GR object."""
        return self._visible

    @visible.setter
    def visible(self, flag):
        self._visible = flag
        if self._visible_callback:
            self._visible_callback(self, flag)

    def setVisibleCallback(self, fp):
        self._visible_callback = fp
