#include <stdio.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "grm.h"


int test_plotmeta(int port)
{
  double plots[2][2][1000];
  int n = sizeof(plots[0][0]) / sizeof(plots[0][0][0]);
  const char *labels[] = {"sin", "cos"};
  grm_args_t *args, *series[2];
  int i;

  void *handle;

  printf("sending data...");
  fflush(stdout);

  handle = grm_open(GRM_SENDER, "localhost", port, NULL, NULL);
  if (handle == NULL)
    {
      fprintf(stderr, "sender could not be created\n");
      return 1;
    }

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
  grm_args_push(args, "keep_aspect_ratio", "i", 0);
  grm_args_push(args, "y_flip", "i", 0);


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
      fprintf(stderr, "Usage: sender_lines <port>\n");
      return 1;
    }
  int port = atoi(argv[1]);
  if (port <= 0 || port > 65536)
    {
      fprintf(stderr, "Port must be between 1 and 65536\n");
      return 1;
    }
  test_plotmeta(port);
  return 1;
}
