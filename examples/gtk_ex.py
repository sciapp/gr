#!/usr/bin/python

import gtk

import ctypes
from os import environ

from numpy.random import uniform, seed
import numpy as np
import gr

class PyCairoContext(ctypes.Structure):
  _fields_ = [("PyObject_HEAD", ctypes.c_byte * object.__basicsize__),
              ("ctx", ctypes.c_void_p),
              ("base", ctypes.c_void_p)]

class PyApp(gtk.Window):

    def __init__(self):
        super(PyApp, self).__init__()

        self.set_title("Gtk example")
        self.resize(500, 500)
        self.set_position(gtk.WIN_POS_CENTER)

        self.connect("destroy", gtk.main_quit)

        drawable = gtk.DrawingArea()
        drawable.connect("expose-event", self.expose)
        self.add(drawable)

        self.show_all()

    def expose(self, widget, event):

        cr = widget.window.cairo_create()

        environ["GKS_WSTYPE"] = "142"
        pc = PyCairoContext.from_address(id(cr))
        environ['GKSconid'] = "%lu" % pc.ctx

        cr.move_to(15, 15)
        cr.set_font_size(14)
        cr.show_text("Contour Plot using Gtk ...")

        seed(0)
        xd = uniform(-2, 2, 100)
        yd = uniform(-2, 2, 100)
        zd = xd * np.exp(-xd**2 - yd**2)

        gr.setviewport(0.15, 0.95, 0.1, 0.9)
        gr.setwindow(-2, 2, -2, 2)
        gr.setspace(-0.5, 0.5, 0, 90)
        gr.setmarkersize(1)
        gr.setmarkertype(gr.MARKERTYPE_SOLID_CIRCLE)
        gr.setcharheight(0.024)
        gr.settextalign(2, 0)
        gr.settextfontprec(3, 0)

        x, y, z = gr.gridit(xd, yd, zd, 200, 200)
        h = np.linspace(-0.5, 0.5, 20)
        gr.surface(x, y, z, 5)
        gr.contour(x, y, h, z, 0)
        gr.polymarker(xd, yd)
        gr.axes(0.25, 0.25, -2, -2, 2, 2, 0.01)

        gr.updatews()

PyApp()
gtk.main()
