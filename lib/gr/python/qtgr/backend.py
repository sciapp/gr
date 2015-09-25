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
