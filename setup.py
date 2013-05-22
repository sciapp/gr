#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import sysconfig
import os
import shlex
import re
import tempfile
from distutils.command.build_ext import build_ext as _build_ext
from distutils.core import setup, Extension
from distutils.ccompiler import new_compiler
from distutils.sysconfig import get_config_var
from subprocess import Popen, PIPE, STDOUT

__author__  = "Christian Felder <c.felder@fz-juelich.de>"
__date__    = "2013-05-21"
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
       
class build_ext(_build_ext):
    
    # workaround for libGKS on win32 no "init" + module_name function
    def get_export_symbols (self, ext):
        """Return the list of symbols that a shared extension has to
        export. This overloaded function does not append "init" + module_name
        to the exported symbols (default in superclass function).
        """
        return ext.export_symbols
    
    # workaround do not use pyd files in this project because we have our
    # own ctypes wrapper. Use dll files instead.
    def get_ext_filename(self, ext_name):
        r"""Convert the name of an extension (eg. "foo.bar") into the name
        of the file from which it will be loaded (eg. "foo/bar.so", or
        "foo\bar.dll not foo\bar.pyd (default in superclass function)").
        """
        ret = _build_ext.get_ext_filename(self, ext_name)
        (pathNoExt, ext) = os.path.splitext(ret)
        if ext == ".pyd":
            ret = pathNoExt + ".dll"
        return ret

_GTK_PACKAGE = "gtk+-2.0"
_grdir = os.getenv("GRDIR", "/usr/local/gr")
_cc = os.getenv("CC", "cc")
_wxconfig = os.getenv("WX_CONFIG")
_qtdir = os.getenv("QTDIR")
_wxdir = os.getenv("WXDIR")
_wxlib = os.getenv("WXLIB")
_gsdir = os.getenv("GSDIR")

# unique platform id used by distutils
_uPlatformId = "%s-%d.%d" %(sysconfig.get_platform(), sys.version_info.major,
                            sys.version_info.minor)
_build_lib = os.path.join("build", "lib." + _uPlatformId) 
_build_3rdparty = os.path.join("build", "3rdparty." + _uPlatformId)
_build_temp = os.path.join("build", "temp." + _uPlatformId)

# prerequisites: static 3rdparty libraries
if sys.platform == "win32":
    # if linking against png, jpeg and zlib use this ordering!
    _libpng = os.path.join(_build_3rdparty, "png.lib")
    _libjpeg = os.path.join(_build_3rdparty, "jpeg.lib")
    _libz = os.path.join(_build_3rdparty, "z.lib")
    
    _libs_msvc = ["msvcrt", "oldnames", "kernel32", "wsock32", "advapi32",
                  "user32", "gdi32", "comdlg32", "winspool"]
    # w32ERR: __imp_ -> fixed by linking against oldnames.lib
    _msvc_extra_compile_args = ["/Zi", "/D_DLL", "/D_POSIX"]
    _msvc_extra_link_args = ["/nodefaultlib", "-dll"]
else:
    _libz = os.path.join(_build_3rdparty, "libz.a")
    _libpng = os.path.join(_build_3rdparty, "libpng.a")
    _libjpeg = os.path.join(_build_3rdparty, "libjpeg.a")
    
    _libs_msvc = []
    _msvc_extra_compile_args = []
    _msvc_extra_link_args = []


_gks_src = ["gks.c", "gksforbnd.c", "font.c", "afm.c", "util.c", "ft.c", "dl.c",
            "malloc.c", "error.c", "mf.c", "wiss.c", "cgm.c", "win.c", "mac.c",
            "ps.c", "pdf.c", "x11.c", "socket.c", "plugin.c", "compress.c",
            "io.c"]
_gks_plugin_src = ["font.cxx", "afm.cxx", "util.cxx", "dl.cxx",
                   "malloc.cxx", "error.cxx", "io.cxx"]
_gks_plugins = ["wxplugin.cxx", "qtplugin.cxx", "gtkplugin.cxx",
                "quartzplugin.m", "svgplugin.cxx", "figplugin.cxx",
                "gsplugin.cxx", "wmfplugin.cxx"]

_libz_src = ["adler32.c", "compress.c", "crc32.c", "deflate.c", "gzclose.c",
             "gzlib.c", "gzread.c", "gzwrite.c", "infback.c", "inffast.c",
             "inflate.c", "inftrees.c", "trees.c", "uncompr.c", "zutil.c"]

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

_gr3_src = ["gr3.c", "gr3_convenience.c", "gr3_html.c", "gr3_povray.c",
            "gr3_png.c", "gr3_jpeg.c", "gr3_gr.c"]

if sys.platform == "darwin":
    os.environ["MACOSX_DEPLOYMENT_TARGET"] = "10.6"
    os.environ["ARCHFLAGS"] = os.getenv("ARCHFLAGS", "-arch x86_64")
    os.environ["LDSHARED"] = get_config_var("LDSHARED").replace("-bundle",
                                                                "-dynamiclib")
    _gks_xftincludes = ["/usr/X11/include/freetype2"]
    _platform_extra_link_args = ["-Wl,-rpath,@loader_path/."]
    _gr3_src.insert(0, "gr3_cgl.c")
    if not _wxconfig:
        Popen(["which", "wx-config"], stdout=PIPE).communicate()[0].rstrip()
elif "linux" in sys.platform:
    _gks_xftincludes = ["/usr/include/freetype2"]
    _platform_extra_link_args = ["-Wl,-rpath,$ORIGIN"]
    _gr3_src.insert(0, "gr3_glx.c")
    if not _wxconfig:
        Popen(["which", "wx-config"], stdout=PIPE).communicate()[0].rstrip()
elif sys.platform == "win32":
    # win32 not tested.
    _gks_xftincludes = []
    _platform_extra_link_args = []
    _gr3_src.insert(0, "gr3_win.c")
    _cc = os.getenv("CC")
    _grdir = os.getenv("GRDIR", "\"C:\\\\gr\"")
#elif sys.platform == "solaris":
#    _gks_xftincludes = ["/usr/include/freetype2"]
#    _platform_extra_link_args.append("-R$ORIGIN")
#    _gr3_src.insert(0, "gr3_glx.c")
else:
    print >>sys.stderr, "Platform \"%s\" is not supported." %sys.platform
    sys.exit(-3)

_gks_src_path = map(lambda p: os.path.join("lib", "gks", p), _gks_src)
_gks_plugin_src_path = map(lambda p: os.path.join("lib", "gks", "plugin", p),
                           _gks_plugin_src)
_libz_src_path = map(lambda p: os.path.join("3rdparty", "zlib", p),
                       _libz_src)
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

if sys.platform == "win32":
    _gks_includes = []
    _gks_include_dirs = []
else:
    _gks_includes = ["/usr/X11R6/include"]
    _gks_include_dirs = list(_gks_includes)    
    _gks_include_dirs.extend(_gks_xftincludes)

_gks_plugin_includes = [os.path.join("3rdparty", "png"),
                        os.path.join("3rdparty", "zlib"),
                        os.path.join("lib", "gks"),
                        "/usr/local/include"]
if sys.platform == "win32":
    _gks_libs = list(_libs_msvc)
else:
    _gks_libs = ["pthread", "dl", "c", "m"]
_gks_xftlibs = ["Xft", "freetype", "fontconfig"]
_gks_xlibs = list(_gks_xftlibs)
_gks_xlibs.extend(["Xt", "X11"])

if sys.platform == "win32":
    _gks_plugin_libs = list(_libs_msvc)
else:
    _gks_plugin_libs = ["c", "m"]
_gks_plugin_xlibs = ["Xt", "X11"]
_gks_plugin_gslibs = ["gs"]

_zlibs = ["z"]
_pnglibs = ["png"]
_jpeglibs = ["jpeg"]

_gks_libraries = list(_gks_libs)
#_gks_libraries.extend(_zlibs)
if sys.platform != "win32":
    _gks_libraries.extend(_gks_xlibs)

_gr_macro = ("GRDIR", "\"%s\"" %_grdir)
_gks_macros = [("HAVE_ZLIB", ), ("XFT", ), _gr_macro]
if sys.platform == "win32":
    _gks_macros.append(("NO_X11", 1))
    _gks_extra_link_args = ["/nodefaultlib", "-def:lib\gks\libgks.def"]
    _gks_extra_compile_args = list(_msvc_extra_compile_args)
    _gks_export_symbols = os.path.join("lib", "gks", "libgks.def")
else:
    _gks_extra_compile_args = []
    _gks_extra_link_args = ["-L/usr/X11R6/lib"]
_gks_extra_link_args.extend(_platform_extra_link_args)
#_gks_extra_link_args.append(_libz)
if sys.platform == "darwin":
    _gks_extra_link_args.append("-Wl,-install_name,@rpath/libGKS.so")
_gksExt = Extension("libGKS", _gks_src_path,
                    define_macros=_gks_macros,
                    include_dirs=_gks_include_dirs,
                    libraries=_gks_libraries,
                    extra_link_args=_gks_extra_link_args,
                    extra_compile_args=_gks_extra_compile_args,
                    export_symbols=None)
_ext_modules = [_gksExt]

# prerequisites: build static 3rdparty libraries
if "clean" not in sys.argv:
    if sys.platform == "win32":
        _extra_preargs = list(_msvc_extra_compile_args)
    else:
        _extra_preargs = ["-fPIC"]
    if not os.path.isdir("build"):
        os.mkdir("build")
    if not os.path.isdir(_build_3rdparty):
        os.mkdir(_build_3rdparty)
    compiler = new_compiler()
    if not os.path.isfile(_libz):
        obj = compiler.compile(_libz_src_path, extra_preargs=_extra_preargs)
        compiler.create_static_lib(obj, "z", output_dir=_build_3rdparty)
    if not os.path.isfile(_libpng):
        _png_extra_preargs = list(_extra_preargs)
        _png_extra_preargs.append("-I3rdparty/zlib")
        obj = compiler.compile(_libpng_src_path,
                               extra_preargs=_png_extra_preargs)
        compiler.create_static_lib(obj, "png", output_dir=_build_3rdparty)
    if not os.path.isfile(_libjpeg):  
        obj = compiler.compile(_libjpeg_src_path, extra_preargs=_extra_preargs)
        compiler.create_static_lib(obj, "jpeg", output_dir=_build_3rdparty)
else:
    try:
        map(lambda p: os.remove(os.path.join(_build_3rdparty, p)),
            os.listdir(_build_3rdparty))
        os.rmdir(_build_3rdparty)
    except OSError:
        pass

_gks_wx_libraries = list(_gks_plugin_libs)
_gks_wx_includes = list(_gks_plugin_includes)
_wxlibs = None
if _wxconfig:
    _wxlibs = shlex.split(Popen([_wxconfig, "--libs"],
                                stdout=PIPE).communicate()[0].rstrip())
    _wx_extra_compile_args = shlex.split(Popen([_wxconfig, "--cxxflags"],
                                               stdout=PIPE).communicate()[0].rstrip())
    _wx_library_dirs = None
    _gks_wx_libraries.extend(_gks_plugin_xlibs)
    _extra_link_args = list(_wxlibs)
    _extra_link_args.append("-L/usr/X11R6/lib")
elif sys.platform == "win32":
    if _wxlib and _wxdir:
        _wxlibs = ["wxmsw29ud_core", "wxbase29ud"]
        _gks_wx_includes.append(os.path.join(_wxdir, "include", "msvc"))
        _gks_wx_includes.append(os.path.join(_wxdir, "include"))
        _wx_library_dirs = [_wxlib]
        _wx_extra_compile_args = list(_msvc_extra_compile_args)
        _wx_extra_compile_args.append("/DWXUSINGDLL")
        _wx_extra_compile_args.append("/DwxMSVC_VERSION_AUTO")
        _wx_extra_compile_args.append("/D_UNICODE")
        _extra_link_args = ["-dll"]
#        _gks_wx_libraries.extend(_libs_msvc)
    else:
        print >>sys.stderr, "WXDIR or WXLIB not set. Build without wx support."

if _wxlibs:
    _gksWxExt = Extension("wxplugin", _plugins_path["wxplugin.cxx"],
                          define_macros=[_gr_macro],
                          include_dirs=_gks_wx_includes,
                          library_dirs=_wx_library_dirs,
                          libraries=_gks_wx_libraries,
                          extra_compile_args=_wx_extra_compile_args,
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
        if sys.platform == "win32":
# do not use _msvc_extra_link_args because /nodefaultlib causes
# error LNK2019: unresolved external symbol ""__declspec(dllimport)
#                void __cdecl std::_Xbad_alloc(void)" ...
            _qt_extra_link_args = ["-dll"]
            _gks_qt_libraries = ["QtGui%d" %_qtversion[0],
                                 "QtCore%d" %_qtversion[0]]
            _gks_qt_libraries.extend(_libs_msvc)
        else:
            _qt_extra_link_args = ["-L/usr/X11R6/lib",
                                   "-L%s" %os.path.join(_qtdir, "lib")]
            _gks_qt_libraries = ["QtGui", "QtCore"]
        _qt_include_dirs = [os.path.join(_qtdir, "include")]
        _gks_qt_includes = list(_gks_plugin_includes)
        _gks_qt_includes.extend(_qt_include_dirs)
        _gksQtExt = Extension("qtplugin", _plugins_path["qtplugin.cxx"],
                              define_macros=[_gr_macro],
                              include_dirs=_gks_qt_includes,
                              libraries=_gks_qt_libraries,
                              library_dirs=[os.path.join(_qtdir, "lib")],
                              extra_link_args=_qt_extra_link_args,
                              extra_compile_args=_msvc_extra_compile_args)
        _ext_modules.append(_gksQtExt)
    else:
        print >>sys.stderr, "Unable to obtain Qt version number."
else:
    print >>sys.stderr, "QTDIR not set. Build without Qt4 support."
    
# check for ghostscript support
if "clean" not in sys.argv:
    _gks_gs_library_dirs = None
    _gks_gs_includes = list(_gks_plugin_includes)
    if sys.platform == "win32":
        _gks_gs_libraries = ["gsdll32"]
        _gks_gs_libraries.extend(_libs_msvc)
        _gks_gs_extra_link_args = list(_msvc_extra_link_args)
        if _gsdir:
            _gks_gs_library_dirs = [os.path.join(_gsdir, "bin")]
            _gks_gs_includes.append(os.path.join(_gsdir, "include"))
        else:
            print >>sys.stderr, ("GSDIR not set. " +
                                 "Build without Ghostscript support.")
    else:
        _gks_gs_includes.append("/usr/local/include/ghostscript")
        _gks_gs_libraries = list(_gks_plugin_xlibs)
        _gks_gs_libraries.extend(_gks_plugin_gslibs)
        _gks_gs_extra_link_args = ["-L/usr/X11R6/lib"]
        
    _gksGsExt = Extension("gsplugin", _plugins_path["gsplugin.cxx"],
                           define_macros=[_gr_macro],
                           include_dirs=_gks_gs_includes,
                           library_dirs=_gks_gs_library_dirs,
                           libraries=_gks_gs_libraries,
                           extra_link_args=_gks_gs_extra_link_args,
                           extra_compile_args=_msvc_extra_compile_args)
    
    if sys.platform == "win32" and _gsdir:
        _ext_modules.append(_gksGsExt)
    
    if _cc:
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
            _ext_modules.append(_gksGsExt)
        else:
            print >>sys.stderr, ("Ghostscript API not found. " +
                                 "Build without Ghostscript support.")
            inp = raw_input("Do you want to continue? [y/n]: ")
            if inp != 'y':
                print >>sys.stderr, "exiting"
                sys.exit(-2)
            
    # check for GTK PACKAGE support
    if sys.platform != "win32":
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
    
_gks_svg_libraries = list(_pnglibs) # w32ERR: __imp_
_gks_svg_libraries.extend(_zlibs)   # w32ERR: __imp_
_gks_svg_libraries.extend(_libs_msvc)
#_gks_svg_libraries = [_libpng, _libz]
if sys.platform == "win32":
    _gks_svg_libraries.extend(_libs_msvc)
_gksSvgExt = Extension("svgplugin", _plugins_path["svgplugin.cxx"],
                       define_macros=[_gr_macro],
                       include_dirs=_gks_plugin_includes,
                       library_dirs=[_build_3rdparty],
                       libraries=_gks_svg_libraries,
                       extra_compile_args=_msvc_extra_compile_args)
_ext_modules.append(_gksSvgExt)

_gks_fig_libraries = list(_pnglibs) # w32ERR: __imp_
_gks_fig_libraries.extend(_zlibs)   # w32ERR: __imp_
_gks_fig_libraries.extend(_libs_msvc)
#_gks_fig_libraries = [_libpng, _libz]
_gksFigExt = Extension("figplugin", _plugins_path["figplugin.cxx"],
                       define_macros=[_gr_macro],
                       include_dirs=_gks_plugin_includes,
                       library_dirs=[_build_3rdparty],
                       libraries=_gks_fig_libraries,
                       extra_compile_args=_msvc_extra_compile_args,
                       extra_link_args=_msvc_extra_link_args)
_ext_modules.append(_gksFigExt)

_gksWmfExt = Extension("wmfplugin", _plugins_path["wmfplugin.cxx"],
                       define_macros=[_gr_macro],
                       include_dirs=_gks_plugin_includes,
                       libraries=_libs_msvc)
_ext_modules.append(_gksWmfExt)

if sys.platform == "darwin":
    _gksQuartzExt = Extension("quartzplugin", _plugins_path["quartzplugin.m"],
                              define_macros=[_gr_macro],
                              include_dirs=_gks_plugin_includes,
                              libraries=["objc"],
                              extra_link_args=["-framework", "Foundation",
                                               "-framework",
                                               "ApplicationServices"])
    _ext_modules.append(_gksQuartzExt)
    
## prerequisites: build static 3rdparty libraries
#if "clean" not in sys.argv:
#    if sys.platform == "win32":
#        _extra_preargs = list(_msvc_extra_compile_args)
#    else:
#        _extra_preargs = ["-fPIC"]
#    if not os.path.isdir("build"):
#        os.mkdir("build")
#    if not os.path.isdir(_build_3rdparty):
#        os.mkdir(_build_3rdparty)
#    compiler = new_compiler()
#    if not os.path.isfile(_libz):
#        obj = compiler.compile(_libz_src_path, extra_preargs=_extra_preargs)
#        compiler.create_static_lib(obj, "z", output_dir=_build_3rdparty)
#    if not os.path.isfile(_libpng):
#        _png_extra_preargs = list(_extra_preargs)
#        _png_extra_preargs.append("-I3rdparty/zlib")
#        obj = compiler.compile(_libpng_src_path,
#                               extra_preargs=_png_extra_preargs)
#        compiler.create_static_lib(obj, "png", output_dir=_build_3rdparty)
#    if not os.path.isfile(_libjpeg):  
#        obj = compiler.compile(_libjpeg_src_path, extra_preargs=_extra_preargs)
#        compiler.create_static_lib(obj, "jpeg", output_dir=_build_3rdparty)
#else:
#    try:
#        map(lambda p: os.remove(os.path.join(_build_3rdparty, p)),
#            os.listdir(_build_3rdparty))
#        os.rmdir(_build_3rdparty)
#    except OSError:
#        pass

# libGR
_gr_include_dirs = list(_gks_xftincludes)
_gr_include_dirs.append(os.path.join("lib", "gks"))
_gr_include_dirs.append(os.path.join("3rdparty", "zlib"))
_gr_include_dirs.append(os.path.join("3rdparty", "png"))
_gr_include_dirs.append(os.path.join("3rdparty", "jpeg"))
#_gr_libraries = list(_gks_libraries)
_gr_extra_link_args = list(_platform_extra_link_args)
if sys.platform != "win32":
    _gr_libraries = ["GKS"]
    _gr_extra_link_args.append("-L/usr/X11R6/lib")
    _gr_library_dirs = [_build_lib, _build_3rdparty]
    # important: lib ordering png, jpeg, z
    _gr_extra_link_args.append(_libpng)
    _gr_extra_link_args.append(_libjpeg)
    _gr_extra_link_args.append(_libz)
else:
    _gr_libraries = ["libGKS"] 
    _gr_libraries.extend(_pnglibs)
    _gr_libraries.extend(_jpeglibs)
    _gr_libraries.extend(_zlibs)
    _gr_library_dirs = [_build_lib, _build_3rdparty,
                        os.path.join(_build_temp, "Release", "lib", "gks")]
    _gr_libraries.extend(_libs_msvc)
if sys.platform == "darwin":
    _gr_extra_link_args.append("-Wl,-install_name,@rpath/libGR.so")
_grExt = Extension("libGR", _gr_src_path,
                   define_macros=[("HAVE_ZLIB", ), ("XFT", ), _gr_macro],
                   include_dirs=_gr_include_dirs,
                   libraries=_gr_libraries,
                   library_dirs=_gr_library_dirs,
                   extra_link_args=_gr_extra_link_args)
_ext_modules.append(_grExt)

#libGR3
_gr3_include_dirs = list(_gr_include_dirs)
_gr3_include_dirs.append(os.path.join("lib", "gr"))
#_gr3_libraries = list(_gr_libraries)
_gr3_libraries = []
_gr3_libraries.append("GR")
#_gr3_extra_link_args = ["-L/usr/X11R6/lib"]
_gr3_extra_link_args = []
_gr3_extra_link_args.extend(_platform_extra_link_args)
# important: lib ordering png, jpeg, z
_gr3_extra_link_args.append(_libpng)
_gr3_extra_link_args.append(_libjpeg)
_gr3_extra_link_args.append(_libz)
if sys.platform == "darwin":
    framework = ["-framework", "OpenGL", "-framework", "Cocoa"]
    _gr3_extra_link_args.extend(framework)
    _gr3_extra_link_args.append("-Wl,-install_name,@rpath/libGR3.so")
else:
    _gr3_libraries.append("GL")
    _gr3_libraries.append("X11")
    _gr3_libraries.append(_libz)
#    _gr3_libraries.extend(_zlibs)
_gr3Ext = Extension("libGR3", _gr3_src_path,
#                    define_macros=[("HAVE_ZLIB", ), ("XFT", ), _gr_macro],
                    include_dirs=_gr3_include_dirs,
                    libraries=_gr3_libraries,
                    library_dirs=[_build_lib],
                    extra_link_args=_gr3_extra_link_args)
if sys.platform != "win32":
    _ext_modules.append(_gr3Ext)

setup(cmdclass={"build_ext": build_ext},
      name="gr",
      version=__version__,
      description="GR, a universal framework for visualization applications",
      author="Scientific IT-Systems",
      author_email="j.heinen@fz-juelich.de",
      license="GNU General Public License",
      url="https://iffwww.iff.kfa-juelich.de/portal/doku.php?id=gr",
      package_dir={'': "lib/gr/python",
                   "gr3": "lib/gr3"},
      py_modules=["gr", "pygr"],
      packages=["gr3", "qtgr", "qtgr.events"],
      ext_modules=_ext_modules)
