# coding: utf-8
"""
simple, matlab-style api
"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals


import collections
import numpy as np
import gr
import gr3


def plot(*args, **kwargs):
    global _plt
    _plt.kwargs.update(kwargs)
    if _plt.kwargs['ax']:
        _plt.args += _plot_args(args)
    else:
        _plt.args = _plot_args(args)
    _plot_data(kind='line')


def oplot(*args, **kwargs):
    global _plt
    _plt.kwargs.update(kwargs)
    _plt.args += _plot_args(args)
    _plot_data(kind='line')


def scatter(*args, **kwargs):
    global _plt
    _plt.kwargs.update(kwargs)
    _plt.args = _plot_args(args, fmt='xyac')
    _plot_data(kind='scatter')


def polar(*args, **kwargs):
    global _plt
    _plt.kwargs.update(kwargs)
    _plt.args = _plot_args(args)
    _plot_data(kind='polar')


def trisurf(*args, **kwargs):
    global _plt
    _plt.kwargs.update(kwargs)
    _plt.args = _plot_args(args, fmt='xyzc')
    _plot_data(kind='trisurf')


def tricont(*args, **kwargs):
    global _plt
    _plt.kwargs.update(kwargs)
    _plt.args = _plot_args(args, fmt='xyzc')
    _plot_data(kind='tricont')


def stem(*args, **kwargs):
    global _plt
    _plt.kwargs.update(kwargs)
    _plt.args = _plot_args(args)
    _plot_data(kind='stem')


def _hist(x, nbins=0):
    x = np.array(x)
    x_min = x.min()
    x_max = x.max()
    if nbins <= 1:
        nbins = int(np.round(3.3*np.log10(len(x))))+1
    binned_x = np.array(np.floor((x - x_min) / (x_max - x_min) * nbins), dtype=int)
    binned_x[binned_x == nbins] = nbins-1
    counts = np.bincount(binned_x)
    edges = np.linspace(x_min, x_max, nbins + 1)
    return counts, edges


def histogram(x, **kwargs):
    global _plt
    _plt.kwargs.update(kwargs)
    hist, bins = _hist(x)
    _plt.args = [(np.array(bins), np.array(hist), None, None, "")]
    _plot_data(kind='hist')


def contour(*args, **kwargs):
    global _plt
    _plt.kwargs.update(kwargs)
    _plt.args = _plot_args(args, fmt='xyzc')
    _plot_data(kind='contour')


def contourf(*args, **kwargs):
    global _plt
    _plt.kwargs.update(kwargs)
    _plt.args = _plot_args(args, fmt='xyzc')
    _plot_data(kind='contourf')


def hexbin(*args, **kwargs):
    global _plt
    _plt.kwargs.update(kwargs)
    _plt.args = _plot_args(args)
    _plot_data(kind='hexbin')


def heatmap(d, **kwargs):
    global _plt
    d = np.array(d, copy=False)
    if len(d.shape) != 2:
        raise ValueError('expected 2-D array')
    width, height = d.shape
    _plt.kwargs.update(kwargs)
    _plt.args = [(np.arange(width), np.arange(height), d, None, "")]
    _plot_data(kind='heatmap')


def wireframe(*args, **kwargs):
    global _plt
    _plt.kwargs.update(kwargs)
    _plt.args = _plot_args(args, fmt='xyzc')
    _plot_data(kind='wireframe')


def surface(*args, **kwargs):
    global _plt
    _plt.kwargs.update(kwargs)
    _plt.args = _plot_args(args, fmt='xyzc')
    _plot_data(kind='surface')


def plot3(*args, **kwargs):
    global _plt
    _plt.kwargs.update(kwargs)
    _plt.args = _plot_args(args, fmt='xyac')
    _plot_data(kind='plot3')


def scatter3(*args, **kwargs):
    global _plt
    _plt.kwargs.update(kwargs)
    _plt.args = _plot_args(args, fmt='xyac')
    _plot_data(kind='scatter3')


def isosurface(v, **kwargs):
    global _plt
    _plt.kwargs.update(kwargs)
    _plt.args = [(None, None, v, None, '')]
    _plot_data(kind='isosurface')


def imshow(image, **kwargs):
    global _plt
    _plt.kwargs.update(kwargs)
    _plt.args = [(None, None, image, None, "")]
    _plot_data(kind='imshow')


def title(s):
    _plot_data(title=s)


def xlabel(s):
    _plot_data(xlabel=s)


def ylabel(s):
    _plot_data(ylabel=s)


def xlim(a):
    _plot_data(xlim=a)


def ylim(a):
    _plot_data(ylim=a)


def savefig(filename):
    gr.beginprint(filename)
    _plot_data()
    gr.endprint()


def legend(*args, **kwargs):
    if not all(isinstance(s, basestring) for s in args):
        raise TypeError('list of strings expected')
    _plot_data(labels=args)


def figure(**kwargs):
    global _plt
    _plt = _Figure()
    _plt.kwargs.update(kwargs)
    return _plt


def hold(flag):
    global _plt
    _plt.kwargs['ax'] = flag
    _plt.kwargs['clear'] = not flag


def subplot(nr, nc, p):
    global _plt
    x_min = y_min = 1
    x_max = y_max = 0
    for i in p:
        r = nr - (i-1) // nc
        c = (i-1) % nc + 1
        x_min = min(x_min, (c-1)/nc)
        x_max = max(x_max, c/nc)
        y_min = min(y_min, (r-1)/nr)
        y_max = max(y_max, r/nr)
    _plt.kwargs['subplot'] = [xmin, xmax, ymin, ymax]
    _plt.kwargs['clear'] = (p[0] == 1)
    _plt.kwargs['update'] = (p[-1] == nr * nc)


class _Figure(object):
    def __init__(self, width=600, height=450):
        self.args = []
        self.kwargs = {
            'size': (width, height),
            'ax': False,
            'subplot': [0, 1 , 0, 1],
            'clear': True,
            'update': True
        }
_plt = _Figure()


def _colormap():
    rgba = np.ones((256, 4), np.float32)
    for color_index in range(256):
        color = gr.inqcolor(1000+color_index)
        rgba[color_index, 0] = ( color        % 256) / 255.0
        rgba[color_index, 1] = ((color >> 8)  % 256) / 255.0
        rgba[color_index, 2] = ((color >> 16) % 256) / 255.0
    return rgba


def _set_viewport(kind, subplot):
    global _plt
    metric_width, metric_height, pixel_width, pixel_height = gr.inqdspsize()
    if 'figsize' in _plt.kwargs:
        horizontal_pixels_per_inch = pixel_width * 0.0254 / metric_width
        vertical_pixels_per_inch = pixel_height * 0.0254 / metric_height
        width = _plt.kwargs['figsize'][0] * horizontal_pixels_per_inch
        height = _plt.kwargs['figsize'][1] * vertical_pixels_per_inch
    else:
        dpi = pixel_width / metric_width * 0.0254
        if dpi > 200:
            width, height = tuple(x * dpi / 100 for x in _plt.kwargs['size'])
        else:
            width, height = _plt.kwargs['size']

    viewport = [0, 0, 0, 0]
    vp = subplot[:]
    if width > height:
        aspect_ratio = height/width
        metric_size = metric_width * width / pixel_width
        gr.setwsviewport(0, metric_size, 0, metric_size*aspect_ratio)
        gr.setwswindow(0, 1, 0, aspect_ratio)
        vp[2] *= aspect_ratio
        vp[3] *= aspect_ratio
    else:
        aspect_ratio = width/ height
        metric_size = metric_height * height / pixel_height
        gr.setwsviewport(0, metric_size * aspect_ratio, 0, metric_size)
        gr.setwswindow(0, aspect_ratio, 0, 1)
        vp[0] *= aspect_ratio
        vp[1] *= aspect_ratio
    viewport[0] = vp[0] + 0.125 * (vp[1]-vp[0])
    viewport[1] = vp[0] + 0.925 * (vp[1]-vp[0])
    viewport[2] = vp[2] + 0.125 * (vp[3]-vp[2])
    viewport[3] = vp[2] + 0.925 * (vp[3]-vp[2])

    if width > height:
        viewport[2] += (1 - (subplot[3] - subplot[2])**2) * 0.02
    if kind in ('wireframe', 'surface', 'plot3', 'scatter3', 'trisurf'):
        viewport[1] -= 0.0525
    if kind in ('contour', 'contourf', 'surface', 'trisurf', 'heatmap', 'hexbin'):
        viewport[1] -= 0.1
    gr.setviewport(*viewport)
    _plt.kwargs['viewport'] = viewport
    _plt.kwargs['vp'] = vp
    _plt.kwargs['ratio'] = aspect_ratio

    if 'backgroundcolor' in _plt.kwargs:
        gr.savestate()
        gr.selntran(0)
        gr.setfillintstyle(gr.INTSTYLE_SOLID)
        gr.setfillcolorind(_plt.kwargs['backgroundcolor'])
        if width > height:
            gr.fillrect(subplot[0], subplot[1],
                        subplot[2] * aspect_ratio, subplot[3] * aspect_ratio)
        else:
            gr.fillrect(subplot[0] * aspect_ratio, subplot[1] * aspect_ratio,
                        subplot[2], subplot[3])
        gr.selntran(1)
        gr.restorestate()

    if kind == 'polar':
        x_min, x_max, y_min, y_max = viewport
        x_center = 0.5 * (x_min + x_max)
        y_center = 0.5 * (y_min + y_max)
        r = 0.5 * min(x_max - x_min, y_max - y_min)
        gr.setviewport(x_center - r, x_center + r, y_center - r, y_center + r)



def _minmax():
    global _plt
    x_min = y_min = z_min = float('infinity')
    x_max = y_max = z_max = float('-infinity')

    for x, y, z, c, spec in _plt.args:
        x_min = min(x.min(), x_min)
        x_max = max(x.max(), x_max)
        y_min = min(y.min(), y_min)
        y_max = max(y.max(), y_max)
        if z is not None:
            z_min = min(z.min(), z_min)
            z_max = max(z.max(), z_max)

    _plt.kwargs['xrange'] = _plt.kwargs.get('xlim', (x_min, x_max))
    _plt.kwargs['yrange'] = _plt.kwargs.get('ylim', (y_min, y_max))
    _plt.kwargs['zrange'] = _plt.kwargs.get('zlim', (z_min, z_max))


def _set_window(kind):
    global _plt
    scale = 0
    if kind != 'polar':
        scale |= gr.OPTION_X_LOG if _plt.kwargs.get('xlog', False) else 0
        scale |= gr.OPTION_Y_LOG if _plt.kwargs.get('ylog', False) else 0
        scale |= gr.OPTION_Z_LOG if _plt.kwargs.get('zlog', False) else 0
        scale |= gr.OPTION_FLIP_X if _plt.kwargs.get('xflip', False) else 0
        scale |= gr.OPTION_FLIP_Y if _plt.kwargs.get('yflip', False) else 0
        scale |= gr.OPTION_FLIP_Z if _plt.kwargs.get('zflip', False) else 0

    _minmax()
    if kind in ('wireframe', 'surface', 'plot3', 'scatter3', 'polar', 'trisurf'):
        major_count = 2
    else:
        major_count = 5

    x_min, x_max = _plt.kwargs['xrange']
    if not scale & gr.OPTION_X_LOG:
        x_min, x_max = gr.adjustlimits(x_min, x_max)
        x_major_count = major_count
        x_tick = gr.tick(x_min, x_max) / x_major_count
    else:
        x_tick = x_major_count = 1
    if not scale & gr.OPTION_FLIP_X:
        xorg = (x_min, x_max)
    else:
        xorg = (x_max, x_min)
    _plt.kwargs['xaxis'] = x_tick, xorg, x_major_count

    y_min, y_max = _plt.kwargs['yrange']
    if kind in ('hist', 'stem') and 'ylim' not in _plt.kwargs:
        y_min = 0
    if not scale & gr.OPTION_Y_LOG:
        y_min, y_max = gr.adjustlimits(y_min, y_max)
        y_major_count = major_count
        y_tick = gr.tick(y_min, y_max) / y_major_count
    else:
        y_tick = y_major_count = 1
    if not scale & gr.OPTION_FLIP_Y:
        yorg = (y_min, y_max)
    else:
        yorg = (y_max, y_min)
    _plt.kwargs['yaxis'] = y_tick, yorg, y_major_count

    _plt.kwargs['window'] = (x_min, x_max, y_min, y_max)
    if kind == 'polar':
        gr.setwindow(-1, 1, -1, 1)
    else:
        gr.setwindow(x_min, x_max, y_min, y_max)

    if kind in ('wireframe', 'surface', 'plot3', 'scatter3', 'trisurf'):
        z_min, z_max = _plt.kwargs['zrange']
        if not scale & gr.OPTION_Z_LOG:
            z_min, z_max = gr.adjustlimits(z_min, z_max)
            z_major_count = major_count
            z_tick = gr.tick(z_min, z_max) / z_major_count
        else:
            z_tick = z_major_count = 1
        if not scale & gr.OPTION_FLIP_Z:
            zorg = (z_min, z_max)
        else:
            zorg = (z_max, z_min)
        _plt.kwargs['zaxis'] = z_tick, zorg, z_major_count

        rotation = _plt.kwargs.get('rotation', 40)
        tilt = _plt.kwargs.get('tilt', 70)
        gr.setspace(z_min, z_max, rotation, tilt)

    _plt.kwargs['scale'] = scale
    gr.setscale(scale)

def _draw_axes(kind, pass_=1):
    global _plt
    viewport = _plt.kwargs['viewport']
    vp = _plt.kwargs['vp']
    x_tick, x_org, x_major_count = _plt.kwargs['xaxis']
    y_tick, y_org, y_major_count = _plt.kwargs['yaxis']

    gr.setlinecolorind(1)
    gr.setlinewidth(1)
    diag = ((viewport[1]-viewport[0])**2 + (viewport[3]-viewport[2])**2)**0.5
    charheight = max(0.018 * diag, 0.012)
    gr.setcharheight(charheight)
    ticksize = 0.0075 * diag
    if kind in ('wireframe', 'surface', 'plot3', 'scatter3', 'trisurf'):
        z_tick, z_org, z_major_count = _plt.kwargs['zaxis']
        if pass_ == 1:
            gr.grid3d(x_tick, 0, z_tick, x_org[0], y_org[1], z_org[0], 2, 0, 2)
            gr.grid3d(0, y_tick, 0, x_org[0], y_org[1], z_org[0], 0, 2, 0)
        else:
            gr.axes3d(x_tick, 0, z_tick, x_org[0], y_org[0], z_org[0], x_major_count, 0, z_major_count, -ticksize)
            gr.axes3d(0, y_tick, 0, x_org[1], y_org[0], z_org[0], 0, y_major_count, 0, ticksize)
    else:
        if kind == 'heatmap':
            ticksize = -ticksize
        else:
            gr.grid(x_tick, y_tick, 0, 0, x_major_count, y_major_count)
        gr.axes(x_tick, y_tick, x_org[0], y_org[0], x_major_count, y_major_count, ticksize)
        gr.axes(x_tick, y_tick, x_org[1], y_org[1], -x_major_count, -y_major_count, -ticksize)

    if 'title' in _plt.kwargs:
        gr.savestate()
        gr.settextalign(gr.TEXT_HALIGN_CENTER, gr.TEXT_VALIGN_TOP)
        gr.textext(0.5*(viewport[0] + viewport[1]), vp[3], _plt.kwargs['title'])
        gr.restorestate()

    if kind in ('wireframe', 'surface', 'plot3', 'scatter3', 'trisurf'):
        x_label = _plt.kwargs.get('xlabel', '')
        y_label = _plt.kwargs.get('ylabel', '')
        z_label = _plt.kwargs.get('zlabel', '')
        gr.titles3d(x_label, y_label, z_label)
    else:
        if 'xlabel' in _plt.kwargs:
            gr.savestate()
            gr.settextalign(gr.TEXT_HALIGN_CENTER, gr.TEXT_VALIGN_BOTTOM)
            gr.textext(0.5 * (viewport[0] + viewport[1]), vp[2] + 0.5 * charheight, _plt.kwargs['xlabel'])
            gr.restorestate()
        if 'ylabel' in _plt.kwargs:
            gr.savestate()
            gr.settextalign(gr.TEXT_HALIGN_CENTER, gr.TEXT_VALIGN_TOP)
            gr.setcharup(-1, 0)
            gr.textext(vp[0] + 0.5 * charheight, 0.5 * (viewport[2] + viewport[3]), _plt.kwargs['ylabel'])
            gr.restorestate()


def _draw_polar_axes():
    global _plt
    viewport = _plt.kwargs['viewport']
    diag = ((viewport[1]-viewport[0])**2 + (viewport[3]-viewport[2])**2)**0.5
    charheight = max(0.018 * diag, 0.012)


    window = _plt.kwargs['window']
    r_min, r_max = window[2], window[3]

    gr.savestate()
    gr.setcharheight(charheight)
    gr.setlinetype(gr.LINETYPE_SOLID)

    tick = 0.5 * gr.tick(r_min, r_max)
    n = int(round((r_max - r_min) / tick + 0.5))
    for i in range(n+1):
        r = i / n
        if i % 2 == 0:
            gr.setlinecolorind(88)
            if i > 0:
                gr.drawarc(-r, r, -r, r, 0, 359)
            gr.settextalign(gr.TEXT_HALIGN_LEFT, gr.TEXT_VALIGN_HALF)
            x, y = gr.wctondc(0.05, r)
            gr.text(x, y, "%g" % (r_min + i * tick))
        else:
            gr.setlinecolorind(90)
            gr.drawarc(-r, r, -r, r, 0, 359)
    for alpha in range(0, 360, 45):
        sinf = np.sin(np.radians(alpha+90))
        cosf = np.cos(np.radians(alpha+90))
        gr.polyline([sinf, 0], [cosf, 0])
        gr.settextalign(gr.TEXT_HALIGN_CENTER, gr.TEXT_VALIGN_HALF)
        x, y = gr.wctondc(1.1 * sinf, 1.1 * cosf)
        gr.textext(x, y, "%d^o" % alpha)
    gr.restorestate()

def _draw_legend():
    global _plt
    viewport = _plt.kwargs['viewport']
    num_labels = len(_plt.kwargs['labels'])
    location = _plt.kwargs.get('location', 1)
    gr.savestate()
    gr.selntran(0)
    gr.setscale(0)
    w = 0
    for label in _plt.kwargs['labels']:
        tbx, tby = gr.inqtextext(0, 0, label)
        w = max(w, tbx[2])

    num_lines = len(_plt.args)
    h = (num_lines + 1) * 0.03
    if location in (8, 9, 10):
        px = 0.5 * (viewport[0] + viewport[1] - w)
    elif location in (2, 3, 6):
        px = viewport[0] + 0.11
    else:
        px = viewport[1] - 0.05 - w
    if location in (5, 6, 7, 10):
        py = 0.5 * (viewport[2] + viewport[3] + h) - 0.03
    elif location in (3, 4, 8):
        py = viewport[2] + h
    else:
        py = viewport[3] - 0.06

    gr.setfillintstyle(gr.INTSTYLE_SOLID)
    gr.setfillcolorind(0)
    gr.fillrect(px - 0.08, px + w + 0.02, py + 0.03, py - 0.03 * num_lines)
    gr.setlinetype(gr.LINETYPE_SOLID)
    gr.setlinecolorind(1)
    gr.setlinewidth(1)
    gr.drawrect(px - 0.08, px + w + 0.02, py + 0.03, py - 0.03 * num_lines)
    i = 0
    gr.uselinespec(" ")
    for (x, y, z, c, spec) in _plt.args:
        gr.savestate()
        mask = gr.uselinespec(spec)
        if mask in (0, 1, 3, 4, 5):
            gr.polyline([px - 0.07, px - 0.01], [py, py])
        if mask & 2:
            gr.polymarker([px - 0.06, px - 0.02], [py, py])
        gr.restorestate()
        gr.settextalign(gr.TEXT_HALIGN_LEFT, gr.TEXT_VALIGN_HALF)
        if i < num_labels:
            gr.textext(px, py, _plt.kwargs['labels'][i])
            i += 1
        py -= 0.03
    gr.selntran(1)
    gr.restorestate()


def _colorbar(off=0.0, colors=256):
    global _plt
    gr.savestate()
    viewport = _plt.kwargs['viewport']
    zmin, zmax = _plt.kwargs['zrange']
    gr.setwindow(0, 1, zmin, zmax)
    gr.setviewport(viewport[1] + 0.02 + off, viewport[1] + 0.05 + off,
                   viewport[2], viewport[3])

    l = [1000+int(255*i/(colors-1)) for i in range(colors)]

    gr.cellarray(0, 1, zmax, zmin, 1, colors, l)
    diag = ((viewport[1] - viewport[0])**2 + (viewport[3] - viewport[2])**2)**0.5
    charheight = max(0.016 * diag, 0.012)
    gr.setcharheight(charheight)
    if _plt.kwargs['scale'] & gr.OPTION_Z_LOG:
        gr.setscale(gr.OPTION_Y_LOG)
        gr.axes(0, 2, 1, zmin, 0, 1, 0.005)
    else:
        ztick = 0.5 * gr.tick(zmin, zmax)
        gr.axes(0, ztick, 1, zmin, 0, 1, 0.005)
    gr.restorestate()


def _plot_data(**kwargs):
    global _plt
    _plt.kwargs.update(kwargs)
    if not _plt.args:
        return
    kind = _plt.kwargs.get('kind', 'line')
    if _plt.kwargs['clear']:
        gr.clearws()
    if kind in ('imshow', 'isosurface'):
        _set_viewport(kind, _plt.kwargs['subplot'])
    elif not _plt.kwargs['ax']:
        _set_viewport(kind, _plt.kwargs['subplot'])
        _set_window(kind)
        if kind == 'polar':
            _draw_polar_axes()
        else:
            _draw_axes(kind)

    gr.setcolormap(_plt.kwargs.get('colormap', gr.COLORMAP_COOLWARM))
    gr.uselinespec(" ")
    for x, y, z, c, spec in _plt.args:
        gr.savestate()
        if 'alpha' in _plt.kwargs:
            gr.settransparency(_plt.kwargs['alpha'])
        if kind == 'line':
            mask = gr.uselinespec(spec)
            if mask in (0, 1, 3, 4, 5):
                gr.polyline(x, y)
            if mask & 2:
                gr.polymarker(x, y)
        elif kind == 'scatter':
            gr.setmarkertype(gr.MARKERTYPE_SOLID_CIRCLE)
            if z is not None or c is not None:
                if c is not None:
                    c_min = c.min()
                    c_ptp = c.ptp()
                for i in range(len(x)):
                    if z is not None:
                        gr.setmarkersize(z[i] / 100.0)
                    if c is not None:
                        c_index = 1000 + int(255 * (c[i]-c_min)/c_ptp)
                        gr.setmarkercolorind(c_index)
                    gr.polymarker([x[i]], [y[i]])
            else:
                gr.polymarker(x, y)
        elif kind == 'stem':
            gr.setlinecolorind(1)
            gr.polyline(_plt.kwargs['window'][:2], [0, 0])
            gr.setmarkertype(gr.MARKERTYPE_SOLID_CIRCLE)
            gr.uselinespec(spec)
            for xi, yi in zip(x, y):
                gr.polyline([xi, xi], [0, yi])
            gr.polymarker(x, y)
        elif kind == 'hist':
            y_min = _plt.kwargs['window'][2]
            for i in range(1, len(y)+1):
                gr.setfillcolorind(989)
                gr.setfillintstyle(gr.INTSTYLE_SOLID)
                gr.fillrect(x[i-1], x[i], y_min, y[i-1])
                gr.setfillcolorind(1)
                gr.setfillintstyle(gr.INTSTYLE_HOLLOW)
                gr.fillrect(x[i-1], x[i], y_min, y[i-1])
        elif kind == 'contour':
            z_min, z_max = _plt.kwargs['zrange']
            gr.setspace(z_min, z_max, 0, 90)
            h = [z_min + i/19*(z_max-z_min) for i in range(20)]
            if x.shape == y.shape == z.shape:
                x, y, z = gr.gridit(x, y, z, 200, 200)
            z.shape = np.prod(z.shape)
            gr.contour(x, y, h, z, 1000)
            _colorbar(0, 20)
        elif kind == 'contourf':
            z_min, z_max = _plt.kwargs['zrange']
            gr.setspace(z_min, z_max, 0, 90)
            scale = _plt.kwargs['scale']
            gr.setscale(scale)
            if x.shape == y.shape == z.shape:
                x, y, z = gr.gridit(x, y, z, 200, 200)
                z.shape = (200, 200)
            gr.surface(x, y, z, gr.OPTION_CELL_ARRAY)
            _colorbar()
        elif kind == 'hexbin':
            nbins = _plt.kwargs.get('nbins', 40)
            cntmax = gr.hexbin(x, y, nbins)
            if cntmax > 0:
                _plt.kwargs['zrange'] = (0, cntmax)
                _colorbar()
        elif kind == 'heatmap':
            x_min, x_max, y_min, y_max = _plt.kwargs['window']
            width, height = z.shape
            cmap = _colormap()
            icmap = np.zeros(256, np.uint32)
            for i in range(256):
                r, g, b, a = cmap[i]
                icmap[i] = (int(r*255) << 0) + (int(g*255) << 8) + (int(b*255) << 16) + (int(a*255) << 24)
            z_range = np.ptp(z)
            if z_range > 0:
                data = (z - np.min(z)) / z_range * 255
            else:
                data = np.zeros((width, height))
            rgba = np.zeros((width, height), np.uint32)
            for x in range(width):
                for y in range(height):
                    rgba[x, y] = icmap[int(data[x, y])]
            gr.drawimage(x_min, x_max, y_min, y_max, width, height, rgba)
            _colorbar()
        elif kind == 'wireframe':
            if x.shape == y.shape == z.shape:
                x, y, z = gr.gridit(x, y, z, 50, 50)
            gr.setfillcolorind(0)
            z.shape = np.prod(z.shape)
            gr.surface(x, y, z, gr.OPTION_FILLED_MESH)
            _draw_axes(kind, 2)

        elif kind == 'surface':
            if x.shape == y.shape == z.shape:
                x, y, z = gr.gridit(x, y, z, 200, 200)
            z.shape = np.prod(z.shape)
            if _plt.kwargs.get('accelerate', True):
                gr3.clear()
                gr3.surface(x, y, z, gr.OPTION_COLORED_MESH)
            else:
                gr.surface(x, y, z, gr.OPTION_COLORED_MESH)
            _draw_axes(kind, 2)
            _colorbar(0.05)
        elif kind == 'plot3':
            gr.polyline3d(x, y, z)
            _draw_axes(kind, 2)
        elif kind == 'scatter3':
            gr.polymarker3d(x, y, z)
            _draw_axes(kind, 2)
        elif kind == 'imshow':
            _plot_img(z)
        elif kind == 'isosurface':
            _plot_iso(z)
        elif kind == 'polar':
            gr.uselinespec(spec)
            _plot_polar(x, y)
        elif kind == 'trisurf':
            gr.trisurface(x, y, z)
            _draw_axes(kind, 2)
            _colorbar(0.05)
        elif kind == 'tricont':
            zmin, zmax = _plt.kwargs['zrange']
            levels = np.linspace(zmin, zmax, 20)
            gr.tricontour(x, y, z, levels)
        gr.restorestate()
    if kind in ('line', 'scatter', 'stem') and 'labels' in _plt.kwargs:
        _draw_legend()

    if _plt.kwargs['update']:
        gr.updatews()
        if gr.isinline():
            return gr.show()


def _plot_img(I):
    global _plt

    if isinstance(I, basestring):
        width, height, data = gr.readimage(I)
        if width == 0 or height == 0:
            return
    else:
        I = np.array(I)
        width, height = I.shape
        data = np.array(1000+(1.0*I - I.min()) / I.ptp() * 255, np.int32)

    if _plt.kwargs['clear']:
        gr.clearws()

    if not _plt.kwargs['ax']:
        _set_viewport('line', _plt.kwargs['subplot'])

    viewport = _plt.kwargs['viewport']
    vp = _plt.kwargs['vp']

    if width * (viewport[3] - viewport[2]) < height * (viewport[1] - viewport[0]):
        w = width / height * (viewport[3] - viewport[2])
        x_min = max(0.5 * (viewport[0] + viewport[1] - w), viewport[0])
        x_max = min(0.5 * (viewport[0] + viewport[1] + w), viewport[1])
        y_min = viewport[2]
        y_max = viewport[3]
    else:
        h = height / width * (viewport[1] - viewport[0])
        x_min = viewport[0]
        x_max = viewport[1]
        y_min = max(0.5 * (viewport[3] + viewport[2] - h), viewport[2])
        y_max = min(0.5 * (viewport[3] + viewport[2] + h), viewport[3])

    gr.setcolormap(_plt.kwargs.get('cmap', 1))
    gr.selntran(0)
    if isinstance(I, basestring):
        gr.drawimage(x_min, x_max, y_min, y_max, width, height, data)
    else:
        gr.cellarray(x_min, x_max, y_min, y_max, width, height, data)

    if 'title' in _plt.kwargs:
        gr.savestate()
        gr.settextalign(gr.TEXT_HALIGN_CENTER, gr.TEXT_VALIGN_TOP)
        gr.textext(0.5 * (viewport[0] + viewport[1]), vp[3], _plt.kwargs['title'])
        gr.restorestate()
    gr.selntran(1)


def _plot_iso(v):
    global _plt
    viewport = _plt.kwargs['viewport']
    if viewport[3] - viewport[2] < viewport[1] - viewport[0]:
        width = viewport[3] - viewport[2]
        center_x = 0.5 * (viewport[0] + viewport[1])
        x_min = max(center_x - 0.5 * width, viewport[0])
        x_max = min(center_x + 0.5 * width, viewport[1])
        y_min = viewport[2]
        y_max = viewport[3]
    else:
        height = viewport[1] - viewport[0]
        center_y = 0.5 * (viewport[2] + viewport[3])
        x_min = viewport[0]
        x_max = viewport[1]
        y_min = max(center_y - 0.5 * height, viewport[2])
        y_max = min(center_y + 0.5 * height, viewport[3])

    gr.selntran(0)
    usable_vs = v[np.abs(v) != np.inf]
    if np.prod(usable_vs.shape) == 0:
        return
    v_max = usable_vs.max()
    v_min = usable_vs.min()
    if v_min == v_max:
        return
    uint16_max = np.iinfo(np.uint16).max
    v = (np.clip(v, v_min, v_max) - v_min) / (v_max - v_min) * uint16_max
    values = v.astype(np.uint16)
    nx, ny, nz = v.shape
    isovalue = _plt.kwargs.get('isovalue', 0.5)
    rotation = np.radians(_plt.kwargs.get('rotation', 40))
    tilt = np.radians(_plt.kwargs.get('tilt', 70))
    gr3.clear()
    mesh = gr3.createisosurfacemesh(values, (2/(nx-1), 2/(ny-1), 2/(nz-1)),
                                    (-1., -1., -1.),
                                    int(isovalue * uint16_max))
    color = _plt.kwargs.get('color', (0.0, 0.5, 0.8))
    gr3.setbackgroundcolor(1, 1, 1, 0)
    gr3.drawmesh(mesh, 1, (0, 0, 0), (0, 0, 1), (0, 1, 0), color, (1, 1, 1))
    r = 2.5
    gr3.cameralookat(r*np.sin(tilt)*np.sin(rotation), r*np.cos(tilt), r*np.sin(tilt)*np.cos(rotation), 0, 0, 0, 0, 1, 0)
    gr3.drawimage(x_min, x_max, y_min, y_max, 500, 500, gr3.GR3_Drawable.GR3_DRAWABLE_GKS)
    gr3.deletemesh(mesh)
    gr.selntran(1)


def _plot_polar(theta, rho):
    global _plt
    window = _plt.kwargs['window']
    r_min, r_max = window[2:]
    rho = (rho - r_min) / (r_max - r_min)
    x = rho * np.cos(theta)
    y = rho * np.sin(theta)
    gr.polyline(x, y)


def _convert_to_array(obj, may_be_2d=False, xvalues=None, always_flatten=False):
    global _plt
    if callable(obj):
        if xvalues is None:
            raise TypeError('object is callable, but xvalues is None')
        if len(xvalues.shape) == 1:
            a = np.fromiter((obj(x) for x in xvalues), np.float64)
        else:
            a = np.fromiter((obj(x, y) for x, y in xvalues), np.float64)
    else:
        a = obj
        try:
            if obj.__iter__() is obj:
                a = np.fromiter(obj, np.float64, len(xvalues))
        except (AttributeError, TypeError):
            pass
    try:
        a = np.array(a, copy=False)
    except TypeError:
        raise TypeError("expected a sequence, but got '{}'".format(type(obj)))
    if always_flatten:
        a.shape = np.prod(a.shape)
    # Ensure a shape of (n,) or (n, 2) if may_be_2d is True
    dimension = sum(i != 1 for i in a.shape)
    if may_be_2d and dimension > 2:
        raise TypeError("expected a 1d- or 2d-sequence, but got shape {}".format(a.shape))
    if not may_be_2d and dimension > 1:
        raise TypeError("expected a 1d-sequence, but got shape {}".format(a.shape))
    if dimension == 0:
        dimension = 1
        a.shape = [1]
    else:
        a.shape = [i for i in a.shape if i != 1]
    if dimension == 2:
        if 2 not in a.shape:
            raise TypeError("expected a sequence of pairs, but got shape {}".format(a.shape))
        if a.shape[0] == 2 and a.shape[1] != 2:
            a = a.T
    n = len(a)
    assert a.shape == (n,) or a.shape == (n, 2)

    if a.dtype == complex:
        if dimension != 1:
            raise TypeError("expected a sequence of complex values, but got shape {}".format(a.shape))
        a = np.stack((np.real(a), np.imag(a)), axis=1)
        dimension = 2
    elif a.dtype != np.float64:
        try:
            a = a.astype(np.float64)
        except (TypeError, ValueError):
            raise TypeError("expected a sequence of real values, but got '{}'".format(a.dtype))
    return a


def _plot_args(args, fmt='xys'):
    global _plt
    args = list(args)
    parsed_args = []
    while args:
        # Try to read x, y, z and c
        x = y = z = c = None
        if fmt == 'xyzc' and len(args) == 1:
            try:
                a = np.array(args[0])
                if max(a.shape) == 1:
                    a.shape = [1, 1]
                else:
                    a.shape = [i for i in a.shape if i != 1]
                if len(a.shape) == 1:
                    a.shape = [len(a), 1]
                if a.dtype == complex:
                    raise TypeError()
                x = np.arange(1, a.shape[0]+1)
                y = np.arange(1, a.shape[1]+1)
                z = a.astype(np.float64)
                args.pop(0)
            except TypeError:
                x = y = z = c = None

        if x is None:
            a = _convert_to_array(args.pop(0), may_be_2d=True)

            if len(a.shape) == 2:
                x, y = a[:, 0], a[:, 1]
            else:
                if fmt == 'xys':
                    try:
                        x = a
                        y = _convert_to_array(args[0], xvalues=x)
                        args.pop(0)
                    except (TypeError, IndexError):
                        y = a
                        x = np.arange(1, len(a)+1)
                elif fmt == 'xyac' or fmt == 'xyzc':
                    try:
                        x = a
                        if fmt == 'xyac' and len(args) >= 1:
                            y = _convert_to_array(args[0], xvalues=x)
                        if len(args) >= 2:
                            y = _convert_to_array(args[0])
                            xy_y, xy_x = np.meshgrid(y, x)
                            xy_x.shape = np.prod(xy_x.shape)
                            xy_y.shape = np.prod(xy_y.shape)
                            xy = np.stack((xy_x, xy_y), axis=1)
                            if fmt == 'xyzc':
                                z = _convert_to_array(args[1], xvalues=xy, always_flatten=True)
                                if z.shape != x.shape or z.shape != y.shape:
                                    z.shape = (len(x), len(y))
                            else:
                                z = _convert_to_array(args[1], xvalues=x)
                        if len(args) >= 3:
                            if fmt == 'xyzc':
                                c = _convert_to_array(args[2], xvalues=xy, always_flatten=True)
                                c.shape = (len(x), len(y))
                            else:
                                c = _convert_to_array(args[2], xvalues=x)
                    except TypeError:
                        pass
                    if y is None:
                        raise TypeError('expected callable or sequence of real values')
                    for v in (y, z, c):
                        if v is None:
                            break
                        args.pop(0)
                else:
                    raise TypeError("Invalid format: '{}'".format(fmt))

        # Remove unused values
        if z is None:
            if len(x) > len(y):
                x = x[:len(y)]
            elif len(x) < len(y):
                y = y[:len(x)]
        else:
            if fmt == 'xyzc':
                if z.shape[0] > len(x):
                    z = z[:len(x), :]
                elif z.shape[0] < len(x):
                    x = x[:z.shape[0]]
                if len(z.shape) > 1 and z.shape[1] > len(y):
                    z = z[:, len(y)]
                elif len(z.shape) > 1 and z.shape[1] < len(y):
                    y = y[:z.shape[1]]
                if c is not None:
                    if c.shape[0] > len(x):
                        c = c[:len(x), :]
                    elif c.shape[0] < len(x):
                        x = x[:c.shape[0]]
                        z = z[:c.shape[0], 0]
                    if c.shape[1] > len(y):
                        c = c[:, len(y)]
                    elif c.shape[1] < len(y):
                        y = y[:c.shape[1]]
                        z = z[:, :c.shape[1]]
                if z is not None:
                    z = np.ascontiguousarray(z.T)
                if c is not None:
                    c = np.ascontiguousarray(c.T)
            else:
                if len(x) > len(y):
                    x = x[:len(y)]
                else:
                    y = y[:len(x)]
                if len(z) > len(x):
                    z = z[:len(x)]
                else:
                    x = x[:len(z)]
                    y = y[:len(z)]
                if c is not None:
                    if len(c) > len(z):
                        c = c[:len(z)]
                    else:
                        x = x[:len(c)]
                        y = y[:len(c)]
                        z = z[:len(c)]
        # Try to read the spec if available
        spec = ""
        if fmt == 'xys' and len(args) > 0 and isinstance(args[0], basestring):
            spec = args.pop(0)
        parsed_args.append((x, y, z, c, spec))
    return parsed_args
