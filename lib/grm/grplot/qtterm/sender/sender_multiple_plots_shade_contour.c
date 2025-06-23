#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "grm.h"


static double gauss(void)
{
  double u = ((double)rand() / (RAND_MAX)) * 2 - 1;
  double v = ((double)rand() / (RAND_MAX)) * 2 - 1;
  double r = u * u + v * v;
  if (r == 0 || r > 1) return gauss();
  double c = sqrt(-2 * log(r) / r);
  return u * c;
}

static grm_args_t *test_shading(int number_of_points)
{
  int i;
  double *d_x = (double *)malloc(sizeof(double) * number_of_points);
  double *d_y = (double *)malloc(sizeof(double) * number_of_points);
  srand(10);

  for (i = 0; i < number_of_points; i++)
    {
      d_x[i] = gauss();
      d_y[i] = gauss();
    }

  grm_args_t *subplot, *series;

  series = grm_args_new();
  grm_args_push(series, "x", "nD", number_of_points, d_x);
  grm_args_push(series, "y", "nD", number_of_points, d_y);

  subplot = grm_args_new();
  grm_args_push(subplot, "series", "A", &series);
  grm_args_push(subplot, "subplot", "dddd", 0., 0.5, 0., 0.5);
  grm_args_push(subplot, "kind", "s", "shade");
  grm_args_push(subplot, "x_bins", "i", 200);
  grm_args_push(subplot, "y_bins", "i", 200);
  grm_args_push(subplot, "transformation", "i", 3);
  grm_args_push(subplot, "colormap", "i", 3);
  grm_args_push(subplot, "id", "s", "1");

  return subplot;
}

static grm_args_t *test_contourf(void)
{
  double x[100], y[100], z[100];
  int n = sizeof(x) / sizeof(x[0]);
  int i;
  grm_args_t *series, *subplot;

  for (i = 0; i < n; ++i)
    {
      x[i] = (double)rand() / RAND_MAX * 8.0 - 4.0;
      y[i] = (double)rand() / RAND_MAX * 8.0 - 4.0;
      z[i] = sin(x[i]) + cos(y[i]);
    }

  printf("filling argument container...\n");

  series = grm_args_new();
  grm_args_push(series, "x", "nD", n, x);
  grm_args_push(series, "y", "nD", n, y);
  grm_args_push(series, "z", "nD", n, z);

  subplot = grm_args_new();
  grm_args_push(subplot, "series", "A", &series);
  grm_args_push(subplot, "subplot", "dddd", 0., 0.5, 0.5, 1.);
  grm_args_push(subplot, "kind", "s", "contourf");
  grm_args_push(subplot, "id", "s", "2");

  return subplot;
}

int main(int argc, char **argv)
{
  if (argc != 2)
    {
      fprintf(stderr, "Usage: sender_multiple_plots_shade_contour <port>\n");
      return 1;
    }
  int port = atoi(argv[1]);
  if (port <= 0 || port > 65536)
    {
      fprintf(stderr, "Port must be between 1 and 65536\n");
      return 1;
    }

  void *handle;
  grm_args_t *plots = grm_args_new();
  handle = grm_open(GRM_SENDER, "localhost", port, NULL, NULL);
  if (handle == NULL)
    {
      fprintf(stderr, "sender could not be created\n");
      return 1;
    }


  grm_args_push(plots, "subplots", "aa", test_contourf(), test_shading(100));
  grm_send_args(handle, plots);
  grm_args_delete(plots);
  grm_close(handle);
  grm_finalize();
}
