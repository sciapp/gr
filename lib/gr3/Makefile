       GRDIR = /usr/local/gr
      LIBDIR = $(DESTDIR)$(GRDIR)/lib
      INCDIR = $(DESTDIR)$(GRDIR)/include
   PYTHONDIR = $(LIBDIR)/python

    INCLUDES = -I../../3rdparty/png -I../../3rdparty/jpeg -I../gr
      GRLIBS = -L../gr -lGR
      GLLIBS = -lGL -lX11
     PNGLIBS = ../../3rdparty/png/libpng.a
    JPEGLIBS = ../../3rdparty/jpeg/libjpeg.a
       ZLIBS = -lz
      CFLAGS = -O3 -Wall -Wextra -pedantic -g -fPIC $(EXTRA_CFLAGS)
     SOFLAGS = -shared
INSTALL_NAME = -Wl,-install_name,$(LIBDIR)/libGR3.so
     LDFLAGS = $(EXTRA_LDFLAGS)

default: gr3.c gr3.h
	make `uname`
Linux:
	make targets INSTALL_NAME="" LDFLAGS="-Wl,-rpath=$(LIBDIR)" \
	GR3_PLATFORM_SPECIFIC_C="gr3_glx.c"
Darwin:
	make targets SOFLAGS="-dynamiclib" \
	GLLIBS="-framework OpenGL -framework Cocoa" SOFLAGS="-dynamiclib" \
	GR3_PLATFORM_SPECIFIC_C="gr3_cgl.c"

targets: prerequisites libGR3.so

prerequisites:
	@make -C ../gr

libGR3.so: gr3.c $(GR3_PLATFORM_SPECIFIC_C) gr3_convenience.c gr3_html.c gr3_povray.c gr3_png.c gr3_jpeg.c gr3_gr.c gr3_mc.c
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
	rm -f *.pyc
