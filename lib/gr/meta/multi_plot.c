#ifdef __unix__
#define _XOPEN_SOURCE 500
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "gr.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static void test_multiple_plots(void)
{
  double plots[2][2][1000];
  int n = sizeof(plots[0][0]) / sizeof(plots[0][0][0]);
  gr_meta_args_t *args;
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

  args = gr_newmeta();
  gr_meta_args_push(args, "x", "nD", n, plots[0][0]);
  gr_meta_args_push(args, "y", "nD", n, plots[0][1]);

  printf("plotting sin...\n");
  gr_plotmeta(args);
  printf("Press any key to continue...\n");
  getchar();

  gr_meta_args_clear(args);
  gr_meta_args_push(args, "x", "nD", n, plots[1][0]);
  gr_meta_args_push(args, "y", "nD", n, plots[1][1]);

  printf("plotting cos...\n");
  gr_plotmeta(args);
  printf("Press any key to continue...\n");
  getchar();

  gr_switchmeta(1);

  gr_meta_args_clear(args);
  gr_meta_args_push(args, "x", "nD", n, plots[0][0]);
  gr_meta_args_push(args, "y", "nD", n, plots[0][1]);

  printf("plotting sin...\n");
  gr_plotmeta(args);
  printf("Press any key to continue...\n");
  getchar();

  gr_meta_args_clear(args);
  gr_meta_args_push(args, "x", "nD", n, plots[1][0]);
  gr_meta_args_push(args, "y", "nD", n, plots[1][1]);
  gr_meta_args_push(args, "id", "s", ":.2");

  printf("plotting sin AND cos...\n");
  gr_plotmeta(args);
  printf("Press any key to continue...\n");
  getchar();

  gr_deletemeta(args);
  gr_finalizemeta();
}

int main(void)
{
  test_multiple_plots();

  return 0;
}
