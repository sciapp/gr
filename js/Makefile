ifeq ($(strip $(THIRDPARTYDIR)),)
override THIRDPARTYDIR = $(abspath $(CURDIR)/../3rdparty/build)
endif

ifneq ($(notdir $(CC)),emcc)
rerun_with_emmake:
	 emmake make
endif

OPTS = \
		-s ERROR_ON_UNDEFINED_SYMBOLS=0 \
		-s LEGACY_GL_EMULATION=1 \
		-s 'EXTRA_EXPORTED_RUNTIME_METHODS=["ccall", "cwrap", "getValue", "setValue", "addFunction", "removeFunction", "UTF8ToString", "stringToUTF8", "lengthBytesUTF8"]' \
		-s NO_EXIT_RUNTIME=0 \
		-s WASM=0 \
		--memory-init-file 0 \
		-s TOTAL_MEMORY=33554432 \
		-s ALLOW_MEMORY_GROWTH=1 \
		-s RESERVED_FUNCTION_POINTERS=4 \
		-s ALLOW_TABLE_GROWTH=1 \
		--no-heap-copy
ifndef DEBUG
    OPTS += -O2
else
    OPTS += -s ASSERTIONS=2 -s SAFE_HEAP=1 -s STACK_OVERFLOW_CHECK=2
endif
 DEFINES = -DEMSCRIPTEN -D__SVR4 -DGRDIR=\"\"
 JPEGDIR = ../3rdparty/jpeg
  PNGDIR = ../3rdparty/libpng16
 ZLIBDIR = ../3rdparty/zlib
QHULLDIR = ../3rdparty/qhull
  GKSDIR = ../lib/gks
   GRDIR = ../lib/gr
  GR3DIR = ../lib/gr3
INCLUDES = -I$(THIRDPARTYDIR)/include/ -I$(GR3DIR) -I$(GRDIR) -I$(GKSDIR) -I$(JPEGDIR) -I$(PNGDIR) -I$(ZLIBDIR) -I$(QHULLDIR)
 GKSOBJS = jsplugin.o $(GKSDIR)/gks.o $(GKSDIR)/font.o $(GKSDIR)/afm.o $(GKSDIR)/util.o \
           $(GKSDIR)/malloc.o $(GKSDIR)/error.o $(GKSDIR)/wiss.o $(GKSDIR)/io.o
  GROBJS = $(GRDIR)/gr.o $(GRDIR)/io.o $(GRDIR)/text.o $(GRDIR)/gridit.o \
           $(GRDIR)/strlib.o $(GRDIR)/contour.o $(GRDIR)/spline.o \
           $(GRDIR)/import.o $(GRDIR)/grforbnd.o $(GRDIR)/md5.o \
           $(GRDIR)/interp2.o $(GRDIR)/meta.o $(GRDIR)/shade.o $(GRDIR)/delaunay.o \
           $(GRDIR)/contourf.o $(GRDIR)/image.o
 GR3OBJS = $(GR3DIR)/gr3.o $(GR3DIR)/gr3_convenience.o \
           $(GR3DIR)/gr3_html.o $(GR3DIR)/gr3_povray.o $(GR3DIR)/gr3_png.o \
           $(GR3DIR)/gr3_jpeg.o $(GR3DIR)/gr3_gr.o $(GR3DIR)/gr3_mc.o
    OBJS = $(GR3OBJS) $(GROBJS) $(GKSOBJS)
    LIBS = $(JPEGDIR)/libjpeg.a $(PNGDIR)/libpng.a $(ZLIBDIR)/libz.a $(QHULLDIR)/libqhull.a

.PHONY: default clean .SECONDARY .FORCE

.SECONDARY:

.FORCE:

default: gr.js

../lib/gr/gr_version.h: .FORCE
	$(MAKE) -C ../lib/gr gr_version.h

gr.js: module.js libGR.js api.js jsterm.js
	cat $^ > $@

fonts:
	mkdir fonts
	cp -p $(GKSDIR)/fonts/gksfont.dat fonts/

$(JPEGDIR)/libjpeg.a:
	$(MAKE) -C $(JPEGDIR)

$(PNGDIR)/libpng.a:
	$(MAKE) -C $(PNGDIR)

$(ZLIBDIR)/libz.a:
	$(MAKE) -C $(ZLIBDIR)

$(QHULLDIR)/libqhull.a:
	$(MAKE) -C $(QHULLDIR)

$(GRDIR)/gr.o: ../lib/gr/gr_version.h

%.o: %.c $(LIBS)
	$(CC) -c $(OPTS) $(DEFINES) -o $@ $< $(INCLUDES)

libGR.js: $(OBJS) $(LIBS) library.js fonts
	$(CC) $(OPTS) $(DEFINES) \
	--embed-file fonts/gksfont.dat --js-library library.js \
	-o $@ $(OBJS) $(LIBS) \
	-s EXPORTED_FUNCTIONS="['_gr_opengks', '_gr_closegks', '_gr_inqdspsize', '_gr_openws', '_gr_closews', '_gr_activatews', '_gr_deactivatews', '_gr_clearws', '_gr_updatews', '_gr_polyline', '_gr_polymarker', '_gr_text', '_gr_inqtext', '_gr_fillarea', '_gr_cellarray', '_gr_spline', '_gr_gridit', '_gr_setlinetype', '_gr_inqlinetype', '_gr_setlinewidth', '_gr_inqlinewidth', '_gr_setlinecolorind', '_gr_inqlinecolorind', '_gr_setmarkertype', '_gr_inqmarkertype', '_gr_setmarkersize', '_gr_setmarkercolorind', '_gr_inqmarkercolorind', '_gr_settextfontprec', '_gr_setcharexpan', '_gr_setcharspace', '_gr_settextcolorind', '_gr_setcharheight', '_gr_setcharup', '_gr_settextpath', '_gr_settextalign', '_gr_setfillintstyle', '_gr_setfillstyle', '_gr_setfillcolorind', '_gr_setcolorrep', '_gr_setscale', '_gr_inqscale', '_gr_setwindow', '_gr_inqwindow', '_gr_setviewport', '_gr_inqviewport', '_gr_selntran', '_gr_setclip', '_gr_setwswindow', '_gr_setwsviewport', '_gr_createseg', '_gr_copysegws', '_gr_redrawsegws', '_gr_setsegtran', '_gr_closeseg', '_gr_emergencyclosegks', '_gr_updategks', '_gr_setspace', '_gr_inqspace', '_gr_textext', '_gr_inqtextext', '_gr_axes', '_gr_grid', '_gr_verrorbars', '_gr_herrorbars', '_gr_polyline3d', '_gr_axes3d', '_gr_titles3d', '_gr_surface', '_gr_contour', '_gr_hexbin', '_gr_setcolormap', '_gr_inqcolormap', '_gr_colorbar', '_gr_inqcolor', '_gr_inqcolorfromrgb', '_gr_hsvtorgb', '_gr_tick', '_gr_validaterange', '_gr_adjustrange', '_gr_beginprint', '_gr_beginprintext', '_gr_endprint', '_gr_ndctowc', '_gr_wctondc', '_gr_drawrect', '_gr_fillrect', '_gr_drawarc', '_gr_fillarc', '_gr_drawpath', '_gr_setarrowstyle', '_gr_drawarrow', '_gr_readimage', '_gr_drawimage', '_gr_importgraphics', '_gr_setshadow', '_gr_settransparency', '_gr_setcoordxform', '_gr_begingraphics', '_gr_endgraphics', '_gr_drawgraphics', '_gr_mathtex', '_gr_beginselection', '_gr_endselection', '_gr_moveselection', '_gr_resizeselection', '_gr_inqbbox', '_gr_precision', '_gr_setregenflags', '_gr_inqregenflags', '_gr_savestate', '_gr_restorestate', '_gr_selectcontext', '_gr_destroycontext', '_gr_uselinespec', '_gr_selntran', '_gks_get_dash_list', '_gr_shade', '_gr_shadepoints', '_gr_shadelines', '_gr_panzoom', '_gr_readmeta', '_gr_newmeta', '_gr_meta_args_push', '_gr_deletemeta', '_gr_plotmeta', '_gr_get_stdout', '_gr_dumpmeta_json', '_gr_dumpmeta', '_gr_inputmeta', '_gr_mergemeta', '_gr_mergemeta_named', '_gr_switchmeta', '_gr_meta_get_box', '_gr_registermeta', '_gr_unregistermeta', '_gr_dumpmeta_json_str', '_gr_load_from_str']"

clean:
	rm -f libGR.js gr.js *.js.mem
	rm -rf fonts
	cd $(JPEGDIR) && make clean
	cd $(PNGDIR) && make clean
	cd $(ZLIBDIR) && make clean
	cd $(QHULLDIR) && make clean
	rm -rf $(OBJS)
