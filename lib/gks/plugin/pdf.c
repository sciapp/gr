#if !defined(NO_AV) && !defined(NO_MUPDF)

#include <stdio.h>

#include "pdf.h"

pdf_t vc_pdf_from_file(const char *path)
{
    pdf_t pdf = (pdf_t) malloc(sizeof(struct pdf_t_));

    pdf->ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
    fz_register_document_handlers(pdf->ctx);
    pdf->doc = fz_open_document(pdf->ctx, path);
#ifdef MUPDF_API_VERSION_17
    pdf->num_pages = fz_count_pages(pdf->ctx, pdf->doc);
#else
    pdf->num_pages = fz_count_pages(pdf->doc);
#endif
    pdf->path = path;

    return pdf;
}

pdf_t vc_pdf_from_memory(unsigned char *data, int len)
{
    fz_stream *stream;

    pdf_t pdf = (pdf_t) malloc(sizeof(struct pdf_t_));

    pdf->ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
    fz_register_document_handlers(pdf->ctx);
    stream = fz_open_memory(pdf->ctx, data, len);
    pdf->doc = fz_open_document_with_stream(pdf->ctx, "pdf", stream);
#ifdef MUPDF_API_VERSION_17
    pdf->num_pages = fz_count_pages(pdf->ctx, pdf->doc);
#else
    pdf->num_pages = fz_count_pages(pdf->doc);
#endif
    pdf->path = "stream";

    return pdf;
}

frame_t vc_frame_from_pdf(pdf_t pdf, int page, int width, int height)
{
    double transx, transy, zoom;
    fz_matrix transform, scale_mat, transl_mat;
    fz_rect rect;
    fz_irect bbox;
    fz_pixmap *pix;
    fz_device *dev;
    fz_page *page_o;
    unsigned char *data;

    frame_t frame = (frame_t) malloc(sizeof(struct frame_t_));

#ifdef MUPDF_API_VERSION_17
    page_o = fz_load_page(pdf->ctx, pdf->doc, page - 1);
#else
    page_o = fz_load_page(pdf->doc, page - 1);
#endif

    transx = 0;
    transy = 0;
    zoom = 1.0;
    fz_scale(&scale_mat, zoom, zoom);
    fz_translate(&transl_mat, transx, transy);
    fz_concat(&transform, &scale_mat, &transl_mat);

    /*
     * Take the page bounds and transform them by the same matrix that
     * we will use to render the page.
     */

#ifdef MUPDF_API_VERSION_17
    fz_bound_page(pdf->ctx, page_o, &rect);
#else
    fz_bound_page(pdf->doc, page_o, &rect);
#endif
    fz_transform_rect(&rect, &transform);
    fz_round_rect(&bbox, &rect);

    /*
     * Create a blank pixmap to hold the result of rendering. The
     * pixmap bounds used here are the same as the transformed page
     * bounds, so it will contain the entire page.
     */

    pix = fz_new_pixmap(pdf->ctx, fz_device_rgb(pdf->ctx), width, height);
    fz_clear_pixmap_with_value(pdf->ctx, pix, 0xff);

    /*
     * Create a draw device with the pixmap as its target.
     * Run the page with the transform.
     */

    dev = fz_new_draw_device(pdf->ctx, pix);
#ifdef MUPDF_API_VERSION_17
    fz_run_page(pdf->ctx, page_o, dev, &transform, NULL);
#else
    fz_run_page(pdf->doc, page_o, dev, &transform, NULL);
#endif

    frame->data = (unsigned char *) malloc(width * height * 4 * sizeof(unsigned char));
    data = fz_pixmap_samples(pdf->ctx, pix);
    memmove(frame->data, data, width * height * 4 * sizeof(unsigned char));
    frame->width  = width;
    frame->height = height;

#ifdef MUPDF_API_VERSION_17
    fz_drop_device(pdf->ctx, dev);
  fz_drop_pixmap(pdf->ctx, pix);
  fz_drop_page(pdf->ctx, page_o);
#else
    fz_free_device(dev);
    fz_drop_pixmap(pdf->ctx, pix);
    fz_free_page(pdf->doc, page_o);
#endif

    return frame;
}

frame_t *vc_pdf_to_frames(pdf_t pdf, int width, int height)
{
    int i;
    frame_t *frames;

    frames = (frame_t *) malloc(sizeof(frame_t) * pdf->num_pages);
    for (i = 0; i < pdf->num_pages; i++) {
        frames[i] = vc_frame_from_pdf(pdf, i + 1, width, height);
    }

    return frames;
}

int vc_pdf_get_number_of_pages(pdf_t pdf)
{
    return pdf->num_pages;
}

void vc_pdf_close(pdf_t pdf)
{
#ifdef MUPDF_API_VERSION_17
    fz_drop_document(pdf->ctx, pdf->doc);
  fz_drop_context(pdf->ctx);
#else
    fz_close_document(pdf->doc);
    fz_free_context(pdf->ctx);
#endif
    pdf->num_pages = -1;
}

#endif