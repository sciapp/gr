#include <stdio.h>

#include "grm.h"


static double plots[2][2][3] = {{{0.0, 0.5, 1.0}, {0.1, 0.25, 0.9}}, {{0.0, 0.5, 1.0}, {0.2, 0.75, 0.95}}};
static int n_series = sizeof(plots) / sizeof(plots[0]);
static int n_points = sizeof(plots[0][0]) / sizeof(plots[0][0][0]);
static const char *labels[] = {"plot 1", "plot 2"};


static int disk_writer(const char *filepath, unsigned int id, const char *message)
{
  FILE *f;
  int error = 0;

  f = fopen(filepath, "w");
  if (f == NULL)
    {
      error = 1;
      goto cleanup;
    }
  if (fputs(message, f) == EOF)
    {
      error = 1;
      goto cleanup;
    }

cleanup:
  if (f != NULL && fclose(f) == EOF)
    {
      error = 1;
    }
  return error;
}

static int test_send_ref(void)
{
  void *handle;

  printf("writing data...");
  fflush(stdout);

  handle = grm_open(GRM_SENDER, "custom_sender.out", 0, NULL, disk_writer);
  if (handle == NULL)
    {
      fprintf(stderr, "\"grm_open\" failed.\n");
      return 1;
    }

  grm_send_ref(handle, "series", 'O', "[", n_series);
  grm_send_ref(handle, "x", 'D', plots[0][0], n_points);
  grm_send_ref(handle, "y", 'D', plots[0][1], n_points);
  grm_send_ref(handle, NULL, 'O', ",", 0);
  grm_send_ref(handle, "x", 'D', plots[1][0], n_points);
  grm_send_ref(handle, "y", 'D', plots[1][1], n_points);
  grm_send_ref(handle, NULL, 'O', "]", 0);
  grm_send_ref(handle, "labels", 'S', labels, n_series);
  grm_send_ref(handle, "kind", 's', "line", 0);
  grm_send_ref(handle, NULL, '\0', NULL, 0);

  printf("\twritten\n");

  grm_close(handle);

  return 0;
}

int main(void)
{
  return test_send_ref();
}
