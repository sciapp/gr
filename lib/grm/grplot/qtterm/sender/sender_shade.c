#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "grm.h"

#define NUMBEROFPOINTS 500000

double gauss(void)
{
  double u = ((double)rand() / (RAND_MAX)) * 2 - 1;
  double v = ((double)rand() / (RAND_MAX)) * 2 - 1;
  double r = u * u + v * v;
  if (r == 0 || r > 1) return gauss();
  double c = sqrt(-2 * log(r) / r);
  return u * c;
}

int test_shading(int number_of_points)
{
  int i, number_of_points_in_file;
  double *d_x = NULL, *d_y = NULL;
  d_x = (double *)malloc(sizeof(double) * number_of_points);
  d_y = (double *)malloc(sizeof(double) * number_of_points);

  srand(10);

  for (i = 0; i < number_of_points; i++)
    {
      d_x[i] = gauss();
      d_y[i] = gauss();
    }
  grm_args_t *args, *subplot, *series;

  void *handle;

  printf("sending data...");
  fflush(stdout);

  handle = grm_open(GRM_SENDER, "localhost", 8002, NULL, NULL);
  if (handle == NULL)
    {
      fprintf(stderr, "sender could not be created\n");
      return 1;
    }

  printf("filling argument container...\n");


  series = grm_args_new();
  subplot = grm_args_new();

  grm_args_push(series, "x", "nD", number_of_points, d_x);
  grm_args_push(series, "y", "nD", number_of_points, d_y);

  grm_args_push(subplot, "series", "A", &series);
  grm_args_push(subplot, "kind", "s", "shade");
  grm_args_push(subplot, "xbins", "i", 250);
  grm_args_push(subplot, "ybins", "i", 250);
  grm_args_push(subplot, "xform", "i", 3);
  grm_args_push(subplot, "colormap", "i", 3);

  args = grm_args_new();
  grm_args_push(args, "subplots", "A", &subplot);

  printf("plotting data...\n");

  grm_send_args(handle, args);
  grm_args_delete(args);
  grm_close(handle);
  grm_finalize();
  return 0;
}

int main(int argcount, char **argv)
{
  if (argcount > 1)
    {
      return test_shading(atoi(argv[1]));
    }
  else
    {
      return test_shading(NUMBEROFPOINTS);
    }
}
