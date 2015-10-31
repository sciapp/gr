#!/usr/bin/env python
# -*- animation -*-
"""
3D animation of a spring pendulum
"""

import math
import gr
import gr3

gr.setviewport(0, 1, 0, 1)
gr3.setbackgroundcolor(1, 1, 1, 1)

for t in range(200):
    f = 0.0375 * (math.cos(t*0.2) * 0.995**t + 1.3)
    n = 90
    points = [(math.sin(i*math.pi/8), n*0.035-i*f, math.cos(i*math.pi/8)) for i in range(n-5)]
    points.append((0, points[-1][1], 0))
    points.append((0, points[-1][1]-0.5, 0))
    points.append((0, points[-1][1]-1, 0))
    points.insert(0, (0, points[0][1], 0))
    points.insert(0, (0, points[0][1]+2, 0))
    colors = [(1, 1, 1)]*n
    radii = [0.1]*n
    gr3.clear()
    gr3.drawtubemesh(n, points, colors, radii)
    gr3.drawspheremesh(1, points[-1], colors, 0.75)
    gr.clearws()
    gr3.drawimage(0, 1, 0, 1, 500, 500, gr3.GR3_Drawable.GR3_DRAWABLE_GKS)
    gr.updatews()
