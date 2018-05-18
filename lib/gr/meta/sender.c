#include <stdio.h>

#include "gr.h"


typedef struct {
  int n;
  double *x;
  double *y;
} data_t;

static double x[3] = {0.0, 0.5, 1.0};
static double y[3] = {0.1, 0.25, 0.9};
static int n = sizeof(x) / sizeof(x[0]);


int test_sendmeta_ref(void) {
  void *handle;

  printf("sending data...");
  fflush(stdout);

  handle = gr_openmeta(GR_TARGET_SOCKET, "localhost", 8001);
  if (handle == NULL) {
    fprintf(stderr, "sender could not be created\n");
    return 1;
  }

  gr_sendmeta_ref(handle, "n", 'i', &n, 1);
  gr_sendmeta_ref(handle, "x", 'D', &x, n);
  gr_sendmeta_ref(handle, "y", 'D', &y, n);
  gr_sendmeta(handle, ")");

  printf("\tsent\n");

  gr_closemeta(handle);

  return 0;
}

int test_sendmeta_args(void) {
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
    gr_meta_args_delete(args);
    return 1;
  }

  gr_sendmeta(handle, "s(content:s(");
  gr_sendmeta_args(handle, args);
  gr_sendmeta(handle, ")");

  printf("\tsent\n");

  gr_closemeta(handle);
  gr_meta_args_delete(args);

  return 0;
}

int main(void) {
  return test_sendmeta_ref();
  /* return test_sendmeta_args(); */
}
