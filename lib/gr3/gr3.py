__all__ = ['GR3_InitAttribute',
           'GR3_Error',
           'GR3_Exception',
           'GR3_Quality',
           'GR3_Drawable',
           'init',
           'terminate',
           'getimage',
           'export',
           'getrenderpathstring',
           'setlogcallback',
           'drawimage',
           'createmesh',
           'drawmesh',
           'deletemesh',
           'setquality',
           'clear',
           'cameralookat',
           'setcameraprojectionparameters',
           'setlightdirection',
           'drawcylindermesh',
           'drawconemesh',
           'drawspheremesh',
           'setbackgroundcolor']

import ctypes
import ctypes.util
import numpy
import sys
import os

# Shared Library should be in the same directory as the python module
if sys.platform == 'win32':
    _gr3 = ctypes.CDLL(os.path.join(os.path.dirname(os.path.realpath(__file__)),"libgr3.dll"))
else:
    _gr3 = ctypes.CDLL(os.path.join(os.path.dirname(os.path.realpath(__file__)),"libGR3.so"))
    
class GR3_InitAttribute(object):
    GR3_IA_END_OF_LIST = 0
    GR3_IA_FRAMEBUFFER_WIDTH = 1
    GR3_IA_FRAMEBUFFER_HEIGHT = 2

class GR3_Error(object):
    GR3_ERROR_NONE = 0
    GR3_ERROR_INVALID_VALUE = 1
    GR3_ERROR_INVALID_ATTRIBUTE = 2
    GR3_ERROR_INIT_FAILED = 3
    GR3_ERROR_OPENGL_ERR = 4
    GR3_ERROR_OUT_OF_MEM = 5
    GR3_ERROR_NOT_INITIALIZED = 6
    GR3_ERROR_CAMERA_NOT_INITIALIZED = 7
    GR3_ERROR_UNKNOWN_FILE_EXTENSION = 8
    GR3_ERROR_CANNOT_OPEN_FILE = 9
    GR3_ERROR_EXPORT = 10

class GR3_Quality(object):
    GR3_QUALITY_OPENGL_NO_SSAA  = 0
    GR3_QUALITY_OPENGL_2X_SSAA  = 2
    GR3_QUALITY_OPENGL_4X_SSAA  = 4
    GR3_QUALITY_OPENGL_8X_SSAA  = 8
    GR3_QUALITY_OPENGL_16X_SSAA = 16
    GR3_QUALITY_POVRAY_NO_SSAA  = 0+1
    GR3_QUALITY_POVRAY_2X_SSAA  = 2+1
    GR3_QUALITY_POVRAY_4X_SSAA  = 4+1
    GR3_QUALITY_POVRAY_8X_SSAA  = 8+1
    GR3_QUALITY_POVRAY_16X_SSAA = 16+1

class GR3_Drawable(object):
    GR3_DRAWABLE_OPENGL = 1
    GR3_DRAWABLE_GKS = 2

class GR3_Exception(Exception):
    def __init__(self,error_code):
        Exception.__init__(self,geterrorstring(error_code))

def init(attrib_list=[]):
    global _gr3
    _attrib_list = numpy.array(list(attrib_list)+[GR3_InitAttribute.GR3_IA_END_OF_LIST],ctypes.c_uint)
    if py_log_callback:
        lib_found = ctypes.util.find_library(os.path.join(os.path.dirname(os.path.realpath(__file__)),"libGR3.so"))
        if lib_found:
            py_log_callback("Loaded dynamic library from "+lib_found)
        else:
            py_log_callback("Loaded dynamic library unknown.")
    err = _gr3.gr3_init(_attrib_list.ctypes.data_as(ctypes.POINTER(ctypes.c_int)))
    if err:
        raise GR3_Exception(err)

def drawimage(xmin,xmax,ymin,ymax,pixel_width,pixel_height, window):
    global _gr3
    err = _gr3.gr3_drawimage(ctypes.c_float(xmin),ctypes.c_float(xmax),
                       ctypes.c_float(ymin),ctypes.c_float(ymax),
                         ctypes.c_int(pixel_width),ctypes.c_int(pixel_height),ctypes.c_int(window))
    if err:
        raise GR3_Exception(err)

def terminate():
    global _gr3
    _gr3.gr3_terminate()

def setquality(quality):
    global _gr3
    err = _gr3.gr3_setquality(quality)
    if err:
        raise GR3_Exception(err)

def getimage(width, height, use_alpha = True):
    global _gr3
    bpp = 4 if use_alpha else 3
    _bitmap = numpy.zeros((width*height*bpp),ctypes.c_ubyte)
    err = _gr3.gr3_getimage(ctypes.c_uint(width),ctypes.c_uint(height),ctypes.c_uint(use_alpha),_bitmap.ctypes.data_as(ctypes.POINTER(ctypes.c_ubyte)))
    if err:
        raise GR3_Exception(err)
    return _bitmap

def export(filename, width, height):
    global _gr3
    _filename = numpy.array(filename+'\0',ctypes.c_char)
    err = _gr3.gr3_export(_filename.ctypes.data_as(ctypes.POINTER(ctypes.c_char)),ctypes.c_uint(width),ctypes.c_uint(height))
    if err:
        raise GR3_Exception(err)

def geterrorstring(error_code):
    for error_string in dir(GR3_Error):
        if error_string[:len('GR3_ERROR_')] == 'GR3_ERROR_':
            if GR3_Error.__dict__[error_string] == error_code:
                return error_string
    return 'GR3_ERROR_UNKNOWN'

def getrenderpathstring():
    global _gr3
    _gr3.gr3_getrenderpathstring.restype = ctypes.c_char_p
    return str(_gr3.gr3_getrenderpathstring())

py_log_callback = None
log_callback = None
def setlogcallback(func):
    global log_callback, py_log_callback
    global _gr3
    LOGCALLBACK = ctypes.CFUNCTYPE(None, ctypes.c_char_p)
    log_callback = LOGCALLBACK(func)
    py_log_callback = func
    _gr3.gr3_setlogcallback.argtypes = [LOGCALLBACK]
    _gr3.gr3_setlogcallback.restype = None
    _gr3.gr3_setlogcallback(log_callback)


def createmesh(n, vertices, normals, colors):
    _mesh = ctypes.c_uint(0)
    vertices = numpy.array(vertices, ctypes.c_float)
    normals = numpy.array(normals, ctypes.c_float)
    colors = numpy.array(colors, ctypes.c_float)
    err = _gr3.gr3_createmesh(ctypes.byref(_mesh),ctypes.c_uint(n),vertices.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), normals.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), colors.ctypes.data_as(ctypes.POINTER(ctypes.c_float)))
    if err:
        raise GR3_Exception(err)
    return _mesh

def drawcylindermesh(n, positions, directions, colors, radii, lengths):
    positions = numpy.array(positions, ctypes.c_float)
    directions = numpy.array(directions, ctypes.c_float)
    colors = numpy.array(colors, ctypes.c_float)
    radii = numpy.array(radii, ctypes.c_float)
    lengths = numpy.array(lengths, ctypes.c_float)
    _gr3.gr3_drawcylindermesh(ctypes.c_uint(n), positions.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), directions.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), colors.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), radii.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), lengths.ctypes.data_as(ctypes.POINTER(ctypes.c_float)))

def drawconemesh(n, positions, directions, colors, radii, lengths):
    positions = numpy.array(positions, ctypes.c_float)
    directions = numpy.array(directions, ctypes.c_float)
    colors = numpy.array(colors, ctypes.c_float)
    radii = numpy.array(radii, ctypes.c_float)
    lengths = numpy.array(lengths, ctypes.c_float)
    _gr3.gr3_drawconemesh(ctypes.c_uint(n), positions.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), directions.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), colors.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), radii.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), lengths.ctypes.data_as(ctypes.POINTER(ctypes.c_float)))

def drawspheremesh(n, positions,colors, radii):
    positions = numpy.array(positions, ctypes.c_float)
    colors = numpy.array(colors, ctypes.c_float)
    radii = numpy.array(radii, ctypes.c_float)
    _gr3.gr3_drawspheremesh(ctypes.c_uint(n), positions.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), colors.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), radii.ctypes.data_as(ctypes.POINTER(ctypes.c_float)))

def drawmesh(mesh, n, positions, directions, ups, colors, scales):
    positions = numpy.array(positions, ctypes.c_float)
    directions = numpy.array(directions, ctypes.c_float)
    ups = numpy.array(ups, ctypes.c_float)
    colors = numpy.array(colors, ctypes.c_float)
    scales = numpy.array(scales, ctypes.c_float)
    _gr3.gr3_drawmesh(mesh,ctypes.c_uint(n),positions.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), directions.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), ups.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), colors.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), scales.ctypes.data_as(ctypes.POINTER(ctypes.c_float)))
    
def deletemesh(mesh):
    _gr3.gr3_deletemesh(mesh)

def clear():
    _gr3.gr3_clear()

def setbackgroundcolor(red, green, blue, alpha):
    _gr3.gr3_setbackgroundcolor(ctypes.c_float(red),ctypes.c_float(green),ctypes.c_float(blue),ctypes.c_float(alpha))

def cameralookat(camera_x, camera_y, camera_z, center_x, center_y, center_z, up_x, up_y, up_z):
    _gr3.gr3_cameralookat(ctypes.c_float(camera_x), ctypes.c_float(camera_y), ctypes.c_float(camera_z), ctypes.c_float(center_x), ctypes.c_float(center_y), ctypes.c_float(center_z), ctypes.c_float(up_x), ctypes.c_float(up_y), ctypes.c_float(up_z))

def setcameraprojectionparameters(vertical_field_of_view, zNear, zFar):
    _gr3.gr3_setcameraprojectionparameters(ctypes.c_float(vertical_field_of_view), ctypes.c_float(zNear), ctypes.c_float(zFar))
    
def setlightdirection(*xyz):
    if len(xyz) == 3:
        x,y,z = xyz
        _gr3.gr3_setlightdirection(ctypes.c_float(x), ctypes.c_float(y), ctypes.c_float(z))
    elif len(xyz) == 1:
        if xyz[0] is None:
            _gr3.gr3_setlightdirection(ctypes.c_float(0), ctypes.c_float(0), ctypes.c_float(0))
        else:
            raise TypeError("if gr3_setlightdirection() is called with 1 argument, it should be None")
    else:
        raise TypeError("gr3_setlightdirection() takes exactly 1 or exactly 3 arguments (%d given)" %len(xyz))
        
_gr3.gr3_init.argtypes = [ctypes.POINTER(ctypes.c_int)]
_gr3.gr3_terminate.argtypes = []
_gr3.gr3_getrenderpathstring.argtypes = []
_gr3.gr3_geterrorstring.argtypes = [ctypes.c_int]
_gr3.gr3_setlogcallback.argtypes = [ctypes.CFUNCTYPE(None, ctypes.c_char_p)]
_gr3.gr3_clear.argtypes = []
_gr3.gr3_setquality.argtypes = [ctypes.c_int]
_gr3.gr3_getimage.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.POINTER(ctypes.c_ubyte)]
_gr3.gr3_export.argtypes = [ctypes.POINTER(ctypes.c_char), ctypes.c_int, ctypes.c_int]
_gr3.gr3_drawimage.argtypes = [ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_int, ctypes.c_int, ctypes.c_int]
_gr3.gr3_createmesh.argtypes = [ctypes.POINTER(ctypes.c_int), ctypes.c_int, ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float)]
_gr3.gr3_drawmesh.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float)]
_gr3.gr3_deletemesh.argtypes = [ctypes.c_int]

_gr3.gr3_cameralookat.argtypes = [ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float]
_gr3.gr3_setcameraprojectionparameters.argtypes = [ctypes.c_float,ctypes.c_float,ctypes.c_float]
_gr3.gr3_setlightdirection.argtypes = [ctypes.c_float,ctypes.c_float,ctypes.c_float]
_gr3.gr3_setbackgroundcolor.argtypes = [ctypes.c_float,ctypes.c_float,ctypes.c_float,ctypes.c_float]
_gr3.gr3_drawconemesh.argtypes = [ctypes.c_uint, ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float)]
_gr3.gr3_drawcylindermesh.argtypes = [ctypes.c_uint, ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float)]
_gr3.gr3_drawspheremesh.argtypes = [ctypes.c_uint, ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float)]
