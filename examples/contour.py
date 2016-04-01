#!/usr/bin/env python
"""
3D contour plot
"""

import os
from gr.pygr import *
import numpy as np

z = np.loadtxt(os.path.join(os.path.dirname(os.path.realpath(__file__)),
                            "fecr.dat")).reshape(200, 200)
contour(z)
