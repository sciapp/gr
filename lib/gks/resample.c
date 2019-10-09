#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "gkscore.h"
#include "gks.h"

#ifndef INFINITY
#define INFINITY (1.0 / 0.0)
#endif
#ifndef M_PI
#define M_PI (3.141592653589793)
#endif

static double lanczos(double x, int a)
{
  if (x == 0.0)
    {
      return 1.0;
    }
  else if (-a < x && x < a)
    {
      return (a * sin(M_PI * x) * sin(M_PI * x / a)) / (x * x * M_PI * M_PI);
    }
  return 0.0;
}

static double integrate_box(double left, double right, int width)
{
  if (left > width / 2.0)
    {
      return 0.0;
    }
  if (right < -width / 2.0)
    {
      return 0.0;
    }
  if (left < -width / 2.0)
    {
      left = -width / 2.0;
    }
  if (right > width / 2.0)
    {
      right = width / 2.0;
    }
  return (right - left) / width;
}

static void upsample_horizontal_rgba(const unsigned char *source_img, double *result_img, size_t width, size_t height,
                                     size_t new_w, size_t stride, int w, int swapx, int swapy)
{
  int i, j, h, l, jx, iy;
  /* array to store filter values once to increase efficiency */
  double *horizontal_values = (double *)gks_malloc(sizeof(double) * new_w * 2 * w);

  /* precompute horizontal linear filter values*/
  for (jx = 0; jx < new_w; jx++)
    {
      j = jx;
      if (swapx)
        {
          j = (int)new_w - 1 - jx;
        }
      double sum = 0.0;
      double destination_position = (double)j / (new_w - 1) * width - 0.5;
      int source_position_offset = (int)floor(destination_position - (w - 1));
      for (h = 0; h < 2 * w; h++)
        {
          int source_position = h + source_position_offset;
          if (source_position < 0)
            {
              continue;
            }
          if (source_position > width - 1)
            {
              break;
            }
          double linear_factor = integrate_box(source_position - destination_position - 0.5,
                                               source_position - destination_position + 0.5, w);
          sum += linear_factor;
          horizontal_values[j * 2 * w + h] = linear_factor;
        }
      for (i = 0; i < 2 * w; i++)
        {
          horizontal_values[(int)(j * 2 * w + i)] /= sum;
        }
    }

  /* resample width */
  for (iy = 0; iy < height; iy++)
    {
      i = iy;
      if (swapy)
        {
          i = (int)height - 1 - iy;
        }
      for (jx = 0; jx < new_w; jx++)
        {
          j = jx;
          if (swapx)
            {
              j = (int)new_w - 1 - jx;
            }
          double destination_position = (double)j / (new_w - 1) * width - 0.5;
          int source_position_offset = (int)floor(destination_position - (w - 1));
          for (h = 0; h < 2 * w; h++)
            {
              int source_position = source_position_offset + h;
              if (source_position < 0)
                {
                  continue;
                }
              if (source_position > width - 1)
                {
                  break;
                }
              double linear_factor = horizontal_values[j * 2 * w + h];
              for (l = 0; l < 4; l++)
                {
                  /* calculate the influence of the image points */
                  result_img[(i * new_w + j) * 4 + l] +=
                      source_img[(i * stride + source_position) * 4 + l] * linear_factor;
                }
            }
        }
    }
  gks_free(horizontal_values);
}

static void downsample_horizontal_rgba(const unsigned char *source_img, double *result_img, size_t width, size_t height,
                                       size_t new_w, size_t stride, int w, int swapx, int swapy)
{
  int i, j, h, l, iy, jx;
  /* array to store filter values once to increase efficiency */
  int num_steps = (int)ceil((double)width / new_w * w) * 2;
  double *horizontal_values = (double *)gks_malloc(sizeof(double) * new_w * num_steps);

  /* precompute horizontal linear filter values*/
  for (jx = 0; jx < new_w; jx++)
    {
      j = jx;
      if (swapx)
        {
          j = (int)new_w - 1 - jx;
        }
      double sum = 0;
      int source_index_offset =
          (int)ceil((double)j / (double)(new_w - 1) * (double)width - 0.5 - (double)width / new_w * w);
      for (h = 0; h < num_steps; h++)
        {
          int source_index = h + source_index_offset;
          double source_position = (source_index_offset + h + 0.5) / (double)width * (double)(new_w - 1);
          if (source_index < 0)
            {
              continue;
            }
          if (source_index > width - 1)
            {
              break;
            }
          double linear_factor = integrate_box(source_position - j - 0.5, source_position - j + 0.5, w);
          sum += linear_factor;
          horizontal_values[j * num_steps + h] = linear_factor;
        }
      for (i = 0; i < num_steps; i++)
        {
          horizontal_values[j * num_steps + i] /= sum;
        }
    }

  /* resample width */
  for (iy = 0; iy < height; iy++)
    {
      i = iy;
      if (swapy)
        {
          i = (int)height - 1 - iy;
        }
      for (jx = 0; jx < new_w; jx++)
        {
          j = jx;
          if (swapx)
            {
              j = (int)new_w - 1 - jx;
            }
          int source_index_offset =
              (int)ceil((double)j / (double)(new_w - 1) * (double)width - 0.5 - (double)width / new_w * w);
          for (h = 0; h < num_steps; h++)
            {
              int source_index = source_index_offset + h;
              if (source_index < 0)
                {
                  continue;
                }
              if (source_index > width - 1)
                {
                  break;
                }
              double linear_factor = horizontal_values[j * num_steps + h];
              for (l = 0; l < 4; l++)
                {
                  /* calculate the influence of the image points */
                  result_img[(i * new_w + j) * 4 + l] +=
                      source_img[(i * stride + source_index) * 4 + l] * linear_factor;
                }
            }
        }
    }
  gks_free(horizontal_values);
}

static void upsample_vertical_rgba(const double *source_img, unsigned char *result_img, size_t height, size_t new_w,
                                   size_t new_h, int w, int swapx, int swapy)
{
  int i, j, h, l, ix, jy;
  /* array to store filter values once to increase efficiency */
  double *vertical_values = (double *)gks_malloc(sizeof(double) * new_h * 2 * w);

  /* precompute vertical linear filter values*/
  for (jy = 0; jy < new_h; jy++)
    {
      j = jy;
      if (swapy)
        {
          j = (int)new_h - 1 - jy;
        }
      double sum = 0.0;
      double destination_position = (double)j / (new_h - 1) * height - 0.5;
      int source_position_offset = (int)floor(destination_position - (w - 1));
      for (h = 0; h < 2 * w; h++)
        {
          int source_position = h + source_position_offset;
          if (source_position < 0)
            {
              continue;
            }
          if (source_position > height - 1)
            {
              break;
            }
          double linear_factor = integrate_box(source_position - destination_position - 0.5,
                                               source_position - destination_position + 0.5, w);
          sum += linear_factor;
          vertical_values[(int)(j * 2 * w + h)] = linear_factor;
        }
      for (i = 0; i < 2 * w; i++)
        {
          vertical_values[(int)(j * 2 * w + i)] /= sum;
        }
    }

  /* resample height */
  for (ix = 0; ix < new_w; ix++)
    {
      i = ix;
      if (swapx)
        {
          i = (int)new_w - 1 - ix;
        }
      for (jy = 0; jy < new_h; jy++)
        {
          j = jy;
          if (swapy)
            {
              j = (int)new_h - 1 - jy;
            }
          double destination_position = (double)j / (new_h - 1) * height - 0.5;
          int source_position_offset = (int)floor(destination_position - (w - 1));
          double rgba[4] = {0};
          for (h = 0; h < 2 * w; h++)
            {
              int source_position = source_position_offset + h;
              if (source_position < 0)
                {
                  continue;
                }
              if (source_position > height - 1)
                {
                  break;
                }
              double linear_factor = vertical_values[(int)(j * 2 * w + h)];
              for (l = 0; l < 4; l++)
                {
                  /* calculate the influence of image points */
                  rgba[l] += source_img[(new_w * source_position + i) * 4 + l] * linear_factor;
                }
            }
          result_img[(j * new_w + i) * 4 + 0] = (unsigned char)round(rgba[0]);
          result_img[(j * new_w + i) * 4 + 1] = (unsigned char)round(rgba[1]);
          result_img[(j * new_w + i) * 4 + 2] = (unsigned char)round(rgba[2]);
          result_img[(j * new_w + i) * 4 + 3] = (unsigned char)round(rgba[3]);
        }
    }
  gks_free(vertical_values);
}

static void downsample_vertical_rgba(const double *source_img, unsigned char *result_img, size_t height, size_t new_w,
                                     size_t new_h, int w, int swapx, int swapy)
{
  int i, j, h, l, ix, jy;
  int num_steps = (int)ceil((double)height / new_h * w) * 2;
  /* array to store filter values once to increase efficiency */
  double *vertical_values = (double *)gks_malloc(sizeof(double) * new_h * num_steps);

  /* precompute vertical linear filter values*/
  for (jy = 0; jy < new_h; jy++)
    {
      j = jy;
      if (swapy)
        {
          j = (int)new_h - 1 - jy;
        }
      double sum = 0.0;
      int source_index_offset = (int)ceil(j / (double)(new_h - 1) * height - 0.5 - (double)height / new_h * w);
      for (h = 0; h < num_steps; h++)
        {
          int source_index = source_index_offset + h;
          double source_position = (source_index_offset + h + 0.5) / (double)height * (double)(new_h - 1);
          if (source_index < 0)
            {
              continue;
            }
          if (source_index > height - 1)
            {
              break;
            }
          double linear_factor = integrate_box(source_position - j - 0.5, source_position - j + 0.5, w);
          sum += linear_factor;
          vertical_values[j * num_steps + h] = linear_factor;
        }
      for (i = 0; i < num_steps; i++)
        {
          vertical_values[j * num_steps + i] /= sum;
        }
    }

  /* resample height */
  for (ix = 0; ix < new_w; ix++)
    {
      i = ix;
      if (swapx)
        {
          i = (int)new_w - 1 - ix;
        }
      for (jy = 0; jy < new_h; jy++)
        {
          j = jy;
          if (swapy)
            {
              j = (int)new_h - 1 - jy;
            }
          int source_index_offset =
              (int)ceil(j / (double)(new_h - 1) * (double)height - 0.5 - (double)height / new_h * w);
          double rgba[4] = {0};
          for (h = 0; h < num_steps; h++)
            {
              int source_index = h + source_index_offset;
              if (source_index < 0)
                {
                  continue;
                }
              if (source_index > height - 1)
                {
                  break;
                }
              double linear_factor = vertical_values[j * num_steps + h];
              for (l = 0; l < 4; l++)
                {
                  /* calculate the influence of image points */
                  rgba[l] += source_img[(new_w * source_index + i) * 4 + l] * linear_factor;
                }
            }

          for (l = 0; l < 4; l++)
            {
              result_img[(j * new_w + i) * 4 + l] = (unsigned char)round(rgba[l]);
            }
        }
    }
  gks_free(vertical_values);
}

/*!
 * Use linear resampling to rescale the data image.
 *
 * \param[in] source_img Data array of the original image
 * \param[out] result_img Data array with resampled values
 * \param[in] width Size of a row in source_img
 * \param[in] height Number of rows in source_img
 * \param[in] new_w Size of a row in result_img
 * \param[in] new_h Number of rows in result_img
 * \param[in] stride Stride of source_img
 * \param[in] w Width of linear filter
 *
 */
static void resample_rgba(const unsigned char *source_img, unsigned char *result_img, size_t width, size_t height,
                          size_t new_w, size_t new_h, size_t stride, int w, int swapx, int swapy)
{
  if (w <= 0)
    {
      gks_fatal_error("w greater than 0 required!\n");
    }
  double *one_dir_img = (double *)calloc(4 * height * new_w, sizeof(double));
  if (new_w > width)
    {
      upsample_horizontal_rgba(source_img, one_dir_img, width, height, new_w, stride, w, swapx, swapy);
    }
  else
    {
      downsample_horizontal_rgba(source_img, one_dir_img, width, height, new_w, stride, w, swapx, swapy);
    }
  if (new_h > height)
    {
      upsample_vertical_rgba(one_dir_img, result_img, height, new_w, new_h, w, swapx, swapy);
    }
  else
    {
      downsample_vertical_rgba(one_dir_img, result_img, height, new_w, new_h, w, swapx, swapy);
    }
}

/*!
 * Use lanczos resampling to rescale the data image.
 *
 * \param[in] source_img Data array of the original image
 * \param[out] result_img Data array with resampled values
 * \param[in] width Size of a row in source_img
 * \param[in] height Number of rows in source_img
 * \param[in] new_w Size of a row in result_img
 * \param[in] new_h Number of rows in result_img
 * \param[in] stride Stride of source_img
 * \param[in] a Half width of lanczos filter
 * \param[in] min_val Lower border for colour values
 * \param[in] max_val Upperborder for colour values
 * \param[in] swapx True if x coordinates are swapped in source_img
 * \param[in] swapy True if y coordinates are swapped in source_img
 *
 * Best results will be generated when a = 2 or a = 3.
 */
static void upsample_horizonal_rgba_lanczos(const unsigned char *source_img, double *result_img, size_t width,
                                            size_t height, size_t new_w, size_t stride, int a, int min_val, int max_val,
                                            int swapx, int swapy)
{
  size_t j, h;
  size_t jx, hy;
  int i, l;
  double *horizontal_values = (double *)gks_malloc(sizeof(double) * new_w * a * 2);

  /* precompute horizontal lanczos filter values*/
  for (j = 0; j < new_w; j++)
    {
      jx = j;
      if (swapx)
        {
          jx = (int)new_w - 1 - j;
        }
      double destination_position = jx / (double)(new_w - 1) * (double)width - 0.5;
      double sum = 0;
      for (i = 0; i < 2 * a; i++)
        {
          int source_position = (int)floor(destination_position - (a - 1)) + i;
          if (source_position < 0)
            {
              continue;
            }
          if (source_position > width - 1)
            {
              break;
            }
          double lanczos_factor = lanczos(source_position - destination_position, a);
          sum += lanczos_factor;
          horizontal_values[jx * 2 * a + i] = lanczos_factor;
        }
      for (i = 0; i < 2 * a; i++)
        {
          horizontal_values[jx * 2 * a + i] /= sum;
        }
    }

  /* resample width */
  for (h = 0; h < height; h++)
    {
      hy = h;
      if (swapy)
        {
          hy = (int)height - 1 - h;
        }
      for (j = 0; j < new_w; j++)
        {
          jx = j;
          if (swapx)
            {
              jx = (int)new_w - 1 - j;
            }
          /* linspace between -0.5 and width-0.5 */
          double destination_position = jx / (double)(new_w - 1) * (double)width - 0.5;
          int source_position_offset = (int)floor(destination_position - (a - 1));
          double result[4] = {0};
          /* filter values are not zero */
          for (i = 0; i < 2 * a; i++)
            {
              int source_position = i + source_position_offset;
              if (source_position < 0)
                {
                  continue;
                }
              if (source_position > width - 1)
                {
                  break;
                }
              double lanczos_factor = horizontal_values[jx * 2 * a + i];
              for (l = 0; l < 4; l++)
                {
                  result[l] += lanczos_factor * source_img[(hy * stride + source_position) * 4 + l];
                }
            }
          for (l = 0; l < 4; l++)
            {
              result_img[(hy * new_w + jx) * 4 + l] = result[l];
            }
        }
    }
  gks_free(horizontal_values);
}

static void downsample_horizonal_rgba_lanczos(const unsigned char *source_img, double *result_img, size_t width,
                                              size_t height, size_t new_w, size_t stride, int a, int min_val,
                                              int max_val, int swapx, int swapy)
{
  size_t j, h;
  size_t jx, hy;
  int i, l;
  int num_steps = (int)ceil((double)width / new_w * a) * 2;
  double *horizontal_values = (double *)gks_malloc(sizeof(double) * new_w * num_steps);

  /* precompute horizontal lanczos filter values*/
  for (j = 0; j < new_w; j++)
    {
      jx = j;
      if (swapx)
        {
          jx = (int)new_w - 1 - j;
        }
      double sum = 0;
      int source_index_offset = (int)ceil(jx / (double)(new_w - 1) * (double)width - 0.5 - (double)width / new_w * a);
      for (i = 0; i < num_steps; i++)
        {
          int source_index = source_index_offset + i;
          double source_position = (source_index_offset + i + 0.5) / (double)width * (double)(new_w - 1);
          if (source_index < 0)
            {
              continue;
            }
          if (source_index > width - 1)
            {
              break;
            }
          double lanczos_factor = lanczos(source_position - jx, a);
          sum += lanczos_factor;
          horizontal_values[jx * num_steps + i] = lanczos_factor;
        }
      for (i = 0; i < num_steps; i++)
        {
          horizontal_values[jx * num_steps + i] /= sum;
        }
    }

  /* resample width */
  for (h = 0; h < height; h++)
    {
      hy = h;
      if (swapy)
        {
          hy = (int)height - 1 - h;
        }
      for (j = 0; j < new_w; j++)
        {
          jx = j;
          if (swapx)
            {
              jx = (int)new_w - 1 - j;
            }
          int source_index_offset =
              (int)ceil(jx / (double)(new_w - 1) * (double)width - 0.5 - (double)width / new_w * a);
          double result[4] = {0};
          for (i = 0; i < num_steps; i++)
            {
              int source_index = source_index_offset + i;
              if (source_index < 0)
                {
                  continue;
                }
              if (source_index > width - 1)
                {
                  break;
                }
              double lanczos_factor = horizontal_values[jx * num_steps + i];
              for (l = 0; l < 4; l++)
                {
                  result[l] += lanczos_factor * source_img[(hy * stride + source_index) * 4 + l];
                }
            }
          for (l = 0; l < 4; l++)
            {
              result_img[(hy * new_w + jx) * 4 + l] = result[l];
            }
        }
    }
  gks_free(horizontal_values);
}

static void upsample_vertical_rgba_lanczos(const double *source_img, unsigned char *result_img, size_t width,
                                           size_t height, size_t new_h, size_t stride, int a, int min_val, int max_val,
                                           int swapx, int swapy)
{
  size_t j, h;
  size_t jx, hy;
  int i, l;
  double *vertical_values = (double *)gks_malloc(sizeof(double) * new_h * a * 2);

  /* precompute vertical lanczos filter values */
  for (h = 0; h < new_h; h++)
    {
      hy = h;
      if (swapy)
        {
          hy = (int)new_h - 1 - h;
        }
      double destination_position = hy / (double)(new_h - 1) * (double)height - 0.5;
      double sum = 0;
      for (i = 0; i < 2 * a; i++)
        {
          int source_position = (int)floor(destination_position - (a - 1)) + i;
          if (source_position < 0)
            {
              continue;
            }
          if (source_position > height - 1)
            {
              break;
            }
          double lanczos_factor = lanczos(source_position - destination_position, a);
          sum += lanczos_factor;
          vertical_values[hy * 2 * a + i] = lanczos_factor;
        }
      for (i = 0; i < 2 * a; i++)
        {
          vertical_values[hy * 2 * a + i] /= sum;
        }
    }

  /* resample height*/
  for (j = 0; j < width; j++)
    {
      jx = j;
      if (swapx)
        {
          jx = (int)width - 1 - j;
        }
      for (h = 0; h < new_h; h++)
        {
          hy = h;
          if (swapy)
            {
              hy = (int)new_h - 1 - h;
            }
          double destination_position = hy / (double)(new_h - 1) * (double)height - 0.5;
          int source_position_offset = (int)floor(destination_position - (a - 1));
          double result[4] = {0};
          /* where filter is not null */
          for (i = 0; i < 2 * a; i++)
            {
              int source_position = i + source_position_offset;
              if (source_position < 0)
                {
                  continue;
                }
              if (source_position > height - 1)
                {
                  break;
                }
              double lanczos_factor = vertical_values[hy * 2 * a + i];
              for (l = 0; l < 4; l++)
                {
                  result[l] += lanczos_factor * source_img[(source_position * width + jx) * 4 + l];
                }
            }
          for (l = 0; l < 4; l++)
            {
              /* clipping */
              if (result[l] > max_val)
                {
                  result[l] = max_val;
                }
              else if (result[l] < min_val)
                {
                  result[l] = min_val;
                }
              result_img[(h * width + j) * 4 + l] = (unsigned char)round(result[l]);
            }
        }
    }
  gks_free(vertical_values);
}

static void downsample_vertical_rgba_lanczos(const double *source_img, unsigned char *result_img, size_t width,
                                             size_t height, size_t new_h, size_t stride, int a, int min_val,
                                             int max_val, int swapx, int swapy)
{
  size_t j, h;
  size_t jx, hy;
  int i, l;
  int num_steps = (int)ceil((double)height / new_h * a) * 2;
  double *vertical_values = (double *)gks_malloc(sizeof(double) * new_h * num_steps);

  /* precompute vertical lanczos filter values */
  for (h = 0; h < new_h; h++)
    {
      hy = h;
      if (swapy)
        {
          hy = (int)new_h - 1 - h;
        }
      double sum = 0;
      int source_index_offset = (int)ceil(hy / (double)(new_h - 1) * (height)-0.5 - (double)height / new_h * a);
      for (i = 0; i < num_steps; i++)
        {
          int source_index = source_index_offset + i;
          double source_position = (source_index_offset + i + 0.5) / (double)height * (double)(new_h - 1);
          if (source_index < 0)
            {
              continue;
            }
          if (source_index > height - 1)
            {
              break;
            }
          double lanczos_factor = lanczos(source_position - hy, a);
          sum += lanczos_factor;
          vertical_values[hy * num_steps + i] = lanczos_factor;
        }
      for (i = 0; i < num_steps; i++)
        {
          vertical_values[hy * num_steps + i] /= sum;
        }
    }

  /* resample height*/
  for (j = 0; j < width; j++)
    {
      jx = j;
      if (swapx)
        {
          jx = (int)width - 1 - j;
        }
      for (h = 0; h < new_h; h++)
        {
          hy = h;
          if (swapy)
            {
              hy = (int)new_h - 1 - h;
            }
          int source_index_offset =
              (int)ceil(hy / (double)(new_h - 1) * (double)height - 0.5 - (double)height / new_h * a);
          double result[4] = {0};
          /* where filter is not null */
          for (i = 0; i < num_steps; i++)
            {
              int source_index = i + source_index_offset;
              if (source_index < 0)
                {
                  continue;
                }
              if (source_index > height - 1)
                {
                  break;
                }
              double lanczos_factor = vertical_values[hy * num_steps + i];
              for (l = 0; l < 4; l++)
                {
                  result[l] += lanczos_factor * source_img[(source_index * width + jx) * 4 + l];
                }
            }
          for (l = 0; l < 4; l++)
            {
              /* clipping */
              if (result[l] > max_val)
                {
                  result[l] = max_val;
                }
              else if (result[l] < min_val)
                {
                  result[l] = min_val;
                }
              result_img[(h * width + j) * 4 + l] = (unsigned char)round(result[l]);
            }
        }
    }
  gks_free(vertical_values);
}

/*!
 * Use lanczos resampling to rescale the data image.
 *
 * \param[in] source_img Data array of the original image
 * \param[out] result_img Data array with resampled values
 * \param[in] width Size of a row in source_img
 * \param[in] height Number of rows in source_img
 * \param[in] new_w Size of a row in result_img
 * \param[in] new_h Number of rows in result_img
 * \param[in] stride Stride of source_img
 * \param[in] a Half width of lanczos filter
 * \param[in] min_val Lower border for colour values
 * \param[in] max_val Upperborder for colour values
 * \param[in] swapx True if x coordinates are swapped in source_img
 * \param[in] swapy True if y coordinates are swapped in source_img
 *
 * Best results will be generated when a = 2 or a = 3.
 */
static void resample_rgba_lanczos(const unsigned char *source_img, unsigned char *result_img, size_t width,
                                  size_t height, size_t new_w, size_t new_h, size_t stride, int a, int min_val,
                                  int max_val, int swapx, int swapy)
{
  if (a <= 0)
    {
      gks_fatal_error("a greater than 0 required!\n");
    }
  double *one_dir_img = (double *)gks_malloc(4 * (int)height * (int)new_w * sizeof(double));
  if (new_w > width)
    {
      upsample_horizonal_rgba_lanczos(source_img, one_dir_img, width, height, new_w, stride, a, min_val, max_val, swapx,
                                      swapy);
    }
  else
    {
      downsample_horizonal_rgba_lanczos(source_img, one_dir_img, width, height, new_w, stride, a, min_val, max_val,
                                        swapx, swapy);
    }

  if (new_h > height)
    {
      upsample_vertical_rgba_lanczos(one_dir_img, result_img, new_w, height, new_h, stride, a, min_val, max_val, swapx,
                                     swapy);
    }
  else
    {
      downsample_vertical_rgba_lanczos(one_dir_img, result_img, new_w, height, new_h, stride, a, min_val, max_val,
                                       swapx, swapy);
    }
  gks_free(one_dir_img);
}

static void resample_rgba_nearest(const unsigned char *source_img, unsigned char *result_img, size_t width,
                                  size_t height, size_t new_w, size_t new_h, size_t stride, int swapx, int swapy)
{
  int i, j, ix, iy, rgb;
  for (j = 0; j < new_h; j++)
    {
      iy = (int)height * j / (int)new_h;
      if (swapy)
        {
          iy = (int)height - 1 - iy;
        }
      for (i = 0; i < new_w; i++)
        {
          ix = (int)width * i / (int)new_w;
          if (swapx)
            {
              ix = (int)width - 1 - ix;
            }

          result_img[(j * new_w + i) * 4 + 0] = source_img[(iy * stride + ix) * 4 + 0];
          result_img[(j * new_w + i) * 4 + 1] = source_img[(iy * stride + ix) * 4 + 1];
          result_img[(j * new_w + i) * 4 + 2] = source_img[(iy * stride + ix) * 4 + 2];
          result_img[(j * new_w + i) * 4 + 3] = source_img[(iy * stride + ix) * 4 + 3];
        }
    }
}

/*!
 * Method switches between different resample filters depending on the resample flag status.
 *
 * \param[in] source_img Data array of the original image
 * \param[out] result_img Data array with resampled values
 * \param[in] width Size of a row in source_img
 * \param[in] height Number of rows in source_img
 * \param[in] new_w Size of a row in result_img
 * \param[in] new_h Number of rows in result_img
 * \param[in] stride Stride of source_img
 * \param[in] swapx True if x coordinates are swapped in source_img
 * \param[in] swapy True if y coordinates are swapped in source_img
 *
 * Calculate new image data with linear or lanczos interpolation to receive smoother results.
 * This way the source_img can be up or downscaled. It is an alternative for the nearest neighbour resampling.
 */
void gks_resample(const unsigned char *source_img, unsigned char *result_img, size_t width, size_t height, size_t new_w,
                  size_t new_h, size_t stride, int swapx, int swapy, int resample_method)
{
  if (resample_method == GKS_K_RESAMPLE_LINEAR)
    {
      resample_rgba(source_img, result_img, width, height, new_w, new_h, stride, 1, swapx, swapy);
    }
  else if (resample_method == GKS_K_RESAMPLE_LANCZOS)
    {
      resample_rgba_lanczos(source_img, result_img, width, height, new_w, new_h, stride, 3, 0, 255, swapx, swapy);
    }
  else if (resample_method == GKS_K_RESAMPLE_NEAREST)
    {
      resample_rgba_nearest(source_img, result_img, width, height, new_w, new_h, stride, swapx, swapy);
    }
  else
    {
      gks_fatal_error(
          "Only GKS_K_RESAMPLE_LINEAR, GKS_K_RESAMPLE_NEAREST and GKS_K_RESAMPLE_LANCZOS are valid for resample\n");
    }
}
