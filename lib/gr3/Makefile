       GRDIR = /usr/local/gr
      LIBDIR = $(GRDIR)/lib
      INCDIR = $(GRDIR)/include
   PYTHONDIR = $(LIBDIR)/python

    INCLUDES = -I../png -I../jpeg -I../gr
      GRLIBS = -L../gr -lGR
      GLLIBS = -lGL -lX11
     PNGLIBS = ../png/libpng.a
    JPEGLIBS = ../jpeg/libjpeg.a
       ZLIBS = -lz
      CFLAGS = -Wall -Wextra -pedantic -g -fPIC
     SOFLAGS = -shared
INSTALL_NAME = -Wl,-install_name,$(LIBDIR)/libGR3.so
     LDFLAGS =

default: gr3.c gr3.h
	make `uname`
Linux:
	make targets INSTALL_NAME="" LDFLAGS="-Wl,-rpath=$(LIBDIR)"
Darwin:
	make targets SOFLAGS="-dynamiclib" \
	GLLIBS="-framework OpenGL -framework Cocoa" SOFLAGS="-dynamiclib"

targets: prerequisites libGR3.so gr3.py

prerequisites:
	@make -C ../gr

libGR3.so: gr3.c
	${CC} -o $@ $? $(CFLAGS) $(INCLUDES) ${ZLIBS} $(GRLIBS) $(GLLIBS) $(PNGLIBS) \
	$(JPEGLIBS) $(SOFLAGS) $(LDFLAGS) $(INSTALL_NAME)

gr3.c: gr3.h

install: targets
	@if [ ! -d $(GRDIR) ]; then mkdir -m 755 $(GRDIR); fi
	@if [ ! -d $(LIBDIR) ]; then mkdir -m 755 $(LIBDIR); fi
	cp -p libGR3.so $(LIBDIR)
	@if [ ! -d $(PYTHONDIR) ]; then mkdir -m 755 $(PYTHONDIR); fi
	cp -p gr3.py $(PYTHONDIR)
	@if [ ! -d $(INCDIR) ]; then mkdir -m 755 $(INCDIR); fi
	cp -p gr3.h $(INCDIR)

clean:
	@make -C ../gr clean
	rm -rf libGR3.so*
	rm -f *.pyc
