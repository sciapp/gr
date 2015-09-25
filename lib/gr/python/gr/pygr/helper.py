# -*- coding: utf-8 -*-
"""Python GR Helper Classes

Exported Classes:

"""
# standard library
import math
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


class DomainChecker(object):

    @staticmethod
    def isInLogDomain(*args):
        res = True
        for value in args:
            if value <= gr.precision:
                res = False
                break
        return res

    @staticmethod
    def isInWindowDomain(xmin, xmax, ymin, ymax):
        return gr.validaterange(xmin, xmax) and gr.validaterange(ymin, ymax)


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
