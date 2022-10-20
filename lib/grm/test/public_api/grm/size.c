#ifdef __unix__
#define _XOPEN_SOURCE 500
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "grm.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


static void test_size_container(void)
{
  double plots[2][1000];
  int n = sizeof(plots[0]) / sizeof(plots[0][0]);
  grm_args_t *args, *size[2];
  int i;

  printf("filling argument container...\n");

  for (i = 0; i < n; ++i)
    {
      plots[0][i] = i * 2 * M_PI / n;
      plots[1][i] = sin(i * 2 * M_PI / n);
    }

  for (i = 0; i < 2; ++i)
    {
      size[i] = grm_args_new();
      grm_args_push(size[i], "value", "d", 2.0 + (1 - i) * 1.0);
      grm_args_push(size[i], "unit", "s", "dm");
    }

  args = grm_args_new();
  grm_args_push(args, "x", "nD", n, plots[0]);
  grm_args_push(args, "y", "nD", n, plots[1]);
  grm_args_push(args, "kind", "s", "line");
  grm_args_push(args, "hold_plots", "i", 1);
  grm_args_push(args, "size", "nA", 2, size);

  printf("plotting data in a window of size (3.0 dm, 2.0 dm)...\n");

  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();

  size[0] = grm_args_new();
  grm_args_push(size[0], "value", "d", 10.0);
  grm_args_push(size[0], "unit", "s", "cm");
  size[1] = grm_args_new();
  grm_args_push(size[1], "value", "i", 500);
  grm_args_push(size[1], "unit", "s", "px");
  grm_args_push(args, "size", "nA", 2, size);

  printf("Change the output size to (10.0 cm, 500 px)...\n");

  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();

  grm_args_push(args, "size", "aa", grm_length(0.25, "ft"), grm_length(3, "in"));

  printf("Change the output size  to (0.25 ft, 3 in)...\n");

  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}

int main(void)
{
  test_size_container();

  return 0;
}
