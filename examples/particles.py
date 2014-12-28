#!/usr/bin/env python
# -*- animation -*-
"""
Simple particle simulation
"""

import numpy as np
import gr, time
from numba.decorators import jit

N = 300                    # number of particles
M = 0.05 * np.ones(N)      # masses
size = 0.04                # particle size

def step(dt, size, a):
    # update positions
    a[0] += dt * a[1]

    n = a.shape[1]
    D = np.empty((n, n), dtype=np.float)
    for i in range(n):
        for j in range(n):
            dx = a[0, i, 0] - a[0, j, 0]
            dy = a[0, i, 1] - a[0, j, 1]
            D[i, j] = np.sqrt(dx*dx + dy*dy)

    # find pairs of particles undergoing a collision
    for i in range(n):
        for j in range(i + 1, n):
            if D[i, j] < 2*size:
                # relative location & velocity vectors
                r_rel = a[0, i] - a[0, j]
                v_rel = a[1, i] - a[1, j]

                # momentum vector of the center of mass
                v_cm = (M[i] * a[1, i] + M[j] * a[1, j]) / (M[i] + M[j])

                # collisions of spheres reflect v_rel over r_rel
                rr_rel = np.dot(r_rel, r_rel)
                vr_rel = np.dot(v_rel, r_rel)
                v_rel = 2 * r_rel * vr_rel / rr_rel - v_rel

                # assign new velocities
                a[1, i] = v_cm + v_rel * M[j] / (M[i] + M[j])
                a[1, j] = v_cm - v_rel * M[i] / (M[i] + M[j])

    # check for crossing boundary
    for i in range(n):
        if a[0, i][0] < -2 + size:
            a[0, i][0] = -2 + size
            a[1, i][0] *= -1
        elif a[0, i][0] > 2 - size:
            a[0, i][0] = 2 - size
            a[1, i][0] *= -1
        if a[0, i][1] < -2 + size:
            a[0, i][1] = -2 + size
            a[1, i][1] *= -1
        elif a[0, i][1] > 2 - size:
            a[0, i][1] = 2 - size
            a[1, i][1] *= -1

    return a

np.random.seed(0)
a = np.empty([2, N, 2], dtype=float)
a[0, :] = -0.5 + np.random.random((N, 2))      # positions
a[1, :] = -0.5 + np.random.random((N, 2))      # velocities
a[0, :] *= (4 - 2*size)
dt = 1. / 30

step_numba = jit('f8[:,:,:](f8, f8, f8[:,:,:])')(step)

gr.setwindow(-2, 2, -2, 2)
gr.setviewport(0, 1, 0, 1)
gr.setmarkertype(gr.MARKERTYPE_SOLID_CIRCLE)
gr.setmarkersize(1.0)

start = time.time()
t0 = start

n = 0
t = 0
worker = 'CPython'

while t < 6:

    if t > 3:
        if worker == 'CPython':
            t0 = now
            n = 0
        a = step_numba(dt, size, a)
        worker = 'Numba'
    else:
        a = step(dt, size, a)

    gr.clearws()
    gr.setmarkercolorind(75)
    gr.polymarker(a[0, :, 0], a[0, :, 1])
    if n > 0:
        gr.text(0.01, 0.95, '%10s: %4d fps' % (worker, int(n / (now - t0))))
    gr.updatews()

    now = time.time()
    n += 1
    t = now - start
