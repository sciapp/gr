  CFLAGS = -O -I. -fPIC -fno-common $(ARCHS)
    OBJS = blocksort.o \
        huffman.o \
        crctable.o \
        randtable.o \
        compress.o \
        decompress.o \
        bzlib.o

libbz2.a: $(OBJS)
	$(AR) crv $@ $?
	$(AR) ts $@

distclean: clean

clean:
	rm -f libbz2.a *.o *.bak

depend:
	makedepend -Y -- \
        huffman.c \
        crctable.c \
        randtable.c \
        compress.c \
        decompress.c \
        bzlib.o 2> /dev/null

# DO NOT DELETE THIS LINE -- make depend depends on it.

huffman.o: bzlib_private.h bzlib.h
crctable.o: bzlib_private.h bzlib.h
randtable.o: bzlib_private.h bzlib.h
compress.o: bzlib_private.h bzlib.h
decompress.o: bzlib_private.h bzlib.h
