ifeq ($(strip $(PREFIX)),)
override PREFIX = $(abspath $(CURDIR)/../build)
endif

 FTDEFINES = -DFT2_BUILD_LIBRARY -DDARWIN_NO_CARBON \
             '-DFT_CONFIG_MODULES_H="slimftmodules.h"' \
             '-DFT_CONFIG_OPTIONS_H="slimftoptions.h"'
FTINCLUDES = -Isrc -Iinclude
  FTCFLAGS = -O -fPIC -fno-common $(FTINCLUDES) $(FTDEFINES) $(ARCHS)

ifeq ($(shell uname -s),Darwin)
 FTCFLAGS += -mmacosx-version-min=10.11
endif

      OBJS = \
        ftbase.o \
        ftbbox.o \
        ftbitmap.o \
        ftdebug.o \
        ftgasp.o \
        ftglyph.o \
        ftinit.o \
        ftstroke.o \
        ftsynth.o \
        ftsystem.o \
        fttype1.o \
        ftxf86.o \
        cff.o \
        psaux.o \
        pshinter.o \
        psnames.o \
        raster.o \
        sfnt.o \
        smooth.o \
        truetype.o \
        type1.o \
        type1cid.o

default: install

libfreetype.a: $(OBJS)
	$(AR) crv $@ $?
	$(AR) ts $@

%.o: src/base/%.c
	$(CC) $(FTCFLAGS) -o $@ -c $<
%.o: src/cff/%.c
	$(CC) $(FTCFLAGS) -o $@ -c $<
%.o: src/cid/%.c
	$(CC) $(FTCFLAGS) -o $@ -c $<
%.o: src/psaux/%.c
	$(CC) $(FTCFLAGS) -o $@ -c $<
%.o: src/pshinter/%.c
	$(CC) $(FTCFLAGS) -o $@ -c $<
%.o: src/psnames/%.c
	$(CC) $(FTCFLAGS) -o $@ -c $<
%.o: src/raster/%.c
	$(CC) $(FTCFLAGS) -o $@ -c $<
%.o: src/smooth/%.c
	$(CC) $(FTCFLAGS) -o $@ -c $<
%.o: src/sfnt/%.c
	$(CC) $(FTCFLAGS) -o $@ -c $<
%.o: src/truetype/%.c
	$(CC) $(FTCFLAGS) -o $@ -c $<
%.o: src/type1/%.c
	$(CC) $(FTCFLAGS) -o $@ -c $<

distclean: clean

clean:
	rm -f libfreetype.a *.o

$(PREFIX)/lib/libfreetype.a: libfreetype.a
	mkdir -p $(PREFIX)/lib/
	mkdir -p $(PREFIX)/include/
	cp libfreetype.a $(PREFIX)/lib/
	cp include/ft2build.h $(PREFIX)/include/
	cp -r include/freetype $(PREFIX)/include/

install: $(PREFIX)/lib/libfreetype.a

.PHONY: default install clean distclean
