#!/usr/bin/env python
# -*- no-plot -*-
"""
Calculate Mandelbrot set using OpenCL
"""

import pyopencl as cl
from timeit import default_timer as timer

import numpy as np
import gr


platform = cl.get_platforms()
gpu_devices = platform[0].get_devices(device_type=cl.device_type.GPU)

info_value = gpu_devices[0].get_info(getattr(cl.device_info, 'EXTENSIONS'))
if not 'cl_khr_fp64' in info_value:
  print("GPU has no support for double floating-point precision")
  exit(-1)

ctx = cl.Context(devices=gpu_devices)
queue = cl.CommandQueue(ctx)

prg = cl.Program(ctx, """
#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable
__kernel void mandelbrot(__global double2 *q, __global ushort *output,
                         double const min_x, double const max_x,
                         double const min_y, double const max_y,
                         ushort const width, ushort const height,
                         ushort const iters)
{
    int ci = 0, inc = 1;
    int gid = get_global_id(0);
    double nreal, real = 0;
    double imag = 0;

    q[gid].x = min_x + (gid % width) * (max_x - min_x) / width;
    q[gid].y = min_y + (gid / width) * (max_y - min_y) / height;

    output[gid] = iters;

    for (int curiter = 0; curiter < iters; curiter++) {
        nreal = real * real - imag * imag + q[gid].x;
        imag = 2 * real * imag + q[gid].y;
        real = nreal;

        if (real * real + imag * imag >= 4) {
            output[gid] = ci;
            return;
        }
        ci += inc;
        if (ci == 0 || ci == 255)
            inc = -inc;
    }
}
""").build()


def calc_fractal(q, min_x, max_x, min_y, max_y, width, height, iters):
    global ctx, queue, prg

    output = np.empty(q.shape, dtype=np.uint16)

    mf = cl.mem_flags
    q_opencl = cl.Buffer(ctx, mf.READ_ONLY | mf.COPY_HOST_PTR, hostbuf=q)
    output_opencl = cl.Buffer(ctx, mf.WRITE_ONLY, output.nbytes)

    prg.mandelbrot(queue, output.shape, None, q_opencl, output_opencl,
                   np.double(min_x), np.double(max_x),
                   np.double(min_y), np.double(max_y),
                   np.uint16(width), np.uint16(height), np.uint16(iters))

    cl.enqueue_copy(queue, output, output_opencl).wait()

    return output

def create_fractal(min_x, max_x, min_y, max_y, width, height, iters):
    q = np.zeros(width * height).astype(np.complex128)

    output = calc_fractal(q, min_x, max_x, min_y, max_y, width, height, iters)

    return output


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
