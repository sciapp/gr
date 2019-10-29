#ifndef _GIF_H_
#define _GIF_H_

#ifdef __cplusplus
extern "C"
{
#endif

  void median_cut(unsigned char *pixels, unsigned char *color_table, int num_pixels, int num_colors, int num_channels);
  unsigned char color_index_for_rgb(const unsigned char *rgb_pixel, const unsigned char *color_table,
                                    int color_table_size, int num_channels);

#ifdef __cplusplus
}
#endif

#endif
