#!/usr/bin/env python
# -*- no-plot -*-
"""
Compare line drawing performance of Matplotlib vs. GR

These are the results on an iMac 3,4 GHz Intel Core i7::

    fps (mpl):   29
    fps  (GR): 1002
      speedup:   34.6
"""

from pylab import plot, draw, pause
from numpy import arange, sin, pi
from time import time, sleep

x = arange(0, 2 * pi, 0.01)

tstart = time()
line, = plot(x, sin(x))
for i in arange(1, 200):
    line.set_ydata(sin(x + i / 10.0))
    draw()
    pause(0.0001)

fps_mpl = int(200 / (time() - tstart))
print('fps (mpl): %4d' % fps_mpl)

from gr.pygr import plot

tstart = time()
for i in arange(1, 200):
    plot(x, sin(x + i / 10.0))
    sleep(0.0001)

fps_gr = int(200 / (time() - tstart))
print('fps  (GR): %4d' % fps_gr)

print('  speedup: %6.1f' % (float(fps_gr) / fps_mpl))
