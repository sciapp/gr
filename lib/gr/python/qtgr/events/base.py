# -*- coding: utf-8 -*-
"""..."""
# standard library
import logging
# local library
from qtgr.backend import QtCore
from gr.pygr import DeviceCoordConverter
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

class EventMeta(QtCore.QEvent):

    def __init__(self, type):
        if isinstance(type, int):
            type = QtCore.QEvent.Type(type)
        super(EventMeta, self).__init__(type)
        self._type = type

    def type(self):
        return self._type

class MouseLocationEventMeta(EventMeta):

    def __init__(self, type, width, height, x, y, window=None, scale=None):
        super(MouseLocationEventMeta, self).__init__(type)
        self._coords = DeviceCoordConverter(width, height, window=window,
                                            scale=scale)
        self._coords.setDC(x, y)

    def getWindow(self):
        return self._coords.getWindow()

    def getWC(self, viewport):
        return self._coords.getWC(viewport)

    def getNDC(self):
        return self._coords.getNDC()

    def getDC(self):
        return self._coords.getDC()
