#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
import shlex
import re
from distutils.core import setup, Extension
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

_grdir = os.getenv("GRDIR", "/usr/local/gr")
_wxconfig = os.getenv("WX_CONFIG",
                      Popen(["which", "wx-config"],
                            stdout=PIPE).communicate()[0].rstrip())
_qtdir = os.getenv("QTDIR", None)

_gks_src = ["gks.c", "gksforbnd.c", "font.c", "afm.c", "util.c", "ft.c", "dl.c",
            "malloc.c", "error.c", "mf.c", "wiss.c", "cgm.c", "win.c", "mac.c",
            "ps.c", "pdf.c", "x11.c", "socket.c", "plugin.c", "compress.c",
            "io.c"]
_gks_plugin_src = ["font.cxx", "afm.cxx", "util.cxx", "dl.cxx",
                   "malloc.cxx", "error.cxx", "io.cxx"]
_gks_plugins = ["wxplugin.cxx", "qtplugin.cxx", "svgplugin.cxx",
                "figplugin.cxx", "gsplugin.cxx"]

_gks_src_path = map(lambda p: os.path.join("lib", "gks", p), _gks_src)
_gks_plugin_src_path = map(lambda p: os.path.join("lib", "gks", "plugin", p),
                           _gks_plugin_src)

if sys.platform == "darwin":
    os.environ["MACOSX_DEPLOYMENT_TARGET"] = "10.6"
    os.environ["ARCHFLAGS"] = os.getenv("ARCHFLAGS", "-arch x86_64")
    _gks_xftincludes = ["/usr/X11/include/freetype2"]
else:
    _gks_xftincludes = ["/usr/include/freetype2"]
    
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

_gks_libraries = list(_gks_libs)
_gks_libraries.extend(_gks_zlibs)
_gks_libraries.extend(_gks_xlibs)

_gr_macro = ("GRDIR", "\"%s\"" %_grdir)

_gksExt = Extension("libGKS", _gks_src_path,
                    define_macros=[("HAVE_ZLIB", ), ("XFT", ), _gr_macro],
                    include_dirs=_gks_include_dirs,
                    libraries=_gks_libraries,
                    # next line needed for mac?
#                    extra_compile_args=["-fpascal-strings"],
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
