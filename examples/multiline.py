#!/usr/bin/env python
# -*- no-plot -*-
"""
Multiline text example
"""

import gr
from math import sin, cos, pi
import time

hor_align = {'Left':1, 'Center':2, 'Right':3}
vert_align = {'Top':1, 'Cap':2, 'Half':3, 'Base':4, 'Bottom':5}

gr.selntran(0)
gr.setcharheight(0.024)

for angle in range(361):

  gr.setcharup(sin(-angle * pi/180), cos(-angle * pi/180))
  gr.setmarkertype(2)
  gr.clearws()

  for halign in hor_align:
    for valign in vert_align:
      gr.settextalign(hor_align[halign], vert_align[valign])
      x = -0.1 + hor_align[halign] * 0.3;
      y = 1.1 - vert_align[valign] * 0.2;
      s = halign + '\n' + valign + '\n' + 'third line'
      gr.polymarker([x], [y])
      gr.text(x, y, s)
      tbx, tby = gr.inqtext(x, y, s)
      gr.fillarea(tbx, tby)

  gr.updatews()
  time.sleep(0.02)
