#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "gif.h"


typedef struct {
  int prefix_symbol;
  unsigned char new_byte;
} LZWEntry;

#define SYMBOL_NONE (-1)
#define SYMBOL_CLEAR (0x100)
#define SYMBOL_STOP (0x101)
/* COLOR QUANTIZATION CODE START */
#define distance_squared(a, b) (((a)[0]-(b)[0])*((a)[0]-(b)[0])+((a)[1]-(b)[1])*((a)[1]-(b)[1])+((a)[2]-(b)[2])*((a)[2]-(b)[2]))

static int compare_color_component(const void *left, const void *right) {
  int l = ((const unsigned char *)left)[0];
  int r = ((const unsigned char *)right)[0];
  return l-r;
}

static void median_cut(unsigned char *pixels, unsigned char *color_table, int num_pixels, int num_colors, int format) {
  /* Perform a color quantization similar to the modified median cut quantization algorithm */
  if (num_pixels <= 0) {
    int i;
    for (i = 0; i < num_colors; i++) {
      color_table[3*i+0] = 0;
      color_table[3*i+1] = 0;
      color_table[3*i+2] = 0;
    }
    return;
  }
  if (num_colors == 1) {
    color_table[0] = pixels[num_pixels/2*format+0];
    color_table[1] = pixels[num_pixels/2*format+1];
    color_table[2] = pixels[num_pixels/2*format+2];
  } else {
    int i;
    int cut_axis;
    unsigned char cut_value;
    int left_pixels;
    unsigned char minred, mingreen, minblue, maxred, maxgreen, maxblue;
    minred = maxred = pixels[0];
    mingreen = maxgreen = pixels[1];
    minblue = maxblue = pixels[2];
    for (i = 1; i < num_pixels; i++) {
      if (pixels[i*format+0] < minred) minred = pixels[i*format+0];
      if (pixels[i*format+0] > maxred) maxred = pixels[i*format+0];
      if (pixels[i*format+1] < mingreen) mingreen = pixels[i*format+1];
      if (pixels[i*format+1] > maxgreen) maxgreen = pixels[i*format+1];
      if (pixels[i*format+2] < minblue) minblue = pixels[i*format+2];
      if (pixels[i*format+2] > maxblue) maxblue = pixels[i*format+2];
    }
    if (maxred - minred < maxgreen - mingreen) {
      if (maxgreen - mingreen < maxblue - minblue) {
        cut_axis = 2;
        cut_value = (minblue+maxblue)/2;
      } else {
        cut_axis = 1;
        cut_value = (mingreen+maxgreen)/2;
      }
    } else {
      if (maxred - minred < maxblue - minblue) {
        cut_axis = 2;
        cut_value = (minblue+maxblue)/2;
      } else {
        cut_axis = 0;
        cut_value = (minred+maxred)/2;
      }
    }
    qsort(pixels+cut_axis, num_pixels, format, compare_color_component);
    for (left_pixels = num_colors/2; (left_pixels < num_pixels-num_colors/2) && pixels[left_pixels*format+cut_axis] < cut_value; left_pixels++);
    median_cut(pixels, color_table, left_pixels, num_colors/2, format);
    median_cut(pixels+left_pixels*format, color_table+num_colors/2*3, num_pixels-left_pixels, num_colors/2, format);
  }
}

static unsigned char color_index_for_rgb(const unsigned char *rgb_pixel, const unsigned char *color_table) {
  /* Return the color index most closely matching this RGB color. */
  int color_index;
  int closest_color_index = 0;
  float closest_color_index_distance = -1;
  for (color_index = 0; color_index < 256; ++color_index) {
    float color_index_distance = distance_squared(&color_table[color_index*3], rgb_pixel);
    if (closest_color_index_distance < 0 || closest_color_index_distance > color_index_distance) {
      closest_color_index_distance = color_index_distance;
      closest_color_index = color_index;
    }
  }
  return closest_color_index;
}

/* COLOR QUANTIZATION CODE END */

/* LZW CODE START */

static int find_symbol_table_index(const LZWEntry *symbol_table, int symbol_index, unsigned char new_byte, int max_index) {
  int i;
  for (i = 0; i < max_index; i++) {
    if (i == SYMBOL_CLEAR || i == SYMBOL_STOP) {
      /* Skip reserved entries */
      continue;
    }
    if (symbol_table[i].prefix_symbol == symbol_index && symbol_table[i].new_byte == new_byte) {
      return i;
    }
  }
  return SYMBOL_NONE;
}

static void write_lzw_symbol(unsigned short symbol, int symbol_length, FILE *fp, long *size_position_p, int *bytes_written_p, unsigned char *buffered_byte_p, int *bit_offset_p) {
  unsigned char buffered_byte = *buffered_byte_p;
  int bit_offset = *bit_offset_p;
  int bytes_written = *bytes_written_p;

  while (symbol_length > 0) {
    buffered_byte |= ((symbol & (0x00ff >> bit_offset)) << bit_offset);
    symbol >>= (8-bit_offset);
    if (symbol_length + bit_offset >= 8) {
      symbol_length -= (8-bit_offset);
      fputc(buffered_byte, fp);
      bit_offset = 0;
      buffered_byte = 0;
      bytes_written++;
      if (bytes_written == 255) {
        bytes_written = 0;
        *size_position_p = ftell(fp);
        fputc(0xff, fp);
      }
    } else {
      bit_offset = bit_offset+symbol_length;
      symbol_length = 0;
    }
  }
  *buffered_byte_p = buffered_byte;
  *bit_offset_p = bit_offset;
  *bytes_written_p = bytes_written;

}

static void flush_lzw(FILE *fp, long *size_position_p, int *bytes_written_p, unsigned char *buffered_byte_p, int *bit_offset_p){
  unsigned char buffered_byte = *buffered_byte_p;
  int bit_offset = *bit_offset_p;
  int bytes_written = *bytes_written_p;

  if (bit_offset > 0) {
    fputc(buffered_byte, fp);
    bit_offset = 0;
    buffered_byte = 0;
    bytes_written++;
  }
  if (bytes_written > 0) {
    if (bytes_written != 255) {
      fseek(fp, *size_position_p, SEEK_SET);
      fputc(bytes_written, fp);
      fseek(fp, 0, SEEK_END);
    }
    fputc(0x00, fp);
  }
  *buffered_byte_p = buffered_byte;
  *bit_offset_p = bit_offset;
  *bytes_written_p = bytes_written;
}

/* LZW CODE END */

void gif_open(gif_writer *gwp, const char *file_name) {
  gwp->fp = fopen(file_name, "wb");
  gwp->width = -1;
  gwp->height = -1;
}

static void gif_write_header(gif_writer *gwp, unsigned short width, unsigned short height) {
  FILE *fp = gwp->fp;
  gwp->width = width;
  gwp->height = height;
  /* GIF Header */
  fputs("GIF89a", fp);
  fwrite(&width, 2, 1, fp);
  fwrite(&height, 2, 1, fp);
  fputc(0x00, fp); /* No Global Color Table */
  fputc(0x00, fp); /* Background Color 0 */
  fputc(0x00, fp); /* No Pixel Aspect Ratio */
  /* Application Extension Header */
  fputs("\x21\xff\x0bNETSCAPE2.0\x03\x01", fp);
  fputc(0x00, fp);
  fputc(0x00, fp);
  fputc(0x00, fp);
}

void gif_write(gif_writer *gwp, const unsigned char *rgb_image, unsigned short width, unsigned short height, int format, int delay) {
  LZWEntry symbol_table[4096];
  int pixel_index = 0;
  unsigned char color_table[256*3];
  assert(gwp);
  assert(gwp->fp);
  if (gwp->width == -1) {
    gif_write_header(gwp, width, height);
  }
  assert(width == gwp->width);
  assert(height == gwp->height);

  /* Graphics Control Extension Header */
  fputs("\x21\xf9", gwp->fp);
  fputs("\x04\x08", gwp->fp);
  fwrite(&delay, 2, 1, gwp->fp);
  fputc(0x00, gwp->fp);
  fputc(0x00, gwp->fp);

  /* Image Data */
  fputc(0x2c, gwp->fp);
  fputc(0x00, gwp->fp);
  fputc(0x00, gwp->fp);
  fputc(0x00, gwp->fp);
  fputc(0x00, gwp->fp);
  fwrite(&width, 2, 1, gwp->fp);
  fwrite(&height, 2, 1, gwp->fp);
  fputc(0x87, gwp->fp);
  {
    /* Local Color Table */
    unsigned char *rgb_copy = (unsigned char *)malloc(width*height*format);
    assert(rgb_copy);
    memset(color_table, 0, 256*3);
    memmove(rgb_copy, rgb_image, width*height*format);
    median_cut(rgb_copy, color_table, width*height, 256, format);
    free(rgb_copy);
    fwrite(color_table, 3, 256, gwp->fp);
  }

  fputc(0x08, gwp->fp); /* 8bit based LZW data */
  {
    /* LZW stream compression and output */
    unsigned char buffered_byte = 0;
    int bit_offset = 0;
    int bytes_written = 0;
    int i;
    int free_symbol = 0x102;
    int symbol = SYMBOL_NONE;
    int max_used_symbol_length = 9;
    for (i = 0; i < 256; ++i) {
      symbol_table[i].prefix_symbol = SYMBOL_NONE;
      symbol_table[i].new_byte = i;
    }
    /* The number of bytes written will be updated later if there were less than 255 in the last block */
    long size_position = ftell(gwp->fp);
    fputc(255, gwp->fp);
    write_lzw_symbol(SYMBOL_CLEAR, max_used_symbol_length, gwp->fp, &size_position, &bytes_written, &buffered_byte, &bit_offset);
    for (pixel_index = 0; pixel_index < width*height; ++pixel_index) {
      unsigned char new_byte = color_index_for_rgb(&rgb_image[pixel_index*format], color_table);
      int symbol_table_index = find_symbol_table_index(symbol_table, symbol, new_byte, free_symbol);
      if (symbol_table_index != SYMBOL_NONE) {
        symbol = symbol_table_index;
      } else {
        if (free_symbol < 4096){
          symbol_table[free_symbol].prefix_symbol = symbol;
          symbol_table[free_symbol].new_byte = new_byte;
          if (free_symbol > (1<<max_used_symbol_length)) {
            max_used_symbol_length += 1;
          }
          free_symbol++;
        }
        if (symbol != SYMBOL_NONE) {
          write_lzw_symbol(symbol, max_used_symbol_length, gwp->fp, &size_position, &bytes_written, &buffered_byte, &bit_offset);
        }
        symbol = new_byte;
      }
    }
    if (symbol != SYMBOL_NONE) {
      write_lzw_symbol(symbol, max_used_symbol_length, gwp->fp, &size_position, &bytes_written, &buffered_byte, &bit_offset);
    }
    write_lzw_symbol(SYMBOL_STOP, max_used_symbol_length, gwp->fp, &size_position, &bytes_written, &buffered_byte, &bit_offset);
    flush_lzw(gwp->fp, &size_position, &bytes_written, &buffered_byte, &bit_offset);
  }
}

void gif_close(gif_writer *gwp) {
  /* End of GIF file */
  fputc(0x3b, gwp->fp);
  fclose(gwp->fp);
  gwp->fp = NULL;
  gwp->width = -1;
  gwp->height = -1;
}
