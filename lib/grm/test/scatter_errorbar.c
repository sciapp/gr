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

#define LENGTH 200

static void test_plot(void)
{
  double plot[2][LENGTH];
  int n = LENGTH;
  double errors[2][LENGTH];

  grm_args_t *args;
  grm_args_t *error;
  int i;

  srand(151515);
  printf("filling argument container...\n");

  for (i = 0; i < LENGTH; ++i)
    {
      plot[0][i] = i / 200. * M_PI * 3;
      plot[1][i] = cos(plot[0][i]) + rand() / (double)RAND_MAX;
      errors[0][i] = rand() / (double)RAND_MAX * (0.5 + cos(i / 200. * M_PI)) / 5;
      errors[1][i] = rand() / (double)RAND_MAX * (0.5 + cos(i / 200. * M_PI)) / 5;
    }

  error = grm_args_new();
  grm_args_push(error, "absolute", "nDD", LENGTH, errors[0], errors[1]);
  grm_args_push(error, "upwardscap_color", "i", -1);
  grm_args_push(error, "downwardscap_color", "i", -1);
  grm_args_push(error, "errorbar_color", "i", 4);

  args = grm_args_new();
  grm_args_push(args, "x", "nD", n, plot[0]);
  grm_args_push(args, "y", "nD", n, plot[1]);
  grm_args_push(args, "error", "a", error);
  grm_args_push(args, "kind", "s", "scatter");
  grm_args_push(args, "size", "dd", 1000., 1000.);

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
