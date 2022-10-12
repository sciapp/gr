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
#define NBINS 20

static void test_plot(void)
{
  double plot[LENGTH];
  double weights[LENGTH];
  int n = LENGTH;
  double errors[2][NBINS];

  grm_args_t *args, *error;
  int i;

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
  grm_args_push(error, "upwardscap_color", "i", 2);
  grm_args_push(error, "downwardscap_color", "i", 3);
  grm_args_push(error, "errorbar_color", "i", 4);

  args = grm_args_new();
  grm_args_push(args, "x", "nD", n, plot);
  grm_args_push(args, "weights", "nD", n, weights);
  grm_args_push(args, "error", "a", error);
  grm_args_push(args, "kind", "s", "hist");
  grm_args_push(args, "nbins", "i", NBINS);
  /* Color as a RGB-list */
  grm_args_push(args, "bar_color", "ddd", 0., 0., 1.);
  grm_args_push(args, "edge_color", "ddd", 1., 0., 0.);
  /* Color as an index */
  /*grm_args_push(args, "bar_color", "i", 989);*/
  /*grm_args_push(args, "edge_color", "i", 1);*/
  grm_args_push(args, "title", "s", "Histogram of a sine wave [0; 2pi] with 20 bins and negative weights");

  printf("plotting data...\n");

  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}

int main(void)
{
  test_plot();
  grm_finalize();

  return 0;
}
