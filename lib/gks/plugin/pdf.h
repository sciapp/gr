#ifndef _PDF_H_
#define _PDF_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define __STDC_CONSTANT_MACROS
#include <mupdf/fitz.h>
#include "vc.h"

  typedef struct pdf_t_ *pdf_t;

  struct pdf_t_
  {
    fz_context *ctx;
    fz_document *doc;
    const char *path;
    int num_pages;
  };

  pdf_t vc_pdf_from_file(const char *path);
  pdf_t vc_pdf_from_memory(unsigned char *data, int len);
  int vc_pdf_get_number_of_pages(pdf_t pdf);
  frame_t *vc_pdf_to_frames(pdf_t pdf, int width, int height);
  void vc_pdf_close(pdf_t pdf);

#ifdef __cplusplus
}
#endif

#endif