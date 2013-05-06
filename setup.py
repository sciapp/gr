#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import sysconfig
import os
import shlex
import re
import tempfile
from distutils.core import setup, Extension
from distutils.ccompiler import new_compiler
from subprocess import Popen, PIPE, STDOUT

__author__  = "Christian Felder <c.felder@fz-juelich.de>"
__date__    = "2013-04-12"
__version__ = "0.2.0"
__copyright__ = """Copyright 2012, 2013 Forschungszentrum Juelich GmbH

This file is part of GR, a universal framework for visualization applications.
Visit https://iffwww.iff.kfa-juelich.de/portal/doku.php?id=gr for the latest
version.

GR was developed by the Scientific IT-Systems group at the Peter Grünberg
Institute at Forschunsgzentrum Jülich. The main development has been done
by Josef Heinen who currently maintains the software.

GR is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This framework is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GR. If not, see <http://www.gnu.org/licenses/>.
 
"""

_GTK_PACKAGE = "gtk+-2.0"
_grdir = os.getenv("GRDIR", "/usr/local/gr")
_cc = os.getenv("CC", "cc")
_wxconfig = os.getenv("WX_CONFIG",
                      Popen(["which", "wx-config"],
                            stdout=PIPE).communicate()[0].rstrip())
_qtdir = os.getenv("QTDIR", None)

_build_3rdparty = os.path.join("build",
                               "3rdparty.%s-%d.%d" %(sysconfig.get_platform(),
                                                     sys.version_info.major,
                                                     sys.version_info.minor))

_gks_src = ["gks.c", "gksforbnd.c", "font.c", "afm.c", "util.c", "ft.c", "dl.c",
            "malloc.c", "error.c", "mf.c", "wiss.c", "cgm.c", "win.c", "mac.c",
            "ps.c", "pdf.c", "x11.c", "socket.c", "plugin.c", "compress.c",
            "io.c"]
_gks_plugin_src = ["font.cxx", "afm.cxx", "util.cxx", "dl.cxx",
                   "malloc.cxx", "error.cxx", "io.cxx"]
_gks_plugins = ["wxplugin.cxx", "qtplugin.cxx", "gtkplugin.cxx",
                "quartzplugin.m", "svgplugin.cxx", "figplugin.cxx",
                "gsplugin.cxx", "wmfplugin.cxx"]

_libpng_src = ["png.c", "pngerror.c", "pngget.c", "pngmem.c", "pngpread.c", 
               "pngread.c", "pngrio.c", "pngrtran.c", "pngrutil.c", "pngset.c",
               "pngtrans.c", "pngwio.c", "pngwrite.c", "pngwtran.c",
               "pngwutil.c"]

_libjpeg_src = ["jaricom.c", "jcapimin.c", "jcapistd.c", "jcarith.c",
                "jccoefct.c", "jccolor.c", "jcdctmgr.c", "jchuff.c", "jcinit.c",
                "jcmainct.c", "jcmarker.c", "jcmaster.c", "jcomapi.c",
                "jcparam.c", "jcprepct.c", "jcsample.c", "jctrans.c",
                "jdapimin.c", "jdapistd.c", "jdarith.c", "jdatadst.c",
                "jdatasrc.c", "jdcoefct.c", "jdcolor.c", "jddctmgr.c",
                "jdhuff.c", "jdinput.c", "jdmainct.c", "jdmarker.c",
                "jdmaster.c", "jdmerge.c", "jdpostct.c", "jdsample.c",
                "jdtrans.c", "jerror.c", "jfdctflt.c", "jfdctfst.c",
                "jfdctint.c", "jidctflt.c", "jidctfst.c", "jidctint.c",
                "jmemmgr.c", "jmemnobs.c", "jquant1.c", "jquant2.c", "jutils.c"]

_gr_src = ["gr.c", "text.c", "contour.c", "spline.c", "gridit.c", "strlib.c",
           "io.c", "image.c", "md5.c", "import.c", "grforbnd.c"]

_gr3_src = ["gr3_convenience.c", "gr3_html.c", "gr3_povray.c", "gr3_png.c",
            "gr3_jpeg.c", "gr3_gr.c"]

if sys.platform == "darwin":
    os.environ["MACOSX_DEPLOYMENT_TARGET"] = "10.6"
    os.environ["ARCHFLAGS"] = os.getenv("ARCHFLAGS", "-arch x86_64")
    _gks_xftincludes = ["/usr/X11/include/freetype2"]
    _gr3_src.insert(0, "gr3_cgl.c")
elif "linux" in sys.platform:
    _gks_xftincludes = ["/usr/include/freetype2"]
    _gr3_src.insert(0, "gr3_glx.c")
elif sys.platform == "win32":
    _gr3_src.insert(0, "gr3_win.c")

_gks_src_path = map(lambda p: os.path.join("lib", "gks", p), _gks_src)
_gks_plugin_src_path = map(lambda p: os.path.join("lib", "gks", "plugin", p),
                           _gks_plugin_src)
_libpng_src_path = map(lambda p: os.path.join("3rdparty", "png", p),
                       _libpng_src)
_libjpeg_src_path = map(lambda p: os.path.join("3rdparty", "jpeg", p),
                       _libjpeg_src)
_gr_src_path = map(lambda p: os.path.join("lib", "gr", p), _gr_src)
_gr3_src_path = map(lambda p: os.path.join("lib", "gr3", p), _gr3_src)
    
_plugins_path = {}
for plugin_src in _gks_plugins:
    _plugins_path[plugin_src] = list(_gks_plugin_src_path)
    _plugins_path[plugin_src].append(os.path.join("lib", "gks", "plugin",
                                                  plugin_src))

_gks_includes = ["/usr/X11R6/include"]    
_gks_include_dirs = list(_gks_includes)
_gks_include_dirs.extend(_gks_xftincludes)

_gks_plugin_includes = [os.path.join("3rdparty", "png"),
                        os.path.join("lib", "gks"),
                        "/usr/local/include"]

_gks_libs = ["pthread", "dl", "c", "m"]
_gks_xftlibs = ["Xft", "freetype", "fontconfig"]
_gks_xlibs = list(_gks_xftlibs)
_gks_xlibs.extend(["Xt", "X11"])
_gks_zlibs = ["z"]

_gks_plugin_libs = ["c", "m"]
_gks_plugin_xlibs = ["Xt", "X11"]
_gks_plugin_gslibs = ["gs"]

_pnglibs = ["png"]
#_jpeglibs = ["jpeg"]

_gks_libraries = list(_gks_libs)
_gks_libraries.extend(_gks_zlibs)
_gks_libraries.extend(_gks_xlibs)

_gr_macro = ("GRDIR", "\"%s\"" %_grdir)

_gksExt = Extension("libGKS", _gks_src_path,
                    define_macros=[("HAVE_ZLIB", ), ("XFT", ), _gr_macro],
                    include_dirs=_gks_include_dirs,
                    libraries=_gks_libraries,
                    extra_link_args=["-L/usr/X11R6/lib"])

_ext_modules = [_gksExt]

if _wxconfig:
    _wxlibs = shlex.split(Popen([_wxconfig, "--libs"],
                                stdout=PIPE).communicate()[0].rstrip())
    _wx_includes = shlex.split(Popen([_wxconfig, "--cxxflags"],
                                     stdout=PIPE).communicate()[0].rstrip())
    _gks_wx_libraries = list(_gks_plugin_libs)
    _gks_wx_libraries.extend(_gks_plugin_xlibs)
    _extra_link_args = list(_wxlibs)
    _extra_link_args.append("-L/usr/X11R6/lib")
    _gksWxExt = Extension("wxplugin", _plugins_path["wxplugin.cxx"],
                          define_macros=[_gr_macro],
                          include_dirs=_gks_plugin_includes,
                          libraries=_gks_wx_libraries,
                          extra_compile_args=_wx_includes,
                          extra_link_args=_extra_link_args)
    _ext_modules.append(_gksWxExt)
    
if _qtdir:
    _qmake = os.path.join(_qtdir, "bin", "qmake")
    _qtversion = Popen([_qmake, "-v"], stdout=PIPE, stderr=STDOUT).communicate()[0].rstrip()
    match = re.search("\d\.\d\.\d", _qtversion)
    if match:
        _qtversion = map(lambda s: int(s), match.group(0).split('.'))
        if _qtversion[0] < 4:
            if "clean" not in sys.argv:
                print >>sys.stderr, ("QTDIR points to old Qt version %s."
                                     %'.'.join(map(lambda i: str(i),
                                                   _qtversion)))
            
                inp = raw_input("Do you want to continue? [y/n]: ")
                if inp != 'y':
                    print >>sys.stderr, "exiting"
                    print >>sys.stderr, """
Please retry with a valid QTDIR setting, e.g.
/usr/lib64/qt4    (Red Hat)
/usr/share/qt4    (Ubuntu)
/usr/local/qt4"""
                    sys.exit(-1)
        # build
        _qt_include_dirs = [os.path.join(_qtdir, "include")]
        _gks_qt_includes = list(_gks_plugin_includes)
        _gks_qt_includes.extend(_qt_include_dirs)
        _gks_qt_libraries = ["QtGui", "QtCore"]
        _gksQtExt = Extension("qtplugin", _plugins_path["qtplugin.cxx"],
                              define_macros=[_gr_macro],
                              include_dirs=_gks_qt_includes,
                              libraries=_gks_qt_libraries,
                              extra_link_args=["-L/usr/X11R6/lib",
                                               "-L%s" %os.path.join(_qtdir,
                                                                    "lib")])
        _ext_modules.append(_gksQtExt)
    else:
        print >>sys.stderr, "Unable to obtain Qt version number."
else:
    print >>sys.stderr, "QTDIR not set. Build without Qt4 support."
    
# check for ghostscript support
if "clean" not in sys.argv:
    (fd, tmpsrc) = tempfile.mkstemp(suffix=".c", prefix="a")
    (fd2, tmpout) = tempfile.mkstemp(suffix=".out", prefix="a")
    os.write(fd, """#include <stdio.h>
#include <stdlib.h>
#include <ghostscript/iapi.h>

int main()
{
  gsapi_revision_t r;
  if (gsapi_revision(&r, sizeof(gsapi_revision_t)) == 0)
    fprintf(stderr, \"%ld\\n\", r.revision);
  exit(0);
}

""")
    os.close(fd)
    if sys.platform == "darwin":
        _gscc = Popen([_cc, "-o%s" %tmpout, tmpsrc, "-lgs", "-L/usr/X11/lib",
                       "-lXt", "-lX11", "-liconv"], stdout=PIPE, stderr=STDOUT)
    else:
        _gscc = Popen([_cc, "-o %s" %tmpout, tmpsrc, "-lgs"],
                      stdout=PIPE, stderr=STDOUT)
    _gscc.communicate()
    try:
        os.remove(tmpsrc)
        os.remove(tmpout)
    except OSError:
        pass
    if _gscc.returncode == 0:
        _gks_gs_includes = list(_gks_plugin_includes)
        _gks_gs_includes.append("/usr/local/include/ghostscript")
        _gks_gs_libraries = list(_gks_plugin_xlibs)
        _gks_gs_libraries.extend(_gks_plugin_gslibs)
        _gksGsExt = Extension("gsplugin", _plugins_path["gsplugin.cxx"],
                               define_macros=[_gr_macro],
                               include_dirs=_gks_gs_includes,
                               libraries=_gks_gs_libraries,
                               extra_link_args=["-L/usr/X11R6/lib"])
        _ext_modules.append(_gksGsExt)
    else:
        print >>sys.stderr, ("Ghostscript API not found. " +
                             "Build without Ghostscript support.")
        inp = raw_input("Do you want to continue? [y/n]: ")
        if inp != 'y':
            print >>sys.stderr, "exiting"
            sys.exit(-2)
            
    # check for GTK PACKAGE support
    _gtk_cflags = shlex.split(Popen(["pkg-config", _GTK_PACKAGE, "--cflags"],
                                    stdout=PIPE,
                                    stderr=PIPE).communicate()[0].rstrip())
    if _gtk_cflags:
        _gksGtkExt = Extension("gtkplugin", _plugins_path["gtkplugin.cxx"],
                               define_macros=[_gr_macro],
                               include_dirs=_gks_plugin_includes,
                               extra_compile_args=_gtk_cflags,
                               extra_link_args=["-L/usr/X11R6/lib"])
        _ext_modules.append(_gksGtkExt)
    
_gks_svg_libraries = list(_pnglibs)
_gks_svg_libraries.extend(_gks_zlibs)
_gksSvgExt = Extension("svgplugin", _plugins_path["svgplugin.cxx"],
                       define_macros=[_gr_macro],
                       include_dirs=_gks_plugin_includes,
                       libraries=_gks_svg_libraries)
_ext_modules.append(_gksSvgExt)

_gks_fig_libraries = list(_pnglibs)
_gks_fig_libraries.extend(_gks_zlibs)
_gksFigExt = Extension("figplugin", _plugins_path["figplugin.cxx"],
                       define_macros=[_gr_macro],
                       include_dirs=_gks_plugin_includes,
                       libraries=_gks_fig_libraries)
_ext_modules.append(_gksFigExt)

_gksWmfExt = Extension("wmfplugin", _plugins_path["wmfplugin.cxx"],
                       define_macros=[_gr_macro],
                       include_dirs=_gks_plugin_includes)
_ext_modules.append(_gksWmfExt)

if sys.platform == "darwin":
    _gksQuartzExt = Extension("quartzplugin", _plugins_path["quartzplugin.m"],
                              define_macros=[_gr_macro],
                              include_dirs=_gks_plugin_includes,
                              libraries=["objc"],
                              extra_link_args=["-framework Foundation",
                                               "-framework " +
                                               "ApplicationServices"])
    _ext_modules.append(_gksQuartzExt)
    
# prerequisites: build static 3rdparty libraries
_libpng = os.path.join(_build_3rdparty, "libpng.a")
_libjpeg = os.path.join(_build_3rdparty, "libjpeg.a")
if "clean" not in sys.argv:
    compiler = new_compiler()
    if not os.path.isfile(_libpng):
        obj = compiler.compile(_libpng_src_path, extra_preargs=["-fPIC"])
        compiler.create_static_lib(obj, "png", output_dir=_build_3rdparty)
    if not os.path.isfile(_libjpeg):  
        obj = compiler.compile(_libjpeg_src_path, extra_preargs=["-fPIC"])
        compiler.create_static_lib(obj, "jpeg", output_dir=_build_3rdparty)
else:
    try:
        map(lambda p: os.remove(os.path.join(_build_3rdparty, p)),
            os.listdir(_build_3rdparty))
        os.rmdir(_build_3rdparty)
    except OSError:
        pass

# libGR
_gr_include_dirs = list(_gks_xftincludes)
_gr_include_dirs.append(os.path.join("lib", "gks"))
_gr_include_dirs.append(os.path.join("3rdparty", "png"))
_gr_include_dirs.append(os.path.join("3rdparty", "jpeg"))
_gr_libraries = list(_gks_libraries)
_gr_extra_link_args = ["-L/usr/X11R6/lib", _libjpeg, _libpng]
_grExt = Extension("libGR", _gr_src_path,
                   define_macros=[("HAVE_ZLIB", ), ("XFT", ), _gr_macro],
                   include_dirs=_gr_include_dirs,
                   libraries=_gr_libraries,
                   extra_link_args=_gr_extra_link_args)
_ext_modules.append(_grExt)

setup(name="gr",
      version=__version__,
      description="GR, a universal framework for visualization applications",
      author="Scientific IT-Systems",
      author_email="j.heinen@fz-juelich.de",
      license="GNU General Public License",
      url="https://iffwww.iff.kfa-juelich.de/portal/doku.php?id=gr",
      package_dir={'': "lib/gr/python"},
      py_modules=["gr", "pygr"],
      packages=["qtgr", "qtgr.events"],
      ext_modules=_ext_modules)
