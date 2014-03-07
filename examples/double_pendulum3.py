#!/usr/bin/env python
# -*- animation -*-
"""
3D animation of a double pendulum
"""

from numpy import sin, cos, pi, array
import time
import gr
import gr3

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

def double_pendulum(theta, length, mass):
    gr.clearws()
    gr.setviewport(0, 1, 0, 1)
    
    direction = []
    position = [(0, 0, 0)]
    for i in range(2):
        direction.append((sin(theta[i]) * length[i] * 2,
                          -cos(theta[i]) * length[i] * 2, 0))
        position.append([position[-1][j] + direction[-1][j] for j in range(3)])
    
    gr3.clear()
    # draw pivot point
    gr3.drawcylindermesh(1, (0, 0.2, 0), (0, 1, 0), (0.4, 0.4, 0.4), 0.4, 0.05)
    gr3.drawcylindermesh(1, (0, 0.2, 0), (0, -1, 0), (0.4, 0.4, 0.4), 0.05, 0.2)
    gr3.drawspheremesh(1, (0,0,0), (0.4, 0.4, 0.4), 0.05)
    # draw rods
    gr3.drawcylindermesh(2, position, direction,
                         (0.6, 0.6, 0.6) * 2, (0.05, 0.05),
                         [l * 2 for l in length])
    # draw bobs
    gr3.drawspheremesh(2, position[1:], (1, 1, 1) * 2, [m * 0.2 for m in mass]) 

    gr3.drawimage(0, 1, 0, 1, 500, 500, gr3.GR3_Drawable.GR3_DRAWABLE_GKS)
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

gr3.init()
gr3.setcameraprojectionparameters(45, 1, 100)
gr3.cameralookat(6, -2, 4, 0, -2, 0, 0, 1, 0)
gr3.setbackgroundcolor(1, 1, 1, 1)
gr3.setlightdirection(1, 1, 10)

now = time.clock()

while t < 30:
    start = now

    t, state = rk4(t, dt, state, pendulum_derivs)
    t1, w1, t2, w2 = state
    double_pendulum([t1, t2], [l1, l2], [m1, m2])

    now = time.clock()
    if start + dt > now:
        time.sleep(start + dt - now)

