#include <stdio.h>

#include "gr.h"


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

int test_recvmeta(void)
{
  gr_meta_args_t *args;
  void *handle;

  printf("waiting for data... ");
  fflush(stdout);

  handle = gr_openmeta(GR_RECEIVER, "custom_sender.out", 0, disk_reader, NULL);
  if (handle == NULL)
    {
      fprintf(stderr, "\"gr_openmeta\" failed.\n");
      return 1;
    }

  if ((args = gr_recvmeta(handle, NULL)) == NULL)
    {
      gr_deletemeta(handle);
      return 2;
    }

  printf("received\n");
  gr_dumpmeta(args, stdout);
  printf("\njson dump:\n");
  gr_dumpmeta_json(args, stdout);

  gr_closemeta(handle);
  gr_deletemeta(args);

  return 0;
}

int main(void)
{
  return test_recvmeta();
}
