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


int test_plotmeta(void) {
  double plots[2][2][1000];
  int n = sizeof(plots[0][0]) / sizeof(plots[0][0][0]);
  const char *labels[] = {"sin", "cos"};
  gr_meta_args_t *args, *series[2];
  int i;

  printf("filling argument container...\n");

  for (i = 0; i < n; ++i) {
    plots[0][0][i] = i * 2 * M_PI / n;
    plots[0][1][i] = sin(i * 2 * M_PI / n);
  }
  for (i = 0; i < n; ++i) {
    plots[1][0][i] = i * 2 * M_PI / n;
    plots[1][1][i] = cos(i * 2 * M_PI / n);
  }

  for (i = 0; i < 2; ++i) {
    series[i] = gr_newmeta();
    gr_meta_args_push_kwarg(series[i], "x", "nD", n, plots[i][0]);
    gr_meta_args_push_kwarg(series[i], "y", "nD", n, plots[i][1]);
  }

  args = gr_newmeta();
  gr_meta_args_push_kwarg(args, "series", "nA", 2, series);
  gr_meta_args_push_kwarg(args, "labels", "nS", 2, labels);
  gr_meta_args_push_kwarg(args, "kind", "s", "line");

  printf("plotting data...\n");

  gr_plotmeta(args);

  printf("Press any key to continue...\n");
  getchar();

  gr_deletemeta(args);

  return 0;
}

int main(void) {
  return test_plotmeta();
}
