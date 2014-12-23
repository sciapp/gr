       GRDIR = /usr/local/gr
      BINDIR = $(DESTDIR)$(GRDIR)/bin
      LIBDIR = $(DESTDIR)$(GRDIR)/lib
      INCDIR = $(DESTDIR)$(GRDIR)/include
   PYTHONDIR = $(LIBDIR)/python
    JULIADIR = $(LIBDIR)/julia

      GROBJS = gr.o text.o contour.o spline.o gridit.o strlib.o io.o image.o \
               md5.o import.o grforbnd.o
   MUPDFDEFS = 
     DEFINES = $(MUPDFDEFS)
    INCLUDES = -I../gks -I../../3rdparty/jpeg -I../../3rdparty/png
          CC = cc
      CFLAGS = $(DEFINES) -O3 -Wall -fPIC $(EXTRA_CFLAGS)
          AR = ar
      RANLIB = ar ts
     SOFLAGS = -shared
INSTALL_NAME = -Wl,-install_name,$(LIBDIR)/libGR.so
     LDFLAGS = $(EXTRA_LDFLAGS)
        LIBS = -lc -lm
ifeq ($(MUPDFDEFS), -DNO_MUPDF)
    JPEGLIBS = ../../3rdparty/jpeg/libjpeg.a
else
    JPEGLIBS = 
endif
     PNGLIBS = ../../3rdparty/png/libpng.a
     GKSLIBS = ../gks/libGKS.a
      FTLIBS = `$(FT_CONFIG) --libs 2>/dev/null`
     XFTLIBS = -lXft -lfontconfig
   MUPDFLIBS = -lmupdf $(FTLIBS) -lbz2 -ljbig2dec -ljpeg -lopenjp2
     X11LIBS = $(X11PATH) $(XFTLIBS) -lXt -lX11
       ZLIBS = -lz

.c.o:
	$(CC) -c $(INCLUDES) $(CFLAGS) $<

default:
	@make -C ../.. Makedefs
	@make `cat ../../Makedefs`

Linux:
	@make targets \
	INSTALL_NAME="" LDFLAGS="-Wl,-rpath=$(LIBDIR)"
Darwin:
	@make targets LD="$(CC)" \
	SOFLAGS="-dynamiclib"

targets: prerequisites libGR.a libGR.so

prerequisites:
	@for d in jpeg png; do make -C ../../3rdparty/$$d; done
	@for d in gks gks/plugin; do make -C ../$$d; done

libGR.a: $(GROBJS)
	$(AR) crv $@ $?
	$(RANLIB) $@

libGR.so: $(GROBJS)
	$(CC) -o $@ $(SOFLAGS) $(GROBJS) $(JPEGLIBS) $(PNGLIBS) $(GKSLIBS) \
	$(MUPDFLIBS) $(ZLIBS) $(X11LIBS) $(LIBS) $(LDFLAGS) \
	$(INSTALL_NAME)

install:
	@make -C ../gks install
	@if [ ! -d $(DESTDIR)$(GRDIR) ]; then mkdir -m 755 $(DESTDIR)$(GRDIR); fi
	@if [ ! -d $(BINDIR) ]; then mkdir -m 755 $(BINDIR); fi
	@if [ ! -d $(LIBDIR) ]; then mkdir -m 755 $(LIBDIR); fi
	cp -p libGR.* $(LIBDIR)
	@if [ ! -d $(PYTHONDIR) ]; then mkdir -m 755 $(PYTHONDIR); fi
	cp -a python/gr $(PYTHONDIR)
	cp -a python/qtgr $(PYTHONDIR)
	@if [ ! -d $(JULIADIR) ]; then mkdir -m 755 $(JULIADIR); fi
	cp -p julia/* $(JULIADIR)
	@if [ ! -d $(INCDIR) ]; then mkdir -m 755 $(INCDIR); fi
	cp -p gr.h $(INCDIR)

clean:
	@for d in gks gks/plugin; do make -C ../$$d clean; done
	@for d in jpeg png; do make -C ../../3rdparty/$$d clean; done
	rm -f so_locations *.so *.a *.o *.bak *~

depend:
	makedepend -Y -- gr.c text.c contour.c spline.c gridit.c strlib.c io.c \
	image.c md5.c import.c grforbnd.c 2> /dev/null

# DO NOT DELETE THIS LINE -- make depend depends on it.

gr.o: gr.h text.h spline.h contour.h strlib.h io.h md5.h
contour.o: gr.h contour.h
spline.o: spline.h
gridit.o: gridit.h
strlib.o: strlib.h
io.o: io.h
image.o: gr.h
md5.o: md5.h
import.o: gr.h
grforbnd.o: gr.h
