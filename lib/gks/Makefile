       GRDIR = /usr/local/gr
      BINDIR = $(GRDIR)/bin
      LIBDIR = $(GRDIR)/lib
      INCDIR = $(GRDIR)/include
     FONTDIR = $(GRDIR)/fonts

     GKSOBJS = gks.o gksforbnd.o font.o afm.o util.o dl.o malloc.o error.o \
               mf.o wiss.o cgm.o win.o mac.o ps.o pdf.o x11.o socket.o \
               plugin.o compress.o io.o

     DEFINES = -DHAVE_ZLIB -DXFT
    INCLUDES = -I/usr/X11R6/include
 XFTINCLUDES = -I/usr/include/freetype2
          CC = cc
      CFLAGS = -O -Wall -fPIC
          LD = cc
     LDFLAGS =
          AR = ar
      RANLIB = ar ts
     SOFLAGS = -shared
        LIBS = -lpthread -ldl -lc -lm
     XFTLIBS = -lXft -lfontconfig
       XLIBS = $(XFTLIBS) -L/usr/X11R6/lib -lXt -lX11
       ZLIBS = -lz

.c.o:
	$(CC) -c $(CFLAGS) -DGRDIR=\"$(GRDIR)\" $(DEFINES) $(INCLUDES) $(XFTINCLUDES) $<

default:
	@make `uname`

targets: libGKS.a libGKS.so demo gksm plugins

Linux:
	@make targets
Darwin:
	@make targets CFLAGS="$(CFLAGS) -fpascal-strings" \
	XFTINCLUDES="-I/usr/X11/include/freetype2"
	LD="$(CC)" LDFLAGS="-framework Carbon" \
	SOFLAGS="-dynamiclib -framework Carbon -Wl,-install_name,$(LIBDIR)/libGKS.so"

libGKS.a: $(GKSOBJS)
	$(AR) crv $@ $?
	$(RANLIB) $@

libGKS.so: $(GKSOBJS)
	$(LD) -o $@ $(SOFLAGS) $(GKSOBJS) $(ZLIBS) $(XLIBS) $(LIBS)

demo: demo.c libGKS.a
	$(CC) -o demo $(LDFLAGS) demo.c libGKS.a $(ZLIBS) $(XLIBS) $(LIBS)

gksm: gksm.c libGKS.a
	$(CC) -o gksm $(LDFLAGS) gksm.c libGKS.a $(ZLIBS) $(XLIBS) $(LIBS)

plugins:
	@make -C plugin

pasdemo: default
	fpc demo.p -g -Fl./

install:
	@if [ ! -d $(GRDIR) ]; then mkdir -m 755 $(GRDIR); fi
	@if [ ! -d $(LIBDIR) ]; then mkdir -m 755 $(LIBDIR); fi
	cp -p libGKS.* $(LIBDIR)
	cp -p plugin/*plugin.so $(LIBDIR)
	@if [ ! -d $(BINDIR) ]; then mkdir -m 755 $(BINDIR); fi
	cp -p gksm $(BINDIR)
	@if [ ! -d $(FONTDIR) ]; then mkdir -m 755 $(FONTDIR); fi
	cp -p gksfont.dat $(FONTDIR)
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
