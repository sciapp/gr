#include <stdio.h>

#include "gr.h"


typedef struct {
  int n;
  double *x;
  double *y;
} data_t;


int test_sendmeta(void) {
  double x[3] = {0.0, 0.5, 1.0};
  double y[3] = {0.1, 0.25, 0.9};
  int n = sizeof(x) / sizeof(x[0]);
  data_t data;
  gr_meta_args_t *args;
  void *handle;

  printf("filling argument container...\n");

  data.n = n;
  data.x = x;
  data.y = y;
  args = gr_meta_args_new();
  gr_meta_args_push_kwarg_buf(args, "data", "nDD", &data, 1);
  gr_meta_args_push_kwarg(args, "color", "ddd", 1.0, 0.0, 0.5);

  printf("sending data...");
  fflush(stdout);

  handle = gr_openmeta(GR_TARGET_SOCKET, "localhost", 8001);
  if (handle == NULL) {
    fprintf(stderr, "sender could not be created\n");
    return -1;
  }

  gr_sendmeta(handle, "s(content:s(");
  gr_sendmeta_args(handle, args);
  gr_sendmeta(handle, ")");

  gr_closemeta(handle);

  printf("\tsent\n");

  return 0;
}

int main(void) {
  return test_sendmeta();
}
