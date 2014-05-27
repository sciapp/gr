#!/usr/bin/env python
"""
Create a contour plot of irregular distributed data
"""

from numpy.random import uniform, seed
import numpy as np
import gr

seed(0)
xd = uniform(-2, 2, 100)
yd = uniform(-2, 2, 100)
zd = xd * np.exp(-xd**2 - yd**2)

gr.setviewport(0.1, 0.95, 0.1, 0.95)
gr.setwindow(-2, 2, -2, 2)
gr.setspace(-0.5, 0.5, 0, 90)
gr.setmarkersize(1)
gr.setmarkertype(gr.MARKERTYPE_SOLID_CIRCLE)
gr.setcharheight(0.024)
gr.settextalign(2, 0)
gr.settextfontprec(3, 0)

x, y, z = gr.gridit(xd, yd, zd, 200, 200)
h = np.linspace(-0.5, 0.5, 20)
gr.surface(x, y, z, 5)
gr.contour(x, y, h, z, 0)
gr.polymarker(xd, yd)
gr.axes(0.25, 0.25, -2, -2, 2, 2, 0.01)

gr.updatews()
