ifeq ($(strip $(THIRDPARTYDIR)),)
override THIRDPARTYDIR = $(abspath $(CURDIR)/../../3rdparty/build)
endif

       GRDIR = /usr/local/gr
      LIBDIR = $(DESTDIR)$(GRDIR)/lib
      INCDIR = $(DESTDIR)$(GRDIR)/include

UNAME := $(shell uname)

     GRMOBJS = grm.o
     DEFINES =
    INCLUDES = -I../gks -I../gr -I../gr3 -I$(THIRDPARTYDIR)/include
          CC = cc
      CFLAGS = $(DEFINES) -O3 -Wall -fPIC $(EXTRA_CFLAGS)
          AR = ar
      RANLIB = ar ts
ifeq ($(UNAME), Darwin)
     CFLAGS += -mmacosx-version-min=10.11
INSTALL_NAME = -Wl,-install_name,@rpath/libGRM.so
     LDFLAGS = $(EXTRA_LDFLAGS) -Wl,-rpath,@loader_path/. -mmacosx-version-min=10.11
     SOFLAGS = -dynamiclib
else
INSTALL_NAME =
     LDFLAGS = $(EXTRA_LDFLAGS) -Wl,-rpath,'$$ORIGIN/.'
     SOFLAGS = -shared
endif
      GRLIBS = -L ../gr/ -lGR
     GR3LIBS = -L ../gr3/ -lGR3
        LIBS = $(GRLIBS) $(GR3LIBS) -lm

.c.o:
	$(CC) -c $(INCLUDES) $(CFLAGS) $<

default:
	$(MAKE) -C ../.. Makedefs
	$(MAKE) `cat ../../Makedefs`

all: targets

targets: prerequisites libGRM.a libGRM.so

prerequisites:
	$(MAKE) -C ../../3rdparty/ PREFIX=$(THIRDPARTYDIR)

libGR:
	$(MAKE) -C ../gr

libGR3:
	$(MAKE) -C ../gr3

libGRM.a: $(GRMOBJS)
	$(AR) crv $@ $?
	$(RANLIB) $@

libGRM.so: $(GRMOBJS) libGR libGR3
	$(CC) -o $@ $(SOFLAGS) $(GRMOBJS) $(LIBS) $(LDFLAGS) $(INSTALL_NAME)

install:
	@if [ ! -d $(DESTDIR)$(GRDIR) ]; then mkdir -m 755 $(DESTDIR)$(GRDIR); fi
	@if [ ! -d $(LIBDIR) ]; then mkdir -m 755 $(LIBDIR); fi
	cp -p libGRM.* $(LIBDIR)
	@if [ ! -d $(INCDIR) ]; then mkdir -m 755 $(INCDIR); fi
	cp -p grm.h $(INCDIR)

clean:
	rm -f *.so *.a *.o

.PHONY: all targets install clean prerequisites libGR libGR3
