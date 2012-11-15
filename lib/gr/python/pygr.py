__all__ = ['delay', 'gr', 'plot', 'plot3d', 'readfile']

import math
import time
import gr

def _guessdimension(len):
    x = int(math.sqrt(len))
    d = []
    while x >= 1:
        y = len / x
        if x * y == len:
            d.append((x, y))
        x -= 1
    return sorted(d, key=lambda t: t[1] - t[0])

def readfile(path, separator = ''):
    fp = open(path, "r")
    if separator != '':
        while True:
            line = fp.readline()
            if not line:
                break
            if line[0] == separator:
                break
    data = []
    for line in fp.readlines():
        for item in line.split():
            data.append(float(item))
    fp.close()
    return data
 
def plot(x, y,
         bgcolor = 0,
         viewport = (0.1, 0.95, 0.1, 0.95),
         window = None,
         scale = 0,
         grid = True,
         linetype = gr.LINETYPE_SOLID,
         markertype = gr.MARKERTYPE_DOT,
         clear = True,
         update = True):
    if clear:
        gr.clearws()
    n = len(x)
    if window == None:
        if scale & gr.OPTION_X_LOG == 0:
            xmin, xmax = gr.adjustrange(min(x), max(x))
        else:
            xmin, xmax = (min(x), max(x))
        if scale & gr.OPTION_Y_LOG == 0:
            ymin, ymax = gr.adjustrange(min(y), max(y))
        else:
            ymin, ymax = (min(y), max(y))
    else:
        xmin, xmax, ymin, ymax = window
    if scale & gr.OPTION_X_LOG == 0:
        majorx = 5
        xtick = gr.tick(xmin, xmax) / majorx
    else:
        xtick = majorx = 1
    if scale & gr.OPTION_Y_LOG == 0:
        majory = 5
        ytick = gr.tick(ymin, ymax) /majory 
    else:
        ytick = majory = 1
    gr.setviewport(viewport[0], viewport[1], viewport[2], viewport[3])
    gr.setwindow(xmin, xmax, ymin, ymax)
    gr.setscale(scale)
    if bgcolor != 0:
        gr.setfillintstyle(1)
        gr.setfillcolorind(bgcolor)
        gr.fillrect(xmin, xmax, ymin, ymax)
    charheight = 0.024 * (viewport[3] - viewport[2])
    gr.setcharheight(charheight)
    gr.axes(xtick, ytick, xmin, ymin,  majorx,  majory,  0.01)
    gr.axes(xtick, ytick, xmax, ymax, -majorx, -majory, -0.01)
    if grid:
         gr.grid(xtick, ytick, xmax, ymax, majorx, majory)
    gr.setlinetype(linetype)
    gr.polyline(n, x, y)
    if markertype != gr.MARKERTYPE_DOT:
        gr.setmarkertype(markertype)
        gr.polymarker(n, x, y)
    if update:
        gr.updatews()

def plot3d(z,
           viewport = (0.1, 0.9, 0.1, 0.9),
           rotation = 30,
           tilt = 70,
           colormap = 1,
           option = 4,
           contours = True,
           xtitle = '',
           ytitle = '',
           ztitle = ''):
    gr.clearws()
    xmin, ymin = (1, 1)
    xmax, ymax = _guessdimension(len(z))[0]
    xtick = gr.tick(xmin, xmax) / 5
    ytick = gr.tick(ymin, ymax) / 5
    x = range(1, xmax + 1)
    y = range(1, ymax + 1)
    zmin = min(z)
    zmax = max(z)
    zmin, zmax = gr.adjustrange(zmin, zmax)
    ztick = gr.tick(zmin, zmax) / 5
    gr.setviewport(viewport[0], viewport[1], viewport[2], viewport[3])
    gr.setwindow(xmin, xmax, ymin, ymax)
    gr.setspace(zmin, zmax, rotation, tilt)
    charheight = 0.024 * (viewport[3] - viewport[2])
    gr.setcharheight(charheight)
    if rotation != 0 or tilt != 90:
        gr.axes3d(xtick, 0, ztick, xmin, ymin, zmin, 5, 0, 5, -0.01)
        gr.axes3d(0, ytick,  0, xmax, ymin, zmin, 0, 5, 0,  0.01)
    gr.setcolormap(colormap)
    gr.surface(xmax, ymax, x, y, z, option)
    if contours:
        gr.contour(xmax, ymax, 0, x, y, range(1), z, 0)
    if rotation == 0 and tilt == 90:
        gr.axes(xtick, ytick, xmin, ymin, 5, 5, -0.01)
    if xtitle != '' or ytitle != '' or ztitle != '':
        gr.titles3d(xtitle, ytitle, ztitle)
    gr.updatews()

def delay(seconds):
    time.sleep(seconds)

