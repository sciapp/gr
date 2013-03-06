INCLUDES = -I../jbig2dec -I../jpeg -I ../png -I../openjpeg -I../zlib \
           -I/usr/X11/include/freetype2
  CFLAGS = -O -I. -fPIC -fno-common $(INCLUDES) $(ARCHS)
    OBJS = \
        base_context.o \
        base_error.o \
        base_geometry.o \
        base_getopt.o \
        base_hash.o \
        base_memory.o \
        base_string.o \
        base_time.o \
        base_trans.o \
        base_xml.o \
        crypt_aes.o \
        crypt_arc4.o \
        crypt_md5.o \
        crypt_sha2.o \
        dev_bbox.o \
        dev_list.o \
        dev_null.o \
        dev_text.o \
        dev_trace.o \
        doc_document.o \
        doc_interactive.o \
        doc_link.o \
        doc_outline.o \
        doc_search.o \
        filt_basic.o \
        filt_dctd.o \
        filt_faxd.o \
        filt_flate.o \
        filt_jbig2d.o \
        filt_lzwd.o \
        filt_predict.o \
        image_jpeg.o \
        image_jpx.o \
        image_md5.o \
        image_png.o \
        image_save.o \
        image_tiff.o \
        memento.o \
        res_bitmap.o \
        res_colorspace.o \
        res_font.o \
        res_halftone.o \
        res_path.o \
        res_pixmap.o \
        res_shade.o \
        res_store.o \
        res_text.o \
        stm_buffer.o \
        stm_comp_buf.o \
        stm_open.o \
        stm_output.o \
        stm_read.o \
        pdf_annot.o \
        pdf_cmap.o \
        pdf_cmap_load.o \
        pdf_cmap_parse.o \
        pdf_cmap_table.o \
        pdf_colorspace.o \
        pdf_crypt.o \
        pdf_device.o \
        pdf_encoding.o \
        pdf_event.o \
        pdf_font.o \
        pdf_fontfile.o \
        pdf_form.o \
        pdf_function.o \
        pdf_image.o \
        pdf_interpret.o \
        pdf_js_none.o \
        pdf_jsimp_cpp.o \
        pdf_lex.o \
        pdf_metrics.o \
        pdf_nametree.o \
        pdf_object.o \
        pdf_outline.o \
        pdf_page.o \
        pdf_parse.o \
        pdf_pattern.o \
        pdf_repair.o \
        pdf_shade.o \
        pdf_store.o \
        pdf_stream.o \
        pdf_type3.o \
        pdf_unicode.o \
        pdf_write.o \
        pdf_xobject.o \
        pdf_xref.o \
        pdf_xref_aux.o \
        xps_common.o \
        xps_doc.o \
        xps_glyphs.o \
        xps_gradient.o \
        xps_image.o \
        xps_outline.o \
        xps_path.o \
        xps_resource.o \
        xps_tile.o \
        xps_util.o \
        xps_zip.o \
        mucbz.o \
        draw_affine.o \
        draw_blend.o \
        draw_device.o \
        draw_edge.o \
        draw_glyph.o \
        draw_mesh.o \
        draw_paint.o \
        draw_path.o \
        draw_scale.o \
        draw_unpack.o

libmupdf.a: $(OBJS)
	$(AR) crv $@ $?
	$(AR) ts $@

example: example.c libmupdf.a
	$(CC) $? -o $@ -I. libmupdf.a ../jbig2dec/libjbig2dec.a ../openjpeg/libopenjpeg.a ../jpeg/libjpeg.a ../png/libpng.a -lfreetype -lm -lz

distclean: clean

clean:
	rm -f libmupdf.a *.o *.bak
	rm -f example out.png

depend:
	makedepend -Y -- \
        base_context.c \
        base_error.c \
        base_geometry.c \
        base_getopt.c \
        base_hash.c \
        base_memory.c \
        base_string.c \
        base_time.c \
        base_trans.c \
        base_xml.c \
        crypt_aes.c \
        crypt_arc4.c \
        crypt_md5.c \
        crypt_sha2.c \
        dev_bbox.c \
        dev_list.c \
        dev_null.c \
        dev_text.c \
        dev_trace.c \
        doc_document.c \
        doc_interactive.c \
        doc_link.c \
        doc_outline.c \
        doc_search.c \
        filt_basic.c \
        filt_dctd.c \
        filt_faxd.c \
        filt_flate.c \
        filt_jbig2d.c \
        filt_lzwd.c \
        filt_predict.c \
        image_jpeg.c \
        image_jpx.c \
        image_md5.c \
        image_png.c \
        image_save.c \
        image_tiff.c \
        memento.c \
        res_bitmap.c \
        res_colorspace.c \
        res_font.c \
        res_halftone.c \
        res_path.c \
        res_pixmap.c \
        res_shade.c \
        res_store.c \
        res_text.c \
        stm_buffer.c \
        stm_comp_buf.c \
        stm_open.c \
        stm_output.c \
        stm_read.c \
        pdf_annot.c \
        pdf_cmap.c \
        pdf_cmap_load.c \
        pdf_cmap_parse.c \
        pdf_cmap_table.c \
        pdf_colorspace.c \
        pdf_crypt.c \
        pdf_device.c \
        pdf_encoding.c \
        pdf_event.c \
        pdf_font.c \
        pdf_fontfile.c \
        pdf_form.c \
        pdf_function.c \
        pdf_image.c \
        pdf_interpret.c \
        pdf_js_none.c \
        pdf_jsimp_cpp.c \
        pdf_lex.c \
        pdf_metrics.c \
        pdf_nametree.c \
        pdf_object.c \
        pdf_outline.c \
        pdf_page.c \
        pdf_parse.c \
        pdf_pattern.c \
        pdf_repair.c \
        pdf_shade.c \
        pdf_store.c \
        pdf_stream.c \
        pdf_type3.c \
        pdf_unicode.c \
        pdf_write.c \
        pdf_xobject.c \
        pdf_xref.c \
        pdf_xref_aux.c \
        xps_common.c \
        xps_doc.c \
        xps_glyphs.c \
        xps_gradient.c \
        xps_image.c \
        xps_outline.c \
        xps_path.c \
        xps_resource.c \
        xps_tile.c \
        xps_util.c \
        xps_zip.c \
        mucbz.c \
        draw_affine.c \
        draw_blend.c \
        draw_device.c \
        draw_edge.c \
        draw_glyph.c \
        draw_mesh.c \
        draw_paint.c \
        draw_path.c \
        draw_scale.c \
        draw_unpack.c 2> /dev/null

# DO NOT DELETE THIS LINE -- make depend depends on it.

base_context.o: fitz-internal.h fitz.h memento.h
base_error.o: fitz-internal.h fitz.h memento.h
base_geometry.o: fitz-internal.h fitz.h memento.h
base_hash.o: fitz-internal.h fitz.h memento.h
base_memory.o: fitz-internal.h fitz.h memento.h
base_string.o: fitz-internal.h fitz.h memento.h
base_trans.o: fitz-internal.h fitz.h memento.h
base_xml.o: fitz.h memento.h
crypt_aes.o: fitz-internal.h fitz.h memento.h
crypt_arc4.o: fitz-internal.h fitz.h memento.h
crypt_md5.o: fitz-internal.h fitz.h memento.h
crypt_sha2.o: fitz-internal.h fitz.h memento.h
dev_bbox.o: fitz-internal.h fitz.h memento.h
dev_list.o: fitz-internal.h fitz.h memento.h
dev_null.o: fitz-internal.h fitz.h memento.h
dev_text.o: fitz-internal.h fitz.h memento.h
dev_trace.o: fitz-internal.h fitz.h memento.h
doc_document.o: fitz-internal.h fitz.h memento.h
doc_interactive.o: fitz.h memento.h mupdf-internal.h mupdf.h fitz-internal.h
doc_link.o: fitz-internal.h fitz.h memento.h
doc_outline.o: fitz-internal.h fitz.h memento.h
doc_search.o: fitz-internal.h fitz.h memento.h
filt_basic.o: fitz-internal.h fitz.h memento.h
filt_dctd.o: fitz-internal.h fitz.h memento.h
filt_faxd.o: fitz-internal.h fitz.h memento.h
filt_flate.o: fitz-internal.h fitz.h memento.h
filt_jbig2d.o: fitz-internal.h fitz.h memento.h
filt_lzwd.o: fitz-internal.h fitz.h memento.h
filt_predict.o: fitz-internal.h fitz.h memento.h
image_jpeg.o: fitz-internal.h fitz.h memento.h
image_jpx.o: fitz-internal.h fitz.h memento.h
image_md5.o: fitz-internal.h fitz.h memento.h
image_png.o: fitz-internal.h fitz.h memento.h
image_save.o: fitz-internal.h fitz.h memento.h
image_tiff.o: fitz-internal.h fitz.h memento.h
memento.o: memento.h
res_bitmap.o: fitz-internal.h fitz.h memento.h
res_colorspace.o: fitz-internal.h fitz.h memento.h
res_font.o: fitz-internal.h fitz.h memento.h
res_halftone.o: fitz-internal.h fitz.h memento.h
res_path.o: fitz-internal.h fitz.h memento.h
res_pixmap.o: fitz-internal.h fitz.h memento.h
res_shade.o: fitz-internal.h fitz.h memento.h
res_store.o: fitz-internal.h fitz.h memento.h
res_text.o: fitz-internal.h fitz.h memento.h
stm_buffer.o: fitz-internal.h fitz.h memento.h
stm_comp_buf.o: fitz-internal.h fitz.h memento.h
stm_open.o: fitz-internal.h fitz.h memento.h
stm_output.o: fitz-internal.h fitz.h memento.h
stm_read.o: fitz-internal.h fitz.h memento.h
pdf_annot.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_cmap.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_cmap_load.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_cmap_parse.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_cmap_table.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_cmap_table.o: cmap_cns.h cmap_gb.h cmap_japan.h cmap_korea.h
pdf_colorspace.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_crypt.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_device.o: mupdf-internal.h mupdf.h fitz.h memento.h fitz-internal.h
pdf_encoding.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_encoding.o: data_encodings.h data_glyphlist.h
pdf_event.o: fitz.h memento.h mupdf-internal.h mupdf.h fitz-internal.h
pdf_font.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_fontfile.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_fontfile.o: font_base14.h font_droid.h font_cjk_full.h
pdf_form.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_function.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_image.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_interpret.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_js_none.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_jsimp_cpp.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_jsimp_cpp.o: pdf_jsimp_cpp.h
pdf_lex.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_metrics.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_nametree.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_object.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_outline.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_page.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_parse.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_pattern.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_repair.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_shade.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_store.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_stream.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_type3.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_unicode.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_write.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_xobject.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_xref.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
pdf_xref_aux.o: fitz-internal.h fitz.h memento.h mupdf-internal.h mupdf.h
xps_common.o: muxps-internal.h muxps.h fitz.h memento.h fitz-internal.h
xps_doc.o: muxps-internal.h muxps.h fitz.h memento.h fitz-internal.h
xps_glyphs.o: muxps-internal.h muxps.h fitz.h memento.h fitz-internal.h
xps_gradient.o: muxps-internal.h muxps.h fitz.h memento.h fitz-internal.h
xps_image.o: muxps-internal.h muxps.h fitz.h memento.h fitz-internal.h
xps_outline.o: muxps-internal.h muxps.h fitz.h memento.h fitz-internal.h
xps_path.o: muxps-internal.h muxps.h fitz.h memento.h fitz-internal.h
xps_resource.o: muxps-internal.h muxps.h fitz.h memento.h fitz-internal.h
xps_tile.o: muxps-internal.h muxps.h fitz.h memento.h fitz-internal.h
xps_util.o: muxps-internal.h muxps.h fitz.h memento.h fitz-internal.h
xps_zip.o: muxps-internal.h muxps.h fitz.h memento.h fitz-internal.h
mucbz.o: fitz-internal.h fitz.h memento.h mucbz.h
draw_affine.o: fitz-internal.h fitz.h memento.h
draw_blend.o: fitz-internal.h fitz.h memento.h
draw_device.o: fitz-internal.h fitz.h memento.h
draw_edge.o: fitz-internal.h fitz.h memento.h
draw_glyph.o: fitz-internal.h fitz.h memento.h
draw_mesh.o: fitz-internal.h fitz.h memento.h
draw_paint.o: fitz-internal.h fitz.h memento.h
draw_path.o: fitz-internal.h fitz.h memento.h
draw_scale.o: fitz-internal.h fitz.h memento.h
draw_unpack.o: fitz-internal.h fitz.h memento.h
