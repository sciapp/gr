       GRDIR = /usr/local/gr
      BINDIR = $(DESTDIR)$(GRDIR)/bin
      LIBDIR = $(DESTDIR)$(GRDIR)/lib
      INCDIR = $(DESTDIR)$(GRDIR)/include
     FONTDIR = $(DESTDIR)$(GRDIR)/fonts

UNAME := $(shell uname)

     GKSOBJS = gks.o gksforbnd.o font.o afm.o util.o ft.o dl.o malloc.o \
               error.o mf.o wiss.o cgm.o win.o mac.o ps.o pdf.o x11.o socket.o \
               plugin.o compress.o io.o

      GSDEFS =
   CAIRODEFS =
     X11DEFS =
     XFTDEFS =
      FTDEFS =
       FTINC = -I../../3rdparty/freetype/include
ifeq ($(UNAME), Darwin)
      X11INC = -I/opt/X11/include
else
      X11INC = -I/usr/X11R6/include
endif
          CC = cc
     DEFINES = -DHAVE_ZLIB $(GSDEFS) $(CAIRODEFS) $(X11DEFS) $(XFTDEFS) $(FTDEFS)
      CFLAGS = -O3 -Wall -fPIC $(EXTRA_CFLAGS)
          LD = cc
          AR = ar
      RANLIB = ar ts
ifeq ($(UNAME), Darwin)
INSTALL_NAME = -Wl,-install_name,@rpath/libGKS.so
     LDFLAGS = $(EXTRA_LDFLAGS) -Wl,-rpath,@loader_path/.
     SOFLAGS = -dynamiclib
else
INSTALL_NAME =
     LDFLAGS = $(EXTRA_LDFLAGS) -Wl,-rpath,'$$ORIGIN/.'
     SOFLAGS = -shared
endif
        LIBS = -lpthread -ldl -lc -lm
      FTLIBS = ../../3rdparty/freetype/libfreetype.a
     XFTLIBS = -lXft -lfontconfig
     X11LIBS = $(X11PATH) $(XFTLIBS) -lXt -lX11
        ZINC = -I../../3rdparty/zlib
       ZLIBS = ../../3rdparty/zlib/libz.a

.c.o:
	$(CC) -c -DGRDIR=\"$(GRDIR)\" $(DEFINES) $(FTINC) $(X11INC) $(ZINC) $(CFLAGS) $<

default:
	@make -C ../.. Makedefs
	@make `cat ../../Makedefs`

Linux:
	@make targets
Darwin:
ifeq ($(CC),cc)
	@make targets CFLAGS="$(CFLAGS) -fpascal-strings"
else
	@make targets
endif

targets: prerequisites libGKS.a libGKS.so demo gksm plugins

prerequisites:
	@for d in bzip2 freetype jpeg libpng16 zlib; do make -C ../../3rdparty/$$d; done

libGKS.a: $(GKSOBJS)
	$(AR) crv $@ $?
	$(RANLIB) $@

libGKS.so: $(GKSOBJS)
	$(CC) -o $@ $(SOFLAGS) $(GKSOBJS) $(FTLIBS) $(ZLIBS) $(X11LIBS) \
	$(LIBS) $(LDFLAGS) $(INSTALL_NAME)

demo: demo.c libGKS.a
	$(CC) -o demo $(LDFLAGS) demo.c libGKS.a $(FTLIBS) $(ZLIBS) $(X11LIBS) $(LIBS)

gksm: gksm.c libGKS.a
	$(CC) -o gksm $(LDFLAGS) gksm.c libGKS.a $(FTLIBS) $(ZLIBS) $(X11LIBS) $(LIBS)

plugins:
	@make -C plugin

pasdemo: default
	fpc -Px86_64 demo.p -g -Fl./ -k-rpath -k./

install:
	@if [ ! -d $(DESTDIR)$(GRDIR) ]; then mkdir -m 755 $(DESTDIR)$(GRDIR); fi
	@if [ ! -d $(LIBDIR) ]; then mkdir -m 755 $(LIBDIR); fi
	cp -p libGKS.* $(LIBDIR)
	cp -p plugin/*plugin.so $(LIBDIR)
	@if [ ! -d $(BINDIR) ]; then mkdir -m 755 $(BINDIR); fi
	cp -p gksm $(BINDIR)
	@if [ ! -d $(FONTDIR) ]; then mkdir -m 755 $(FONTDIR); fi
	cp -a fonts/* $(FONTDIR)
	@if [ ! -d $(INCDIR) ]; then mkdir -m 755 $(INCDIR); fi
	cp -p gks.h $(INCDIR)

clean:
	@make -C plugin clean
	rm -f demo gksm pasdemo so_locations *.so *.a *.o *.ppu link.res
	rm -f *.bak *~

depend:
	makedepend -Y -- \
	gks.c gksforbnd.c font.c afm.c util.c dl.c malloc.c error.c \
	mf.c wiss.c cgm.c win.c mac.c ps.c pdf.c x11.c socket.c plugin.c \
	compress.c io.c 2> /dev/null

# DO NOT DELETE THIS LINE -- make depend depends on it.

gks.o: gks.h gkscore.h
gksforbnd.o: gks.h gksforbnd.h
font.o: gks.h gkscore.h
afm.o: gkscore.h
util.o: gks.h gkscore.h
dl.o: gks.h gkscore.h
malloc.o: gkscore.h
error.o: gkscore.h
mf.o: gkscore.h gks.h
wiss.o: gks.h gkscore.h
cgm.o: cgm.h gks.h gkscore.h
win.o: gks.h gkscore.h
mac.o: gks.h gkscore.h
ps.o: gks.h gkscore.h
pdf.o: gks.h gkscore.h
x11.o: gks.h gkscore.h icon.bm
socket.o: gks.h gkscore.h
plugin.o: gkscore.h
compress.o: gkscore.h
io.o: gkscore.h
