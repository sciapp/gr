
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gr.h"

#include <jpeglib.h>
#include <jerror.h>
#include <png.h>

static
int read_jpeg_image(char *path, int *width, int *height, int **data)
{
  FILE *stream;
  int i, x, y;
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  JSAMPARRAY buffer;
  int row_stride, bpix;
  unsigned int r, g, b;
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
      *data = dataP =
	(int *) malloc(cinfo.output_width * cinfo.output_height *
		       sizeof(int));

      row_stride =
	cinfo.output_width * cinfo.output_height * 3 * sizeof(char);
      buffer =
	(*cinfo.mem->alloc_sarray) ((j_common_ptr) & cinfo, JPOOL_IMAGE,
				    row_stride, 1);
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
	      *dataP++ = r | g | b;
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

static
int read_png_image(char *path, int *width, int *height, int **data)
{
  static FILE *stream;
  unsigned char header[PNG_BYTES_TO_CHECK];
  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep *row_pointers;
  int x, y;
  unsigned int r, g, b;
  int *dataP;
  int ret = -1;
  size_t nbytes;

  if ((stream = fopen(path, "rb")) != NULL)
    {
      nbytes = fread(header, 1, PNG_BYTES_TO_CHECK, stream);
      if (nbytes > 0 && !png_sig_cmp(header, 0, PNG_BYTES_TO_CHECK))
	{
	  png_ptr =
	    png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

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

		  if (info_ptr->color_type == PNG_COLOR_TYPE_RGB ||
		      info_ptr->color_type == PNG_COLOR_TYPE_RGBA)
		    {
		      int bytes_per_pixel;

		      bytes_per_pixel =
			info_ptr->color_type == PNG_COLOR_TYPE_RGB ? 3 : 4;

		      *width = info_ptr->width;
		      *height = info_ptr->height;
		      *data = dataP =
			(int *) malloc(info_ptr->width * info_ptr->height *
				       sizeof(int));

		      (void) png_set_interlace_handling(png_ptr);
		      png_read_update_info(png_ptr, info_ptr);

		      row_pointers =
			(png_bytep *) malloc(sizeof(png_bytep) * *height);
		      for (y = 0; y < *height; y++)
			row_pointers[y] =
			  (png_byte *) malloc(info_ptr->rowbytes);

		      png_read_image(png_ptr, row_pointers);

		      for (y = 0; y < *height; y++)
			{
			  png_byte *row = row_pointers[y];
			  for (x = 0; x < *width; x++)
			    {
			      png_byte *ptr = &(row[x * bytes_per_pixel]);
			      r = ptr[0];
			      g = ptr[1] << 8;
			      b = ptr[2] << 16;
			      *dataP++ = r | g | b;
			    }
			}

		      for (y = 0; y < *height; y++)
			free(row_pointers[y]);
		      free(row_pointers);

		      ret = 0;
		    }
		  else
		    fprintf(stderr, "unsupported PNG color type\n");
		}
	      else
		fprintf(stderr,
			"PNG information structure allocation error\n");

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
	  else if ((!strncmp(header, "\377\330\377\340", 4) ||
		    !strncmp(header, "\377\330\377\356", 4)) &&
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
