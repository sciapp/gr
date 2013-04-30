#!/usr/bin/env python
# -*- coding: utf-8 -*-

from distutils.core import setup, Extension
import os
import sys

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

#os.environ["CFLAGS"] = "-fPIC"
#os.environ["LDFLAGS"] = "-L/usr/X11R6/lib -dynamiclib"

#DISTUTILS_USE_SDK 
os.environ["MACOSX_DEPLOYMENT_TARGET"] = "10.8"

_gks_src = ["gks.c", "gksforbnd.c", "font.c", "afm.c", "util.c", "ft.c", "dl.c",
            "malloc.c", "error.c", "mf.c", "wiss.c", "cgm.c", "win.c", "mac.c",
            "ps.c", "pdf.c", "x11.c", "socket.c", "plugin.c", "compress.c",
            "io.c"]

_gks_src_path = map(lambda p: os.path.join("lib/gks", p), _gks_src)

_gks_includes = ["/usr/X11R6/include"]

if sys.platform == "darwin":
    _gks_xftincludes = ["/usr/X11/include/freetype2"]
#    os.environ["CFLAGS"] += " -fpascal-strings"
else:
    _gks_xftincludes = ["/usr/include/freetype2"]
    
_gks_include_dirs = list(_gks_includes)
_gks_include_dirs.extend(_gks_xftincludes)

_gks_libs = ["pthread", "dl", "c", "m"]
_gks_xftlibs = ["Xft", "freetype", "fontconfig"]
_gks_xlibs = list(_gks_xftlibs)
_gks_xlibs.extend(["Xt", "X11"])
_gks_zlibs = ["z"]

_gks_libraries = list(_gks_libs)
_gks_libraries.extend(_gks_zlibs)
_gks_libraries.extend(_gks_xlibs)

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
      ext_modules=[
                   Extension("libGKS", _gks_src_path,
                             define_macros=[("HAVE_ZLIB", ), ("XFT", ),
                                            ("GRDIR", "\"%s\""
                                             %os.getenv("GRDIR",
                                                        "/usr/local/gr"))],
                             include_dirs=_gks_include_dirs,
                             libraries=_gks_libraries,
#                             extra_compile_args=["-Wimplicit-function-declaration"],
                             extra_link_args=["-L/usr/X11R6/lib"]
                             )
                   
                   ]
      )