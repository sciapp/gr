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


static void test_hold_append(void)
{
  double plots[2][2][1000];
  int n = sizeof(plots[0][0]) / sizeof(plots[0][0][0]);
  gr_meta_args_t *args, *series[2];
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

  for (i = 0; i < 2; ++i)
    {
      series[i] = gr_newmeta();
      gr_meta_args_push(series[i], "x", "nD", n, plots[i][0]);
      gr_meta_args_push(series[i], "y", "nD", n, plots[i][1]);
    }

  args = gr_newmeta();
  gr_meta_args_push(args, "append_plots", "i", 1); /* Automatically create new plots, if no `plot_id` is given */
  gr_meta_args_push(args, "hold_plots", "i", 1);   /* Do not delete contents of the default plot automatically */
  gr_mergemeta(args);
  gr_meta_args_push(args, "series", "a", series[0]);

  printf("plotting data...\n");
  gr_plotmeta(args);
  printf("Press any key to continue...\n");
  getchar();

  gr_deletemeta(args);
  args = gr_newmeta();
  gr_meta_args_push(args, "size", "dd", 800.0, 800.0);
  gr_meta_args_push(args, "plot_id", "i", 0); /* Avoid creating a new plot */
  printf("plotting data...\n");
  gr_plotmeta(args);
  printf("Press any key to continue...\n");
  getchar();

  gr_deletemeta(args);
  args = gr_newmeta();
  gr_meta_args_push(args, "series", "a", series[1]);
  gr_mergemeta(args); /* This call will create a new plot with id `1` */
  printf("plotting data...\n");
  gr_switchmeta(1);
  gr_plotmeta(NULL);
  printf("Press any key to continue...\n");
  getchar();

  gr_deletemeta(args);
}

static void test_plotmeta(void)
{
  test_hold_append();
  gr_finalizemeta();
}

int main(void)
{
  test_plotmeta();

  return 0;
}
