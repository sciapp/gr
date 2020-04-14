#include <stdio.h>

#include "grm.h"


#define MESSAGE_SIZE 512


static const char *disk_reader(const char *filepath, unsigned int id)
{
  FILE *f;
  int error = 0;
  static char message[MESSAGE_SIZE];

  f = fopen(filepath, "r");
  if (f == NULL)
    {
      error = 1;
      goto cleanup;
    }
  if (fgets(message, MESSAGE_SIZE, f) == NULL)
    {
      error = 1;
      goto cleanup;
    }
  fprintf(stderr, "Read message from file:\"%s\"\n", message);

cleanup:
  if (f != NULL && fclose(f) == EOF)
    {
      error = 1;
    }
  if (error)
    {
      return NULL;
    }
  return message;
}

int test_recv(void)
{
  grm_args_t *args;
  void *handle;

  printf("waiting for data... ");
  fflush(stdout);

  handle = grm_open(GRM_RECEIVER, "custom_sender.out", 0, disk_reader, NULL);
  if (handle == NULL)
    {
      fprintf(stderr, "\"grm_open\" failed.\n");
      return 1;
    }

  if ((args = grm_recv(handle, NULL)) == NULL)
    {
      grm_args_delete(handle);
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
