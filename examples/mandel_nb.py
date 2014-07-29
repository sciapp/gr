#!/usr/bin/env python
# -*- no-plot -*-
"""
Calculate Mandelbrot set using Numba
"""

from numba import autojit
from timeit import default_timer as timer

from numpy import zeros, uint8
import gr

@autojit
def mandel(x, y, iters):
    c = complex(x, y)
    z = 0.0j
    ci = 0
    inc = 1

    for i in range(iters):
        z = z * z + c
        if (z.real * z.real + z.imag * z.imag) >= 4:
            return ci
        ci += inc
        if ci == 0 or ci == 255:
            inc = -inc

    return 255

@autojit
def create_fractal(min_x, max_x, min_y, max_y, image, iters):
    height = image.shape[0]
    width = image.shape[1]

    pixel_size_x = (max_x - min_x) / width
    pixel_size_y = (max_y - min_y) / height
    for x in range(width):
        real = min_x + x * pixel_size_x
        for y in range(height):
            imag = min_y + y * pixel_size_y
            color = mandel(real, imag, iters)
            image[y, x] = color

    return image

x = -0.9223327810370947027656057193752719757635
y = 0.3102598350874576432708737495917724836010

f = 0.5
for i in range(200):
    start = timer()
    image = zeros((500, 500), dtype=uint8)
    pixels = create_fractal(x-f, x+f, y-f, y+f, image, 400)
    dt = timer() - start

    print("Mandelbrot created in %f s" % dt)
    ca = 1000.0 + pixels.ravel()

    gr.clearws()
    gr.setviewport(0, 1, 0, 1)
    gr.setcolormap(13)
    gr.cellarray(0, 1, 0, 1, 500, 500, ca)
    gr.updatews()

    f *= 0.9
