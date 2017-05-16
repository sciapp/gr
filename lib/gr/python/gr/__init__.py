# -*- coding: utf-8 -*-
"""
This is a procedural interface to the GR plotting library,
which may be imported directly, e.g.:

import gr
"""

import os
from numpy import array, ndarray, float64, int32
from ctypes import c_int, c_double, c_char_p, c_void_p, c_uint8
from ctypes import byref, POINTER, addressof, CDLL, CFUNCTYPE
from ctypes import create_string_buffer, cast
from sys import version_info, platform
from platform import python_implementation
# local library
from gr._version import __version__, __revision__

# Detect whether this is a site-package installation
if os.path.isdir(os.path.join(os.path.dirname(__file__), "fonts")):
    # Set GRDIR environment accordingly to site-package installation.
    # (needed for finding GKSTerm on OSX)
    os.environ["GRDIR"] = os.getenv("GRDIR",
                                    os.path.realpath(os.path.dirname(__file__)))
    os.environ["GKS_FONTPATH"] = os.getenv("GKS_FONTPATH", os.environ["GRDIR"])

_impl = python_implementation()
_mime_type = None

try:
    from IPython.display import clear_output, display, SVG, Image, HTML
    from base64 import b64encode
except ImportError:
    clear_output = None


class floatarray:
    def __init__(self, n, a):
        if isinstance(a, ndarray):
            if _impl == 'PyPy':
                self.array = array(a, float64)
                self.data = cast(self.array.__array_interface__['data'][0],
                                 POINTER(c_double))
            else:
                self.array = array(a, c_double)
                self.data = self.array.ctypes.data_as(POINTER(c_double))
        else:
            self.data = (c_double * n)()
            status = 0
            for i in range(n):
                try:
                    self.data[i] = a[i]
                except:
                    if not status:
                        status = 1
                        print('Float array lookup failure')
                    self.data[i] = 0


class intarray:
    def __init__(self, n, a):
        if isinstance(a, ndarray):
            if _impl == 'PyPy':
                self.array = array(a, int32)
                self.data = cast(self.array.__array_interface__['data'][0],
                                 POINTER(c_int))
            else:
                self.array = array(a, c_int)
                self.data = self.array.ctypes.data_as(POINTER(c_int))
        else:
            self.data = (c_int * n)()
            status = 0
            for i in range(n):
                try:
                    self.data[i] = a[i]
                except:
                    if not status:
                        status = 1
                        print('Integer array lookup failure')
                    self.data[i] = 0


class uint8array:
    def __init__(self, a):
        if _impl == 'PyPy':
            self.array = array(a, uint8)
            self.data = cast(self.array.__array_interface__['data'][0],
                             POINTER(c_uint8))
        else:
            self.array = array(a, c_uint8)
            self.data = self.array.ctypes.data_as(POINTER(c_uint8))


class nothing:
    def __init__(self):
        self.array = None
        self.data = None


def char(string):
    if version_info[0] == 2 and not isinstance(string, unicode):
        try:
            string = unicode(string, 'utf-8')
        except:
            string = unicode(string, 'latin-1')
    string = string.replace(u'\u2212', '-')
    chars = string.encode('latin-1', 'replace')
    s = create_string_buffer(chars)
    return cast(s, c_char_p)


def opengks():
    __gr.gr_opengks()


def closegks():
    __gr.gr_closegks()


def inqdspsize():
    mwidth = c_double()
    mheight = c_double()
    width = c_int()
    height = c_int()
    __gr.gr_inqdspsize(byref(mwidth), byref(mheight), byref(width), byref(height))
    return [mwidth.value, mheight.value, width.value, height.value]


def openws(workstation_id, connection, workstation_type):
    """
    Open a graphical workstation.

    **Parameters:**

    `workstation_id` :
        A workstation identifier.
    `connection` :
        A connection identifier.
    `workstation_type` :
        The desired workstation type.

    Available workstation types:

    +-------------+------------------------------------------------------+
    |            5|Workstation Independent Segment Storage               |
    +-------------+------------------------------------------------------+
    |         7, 8|Computer Graphics Metafile (CGM binary, clear text)   |
    +-------------+------------------------------------------------------+
    |           41|Windows GDI                                           |
    +-------------+------------------------------------------------------+
    |           51|Mac Quickdraw                                         |
    +-------------+------------------------------------------------------+
    |      61 - 64|PostScript (b/w, color)                               |
    +-------------+------------------------------------------------------+
    |     101, 102|Portable Document Format (plain, compressed)          |
    +-------------+------------------------------------------------------+
    |    210 - 213|X Windows                                             |
    +-------------+------------------------------------------------------+
    |          214|Sun Raster file (RF)                                  |
    +-------------+------------------------------------------------------+
    |     215, 218|Graphics Interchange Format (GIF87, GIF89)            |
    +-------------+------------------------------------------------------+
    |          216|Motif User Interface Language (UIL)                   |
    +-------------+------------------------------------------------------+
    |          320|Windows Bitmap (BMP)                                  |
    +-------------+------------------------------------------------------+
    |          321|JPEG image file                                       |
    +-------------+------------------------------------------------------+
    |          322|Portable Network Graphics file (PNG)                  |
    +-------------+------------------------------------------------------+
    |          323|Tagged Image File Format (TIFF)                       |
    +-------------+------------------------------------------------------+
    |          370|Xfig vector graphics file                             |
    +-------------+------------------------------------------------------+
    |          371|Gtk                                                   |
    +-------------+------------------------------------------------------+
    |          380|wxWidgets                                             |
    +-------------+------------------------------------------------------+
    |          381|Qt4                                                   |
    +-------------+------------------------------------------------------+
    |          382|Scaleable Vector Graphics (SVG)                       |
    +-------------+------------------------------------------------------+
    |          390|Windows Metafile                                      |
    +-------------+------------------------------------------------------+
    |          400|Quartz                                                |
    +-------------+------------------------------------------------------+
    |          410|Socket driver                                         |
    +-------------+------------------------------------------------------+
    |          415|0MQ driver                                            |
    +-------------+------------------------------------------------------+
    |          420|OpenGL                                                |
    +-------------+------------------------------------------------------+
    |          430|HTML5 Canvas                                          |
    +-------------+------------------------------------------------------+

    """
    __gr.gr_openws(c_int(workstation_id), char(connection), c_int(workstation_type))


def closews(workstation_id):
    """
    Close the specified workstation.

    **Parameters:**

    `workstation_id` :
        A workstation identifier.

    """
    __gr.gr_closews(c_int(workstation_id))


def activatews(workstation_id):
    """
    Activate the specified workstation.

    **Parameters:**

    `workstation_id` :
        A workstation identifier.

    """
    __gr.gr_activatews(c_int(workstation_id))


def deactivatews(workstation_id):
    """
    Deactivate the specified workstation.

    **Parameters:**

    `workstation_id` :
        A workstation identifier.

    """
    __gr.gr_deactivatews(c_int(workstation_id))


def clearws():
    if isinline() and clear_output:
        clear_output(wait=True)
    __gr.gr_clearws()


def updatews():
    __gr.gr_updatews()


def _assertEqualLength(*args):
    if args and all(len(args[0]) == len(arg) for arg in args):
        return len(args[0])
    else:
        raise AttributeError("Sequences must have same length.")


def polyline(x, y):
    """
    Draw a polyline using the current line attributes, starting from the
    first data point and ending at the last data point.

    **Parameters:**

    `x` :
        A list containing the X coordinates
    `y` :
        A list containing the Y coordinates

    The values for `x` and `y` are in world coordinates. The attributes that
    control the appearance of a polyline are linetype, linewidth and color
    index.

    """
    n = _assertEqualLength(x, y)
    _x = floatarray(n, x)
    _y = floatarray(n, y)
    __gr.gr_polyline(c_int(n), _x.data, _y.data)


def polymarker(x, y):
    """
    Draw marker symbols centered at the given data points.

    **Parameters:**

    `x` :
        A list containing the X coordinates
    `y` :
        A list containing the Y coordinates

    The values for `x` and `y` are in world coordinates. The attributes that
    control the appearance of a polymarker are marker type, marker size
    scale factor and color index.

    """
    n = _assertEqualLength(x, y)
    _x = floatarray(n, x)
    _y = floatarray(n, y)
    __gr.gr_polymarker(c_int(n), _x.data, _y.data)


def text(x, y, string):
    """
    Draw a text at position `x`, `y` using the current text attributes.

    **Parameters:**

    `x` :
        The X coordinate of starting position of the text string
    `y` :
        The Y coordinate of starting position of the text string
    `string` :
        The text to be drawn

    The values for `x` and `y` are in normalized device coordinates.
    The attributes that control the appearance of text are text font and precision,
    character expansion factor, character spacing, text color index, character
    height, character up vector, text path and text alignment.

    """
    __gr.gr_text(c_double(x), c_double(y), char(string))


def inqtext(x, y, string):
    tbx = (c_double * 4)()
    tby = (c_double * 4)()
    __gr.gr_inqtext(c_double(x), c_double(y), char(string), tbx, tby)
    return [[tbx[0], tbx[1], tbx[2], tbx[3]],
            [tby[0], tby[1], tby[2], tby[3]]]


def fillarea(x, y):
    """
    Allows you to specify a polygonal shape of an area to be filled.

    **Parameters:**

    `x` :
        A list containing the X coordinates
    `y` :
        A list containing the Y coordinates

    The attributes that control the appearance of fill areas are fill area interior
    style, fill area style index and fill area color index.

    """
    n = _assertEqualLength(x, y)
    _x = floatarray(n, x)
    _y = floatarray(n, y)
    __gr.gr_fillarea(c_int(n), _x.data, _y.data)


def cellarray(xmin, xmax, ymin, ymax, dimx, dimy, color):
    """
    Display rasterlike images in a device-independent manner. The cell array
    function partitions a rectangle given by two corner points into DIMX X DIMY
    cells, each of them colored individually by the corresponding color index
    of the given cell array.

    **Parameters:**

    `xmin`, `ymin` :
        Lower left point of the rectangle
    `xmax`, `ymax` :
        Upper right point of the rectangle
    `dimx`, `dimy` :
        X and Y dimension of the color index array
    `color` :
        Color index array

    The values for `xmin`, `xmax`, `ymin` and `ymax` are in world coordinates.

    """
    _color = intarray(dimx * dimy, color)
    __gr.gr_cellarray(c_double(xmin), c_double(xmax), c_double(ymin), c_double(ymax),
                      c_int(dimx), c_int(dimy), c_int(1), c_int(1),
                      c_int(dimx), c_int(dimy), _color.data)


def spline(px, py, m, method):
    """
    Generate a cubic spline-fit, starting from the first data point and
    ending at the last data point.

    **Parameters:**

    `x` :
        A list containing the X coordinates
    `y` :
        A list containing the Y coordinates
    `m` :
        The number of points in the polygon to be drawn (`m` > len(`x`))
    `method` :
        The smoothing method

    The values for `x` and `y` are in world coordinates. The attributes that
    control the appearance of a spline-fit are linetype, linewidth and color
    index.

    If `method` is > 0, then a generalized cross-validated smoothing spline is calculated.
    If `method` is 0, then an interpolating natural cubic spline is calculated.
    If `method` is < -1, then a cubic B-spline is calculated.

    """
    n = _assertEqualLength(px, py)
    _px = floatarray(n, px)
    _py = floatarray(n, py)
    __gr.gr_spline(c_int(n), _px.data, _py.data, c_int(m), c_int(method))


def gridit(xd, yd, zd, nx, ny):
    nd = _assertEqualLength(xd, yd, zd)
    _xd = floatarray(nd, xd)
    _yd = floatarray(nd, yd)
    _zd = floatarray(nd, zd)
    x = (c_double * nx)()
    y = (c_double * ny)()
    z = (c_double * (nx * ny))()
    __gr.gr_gridit(c_int(nd), _xd.data, _yd.data, _zd.data,
                   c_int(nx), c_int(ny), x, y, z)
    return [x[:], y[:], z[:]]


def setlinetype(style):
    """
    Specify the line style for polylines.

    **Parameters:**

    `style` :
        The polyline line style

    The available line types are:

    +---------------------------+----+---------------------------------------------------+
    |LINETYPE_SOLID             |   1|Solid line                                         |
    +---------------------------+----+---------------------------------------------------+
    |LINETYPE_DASHED            |   2|Dashed line                                        |
    +---------------------------+----+---------------------------------------------------+
    |LINETYPE_DOTTED            |   3|Dotted line                                        |
    +---------------------------+----+---------------------------------------------------+
    |LINETYPE_DASHED_DOTTED     |   4|Dashed-dotted line                                 |
    +---------------------------+----+---------------------------------------------------+
    |LINETYPE_DASH_2_DOT        |  -1|Sequence of one dash followed by two dots          |
    +---------------------------+----+---------------------------------------------------+
    |LINETYPE_DASH_3_DOT        |  -2|Sequence of one dash followed by three dots        |
    +---------------------------+----+---------------------------------------------------+
    |LINETYPE_LONG_DASH         |  -3|Sequence of long dashes                            |
    +---------------------------+----+---------------------------------------------------+
    |LINETYPE_LONG_SHORT_DASH   |  -4|Sequence of a long dash followed by a short dash   |
    +---------------------------+----+---------------------------------------------------+
    |LINETYPE_SPACED_DASH       |  -5|Sequence of dashes double spaced                   |
    +---------------------------+----+---------------------------------------------------+
    |LINETYPE_SPACED_DOT        |  -6|Sequence of dots double spaced                     |
    +---------------------------+----+---------------------------------------------------+
    |LINETYPE_DOUBLE_DOT        |  -7|Sequence of pairs of dots                          |
    +---------------------------+----+---------------------------------------------------+
    |LINETYPE_TRIPLE_DOT        |  -8|Sequence of groups of three dots                   |
    +---------------------------+----+---------------------------------------------------+

    """
    __gr.gr_setlinetype(c_int(style))


def inqlinetype():
    ltype = c_int()
    __gr.gr_inqlinetype(byref(ltype))
    return ltype.value


def setlinewidth(width):
    """
    Define the line width of subsequent polyline output primitives.

    **Parameters:**

    `width` :
        The polyline line width scale factor

    The line width is calculated as the nominal line width generated
    on the workstation multiplied by the line width scale factor.
    This value is mapped by the workstation to the nearest available line width.
    The default line width is 1.0, or 1 times the line width generated on the graphics device.

    """
    __gr.gr_setlinewidth(c_double(width))


def inqlinewidth():
    width = c_double()
    __gr.gr_inqlinewidth(byref(width))
    return width.value


def setlinecolorind(color):
    """
    Define the color of subsequent polyline output primitives.

    **Parameters:**

    `color` :
        The polyline color index (COLOR < 1256)

    """
    __gr.gr_setlinecolorind(c_int(color))


def inqlinecolorind():
    coli = c_int()
    __gr.gr_inqlinecolorind(byref(coli))
    return coli.value


def setmarkertype(style):
    """
    Specifiy the marker type for polymarkers.

    **Parameters:**

    `style` :
        The polymarker marker type

    The available marker types are:

    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_DOT               |    1|Smallest displayable dot                        |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_PLUS              |    2|Plus sign                                       |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_ASTERISK          |    3|Asterisk                                        |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_CIRCLE            |    4|Hollow circle                                   |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_DIAGONAL_CROSS    |    5|Diagonal cross                                  |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_SOLID_CIRCLE      |   -1|Filled circle                                   |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_TRIANGLE_UP       |   -2|Hollow triangle pointing upward                 |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_SOLID_TRI_UP      |   -3|Filled triangle pointing upward                 |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_TRIANGLE_DOWN     |   -4|Hollow triangle pointing downward               |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_SOLID_TRI_DOWN    |   -5|Filled triangle pointing downward               |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_SQUARE            |   -6|Hollow square                                   |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_SOLID_SQUARE      |   -7|Filled square                                   |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_BOWTIE            |   -8|Hollow bowtie                                   |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_SOLID_BOWTIE      |   -9|Filled bowtie                                   |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_HGLASS            |  -10|Hollow hourglass                                |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_SOLID_HGLASS      |  -11|Filled hourglass                                |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_DIAMOND           |  -12|Hollow diamond                                  |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_SOLID_DIAMOND     |  -13|Filled Diamond                                  |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_STAR              |  -14|Hollow star                                     |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_SOLID_STAR        |  -15|Filled Star                                     |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_TRI_UP_DOWN       |  -16|Hollow triangles pointing up and down overlaid  |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_SOLID_TRI_RIGHT   |  -17|Filled triangle point right                     |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_SOLID_TRI_LEFT    |  -18|Filled triangle pointing left                   |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_HOLLOW PLUS       |  -19|Hollow plus sign                                |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_SOLID PLUS        |  -20|Solid plus sign                                 |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_PENTAGON          |  -21|Pentagon                                        |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_HEXAGON           |  -22|Hexagon                                         |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_HEPTAGON          |  -23|Heptagon                                        |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_OCTAGON           |  -24|Octagon                                         |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_STAR_4            |  -25|4-pointed star                                  |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_STAR_5            |  -26|5-pointed star (pentagram)                      |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_STAR_6            |  -27|6-pointed star (hexagram)                       |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_STAR_7            |  -28|7-pointed star (heptagram)                      |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_STAR_8            |  -29|8-pointed star (octagram)                       |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_VLINE             |  -30|verical line                                    |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_HLINE             |  -31|horizontal line                                 |
    +-----------------------------+-----+------------------------------------------------+
    |MARKERTYPE_OMARK             |  -32|o-mark                                          |
    +-----------------------------+-----+------------------------------------------------+

    Polymarkers appear centered over their specified coordinates.

    """
    __gr.gr_setmarkertype(c_int(style))


def inqmarkertype():
    mtype = c_int()
    __gr.gr_inqmarkertype(byref(mtype))
    return mtype.value


def setmarkersize(size):
    """
    Specify the marker size for polymarkers.

    **Parameters:**

    `size` :
        Scale factor applied to the nominal marker size

    The polymarker size is calculated as the nominal size generated on the graphics device
    multiplied by the marker size scale factor.

    """
    __gr.gr_setmarkersize(c_double(size))


def setmarkercolorind(color):
    """
    Define the color of subsequent polymarker output primitives.

    **Parameters:**

    `color` :
        The polymarker color index (COLOR < 1256)

    """
    __gr.gr_setmarkercolorind(c_int(color))


def inqmarkercolorind():
    coli = c_int()
    __gr.gr_inqmarkercolorind(byref(coli))
    return coli.value


def settextfontprec(font, precision):
    """
    Specify the text font and precision for subsequent text output primitives.

    **Parameters:**

    `font` :
        Text font (see tables below)
    `precision` :
        Text precision (see table below)

    The available text fonts are:

    +--------------------------------------+-----+
    |FONT_TIMES_ROMAN                      |  101|
    +--------------------------------------+-----+
    |FONT_TIMES_ITALIC                     |  102|
    +--------------------------------------+-----+
    |FONT_TIMES_BOLD                       |  103|
    +--------------------------------------+-----+
    |FONT_TIMES_BOLDITALIC                 |  104|
    +--------------------------------------+-----+
    |FONT_HELVETICA                        |  105|
    +--------------------------------------+-----+
    |FONT_HELVETICA_OBLIQUE                |  106|
    +--------------------------------------+-----+
    |FONT_HELVETICA_BOLD                   |  107|
    +--------------------------------------+-----+
    |FONT_HELVETICA_BOLDOBLIQUE            |  108|
    +--------------------------------------+-----+
    |FONT_COURIER                          |  109|
    +--------------------------------------+-----+
    |FONT_COURIER_OBLIQUE                  |  110|
    +--------------------------------------+-----+
    |FONT_COURIER_BOLD                     |  111|
    +--------------------------------------+-----+
    |FONT_COURIER_BOLDOBLIQUE              |  112|
    +--------------------------------------+-----+
    |FONT_SYMBOL                           |  113|
    +--------------------------------------+-----+
    |FONT_BOOKMAN_LIGHT                    |  114|
    +--------------------------------------+-----+
    |FONT_BOOKMAN_LIGHTITALIC              |  115|
    +--------------------------------------+-----+
    |FONT_BOOKMAN_DEMI                     |  116|
    +--------------------------------------+-----+
    |FONT_BOOKMAN_DEMIITALIC               |  117|
    +--------------------------------------+-----+
    |FONT_NEWCENTURYSCHLBK_ROMAN           |  118|
    +--------------------------------------+-----+
    |FONT_NEWCENTURYSCHLBK_ITALIC          |  119|
    +--------------------------------------+-----+
    |FONT_NEWCENTURYSCHLBK_BOLD            |  120|
    +--------------------------------------+-----+
    |FONT_NEWCENTURYSCHLBK_BOLDITALIC      |  121|
    +--------------------------------------+-----+
    |FONT_AVANTGARDE_BOOK                  |  122|
    +--------------------------------------+-----+
    |FONT_AVANTGARDE_BOOKOBLIQUE           |  123|
    +--------------------------------------+-----+
    |FONT_AVANTGARDE_DEMI                  |  124|
    +--------------------------------------+-----+
    |FONT_AVANTGARDE_DEMIOBLIQUE           |  125|
    +--------------------------------------+-----+
    |FONT_PALATINO_ROMAN                   |  126|
    +--------------------------------------+-----+
    |FONT_PALATINO_ITALIC                  |  127|
    +--------------------------------------+-----+
    |FONT_PALATINO_BOLD                    |  128|
    +--------------------------------------+-----+
    |FONT_PALATINO_BOLDITALIC              |  129|
    +--------------------------------------+-----+
    |FONT_ZAPFCHANCERY_MEDIUMITALIC        |  130|
    +--------------------------------------+-----+
    |FONT_ZAPFDINGBATS                     |  131|
    +--------------------------------------+-----+

    The available text precisions are:

    +---------------------------+---+--------------------------------------+
    |TEXT_PRECISION_STRING      |  0|String precision (higher quality)     |
    +---------------------------+---+--------------------------------------+
    |TEXT_PRECISION_CHAR        |  1|Character precision (medium quality)  |
    +---------------------------+---+--------------------------------------+
    |TEXT_PRECISION_STROKE      |  2|Stroke precision (lower quality)      |
    +---------------------------+---+--------------------------------------+

    The appearance of a font depends on the text precision value specified.
    STRING, CHARACTER or STROKE precision allows for a greater or lesser
    realization of the text primitives, for efficiency. STRING is the default
    precision for GR and produces the highest quality output.

    """
    __gr.gr_settextfontprec(c_int(font), c_int(precision))


def setcharexpan(factor):
    """
    Set the current character expansion factor (width to height ratio).

    **Parameters:**

    `factor` :
        Text expansion factor applied to the nominal text width-to-height ratio

    `setcharexpan` defines the width of subsequent text output primitives. The expansion
    factor alters the width of the generated characters, but not their height. The default
    text expansion factor is 1, or one times the normal width-to-height ratio of the text.

    """
    __gr.gr_setcharexpan(c_double(factor))


def setcharspace(spacing):
    __gr.gr_setcharspace(c_double(spacing))


def settextcolorind(color):
    """
    Sets the current text color index.

    **Parameters:**

    `color` :
        The text color index (COLOR < 1256)

    `settextcolorind` defines the color of subsequent text output primitives.
    GR uses the default foreground color (black=1) for the default text color index.

    """
    __gr.gr_settextcolorind(c_int(color))


def setcharheight(height):
    """
    Set the current character height.

    **Parameters:**

    `height` :
        Text height value

    `setcharheight` defines the height of subsequent text output primitives. Text height
    is defined as a percentage of the default window. GR uses the default text height of
    0.027 (2.7% of the height of the default window).

    """
    __gr.gr_setcharheight(c_double(height))


def setcharup(ux, uy):
    """
    Set the current character text angle up vector.

    **Parameters:**

    `ux`, `uy` :
        Text up vector

    `setcharup` defines the vertical rotation of subsequent text output primitives.
    The text up vector is initially set to (0, 1), horizontal to the baseline.

    """
    __gr.gr_setcharup(c_double(ux), c_double(uy))


def settextpath(path):
    """
    Define the current direction in which subsequent text will be drawn.

    **Parameters:**

    `path` :
        Text path (see table below)

    +----------------------+---+---------------+
    |TEXT_PATH_RIGHT       |  0|left-to-right  |
    +----------------------+---+---------------+
    |TEXT_PATH_LEFT        |  1|right-to-left  |
    +----------------------+---+---------------+
    |TEXT_PATH_UP          |  2|downside-up    |
    +----------------------+---+---------------+
    |TEXT_PATH_DOWN        |  3|upside-down    |
    +----------------------+---+---------------+

    """
    __gr.gr_settextpath(c_int(path))


def settextalign(horizontal, vertical):
    """
    Set the current horizontal and vertical alignment for text.

    **Parameters:**

    `horizontal` :
        Horizontal text alignment (see the table below)
    `vertical` :
        Vertical text alignment (see the table below)

    `settextalign` specifies how the characters in a text primitive will be aligned
    in horizontal and vertical space. The default text alignment indicates horizontal left
    alignment and vertical baseline alignment.

    +-------------------------+---+----------------+
    |TEXT_HALIGN_NORMAL       |  0|                |
    +-------------------------+---+----------------+
    |TEXT_HALIGN_LEFT         |  1|Left justify    |
    +-------------------------+---+----------------+
    |TEXT_HALIGN_CENTER       |  2|Center justify  |
    +-------------------------+---+----------------+
    |TEXT_HALIGN_RIGHT        |  3|Right justify   |
    +-------------------------+---+----------------+

    +-------------------------+---+------------------------------------------------+
    |TEXT_VALIGN_NORMAL       |  0|                                                |
    +-------------------------+---+------------------------------------------------+
    |TEXT_VALIGN_TOP          |  1|Align with the top of the characters            |
    +-------------------------+---+------------------------------------------------+
    |TEXT_VALIGN_CAP          |  2|Aligned with the cap of the characters          |
    +-------------------------+---+------------------------------------------------+
    |TEXT_VALIGN_HALF         |  3|Aligned with the half line of the characters    |
    +-------------------------+---+------------------------------------------------+
    |TEXT_VALIGN_BASE         |  4|Aligned with the base line of the characters    |
    +-------------------------+---+------------------------------------------------+
    |TEXT_VALIGN_BOTTOM       |  5|Aligned with the bottom line of the characters  |
    +-------------------------+---+------------------------------------------------+

    """
    __gr.gr_settextalign(c_int(horizontal), c_int(vertical))


def setfillintstyle(style):
    """
    Set the fill area interior style to be used for fill areas.

    **Parameters:**

    `style` :
        The style of fill to be used

    `setfillintstyle` defines the interior style  for subsequent fill area output
    primitives. The default interior style is HOLLOW.

    +---------+---+--------------------------------------------------------------------------------+
    |HOLLOW   |  0|No filling. Just draw the bounding polyline                                     |
    +---------+---+--------------------------------------------------------------------------------+
    |SOLID    |  1|Fill the interior of the polygon using the fill color index                     |
    +---------+---+--------------------------------------------------------------------------------+
    |PATTERN  |  2|Fill the interior of the polygon using the style index as a pattern index       |
    +---------+---+--------------------------------------------------------------------------------+
    |HATCH    |  3|Fill the interior of the polygon using the style index as a cross-hatched style |
    +---------+---+--------------------------------------------------------------------------------+

    """
    __gr.gr_setfillintstyle(c_int(style))


def setfillstyle(index):
    """
    Sets the fill style to be used for subsequent fill areas.

    **Parameters:**

    `index` :
        The fill style index to be used

    `setfillstyle` specifies an index when PATTERN fill or HATCH fill is requested by the
    `setfillintstyle` function. If the interior style is set to PATTERN, the fill style
    index points to a device-independent pattern table. If interior style is set to HATCH
    the fill style index indicates different hatch styles. If HOLLOW or SOLID is specified
    for the interior style, the fill style index is unused.

    """
    __gr.gr_setfillstyle(c_int(index))


def setfillcolorind(color):
    """
    Sets the current fill area color index.

    **Parameters:**

    `color` :
        The fill area color index (COLOR < 1256)

    `setfillcolorind` defines the color of subsequent fill area output primitives.
    GR uses the default foreground color (black=1) for the default fill area color index.

    """
    __gr.gr_setfillcolorind(c_int(color))


def setcolorrep(index, red, green, blue):
    """
    `setcolorrep` allows to redefine an existing color index representation by specifying
    an RGB color triplet.

    **Parameters:**

    `index` :
        Color index in the range 0 to 1256
    `red` :
        Red intensity in the range 0.0 to 1.0
    `green` :
        Green intensity in the range 0.0 to 1.0
    `blue`:
        Blue intensity in the range 0.0 to 1.0

    """
    __gr.gr_setcolorrep(c_int(index), c_double(red), c_double(green), c_double(blue))


def setscale(options):
    """
    `setscale` sets the type of transformation to be used for subsequent GR output
    primitives.

    **Parameters:**

    `options` :
        Scale specification (see Table below)

    +---------------+--------------------+
    |OPTION_X_LOG   |Logarithmic X-axis  |
    +---------------+--------------------+
    |OPTION_Y_LOG   |Logarithmic Y-axis  |
    +---------------+--------------------+
    |OPTION_Z_LOG   |Logarithmic Z-axis  |
    +---------------+--------------------+
    |OPTION_FLIP_X  |Flip X-axis         |
    +---------------+--------------------+
    |OPTION_FLIP_Y  |Flip Y-axis         |
    +---------------+--------------------+
    |OPTION_FLIP_Z  |Flip Z-axis         |
    +---------------+--------------------+

    `setscale` defines the current transformation according to the given scale
    specification which may be or'ed together using any of the above options. GR uses
    these options for all subsequent output primitives until another value is provided.
    The scale options are used to transform points from an abstract logarithmic or
    semi-logarithmic coordinate system, which may be flipped along each axis, into the
    world coordinate system.

    Note: When applying a logarithmic transformation to a specific axis, the system
    assumes that the axes limits are greater than zero.

    """
    return __gr.gr_setscale(c_int(options))


def inqscale():
    options = c_int()
    __gr.gr_inqscale(byref(options))
    return options.value


def setwindow(xmin, xmax, ymin, ymax):
    """
    `setwindow` establishes a window, or rectangular subspace, of world coordinates to be
    plotted. If you desire log scaling or mirror-imaging of axes, use the SETSCALE function.

    **Parameters:**

    `xmin` :
        The left horizontal coordinate of the window (`xmin` < `xmax`).
    `xmax` :
        The right horizontal coordinate of the window.
    `ymin` :
        The bottom vertical coordinate of the window (`ymin` < `ymax`).
    `ymax` :
        The top vertical coordinate of the window.

    `setwindow` defines the rectangular portion of the World Coordinate space (WC) to be
    associated with the specified normalization transformation. The WC window and the
    Normalized Device Coordinates (NDC) viewport define the normalization transformation
    through which all output primitives are mapped. The WC window is mapped onto the
    rectangular NDC viewport which is, in turn, mapped onto the display surface of the
    open and active workstation, in device coordinates. By default, GR uses the range
    [0,1] x [0,1], in world coordinates, as the normalization transformation window.

    """
    __gr.gr_setwindow(c_double(xmin), c_double(xmax), c_double(ymin), c_double(ymax))


def inqwindow():
    xmin = c_double()
    xmax = c_double()
    ymin = c_double()
    ymax = c_double()
    __gr.gr_inqwindow(byref(xmin), byref(xmax), byref(ymin), byref(ymax))
    return [xmin.value, xmax.value, ymin.value, ymax.value]


def setviewport(xmin, xmax, ymin, ymax):
    """
    `setviewport` establishes a rectangular subspace of normalized device coordinates.

    **Parameters:**

    `xmin` :
        The left horizontal coordinate of the viewport.
    `xmax` :
        The right horizontal coordinate of the viewport (0 <= `xmin` < `xmax` <= 1).
    `ymin` :
        The bottom vertical coordinate of the viewport.
    `ymax` :
        The top vertical coordinate of the viewport (0 <= `ymin` < `ymax` <= 1).

    `setviewport` defines the rectangular portion of the Normalized Device Coordinate
    (NDC) space to be associated with the specified normalization transformation. The
    NDC viewport and World Coordinate (WC) window define the normalization transformation
    through which all output primitives pass. The WC window is mapped onto the rectangular
    NDC viewport which is, in turn, mapped onto the display surface of the open and active
    workstation, in device coordinates.

    """
    __gr.gr_setviewport(c_double(xmin), c_double(xmax), c_double(ymin), c_double(ymax))


def inqviewport():
    xmin = c_double()
    xmax = c_double()
    ymin = c_double()
    ymax = c_double()
    __gr.gr_inqviewport(byref(xmin), byref(xmax), byref(ymin), byref(ymax))
    return [xmin.value, xmax.value, ymin.value, ymax.value]


def selntran(transform):
    """
    `selntran` selects a predefined transformation from world coordinates to normalized
    device coordinates.

    **Parameters:**

    `transform` :
        A normalization transformation number.

    +------+----------------------------------------------------------------------------------------------------+
    |     0|Selects the identity transformation in which both the window and viewport have the range of 0 to 1  |
    +------+----------------------------------------------------------------------------------------------------+
    |  >= 1|Selects a normalization transformation as defined by `setwindow` and `setviewport`                  |
    +------+----------------------------------------------------------------------------------------------------+

    """
    __gr.gr_selntran(c_int(transform))


def setclip(indicator):
    """
    Set the clipping indicator.

    **Parameters:**

    `indicator` :
        An indicator specifying whether clipping is on or off.

    +----+---------------------------------------------------------------+
    |   0|Clipping is off. Data outside of the window will be drawn.     |
    +----+---------------------------------------------------------------+
    |   1|Clipping is on. Data outside of the window will not be drawn.  |
    +----+---------------------------------------------------------------+

    `setclip` enables or disables clipping of the image drawn in the current window.
    Clipping is defined as the removal of those portions of the graph that lie outside of
    the defined viewport. If clipping is on, GR does not draw generated output primitives
    past the viewport boundaries. If clipping is off, primitives may exceed the viewport
    boundaries, and they will be drawn to the edge of the workstation window.
    By default, clipping is on.

    """
    __gr.gr_setclip(c_int(indicator))


def setwswindow(xmin, xmax, ymin, ymax):
    """
    Set the area of the NDC viewport that is to be drawn in the workstation window.

    **Parameters:**

    `xmin` :
        The left horizontal coordinate of the workstation window.
    `xmax` :
        The right horizontal coordinate of the workstation window (0 <= `xmin` < `xmax` <= 1).
    `ymin` :
        The bottom vertical coordinate of the workstation window.
    `ymax` :
        The top vertical coordinate of the workstation window (0 <= `ymin` < `ymax` <= 1).

    `setwswindow` defines the rectangular area of the Normalized Device Coordinate space
    to be output to the device. By default, the workstation transformation will map the
    range [0,1] x [0,1] in NDC onto the largest square on the workstation’s display
    surface. The aspect ratio of the workstation window is maintained at 1 to 1.

    """
    __gr.gr_setwswindow(c_double(xmin), c_double(xmax), c_double(ymin), c_double(ymax))


def setwsviewport(xmin, xmax, ymin, ymax):
    """
    Define the size of the workstation graphics window in meters.

    **Parameters:**

    `xmin` :
        The left horizontal coordinate of the workstation viewport.
    `xmax` :
        The right horizontal coordinate of the workstation viewport.
    `ymin` :
        The bottom vertical coordinate of the workstation viewport.
    `ymax` :
        The top vertical coordinate of the workstation viewport.

    `setwsviewport` places a workstation window on the display of the specified size in
    meters. This command allows the workstation window to be accurately sized for a
    display or hardcopy device, and is often useful for sizing graphs for desktop
    publishing applications.

    """
    __gr.gr_setwsviewport(c_double(xmin), c_double(xmax), c_double(ymin), c_double(ymax))


def createseg(segment):
    __gr.gr_createseg(c_int(segment))


def copysegws(segment):
    __gr.gr_copysegws(c_int(segment))


def redrawsegws():
    __gr.gr_redrawsegws()


def setsegtran(segment, fx, fy, transx, transy, phi, scalex, scaley):
    __gr.gr_setsegtran(c_int(segment), c_double(fx), c_double(fy),
                       c_double(transx), c_double(transy), c_double(phi),
                       c_double(scalex), c_double(scaley))


def closeseg():
    __gr.gr_closegks()


def emergencyclosegks():
    __gr.gr_emergencyclosegks()


def updategks():
    __gr.gr_updategks()


def setspace(zmin, zmax, rotation, tilt):
    """
    Set the abstract Z-space used for mapping three-dimensional output primitives into
    the current world coordinate space.

    **Parameters:**

    `zmin` :
        Minimum value for the Z-axis.
    `zmax` :
        Maximum value for the Z-axis.
    `rotation` :
        Angle for the rotation of the X axis, in degrees.
    `tilt` :
        Viewing angle of the Z axis in degrees.

    `setspace` establishes the limits of an abstract Z-axis and defines the angles for
    rotation and for the viewing angle (tilt) of a simulated three-dimensional graph,
    used for mapping corresponding output primitives into the current window.
    These settings are used for all subsequent three-dimensional output primitives until
    other values are specified. Angles of rotation and viewing angle must be specified
    between 0° and 90°.

    """
    return __gr.gr_setspace(c_double(zmin), c_double(zmax),
                            c_int(rotation), c_int(tilt))


def inqspace():
    zmin = c_double()
    zmax = c_double()
    rotation = c_int()
    tilt = c_int()
    __gr.gr_inqspace(byref(zmin), byref(zmax), byref(rotation), byref(tilt))
    return [zmin.value, zmax.value, rotation.value, tilt.value]


def textext(x, y, string):
    """
    Draw a text at position `x`, `y` using the current text attributes. Strings can be
    defined to create basic mathematical expressions and Greek letters.

    **Parameters:**

    `x` :
        The X coordinate of starting position of the text string
    `y` :
        The Y coordinate of starting position of the text string
    `string` :
        The text to be drawn

    The values for X and Y are in normalized device coordinates.
    The attributes that control the appearance of text are text font and precision,
    character expansion factor, character spacing, text color index, character
    height, character up vector, text path and text alignment.

    The character string is interpreted to be a simple mathematical formula.
    The following notations apply:

    Subscripts and superscripts: These are indicated by carets ('^') and underscores
    ('_'). If the sub/superscript contains more than one character, it must be enclosed
    in curly braces ('{}').

    Fractions are typeset with A '/' B, where A stands for the numerator and B for the
    denominator.

    To include a Greek letter you must specify the corresponding keyword after a
    backslash ('\') character. The text translator produces uppercase or lowercase
    Greek letters depending on the case of the keyword.

    +--------+---------+
    |Letter  |Keyword  |
    +--------+---------+
    |Α α     |alpha    |
    +--------+---------+
    |Β β     |beta     |
    +--------+---------+
    |Γ γ     |gamma    |
    +--------+---------+
    |Δ δ     |delta    |
    +--------+---------+
    |Ε ε     |epsilon  |
    +--------+---------+
    |Ζ ζ     |zeta     |
    +--------+---------+
    |Η η     |eta      |
    +--------+---------+
    |Θ θ     |theta    |
    +--------+---------+
    |Ι ι     |iota     |
    +--------+---------+
    |Κ κ     |kappa    |
    +--------+---------+
    |Λ λ     |lambda   |
    +--------+---------+
    |Μ μ     |mu       |
    +--------+---------+
    |Ν ν     |nu       |
    +--------+---------+
    |Ξ ξ     |xi       |
    +--------+---------+
    |Ο ο     |omicron  |
    +--------+---------+
    |Π π     |pi       |
    +--------+---------+
    |Ρ ρ     |rho      |
    +--------+---------+
    |Σ σ     |sigma    |
    +--------+---------+
    |Τ τ     |tau      |
    +--------+---------+
    |Υ υ     |upsilon  |
    +--------+---------+
    |Φ φ     |phi      |
    +--------+---------+
    |Χ χ     |chi      |
    +--------+---------+
    |Ψ ψ     |psi      |
    +--------+---------+
    |Ω ω     |omega    |
    +--------+---------+

    For more sophisticated mathematical formulas, you should use the `gr.mathtex`
    function.

    """
    return __gr.gr_textext(c_double(x), c_double(y), char(string))


def inqtextext(x, y, string):
    tbx = (c_double * 4)()
    tby = (c_double * 4)()
    __gr.gr_inqtextext(c_double(x), c_double(y), char(string), tbx, tby)
    return [[tbx[0], tbx[1], tbx[2], tbx[3]],
            [tby[0], tby[1], tby[2], tby[3]]]


_axeslbl_callback = CFUNCTYPE(c_void_p, c_double, c_double, c_char_p, c_double)


def axeslbl(x_tick, y_tick, x_org, y_org, major_x, major_y, tick_size,
            fpx=0, fpy=0):
    if fpx is None:
        fpx = 0
    if fpy is None:
        fpy = 0

    cfpx = _axeslbl_callback(fpx)
    cfpy = _axeslbl_callback(fpy)
    __gr.gr_axeslbl(c_double(x_tick), c_double(y_tick),
                    c_double(x_org), c_double(y_org),
                    c_int(major_x), c_int(major_y), c_double(tick_size),
                    cfpx, cfpy)


def axes(x_tick, y_tick, x_org, y_org, major_x, major_y, tick_size):
    """
    Draw X and Y coordinate axes with linearly and/or logarithmically spaced tick marks.

    **Parameters:**

    `x_tick`, `y_tick` :
        The interval between minor tick marks on each axis.
    `x_org`, `y_org` :
        The world coordinates of the origin (point of intersection) of the X
        and Y axes.
    `major_x`, `major_y` :
        Unitless integer values specifying the number of minor tick intervals
        between major tick marks. Values of 0 or 1 imply no minor ticks.
        Negative values specify no labels will be drawn for the associated axis.
    `tick_size` :
        The length of minor tick marks specified in a normalized device
        coordinate unit. Major tick marks are twice as long as minor tick marks.
        A negative value reverses the tick marks on the axes from inward facing
        to outward facing (or vice versa).

    Tick marks are positioned along each axis so that major tick marks fall on the axes
    origin (whether visible or not). Major tick marks are labeled with the corresponding
    data values. Axes are drawn according to the scale of the window. Axes and tick marks
    are drawn using solid lines; line color and width can be modified using the
    `setlinetype` and `setlinewidth` functions. Axes are drawn according to
    the linear or logarithmic transformation established by the `setscale` function.

    """
    __gr.gr_axes(c_double(x_tick), c_double(y_tick),
                 c_double(x_org), c_double(y_org),
                 c_int(major_x), c_int(major_y), c_double(tick_size))


def grid(x_tick, y_tick, x_org, y_org, major_x, major_y):
    """
    Draw a linear and/or logarithmic grid.

    **Parameters:**

    `x_tick`, `y_tick` :
        The length in world coordinates of the interval between minor grid
        lines.
    `x_org`, `y_org` :
        The world coordinates of the origin (point of intersection) of the grid.
    `major_x`, `major_y` :
        Unitless integer values specifying the number of minor grid lines
        between major grid lines. Values of 0 or 1 imply no grid lines.

    Major grid lines correspond to the axes origin and major tick marks whether visible
    or not. Minor grid lines are drawn at points equal to minor tick marks. Major grid
    lines are drawn using black lines and minor grid lines are drawn using gray lines.

    """
    __gr.gr_grid(c_double(x_tick), c_double(y_tick),
                 c_double(x_org), c_double(y_org),
                 c_int(major_x), c_int(major_y))


def grid3d(x_tick, y_tick, z_tick, x_org, y_org, z_org,
           major_x, major_y, major_z):
    """
    Draw a linear and/or logarithmic grid.

    **Parameters:**

    `x_tick`, `y_tick`, `z_tick` :
        The length in world coordinates of the interval between minor grid
        lines.
    `x_org`, `y_org`, `z_org` :
        The world coordinates of the origin (point of intersection) of the grid.
    `major_x`, `major_y`, `major_z` :
        Unitless integer values specifying the number of minor grid lines
        between major grid lines. Values of 0 or 1 imply no grid lines.

    Major grid lines correspond to the axes origin and major tick marks whether visible
    or not. Minor grid lines are drawn at points equal to minor tick marks. Major grid
    lines are drawn using black lines and minor grid lines are drawn using gray lines.

    """
    __gr.gr_grid3d(c_double(x_tick), c_double(y_tick), c_double(z_tick),
                   c_double(x_org), c_double(y_org), c_double(z_org),
                   c_int(major_x), c_int(major_y), c_int(major_z))


def verrorbars(px, py, e1, e2):
    """
    Draw a standard vertical error bar graph.

    **Parameters:**

    `px` :
        A list of length N containing the X coordinates
    `py` :
        A list of length N containing the Y coordinates
    `e1` :
         The absolute values of the lower error bar data
    `e2` :
         The absolute values of the upper error bar data

    """
    n = _assertEqualLength(px, py, e1, e2)
    _px = floatarray(n, px)
    _py = floatarray(n, py)
    _e1 = floatarray(n, e1)
    _e2 = floatarray(n, e2)
    __gr.gr_verrorbars(c_int(n), _px.data, _py.data, _e1.data, _e2.data)


def herrorbars(px, py, e1, e2):
    """
    Draw a standard horizontal error bar graph.

    **Parameters:**

    `px` :
        A list of length N containing the X coordinates
    `py` :
        A list of length N containing the Y coordinates
    `e1` :
         The absolute values of the lower error bar data
    `e2` :
         The absolute values of the upper error bar data

    """
    n = _assertEqualLength(px, py, e1, e2)
    _px = floatarray(n, px)
    _py = floatarray(n, py)
    _e1 = floatarray(n, e1)
    _e2 = floatarray(n, e2)
    __gr.gr_herrorbars(c_int(n), _px.data, _py.data, _e1.data, _e2.data)


def polyline3d(px, py, pz):
    """
    Draw a 3D curve using the current line attributes, starting from the
    first data point and ending at the last data point.

    **Parameters:**

    `x` :
        A list of length N containing the X coordinates
    `y` :
        A list of length N containing the Y coordinates
    `z` :
        A list of length N containing the Z coordinates

    The values for `x`, `y` and `z` are in world coordinates. The attributes that
    control the appearance of a polyline are linetype, linewidth and color
    index.

    """
    n = _assertEqualLength(px, py, pz)
    _px = floatarray(n, px)
    _py = floatarray(n, py)
    _pz = floatarray(n, pz)
    __gr.gr_polyline3d(c_int(n), _px.data, _py.data, _pz.data)


def polymarker3d(px, py, pz):
    """
    Draw marker symbols centered at the given 3D data points.

    **Parameters:**

    `x` :
        A list of length N containing the X coordinates
    `y` :
        A list of length N containing the Y coordinates
    `z` :
        A list of length N containing the Z coordinates

    The values for `x`, `y` and `z` are in world coordinates. The attributes
    that control the appearance of a polymarker are marker type, marker size
    scale factor and color index.

    """
    n = _assertEqualLength(px, py, pz)
    _px = floatarray(n, px)
    _py = floatarray(n, py)
    _pz = floatarray(n, pz)
    __gr.gr_polymarker3d(c_int(n), _px.data, _py.data, _pz.data)


def axes3d(x_tick, y_tick, z_tick, x_org, y_org, z_org,
           major_x, major_y, major_z, tick_size):
    """
    Draw X, Y and Z coordinate axes with linearly and/or logarithmically
    spaced tick marks.

    **Parameters:**

    `x_tick`, `y_tick`, `z_tick` :
        The interval between minor tick marks on each axis.
    `x_org`, `y_org`, `z_org` :
        The world coordinates of the origin (point of intersection) of the X
        and Y axes.
    `major_x`, `major_y`, `major_z` :
        Unitless integer values specifying the number of minor tick intervals
        between major tick marks. Values of 0 or 1 imply no minor ticks.
        Negative values specify no labels will be drawn for the associated axis.
    `tick_size` :
        The length of minor tick marks specified in a normalized device
        coordinate unit. Major tick marks are twice as long as minor tick marks.
        A negative value reverses the tick marks on the axes from inward facing
        to outward facing (or vice versa).

    Tick marks are positioned along each axis so that major tick marks fall on the axes
    origin (whether visible or not). Major tick marks are labeled with the corresponding
    data values. Axes are drawn according to the scale of the window. Axes and tick marks
    are drawn using solid lines; line color and width can be modified using the
    `setlinetype` and `setlinewidth` functions. Axes are drawn according to
    the linear or logarithmic transformation established by the `setscale` function.

    """
    __gr.gr_axes3d(c_double(x_tick), c_double(y_tick), c_double(z_tick),
                   c_double(x_org), c_double(y_org), c_double(z_org),
                   c_int(major_x), c_int(major_y), c_int(major_z),
                   c_double(tick_size))


def titles3d(x_title, y_title, z_title):
    """
    Display axis titles just outside of their respective axes.

    **Parameters:**

    `x_title`, `y_title`, `z_title` :
        The text to be displayed on each axis

    """
    __gr.gr_titles3d(char(x_title), char(y_title), char(z_title))


def surface(px, py, pz, option):
    """
    Draw a three-dimensional surface plot for the given data points.

    **Parameters:**

    `x` :
        A list containing the X coordinates
    `y` :
        A list containing the Y coordinates
    `z` :
        A list of length `len(x)` * `len(y)` or an appropriately dimensioned
        array containing the Z coordinates
    `option` :
        Surface display option (see table below)

    `x` and `y` define a grid. `z` is a singly dimensioned array containing at least
    `nx` * `ny` data points. Z describes the surface height at each point on the grid.
    Data is ordered as shown in the following table:

    +------------------+--+--------------------------------------------------------------+
    |LINES             | 0|Use X Y polylines to denote the surface                       |
    +------------------+--+--------------------------------------------------------------+
    |MESH              | 1|Use a wire grid to denote the surface                         |
    +------------------+--+--------------------------------------------------------------+
    |FILLED_MESH       | 2|Applies an opaque grid to the surface                         |
    +------------------+--+--------------------------------------------------------------+
    |Z_SHADED_MESH     | 3|Applies Z-value shading to the surface                        |
    +------------------+--+--------------------------------------------------------------+
    |COLORED_MESH      | 4|Applies a colored grid to the surface                         |
    +------------------+--+--------------------------------------------------------------+
    |CELL_ARRAY        | 5|Applies a grid of individually-colored cells to the surface   |
    +------------------+--+--------------------------------------------------------------+
    |SHADED_MESH       | 6|Applies light source shading to the 3-D surface               |
    +------------------+--+--------------------------------------------------------------+

    """
    nx = len(px)
    ny = len(py)
    nz = len(pz)
    if isinstance(pz, ndarray):
        if len(pz.shape) == 1:
            out_of_bounds = nz != nx * ny
        elif len(pz.shape) == 2:
            out_of_bounds = pz.shape[0] != nx or pz.shape[1] != ny
        else:
            out_of_bounds = True
    else:
        out_of_bounds = nz != nx * ny
    if not out_of_bounds:
        _px = floatarray(nx, px)
        _py = floatarray(ny, py)
        _pz = floatarray(nx * ny, pz)
        __gr.gr_surface(c_int(nx), c_int(ny), _px.data, _py.data, _pz.data,
                        c_int(option))
    else:
        raise AttributeError("Sequences have incorrect length or dimension.")


def contour(px, py, h, pz, major_h):
    """
    Draw contours of a three-dimensional data set whose values are specified over a
    rectangular mesh. Contour lines may optionally be labeled.

    **Parameters:**

    `x` :
        A list containing the X coordinates
    `y` :
        A list containing the Y coordinates
    `h` :
        A list containing the Z coordinate for the height values
    `z` :
        A list of length `len(x)` * `len(y)` or an appropriately dimensioned
        array containing the Z coordinates
    `major_h` :
        Directs GR to label contour lines. For example, a value of 3 would label
        every third line. A value of 1 will label every line. A value of 0
        produces no labels. To produce colored contour lines, add an offset
        of 1000 to `major_h`.

    """
    nx = len(px)
    ny = len(py)
    nz = len(pz)
    nh = len(h)
    if isinstance(pz, ndarray):
        if len(pz.shape) == 1:
            out_of_bounds = nz != nx * ny
        elif len(pz.shape) == 2:
            out_of_bounds = pz.shape[0] != nx or pz.shape[1] != ny
        else:
            out_of_bounds = True
    else:
        out_of_bounds = nz != nx * ny
    if not out_of_bounds:
        _px = floatarray(nx, px)
        _py = floatarray(ny, py)
        _h = floatarray(nh, h)
        _pz = floatarray(nz, pz)
        __gr.gr_contour(c_int(nx), c_int(ny), c_int(nh),
                        _px.data, _py.data, _h.data, _pz.data, c_int(major_h))
    else:
        raise AttributeError("Sequences have incorrect length or dimension.")


def hexbin(x, y, nbins):
    n = _assertEqualLength(x, y)
    _x = floatarray(n, x)
    _y = floatarray(n, y)
    return __gr.gr_hexbin(c_int(n), _x.data, _y.data, c_int(nbins))


def setcolormap(index):
    __gr.gr_setcolormap(c_int(index))


def colorbar():
    __gr.gr_colorbar()


def inqcolor(color):
    rgb = c_int()
    __gr.gr_inqcolor(c_int(color), byref(rgb))
    return rgb.value


def inqcolorfromrgb(red, green, blue):
    return __gr.gr_inqcolorfromrgb(c_double(red),
                                   c_double(green),
                                   c_double(green))

def hsvtorgb(h, s, v):
    r = c_double()
    g = c_double()
    b = c_double()
    __gr.gr_hsvtorgb(c_double(h), c_double(s), c_double(v),
                     byref(r), byref(g), byref(b))
    return [r.value, g.value, b.value]

def tick(amin, amax):
    return __gr.gr_tick(c_double(amin), c_double(amax))


def validaterange(amin, amax):
    return __gr.gr_validaterange(c_double(amin), c_double(amax))


def adjustlimits(amin, amax):
    _amin = c_double(amin)
    _amax = c_double(amax)
    __gr.gr_adjustlimits(byref(_amin), byref(_amax))
    return [_amin.value, _amax.value]


def adjustrange(amin, amax):
    _amin = c_double(amin)
    _amax = c_double(amax)
    __gr.gr_adjustrange(byref(_amin), byref(_amax))
    return [_amin.value, _amax.value]


def beginprint(pathname):
    """
    Open and activate a print device.

    **Parameters:**

    `pathname` :
        Filename for the print device.

    `beginprint` opens an additional graphics output device. The device type is obtained
    from the given file extension. The following file types are supported:

    +-------------+---------------------------------------+
    |.ps, .eps    |PostScript                             |
    +-------------+---------------------------------------+
    |.pdf         |Portable Document Format               |
    +-------------+---------------------------------------+
    |.bmp         |Windows Bitmap (BMP)                   |
    +-------------+---------------------------------------+
    |.jpeg, .jpg  |JPEG image file                        |
    +-------------+---------------------------------------+
    |.png         |Portable Network Graphics file (PNG)   |
    +-------------+---------------------------------------+
    |.tiff, .tif  |Tagged Image File Format (TIFF)        |
    +-------------+---------------------------------------+
    |.fig         |Xfig vector graphics file              |
    +-------------+---------------------------------------+
    |.svg         |Scalable Vector Graphics               |
    +-------------+---------------------------------------+
    |.wmf         |Windows Metafile                       |
    +-------------+---------------------------------------+

    """
    __gr.gr_beginprint(char(pathname))


def beginprintext(pathname, mode, fmt, orientation):
    """
    Open and activate a print device with the given layout attributes.

    **Parameters:**

    `pathname` :
        Filename for the print device.
    `mode` :
        Output mode (Color, GrayScale)
    `fmt` :
        Output format (see table below)
    `orientation` :
        Page orientation (Landscape, Portait)

    The available formats are:

    +-----------+---------------+
    |A4         |0.210 x 0.297  |
    +-----------+---------------+
    |B5         |0.176 x 0.250  |
    +-----------+---------------+
    |Letter     |0.216 x 0.279  |
    +-----------+---------------+
    |Legal      |0.216 x 0.356  |
    +-----------+---------------+
    |Executive  |0.191 x 0.254  |
    +-----------+---------------+
    |A0         |0.841 x 1.189  |
    +-----------+---------------+
    |A1         |0.594 x 0.841  |
    +-----------+---------------+
    |A2         |0.420 x 0.594  |
    +-----------+---------------+
    |A3         |0.297 x 0.420  |
    +-----------+---------------+
    |A5         |0.148 x 0.210  |
    +-----------+---------------+
    |A6         |0.105 x 0.148  |
    +-----------+---------------+
    |A7         |0.074 x 0.105  |
    +-----------+---------------+
    |A8         |0.052 x 0.074  |
    +-----------+---------------+
    |A9         |0.037 x 0.052  |
    +-----------+---------------+
    |B0         |1.000 x 1.414  |
    +-----------+---------------+
    |B1         |0.500 x 0.707  |
    +-----------+---------------+
    |B10        |0.031 x 0.044  |
    +-----------+---------------+
    |B2         |0.500 x 0.707  |
    +-----------+---------------+
    |B3         |0.353 x 0.500  |
    +-----------+---------------+
    |B4         |0.250 x 0.353  |
    +-----------+---------------+
    |B6         |0.125 x 0.176  |
    +-----------+---------------+
    |B7         |0.088 x 0.125  |
    +-----------+---------------+
    |B8         |0.062 x 0.088  |
    +-----------+---------------+
    |B9         |0.044 x 0.062  |
    +-----------+---------------+
    |C5E        |0.163 x 0.229  |
    +-----------+---------------+
    |Comm10E    |0.105 x 0.241  |
    +-----------+---------------+
    |DLE        |0.110 x 0.220  |
    +-----------+---------------+
    |Folio      |0.210 x 0.330  |
    +-----------+---------------+
    |Ledger     |0.432 x 0.279  |
    +-----------+---------------+
    |Tabloid    |0.279 x 0.432  |
    +-----------+---------------+

    """
    __gr.gr_beginprintext(char(pathname), char(mode), char(fmt), char(orientation))


def endprint():
    __gr.gr_endprint()


def ndctowc(x, y):
    _x = c_double(x)
    _y = c_double(y)
    __gr.gr_ndctowc(byref(_x), byref(_y))
    return [_x.value, _y.value]


def wctondc(x, y):
    _x = c_double(x)
    _y = c_double(y)
    __gr.gr_wctondc(byref(_x), byref(_y))
    return [_x.value, _y.value]


def wc3towc(x, y, z):
    _x = c_double(x)
    _y = c_double(y)
    _z = c_double(z)
    __gr.gr_wc3towc(byref(_x), byref(_y), byref(_z))
    return [_x.value, _y.value, _z.value]


def drawrect(xmin, xmax, ymin, ymax):
    """
    Draw a rectangle using the current line attributes.

    **Parameters:**

    `xmin` :
        Lower left edge of the rectangle
    `xmax` :
        Lower right edge of the rectangle
    `ymin` :
        Upper left edge of the rectangle
    `ymax` :
        Upper right edge of the rectangle

    """
    __gr.gr_drawrect(c_double(xmin), c_double(xmax), c_double(ymin), c_double(ymax))


def fillrect(xmin, xmax, ymin, ymax):
    """
    Draw a filled rectangle using the current fill attributes.

    **Parameters:**

    `xmin` :
        Lower left edge of the rectangle
    `xmax` :
        Lower right edge of the rectangle
    `ymin` :
        Upper left edge of the rectangle
    `ymax` :
        Upper right edge of the rectangle

    """
    __gr.gr_fillrect(c_double(xmin), c_double(xmax), c_double(ymin), c_double(ymax))


def drawarc(xmin, xmax, ymin, ymax, a1, a2):
    """
    Draw a circular or elliptical arc covering the specified rectangle.

    **Parameters:**

    `xmin` :
        Lower left edge of the rectangle
    `xmax` :
        Lower right edge of the rectangle
    `ymin` :
        Upper left edge of the rectangle
    `ymax` :
        Upper right edge of the rectangle
    `a1` :
        The start angle
    `a2` :
        The end angle

    The resulting arc begins at `a1` and ends at `a2` degrees. Angles are interpreted
    such that 0 degrees is at the 3 o'clock position. The center of the arc is the center
    of the given rectangle.

    """
    __gr.gr_drawarc(c_double(xmin), c_double(xmax), c_double(ymin), c_double(ymax),
                    c_int(a1), c_int(a2))


def fillarc(xmin, xmax, ymin, ymax, a1, a2):
    """
    Fill a circular or elliptical arc covering the specified rectangle.

    **Parameters:**

    `xmin` :
        Lower left edge of the rectangle
    `xmax` :
        Lower right edge of the rectangle
    `ymin` :
        Upper left edge of the rectangle
    `ymax` :
        Upper right edge of the rectangle
    `a1` :
        The start angle
    `a2` :
        The end angle

    The resulting arc begins at `a1` and ends at `a2` degrees. Angles are interpreted
    such that 0 degrees is at the 3 o'clock position. The center of the arc is the center
    of the given rectangle.

    """
    __gr.gr_fillarc(c_double(xmin), c_double(xmax), c_double(ymin), c_double(ymax),
                    c_int(a1), c_int(a2))


def drawpath(points, codes, fill):
    """
    Draw simple and compound outlines consisting of line segments and bezier curves.

    **Parameters:**

    `points` :
        (N, 2) array of (x, y) vertices
    `codes` :
        N-length array of path codes
    `fill` :
        A flag indication whether resulting path is to be filled or not

    The following path codes are recognized:

    +----------+-----------------------------------------------------------+
    |      STOP|end the entire path                                        |
    +----------+-----------------------------------------------------------+
    |    MOVETO|move to the given vertex                                   |
    +----------+-----------------------------------------------------------+
    |    LINETO|draw a line from the current position to the given vertex  |
    +----------+-----------------------------------------------------------+
    |    CURVE3|draw a quadratic Bézier curve                              |
    +----------+-----------------------------------------------------------+
    |    CURVE4|draw a cubic Bézier curve                                  |
    +----------+-----------------------------------------------------------+
    | CLOSEPOLY|draw a line segment to the start point of the current path |
    +----------+-----------------------------------------------------------+

    """
    _len = len(points)
    _points = floatarray(_len, points)
    if codes is not None:
        _codes = uint8array(codes)
    else:
        _codes = nothing()
    __gr.gr_drawpath(c_int(_len), _points.data, _codes.data, c_int(fill))


def setarrowstyle(style):
    """
    Set the arrow style to be used for subsequent arrow commands.

    **Parameters:**

    `style` :
        The arrow style to be used

    `setarrowstyle` defines the arrow style for subsequent arrow primitives.
    The default arrow style is 1.

    +---+----------------------------------+
    |  1|simple, single-ended              |
    +---+----------------------------------+
    |  2|simple, single-ended, acute head  |
    +---+----------------------------------+
    |  3|hollow, single-ended              |
    +---+----------------------------------+
    |  4|filled, single-ended              |
    +---+----------------------------------+
    |  5|triangle, single-ended            |
    +---+----------------------------------+
    |  6|filled triangle, single-ended     |
    +---+----------------------------------+
    |  7|kite, single-ended                |
    +---+----------------------------------+
    |  8|filled kite, single-ended         |
    +---+----------------------------------+
    |  9|simple, double-ended              |
    +---+----------------------------------+
    | 10|simple, double-ended, acute head  |
    +---+----------------------------------+
    | 11|hollow, double-ended              |
    +---+----------------------------------+
    | 12|filled, double-ended              |
    +---+----------------------------------+
    | 13|triangle, double-ended            |
    +---+----------------------------------+
    | 14|filled triangle, double-ended     |
    +---+----------------------------------+
    | 15|kite, double-ended                |
    +---+----------------------------------+
    | 16|filled kite, double-ended         |
    +---+----------------------------------+
    | 17|double line, single-ended         |
    +---+----------------------------------+
    | 18|double line, double-ended         |
    +---+----------------------------------+

    """
    __gr.gr_setarrowstyle(c_int(style))


def setarrowsize(size):
    """
    Set the arrow size to be used for subsequent arrow commands.

    **Parameters:**

    `size` :
        The arrow size to be used

    `setarrowsize` defines the arrow size for subsequent arrow primitives.
    The default arrow size is 1.

    """
    __gr.gr_setarrowsize(c_double(size))


def drawarrow(x1, y1, x2, y2):
    """
    Draw an arrow between two points.

    **Parameters:**

    `x1`, `y1` :
        Starting point of the arrow (tail)
    `x2`, `y2` :
        Head of the arrow

    Different arrow styles (angles between arrow tail and wing, optionally filled
    heads, double headed arrows) are available and can be set with the `setarrowstyle`
    function.

    """
    __gr.gr_drawarrow(c_double(x1), c_double(y1), c_double(x2), c_double(y2))


def readimage(path):
    width = c_int()
    height = c_int()
    _data = POINTER(c_int)()
    __gr.gr_readimage(char(path), byref(width), byref(height), byref(_data))
    _type = (c_int * (width.value * height.value))
    try:
        data = _type.from_address(addressof(_data.contents))
    except:
        data = []
    return [width.value, height.value, data]


def drawimage(xmin, xmax, ymin, ymax, width, height, data, model=0):
    """
    Draw an image into a given rectangular area.

    **Parameters:**

    `xmin`, `ymin` :
        First corner point of the rectangle
    `xmax`, `ymax` :
        Second corner point of the rectangle
    `width`, `height` :
        The width and the height of the image
    `data` :
        An array of color values dimensioned `width` by `height`
    `model` :
        Color model (default=0)

    The available color models are:

    +-----------------------+---+-----------+
    |MODEL_RGB              |  0|   AABBGGRR|
    +-----------------------+---+-----------+
    |MODEL_HSV              |  1|   AAVVSSHH|
    +-----------------------+---+-----------+


    The points (`xmin`, `ymin`) and (`xmax`, `ymax`) are world coordinates defining
    diagonally opposite corner points of a rectangle. This rectangle is divided into
    `width` by `height` cells. The two-dimensional array `data` specifies colors
    for each cell.

    """
    _data = intarray(width * height, data)
    __gr.gr_drawimage(c_double(xmin), c_double(xmax), c_double(ymin), c_double(ymax),
                      c_int(width), c_int(height), _data.data, c_int(model))


def importgraphics(path):
    return __gr.gr_importgraphics(char(path))


def setshadow(offsetx, offsety, blur):
    """
    `setshadow` allows drawing of shadows, realized by images painted underneath,
    and offset from, graphics objects such that the shadow mimics the effect of a light
    source cast on the graphics objects.

    **Parameters:**

    `offsetx` :
        An x-offset, which specifies how far in the horizontal direction the
        shadow is offset from the object
    `offsety` :
        A y-offset, which specifies how far in the vertical direction the shadow
        is offset from the object
    `blur` :
        A blur value, which specifies whether the object has a hard or a diffuse
        edge

    """
    __gr.gr_setshadow(c_double(offsetx), c_double(offsety), c_double(blur))


def settransparency(alpha):
    """
    Set the value of the alpha component associated with GR colors

    **Parameters:**

    `alpha` :
        An alpha value (0.0 - 1.0)

    """
    __gr.gr_settransparency(c_double(alpha))


def setcoordxform(mat):
    """
    Change the coordinate transformation according to the given matrix.

    **Parameters:**

    `mat[3][2]` :
        2D transformation matrix

    """
    _mat = floatarray(6, mat)
    __gr.gr_setcoordxform(_mat.data)


def begingraphics(path):
    """
    Open a file for graphics output.

    **Parameters:**

    `path` :
        Filename for the graphics file.

    `begingraphics` allows to write all graphics output into a XML-formatted file until
    the `endgraphics` functions is called. The resulting file may later be imported with
    the `importgraphics` function.

    """
    __gr.gr_begingraphics(char(path))


def endgraphics():
    __gr.gr_endgraphics()


def getgraphics():
    _string = c_char_p();
    _string = __gr.gr_getgraphics()
    return _string


def drawgraphics(string):
    return __gr.gr_drawgraphics(char(string))


def mathtex(x, y, string):
    """
    Generate a character string starting at the given location. Strings can be defined
    to create mathematical symbols and Greek letters using LaTeX syntax.

    **Parameters:**

    `x`, `y` :
        Position of the text string specified in world coordinates
    `string` :
        The text string to be drawn

    """
    return __gr.gr_mathtex(c_double(x), c_double(y), char(string))


def beginselection(index, kind):
    __gr.gr_beginselection(c_int(index), c_int(kind))


def endselection():
    __gr.gr_endselection()


def moveselection(x, y):
    __gr.gr_moveselection(c_double(x), c_double(y))


def resizeselection(kind, x, y):
    __gr.gr_resizeselection(c_int(kind), c_double(x), c_double(y))


def inqbbox():
    xmin = c_double()
    xmax = c_double()
    ymin = c_double()
    ymax = c_double()
    __gr.gr_inqbbox(byref(xmin), byref(xmax), byref(ymin), byref(ymax))
    return [xmin.value, xmax.value, ymin.value, ymax.value]


def mimetype():
    global _mime_type
    return _mime_type


def isinline():
    global _mime_type
    return (_mime_type and _mime_type != "mov")


def inline(mime="svg"):
    global _mime_type
    if _mime_type != mime:
        os.environ["GKS_WSTYPE"] = mime
        emergencyclosegks()
        _mime_type = mime


def show():
    global _mime_type
    emergencyclosegks()
    if _mime_type == 'svg':
        content = SVG(data=open('gks.svg', 'rb').read())
        display(content)
    elif _mime_type == 'png':
        content = Image(data=open('gks.png', 'rb').read(), width=465, height=465)
        display(content)
    elif _mime_type == 'mov':
        content = HTML(data='<video controls autoplay type="video/mp4" src="data:video/mp4;base64,{0}">'.format(b64encode(open('gks.mov', 'rb').read())))
        return content
    return None


def setregenflags(flags):
    __gr.gr_setregenflags(c_int(flags))


def inqregenflags():
    return __gr.gr_inqregenflags()


def savestate():
    __gr.gr_savestate()


def restorestate():
    __gr.gr_restorestate()


def selectcontext(context):
    __gr.gr_selectcontext(c_int(context))


def destroycontext(context):
    __gr.gr_destroycontext(c_int(context))


def uselinespec(linespec):
    return __gr.gr_uselinespec(char(linespec))


def trisurface(px, py, pz):
    """
    Draw a triangular surface plot for the given data points.

    **Parameters:**

    `x` :
        A list containing the X coordinates
    `y` :
        A list containing the Y coordinates
    `z` :
        A list containing the Z coordinates

    """
    nx = len(px)
    ny = len(py)
    nz = len(pz)
    _px = floatarray(nx, px)
    _py = floatarray(ny, py)
    _pz = floatarray(nz, pz)
    n = min(nx, ny, nz)
    __gr.gr_trisurface(c_int(n), _px.data, _py.data, _pz.data)


def tricontour(px, py, pz, levels):
    """
    Draw a contour plot for the given triangle mesh.

    **Parameters:**

    `x` :
        A list containing the X coordinates
    `y` :
        A list containing the Y coordinates
    `z` :
        A list containing the Z coordinates
    `levels` :
        A list containing the contour levels

    """
    nx = len(px)
    ny = len(py)
    nz = len(pz)
    nlevels = len(levels)
    _px = floatarray(nx, px)
    _py = floatarray(ny, py)
    _pz = floatarray(nz, pz)
    _levels = floatarray(nlevels, levels)
    n = min(nx, ny, nz)
    __gr.gr_tricontour(c_int(n), _px.data, _py.data, _pz.data, c_int(nlevels), _levels.data)


_grPkgDir = os.path.realpath(os.path.dirname(__file__))
_grLibDir = os.getenv("GRLIB", _grPkgDir)
_gksFontPath = os.path.join(_grPkgDir, "fonts")
if os.access(_gksFontPath, os.R_OK):
    os.environ["GKS_FONTPATH"] = os.getenv("GKS_FONTPATH", _grPkgDir)

if platform == "win32":
    libext = ".dll"
else:
    libext = ".so"

_grLib = os.path.join(_grLibDir, "libGR" + libext)
if not os.getenv("GRLIB") and not os.access(_grLib, os.R_OK):
    _grLibDir = os.path.join(_grPkgDir, "..", "..")
    _grLib = os.path.join(_grLibDir, "libGR" + libext)
if platform == "win32":
    os.environ["PATH"] = os.getenv("PATH", "") + ";" + _grLibDir
__gr = CDLL(_grLib)

__gr.gr_opengks.argtypes = []
__gr.gr_closegks.argtypes = []
__gr.gr_inqdspsize.argtypes = [POINTER(c_double), POINTER(c_double),
                               POINTER(c_int), POINTER(c_int)]
__gr.gr_openws.argtypes = [c_int, c_char_p, c_int]
__gr.gr_closews.argtypes = [c_int]
__gr.gr_activatews.argtypes = [c_int]
__gr.gr_deactivatews.argtypes = [c_int]
__gr.gr_clearws.argtypes = []
__gr.gr_updatews.argtypes = []
__gr.gr_polyline.argtypes = [c_int, POINTER(c_double), POINTER(c_double)]
__gr.gr_polymarker.argtypes = [c_int, POINTER(c_double), POINTER(c_double)]
__gr.gr_text.argtypes = [c_double, c_double, c_char_p]
__gr.gr_fillarea.argtypes = [c_int, POINTER(c_double), POINTER(c_double)]
__gr.gr_cellarray.argtypes = [
    c_double, c_double, c_double, c_double, c_int, c_int, c_int, c_int, c_int, c_int,
    POINTER(c_int)]
__gr.gr_spline.argtypes = [c_int, POINTER(c_double), POINTER(c_double), c_int, c_int]
__gr.gr_gridit.argtypes = [
    c_int, POINTER(c_double), POINTER(c_double), POINTER(c_double), c_int, c_int,
    POINTER(c_double), POINTER(c_double), POINTER(c_double)]
__gr.gr_setlinetype.argtypes = [c_int]
__gr.gr_inqlinetype.argtypes = [POINTER(c_int)]
__gr.gr_setlinewidth.argtypes = [c_double]
__gr.gr_inqlinewidth.argtypes = [POINTER(c_double)]
__gr.gr_setlinecolorind.argtypes = [c_int]
__gr.gr_inqlinecolorind.argtypes = [POINTER(c_int)]
__gr.gr_setmarkertype.argtypes = [c_int]
__gr.gr_inqmarkertype.argtypes = [POINTER(c_int)]
__gr.gr_setmarkersize.argtypes = [c_double]
__gr.gr_setmarkercolorind.argtypes = [c_int]
__gr.gr_inqmarkercolorind.argtypes = [POINTER(c_int)]
__gr.gr_settextfontprec.argtypes = [c_int, c_int]
__gr.gr_setcharexpan.argtypes = [c_double]
__gr.gr_setcharspace.argtypes = [c_double]
__gr.gr_settextcolorind.argtypes = [c_int]
__gr.gr_setcharheight.argtypes = [c_double]
__gr.gr_setcharup.argtypes = [c_double, c_double]
__gr.gr_settextpath.argtypes = [c_int]
__gr.gr_settextalign.argtypes = [c_int, c_int]
__gr.gr_setfillintstyle.argtypes = [c_int]
__gr.gr_setfillstyle.argtypes = [c_int]
__gr.gr_setfillcolorind.argtypes = [c_int]
__gr.gr_setcolorrep.argtypes = [c_int, c_double, c_double, c_double]
__gr.gr_setwindow.argtypes = [c_double, c_double, c_double, c_double]
__gr.gr_inqwindow.argtypes = [POINTER(c_double), POINTER(c_double),
                              POINTER(c_double), POINTER(c_double)]
__gr.gr_setviewport.argtypes = [c_double, c_double, c_double, c_double]
__gr.gr_inqviewport.argtypes = [POINTER(c_double), POINTER(c_double),
                                POINTER(c_double), POINTER(c_double)]
__gr.gr_selntran.argtypes = [c_int]
__gr.gr_setclip.argtypes = [c_int]
__gr.gr_setwswindow.argtypes = [c_double, c_double, c_double, c_double]
__gr.gr_setwsviewport.argtypes = [c_double, c_double, c_double, c_double]
__gr.gr_createseg.argtypes = [c_int]
__gr.gr_copysegws.argtypes = [c_int]
__gr.gr_redrawsegws.argtypes = []
__gr.gr_setsegtran.argtypes = [
    c_int, c_double, c_double, c_double, c_double, c_double, c_double, c_double]
__gr.gr_closeseg.argtypes = []
__gr.gr_emergencyclosegks.argtypes = []
__gr.gr_updategks.argtypes = []
__gr.gr_setspace.argtypes = [c_double, c_double, c_int, c_int]
__gr.gr_inqspace.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_int),
                             POINTER(c_int)]
__gr.gr_setscale.argtypes = [c_int]
__gr.gr_inqscale.argtypes = [POINTER(c_int)]
__gr.gr_textext.argtypes = [c_double, c_double, c_char_p]
__gr.gr_inqtextext.argtypes = [c_double, c_double, c_char_p, POINTER(c_double),
                               POINTER(c_double)]
__gr.gr_axeslbl.argtypes = [c_double, c_double, c_double, c_double, c_int, c_int,
                            c_double, _axeslbl_callback, _axeslbl_callback]
__gr.gr_axes.argtypes = [c_double, c_double, c_double, c_double, c_int, c_int, c_double]
__gr.gr_grid.argtypes = [c_double, c_double, c_double, c_double, c_int, c_int]
__gr.gr_verrorbars.argtypes = [c_int, POINTER(c_double), POINTER(c_double),
                               POINTER(c_double), POINTER(c_double)]
__gr.gr_herrorbars.argtypes = [c_int, POINTER(c_double), POINTER(c_double),
                               POINTER(c_double), POINTER(c_double)]
__gr.gr_polyline3d.argtypes = [c_int, POINTER(c_double), POINTER(c_double),
                               POINTER(c_double)]
__gr.gr_polymarker3d.argtypes = [c_int, POINTER(c_double), POINTER(c_double),
                                 POINTER(c_double)]
__gr.gr_axes3d.argtypes = [
    c_double, c_double, c_double, c_double, c_double, c_double, c_int, c_int, c_int,
    c_double]
__gr.gr_grid3d.argtypes = [
    c_double, c_double, c_double, c_double, c_double, c_double, c_int, c_int, c_int]
__gr.gr_titles3d.argtypes = [c_char_p, c_char_p, c_char_p]
__gr.gr_surface.argtypes = [c_int, c_int, POINTER(c_double), POINTER(c_double),
                            POINTER(c_double), c_int]
__gr.gr_contour.argtypes = [
    c_int, c_int, c_int, POINTER(c_double), POINTER(c_double), POINTER(c_double),
    POINTER(c_double), c_int]
__gr.gr_hexbin.argtypes = [c_int, POINTER(c_double), POINTER(c_double), c_int]
__gr.gr_hexbin.restype = c_int
__gr.gr_setcolormap.argtypes = [c_int]
__gr.gr_colorbar.argtypes = []
__gr.gr_inqcolor.argtypes = [c_int, POINTER(c_int)]
__gr.gr_inqcolorfromrgb.argtypes = [c_double, c_double, c_double]
__gr.gr_inqcolorfromrgb.restype = c_int
__gr.gr_hsvtorgb.argtypes = [c_double, c_double, c_double]
__gr.gr_tick.argtypes = [c_double, c_double]
__gr.gr_tick.restype = c_double
__gr.gr_validaterange.argtypes = [c_double, c_double]
__gr.gr_validaterange.restype = c_int
__gr.gr_adjustlimits.argtypes = [POINTER(c_double), POINTER(c_double)]
__gr.gr_adjustrange.argtypes = [POINTER(c_double), POINTER(c_double)]
__gr.gr_beginprint.argtypes = [c_char_p]
__gr.gr_beginprintext.argtypes = [c_char_p, c_char_p, c_char_p, c_char_p]
__gr.gr_endprint.argtypes = []
__gr.gr_ndctowc.argtypes = [POINTER(c_double), POINTER(c_double)]
__gr.gr_wctondc.argtypes = [POINTER(c_double), POINTER(c_double)]
__gr.gr_wc3towc.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
__gr.gr_drawrect.argtypes = [c_double, c_double, c_double, c_double]
__gr.gr_fillrect.argtypes = [c_double, c_double, c_double, c_double]
__gr.gr_drawarc.argtypes = [c_double, c_double, c_double, c_double, c_int, c_int]
__gr.gr_fillarc.argtypes = [c_double, c_double, c_double, c_double, c_int, c_int]
__gr.gr_drawpath.argtypes = [c_int, POINTER(c_double), POINTER(c_uint8), c_int]
__gr.gr_setarrowstyle.argtypes = [c_int]
__gr.gr_setarrowsize.argtypes = [c_double]
__gr.gr_drawarrow.argtypes = [c_double, c_double, c_double, c_double]
__gr.gr_readimage.argtypes = [c_char_p, POINTER(c_int), POINTER(c_int),
                              POINTER(POINTER(c_int))]
__gr.gr_drawimage.argtypes = [c_double, c_double, c_double, c_double,
                              c_int, c_int, POINTER(c_int), c_int]
__gr.gr_importgraphics.argtypes = [c_char_p]
__gr.gr_importgraphics.restype = c_int
__gr.gr_setshadow.argtypes = [c_double, c_double, c_double]
__gr.gr_settransparency.argtypes = [c_double]
__gr.gr_setcoordxform.argtypes = [POINTER(c_double)]
__gr.gr_begingraphics.argtypes = [c_char_p]
__gr.gr_endgraphics.argtypes = []
__gr.gr_getgraphics.argtypes = []
__gr.gr_getgraphics.restype = c_char_p
__gr.gr_drawgraphics.argtypes = [c_char_p]
__gr.gr_drawgraphics.restype = c_int
__gr.gr_mathtex.argtypes = [c_double, c_double, c_char_p]
__gr.gr_beginselection.argtypes = [c_int, c_int]
__gr.gr_endselection.argtypes = []
__gr.gr_moveselection.argtypes = [c_double, c_double]
__gr.gr_resizeselection.argtypes = [c_int, c_double, c_double]
__gr.gr_inqbbox.argtypes = [POINTER(c_double), POINTER(c_double),
                            POINTER(c_double), POINTER(c_double)]
__gr.gr_precision.argtypes = []
__gr.gr_precision.restype = c_double
__gr.gr_setregenflags.argtypes = [c_int]
__gr.gr_inqregenflags.argtypes = []
__gr.gr_inqregenflags.restype = c_int
__gr.gr_savestate.argtypes = []
__gr.gr_restorestate.argtypes = []
__gr.gr_selectcontext.argtypes = [c_int]
__gr.gr_destroycontext.argtypes = [c_int]
__gr.gr_uselinespec.argtypes = [c_char_p]
__gr.gr_uselinespec.restype = c_int
__gr.gr_trisurface.argtypes = [
    c_int, POINTER(c_double), POINTER(c_double), POINTER(c_double)]
__gr.gr_tricontour.argtypes = [
    c_int, POINTER(c_double), POINTER(c_double), POINTER(c_double),
    c_int, POINTER(c_double)]


precision = __gr.gr_precision()

ASF_BUNDLED = 0
ASF_INDIVIDUAL = 1

NOCLIP = 0
CLIP = 1

COORDINATES_WC = 0
COORDINATES_NDC = 1

INTSTYLE_HOLLOW = 0
INTSTYLE_SOLID = 1
INTSTYLE_PATTERN = 2
INTSTYLE_HATCH = 3

TEXT_HALIGN_NORMAL = 0
TEXT_HALIGN_LEFT = 1
TEXT_HALIGN_CENTER = 2
TEXT_HALIGN_RIGHT = 3
TEXT_VALIGN_NORMAL = 0
TEXT_VALIGN_TOP = 1
TEXT_VALIGN_CAP = 2
TEXT_VALIGN_HALF = 3
TEXT_VALIGN_BASE = 4
TEXT_VALIGN_BOTTOM = 5

TEXT_PATH_RIGHT = 0
TEXT_PATH_LEFT = 1
TEXT_PATH_UP = 2
TEXT_PATH_DOWN = 3

TEXT_PRECISION_STRING = 0
TEXT_PRECISION_CHAR = 1
TEXT_PRECISION_STROKE = 2

LINETYPE_SOLID = 1
LINETYPE_DASHED = 2
LINETYPE_DOTTED = 3
LINETYPE_DASHED_DOTTED = 4
LINETYPE_DASH_2_DOT = -1
LINETYPE_DASH_3_DOT = -2
LINETYPE_LONG_DASH = -3
LINETYPE_LONG_SHORT_DASH = -4
LINETYPE_SPACED_DASH = -5
LINETYPE_SPACED_DOT = -6
LINETYPE_DOUBLE_DOT = -7
LINETYPE_TRIPLE_DOT = -8

MARKERTYPE_DOT = 1
MARKERTYPE_PLUS = 2
MARKERTYPE_ASTERISK = 3
MARKERTYPE_CIRCLE = 4
MARKERTYPE_DIAGONAL_CROSS = 5
MARKERTYPE_SOLID_CIRCLE = -1
MARKERTYPE_TRIANGLE_UP = -2
MARKERTYPE_SOLID_TRI_UP = -3
MARKERTYPE_TRIANGLE_DOWN = -4
MARKERTYPE_SOLID_TRI_DOWN = -5
MARKERTYPE_SQUARE = -6
MARKERTYPE_SOLID_SQUARE = -7
MARKERTYPE_BOWTIE = -8
MARKERTYPE_SOLID_BOWTIE = -9
MARKERTYPE_HOURGLASS = -10
MARKERTYPE_SOLID_HGLASS = -11
MARKERTYPE_DIAMOND = -12
MARKERTYPE_SOLID_DIAMOND = -13
MARKERTYPE_STAR = -14
MARKERTYPE_SOLID_STAR = -15
MARKERTYPE_TRI_UP_DOWN = -16
MARKERTYPE_SOLID_TRI_RIGHT = -17
MARKERTYPE_SOLID_TRI_LEFT = -18
MARKERTYPE_HOLLOW_PLUS = -19
MARKERTYPE_SOLID_PLUS = -20
MARKERTYPE_PENTAGON = -21
MARKERTYPE_HEXAGON = -22
MARKERTYPE_HEPTAGON = -23
MARKERTYPE_OCTAGON = -24
MARKERTYPE_STAR_4 = -25
MARKERTYPE_STAR_5 = -26
MARKERTYPE_STAR_6 = -27
MARKERTYPE_STAR_7 = -28
MARKERTYPE_STAR_8 = -29
MARKERTYPE_VLINE = -30
MARKERTYPE_HLINE = -31
MARKERTYPE_OMARK = -32

OPTION_X_LOG = 1
OPTION_Y_LOG = 2
OPTION_Z_LOG = 4
OPTION_FLIP_X = 8
OPTION_FLIP_Y = 16
OPTION_FLIP_Z = 32

OPTION_LINES = 0
OPTION_MESH = 1
OPTION_FILLED_MESH = 2
OPTION_Z_SHADED_MESH = 3
OPTION_COLORED_MESH = 4
OPTION_CELL_ARRAY = 5
OPTION_SHADED_MESH = 6

MODEL_RGB = 0
MODEL_HSV = 1

COLORMAP_UNIFORM = 0
COLORMAP_TEMPERATURE = 1
COLORMAP_GRAYSCALE = 2
COLORMAP_GLOWING = 3
COLORMAP_RAINBOWLIKE = 4
COLORMAP_GEOLOGIC = 5
COLORMAP_GREENSCALE = 6
COLORMAP_CYANSCALE = 7
COLORMAP_BLUESCALE = 8
COLORMAP_MAGENTASCALE = 9
COLORMAP_REDSCALE = 10
COLORMAP_FLAME = 11
COLORMAP_BROWNSCALE = 12
COLORMAP_PILATUS = 13
COLORMAP_AUTUMN = 14
COLORMAP_BONE = 15
COLORMAP_COOL = 16
COLORMAP_COPPER = 17
COLORMAP_GRAY = 18
COLORMAP_HOT = 19
COLORMAP_HSV = 20
COLORMAP_JET = 21
COLORMAP_PINK = 22
COLORMAP_SPECTRAL = 23
COLORMAP_SPRING = 24
COLORMAP_SUMMER = 25
COLORMAP_WINTER = 26
COLORMAP_GIST_EARTH = 27
COLORMAP_GIST_HEAT = 28
COLORMAP_GIST_NCAR = 29
COLORMAP_GIST_RAINBOW = 30
COLORMAP_GIST_STERN = 31
COLORMAP_AFMHOT = 32
COLORMAP_BRG = 33
COLORMAP_BWR = 34
COLORMAP_COOLWARM = 35
COLORMAP_CMRMAP = 36
COLORMAP_CUBEHELIX = 37
COLORMAP_GNUPLOT = 38
COLORMAP_GNUPLOT2 = 39
COLORMAP_OCEAN = 40
COLORMAP_RAINBOW = 41
COLORMAP_SEISMIC = 42
COLORMAP_TERRAIN = 43
COLORMAP_VIRIDIS = 44
COLORMAP_INFERNO = 45
COLORMAP_PLASMA = 46
COLORMAP_MAGMA = 47

COLORMAPS = [("UNIFORM", COLORMAP_UNIFORM),
             ("TEMPERATURE", COLORMAP_TEMPERATURE),
             ("GRAYSCALE", COLORMAP_GRAYSCALE),
             ("GLOWING", COLORMAP_GLOWING),
             ("RAINBOWLIKE", COLORMAP_RAINBOWLIKE),
             ("GEOLOGIC", COLORMAP_GEOLOGIC),
             ("GREENSCALE", COLORMAP_GREENSCALE),
             ("CYANSCALE", COLORMAP_CYANSCALE),
             ("BLUESCALE", COLORMAP_BLUESCALE),
             ("MAGENTASCALE", COLORMAP_MAGENTASCALE),
             ("REDSCALE", COLORMAP_REDSCALE),
             ("FLAME", COLORMAP_FLAME),
             ("BROWNSCALE", COLORMAP_BROWNSCALE),
             ("PILATUS", COLORMAP_PILATUS),
             ("AUTUMN", COLORMAP_AUTUMN),
             ("BONE", COLORMAP_BONE),
             ("COOL", COLORMAP_COOL),
             ("COPPER", COLORMAP_COPPER),
             ("GRAY", COLORMAP_GRAY),
             ("HOT", COLORMAP_HOT),
             ("HSV", COLORMAP_HSV),
             ("JET", COLORMAP_JET),
             ("PINK", COLORMAP_PINK),
             ("SPECTRAL", COLORMAP_SPECTRAL),
             ("SPRING", COLORMAP_SPRING),
             ("SUMMER", COLORMAP_SUMMER),
             ("WINTER", COLORMAP_WINTER),
             ("GIST_EARTH", COLORMAP_GIST_EARTH),
             ("GIST_HEAT", COLORMAP_GIST_HEAT),
             ("GIST_NCAR", COLORMAP_GIST_NCAR),
             ("GIST_RAINBOW", COLORMAP_GIST_RAINBOW),
             ("GIST_STERN", COLORMAP_GIST_STERN),
             ("AFMHOT", COLORMAP_AFMHOT),
             ("BRG", COLORMAP_BRG),
             ("BWR", COLORMAP_BWR),
             ("COOLWARM", COLORMAP_COOLWARM),
             ("CMRMAP", COLORMAP_CMRMAP),
             ("CUBEHELIX", COLORMAP_CUBEHELIX),
             ("GNUPLOT", COLORMAP_GNUPLOT),
             ("GNUPLOT2", COLORMAP_GNUPLOT2),
             ("OCEAN", COLORMAP_OCEAN),
             ("RAINBOW", COLORMAP_RAINBOW),
             ("SEISMIC", COLORMAP_SEISMIC),
             ("TERRAIN", COLORMAP_TERRAIN),
             ("VIRIDIS", COLORMAP_VIRIDIS),
             ("INFERNO", COLORMAP_INFERNO),
             ("PLASMA", COLORMAP_PLASMA),
             ("MAGMA", COLORMAP_MAGMA),
            ]

FONT_TIMES_ROMAN = 101
FONT_TIMES_ITALIC = 102
FONT_TIMES_BOLD = 103
FONT_TIMES_BOLDITALIC = 104
FONT_HELVETICA = 105
FONT_HELVETICA_OBLIQUE = 106
FONT_HELVETICA_BOLD = 107
FONT_HELVETICA_BOLDOBLIQUE = 108
FONT_COURIER = 109
FONT_COURIER_OBLIQUE = 110
FONT_COURIER_BOLD = 111
FONT_COURIER_BOLDOBLIQUE = 112
FONT_SYMBOL = 113
FONT_BOOKMAN_LIGHT = 114
FONT_BOOKMAN_LIGHTITALIC = 115
FONT_BOOKMAN_DEMI = 116
FONT_BOOKMAN_DEMIITALIC = 117
FONT_NEWCENTURYSCHLBK_ROMAN = 118
FONT_NEWCENTURYSCHLBK_ITALIC = 119
FONT_NEWCENTURYSCHLBK_BOLD = 120
FONT_NEWCENTURYSCHLBK_BOLDITALIC = 121
FONT_AVANTGARDE_BOOK = 122
FONT_AVANTGARDE_BOOKOBLIQUE = 123
FONT_AVANTGARDE_DEMI = 124
FONT_AVANTGARDE_DEMIOBLIQUE = 125
FONT_PALATINO_ROMAN = 126
FONT_PALATINO_ITALIC = 127
FONT_PALATINO_BOLD = 128
FONT_PALATINO_BOLDITALIC = 129
FONT_ZAPFCHANCERY_MEDIUMITALIC = 130
FONT_ZAPFDINGBATS = 131

# gr.beginprint types
PRINT_PS = "ps"
PRINT_EPS = "eps"
PRINT_PDF = "pdf"
PRINT_PGF = "pgf"
PRINT_BMP = "bmp"
PRINT_JPEG = "jpeg"
PRINT_JPG = "jpg"
PRINT_PNG = "png"
PRINT_TIFF = "tiff"
PRINT_TIF = "tif"
PRINT_FIG = "fig"
PRINT_SVG = "svg"
PRINT_WMF = "wmf"

PRINT_TYPE = {PRINT_PS: "PostScript (*.ps)",
              PRINT_EPS: "Encapsulated PostScript (*.eps)",
              PRINT_PDF: "Portable Document Format (*.pdf)",
              PRINT_PGF: "PGF/TikZ Graphics Format for TeX (*.pgf)",
              PRINT_BMP: "Windows Bitmap (*.bmp)",
              PRINT_JPEG: "JPEG image (*.jpg *.jpeg)",
              PRINT_PNG: "Portable Network Graphics (*.png)",
              PRINT_TIFF: "Tagged Image File Format (*.tif *.tiff)",
              PRINT_FIG: "Figure (*.fig)",
              PRINT_SVG: "Scalable Vector Graphics (*.svg)",
              PRINT_WMF: "Windows Metafile (*.wmf)"}

# multiple keys
PRINT_TYPE[PRINT_JPG] = PRINT_TYPE[PRINT_JPEG]
PRINT_TYPE[PRINT_TIF] = PRINT_TYPE[PRINT_TIFF]

# gr.begingraphics types
GRAPHIC_GRX = "grx"

GRAPHIC_TYPE = {GRAPHIC_GRX: "Graphics Format (*.grx)"}

# regeneration flags
MPL_SUPPRESS_CLEAR = 1
MPL_POSTPONE_UPDATE = 2

