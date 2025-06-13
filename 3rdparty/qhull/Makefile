ifeq ($(strip $(PREFIX)),)
override PREFIX = $(abspath $(CURDIR)/../build)
endif

 CFLAGS = -O -fPIC -fno-common $(ARCHS)
ifeq ($(notdir $(CC)), emcc)
else ifndef OS
ifeq ($(shell uname),Darwin)
CFLAGS += -mmacosx-version-min=10.15
endif
else ifeq ($(shell [[ "$(OS)" =~ [dD]"arwin" ]] && echo macos), macos)
CFLAGS += -mmacosx-version-min=10.15
endif

   OBJS = \
        global_r.o \
        stat_r.o \
        geom2_r.o \
        poly2_r.o \
        merge_r.o \
        libqhull_r.o \
        geom_r.o \
        poly_r.o \
        qset_r.o \
        mem_r.o \
        random_r.o \
        usermem_r.o \
        userprintf_r.o \
        io_r.o \
        user_r.o \
        rboxlib_r.o \
        userprintf_rbox_r.o

default: install

libqhull_r.a: $(OBJS)
	$(AR) crv $@ $?
	$(AR) ts $@

distclean: clean

clean:
	rm -f libqhull_r.a *.o

$(PREFIX)/lib/libqhull_r.a: libqhull_r.a
	mkdir -p $(PREFIX)/lib/
	mkdir -p $(PREFIX)/include/libqhull_r
	cp libqhull_r.a $(PREFIX)/lib/
	cp *.h $(PREFIX)/include/libqhull_r/

install: $(PREFIX)/lib/libqhull_r.a

depend:
	makedepend -Y -- \
        global_r.c \
        stat_r.c \
        geom2_r.c \
        poly2_r.c \
        merge_r.c \
        libqhull_r.c \
        geom_r.c \
        poly_r.c \
        qset_r.c \
        mem_r.c \
        random_r.c \
        usermem_r.c \
        userprintf_r.c \
        io_r.c \
        user_r.c \
        rboxlib_r.c \
        userprintf_rbox_r.c 2> /dev/null

.PHONY: default install clean distclean depend

# DO NOT DELETE THIS LINE -- make depend depends on it.

global_r.o: qhull_ra.h libqhull_r.h user_r.h mem_r.h qset_r.h stat_r.h
global_r.o: random_r.h geom_r.h merge_r.h poly_r.h io_r.h
stat_r.o: qhull_ra.h libqhull_r.h user_r.h mem_r.h qset_r.h stat_r.h
stat_r.o: random_r.h geom_r.h merge_r.h poly_r.h io_r.h
geom2_r.o: qhull_ra.h libqhull_r.h user_r.h mem_r.h qset_r.h stat_r.h
geom2_r.o: random_r.h geom_r.h merge_r.h poly_r.h io_r.h
poly2_r.o: qhull_ra.h libqhull_r.h user_r.h mem_r.h qset_r.h stat_r.h
poly2_r.o: random_r.h geom_r.h merge_r.h poly_r.h io_r.h
merge_r.o: qhull_ra.h libqhull_r.h user_r.h mem_r.h qset_r.h stat_r.h
merge_r.o: random_r.h geom_r.h merge_r.h poly_r.h io_r.h
libqhull_r.o: qhull_ra.h libqhull_r.h user_r.h mem_r.h qset_r.h stat_r.h
libqhull_r.o: random_r.h geom_r.h merge_r.h poly_r.h io_r.h
geom_r.o: qhull_ra.h libqhull_r.h user_r.h mem_r.h qset_r.h stat_r.h
geom_r.o: random_r.h geom_r.h merge_r.h poly_r.h io_r.h
poly_r.o: qhull_ra.h libqhull_r.h user_r.h mem_r.h qset_r.h stat_r.h
poly_r.o: random_r.h geom_r.h merge_r.h poly_r.h io_r.h
qset_r.o: libqhull_r.h user_r.h mem_r.h qset_r.h stat_r.h
mem_r.o: libqhull_r.h user_r.h mem_r.h qset_r.h stat_r.h
random_r.o: libqhull_r.h user_r.h mem_r.h qset_r.h stat_r.h random_r.h
usermem_r.o: libqhull_r.h user_r.h mem_r.h qset_r.h stat_r.h
userprintf_r.o: libqhull_r.h user_r.h mem_r.h qset_r.h stat_r.h poly_r.h
io_r.o: qhull_ra.h libqhull_r.h user_r.h mem_r.h qset_r.h stat_r.h random_r.h
io_r.o: geom_r.h merge_r.h poly_r.h io_r.h
user_r.o: qhull_ra.h libqhull_r.h user_r.h mem_r.h qset_r.h stat_r.h
user_r.o: random_r.h geom_r.h merge_r.h poly_r.h io_r.h
rboxlib_r.o: libqhull_r.h user_r.h mem_r.h qset_r.h stat_r.h random_r.h
userprintf_rbox_r.o: libqhull_r.h user_r.h mem_r.h qset_r.h stat_r.h
