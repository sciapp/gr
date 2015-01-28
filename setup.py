#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import with_statement, print_function

import sys
import os
import shlex
import re
import tempfile
import distutils.command
import distutils.command.install
from distutils.core import Command
from distutils.command.build_ext import build_ext as _build_ext
from distutils.command.build import build as _build
from distutils.command.clean import clean as _clean
from distutils.core import setup, Extension
from distutils.ccompiler import new_compiler
from distutils.sysconfig import get_config_var
from distutils.sysconfig import get_python_lib
from distutils.util import get_platform
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
import vcversioner


__author__ = "Christian Felder <c.felder@fz-juelich.de>"
__version__ = vcversioner.find_version(
                   version_module_paths=[os.path.join("lib", "gr", "python",
                                                      "gr",
                                                      "_version.py")]).version
__copyright__ = """Copyright 2012 - 2014 Forschungszentrum Juelich GmbH

This file is part of GR, a universal framework for visualization applications.
Visit http://gr-framework.org for the latest
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


class build_static(Command):

    # prerequisites: build static 3rdparty libraries
    description = "build static 3rdparty libraries"

    user_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        if sys.platform == "win32":
            _extra_preargs = list(_msvc_extra_compile_args)
        else:
            _extra_preargs = ["-fPIC"]
        if not os.path.isdir("build"):
            os.mkdir("build")
        if not os.path.isdir(_build_3rdparty):
            os.mkdir(_build_3rdparty)
        if not os.path.isdir(_build_temp):
            os.mkdir(_build_temp)
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
            obj = compiler.compile(_libjpeg_src_path,
                                   extra_preargs=_extra_preargs)
            compiler.create_static_lib(obj, "jpeg", output_dir=_build_3rdparty)
#        # create batch scripts
#        if sys.platform == "win32":
#            _grvarsallbat = os.path.join(_build_temp, "grvarsall.bat")
#            with open(_grvarsallbat, "wb") as fd:
#                paths = ["%PATH%"]
#                fd.write("@echo off\r\n")
#                if _wxlib:
#                    fd.write("set WXLIB=%s\r\n" %_wxlib)
#                    paths.append("%WXLIB%")
#                if _gsdir:
#                    fd.write("set GSDIR=%s\r\n" %_gsdir)
#                    paths.append("%GSDIR%\\bin")
#                fd.write("set PATH=%s\r\n" %(';'.join(paths)))


class clean_static(Command):

    description = "clean up temporary files from 'build_static' command"

    user_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        print("removing '", _build_3rdparty, "' (and everything under it)")
        try:
            map(lambda p: os.remove(os.path.join(_build_3rdparty, p)),
                os.listdir(_build_3rdparty))
            os.rmdir(_build_3rdparty)
        except OSError:
            pass


class clean(_clean, clean_static):

    def run(self):
        clean_static.run(self)
        _clean.run(self)
        if sys.platform == "darwin":
            os.system("xcodebuild -project lib/gks/quartz/GKSTerm.xcodeproj clean")


class check_ext(Command):

    description = "dynamically generate list of C/C++ extensions for build_ext"

    # List of option tuples: long name, short name (None if no short
    # name), and help string.
    user_options = [('disable-wx', None, "Disable wx plugin"),
                    ('disable-qt', None, "Disable qt plugin"),
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
                    ('disable-freetype', None,
                     "Disable freetype libraries (disables also mupdf)"),
                    ('disable-xft', None,
                     "Disable Xft libraries (disables also freetype)"),
                    ('disable-xt', None,
                     "Disable Xt libraries (disables also x11 and freetype)"),
                    ('disable-mupdf', None, "Disable mupdf libraries"),
                   ]

    def initialize_options(self):
        self.isLinuxOrDarwin = any(s in sys.platform for s in
                                   ["linux", "darwin"])
        self.isLinux = ("linux" in sys.platform)
        self.isDarwin = (sys.platform == "darwin")
        self.isWin32 = (sys.platform == "win32")
        # extensions list
        self.ext_modules = []
        # -- user options -------------------------------------
        self.disable_wx = False
        self.disable_qt = False
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
        # -- environment -------------------------------------
        self.cc = os.getenv("CC", "cc")
        self.grdir = os.getenv("GRDIR",
                               os.path.join(get_python_lib(plat_specific=True,
                                                           prefix=''), "gr"))
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
        # -- wx -------------------------------------
        self.wxconfig = os.getenv("WX_CONFIG")
        self.wxdir = os.getenv("WXDIR")
        self.wxlib = [os.getenv("WXLIB")] if os.getenv("WXLIB") else []
        self.wxinc = []
        self.wxlibs = []
        self.wxldflags = []
        self.wxcxx = []
        # -- qt -------------------------------------
        if not self.isWin32:
            self.qtdir = os.getenv("QTDIR", "/usr/local")
        else:
            self.qtdir = os.getenv("QTDIR", "")
        self.qtinc = [os.path.join(self.qtdir, "include")]
        self.qtlib = [os.path.join(self.qtdir, "lib")]
        self.qtlibs = []
        self.qtldflags = []
        self.qmake = ""
        self.qtversion = None
        qmake = os.path.join(self.qtdir, "bin", "qmake")
        if self.isWin32:
            qmake += ".exe"
        if os.path.isfile(qmake):
            self.qmake = qmake
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

    def _test_gs(self, gsinc=[], gslibs=[], gscflags=[], gsldflags=[]):
        cflags = ["-I" + i for i in gsinc]
        ldflags = ["-l" + l for l in gslibs]
        (fd, tmpsrc) = tempfile.mkstemp(suffix=".c", prefix="a")
        (_fd2, tmpout) = tempfile.mkstemp(suffix=".out", prefix="a")
        os.write(fd, b"""#include <stdio.h>
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
        cmd = [self.cc, "-o", tmpout, tmpsrc]
        cmd.extend(cflags)
        cmd.extend(gscflags)
        cmd.extend(ldflags)
        cmd.extend(gsldflags)
        gscc = Popen(cmd, stdout=PIPE, stderr=STDOUT)
        gscc.communicate()
        try:
            os.remove(tmpsrc)
            os.remove(tmpout)
        except OSError:
            pass
        return gscc.returncode == 0

    def _test_mupdf(self, mupdfinc=[], mupdflibs=[], mupdfcflags=[],
                    mupdfldflags=[]):
        cflags = ["-I" + i for i in mupdfinc]
        ldflags = ["-l" + l for l in mupdflibs]
        (fd, tmpsrc) = tempfile.mkstemp(suffix=".c", prefix="a")
        (_fd2, tmpout) = tempfile.mkstemp(suffix=".out", prefix="a")
        os.write(fd, b"""#include <mupdf/fitz.h>

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

""")
        os.close(fd)
        cmd = [self.cc, "-o", tmpout, tmpsrc]
        cmd.extend(cflags)
        cmd.extend(mupdfcflags)
        cmd.extend(ldflags)
        cmd.extend(mupdfldflags)
        gscc = Popen(cmd, stdout=PIPE, stderr=STDOUT)
        gscc.communicate()
        try:
            os.remove(tmpsrc)
            os.remove(tmpout)
        except OSError:
            pass
        return gscc.returncode == 0

    def _test_mov(self, mupdfinc=[], mupdflibs=[], mupdfcflags=[],
                  mupdfldflags=[]):
        cflags = ["-I" + i for i in mupdfinc]
        ldflags = ["-l" + l for l in mupdflibs]
        (fd, tmpsrc) = tempfile.mkstemp(suffix=".c", prefix="a")
        (_fd2, tmpout) = tempfile.mkstemp(suffix=".out", prefix="a")
        os.write(fd, b"""#include <stdio.h>
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

""")
        os.close(fd)
        cmd = [self.cc, "-o", tmpout, tmpsrc]
        cmd.extend(cflags)
        cmd.extend(mupdfcflags)
        cmd.extend(ldflags)
        cmd.extend(mupdfldflags)
        gscc = Popen(cmd, stdout=PIPE, stderr=STDOUT)
        gscc.communicate()
        try:
            os.remove(tmpsrc)
            os.remove(tmpout)
        except OSError:
            pass
        return gscc.returncode == 0

    def _test_opengl(self, glinc=[], gllibs=[], glcflags=[], glldflags=[]):
        cflags = ["-I" + i for i in glinc]
        ldflags = ["-l" + l for l in gllibs]
        (fd, tmpsrc) = tempfile.mkstemp(suffix=".c", prefix="a")
        (_fd2, tmpout) = tempfile.mkstemp(suffix=".out", prefix="a")
        os.write(fd, b"""#include <stdio.h>
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

""")
        os.close(fd)
        cmd = [self.cc, "-o", tmpout, tmpsrc]
        cmd.extend(cflags)
        cmd.extend(glcflags)
        cmd.extend(ldflags)
        cmd.extend(glldflags)
        gscc = Popen(cmd, stdout=PIPE, stderr=STDOUT)
        gscc.communicate()
        try:
            os.remove(tmpsrc)
            os.remove(tmpout)
        except OSError:
            pass
        return gscc.returncode == 0

    def _test_xft(self, xftinc=[], xftlib=[], xftlibs=[], xftcflags=[],
                  xftldflags=[]):
        cflags = ["-I" + i for i in xftinc]
        ldflags = ["-L" + ld for ld in xftlib]
        ld = ["-l" + l for l in xftlibs]
        ldflags.extend(ld)
        (fd, tmpsrc) = tempfile.mkstemp(suffix=".c", prefix="a")
        (_fd2, tmpout) = tempfile.mkstemp(suffix=".out", prefix="a")
        os.write(fd, b"""#include <stdio.h>
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

""")
        os.close(fd)
        cmd = [self.cc, "-o", tmpout, tmpsrc]
        cmd.extend(cflags)
        cmd.extend(xftcflags)
        cmd.extend(ldflags)
        cmd.extend(xftldflags)
        gscc = Popen(cmd, stdout=PIPE, stderr=STDOUT)
        gscc.communicate()
        try:
            os.remove(tmpsrc)
            os.remove(tmpout)
        except OSError:
            pass
        return gscc.returncode == 0

    def _test_xt(self, xtinc=[], xtlib=[], xtlibs=[], xtcflags=[],
                 xtldflags=[]):
        cflags = ["-I" + i for i in xtinc]
        ldflags = ["-L" + ld for ld in xtlib]
        ld = ["-l" + l for l in xtlibs]
        ldflags.extend(ld)
        (fd, tmpsrc) = tempfile.mkstemp(suffix=".c", prefix="a")
        (_fd2, tmpout) = tempfile.mkstemp(suffix=".out", prefix="a")
        os.write(fd, b"""#include <stdio.h>
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

""")
        os.close(fd)
        cmd = [self.cc, "-o", tmpout, tmpsrc]
        cmd.extend(cflags)
        cmd.extend(xtcflags)
        cmd.extend(ldflags)
        cmd.extend(xtldflags)
        gscc = Popen(cmd, stdout=PIPE, stderr=STDOUT)
        gscc.communicate()
        try:
            os.remove(tmpsrc)
            os.remove(tmpout)
        except OSError:
            pass
        return gscc.returncode == 0

    def _test_freetype(self, ftinc=[], ftlibs=[], ftcflags=[], ftldflags=[]):
        cflags = ["-I" + i for i in ftinc]
        ldflags = ["-l" + l for l in ftlibs]
        (fd, tmpsrc) = tempfile.mkstemp(suffix=".c", prefix="a")
        (_fd2, tmpout) = tempfile.mkstemp(suffix=".out", prefix="a")
        os.write(fd, b"""#include <stdio.h>
#include <stdlib.h>

#include <ft2build.h>

int main()
{
  exit(0);
}

""")
        os.close(fd)
        cmd = [self.cc, "-o", tmpout, tmpsrc]
        cmd.extend(cflags)
        cmd.extend(ftcflags)
        cmd.extend(ldflags)
        cmd.extend(ftldflags)
        gscc = Popen(cmd, stdout=PIPE, stderr=STDOUT)
        gscc.communicate()
        try:
            os.remove(tmpsrc)
            os.remove(tmpout)
        except OSError:
            pass
        return gscc.returncode == 0

    def _finalize_LinuxOrDarwinPost(self):
        pass

    def _finalize_Win32Post(self):
        # -- qt -------------------------------------
        if not self.disable_qt:
            self.qtlibs = ["QtGui%d" % self.qtversion[0],
                           "QtCore%d" % self.qtversion[0]]

    def _finalize_LinuxOrDarwinPre(self):
        # -- wx -------------------------------------
        if not self.disable_wx and not self.wxconfig:
            self.wxconfig = Popen(["which", "wx-config"],
                              stdout=PIPE).communicate()[0].rstrip()
            if not os.path.isfile(self.wxconfig):
                self.disable_wx = True
            else:
                self.wxldflags = shlex.split(Popen([self.wxconfig, "--libs"],
                                stdout=PIPE).communicate()[0].rstrip())
                self.wxcxx = shlex.split(Popen([self.wxconfig, "--cxxflags"],
                               stdout=PIPE).communicate()[0].rstrip())
        # -- qt -------------------------------------
        if not self.disable_qt:
            if not self.qmake:
                self.qmake = Popen(["which", "qmake"],
                               stdout=PIPE).communicate()[0].rstrip()
            if os.path.isdir("/usr/local/Cellar"):
                self.qtlibs = []
            else:
                self.qtlibs = ["QtGui", "QtCore"]
        # -- x11 -------------------------------------
        if self.isLinux:
            x11ldflags = None
            try:
                x11ldflags = shlex.split(Popen(["pkg-config", "x11", "--libs"],
                   stdout=PIPE, stderr=PIPE).communicate()[0].rstrip())
            except OSError:
                pass
            if x11ldflags:
                self.x11ldflags = x11ldflags
                self.x11cflags = shlex.split(Popen(["pkg-config", "x11",
                                                    "--cflags"], stdout=PIPE,
                               stderr=PIPE).communicate()[0].rstrip())
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
            ftconfig = Popen(["which", "freetype-config"],
                             stdout=PIPE).communicate()[0].rstrip()
            self.disable_freetype = not bool(ftconfig)
            if not self.disable_freetype:
                self.ftconfig = ftconfig
                self.ftldflags = shlex.split(Popen([self.ftconfig, "--libs"],
                               stdout=PIPE).communicate()[0].rstrip())
                self.ftcflags = shlex.split(Popen([self.ftconfig, "--cflags"],
                              stdout=PIPE).communicate()[0].rstrip())
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
                gtkcflags = shlex.split(Popen(["pkg-config", self.gtk_package,
                                               "--cflags"], stdout=PIPE,
                              stderr=PIPE).communicate()[0].rstrip())
            except OSError:
                pass
            if gtkcflags:
                self.gtkcflags = gtkcflags
                self.gtkldflags = shlex.split(Popen(["pkg-config",
                                     self.gtk_package, "--libs"], stdout=PIPE,
                                stderr=PIPE).communicate()[0].rstrip())
            else:
                self.disable_gtk = True
        # -- mupdf -------------------------------------
        if not self.disable_mupdf:
            for p in [os.path.join(os.sep, "usr", "local", "include"),
                      os.path.join(os.sep, "usr", "include")]:
                mupdf = os.path.join(p, "mupdf")
                if os.path.isdir(mupdf):
                    self.mupdfinc = [p]
                    break
            if not self.mupdfinc:
                self.disable_mupdf = True
            else:
                # mupdf compiled without ssl support
                self.mupdflibs = ["mupdf", "jbig2dec", "jpeg", "openjp2"]
                self.mupdfldflags = self.ftldflags
                if self.isDarwin:
                    self.mupdfldflags.append("-L/usr/local/lib")
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
                if self.disable_mupdf:
                    self.mupdfinc = []
                    self.mupdflibs = []
                    self.mupdfldflags = []
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
            self.platform_ldflags = ["-mmacosx-version-min=10.6",
                                     "-Wl,-rpath,@loader_path/."]
        elif self.isLinux:
            self.platform_ldflags = ["-Wl,-rpath,$ORIGIN"]

    def _finalize_Win32Pre(self):
        # -- wx -------------------------------------
        if not (self.wxdir and self.wxlib):
            self.disable_wx = True
        else:
            self.wxlibs = ["wxmsw29ud_core", "wxbase29ud"]
            self.wxinc = [os.path.join(self.wxdir, "include", "msvc"),
                          os.path.join(self.wxdir, "include")]
        # -- qt -------------------------------------
        if not self.disable_qt:
            # do not use _msvc_extra_link_args because /nodefaultlib causes
            # error LNK2019: unresolved external symbol ""__declspec(dllimport)
            #                void __cdecl std::_Xbad_alloc(void)" ...
            self.qtldflags = ["-dll"]
        # -- gs -------------------------------------
        if not self.disable_gs and self.gsdir:
            self.gsinc = [os.path.join(self.gsdir, "include")]
            self.gslib = [os.path.join(self.gsdir, "bin")]
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
        # -- platform finalizers (pre) -------------------------------------
        if self.isLinuxOrDarwin:
            self._finalize_LinuxOrDarwinPre()
        elif self.isWin32:
            self._finalize_Win32Pre()

        # -- platform independend finalizers -------------------------------
        # qt
        if not self.disable_qt:
            if not os.path.isfile(self.qmake):
                self.disable_qt = True
            else:
                qtversion = Popen([self.qmake, "-v"], stdout=PIPE,
                              stderr=STDOUT).communicate()[0].rstrip()
                match = re.search("\d\.\d\.\d", qtversion)
                if match:
                    self.qtversion = list(map(lambda s: int(s),
                                              match.group(0).split('.')))
                if not self.qtversion or self.qtversion[0] < 4:
                    self.disable_qt = True
                else:
                    qquery = Popen([self.qmake, "-query", "QT_INSTALL_HEADERS"],
                                    stdout=PIPE)
                    std = qquery.communicate()
                    if qquery.returncode == 0:
                        self.qtinc = [std[0].rstrip()]
                    qquery = Popen([self.qmake, "-query", "QT_INSTALL_LIBS"],
                                    stdout=PIPE)
                    std = qquery.communicate()
                    if qquery.returncode == 0:
                        self.qtlib = [std[0].rstrip()]

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
        if self.isLinuxOrDarwin or self.isWin32:
            print(" isLinuxOrDarwin: ", self.isLinuxOrDarwin)
            print("         isLinux: ", self.isLinux)
            print("        isDarwin: ", self.isDarwin)
            print("         isWin32: ", self.isWin32)
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
            print("           qmake: ", self.qmake)
            print("           qtdir: ", self.qtdir)
            print("           qtinc: ", self.qtinc)
            print("           qtlib: ", self.qtlib)
            print("          qtlibs: ", self.qtlibs)
            print("       qtldflags: ", self.qtldflags)
            print("      Qt version: ", self.qtversion)
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
            print("")
            print("      opengllibs: ", self.gllibs)
            print("    opengldflags: ", self.glldflags)
            print("")
            print("     disable-x11: ", self.disable_x11)
            print("      disable-xt: ", self.disable_xt)
            print("     disable-xft: ", self.disable_xft)
            print("      disable-wx: ", self.disable_wx)
            print("      disable-qt: ", self.disable_qt)
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
            print("")

            zinc = [os.path.join("3rdparty", "zlib")]
            defines = [("HAVE_ZLIB",), ("GRDIR", "\"%s\"" % self.grdir)]
            if self.disable_x11:
                defines.append(("NO_X11", 1))
            if self.disable_freetype:
                defines.append(("NO_FT", 1))
            if self.disable_mupdf:
                defines.append(("NO_MUPDF", 1))

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
                cflags.append("-mmacosx-version-min=10.6")
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
            pnginc = [os.path.join("3rdparty", "png")]
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
                ldflags.extend(self.platform_ldflags)
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

            # -- svg -------------------------------------
            if not self.disable_svg:
                inc = list(gksinc)
                inc.extend(pnginc)
                inc.extend(zinc)
                lib = [_build_3rdparty]
                libs = list(_pnglibs) # w32ERR: __imp_
                libs.extend(_zlibs)   # w32ERR: __imp_
                ldflags = []
                cflags = []
                ldflags.extend(self.platform_ldflags)
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
                lib = [_build_3rdparty]
                libs = list(_pnglibs) # w32ERR: __imp_
                libs.extend(_zlibs)   # w32ERR: __imp_
                ldflags = []
                cflags = []
                ldflags.extend(self.platform_ldflags)
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
                lib = [_build_3rdparty]
                libs = list(_pnglibs) # w32ERR: __imp_
                libs.extend(_zlibs)   # w32ERR: __imp_
                ldflags = []
                cflags = []
                ldflags.extend(self.platform_ldflags)
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
                lib = [_build_3rdparty]
                libs = list(_pnglibs) # w32ERR: __imp_
                libs.extend(_zlibs)   # w32ERR: __imp_
                ldflags = []
                cflags = []
                ldflags.extend(self.platform_ldflags)
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
                ldflags.extend(self.platform_ldflags)
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
                    ffmpeglibs.extend(["theora", "ogg", "pvx"])
                libs.extend(_zlibs)
                libs.append("pthread")
                ldflags = list(self.ftldflags)
                cflags = list(self.ftcflags)
                ldflags.extend(self.platform_ldflags)
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
                ldflags.extend(self.platform_ldflags)
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

            # -- qt -------------------------------------
            if not self.disable_qt:
                inc = list(self.qtinc)
                inc.extend(gksinc)
                lib = list(self.qtlib)
                libs = list(self.qtlibs)
                ldflags = list(self.qtldflags)
                cflags = []
                ldflags.extend(self.platform_ldflags)
                if self.isWin32:
# do not use _msvc_extra_link_args because /nodefaultlib causes
# error LNK2019: unresolved external symbol ""__declspec(dllimport)
#                void __cdecl std::_Xbad_alloc(void)" ...
                    ldflags.append("-dll")
                    libs.extend(_libs_msvc)
                    cflags.extend(_msvc_extra_compile_args)
                gksQtExt = Extension("gr.qtplugin",
                                     _plugins_path["qtplugin.cxx"],
                                     define_macros=defines,
                                     include_dirs=inc,
                                     library_dirs=lib,
                                     libraries=libs,
                                     extra_link_args=ldflags,
                                     extra_compile_args=cflags)
                self.ext_modules.append(gksQtExt)

            # -- gtk -------------------------------------
            if not self.disable_gtk:
                inc = list(gksinc)
#                inc.extend(self.x11inc)
                ldflags = list(self.gtkldflags)
                cflags = list(self.gtkcflags)
                ldflags.extend(self.platform_ldflags)
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
            lib.append(_build_3rdparty)
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
            staticlibs = [_libpng, _libjpeg, _libz]
            ldflags.extend(staticlibs)
            cflags = list(self.x11cflags)
            cflags.extend(self.ftcflags)
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
                ldflags.extend(self.ftldflags)
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
        else:
            print("Platform \"", sys.platform, "\" is not supported.",
                  file=sys.stderr)
            sys.exit(-3)


class build_ext(_build_ext, check_ext, build_static):

    # workaround for libGKS on win32 no "init" + module_name function
    def get_export_symbols (self, ext):
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
        build_static.initialize_options(self)
        _build_ext.initialize_options(self)

    def finalize_options(self):
        check_ext.finalize_options(self)
        build_static.initialize_options(self)
        _build_ext.finalize_options(self)
        self.extensions = self.ext_modules

    def run(self):
        check_ext.run(self)
        build_static.run(self)
        _build_ext.run(self)
        if not self.disable_quartz:
            os.system("xcodebuild -project lib/gks/quartz/GKSTerm.xcodeproj")
            os.system("ditto lib/gks/quartz/build/Release/GKSTerm.app " +
                      os.path.join(_build_scripts, "GKSTerm.app"))


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
_build_temp = os.path.join("build", "temp." + _uPlatformId)
_build_scripts = os.path.join("build", "scripts-%d.%d" % (sys.version_info[0],
                                                          sys.version_info[1]))

# additional data files in the distribtion
_gks_fonts = os.path.join("lib", "gks", "fonts")
_gks_fonts_path = list(map(lambda f: os.path.join(_gks_fonts, f),
                           os.listdir(_gks_fonts)))
_gks_fonts_path.append(os.path.join("lib", "gks", "gksfont.dat"))
_data_files = [(os.path.join(get_python_lib(plat_specific=True, prefix=''),
                             "gr", "fonts"), _gks_fonts_path)]

# prerequisites: static 3rdparty libraries
if sys.platform == "win32":
    # if linking against png, jpeg and zlib use this ordering!
    _libpng = os.path.join(_build_3rdparty, "png.lib")
    _libjpeg = os.path.join(_build_3rdparty, "jpeg.lib")
    _libz = os.path.join(_build_3rdparty, "z.lib")

    _libs_msvc = ["msvcrt", "oldnames", "kernel32", "wsock32", "advapi32",
                  "user32", "gdi32", "comdlg32", "winspool"]
    # w32ERR: __imp_ -> fixed by linking against oldnames.lib
    _msvc_extra_compile_args = ["/Zi", "/D_DLL", "/D_POSIX", "/nologo"]
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
                "gsplugin.cxx", "wmfplugin.cxx", "movplugin.cxx",
                "htmplugin.cxx", "pgfplugin.cxx"]

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
            "gr3_png.c", "gr3_jpeg.c", "gr3_gr.c", "gr3_mc.c"]

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
_libpng_src_path = list(map(lambda p: os.path.join("3rdparty", "png", p),
                            _libpng_src))
_libjpeg_src_path = list(map(lambda p: os.path.join("3rdparty", "jpeg", p),
                             _libjpeg_src))
_gr_src_path = list(map(lambda p: os.path.join("lib", "gr", p), _gr_src))
_gr3_src_path = list(map(lambda p: os.path.join("lib", "gr3", p), _gr3_src))

_plugins_path = {}
for plugin_src in _gks_plugins:
    _plugins_path[plugin_src] = list(_gks_plugin_src_path)
    _plugins_path[plugin_src].append(os.path.join("lib", "gks", "plugin",
                                                  plugin_src))

_zlibs = ["z"]
_pnglibs = ["png"]
_jpeglibs = ["jpeg"]


# -- setup -----------------------------------------------------------------
# [OSX] Install GSKTerm.app.
if sys.platform == "darwin":
    _scripts = [os.path.join(_build_scripts, "GKSTerm.app")]
else:
    _scripts = None

setup(cmdclass={"build_ext": build_ext, "check_ext": check_ext,
                "build_static": build_static, "clean_static": clean_static,
                "clean": clean, "tests": run_tests},
      name="gr",
      version=__version__,
      description="Python visualization framework",
      long_description="""
        GR is a universal framework for cross-platform visualization
        applications. It offers developers a compact, portable and consistent
        graphics library for their programs. Applications range from
        publication quality 2D graphs to the representation of complex 3D
        scenes.
        """,
      author="Scientific IT-Systems",
      author_email="j.heinen@fz-juelich.de",
      license="GNU General Public License",
      url="http://gr-framework.org",
      package_dir={'': "lib/gr/python",
                   "gr3": "lib/gr3/python/gr3"},
      requires=["numpy"],
      packages=["gr", "gr.pygr", "gr.matplotlib", "gr3",
                "qtgr", "qtgr.events"],
      # ext_modules dummy entry
      # check_ext dynamically generates a list of Extensions
      ext_modules=[Extension("", [""])],
      scripts=_scripts,
      data_files=_data_files)
