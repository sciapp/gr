#! /usr/bin/env python
import pygtk
pygtk.require('2.0')
import gtk, gobject, cairo
import os, ctypes
import gr

class PyCairoContext(ctypes.Structure):
  _fields_ = [("PyObject_HEAD", ctypes.c_byte * object.__basicsize__),
              ("ctx", ctypes.c_void_p),
              ("base", ctypes.c_void_p)]

# Create a GTK+ widget on which we will draw using Cairo
class Screen(gtk.DrawingArea):

    # Draw in response to an expose-event
    __gsignals__ = { "expose-event": "override" }

    # Handle the expose-event by drawing
    def do_expose_event(self, event):

        # Create the cairo context
        cr = self.window.cairo_create()

        os.environ["GKS_WSTYPE"] = "142"
        os.environ["GKS_DOUBLE_BUF"] = "True"
        pc = PyCairoContext.from_address(id(cr))
        os.environ['GKSconid'] = "%lu" % pc.ctx

        # Restrict Cairo to the exposed area; avoid extra work
        cr.rectangle(event.area.x, event.area.y,
                event.area.width, event.area.height)
        cr.clip()

        self.draw(cr, *self.window.get_size())

    def draw(self, cr, width, height):
        # Fill the background with gray
        cr.set_source_rgb(0.5, 0.5, 0.5)
        cr.rectangle(0, 0, width, height)
        gr.text(0.1, 0.9, "Hello World")
        cr.fill()

# GTK mumbo-jumbo to show the widget in a window and quit when it's closed
def run(Widget):
    window = gtk.Window()
    window.connect("delete-event", gtk.main_quit)
    widget = Widget()
    widget.show()
    window.add(widget)
    window.present()
    gtk.main()

if __name__ == "__main__":
    run(Screen)
