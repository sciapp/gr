#!/usr/bin/env python
# -*- coding: utf-8 -*-
# -*- no-plot -*-
"""
Rendering slices and an isosurface of MRI data.
"""

import numpy as np
import gr
import gr3

def draw(mesh, x=None, y=None, z=None):
    gr3.clear()
    gr3.drawmesh(mesh, 1, (0,0,0), (0,0,1), (0,1,0), (1,1,1), (1,1,1))
    gr3.drawslicemeshes(data,  x=x, y=y, z=z)
    gr.clearws()
    gr3.drawimage(0, 1, 0, 1, 500, 500, gr3.GR3_Drawable.GR3_DRAWABLE_GKS)
    gr.updatews()

data = np.fromfile("mri.raw", np.uint16)
data = data.reshape((64, 64, 93))
data[data > 2000] = 2000
data[:, :, :] = data / 2000.0 * np.iinfo(np.uint16).max

gr.setviewport(0, 1, 0, 1)
gr3.init()
gr3.cameralookat(-3, 2, -2, 0, 0, 0, 0, 0, -1)
mesh = gr3.createisosurfacemesh(data, isolevel=40000)

gr.setcolormap(1)
for z in np.linspace(0, 1, 300):
    draw(mesh, x=0.9, z=z)
for y in np.linspace(1, 0.5, 300):
    draw(mesh, x=0.9, y=y, z=1)
gr.setcolormap(19)
for x in np.linspace(0.9, 0, 300):
    draw(mesh, x=x, y=0.5, z=1)
for x in np.linspace(0, 0.9, 300):
    draw(mesh, x=x, z=1)

gr3.terminate()
