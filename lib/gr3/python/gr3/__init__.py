# coding: utf-8

from __future__ import division

__all__ = ['GR3_InitAttribute',
           'GR3_Error',
           'GR3_Exception',
           'GR3_Quality',
           'GR3_Drawable',
           'init',
           'terminate',
           'useframebuffer',
           'usecurrentframebuffer',
           'getimage',
           'export',
           'getrenderpathstring',
           'setlogcallback',
           'drawimage',
           'createmesh',
           'createindexedmesh',
           'drawmesh',
           'createheightmapmesh',
           'drawheightmap',
           'deletemesh',
           'setquality',
           'clear',
           'cameralookat',
           'setcameraprojectionparameters',
           'setlightdirection',
           'drawcylindermesh',
           'drawconemesh',
           'drawspheremesh',
           'drawcubemesh',
           'setbackgroundcolor',
           'triangulate',
           'createisosurfacemesh',
           'drawisosurfacemesh',
           'drawtubemesh',
           'createtubemesh',
           'drawspins',
           'drawmolecule',
           'drawxslicemesh',
           'drawyslicemesh',
           'drawzslicemesh',
           'createxslicemesh',
           'createyslicemesh',
           'createzslicemesh',
           'drawslicemeshes',
           'createslicemeshes',
           'drawtrianglesurface']


import sys
if any([module.startswith('OpenGL') for module in sys.modules]):
    import warnings
    warnings.warn("Importing gr3 after importing pyOpenGL (or any of its modules) might cause problems on some platforms. Please import gr3 first to avoid this.")

from sys import version_info, platform
from platform import python_implementation

from ctypes import c_int, c_uint, c_ushort, c_ubyte, c_float, c_double, \
                   c_char, c_char_p, c_ulong, py_object, POINTER, byref, \
                   CFUNCTYPE, CDLL, create_string_buffer, cast, pointer
import ctypes.util
import numpy
import os
import gr


try:
    from IPython.display import HTML, Image
except:
    _have_ipython = False
else:
    _have_ipython = True


_impl = python_implementation()
if _impl != 'PyPy':
    from ctypes import pythonapi

_gr3PkgDir = os.path.realpath(os.path.dirname(__file__))
_gr3LibDir = os.getenv("GR3LIB", _gr3PkgDir)
if sys.platform == "win32":
    libext = ".dll"
else:
    libext = ".so"

# Detect whether this is a site-package installation
if os.access(os.path.join(_gr3PkgDir, "libGR3" + libext), os.R_OK):
    # Set GRDIR environment accordingly to site-package installation.
    # (needed for finding GKSTerm on OSX)
    os.environ["GRDIR"] = os.getenv("GRDIR",
        os.path.join(os.path.realpath(os.path.dirname(__file__)), "..", "gr"))

_gr3Lib = os.path.join(_gr3LibDir, "libGR3" + libext)
if not os.getenv("GR3LIB") and not os.access(_gr3Lib, os.R_OK):
    _gr3LibDir = os.path.join(_gr3PkgDir, "..", "..")
    _gr3Lib = os.path.join(_gr3LibDir, "libGR3" + libext)
_gr3 = CDLL(_gr3Lib)


class intarray:
    def __init__(self, a):
        if _impl == 'PyPy':
            self.array = numpy.array(a, numpy.int32)
            self.data = cast(self.array.__array_interface__['data'][0],
                             POINTER(c_int))
        else:
            self.array = numpy.array(a, c_int)
            self.data = self.array.ctypes.data_as(POINTER(c_int))


class floatarray:
    def __init__(self, a, copy=False):
        if _impl == 'PyPy':
            self.array = numpy.array(a, numpy.float32, copy=copy)
            self.data = cast(self.array.__array_interface__['data'][0],
                             POINTER(c_float))
        else:
            self.array = numpy.array(a, c_float)
            self.data = self.array.ctypes.data_as(POINTER(c_float))


def char(string):
    if version_info[0] == 3:
        s = create_string_buffer(string.encode('iso8859-15'))
    else:
        s = create_string_buffer(string)
    return cast(s, c_char_p)


_string_types = [str]
if version_info[0] == 2:
    _string_types.append(unicode)


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


class GR3_SurfaceOption(object):
    GR3_SURFACE_DEFAULT     =  0
    GR3_SURFACE_NORMALS     =  1
    GR3_SURFACE_FLAT        =  2
    GR3_SURFACE_GRTRANSFORM =  4
    GR3_SURFACE_GRCOLOR     =  8
    GR3_SURFACE_GRZSHADED   = 16


class GR3_Exception(Exception):
    def __init__(self, error_code, line=0, file=""):
        message = geterrorstring(error_code)
        if line and file:
            message = "{} (l. {}): {}".format(file, line, message)
        Exception.__init__(self, message)


def _error_check(result, *args):
    line = c_int(0)
    file = c_char_p()
    error_code = _gr3.gr3_geterror(1, pointer(line), pointer(file))
    if error_code:
        exception = GR3_Exception(error_code, line.value, file.value)
        raise exception
    return result


def init(attrib_list=[]):
    """
    **Parameters:**

    `attrib_list` :
        a list that can specify details about context creation. The attributes which use unsigned integer values are followed by these values.

    **Raise:**

    `gr3.GR3_Error.GR3_ERROR_EXPORT`: Raises GR3_Exception

        +-----------------------------+-----------------------------------------------------------------------------------------------+
        | GR3_ERROR_NONE              | on success                                                                                    |
        +-----------------------------+-----------------------------------------------------------------------------------------------+
        | GR3_ERROR_INVALID_VALUE     | if one of the attributes' values is out of the allowed range                                  |
        +-----------------------------+-----------------------------------------------------------------------------------------------+
        | GR3_ERROR_INVALID_ATTRIBUTE | if the list contained an unkown attribute or two mutually exclusive attributes were both used |
        +-----------------------------+-----------------------------------------------------------------------------------------------+
        | GR3_ERROR_OPENGL_ERR        | if an OpenGL error occured                                                                    |
        +-----------------------------+-----------------------------------------------------------------------------------------------+
        | GR3_ERROR_INIT_FAILED       | if an error occured during initialization of the "window toolkit" (CGL, GLX, WIN...)          |
        +-----------------------------+-----------------------------------------------------------------------------------------------+

    """
    global _gr3
    _attrib_list = intarray(list(attrib_list)+[GR3_InitAttribute.GR3_IA_END_OF_LIST])
    if py_log_callback:
        lib_found = ctypes.util.find_library(os.path.join(os.path.dirname(os.path.realpath(__file__)),"libGR3.so"))
        if lib_found:
            py_log_callback("Loaded dynamic library from "+lib_found)
        else:
            py_log_callback("Loaded dynamic library unknown.")
    _gr3.gr3_init(_attrib_list.data)

def drawimage(xmin, xmax, ymin, ymax, pixel_width, pixel_height, window):
    global _gr3
    _gr3.gr3_drawimage(c_float(xmin), c_float(xmax),
                       c_float(ymin), c_float(ymax),
                         c_int(pixel_width), c_int(pixel_height), c_int(window))


def terminate():
    """
    This function terminates the gr3 context. After calling this function, gr3 is in the same state as when it was first loaded, except for context-independent variables, i.e. the logging callback.

    .. note:: It is safe to call this function even if the context is not initialized.

    .. seealso:: Functions :py:func:`gr.init()`
    """
    global _gr3
    _gr3.gr3_terminate()

def useframebuffer(framebuffer):
    """
    Set the framebuffer used for drawing to OpenGL (using gr3.drawimage). This function is only needed when you do not want to render to 0, the default framebuffer.

    **Parameters:**

        `framebuffer`: the framebuffer object GR3 should render to.

    """
    global _gr3
    _gr3.gr3_useframebuffer(c_uint(framebuffer))

def usecurrentframebuffer():
    """
    Use the currently bound framebuffer as the framebuffer used for drawing to OpenGL (using gr3.drawimage). This function is only needed when you do not want to render to 0, the default framebuffer.
    """
    global _gr3
    _gr3.gr3_usecurrentframebuffer()

def setquality(quality):
    """
    Set rendering quality

    **Parameters:**
        `quality`:
            The quality to set
    """
    global _gr3
    _gr3.gr3_setquality(quality)


def getimage(width, height, use_alpha = True):
    global _gr3
    bpp = 4 if use_alpha else 3
    _bitmap = numpy.zeros((width*height*bpp), c_ubyte)
    _gr3.gr3_getimage(c_uint(width), c_uint(height), c_uint(use_alpha), _bitmap.ctypes.data_as(POINTER(c_ubyte)))

    return _bitmap

def export(filename, width, height):
    global _gr3
    _filename = char(filename)
    _gr3.gr3_export(_filename, c_uint(width), c_uint(height))
    content = None

    if gr.mimetype() != None and _have_ipython:
        _, _fileextension = os.path.splitext(filename)
        if _fileextension == '.html':
            content = HTML('<iframe src="%s" width=%d height=%d></iframe>' %
                           (filename, width, height))
        elif _fileextension in ['.png', '.jpg', '.jpeg']:
            content = Image(data=open(filename, 'rb').read(),
                            width=width, height=height)
    return content

def geterrorstring(error_code):
    """
    **Parameters:**

        `error_code`: The error code whose representation will be returned.

    This function returns a string representation of a given error code.
    """
    for error_string in dir(GR3_Error):
        if error_string[:len('GR3_ERROR_')] == 'GR3_ERROR_':
            if GR3_Error.__dict__[error_string] == error_code:
                return error_string
    return 'GR3_ERROR_UNKNOWN'

def getrenderpathstring():
    """
    This function allows the user to find out how his commands are rendered.

    **Returns:**

    If gr3 is initialized, a string in the format: `"gr3 - " + window toolkit + " - " + framebuffer extension + " - " + OpenGL version + " - " + OpenGL renderer string`.
    For example `"gr3 - GLX - GL_ARB_framebuffer_object - 2.1 Mesa 7.10.2 - Software Rasterizer"` might be returned on a Linux system (using GLX) with an available GL_ARB_framebuffer_object implementation.
    If gr3 is not initialized `"Not initialized"` is returned.

    """
    global _gr3
    _gr3.gr3_getrenderpathstring.restype = c_char_p
    return str(_gr3.gr3_getrenderpathstring())

py_log_callback = None
log_callback = None
def setlogcallback(func):
    """
    **Parameters:**

        'func': The logging callback, a function which gets a const char pointer as its only argument and returns nothing.

    During software development it will often be helpful to get debug output from gr3. This information is not printed, but reported directly to the user by calling a logging callback. This function allows to set this callback or disable it again by calling it with `NULL`.

    """
    global log_callback, py_log_callback
    global _gr3
    LOGCALLBACK = CFUNCTYPE(None, c_char_p)
    log_callback = LOGCALLBACK(func)
    py_log_callback = func
    _gr3.gr3_setlogcallback.argtypes = [LOGCALLBACK]
    _gr3.gr3_setlogcallback.restype = None
    _gr3.gr3_setlogcallback(log_callback)


def createmesh(n, vertices, normals, colors):
    """
    This function creates a int from vertex position, normal and color data.
    Returns a mesh.

    **Parameters:**

        `vertices` : the vertex positions

        `normals` : the vertex normals

        `colors` : the vertex colors, they should be white (1,1,1) if you want to change the color for each drawn mesh

        `n` : the number of vertices in the mesh

    **Raises:**

    `gr3.GR3_Error.GR3_ERROR_EXPORT`: Raises GR3_Exception

        +----------------------+-------------------------------+
        | GR3_ERROR_NONE       | on success                    |
        +----------------------+-------------------------------+
        | GR3_ERROR_OPENGL_ERR | if an OpenGL error occured    |
        +----------------------+-------------------------------+
        | GR3_ERROR_OUT_OF_MEM | if a memory allocation failed |
        +----------------------+-------------------------------+
    """
    _mesh = c_uint(0)
    vertices = floatarray(vertices)
    normals = floatarray(normals)
    colors = floatarray(colors)
    _gr3.gr3_createmesh(byref(_mesh), c_uint(n), vertices.data, normals.data, colors.data)

    return _mesh

def createindexedmesh(num_vertices, vertices, normals, colors, num_indices, indices):
    """
    This function creates an indexed mesh from vertex information (position,
    normal and color) and triangle information (indices).
    Returns a mesh.

    **Parameters:**

        `num_vertices` : the number of vertices in the mesh

        `vertices` : the vertex positions

        `normals` : the vertex normals

        `colors` : the vertex colors, they should be white (1,1,1) if you want to change the color for each drawn mesh

        `num_indices` : the number of indices in the mesh (three times the number of triangles)

        `indices` : the index array (vertex indices for each triangle)

    **Raises:**

    `gr3.GR3_Error.GR3_ERROR_EXPORT`: Raises GR3_Exception

        +----------------------+-------------------------------+
        | GR3_ERROR_NONE       | on success                    |
        +----------------------+-------------------------------+
        | GR3_ERROR_OPENGL_ERR | if an OpenGL error occured    |
        +----------------------+-------------------------------+
        | GR3_ERROR_OUT_OF_MEM | if a memory allocation failed |
        +----------------------+-------------------------------+
    """
    _mesh = c_uint(0)
    vertices = floatarray(vertices)
    normals = floatarray(normals)
    colors = floatarray(colors)
    indices = intarray(indices)
    _gr3.gr3_createindexedmesh(byref(_mesh), c_uint(num_vertices), vertices.data, normals.data, colors.data, c_uint(num_indices), indices.data)

    return _mesh

def createheightmapmesh(heightmap, num_columns, num_rows):
    heightmap = floatarray(heightmap)
    return _gr3.gr3_createheightmapmesh(heightmap.data, c_int(num_columns), c_int(num_rows))

def drawheightmap(heightmap, num_columns, num_rows, positions, scales):
    heightmap = floatarray(heightmap)
    positions = floatarray(positions)
    scales = floatarray(scales)
    _gr3.gr3_drawheightmap(heightmap.data, c_int(num_columns), c_int(num_rows), positions.data, scales.data)



def drawcylindermesh(n, positions, directions, colors, radii, lengths):
    """
    This function allows drawing a cylinder without requiring a mesh.

    .. seealso::

        Function :py:func:`gr.drawmesh()`
    """
    positions = floatarray(positions)
    directions = floatarray(directions)
    colors = floatarray(colors)
    radii = floatarray(radii)
    lengths = floatarray(lengths)
    _gr3.gr3_drawcylindermesh(c_uint(n), positions.data, directions.data, colors.data, radii.data, lengths.data)

def drawconemesh(n, positions, directions, colors, radii, lengths):
    """
    This function allows drawing a cylinder without requiring a mesh.

    .. seealso::

        Function :py:func:`gr.drawmesh()`
    """
    positions = floatarray(positions)
    directions = floatarray(directions)
    colors = floatarray(colors)
    radii = floatarray(radii)
    lengths = floatarray(lengths)
    _gr3.gr3_drawconemesh(c_uint(n), positions.data, directions.data, colors.data, radii.data, lengths.data)

def drawspheremesh(n, positions, colors, radii):
    """
    This function allows drawing a sphere without requiring a mesh.

    .. seealso::

        Function :py:func:`gr.drawmesh()`
    """
    positions = floatarray(positions)
    colors = floatarray(colors)
    radii = floatarray(radii)
    _gr3.gr3_drawspheremesh(c_uint(n), positions.data, colors.data, radii.data)

def drawmesh(mesh, n, positions, directions, ups, colors, scales):
    """
    This function adds a mesh to the draw list, so it will be drawn when the user calls gr3.getpixmap_(). The given data stays owned by the user, a copy will be saved in the draw list and the mesh reference counter will be increased.

    **Parameters:**

        `mesh` : The mesh to be drawn

        `positions` : The positions where the meshes should be drawn

        `directions` : The forward directions the meshes should be facing at

        `ups` : The up directions

        `colors` : The colors the meshes should be drawn in, it will be multiplied with each vertex color

        `scales` : The scaling factors

        `n` : The number of meshes to be drawn

    .. note:

        This function does not return an error code, because of its asynchronous nature. If gr3.getpixmap_() returns a `GR3_ERROR_OPENGL_ERR`, this might be caused by this function saving unuseable data into the draw list.
    """
    positions = floatarray(positions)
    directions = floatarray(directions)
    ups = floatarray(ups)
    colors = floatarray(colors)
    scales = floatarray(scales)
    _gr3.gr3_drawmesh(mesh, c_uint(n), positions.data, directions.data, ups.data, colors.data, scales.data)

def drawcubemesh(n, positions, directions, ups, colors, scales):
    positions = floatarray(positions)
    directions = floatarray(directions)
    ups = floatarray(ups)
    colors = floatarray(colors)
    scales = floatarray(scales)
    _gr3.gr3_drawcubemesh(c_uint(n), positions.data, directions.data, ups.data, colors.data, scales.data)


def deletemesh(mesh):
    """
    This function marks a mesh for deletion and removes the user's reference from the mesh's referenc counter, so a user must not use the mesh after calling this function. If the mesh is still in use for draw calls, the mesh will not be truly deleted until gr3.clear() is called.

    **Parameters:**

        `mesh` : The mesh that should be marked for deletion

    """
    if hasattr(mesh, 'value'):
        mesh = mesh.value
    _gr3.gr3_deletemesh(mesh)

def clear():
    """
    This function clears the draw list.

    **Raises:**

    `gr3.GR3_Error.GR3_ERROR_EXPORT`: Raises GR3_Exception

        +---------------------------+-------------------------------------------------------------+
        | GR3_ERROR_NONE            | on success                                                  |
        +---------------------------+-------------------------------------------------------------+
        | GR3_ERROR_OPENGL_ERR      | if an OpenGL error occured                                  |
        +---------------------------+-------------------------------------------------------------+
        | GR3_ERROR_NOT_INITIALIZED | if the function was called without calling gr3_init() first |
        +---------------------------+-------------------------------------------------------------+


    """
    _gr3.gr3_clear()

def setbackgroundcolor(red, green, blue, alpha):
    """
    This function sets the background color.
    """
    _gr3.gr3_setbackgroundcolor(c_float(red), c_float(green), c_float(blue), c_float(alpha))

def cameralookat(camera_x, camera_y, camera_z, center_x, center_y, center_z, up_x, up_y, up_z):
    """
    This function sets the view matrix by getting the position of the camera, the position of the center of focus and the direction which should point up. This function takes effect when the next image is created. Therefore if you want to take pictures of the same data from different perspectives, you can call and  gr3.cameralookat(), gr3.getpixmap_(), gr3.cameralookat(), gr3.getpixmap_(), ... without calling gr3_clear() and gr3_drawmesh() again.

    **Parameters:**

        `camera_x` : The x-coordinate of the camera

        `camera_y` : The y-coordinate of the camera

        `camera_z` : The z-coordinate of the camera

        `center_x` : The x-coordinate of the center of focus

        `center_y` : The y-coordinate of the center of focus

        `center_z` : The z-coordinate of the center of focus

        `up_x` : The x-component of the up direction

        `up_y` : The y-component of the up direction

        `up_z` : The z-component of the up direction


    .. note::
        Source: http://www.opengl.org/sdk/docs/man/xhtml/gluLookAt.xml
            (as of 10/24/2011, licensed under SGI Free Software Licence B)

    """
    _gr3.gr3_cameralookat(c_float(camera_x), c_float(camera_y), c_float(camera_z), c_float(center_x), c_float(center_y), c_float(center_z), c_float(up_x), c_float(up_y), c_float(up_z))

def setcameraprojectionparameters(vertical_field_of_view, zNear, zFar):
    """
    This function sets the projection parameters. This function takes effect when the next image is created.

    **Parameters:**

        `vertical_field_of_view` : This parameter is the vertical field of view in degrees. It must be greater than 0 and less than 180.

        `zNear` : The distance to the near clipping plane.

        `zFar` : The distance to the far clipping plane.

    **Raises:**

    `gr3.GR3_Error.GR3_ERROR_EXPORT`: Raises GR3_Exception

        +-------------------------+-------------------------------------------------------+
        | GR3_ERROR_NONE          | on success                                            |
        +-------------------------+-------------------------------------------------------+
        | GR3_ERROR_INVALID_VALUE | if one (or more) of the arguments is out of its range |
        +-------------------------+-------------------------------------------------------+


    .. note::

        The ratio between zFar and zNear influences the precision of the depth buffer, the greater `$ \\\\frac{zFar}{zNear} $`, the more likely are errors. So you should try to keep both values as close to each other as possible while making sure everything you want to be visible, is visible.

    """
    _gr3.gr3_setcameraprojectionparameters(c_float(vertical_field_of_view), c_float(zNear), c_float(zFar))

def setlightdirection(*xyz):
    """
    This function sets the direction of light. If it is called with (0, 0, 0), the light is always pointing into the same direction as the camera.

    **Parameters:**

        `x` : The x-component of the light's direction

        `y` : The y-component of the light's direction

        `z` : The z-component of the light's direction

    """
    if len(xyz) == 3:
        x, y, z = xyz
        _gr3.gr3_setlightdirection(c_float(x), c_float(y), c_float(z))
    elif len(xyz) == 1:
        if xyz[0] is None:
            _gr3.gr3_setlightdirection(c_float(0), c_float(0), c_float(0))
        else:
            raise TypeError("if gr3_setlightdirection() is called with 1 argument, it should be None")
    else:
        raise TypeError("gr3_setlightdirection() takes exactly 1 or exactly 3 arguments (%d given)" %len(xyz))


def PyBuffer_FromMemory(address, length):
    return buffer((c_char * length).from_address(address))


def triangulate(grid, step, offset, isolevel, slices = None):
    data = grid.ctypes.data_as(POINTER(c_ushort))
    isolevel = c_ushort(isolevel)
    if slices is None:
        dim_x, dim_y, dim_z = map(c_uint, grid.shape)
        stride_x, stride_y, stride_z = (0, 0, 0)
    else:
        stride_x = grid.shape[2]*grid.shape[1]
        stride_y = grid.shape[2]
        stride_z = 1
        for i, slice in enumerate(slices):
            if slice[0] >= slice[1]:
                raise ValueError("first slice value must be less than the second value (axis=%d)" % i)
            if slice[0] < 0:
                raise ValueError("slice values must be positive (axis=%d)" % i)
            if slice[1] > grid.shape[i]:
                raise ValueError("second slice value must be at most as large as the grid dimension (axis=%d)" % i)
        dim_x, dim_y, dim_z = [c_uint(slice[1]-slice[0]) for slice in slices]
        data_offset = 2*(stride_x*slices[0][0] + stride_y*slices[1][0] + stride_z*slices[2][0])
        data_address = cast(byref(data), POINTER(c_ulong)).contents.value
        data_address += data_offset
        data = cast(POINTER(c_ulong)(c_ulong(data_address)), POINTER(POINTER(c_ushort))).contents
        offset = [offset[i] + slices[i][0]*step[i] for i in range(3)]
    stride_x = c_uint(stride_x)
    stride_y = c_uint(stride_y)
    stride_z = c_uint(stride_z)
    step_x, step_y, step_z = map(c_double, step)
    offset_x, offset_y, offset_z = map(c_double, offset)
    triangles_p = POINTER(c_float)()
    num_triangles = _gr3.gr3_triangulate(data, isolevel,
                                         dim_x, dim_y, dim_z,
                                         stride_x, stride_y, stride_z,
                                         step_x, step_y, step_z,
                                         offset_x, offset_y, offset_z,
                                         byref(triangles_p))
    if _impl != 'PyPy':
        buffer_from_memory = pythonapi.PyBuffer_FromMemory
    else:
        buffer_from_memory = PyBuffer_FromMemory
    buffer_from_memory.restype = py_object
    buffer = buffer_from_memory(triangles_p, 4*3*3*2*num_triangles)
    triangles = numpy.frombuffer(buffer, numpy.float32).copy()
    _gr3.gr3_free(triangles_p)
    triangles.shape = (num_triangles, 2, 3, 3)
    vertices = triangles[:, 0, :, :]
    normals = triangles[:, 1, :, :]
    return vertices, normals

def createisosurfacemesh(grid, step=None, offset=None, isolevel=None):
    """
    This function creates an isosurface from voxel data using the
    marching cubes algorithm.
    Returns a mesh.

    **Parameters:**

        `grid` : 3D numpy array containing the voxel data

        `step` : voxel sizes in each direction

        `offset` : coordinate origin in each direction

        `isolevel` : isovalue at which the surface will be created

    **Raises:**

    `gr3.GR3_Error.GR3_ERROR_EXPORT`: Raises GR3_Exception

        +----------------------+-------------------------------+
        | GR3_ERROR_NONE       | on success                    |
        +----------------------+-------------------------------+
        | GR3_ERROR_OPENGL_ERR | if an OpenGL error occured    |
        +----------------------+-------------------------------+
        | GR3_ERROR_OUT_OF_MEM | if a memory allocation failed |
        +----------------------+-------------------------------+
    """
    try:
        # integral values
        input_max = numpy.iinfo(grid.dtype).max
    except ValueError:
        # floating point values are expected to be in range [0, 1]
        input_max = 1
        grid[grid > 1] = 1
    if isolevel is None:
        isolevel = 0.5 * input_max
    elif isolevel < 0:
        isolevel = 0
    elif isolevel > input_max:
        isolevel = input_max
    scaling_factor = 1.0 * numpy.iinfo(numpy.uint16).max / input_max
    isolevel = numpy.uint16(isolevel * scaling_factor)
    grid = (grid * scaling_factor).astype(numpy.uint16)
    nx, ny, nz = grid.shape
    if step is None and offset is None:
        step = (2.0/(nx-1), 2.0/(ny-1), 2.0/(nz-1))
        offset = (-1.0, -1.0, -1.0)
    elif offset is None:
        offset = (-step[0] * (nx-1) / 2.0,
                  -step[1] * (ny-1) / 2.0,
                  -step[2] * (nz-1) / 2.0)
    elif step is None:
        step = (-offset[0] * 2.0 / (nx-1),
                -offset[1] * 2.0 / (ny-1),
                -offset[2] * 2.0 / (nz-1))
    _mesh = c_uint(0)
    data = grid.ctypes.data_as(POINTER(c_ushort))
    isolevel = c_ushort(isolevel)
    dim_x, dim_y, dim_z = map(c_uint, grid.shape)
    stride_x, stride_y, stride_z = [c_uint(stride // grid.itemsize) for stride in grid.strides]
    step_x, step_y, step_z = map(c_double, step)
    offset_x, offset_y, offset_z = map(c_double, offset)
    _gr3.gr3_createisosurfacemesh(byref(_mesh), data, isolevel,
                                  dim_x, dim_y, dim_z,
                                  stride_x, stride_y, stride_z,
                                  step_x, step_y, step_z,
                                  offset_x, offset_y, offset_z)

    return _mesh


def drawisosurfacemesh(grid, step=None, offset=None, isovalue=None, position=(0, 0, 0), direction=(0, 0, 1), up=(0, 1, 0), color=(1, 1, 1), scale=(1, 1, 1)):
    """
    This function creates and draws an isosurface from voxel data using the
    marching cubes algorithm.
    """
    mesh = createisosurfacemesh(grid, step, offset, isovalue)
    drawmesh(mesh, 1, position, direction, up, color, scale)
    deletemesh(mesh)

def createsurfacemesh(nx, ny, px, py, pz, option=0):
    """
    Create a mesh of a surface plot similar to gr_surface.
    Uses the current colormap. To apply changes of the colormap
    a new mesh has to be created.

    **Parameters:**

        `mesh` :    the mesh handle

        `nx` :      number of points in x-direction

        `ny` :      number of points in y-direction

        `px` :      an array containing the x-coordinates

        `py` :      an array containing the y-coordinates

        `pz` :      an array of length nx * ny containing the z-coordinates

        `option` : option for the surface mesh; the GR3_SURFACE constants can be combined with bitwise or. See the table below.

    +-------------------------+----+----------------------------------------------------------------------------------------+
    | GR3_SURFACE_DEFAULT     |  0 | default behavior                                                                       |
    +-------------------------+----+----------------------------------------------------------------------------------------+
    | GR3_SURFACE_NORMALS     |  1 | interpolate the vertex normals from the gradient                                       |
    +-------------------------+----+----------------------------------------------------------------------------------------+
    | GR3_SURFACE_FLAT        |  2 | set all z-coordinates to zero                                                          |
    +-------------------------+----+----------------------------------------------------------------------------------------+
    | GR3_SURFACE_GRTRANSFORM |  4 | use gr_inqwindow, gr_inqspace and gr_inqscale to transform the data to NDC coordinates |
    +-------------------------+----+----------------------------------------------------------------------------------------+
    | GR3_SURFACE_GRCOLOR     |  8 | color the surface according to the current gr colormap                                 |
    +-------------------------+----+----------------------------------------------------------------------------------------+
    | GR3_SURFACE_GRZSHADED   | 16 | like GR3_SURFACE_GRCOLOR, but use the z-value directly as color index                  |
    +-------------------------+----+----------------------------------------------------------------------------------------+
    """
    _mesh = c_uint(0)
    px = floatarray(px, copy=False)
    py = floatarray(py, copy=False)
    pz = floatarray(pz, copy=False)
    _gr3.gr3_createsurfacemesh(byref(_mesh), c_int(nx), c_int(ny),
                               px.data,
                               py.data,
                               pz.data,
                               c_int(option))


    return _mesh


def drawtubemesh(n, points, colors, radii, num_steps=10, num_segments=20):
    """
    Draw a tube following a path given by a list of points. The colors and
    radii arrays specify the color and radius at each point.

    **Parameters:**

        `n` :            the number of points given

        `points` :       the points the tube should go through

        `colors` :       the color at each point

        `radii` :        the desired tube radius at each point

        `num_steps` :    the number of steps between each point, allowing for a more smooth tube

        `num_segments` : the number of segments each ring of the tube consists of, e.g. 3 would yield a triangular tube
    """
    points = floatarray(points)
    colors = floatarray(colors)
    radii = floatarray(radii)
    _gr3.gr3_drawtubemesh(c_uint(n), points.data, colors.data, radii.data, c_int(num_steps), c_int(num_segments))



def createtubemesh(n, points, colors, radii, num_steps=10, num_segments=20):
    """
    Create a mesh object in the shape of a tube following a path given by a
    list of points. The colors and radii arrays specify the color and radius at
    each point.

    **Parameters:**

        `n` :            the number of points given

        `points` :       the points the tube should go through

        `colors` :       the color at each point

        `radii` :        the desired tube radius at each point

        `num_steps` :    the number of steps between each point, allowing for a more smooth tube

        `num_segments` : the number of segments each ring of the tube consists of, e.g. 3 would yield a triangular tube
    """
    _mesh = c_uint(0)
    points = floatarray(points)
    colors = floatarray(colors)
    radii = floatarray(radii)
    _gr3.gr3_createtubemesh(byref(_mesh), c_uint(n), points.data, colors.data, radii.data, c_int(num_steps), c_int(num_segments))

    return _mesh


def drawmesh_grlike(mesh, n, positions, directions, ups, colors, scales):
    """
    Draw a mesh with the projection of gr. It uses the current
    projection parameters (rotation, tilt) of gr.
    This function alters the projection type, the projection parameters,
    the viewmatrix and the light direction. If necessary, the user has to
    save them before the call to this function and restore them after
    the call to gr3_drawimage.

    **Parameters:**

        `mesh` :       the mesh to be drawn

        `n` :          the number of meshes to be drawn

        `positions` :  the positions where the meshes should be drawn

        `directions` : the forward directions the meshes should be facing at

        `ups` :        the up directions

        `colors` :     the colors the meshes should be drawn in, it will be multiplied with each vertex color

        `scales` :     the scaling factors
    """
    positions = floatarray(positions)
    directions = floatarray(directions)
    ups = floatarray(ups)
    colors = floatarray(colors)
    scales = floatarray(scales)
    _gr3.gr3_drawmesh_grlike(mesh, c_uint(n),
                             positions.data,
                             directions.data,
                             ups.data,
                             colors.data,
                             scales.data)


def drawsurface(mesh):
    """
    Convenience function for drawing a surfacemesh

    **Parameters:**

        `mesh` : the mesh to be drawn
    """
    _gr3.gr3_drawsurface(mesh)


def surface(px, py, pz, option=0):
    """
    Create a surface plot with gr3 and draw it with gks as cellarray

    **Parameters:**

        `nx` :     number of points in x-direction

        `ny` :     number of points in y-direction

        `px` :     an array containing the x-coordinates

        `py` :     an array containing the y-coordinates

        `pz` :     an array of length nx * ny containing the z-coordinates

        `option` : see the option parameter of gr_surface. OPTION_COLORED_MESH and OPTION_Z_SHADED_MESH are supported.
    """
    if option in (gr.OPTION_Z_SHADED_MESH, gr.OPTION_COLORED_MESH):
        nx = len(px)
        ny = len(py)
        px = floatarray(px, copy=False)
        py = floatarray(py, copy=False)
        pz = floatarray(pz, copy=False)
        _gr3.gr3_surface(c_int(nx), c_int(ny),
                         px.data,
                         py.data,
                         pz.data,
                         c_int(option))
    else:
        gr.surface(px, py, pz, option)
    return


def drawspins(positions, directions, colors=None,
              cone_radius=0.4, cylinder_radius=0.2,
              cone_height=1.0, cylinder_height=1.0):
    positions = numpy.array(positions, numpy.float32)
    positions.shape = (numpy.prod(positions.shape) // 3, 3)
    n = len(positions)
    directions = numpy.array(directions, numpy.float32)
    directions.shape = (n, 3)
    if colors is None:
        colors = numpy.ones((n, 3), numpy.float32)
    else:
        colors = numpy.array(colors, numpy.float32)
        colors.shape = (n, 3)
    _gr3.gr3_drawspins(n, positions.ctypes.data_as(POINTER(c_float)),
                       directions.ctypes.data_as(POINTER(c_float)),
                       colors.ctypes.data_as(POINTER(c_float)), c_float(cone_radius),
                       c_float(cylinder_radius), c_float(cone_height), c_float(cylinder_height))


# Atom color rgb tuples (used for rendering, may be changed by users)
ATOM_COLORS = numpy.array([(0, 0, 0),  # Avoid atomic number to index conversion
                        (255, 255, 255), (217, 255, 255), (204, 128, 255),
                        (194, 255, 0), (255, 181, 181), (144, 144, 144),
                        (48, 80, 248), (255, 13, 13), (144, 224, 80),
                        (179, 227, 245), (171, 92, 242), (138, 255, 0),
                        (191, 166, 166), (240, 200, 160), (255, 128, 0),
                        (255, 255, 48), (31, 240, 31), (128, 209, 227),
                        (143, 64, 212), (61, 225, 0), (230, 230, 230),
                        (191, 194, 199), (166, 166, 171), (138, 153, 199),
                        (156, 122, 199), (224, 102, 51), (240, 144, 160),
                        (80, 208, 80), (200, 128, 51), (125, 128, 176),
                        (194, 143, 143), (102, 143, 143), (189, 128, 227),
                        (225, 161, 0), (166, 41, 41), (92, 184, 209),
                        (112, 46, 176), (0, 255, 0), (148, 255, 255),
                        (148, 224, 224), (115, 194, 201), (84, 181, 181),
                        (59, 158, 158), (36, 143, 143), (10, 125, 140),
                        (0, 105, 133), (192, 192, 192), (255, 217, 143),
                        (166, 117, 115), (102, 128, 128), (158, 99, 181),
                        (212, 122, 0), (148, 0, 148), (66, 158, 176),
                        (87, 23, 143), (0, 201, 0), (112, 212, 255),
                        (255, 255, 199), (217, 225, 199), (199, 225, 199),
                        (163, 225, 199), (143, 225, 199), (97, 225, 199),
                        (69, 225, 199), (48, 225, 199), (31, 225, 199),
                        (0, 225, 156), (0, 230, 117), (0, 212, 82),
                        (0, 191, 56), (0, 171, 36), (77, 194, 255),
                        (77, 166, 255), (33, 148, 214), (38, 125, 171),
                        (38, 102, 150), (23, 84, 135), (208, 208, 224),
                        (255, 209, 35), (184, 184, 208), (166, 84, 77),
                        (87, 89, 97), (158, 79, 181), (171, 92, 0),
                        (117, 79, 69), (66, 130, 150), (66, 0, 102),
                        (0, 125, 0), (112, 171, 250), (0, 186, 255),
                        (0, 161, 255), (0, 143, 255), (0, 128, 255),
                        (0, 107, 255), (84, 92, 242), (120, 92, 227),
                        (138, 79, 227), (161, 54, 212), (179, 31, 212),
                        (179, 31, 186), (179, 13, 166), (189, 13, 135),
                        (199, 0, 102), (204, 0, 89), (209, 0, 79),
                        (217, 0, 69), (224, 0, 56), (230, 0, 46),
                        (235, 0, 38), (255, 0, 255), (255, 0, 255),
                        (255, 0, 255), (255, 0, 255), (255, 0, 255),
                        (255, 0, 255), (255, 0, 255), (255, 0, 255),
                        (255, 0, 255)], dtype=numpy.float32)/255.0

# Atom numbers mapped to their symbols
ATOM_NUMBERS = {"H": 1, "HE": 2, "LI": 3, "BE": 4, "B": 5, "C": 6, "N": 7,
                "O": 8, "F": 9, "NE": 10, "NA": 11, "MG": 12, "AL": 13,
                "SI": 14, "P": 15, "S": 16, "CL": 17, "AR": 18, "K": 19,
                "CA": 20, "SC": 21, "TI": 22, "V": 23, "CR": 24, "MN": 25,
                "FE": 26, "CO": 27, "NI": 28, "CU": 29, "ZN": 30, "GA": 31,
                "GE": 32, "AS": 33, "SE": 34, "BR": 35, "KR": 36, "RB": 37,
                "SR": 38, "Y": 39, "ZR": 40, "NB": 41, "MO": 42, "TC": 43,
                "RU": 44, "RH": 45, "PD": 46, "AG": 47, "CD": 48, "IN": 49,
                "SN": 50, "SB": 51, "TE": 52, "I": 53, "XE": 54, "CS": 55,
                "BA": 56, "LA": 57, "CE": 58, "PR": 59, "ND": 60, "PM": 61,
                "SM": 62, "EU": 63, "GD": 64, "TB": 65, "DY": 66, "HO": 67,
                "ER": 68, "TM": 69, "YB": 70, "LU": 71, "HF": 72, "TA": 73,
                "W": 74, "RE": 75, "OS": 76, "IR": 77, "PT": 78, "AU": 79,
                "HG": 80, "TL": 81, "PB": 82, "BI": 83, "PO": 84, "AT": 85,
                "RN": 86, "FR": 87, "RA": 88, "AC": 89, "TH": 90, "PA": 91,
                "U": 92, "NP": 93, "PU": 94, "AM": 95, "CM": 96, "BK": 97,
                "CF": 98, "ES": 99, "FM": 100, "MD": 101, "NO": 102,
                "LR": 103, "RF": 104, "DB": 105, "SG": 106, "BH": 107,
                "HS": 108, "MT": 109, "DS": 110, "RG": 111, "CN": 112,
                "UUB": 112, "UUT": 113, "UUQ": 114, "UUP": 115, "UUH": 116,
                "UUS": 117, "UUO": 118}

# Atom valence radii in Å (used for bond calculation)
ATOM_VALENCE_RADII = numpy.array([0,  # Avoid atomic number to index conversion
                               230, 930, 680, 350, 830, 680, 680, 680, 640,
                               1120, 970, 1100, 1350, 1200, 750, 1020, 990,
                               1570, 1330, 990, 1440, 1470, 1330, 1350, 1350,
                               1340, 1330, 1500, 1520, 1450, 1220, 1170, 1210,
                               1220, 1210, 1910, 1470, 1120, 1780, 1560, 1480,
                               1470, 1350, 1400, 1450, 1500, 1590, 1690, 1630,
                               1460, 1460, 1470, 1400, 1980, 1670, 1340, 1870,
                               1830, 1820, 1810, 1800, 1800, 1990, 1790, 1760,
                               1750, 1740, 1730, 1720, 1940, 1720, 1570, 1430,
                               1370, 1350, 1370, 1320, 1500, 1500, 1700, 1550,
                               1540, 1540, 1680, 1700, 2400, 2000, 1900, 1880,
                               1790, 1610, 1580, 1550, 1530, 1510, 1500, 1500,
                               1500, 1500, 1500, 1500, 1500, 1500, 1600, 1600,
                               1600, 1600, 1600, 1600, 1600, 1600, 1600, 1600,
                               1600, 1600, 1600, 1600, 1600, 1600],
                              dtype=numpy.float32)/1000.0


# Atom radii in Å (used for rendering, scaled down by factor 0.4, may be
# changed by users)
ATOM_RADII = numpy.array(ATOM_VALENCE_RADII, copy=True)*0.4


def _readxyzfile(filename):
    with open(filename, 'r') as xyzfile:
        n = None
        has_spins = None
        atoms = []
        skip = 0
        for line in xyzfile.readlines():
            if skip > 0:
                skip -= 1
                continue
            if line.strip().startswith('#'):
                continue
            if n is None:
                n = int(line.strip())
                skip = 1
                continue
            atoms.append(line.strip().split())
            if has_spins is None:
                has_spins = len(atoms[-1]) == 7
            if has_spins:
                if len(atoms[-1]) != 7:
                    raise RuntimeError("line '%s' is invalid for xyz file with spins" % line)
            else:
                if len(atoms[-1]) != 4:
                    raise RuntimeError("line '%s' is invalid for xyz file" % line)
            if len(atoms) >= n:
                break
    positions = []
    colors = []
    radii = []
    if has_spins:
        spins = []
    else:
        spins = None
    for i, atom in enumerate(atoms):
        element = atom[0].upper()
        positions.append(tuple(map(float, atom[1:4])))
        if has_spins:
            spins.append(tuple(map(float, atom[4:7])))
        atom_number = ATOM_NUMBERS[element]
        colors.append(ATOM_COLORS[atom_number])
        radii.append(ATOM_RADII[atom_number])
    return positions, colors, radii, spins


def drawmolecule(positions_or_filename, colors=None, radii=None, spins=None,
                 bond_radius=0.1, bond_color=(0.8, 0.8, 0.8), bond_delta=1.0,
                 set_camera=True, rotation=0, tilt=0):
    if type(positions_or_filename) in _string_types:
        filename = positions_or_filename
        positions, colors, radii, spins = _readxyzfile(filename)
    else:
        positions = positions_or_filename
    positions = numpy.array(positions, numpy.float32)
    positions.shape = (numpy.prod(positions.shape) // 3, 3)
    n = len(positions)
    if colors is None:
        colors = numpy.ones((n, 3), numpy.float32)
    else:
        colors = numpy.array(colors, numpy.float32)
        colors.shape = (n, 3)
    if radii is None:
        radii = 0.3*numpy.ones(n, numpy.float32)
    else:
        radii = numpy.array(radii, numpy.float32)
        radii.shape = n
    bond_color = numpy.array(bond_color, numpy.float32)
    bond_color.shape = 3

    if set_camera:
        cx, cy, cz = numpy.mean(positions, axis=0)
        dx, dy, dz = positions.ptp(axis=0)
        d = max(dx, dy)/2 / 0.4142 + 3
        r = dz/2+d
        rx = r*numpy.sin(numpy.radians(tilt))*numpy.sin(numpy.radians(rotation))
        ry = r*numpy.sin(numpy.radians(tilt))*numpy.cos(numpy.radians(rotation))
        rz = r*numpy.cos(numpy.radians(tilt))
        ux = numpy.sin(numpy.radians(tilt+90))*numpy.sin(numpy.radians(rotation))
        uy = numpy.sin(numpy.radians(tilt+90))*numpy.cos(numpy.radians(rotation))
        uz = numpy.cos(numpy.radians(tilt+90))
        cameralookat(cx+rx, cy+ry, cz+rz, cx, cy, cz, ux, uy, uz)
        setcameraprojectionparameters(45, d-radii.max()-3, d+dz+radii.max()+3)

    _gr3.gr3_drawmolecule(n, positions.ctypes.data_as(POINTER(c_float)),
                          colors.ctypes.data_as(POINTER(c_float)),
                          radii.ctypes.data_as(POINTER(c_float)),
                          c_float(bond_radius),
                          bond_color.ctypes.data_as(POINTER(c_float)),
                          c_float(bond_delta))

    if spins is not None and spins is not False:
        spins = numpy.array(spins, numpy.float32)
        spins.shape = (n, 3)
        drawspins(positions, spins, colors)


def drawxslicemesh(grid, x=0.5, step=None, offset=None, position=(0, 0, 0), direction=(0, 0, 1), up=(0, 1, 0), color=(1, 1, 1), scale=(1, 1, 1)):
    """
    Draw a yz-slice through the given data, using the current GR colormap.

    **Parameters:**

        `grid` :      3D numpy array containing the voxel data

        `x` :         the position of the slice through the yz-plane (0 to 1)

        `step` :      voxel sizes in each direction

        `offset` :    coordinate origin in each direction

        `position` :  the positions where the meshes should be drawn

        `direction` : the forward directions the meshes should be facing at

        `up` :        the up directions

        `color` :     the colors the meshes should be drawn in, it will be multiplied with each vertex color

        `scale` :     the scaling factors
    """
    mesh = createxslicemesh(grid, x, step, offset)
    drawmesh(mesh, 1, position, direction, up, color, scale)
    deletemesh(mesh)


def drawyslicemesh(grid, y=0.5, step=None, offset=None, position=(0, 0, 0), direction=(0, 0, 1), up=(0, 1, 0), color=(1, 1, 1), scale=(1, 1, 1)):
    """
    Draw a xz-slice through the given data, using the current GR colormap.

    **Parameters:**

        `grid` :      3D numpy array containing the voxel data

        `y` :         the position of the slice through the xz-plane (0 to 1)

        `step` :      voxel sizes in each direction

        `offset` :    coordinate origin in each direction

        `position` :  the positions where the meshes should be drawn

        `direction` : the forward directions the meshes should be facing at

        `up` :        the up directions

        `color` :     the colors the meshes should be drawn in, it will be multiplied with each vertex color

        `scale` :     the scaling factors
    """
    mesh = createyslicemesh(grid, y, step, offset)
    drawmesh(mesh, 1, position, direction, up, color, scale)
    deletemesh(mesh)


def drawzslicemesh(grid, z=0.5, step=None, offset=None, position=(0, 0, 0), direction=(0, 0, 1), up=(0, 1, 0), color=(1, 1, 1), scale=(1, 1, 1)):
    """
    Draw a xy-slice through the given data, using the current GR colormap.

    **Parameters:**

        `grid` :      3D numpy array containing the voxel data

        `x` :         the position of the slice through the yz-plane (0 to 1)

        `y` :         the position of the slice through the xz-plane (0 to 1)

        `z` :         the position of the slice through the xy-plane (0 to 1)

        `step` :      voxel sizes in each direction

        `offset` :    coordinate origin in each direction

        `position` :  the positions where the meshes should be drawn

        `direction` : the forward directions the meshes should be facing at

        `up` :        the up directions

        `color` :     the colors the meshes should be drawn in, it will be multiplied with each vertex color

        `scale` :     the scaling factors
    """
    mesh = createzslicemesh(grid, z, step, offset)
    drawmesh(mesh, 1, position, direction, up, color, scale)
    deletemesh(mesh)


def createxslicemeshes(grid, x=0.5, step=None, offset=None):
    """
    Creates a meshes for a slices through the yz-plane of the given data,
    using the current GR colormap. Use the x parameter to set the position of
    the yz-slice.
    Returns a mesh for the yz-slice.

    **Parameters:**

        `grid` :      3D numpy array containing the voxel data

        `x` :         the position of the slice through the yz-plane (0 to 1)

        `step` :      voxel sizes in each direction

        `offset` :    coordinate origin in each direction
    """
    return createslicemeshes(grid, step, offset, x=x)[0]


def createyslicemeshes(grid, y=0.5, step=None, offset=None):
    """
    Creates a meshes for a slices through the xz-plane of the given data,
    using the current GR colormap. Use the y parameter to set the position of
    the xz-slice.
    Returns a mesh for the xz-slice.

    **Parameters:**

        `grid` :      3D numpy array containing the voxel data

        `y` :         the position of the slice through the xz-plane (0 to 1)

        `step` :      voxel sizes in each direction

        `offset` :    coordinate origin in each direction
    """
    return createslicemeshes(grid, step, offset, y=y)[1]


def createzslicemeshes(grid, z=0.5, step=None, offset=None):
    """
    Creates a meshes for a slices through the xy-plane of the given data,
    using the current GR colormap. Use the z parameter to set the position of
    the xy-slice.
    Returns a mesh for the xy-slice.

    **Parameters:**

        `grid` :      3D numpy array containing the voxel data

        `z` :         the position of the slice through the xy-plane (0 to 1)

        `step` :      voxel sizes in each direction

        `offset` :    coordinate origin in each direction
    """
    return createslicemeshes(grid, step, offset, z=z)[2]


def drawslicemeshes(grid, x=None, y=None, z=None, step=None, offset=None, position=(0, 0, 0), direction=(0, 0, 1), up=(0, 1, 0), color=(1, 1, 1), scale=(1, 1, 1)):
    """
    Draw slices through the given data, using the current GR colormap.
    Use the parameters x, y or z to specify what slices should be drawn and at
    which positions they should go through the data. If neither x nor y nor
    z are set, 0.5 will be used for all three.

    **Parameters:**

        `grid` :      3D numpy array containing the voxel data

        `x` :         the position of the slice through the yz-plane (0 to 1)

        `y` :         the position of the slice through the xz-plane (0 to 1)

        `z` :         the position of the slice through the xy-plane (0 to 1)

        `step` :      voxel sizes in each direction

        `offset` :    coordinate origin in each direction

        `position` :  the positions where the meshes should be drawn

        `direction` : the forward directions the meshes should be facing at

        `up` :        the up directions

        `color` :     the colors the meshes should be drawn in, it will be multiplied with each vertex color

        `scale` :     the scaling factors
    """
    meshes = createslicemeshes(grid, x, y, z, step, offset)
    for mesh in meshes:
        if mesh is not None:
            drawmesh(mesh, 1, position, direction, up, color, scale)
            deletemesh(mesh)


def createslicemeshes(grid, x=None, y=None, z=None, step=None, offset=None):
    """
    Creates meshes for slices through the given data, using the current GR
    colormap. Use the parameters x, y or z to specify what slices should be
    drawn and at which positions they should go through the data. If neither
    x nor y nor z are set, 0.5 will be used for all three.
    Returns meshes for the yz-slice, the xz-slice and the xy-slice.

    **Parameters:**

        `grid` :      3D numpy array containing the voxel data

        `x` :         the position of the slice through the yz-plane (0 to 1)

        `y` :         the position of the slice through the xz-plane (0 to 1)

        `z` :         the position of the slice through the xy-plane (0 to 1)

        `step` :      voxel sizes in each direction

        `offset` :    coordinate origin in each direction
    """
    if x is None and y is None and z is None:
        x = 0.5
        y = 0.5
        z = 0.5
    try:
        # integral values
        input_max = numpy.iinfo(grid.dtype).max
    except ValueError:
        # floating point values are expected to be in range [0, 1]
        input_max = 1
        grid[grid > 1] = 1
    scaling_factor = 1.0 * numpy.iinfo(numpy.uint16).max / input_max
    grid = (grid * scaling_factor).astype(numpy.uint16)
    nx, ny, nz = grid.shape
    if step is None and offset is None:
        step = (2.0/(nx-1), 2.0/(ny-1), 2.0/(nz-1))
        offset = (-1.0, -1.0, -1.0)
    elif offset is None:
        offset = (-step[0] * (nx-1) / 2.0,
                  -step[1] * (ny-1) / 2.0,
                  -step[2] * (nz-1) / 2.0)
    elif step is None:
        step = (-offset[0] * 2.0 / (nx-1),
                -offset[1] * 2.0 / (ny-1),
                -offset[2] * 2.0 / (nz-1))
    data = grid.ctypes.data_as(POINTER(c_ushort))
    dim_x, dim_y, dim_z = map(c_uint, grid.shape)
    stride_x, stride_y, stride_z = [c_uint(stride // grid.itemsize) for stride in grid.strides]
    step_x, step_y, step_z = map(c_double, step)
    offset_x, offset_y, offset_z = map(c_double, offset)
    if x is not None:
        if x > 1:
            x = 1
        if x < 0:
            x = 0
        x = c_uint(int(x*dim_x.value))
        _mesh_x = c_uint(0)
        _gr3.gr3_createxslicemesh(byref(_mesh_x), data, x,
                                  dim_x, dim_y, dim_z,
                                  stride_x, stride_y, stride_z,
                                  step_x, step_y, step_z,
                                  offset_x, offset_y, offset_z)
    else:
        _mesh_x = None
    if y is not None:
        if z > 1:
            z = 1
        if z < 0:
            z = 0
        y = c_uint(int(y*dim_y.value))
        _mesh_y = c_uint(0)
        _gr3.gr3_createyslicemesh(byref(_mesh_y), data, y,
                                  dim_x, dim_y, dim_z,
                                  stride_x, stride_y, stride_z,
                                  step_x, step_y, step_z,
                                  offset_x, offset_y, offset_z)
    else:
        _mesh_y = None
    if z is not None:
        if z > 1:
            z = 1
        if z < 0:
            z = 0
        z = c_uint(int(z*dim_z.value))
        _mesh_z = c_uint(0)
        _gr3.gr3_createzslicemesh(byref(_mesh_z), data, z,
                                  dim_x, dim_y, dim_z,
                                  stride_x, stride_y, stride_z,
                                  step_x, step_y, step_z,
                                  offset_x, offset_y, offset_z)
    else:
        _mesh_z = None
    return _mesh_x, _mesh_y, _mesh_z


def drawtrianglesurface(vertices):
    """
    Renders a triangle mesh using the current GR colormap and projection.

    **Parameters:**

        `vertices` : the vertices of the triangle mesh
    """
    global _gr3
    vertices = numpy.array(vertices, copy=False)
    assert len(vertices.shape) <= 3
    if len(vertices.shape) == 3:
        assert vertices.shape[1:3] == (3, 3)
        n = len(vertices)
    elif len(vertices.shape) == 2:
        assert vertices.shape[1:] == (3,)
        n = len(vertices) // 3
    else:
        n = len(vertices) // 3 // 3
    vertices.shape = (n, 3, 3)
    vertices = numpy.array(vertices, numpy.float32)
    _gr3.gr3_drawtrianglesurface(n, vertices.ctypes.data_as(POINTER(c_float)))


_gr3.gr3_init.argtypes = [POINTER(c_int)]
_gr3.gr3_terminate.argtypes = []
_gr3.gr3_useframebuffer.argtypes = [c_uint]
_gr3.gr3_usecurrentframebuffer.argtypes = []
_gr3.gr3_getrenderpathstring.argtypes = []
_gr3.gr3_geterrorstring.argtypes = [c_int]
_gr3.gr3_setlogcallback.argtypes = [CFUNCTYPE(None, c_char_p)]
_gr3.gr3_clear.argtypes = []
_gr3.gr3_setquality.argtypes = [c_int]
_gr3.gr3_getimage.argtypes = [c_int, c_int, c_int, POINTER(c_ubyte)]
_gr3.gr3_export.argtypes = [POINTER(c_char), c_uint, c_uint]
_gr3.gr3_drawimage.argtypes = [c_float, c_float, c_float, c_float,
                               c_int, c_int, c_int]
_gr3.gr3_createmesh.argtypes = [POINTER(c_uint), c_uint, POINTER(c_float),
                                POINTER(c_float), POINTER(c_float)]
_gr3.gr3_createindexedmesh.restype = c_int
_gr3.gr3_createindexedmesh.argtypes = [POINTER(c_uint), c_uint,
                                       POINTER(c_float), POINTER(c_float),
                                       POINTER(c_float), c_uint, POINTER(c_int)]
_gr3.gr3_createheightmapmesh.argtypes = [POINTER(c_float), c_int, c_int]
_gr3.gr3_drawheightmap.argtypes = [POINTER(c_float), c_int, c_int,
                                   POINTER(c_float), POINTER(c_float)]
_gr3.gr3_drawmesh.argtypes = [c_uint, c_uint, POINTER(c_float),
                              POINTER(c_float), POINTER(c_float),
                              POINTER(c_float), POINTER(c_float)]
_gr3.gr3_deletemesh.argtypes = [c_int]

_gr3.gr3_cameralookat.argtypes = [c_float, c_float, c_float,
                                  c_float, c_float, c_float,
                                  c_float, c_float, c_float]
_gr3.gr3_setcameraprojectionparameters.argtypes = [c_float, c_float,
                                                   c_float]
_gr3.gr3_setlightdirection.argtypes = [c_float, c_float, c_float]
_gr3.gr3_setbackgroundcolor.argtypes = [c_float, c_float, c_float,
                                        c_float]
_gr3.gr3_drawconemesh.argtypes = [c_uint, POINTER(c_float),
                                  POINTER(c_float), POINTER(c_float),
                                  POINTER(c_float), POINTER(c_float)]
_gr3.gr3_drawcylindermesh.argtypes = [c_uint, POINTER(c_float),
                                      POINTER(c_float), POINTER(c_float),
                                      POINTER(c_float), POINTER(c_float)]
_gr3.gr3_drawspheremesh.argtypes = [c_uint, POINTER(c_float),
                                    POINTER(c_float), POINTER(c_float)]
_gr3.gr3_drawcubemesh.argtypes = [c_uint, POINTER(c_float),
                                  POINTER(c_float), POINTER(c_float),
                                  POINTER(c_float), POINTER(c_float)]
_gr3.gr3_triangulate.restype = c_uint
_gr3.gr3_triangulate.argtypes = [POINTER(c_ushort), c_ushort, c_uint,
                                 c_uint, c_uint, c_uint, c_uint, c_uint,
                                 c_double, c_double, c_double, c_double,
                                 c_double, c_double,
                                 POINTER(POINTER(c_float))]

_gr3.gr3_createisosurfacemesh.restype = c_int
_gr3.gr3_createisosurfacemesh.argtypes = [POINTER(c_uint),
                                          POINTER(c_ushort), c_ushort,
                                          c_uint, c_uint, c_uint,
                                          c_uint, c_uint, c_uint,
                                          c_double, c_double, c_double,
                                          c_double, c_double, c_double]

_gr3.gr3_createsurfacemesh.argtypes = [POINTER(c_uint), c_int, c_int,
                                       POINTER(c_float), POINTER(c_float),
                                       POINTER(c_float), c_int]
_gr3.gr3_createsurfacemesh.restype = c_int
_gr3.gr3_drawmesh_grlike.argtypes = [c_uint, c_uint, POINTER(c_float),
                                     POINTER(c_float), POINTER(c_float),
                                     POINTER(c_float), POINTER(c_float)]
_gr3.gr3_drawmesh_grlike.restype = None
_gr3.gr3_drawsurface.argtypes = [c_uint]
_gr3.gr3_drawsurface.restype = None
_gr3.gr3_surface.argtype = [c_int, c_int, POINTER(c_float),
                            POINTER(c_float), POINTER(c_float), c_int]
_gr3.gr3_surface.restype =  None

_gr3.gr3_drawtubemesh.argtype = [c_uint, POINTER(c_float), POINTER(c_float),
                                 POINTER(c_float), c_int, c_int]
_gr3.gr3_drawtubemesh.restype = c_int

_gr3.gr3_createtubemesh.argtype = [POINTER(c_uint), c_uint, POINTER(c_float),
                                   POINTER(c_float), POINTER(c_float), c_int,
                                   c_int]
_gr3.gr3_createtubemesh.restype = c_int

_gr3.gr3_drawspins.argtype = [c_int, POINTER(c_float), POINTER(c_float),
                              POINTER(c_float), c_float, c_float, c_float,
                              c_float]
_gr3.gr3_drawspins.restype = None

_gr3.gr3_drawmolecule.argtype = [c_int, POINTER(c_float), POINTER(c_float),
                                 POINTER(c_float), c_float, POINTER(c_float),
                                 c_float]
_gr3.gr3_drawmolecule.restype = None

_gr3.gr3_geterror.argtype = [c_int, POINTER(c_int), POINTER(c_char_p)]
_gr3.gr3_geterror.restype = c_int

_gr3.gr3_createxslicemesh.restype = None
_gr3.gr3_createxslicemesh.argtypes = [POINTER(c_uint),
                                      POINTER(c_ushort), c_uint,
                                      c_uint, c_uint, c_uint,
                                      c_uint, c_uint, c_uint,
                                      c_double, c_double, c_double,
                                      c_double, c_double, c_double]

_gr3.gr3_createyslicemesh.restype = None
_gr3.gr3_createyslicemesh.argtypes = [POINTER(c_uint),
                                      POINTER(c_ushort), c_uint,
                                      c_uint, c_uint, c_uint,
                                      c_uint, c_uint, c_uint,
                                      c_double, c_double, c_double,
                                      c_double, c_double, c_double]

_gr3.gr3_createzslicemesh.restype = None
_gr3.gr3_createzslicemesh.argtypes = [POINTER(c_uint),
                                      POINTER(c_ushort), c_uint,
                                      c_uint, c_uint, c_uint,
                                      c_uint, c_uint, c_uint,
                                      c_double, c_double, c_double,
                                      c_double, c_double, c_double]

_gr3.gr3_drawxslicemesh.restype = None
_gr3.gr3_drawxslicemesh.argtypes = [POINTER(c_ushort), c_uint,
                                    c_uint, c_uint, c_uint,
                                    c_uint, c_uint, c_uint,
                                    c_double, c_double, c_double,
                                    c_double, c_double, c_double]

_gr3.gr3_drawyslicemesh.restype = None
_gr3.gr3_drawyslicemesh.argtypes = [POINTER(c_ushort), c_uint,
                                    c_uint, c_uint, c_uint,
                                    c_uint, c_uint, c_uint,
                                    c_double, c_double, c_double,
                                    c_double, c_double, c_double]

_gr3.gr3_drawzslicemesh.restype = None
_gr3.gr3_drawzslicemesh.argtypes = [POINTER(c_ushort), c_uint,
                                    c_uint, c_uint, c_uint,
                                    c_uint, c_uint, c_uint,
                                    c_double, c_double, c_double,
                                    c_double, c_double, c_double]

_gr3.gr3_drawtrianglesurface.restype = None
_gr3.gr3_drawtrianglesurface.argtypes = [c_int, POINTER(c_float)]


for symbol in dir(_gr3):
    if symbol.startswith('gr3_') and symbol != 'gr3_geterror':
        getattr(_gr3, symbol).errcheck = _error_check
