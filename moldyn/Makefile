     GR3DIR = ../lib/gr3
    LIBDIR = $(GR3DIR)
MOLDYNOBJS = moldyn.o moldyn_utils.o moldyn_filereader.o moldyn_gr3.o \
             moldyn_glut.o moldyn_glut_menu.o moldyn_glut_key_bindings.o
  INCLUDES = -I.
   GR3LIBS = -L$(GR3DIR) -lGR3
    GLLIBS = -lGL -lX11 -lglut
      LIBS = -lm
    CFLAGS = -Wall -Wextra -pedantic -g

default:
	make `uname`
Linux:
	make targets
Darwin:
	make targets GLLIBS="-framework OpenGL -framework Cocoa -framework GLUT"

targets: prerequisites moldyn libGR3.so

prerequisites:
	@make -C ../lib/gr3

.c.o:
	$(CC) -c $(CFLAGS) $(INCLUDES) $<

moldyn: $(MOLDYNOBJS)
	${CC} -o $@ *.o $(CFLAGS) $(INCLUDES) $(GR3LIBS) $(GLLIBS) $(LIBS)

gr3.h:
	cp $(GR3DIR)/gr3.h .

libGR3.so:
	cp $(GR3DIR)/libGR3.so .

moldyn_filereader.c: moldyn_element_infos.h
moldyn_gr3.c: gr3.h

install: default

clean:
	@make -C ../lib/gr3 clean
	rm -f libGR3.so
	rm -f gr3.h
	rm -f *.pyc
	rm -f *.o
	rm -f moldyn
	rm -f moldyn.dSYM
	rm -f libGR3.so