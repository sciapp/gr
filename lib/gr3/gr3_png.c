#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <libpng16/png.h>
#include "gr3.h"
#include "gr3_internals.h"

int gr3_export_png_(const char *filename, int width, int height) {
  FILE *pngfp;
  int *pixels;
  int err;
  int i;
  
  png_structp png_ptr;
  png_infop info_ptr;
  png_bytepp row_pointers;
  
  pngfp = fopen(filename, "wb");
  if (!pngfp) {
    RETURN_ERROR(GR3_ERROR_CANNOT_OPEN_FILE);
  }
  pixels = (int *)malloc(width * height * sizeof(int));
  if (!pixels) {
    RETURN_ERROR(GR3_ERROR_OUT_OF_MEM);
  }
  
  err = gr3_getimage(width, height, 1, (char *)pixels);
  if (err != GR3_ERROR_NONE) {
    fclose(pngfp);
    free(pixels);
    return err;
  }
  
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    fclose(pngfp);
    free(pixels);
    RETURN_ERROR(GR3_ERROR_EXPORT);
  }
  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    fclose(pngfp);
    free(pixels);
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    RETURN_ERROR(GR3_ERROR_EXPORT);
  }
  png_init_io(png_ptr, pngfp);
  png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  
  row_pointers = malloc(height*sizeof(png_bytep));
  for (i = 0; i < height; i++) {
    row_pointers[i]=(png_bytep)(pixels+(height-i-1)*width);
  }
  png_set_rows(png_ptr, info_ptr, (void *)row_pointers);
  png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
  
  png_destroy_write_struct(&png_ptr, &info_ptr);
  fclose(pngfp);
  free(row_pointers);
  free(pixels);
  return GR3_ERROR_NONE;
}

int gr3_readpngtomemory_(int *pixels, const char *pngfile, int width, int height) {
  FILE *png_fp;
  png_structp png_ptr;
  png_infop info_ptr = NULL;
  png_infop end_info = NULL;
  unsigned char sig[8];
  png_bytep *row_pointers;
  int i;
  png_fp = fopen(pngfile,"rb");
  if (!png_fp) {
    return 1;
  }
  i = fread(sig, 1, 8, png_fp);
  if (!png_check_sig(sig, 8)){
    return 2;
  }
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    return 3;
  }
  info_ptr = png_create_info_struct(png_ptr);
  end_info = png_create_info_struct(png_ptr);
  if (!info_ptr || !end_info) {
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    return 4;
  }
  png_init_io(png_ptr, png_fp);
  png_set_sig_bytes(png_ptr, 8);
  png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
  row_pointers = png_get_rows(png_ptr, info_ptr);
  for (i = 0; i < height; i++) {
    memmove(pixels+(height-1-i)*width,row_pointers[i],width*sizeof(int));
  }
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
  fclose(png_fp);
  return 0;
}
