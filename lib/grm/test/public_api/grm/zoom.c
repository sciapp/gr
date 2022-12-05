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

static void test_zoom_using_interactions(void)
{
  double plots[2][1000];
  int n = sizeof(plots[0]) / sizeof(plots[0][0]);
  grm_args_t *args, *input_args;
  int i;

  printf("filling argument container...\n");

  for (i = 0; i < n; ++i)
    {
      plots[0][i] = i * 2 * M_PI / n;
      plots[1][i] = 2 * sin(i * 2 * M_PI / n);
    }

  args = grm_args_new();

  grm_args_push(args, "x", "nD", n, plots[0]);
  grm_args_push(args, "y", "nD", n, plots[1]);
  grm_plot(args);
  printf("Press any key to continue...\n");
  getchar();

  input_args = grm_args_new();
  grm_args_push(input_args, "x", "i", 300);
  grm_args_push(input_args, "y", "i", 225);
  grm_args_push(input_args, "factor", "d", 2.0);

  grm_input(input_args);
  grm_render();
  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
  grm_args_delete(input_args);
}

int main(void)
{
  test_zoom_using_interactions();

  return 0;
}
