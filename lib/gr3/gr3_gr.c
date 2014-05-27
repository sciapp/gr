#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "gr.h"
#include "gr3.h"
#include "gr3_internals.h"

int gr3_drawimage_gks_(float xmin, float xmax, float ymin, float ymax, int width, int height) {
  double _xmin = (double) xmin, _xmax = (double) xmax;
  double _ymin = (double) ymin, _ymax = (double) ymax;
  char *pixels;
  int err;
  gr3_log_("gr3_drawimage_gks_();");
  pixels = (char *)malloc(sizeof(int)*width*height);
  if (!pixels) {
    return GR3_ERROR_OUT_OF_MEM;
  }
  err = gr3_getimage(width,height,1,pixels);
  if (err != GR3_ERROR_NONE) {
    free(pixels);
    return err;
  }
  gr_drawimage(_xmin, _xmax, _ymax, _ymin, width, height, (int *)pixels);
  free(pixels);
  return GR3_ERROR_NONE;
}
