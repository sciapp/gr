# -*- coding: utf-8 -*-
"""Qt backend GR module

The default backend order (PySide, PyQt4) can be overwritten with:
    gr.QT_BACKEND_ORDER = ["PyQt4", "PySide"]
"""

import sys

# local library
import gr
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

QT_PYSIDE = "PySide"
QT_PYQT4 = "PyQt4"

# modules and functions that we provide
QtCore = None
QtGui = None
getGKSConnectionId = None

# selecting the backend order:
if hasattr(gr, "QT_BACKEND_ORDER"):
    # new way to specify it
    QT_BACKEND_ORDER = gr.QT_BACKEND_ORDER
elif hasattr(sys, "QT_BACKEND_ORDER"):
    # backwards compatible way
    QT_BACKEND_ORDER = sys.QT_BACKEND_ORDER
elif 'PyQt4' in sys.modules and 'PySide' not in sys.modules:
    # only PyQt already loaded, use it
    QT_BACKEND_ORDER = [QT_PYQT4, QT_PYSIDE]
else:
    # fallback on default order
    QT_BACKEND_ORDER = [QT_PYSIDE, QT_PYQT4]


def _importPySide():
    global QtCore, QtGui, getGKSConnectionId

    from PySide import QtCore
    from PySide import QtGui
    try:
        from PySide import shiboken
    except ImportError:
        import shiboken # Anaconda

    def getGKSConnectionId(widget, painter):
        return "%x!%x" % (long(shiboken.getCppPointer(widget)[0]),
                          long(shiboken.getCppPointer(painter)[0]))


def _importPyQt4():
    global QtCore, QtGui, getGKSConnectionId

    from PyQt4 import QtCore
    from PyQt4 import QtGui
    import sip

    # a bit of compatibility...
    QtCore.Signal = QtCore.pyqtSignal

    def getGKSConnectionId(widget, painter):
        return "%x!%x" % (sip.unwrapinstance(widget),
                          sip.unwrapinstance(painter))


_importers = {QT_PYSIDE: _importPySide,
              QT_PYQT4:  _importPyQt4}

for backend in QT_BACKEND_ORDER:
    try:
        _importers[backend]()
    except ImportError:
        pass
    else:
        break
else:
    raise ImportError('None of the selected Qt backends (%s) were available' %
                      QT_BACKEND_ORDER)
