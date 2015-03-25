#!/usr/bin/env python
# -*- no-plot -*-
"""
Calculate Mandelbrot set using NumbaPro (vectorized version)
"""

from numbapro import vectorize
from timeit import default_timer as timer

import numpy as np
import gr

sig = 'i8(uint32, f8, f8, f8, f8, uint32, uint32, uint32)'

@vectorize([sig], target='parallel')
def mandel(tid, min_x, max_x, min_y, max_y, width, height, iters):
    pixel_size_x = (max_x - min_x) / width
    pixel_size_y = (max_y - min_y) / height

    x = tid % width
    y = tid / width

    real = min_x + x * pixel_size_x
    imag = min_y + y * pixel_size_y

    c = complex(real, imag)
    z = 0.0j
    ci = 0
    inc = 1

    for i in xrange(iters):
        z = z * z + c
        if (z.real * z.real + z.imag * z.imag) >= 4:
            return ci
        ci += inc
        if ci == 0 or ci == 255:
            inc = -inc

    return 255


def create_fractal(min_x, max_x, min_y, max_y, width, height, iters):
    tids = np.arange(width * height, dtype=np.uint32)
    return mandel(tids, np.float64(min_x), np.float64(max_x), np.float64(min_y),
                  np.float64(max_y), np.uint32(height), np.uint32(width),
                  np.uint32(iters))

x = -0.9223327810370947027656057193752719757635
y = 0.3102598350874576432708737495917724836010

f = 0.5
for i in range(200):
    start = timer()
    pixels = create_fractal(x-f, x+f, y-f, y+f, 500, 500, 400)
    dt = timer() - start

    print("Mandelbrot created in %f s" % dt)
    ca = 1000.0 + pixels.ravel()

    gr.clearws()
    gr.setviewport(0, 1, 0, 1)
    gr.setcolormap(13)
    gr.cellarray(0, 1, 0, 1, 500, 500, ca)
    gr.updatews()

    f *= 0.9
