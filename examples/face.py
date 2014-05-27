#!/usr/bin/env python
"""
Simple surface plot example
"""

from gr import * 
from math import *

x = [-2 + i * 0.5 for i in range(0, 29)]
y = [-7 + i * 0.5 for i in range(0, 29)]
z = list(range(0, 841))

for i in range(0, 29):
  for j in range(0, 29):
    r1 = sqrt((x[j] - 5)**2 + y[i]**2)
    r2 = sqrt((x[j] + 5)**2 + y[i]**2)
    z[i * 29 - 1 + j] = (exp(cos(r1)) + exp(cos(r2)) - 0.9) * 25

setcharheight(24.0/500)
settextalign(TEXT_HALIGN_CENTER, TEXT_VALIGN_TOP)
textext(0.5, 0.9, "Surface Example")
(tbx, tby) = inqtextext(0.5, 0.9, "Surface Example")
fillarea(tbx, tby)

setwindow(-2, 12, -7, 7)
setspace(-80, 200, 45, 70)

setcharheight(14.0/500)
axes3d(1, 0, 20, -2, -7, -80, 2, 0, 2, -0.01)
axes3d(0, 1,  0, 12, -7, -80, 0, 2, 0,  0.01)
titles3d("X-Axis", "Y-Axis", "Z-Axis")

surface(x, y, z, 3)
surface(x, y, z, 1)

updatews()
