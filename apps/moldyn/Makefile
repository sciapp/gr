      GRDIR = /usr/local/gr
GR3INCLUDES = -I$(GRDIR)/include
    GR3LIBS = -L$(GRDIR)/lib -lGR3
     GLLIBS = 
      XLIBS = 
     CFLAGS = -Wall -pedantic
    LDFLAGS =
JPEG2PSOBJS = jpeg2ps/jpeg2ps.o jpeg2ps/readjpeg.o jpeg2ps/asc85ec.o
       OBJS = moldyn.o  moldyn_utilities.o moldyn_element_information.o moldyn_graphics.o ${JPEG2PSOBJS}
    DEFINES = -DMOLDYN_VERSION="\"MolDyn - 8.0.0\"" 
-DMOLDYN_REVISION="\"Rev: `date +"%b %e %Y"`\""

default:
	@make `uname`

Linux:
	make moldyn GLLIBS="-lglut -lGLU -lGL -lm" \
	LDFLAGS="-Wl,-rpath,$(GRDIR)/lib" XLIBS="-lX11"

Darwin:
	make moldyn GLLIBS="-framework OpenGL -framework GLUT -L/System/Library/Frameworks/OpenGL.framework/Libraries -lGLU -lGL -lobjc"

%.o: %.c
	$(CC) ${CFLAGS} ${DEFINES} -c -o $@ $<

moldyn_graphics.o: moldyn_graphics.c
	$(CC) ${CFLAGS} ${GR3INCLUDES} ${DEFINES} -c -o $@ moldyn_graphics.c

moldyn: ${OBJS}
	$(CC) ${LDFLAGS} ${GR3LIBS} ${GLLIBS} ${XLIBS} -o $@ ${OBJS}

install: moldyn
	cp -p moldyn ${GRDIR}/bin

clean:
	@make -C jpeg2ps clean
	rm -f moldyn
	rm -f *.o
	rm -f *.eps *.jpg *.pov *.png *.html *.mov
	rm -rf moldyn.dSYM
