 CFLAGS = -O -fPIC -fno-common $(ARCHS)
   LIBS = libqhull.a

   OBJS = \
        global.o \
        stat.o \
        geom2.o \
        poly2.o \
        merge.o \
        libqhull.o \
        geom.o \
        poly.o \
        qset.o \
        mem.o \
        random.o \
        usermem.o \
        userprintf.o \
        io.o \
        user.o \
        rboxlib.o \
        userprintf_rbox.o

libqhull.a: $(OBJS)
	$(AR) crv $@ $?
	$(AR) ts $@

distclean: clean

clean:
	rm -f libqhull.a *.o

depend:
	makedepend -Y -- \
        global.c \
        stat.c \
        geom2.c \
        poly2.c \
        merge.c \
        libqhull.c \
        geom.c \
        poly.c \
        qset.c \
        mem.c \
        random.c \
        usermem.c \
        userprintf.c \
        io.c \
        user.c \
        rboxlib.c \
        userprintf_rbox.c 2> /dev/null

# DO NOT DELETE THIS LINE -- make depend depends on it.

global.o: qhull_a.h libqhull.h user.h mem.h qset.h stat.h random.h geom.h
global.o: merge.h poly.h io.h
stat.o: qhull_a.h libqhull.h user.h mem.h qset.h stat.h random.h geom.h
stat.o: merge.h poly.h io.h
geom2.o: qhull_a.h libqhull.h user.h mem.h qset.h stat.h random.h geom.h
geom2.o: merge.h poly.h io.h
poly2.o: qhull_a.h libqhull.h user.h mem.h qset.h stat.h random.h geom.h
poly2.o: merge.h poly.h io.h
merge.o: qhull_a.h libqhull.h user.h mem.h qset.h stat.h random.h geom.h
merge.o: merge.h poly.h io.h
libqhull.o: qhull_a.h libqhull.h user.h mem.h qset.h stat.h random.h geom.h
libqhull.o: merge.h poly.h io.h
geom.o: qhull_a.h libqhull.h user.h mem.h qset.h stat.h random.h geom.h
geom.o: merge.h poly.h io.h
poly.o: qhull_a.h libqhull.h user.h mem.h qset.h stat.h random.h geom.h
poly.o: merge.h poly.h io.h
qset.o: user.h qset.h mem.h
mem.o: user.h mem.h
random.o: libqhull.h user.h mem.h qset.h stat.h random.h
usermem.o: libqhull.h user.h mem.h qset.h stat.h
userprintf.o: libqhull.h user.h mem.h qset.h stat.h
io.o: qhull_a.h libqhull.h user.h mem.h qset.h stat.h random.h geom.h merge.h
io.o: poly.h io.h
user.o: qhull_a.h libqhull.h user.h mem.h qset.h stat.h random.h geom.h
user.o: merge.h poly.h io.h
rboxlib.o: libqhull.h user.h mem.h qset.h stat.h random.h
userprintf_rbox.o: libqhull.h user.h mem.h qset.h stat.h
