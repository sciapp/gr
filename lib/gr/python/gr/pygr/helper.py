# -*- coding: utf-8 -*-
"""Python GR Helper Classes

Exported Classes:

"""
# standard library
import math
# local library
from gr._version import __version__, __revision__

__author__ = "Christian Felder <c.felder@fz-juelich.de>"
__copyright__ = """Copyright 2012 - 2014 Forschungszentrum Juelich GmbH

This file is part of GR, a universal framework for visualization applications.
Visit http://gr-framework.org for the latest
version.

GR was developed by the Scientific IT-Systems group at the Peter Gr�nberg
Institute at Forschunsgzentrum J�lich. The main development has been done
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


class DomainChecker(object):

    _ZERO = 1e-8
    _EPSILON = 1e-8

    @staticmethod
    def isInLogDomain(*args):
        res = True
        for value in args:
            if value <= DomainChecker._ZERO:
                res = False
                break
        return res

    @staticmethod
    def isInWindowDomain(xmin, xmax, ymin, ymax):
        res = True
        if (math.isnan(xmin) or math.isinf(xmin) or math.isnan(xmax)
            or math.isinf(xmax) or math.isnan(ymin) or math.isinf(ymin) or
            math.isnan(ymax) or math.isinf(ymax) or
            xmin > max or ymin > ymax or abs(xmax - xmin) < DomainChecker._EPSILON or
            abs(ymax - ymin) < DomainChecker._EPSILON):
            res = False
        return res


class ColorIndexGenerator(object):

    _distinct_colors = range(980, 1000)
    _n = len(_distinct_colors)
    _curIdx = 0

    def __init__(self):
        self._i = 0

    @staticmethod
    def nextColorIndex():
        idx = ColorIndexGenerator._distinct_colors[ColorIndexGenerator._curIdx]
        ColorIndexGenerator._curIdx = ((ColorIndexGenerator._curIdx + 1)
                                       % ColorIndexGenerator._n)
        return idx

    def getNextColorIndex(self):
        idx = ColorIndexGenerator._distinct_colors[self._i]
        self._i = (self._i + 1) % ColorIndexGenerator._n
        return idx

    def reset(self):
        self._i = 0
