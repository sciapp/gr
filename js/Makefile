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
		--no-heap-copy \
		-s USE_ICU=1
ifndef DEBUG
    OPTS  += -O3
else
    OPTS  += -s ASSERTIONS=2 -s SAFE_HEAP=1 -s STACK_OVERFLOW_CHECK=2
endif
   DEFINES = -DEMSCRIPTEN -D__SVR4 -DGRDIR=\"\" -DNO_GL -DNO_THREADS
    CFLAGS = $(DEFINES) $(OPTS) $(EXTRA_CFLAGS)
  CXXFLAGS = $(DEFINES) $(OPTS) -std=c++17 $(EXTRA_CXXFLAGS)
   LDFLAGS = $(DEFINES) $(OPTS) $(EXTRA_LDFLAGS)
   JPEGDIR = ../3rdparty/jpeg
    PNGDIR = ../3rdparty/libpng16
   ZLIBDIR = ../3rdparty/zlib
  QHULLDIR = ../3rdparty/qhull
     FTDIR = ../3rdparty/freetype
XERCESCDIR = ../3rdparty/xerces-c
    GKSDIR = ../lib/gks
     GRDIR = ../lib/gr
    GR3DIR = ../lib/gr3
    GRMDIR = ../lib/grm
  INCLUDES = -I$(THIRDPARTYDIR)/include/ \
             -I$(GRMDIR)/include \
             -I$(GRMDIR)/src \
             -I$(GR3DIR) \
             -I$(GRDIR) \
             -I$(GKSDIR) \
             -I$(JPEGDIR) \
             -I$(PNGDIR) \
             -I$(ZLIBDIR) \
             -I$(QHULLDIR) \
             -I$(FTDIR)
   GKSOBJS = jsplugin.o \
             $(GKSDIR)/afm.o \
             $(GKSDIR)/error.o \
             $(GKSDIR)/font.o \
             $(GKSDIR)/ft.o \
             $(GKSDIR)/gks.o \
             $(GKSDIR)/io.o \
             $(GKSDIR)/malloc.o \
             $(GKSDIR)/util.o \
             $(GKSDIR)/wiss.o
    GROBJS = $(GRDIR)/contour.o \
             $(GRDIR)/contourf.o \
             $(GRDIR)/delaunay.o \
             $(GRDIR)/gr.o \
             $(GRDIR)/grforbnd.o \
             $(GRDIR)/gridit.o \
             $(GRDIR)/image.o \
             $(GRDIR)/import.o \
             $(GRDIR)/interp2.o \
             $(GRDIR)/mathtex2.o \
             $(GRDIR)/mathtex2.tab.o \
             $(GRDIR)/mathtex2_kerning.o \
             $(GRDIR)/md5.o \
             $(GRDIR)/shade.o \
             $(GRDIR)/spline.o \
             $(GRDIR)/stream.o \
             $(GRDIR)/strlib.o \
             $(GRDIR)/text.o
   GR3OBJS = $(GR3DIR)/gr3.o \
             $(GR3DIR)/gr3_convenience.o \
             $(GR3DIR)/gr3_gr.o \
             $(GR3DIR)/gr3_html.o \
             $(GR3DIR)/gr3_jpeg.o \
             $(GR3DIR)/gr3_mc.o \
             $(GR3DIR)/gr3_png.o \
             $(GR3DIR)/gr3_povray.o \
             $(GR3DIR)/gr3_sr.o
   GRMOBJS = $(GRMDIR)/src/grm/args.o \
             $(GRMDIR)/src/grm/backtrace.o \
             $(GRMDIR)/src/grm/base64.o \
             $(GRMDIR)/src/grm/dump.o \
             $(GRMDIR)/src/grm/dynamic_args_array.o \
             $(GRMDIR)/src/grm/error.o \
             $(GRMDIR)/src/grm/event.o \
             $(GRMDIR)/src/grm/interaction.o \
             $(GRMDIR)/src/grm/json.o \
             $(GRMDIR)/src/grm/layout.o \
             $(GRMDIR)/src/grm/layout_c.o \
             $(GRMDIR)/src/grm/layout_error.o \
             $(GRMDIR)/src/grm/logging.o \
             $(GRMDIR)/src/grm/memwriter.o \
             $(GRMDIR)/src/grm/net.o \
             $(GRMDIR)/src/grm/plot.o \
             $(GRMDIR)/src/grm/util.o \
             $(GRMDIR)/src/grm/utilcpp.o \
             $(GRMDIR)/src/grm/import.o \
             $(GRMDIR)/src/grm/datatype/double_map.o \
             $(GRMDIR)/src/grm/datatype/size_t_list.o \
             $(GRMDIR)/src/grm/datatype/string_array_map.o \
             $(GRMDIR)/src/grm/datatype/string_list.o \
             $(GRMDIR)/src/grm/datatype/string_map.o \
             $(GRMDIR)/src/grm/datatype/uint_map.o \
             $(GRMDIR)/src/grm/dom_render/context.o \
             $(GRMDIR)/src/grm/dom_render/Drawable.o \
             $(GRMDIR)/src/grm/dom_render/ManageCustomColorIndex.o \
             $(GRMDIR)/src/grm/dom_render/ManageGRContextIds.o \
             $(GRMDIR)/src/grm/dom_render/ManageZIndex.o \
             $(GRMDIR)/src/grm/dom_render/render.o \
             $(GRMDIR)/src/grm/dom_render/graphics_tree/Comment.o \
             $(GRMDIR)/src/grm/dom_render/graphics_tree/Document.o \
             $(GRMDIR)/src/grm/dom_render/graphics_tree/Element.o \
             $(GRMDIR)/src/grm/dom_render/graphics_tree/Node.o \
             $(GRMDIR)/src/grm/dom_render/graphics_tree/Value.o \
             $(GRMDIR)/src/grm/dom_render/graphics_tree/util.o
      OBJS = $(GRMOBJS) $(GR3OBJS) $(GROBJS) $(GKSOBJS)
      LIBS = $(JPEGDIR)/libjpeg.a \
             $(PNGDIR)/libpng.a \
             $(ZLIBDIR)/libz.a \
             $(QHULLDIR)/libqhull_r.a \
             $(FTDIR)/libfreetype.a \
             $(XERCESCDIR)/libxerces-c.a

.PHONY: default clean .SECONDARY .FORCE

.SUFFIXES:
.SUFFIXES: .c .cxx .o

.SECONDARY:

.FORCE:

default: gr.js

../lib/gr/gr_version.h: .FORCE
	$(MAKE) -C ../lib/gr gr_version.h

gr.js: module.js libGR.js api.js jsterm.js
	cat $^ > $@

fonts:
	mkdir fonts
	cp -p $(GKSDIR)/fonts/CMUSerif-Math.ttf $(GKSDIR)/fonts/DejaVuSans.ttf $(GKSDIR)/fonts/STIXTwoMath.ttf $(GKSDIR)/fonts/gksfont.dat $(GKSDIR)/fonts/mathtex2_offsets.bin fonts/

$(JPEGDIR)/libjpeg.a:
	$(MAKE) -C $(JPEGDIR)

$(PNGDIR)/libpng.a:
	$(MAKE) -C $(PNGDIR)

$(ZLIBDIR)/libz.a:
	$(MAKE) -C $(ZLIBDIR)

$(QHULLDIR)/libqhull_r.a:
	$(MAKE) -C $(QHULLDIR)

$(FTDIR)/libfreetype.a:
	$(MAKE) -C $(FTDIR)

$(XERCESCDIR)/libxerces-c.a:
	$(MAKE) -C $(XERCESCDIR) libxerces-c.a

$(GRDIR)/gr.o: ../lib/gr/gr_version.h

%.o: %.c $(LIBS)
	$(CC) -c $(CFLAGS) -o $@ $< $(INCLUDES)

%.o: %.cxx $(LIBS)
	$(CXX) -c $(CXXFLAGS) -o $@ $< $(INCLUDES)

libGR.js: $(OBJS) $(LIBS) library.js fonts
	$(CXX) $(LDFLAGS) \
	--embed-file fonts/gksfont.dat --embed-file fonts/CMUSerif-Math.ttf --embed-file fonts/DejaVuSans.ttf --embed-file fonts/STIXTwoMath.ttf --embed-file fonts/mathtex2_offsets.bin \
	--js-library library.js \
	-o $@ $(OBJS) $(LIBS) \
	-s EXPORTED_FUNCTIONS="['_gr_opengks', '_gr_closegks', '_gr_inqdspsize', '_gr_openws', '_gr_closews', '_gr_activatews', '_gr_deactivatews', '_gr_clearws', '_gr_updatews', '_gr_polyline', '_gr_polymarker', '_gr_text', '_gr_inqtext', '_gr_fillarea', '_gr_cellarray', '_gr_spline', '_gr_gridit', '_gr_setlinetype', '_gr_inqlinetype', '_gr_setlinewidth', '_gr_inqlinewidth', '_gr_setlinecolorind', '_gr_inqlinecolorind', '_gr_setmarkertype', '_gr_inqmarkertype', '_gr_setmarkersize', '_gr_setmarkercolorind', '_gr_inqmarkercolorind', '_gr_settextfontprec', '_gr_setcharexpan', '_gr_setcharspace', '_gr_settextcolorind', '_gr_setcharheight', '_gr_setcharup', '_gr_settextpath', '_gr_settextalign', '_gr_setfillintstyle', '_gr_setfillstyle', '_gr_setfillcolorind', '_gr_setcolorrep', '_gr_setscale', '_gr_inqscale', '_gr_setwindow', '_gr_inqwindow', '_gr_setviewport', '_gr_inqviewport', '_gr_selntran', '_gr_setclip', '_gr_setwswindow', '_gr_setwsviewport', '_gr_createseg', '_gr_copysegws', '_gr_redrawsegws', '_gr_setsegtran', '_gr_closeseg', '_gr_emergencyclosegks', '_gr_updategks', '_gr_setspace', '_gr_inqspace', '_gr_textext', '_gr_inqtextext', '_gr_setscientificformat', '_gr_axes', '_gr_grid', '_gr_verrorbars', '_gr_herrorbars', '_gr_polyline3d', '_gr_axes3d', '_gr_titles3d', '_gr_surface', '_gr_contour', '_gr_hexbin', '_gr_setcolormap', '_gr_inqcolormap', '_gr_colorbar', '_gr_inqcolor', '_gr_inqcolorfromrgb', '_gr_hsvtorgb', '_gr_tick', '_gr_validaterange', '_gr_adjustrange', '_gr_beginprint', '_gr_beginprintext', '_gr_endprint', '_gr_ndctowc', '_gr_wctondc', '_gr_drawrect', '_gr_fillrect', '_gr_drawarc', '_gr_fillarc', '_gr_drawpath', '_gr_setarrowstyle', '_gr_drawarrow', '_gr_readimage', '_gr_drawimage', '_gr_importgraphics', '_gr_setshadow', '_gr_settransparency', '_gr_setcoordxform', '_gr_begingraphics', '_gr_endgraphics', '_gr_drawgraphics', '_gr_mathtex', '_gr_beginselection', '_gr_endselection', '_gr_moveselection', '_gr_resizeselection', '_gr_inqbbox', '_gr_precision', '_gr_setregenflags', '_gr_inqregenflags', '_gr_savestate', '_gr_restorestate', '_gr_selectcontext', '_gr_destroycontext', '_gr_uselinespec', '_gr_selntran', '_gks_get_dash_list', '_gr_shade', '_gr_shadepoints', '_gr_shadelines', '_gr_panzoom', '_gr_path', '_gr_setborderwidth', '_gr_inqborderwidth', '_gr_setbordercolorind', '_gr_inqbordercolorind', '_gr_selectclipxform', '_gr_inqclipxform', '_gr_settextencoding', '_grm_read', '_grm_args_new', '_grm_args_push', '_grm_args_delete', '_grm_plot', '_grm_get_stdout', '_grm_dump_json', '_grm_dump', '_grm_input', '_grm_merge', '_grm_merge_named', '_grm_switch', '_grm_get_box', '_grm_get_tooltip', '_grm_register', '_grm_unregister', '_grm_dump_json_str', '_grm_load_from_str', '_grm_is3d']"

clean:
	rm -f libGR.js gr.js *.js.mem
	rm -rf fonts
	cd $(JPEGDIR) && make clean
	cd $(PNGDIR) && make clean
	cd $(ZLIBDIR) && make clean
	cd $(QHULLDIR) && make clean
	cd $(FTDIR) && make clean
	cd $(XERCESCDIR) && make clean
	rm -rf $(OBJS)
