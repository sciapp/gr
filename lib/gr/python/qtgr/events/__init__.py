# -*- coding: utf-8 -*-

# standard library
import logging
# local library
from qtgr.backend import QtCore
from qtgr.events.base import EventMeta, MouseLocationEventMeta
from gr.pygr.base import GRViewPort
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

    def __init__(self, type, width, height, x, y, buttons, modifiers):
        super(MouseEvent, self).__init__(type, width, height, x, y)
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

    def __init__(self, type, width, height, x, y, viewport, window=None):
        super(PickEvent, self).__init__(type, width, height, x, y, window)
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

class EventFilter(QtCore.QObject):

    def __init__(self, parent, manager):
        super(EventFilter, self).__init__(parent)
        self._manager = manager
        self._last_btn_mask = None

    def handleEvent(self, event):
        etype = event.type()
        cls = type(event)
        if self._manager.hasHandler(etype):
            handles = self._manager.getHandler(etype)
        elif self._manager.hasHandler(etype, cls):
            handles = self._manager.getHandler(etype, cls)
        else:
            handles = []
        for handle in handles:
            handle(event)

    def wheelEvent(self, type, target, event):
        """transform QWheelEvent to WheelEvent"""
        btn_mask = MouseEvent.NO_BUTTON
        if event.buttons() & QtCore.Qt.LeftButton:
            btn_mask |= MouseEvent.LEFT_BUTTON
        if event.buttons() & QtCore.Qt.RightButton:
            btn_mask |= MouseEvent.RIGHT_BUTTON

        return WheelEvent(type, target.dwidth, target.dheight, event.x(),
                          event.y(), btn_mask, event.delta())

    def mouseEvent(self, type, target, event):
        """transform QMouseEvent to MouseEvent"""
        btn_mask = MouseEvent.NO_BUTTON
        mod_mask = MouseEvent.NO_MODIFIER
        if event.buttons() & QtCore.Qt.LeftButton:
            btn_mask |= MouseEvent.LEFT_BUTTON
        if event.buttons() & QtCore.Qt.RightButton:
            btn_mask |= MouseEvent.RIGHT_BUTTON

        # special case: store last btn_mask in MouseEvent of type MOUSE_RELEASE
        #               to indicate which button has been released.
        if (type == MouseEvent.MOUSE_RELEASE and
            btn_mask == MouseEvent.NO_BUTTON):
            btn_mask = self._last_btn_mask

        if event.modifiers() & QtCore.Qt.ShiftModifier:
            mod_mask |= MouseEvent.SHIFT_MODIFIER
        if event.modifiers() & QtCore.Qt.ControlModifier:
            mod_mask |= MouseEvent.CONTROL_MODIFIER
        if event.modifiers() & QtCore.Qt.AltModifier:
            mod_mask |= MouseEvent.ALT_MODIFIER
        if event.modifiers() & QtCore.Qt.MetaModifier:
            mod_mask |= MouseEvent.META_MODIFIER
        if event.modifiers() & QtCore.Qt.KeypadModifier:
            mod_mask |= MouseEvent.KEYPAD_MODIFIER
        if event.modifiers() & QtCore.Qt.GroupSwitchModifier:
            mod_mask |= MouseEvent.GROUP_SWITCH_MODIFIER

        mEvent = MouseEvent(type, target.dwidth, target.dheight,
                            event.x(), event.y(), btn_mask, mod_mask)
        # special case:
        # save last btn_mask for handling in MouseEvent.MOUSE_RELEASE
        self._last_btn_mask = btn_mask
        return mEvent

    def eventFilter(self, target, event):
        type = event.type()
        newevent = None
        if type == QtCore.QEvent.MouseMove:
            newevent = self.mouseEvent(MouseEvent.MOUSE_MOVE, target, event)
        elif type == QtCore.QEvent.MouseButtonPress:
            newevent = self.mouseEvent(MouseEvent.MOUSE_PRESS, target, event)
        elif type == QtCore.QEvent.MouseButtonRelease:
            newevent = self.mouseEvent(MouseEvent.MOUSE_RELEASE, target, event)
        elif type == QtCore.QEvent.Wheel:
            newevent = self.wheelEvent(WheelEvent.WHEEL_MOVE, target, event)
        else:
            self.handleEvent(event)
        if newevent:
            return QtCore.QCoreApplication.sendEvent(target, newevent)
        return False

class CallbackManager(object):

    def __init__(self):
        self._handler = {}

    def hasHandler(self, type, cls=None):
        return (type, cls) in self._handler

    def addHandler(self, type, handle, cls=None):
        if self.hasHandler(type, cls):
            self._handler[type, cls].append(handle)
        else:
            self._handler[type, cls] = [handle]

    def removeHandler(self, type, handle, cls=None):
        """

        @raise   KeyError: if type not in self._handler
        @raise ValueError: if handle not in self._handler[type]

        """
        self._handler[type, cls].remove(handle)
        if not self._handler[type, cls]: # if empty
            self._handler.pop((type, cls))

    def getHandler(self, type, cls=None):
        """

        @raise KeyError: if type not in self._handler

        """
        return self._handler[type, cls]

class GUIConnector(object):

    def __init__(self, parent):
        self._manager = CallbackManager()
        self._eventFilter = EventFilter(parent, self._manager)
        parent.installEventFilter(self._eventFilter)

    def connect(self, type, handle, cls=None):
        self._manager.addHandler(type, handle, cls)

    def disconnect(self, type, handle, cls=None):
        self._manager.removeHandler(type, handle, cls)
