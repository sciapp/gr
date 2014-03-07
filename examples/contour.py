#!/usr/bin/env python
"""
3D contour plot
"""

import os
from gr.pygr import *

z = readfile(os.path.join(os.path.dirname(os.path.realpath(__file__)),
                          "fecr.dat"))
plot3d(z, rotation=0, tilt=90)
