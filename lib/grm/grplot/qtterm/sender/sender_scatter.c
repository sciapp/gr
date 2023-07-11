#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "grm.h"

static double plots[4][4][5] = {{
    {0.0, 0.2, 0.4, 0.6, 1.0},
    {0.0, 0.5, 1.0, 0.6, 0.4},
    {200, 250, 300, 400, 500}, // sizes
    {0, 50, 100, 255, 200}     // colors
}};
static int n_points = sizeof(plots[0][0]) / sizeof(plots[0][0][0]);


int test_sendmeta_ref(void)
{
  void *handle;
  int *color, *markertype;
  int c, mt;
  color = &c;
  markertype = &mt;
  printf("sending data...");
  fflush(stdout);

  handle = grm_open(GRM_SENDER, "localhost", 8002, NULL, NULL);
  if (handle == NULL)
    {
      fprintf(stderr, "sender could not be created\n");
      return 1;
    }

  grm_send_ref(handle, "series", 'O', "[", 1);
  grm_send_ref(handle, "x", 'D', plots[0][0], n_points);
  grm_send_ref(handle, "y", 'D', plots[0][1], n_points);
  grm_send_ref(handle, "z", 'D', plots[0][2], n_points);
  grm_send_ref(handle, "c", 'D', plots[0][3], n_points); // set color as array

  mt = -9;
  grm_send_ref(handle, "markertype", 'i', markertype, 1); // set markertype

  grm_send_ref(handle, NULL, 'O', ",", 0);
  grm_send_ref(handle, "x", 'D', plots[0][1], n_points);
  grm_send_ref(handle, "y", 'D', plots[0][0], n_points);
  grm_send_ref(handle, "z", 'D', plots[0][2], n_points);

  c = 150;
  grm_send_ref(handle, "c", 'i', color, 1); // set color as integer

  mt = 4;
  grm_send_ref(handle, "markertype", 'i', markertype, 1); // set markertype

  grm_send_ref(handle, NULL, 'O', "]", 0);
  grm_send_ref(handle, "kind", 's', "scatter", 0);
  grm_send_ref(handle, NULL, '\0', NULL, 0);

  printf("\tsent\n");

  grm_close(handle);
  grm_finalize();
  return 0;
}


int main(void)
{
  return test_sendmeta_ref();
}
