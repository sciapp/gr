  CFLAGS = -O -I. -fPIC -fno-common $(ARCHS)
    OBJS = \
        jbig2.o \
        jbig2_arith.o \
        jbig2_arith_iaid.o \
        jbig2_arith_int.o \
        jbig2_generic.o \
        jbig2_halftone.o \
        jbig2_huffman.o \
        jbig2_image.o \
        jbig2_metadata.o \
        jbig2_mmr.o \
        jbig2_page.o \
        jbig2_refinement.o \
        jbig2_segment.o \
        jbig2_symbol_dict.o \
        jbig2_text.o

libjbig2dec.a: $(OBJS)
	$(AR) crv $@ $?
	$(AR) ts $@

distclean: clean

clean:
	rm -f libjbig2dec.a *.o *.bak

depend:
	makedepend -Y -- \
        jbig2.c \
        jbig2_arith.c \
        jbig2_arith_iaid.c \
        jbig2_arith_int.c \
        jbig2_generic.c \
        jbig2_halftone.c \
        jbig2_huffman.c \
        jbig2_image.c \
        jbig2_metadata.c \
        jbig2_mmr.c \
        jbig2_page.c \
        jbig2_refinement.c \
        jbig2_segment.c \
        jbig2_symbol_dict.c \
        jbig2_text.c 2> /dev/null

# DO NOT DELETE THIS LINE -- make depend depends on it.

jbig2.o: os_types.h jbig2.h jbig2_priv.h
jbig2_arith.o: os_types.h jbig2.h jbig2_priv.h jbig2_arith.h
jbig2_arith_iaid.o: os_types.h jbig2.h jbig2_priv.h jbig2_arith.h
jbig2_arith_iaid.o: jbig2_arith_iaid.h
jbig2_arith_int.o: os_types.h jbig2.h jbig2_priv.h jbig2_arith.h
jbig2_arith_int.o: jbig2_arith_int.h
jbig2_generic.o: os_types.h jbig2.h jbig2_priv.h jbig2_image.h jbig2_arith.h
jbig2_generic.o: jbig2_generic.h jbig2_mmr.h
jbig2_halftone.o: os_types.h jbig2.h jbig2_priv.h jbig2_arith.h
jbig2_halftone.o: jbig2_generic.h jbig2_mmr.h
jbig2_huffman.o: os_types.h jbig2.h jbig2_priv.h jbig2_huffman.h
jbig2_huffman.o: jbig2_hufftab.h
jbig2_image.o: os_types.h jbig2.h jbig2_priv.h jbig2_image.h
jbig2_metadata.o: os_types.h jbig2.h jbig2_priv.h jbig2_metadata.h
jbig2_mmr.o: os_types.h jbig2.h jbig2_priv.h jbig2_arith.h jbig2_generic.h
jbig2_mmr.o: jbig2_mmr.h
jbig2_page.o: os_types.h jbig2.h jbig2_priv.h
jbig2_refinement.o: os_types.h jbig2.h jbig2_priv.h jbig2_arith.h
jbig2_refinement.o: jbig2_generic.h jbig2_image.h
jbig2_segment.o: os_types.h jbig2.h jbig2_priv.h jbig2_symbol_dict.h
jbig2_segment.o: jbig2_metadata.h
jbig2_symbol_dict.o: os_types.h jbig2.h jbig2_priv.h jbig2_arith.h
jbig2_symbol_dict.o: jbig2_arith_int.h jbig2_arith_iaid.h jbig2_huffman.h
jbig2_symbol_dict.o: jbig2_generic.h jbig2_mmr.h jbig2_symbol_dict.h
jbig2_symbol_dict.o: jbig2_text.h
jbig2_text.o: os_types.h jbig2.h jbig2_priv.h jbig2_arith.h jbig2_arith_int.h
jbig2_text.o: jbig2_arith_iaid.h jbig2_huffman.h jbig2_generic.h
jbig2_text.o: jbig2_symbol_dict.h jbig2_text.h
