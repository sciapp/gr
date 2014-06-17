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
           'createisosurfacemesh']


import sys
if any([module.startswith('OpenGL') for module in sys.modules]):
    import warnings
    warnings.warn("Importing gr3 after importing pyOpenGL (or any of its modules) might cause problems on some platforms. Please import gr3 first to avoid this.")

import ctypes
import ctypes.util
import numpy
import os

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
_gr3 = ctypes.CDLL(_gr3Lib)
 
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
    """
    **Parameters:**

    `attrib_list` :
        a list that can specify details about context creation. The attributes which use unsigned integer values are followed by these values.

    **Raise:**

    `gr3.GR3_Error.GR3_ERROR_EXPORT`: Raises GR3_Exception

        +-----------------------------+-----------------------------------------------------------------------------------------------+
        | GR3_ERROR_NONE              | on succsess                                                                                   | 
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
    """
    This function terminates the gr3 context. After calling this function, gr3 is in the same state as when it was first loaded, except for context-independent variables, i.e. the logging callback.
    
    .. note:: It is safe to call this function even if the context is not initialized.
    
    .. seealso:: Functions :py:func:`gr.init()`
    """
    global _gr3
    _gr3.gr3_terminate()

def setquality(quality):
    """
    Set rendering quality

    **Parameters:**
        `quality`:  
            The quality to set 
    """
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
    _gr3.gr3_getrenderpathstring.restype = ctypes.c_char_p
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
    LOGCALLBACK = ctypes.CFUNCTYPE(None, ctypes.c_char_p)
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
    _mesh = ctypes.c_uint(0)
    vertices = numpy.array(vertices, ctypes.c_float)
    normals = numpy.array(normals, ctypes.c_float)
    colors = numpy.array(colors, ctypes.c_float)
    err = _gr3.gr3_createmesh(ctypes.byref(_mesh),ctypes.c_uint(n),vertices.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), normals.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), colors.ctypes.data_as(ctypes.POINTER(ctypes.c_float)))
    if err:
        raise GR3_Exception(err)
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
    _mesh = ctypes.c_uint(0)
    vertices = numpy.array(vertices, ctypes.c_float)
    normals = numpy.array(normals, ctypes.c_float)
    colors = numpy.array(colors, ctypes.c_float)
    indices = numpy.array(indices, ctypes.c_int)
    err = _gr3.gr3_createindexedmesh(ctypes.byref(_mesh),ctypes.c_uint(num_vertices),vertices.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), normals.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), colors.ctypes.data_as(ctypes.POINTER(ctypes.c_float)),ctypes.c_uint(num_indices),indices.ctypes.data_as(ctypes.POINTER(ctypes.c_int)))
    if err:
        raise GR3_Exception(err)
    return _mesh

def createheightmapmesh(heightmap, num_columns, num_rows):
    heightmap = numpy.array(heightmap, ctypes.c_float)
    return _gr3.gr3_createheightmapmesh(heightmap.ctypes.data_as(ctypes.POINTER(ctypes.c_float)),ctypes.c_int(num_columns),ctypes.c_int(num_rows))

def drawheightmap(heightmap, num_columns, num_rows, positions, scales):
    heightmap = numpy.array(heightmap, ctypes.c_float)
    positions = numpy.array(positions, ctypes.c_float)
    scales = numpy.array(scales, ctypes.c_float)
    _gr3.gr3_drawheightmap(heightmap.ctypes.data_as(ctypes.POINTER(ctypes.c_float)),ctypes.c_int(num_columns),ctypes.c_int(num_rows),positions.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), scales.ctypes.data_as(ctypes.POINTER(ctypes.c_float)))
    


def drawcylindermesh(n, positions, directions, colors, radii, lengths):
    """
    This function allows drawing a cylinder without requiring a mesh.

    .. seealso::

        Function :py:func:`gr.drawmesh()` 
    """
    positions = numpy.array(positions, ctypes.c_float)
    directions = numpy.array(directions, ctypes.c_float)
    colors = numpy.array(colors, ctypes.c_float)
    radii = numpy.array(radii, ctypes.c_float)
    lengths = numpy.array(lengths, ctypes.c_float)
    _gr3.gr3_drawcylindermesh(ctypes.c_uint(n), positions.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), directions.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), colors.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), radii.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), lengths.ctypes.data_as(ctypes.POINTER(ctypes.c_float)))

def drawconemesh(n, positions, directions, colors, radii, lengths):
    """
    This function allows drawing a cylinder without requiring a mesh.

    .. seealso::

        Function :py:func:`gr.drawmesh()` 
    """
    positions = numpy.array(positions, ctypes.c_float)
    directions = numpy.array(directions, ctypes.c_float)
    colors = numpy.array(colors, ctypes.c_float)
    radii = numpy.array(radii, ctypes.c_float)
    lengths = numpy.array(lengths, ctypes.c_float)
    _gr3.gr3_drawconemesh(ctypes.c_uint(n), positions.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), directions.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), colors.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), radii.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), lengths.ctypes.data_as(ctypes.POINTER(ctypes.c_float)))

def drawspheremesh(n, positions,colors, radii):
    """
    This function allows drawing a sphere without requiring a mesh.

    .. seealso::

        Function :py:func:`gr.drawmesh()` 
    """
    positions = numpy.array(positions, ctypes.c_float)
    colors = numpy.array(colors, ctypes.c_float)
    radii = numpy.array(radii, ctypes.c_float)
    _gr3.gr3_drawspheremesh(ctypes.c_uint(n), positions.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), colors.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), radii.ctypes.data_as(ctypes.POINTER(ctypes.c_float)))

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
    positions = numpy.array(positions, ctypes.c_float)
    directions = numpy.array(directions, ctypes.c_float)
    ups = numpy.array(ups, ctypes.c_float)
    colors = numpy.array(colors, ctypes.c_float)
    scales = numpy.array(scales, ctypes.c_float)
    _gr3.gr3_drawmesh(mesh,ctypes.c_uint(n),positions.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), directions.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), ups.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), colors.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), scales.ctypes.data_as(ctypes.POINTER(ctypes.c_float)))
    
def drawcubemesh(n, positions, directions, ups, colors, scales):
    positions = numpy.array(positions, ctypes.c_float)
    directions = numpy.array(directions, ctypes.c_float)
    ups = numpy.array(ups, ctypes.c_float)
    colors = numpy.array(colors, ctypes.c_float)
    scales = numpy.array(scales, ctypes.c_float)
    _gr3.gr3_drawcubemesh(ctypes.c_uint(n),positions.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), directions.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), ups.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), colors.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), scales.ctypes.data_as(ctypes.POINTER(ctypes.c_float)))
    
    
def deletemesh(mesh):
    """
    This function marks a mesh for deletion and removes the user's reference from the mesh's referenc counter, so a user must not use the mesh after calling this function. If the mesh is still in use for draw calls, the mesh will not be truly deleted until gr3.clear() is called.
    
    **Parameters:**
    
        `mesh` : The mesh that should be marked for deletion

    """
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
    _gr3.gr3_setbackgroundcolor(ctypes.c_float(red),ctypes.c_float(green),ctypes.c_float(blue),ctypes.c_float(alpha))

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
    _gr3.gr3_cameralookat(ctypes.c_float(camera_x), ctypes.c_float(camera_y), ctypes.c_float(camera_z), ctypes.c_float(center_x), ctypes.c_float(center_y), ctypes.c_float(center_z), ctypes.c_float(up_x), ctypes.c_float(up_y), ctypes.c_float(up_z))

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
    _gr3.gr3_setcameraprojectionparameters(ctypes.c_float(vertical_field_of_view), ctypes.c_float(zNear), ctypes.c_float(zFar))
    
def setlightdirection(*xyz):
    """
    This function sets the direction of light. If it is called with (0, 0, 0), the light is always pointing into the same direction as the camera.

    **Parameters:**

        `x` : The x-component of the light's direction

        `y` : The y-component of the light's direction

        `z` : The z-component of the light's direction

    """
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

def triangulate(grid, step, offset, isolevel, slices = None):
    data = grid.ctypes.data_as(ctypes.POINTER(ctypes.c_ushort))
    isolevel = ctypes.c_ushort(isolevel)
    if slices is None:
        dim_x, dim_y, dim_z = map(ctypes.c_uint, grid.shape)
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
        dim_x, dim_y, dim_z = [ctypes.c_uint(slice[1]-slice[0]) for slice in slices]
        data_offset = 2*(stride_x*slices[0][0] + stride_y*slices[1][0] + stride_z*slices[2][0])
        data_address = cast(ctypes.byref(data), ctypes.POINTER(ctypes.c_ulong)).contents.value
        data_address += data_offset
        data = cast(ctypes.POINTER(ctypes.c_ulong)(ctypes.c_ulong(data_address)), ctypes.POINTER(ctypes.POINTER(ctypes.c_ushort))).contents
        offset = [offset[i] + slices[i][0]*step[i] for i in range(3)]
    stride_x = ctypes.c_uint(stride_x)
    stride_y = ctypes.c_uint(stride_y)
    stride_z = ctypes.c_uint(stride_z)
    step_x, step_y, step_z = map(ctypes.c_double, step)
    offset_x, offset_y, offset_z = map(ctypes.c_double, offset)
    triangles_p = ctypes.POINTER(ctypes.c_float)()
    num_triangles = _gr3.gr3_triangulate(data, isolevel,
                                    dim_x, dim_y, dim_z,
                                    stride_x, stride_y, stride_z,
                                    step_x, step_y, step_z,
                                    offset_x, offset_y, offset_z,
                                    ctypes.byref(triangles_p))
    buffer_from_memory = ctypes.pythonapi.PyBuffer_FromMemory
    buffer_from_memory.restype = ctypes.py_object
    buffer = buffer_from_memory(triangles_p, 4*3*3*2*num_triangles)
    triangles = numpy.frombuffer(buffer, numpy.float32).copy()
    _gr3.gr3_free(triangles_p)
    triangles.shape = (num_triangles, 2, 3, 3)
    vertices = triangles[:,0,:,:]
    normals = triangles[:,1,:,:]
    return vertices, normals

def createisosurfacemesh(grid, step, offset, isolevel):
    """
    This function creates an isosurface from voxel data using the
    marching cubes algorithm.
    Returns a mesh.

    **Parameters:**

        `grid` : 3D numpy array of the voxel data

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
    _mesh = ctypes.c_uint(0)
    data = grid.ctypes.data_as(ctypes.POINTER(ctypes.c_ushort))
    isolevel = ctypes.c_ushort(isolevel)
    dim_x, dim_y, dim_z = map(ctypes.c_uint, grid.shape)
    stride_x, stride_y, stride_z = map(lambda x: ctypes.c_uint(x / grid.itemsize), grid.strides)
    step_x, step_y, step_z = map(ctypes.c_double, step)
    offset_x, offset_y, offset_z = map(ctypes.c_double, offset)
    err = _gr3.gr3_createisosurfacemesh(ctypes.byref(_mesh), data, isolevel,
                                        dim_x, dim_y, dim_z,
                                        stride_x, stride_y, stride_z,
                                        step_x, step_y, step_z,
                                        offset_x, offset_y, offset_z)
    if err:
        raise GR3_Exception(err)
    return _mesh

_gr3.gr3_init.argtypes = [ctypes.POINTER(ctypes.c_int)]
_gr3.gr3_terminate.argtypes = []
_gr3.gr3_getrenderpathstring.argtypes = []
_gr3.gr3_geterrorstring.argtypes = [ctypes.c_int]
_gr3.gr3_setlogcallback.argtypes = [ctypes.CFUNCTYPE(None, ctypes.c_char_p)]
_gr3.gr3_clear.argtypes = []
_gr3.gr3_setquality.argtypes = [ctypes.c_int]
_gr3.gr3_getimage.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.POINTER(ctypes.c_ubyte)]
_gr3.gr3_export.argtypes = [ctypes.POINTER(ctypes.c_char), ctypes.c_uint, ctypes.c_uint]
_gr3.gr3_drawimage.argtypes = [ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_int, ctypes.c_int, ctypes.c_int]
_gr3.gr3_createmesh.argtypes = [ctypes.POINTER(ctypes.c_uint), ctypes.c_uint, ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float)]
_gr3.gr3_createindexedmesh.restype = ctypes.c_int
_gr3.gr3_createindexedmesh.argtypes = [ctypes.POINTER(ctypes.c_uint), ctypes.c_uint, ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float),ctypes.c_uint,ctypes.POINTER(ctypes.c_int)]
_gr3.gr3_createheightmapmesh.argtypes = [ctypes.POINTER(ctypes.c_float), ctypes.c_int, ctypes.c_int]
_gr3.gr3_drawheightmap.argtypes = [ctypes.POINTER(ctypes.c_float), ctypes.c_int, ctypes.c_int, ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float)]
_gr3.gr3_drawmesh.argtypes = [ctypes.c_uint, ctypes.c_uint, ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float)]
_gr3.gr3_deletemesh.argtypes = [ctypes.c_int]

_gr3.gr3_cameralookat.argtypes = [ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float]
_gr3.gr3_setcameraprojectionparameters.argtypes = [ctypes.c_float,ctypes.c_float,ctypes.c_float]
_gr3.gr3_setlightdirection.argtypes = [ctypes.c_float,ctypes.c_float,ctypes.c_float]
_gr3.gr3_setbackgroundcolor.argtypes = [ctypes.c_float,ctypes.c_float,ctypes.c_float,ctypes.c_float]
_gr3.gr3_drawconemesh.argtypes = [ctypes.c_uint, ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float)]
_gr3.gr3_drawcylindermesh.argtypes = [ctypes.c_uint, ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float)]
_gr3.gr3_drawspheremesh.argtypes = [ctypes.c_uint, ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float)]
_gr3.gr3_drawcubemesh.argtypes = [ctypes.c_uint, ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float)]
_gr3.gr3_triangulate.restype = ctypes.c_uint
_gr3.gr3_triangulate.argtypes = (ctypes.POINTER(ctypes.c_ushort), ctypes.c_ushort, ctypes.c_uint, ctypes.c_uint, ctypes.c_uint, ctypes.c_uint, ctypes.c_uint, ctypes.c_uint, ctypes.c_double, ctypes.c_double, ctypes.c_double, ctypes.c_double, ctypes.c_double, ctypes.c_double, ctypes.POINTER(ctypes.POINTER(ctypes.c_float)))

_gr3.gr3_createisosurfacemesh.restype = ctypes.c_int
_gr3.gr3_createisosurfacemesh.argtypes = (ctypes.POINTER(ctypes.c_uint),
        ctypes.POINTER(ctypes.c_ushort), ctypes.c_ushort,
        ctypes.c_uint, ctypes.c_uint, ctypes.c_uint,
        ctypes.c_uint, ctypes.c_uint, ctypes.c_uint,
        ctypes.c_double, ctypes.c_double, ctypes.c_double,
        ctypes.c_double, ctypes.c_double, ctypes.c_double)
