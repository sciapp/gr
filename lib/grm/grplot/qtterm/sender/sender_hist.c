#ifdef __unix__
#define _XOPEN_SOURCE 500
#endif

#include <math.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>

#include "grm.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define LENGTH 2000
#define NBINS 40

static void test_plot(int port)
{
  double plot[LENGTH];
  double weights[LENGTH];
  int n = LENGTH;
  double errors[2][NBINS];

  grm_args_t *args, *error, *series[2];
  int i;
  void *handle = grm_open(GRM_SENDER, "localhost", port, NULL, NULL);
  if (handle == NULL)
    {
      fprintf(stderr, "sender could not be created\n");
    }
  printf("filling argument container...\n");

  for (i = 0; i < n; ++i)
    {
      plot[i] = sin(2 * M_PI * i / n);
      weights[i] = -1.0;
    }
  for (i = 0; i < NBINS; ++i)
    {
      errors[0][i] = fabs(sin(M_PI * i / (NBINS - 1)));
      errors[1][i] = fabs(sin(M_PI * i / (NBINS - 1)));
    }

  error = grm_args_new();
  grm_args_push(error, "relative", "nDD", NBINS, errors[0], errors[1]);
  grm_args_push(error, "upwards_cap_color", "i", 2);
  grm_args_push(error, "downwards_cap_color", "i", 3);
  grm_args_push(error, "error_bar_color", "i", 4);

  args = grm_args_new();

  series[0] = grm_args_new();
  grm_args_push(series[0], "x", "nD", n, plot);
  grm_args_push(series[0], "num_bins", "i", NBINS);
  //    Not yet supported:
  //    grm_args_push(series[0], "bar_color", "ddd", 1., 0., 0.);
  //    grm_args_push(series[0], "edge_color", "ddd", 0., 1., 0.);
  grm_args_push(series[0], "weights", "nD", n, weights);


  series[1] = grm_args_new();
  grm_args_push(series[1], "x", "nD", n, plot);
  //    Not yet supported:
  //    grm_args_push(series[1], "bar_color", "ddd", 0.5, 1.,0.5);
  //    grm_args_push(series[1], "edge_color", "ddd", 0., 0., 0.);
  grm_args_push(series[1], "num_bins", "i", NBINS / 3);


  grm_args_push(args, "series", "nA", 2, series);
  //  Color on per subplot basis:
  grm_args_push(args, "bar_color", "ddd", 1., 0., 0.);
  grm_args_push(args, "edge_color", "ddd", 0., 0., 1.);
  grm_args_push(args, "kind", "s", "histogram");
  grm_args_push(args, "title", "s", "Histogram of two sine waves [0; 2pi] with 40 and 13 bins");

  printf("plotting data...\n");

  grm_send_args(handle, args);

  grm_args_delete(args);
}

int main(int argc, char **argv)
{
  if (argc != 2)
    {
      fprintf(stderr, "usage: sender_hist <port>\n");
      return 1;
    }
  int port = atoi(argv[1]);
  if (port <= 0 || port > 65536)
    {
      fprintf(stderr, "port must be between 1 and 65536\n");
      return 1;
    }
  test_plot(port);
  grm_finalize();

  return 0;
}
