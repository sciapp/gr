#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "grm.h"
#include "c_rand.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define PLOT_WIDTH 3600
#define PLOT_HEIGHT 2700

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


static int test_line(void)
{
  const int n = 200;
  double *x = NULL, *y = NULL;
  grm_args_t *args = NULL;
  int was_successful = 0;

  x = lin_range(NULL, 0.0, 2 * M_PI, n, 1);
  cleanup_if(x == NULL);
  y = mapx(sin, NULL, x, n);
  cleanup_if(y == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n, y);
  grm_args_push(args, "kind", "s", "line");
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

  was_successful = grm_plot(args);

cleanup:
  free(x);
  free(y);
  if (args != NULL)
    {
      grm_args_delete(args);
    }

  return was_successful;
}


static double test_scatter_y(double x)
{
  return x - x * x;
}

static int test_scatter(void)
{
  const int n = 51;
  double *x = NULL, *y = NULL, *sz = NULL, *c = NULL;
  grm_args_t *args = NULL;
  int was_successful = 0;

  x = lin_range(NULL, 0.0, 1.0, n, 0);
  cleanup_if(x == NULL);
  y = mapx(test_scatter_y, NULL, x, n);
  cleanup_if(y == NULL);
  sz = lin_range(NULL, 0.5, 3.0, n, 0);
  cleanup_if(sz == NULL);
  c = lin_range(NULL, 0.0, 255.0, n, 0);
  cleanup_if(c == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n, y);
  grm_args_push(args, "kind", "s", "scatter");
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

  was_successful = grm_plot(args);
  cleanup_if(!was_successful);

  grm_args_push(args, "z", "nD", n, sz);
  grm_args_push(args, "c", "nD", n, c);

  was_successful = grm_plot(args);

cleanup:
  free(x);
  free(y);
  free(sz);
  free(c);
  if (args != NULL)
    {
      grm_args_delete(args);
    }

  return was_successful;
}


static int test_stem(void)
{
  const int n = 51;
  double *x = NULL, *y = NULL;
  grm_args_t *args = NULL;
  int was_successful = 0;

  x = lin_range(NULL, 0.0, 1.0, n, 0);
  cleanup_if(x == NULL);
  y = mapx(test_scatter_y, NULL, x, n);
  cleanup_if(y == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n, y);
  grm_args_push(args, "kind", "s", "stem");
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

  was_successful = grm_plot(args);

cleanup:
  free(x);
  free(y);
  if (args != NULL)
    {
      grm_args_delete(args);
    }

  return was_successful;
}


static int test_histogram(void)
{
  const int n = 10000;
  double *x = NULL;
  grm_args_t *args = NULL;
  int was_successful = 0;

  x = randn(NULL, n);
  cleanup_if(x == NULL);
  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "kind", "s", "hist");
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

  was_successful = grm_plot(args);

cleanup:
  free(x);
  if (args != NULL)
    {
      grm_args_delete(args);
    }

  return was_successful;
}


static int test_line_only_y(void)
{
  const int n = 50;
  double *y[] = {randn(NULL, n), randn(NULL, n), randn(NULL, n), randn(NULL, n)};
  grm_args_t *args = NULL, *series[] = {NULL, NULL, NULL, NULL};
  int i;
  int was_successful = 0;

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
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

  was_successful = grm_plot(args);
  cleanup_if(!was_successful);

  grm_args_delete(args);
  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "series", "nA", array_size(series), series);
  grm_args_push(args, "kind", "s", "line");
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);
  /* TODO: Use `hold_plots` for a real `oplot`! */
  /* grm_args_push(args, "hold_plots", "i", 1); */

  was_successful = grm_plot(args);

cleanup:
  for (i = 0; i < (int)array_size(y); ++i)
    {
      free(y[i]);
    }
  if (args != NULL)
    {
      grm_args_delete(args);
    }

  return was_successful;
}


static double test_plot3d_y(double x)
{
  return cos(x) * x;
}

static double test_plot3d_z(double x)
{
  return sin(x) * x;
}

static int test_plot3d(void)
{
  const int n = 1000;
  double *x = NULL, *y = NULL, *z = NULL;
  grm_args_t *args = NULL;
  int was_successful = 0;

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
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

  was_successful = grm_plot(args);

cleanup:
  free(x);
  free(y);
  free(z);
  if (args != NULL)
    {
      grm_args_delete(args);
    }

  return was_successful;
}

static int test_polar(void)
{
  const int n = 40;
  double *angles = NULL, *radii = NULL;
  grm_args_t *args = NULL;
  int was_successful = 0;

  angles = lin_range(NULL, 0.0, 2 * M_PI, n, 0);
  cleanup_if(angles == NULL);
  radii = lin_range(NULL, 0.0, 2.0, n, 0);
  cleanup_if(radii == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  /* TODO: Support `angles` and `radii` names in GRM! */
  grm_args_push(args, "x", "nD", n, angles);
  grm_args_push(args, "y", "nD", n, radii);
  grm_args_push(args, "kind", "s", "polar_line");
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

  was_successful = grm_plot(args);

cleanup:
  free(angles);
  free(radii);
  if (args != NULL)
    {
      grm_args_delete(args);
    }

  return was_successful;
}

static double test_scatter3_x(double x)
{
  return 2 * x - 1;
}

static double test_scatter3_c(double c)
{
  return 999 * c + 1;
}

static int test_scatter3(void)
{
  const int n = 100;
  double *x_rand = NULL, *y_rand = NULL, *z_rand = NULL, *c_rand = NULL;
  double *x = NULL, *y = NULL, *z = NULL, *c = NULL;
  grm_args_t *args = NULL;
  int was_successful = 0;

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
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

  was_successful = grm_plot(args);
  cleanup_if(!was_successful);

  grm_args_push(args, "c", "nD", n, c);

  was_successful = grm_plot(args);

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

  return was_successful;
}

static int test_hexbin(void)
{
  const int n = 100000;
  double *x = NULL, *y = NULL;
  grm_args_t *args = NULL;
  int was_successful = 0;

  x = randn(NULL, n);
  cleanup_if(x == NULL);
  y = randn(NULL, n);
  cleanup_if(y == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n, y);
  grm_args_push(args, "kind", "s", "hexbin");
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

  was_successful = grm_plot(args);

cleanup:
  free(x);
  free(y);
  if (args != NULL)
    {
      grm_args_delete(args);
    }

  return was_successful;
}

static double test_contour1_x(double x)
{
  return 8 * x - 4;
}

static double test_contour1_z(double x, double y)
{
  return sin(x) + cos(y);
}

static int test_contour1(void)
{
  const int n = 100;
  double *x_rand = NULL, *y_rand = NULL;
  double *x = NULL, *y = NULL, *z = NULL;
  grm_args_t *args = NULL;
  int was_successful = 0;

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
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

  was_successful = grm_plot(args);
  cleanup_if(!was_successful);
  grm_args_delete(args);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n, y);
  grm_args_push(args, "z", "nD", n, z);
  grm_args_push(args, "kind", "s", "contourf");
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

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

  return was_successful;
}

static int test_contour2(void)
{
  const int n = 40;
  double *x = NULL, *y = NULL, *z = NULL;
  grm_args_t *args = NULL;
  int was_successful = 0;

  x = lin_range(NULL, -2, 2, n, 0);
  cleanup_if(x == NULL);
  y = lin_range(NULL, 0.0, M_PI, n / 2, 0);
  cleanup_if(y == NULL);
  z = mapxty(test_contour1_z, NULL, x, y, n, n / 2);
  cleanup_if(z == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n / 2, y);
  grm_args_push(args, "z", "nD", n * (n / 2), z);
  grm_args_push(args, "kind", "s", "contour");
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

  was_successful = grm_plot(args);
  cleanup_if(!was_successful);
  grm_args_delete(args);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n / 2, y);
  grm_args_push(args, "z", "nD", n * (n / 2), z);
  grm_args_push(args, "kind", "s", "contourf");
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

  grm_plot(args);

cleanup:
  free(x);
  free(y);
  free(z);
  if (args != NULL)
    {
      grm_args_delete(args);
    }

  return was_successful;
}

static double test_contourf1_x(double x)
{
  return 8 * x - 4;
}

static double test_contourf1_z(double x, double y)
{
  return sin(x) + cos(y);
}

static int test_contourf1(void)
{
  const int n = 100;
  double *x_rand = NULL, *y_rand = NULL;
  double *x = NULL, *y = NULL, *z = NULL;
  grm_args_t *args = NULL;
  int was_successful = 0;

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
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

  was_successful = grm_plot(args);

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

  return was_successful;
}

static int test_contourf2(void)
{
  const int n = 40;
  double *x = NULL, *y = NULL, *z = NULL;
  grm_args_t *args = NULL;
  int was_successful = 0;

  x = lin_range(NULL, -2, 2, n, 0);
  cleanup_if(x == NULL);
  y = lin_range(NULL, 0.0, M_PI, n / 2, 0);
  cleanup_if(y == NULL);
  z = mapxty(test_contourf1_z, NULL, x, y, n, n / 2);
  cleanup_if(z == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n / 2, y);
  grm_args_push(args, "z", "nD", n * (n / 2), z);
  grm_args_push(args, "kind", "s", "contourf");
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

  was_successful = grm_plot(args);

cleanup:
  free(x);
  free(y);
  free(z);
  if (args != NULL)
    {
      grm_args_delete(args);
    }

  return was_successful;
}

static double test_tricont_x(double x)
{
  return 8 * x - 4;
}

static double test_tricont_z(double x, double y)
{
  return sin(x) + cos(y);
}

static int test_tricont(void)
{
  const int n = 100;
  double *x_rand = NULL, *y_rand = NULL;
  double *x = NULL, *y = NULL, *z = NULL;
  grm_args_t *args = NULL;
  int was_successful = 0;

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
  grm_args_push(args, "kind", "s", "tricontour");
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

  was_successful = grm_plot(args);

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

  return was_successful;
}

static double test_surface1_x(double x)
{
  return 8 * x - 4;
}

static double test_surface1_z(double x, double y)
{
  return sin(x) + cos(y);
}

static int test_surface1(void)
{
  const int n = 100;
  double *x_rand = NULL, *y_rand = NULL;
  double *x = NULL, *y = NULL, *z = NULL;
  grm_args_t *args = NULL;
  int was_successful = 0;

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
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

  was_successful = grm_plot(args);

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

  return was_successful;
}

static int test_surface2(void)
{
  const int n = 40;
  double *x = NULL, *y = NULL, *z = NULL;
  grm_args_t *args = NULL;
  int was_successful = 0;

  x = lin_range(NULL, -2, 2, n, 0);
  cleanup_if(x == NULL);
  y = lin_range(NULL, 0.0, M_PI, n / 2, 0);
  cleanup_if(y == NULL);
  z = mapxty(test_surface1_z, NULL, x, y, n, n / 2);
  cleanup_if(z == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n / 2, y);
  grm_args_push(args, "z", "nD", n * (n / 2), z);
  grm_args_push(args, "kind", "s", "surface");
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

  was_successful = grm_plot(args);

cleanup:
  free(x);
  free(y);
  free(z);
  if (args != NULL)
    {
      grm_args_delete(args);
    }

  return was_successful;
}

static double test_trisurf_x(double x)
{
  return 8 * x - 4;
}

static double test_trisurf_z(double x, double y)
{
  return sin(x) + cos(y);
}

static int test_trisurf(void)
{
  const int n = 100;
  double *x_rand = NULL, *y_rand = NULL;
  double *x = NULL, *y = NULL, *z = NULL;
  grm_args_t *args = NULL;
  int was_successful = 0;

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
  grm_args_push(args, "kind", "s", "trisurface");
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

  was_successful = grm_plot(args);

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

  return was_successful;
}

static double peak(double x, double y)
{
  return 3 * pow(1 - x, 2) * exp(-pow(x, 2) - pow(y + 1, 2)) -
         10 * (x / 5 - pow(x, 3) - pow(y, 5)) * exp(-pow(x, 2) - pow(y, 2)) - 1 / 3.0 * exp(-pow(x + 1, 2) - pow(y, 2));
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
          z[n * i + j] = peak(x[i], y[j]);
        }
    }

cleanup:
  free(x);
  free(y);

  return z;
}

static int test_surface_peaks(void)
{
  const int n = 49;
  double *z = NULL;
  grm_args_t *args = NULL;
  int was_successful = 0;

  z = peaks(n);
  cleanup_if(z == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "z", "nD", n * n, z);
  grm_args_push(args, "z_dims", "ii", n, n);
  grm_args_push(args, "kind", "s", "surface");
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

  was_successful = grm_plot(args);

cleanup:
  free(z);
  if (args != NULL)
    {
      grm_args_delete(args);
    }

  return was_successful;
}

static double test_wireframe_z(double x, double y)
{
  return sin(x) + cos(y);
}

static int test_wireframe(void)
{
  const int n = 40;
  double *x = NULL, *y = NULL, *z = NULL;
  grm_args_t *args = NULL;
  int was_successful = 0;

  x = lin_range(NULL, -2, 2, n, 0);
  cleanup_if(x == NULL);
  y = lin_range(NULL, 0.0, M_PI, n / 2, 0);
  cleanup_if(y == NULL);
  z = mapxty(test_wireframe_z, NULL, x, y, n, n / 2);
  cleanup_if(z == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n / 2, y);
  grm_args_push(args, "z", "nD", n * (n / 2), z);
  grm_args_push(args, "kind", "s", "wireframe");
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

  was_successful = grm_plot(args);

cleanup:
  free(x);
  free(y);
  free(z);
  if (args != NULL)
    {
      grm_args_delete(args);
    }

  return was_successful;
}

static double test_heatmap_z(double x, double y)
{
  return sin(x) + cos(y);
}

static int test_heatmap_and_imshow(void)
{
  const int n = 40;
  double *x = NULL, *y = NULL, *z = NULL;
  grm_args_t *args = NULL;
  int was_successful = 0;

  x = lin_range(NULL, -2, 2, n, 0);
  cleanup_if(x == NULL);
  y = lin_range(NULL, 0.0, M_PI, n / 2, 0);
  cleanup_if(y == NULL);
  z = mapxty(test_heatmap_z, NULL, x, y, n, n / 2);
  cleanup_if(z == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "z", "nD", n * (n / 2), z);
  grm_args_push(args, "z_dims", "ii", n, n / 2);
  grm_args_push(args, "kind", "s", "heatmap");
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

  was_successful = grm_plot(args);
  cleanup_if(!was_successful);

  grm_args_delete(args);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "c", "nD", n * (n / 2), z);
  grm_args_push(args, "c_dims", "ii", n, n / 2);
  grm_args_push(args, "kind", "s", "imshow");
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

  was_successful = grm_plot(args);

cleanup:
  free(x);
  free(y);
  free(z);
  if (args != NULL)
    {
      grm_args_delete(args);
    }

  return was_successful;
}

static int test_volume(void)
{
  const int n = 50;
  double *c = NULL;
  grm_args_t *args = NULL;
  int was_successful = 0;

  c = randn(NULL, n * n * n);
  cleanup_if(c == NULL);
  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "c", "nD", n * n * n, c);
  grm_args_push(args, "c_dims", "iii", n, n, n);
  grm_args_push(args, "kind", "s", "volume");
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

  was_successful = grm_plot(args);

cleanup:
  free(c);
  if (args != NULL)
    {
      grm_args_delete(args);
    }

  return was_successful;
}

static int test_shade(void)
{
  const int n = 1000000;
  double *x = NULL, *y = NULL;
  grm_args_t *args = NULL;
  int was_successful = 0;

  x = randn(NULL, n);
  cleanup_if(x == NULL);
  y = randn(NULL, n);
  cleanup_if(y == NULL);
  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n, y);
  grm_args_push(args, "kind", "s", "shade");
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

  was_successful = grm_plot(args);

cleanup:
  free(x);
  free(y);
  if (args != NULL)
    {
      grm_args_delete(args);
    }

  return was_successful;
}

static int test_isosurface(void)
{
  int i, j, k;
  const int n = 40;
  double *s = NULL, *v = NULL;
  grm_args_t *args = NULL;
  int dims[3] = {n, n, n};
  int was_successful = 0;

  s = lin_range(NULL, -1, 1, n, 0);
  cleanup_if(s == NULL);

  v = malloc(n * n * n * sizeof(double));
  cleanup_if(v == NULL);
  for (i = 0; i < n; i++)
    {
      for (j = 0; j < n; j++)
        {
          for (k = 0; k < n; k++)
            {
              v[k * n * n + j * n + i] = 1 - sqrt(s[i] * s[i] + s[j] * s[j] + s[k] * s[k]);
            }
        }
    }

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "kind", "s", "isosurface");
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);
  grm_args_push(args, "c", "nD", n * n * n, v);
  grm_args_push(args, "c_dims", "nI", 3, dims);
  grm_args_push(args, "isovalue", "d", 0.2);

  was_successful = grm_plot(args);

cleanup:
  free(s);
  free(v);
  if (args != NULL)
    {
      grm_args_delete(args);
    }

  return was_successful;
}

static int test_barplot(void)
{
  const int n = 20;
  double *x = NULL;
  grm_args_t *args = NULL;
  int was_successful = 0;

  x = randn(NULL, n);
  cleanup_if(x == NULL);
  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "y", "nD", n, x);
  grm_args_push(args, "kind", "s", "barplot");
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

  was_successful = grm_plot(args);

cleanup:
  free(x);
  if (args != NULL)
    {
      grm_args_delete(args);
    }

  return was_successful;
}

static int test_stairs(void)
{
  const int n = 51;
  double *x = NULL, *y = NULL;
  grm_args_t *args = NULL;
  int was_successful = 0;

  x = lin_range(NULL, 0.0, 1.0, n, 0);
  cleanup_if(x == NULL);
  y = mapx(test_scatter_y, NULL, x, n);
  cleanup_if(y == NULL);

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n, y);
  grm_args_push(args, "kind", "s", "stairs");
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

  was_successful = grm_plot(args);

cleanup:
  free(x);
  free(y);
  if (args != NULL)
    {
      grm_args_delete(args);
    }

  return was_successful;
}

static double test_polarheatmap_z(double x, double y)
{
  return sin(2.0 * x) * cos(y);
}

static int test_polarheatmap(void)
{
  int i, j;
  const int n = 200, m = 360;
  double *phi = NULL, *theta = NULL, *z = NULL, *zv = NULL;
  grm_args_t *args = NULL;
  int was_successful = 0;

  phi = lin_range(NULL, 0.0, 7, n, 0);
  cleanup_if(phi == NULL);
  theta = lin_range(NULL, 0.0, 2.0 * M_PI, m, 0);
  cleanup_if(theta == NULL);
  z = mapxty(test_polarheatmap_z, NULL, phi, theta, n, m);
  cleanup_if(z == NULL);

  zv = malloc(n * m * sizeof(double));
  for (i = 0; i < n; i++)
    {
      for (j = 0; j < m; j++)
        {
          zv[i * m + j] = z[j * n + i];
        }
    }

  args = grm_args_new();
  cleanup_if(args == NULL);
  grm_args_push(args, "x", "nD", m, theta);
  grm_args_push(args, "y", "nD", n, phi);
  grm_args_push(args, "z", "nD", n * m, zv);
  grm_args_push(args, "kind", "s", "polar_heatmap");
  grm_args_push(args, "size", "ii", PLOT_WIDTH, PLOT_HEIGHT);

  was_successful = grm_plot(args);

cleanup:
  free(phi);
  free(theta);
  free(z);
  free(zv);
  if (args != NULL)
    {
      grm_args_delete(args);
    }

  return was_successful;
}

int main(void)
{
  int was_successful = 0;

  /* 1      */ cleanup_if(!(was_successful = test_line()));
  /* 2, 3   */ cleanup_if(!(was_successful = test_scatter()));
  /* 4      */ cleanup_if(!(was_successful = test_stem()));
  /* 5      */ cleanup_if(!(was_successful = test_histogram()));
  /* 6, 7   */ cleanup_if(!(was_successful = test_line_only_y()));
  /* 8      */ cleanup_if(!(was_successful = test_plot3d()));
  /* 9      */ cleanup_if(!(was_successful = test_polar()));
  /* 10, 11 */ cleanup_if(!(was_successful = test_scatter3()));
  /* 12     */ cleanup_if(!(was_successful = test_hexbin()));
  /* 13, 14 */ cleanup_if(!(was_successful = test_contour1()));
  /* 15, 16 */ cleanup_if(!(was_successful = test_contour2()));
  /* 17     */ cleanup_if(!(was_successful = test_tricont()));
  /* 18     */ cleanup_if(!(was_successful = test_surface1()));
  /* 19     */ cleanup_if(!(was_successful = test_surface2()));
  /* 20     */ cleanup_if(!(was_successful = test_trisurf()));
  /* 21     */ cleanup_if(!(was_successful = test_surface_peaks()));
  /* 22     */ cleanup_if(!(was_successful = test_wireframe()));
  /* 23, 24 */ cleanup_if(!(was_successful = test_heatmap_and_imshow()));
  /* 25     */ cleanup_if(!(was_successful = test_polarheatmap()));
  /* 26     */ cleanup_if(!(was_successful = test_isosurface()));
  /* 27     */ cleanup_if(!(was_successful = test_volume()));
  /* 28     */ cleanup_if(!(was_successful = test_shade()));
  /*
   * Temporarily disabled
   * #if defined(__x86_64__) || defined(_M_X64)
   *   29 * cleanup_if(!(was_successful = test_barplot()));
   * #endif
   */
  /* 30     */ cleanup_if(!(was_successful = test_stairs()));

cleanup:
  grm_finalize();

  return was_successful ? 0 : 1;
}
