#!/usr/bin/env python
# -*- no-plot -*-
"""
3D surface plot from neutron spectrum data
"""

import os
from gr.pygr import *

os.environ["GR_DISPLAY"] = "localhost:"
os.system("glgr")
delay(2)

counts = readfile(os.path.join(os.path.dirname(os.path.realpath(__file__)),
                               "kws2.dat"),
                  separator='$')
plot3d(counts,
       rotation=45, tilt=30,
       colormap=4, contours=False,
       xtitle='X',
       ytitle='Y',
       ztitle="Counts")
