       GRDIR = /usr/local/gr
      LIBDIR = $(DESTDIR)$(GRDIR)/lib
      INCDIR = $(DESTDIR)$(GRDIR)/include
   PYTHONDIR = $(LIBDIR)/python

UNAME := $(shell uname)

    INCLUDES = -I../../3rdparty -I../../3rdparty/jpeg -I../gr
      GRLIBS = -L../gr -lGR
     PNGLIBS = ../../3rdparty/libpng16/libpng.a
    JPEGLIBS = ../../3rdparty/jpeg/libjpeg.a
       ZLIBS = ../../3rdparty/zlib/libz.a
      CFLAGS = -O3 -Wall -Wextra -pedantic -g -fPIC $(EXTRA_CFLAGS)
ifeq ($(UNAME), Darwin)
      GLLIBS = -framework OpenGL -framework Cocoa
INSTALL_NAME = -Wl,-install_name,@rpath/libGR3.so
     LDFLAGS = $(EXTRA_LDFLAGS) -Wl,-rpath,@loader_path/.
     SOFLAGS = -dynamiclib
else
      GLLIBS = -lGL -lX11
INSTALL_NAME =
     LDFLAGS = $(EXTRA_LDFLAGS) -Wl,-rpath,'$$ORIGIN/.'
     SOFLAGS = -shared
endif

default: gr3.c gr3.h
	make `uname`
Linux:
	make targets \
	GR3_PLATFORM_SPECIFIC_C="gr3_glx.c"
Darwin:
	make targets \
	GR3_PLATFORM_SPECIFIC_C="gr3_cgl.c"

targets: prerequisites libGR3.so

prerequisites:
	@make -C ../gr

libGR3.so: gr3.c $(GR3_PLATFORM_SPECIFIC_C) gr3_convenience.c gr3_html.c gr3_povray.c gr3_png.c gr3_jpeg.c gr3_gr.c gr3_mc.c gr3_slices.c
	${CC} -o $@ $^ $(CFLAGS) $(INCLUDES) ${ZLIBS} $(JPEGLIBS) $(GRLIBS) $(GLLIBS) $(PNGLIBS) \
	$(SOFLAGS) $(LDFLAGS) $(INSTALL_NAME)

gr3.c: gr3.h gr3_internals.h
gr3_internals.h: gr3.h gr3_glx.h gr3_cgl.h
gr3_glx.c: gr3_glx.h
gr3_cgl.c: gr3_cgl.h
gr3_convenience.c: gr3_internals.h
gr3_html.c: gr3_internals.h
gr3_povray.c: gr3_internals.h
gr3_png.c: gr3_internals.h
gr3_jpeg.c: gr3_internals.h
gr3_gr.c: gr3_internals.h
gr3_mc.c: gr3.h gr3_mc_data.h
gr3_slices.c: gr3.h

install: targets
	@if [ ! -d $(DESTDIR)$(GRDIR) ]; then mkdir -m 755 $(DESTDIR)$(GRDIR); fi
	@if [ ! -d $(LIBDIR) ]; then mkdir -m 755 $(LIBDIR); fi
	cp -p libGR3.so $(LIBDIR)
	@if [ ! -d $(PYTHONDIR) ]; then mkdir -m 755 $(PYTHONDIR); fi
	cp -a python/gr3 $(PYTHONDIR)
	@if [ ! -d $(INCDIR) ]; then mkdir -m 755 $(INCDIR); fi
	cp -p gr3.h $(INCDIR)

clean:
	@make -C ../gr clean
	rm -rf libGR3.so*
	rm -f *.o
	rm -f *.pyc
