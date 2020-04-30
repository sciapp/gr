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


static void new_plot_callback(const grm_event_t *event)
{
  fprintf(stderr, "Got new plot event, plot_id: %d\n", event->new_plot_event.plot_id);
}

static void size_callback(const grm_event_t *event)
{
  fprintf(stderr, "Got size event, size: (%d, %d)\n", event->size_event.width, event->size_event.width);
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

  grm_register(GRM_EVENT_NEW_PLOT, new_plot_callback);
  grm_register(GRM_EVENT_SIZE, size_callback);

  printf("plotting data...\n");

  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();

  grm_args_push(args, "size", "dd", 1000.0, 1000.0);

  printf("plotting data...\n");

  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();


  grm_args_push(args, "size", "dd", 1000.0, 1000.0);

  printf("plotting data...\n");

  grm_switch(1);
  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}

static void test_plot(void)
{
  test_line();
  grm_finalize();
}

int main(void)
{
  test_plot();

  return 0;
}
