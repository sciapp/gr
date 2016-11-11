
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>
#include <libpng16/png.h>

#if !defined(VMS) && !defined(_WIN32)
#include <unistd.h>
#endif

#include "gks.h"
#include "gkscore.h"

#define MEMORY_INCREMENT 32768

#define RESERVED_COLORS 32
#define MAX_POINTS 2048
#define PATTERNS 120
#define HATCH_STYLE 108
#define RESOLUTION 1200
#define PPI 80
#define FACTOR RESOLUTION/PPI

#define MWIDTH  0.254
#define MHEIGHT 0.1905
#define WIDTH   1024
#define HEIGHT  768

#define DrawBorder 0

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#ifdef _WIN32

#include <windows.h>
#define DLLEXPORT __declspec(dllexport)

#ifdef __cplusplus
extern "C"
{
#endif

#else

#ifdef __cplusplus
#define DLLEXPORT extern "C"
#else
#define DLLEXPORT
#endif

#endif

DLLEXPORT void gks_figplugin(
  int fctid, int dx, int dy, int dimx, int *i_arr,
  int len_f_arr_1, double *f_arr_1, int len_f_arr_2, double *f_arr_2,
  int len_c_arr, char *c_arr, void **ptr);

#ifdef _WIN32
#ifdef __cplusplus
}
#endif
#endif

#define MAX_TNR 9

#define WC_to_NDC(xw, yw, tnr, xn, yn) \
  xn = a[tnr] * (xw) + b[tnr]; \
  yn = c[tnr] * (yw) + d[tnr]

#define WC_to_NDC_rel(xw, yw, tnr, xn, yn) \
  xn = a[tnr] * (xw); \
  yn = c[tnr] * (yw)

#define NDC_to_DC(xn, yn, xd, yd) \
  xd = (int) (p->a * (xn) + p->b); \
  yd = (int) (p->c * (yn) + p->d)

#define CharXform(xrel, yrel, x, y) \
  x = cos(p->alpha) * (xrel) - sin(p->alpha) * (yrel); \
  y = sin(p->alpha) * (xrel) + cos(p->alpha) * (yrel);

#define nint(a) ((int)(a + 0.5))

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

static
gks_state_list_t *gkss;

static
double a[MAX_TNR], b[MAX_TNR], c[MAX_TNR], d[MAX_TNR];

typedef unsigned char Byte;
typedef unsigned long uLong;

typedef struct FIG_stream_t
{
  Byte *buffer;
  uLong size, length;
}
FIG_stream;

typedef struct FIG_point_t
{
  int x, y;
}
FIG_point;

typedef struct ws_state_list_t
{
  int conid, state, wtype;
  char *path;
  double a, b, c, d;
  double window[4], viewport[4];
  char rgb[MAX_COLOR][7];
  int width, height;
  int color, linewidth;
  double alpha, angle;
  int capheight;
  int pattern;
  FIG_stream *stream;
  FIG_point *points;
  int npoints, max_points;
  int empty, page_counter, offset;
  int font, size;
  int img_counter;
}
ws_state_list;

static
ws_state_list *p;

static
int fonts[] = {
  0, 1, 2, 3, 16, 17, 18, 19, 12, 13, 14, 15,
  32, 32, 32, 32, 8, 9, 10, 11, 24, 25, 26, 27,
  4, 5, 6, 7, 28, 29, 30, 31
};

static
double capheights[29] = {
  0.662, 0.660, 0.681, 0.662,
  0.729, 0.729, 0.729, 0.729,
  0.583, 0.583, 0.583, 0.583,
  0.667,
  0.681, 0.681, 0.681, 0.681,
  0.722, 0.722, 0.722, 0.722,
  0.739, 0.739, 0.739, 0.739,
  0.694, 0.693, 0.683, 0.683
};

static
int map[32] = {
  22, 9, 5, 14, 18, 26, 13, 1,
  24, 11, 7, 16, 20, 28, 13, 3,
  23, 10, 6, 15, 19, 27, 13, 2,
  25, 12, 8, 17, 21, 29, 13, 4
};

static
double xfac[4] = { 0, 0, -0.5, -1 };

static
double yfac[6] = { 0, -1.2, -1, -0.5, 0, 0.2 };

static
int predef_font[] = { 1, 1, 1, -2, -3, -4 };

static
int predef_prec[] = { 0, 1, 2, 2, 2, 2 };

static
int predef_ints[] = { 0, 1, 3, 3, 3 };

static
int predef_styli[] = { 1, 1, 1, 2, 3 };

static
void fig_memcpy(FIG_stream * p, char *s, size_t n)
{
  if (p->length + n >= p->size)
    {
      while (p->length + n >= p->size)
	p->size += MEMORY_INCREMENT;
      p->buffer = (Byte *) realloc(p->buffer, p->size);
    }

  memmove(p->buffer + p->length, s, n);
  p->length += n;
}

static
void fig_printf(FIG_stream * p, const char *args, ...)
{
  va_list ap;
  char fmt[BUFSIZ], s[BUFSIZ];

  strcpy(fmt, args);

  va_start(ap, args);
  vsprintf(s, fmt, ap);
  va_end(ap);

  fig_memcpy(p, s, strlen(s));
}

static
FIG_stream *fig_alloc_stream(void)
{
  FIG_stream *p;

  p = (FIG_stream *) calloc(1, sizeof(FIG_stream));
  p->buffer = NULL;
  p->size = p->length = 0;

  return p;
}

static
void fig_header(void)
{
  int i;

  fig_printf(p->stream, "#FIG 3.2\nLandscape\nCenter\nMetric\nLetter"
	     "\n100.00\nSingle\n-2\n%d 2\n", RESOLUTION);
  for (i = 0; i < MAX_COLOR; ++i)
    {
      fig_printf(p->stream, "0 %d #%s\n", RESERVED_COLORS + i,\
		 p->rgb[i]);
    }
}

static
void set_norm_xform(int tnr, double *wn, double *vp)
{
  a[tnr] = (vp[1] - vp[0]) / (wn[1] - wn[0]);
  b[tnr] = vp[0] - wn[0] * a[tnr];
  c[tnr] = (vp[3] - vp[2]) / (wn[3] - wn[2]);
  d[tnr] = vp[2] - wn[2] * c[tnr];
}

static
void init_norm_xform(void)
{
  int tnr;

  for (tnr = 0; tnr < MAX_TNR; tnr++)
    set_norm_xform(tnr, gkss->window[tnr], gkss->viewport[tnr]);
}

static
void set_xform(void)
{
  p->a = p->width / (p->window[1] - p->window[0]);
  p->b = -p->window[0] * p->a;
  p->c = p->height / (p->window[2] - p->window[3]);
  p->d = p->height - p->window[2] * p->c;
}

static
void seg_xform(double *x, double *y)
{
  double xx;

  xx = *x * gkss->mat[0][0] + *y * gkss->mat[0][1] + gkss->mat[2][0];
  *y = *x * gkss->mat[1][0] + *y * gkss->mat[1][1] + gkss->mat[2][1];
  *x = xx;
}

static
void seg_xform_rel(double *x, double *y)
{
  double xx;

  xx = *x * gkss->mat[0][0] + *y * gkss->mat[0][1];
  *y = *x * gkss->mat[1][0] + *y * gkss->mat[1][1];
  *x = xx;
}

static
void set_color_rep(int color, double red, double green, double blue)
{
  if (color >= 0 && color < MAX_COLOR)
    {
      sprintf(p->rgb[color], "%02x%02x%02x", (int) (red * 255),
	      (int) (green * 255), (int) (blue * 255));
    }
}

static
void init_colors(void)
{
  int color;
  double red, green, blue;

  for (color = 0; color < MAX_COLOR; color++)
    {
      gks_inq_rgb(color, &red, &green, &blue);
      set_color_rep(color, red, green, blue);
    }
}

static
void resize_window(void)
{
  p->width = nint((p->viewport[1] - p->viewport[0]) / MWIDTH * WIDTH);
  p->height = nint((p->viewport[3] - p->viewport[2]) / MHEIGHT * HEIGHT);
}

static
void draw_marker(double xn, double yn, int mtype, double mscale, int mcolor)
{
  int r, x, y, i;
  double scale, xr, yr;
  int pc, op;

#include "marker.h"

  r = (int) (3 * mscale);
  scale = 0.01 * mscale / 3.0;

  xr = r;
  yr = 0;
  seg_xform_rel(&xr, &yr);
  r = nint(sqrt(xr * xr + yr * yr));

  NDC_to_DC(xn, yn, x, y);

  pc = 0;
  mtype = (r > 0) ? mtype + marker_off : marker_off + 1;

  do
    {
      op = marker[mtype][pc];
      switch (op)
	{
	case 1:		/* point */
	  fig_printf(p->stream, "2 1 0 1 %d 0 50 -1 -1 0.000 0 0 -1 0 0 2\n",
		     mcolor + 32);
	  fig_printf(p->stream, "         %d %d %d %d\n", x * FACTOR,\
		     y * FACTOR, x * FACTOR + 1, y * FACTOR);
	  break;

	case 2:		/* line */
	  for (i = 0; i < 2; i++)
	    {
	      xr = scale * marker[mtype][pc + 2 * i + 1];
	      yr = -scale * marker[mtype][pc + 2 * i + 2];
	      seg_xform_rel(&xr, &yr);
	      if (i == 0)
		{
		  fig_printf(p->stream,
			     "2 1 0 1 %d 0 50 -1 -1 0.000 0 0 -1 0 0" " 2\n",
			     mcolor + 32);
		  fig_printf(p->stream, "         %d %d ",
			     (int) (x - xr) * FACTOR, (int) (y - yr) * FACTOR);
		}
	      else
		fig_printf(p->stream, "%d %d\n", (int) (x - xr) * FACTOR,
			   (int) (y + yr) * FACTOR);
	    }
	  pc += 4;
	  break;

	case 3:		/* polyline */
	  fig_printf(p->stream, "2 1 0 1 %d 0 50 -1 -1 0.000 0 0 -1 0 0 %d"
		     "\n", mcolor + 32, marker[mtype][pc + 1]);
	  fig_printf(p->stream, "         ");
	  for (i = 0; i < marker[mtype][pc + 1]; i++)
	    {
	      xr = scale * marker[mtype][pc + 2 + 2 * i];
	      yr = -scale * marker[mtype][pc + 3 + 2 * i];
	      seg_xform_rel(&xr, &yr);
	      fig_printf(p->stream, "%d %d ", (int) (x - xr) * FACTOR,
			 (int) (y + yr) * FACTOR);
	    }
	  fig_printf(p->stream, "\n");
	  pc += 1 + 2 * marker[mtype][pc + 1];
	  break;

	case 4:		/* filled polygon */
	case 5:		/* hollow polygon */
	  if (op == 5)
	    fig_printf(p->stream, "2 1 0 1 %d 0 50 -1 -1 0.000 0 0 -1 0 0 %d"
		       "\n", mcolor + 32, marker[mtype][pc + 1]);
	  else
	    fig_printf(p->stream, "2 1 0 1 0 %d 50 -1 20 0.000 0 0 -1 0 0 %d"
		       "\n", mcolor + 32, marker[mtype][pc + 1]);
	  fig_printf(p->stream, "         ");
	  for (i = 0; i < marker[mtype][pc + 1]; i++)
	    {
	      xr = scale * marker[mtype][pc + 2 + 2 * i];
	      yr = -scale * marker[mtype][pc + 3 + 2 * i];
	      seg_xform_rel(&xr, &yr);
	      fig_printf(p->stream, "%d %d ", (int) (x - xr) * FACTOR,
			 (int) (y + yr) * FACTOR);
	    }
	  fig_printf(p->stream, "\n");
	  pc += 1 + 2 * marker[mtype][pc + 1];
	  if (op == 5)
	    p->color = mcolor;
	  break;

	case 6:		/* arc */
	  fig_printf(p->stream, "1 3 0 1 %d 0 50 -1 -1 0.000 1 0.0000",
		     mcolor + 32);
	  fig_printf(p->stream, " %d %d %d %d %d %d %d %d\n", x * FACTOR,\
		     y * FACTOR, r * FACTOR, r * FACTOR, x * FACTOR,\
		     y * FACTOR, r * FACTOR, (y + r) * FACTOR);
	  break;

	case 7:		/* filled arc */
	case 8:		/* hollow arc */
	  if (op == 8)
	    {
	      fig_printf(p->stream, "1 3 0 1 0 0 50 -1 -1 0.000 1 0.0000");
	      fig_printf(p->stream, " %d %d %d %d %d %d %d %d\n", x * FACTOR,
			 y * FACTOR, r * FACTOR, r * FACTOR, x * FACTOR,\
			 y * FACTOR, r * FACTOR, (y + r) * FACTOR);
	    }
	  else
	    {
	      fig_printf(p->stream, "1 3 0 1 %d %d 50 -1 20 0.000 1 0.0000",
			 mcolor + 32, mcolor + 32);
	      fig_printf(p->stream, " %d %d %d %d %d %d %d %d\n", x * FACTOR,
			 y * FACTOR, r * FACTOR, r * FACTOR, x * FACTOR, \
			 y * FACTOR, r * FACTOR, (y + r) * FACTOR);
	    }
	  break;
	}
      pc++;
    }
  while (op != 0);
}

static
void marker_routine(int n, double *px, double *py, int mtype, double mscale,
		    int mcolor)
{
  double x, y;
  double *clrt = gkss->viewport[gkss->cntnr];
  int i, draw;

  for (i = 0; i < n; i++)
    {
      WC_to_NDC(px[i], py[i], gkss->cntnr, x, y);
      seg_xform(&x, &y);

      if (gkss->clip == GKS_K_CLIP)
	draw = (x >= clrt[0] && x <= clrt[1] && y >= clrt[2] && y <= clrt[3]);
      else
	draw = 1;

      if (draw)
	draw_marker(x, y, mtype, mscale, mcolor);
    }
}

static
void polymarker(int n, double *px, double *py)
{
  int mk_type, mk_color;
  double mk_size;

  mk_type = gkss->asf[3] ? gkss->mtype : gkss->mindex;
  mk_size = gkss->asf[4] ? gkss->mszsc : 1;
  mk_color = gkss->asf[5] ? gkss->pmcoli : 1;

  marker_routine(n, px, py, mk_type, mk_size, mk_color);
}

static
void stroke(void)
{
  int i;

  fig_printf(p->stream, "2 1 0 %d %d 0 50 -1 -1 0.000 0 0 -1 0 0 %d\n",
	     p->linewidth, p->color + 32, p->npoints);
  fig_printf(p->stream, "         ");
  for (i = 0; i < p->npoints; i++)
    {
      fig_printf(p->stream, "%d %d ", p->points[i].x * FACTOR,
		 p->points[i].y * FACTOR);
    }
  fig_printf(p->stream, "\n");

  p->npoints = 0;
}

static
void move_to(double x, double y)
{
  if (p->npoints > 0)
    stroke();

  NDC_to_DC(x, y, p->points[p->npoints].x, p->points[p->npoints].y);
  p->npoints++;
}

static
void line_to(double x, double y)
{
  NDC_to_DC(x, y, p->points[p->npoints].x, p->points[p->npoints].y);
  p->npoints++;
}

static
void move(double x, double y)
{
  gks_move(x, y, move_to);
}

static
void draw(double x, double y)
{
  gks_dash(x, y, move_to, line_to);
}

static
void line_routine(int n, double *px, double *py, int linetype, int tnr)
{
  double x, y;
  int i, x0, y0, xi, yi;

  WC_to_NDC(px[0], py[0], tnr, x, y);
  seg_xform(&x, &y);
  NDC_to_DC(x, y, x0, y0);

  if (linetype == 0)
    fig_printf(p->stream, "2 1 0 1 %d 0 50 -1 -1 0.000 0 0 -1 0 0 %d\n",
	       p->color + 32, n + 1);
  else
    fig_printf(p->stream, "2 1 0 1 %d 0 50 -1 -1 0.000 0 0 -1 0 0 %d\n",
	       p->color + 32, n);
  fig_printf(p->stream, "         %d %d ", x0 * FACTOR, y0 * FACTOR);
  for (i = 1; i < n; i++)
    {
      WC_to_NDC(px[i], py[i], tnr, x, y);
      seg_xform(&x, &y);
      NDC_to_DC(x, y, xi, yi);

      fig_printf(p->stream, "%d %d ", xi * FACTOR, yi * FACTOR);
    }
  if (linetype == 0)
    fig_printf(p->stream, "%d %d ", x0 * FACTOR, y0 * FACTOR);

  fig_printf(p->stream, "\n");
}

static
void fill_routine(int n, double *px, double *py, int tnr)
{
  int i;
  double x, y;
  int ix, iy;
  int pat;

  if (p->pattern)
    {
      switch (p->pattern + 1)
	{
	case 13:  pat = 0;  break;
	case 12:  pat = 1;  break;
	case 11:  pat = 2;  break;
	case 10:  pat = 3;  break;
	case 9:   pat = 4;  break;
	case 8:   pat = 10; break;
	case 7:   pat = 11; break;
	case 6:   pat = 12; break;
	case 5:   pat = 15; break;
	case 4:   pat = 16; break;
	case 3:   pat = 18; break;
	case 2:   pat = 19; break;
	case 113: pat = 44; break;
	default:  pat = 20; break;
	}
      fig_printf(p->stream, "2 3 0 0 0 7 50 -1 %d 0.000 0 0 -1 0 0 %d\n",
		 pat, n);
    }
  else
    fig_printf(p->stream, "2 1 0 0 0 %d 50 -1 20 0.000 0 0 -1 0 0 %d\n",
	       p->color + 32, n);
  fig_printf(p->stream, "         ");
  for (i = 0; i < n; i++)
    {
      WC_to_NDC(px[i], py[i], tnr, x, y);
      seg_xform(&x, &y);
      NDC_to_DC(x, y, ix, iy);

      fig_printf(p->stream, "%d %d ", ix * FACTOR, iy * FACTOR);
    }
  fig_printf(p->stream, "\n");
}

static
void fillarea(int n, double *px, double *py)
{
  int fl_inter, fl_style, fl_color;

  fl_inter = gkss->asf[10] ? gkss->ints : predef_ints[gkss->findex - 1];
  fl_style = gkss->asf[11] ? gkss->styli : predef_styli[gkss->findex - 1];
  fl_color = gkss->asf[12] ? gkss->facoli : 1;

  p->pattern = 0;
  if (fl_inter == GKS_K_INTSTYLE_HOLLOW)
    {
      p->color = fl_color;
      line_routine(n, px, py, DrawBorder, gkss->cntnr);
    }
  else if (fl_inter == GKS_K_INTSTYLE_SOLID)
    {
      p->color = fl_color;
      fill_routine(n, px, py, gkss->cntnr);
    }
  else if (fl_inter == GKS_K_INTSTYLE_PATTERN ||
	   fl_inter == GKS_K_INTSTYLE_HATCH)
    {
      p->color = fl_color;
      if (fl_inter == GKS_K_INTSTYLE_HATCH)
	fl_style += HATCH_STYLE;
      if (fl_style >= PATTERNS)
	fl_style = 1;
      p->pattern = fl_style;
      fill_routine(n, px, py, gkss->cntnr);
    }
}

static
void polyline(int n, double *px, double *py)
{
  int ln_type, ln_color;
  double ln_width;
  int width;

  if (n > p->max_points)
    {
      p->points = (FIG_point *) realloc(p->points, n * sizeof(FIG_point));
      p->max_points = n;
    }

  ln_type = gkss->asf[0] ? gkss->ltype : gkss->lindex;
  ln_width = gkss->asf[1] ? gkss->lwidth : 1;
  ln_color = gkss->asf[2] ? gkss->plcoli : 1;

  width = nint(ln_width);
  if (width < 1)
    width = 0;

  p->linewidth = width;
  p->color = ln_color;

  gks_set_dev_xform(gkss, p->window, p->viewport);
  gks_emul_polyline(n, px, py, ln_type, gkss->cntnr, move, draw);

  if (p->npoints > 0)
    stroke();
}

static
void text_routine(double x, double y, int nchars, char *chars)
{
  int xstart, ystart, width, height, ch;
  double xrel, yrel, ax, ay;
  int i;

  NDC_to_DC(x, y, xstart, ystart);

  width = 0;
  height = p->capheight;

  xrel = width * xfac[gkss->txal[0]];
  yrel = p->capheight * yfac[gkss->txal[1]];
  CharXform(xrel, yrel, ax, ay);
  xstart += (int) ax;
  ystart -= (int) ay;

  fig_printf(p->stream, "%d %d %d %d ", FACTOR * height, FACTOR * width,
	     FACTOR * xstart, FACTOR * ystart);

  for (i = 0; i < nchars; ++i)
    {
      ch = (chars[i] < 0) ? chars[i] + 256 : chars[i];
      if (ch > 127)
	fig_printf(p->stream, "\\%o;", ch);
      else
	fig_printf(p->stream, "%c", ch);
    }
  fig_printf(p->stream, "\\001\n");
}

static
void set_font(int font)
{
  double scale, ux, uy, angle;
  int size;
  double width, height, capheight;

  if (gkss->txal[0] == GKS_K_TEXT_HALIGN_CENTER)
    fig_printf(p->stream, "1 ");
  else if (gkss->txal[0] == GKS_K_TEXT_HALIGN_RIGHT)
    fig_printf(p->stream, "2 ");
  else
    fig_printf(p->stream, "0 ");

  fig_printf(p->stream, "%d 50 -1 ", p->color + 32);

  font = abs(font);
  if (font >= 101 && font <= 129)
    font -= 100;
  else if (font >= 1 && font <= 32)
    font = map[font - 1];
  else
    font = 9;

  WC_to_NDC_rel(gkss->chup[0], gkss->chup[1], gkss->cntnr, ux, uy);
  seg_xform_rel(&ux, &uy);

  p->alpha = -atan2(ux, uy);
  angle = p->alpha * 180 / M_PI;
  if (angle < 0)
    angle += 360;
  p->angle = angle;

  scale = sqrt(gkss->chup[0] * gkss->chup[0] + gkss->chup[1] * gkss->chup[1]);
  ux = gkss->chup[0] / scale * gkss->chh;
  uy = gkss->chup[1] / scale * gkss->chh;
  WC_to_NDC_rel(ux, uy, gkss->cntnr, ux, uy);

  width = 0;
  height = sqrt(ux * ux + uy * uy);
  seg_xform_rel(&width, &height);

  height = sqrt(width * width + height * height);
  capheight = nint(height * (fabs(p->c) + 1));
  p->capheight = nint(capheight);

  size = nint(capheight / capheights[font - 1]);

  if (font > 13)
    font += 3;

  p->font = fonts[font - 1];
  fig_printf(p->stream, "%d ", p->font);
  fig_printf(p->stream, "%d ", size);
  fig_printf(p->stream, "%.4g 4 ", M_PI * p->angle / 180.0);
}

static
void text(double px, double py, int nchars, char *chars)
{
  int tx_font, tx_prec, tx_color;
  double x, y;

  tx_font = gkss->asf[6] ? gkss->txfont : predef_font[gkss->tindex - 1];
  tx_prec = gkss->asf[6] ? gkss->txprec : predef_prec[gkss->tindex - 1];
  tx_color = gkss->asf[9] ? gkss->txcoli : 1;

  p->color = tx_color;
  if (tx_prec == GKS_K_TEXT_PRECISION_STRING)
    {
      fig_printf(p->stream, "4 ");
      p->color = tx_color;
      set_font(tx_font);

      WC_to_NDC(px, py, gkss->cntnr, x, y);
      seg_xform(&x, &y);

      text_routine(x, y, nchars, chars);
    }
  else
    gks_emul_text(px, py, nchars, chars, line_routine, fill_routine);
}

static
void cellarray(double xmin, double xmax, double ymin, double ymax,
	       int dx, int dy, int dimx, int *colia, int true_color)
{
  double x1, y1, x2, y2;
  int ix1, ix2, iy1, iy2;
  int x, y, width, height;
  int red, green, blue;
  int i, j, ix, iy, ind, rgb;
  int swapx, swapy;
  png_byte bit_depth = 8;
  png_byte color_type = PNG_COLOR_TYPE_RGB;
  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep *row_pointers;
  char path[MAXPATHLEN];
  FILE *stream;

  WC_to_NDC(xmin, ymax, gkss->cntnr, x1, y1);
  seg_xform(&x1, &y1);
  NDC_to_DC(x1, y1, ix1, iy1);

  WC_to_NDC(xmax, ymin, gkss->cntnr, x2, y2);
  seg_xform(&x2, &y2);
  NDC_to_DC(x2, y2, ix2, iy2);

  width = abs(ix2 - ix1);
  height = abs(iy2 - iy1);
  if (width == 0 || height == 0) return;
  x = min(ix1, ix2);
  y = min(iy1, iy2);

  gks_filepath(path, p->path, "fig", p->page_counter, p->img_counter);
  if ((stream = fopen(path, "wb")) == NULL)
    {
      gks_perror("can't open image file");
      perror("open");
      return;
    }

  swapx = ix1 > ix2;
  swapy = iy1 < iy2;

  row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * height);
  for (j = 0; j < height; ++j)
    {
      row_pointers[j] = (png_byte *) malloc(width * 3);
    }
  for (j = 0; j < height; j++)
    {
      png_byte *row = row_pointers[j];
      iy = dy * j / height;
      if (swapy)
	iy = dy - 1 - iy;
      for (i = 0; i < width; i++)
	{
	  png_byte *ptr = &(row[i * 3]);
	  ix = dx * i / width;
	  if (swapx)
	    ix = dx - 1 - ix;
	  if (!true_color)
	    {
	      ind = colia[iy * dimx + ix];
	      sscanf(p->rgb[ind], "%02x%02x%02x", &red, &green, &blue);
	    }
	  else
	    {
	      rgb = colia[iy * dimx + ix];
	      red = (rgb & 0xff);
	      green = (rgb & 0xff00) >> 8;
	      blue = (rgb & 0xff0000) >> 16;
	    }
	  ptr[0] = red;
	  ptr[1] = green;
	  ptr[2] = blue;
	}
    }

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  info_ptr = png_create_info_struct(png_ptr);
  png_init_io(png_ptr, stream);
  png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, color_type,
	       PNG_FILTER_TYPE_BASE, PNG_COMPRESSION_TYPE_BASE,
	       PNG_FILTER_TYPE_BASE);
  png_write_info(png_ptr, info_ptr);
  png_write_image(png_ptr, row_pointers);
  png_write_end(png_ptr, NULL);
  for (j = 0; j < height; ++j)
    {
      free(row_pointers[j]);
    }
  free(row_pointers);
  fclose(stream);

  fig_printf(p->stream, "2 5 0 1 0 -1 50 -1 -1 0.000 0 0 -1 0 0 5\n");
  fig_printf(p->stream, "        0 %s\n", path);
  fig_printf(p->stream, "        %d %d %d %d %d %d %d %d %d %d\n", x * FACTOR,
	     y * FACTOR, (x + width) * FACTOR, y * FACTOR,\
	     (x + width) * FACTOR, (y + height) * FACTOR, x * FACTOR,\
	     (y + height) * FACTOR, x * FACTOR, y * FACTOR);
  p->img_counter++;
}

static
void write_page(void)
{
  char path[MAXPATHLEN];
  int fd;

  p->page_counter++;

  if (p->conid == 0)
    {
      gks_filepath(path, p->path, "fig", p->page_counter, 0);
      fd = gks_open_file(path, "w");
    }
  else
    fd = p->conid;

  if (fd >= 0)
    {
      gks_write_file(fd, p->stream->buffer, p->stream->length);
      if (fd != p->conid)
        gks_close_file(fd);

      p->stream->length = 0;
    }
  else
    {
      gks_perror("can't open FIG file");
      perror("open");
    }
}

void gks_figplugin(
  int fctid, int dx, int dy, int dimx, int *ia,
  int lr1, double *r1, int lr2, double *r2,
  int lc, char *chars, void **ptr)
{
  p = (ws_state_list *) * ptr;

  switch (fctid)
    {
/* open workstation */
    case 2:
      gkss = (gks_state_list_t *) * ptr;

      gks_init_core(gkss);

      p = (ws_state_list *) calloc(1, sizeof(ws_state_list));

      p->conid = ia[1];
      p->path = chars;

      p->height = 640;
      p->width = 640;
      p->window[0] = p->window[2] = 0.0;
      p->window[1] = p->window[3] = 1.0;
      p->viewport[0] = p->viewport[2] = 0;
      p->viewport[1] = (double) p->width * MWIDTH / WIDTH;
      p->viewport[3] = (double) p->height * MHEIGHT / HEIGHT;

      p->stream = fig_alloc_stream();

      p->max_points = MAX_POINTS;
      p->points = (FIG_point *) gks_malloc(p->max_points * sizeof(FIG_point));
      p->npoints = 0;

      p->empty = 1;
      p->page_counter = 0;
      p->offset = 0;
      p->img_counter = 1;

      set_xform();
      init_norm_xform();
      init_colors();

      *ptr = p;
      break;

/* close workstation */
    case 3:
      if (!p->empty)
	write_page();

      free(p->stream->buffer);
      free(p->points);
      free(p);
      break;

/* activate workstation */
    case 4:
      p->state = GKS_K_WS_ACTIVE;
      break;

/* deactivate workstation */
    case 5:
      p->state = GKS_K_WS_INACTIVE;
      break;

/* clear workstation */
    case 6:
      if (!p->empty)
	{
	  p->empty = 1;
	  write_page();
	}
      break;

/* update workstation */
    case 8:
      break;

/* polyline */
    case 12:
      if (p->state == GKS_K_WS_ACTIVE)
	{
	  if (p->empty)
	    {
	      fig_header();
	    }
	  polyline(ia[0], r1, r2);
	  p->empty = 0;
	}
      break;

/* polymarker */
    case 13:
      if (p->state == GKS_K_WS_ACTIVE)
	{
	  if (p->empty)
	    {
	      fig_header();
	    }
	  polymarker(ia[0], r1, r2);
	  p->empty = 0;
	}
      break;

/* text */
    case 14:
      if (p->state == GKS_K_WS_ACTIVE)
	{
	  if (p->empty)
	    {
	      fig_header();
	    }
	  text(r1[0], r2[0], strlen(chars), chars);
	  p->empty = 0;
	}
      break;

/* fill area */
    case 15:
      if (p->state == GKS_K_WS_ACTIVE)
	{
	  if (p->empty)
	    {
	      fig_header();
	    }
	  fillarea(ia[0], r1, r2);
	  p->empty = 0;
	}
      break;

/* cell array */
    case 16:
    case DRAW_IMAGE:
      if (p->state == GKS_K_WS_ACTIVE)
	{
	  int true_color = fctid == DRAW_IMAGE;

	  if (p->empty)
	    {
	      fig_header();
	    }
	  cellarray(r1[0], r1[1], r2[0], r2[1], dx, dy, dimx, ia, true_color);
	  p->empty = 0;
	}
      break;

/* set color representation */
    case 48:
      set_color_rep(ia[1], r1[0], r1[1], r1[2]);
      break;

    case 49:
/* set window */
      set_norm_xform(*ia, gkss->window[*ia], gkss->viewport[*ia]);
      break;

    case 50:
/* set viewport */
      set_norm_xform(*ia, gkss->window[*ia], gkss->viewport[*ia]);
      break;

/* set workstation window */
    case 54:
      p->window[0] = r1[0];
      p->window[1] = r1[1];
      p->window[2] = r2[0];
      p->window[3] = r2[1];

      set_xform();
      init_norm_xform();
      break;

/* set workstation viewport */
    case 55:
      p->viewport[0] = r1[0];
      p->viewport[1] = r1[1];
      p->viewport[2] = r2[0];
      p->viewport[3] = r2[1];

      resize_window();
      set_xform();
      init_norm_xform();
      break;

    default:
      ;
    }
}
