#include "gr3.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <jpeglib.h>
#include "gr3_internals.h"

int gr3_export_jpeg_(const char *filename, int width, int height) {
  FILE *jpegfp;
  char *pixels;
  int err;
  
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  
  jpegfp = fopen(filename, "wb");
  if (!jpegfp) {
    RETURN_ERROR(GR3_ERROR_CANNOT_OPEN_FILE);
  }
  
  pixels = (char *)malloc(width * height * 3);
  if (!pixels) {
    RETURN_ERROR(GR3_ERROR_OUT_OF_MEM);
  }
  
  err = gr3_getimage(width, height, FALSE, pixels);
  if (err != GR3_ERROR_NONE) {
    fclose(jpegfp);
    free(pixels);
    return err;
  }
  
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  jpeg_stdio_dest(&cinfo, jpegfp);
  
  cinfo.image_width = width;
  cinfo.image_height = height;
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;
  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, 100, TRUE);
  jpeg_start_compress(&cinfo, TRUE);
  while (cinfo.next_scanline < cinfo.image_height) {
    JSAMPROW row_pointer[1];
    row_pointer[0] = (JSAMPLE *)(pixels+3*(height-cinfo.next_scanline-1)*width);
    jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }
  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);
  fclose(jpegfp);
  free(pixels);
  return GR3_ERROR_NONE;
}
