#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "grm.h"


static int test_contourf(int port)
{
  double x[100], y[100], z[100];
  int n = sizeof(x) / sizeof(x[0]);
  int i;
  grm_args_t *series, *subplot, *args;
  void *handle;

  handle = grm_open(GRM_SENDER, "localhost", port, NULL, NULL);
  if (handle == NULL)
    {
      fprintf(stderr, "sender could not be created\n");
      return 1;
    }

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

  grm_send_args(handle, args);
  grm_args_delete(args);

  grm_close(handle);
  grm_finalize();
  return 0;
}


int main(int argc, char **argv)
{
  if (argc != 2)
    {
      fprintf(stderr, "Usage: sender_contour <port>\n");
      return 1;
    }
  int port = atoi(argv[1]);
  if (port <= 0 || port > 65536)
    {
      fprintf(stderr, "Port must be between 1 and 65536\n");
      return 1;
    }
  return test_contourf(port);
}
