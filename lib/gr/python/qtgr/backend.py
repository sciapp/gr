#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""Qt backend GR module

The default backend order (PySide, PyQt4) can be overwritten with:
    setattr(sys, "QT_BACKEND_ORDER", ["PyQt4", "PySide"])
"""

import sys
# local library
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
QT_BACKEND_ORDER = getattr(sys, "QT_BACKEND_ORDER", [QT_PYSIDE, QT_PYQT4])

del sys
