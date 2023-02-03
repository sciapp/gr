#include <math.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>

#include "grm.h"

#define RAND_SEED 1234

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef array_size
#define array_size(a) ((sizeof(a) / sizeof((a)[0])))
#endif

/* Redefine `rand` as our custom `rnd` to be compatible with Julia */
#define rand(buf, num_elements) rnd(buf, num_elements)

#define goto_if(condition, goto_label) \
  do                                   \
    {                                  \
      if (condition)                   \
        {                              \
          goto goto_label;             \
        }                              \
    }                                  \
  while (0)
#define cleanup_if(condition) goto_if((condition), cleanup)
#define error_cleanup_if(condition) goto_if((condition), error_cleanup)

static int rand_initialized = 0;

static void init_rand(void)
{
  srand(RAND_SEED);
  rand_initialized = 1;
}

static double *rnd(double *buf, unsigned int num_elements)
{
  unsigned int i;

  if (!rand_initialized)
    {
      init_rand();
    }
  if (buf == NULL)
    {
      buf = malloc(num_elements * sizeof(double));
      if (buf == NULL)
        {
          return NULL;
        }
    }

  for (i = 0; i < num_elements; ++i)
    {
      buf[i] = (double)(rand)() / ((unsigned int)RAND_MAX + 1);
    }

  return buf;
}

static double *randn(double *buf, unsigned int num_elements)
{
  unsigned int i;

  if (!rand_initialized)
    {
      init_rand();
    }
  if (buf == NULL)
    {
      buf = malloc(num_elements * sizeof(double));
      if (buf == NULL)
        {
          return NULL;
        }
    }

  /* Use the Box-Muller transform to generate normally distributed random numbers from uniformly distributed numbers
   * See <https://en.wikipedia.org/wiki/Box%E2%80%93Muller_transform#Implementation> as a reference
   */
  for (i = 0; i < num_elements; i += 2)
    {
      double u1, u2, mag;

      /* u1 should be larger than the machine epsilon to avoid numerical problems with the later `log` transform */
      do
        {
          u1 = (double)(rand)() / ((unsigned int)RAND_MAX + 1);
        }
      while (u1 <= DBL_EPSILON);
      u2 = (double)(rand)() / ((unsigned int)RAND_MAX + 1);

      mag = sqrt(-2.0 * log(u1));
      buf[i] = mag * cos(2 * M_PI * u2);
      if (i < num_elements - 1)
        {
          buf[i + 1] = mag * sin(2 * M_PI * u2);
        }
    }

  return buf;
}

static double *lin_range(double *buf, double start, double end, unsigned int num_elements, int exclude_last)
{
  unsigned int i;

  if (buf == NULL)
    {
      buf = malloc(num_elements * sizeof(double));
      if (buf == NULL)
        {
          return NULL;
        }
    }

  for (i = 0; i < num_elements; ++i)
    {
      buf[i] = start + i * (end - start) / (num_elements - (exclude_last ? 0 : 1));
    }

  return buf;
}

static double *mapx(double (*func)(double), double *dest_buf, const double *x, unsigned int x_n)
{
  unsigned int i;

  if (dest_buf == NULL)
    {
      dest_buf = malloc(x_n * sizeof(double));
      if (dest_buf == NULL)
        {
          return NULL;
        }
    }

  for (i = 0; i < x_n; ++i)
    {
      dest_buf[i] = func(x[i]);
    }

  return dest_buf;
}

static double *mapxy(double (*func)(double, double), double *dest_buf, const double *x, const double *y,
                     unsigned int x_n)
{
  unsigned int i;

  if (dest_buf == NULL)
    {
      dest_buf = malloc(x_n * sizeof(double));
      if (dest_buf == NULL)
        {
          return NULL;
        }
    }

  for (i = 0; i < x_n; ++i)
    {
      dest_buf[i] = func(x[i], y[i]);
    }

  return dest_buf;
}

static double *mapxty(double (*func)(double, double), double *dest_buf, const double *x, const double *y,
                      unsigned int x_n, unsigned int y_n)
{
  unsigned int i, j;

  if (dest_buf == NULL)
    {
      dest_buf = malloc(x_n * y_n * sizeof(double));
      if (dest_buf == NULL)
        {
          return NULL;
        }
    }

  for (j = 0; j < y_n; ++j)
    {
      for (i = 0; i < x_n; ++i)
        {
          dest_buf[j * x_n + i] = func(x[i], y[j]);
        }
    }

  return dest_buf;
}


static void test_line(void)
{
  const int n = 200;
  double *x = NULL, *y = NULL;
  grm_args_t *args = NULL;

  x = lin_range(NULL, 0.0, 2 * M_PI, n, 1);
  cleanup_if(x == NULL);
  y = mapx(sin, NULL, x, n);
  cleanup_if(y == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n, y);
  grm_args_push(args, "kind", "s", "line");

  grm_plot(args);

cleanup:
  free(x);
  free(y);
  if (args != NULL)
    {
      grm_args_delete(args);
    }
}


static double test_scatter_y(double x)
{
  return x - x * x;
}

static void test_scatter(void)
{
  const int n = 51;
  double *x = NULL, *y = NULL, *sz = NULL, *c = NULL;
  grm_args_t *args = NULL;

  x = lin_range(NULL, 0.0, 1.0, n, 0);
  cleanup_if(x == NULL);
  y = mapx(test_scatter_y, NULL, x, n);
  cleanup_if(y == NULL);
  sz = lin_range(NULL, 50.0, 300.0, n, 0);
  cleanup_if(sz == NULL);
  c = lin_range(NULL, 0.0, 255.0, n, 0);
  cleanup_if(c == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n, y);
  grm_args_push(args, "kind", "s", "scatter");

  grm_plot(args);

  grm_args_push(args, "z", "nD", n, sz);
  grm_args_push(args, "c", "nD", n, c);

  grm_plot(args);

cleanup:
  free(x);
  free(y);
  free(sz);
  free(c);
  if (args != NULL)
    {
      grm_args_delete(args);
    }
}


static void test_stem(void)
{
  const int n = 51;
  double *x = NULL, *y = NULL;
  grm_args_t *args = NULL;

  x = lin_range(NULL, 0.0, 1.0, n, 0);
  cleanup_if(x == NULL);
  y = mapx(test_scatter_y, NULL, x, n);
  cleanup_if(y == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n, y);
  grm_args_push(args, "kind", "s", "stem");

  grm_plot(args);

cleanup:
  free(x);
  free(y);
  if (args != NULL)
    {
      grm_args_delete(args);
    }
}


static void test_histogram(void)
{
  const int n = 10000;
  double *x = NULL;
  grm_args_t *args = NULL;

  x = randn(NULL, n);
  cleanup_if(x == NULL);
  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "kind", "s", "hist");

  grm_plot(args);

cleanup:
  free(x);
  if (args != NULL)
    {
      grm_args_delete(args);
    }
}


static void test_line_only_y(void)
{
  const int n = 50;
  double *y[] = {randn(NULL, n), randn(NULL, n), randn(NULL, n), randn(NULL, n)};
  grm_args_t *args = NULL, *series[] = {NULL, NULL, NULL, NULL};
  int i;

  for (i = 0; i < (int)array_size(series); ++i)
    {
      cleanup_if(y[i] == NULL);
      series[i] = grm_args_new();
      cleanup_if(series[i] == NULL);
      grm_args_push(series[i], "y", "nD", n, y[i]);
    }

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "y", "nD", n, y[0]);
  grm_args_push(args, "kind", "s", "line");

  grm_plot(args);

  grm_args_delete(args);
  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "series", "nA", array_size(series), series);
  grm_args_push(args, "kind", "s", "line");
  /* TODO: Use `hold_plots` for a real `oplot`! */
  /* grm_args_push(args, "hold_plots", "i", 1); */

  grm_plot(args);

cleanup:
  for (i = 0; i < (int)array_size(y); ++i)
    {
      free(y[i]);
    }
  if (args != NULL)
    {
      grm_args_delete(args);
    }
}


static double test_plot3d_y(double x)
{
  return cos(x) * x;
}

static double test_plot3d_z(double x)
{
  return sin(x) * x;
}

static void test_plot3d(void)
{
  const int n = 1000;
  double *x = NULL, *y = NULL, *z = NULL;
  grm_args_t *args = NULL;

  x = lin_range(NULL, 0.0, 30.0, n, 0);
  cleanup_if(x == NULL);
  y = mapx(test_plot3d_y, NULL, x, n);
  cleanup_if(y == NULL);
  z = mapx(test_plot3d_z, NULL, x, n);
  cleanup_if(z == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n, y);
  grm_args_push(args, "z", "nD", n, z);
  grm_args_push(args, "kind", "s", "plot3");

  grm_plot(args);

cleanup:
  free(x);
  free(y);
  free(z);
  if (args != NULL)
    {
      grm_args_delete(args);
    }
}

static void test_polar(void)
{
  const int n = 40;
  double *angles = NULL, *radii = NULL;
  grm_args_t *args = NULL;

  angles = lin_range(NULL, 0.0, 2 * M_PI, n, 1);
  cleanup_if(angles == NULL);
  radii = lin_range(NULL, 0.0, 2.0, n, 1);
  cleanup_if(radii == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  /* TODO: Support `angles` and `radii` names in GRM! */
  grm_args_push(args, "x", "nD", n, angles);
  grm_args_push(args, "y", "nD", n, radii);
  grm_args_push(args, "kind", "s", "polar");

  grm_plot(args);

cleanup:
  free(angles);
  free(radii);
  if (args != NULL)
    {
      grm_args_delete(args);
    }
}

static double test_scatter3_x(double x)
{
  return 2 * x - 1;
}

static double test_scatter3_c(double c)
{
  return 999 * c + 1;
}

static void test_scatter3(void)
{
  const int n = 100;
  double *x_rand = NULL, *y_rand = NULL, *z_rand = NULL, *c_rand = NULL;
  double *x = NULL, *y = NULL, *z = NULL, *c = NULL;
  grm_args_t *args = NULL;

  x_rand = rand(NULL, n);
  cleanup_if(x_rand == NULL);
  y_rand = rand(NULL, n);
  cleanup_if(y_rand == NULL);
  z_rand = rand(NULL, n);
  cleanup_if(z_rand == NULL);
  c_rand = rand(NULL, n);
  cleanup_if(c_rand == NULL);

  x = mapx(test_scatter3_x, NULL, x_rand, n);
  cleanup_if(x == NULL);
  y = mapx(test_scatter3_x, NULL, y_rand, n);
  cleanup_if(y == NULL);
  z = mapx(test_scatter3_x, NULL, z_rand, n);
  cleanup_if(z == NULL);
  c = mapx(test_scatter3_c, NULL, c_rand, n);
  cleanup_if(c == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n, y);
  grm_args_push(args, "z", "nD", n, z);
  grm_args_push(args, "kind", "s", "scatter3");

  grm_plot(args);

  grm_args_push(args, "c", "nD", n, c);

  grm_plot(args);

cleanup:
  free(x_rand);
  free(y_rand);
  free(z_rand);
  free(x);
  free(y);
  free(z);
  if (args != NULL)
    {
      grm_args_delete(args);
    }
}

static void test_hexbin(void)
{
  const int n = 100000;
  double *x = NULL, *y = NULL;
  grm_args_t *args = NULL;

  x = rand(NULL, n);
  cleanup_if(x == NULL);
  y = rand(NULL, n);
  cleanup_if(y == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n, y);
  grm_args_push(args, "kind", "s", "hexbin");

  grm_plot(args);

cleanup:
  free(x);
  free(y);
  if (args != NULL)
    {
      grm_args_delete(args);
    }
}

static double test_contour1_x(double x)
{
  return 8 * x - 4;
}

static double test_contour1_z(double x, double y)
{
  return sin(x) + cos(y);
}

static void test_contour1(void)
{
  const int n = 100;
  double *x_rand = NULL, *y_rand = NULL;
  double *x = NULL, *y = NULL, *z = NULL;
  grm_args_t *args = NULL;

  x_rand = rand(NULL, n);
  cleanup_if(x_rand == NULL);
  y_rand = rand(NULL, n);
  cleanup_if(y_rand == NULL);

  x = mapx(test_contour1_x, NULL, x_rand, n);
  cleanup_if(x == NULL);
  y = mapx(test_contour1_x, NULL, y_rand, n);
  cleanup_if(y == NULL);
  z = mapxy(test_contour1_z, NULL, x, y, n);
  cleanup_if(z == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n, y);
  grm_args_push(args, "z", "nD", n, z);
  grm_args_push(args, "kind", "s", "contour");

  grm_plot(args);

cleanup:
  free(x_rand);
  free(y_rand);
  free(x);
  free(y);
  free(z);
  if (args != NULL)
    {
      grm_args_delete(args);
    }
}

static void test_contour2(void)
{
  const int n = 40;
  double *x = NULL, *y = NULL, *z = NULL;
  grm_args_t *args = NULL;

  x = lin_range(NULL, -2, 2, n, 1);
  cleanup_if(x == NULL);
  y = lin_range(NULL, 0.0, M_PI, n / 2, 1);
  cleanup_if(y == NULL);
  z = mapxty(test_contour1_z, NULL, x, y, n, n / 2);
  cleanup_if(z == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n / 2, y);
  grm_args_push(args, "z", "nD", n * (n / 2), z);
  grm_args_push(args, "kind", "s", "contour");

  grm_plot(args);

cleanup:
  free(x);
  free(y);
  free(z);
  if (args != NULL)
    {
      grm_args_delete(args);
    }
}

static double test_contourf1_x(double x)
{
  return 8 * x - 4;
}

static double test_contourf1_z(double x, double y)
{
  return sin(x) + cos(y);
}

static void test_contourf1(void)
{
  const int n = 100;
  double *x_rand = NULL, *y_rand = NULL;
  double *x = NULL, *y = NULL, *z = NULL;
  grm_args_t *args = NULL;

  x_rand = rand(NULL, n);
  cleanup_if(x_rand == NULL);
  y_rand = rand(NULL, n);
  cleanup_if(y_rand == NULL);

  x = mapx(test_contourf1_x, NULL, x_rand, n);
  cleanup_if(x == NULL);
  y = mapx(test_contourf1_x, NULL, y_rand, n);
  cleanup_if(y == NULL);
  z = mapxy(test_contourf1_z, NULL, x, y, n);
  cleanup_if(z == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n, y);
  grm_args_push(args, "z", "nD", n, z);
  grm_args_push(args, "kind", "s", "contourf");

  grm_plot(args);

cleanup:
  free(x_rand);
  free(y_rand);
  free(x);
  free(y);
  free(z);
  if (args != NULL)
    {
      grm_args_delete(args);
    }
}

static void test_contourf2(void)
{
  const int n = 40;
  double *x = NULL, *y = NULL, *z = NULL;
  grm_args_t *args = NULL;

  x = lin_range(NULL, -2, 2, n, 1);
  cleanup_if(x == NULL);
  y = lin_range(NULL, 0.0, M_PI, n / 2, 1);
  cleanup_if(y == NULL);
  z = mapxty(test_contourf1_z, NULL, x, y, n, n / 2);
  cleanup_if(z == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n / 2, y);
  grm_args_push(args, "z", "nD", n * (n / 2), z);
  grm_args_push(args, "kind", "s", "contourf");

  grm_plot(args);

cleanup:
  free(x);
  free(y);
  free(z);
  if (args != NULL)
    {
      grm_args_delete(args);
    }
}

static double test_tricont_x(double x)
{
  return 8 * x - 4;
}

static double test_tricont_z(double x, double y)
{
  return sin(x) + cos(y);
}

static void test_tricont(void)
{
  const int n = 100;
  double *x_rand = NULL, *y_rand = NULL;
  double *x = NULL, *y = NULL, *z = NULL;
  grm_args_t *args = NULL;

  x_rand = rand(NULL, n);
  cleanup_if(x_rand == NULL);
  y_rand = rand(NULL, n);
  cleanup_if(y_rand == NULL);

  x = mapx(test_tricont_x, NULL, x_rand, n);
  cleanup_if(x == NULL);
  y = mapx(test_tricont_x, NULL, y_rand, n);
  cleanup_if(y == NULL);
  z = mapxy(test_tricont_z, NULL, x, y, n);
  cleanup_if(z == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n, y);
  grm_args_push(args, "z", "nD", n, z);
  grm_args_push(args, "kind", "s", "tricont");

  grm_plot(args);

cleanup:
  free(x_rand);
  free(y_rand);
  free(x);
  free(y);
  free(z);
  if (args != NULL)
    {
      grm_args_delete(args);
    }
}

static double test_surface1_x(double x)
{
  return 8 * x - 4;
}

static double test_surface1_z(double x, double y)
{
  return sin(x) + cos(y);
}

static void test_surface1(void)
{
  const int n = 100;
  double *x_rand = NULL, *y_rand = NULL;
  double *x = NULL, *y = NULL, *z = NULL;
  grm_args_t *args = NULL;

  x_rand = rand(NULL, n);
  cleanup_if(x_rand == NULL);
  y_rand = rand(NULL, n);
  cleanup_if(y_rand == NULL);

  x = mapx(test_surface1_x, NULL, x_rand, n);
  cleanup_if(x == NULL);
  y = mapx(test_surface1_x, NULL, y_rand, n);
  cleanup_if(y == NULL);
  z = mapxy(test_surface1_z, NULL, x, y, n);
  cleanup_if(z == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n, y);
  grm_args_push(args, "z", "nD", n, z);
  grm_args_push(args, "kind", "s", "surface");

  grm_plot(args);

cleanup:
  free(x_rand);
  free(y_rand);
  free(x);
  free(y);
  free(z);
  if (args != NULL)
    {
      grm_args_delete(args);
    }
}

static void test_surface2(void)
{
  const int n = 40;
  double *x = NULL, *y = NULL, *z = NULL;
  grm_args_t *args = NULL;

  x = lin_range(NULL, -2, 2, n, 1);
  cleanup_if(x == NULL);
  y = lin_range(NULL, 0.0, M_PI, n / 2, 1);
  cleanup_if(y == NULL);
  z = mapxty(test_surface1_z, NULL, x, y, n, n / 2);
  cleanup_if(z == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n / 2, y);
  grm_args_push(args, "z", "nD", n * (n / 2), z);
  grm_args_push(args, "kind", "s", "surface");

  grm_plot(args);

cleanup:
  free(x);
  free(y);
  free(z);
  if (args != NULL)
    {
      grm_args_delete(args);
    }
}

static double test_trisurf_x(double x)
{
  return 8 * x - 4;
}

static double test_trisurf_z(double x, double y)
{
  return sin(x) + cos(y);
}

static void test_trisurf(void)
{
  const int n = 100;
  double *x_rand = NULL, *y_rand = NULL;
  double *x = NULL, *y = NULL, *z = NULL;
  grm_args_t *args = NULL;

  x_rand = rand(NULL, n);
  cleanup_if(x_rand == NULL);
  y_rand = rand(NULL, n);
  cleanup_if(y_rand == NULL);

  x = mapx(test_trisurf_x, NULL, x_rand, n);
  cleanup_if(x == NULL);
  y = mapx(test_trisurf_x, NULL, y_rand, n);
  cleanup_if(y == NULL);
  z = mapxy(test_trisurf_z, NULL, x, y, n);
  cleanup_if(z == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n, y);
  grm_args_push(args, "z", "nD", n, z);
  grm_args_push(args, "kind", "s", "trisurf");

  grm_plot(args);

cleanup:
  free(x_rand);
  free(y_rand);
  free(x);
  free(y);
  free(z);
  if (args != NULL)
    {
      grm_args_delete(args);
    }
}

static double *peaks(unsigned int n)
{
  double *x = NULL, *y = NULL, *z = NULL;
  int i, j;

  x = lin_range(NULL, -3, 3, n, 0);
  cleanup_if(x == NULL);
  y = lin_range(NULL, -3, 3, n, 0);
  cleanup_if(y == NULL);
  z = malloc(n * n * sizeof(double));
  cleanup_if(z == NULL);

  for (i = 0; i < n; ++i)
    {
      for (j = 0; j < n; ++j)
        {
          z[n * i + j] = 3 * (1 - x[i]) * (1 - x[i]) * exp(-x[i] * x[i] - (y[j] + 1) * (y[j] + 1)) -
                         10 * (x[i] / 5 - x[i] * x[i] * x[i] - y[j] * y[j] * y[j] * y[j] * y[j]) *
                             exp(-x[i] * x[i] - y[j] * y[j]) -
                         exp(-(x[i] + 1) * (x[i] + 1) - y[j] * y[j]) / 3;
        }
    }

cleanup:
  free(x);
  free(y);

  return z;
}

static void test_surface_peaks(void)
{
  const int n = 49;
  double *z = NULL;
  grm_args_t *args = NULL;

  z = peaks(n);
  cleanup_if(z == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "z", "nD", n * n, z);
  grm_args_push(args, "z_dims", "ii", n, n);
  grm_args_push(args, "kind", "s", "surface");

  grm_plot(args);

cleanup:
  free(z);
  if (args != NULL)
    {
      grm_args_delete(args);
    }
}

static double test_wireframe_z(double x, double y)
{
  return sin(x) + cos(y);
}

static void test_wireframe(void)
{
  const int n = 40;
  double *x = NULL, *y = NULL, *z = NULL;
  grm_args_t *args = NULL;

  x = lin_range(NULL, -2, 2, n, 1);
  cleanup_if(x == NULL);
  y = lin_range(NULL, 0.0, M_PI, n / 2, 1);
  cleanup_if(y == NULL);
  z = mapxty(test_wireframe_z, NULL, x, y, n, n / 2);
  cleanup_if(z == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n / 2, y);
  grm_args_push(args, "z", "nD", n * (n / 2), z);
  grm_args_push(args, "kind", "s", "wireframe");

  grm_plot(args);

cleanup:
  free(x);
  free(y);
  free(z);
  if (args != NULL)
    {
      grm_args_delete(args);
    }
}

static double test_heatmap_z(double x, double y)
{
  return sin(x) + cos(y);
}

static void test_heatmap_and_imshow(void)
{
  const int n = 40;
  double *x = NULL, *y = NULL, *z = NULL;
  grm_args_t *args = NULL;

  x = lin_range(NULL, -2, 2, n, 1);
  cleanup_if(x == NULL);
  y = lin_range(NULL, 0.0, M_PI, n / 2, 1);
  cleanup_if(y == NULL);
  z = mapxty(test_heatmap_z, NULL, x, y, n, n / 2);
  cleanup_if(z == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "z", "nD", n * (n / 2), z);
  grm_args_push(args, "z_dims", "ii", n / 2, n);
  grm_args_push(args, "kind", "s", "heatmap");

  grm_plot(args);

  grm_args_delete(args);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "c", "nD", n * (n / 2), z);
  grm_args_push(args, "c_dims", "ii", n / 2, n);
  grm_args_push(args, "kind", "s", "imshow");

  grm_plot(args);

cleanup:
  free(x);
  free(y);
  free(z);
  if (args != NULL)
    {
      grm_args_delete(args);
    }
}

static void test_volume(void)
{
  const int n = 50;
  double *c = NULL;
  grm_args_t *args = NULL;

  c = randn(NULL, n * n * n);
  cleanup_if(c == NULL);
  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "c", "nD", n * n * n, c);
  grm_args_push(args, "c_dims", "iii", n, n, n);
  grm_args_push(args, "kind", "s", "volume");

  grm_plot(args);

cleanup:
  free(c);
  if (args != NULL)
    {
      grm_args_delete(args);
    }
}

static void test_shade(void)
{
  const int n = 1000000;
  double *x = NULL, *y = NULL;
  grm_args_t *args = NULL;

  x = randn(NULL, n);
  cleanup_if(x == NULL);
  y = randn(NULL, n);
  cleanup_if(y == NULL);
  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n, y);
  grm_args_push(args, "kind", "s", "shade");

  grm_plot(args);

cleanup:
  free(x);
  free(y);
  if (args != NULL)
    {
      grm_args_delete(args);
    }
}


int main(void)
{
  test_line();
  test_scatter();
  test_stem();
  test_histogram();
  test_line_only_y();
  test_plot3d();
  test_polar();
  test_scatter3();
  test_hexbin();
  test_contour1();
  test_contour2();
  test_contourf1();
  test_contourf2();
  test_tricont();
  test_surface1();
  test_surface2();
  test_trisurf();
  test_surface_peaks();
  test_wireframe();
  test_heatmap_and_imshow();
  /* TODO: Implement polarheatmap in GRM */
  /* TODO: Add isosurface plot */
  test_volume();
  test_shade();
  /* TODO: Add last plot: combination of surface, contour and polymarker */

  grm_finalize();

  return 0;
}
