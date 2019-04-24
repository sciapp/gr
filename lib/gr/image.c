#ifdef _WIN32
#include <windows.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <jpeglib.h>
#include <jerror.h>
#include <png.h>

#include "gr.h"

static int read_jpeg_image(char *path, int *width, int *height, int **data)
{
  FILE *stream;
  int i, x, y;
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  JSAMPARRAY buffer;
  int row_stride, bpix;
  unsigned int r, g, b, a;
  int *dataP;

  if ((stream = fopen(path, "rb")) != NULL)
    {
      cinfo.err = jpeg_std_error(&jerr);
      jpeg_create_decompress(&cinfo);

      jpeg_stdio_src(&cinfo, stream);
      jpeg_read_header(&cinfo, TRUE);
      jpeg_start_decompress(&cinfo);

      *width = cinfo.output_width;
      *height = cinfo.output_height;
      *data = dataP = (int *)malloc(cinfo.output_width * cinfo.output_height * sizeof(int));

      row_stride = cinfo.output_width * cinfo.output_components;
      buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);
      bpix = cinfo.output_components;
      y = 0;
      while (cinfo.output_scanline < cinfo.output_height)
        {
          jpeg_read_scanlines(&cinfo, buffer, 1);
          x = 0;
          for (i = 0; i < bpix * cinfo.output_width; i += bpix)
            {
              r = buffer[0][i];
              g = buffer[0][i + 1] << 8;
              b = buffer[0][i + 2] << 16;
              if (bpix == 4)
                a = buffer[0][i + 3] << 24;
              else
                a = 255 << 24;
              *dataP++ = r | g | b | a;
              x++;
            }
          y++;
        }
      jpeg_finish_decompress(&cinfo);

      jpeg_destroy_decompress(&cinfo);
      fclose(stream);

      return 0;
    }
  else
    return -1;
}

#define PNG_BYTES_TO_CHECK 4

static int read_png_image(char *path, int *width, int *height, int **data)
{
  static FILE *stream;
  unsigned char header[PNG_BYTES_TO_CHECK];
  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep *row_pointers;
  int x, y, channels, color_type;
  unsigned int r, g, b, a;
  int *dataP;
  int ret = -1;
  size_t nbytes;

  if ((stream = fopen(path, "rb")) != NULL)
    {
      nbytes = fread(header, 1, PNG_BYTES_TO_CHECK, stream);
      if (nbytes > 0 && !png_sig_cmp(header, 0, PNG_BYTES_TO_CHECK))
        {
          png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

          if (png_ptr)
            {
              info_ptr = png_create_info_struct(png_ptr);
              if (info_ptr)
                {
                  if (setjmp(png_jmpbuf(png_ptr)))
                    {
                      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
                      fclose(stream);
                      return -1;
                    }

                  png_init_io(png_ptr, stream);
                  png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);

                  png_read_info(png_ptr, info_ptr);
                  color_type = png_get_color_type(png_ptr, info_ptr);

                  if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);

                  if (color_type == PNG_COLOR_TYPE_GRAY)
                    channels = 1;
                  else if (color_type == PNG_COLOR_TYPE_RGBA)
                    channels = 4;
                  else
                    channels = 3;

                  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
                    {
                      png_set_tRNS_to_alpha(png_ptr);
                      channels += 1;
                    }

                  if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE ||
                      color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGBA)
                    {
                      *width = png_get_image_width(png_ptr, info_ptr);
                      *height = png_get_image_height(png_ptr, info_ptr);
                      *data = dataP = (int *)malloc(*width * *height * sizeof(int));

                      (void)png_set_interlace_handling(png_ptr);
                      png_read_update_info(png_ptr, info_ptr);

                      row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * *height);
                      for (y = 0; y < *height; y++)
                        row_pointers[y] = (png_byte *)malloc(png_get_rowbytes(png_ptr, info_ptr));

                      png_read_image(png_ptr, row_pointers);

                      for (y = 0; y < *height; y++)
                        {
                          png_byte *row = row_pointers[y];
                          for (x = 0; x < *width; x++)
                            {
                              png_byte *ptr = &(row[x * channels]);
                              if (channels == 1)
                                r = g = b = ptr[0];
                              else
                                {
                                  r = ptr[0];
                                  g = ptr[1];
                                  b = ptr[2];
                                }
                              if (channels > 3)
                                a = ptr[3];
                              else
                                a = 255;
                              *dataP++ = r | (g << 8) | (b << 16) | (a << 24);
                            }
                        }

                      for (y = 0; y < *height; y++) free(row_pointers[y]);
                      free(row_pointers);

                      ret = 0;
                    }
                  else
                    fprintf(stderr, "unsupported PNG color type\n");
                }
              else
                fprintf(stderr, "PNG information structure allocation error\n");

              png_destroy_read_struct(&png_ptr, NULL, NULL);
            }
          else
            fprintf(stderr, "PNG structure allocation error\n");
        }
      else
        fprintf(stderr, "file %s is not recognized as a PNG file\n", path);

      fclose(stream);
    }
  else
    fprintf(stderr, "file %s could not be opened for reading\n", path);

  return ret;
}

int gr_readimage(char *path, int *width, int *height, int **data)
{
  FILE *stream;
  char header[10];
  int ret = -1;
  size_t nbytes;

  if ((stream = fopen(path, "rb")) != NULL)
    {
      nbytes = fread(header, 1, 10, stream);
      fclose(stream);

      if (nbytes > 0)
        {
          if (!strncmp(header, "\211PNG\r\n\032\n", 8))
            {
              ret = read_png_image(path, width, height, data);
            }
          else if ((!strncmp(header, "\377\330\377\340", 4) || !strncmp(header, "\377\330\377\356", 4)) &&
                   !strncmp(header + 6, "JFIF", 4))
            {
              ret = read_jpeg_image(path, width, height, data);
            }
          else
            ret = -1;
        }
      else
        ret = -1;
    }

  return ret;
}
