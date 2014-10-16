#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Draw text at an arbitrary position on the Axes using gr.pygr.
"""

# local library
import gr
from gr.pygr import Plot, PlotAxes, PlotCurve, Text

if __name__ == '__main__':
    tx, ty = 0, -20
    x = [-3.3 + t * .1 for t in range(66)]
    y = [t ** 5 - 13 * t ** 3 + 36 * t for t in x]
    txtfmt = "Text drawn on\n(%g, %g) with\nhalign left, valign top"

    plt = Plot((.1, .95, .1, .88))
    plt.title = "Text on Axes Example"
    plt.subTitle ="Show usage of gr.pygr.Text"
    plt.xlabel = "x"
    plt.ylabel = "y"

    curve = PlotCurve(x, y, legend="foo bar")
    axes = PlotAxes(plt.viewport).addCurves(curve)
    axes.setWindow(-4.0, 4.0, -60.0, 40.0)
    text = Text(tx, -ty, txtfmt % (tx, -ty), axes, .02)
    plt.addAxes(axes)
    text2 = Text(tx, ty, txtfmt % (tx, ty), axes, .02)
    tbx, tby = text2.getBoundingBox()

    plt.drawGR()
    text.drawGR()
    text2.drawGR()

    # set viewport and window accordingly to draw in NDC space
    gr.setviewport(0, axes.sizex, 0, axes.sizey)
    gr.setwindow(0, axes.sizex, 0, axes.sizey)
    gr.fillarea(tbx, tby)

