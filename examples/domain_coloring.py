#!/usr/bin/env python
# -*- animation -*-
"""
Visualize a complex-valued function
"""

import numpy as np
import gr


def domain_colors(w, n):
    H = np.mod(np.angle(w) / (2 * np.pi) + 1, 1)
    m = 0.7
    M = 1
    isol = m + (M - m) * (H * n - np.floor(H * n))
    modul = np.absolute(w)
    Logm = np.log(modul)
    Logm = np.nan_to_num(Logm) * n / (2 * np.pi)
    modc = m + (M - m) * (Logm - np.floor(Logm))

    V = modc * isol
    S = 0.9 * np.ones_like(H, float)
    HSV = np.dstack((H, S, V))

    return HSV


def func_vals(f, re, im,  N):
    # evaluates the complex function at the nodes of the grid
    # re and im are tuples defining the rectangular region
    # N is the number of nodes per unit interval
    l = re[1] - re[0]
    h = im[1] - im[0]
    resL = N * l  # horizontal resolution
    resH = N * h  # vertical resolution
    x = np.linspace(re[0], re[1], resL)
    y = np.linspace(im[0], im[1], resH)
    x, y = np.meshgrid(x, y)
    z = x + 1j * y
    w = f(z)
    return w


def plot_domain(color_func, f, re=[-1, 1], im=[-1, 1], N=100, n=15):
    w = func_vals(f, re, im, N)
    domc = color_func(w, n) * 255
    width, height = domc.shape[:2]
    domc = np.append(domc, np.ones((width, height, 1)) * 255, axis=2)
    domc = domc.astype(np.uint8)
    domc = domc.view('<i4')
    gr.clearws()
    gr.setviewport(0, 1, 0, 1)
    gr.drawimage(0, 1, 0, 1, width, height, domc, model=gr.MODEL_HSV)
    gr.updatews()


f = lambda z: (z**2 - 1) * (z - 2 - 1j)**2 / (z**2 + 2 + 2j)

for n in range(5, 30):
    plot_domain(domain_colors, f, re=(-3, 3), im=(-3, 3), n=n)
