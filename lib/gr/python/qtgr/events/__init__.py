# -*- coding: utf-8 -*-

# standard library
import logging
# local library
import gr
from gr.pygr import DeviceCoordConverter
from qtgr.backend import QtCore
from qtgr.events.mouse import MouseEvent, WheelEvent, PickEvent, LegendEvent,\
    ROIEvent, MouseGestureEvent
from qtgr.events.gestures import MouseGestureBase, PanGesture, SelectGesture
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

        # In order to support multiple plots in one widget
        # it is necessary to set the window in respect to the current
        # `PlotAxes` below the cursor. Otherwise the window will be determined
        # using the internal state machine of ``gr``.
        coords = DeviceCoordConverter(target.dwidth, target.dheight)
        coords.setDC(event.x(), event.y())
        plots = target._getPlotsForPoint(coords.getNDC())
        window, scale = None, None
        if plots and len(plots) == 1:  # unambitious plot
            axes = plots[0].getAxes(0)  # use first `PlotAxes`
            if axes:
                window = axes.getWindow()
                scale = axes.scale
                gr.setscale(axes.scale)

        mEvent = MouseEvent(type, target.dwidth, target.dheight,
                            event.x(), event.y(), btn_mask, mod_mask,
                            window=window, scale=scale)
        # special case:
        # save last btn_mask for handling in MouseEvent.MOUSE_RELEASE
        self._last_btn_mask = btn_mask
        return mEvent

    def handleGesture(self, target, event):
        """Intercept `QtCore.QEvent.Gesture` events and post new events to the
        Event-Loop."""
        gestures = event.gestures()
        _log.debug("handleGesture: gestures: %r", gestures)
        for g in gestures:
            if isinstance(g, MouseGestureBase):
                etype = None
                finish = (g.state() == QtCore.Qt.GestureFinished)
                p0 = g.startPoint.getDC()
                if isinstance(g, PanGesture):
                    etype = MouseGestureEvent.MOUSE_PAN
                elif isinstance(g, SelectGesture):
                    etype = MouseGestureEvent.MOUSE_SELECT
                else:
                    raise ValueError("Unknown MouseGestureBase type")
                mpEvent = MouseGestureEvent(etype, target.dwidth,
                                            target.dheight, p0.x, p0.y,
                                            MouseEvent.NO_BUTTON,
                                            MouseEvent.NO_MODIFIER,
                                            g.offset, finish)
                QtCore.QCoreApplication.postEvent(target, mpEvent)

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
        elif type == QtCore.QEvent.Gesture:
            self.handleGesture(target, event)
            # Intercept Gesture events only once otherwise new events may be
            # posted multiple times to the Event-Loop.
            return True
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
