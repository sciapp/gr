     JSC = /System/Library/Frameworks/JavaScriptCore.framework/Versions/Current/Resources/jsc
    OPTS = -s LEGACY_GL_EMULATION=1
 DEFINES = -DEMSCRIPTEN -D__SVR4 -DGRDIR=\"\" 
 JPEGDIR = ../3rdparty/jpeg
  PNGDIR = ../3rdparty/png
 ZLIBDIR = ../3rdparty/zlib
  GKSDIR = ../lib/gks
   GRDIR = ../lib/gr
  GR3DIR = ../lib/gr3
INCLUDES = -I$(GR3DIR) -I$(GRDIR) -I$(GKSDIR) -I$(JPEGDIR) -I$(PNGDIR) -I$(ZLIBDIR)
 GKSOBJS = js.o $(GKSDIR)/gks.o $(GKSDIR)/font.o $(GKSDIR)/afm.o $(GKSDIR)/util.o \
           $(GKSDIR)/malloc.o $(GKSDIR)/error.o $(GKSDIR)/wiss.o $(GKSDIR)/io.o
  GROBJS = $(GRDIR)/gr.o $(GRDIR)/io.o $(GRDIR)/text.o $(GRDIR)/gridit.o \
           $(GRDIR)/strlib.o $(GRDIR)/contour.o $(GRDIR)/spline.o \
           $(GRDIR)/import.o $(GRDIR)/grforbnd.o $(GRDIR)/md5.o
 GR3OBJS = $(GR3DIR)/gr3.o $(GR3DIR)/gr3_convenience.o \
           $(GR3DIR)/gr3_html.o $(GR3DIR)/gr3_povray.o $(GR3DIR)/gr3_png.o \
           $(GR3DIR)/gr3_jpeg.o $(GR3DIR)/gr3_gr.o $(GR3DIR)/gr3_mc.o 
    OBJS = $(GR3OBJS) $(GROBJS) $(GKSOBJS)
    LIBS = $(JPEGDIR)/libjpeg.a $(PNGDIR)/libpng.a $(ZLIBDIR)/libz.a
 BACKEND = $(GKSDIR)/pdf.c

.PHONY: default svg clean js.c .SECONDARY

.SECONDARY:

default: gksdemo.pdf gr3demo.pdf

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
	cp -p $(GKSDIR)/fonts/gksfont.dat fonts/

$(JPEGDIR)/libjpeg.a:
	emmake make -C $(JPEGDIR)

$(PNGDIR)/libpng.a:
	emmake make -C $(PNGDIR)

$(ZLIBDIR)/libz.a:
	emmake make -C $(ZLIBDIR)

%.o: %.c
	emcc $(OPTS) $(DEFINES) -DBACKEND=\"$(BACKEND)\" -o $@ $^ $(INCLUDES)

%.js: %.o $(OBJS) $(LIBS)
	emcc $(OPTS) -DBACKEND=\"$(BACKEND)\" -o $@ $< $(OBJS) $(LIBS) \
	--embed-file fonts/gksfont.dat

clean:
	rm -f *.js js.c *.o *.pdf *.svg *.png
	rm -rf fonts
	cd $(JPEGDIR) && make clean
	cd $(PNGDIR) && make clean
	cd $(ZLIBDIR) && make clean
	rm -rf $(OBJS)
