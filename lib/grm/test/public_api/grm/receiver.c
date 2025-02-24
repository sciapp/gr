#ifdef __unix__
#define _POSIX_C_SOURCE 200112L
#endif

#include <stdio.h>
#include <string.h>

#include "grm.h"

static void *handle;

void on_request_event(const grm_event_t *event)
{
  grm_request_event_t *request_event = (grm_request_event_t *)event;

  if (strcmp(request_event->request_string, "graphics_tree") == 0)
    {
      char *graphics_tree_str = grm_dump_graphics_tree_str();
      char *graphics_tree_str_encoded = grm_base64_encode(NULL, graphics_tree_str, strlen(graphics_tree_str), NULL);
      printf("\nsending requested graphics tree...");
      grm_send(handle, "o(data:s)", graphics_tree_str_encoded);
      printf("sent\n");
      free(graphics_tree_str_encoded);
      free(graphics_tree_str);
    }
}


int test_recv(void)
{
  grm_args_t *args;

  /* Do not create any graphics output on `grm_plot` (dummy output device) */
  setenv("GKS_WSTYPE", "100", 1);
  /* Suppress debug output (is generated in this function) */
  setenv("GR_DEBUG", "0", 1);

  grm_register(GRM_EVENT_REQUEST, on_request_event);

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
  printf("\npretty dump:\n");
  grm_dump(args, stdout);
  printf("\njson dump:\n");
  grm_dump_json(args, stdout);

  grm_plot(args);

  grm_args_delete(args);

  printf("\nwaiting for data... ");
  fflush(stdout);
  if ((args = grm_recv(handle, NULL)) == NULL)
    {
      grm_args_delete(args);
      return 2;
    }
  printf("received\n");
  printf("\npretty dump:\n");
  grm_dump(args, stdout);
  printf("\njson dump:\n");
  grm_dump_json(args, stdout);

  grm_merge(args);

  grm_args_delete(args);
  grm_close(handle);

  grm_finalize();

  return 0;
}

int main(void)
{
  return test_recv();
}
