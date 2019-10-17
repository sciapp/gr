ifeq ($(strip $(THIRDPARTYDIR)),)
override THIRDPARTYDIR = $(abspath $(CURDIR)/../../3rdparty/build)
endif

            GRDIR = /usr/local/gr
           BINDIR = $(DESTDIR)$(GRDIR)/bin
           LIBDIR = $(DESTDIR)$(GRDIR)/lib
           INCDIR = $(DESTDIR)$(GRDIR)/include
          FONTDIR = $(DESTDIR)$(GRDIR)/fonts

UNAME := $(shell uname)

     GKSOBJS = gks.o gksforbnd.o font.o afm.o util.o dl.o malloc.o \
               error.o mf.o wiss.o win.o ps.o pdf.o socket.o \
               plugin.o compress.o io.o ft.o resample.o

      GSDEFS =
          CC = cc
     DEFINES = -DHAVE_ZLIB $(GSDEFS)
      CFLAGS = -O3 -Wall -fPIC $(EXTRA_CFLAGS)
          LD = cc
          AR = ar
      RANLIB = ar ts
ifeq ($(UNAME), Darwin)
	 CFLAGS += -mmacosx-version-min=10.11
INSTALL_NAME = -Wl,-install_name,@rpath/libGKS.so
     LDFLAGS = $(EXTRA_LDFLAGS) -Wl,-rpath,@loader_path/. -mmacosx-version-min=10.11
     SOFLAGS = -dynamiclib
else
INSTALL_NAME =
     LDFLAGS = $(EXTRA_LDFLAGS) -Wl,-rpath,'$$ORIGIN/.'
     SOFLAGS = -shared
endif
        LIBS = -lpthread -ldl -lc -lm
      FTDEFS =
       FTINC = -I$(THIRDPARTYDIR)/include
      FTLIBS = $(THIRDPARTYDIR)/lib/libfreetype.a
        ZINC = -I$(THIRDPARTYDIR)/include
       ZLIBS = $(THIRDPARTYDIR)/lib/libz.a

.c.o:
	$(CC) -c -DGRDIR=\"$(GRDIR)\" $(DEFINES) $(FTINC) $(ZINC) $(CFLAGS) $<

default:
	$(MAKE) -C ../.. Makedefs
	$(MAKE) `cat ../../Makedefs`

all: targets

targets: libGKS.a libGKS.so demo gksm plugins gksqt

ifeq ($(UNAME), Darwin)
targets: gksterm
endif

$(THIRDPARTYDIR)/lib/libz.a:
	$(MAKE) -C ../../3rdparty/zlib PREFIX=$(THIRDPARTYDIR)

$(THIRDPARTYDIR)/lib/libfreetype.a:
	$(MAKE) -C ../../3rdparty/freetype PREFIX=$(THIRDPARTYDIR)

$(THIRDPARTYDIR)/lib/libzmq.a:
	$(MAKE) -C ../../3rdparty/zeromq PREFIX=$(THIRDPARTYDIR)

libGKS.a: $(GKSOBJS)
	$(AR) crv $@ $?
	$(RANLIB) $@

libGKS.so: $(GKSOBJS) $(THIRDPARTYDIR)/lib/libfreetype.a $(THIRDPARTYDIR)/lib/libz.a
	$(CC) -o $@ $(SOFLAGS) $(GKSOBJS) $(FTLIBS) $(ZLIBS) $(LIBS) \
	$(LDFLAGS) $(INSTALL_NAME)

demo: demo.c libGKS.a $(THIRDPARTYDIR)/lib/libz.a
	$(CC) -o demo $(LDFLAGS) demo.c libGKS.a $(ZLIBS) $(LIBS)

gksm: gksm.c libGKS.a $(THIRDPARTYDIR)/lib/libz.a
	$(CC) -o gksm $(LDFLAGS) gksm.c libGKS.a $(ZLIBS) $(LIBS)

gksqt:
	$(MAKE) -C qt -f makefile.mak GRDIR=$(GRDIR)

ifeq ($(UNAME), Darwin)
gksterm: $(THIRDPARTYDIR)/lib/libzmq.a
	(env CC=cc xcodebuild -project quartz/GKSTerm.xcodeproj)
endif

ifeq ($(UNAME), Darwin)
plugins: $(THIRDPARTYDIR)/lib/libzmq.a
	$(MAKE) -C plugin
else
plugins:
	$(MAKE) -C plugin
endif

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
	$(MAKE) -C qt -f makefile.mak install
ifeq ($(UNAME), Darwin)
	@if [ ! -d $(DESTDIR)$(GRDIR)/Applications ]; then \
	mkdir -m 755 $(DESTDIR)$(GRDIR)/Applications; fi
	@ditto quartz/build/Release/GKSTerm.app \
	$(DESTDIR)$(GRDIR)/Applications/GKSTerm.app
endif

clean:
	$(MAKE) -C plugin clean
	$(MAKE) -C qt -f makefile.mak clean
	rm -f qt/.qmake.stash
	rm -f demo gksm pasdemo so_locations *.so *.a *.o *.ppu link.res
	rm -f *.bak *~
ifeq ($(UNAME), Darwin)
	(env CC=cc xcodebuild -project quartz/GKSTerm.xcodeproj clean)
endif
	cp -p quartz/project.pbxproj quartz/GKSTerm.xcodeproj/

depend:
	makedepend -Y -- \
	gks.c gksforbnd.c font.c afm.c util.c dl.c malloc.c error.c \
	mf.c wiss.c win.c ps.c pdf.c socket.c plugin.c \
	compress.c io.c ft.c resample.c 2> /dev/null

.PHONY: default all targets prerequisites plugins install clean depend

ft.o: $(THIRDPARTYDIR)/lib/libfreetype.a
pdf.o: $(THIRDPARTYDIR)/lib/libz.a

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
win.o: gks.h gkscore.h
ps.o: gks.h gkscore.h
pdf.o: gks.h gkscore.h
socket.o: gks.h gkscore.h
plugin.o: gkscore.h
compress.o: gkscore.h
io.o: gkscore.h
