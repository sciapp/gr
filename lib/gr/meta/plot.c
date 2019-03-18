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


static void test_consecutive_plots(void)
{
  double plots[2][2][1000];
  int n = sizeof(plots[0][0]) / sizeof(plots[0][0][0]);
  gr_meta_args_t *args;
  int i;

  printf("filling argument container...\n");

  for (i = 0; i < n; ++i)
    {
      plots[0][0][i] = i * 2 * M_PI / n;
      plots[0][1][i] = 2 * sin(i * 2 * M_PI / n);
    }
  for (i = 0; i < n; ++i)
    {
      plots[1][0][i] = i * 2 * M_PI / n;
      plots[1][1][i] = sin(i * 2 * M_PI / n);
    }

  args = gr_newmeta();
  for (i = 0; i < 2; ++i)
    {
      gr_meta_args_push(args, "x", "nD", n, plots[i][0]);
      gr_meta_args_push(args, "y", "nD", n, plots[i][1]);
      gr_plotmeta(args);
      printf("Press any key to continue...\n");
      getchar();
    }

  gr_deletemeta(args);
}

static void test_line(void)
{
  double plots[2][2][1000];
  int n = sizeof(plots[0][0]) / sizeof(plots[0][0][0]);
  const char *labels[] = {"sin", "cos"};
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
  gr_meta_args_push(args, "series", "nA", 2, series);
  gr_meta_args_push(args, "labels", "nS", 2, labels);
  gr_meta_args_push(args, "kind", "s", "line");

  printf("plotting data...\n");

  gr_plotmeta(args);

  printf("Press any key to continue...\n");
  getchar();

  gr_deletemeta(args);
}

static void test_contourf(void)
{
  double x[100], y[100], z[100];
  int n = sizeof(x) / sizeof(x[0]);
  int i;
  gr_meta_args_t *series, *subplot, *args;

  for (i = 0; i < n; ++i)
    {
      x[i] = (double)rand() / RAND_MAX * 8.0 - 4.0;
      y[i] = (double)rand() / RAND_MAX * 8.0 - 4.0;
      z[i] = sin(x[i]) + cos(y[i]);
    }

  printf("filling argument container...\n");

  series = gr_newmeta();
  gr_meta_args_push(series, "x", "nD", n, x);
  gr_meta_args_push(series, "y", "nD", n, y);
  gr_meta_args_push(series, "z", "nD", n, z);

  subplot = gr_newmeta();
  gr_meta_args_push(subplot, "series", "A(1)", &series);
  gr_meta_args_push(subplot, "kind", "s", "contourf");

  args = gr_newmeta();
  gr_meta_args_push(args, "subplots", "A(1)", &subplot);

  printf("plotting data...\n");

  gr_plotmeta(args);

  printf("Press any key to continue...\n");
  getchar();

  gr_deletemeta(args);
}

static void test_plotmeta(void)
{
  test_line();
  test_consecutive_plots();
  test_contourf();
  gr_finalizemeta();
}

int main(void)
{
  test_plotmeta();

  return 0;
}
