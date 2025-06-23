ifeq ($(strip $(PREFIX)),)
override PREFIX = $(abspath $(CURDIR)/../build)
endif

 PNGCFLAGS = -O -I. -I../zlib/ -fPIC -fno-common $(PNG_EXTRA_CFLAGS) $(ARCHS)
ifeq ($(notdir $(CC)), emcc)
else ifndef OS
 ifeq ($(shell uname),Darwin)
  PNGCFLAGS += -mmacosx-version-min=10.15
  ifeq ($(shell arch),arm64)
   PNGCFLAGS += -DPNG_ARM_NEON_OPT=0
  endif
 else ifeq ($(shell arch),aarch64)
  PNGCFLAGS += -DPNG_ARM_NEON_OPT=0
 endif
else ifeq ($(shell [[ "$(OS)" =~ [dD]"arwin" ]] && echo macos), macos)
 PNGCFLAGS += -mmacosx-version-min=10.15
 ifeq ($(shell arch),arm64)
  PNGCFLAGS += -DPNG_ARM_NEON_OPT=0
 endif
else ifeq ($(shell [[ "$(OS)" =~ "aarch64" ]] && echo aarch64), aarch64)
 PNGCFLAGS += -DPNG_ARM_NEON_OPT=0
endif
      OBJS = \
	png.o \
	pngerror.o \
	pngget.o \
	pngmem.o \
	pngpread.o \
	pngread.o \
	pngrio.o \
	pngrtran.o \
	pngrutil.o \
	pngset.o \
	pngtrans.o \
	pngwio.o \
	pngwrite.o \
	pngwtran.o \
	pngwutil.o

.c.o:
	$(CC) $(PNGCFLAGS) -o $@ -c $<

default: install

libpng.a: $(OBJS)
	$(AR) crv $@ $?
	$(AR) ts $@

distclean: clean

clean:
	rm -f libpng.a *.o *.bak

depend:
	makedepend -Y -- \
	png.c \
	pngerror.c \
	pngget.c \
	pngmem.c \
	pngpread.c \
	pngread.c \
	pngrio.c \
	pngrtran.c \
	pngrutil.c \
	pngset.c \
	pngtrans.c \
	pngwio.c \
	pngwrite.c \
	pngwtran.c \
	pngwutil.c

$(PREFIX)/lib/libpng.a: libpng.a
	mkdir -p $(PREFIX)/lib/
	mkdir -p $(PREFIX)/include/
	cp libpng.a $(PREFIX)/lib/
	cp *.h $(PREFIX)/include/

install: $(PREFIX)/lib/libpng.a

.PHONY: default install clean distclean depend

# DO NOT DELETE THIS LINE -- make depend depends on it.

png.o: png.h
pngerror.o: png.h
pngget.o: png.h
pngmem.o: png.h
pngpread.o: png.h
pngread.o: png.h
pngrio.o: png.h
pngrtran.o: png.h
pngrutil.o: png.h
pngset.o: png.h
pngtrans.o: png.h
pngwio.o: png.h
pngwrite.o: png.h
pngwtran.o: png.h
pngwutil.o: png.h
