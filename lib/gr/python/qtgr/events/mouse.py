# -*- coding: utf-8 -*-

# standard library
import logging
# local library
from gr.pygr.base import GRViewPort
from qtgr.backend import QtCore
from qtgr.events.base import MouseLocationEventMeta
from gr._version import __version__, __revision__

__author__ = "Christian Felder <c.felder@fz-juelich.de>"
__copyright__ = """Copyright (c) 2012-2015: Josef Heinen, Florian Rhiem,
Christian Felder, and other contributors:

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


class MouseEvent(MouseLocationEventMeta):

    MOUSE_MOVE = QtCore.QEvent.registerEventType()
    MOUSE_PRESS = QtCore.QEvent.registerEventType()
    MOUSE_RELEASE = QtCore.QEvent.registerEventType()

    NO_BUTTON = 0x0
    LEFT_BUTTON = 0x1
    RIGHT_BUTTON = 0x2

    NO_MODIFIER = 0x00
    SHIFT_MODIFIER = 0x02
    CONTROL_MODIFIER = 0x04
    ALT_MODIFIER = 0x08
    META_MODIFIER = 0x10
    KEYPAD_MODIFIER = 0x20
    GROUP_SWITCH_MODIFIER = 0x40

    def __init__(self, type, width, height, x, y, buttons, modifiers,
                 window=None, scale=None):
        super(MouseEvent, self).__init__(type, width, height, x, y,
                                         window=window, scale=scale)
        self._buttons = buttons
        self._modifiers = modifiers

    def type(self):
        return self._type

    def getButtons(self):
        return self._buttons

    def getModifiers(self):
        return self._modifiers


class WheelEvent(MouseLocationEventMeta):

    WHEEL_MOVE = QtCore.QEvent.registerEventType()

    def __init__(self, type, width, height, x, y, buttons, delta):
        super(WheelEvent, self).__init__(type, width, height, x, y)
        self._buttons = buttons
        self._delta = delta

    def getDelta(self):
        return self._delta

    def getDegree(self):
        return self.getDelta() / 8.

    def getSteps(self):
        return self.getDegree() / 15.

    def getButtons(self):
        return self._buttons


class PickEvent(MouseLocationEventMeta, GRViewPort):

    PICK_MOVE = QtCore.QEvent.registerEventType()
    PICK_PRESS = QtCore.QEvent.registerEventType()

    def __init__(self, type, width, height, x, y, viewport, window=None,
                 scale=None):
        super(PickEvent, self).__init__(type, width, height, x, y,
                                        window=window, scale=scale)
        GRViewPort.__init__(self, viewport)


class ROIEvent(MouseEvent):

    ROI_CLICKED = QtCore.QEvent.registerEventType()
    ROI_OVER = QtCore.QEvent.registerEventType()

    def __init__(self, type, width, height, x, y, buttons, modifiers, roi):
        super(ROIEvent, self).__init__(type, width, height, x, y, buttons,
                                       modifiers)
        self._roi = roi

    @property
    def roi(self):
        """Get RegionOfInterest instance."""
        return self._roi


class LegendEvent(ROIEvent):

    def __init__(self, type, width, height, x, y, buttons, modifiers, roi):
        super(LegendEvent, self).__init__(type, width, height, x, y, buttons,
                                          modifiers, roi)

    @property
    def curve(self):
        """Get PlotCurve instance referenced by this legend item."""
        return self._roi.reference


class MouseGestureEvent(MouseEvent):

    MOUSE_PAN = QtCore.QEvent.registerEventType()
    MOUSE_SELECT = QtCore.QEvent.registerEventType()

    def __init__(self, type, width, height, x, y, buttons, modifiers, offset,
                 finish=None):
        MouseEvent.__init__(self, type, width, height, x, y, buttons, modifiers)
        self._offset = offset
        self._finish = finish

    def getOffset(self):
        """Get current offset in NDC space."""
        return self._offset

    def isFinished(self):
        return self._finish
