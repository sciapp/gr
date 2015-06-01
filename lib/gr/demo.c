/*
   cc -o demo demo.c -I/usr/local/gr/include -L/usr/local/gr/lib -lGR
 */

#include <stdlib.h>
#include <math.h>

#include "gr.h"
#include "gks.h"

#define CURVES 20
#define POINTS 425

int main(void)
{
  double x[POINTS], y[CURVES][POINTS];
  int i, j, k;

  putenv("GKS_DOUBLE_BUF=1");
#ifdef __APPLE__
  putenv("GKS_WSTYPE=211");
#endif

  srand(0);
  for (j = 0; j < CURVES; j++)
    {
      x[0] = y[j][0] = 0;
      for (i = 1; i < POINTS; i++)
        {
          x[i] = i;
          y[j][i] = y[j][i-1] + (-0.5 + (double) rand() / RAND_MAX);
        }
    }

  k = 0;
  while (1)
    {
      gr_clearws();
      gr_setviewport(0.1, 0.95, 0.1, 0.95);
      gr_setwindow(k, k+POINTS, -20, 20);
      gr_setlinecolorind(1);
      gr_setcharheight(0.02);
      gr_axes(20, 1, k, -20, 5, 5, -0.01);
      for (j = 0; j < CURVES; j++)
        {
          gr_setlinecolorind(980 + j);
          gr_polyline(POINTS, x, y[j]);
        }
      gr_updatews();
      k++;
      for (i = 1; i < POINTS; i++)
        {
          x[i-1] = x[i];
          for (j = 0; j < CURVES; j++)
            y[j][i-1] = y[j][i];
        }
      x[POINTS-1] = k + POINTS;
      for (j = 0; j < CURVES; j++)
        y[j][POINTS-1] = y[j][POINTS-2] + (-0.5 + (double) rand() / RAND_MAX);
    }
}
