#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "grm.h"

#define RES 10

static grm_args_t *plot1()
{
  double plots[2][2][RES];
  int n = sizeof(plots[0][0]) / sizeof(plots[0][0][0]);
  const char *labels[] = {"sin", "cos"};
  grm_args_t *args, *series[2];
  int i;

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
  grm_args_push(args, "subplot", "dddd", 0., 0.5, 0., 0.5);
  //    grm_args_push(args, "id", "s", ".1");

  return args;
}

static grm_args_t *plot2(void)
{
  double plots[2][2][RES];
  int n = sizeof(plots[0][0]) / sizeof(plots[0][0][0]);
  const char *labels[] = {"x*x", "x"};
  grm_args_t *args, *series[2];
  int i;

  for (i = 0; i < n; ++i)
    {
      plots[0][0][i] = i * 2 * M_PI / n;
      plots[0][1][i] = plots[0][0][i] * plots[0][0][i];
    }
  for (i = 0; i < n; ++i)
    {
      plots[1][0][i] = i * 2 * M_PI / n;
      plots[1][1][i] = plots[0][0][i];
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
  grm_args_push(args, "subplot", "dddd", 0.5, 1., 0.0, 0.5);
  //    grm_args_push(args, "id", "s", ".2");

  return args;
}

static grm_args_t *plot3(void)
{
  double plots[2][2][RES];
  int n = sizeof(plots[0][0]) / sizeof(plots[0][0][0]);
  const char *labels[] = {"sin", "cos"};
  grm_args_t *args, *series[2];
  int i;

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
  grm_args_push(args, "subplot", "dddd", 0., 0.5, 0.5, 1.0);
  //    grm_args_push(args, "id", "s", ".3");

  return args;
}

static grm_args_t *plot4(void)
{
  double plots[2][2][RES];
  int n = sizeof(plots[0][0]) / sizeof(plots[0][0][0]);
  const char *labels[] = {"x*x", "x"};
  grm_args_t *args, *series[2];
  int i;

  for (i = 0; i < n; ++i)
    {
      plots[0][0][i] = i * 2 * M_PI / n;
      plots[0][1][i] = plots[0][0][i] * plots[0][0][i];
    }
  for (i = 0; i < n; ++i)
    {
      plots[1][0][i] = i * 2 * M_PI / n;
      plots[1][1][i] = plots[0][0][i];
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
  grm_args_push(args, "kind", "s", "scatter");
  grm_args_push(args, "subplot", "dddd", 0.5, 1.0, 0.5, 1.0);
  //    grm_args_push(args, "id", "s", ".4");

  return args;
}

int main(int argc, char **argv)
{
  if (argc != 2)
    {
      fprintf(stderr, "Usage: sender_multiple_plots <port>\n");
      return 1;
    }
  int port = atoi(argv[1]);
  if (port <= 0 || port > 65536)
    {
      fprintf(stderr, "Port must be between 1 and 65536\n");
      return 1;
    }

  void *handle;
  grm_args_t *plots = grm_args_new();
  handle = grm_open(GRM_SENDER, "localhost", port, NULL, NULL);
  if (handle == NULL)
    {
      fprintf(stderr, "sender could not be created\n");
      return 1;
    }


  grm_args_push(plots, "subplots", "aaaa", plot1(), plot2(), plot3(), plot4());
  grm_send_args(handle, plots);
  grm_args_delete(plots);
  grm_close(handle);
  grm_finalize();
}
