#include <stdlib.h>
#include "gif.h"

#define distance_squared(a, b)                                                     \
  (((a)[2] - (b)[0]) * ((a)[2] - (b)[0]) + ((a)[1] - (b)[1]) * ((a)[1] - (b)[1]) + \
   ((a)[0] - (b)[2]) * ((a)[0] - (b)[2]))

static int compare_color_component(const void *left, const void *right)
{
  int l = ((const unsigned char *)left)[0];
  int r = ((const unsigned char *)right)[0];
  return l - r;
}

void median_cut(unsigned char *pixels, unsigned char *color_table, int num_pixels, int num_colors, int num_channels)
{
  /* Perform a color quantization similar to the modified median cut quantization algorithm */
  if (num_pixels <= 0)
    {
      int i;
      for (i = 0; i < num_colors; i++)
        {
          color_table[4 * i + 0] = 0;
          color_table[4 * i + 1] = 0;
          color_table[4 * i + 2] = 0;
          color_table[4 * i + 3] = 0;
        }
      return;
    }
  if (num_colors == 1)
    {
      color_table[0] = pixels[num_pixels / 2 * num_channels + 2]; /* B */
      color_table[1] = pixels[num_pixels / 2 * num_channels + 1]; /* G */
      color_table[2] = pixels[num_pixels / 2 * num_channels + 0]; /* R */
      color_table[3] = pixels[num_pixels / 2 * num_channels + 3]; /* A */
    }
  else
    {
      int i;
      int cut_axis;
      unsigned char cut_value;
      int left_pixels;
      unsigned char minred, mingreen, minblue, maxred, maxgreen, maxblue;
      minred = maxred = pixels[0];
      mingreen = maxgreen = pixels[1];
      minblue = maxblue = pixels[2];
      for (i = 1; i < num_pixels; i++)
        {
          if (pixels[i * num_channels + 0] < minred) minred = pixels[i * num_channels + 0];
          if (pixels[i * num_channels + 0] > maxred) maxred = pixels[i * num_channels + 0];
          if (pixels[i * num_channels + 1] < mingreen) mingreen = pixels[i * num_channels + 1];
          if (pixels[i * num_channels + 1] > maxgreen) maxgreen = pixels[i * num_channels + 1];
          if (pixels[i * num_channels + 2] < minblue) minblue = pixels[i * num_channels + 2];
          if (pixels[i * num_channels + 2] > maxblue) maxblue = pixels[i * num_channels + 2];
        }
      if (maxred - minred < maxgreen - mingreen)
        {
          if (maxgreen - mingreen < maxblue - minblue)
            {
              cut_axis = 2;
              cut_value = (minblue + maxblue) / 2;
            }
          else
            {
              cut_axis = 1;
              cut_value = (mingreen + maxgreen) / 2;
            }
        }
      else
        {
          if (maxred - minred < maxblue - minblue)
            {
              cut_axis = 2;
              cut_value = (minblue + maxblue) / 2;
            }
          else
            {
              cut_axis = 0;
              cut_value = (minred + maxred) / 2;
            }
        }
      qsort(pixels + cut_axis, num_pixels, num_channels, compare_color_component);
      for (left_pixels = num_colors / 2;
           (left_pixels < num_pixels - num_colors / 2) && pixels[left_pixels * num_channels + cut_axis] < cut_value;
           left_pixels++)
        ;
      median_cut(pixels, color_table, left_pixels, num_colors / 2, num_channels);
      median_cut(pixels + left_pixels * num_channels, color_table + num_colors / 2 * num_channels,
                 num_pixels - left_pixels, num_colors / 2, num_channels);
    }
}

unsigned char color_index_for_rgb(const unsigned char *rgb_pixel, const unsigned char *color_table,
                                  int color_table_size, int num_channels)
{
  /* Return the color index most closely matching this RGB color. */
  int color_index;
  int closest_color_index = 0;
  float closest_color_index_distance = -1;
  for (color_index = 0; color_index < color_table_size; ++color_index)
    {
      float color_index_distance = distance_squared(&color_table[color_index * num_channels], rgb_pixel);
      if (closest_color_index_distance < 0 || closest_color_index_distance > color_index_distance)
        {
          closest_color_index_distance = color_index_distance;
          closest_color_index = color_index;
        }
    }
  return closest_color_index;
}
