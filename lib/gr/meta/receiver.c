#include <stdio.h>

#include "gr.h"


int test_recvmeta(void) {
  gr_meta_args_t *args;
  void *handle;

  args = gr_meta_args_new();

  printf("waiting for data... ");
  fflush(stdout);

  handle = gr_openmeta(GR_SOURCE_SOCKET, 8001);
  if (handle == NULL) {
    fprintf(stderr, "receiver could not be created\n");
    gr_meta_args_delete(args);
    return 1;
  }

  if (gr_recvmeta(handle, args) < 0) {
    gr_meta_args_delete(args);
    return 2;
  }

  gr_dumpmeta(args, stdout);

  gr_closemeta(handle);
  gr_meta_args_delete(args);

  return 0;
}

int main(void) {
  return test_recvmeta();
}