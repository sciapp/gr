#!/usr/bin/env python
# -*- no-plot -*-
"""
3D surface plot from neutron spectrum data
"""

import os
from gr.pygr import *
import numpy as np

os.environ["GR_DISPLAY"] = "localhost:"
os.system("glgr")
delay(2)

counts = np.loadtxt(os.path.join(os.path.dirname(os.path.realpath(__file__)),
                                 "kws2.dat"),
                    skiprows=70).reshape(128, 128)
surface(counts,
        rotation=45, tilt=30,
        colormap=4,
        xlabel='X',
        ylabel='Y',
        zlabel="Counts",
        accelerate=False)
