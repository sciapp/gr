
from pylab import plot, draw, pause
from numpy import arange, sin, pi
from time import time

x = arange(0, 2 * pi, 0.01)

tstart = time()
line, = plot(x, sin(x))
for i in arange(1, 200):
    line.set_ydata(sin(x + i / 10.0))
    draw()
    pause(0.001)

fps_mpl = int(200 / (time() - tstart))
print('fps (mpl): %4d' % fps_mpl)

