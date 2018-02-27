     JSC = /System/Library/Frameworks/JavaScriptCore.framework/Versions/Current/Resources/jsc
ifndef DEBUG
    OPTS = -s LEGACY_GL_EMULATION=1 -O3
else
	OPTS = -s LEGACY_GL_EMULATION=1 -s ASSERTIONS=2 -s SAFE_HEAP=1
endif
 DEFINES = -DEMSCRIPTEN -D__SVR4 -DGRDIR=\"\"
 JPEGDIR = ../3rdparty/jpeg
  PNGDIR = ../3rdparty/libpng16
 ZLIBDIR = ../3rdparty/zlib
  GKSDIR = ../lib/gks
   GRDIR = ../lib/gr
  GR3DIR = ../lib/gr3
INCLUDES = -I$(GR3DIR) -I$(GRDIR) -I$(GKSDIR) -I$(JPEGDIR) -I$(PNGDIR)/.. -I$(ZLIBDIR)
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

.PHONY: default svg clean js.c .SECONDARY .FORCE

.SECONDARY:

default: gr.js gksdemo.pdf gr3demo.pdf

.FORCE:

svg:
	make grdemo.svg BACKEND=$(GKSDIR)/plugin/svgplugin.cxx

html:
	make grdemo.html BACKEND=$(GKSDIR)/plugin/svgplugin.cxx

../lib/gr/gr_version.h: .FORCE
	../lib/Version --c > $@
	@chmod 644 $@

gr.js: ../lib/gr/gr_version.h
	make libGR.js BACKEND=jsplugin.cxx
	cat module.js libGR.js api.js >gr.js

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

%.js: %.o $(OBJS) $(LIBS) library.js
	emcc $(OPTS) $(DEFINES) -DBACKEND=\"$(BACKEND)\" \
        --embed-file fonts/gksfont.dat --js-library library.js \
	-o $@ $< $(OBJS) $(LIBS)

libGR.js: $(OBJS) $(LIBS) library.js
	emcc $(OPTS) $(DEFINES)  -DBACKEND=\"$(BACKEND)\" \
	--embed-file fonts/gksfont.dat --js-library library.js \
	-o $@ $(OBJS) $(LIBS) \
	-s EXPORTED_FUNCTIONS="['_gr_opengks', '_gr_closegks', '_gr_inqdspsize', '_gr_openws', '_gr_closews', '_gr_activatews', '_gr_deactivatews', '_gr_clearws', '_gr_updatews', '_gr_polyline', '_gr_polymarker', '_gr_text', '_gr_inqtext', '_gr_fillarea', '_gr_cellarray', '_gr_spline', '_gr_gridit', '_gr_setlinetype', '_gr_inqlinetype', '_gr_setlinewidth', '_gr_inqlinewidth', '_gr_setlinecolorind', '_gr_inqlinecolorind', '_gr_setmarkertype', '_gr_inqmarkertype', '_gr_setmarkersize', '_gr_setmarkercolorind', '_gr_inqmarkercolorind', '_gr_settextfontprec', '_gr_setcharexpan', '_gr_setcharspace', '_gr_settextcolorind', '_gr_setcharheight', '_gr_setcharup', '_gr_settextpath', '_gr_settextalign', '_gr_setfillintstyle', '_gr_setfillstyle', '_gr_setfillcolorind', '_gr_setcolorrep', '_gr_setscale', '_gr_inqscale', '_gr_setwindow', '_gr_inqwindow', '_gr_setviewport', '_gr_inqviewport', '_gr_selntran', '_gr_setclip', '_gr_setwswindow', '_gr_setwsviewport', '_gr_createseg', '_gr_copysegws', '_gr_redrawsegws', '_gr_setsegtran', '_gr_closeseg', '_gr_emergencyclosegks', '_gr_updategks', '_gr_setspace', '_gr_inqspace', '_gr_textext', '_gr_inqtextext', '_gr_axes', '_gr_grid', '_gr_verrorbars', '_gr_herrorbars', '_gr_polyline3d', '_gr_axes3d', '_gr_titles3d', '_gr_surface', '_gr_contour', '_gr_hexbin', '_gr_setcolormap', '_gr_inqcolormap', '_gr_colorbar', '_gr_inqcolor', '_gr_inqcolorfromrgb', '_gr_hsvtorgb', '_gr_tick', '_gr_validaterange', '_gr_adjustrange', '_gr_beginprint', '_gr_beginprintext', '_gr_endprint', '_gr_ndctowc', '_gr_wctondc', '_gr_drawrect', '_gr_fillrect', '_gr_drawarc', '_gr_fillarc', '_gr_drawpath', '_gr_setarrowstyle', '_gr_drawarrow', '_gr_readimage', '_gr_drawimage', '_gr_importgraphics', '_gr_setshadow', '_gr_settransparency', '_gr_setcoordxform', '_gr_begingraphics', '_gr_endgraphics', '_gr_drawgraphics', '_gr_mathtex', '_gr_beginselection', '_gr_endselection', '_gr_moveselection', '_gr_resizeselection', '_gr_inqbbox', '_gr_precision', '_gr_setregenflags', '_gr_inqregenflags', '_gr_savestate', '_gr_restorestate', '_gr_selectcontext', '_gr_destroycontext', '_gr_uselinespec', '_gr_selntran', '_gks_get_dash_list']"

%.html: %.o $(OBJS) $(LIBS)
	emcc -DEMSCRIPTEN -DBACKEND=\"$(BACKEND)\" -D__SVR4 -DGRDIR=\"\" \
	--embed-file fonts/gksfont.dat --js-library library.js -o $@ $< $(OBJS) $(LIBS)

clean:
	rm -f *demo.js js.c *.o *.pdf *.svg *.png libGR.js gr.js *.js.mem
	rm -rf fonts
	cd $(JPEGDIR) && make clean
	cd $(PNGDIR) && make clean
	cd $(ZLIBDIR) && make clean
	rm -rf $(OBJS)
