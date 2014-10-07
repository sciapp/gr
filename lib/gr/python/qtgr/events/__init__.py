# -*- coding: utf-8 -*-

# standard library
import math
import logging
# third party
def _importPySide():
    global QtCore
    from PySide import QtCore

def _importPyQt4():
    global QtCore
    from PyQt4 import QtCore

from qtgr.backend import QT_BACKEND_ORDER, QT_PYSIDE, QT_PYQT4
_imp = {QT_PYSIDE: _importPySide,
        QT_PYQT4: _importPyQt4}
try:
    _imp[QT_BACKEND_ORDER[0]]()
except ImportError:
    _imp[QT_BACKEND_ORDER[1]]()
# local library
import gr
import qtgr
from qtgr.events.base import EventMeta, MouseLocationEventMeta
from gr.pygr.base import GRViewPort
from gr._version import __version__, __revision__

__author__ = "Christian Felder <c.felder@fz-juelich.de>"
__copyright__ = """Copyright 2012-2014 Forschungszentrum Juelich GmbH

This file is part of GR, a universal framework for visualization applications.
Visit http://gr-framework.org for the latest
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
        if self._manager.hasHandler(event.type()):
            for handle in self._manager.getHandler(event.type()):
                handle(event)

    def wheelEvent(self, type, target, event):
        """transform QWheelEvent to WheelEvent"""
        btn_mask = MouseEvent.NO_BUTTON
        if event.buttons() & QtCore.Qt.LeftButton:
            btn_mask |= MouseEvent.LEFT_BUTTON
        if event.buttons() & QtCore.Qt.RightButton:
            btn_mask |= MouseEvent.RIGHT_BUTTON

        wEvent = WheelEvent(type, target.dwidth, target.dheight, event.x(),
                            event.y(), btn_mask, event.delta())
        self.handleEvent(wEvent)

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
        self.handleEvent(mEvent)
        # special case:
        # save last btn_mask for handling in MouseEvent.MOUSE_RELEASE
        self._last_btn_mask = btn_mask

    def eventFilter(self, target, event):
        type = event.type()
        if type == QtCore.QEvent.MouseMove:
            self.mouseEvent(MouseEvent.MOUSE_MOVE, target, event)
        elif type == QtCore.QEvent.MouseButtonPress:
            self.mouseEvent(MouseEvent.MOUSE_PRESS, target, event)
        elif type == QtCore.QEvent.MouseButtonRelease:
            self.mouseEvent(MouseEvent.MOUSE_RELEASE, target, event)
        elif type == QtCore.QEvent.Wheel:
            self.wheelEvent(WheelEvent.WHEEL_MOVE, target, event)
        else:
            self.handleEvent(event)

        return False

class CallbackManager(object):

    def __init__(self):
        self._handler = {}

    def hasHandler(self, type):
        return type in self._handler

    def addHandler(self, type, handle):
        if self.hasHandler(type):
            self._handler[type].append(handle)
        else:
            self._handler[type] = [handle]

    def removeHandler(self, type, handle):
        """
        
        @raise   KeyError: if type not in self._handler
        @raise ValueError: if handle not in self._handler[type]
        
        """
        self._handler[type].remove(handle)
        if not self._handler[type]: # if empty
            self._handler.pop(type)

    def getHandler(self, type):
        """
        
        @raise KeyError: if type not in self._handler
        
        """
        return self._handler[type]

class GUIConnector(object):

    def __init__(self, parent):
        self._manager = CallbackManager()
        self._eventFilter = EventFilter(parent, self._manager)
        parent.installEventFilter(self._eventFilter)

    def connect(self, type, handle):
        self._manager.addHandler(type, handle)

    def disconnect(self, type, handle):
        self._manager.removeHandler(type, handle)
