#!/usr/bin/env python
# -*- animation -*-
"""
Animation of a double pendulum
"""

from numpy import sin, cos, pi, array
import time
import gr

g = 9.8        # gravitational constant

def rk4(x, h, y, f):
    k1 = h * f(x, y)
    k2 = h * f(x + 0.5 * h, y + 0.5 * k1)
    k3 = h * f(x + 0.5 * h, y + 0.5 * k2)
    k4 = h * f(x + h, y + k3)
    return x + h, y + (k1 + 2 * (k2 + k3) + k4) / 6.0

def pendulum_derivs(t, state):
    # The following derivation is from:
    # http://scienceworld.wolfram.com/physics/DoublePendulum.html
    t1, w1, t2, w2 = state
    a = (m1 + m2) * l1
    b = m2 * l2 * cos(t1 - t2)
    c = m2 * l1 * cos(t1 - t2)
    d = m2 * l2
    e = -m2 * l2 * w2**2 * sin(t1 - t2) - g * (m1 + m2) * sin(t1)
    f =  m2 * l1 * w1**2 * sin(t1 - t2) - m2 * g * sin(t2)
    return array([w1, (e*d-b*f) / (a*d-c*b), w2, (a*f-c*e) / (a*d-c*b)])

def pendulum(theta, length, mass):
    l = length[0] + length[1]
    gr.clearws()
    gr.setviewport(0, 1, 0, 1)
    gr.setwindow(-l, l, -l, l)
    gr.setmarkertype(gr.MARKERTYPE_SOLID_CIRCLE)
    gr.setmarkercolorind(86)
    pivot = [0, 0.775]                         # draw pivot point
    gr.fillarea([-0.2, 0.2, 0.2, -0.2], [0.75, 0.75, 0.8, 0.8])
    for i in range(2):
        x = [pivot[0], pivot[0] + sin(theta[i]) * length[i]]
        y = [pivot[1], pivot[1] - cos(theta[i]) * length[i]]
        gr.polyline(x, y)                   # draw rod
        gr.setmarkersize(3 * mass[i])
        gr.polymarker([x[1]], [y[1]])       # draw bob
        pivot = [x[1], y[1]]
    gr.updatews()
    return

l1 = 1.2       # length of rods
l2 = 1.0
m1 = 1.0       # weights of bobs
m2 = 1.5
t1 = 100.0     # inintial angles
t2 = -20.0

w1 = 0.0
w2 = 0.0
t = 0
dt = 0.04
state = array([t1, w1, t2, w2]) * pi / 180

now = time.clock()

while t < 30:
    start = now

    t, state = rk4(t, dt, state, pendulum_derivs)
    t1, w1, t2, w2 = state
    pendulum([t1, t2], [l1, l2], [m1, m2])

    now = time.clock()
    if start + dt > now:
        time.sleep(start + dt - now)

