# coding: utf-8
"""
Visualization of atomic orbitals using gr.pygr.mlab / GR3.
"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals


import numpy as np
from scipy.constants import physical_constants
from scipy.special import eval_genlaguerre, lpmn

from gr.pygr.mlab import isosurface

__author__ = 'Florian Rhiem <f.rhiem@fz-juelich.de>, Ingo Heimbach <i.heimbach@fz-juelich.de>'


def get_laguerre_polynomial(n, k):
    return lambda x: eval_genlaguerre(n, k, x)


def get_legendre_polynomial(n, k):
    def P(x):
        return np.array([lpmn(k, n, z)[0][-1, -1] for z in x])
    return P


def get_radial_function(n, l):
    a0 = physical_constants['Bohr radius'][0]
    L = get_laguerre_polynomial(n-l-1, 2*l+1)
    rho = lambda r: (2.0*r) / (n*a0)
    R = lambda r: np.sqrt((2.0/(n*a0))**3 * factorial(n-l-1)/(2*n*factorial(n+l))) * np.exp(-rho(r)/2.0) * rho(r)**l * L(rho(r))
    return R

def get_spherical_harmonic_function(l, m):
    P = get_legendre_polynomial(l, m)
    if m < 0:
        m = abs(m)
        Y = lambda t, p: np.sqrt(2) * np.sqrt(((2.0*l+1)/(4*np.pi)) * ((1.0*factorial(l-m))/(factorial(l+m)))) * P(np.cos(t)) * np.sin(m*p)
    elif m == 0:
        Y = lambda t, p: np.sqrt(((2.0*l+1)/(4*np.pi)) * ((1.0*factorial(l-m))/(factorial(l+m)))) * P(np.cos(t))
    else:
        Y = lambda t, p: np.sqrt(2) * np.sqrt(((2.0*l+1)/(4*np.pi)) * ((1.0*factorial(l-m))/(factorial(l+m)))) * P(np.cos(t)) * np.cos(m*p)
    return Y


def get_orbital_probability_density_function(n, l, m):
    R = get_radial_function(n, l)
    Y = get_spherical_harmonic_function(l, m)

    def R2(r):
        res = R(r)
        return (res*res.conjugate()).real

    def Y2(t, p):
        res = Y(t, p)
        return (res*res.conjugate()).real

    def probability_density_function(r, t, p):
        return R2(r) * Y2(t, p)

    return probability_density_function


def factorial(n):
    return np.prod(np.arange(1, n+1))


def to_spherical_coordinates(x, y, z):
    r = np.sqrt(x*x+y*y+z*z)
    theta = np.arccos(z/r)
    phi = np.arctan(y/x)
    return r, theta, phi


def main():
    f = get_orbital_probability_density_function(3, 2, 0)
    # create coordinates for box width sidelength 2*3
    r = 1e-9
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
        for alpha in np.linspace(0, 2*np.pi, 600):
            isosurface(values/values.max(), isovalue=0.2, color=(0, 0.5, 0.8), camera_position=(2*np.sin(alpha), 1, 2*np.cos(alpha)))

if __name__ == '__main__':
    main()