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

static void test_multiple_plots(void)
{
  double plots[2][2][1000];
  int n = sizeof(plots[0][0]) / sizeof(plots[0][0][0]);
  grm_args_t *args;
  int i;

  printf("filling argument container...\n");

  for (i = 0; i < n; ++i)
    {
      plots[0][0][i] = i * 2 * M_PI / n;
      plots[0][1][i] = sin(i * 2 * M_PI / n);
    }
  for (i = 0; i < n; ++i)
    {
      plots[1][0][i] = i * 2 * M_PI / n;
      plots[1][1][i] = cos(i * 2 * M_PI / n);
    }

  args = grm_args_new();
  grm_args_push(args, "x", "nD", n, plots[0][0]);
  grm_args_push(args, "y", "nD", n, plots[0][1]);

  printf("plotting sin...\n");
  grm_plot(args);
  printf("Press any key to continue...\n");
  getchar();

  grm_args_clear(args);
  grm_args_push(args, "x", "nD", n, plots[1][0]);
  grm_args_push(args, "y", "nD", n, plots[1][1]);

  printf("plotting cos...\n");
  grm_plot(args);
  printf("Press any key to continue...\n");
  getchar();

  grm_switch(1);

  grm_args_clear(args);
  grm_args_push(args, "x", "nD", n, plots[0][0]);
  grm_args_push(args, "y", "nD", n, plots[0][1]);

  printf("plotting sin...\n");
  grm_plot(args);
  printf("Press any key to continue...\n");
  getchar();

  grm_args_clear(args);
  grm_args_push(args, "x", "nD", n, plots[1][0]);
  grm_args_push(args, "y", "nD", n, plots[1][1]);
  grm_args_push(args, "id", "s", ":.2");

  printf("plotting sin AND cos...\n");
  grm_plot(args);
  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
  grm_finalize();
}

int main(void)
{
  test_multiple_plots();

  return 0;
}
