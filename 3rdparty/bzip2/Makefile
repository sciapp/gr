ifeq ($(strip $(PREFIX)),)
override PREFIX = $(abspath $(CURDIR)/../build)
endif

  CFLAGS = -O -I. -fPIC -fno-common $(ARCHS)
ifeq ($(shell uname),Darwin)
 CFLAGS += -mmacosx-version-min=10.11
endif
    OBJS = blocksort.o \
        huffman.o \
        crctable.o \
        randtable.o \
        compress.o \
        decompress.o \
        bzlib.o

default: install

libbz2.a: $(OBJS)
	$(AR) crv $@ $?
	$(AR) ts $@

distclean: clean

clean:
	rm -f libbz2.a *.o *.bak

$(PREFIX)/lib/libbz2.a: libbz2.a
	mkdir -p $(PREFIX)/lib/
	mkdir -p $(PREFIX)/include/
	cp libbz2.a $(PREFIX)/lib/
	cp bzlib.h $(PREFIX)/include/

install: $(PREFIX)/lib/libbz2.a

depend:
	makedepend -Y -- \
        huffman.c \
        crctable.c \
        randtable.c \
        compress.c \
        decompress.c \
        bzlib.o 2> /dev/null

.PHONY: default install clean distclean depend

# DO NOT DELETE THIS LINE -- make depend depends on it.

huffman.o: bzlib_private.h bzlib.h
crctable.o: bzlib_private.h bzlib.h
randtable.o: bzlib_private.h bzlib.h
compress.o: bzlib_private.h bzlib.h
decompress.o: bzlib_private.h bzlib.h
