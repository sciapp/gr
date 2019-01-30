#include <stdio.h>

#include "gr.h"


static double plots[2][2][3] = {{{0.0, 0.5, 1.0}, {0.1, 0.25, 0.9}}, {{0.0, 0.5, 1.0}, {0.2, 0.75, 0.95}}};
static int n_series = sizeof(plots) / sizeof(plots[0]);
static int n_points = sizeof(plots[0][0]) / sizeof(plots[0][0][0]);
static const char *labels[] = {"plot 1", "plot 2"};


int test_sendmeta_ref(void)
{
  void *handle;

  printf("sending data...");
  fflush(stdout);

  handle = gr_openmeta(GR_SENDER, "localhost", 8002, NULL, NULL);
  if (handle == NULL)
    {
      fprintf(stderr, "sender could not be created\n");
      return 1;
    }

  gr_sendmeta_ref(handle, "series", 'O', "[", n_series);
  gr_sendmeta_ref(handle, "x", 'D', plots[0][0], n_points);
  gr_sendmeta_ref(handle, "y", 'D', plots[0][1], n_points);
  gr_sendmeta_ref(handle, NULL, 'O', ",", 0);
  gr_sendmeta_ref(handle, "x", 'D', plots[1][0], n_points);
  gr_sendmeta_ref(handle, "y", 'D', plots[1][1], n_points);
  gr_sendmeta_ref(handle, NULL, 'O', "]", 0);
  gr_sendmeta_ref(handle, "labels", 'S', labels, n_series);
  gr_sendmeta_ref(handle, "kind", 's', "line", 0);
  gr_sendmeta_ref(handle, NULL, '\0', NULL, 0);

  printf("\tsent\n");

  gr_closemeta(handle);

  return 0;
}

int main(void)
{
  return test_sendmeta_ref();
}
