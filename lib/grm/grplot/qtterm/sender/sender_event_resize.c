#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "grm.h"


int main(int argc, char **argv)
{
  void *handle;
  double size[2] = {600., 600.};
  int port = -1;

  if (argc == 4)
    {
      size[0] = atoi(argv[2]) * 1.0;
      size[1] = atoi(argv[3]) * 1.0;
    }
  else if (argc != 2)
    {
      fprintf(stderr, "Usage: sender_event_resize <port>\n");
      return 1;
    }
  port = atoi(argv[1]);
  if (port <= 0 || port > 65536)
    {
      fprintf(stderr, "Port must be between 1 and 65536\n");
      return 1;
    }

  handle = grm_open(GRM_SENDER, "localhost", port, NULL, NULL);

  grm_send_ref(handle, "size", 'D', size, 2);
  grm_send_ref(handle, NULL, '\0', NULL, 0);

  grm_close(handle);
  grm_finalize();
}
