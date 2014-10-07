# -*- coding: utf-8 -*-
"""..."""
# standard library
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
from gr.pygr import DeviceCoordConverter
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

class EventMeta(QtCore.QEvent):

    def __init__(self, type):
        if isinstance(type, int):
            type = QtCore.QEvent.Type(type)
        super(EventMeta, self).__init__(type)
        self._type = type

    def type(self):
        return self._type

class MouseLocationEventMeta(EventMeta):

    def __init__(self, type, width, height, x, y, window=None):
        super(MouseLocationEventMeta, self).__init__(type)
        self._coords = DeviceCoordConverter(width, height, window=window)
        self._coords.setDC(x, y)

    def getWindow(self):
        return self._coords.getWindow()

    def getWC(self, viewport):
        return self._coords.getWC(viewport)

    def getNDC(self):
        return self._coords.getNDC()

    def getDC(self):
        return self._coords.getDC()
