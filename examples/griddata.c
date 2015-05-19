/*
   cc griddata.c -I/usr/local/gr/include -L/usr/local/gr/lib -lGR
 */

#include <stdlib.h>
#include <math.h>

#include "gr.h"
#include "gks.h"

int main(void)
{
  double xd[100], yd[100], zd[100];
  double x[200], y[200], z[200*200];
  double h[20];
  int i;

  srand(0);
  for (i = 0; i < 100; i++)
    {
      xd[i] = -2 + 4.0 * rand() / RAND_MAX;
      yd[i] = -2 + 4.0 * rand() / RAND_MAX;
      zd[i] = xd[i] * exp(-xd[i]*xd[i] - yd[i]*yd[i]);
    }
 
  gr_setviewport(0.1, 0.95, 0.1, 0.95);
  gr_setwindow(-2, 2, -2, 2);
  gr_setspace(-0.5, 0.5, 0, 90);
  gr_setmarkersize(1);
  gr_setmarkertype(GKS_K_MARKERTYPE_SOLID_CIRCLE);
  gr_setcharheight(0.024);
  gr_settextalign(2, 0);
  gr_settextfontprec(3, 0);
  
  gr_gridit(100, xd, yd, zd, 200, 200, x, y, z);
  for (i = 0; i < 20; i++)
    h[i] = -0.5 + i / 19.0;
  gr_surface(200, 200, x, y, z, 5);
  gr_contour(200, 200, 20, x, y, h, z, 0);
  gr_polymarker(100, xd, yd);
  gr_axes(0.25, 0.25, -2, -2, 2, 2, 0.01);
  
  gr_updatews();
}
