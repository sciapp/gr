ifeq ($(strip $(THIRDPARTYDIR)),)
override THIRDPARTYDIR = $(abspath $(CURDIR)/../../3rdparty/build)
endif

       GRDIR = /usr/local/gr
      LIBDIR = $(DESTDIR)$(GRDIR)/lib
      INCDIR = $(DESTDIR)$(GRDIR)/include
   GRMINCDIR = $(INCDIR)/grm

UNAME := $(shell uname)

     GRMOBJS = args.o base64.o dump.o dynamic_args_array.o error.o event.o interaction.o json.o logging.o memwriter.o \
               net.o plot.o util.o datatype/double_map.o datatype/string_list.o datatype/string_array_map.o \
               datatype/string_map.o datatype/uint_map.o
  GRMHEADERS = args.h dump.h event.h interaction.h net.h plot.h util.h
     DEFINES =
    INCLUDES = -I. -I../gks -I../gr -I../gr3 -I$(THIRDPARTYDIR)/include
          CC = cc
      CFLAGS = $(DEFINES) -O3 -Wall -fPIC -fvisibility=hidden $(EXTRA_CFLAGS)
         AR ?= ar
     RANLIB ?= ar ts
ifeq ($(UNAME), Darwin)
GR_SHARED_LIBRARY_SUFFIX ?= .dylib
     CFLAGS += -mmacosx-version-min=10.11
INSTALL_NAME = -Wl,-install_name,@rpath/libGRM${GR_SHARED_LIBRARY_SUFFIX}
     LDFLAGS = $(EXTRA_LDFLAGS) -Wl,-rpath,@loader_path/. -mmacosx-version-min=10.11
     SOFLAGS = -dynamiclib
else
GR_SHARED_LIBRARY_SUFFIX ?= .so
INSTALL_NAME =
     LDFLAGS = $(EXTRA_LDFLAGS) -Wl,-rpath,'$$ORIGIN/.'
     SOFLAGS = -shared
endif
      GRLIBS = -L ../gr/ -lGR
     GR3LIBS = -L ../gr3/ -lGR3
        LIBS = $(GRLIBS) $(GR3LIBS) -lm

.c.o:
	$(CC) -o $@ -c $(INCLUDES) $(CFLAGS) $<

default:
	$(MAKE) -C ../.. Makedefs
	$(MAKE) `cat ../../Makedefs`

all: targets

targets: prerequisites libGRM.a libGRM${GR_SHARED_LIBRARY_SUFFIX}

prerequisites:
	$(MAKE) -C ../../3rdparty/ PREFIX=$(THIRDPARTYDIR)

libGR:
	$(MAKE) -C ../gr

libGR3:
	$(MAKE) -C ../gr3

libGRM.a: $(GRMOBJS)
	$(AR) crv $@ $?
	$(RANLIB) $@

libGRM${GR_SHARED_LIBRARY_SUFFIX}: $(GRMOBJS) libGR libGR3
	$(CC) -o $@ $(SOFLAGS) $(GRMOBJS) $(LIBS) $(LDFLAGS) $(INSTALL_NAME)

install:
	@if [ ! -d $(DESTDIR)$(GRDIR) ]; then mkdir -m 755 $(DESTDIR)$(GRDIR); fi
	@if [ ! -d $(LIBDIR) ]; then mkdir -m 755 $(LIBDIR); fi
	cp -p libGRM.* $(LIBDIR)
	@if [ ! -d $(INCDIR) ]; then mkdir -m 755 $(INCDIR); fi
	cp -p grm.h $(INCDIR)
	@if [ ! -d $(GRMINCDIR) ]; then mkdir -m 755 $(GRMINCDIR); fi
	cp -p $(GRMHEADERS) $(GRMINCDIR)

clean:
	rm -f *.so *.a *.o datatype/*.o *${GR_SHARED_LIBRARY_SUFFIX}

.PHONY: all targets install clean prerequisites libGR libGR3
