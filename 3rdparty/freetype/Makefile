 DEFINES = -DFT2_BUILD_LIBRARY -DDARWIN_NO_CARBON \
           '-DFT_CONFIG_MODULES_H="slimftmodules.h"' \
           '-DFT_CONFIG_OPTIONS_H="slimftoptions.h"'
INCLUDES = -Isrc -Iinclude
  CFLAGS = -O -fPIC -fno-common $(INCLUDES) $(DEFINES) $(ARCHS)

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

libfreetype.a: $(OBJS)
	$(AR) crv $@ $?
	$(AR) ts $@

%.o: src/base/%.c
	$(CC) $(CFLAGS) -o $@ -c $<
%.o: src/cff/%.c
	$(CC) $(CFLAGS) -o $@ -c $<
%.o: src/cid/%.c
	$(CC) $(CFLAGS) -o $@ -c $<
%.o: src/psaux/%.c
	$(CC) $(CFLAGS) -o $@ -c $<
%.o: src/pshinter/%.c
	$(CC) $(CFLAGS) -o $@ -c $<
%.o: src/psnames/%.c
	$(CC) $(CFLAGS) -o $@ -c $<
%.o: src/raster/%.c
	$(CC) $(CFLAGS) -o $@ -c $<
%.o: src/smooth/%.c
	$(CC) $(CFLAGS) -o $@ -c $<
%.o: src/sfnt/%.c
	$(CC) $(CFLAGS) -o $@ -c $<
%.o: src/truetype/%.c
	$(CC) $(CFLAGS) -o $@ -c $<
%.o: src/type1/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

distclean: clean

clean:
	rm -f libfreetype.a *.o

