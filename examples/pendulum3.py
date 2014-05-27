#!/usr/bin/env python
# -*- animation -*-
"""
3D animation of a pendulum
"""

from numpy import sin, cos, sqrt, pi, array
import time
import gr
import gr3

g = 9.8       # gravitational constant

def rk4(x, h, y, f):
    k1 = h * f(x, y)
    k2 = h * f(x + 0.5 * h, y + 0.5 * k1)
    k3 = h * f(x + 0.5 * h, y + 0.5 * k2)
    k4 = h * f(x + h, y + k3)
    return x + h, y + (k1 + 2 * (k2 + k3) + k4) / 6.0

def damped_pendulum_deriv(t, state): 
    theta, omega = state
    return array([omega, -gamma * omega -g / L * sin(theta)])

def sign(x):
    if x > 0:
        return 1
    elif x < 0:
        return -1
    else:
        return 0

def pendulum(t, theta, omega, acceleration):
    gr.clearws()
    gr.setviewport(0, 1, 0, 1)
    
    x, y = (sin(theta) * 3.0, -cos(theta) * 3.0)
    
    gr3.clear()
    # draw pivot point
    gr3.drawspheremesh(1, (0, 0, 0), (0.4, 0.4, 0.4), 0.1)
    # draw rod
    gr3.drawcylindermesh(1, (0, 0, 0), (x, y, 0), (0.6, 0.6, 0.6), 0.05, 3.0)
    # draw sphere
    gr3.drawspheremesh(1, (x, y, 0), (1, 1, 1), 0.25)
    # show angular velocity
    V = 0.3 * omega - sign(omega) * 0.15
    gr3.drawcylindermesh(1, (x, y, 0), (cos(theta), sin(theta), 0), (0, 0, 1),
                         0.05, V)
    gr3.drawconemesh(1, (x + cos(theta) * V, y + sin(theta) * V, 0),
                     (-y, x, 0), (0, 0, 1), 0.1, sign(omega) * 0.25)
    # show angular acceleration
    A = 0.3 * acceleration
    gr3.drawcylindermesh(1, (x, y, 0), (sin(theta), cos(theta), 0), (1, 0, 0),
                         0.05, A)
    gr3.drawconemesh(1, (x + sin(theta) * A, y + cos(theta) * A, 0),
                     (x, -y, 0), (1, 0, 0), 0.1, 0.25)
    # draw GR3 objects
    gr3.drawimage(0, 1, 0.15, 0.85, 500, 350, gr3.GR3_Drawable.GR3_DRAWABLE_GKS)
    
    gr.settextfontprec(2, gr.TEXT_PRECISION_STRING)
    gr.setcharheight(0.024)
    gr.settextcolorind(1)
    gr.textext(0.05, 0.96, 'Damped Pendulum')
    gr.mathtex(0.05, 0.9, '\\omega=\\dot{\\theta}')
    gr.mathtex(0.05, 0.83, '\\dot{\\omega}=-\\gamma\\omega-\\frac{g}{l}sin(\\theta)')
    gr.setcharheight(0.020)
    gr.textext(0.05, 0.20, 't:%7.2f' % t)
    gr.textext(0.05, 0.16, '\\theta:%7.2f' % (theta / pi * 180))
    gr.settextcolorind(4)
    gr.textext(0.05, 0.12, '\\omega:%7.2f' % omega)
    gr.settextcolorind(2)
    gr.textext(0.05, 0.08, 'y_{A}:%6.2f' % acceleration)
    gr.updatews()
    return

theta = 110.0  # initial angle
gamma = 0.1    # damping coefficient
L = 1          # pendulum length

t = 0
dt = 0.04
state = array([theta * pi / 180, 0])

gr3.init()
gr3.setcameraprojectionparameters(45, 1, 100)
gr3.cameralookat(0, -2, 6, 0, -2, 0, 0, 1, 0)
gr3.setbackgroundcolor(1, 1, 1, 1)
gr3.setlightdirection(1, 1, 10)
 
now = time.clock()

while t < 30:
    start = now

    t, state = rk4(t, dt, state, damped_pendulum_deriv)
    theta, omega = state
    acceleration = sqrt(2 * g * L * (1 - cos(theta)))
    pendulum(t, theta, omega, acceleration)
    
    now = time.clock()
    if start + dt > now:
        time.sleep(start + dt - now)
    
gr3.terminate()
