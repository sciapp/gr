from __future__ import (absolute_import, division, print_function,
                        unicode_literals)

from matplotlib.path import Path
from matplotlib._pylab_helpers import Gcf
from matplotlib.backend_bases import RendererBase, GraphicsContextBase,\
     FigureManagerBase, FigureCanvasBase
import matplotlib.transforms as transforms
from matplotlib.figure import Figure
#from matplotlib.texmanager import TexManager

import numpy as np
import gr

linetype = { 'solid' : 1, 'dashed' : 2, 'dashdot' : 4, 'dotted' : 3 }

class RendererGR(RendererBase):
    """
    Handles drawing/rendering operations using GR
    """
    def __init__(self, dpi):
        self.dpi = dpi
        mwidth, mheight, width, height = gr.inqdspsize()
        mwidth *= 640.0 / width
        gr.setwsviewport(0, mwidth, 0, mwidth * 0.75)
        gr.setwswindow(0, 1, 0, 0.75)
        gr.setviewport(0, 1, 0, 0.75)
        gr.setwindow(0, 640, 0, 480)
        #self.texmanager = TexManager()

    def draw_path(self, gc, path, transform, rgbFace=None):
        codes = path.codes
        path = transform.transform_path(path)
        points = path.vertices
        if rgbFace is not None and len(points) > 2:
            color = gr.inqcolorfromrgb(rgbFace[0], rgbFace[1], rgbFace[2])
            gr.setcolorrep(color, rgbFace[0], rgbFace[1], rgbFace[2])
            gr.setfillintstyle(gr.INTSTYLE_SOLID)
            gr.setfillcolorind(color)
            closed = False
            if codes is not None:
                if codes[-1] == Path.CLOSEPOLY:
                    closed = True
            if closed:
                gr.fillarea(points[:-1, 0], points[:-1, 1])
            else:
                gr.fillarea(points[:, 0], points[:, 1])
        lw = gc.get_linewidth()
        if lw != 0:
            rgb = gc.get_rgb()[:3]
            color = gr.inqcolorfromrgb(rgb[0], rgb[1], rgb[2])
            gr.setcolorrep(color, rgb[0], rgb[1], rgb[2])
            gr.setlinetype(linetype[gc._linestyle])
            gr.setlinewidth(lw)
            gr.setlinecolorind(color)
            gr.polyline(points[:, 0], points[:, 1])

#   def draw_markers(self, gc, marker_path, marker_trans, path, trans,
#                    rgbFace=None):
#       if rgbFace is not None:
#           path = trans.transform_path(path)
#           points = path.vertices
#           gr.setmarkertype(gr.MARKERTYPE_PLUS)
#           gr.polymarker(points[:, 0], points[:, 1])

#   def draw_path_collection(self, gc, master_transform, paths, all_transforms,
#                            offsets, offsetTrans, facecolors, edgecolors,
#                            linewidths, linestyles, antialiaseds, urls,
#                            offset_position):
#       path_ids = []
#       for path, transform in self._iter_collection_raw_paths(
#               master_transform, paths, all_transforms):
#           path_ids.append((path, transforms.Affine2D(transform)))

#       for xo, yo, path_id, gc0, rgbFace in self._iter_collection(
#               gc, master_transform, all_transforms, path_ids, offsets,
#               offsetTrans, facecolors, edgecolors, linewidths, linestyles,
#               antialiaseds, urls, offset_position):
#           path, transform = path_id
#           transform = transforms.Affine2D(
#               transform.get_matrix()).translate(xo, yo)
#           self.draw_path(gc0, path, transform, rgbFace)

#   def draw_quad_mesh(self, gc, master_transform, meshWidth, meshHeight,
#                      coordinates, offsets, offsetTrans, facecolors,
#                      antialiased, edgecolors):
#       pass

    def draw_image(self, gc, x, y, im):
        h, w, s = im.as_rgba_str()
        img = np.fromstring(s, np.uint32)
        img.shape = (h, w)
        gr.drawimage(x, x + w, y + h, y, w, h, img)

    def draw_text(self, gc, x, y, s, prop, angle, ismath=False, mtext=None):
        x, y = gr.wctondc(x, y)
        s = s.replace(u'\u2212', '-')
        fontsize = prop.get_size_in_points()
        rgb = gc.get_rgb()[:3]
        color = gr.inqcolorfromrgb(rgb[0], rgb[1], rgb[2])
        gr.setcharheight(fontsize * 0.0013)
        gr.settextcolorind(color)
        if angle != 0:
            gr.setcharup(-np.sin(angle * np.pi/180), np.cos(angle * np.pi/180))
        else:
            gr.setcharup(0, 1)
        if ismath:
            if ismath=='TeX':
                gr.mathtex(x, y, s)
                #x, y = gr.ndctowc(x, y)
                #png = self.texmanager.make_png(s, fontsize=12, dpi=72)
                #w, h, data = gr.readimage(png)
                #gr.drawimage(x, x+h, y, y+h, w, h, data)
            else:
                if s[:1] == '$':
                    s = s[1:-1]
                gr.textext(x, y, s.encode("latin-1"))
        else:
            gr.text(x, y, s.encode("latin-1"))

    def flipy(self):
        return False

    def get_canvas_width_height(self):
        return 640, 480

    def get_text_width_height_descent(self, s, prop, ismath):
        s = s.replace(u'\u2212', '-').encode("latin-1")
        fontsize = prop.get_size_in_points()
        gr.setcharheight(fontsize * 0.0013)
        gr.setcharup(0, 1)
        (tbx, tby) = gr.inqtextext(0, 0, s)
        width, height, descent = tbx[1], tby[2], 0
        return width, height, descent

    def new_gc(self):
        return GraphicsContextGR()

    def points_to_pixels(self, points):
        return points


class GraphicsContextGR(GraphicsContextBase):
    pass


def draw_if_interactive():
    pass


def show():
    for manager in Gcf.get_all_fig_managers():
        manager.show()


def new_figure_manager(num, *args, **kwargs):
    """
    Create a new figure manager instance
    """
    FigureClass = kwargs.pop('FigureClass', Figure)
    thisFig = FigureClass(*args, **kwargs)
    return new_figure_manager_given_figure(num, thisFig)


def new_figure_manager_given_figure(num, figure):
    """
    Create a new figure manager instance for the given figure.
    """
    canvas = FigureCanvasGR(figure)
    manager = FigureManagerGR(canvas, num)
    return manager


class FigureCanvasGR(FigureCanvasBase):
    """
    The canvas the figure renders into.  Calls the draw and print fig
    methods, creates the renderers, etc...
    """

    def draw(self):
        """
        Draw the figure using the renderer
        """
        gr.clearws()
        renderer = RendererGR(self.figure.dpi)
        self.figure.draw(renderer)
        gr.updatews()


class FigureManagerGR(FigureManagerBase):
    """
    Wrap everything up into a window for the pylab interface
    """
    def show(self):
        canvas = FigureCanvasGR(self.canvas.figure)
        FigureCanvasGR.draw(canvas)


FigureCanvas = FigureCanvasGR
FigureManager = FigureManagerGR
