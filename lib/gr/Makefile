       GRDIR = /usr/local/gr
      BINDIR = $(DESTDIR)$(GRDIR)/bin
      LIBDIR = $(DESTDIR)$(GRDIR)/lib
      INCDIR = $(DESTDIR)$(GRDIR)/include
   PYTHONDIR = $(LIBDIR)/python
    JULIADIR = $(LIBDIR)/julia

UNAME := $(shell uname)

      GROBJS = gr.o text.o contour.o spline.o gridit.o strlib.o io.o image.o \
               delaunay.o interp2.o md5.o import.o meta.o grforbnd.o
   MUPDFDEFS =
      GSDEFS =
     DEFINES = $(MUPDFDEFS) $(GSDEFS)
    INCLUDES = -I../gks -I../../3rdparty/jpeg -I../../3rdparty
          CC = cc
      CFLAGS = $(DEFINES) -O3 -Wall -fPIC $(EXTRA_CFLAGS)
          AR = ar
      RANLIB = ar ts
ifeq ($(UNAME), Darwin)
INSTALL_NAME = -Wl,-install_name,@rpath/libGR.so
     LDFLAGS = $(EXTRA_LDFLAGS) -Wl,-rpath,@loader_path/.
     SOFLAGS = -dynamiclib
else
INSTALL_NAME =
     LDFLAGS = $(EXTRA_LDFLAGS) -Wl,-rpath,'$$ORIGIN/.'
     SOFLAGS = -shared
endif
        LIBS = -lpthread -ldl -lc -lm
    JPEGLIBS = ../../3rdparty/jpeg/libjpeg.a
     BZ2LIBS = ../../3rdparty/bzip2/libbz2.a
     PNGLIBS = ../../3rdparty/libpng16/libpng.a
     GKSLIBS = ../gks/libGKS.a
      FTLIBS = ../../3rdparty/freetype/libfreetype.a
     XFTLIBS = -lXft -lfontconfig
   MUPDFLIBS = -lmupdf -ljbig2dec $(JPEGLIBS) -lopenjp2 $(BZ2LIBS)
     SSLLIBS = -lssl -lcrypto
     X11LIBS = $(X11PATH) $(XFTLIBS) -lXt -lX11
       ZLIBS = ../../3rdparty/zlib/libz.a
      QHLIBS = ../../3rdparty/qhull/libqhull.a

.c.o:
	$(CC) -c $(INCLUDES) $(CFLAGS) $<

default:
	@make -C ../.. Makedefs
	@make `cat ../../Makedefs`

Linux:
	@make targets
Darwin:
	@make targets

targets: prerequisites libGR.a libGR.so demo libqtgr

prerequisites:
	@for d in bzip2 freetype jpeg libpng16 zlib qhull; do make -C ../../3rdparty/$$d; done
	@for d in gks gks/plugin; do make -C ../$$d; done

libGR.a: $(GROBJS)
	$(AR) crv $@ $?
	$(RANLIB) $@

libGR.so: $(GROBJS)
	$(CC) -o $@ $(SOFLAGS) $(GROBJS) $(PNGLIBS) $(GKSLIBS) $(MUPDFLIBS) \
	$(FTLIBS) $(JPEGLIBS) $(SSLLIBS) $(ZLIBS) $(QHLIBS) $(X11LIBS) $(LIBS) \
	$(LDFLAGS) $(INSTALL_NAME)

demo: demo.c libGR.a $(GKSLIBS)
	$(CC) -o demo $(INCLUDES) $(CFLAGS) demo.c libGR.a \
	$(PNGLIBS) $(GKSLIBS) $(MUPDFLIBS) $(FTLIBS) $(JPEGLIBS) $(SSLLIBS) \
	$(ZLIBS) $(QHLIBS) $(X11LIBS) $(LIBS) $(LDFLAGS)

libqtgr:
	@make -C qtgr

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
	@make -C qtgr install

clean:
	@make -C qtgr clean
	@for d in gks gks/plugin; do make -C ../$$d clean; done
	@for d in bzip2 freetype jpeg libpng16 zlib qhull; do make -C ../../3rdparty/$$d clean; done
	rm -f so_locations demo *.so *.a *.o *.bak *~

depend:
	makedepend -Y -- gr.c text.c contour.c spline.c gridit.c strlib.c io.c \
	image.c delaunay.c interp2.c md5.c import.c meta.c grforbnd.c \
        2> /dev/null

# DO NOT DELETE THIS LINE -- make depend depends on it.

gr.o: gr.h text.h spline.h gridit.h contour.h strlib.h io.h md5.h cm.h
contour.o: gr.h contour.h
spline.o: spline.h
gridit.o: gridit.h
strlib.o: strlib.h
io.o: gr.h io.h
image.o: gr.h
delaunay.o: gr.h
interp2.o: gr.h
md5.o: md5.h
import.o: gr.h
meta.o: gr.h
grforbnd.o: gr.h
