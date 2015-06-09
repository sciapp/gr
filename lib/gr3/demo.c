/* 
  cc demo.c -I/usr/local/gr/include -L/usr/local/gr/lib -lGR3 -lGR
 */

#include "gr.h"
#include "gr3.h"

int main(void)
{
  float positions[3] = {0, 0, 0}, colors[3] = {0.5, 0.5, 0.5}, radii[1] = {2};

  gr3_clear();
  gr_setviewport(0.1, 0.95, 0.1, 0.95);
  gr3_setbackgroundcolor(1, 1, 1, 1);
  gr3_drawspheremesh(1, positions, colors, radii);
  gr3_drawimage(0, 1, 0, 1, 500, 500, GR3_DRAWABLE_GKS);
  gr_axes(0.04, 0.04, 0, 0, 5, 5, -0.01);
  gr_settextalign(2, 4);
  gr_text(0.525, 0.95, "GR3 Demo");
  gr3_export("gr3demo.png", 500, 500);
  gr_updatews();
}
