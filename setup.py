#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import with_statement, print_function

import sys
import os
import shlex
import re
import tempfile
import shutil
import distutils.command
import distutils.command.install
from distutils.core import Command
from distutils.command.build_ext import build_ext as _build_ext
from distutils.command.clean import clean as _clean
from distutils.ccompiler import new_compiler
from distutils.sysconfig import get_config_var
from distutils.sysconfig import get_python_lib
from distutils.util import get_platform
from platform import architecture
from subprocess import PIPE, STDOUT
if str == bytes:
    from subprocess import Popen
else:
    # since python 3.x str and bytes type are not equal.
    # the bytes type has no read attribute which is needed,
    # e.g. for rstrip calls
    from subprocess import Popen as _Popen

    class Popen(_Popen):
        def communicate(self, *args, **kwargs):
            stdout, stderr = _Popen.communicate(self, *args, **kwargs)
            if stdout is not None:
                stdout = stdout.decode()
            if stderr is not None:
                stderr = stderr.decode()
            return (stdout, stderr)

from setuptools import setup, Extension
from setuptools.command.build_py import build_py as _build_py
import vcversioner


__author__ = "Christian Felder <c.felder@fz-juelich.de>"
__version__ = vcversioner.find_version(
    version_module_paths=[os.path.join("lib", "gr", "python",
                                       "gr", "_version.py")]).version
__copyright__ = """Copyright (c) 2012-2015: Josef Heinen, Florian Rhiem,
Christian Felder and other contributors:

http://gr-framework.org/credits.html

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

The GR framework can be built with plugins that use code from the
following projects, which have their own licenses:

- MuPDF - a lightweight PDF and XPS viewer (AFPL)
- Ghostscript - an interpreter for the PostScript language and for PDF (AFPL)
- FFmpeg - a multimedia framework (LGPL / GPLv2)

"""


def get_output(*command):
    return Popen(list(command), stdout=PIPE).communicate()[0].rstrip()


def get_words(*command):
    return shlex.split(get_output(*command))


class build_static(Command):

    # prerequisites: build static 3rdparty libraries
    description = "build static 3rdparty libraries"

    user_options = [('static-extras', None, "Build static extra libraries"),
                    ('skip-static-build', None, "Do not compile static "
                                                "libraries"),
                    ('macosx-deployment-target=', None,
                     "The earliest version of MacOS X this package will run on"
                     " (OSX only)"),
                   ]

    def initialize_options(self):
        self.isDarwin = (sys.platform == "darwin")
        self.static_extras = False
        self.skip_static_build = False
        self.macosx_deployment_target = None

    def finalize_options(self):
        # -- macosx-deployment-target ---------------
        if self.isDarwin:
            # check for interpreters MACOSX_DEPLOYMENT_TARGET setting
            version = list(map(int, get_config_var(
                                    "MACOSX_DEPLOYMENT_TARGET").split('.')))
            # require at least 10.7
            if version[0] == 10 and version[1] < 7:
                version[1] = 7
            self.macosx_deployment_target = (
                os.getenv("MACOSX_DEPLOYMENT_TARGET", "%d.%d" % (version[0],
                                                                 version[1])))

    def run(self):
        if self.isDarwin:
            os.environ['MACOSX_DEPLOYMENT_TARGET'] = (
                self.macosx_deployment_target)
        if not self.skip_static_build:
            if sys.platform == "win32":
                _extra_preargs = list(_msvc_extra_compile_args)
            else:
                _extra_preargs = ["-fPIC"]
            if not os.path.isdir("build"):
                os.mkdir("build")
            if not os.path.isdir(_build_3rdparty_lib):
                os.makedirs(_build_3rdparty_lib)
            if not os.path.isdir(_build_temp):
                os.mkdir(_build_temp)
            compiler = new_compiler()
            if not os.path.isfile(_libz):
                obj = compiler.compile(_libz_src_path,
                                       extra_preargs=_extra_preargs)
                compiler.create_static_lib(obj, "z",
                                           output_dir=_build_3rdparty_lib)
            if not os.path.isfile(_libpng):
                _png_extra_preargs = list(_extra_preargs)
                _png_extra_preargs.append("-I3rdparty/zlib")
                obj = compiler.compile(_libpng_src_path,
                                       extra_preargs=_png_extra_preargs)
                compiler.create_static_lib(obj, "png",
                                           output_dir=_build_3rdparty_lib)
            if not os.path.isfile(_libjpeg):
                obj = compiler.compile(_libjpeg_src_path,
                                       extra_preargs=_extra_preargs)
                compiler.create_static_lib(obj, "jpeg",
                                           output_dir=_build_3rdparty_lib)
            if not os.path.isfile(_libqhull):
                obj = compiler.compile(_libqhull_src_path,
                                       extra_preargs=_extra_preargs)
                compiler.create_static_lib(obj, "qhull",
                                           output_dir=_build_3rdparty_lib)
            if self.static_extras:
                real_build_3rdparty = os.path.realpath(_build_3rdparty)
                freetype = os.path.join("3rdparty", "freetype")
                if not os.system("make -C " + freetype + " DIR=" +
                                 real_build_3rdparty):
                    shutil.copy(os.path.join(freetype, "libfreetype.a"),
                                os.path.join(real_build_3rdparty, "lib"))
                    if os.system("make -C 3rdparty extras DIR=" +
                                 real_build_3rdparty):
                        # building static libraries failed
                        sys.exit(-4)


class clean_static(Command):

    description = "clean up temporary files from 'build_static' command"

    user_options = [('no-static-extras', None, "Do not clean static extra "
                                               "libraries")]

    def initialize_options(self):
        self.no_static_extras = False

    def finalize_options(self):
        pass

    def run(self):
        print("removing '", _build_3rdparty, "' (and everything under it)",
              sep='')
        shutil.rmtree(_build_3rdparty, ignore_errors=True)
        if not self.no_static_extras:
            print("cleaning static extras")
            os.system("make -C 3rdparty clean")


class clean(_clean, clean_static):

    user_options = (_clean.user_options + clean_static.user_options)

    def initialize_options(self):
        clean_static.initialize_options(self)
        _clean.initialize_options(self)

    def finalize_options(self):
        clean_static.finalize_options(self)
        _clean.finalize_options(self)

    def run(self):
        gregg = os.path.join("lib", "gr", "python", "gr.egg-info")
        clean_static.run(self)
        print("removing '", gregg, "' (and everything under it)", sep='')
        if os.path.isdir(gregg):
            shutil.rmtree(gregg, ignore_errors=True)
        _clean.run(self)
        if sys.platform == "darwin":
            os.system("xcodebuild -project lib/gks/quartz/GKSTerm.xcodeproj "
                      "clean")


class check_ext(build_static, Command):

    description = "dynamically generate list of C/C++ extensions for build_ext"

    # List of option tuples: long name, short name (None if no short
    # name), and help string.
    user_options = [('disable-wx', None, "Disable wx plugin"),
                    ('disable-qt4', None, "Disable qt4 plugin"),
                    ('disable-qt5', None, "Disable qt5 plugin"),
                    ('disable-gtk', None, "Disable gtk plugin"),
                    ('disable-gs', None, "Disable gs plugin"),
                    ('disable-fig', None, "Disable fig plugin"),
                    ('disable-svg', None, "Disable svg plugin"),
                    ('disable-wmf', None, "Disable wmf plugin"),
                    ('disable-mov', None, "Disable mov plugin"),
                    ('disable-html', None, "Disable html plugin"),
                    ('disable-pgf', None, "Disable pgf plugin"),
                    ('disable-opengl', None, "Disable OpenGL libraries"),
                    ('disable-quartz', None,
                     "Disable quartz plugin (OSX only)"),
                    ('disable-x11', None, "Disable X11 libraries"),
                    ('disable-freetype', None, "Disable freetype libraries"),
                    ('disable-xft', None,
                     "Disable Xft libraries (disables also freetype)"),
                    ('disable-xt', None,
                     "Disable Xt libraries (disables also x11 and freetype)"),
                    ('disable-mupdf', None, "Disable mupdf libraries"),
                    ('disable-cairo', None, "Disable cairo libraries"),
                    ('qmake-qt4=', None, "Qt4 qmake executable"),
                    ('qmake-qt5=', None, "Qt5 qmake executable"),
                   ] + build_static.user_options

    def initialize_options(self):
        build_static.initialize_options(self)
        self.isLinuxOrDarwin = any(s in sys.platform for s in
                                   ["linux", "darwin"])
        self.isLinux = ("linux" in sys.platform)
        self.isDarwin = (sys.platform == "darwin")
        self.isWin32 = (sys.platform == "win32")
        self.is64bit = (architecture()[0] == "64bit")
        # extensions list
        self.ext_modules = []
        # -- user options -------------------------------------
        self.disable_wx = False
        self.disable_qt4 = False
        self.disable_qt5 = False if self.isLinuxOrDarwin else True
        self.disable_gs = False
        self.disable_fig = False
        self.disable_svg = False
        self.disable_html = False
        self.disable_pgf = False
        self.disable_wmf = False
        self.disable_opengl = False
        # only available on OS X
        self.disable_quartz = False if self.isDarwin else True
        # available on Linux and OS X
        self.disable_gtk = False if self.isLinuxOrDarwin else True
        self.disable_x11 = False if self.isLinuxOrDarwin else True
        self.disable_xft = False if self.isLinuxOrDarwin else True
        self.disable_xt = False if self.isLinuxOrDarwin else True
        self.disable_freetype = False if self.isLinuxOrDarwin else True
        self.disable_mupdf = False if self.isLinuxOrDarwin else True
        self.disable_mov = False if self.isLinuxOrDarwin else True
        self.disable_cairo = False if self.isLinuxOrDarwin else True
        # -- environment -------------------------------------
        self.cc = os.getenv("CC", "cc")

        pylib = get_python_lib(plat_specific=True, prefix='')
        eggname = (self.distribution.get_fullname() + "-py%d.%d-%s"
                   % (sys.version_info[0], sys.version_info[1],
                      get_platform()) + ".egg")
        grdir = os.path.join(pylib, eggname, "gr")
        options_install = self.distribution.get_option_dict("install")
        if options_install and ("old_and_unmanageable" in options_install or
                                "single_version_externally_managed" in
                                options_install):
            grdir = os.path.join(pylib, "gr")
        self.grdir = os.getenv("GRDIR", grdir)
        # -- x11 -------------------------------------
        self.x11lib = []
        self.x11inc = []
        self.x11libs = []
        self.x11ldflags = []
        self.x11cflags = []
        # -- gs -------------------------------------
        self.gsdir = os.getenv("GSDIR")
        self.gsinc = []
        self.gslib = []
        self.gslibs = []
        self.gsldflags = []
        # -- cairo -------------------------------------
        self.cairodir = os.getenv("CAIRODIR")
        self.cairoinc = []
        self.cairolib = []
        self.cairolibs = []
        self.cairoldflags = []
        # -- wx -------------------------------------
        self.wxconfig = os.getenv("WX_CONFIG")
        self.wxdir = os.getenv("WXDIR")
        self.wxlib = [os.getenv("WXLIB")] if os.getenv("WXLIB") else []
        self.wxinc = []
        self.wxlibs = []
        self.wxldflags = []
        self.wxcxx = []
        # -- qt4 -------------------------------------
        self.qt4dir = os.getenv("QT4DIR", "")
        self.qt4inc = [os.path.join(self.qt4dir, "include")]
        self.qt4lib = [os.path.join(self.qt4dir, "lib")]
        self.qt4libs = []
        self.qt4ldflags = []
        self.qt4version = None
        self.qmake_qt4 = os.path.join(self.qt4dir, "bin", "qmake")
        if self.isWin32:
            self.qmake_qt4 += ".exe"
        if not os.path.isfile(self.qmake_qt4):
            self.qmake_qt4 = ""
        # -- qt5 -------------------------------------
        self.qt5dir = os.getenv("QT5DIR", "")
        self.qt5inc = [os.path.join(self.qt5dir, "include")]
        self.qt5lib = [os.path.join(self.qt5dir, "lib")]
        self.qt5libs = []
        self.qt5ldflags = []
        self.qt5version = None
        self.qmake_qt5 = os.path.join(self.qt5dir, "bin", "qmake")
        if self.isWin32:
            self.qmake_qt5 += ".exe"
        if not os.path.isfile(self.qmake_qt5):
            self.qmake_qt5 = ""
        # -- freetype -------------------------------------
        self.ftconfig = None
        self.ftldflags = []
        self.ftcflags = []
        # -- gtk -------------------------------------
        self.gtk_package = "gtk+-2.0"
        self.gtkldflags = []
        self.gtkcflags = []
        # -- mupdf -------------------------------------
        self.mupdfinc = []
        self.mupdflibs = []
        self.mupdfldflags = []
        # -- opengl -------------------------------------
        self.gllibs = []
        self.glldflags = []
        # -- platform extra link args -------------------------------------
        self.platform_ldflags = []


    def _test_c_compile(self, src, inc=[], lib=[], libs=[], cflags=[],
                        ldflags=[], caller=None):
        cflags = cflags[:]
        ldflags = ldflags[:]
        cflags.extend("-I" + i for i in inc)
        ldflags.extend("-L" + ld for ld in lib)
        ldflags.extend("-l" + lib for lib in libs)
        fd, tmpsrc = tempfile.mkstemp(suffix=".c", prefix="a")
        _, tmpout = tempfile.mkstemp(suffix=".out", prefix="a")
        os.write(fd, src)
        os.close(fd)
        cmd = [self.cc, "-o", tmpout, tmpsrc]
        cmd.extend(cflags)
        cmd.extend(ldflags)
        cc = Popen(cmd, stdout=PIPE, stderr=STDOUT)
        out, err = cc.communicate()
        try:
            os.remove(tmpsrc)
            os.remove(tmpout)
        except OSError:
            pass
        # report if verbose (default) and compilation failed
        if self.verbose and cc.returncode != 0:
            print("Test: ", caller.__name__ if caller else "[unknown]")
            print(' '.join(cmd))
            print(out)
            print()
        return cc.returncode == 0

    def _test_gs(self, gsinc=[], gslibs=[], gscflags=[], gsldflags=[]):
        return self._test_c_compile(b"""
#include <stdio.h>
#include <stdlib.h>
#include <ghostscript/iapi.h>

int main()
{
  gsapi_revision_t r;
  if (gsapi_revision(&r, sizeof(gsapi_revision_t)) == 0)
    fprintf(stderr, \"%ld\\n\", r.revision);
  exit(0);
}

""",
            gsinc, [], gslibs, gscflags, gsldflags, self._test_gs
        )

    def _test_mupdf(self, mupdfinc=[], mupdflibs=[], mupdfcflags=[],
                    mupdfldflags=[]):
        return self._test_c_compile(b"""
#include <mupdf/fitz.h>

int main(int argc, char **argv)
{
  fz_context  *ctx;
  fz_document *doc;
  fz_rect      rect;
  fz_irect     bbox;
  fz_pixmap   *pix;
  fz_device   *dev;
  fz_page     *page;

  ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
  fz_register_document_handlers(ctx);
  doc = fz_open_document(ctx, argv[1]);
  page = fz_load_page(doc, 0);
  fz_bound_page(doc, page, &rect);
  fz_round_rect(&bbox, &rect);
  pix = fz_new_pixmap_with_bbox(ctx, fz_device_rgb(ctx), &bbox);
  dev = fz_new_draw_device(ctx, pix);
  fz_run_page(doc, page, dev, &fz_identity, NULL);

  fz_free_device(dev);
  fz_drop_pixmap(ctx, pix);
  fz_free_page(doc, page);
  fz_free_context(ctx);
  return 0;
}

""", mupdfinc, [], mupdflibs, mupdfcflags, mupdfldflags, self._test_mupdf)

    def _test_cairo(self, cairoinc=[], cairolibs=[], cairocflags=[],
                    cairoldflags=[]):
        return self._test_c_compile(b"""
#include <cairo/cairo.h>

int main(int argc, char **argv) {
    cairo_t *cr;
    cairo_surface_t *surface;

    surface = (cairo_surface_t *) cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 500, 500);
    cr = cairo_create(surface);

    cairo_move_to(cr, 0, 0);
    cairo_line_to(cr, 500, 500);
    cairo_stroke(cr);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    return 0;
}

""", cairoinc, [], cairolibs, cairocflags, cairoldflags, self._test_cairo)

    def _test_mov(self, mupdfinc=[], mupdflibs=[], mupdfcflags=[],
                  mupdfldflags=[]):
        return self._test_c_compile(b"""
#include <stdio.h>
#include <stdlib.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <mupdf/fitz.h>

int main()
{
  exit(0);
}

""", mupdfinc, [], mupdflibs, mupdfcflags, mupdfldflags, self._test_mupdf)

    def _test_opengl(self, glinc=[], gllibs=[], glcflags=[], glldflags=[]):
        return self._test_c_compile(b"""
#include <stdio.h>
#include <stdlib.h>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

int main()
{
  exit(0);
}

""", glinc, [], gllibs, glcflags, glldflags, self._test_opengl)

    def _test_xft(self, xftinc=[], xftlib=[], xftlibs=[], xftcflags=[],
                  xftldflags=[]):
        return self._test_c_compile(b"""
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

#include <X11/Xft/Xft.h>

int main()
{
  exit(0);
}

""", xftinc, xftlib, xftlibs, xftcflags, xftldflags, self._test_xft)

    def _test_xt(self, xtinc=[], xtlib=[], xtlibs=[], xtcflags=[],
                 xtldflags=[]):
        return self._test_c_compile(b"""
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

#include <X11/Intrinsic.h>

int main()
{
  exit(0);
}

""", xtinc, xtlib, xtlibs, xtcflags, xtldflags, self._test_xt)

    def _test_freetype(self, ftinc=[], ftlibs=[], ftcflags=[], ftldflags=[]):
        return self._test_c_compile(b"""
#include <stdio.h>
#include <stdlib.h>

#include <ft2build.h>

int main()
{
  exit(0);
}

""", ftinc, [], ftlibs, ftcflags, ftldflags, self._test_freetype)

    def _finalize_LinuxOrDarwinPost(self):
        pass

    def _finalize_Win32Post(self):
        # -- qt4 -------------------------------------
        if not self.disable_qt4:
            self.qt4libs = ["QtGui%d" % self.qt4version[0],
                            "QtCore%d" % self.qt4version[0]]
        # -- qt5 -------------------------------------
        if not self.disable_qt5:
            self.qt5libs = ["Qt%dGui" % self.qt5version[0],
                            "Qt%dCore" % self.qt5version[0]]

    def _finalize_LinuxOrDarwinPre(self):
        # -- wx -------------------------------------
        if not self.disable_wx and not self.wxconfig:
            self.wxconfig = get_output("which", "wx-config")
            if not os.path.isfile(self.wxconfig):
                self.disable_wx = True
            else:
                self.wxldflags = get_words(self.wxconfig, "--libs")
                self.wxcxx = get_words(self.wxconfig, "--cxxflags")
        # -- qt -------------------------------------
        def set_qmake_path(major_version=4):
            qmake = getattr(self, "qmake_qt{0:d}".format(major_version))
            if qmake:
                # use full qualified path for qmake
                qmake = get_output("which", qmake)
            else:
                # try to determine qmake version automatically
                qmake = get_output("which", "qmake-qt{0:d}".format(major_version))
                if not qmake:
                    qmake = get_output("which", "qmake")
                    if qmake:
                        qt_version = get_output(qmake, "-query", "QT_VERSION")
                        if not qt_version.startswith("{0:d}.".format(major_version)):
                            qmake = ""
            setattr(self, "qmake_qt{0:d}".format(major_version), qmake)

        if not self.disable_qt4:
            set_qmake_path(major_version=4)
            if os.path.isdir("/usr/local/Cellar"):
                self.qt4libs = []
            else:
                self.qt4libs = ["QtGui", "QtCore"]
        if not self.disable_qt5:
            set_qmake_path(major_version=5)
            if os.path.isdir("/usr/local/Cellar"):
                self.qt5libs = []
            else:
                self.qt5libs = ["Qt5Widgets", "Qt5Gui", "Qt5Core"]
        # -- x11 -------------------------------------
        if self.isLinux:
            x11ldflags = None
            try:
                x11ldflags = get_words("pkg-config", "x11", "--libs")
            except OSError:
                pass
            if x11ldflags:
                self.x11ldflags = x11ldflags
                self.x11cflags = get_words("pkg-config", "x11", "--cflags")
            else:
                self.disable_x11 = True
        else:
            for p in [os.path.join(os.sep, "usr", "X11R6"),
                      os.path.join(os.sep, "usr", "X11")]:
                if os.path.isdir(p):
                    self.x11inc = [os.path.join(p, "include")]
                    self.x11lib = [os.path.join(p, "lib")]
                    break
            if not self.x11lib:
                self.disable_x11 = True
        # -- gs -------------------------------------
        if not self.disable_gs:
            for p in [os.path.join(os.sep, "usr", "local", "include"),
                      os.path.join(os.sep, "usr", "include")]:
                ghost = os.path.join(p, "ghostscript")
                if os.path.isdir(ghost):
                    self.gsinc = [p]
                    break
            if not self.gsinc:
                self.disable_gs = True
            else:
                self.gslibs = ["gs"] if self.isLinux else ["gs", "Xt", "X11",
                                                           "iconv"]
                if self.isDarwin:
                    self.gsldflags = ["-L" + ldir for ldir in self.x11lib]
                    self.gsldflags.append("-L/usr/local/lib")
                self.disable_gs = not self._test_gs(self.gsinc, self.gslibs,
                                                    gsldflags=self.gsldflags)
        # -- freetype -------------------------------------
        if not self.disable_freetype:
            if not self.static_extras:
                ftconfig = get_output("which", "freetype-config")
                self.disable_freetype = not bool(ftconfig)
                if not self.disable_freetype:
                    self.ftconfig = ftconfig
                    self.ftldflags = get_words(self.ftconfig, "--libs")
                    self.ftcflags = get_words(self.ftconfig, "--cflags")
            else:
                self.ftcflags = ["-I" + os.path.join("3rdparty", "freetype",
                                                     "include")]
                self.ftldflags = [os.path.join(_build_3rdparty_lib,
                                               "libfreetype.a")]
            self.disable_freetype = (
                             not self._test_freetype(ftcflags=self.ftcflags,
                                                     ftldflags=self.ftldflags))
        # -- Xft -------------------------------------
        if not self.disable_xft:
            libs = list(self.x11libs)
            libs.append("Xft")
            cflags = list(self.x11cflags)
            cflags.extend(self.ftcflags)
            ldflags = list(self.x11ldflags)
            ldflags.extend(self.ftldflags)
            self.disable_xft = not self._test_xft(self.x11inc, self.x11lib,
                                                  libs, cflags, ldflags)
        # -- Xt -------------------------------------
        if not self.disable_xt:
            libs = list(self.x11libs)
            libs.append("Xt")
            self.disable_xt = not self._test_xt(self.x11inc, self.x11lib, libs,
                                                self.x11cflags,
                                                self.x11ldflags)
        # -- gtk -------------------------------------
        if not self.disable_gtk:
            gtkcflags = None
            try:
                gtkcflags = get_words("pkg-config", self.gtk_package, "--cflags")
            except OSError:
                pass
            if gtkcflags:
                self.gtkcflags = gtkcflags
                self.gtkldflags = get_words("pkg-config", self.gtk_package, "--libs")
            else:
                self.disable_gtk = True
        # -- mupdf -------------------------------------
        if not self.disable_mupdf:
            if not self.static_extras:
                for p in [os.path.join(os.sep, "usr", "local"),
                          os.path.join(os.sep, "usr")]:
                    mupdf = os.path.join(p, "include", "mupdf")
                    if os.path.isdir(mupdf):
                        self.mupdfinc = [os.path.join(p, "include")]
                        self.mupdfldflags.append("-L%s" % os.path.join(p, "lib"))
                        break
                if not self.mupdfinc:
                    self.disable_mupdf = True
                else:
                    # mupdf compiled without ssl support
                    self.mupdflibs.extend(["mupdf", "jbig2dec", "jpeg",
                                          "openjp2", "z", "m"])
                    self.mupdfldflags = self.mupdfldflags + self.ftldflags
            else:
                mupdflibs = ["mupdf", "freetype", "jbig2dec", "jpeg",
                             "openjp2", "z"]
                self.mupdfldflags = [os.path.join(_build_3rdparty_lib,
                                                  "lib" + name + ".a")
                                     for name in mupdflibs]
                self.mupdfldflags.append("-lm")
                self.mupdfinc = [os.path.join(_build_3rdparty, "include")]
            self.disable_mupdf = not self._test_mupdf(self.mupdfinc,
                                                      self.mupdflibs,
                                          mupdfldflags=self.mupdfldflags)
            if self.disable_mupdf:
                # mupdf compiled with ssl support
                self.mupdflibs.append("ssl")
                self.mupdflibs.append("crypto")
                self.disable_mupdf = not self._test_mupdf(self.mupdfinc,
                                                          self.mupdflibs,
                                          mupdfldflags=self.mupdfldflags)
            if not self.disable_mupdf:
                # test shared library build
                self.disable_mupdf = not self._test_mupdf(self.mupdfinc,
                                                          self.mupdflibs,
                                                          ["-fPIC", "-shared"],
                                                          self.mupdfldflags)
            if self.disable_mupdf:
                self.mupdfinc = []
                self.mupdflibs = []
                self.mupdfldflags = []
        # -- cairo -------------------------------------
        if not self.disable_cairo:
            if not self.static_extras:
                for p in [os.path.join(os.sep, "usr", "local"),
                          os.path.join(os.sep, "usr")]:
                    cairo = os.path.join(p, "include", "cairo")
                    if os.path.isdir(cairo):
                        self.cairoinc = [os.path.join(p, "include")]
                        self.cairoldflags.append("-L%s" % os.path.join(p, "lib"))
                        break
                if not self.cairoinc:
                    self.disable_cairo = True
                else:
                    self.cairolibs.extend(["cairo", "pixman-1"])
            else:
                cairolibs = ["cairo", "pixman-1"]
                self.cairoldflags = [os.path.join(_build_3rdparty_lib,
                                                  "lib" + name + ".a")
                                     for name in cairolibs]
                self.cairoldflags.append("-lm")
                self.cairoinc = [os.path.join(_build_3rdparty, "include")]
            self.cairolibs.append("pthread")
            self.disable_cairo = not self._test_cairo(self.cairoinc,
                                                      self.cairolibs,
                                          cairoldflags=self.cairoldflags)
            if not self.disable_cairo:
                # test shared library build
                self.disable_cairo = not self._test_cairo(self.cairoinc,
                                                          self.cairolibs,
                                                          ["-fPIC", "-shared"],
                                                          self.cairoldflags)
            if self.disable_cairo:
                self.cairoinc = []
                self.cairolibs = []
                self.cairoldflags = []
        # -- mov -------------------------------------
        if not self.disable_mov:
            self.disable_mov = not self._test_mov(self.mupdfinc, self.mupdflibs,
                                              mupdfldflags=self.mupdfldflags)
        # -- opengl --------------------------------------------------
        if not self.disable_opengl:
            gllibs, glldflags = [], []
            if self.isDarwin:
                framework = ["-framework", "OpenGL", "-framework", "Cocoa"]
                glldflags.extend(framework)
            elif self.isLinux:
                gllibs.append("GL")
            self.disable_opengl = not self._test_opengl(gllibs=gllibs,
                                                        glldflags=glldflags)
            if not self.disable_opengl:
                self.gllibs, self.glldflags = gllibs, glldflags
        # -- platform extra link args -------------------------------------
        if self.isDarwin:
            self.platform_ldflags = ["-mmacosx-version-min="
                                     + self.macosx_deployment_target,
                                     "-Wl,-rpath,@loader_path/."]
        elif self.isLinux:
            self.platform_ldflags = ["-Wl,-rpath,$ORIGIN"]

    def _finalize_Win32Pre(self):
        # -- wx -------------------------------------
        if not (self.wxdir and self.wxlib):
            self.disable_wx = True
        else:
            if self.is64bit:
                self.wxlibs = ["wxmsw30ud_core", "wxbase30ud"]
            else:
                self.wxlibs = ["wxmsw29ud_core", "wxbase29ud"]
            self.wxinc = [os.path.join(self.wxdir, "include", "msvc"),
                          os.path.join(self.wxdir, "include")]
        # -- qt4 -------------------------------------
        if not self.disable_qt4:
            # do not use _msvc_extra_link_args because /nodefaultlib causes
            # error LNK2019: unresolved external symbol ""__declspec(dllimport)
            #                void __cdecl std::_Xbad_alloc(void)" ...
            self.qt4ldflags = ["-dll"]
        # -- qt5 -------------------------------------
        if not self.disable_qt5:
            # do not use _msvc_extra_link_args because /nodefaultlib causes
            # error LNK2019: unresolved external symbol ""__declspec(dllimport)
            #                void __cdecl std::_Xbad_alloc(void)" ...
            self.qt5ldflags = ["-dll"]
        # -- gs -------------------------------------
        if not self.disable_gs and self.gsdir:
            self.gsinc = [os.path.join(self.gsdir, "include")]
            self.gslib = [os.path.join(self.gsdir, "bin")]
            if self.is64bit:
                self.gslibs = ["gsdll64"]
            else:
                self.gslibs = ["gsdll32"]
            if (not os.path.isdir(self.gsinc[0]) or
                not os.path.isdir(self.gslib[0])):
                self.disable_gs = True
        else:
            self.disable_gs = True
        # -- opengl --------------------------------------------------
        if not self.disable_opengl:
            self.gllibs.append("opengl32")
        # -- grdir -------------------------------------
        self.grdir = os.getenv("GRDIR", ("\\\"%s\\\""
                 % os.path.join(get_python_lib(plat_specific=True, prefix=''),
                                "gr").replace('\\', "\\\\")))

    def finalize_options(self):
        # build static dependencies before testing
        build_static.finalize_options(self)
        build_static.run(self)
        # -- platform finalizers (pre) -------------------------------------
        if self.isLinuxOrDarwin:
            self._finalize_LinuxOrDarwinPre()
        elif self.isWin32:
            self._finalize_Win32Pre()

        # -- platform independend finalizers -------------------------------
        # qt
        def set_qt_flags(major_version=4):
            qmake = getattr(self, "qmake_qt{0:d}".format(major_version))
            disable_qt = False
            if not os.path.isfile(qmake):
                disable_qt = True
            else:
                qtversion = get_output(qmake, "-query", "QT_VERSION")
                if qtversion:
                    qtversion = [int(s) for s in qtversion.split('.')]
                if not qtversion or qtversion[0] != major_version:
                    disable_qt = True
                else:
                    qquery = Popen([qmake, "-query", "QT_INSTALL_HEADERS"],
                                   stdout=PIPE)
                    std = qquery.communicate()
                    if qquery.returncode == 0:
                        qtinc = [std[0].rstrip()]
                    qquery = Popen([qmake, "-query", "QT_INSTALL_LIBS"],
                                   stdout=PIPE)
                    std = qquery.communicate()
                    if qquery.returncode == 0:
                        qtlib = [std[0].rstrip()]
            if disable_qt:
                qtinc = []
                qtlib = []
                qtlibs = []
            for attr_name in ("disable_qt", "qtversion", "qtinc", "qtlib",
                              "qtlibs"):
                attr_value = locals()[
                    attr_name] if attr_name in locals() else None
                if attr_value is not None:
                    attr_name = attr_name.replace("qt", "qt{0:d}".format(
                        major_version))
                    setattr(self, attr_name, attr_value)

        if not self.disable_qt4:
            set_qt_flags(major_version=4)
        if not self.disable_qt5:
            set_qt_flags(major_version=5)

        # -- platform independend tests -------------------------------------
#        if not self.disable_gs:
#            self.disable_gs = not self._test_gs(self.gsinc, self.gslibs,
#                                                gsldflags=self.gsldflags)
#        if not self.disable_mupdf:
#            self.disable_mupdf = not self._test_mupdf(self.mupdfinc,
#                                                      self.mupdflibs,
#                                                  mupdfldflags=self.ftldflags)

        # -- platform finalizers (post) -------------------------------------
        if self.isLinuxOrDarwin:
            self._finalize_LinuxOrDarwinPost()
        elif self.isWin32:
            self._finalize_Win32Post()

        if self.disable_mupdf:
            self.disable_mov = True

        # disable freetype if Xft or Xt is not available
        if not self.disable_freetype:
            if self.disable_xft or self.disable_xt:
                self.disable_freetype = True

        # disable x11 if Xt is not available
        if not self.disable_x11:
            if self.disable_xt:
                self.disable_x11 = True

    def run(self):
        if not (self.isLinuxOrDarwin or self.isWin32):
            print("Platform \"", sys.platform, "\" is not supported.",
                  file=sys.stderr)
            sys.exit(-3)

        print(" isLinuxOrDarwin: ", self.isLinuxOrDarwin)
        print("         isLinux: ", self.isLinux)
        print("        isDarwin: ", self.isDarwin)
        print("         isWin32: ", self.isWin32)
        print("")
        if self.isDarwin:
            print("      OSX target: ", self.macosx_deployment_target)
            print("")
        print("          x11lib: ", self.x11lib)
        print("          x11inc: ", self.x11inc)
        print("         x11libs: ", self.x11libs)
        print("      x11ldflags: ", self.x11ldflags)
        print("       x11cflags: ", self.x11cflags)
        print("")
        print("        wxconfig: ", self.wxconfig)
        print("           wxdir: ", self.wxdir)
        print("           wxlib: ", self.wxlib)
        print("           wxinc: ", self.wxinc)
        print("          wxlibs: ", self.wxlibs)
        print("       wxldflags: ", self.wxldflags)
        print("           wxcxx: ", self.wxcxx)
        print("")
        print("      gtkldflags: ", self.gtkldflags)
        print("       gtkcflags: ", self.gtkcflags)
        print("")
        print("       qmake-qt4: ", self.qmake_qt4)
        print("          qt4dir: ", self.qt4dir)
        print("          qt4inc: ", self.qt4inc)
        print("          qt4lib: ", self.qt4lib)
        print("         qt4libs: ", self.qt4libs)
        print("      qt4ldflags: ", self.qt4ldflags)
        print("     Qt4 version: ", self.qt4version)
        print("")
        print("       qmake-qt5: ", self.qmake_qt5)
        print("          qt5dir: ", self.qt5dir)
        print("          qt5inc: ", self.qt5inc)
        print("          qt5lib: ", self.qt5lib)
        print("         qt5libs: ", self.qt5libs)
        print("      qt5ldflags: ", self.qt5ldflags)
        print("     Qt5 version: ", self.qt5version)
        print("")
        print("           gsdir: ", self.gsdir)
        print("           gsinc: ", self.gsinc)
        print("           gslib: ", self.gslib)
        print("          gslibs: ", self.gslibs)
        print("       gsldflags: ", self.gsldflags)
        print("")
        print("           grdir: ", self.grdir)
        print("")
        print(" freetype-config: ", self.ftconfig)
        print("       ftldflags: ", self.ftldflags)
        print("        ftcflags: ", self.ftcflags)
        print("")
        print("        mupdfinc: ", self.mupdfinc)
        print("       mupdflibs: ", self.mupdflibs)
        print("    mupdfldflags: ", self.mupdfldflags)
        print("")
        print("        cairoinc: ", self.cairoinc)
        print("        cairolib: ", self.cairolib)
        print("       cairolibs: ", self.cairolibs)
        print("    cairoldflags: ", self.cairoldflags)
        print("")
        print("      opengllibs: ", self.gllibs)
        print("    opengldflags: ", self.glldflags)
        print("")
        print("     disable-x11: ", self.disable_x11)
        print("      disable-xt: ", self.disable_xt)
        print("     disable-xft: ", self.disable_xft)
        print("      disable-wx: ", self.disable_wx)
        print("     disable-qt4: ", self.disable_qt4)
        print("     disable-qt5: ", self.disable_qt5)
        print("     disable-gtk: ", self.disable_gtk)
        print("      disable-gs: ", self.disable_gs)
        print("     disable-fig: ", self.disable_fig)
        print("     disable-svg: ", self.disable_svg)
        print("    disable-html: ", self.disable_html)
        print("     disable-pgf: ", self.disable_pgf)
        print("     disable-wmf: ", self.disable_wmf)
        print("     disable-mov: ", self.disable_mov)
        print("  disable-opengl: ", self.disable_opengl)
        print("  disable-quartz: ", self.disable_quartz)
        print("disable-freetype: ", self.disable_freetype)
        print("   disable-mupdf: ", self.disable_mupdf)
        print("   disable-cairo: ", self.disable_cairo)
        print("")

        zinc = [os.path.join("3rdparty", "zlib")]
        defines = [("HAVE_ZLIB",), ("GRDIR", "\"%s\"" % self.grdir)]
        if self.disable_x11:
            defines.append(("NO_X11", 1))
        if self.disable_freetype:
            defines.append(("NO_FT", 1))
        if self.disable_mupdf:
            defines.append(("NO_MUPDF", 1))
        if self.disable_cairo:
            defines.append(("NO_CAIRO", 1))

        # -- GKS -------------------------------------
        inc = list(self.x11inc)
        inc.extend(zinc)
        lib = list(self.x11lib)
        libs = list(self.x11libs)
        ldflags = list(self.x11ldflags)
        ldflags.extend(self.ftldflags)
        cflags = list(self.x11cflags)
        cflags.extend(self.ftcflags)
        if self.isWin32:
            libs.extend(_libs_msvc)
            libs.append("msimg32")
            ldflags.append("/nodefaultlib")
            ldflags.append("-def:lib\gks\libgks.def")
            cflags.extend(_msvc_extra_compile_args)
        ldflags.extend(self.platform_ldflags)
        if self.isDarwin:
            cflags.append("-mmacosx-version-min=" +
                          self.macosx_deployment_target)
            ldflags.append("-Wl,-install_name,@rpath/libGKS.so")
        if not self.disable_freetype:
            libs.append("fontconfig")
            libs.append("Xft")
        if not self.disable_freetype or not self.disable_xt:
            libs.append("Xt")
        gksExt = Extension("gr.libGKS", _gks_src_path,
                           define_macros=defines,
                           include_dirs=inc,
                           library_dirs=lib,
                           libraries=libs,
                           extra_link_args=ldflags,
                           extra_compile_args=cflags,
                           export_symbols=None)
        self.ext_modules.append(gksExt)

        # -- includes -------------------------------------
        # GKS include path (needed for GKS plugins)
        gksinc = [os.path.join("lib", "gks")]
        # 3rdparty include paths (needed for some GKS plugins)
        pnginc = [os.path.join("3rdparty")]
        zinc = [os.path.join("3rdparty", "zlib")]
        jpeginc = [os.path.join("3rdparty", "jpeg")]

        ##
        # -- GKS plugins -------------------------------------

        # -- ghostscript -------------------------------------
        if not self.disable_gs:
            inc = list(self.gsinc)
            inc.extend(gksinc)
            lib = list(self.gslib)
            libs = list(self.gslibs)
            ldflags = list(self.gsldflags)
            cflags = []
            if self.isWin32:
                libs.extend(_libs_msvc)
                cflags.extend(_msvc_extra_compile_args)
            gksGsExt = Extension("gr.gsplugin",
                                 _plugins_path["gsplugin.cxx"],
                                 define_macros=defines,
                                 include_dirs=inc,
                                 library_dirs=lib,
                                 libraries=libs,
                                 extra_link_args=ldflags,
                                 extra_compile_args=cflags)
            self.ext_modules.append(gksGsExt)

        # -- cairo ----------------------------------
        if not self.disable_cairo:
            inc = list(self.cairoinc)
            inc.extend(gksinc)
            inc.extend(pnginc)
            inc.extend(zinc)
            inc.extend(self.x11inc)
            lib = list(self.cairolib)
            libs = list(self.cairolibs)
            libs.extend(self.x11libs)
            ldflags = list(self.cairoldflags)
            cflags = []
            if self.isWin32:
                libs.extend(_libs_msvc)
                cflags.extend(_msvc_extra_compile_args)
            gksCairoExt = Extension("gr.cairoplugin",
                                    _plugins_path["cairoplugin.cxx"],
                                    define_macros=defines,
                                    include_dirs=inc,
                                    library_dirs=lib,
                                    libraries=libs,
                                    extra_link_args=ldflags,
                                    extra_compile_args=cflags)
            self.ext_modules.append(gksCairoExt)

        # -- svg -------------------------------------
        if not self.disable_svg:
            inc = list(gksinc)
            inc.extend(pnginc)
            inc.extend(zinc)
            lib = [_build_3rdparty_lib]
            libs = ["png", "z"]
            ldflags = []
            cflags = []
            if self.isWin32:
                libs.extend(_libs_msvc)
                cflags.extend(_msvc_extra_compile_args)
            gksSvgExt = Extension("gr.svgplugin",
                                  _plugins_path["svgplugin.cxx"],
                                  define_macros=defines,
                                  include_dirs=inc,
                                  library_dirs=lib,
                                  libraries=libs,
                                  extra_link_args=ldflags,
                                  extra_compile_args=cflags)
            self.ext_modules.append(gksSvgExt)

        # -- htm -------------------------------------
        if not self.disable_html:
            inc = list(gksinc)
            inc.extend(pnginc)
            inc.extend(zinc)
            lib = [_build_3rdparty_lib]
            libs = ["png", "z"]
            ldflags = []
            cflags = []
            if self.isWin32:
                libs.extend(_libs_msvc)
                cflags.extend(_msvc_extra_compile_args)
            gksHtmExt = Extension("gr.htmplugin",
                                  _plugins_path["htmplugin.cxx"],
                                  define_macros=defines,
                                  include_dirs=inc,
                                  library_dirs=lib,
                                  libraries=libs,
                                  extra_link_args=ldflags,
                                  extra_compile_args=cflags)
            self.ext_modules.append(gksHtmExt)

        # -- pgf -------------------------------------
        if not self.disable_pgf:
            inc = list(gksinc)
            inc.extend(pnginc)
            inc.extend(zinc)
            lib = [_build_3rdparty_lib]
            libs = ["png", "z"]
            ldflags = []
            cflags = []
            if self.isWin32:
                libs.extend(_libs_msvc)
                cflags.extend(_msvc_extra_compile_args)
            gksPgfExt = Extension("gr.pgfplugin",
                                  _plugins_path["pgfplugin.cxx"],
                                  define_macros=defines,
                                  include_dirs=inc,
                                  library_dirs=lib,
                                  libraries=libs,
                                  extra_link_args=ldflags,
                                  extra_compile_args=cflags)
            self.ext_modules.append(gksPgfExt)

        # -- fig -------------------------------------
        if not self.disable_fig:
            inc = list(gksinc)
            inc.extend(pnginc)
            inc.extend(zinc)
            lib = [_build_3rdparty_lib]
            libs = ["png", "z"]
            ldflags = []
            cflags = []
            if self.isWin32:
                libs.extend(_libs_msvc)
                cflags.extend(_msvc_extra_compile_args)
            gksFigExt = Extension("gr.figplugin",
                                  _plugins_path["figplugin.cxx"],
                                  define_macros=defines,
                                  include_dirs=inc,
                                  library_dirs=lib,
                                  libraries=libs,
                                  extra_link_args=ldflags,
                                  extra_compile_args=cflags)
            self.ext_modules.append(gksFigExt)

        # -- wmf -------------------------------------
        if not self.disable_wmf:
            inc = list(gksinc)
            libs = []
            ldflags = []
            cflags = []
            if self.isWin32:
                libs.extend(_libs_msvc)
                cflags.extend(_msvc_extra_compile_args)
            gksWmfExt = Extension("gr.wmfplugin",
                                  _plugins_path["wmfplugin.cxx"],
                                  define_macros=defines,
                                  include_dirs=inc,
                                  libraries=libs,
                                  extra_link_args=ldflags,
                                  extra_compile_args=cflags)
            self.ext_modules.append(gksWmfExt)

        # -- mov -------------------------------------
        if not self.disable_mov:
            inc = list(self.mupdfinc)
            inc.extend(gksinc)
            libs = list(self.mupdflibs)
            ffmpeglibs = ["avdevice", "avformat", "avfilter", "avcodec",
                          "swscale", "avutil"]
            if self.isDarwin:
                ffmpeglibs.extend(["theora", "ogg", "vpx"])
            ffmpeglibs.append("z")
            if not self.static_extras:
                libs.extend(ffmpeglibs)
            else:
                libs.extend([os.path.join(_build_3rdparty_lib,
                                          "lib" + name + ".a")
                             for name in ffmpeglibs])
            libs.append("pthread")
            ldflags = list(self.mupdfldflags)
            cflags = []
            if self.isWin32:
                libs.extend(_libs_msvc)
#                    ldflags.extend(_msvc_extra_link_args)   # necessary?
                cflags.extend(_msvc_extra_compile_args)
            gksMovExt = Extension("gr.movplugin",
                                  _plugins_path["movplugin.cxx"],
                                  define_macros=defines,
                                  include_dirs=inc,
                                  libraries=libs,
                                  extra_link_args=ldflags,
                                  extra_compile_args=cflags)
            self.ext_modules.append(gksMovExt)

        # -- wx -------------------------------------
        if not self.disable_wx:
            inc = list(self.wxinc)
            inc.extend(gksinc)
            lib = list(self.wxlib)
            libs = list(self.wxlibs)
            ldflags = list(self.wxldflags)
            cflags = list(self.wxcxx)
            if self.isWin32:
                libs.extend(_libs_msvc)
                cflags.extend(_msvc_extra_compile_args)
                cflags.append("/DWXUSINGDLL")
                cflags.append("/DwxMSVC_VERSION_AUTO")
                cflags.append("/D_UNICODE")
            gksWxExt = Extension("gr.wxplugin",
                                 _plugins_path["wxplugin.cxx"],
                                 define_macros=defines,
                                 include_dirs=inc,
                                 library_dirs=lib,
                                 libraries=libs,
                                 extra_link_args=ldflags,
                                 extra_compile_args=cflags)
            self.ext_modules.append(gksWxExt)

        # -- qt4 -------------------------------------
        if not self.disable_qt4:
            inc = list(self.qt4inc)
            inc.extend(gksinc)
            lib = list(self.qt4lib)
            libs = list(self.qt4libs)
            ldflags = list(self.qt4ldflags)
            cflags = []
            if self.isWin32:
# do not use _msvc_extra_link_args because /nodefaultlib causes
# error LNK2019: unresolved external symbol ""__declspec(dllimport)
#                void __cdecl std::_Xbad_alloc(void)" ...
                ldflags.append("-dll")
                libs.extend(_libs_msvc)
                cflags.extend(_msvc_extra_compile_args)
            elif self.isDarwin:
                # check if Qt is built as a framework
                if lib and os.path.isdir(os.path.join(lib[0], 'QtCore.framework')):
                    libs = []
                    for l in self.qt5lib:
                        ldflags.extend(["-F", l])
                        cflags.extend(["-F", l])
                    ldflags.extend(["-framework", "QtGui",
                                    "-framework", "QtCore"])
            gksQt4Ext = Extension("gr.qtplugin",
                                  _plugins_path["qtplugin.cxx"],
                                  define_macros=defines,
                                  include_dirs=inc,
                                  library_dirs=lib,
                                  libraries=libs,
                                  extra_link_args=ldflags,
                                  extra_compile_args=cflags)
            self.ext_modules.append(gksQt4Ext)

        # -- qt5 -------------------------------------
        if not self.disable_qt5:
            inc = list(self.qt5inc)
            inc.extend(gksinc)
            lib = list(self.qt5lib)
            libs = list(self.qt5libs)
            ldflags = list(self.qt5ldflags)
            cflags = ["-std=c++11"]
            if self.isWin32:
# do not use _msvc_extra_link_args because /nodefaultlib causes
# error LNK2019: unresolved external symbol ""__declspec(dllimport)
#                void __cdecl std::_Xbad_alloc(void)" ...
                ldflags.append("-dll")
                libs.extend(_libs_msvc)
                cflags.extend(_msvc_extra_compile_args)
            elif self.isDarwin:
                # check if Qt is built as a framework
                if lib and os.path.isdir(os.path.join(lib[0], 'QtCore.framework')):
                    libs = []
                    for l in self.qt5lib:
                        ldflags.extend(["-F", l])
                        cflags.extend(["-F", l])
                    ldflags.extend(["-framework", "QtWidgets",
                                    "-framework", "QtGui",
                                    "-framework", "QtCore"])
                ldflags.append("-stdlib=libc++")
                cflags.append("-stdlib=libc++")
            gksQt5Ext = Extension("gr.qt5plugin",
                                  _plugins_path["qt5plugin.cxx"],
                                  define_macros=defines,
                                  include_dirs=inc,
                                  library_dirs=lib,
                                  libraries=libs,
                                  extra_link_args=ldflags,
                                  extra_compile_args=cflags)
            self.ext_modules.append(gksQt5Ext)

        # -- gtk -------------------------------------
        if not self.disable_gtk:
            inc = list(gksinc)
#                inc.extend(self.x11inc)
            ldflags = list(self.gtkldflags)
            cflags = list(self.gtkcflags)
            if self.isWin32:
                libs.extend(_libs_msvc)
                cflags.extend(_msvc_extra_compile_args)
            gksGtkExt = Extension("gr.gtkplugin",
                                  _plugins_path["gtkplugin.cxx"],
                                  define_macros=defines,
                                  include_dirs=inc,
                                  extra_compile_args=cflags,
                                  extra_link_args=ldflags)
            self.ext_modules.append(gksGtkExt)

        # -- quartz -------------------------------------
        if not self.disable_quartz:
            inc = list(gksinc)
            libs = ["objc"]
            ldflags = ["-framework", "Foundation",
                       "-framework", "ApplicationServices",
                       "-framework", "AppKit"]
            ldflags.extend(self.platform_ldflags)
            gksQuartzExt = Extension("gr.quartzplugin",
                                     _plugins_path["quartzplugin.m"],
                                     define_macros=defines,
                                     include_dirs=inc,
                                     libraries=libs,
                                     extra_link_args=ldflags)
            self.ext_modules.append(gksQuartzExt)

        # -- GR -------------------------------------
        inc = list(gksinc)
        inc.extend(zinc)
        inc.extend(pnginc)
        inc.extend(jpeginc)
        inc.extend(self.mupdfinc)
        inc.extend(self.x11inc) # at least because this includes wrong png.h
        lib = list(self.x11lib)
        lib.append(_build_lib_grpkg)
        lib.append(_build_3rdparty_lib)
        if self.isLinuxOrDarwin:
            libs = ["GKS"]
        else:
            libs = ["libGKS"]
            lib.append(os.path.join(_build_temp, "Release", "lib", "gks"))
        libs.extend(self.x11libs)
        libs.extend(self.mupdflibs)
        ldflags = list(self.x11ldflags)
        ldflags.extend(self.ftldflags)
        # important: lib ordering png, jpeg, z
        staticlibs = [_libqhull, _libpng, _libjpeg, _libz]
        ldflags.extend(self.mupdfldflags)
        ldflags.extend(staticlibs)
        cflags = list(self.x11cflags)
        if self.isWin32:
            libs.extend(_libs_msvc)
            cflags.extend(_msvc_extra_compile_args)
#                ldflags.extend(_msvc_extra_link_args)
        ldflags.extend(self.platform_ldflags)
        if self.isDarwin:
            ldflags.append("-Wl,-install_name,@rpath/libGR.so")
        grExt = Extension("gr.libGR", _gr_src_path,
                          define_macros=defines,
                          include_dirs=inc,
                          library_dirs=lib,
                          libraries=libs,
                          extra_link_args=ldflags,
                          extra_compile_args=cflags)
        self.ext_modules.append(grExt)
        grinc = inc
        grlib = lib
        grlibs = libs

        # -- GR3 -------------------------------------
        if not self.disable_opengl:
            inc = list(grinc)
            inc.append(os.path.join("lib", "gr"))
            lib = list(grlib)
            libs = list(grlibs)
            libs.extend(self.gllibs)
            ldflags = list(self.x11ldflags)
            ldflags.extend(self.mupdfldflags)
            ldflags.extend(self.platform_ldflags)
            ldflags.extend(self.glldflags)
            # important: lib ordering png, jpeg, z
            staticlibs = [_libpng, _libjpeg, _libz]
            ldflags.extend(staticlibs)
            if self.isLinuxOrDarwin:
                libs.append("GR")
            if self.isDarwin:
                ldflags.append("-Wl,-install_name,@rpath/libGR3.so")
                ldflags.append("-Wl,-rpath,@loader_path/../gr/.")
            elif self.isLinux:
                ldflags.append("-Wl,-rpath,$ORIGIN/../gr")
            elif self.isWin32:
                inc.append("3rdparty")
                lib.append(os.path.join(_build_temp, "Release", "lib",
                                        "gks"))
                lib.append(os.path.join(_build_temp, "Release", "lib",
                                        "gr"))
                libs.append("libGR")
                libs.extend(_libs_msvc)
                cflags.extend(_msvc_extra_compile_args)
                ldflags.append("-dll")
            gr3Ext = Extension("gr3.libGR3", _gr3_src_path,
                               define_macros=defines,
                               include_dirs=inc,
                               library_dirs=lib,
                               libraries=libs,
                               extra_link_args=ldflags,
                               extra_compile_args=cflags)
            self.ext_modules.append(gr3Ext)


class build_ext(_build_ext, check_ext):

    user_options = (_build_ext.user_options + check_ext.user_options)

    # workaround for libGKS on win32 no "init" + module_name function
    def get_export_symbols(self, ext):
        """Return the list of symbols that a shared extension has to
        export. This overloaded function does not append "init" + module_name
        to the exported symbols (default in superclass function).

        """
        return ext.export_symbols

    # workaround do not use pyd files in this project because we have our
    # own ctypes wrapper. Use dll files instead.
    # Replace "cpython-34m" -> get_config_var("SOABI") because we are not
    # providing a real python extension.
    def get_ext_filename(self, ext_name):
        r"""Convert the name of an extension (eg. "foo.bar") into the name
        of the file from which it will be loaded (eg. "foo/bar.so", or
        "foo\bar.dll not foo\bar.pyd (default in superclass function)").
        Remove SOABI part from the filename because no python extension will be
        provided. Instead a ctypes wrapper will be used.

        """
        ret = _build_ext.get_ext_filename(self, ext_name)
        (_pathNoExt, ext) = os.path.splitext(ret)
        if ext == ".pyd":
            ext = ".dll"
        return ret.replace(get_config_var("SO"), ext)

    def initialize_options(self):
        check_ext.initialize_options(self)
        _build_ext.initialize_options(self)

    def finalize_options(self):
        check_ext.finalize_options(self)
        _build_ext.finalize_options(self)
        self.extensions = self.ext_modules

    def run(self):
        check_ext.run(self)
        _build_ext.run(self)
        if not self.disable_quartz:
            os.system("xcodebuild MACOSX_DEPLOYMENT_TARGET=" +
                      self.macosx_deployment_target +
                      " -project lib/gks/quartz/GKSTerm.xcodeproj")
            os.system("ditto lib/gks/quartz/build/Release/GKSTerm.app " +
                      os.path.join(_build_lib_grpkg, "GKSTerm.app"))


class build_py(_build_py):

    def _get_data_files(self):
        data = _build_py._get_data_files(self)
        # special case: search also in gks for gr data files.
        # (needed for gksfonts)
        package = "gr"
        gks_src = self.get_package_dir("gks")
        # Compute package build directory
        build_dir = os.path.join(*([self.build_lib] + package.split('.')))
        # Length of path to strip from found files
        plen = len(gks_src) + 1
        # Strip directory from globbed filenames
        filenames = [
            file[plen:] for file in self.find_data_files(package, gks_src)
        ]
        data.append((package, gks_src, build_dir, filenames))
        return data


class run_tests(build_ext):

    def run(self):
        build_ext.run(self)
        sys.path.insert(0, os.path.join("lib", "gr", "python"))
        os.environ["GRLIB"] = os.path.join(_build_lib, "gr")
        import nose
        import gr
        print("gr version:", gr.__version__)
        print()
        nose.run(argv=[sys.argv[0], "tests.gr", "-v"])


# check wether this is a Debian based system, e.g. Ubuntu
if "deb_system" in distutils.command.install.INSTALL_SCHEMES:
    # Overwrite all site-packages paths with dist-packages in INSTALL_SCHEMES
    # on Debian based systems, e.g. Ubuntu, because site-packages is not in
    # sys.path (PYTHONPATH) per default.
    # Without this patch installation with prefix shows following
    # unexpected behavior:
    # - packages will be installed into dist-packages
    # - data_files will be installed into site-packages (!)
    #   instead of dist-packages (as expected).
    # Installation without prefix works as expected.
    INSTALL_SCHEMES = distutils.command.install.INSTALL_SCHEMES
    for k, d in INSTALL_SCHEMES.items():
        for kk, v in d.items():
            INSTALL_SCHEMES[k][kk] = v.replace("site-packages", "dist-packages")

# unique platform id used by distutils
_uPlatformId = "%s-%d.%d" % (get_platform(), sys.version_info[0],
                             sys.version_info[1])
_build_lib = os.path.join("build", "lib." + _uPlatformId)
_build_lib_grpkg = os.path.join(_build_lib, "gr")
_build_3rdparty = os.path.join("build", "3rdparty." + _uPlatformId)
_build_3rdparty_lib = os.path.join(_build_3rdparty, "lib")
_build_temp = os.path.join("build", "temp." + _uPlatformId)

# prerequisites: static 3rdparty libraries
if sys.platform == "win32":
    # if linking against png, jpeg and zlib use this ordering!
    _libpng = os.path.join(_build_3rdparty_lib, "png.lib")
    _libjpeg = os.path.join(_build_3rdparty_lib, "jpeg.lib")
    _libz = os.path.join(_build_3rdparty_lib, "z.lib")
    _libqhull = os.path.join(_build_3rdparty_lib, "qhull.lib")

    _libs_msvc = ["msvcrt", "oldnames", "kernel32", "wsock32", "advapi32",
                  "user32", "gdi32", "comdlg32", "winspool"]
    # w32ERR: __imp_ -> fixed by linking against oldnames.lib
    _msvc_extra_compile_args = ["/Zi", "/D_DLL", "/D_POSIX", "/nologo"]
    _msvc_extra_link_args = ["/nodefaultlib", "-dll"]
else:
    _libz = os.path.join(_build_3rdparty_lib, "libz.a")
    _libpng = os.path.join(_build_3rdparty_lib, "libpng.a")
    _libjpeg = os.path.join(_build_3rdparty_lib, "libjpeg.a")
    _libqhull = os.path.join(_build_3rdparty_lib, "libqhull.a")

    _libs_msvc = []
    _msvc_extra_compile_args = []
    _msvc_extra_link_args = []

_gks_src = ["gks.c", "gksforbnd.c", "font.c", "afm.c", "util.c", "ft.c", "dl.c",
            "malloc.c", "error.c", "mf.c", "wiss.c", "cgm.c", "win.c", "mac.c",
            "ps.c", "pdf.c", "x11.c", "socket.c", "plugin.c", "compress.c",
            "io.c"]
_gks_plugin_src = ["font.cxx", "afm.cxx", "util.cxx", "dl.cxx",
                   "malloc.cxx", "error.cxx", "io.cxx"]
_gks_plugins = ["wxplugin.cxx", "qtplugin.cxx", "qt5plugin.cxx",
                "gtkplugin.cxx", "quartzplugin.m", "svgplugin.cxx",
                "figplugin.cxx", "gsplugin.cxx", "wmfplugin.cxx",
                "movplugin.cxx", "htmplugin.cxx", "pgfplugin.cxx",
                "cairoplugin.cxx"]

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

_libqhull_src = ["global.c", "stat.c", "geom2.c", "poly2.c", "merge.c",
                 "libqhull.c", "geom.c", "poly.c", "qset.c", "mem.c",
                 "random.c", "usermem.c", "userprintf.c", "io.c", "user.c",
                 "rboxlib.c", "userprintf_rbox.c"]

_gr_src = ["gr.c", "text.c", "contour.c", "spline.c", "gridit.c", "strlib.c",
           "io.c", "image.c", "md5.c", "import.c", "grforbnd.c", "delaunay.c",
           "interp2.c", "meta.c"
          ]

_gr3_src = ["gr3.c", "gr3_convenience.c", "gr3_html.c", "gr3_povray.c",
            "gr3_png.c", "gr3_jpeg.c", "gr3_gr.c", "gr3_mc.c", "gr3_slices.c"]

if sys.platform == "darwin":
    os.environ["ARCHFLAGS"] = os.getenv("ARCHFLAGS", "-arch x86_64")
    os.environ["LDSHARED"] = get_config_var("LDSHARED").replace("-bundle",
                                                                "-dynamiclib")
    _gr3_src.insert(0, "gr3_cgl.c")
elif "linux" in sys.platform:
    _gr3_src.insert(0, "gr3_glx.c")
elif sys.platform == "win32":
    _gr3_src.insert(0, "gr3_win.c")

_gks_src_path = list(map(lambda p: os.path.join("lib", "gks", p), _gks_src))
_gks_plugin_src_path = list(map(lambda p: os.path.join("lib", "gks", "plugin",
                                                       p), _gks_plugin_src))
_libz_src_path = list(map(lambda p: os.path.join("3rdparty", "zlib", p),
                          _libz_src))
_libpng_src_path = list(map(lambda p: os.path.join("3rdparty", "libpng16", p),
                            _libpng_src))
_libjpeg_src_path = list(map(lambda p: os.path.join("3rdparty", "jpeg", p),
                             _libjpeg_src))
_libqhull_src_path = list(map(lambda p: os.path.join("3rdparty", "qhull", p),
                              _libqhull_src))
_gr_src_path = list(map(lambda p: os.path.join("lib", "gr", p), _gr_src))
_gr3_src_path = list(map(lambda p: os.path.join("lib", "gr3", p), _gr3_src))

_plugins_path = {}
for plugin_src in _gks_plugins:
    _plugins_path[plugin_src] = list(_gks_plugin_src_path)
    _plugins_path[plugin_src].append(os.path.join("lib", "gks", "plugin",
                                                  plugin_src))


# -- setup -----------------------------------------------------------------
_long_description = None
try:
    with open("README.rst", 'r') as fd:
        _long_description = fd.read()
except IOError as e:
    print("WARNING: long_description could not be read from file. Error message was:\n",
          e, file=sys.stderr)


classifiers = [
    'Framework :: IPython',
    'Intended Audience :: Science/Research',
    'License :: OSI Approved :: MIT License',
    'Operating System :: MacOS :: MacOS X',
    'Operating System :: Microsoft :: Windows',
    'Operating System :: POSIX :: Linux',
    'Programming Language :: Python',
    'Programming Language :: Python :: 2',
    'Programming Language :: Python :: 3',
    'Topic :: Multimedia :: Graphics',
    'Topic :: Scientific/Engineering :: Visualization',
    ]


setup(cmdclass={"build_ext": build_ext, "check_ext": check_ext,
                "build_static": build_static, "build_py": build_py,
                "clean_static": clean_static, "clean": clean,
                "tests": run_tests},
      name="gr",
      version=__version__,
      description="Python visualization framework",
      long_description=_long_description,
      author="Scientific IT Systems",
      author_email="j.heinen@fz-juelich.de",
      maintainer="Josef Heinen",
      license="MIT License",
      url="http://gr-framework.org",
      package_dir={'': "lib/gr/python",
                   "gr3": "lib/gr3/python/gr3",
                   "gks": "lib/gks" # needed for gksfonts in gr package
                  },
      requires=["numpy"],
      packages=["gr", "gr.pygr", "gr.matplotlib", "gr3",
                "qtgr", "qtgr.events"],
      platforms=["Linux", "OS X", "Windows"],
      # ext_modules dummy entry
      # check_ext dynamically generates a list of Extensions
      ext_modules=[Extension("", [""])],
      package_data={
          "gr": ["fonts/*.afm", "fonts/*.pfb", "fonts/gksfont.dat"],
      },
      classifiers=classifiers,
     )
