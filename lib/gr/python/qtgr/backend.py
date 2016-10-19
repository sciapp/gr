# -*- coding: utf-8 -*-
"""Qt backend GR module

The default backend order (PyQt4, PySide) can be overwritten with:
    gr.QT_BACKEND_ORDER = ["PySide", "PyQt4"]
"""

import collections
import ctypes
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
QT_PYQT5 = "PyQt5"

# modules, functions and classes that we provide
QtCore = None
QtGui = None
QtWidgets = None
QtPrintSupport = None
QApplication = None
QWidget = None
QGesture = None
QGestureRecognizer = None
QPainter = None
QPrinter = None
QPrintDialog = None

getGKSConnectionId = None
QtVersionTuple = None

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
    # check which backend is already imported
    # fallback to default order if no import can be detected
    QT_BACKEND_ORDER = [QT_PYQT4, QT_PYSIDE, QT_PYQT5]
    for i, backend in enumerate(QT_BACKEND_ORDER[:]):
        if backend in sys.modules:
            QT_BACKEND_ORDER.insert(0, QT_BACKEND_ORDER.pop(i))
            break


VersionTuple = collections.namedtuple('VersionTuple', ['major', 'minor', 'patch'])


def _importPySide():
    global QApplication, QtCore, QtGui, QWidget, QGesture, QGestureRecognizer,\
        QPainter, QPrinter, QPrintDialog, getGKSConnectionId
    global QtVersionTuple

    from PySide import QtCore
    from PySide import QtGui
    from PySide.QtGui import QApplication, QWidget, QGesture, \
        QGestureRecognizer, QPainter, QPrinter, QPrintDialog
    try:
        from PySide import shiboken
    except ImportError:
        import shiboken  # Anaconda

    QtVersionTuple = VersionTuple(*QtCore.__version_info__)

    # Load QtCore module (which is a c++ extension module) and export all
    # symbols globally
    # -> The gks plugin loader can load the "qVersion" function with dlsym and
    # determine the correct qt version
    ctypes.CDLL(QtCore.__file__, ctypes.RTLD_GLOBAL)

    def getGKSConnectionId(widget, painter):
        return "%x!%x" % (long(shiboken.getCppPointer(widget)[0]),
                          long(shiboken.getCppPointer(painter)[0]))


def _importPyQt4():
    global QApplication, QtCore, QtGui, QWidget, QGesture, QGestureRecognizer,\
        QPainter, QPrinter, QPrintDialog, getGKSConnectionId
    global QtVersionTuple

    from PyQt4 import QtCore
    from PyQt4 import QtGui
    from PyQt4.QtGui import QApplication, QWidget, QGesture, \
        QGestureRecognizer, QPainter, QPrinter, QPrintDialog
    import sip

    # a bit of compatibility...
    QtCore.Signal = QtCore.pyqtSignal

    QtVersionTuple = VersionTuple(*map(int, QtCore.QT_VERSION_STR.split('.')))

    # Load QtCore module (which is a c++ extension module) and export all
    # symbols globally
    # -> The gks plugin loader can load the "qVersion" function with dlsym and
    # determine the correct qt version
    ctypes.CDLL(QtCore.__file__, ctypes.RTLD_GLOBAL)

    def getGKSConnectionId(widget, painter):
        return "%x!%x" % (sip.unwrapinstance(widget),
                          sip.unwrapinstance(painter))


def _importPyQt5():
    global QApplication, QtCore, QtGui, QtWidgets, QtPrintSupport, QWidget, \
        QGesture, QGestureRecognizer, QPainter, QPrinter, QPrintDialog
    global getGKSConnectionId, QtVersionTuple

    from PyQt5 import QtCore
    from PyQt5 import QtGui
    from PyQt5 import QtWidgets
    from PyQt5 import QtPrintSupport
    from PyQt5.QtGui import QPainter
    from PyQt5.QtWidgets import QApplication, QWidget, QGesture, QGestureRecognizer
    from PyQt5.QtPrintSupport import QPrinter, QPrintDialog
    import sip

    # a bit of compatibility...
    QtCore.Signal = QtCore.pyqtSignal

    QtVersionTuple = VersionTuple(*map(int, QtCore.QT_VERSION_STR.split('.')))

    # Load QtCore module (which is a c++ extension module) and export all
    # symbols globally
    # -> The gks plugin loader can load the "qVersion" function with dlsym and
    # determine the correct qt version
    ctypes.CDLL(QtCore.__file__, ctypes.RTLD_GLOBAL)

    def getGKSConnectionId(widget, painter):
        return "%x!%x" % (sip.unwrapinstance(widget),
                          sip.unwrapinstance(painter))


_importers = {QT_PYSIDE: _importPySide,
              QT_PYQT4:  _importPyQt4,
              QT_PYQT5:  _importPyQt5}

for backend in QT_BACKEND_ORDER:
    try:
        _importers[backend]()
    except (ImportError, RuntimeError):
        pass
    else:
        break
else:
    raise ImportError('None of the selected Qt backends (%s) were available' %
                      QT_BACKEND_ORDER)
