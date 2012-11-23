import os
from pygr import *

os.environ['GR_DISPLAY'] = "localhost:"
os.system('glgr')
delay(2)

counts = readfile('kws2.dat',
                   separator = '$')
plot3d(counts,
       rotation = 45, tilt = 30,
       colormap = 4, contours = False,
       xtitle = 'X',
       ytitle = 'Y',
       ztitle = 'Counts')
