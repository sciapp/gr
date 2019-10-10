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

static double calculate_lanzcos_factor(double source_position, double target_position, int a)
{
  return lanczos(source_position - target_position, a);
}

static double calculate_linear_factor(double source_position, double target_position, int a)
{
  return integrate_box(source_position - target_position - 0.5, source_position - target_position + 0.5, a);
}

static double *calculate_resampling_factors(size_t source_size, size_t target_size, int a, int flip,
                                            double (*factor_func)(double, double, int))
{
  size_t i;
  size_t i_flipped;
  int j;
  double *factors;
  int num_steps;
  if (source_size > target_size)
    {
      num_steps = (int)ceil((double)source_size / target_size * a) * 2;
    }
  else
    {
      num_steps = a * 2;
    }
  factors = (double *)gks_malloc((int)sizeof(double) * (int)target_size * num_steps);
  for (i = 0; i < target_size; i++)
    {
      if (flip)
        {
          i_flipped = target_size - 1 - i;
        }
      else
        {
          i_flipped = i;
        }
      double sum = 0.0;
      double target_position;
      int source_index_offset;

      if (source_size > target_size)
        {
          target_position = i_flipped;
          source_index_offset = (int)ceil((double)i_flipped / (double)(target_size - 1) * (double)source_size - 0.5 -
                                          (double)source_size / target_size * a);
        }
      else
        {
          target_position = i_flipped / (double)(target_size - 1) * source_size - 0.5;
          source_index_offset = (int)floor(target_position - (a - 1));
        }
      for (j = 0; j < num_steps; j++)
        {
          int source_index = source_index_offset + j;
          if (source_index < 0)
            {
              continue;
            }
          if (source_index > source_size - 1)
            {
              break;
            }

          double source_position;
          if (source_size > target_size)
            {
              source_position = (source_index_offset + j + 0.5) / (double)source_size * (double)(target_size - 1);
            }
          else
            {
              source_position = source_index;
            }
          double factor = factor_func(source_position, target_position, a);
          sum += factor;
          factors[i * num_steps + j] = factor;
        }
      for (j = 0; j < num_steps; j++)
        {
          factors[i * num_steps + j] /= sum;
        }
    }
  return factors;
}

static void resample_horizontal_rgba(const unsigned char *source_image, double *target_image, size_t source_width,
                                     size_t source_height, size_t target_width, size_t stride, int a, int flip,
                                     double (*factor_func)(double, double, int))
{
  size_t ix;
  size_t ix_flipped;
  size_t iy;
  int i, j;
  int num_steps;
  double *factors;
  if (source_width > target_width)
    {
      num_steps = (int)ceil((double)source_width / target_width * a) * 2;
    }
  else
    {
      num_steps = a * 2;
    }
  factors = calculate_resampling_factors(source_width, target_width, a, flip, factor_func);

  for (iy = 0; iy < source_height; iy++)
    {
      for (ix = 0; ix < target_width; ix++)
        {
          int source_index_offset;

          if (flip)
            {
              ix_flipped = (int)target_width - 1 - ix;
            }
          else
            {
              ix_flipped = ix;
            }

          if (source_width > target_width)
            {
              source_index_offset = (int)ceil((double)ix_flipped / (double)(target_width - 1) * (double)source_width -
                                              0.5 - (double)source_width / (double)target_width * a);
            }
          else
            {
              source_index_offset =
                  (int)floor((double)ix_flipped / (double)(target_width - 1) * (double)source_width + 0.5 - a);
            }
          for (i = 0; i < num_steps; i++)
            {
              int source_index = source_index_offset + i;
              if (source_index < 0)
                {
                  continue;
                }
              if (source_index > source_width - 1)
                {
                  break;
                }
              double factor = factors[ix * num_steps + i];
              for (j = 0; j < 4; j++)
                {
                  target_image[(iy * target_width + ix) * 4 + j] +=
                      source_image[(iy * stride + source_index) * 4 + j] * factor;
                }
            }
        }
    }
  gks_free(factors);
}

static void resample_vertical_rgba(const double *source_image, unsigned char *target_image, size_t source_width,
                                   size_t source_height, size_t target_height, size_t stride, int a, int flip,
                                   double (*factor_func)(double, double, int))
{
  size_t ix;
  size_t iy;
  size_t iy_flipped;
  int i, j;
  int num_steps;
  double *factors;
  if (source_height > target_height)
    {
      num_steps = (int)ceil((double)source_height / target_height * a) * 2;
    }
  else
    {
      num_steps = 2 * a;
    }
  factors = calculate_resampling_factors(source_height, target_height, a, flip, factor_func);

  for (ix = 0; ix < source_width; ix++)
    {
      for (iy = 0; iy < target_height; iy++)
        {
          int source_index_offset;

          if (flip)
            {
              iy_flipped = (int)target_height - 1 - iy;
            }
          else
            {
              iy_flipped = iy;
            }

          if (source_height > target_height)
            {
              source_index_offset = (int)ceil((double)iy_flipped / (double)(target_height - 1) * (double)source_height -
                                              0.5 - (double)source_height / target_height * a);
            }
          else
            {
              source_index_offset =
                  (int)floor((double)iy_flipped / (double)(target_height - 1) * (double)source_height + 0.5 - a);
            }
          double result[4] = {0};
          for (i = 0; i < num_steps; i++)
            {
              int source_index = source_index_offset + i;
              if (source_index < 0)
                {
                  continue;
                }
              if (source_index > source_height - 1)
                {
                  break;
                }
              double factor = factors[iy * num_steps + i];
              for (j = 0; j < 4; j++)
                {
                  result[j] += source_image[(source_index * stride + ix) * 4 + j] * factor;
                }
            }
          for (j = 0; j < 4; j++)
            {
              if (result[j] > 255)
                {
                  result[j] = 255;
                }
              else if (result[j] < 0)
                {
                  result[j] = 0;
                }
              target_image[(iy * source_width + ix) * 4 + j] = (unsigned char)round(result[j]);
            }
        }
    }
  gks_free(factors);
}


static void resample_rgba(const unsigned char *source_image, unsigned char *target_image, size_t source_width,
                          size_t source_height, size_t target_width, size_t target_height, size_t stride, int a,
                          int flip_x, int flip_y, double (*factor_func)(double, double, int))
{
  if (a <= 0)
    {
      gks_fatal_error("a greater than 0 required!\n");
    }
  double *temp_image = (double *)gks_malloc((int)sizeof(double) * 4 * (int)target_width * (int)source_height);
  resample_horizontal_rgba(source_image, temp_image, source_width, source_height, target_width, stride, a, flip_x,
                           factor_func);
  resample_vertical_rgba(temp_image, target_image, target_width, source_height, target_height, target_width, a, flip_y,
                         factor_func);
  gks_free(temp_image);
}


static void resample_rgba_nearest(const unsigned char *source_image, unsigned char *target_image, size_t source_width,
                                  size_t source_height, size_t target_width, size_t target_height, size_t stride,
                                  int flip_x, int flip_y)
{
  size_t ix, iy, ix_flipped, iy_flipped;
  for (iy = 0; iy < target_height; iy++)
    {
      iy_flipped = source_height * iy / target_height;
      if (flip_y)
        {
          iy_flipped = source_height - 1 - iy_flipped;
        }
      for (ix = 0; ix < target_width; ix++)
        {
          ix_flipped = source_width * ix / target_width;
          if (flip_x)
            {
              ix_flipped = source_width - 1 - ix_flipped;
            }

          target_image[(iy * target_width + ix) * 4 + 0] = source_image[(iy_flipped * stride + ix_flipped) * 4 + 0];
          target_image[(iy * target_width + ix) * 4 + 1] = source_image[(iy_flipped * stride + ix_flipped) * 4 + 1];
          target_image[(iy * target_width + ix) * 4 + 2] = source_image[(iy_flipped * stride + ix_flipped) * 4 + 2];
          target_image[(iy * target_width + ix) * 4 + 3] = source_image[(iy_flipped * stride + ix_flipped) * 4 + 3];
        }
    }
}

/*!
 * Resample an RGBA image using one of several resampling methods.
 *
 * \param[in] source_image RGBA-data of the original image
 * \param[out] target_image RGBA-data for resampled values
 * \param[in] source_width number of columns in source_image
 * \param[in] source_height number of rows in source_image
 * \param[in] target_width number of columns in target_image
 * \param[in] target_height number of rows in target_image
 * \param[in] stride stride of source_image
 * \param[in] flip_x whether the image should also be flipped horizontally
 * \param[in] flip_y whether the image should also be flipped vertically
 *
 *  The available resampling methods are:
 *
 * \verbatim embed:rst:leading-asterisk
 *
 * +------------------------+---+--------------------+
 * |GKS_K_RESAMPLE_NEAREST  |  0|nearest neighbour   |
 * +------------------------+---+--------------------+
 * |GKS_K_RESAMPLE_LINEAR   |  1|linear              |
 * +------------------------+---+--------------------+
 * |GKS_K_RESAMPLE_LANCZOS  |  2|lanczos             |
 * +------------------------+---+--------------------+
 * |GKS_K_RESAMPLE_DEFAULT  |  0|nearest neighbour   |
 * +------------------------+---+--------------------+
 *
 * \endverbatim
 */
void gks_resample(const unsigned char *source_image, unsigned char *target_image, size_t source_width,
                  size_t source_height, size_t target_width, size_t target_height, size_t stride, int flip_x,
                  int flip_y, int resample_method)
{
  switch (resample_method)
    {
    case GKS_K_RESAMPLE_NEAREST:
      resample_rgba_nearest(source_image, target_image, source_width, source_height, target_width, target_height,
                            stride, flip_x, flip_y);
      return;
    case GKS_K_RESAMPLE_LINEAR:
      resample_rgba(source_image, target_image, source_width, source_height, target_width, target_height, stride, 1,
                    flip_x, flip_y, calculate_linear_factor);
      return;
    case GKS_K_RESAMPLE_LANCZOS:
      resample_rgba(source_image, target_image, source_width, source_height, target_width, target_height, stride, 3,
                    flip_x, flip_y, calculate_lanzcos_factor);
      return;
    default:
      gks_fatal_error("Only GKS_K_RESAMPLE_DEFAULT, GKS_K_RESAMPLE_NEAREST, GKS_K_RESAMPLE_LINEAR and "
                      "GKS_K_RESAMPLE_LANCZOS are valid for resample\n");
      return;
    }
}
