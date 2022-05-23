#include <stdio.h>

#include "grm.h"


static double plots[2][2][3] = {{{0.0, 0.5, 1.0}, {0.1, 0.25, 0.9}}, {{0.0, 0.5, 1.0}, {0.2, 0.75, 0.95}}};
static int n_series = sizeof(plots) / sizeof(plots[0]);
static int n_points = sizeof(plots[0][0]) / sizeof(plots[0][0][0]);
static const char *labels[] = {"plot 1", "plot 2"};


int test_send_ref(void)
{
  void *handle;
  grm_args_t *request_args, *response_args;
  char *graphics_tree_str_encoded, *graphics_tree_str;

  printf("sending data... ");
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
  grm_send_ref(handle, NULL, 'O', ",", 0);
  grm_send_ref(handle, "x", 'D', plots[1][0], n_points);
  grm_send_ref(handle, "y", 'D', plots[1][1], n_points);
  grm_send_ref(handle, NULL, 'O', "]", 0);
  grm_send_ref(handle, "labels", 'S', labels, n_series);
  grm_send_ref(handle, "kind", 's', "line", 0);
  grm_send_ref(handle, NULL, '\0', NULL, 0);
  printf("sent\n");

  printf("request graphics tree... ");
  fflush(stdout);
  request_args = grm_args_new();
  grm_args_push(request_args, "request", "s", "graphics_tree");
  grm_send_args(handle, request_args);
  grm_args_delete(request_args);
  printf("sent\n");

  response_args = grm_args_new();
  grm_recv(handle, response_args);
  printf("\nreceived graphics tree (JSON object):\n");
  grm_dump(response_args, stderr);
  grm_args_values(response_args, "data", "s", &graphics_tree_str_encoded);
  graphics_tree_str = grm_base64_decode(NULL, graphics_tree_str_encoded, NULL, NULL);
  printf("\nreceived graphics tree (decoded):\n%s\n", graphics_tree_str);
  free(graphics_tree_str);

  grm_args_delete(response_args);

  grm_close(handle);

  grm_finalize();

  return 0;
}

int main(void)
{
  return test_send_ref();
}
