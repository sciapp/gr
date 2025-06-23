ifeq ($(strip $(PREFIX)),)
override PREFIX = $(abspath $(CURDIR)/../build)
endif

 CXXFLAGS = -Wall -pedantic -O -Iinclude -std=c++11 -fPIC -fno-common $(ARCHS)
ifndef OS
ifeq ($(shell uname),Darwin)
CXXFLAGS += -mmacosx-version-min=10.11
endif
else ifeq ($(shell [[ "$(OS)" =~ [dD]"arwin" ]] && echo macos), macos)
CXXFLAGS += -mmacosx-version-min=10.11
endif
     OBJS = agg_arc.o \
        agg_arrowhead.o \
        agg_bezier_arc.o \
        agg_bspline.o \
        agg_color_rgba.o \
        agg_curves.o \
        agg_embedded_raster_fonts.o \
        agg_gsv_text.o \
        agg_image_filters.o \
        agg_line_aa_basics.o \
        agg_line_profile_aa.o \
        agg_rounded_rect.o \
        agg_sqrt_tables.o \
        agg_trans_affine.o \
        agg_trans_double_path.o \
        agg_trans_single_path.o \
        agg_trans_warp_magnifier.o \
        agg_vcgen_bspline.o \
        agg_vcgen_contour.o \
        agg_vcgen_dash.o \
        agg_vcgen_markers_term.o \
        agg_vcgen_smooth_poly1.o \
        agg_vcgen_stroke.o \
        agg_vpgen_clip_polygon.o \
        agg_vpgen_clip_polyline.o \
        agg_vpgen_segmentator.o \

default: install

libagg.a: $(OBJS)
	$(AR) crv $@ $?
	$(AR) ts $@

distclean: clean

clean:
	rm -f libagg.a *.o

$(PREFIX)/lib/libagg.a: libagg.a
	mkdir -p $(PREFIX)/lib/
	mkdir -p $(PREFIX)/include/
	cp -p libagg.a $(PREFIX)/lib/
	cp -p include/*.h $(PREFIX)/include/

install: $(PREFIX)/lib/libagg.a

%.o: src/%.cpp
	$(CXX) -c $(CXXFLAGS) src/$*.cpp -o $@

