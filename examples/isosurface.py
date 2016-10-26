#!/usr/bin/env python
# -*- coding: utf-8 -*-
# -*- no-plot -*-
"""
Example script for rendering isosurfaces using GR3

The script requires an input file compatible to np.genfromtxt.
"""

import sys
import numpy as np
import gr3

if len(sys.argv) >= 2:
    filename = sys.argv[1]
else:
    filename = 'isosurface_input.txt'

print("Reading file...")
try:
    filedata = np.load(filename+'.npy')
except:
    # write the data to a binary file for faster reading in the future
    filedata = np.genfromtxt(filename, delimiter=',')
    np.save(filename+'.npy', filedata)
print('Done.')

positions = filedata[:, :3].astype(np.int32)
directions = filedata[:, 3:].astype(np.float32)

# define the values used for calculating the isosurface and the isolevel
isovalue = 0
values = directions[:, 2]

# negate the values, as we want negative z to define "inside"
isovalue = -isovalue
values = -values

# transform the values to the range [0, 1]
isovalue -= values.min()
values -= values.min()
isovalue /= values.max()
values /= values.max()

# write them into a contiguous array
nx, ny, nz = positions.max(axis=0)+1
data = np.zeros((nx, ny, nz), np.float32)
ix, iy, iz = positions.T
data[ix, iy, iz] = values

# write out an HTML file
gr3.cameralookat(0, 0, 2, 0, 0, 0, 0, 1, 0)
gr3.drawisosurfacemesh(data, isovalue=isovalue, step=[2.0/(nx-1), 2.0/(ny-1), 2.0/(nz-1)], offset=[-1, -1, -1], position=(0, 0, 0))
gr3.export("output.html", 1024, 1024)
