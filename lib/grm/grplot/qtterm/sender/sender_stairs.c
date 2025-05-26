#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "grm.h"

static double plots[1][2][40] = {
    {{-2.,         -1.8974359,  -1.79487179, -1.69230769, -1.58974359, -1.48717949, -1.38461538, -1.28205128,
      -1.17948718, -1.07692308, -0.97435897, -0.87179487, -0.76923077, -0.66666667, -0.56410256, -0.46153846,
      -0.35897436, -0.25641026, -0.15384615, -0.05128205, 0.05128205,  0.15384615,  0.25641026,  0.35897436,
      0.46153846,  0.56410256,  0.66666667,  0.76923077,  0.87179487,  0.97435897,  1.07692308,  1.17948718,
      1.28205128,  1.38461538,  1.48717949,  1.58974359,  1.69230769,  1.79487179,  1.8974359,   2.},
     {-6.,         -5.12844114, -4.35560276, -3.67501138, -3.08019353, -2.56467574, -2.12198452, -1.74564642,
      -1.42918795, -1.16613564, -0.95001602, -0.7743556,  -0.63268093, -0.51851852, -0.4253949,  -0.3468366,
      -0.27637013, -0.20752204, -0.13381884, -0.04878707, 0.05404676,  0.18115612,  0.33901448,  0.53409532,
      0.7728721,   1.0618183,   1.40740741,  1.81611288,  2.2944082,   2.84876684,  3.48566227,  4.21156796,
      5.0329574,   5.95630405,  6.98808139,  8.13476289,  9.40282203,  10.79873228, 12.32896711, 14.}}};
static int n_series = sizeof(plots) / sizeof(plots[0]);
static int n_points = sizeof(plots[0][0]) / sizeof(plots[0][0][0]);
static const char *labels[] = {"post", "pre", "mid"};


int test_sendmeta_ref(void)
{
  void *handle;

  printf("sending data...");
  fflush(stdout);

  handle = grm_open(GRM_SENDER, "localhost", 8002, NULL, NULL);
  if (handle == NULL)
    {
      fprintf(stderr, "sender could not be created\n");
      return 1;
    }

  grm_send_ref(handle, "series", 'O', "[", n_series);
  grm_send_ref(handle, "x", 'D', plots[0][0], n_points);
  grm_send_ref(handle, "y", 'D', plots[0][1], n_points);
  grm_send_ref(handle, "step_where", 's', "post", 0);
  grm_send_ref(handle, "line_spec", 's', "r", 0);
  grm_send_ref(handle, NULL, 'O', ",", 0);
  grm_send_ref(handle, "x", 'D', plots[0][0], n_points);
  grm_send_ref(handle, "y", 'D', plots[0][1], n_points);
  grm_send_ref(handle, "step_where", 's', "pre", 0);
  grm_send_ref(handle, "line_spec", 's', "g-", 0);
  grm_send_ref(handle, NULL, 'O', ",", 0);
  grm_send_ref(handle, "x", 'D', plots[0][0], n_points);
  grm_send_ref(handle, "y", 'D', plots[0][1], n_points);
  grm_send_ref(handle, "step_where", 's', "mid", 0);
  grm_send_ref(handle, "line_spec", 's', "b-", 0);
  grm_send_ref(handle, NULL, 'O', "]", 0);
  grm_send_ref(handle, "kind", 's', "stairs", 0);
  grm_send_ref(handle, "labels", 'S', labels, 3);
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
