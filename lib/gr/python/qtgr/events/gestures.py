# -*- coding: utf-8 -*-

# standard library
import logging
# local library
from qtgr.backend import QtCore
from qtgr.backend import QGesture, QGestureRecognizer
from qtgr.events.mouse import MouseEvent
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


class MouseGestureBase(QGesture):

    def __init__(self, parent=None):
        QGesture.__init__(self, parent)
        self.startPoint = None
        self.endPoint = None
        self.offset = None  # NDC


class PanGesture(MouseGestureBase):
    """Mouse panning gesture."""

    def __init__(self, parent=None):
        MouseGestureBase.__init__(self, parent)
        self.lastStartPoint = None


class SelectGesture(MouseGestureBase):
    """Mouse select rectangular two point region gesture"""


class MouseGestureBaseRecognizer(QGestureRecognizer):

    GESTURE = MouseGestureBase

    def __init__(self, buttons=MouseEvent.RIGHT_BUTTON,
                 modifiers=MouseEvent.NO_MODIFIER):
        QGestureRecognizer.__init__(self)
        self._buttons = buttons
        self._modifiers = modifiers

    def getButtons(self):
        return self._buttons

    def getModifiers(self):
        return self._modifiers

    def create(self, target):
        return self.GESTURE()

    def reset(self, gesture):
        gesture.startPoint = None
        gesture.endPoint = None
        gesture.offset = None


class PanGestureRecognizer(MouseGestureBaseRecognizer):

    GESTURE = PanGesture

    def __init__(self, buttons=MouseEvent.RIGHT_BUTTON,
                 modifiers=MouseEvent.NO_MODIFIER):
        MouseGestureBaseRecognizer.__init__(self, buttons, modifiers)

    def reset(self, gesture):
        MouseGestureBaseRecognizer.reset(self, gesture)
        gesture.lastStartPoint = None

    def recognize(self, gesture, watched, event):
        result = QGestureRecognizer.Ignore
        type = event.type()
        if (type == MouseEvent.MOUSE_PRESS and
                self.getButtons() == event.getButtons() and
                self.getModifiers() == event.getModifiers()):
            result = QGestureRecognizer.MayBeGesture
            gesture.startPoint = event
            gesture.lastStartPoint = None
            _log.debug("%s: MayBeGesture", self.__class__.__name__)
        elif type == MouseEvent.MOUSE_RELEASE:
            # if gesture is not NoGesture (0) or GestureFinished
            if gesture.state() not in (0, QtCore.Qt.GestureFinished):
                _log.debug("%s: FinishGesture", self.__class__.__name__)
                result = QGestureRecognizer.FinishGesture
                gesture.endPoint = event
                gesture.offset = gesture.endPoint.getNDC() - \
                                 gesture.startPoint.getNDC()
            else:
                result = QGestureRecognizer.CancelGesture
        elif type == MouseEvent.MOUSE_MOVE:
            if gesture.startPoint:
                result = QGestureRecognizer.TriggerGesture
                if gesture.lastStartPoint:
                    gesture.startPoint = gesture.lastStartPoint
                gesture.endPoint = event
                gesture.offset = gesture.endPoint.getNDC() - \
                                 gesture.startPoint.getNDC()
                if gesture.offset.norm() > 1E-3:
                    gesture.lastStartPoint = event
                else:
                    result = QGestureRecognizer.MayBeGesture
        return result


class SelectGestureRecognizer(MouseGestureBaseRecognizer):

    GESTURE = SelectGesture

    def __init__(self, buttons=MouseEvent.LEFT_BUTTON,
                 modifiers=MouseEvent.NO_MODIFIER):
        MouseGestureBaseRecognizer.__init__(self, buttons, modifiers)

    def recognize(self, gesture, watched, event):
        result = QGestureRecognizer.Ignore
        type = event.type()
        if (type == MouseEvent.MOUSE_PRESS and
                self.getButtons() == event.getButtons() and
                self.getModifiers() == event.getModifiers()):
            result = QGestureRecognizer.MayBeGesture
            gesture.startPoint = event
            _log.debug("%s: MayBeGesture", self.__class__.__name__)
        elif type == MouseEvent.MOUSE_RELEASE:
            # if gesture is not NoGesture (0) or GestureFinished
            if gesture.state() not in (0, QtCore.Qt.GestureFinished):
                _log.debug("%s: FinishGesture", self.__class__.__name__)
                result = QGestureRecognizer.FinishGesture
                gesture.endPoint = event
                gesture.offset = gesture.endPoint.getNDC() - \
                                 gesture.startPoint.getNDC()
            else:
                result = QGestureRecognizer.CancelGesture
        elif type == MouseEvent.MOUSE_MOVE:
            if gesture.startPoint:
                result = QGestureRecognizer.TriggerGesture
                gesture.endPoint = event
                gesture.offset = gesture.endPoint.getNDC() - \
                                 gesture.startPoint.getNDC()
                if gesture.offset.norm() <= 1E-3:
                    result = QGestureRecognizer.MayBeGesture
        return result
