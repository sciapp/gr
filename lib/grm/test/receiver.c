#include <stdio.h>

#include "grm.h"


int test_recv(void)
{
  grm_args_t *args;
  void *handle;

  printf("waiting for data... ");
  fflush(stdout);

  handle = grm_open(GRM_RECEIVER, "localhost", 8002, NULL, NULL);
  if (handle == NULL)
    {
      fprintf(stderr, "receiver could not be created\n");
      return 1;
    }

  if ((args = grm_recv(handle, NULL)) == NULL)
    {
      grm_args_delete(args);
      return 2;
    }

  printf("received\n");
  grm_dump(args, stdout);
  printf("\njson dump:\n");
  grm_dump_json(args, stdout);

  grm_close(handle);
  grm_args_delete(args);

  return 0;
}

int main(void)
{
  return test_recv();
}
