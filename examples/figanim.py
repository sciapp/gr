#!/usr/bin/env python
# -*- no-plot -*-
"""
Compare figure output performance of Matplotlib vs. GR
"""

from __future__ import print_function

from numpy import arange, sin, pi
from time import time
from os import environ
from sys import argv, stdout

if len(argv) > 1:
    dev = argv[1]
else:
    dev = 'ps'

import matplotlib
matplotlib.use(dev)

from pylab import clf, plot, draw, savefig

x = arange(0, 2 * pi, 0.01)

tstart = time()
for i in arange(1, 100):
    clf()
    plot(sin(x + i / 10.0))
    savefig('mpl%04d.%s' % (i, dev))
    if i % 2 == 0:
        print('.', end="")
        stdout.flush()


fps_mpl = int(100 / (time() - tstart))
print('fps (mpl): %4d' % fps_mpl)

from gr.pygr import plot

tstart = time()
environ["GKS_WSTYPE"] = dev
for i in arange(1, 100):
    plot(x, sin(x + i / 10.0))
    if i % 2 == 0:
        print('.', end="")
        stdout.flush()

fps_gr = int(100 / (time() - tstart))
print('fps  (GR): %4d' % fps_gr)

print('  speedup: %6.1f' % (float(fps_gr) / fps_mpl))
