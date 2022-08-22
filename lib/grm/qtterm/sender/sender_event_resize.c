#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "grm.h"


int main(int argc, char **argv)
{
  void *handle;
  double size[2] = {600., 600.};

  if (argc == 3)
    {
      size[0] = atoi(argv[1]) * 1.0;
      size[1] = atoi(argv[2]) * 1.0;
    }

  handle = grm_open(GRM_SENDER, "localhost", 8002, NULL, NULL);

  grm_send_ref(handle, "size", 'D', size, 2);
  grm_send_ref(handle, NULL, '\0', NULL, 0);

  grm_close(handle);
  grm_finalize();
}
