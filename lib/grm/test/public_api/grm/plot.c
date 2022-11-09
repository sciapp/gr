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


static void test_consecutive_plots(void)
{
  double plots[2][2][1000];
  int n = sizeof(plots[0][0]) / sizeof(plots[0][0][0]);
  grm_args_t *args;
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

  args = grm_args_new();
  for (i = 0; i < 2; ++i)
    {
      grm_args_push(args, "x", "nD", n, plots[i][0]);
      grm_args_push(args, "y", "nD", n, plots[i][1]);
      grm_plot(args);
      printf("Press any key to continue...\n");
      getchar();
    }

  grm_args_delete(args);
}

static void test_line(void)
{
  double plots[2][2][1000];
  int n = sizeof(plots[0][0]) / sizeof(plots[0][0][0]);
  const char *labels[] = {"sin", "cos"};
  grm_args_t *args, *series[2];
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
      series[i] = grm_args_new();
      grm_args_push(series[i], "x", "nD", n, plots[i][0]);
      grm_args_push(series[i], "y", "nD", n, plots[i][1]);
    }

  args = grm_args_new();
  grm_args_push(args, "series", "nA", 2, series);
  grm_args_push(args, "labels", "nS", 2, labels);
  grm_args_push(args, "kind", "s", "line");

  printf("plotting data...\n");

  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}

static void test_line3d(void)
{
  double x[1000];
  double y[1000];
  double z[1000];
  int n = sizeof(x) / sizeof(x[0]);
  grm_args_t *args;
  int i;

  printf("filling argument container...\n");

  for (i = 0; i < n; ++i)
    {
      x[i] = i * 30.0 / n;
      y[i] = cos(x[i]) * x[i];
      z[i] = sin(x[i]) * x[i];
    }

  args = grm_args_new();
  grm_args_push(args, "kind", "s", "plot3");
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n, y);
  grm_args_push(args, "z", "nD", n, z);

  printf("plotting data...\n");

  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}

static void test_contourf(void)
{
  double x[100], y[100], z[100];
  int n = sizeof(x) / sizeof(x[0]);
  int i;
  grm_args_t *series, *subplot, *args;

  for (i = 0; i < n; ++i)
    {
      x[i] = (double)rand() / RAND_MAX * 8.0 - 4.0;
      y[i] = (double)rand() / RAND_MAX * 8.0 - 4.0;
      z[i] = sin(x[i]) + cos(y[i]);
    }

  printf("filling argument container...\n");

  series = grm_args_new();
  grm_args_push(series, "x", "nD", n, x);
  grm_args_push(series, "y", "nD", n, y);
  grm_args_push(series, "z", "nD", n, z);

  subplot = grm_args_new();
  grm_args_push(subplot, "series", "A(1)", &series);
  grm_args_push(subplot, "kind", "s", "contourf");

  args = grm_args_new();
  grm_args_push(args, "subplots", "A(1)", &subplot);

  printf("plotting data...\n");

  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}

static void test_plot(void)
{
  test_line();
  test_line3d();
  test_consecutive_plots();
  test_contourf();
  grm_finalize();
}

int main(void)
{
  test_plot();

  return 0;
}
