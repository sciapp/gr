#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <libpng16/png.h>

#if !defined(VMS) && !defined(_WIN32)
#include <unistd.h>
#endif

#include "gks.h"
#include "gkscore.h"

#define MEMORY_INCREMENT 32768

#define MAX_POINTS 2048
#define PATTERNS 120
#define HATCH_STYLE 108

#define MWIDTH  0.254
#define MHEIGHT 0.1905
#define WIDTH   1024
#define HEIGHT  768

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

DLLEXPORT void gks_pgfplugin(
    int fctid, int dx, int dy, int dimx, int *i_arr,
    int len_f_arr_1, double *f_arr_1, int len_f_arr_2, double *f_arr_2,
    int len_c_arr, char *c_arr, void **ptr);

#ifdef _WIN32
#ifdef __cplusplus
}
#endif
#endif

#define MAX_TNR 9

#define WC_to_NDC(xw, yw, tnr, xn, yn)          \
  xn = a[tnr] * (xw) + b[tnr];                  \
  yn = c[tnr] * (yw) + d[tnr]

#define WC_to_NDC_rel(xw, yw, tnr, xn, yn)      \
  xn = a[tnr] * (xw);                           \
  yn = c[tnr] * (yw)

#define NDC_to_DC(xn, yn, xd, yd)               \
  xd = (p->a * (xn) + p->b);                    \
  yd = (p->c * (yn) + p->d)

#define CharXform(xrel, yrel, x, y)                     \
  x = cos(p->alpha) * (xrel) - sin(p->alpha) * (yrel);  \
  y = sin(p->alpha) * (xrel) + cos(p->alpha) * (yrel);

#define nint(a) ((int)(a + 0.5))

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

static
gks_state_list_t *gkss;

static
double a[MAX_TNR], b[MAX_TNR], c[MAX_TNR], d[MAX_TNR];

typedef unsigned char Byte;
typedef unsigned long uLong;

typedef struct PGF_stream_t
{
  Byte *buffer;
  uLong size, length;
}
PGF_stream;

typedef struct PGF_point_t
{
  double x, y;
}
PGF_point;

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
  int family, capheight;
  int pattern, have_pattern[PATTERNS];
  PGF_stream *stream, *patternstream;
  PGF_point *points;
  int npoints, max_points;
  int empty, page_counter, offset;
  int cxl[MAX_TNR], cxr[MAX_TNR], cyb[MAX_TNR], cyt[MAX_TNR];
  int cx[MAX_TNR], cy[MAX_TNR], cwidth[MAX_TNR], cheight[MAX_TNR];
  int clip_index, path_index, path_counter;
  double rect[MAX_TNR][2][2];
  int scoped, png_counter, pattern_counter, usesymbols;
  int dashes[10];
}
ws_state_list;

static
ws_state_list *p;

static
const char *fonts[] = {
  "ptm", "phv", "pcr", "psy",
  "pbk", "pnc", "pag", "ppl"
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
void pgf_memcpy(PGF_stream *p, char *s, size_t n)
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
void pgf_printf(PGF_stream *p, const char *args, ...)
{
  va_list ap;
  char fmt[BUFSIZ], s[BUFSIZ];

  strcpy(fmt, args);

  va_start(ap, args);
  vsprintf(s, fmt, ap);
  va_end(ap);

  pgf_memcpy(p, s, strlen(s));
}

static
PGF_stream *pgf_alloc_stream(void)
{
  PGF_stream *p;

  p = (PGF_stream *) calloc(1, sizeof(PGF_stream));
  p->buffer = NULL;
  p->size = p->length = 0;

  return p;
}

static
void set_norm_xform(int tnr, double *wn, double *vp)
{
  a[tnr] = (vp[1] - vp[0]) / (wn[1] - wn[0]);
  b[tnr] = vp[0] - wn[0] * a[tnr];
  c[tnr] = (vp[3] - vp[2]) / (wn[3] - wn[2]);
  d[tnr] = vp[2] - wn[2] * c[tnr];

  NDC_to_DC(vp[0], vp[3], p->rect[tnr][0][0], p->rect[tnr][0][1]);
  NDC_to_DC(vp[1], vp[2], p->rect[tnr][1][0], p->rect[tnr][1][1]);
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
      sprintf(p->rgb[color], "%02X%02X%02X", (int) (red * 255),
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
void draw_marker(double xn, double yn, int mtype, double mscale)
{
  double x, y;
  double scale, xr, yr, x1, x2, y1, y2;
  int pc, op, r, i;

#include "marker.h"

  if (gkss->version > 4)
    mscale *= (p->width + p->height) * 0.001;
  r = (int) (3 * mscale);
  scale = 0.01 * mscale / 3.0;

  xr = r;
  yr = 0;
  seg_xform_rel(&xr, &yr);
  r = nint(sqrt(xr * xr + yr * yr));

  NDC_to_DC(xn, yn, x, y);

  pc = 0;
  mtype = (r > 0) ? mtype + marker_off : marker_off + 1;

  pgf_printf(p->stream, "\\begin{scope}[yscale=-1,yshift=-%f]\n", 2*y);

  do
    {
      op = marker[mtype][pc];
      switch (op)
        {
        case 1:         /* point */
          pgf_printf(p->stream, "\\draw (%f,%f)"
                     " rectangle (%f,%f);\n",
                     x, y, x + 1.0, y + 1.0);
          break;

        case 2:         /* line */
          x1 = scale * marker[mtype][pc + 1];
          y1 = scale * marker[mtype][pc + 2];
          seg_xform_rel(&x1, &y1);

          x2 = scale * marker[mtype][pc + 2 + 1];
          y2 = scale * marker[mtype][pc + 2 + 2];
          seg_xform_rel(&x2, &y2);

          pgf_printf(p->stream, "\\begin{scope}[yscale=-1, yshift=-%f]\n"
                     "\\draw (%f,%f) -- (%f,%f);\n"
                     "\\end{scope}\n",
                     2*y, x - x1, y - y1, x - x2, y - y2);

          pc += 4;
          break;

        case 3:         /* polyline */
        case 4:         /* filled polygon */
        case 5:         /* hollow polygon */
          xr = scale * marker[mtype][pc + 2];
          yr = -scale * marker[mtype][pc + 3];
          seg_xform_rel(&xr, &yr);

          if(op == 4)
            pgf_printf(p->stream, "\\fill[color=mycolor, line width=%dpt]",
                       p->linewidth);
          else
            pgf_printf(p->stream, "\\draw[color=mycolor, line width=%dpt]",
                       p->linewidth);

          pgf_printf(p->stream, " (%f,%f)",
                     x-xr, y-yr);

          for (i = 1; i < marker[mtype][pc + 1]; i++)
            {
              xr = scale * marker[mtype][pc + 2 + 2 * i];
              yr = -scale * marker[mtype][pc + 3 + 2 * i];
              seg_xform_rel(&xr, &yr);

              pgf_printf(p->stream, "  --  (%f,%f)",
                         x-xr, y-yr);
            }

          pgf_printf(p->stream, "  --  cycle;\n");

          pc += 1 + 2 * marker[mtype][pc + 1];
          break;

        case 6:         /* arc */
        case 7:         /* filled arc */
        case 8:         /* hollow arc */
          if(op == 7)
            pgf_printf(p->stream, "\\fill[color=mycolor, line width=%dpt]",
                       p->linewidth);
          else
            pgf_printf(p->stream, "\\draw[color=mycolor, line width=%dpt]",
                       p->linewidth);

          pgf_printf(p->stream, " (%f, %f) arc [start angle=%f, end angle=%f, "
             "radius=%d];\n", x + r, y, 0, 2 * M_PI, r);
          break;

        default:
          break;
        }
      pc++;
    }
  while (op != 0);

  pgf_printf(p->stream, "\\end{scope}\n");
}

static
void polymarker(int n, double *px, double *py)
{
  int mk_type, mk_color, ln_width, i;
  double mk_size, x, y;

  mk_type = gkss->asf[3] ? gkss->mtype : gkss->mindex;
  mk_size = gkss->asf[4] ? gkss->mszsc : 1;
  mk_color = gkss->asf[5] ? gkss->pmcoli : 1;

  ln_width = gkss->version > 4 ? max(1, nint(p->height/500.0)) : 1;
  p->linewidth = ln_width;

  pgf_printf(p->stream, "\\definecolor{mycolor}{HTML}{%s}\n",
             p->rgb[mk_color]);

  for (i = 0; i < n; i++)
    {
      WC_to_NDC(px[i], py[i], gkss->cntnr, x, y);
      seg_xform(&x, &y);

      draw_marker(x, y, mk_type, mk_size);
    }
}

static
void stroke(void)
{
  int i;

  pgf_printf(p->stream, "\\draw[color=mycolor, line width=%dpt] (%f,%f)",
             p->linewidth, p->points[0].x, p->points[0].y);

  for (i = 1; i < p->npoints; i++)
    {
      pgf_printf(p->stream, " -- (%f, %f)",
                 p->points[i].x, p->points[i].y);
    }

  p->npoints = 0;
  pgf_printf(p->stream, ";\n");
}

static
void move(double x, double y)
{
  if (p->npoints > 0)
    stroke();

  NDC_to_DC(x, y, p->points[p->npoints].x, p->points[p->npoints].y);
  p->npoints++;
}

static
void draw(double x, double y)
{
  NDC_to_DC(x, y, p->points[p->npoints].x, p->points[p->npoints].y);
  p->npoints++;
}

static
void line_routine(int n, double *px, double *py, int linetype, int tnr)
{
  double x, y, x0, y0, xi, yi;
  int i;

  WC_to_NDC(px[0], py[0], tnr, x, y);
  seg_xform(&x, &y);
  NDC_to_DC(x, y, x0, y0);

  pgf_printf(p->stream, "\\draw[color=mycolor, line width=%dpt] (%f,%f)",
             p->linewidth, x0, y0);

  for (i = 1; i < n; i++)
    {
      WC_to_NDC(px[i], py[i], tnr, x, y);
      seg_xform(&x, &y);
      NDC_to_DC(x, y, xi, yi);

      pgf_printf(p->stream, " -- (%f,%f)", xi, yi);
    }
  pgf_printf(p->stream, ";\n");
}

static
void fill_routine(int n, double *px, double *py, int tnr)
{
  int i, j, k;
  double x, y, ix, iy;
  int fl_inter, fl_style, size;
  int pattern[33];

  WC_to_NDC(px[0], py[0], tnr, x, y);
  seg_xform(&x, &y);
  NDC_to_DC(x, y, ix, iy);

  fl_inter = gkss->asf[10] ? gkss->ints : predef_ints[gkss->findex - 1];
  if (fl_inter == GKS_K_INTSTYLE_PATTERN || fl_inter == GKS_K_INTSTYLE_HATCH)
    {
      pgf_printf(p->stream, "\\fill[pattern=mypattern%d, pattern color=mycolor, "
                 "thickness=%dpt] (%f,%f)",
                 p->pattern_counter, p->linewidth, ix, iy);
    }
  else if (fl_inter == GKS_K_INTSTYLE_SOLID)
    {
      pgf_printf(p->stream, "\\fill[color=mycolor, line width=%dpt] (%f,%f)",
                 p->linewidth, ix, iy);
    }
  else
    {
      pgf_printf(p->stream, "\\draw[color=mycolor, line width=%dpt] (%f,%f)",
                 p->linewidth, ix, iy);
    }

  for (i = 1; i < n; i++)
    {
      WC_to_NDC(px[i], py[i], tnr, x, y);
      seg_xform(&x, &y);
      NDC_to_DC(x, y, ix, iy);
      pgf_printf(p->stream, " -- (%f,%f)", ix, iy);
    }

  pgf_printf(p->stream, " -- cycle;\n");

  if (fl_inter == GKS_K_INTSTYLE_PATTERN || fl_inter == GKS_K_INTSTYLE_HATCH)
    {
      fl_style = gkss->asf[11] ? gkss->styli : predef_styli[gkss->findex - 1];
      if (fl_inter == GKS_K_INTSTYLE_HATCH)
        {
          fl_style += HATCH_STYLE;
        }
      if (fl_style >= PATTERNS)
        {
          fl_style = 1;
        }
      gks_inq_pattern_array(fl_style, pattern);
      size = pattern[0];
      pgf_printf(p->patternstream,
                 "\\pgfdeclarepatternformonly[\\thickness]"
                 "{mypattern%d}\n{\\pgfpointorigin}{\\pgfpointxy{8}{%d}}"
                 "{\\pgfpointxy{8}{%d}}\n{\n"
                 "\\pgfsetlinewidth{\\thickness}\n",
                 p->pattern_counter, size, size);
      for (j = 1; j < size + 1; j++)
        {
          for (i = 0; i < 8; i++)
            {
              k = (1 << i) & pattern[j];
              if (!(k))
                {
                  pgf_printf(p->patternstream, "\\pgfpathrectangle"
                             "{\\pgfpointxy{%d}{%d}}{\\pgfpointxy{1}{-1}}\n",
                             (i + 7) % 8, size - (j - 1 + (size -1)) % size);
                }
            }
        }
      pgf_printf(p->patternstream, "\\pgfusepath{fill}\n}\n");
    }
  p->pattern_counter++;
}

static
void fillarea(int n, double *px, double *py)
{
  int fl_color;

  fl_color = gkss->asf[12] ? gkss->facoli : 1;
  p->linewidth = gkss->version > 4 ? max(nint(p->height/500.0), 1) : 1;

  pgf_printf(p->stream, "\\definecolor{mycolor}{HTML}{%s}\n",
             p->rgb[fl_color]);

  fill_routine(n, px, py, gkss->cntnr);
}

static
void polyline(int n, double *px, double *py)
{
  int ln_type, ln_color, i;
  double ln_width;
  int width;
  int dashes[10];

  if (n > p->max_points)
    {
      p->points = (PGF_point *) realloc(p->points, n * sizeof(PGF_point));
      p->max_points = n;
    }

  ln_type = gkss->asf[0] ? gkss->ltype : gkss->lindex;
  ln_width = gkss->asf[1] ? gkss->lwidth : 1;
  ln_color = gkss->asf[2] ? gkss->plcoli : 1;

  if (gkss->version > 4)
    width = nint(ln_width * (p->width + p->height) * 0.001);
  else
    width = nint(ln_width);
  if (width < 1)
    width = 0;

  p->linewidth = width;
  p->color = ln_color;
  gks_get_dash_list(ln_type, ln_width, dashes);
  memmove(p->dashes, dashes, sizeof(dashes));
  pgf_printf(p->stream, "\\definecolor{mycolor}{HTML}{%s}\n",
             p->rgb[ln_color]);

  pgf_printf(p->stream, "\\begin{scope}[dash pattern=");
  for (i = 1; i <= dashes[0]; i++)
    {
      if (i % 2 == 1)
        pgf_printf(p->stream, " on %dpt", dashes[i]);
      else
        pgf_printf(p->stream, " off %dpt", dashes[i]);
    }
  pgf_printf(p->stream, "]\n");

  gks_set_dev_xform(gkss, p->window, p->viewport);
  gks_emul_polyline(n, px, py, ln_type, gkss->cntnr, move, draw);

  if (p->npoints > 0)
    stroke();

  pgf_printf(p->stream, "\\end{scope}\n");
}

static
void text_routine(double x, double y, int nchars, char *chars)
{
  int width, height, ch;
  double xrel, yrel, ax, ay;
  double xstart, ystart;

  NDC_to_DC(x, y, xstart, ystart);

  width = 0;
  height = p->capheight;

  xrel = width * xfac[gkss->txal[0]];
  yrel = height * yfac[gkss->txal[1]];
  CharXform(xrel, yrel, ax, ay);
  xstart += ax;
  ystart -= ay;

  pgf_printf(p->stream, "\\begin{scope}[yscale=-1,yshift=-%f]\n"
             "\\draw[mycolor] (%f,%f) node[align=",
             (ystart*2), xstart, ystart);
  if (gkss->txal[0] == GKS_K_TEXT_HALIGN_RIGHT)
    pgf_printf(p->stream, "right");
  else if (gkss->txal[0] == GKS_K_TEXT_HALIGN_LEFT)
    pgf_printf(p->stream, "left");
  else
    pgf_printf(p->stream, "center");

  if (p->angle != 0)
    pgf_printf(p->stream, ", rotate=%f", p->angle);

  pgf_printf(p->stream, "]{");

  if (p->usesymbols)
    pgf_printf(p->stream, "\\Pifont{psy} ");

  for (int i = 0; i < nchars; ++i)
    {
      ch = chars[i];
      switch (ch)
        {
        case '&':
          pgf_printf(p->stream, "\\&");
          break;
        case '%':
          pgf_printf(p->stream, "\\%%");
          break;
        case '$':
          pgf_printf(p->stream, "\\$");
          break;
        case '#':
          pgf_printf(p->stream, "\\#");
          break;
        case '_':
          pgf_printf(p->stream, "\\_");
          break;
        case '{':
          pgf_printf(p->stream, "\\{");
          break;
        case '}':
          pgf_printf(p->stream, "\\}");
          break;
        case '~':
          pgf_printf(p->stream, "\\textasciitilde");
          break;
        case '^':
          pgf_printf(p->stream, "\\textasciicircum");
          break;
        case '\\':
          pgf_printf(p->stream, "\\textbackslash");
          break;
        default:
          pgf_printf(p->stream, "%c", ch);
          break;
        }
    }
  pgf_printf(p->stream, "};\n\\end{scope}\n");
}

static
void set_font(int font)
{
  double scale, ux, uy, angle;
  int size;
  double width, height, capheight;
  int bold, italic;

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

  size = nint(capheight / capheights[font-1]);
  if (font > 13)
    font += 3;
  p->family = (font - 1) / 4;
  bold = (font % 4 == 1 || font % 4 == 2) ? 0 : 1;
  italic = (font % 4 == 2 || font % 4 == 0);

  if (p->family != 3)
    {
      pgf_printf(p->stream, "\\fontfamily{%s}\\fontsize{%d}{%d}",
                 fonts[p->family], size, nint(size*1.2));
      if (bold && italic)
        pgf_printf(p->stream, "\\fontshape{it}\\fontseries{b}");
      else if (italic)
        pgf_printf(p->stream, "\\fontshape{it}");
      else if (bold)
        pgf_printf(p->stream, "\\fontseries{b}");
      pgf_printf(p->stream, "\\selectfont\n");
      p->usesymbols = 0;
    }
  else
    p->usesymbols = 1;
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

  pgf_printf(p->stream, "\\definecolor{mycolor}{HTML}{%s}\n",
             p->rgb[tx_color]);

  if (tx_prec == GKS_K_TEXT_PRECISION_STRING)
    {
      set_font(tx_font);

      WC_to_NDC(px, py, gkss->cntnr, x, y);
      seg_xform(&x, &y);

      text_routine(x, y, nchars, chars);
    }
  else
    {
      gks_emul_text(px, py, nchars, chars, line_routine, fill_routine);
    }
}

static
void cellarray(double xmin, double xmax, double ymin, double ymax,
               int dx, int dy, int dimx, int *colia, int true_color)
{
  double x1, y1, x2, y2, x, y;
  int ix1, ix2, iy1, iy2;
  int width, height;
  int red, green, blue;
  int i, j, ix, iy, ind, rgb;
  int swapx, swapy;
  png_byte bit_depth = 8;
  png_byte color_type = PNG_COLOR_TYPE_RGB;
  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep *row_pointers;
  FILE *stream;
  char filename[MAXPATHLEN];

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

  gks_filepath(filename, p->path, "png", p->page_counter, p->png_counter);
  if ((stream = fopen(filename, "wb")) == NULL)
    {
      gks_perror("can't open temporary file");
      perror("open");
      return;
    }

  swapx = ix1 > ix2;
  swapy = iy1 < iy2;

  row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * height);
  for (j = 0; j < height; j++)
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
  for (j = 0; j < height; j++)
    {
      free(row_pointers[j]);
    }
  free(row_pointers);
  fclose(stream);

  pgf_printf(p->stream, "\\begin{scope}[yscale=-1, yshift=-%f]\n"
             "\\node[anchor=north west] (%s) at (%f,%f)"
             " {\\includegraphics{%s}};\n\\end{scope}\n",
             2*y, filename, x, y, filename);
  p->png_counter++;
}

static
void set_clip_rect(int tnr)
{
  if (p->scoped)
    {
      pgf_printf(p->stream, "\\end{scope}\n");
      p->scoped = 0;
    }

  if (gkss->clip == GKS_K_CLIP)
    {
      if (p->scoped)
        pgf_printf(p->stream, "\\end{scope}\n");
      pgf_printf(p->stream, "\\begin{scope}\n"
                 "\\clip (%f,%f) rectangle (%f,%f);\n",
                 p->rect[tnr][0][0], p->rect[tnr][0][1],
                 p->rect[tnr][0][0] + p->rect[tnr][1][0],
                 p->rect[tnr][0][1] + p->rect[tnr][1][1]);
      p->scoped = 1;
    }
}

static
void set_clipping(int idx)
{
  gkss->clip = idx;
  set_clip_rect(gkss->cntnr);
}

static
void write_page(void)
{
  char filename[MAXPATHLEN];
  char buf[256];
  int fd;

  p->page_counter++;

  if (p->conid == 0)
    {
      gks_filepath(filename, p->path, "tex", p->page_counter, 0);
      fd = gks_open_file(filename, "w");
    }
  else
    fd = p->conid;

  if (fd >= 0)
    {
      sprintf(buf, "\\documentclass[tikz]{standalone}\n"
              "\\usetikzlibrary{patterns}\n"
              "\\usepackage{pifont}\n\n"
              "\\begin{document}\n\\pagenumbering{gobble}\n\\centering\n"
              "\\pgfsetxvec{\\pgfpoint{1pt}{0pt}}\n"
              "\\pgfsetyvec{\\pgfpoint{0pt}{-1pt}}\n");
      gks_write_file(fd, buf, strlen(buf));
      sprintf(buf, "\\newdimen\\thickness\n\\tikzset{\n"
              "thickness/.code={\\thickness=#1},\n"
              "thickness=1pt\n}\n");
      gks_write_file(fd, buf, strlen(buf));
      gks_write_file(fd, p->patternstream->buffer, p->patternstream->length);
      sprintf(buf, "\\begin{tikzpicture}[yscale=-1, "
              "every node/.style={inner sep=0pt, outer sep=1pt, anchor=base west}]\n"
              "\\pgfsetyvec{\\pgfpoint{0pt}{1pt}}\n");
      gks_write_file(fd, buf, strlen(buf));
      gks_write_file(fd, p->stream->buffer, p->stream->length);
      if (p->scoped)
        sprintf(buf, "\\end{scope}\n\\end{tikzpicture}\n\\end{document}\n");
      else
        sprintf(buf, "\\end{tikzpicture}\n\\end{document}\n");
      gks_write_file(fd, buf, strlen(buf));
      if (fd != p->conid)
        gks_close_file(fd);

      p->stream->length = 0;
    }
  else
    {
      gks_perror("can't open TEX file");
      perror("open");
    }
}

static
void select_xform(int tnr)
{
  gkss->cntnr = tnr;
  set_clip_rect(tnr);
}

void set_window(int tnr, double xmin, double xmax, double ymin, double ymax)
{
  gkss->window[tnr][0] = xmin;
  gkss->window[tnr][1] = xmax;
  gkss->window[tnr][2] = ymin;
  gkss->window[tnr][3] = ymax;

  set_xform();
  set_norm_xform(tnr, gkss->window[tnr], gkss->viewport[tnr]);
  gks_set_norm_xform(tnr, gkss->window[tnr], gkss->viewport[tnr]);
}

static
void set_viewport(int tnr, double xmin, double xmax, double ymin, double ymax)
{
  gkss->viewport[tnr][0] = xmin;
  gkss->viewport[tnr][1] = xmax;
  gkss->viewport[tnr][2] = ymin;
  gkss->viewport[tnr][3] = ymax;

  set_norm_xform(tnr, gkss->window[tnr], gkss->viewport[tnr]);
  gks_set_norm_xform(tnr, gkss->window[tnr], gkss->viewport[tnr]);
  if (tnr == gkss->cntnr) {
    set_clip_rect(tnr);
  }
}

void gks_pgfplugin(
    int fctid, int dx, int dy, int dimx, int *ia,
    int lr1, double *r1, int lr2, double *r2,
    int lc, char *chars, void **ptr)
{
  int i;

  p = (ws_state_list *) *ptr;

  switch (fctid)
    {
    case 2:
      /* open workstation */
      gkss = (gks_state_list_t *) * ptr;

      gks_init_core(gkss);

      p = (ws_state_list *) calloc(1, sizeof(ws_state_list));

      p->conid = ia[1];
      p->path = chars;

      p->height = 500;
      p->width = 500;
      p->window[0] = p->window[2] = 0.0;
      p->window[1] = p->window[3] = 1.0;
      p->viewport[0] = p->viewport[2] = 0;
      p->viewport[1] = (double) p->width * MWIDTH / WIDTH;
      p->viewport[3] = (double) p->height * MHEIGHT / HEIGHT;

      p->stream = pgf_alloc_stream();
      p->patternstream = pgf_alloc_stream();

      p->max_points = MAX_POINTS;
      p->points = (PGF_point *) gks_malloc(p->max_points * sizeof(PGF_point));
      p->npoints = 0;

      p->empty = 1;
      p->page_counter = 0;
      p->offset = 0;

      p->png_counter = 0;
      p->pattern_counter = 0;

      set_xform();
      init_norm_xform();
      init_colors();

      for (i = 0; i < PATTERNS; i++)
        p->have_pattern[i] = 0;

      *ptr = p;
      break;

    case 3:
      /* close workstation */
      if (!p->empty)
        write_page();

      free(p->stream->buffer);
      free(p->patternstream->buffer);
      free(p->points);
      free(p);
      break;

    case 4:
      /* activate workstation */
      p->state = GKS_K_WS_ACTIVE;
      break;

    case 5:
      /* deactivate workstation */
      p->state = GKS_K_WS_INACTIVE;
      break;

    case 6:
      /* clear workstation */
      if (!p->empty)
        {
          p->empty = 1;
          write_page();
        }
      break;

    case 8:
      /* update workstation */
      break;

    case 12:
      /* polyline */
      if (p->state == GKS_K_WS_ACTIVE)
        {
          polyline(ia[0], r1, r2);
          p->empty = 0;
        }
      break;

    case 13:
      /* polymarker */
      if (p->state == GKS_K_WS_ACTIVE)
        {
          polymarker(ia[0], r1, r2);
          p->empty = 0;
        }
      break;

    case 14:
      /* text */
      if (p->state == GKS_K_WS_ACTIVE)
        {
          text(r1[0], r2[0], strlen(chars), chars);
          p->empty = 0;
        }
      break;

    case 15:
      /* fill area */
      if (p->state == GKS_K_WS_ACTIVE)
        {
          fillarea(ia[0], r1, r2);
          p->empty = 0;
        }
      break;

    case 16:
    case DRAW_IMAGE:
      /* cell array */
      if (p->state == GKS_K_WS_ACTIVE)
        {
          int true_color = fctid == DRAW_IMAGE;

          cellarray(r1[0], r1[1], r2[0], r2[1], dx, dy, dimx, ia, true_color);
          p->empty = 0;
        }
      break;

    case 48:
      /* set color representation */
      if (p->state == GKS_K_WS_ACTIVE)
        {
          set_color_rep(ia[1], r1[0], r1[1], r1[2]);
        }
      break;

    case 49:
      /* set window */
      if (p->state == GKS_K_WS_ACTIVE)
        {
          set_window(gkss->cntnr, r1[0], r1[1], r2[0], r2[1]);
        }
      break;

    case 50:
      /* set viewport */
      if (p->state == GKS_K_WS_ACTIVE)
        {
          set_viewport(gkss->cntnr, r1[0], r1[1], r2[0], r2[1]);
        }
      break;

    case 52:
      /* select normalization transformation */
      if (p->state == GKS_K_WS_ACTIVE)
        {
          select_xform(ia[0]);
        }
      break;

    case 53:
      /* set clipping inidicator */
      if (p->state == GKS_K_WS_ACTIVE)
        {
          set_clipping(ia[0]);
        }
      break;

    default:
      ;
    }
}
