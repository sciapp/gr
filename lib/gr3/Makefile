     GRDIR = /usr/local/gr
    LIBDIR = $(GRDIR)/lib
    INCDIR = $(GRDIR)/include

  INCLUDES = -I../png -I$(GRDIR)/include
    GRLIBS = -L$(GRDIR)/lib -lGR
    GLLIBS = -lGL -lX11
   PNGLIBS = ../png/libpng.a
      LIBS = -lz
    CFLAGS = -Wall -Wextra -pedantic -g -fPIC
   LDFLAGS = -shared

default:
	make `uname`
Linux:
	make targets
Darwin:
	make targets GLLIBS="-framework OpenGL -framework Cocoa"

targets: prerequisites libGR3.so gr3.py

prerequisites:
	@make -C ../png

libGR3.so: gr3.c
	${CC} -o $@ $? $(CFLAGS) $(INCLUDES) $(GRLIBS) $(GLLIBS) $(PNGLIBS) \
	$(LIBS) $(LDFLAGS)

gr3.c: gr3.h

install: targets
	@if [ ! -d $(GRDIR) ]; then mkdir -m 755 $(GRDIR); fi
	@if [ ! -d $(LIBDIR) ]; then mkdir -m 755 $(LIBDIR); fi
	cp -p libGR3.so $(LIBDIR)
	cp -p gr3.py $(LIBDIR)
	@if [ ! -d $(INCDIR) ]; then mkdir -m 755 $(INCDIR); fi
	cp -p gr3.h $(INCDIR)

clean:
	@make -C ../png clean
	rm -rf libGR3.so*
