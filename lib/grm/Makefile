ifeq ($(strip $(THIRDPARTYDIR)),)
override THIRDPARTYDIR = $(abspath $(CURDIR)/../../3rdparty/build)
endif

       GRDIR = /usr/local/gr
      LIBDIR = $(DESTDIR)$(GRDIR)/lib
      INCDIR = $(DESTDIR)$(GRDIR)/include

UNAME := $(shell uname)

     GRMOBJS = src/grm/args.o \
               src/grm/base64.o \
               src/grm/dump.o \
               src/grm/dynamic_args_array.o \
               src/grm/error.o \
               src/grm/event.o \
               src/grm/interaction.o \
               src/grm/json.o \
               src/grm/logging.o \
               src/grm/memwriter.o \
               src/grm/net.o \
               src/grm/plot.o \
               src/grm/util.o \
               src/grm/datatype/double_map.o \
               src/grm/datatype/string_list.o \
               src/grm/datatype/string_array_map.o \
               src/grm/datatype/string_map.o \
               src/grm/datatype/uint_map.o
     DEFINES =
    INCLUDES = -I./include \
               -I./src \
               -I../gks \
               -I../gr \
               -I../gr3 \
               -I$(THIRDPARTYDIR)/include
          CC = cc
      CFLAGS = $(DEFINES) -std=c90 -O3 -Wall -fPIC -fvisibility=hidden $(EXTRA_CFLAGS)
    CXXFLAGS = $(DEFINES) -std=c++17 -O3 -Wall -fPIC -fvisibility=hidden $(EXTRA_CXXFLAGS)
         AR ?= ar
     RANLIB ?= ar ts
ifeq ($(UNAME), Darwin)
GR_SHARED_LIBRARY_SUFFIX ?= .dylib
     CFLAGS += -mmacosx-version-min=10.14
   CXXFLAGS += -mmacosx-version-min=10.14
INSTALL_NAME = -Wl,-install_name,@rpath/libGRM${GR_SHARED_LIBRARY_SUFFIX}
     LDFLAGS = $(EXTRA_LDFLAGS) -Wl,-rpath,@loader_path/. -mmacosx-version-min=10.14
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

.cxx.o:
	$(CXX) -o $@ -c $(INCLUDES) $(CXXFLAGS) $<

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
	$(CXX) -o $@ $(SOFLAGS) $(GRMOBJS) $(LIBS) $(LDFLAGS) $(INSTALL_NAME)

install:
	@if [ ! -d $(DESTDIR)$(GRDIR) ]; then mkdir -m 755 $(DESTDIR)$(GRDIR); fi
	@if [ ! -d $(LIBDIR) ]; then mkdir -m 755 $(LIBDIR); fi
	cp -p libGRM.* $(LIBDIR)
	@if [ ! -d $(INCDIR) ]; then mkdir -m 755 $(INCDIR); fi
	cp -p include/grm.h $(INCDIR)
	cp -rp include/grm $(INCDIR)/

clean:
	rm -f *.so \
	      *.a \
	      src/grm/*.o \
	      src/grm/datatype/*.o \
	      *${GR_SHARED_LIBRARY_SUFFIX}

.PHONY: all targets install clean prerequisites libGR libGR3
.SUFFIXES:  # Delete the default suffixes
.SUFFIXES: .c .cxx .o  # List all suffixes for suffix rules explicitly
