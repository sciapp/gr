/* 

 Linux:

t=../../3rdparty
for d in libpng16 jpeg zlib; do make -C ${t}/${d}; done
cc -g demo.c gr3.c gr3_glx.c gr3_convenience.c gr3_gr.c \
 gr3_html.c gr3_jpeg.c gr3_mc.c gr3_png.c gr3_povray.c \
 -I${t} -I${t}/jpeg -I${t}/zlib -I/usr/local/gr/include \
 -L/usr/local/gr/lib -lGR -Wl,-rpath,/usr/local/gr/lib \
 ${t}/libpng16/libpng.a ${t}/jpeg/libjpeg.a ${t}/zlib/libz.a \
 -lGL -lX11 -lm

 Darwin:

t=../../3rdparty
for d in libpng16 jpeg zlib; do make -C ${t}/${d}; done
cc -g demo.c gr3.c gr3_cgl.c gr3_convenience.c gr3_gr.c \
 gr3_html.c gr3_jpeg.c gr3_mc.c gr3_png.c gr3_povray.c \
 -I${t} -I${t}/jpeg -I${t}/zlib -I/usr/local/gr/include \
 -L/usr/local/gr/lib -lGR -framework OpenGL \
 ${t}/libpng16/libpng.a ${t}/jpeg/libjpeg.a ${t}/zlib/libz.a \
 -framework OpenGL

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
