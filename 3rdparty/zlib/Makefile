ifeq ($(strip $(PREFIX)),)
override PREFIX = $(abspath $(CURDIR)/../build)
endif

CFLAGS=-O -fPIC -DHAVE_HIDDEN $(ARCHS)
ifeq ($(shell uname),Darwin)
ifneq ($(notdir $(CC)),emcc)
CFLAGS += -mmacosx-version-min=10.11
endif
endif
LIBS=libz.a

OBJS = \
        adler32.o \
        crc32.o \
        deflate.o \
        infback.o \
        inffast.o \
        inflate.o \
        inftrees.o \
        trees.o \
        zutil.o \
        compress.o \
        uncompr.o \
        gzclose.o \
        gzlib.o \
        gzread.o \
        gzwrite.o

default: install

libz.a: $(OBJS)
	$(AR) crv $@ $?
	$(AR) ts $@

distclean: clean

clean:
	rm -f libz.a *.o

depend:
	makedepend -Y -- \
        adler32.c \
        crc32.c \
        deflate.c \
        infback.c \
        inffast.c \
        inflate.c \
        inftrees.c \
        trees.c \
        zutil.c \
        compress.c \
        uncompr.c \
        gzclose.c \
        gzlib.c \
        gzread.c \
        gzwrite.c 2> /dev/null

$(PREFIX)/lib/libz.a: libz.a
	mkdir -p $(PREFIX)/lib/
	mkdir -p $(PREFIX)/include/
	cp libz.a $(PREFIX)/lib/
	cp *.h $(PREFIX)/include/

install: $(PREFIX)/lib/libz.a

.PHONY: default install clean distclean depend

# DO NOT DELETE THIS LINE -- make depend depends on it.

adler32.o: zutil.h zlib.h zconf.h
crc32.o: zutil.h zlib.h zconf.h crc32.h
deflate.o: deflate.h zutil.h zlib.h zconf.h
infback.o: zutil.h zlib.h zconf.h inftrees.h inflate.h inffast.h inffixed.h
inffast.o: zutil.h zlib.h zconf.h inftrees.h inflate.h inffast.h
inflate.o: zutil.h zlib.h zconf.h inftrees.h inflate.h inffast.h inffixed.h
inftrees.o: zutil.h zlib.h zconf.h inftrees.h
trees.o: deflate.h zutil.h zlib.h zconf.h trees.h
zutil.o: zutil.h zlib.h zconf.h gzguts.h
compress.o: zlib.h zconf.h
uncompr.o: zlib.h zconf.h
gzclose.o: gzguts.h zlib.h zconf.h
gzlib.o: gzguts.h zlib.h zconf.h
gzread.o: gzguts.h zlib.h zconf.h
gzwrite.o: gzguts.h zlib.h zconf.h
