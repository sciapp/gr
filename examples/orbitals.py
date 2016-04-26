# coding: utf-8
"""
Visualization of atomic orbitals using gr.pygr.mlab / GR3.

Based on Atomic Orbitals Pt 2 by Christina C. Lee:
https://albi3ro.github.io/M4/prerequisites/Atomic-Orbitals2.html
"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import numpy as np
from scipy.constants import physical_constants
from pygsl.testing import sf

from gr.pygr.mlab import isosurface

__author__ = 'Florian Rhiem <f.rhiem@fz-juelich.de>'


def Yml(m, l, theta, phi):
    """ The θ and ϕ dependence """
    return (-1)**m * sf.legendre_Plm(l, abs(m), np.cos(theta)) * np.exp(1j * m * phi)


def R(n, l, rho):
    """ The radial dependence """
    return sf.laguerre_n(n-l-1, 2*l+1, rho) * np.exp(-rho/2) * rho**l


def norm(n, l):
    """
    A normalization:
    This is dependent on the choice of polynomial representation
    """
    return np.sqrt((2/n)**3 * factorial(n-l-1)/(2*n*factorial(n+l)))


def orbital(n, l, m):
    # we make sure l and m are within proper bounds
    if l > n:
        raise ValueError()
    if abs(m) > l:
        raise ValueError()

    def psi(radii, theta, phi):
        a0 = physical_constants['Bohr radius'][0]
        rho = 2*radii / (n*a0)
        return norm(n, l) * R(n, l, rho) * np.abs(Yml(m, l, theta, phi))
    return psi


def factorial(n):
    return np.prod(np.arange(1, n+1))


def to_spherical_coordinates(x, y, z):
    r = np.sqrt(x*x+y*y+z*z)
    theta = np.arccos(z/r)
    phi = np.arctan(y/x)
    return r, theta, phi


def main():
    n, l, m = 3, 2, 0
    f = orbital(n, l, m)
    r = 1e-9
    # create coordinates for box width sidelength 2*3
    nx = ny = nz = 50
    coordinates = np.zeros((nz, ny, nx, 3))
    coordinates[:, :, :, 0] = np.linspace(-r, r, nx).reshape(1, 1, nx)
    coordinates[:, :, :, 1] = np.linspace(-r, r, ny).reshape(1, ny, 1)
    coordinates[:, :, :, 2] = np.linspace(-r, r, nz).reshape(nz, 1, 1)
    coordinates.shape = (nx*ny*nz, 3)
    x = coordinates[:, 0]
    y = coordinates[:, 1]
    z = coordinates[:, 2]
    radii, theta, phi = to_spherical_coordinates(x, y, z)
    # evaluate the orbital probability density function
    values = f(radii, theta, phi).reshape(nz, ny, nx)
    # Show rotating orbital
    if values.max() > 0:
        for rotation in range(360):
            isosurface(values/values.max(), isovalue=0.25, color=(0, 0.5, 0.8), rotation=rotation)

if __name__ == '__main__':
    main()