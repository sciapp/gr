   INCLUDES =
     CFLAGS = 

all:
	make -C jpeg2ps
	make `uname`

Linux:
	make moldyn INCLUDES="-I -I./GL" LIBS="-lglut -lGLU -lGL -lX11 -lm"

Darwin:
	make moldyn LIBS="-framework OpenGL -framework GLUT -L/System/Library/Frameworks/OpenGL.framework/Libraries -lGLU -lGL -lobjc"

moldyn: moldyn.c GLor.c GLor.h
	$(CC) ${CFLAGS} -o moldyn ${INCLUDES} -Wall -pedantic moldyn.c GLor.c ${LIBS} -ljpeg jpeg2ps/jpeg2ps.o jpeg2ps/readjpeg.o jpeg2ps/asc85ec.o -DRevision="\"Rev: `date +"%b %e %Y"`\""

clean:
	rm -f moldyn.eps moldyn.jpg
	make -C jpeg2ps clean
	rm -f moldyn
	rm -rf moldyn.dSYM
