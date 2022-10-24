#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "grm.h"

static double plots[1][2][3] = {{{0.0, 0.5, 1.0}, {0.1, 0.85, 0.9}}};
static int n_series = sizeof(plots) / sizeof(plots[0]);
static int n_points = sizeof(plots[0][0]) / sizeof(plots[0][0][0]);
static const char *labels[] = {"plot 1"};


int test_sendmeta_ref(void)
{
  void *handle;

  printf("sending data...");
  fflush(stdout);

  handle = grm_open(GRM_SENDER, "localhost", 8002, NULL, NULL);
  if (handle == NULL)
    {
      fprintf(stderr, "sender could not be created\n");
      return 1;
    }

  grm_send_ref(handle, "series", 'O', "[", n_series);
  grm_send_ref(handle, "x", 'D', plots[0][0], n_points);
  grm_send_ref(handle, "y", 'D', plots[0][1], n_points);
  grm_send_ref(handle, NULL, 'O', "]", 0);
  grm_send_ref(handle, "labels", 'S', labels, n_series);
  grm_send_ref(handle, "kind", 's', "line", 0);
  grm_send_ref(handle, NULL, '\0', NULL, 0);

  printf("\tsent\n");

  grm_close(handle);
  grm_finalize();
  return 0;
}


int main(void)
{
  return test_sendmeta_ref();
}
