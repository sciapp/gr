#!/usr/bin/env python
# -*- animation -*-
"""
Solving The 2D Diffusion Equation
"""

import numpy
import gr, time
from numba.decorators import jit

dx = 0.005
dy = 0.005
a = 0.5
dt = dx*dx*dy*dy/(2*a*(dx*dx+dy*dy))
timesteps = 150

nx = int(1/dx)
ny = int(1/dy)
ui = numpy.zeros([nx,ny])
u = numpy.zeros([nx,ny])

for i in range(nx):
    for j in range(ny):
        if (((i*dx-0.5)**2+(j*dy-0.5)**2 <= 0.1) &
            ((i*dx-0.5)**2+(j*dy-0.5)**2 >= 0.05) ):
            ui[i,j] = 1

def diff_step(u, ui):
    for i in range(1,nx-1):
        for j in range(1,ny-1):
            uxx = ( ui[i+1,j] - 2*ui[i,j] + ui[i-1, j] )/(dx*dx)
            uyy = ( ui[i,j+1] - 2*ui[i,j] + ui[i, j-1] )/(dy*dy)
            u[i,j] = ui[i,j]+dt*a*(uxx+uyy)

diff_step_numba = jit('void(f8[:,:], f8[:,:])')(diff_step)

now = time.clock()

t = 0
worker = 'CPython'

for m in range(timesteps):
    gr.clearws()

    start = now
    if t > 5:
        diff_step_numba(u, ui)
        worker = 'Numba'
    else:
        diff_step(u, ui)
    ui = numpy.copy(u)
    now = time.clock()
    t = t + now - start

    c = 1000 + 255 * u.ravel()
    gr.setviewport(0, 1, 0, 1)
    gr.setcolormap(-32)
    gr.cellarray(0, 1, 0, 1, nx, ny, c)
    gr.text(0.01, 0.95, '%10s: %7.2f fps' % (worker, 1.0 / (now - start)))
    gr.updatews()
