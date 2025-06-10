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

int test_shading(int port, int number_of_points)
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

  handle = grm_open(GRM_SENDER, "localhost", port, NULL, NULL);
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
  grm_args_push(subplot, "x_bins", "i", 250);
  grm_args_push(subplot, "y_bins", "i", 250);
  grm_args_push(subplot, "transformation", "i", 3);
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

int main(int argc, char **argv)
{
  if (argc != 2 && argc != 3)
    {
      fprintf(stderr, "Usage: sender_shade <port> [number_of_points]\n");
      return 1;
    }
  int port = atoi(argv[1]);
  if (port <= 0 || port > 65536)
    {
      fprintf(stderr, "Port must be between 1 and 65536\n");
      return 1;
    }

  if (argc == 3)
    {
      return test_shading(port, atoi(argv[2]));
    }
  else
    {
      return test_shading(port, NUMBEROFPOINTS);
    }
}
