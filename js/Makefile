     JSC = /System/Library/Frameworks/JavaScriptCore.framework/Versions/Current/Resources/jsc
  PNGDIR = ../3rdparty/png
 ZLIBDIR = ../3rdparty/zlib
  GKSDIR = ../lib/gks
   GRDIR = ../lib/gr
INCLUDES = -I$(GRDIR) -I$(GKSDIR) -I$(PNGDIR) -I$(ZLIBDIR)
 GKSOBJS = js.o $(GKSDIR)/gks.o $(GKSDIR)/font.o $(GKSDIR)/afm.o $(GKSDIR)/util.o \
           $(GKSDIR)/malloc.o $(GKSDIR)/error.o $(GKSDIR)/wiss.o $(GKSDIR)/io.o
  GROBJS = $(GRDIR)/gr.o $(GRDIR)/io.o $(GRDIR)/text.o $(GRDIR)/gridit.o \
           $(GRDIR)/strlib.o $(GRDIR)/contour.o $(GRDIR)/spline.o \
           $(GRDIR)/import.o $(GRDIR)/grforbnd.o $(GRDIR)/md5.o
    OBJS = $(GROBJS) $(GKSOBJS)
    LIBS = $(PNGDIR)/libpng.a $(ZLIBDIR)/libz.a
 BACKEND = $(GKSDIR)/pdf.c

.PHONY: default svg clean js.c .SECONDARY

.SECONDARY:

default: gksdemo.pdf

svg:
	make grdemo.svg BACKEND=$(GKSDIR)/plugin/svgplugin.cxx

%.pdf: %.js
	$(JSC) $< >$@

%.svg: %.js
	$(JSC) $< >$@

js.c: fonts
	echo "#include BACKEND" >js.c

fonts:
	mkdir fonts
	cp -p $(GKSDIR)/gksfont.dat fonts/

$(PNGDIR)/libpng.a:
	emmake make -C $(PNGDIR)

$(ZLIBDIR)/libz.a:
	emmake make -C $(ZLIBDIR)

%.o: %.c
	emcc -DEMSCRIPTEN -DBACKEND=\"$(BACKEND)\" -D__SVR4 -DGRDIR=\"\" \
	-o $@ $^ $(INCLUDES)

%.js: %.o $(OBJS) $(LIBS)
	emcc -DEMSCRIPTEN -DBACKEND=\"$(BACKEND)\" -D__SVR4 -DGRDIR=\"\" \
	--embed-file fonts/gksfont.dat -o $@ $< $(OBJS) $(LIBS)

clean:
	rm -f *.js js.c *.o *.pdf *.svg
	rm -rf fonts
	cd $(PNGDIR) && make clean
	cd $(ZLIBDIR) && make clean
	rm -rf $(OBJS)
