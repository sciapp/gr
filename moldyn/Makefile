 GLINCLUDES = 
GR3INCLUDES = -I./../lib/gr3/
    GR3LIBS = -L./ -lGR3
     GLLIBS = 
      XLIBS2 = 
     CFLAGS = -Wall -pedantic
JPEG2PSOBJS = jpeg2ps/jpeg2ps.o jpeg2ps/readjpeg.o jpeg2ps/asc85ec.o
       OBJS = moldyn.o  moldyn_utilities.o moldyn_element_information.o moldyn_graphics.o ${JPEG2PSOBJS}
    DEFINES = -DVersion="\"MolDyn - 8.0.0\"" -DRevision="\"Rev: `date +"%b %e %Y"`\""

default:
	make `uname`

Linux:
	make moldyn GLLIBS="-lglut -lGLU -lGL -lm" XLIBS2="-lX11"

Darwin:
	make moldyn GLLIBS="-framework OpenGL -framework GLUT -L/System/Library/Frameworks/OpenGL.framework/Libraries -lGLU -lGL -lobjc"

targets: prerequisites moldyn

prerequisites: libGR3.so jpeg2ps

jpeg2ps:
	make -C jpeg2ps

libGR3.so:
	make -C../lib/gr3
	cp ../lib/gr3/libGR3.so .

%.c: %.h
%.o: %.c
	$(CC) ${CFLAGS} ${DEFINES} -c -o $@ $<

moldyn_graphics.o: moldyn_graphics.c libGR3.so
	$(CC) ${CFLAGS} -I ${GLINCLUDES} ${GR3INCLUDES} ${DEFINES} -c -o $@ moldyn_graphics.c

moldyn: ${OBJS}
	$(CC) ${CFLAGS} ${GLLIBS} ${XLIBS2} ${GR3LIBS} -o $@ ${OBJS}

clean:
	@make -C../lib/gr3 clean
	@make -C jpeg2ps clean
	rm -f moldyn
	rm -f *.o
	rm -f *.eps *.jpg *.pov *.png *.html *.mov
	rm -rf moldyn.dSYM
	rm -f libGR3.so
