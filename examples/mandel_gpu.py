#!/usr/bin/env python
# -*- no-plot -*-
"""
Calculate Mandelbrot set using NumbaPro (GPU version)
"""

from numbapro import *
from timeit import default_timer as timer

import numpy as np
import gr

@cuda.jit(restype=uint32, argtypes=[f8, f8, uint32], device=True)
def mandel(x, y, max_iters):
    c = complex(x, y)
    z = 0.0j
    ci = 0
    inc = 1

    for i in xrange(max_iters):
        z = z * z + c
        if (z.real * z.real + z.imag * z.imag) >= 4:
            return ci
        ci += inc
        if ci == 0 or ci == 255:
            inc = -inc

    return 255

@cuda.jit(argtypes=[f8, f8, f8, f8, uint8[:,:], uint32])
def mandel_kernel(min_x, max_x, min_y, max_y, image, max_iters):
    height = image.shape[0]
    width = image.shape[1]

    pixel_size_x = (max_x - min_x) / width
    pixel_size_y = (max_y - min_y) / height

    startX = cuda.blockDim.x * cuda.blockIdx.x + cuda.threadIdx.x
    startY = cuda.blockDim.y * cuda.blockIdx.y + cuda.threadIdx.y
    gridX = cuda.gridDim.x * cuda.blockDim.x;
    gridY = cuda.gridDim.y * cuda.blockDim.y;

    for x in xrange(startX, width, gridX):
        real = min_x + x * pixel_size_x
        for y in xrange(startY, height, gridY):
            imag = min_y + y * pixel_size_y 
            image[y, x] = mandel(real, imag, max_iters)


x = -0.9223327810370947027656057193752719757635
y = 0.3102598350874576432708737495917724836010

image = np.zeros((500, 500), dtype = np.uint8)
blockdim = (32, 8)
griddim = (32, 16)

f = 0.5
for i in range(200):
    start = timer()
    d_image = cuda.to_device(image)
    mandel_kernel[griddim, blockdim](x-f, x+f, y-f, y+f, d_image, 400) 
    d_image.to_host()
    dt = timer() - start

    print("Mandelbrot created in %f s" % dt)
    ca = 1000.0 + image.ravel()

    gr.clearws()
    gr.setviewport(0, 1, 0, 1)
    gr.setcolormap(13)
    gr.cellarray(0, 1, 0, 1, 500, 500, ca)
    gr.updatews()

    f *= 0.9
