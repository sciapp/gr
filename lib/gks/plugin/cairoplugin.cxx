#ifndef NO_CAIRO

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#ifdef _MSC_VER
typedef __int64 int64_t;
#else
#include <stdint.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>

#include <cairo/cairo.h>
#ifndef NO_X11
#include <cairo/cairo-xlib.h>
#endif

#endif

#ifndef NO_X11

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>

#include <pthread.h>
#include <signal.h>
#endif

#if !defined(VMS) && !defined(_WIN32)
#include <unistd.h>
#endif

#include "gks.h"
#include "gkscore.h"

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

DLLEXPORT void gks_cairoplugin(
  int fctid, int dx, int dy, int dimx, int *i_arr,
  int len_f_arr_1, double *f_arr_1, int len_f_arr_2, double *f_arr_2,
  int len_c_arr, char *c_arr, void **ptr);

#ifdef _WIN32
#ifdef __cplusplus
}
#endif
#endif

#ifndef NO_CAIRO

#define MAX_POINTS 2048
#define PATTERNS 120
#define HATCH_STYLE 108

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
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

typedef struct cairo_point_t
{
  double x, y;
}
cairo_point;

typedef struct ws_state_list_t
{
  int conid, state, wtype;
  double mw, mh;
  int w, h;
  char *path;
  double a, b, c, d;
  double window[4], viewport[4];
  double rgb[MAX_COLOR][3];
  double transparency;
  int width, height;
  int color, linewidth;
  double alpha, angle;
  int family, capheight;
  cairo_surface_t *surface;
  cairo_t *cr;
  cairo_point *points;
#ifndef NO_X11
  Display *dpy;
  Window win;
  pthread_t thread;
  pthread_mutex_t mutex;
  int run, done;
  Atom wmDeleteMessage;
  pthread_t master_thread;
#endif
  int npoints, max_points;
  int empty, page_counter;
  double rect[MAX_TNR][2][2];
  unsigned char *patterns;
  int pattern_counter, use_symbols;
  double dashes[10];
}
ws_state_list;

static
ws_state_list *p;

static
int idle = 0;

static
const char *fonts[] = {
  "Times New Roman", "Arial", "Courier", "Symbol",
  "Bookman Old Style", "Century Schoolbook", "Century Gothic", "Book Antiqua"
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
void resize(int width, int height)
{
  p->width = width;
  p->height = height;
  p->window[0] = p->window[2] = 0.0;
  p->window[1] = p->window[3] = 1.0;
  p->viewport[0] = p->viewport[2] = 0;
  p->viewport[1] = (double) p->width * p->mw / p->w;
  p->viewport[3] = (double) p->height * p->mh / p->h;

  set_xform();
  init_norm_xform();
}

static
void set_color_rep(int color, double red, double green, double blue)
{
  if (color >= 0 && color < MAX_COLOR)
    {
      p->rgb[color][0] = red;
      p->rgb[color][1] = green;
      p->rgb[color][2] = blue;
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
void set_color(int index)
{
  cairo_set_source_rgba(p->cr, p->rgb[index][0], p->rgb[index][1],
                        p->rgb[index][2], p->transparency);
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

  cairo_set_dash(p->cr, p->dashes, 0, 0);

  do
    {
      op = marker[mtype][pc];
      switch (op)
        {
        case 1:         /* point */
          cairo_rectangle(p->cr, x, y, 1.0, 1.0);
          cairo_stroke(p->cr);
          break;

        case 2:         /* line */
          x1 = scale * marker[mtype][pc + 1];
          y1 = scale * marker[mtype][pc + 2];
          seg_xform_rel(&x1, &y1);

          x2 = scale * marker[mtype][pc + 2 + 1];
          y2 = scale * marker[mtype][pc + 2 + 2];
          seg_xform_rel(&x2, &y2);

          cairo_move_to(p->cr, x - x1, y - y1);
          cairo_line_to(p->cr, x - x2, y - y2);
          cairo_stroke(p->cr);
          pc += 4;
          break;

        case 3:         /* polyline */
        case 4:         /* filled polygon */
        case 5:         /* hollow polygon */
          xr = scale * marker[mtype][pc + 2];
          yr = -scale * marker[mtype][pc + 3];
          seg_xform_rel(&xr, &yr);

          cairo_move_to(p->cr, x-xr, y+yr);

          for (i = 1; i < marker[mtype][pc + 1]; i++)
            {
              xr = scale * marker[mtype][pc + 2 + 2 * i];
              yr = -scale * marker[mtype][pc + 3 + 2 * i];
              seg_xform_rel(&xr, &yr);

              cairo_line_to(p->cr, x-xr, y+yr);
            }

          cairo_close_path(p->cr);

          if (op == 4)
            cairo_fill(p->cr);
          else
            cairo_stroke(p->cr);

          pc += 1 + 2 * marker[mtype][pc + 1];
          break;

        case 6:         /* arc */
        case 7:         /* filled arc */
        case 8:         /* hollow arc */
          cairo_arc(p->cr, x, y, r * 1.0, 0, 2 * M_PI);

          if (op == 7)
            cairo_fill(p->cr);
          else
            cairo_stroke(p->cr);
          break;

        default:
          break;
        }
      pc++;
    }
  while (op != 0);
}

static
void polymarker(int n, double *px, double *py)
{
  int mk_type, mk_color, ln_width, i;
  double mk_size, x, y;

  mk_type = gkss->asf[3] ? gkss->mtype : gkss->mindex;
  mk_size = gkss->asf[4] ? gkss->mszsc : 1;
  mk_color = gkss->asf[5] ? gkss->pmcoli : 1;

  ln_width = gkss->version > 4 ? max(1, nint((p->width + p->height) * 0.001)) : 1;
  p->linewidth = ln_width;

  cairo_set_line_width(p->cr, ln_width);
  set_color(mk_color);

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

  cairo_move_to(p->cr, p->points[0].x, p->points[0].y);
  for (i = 1; i < p->npoints; i++)
    {
      cairo_line_to(p->cr, p->points[i].x, p->points[i].y);
    }
  cairo_stroke(p->cr);

  p->npoints = 0;
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

  cairo_set_line_width(p->cr, p->linewidth);

  cairo_move_to(p->cr, x0, y0);

  for (i = 1; i < n; i++)
    {
      WC_to_NDC(px[i], py[i], tnr, x, y);
      seg_xform(&x, &y);
      NDC_to_DC(x, y, xi, yi);

      cairo_line_to(p->cr, xi, yi);
    }
  cairo_stroke(p->cr);
}

static
void fill_routine(int n, double *px, double *py, int tnr)
{
  int i, j, k;
  double x, y, ix, iy;
  int fl_inter, fl_style, size;
  int gks_pattern[33];
  cairo_format_t format = CAIRO_FORMAT_A8;
  int stride = cairo_format_stride_for_width(format, 8);
  cairo_pattern_t *pattern;
  cairo_surface_t *image;

  WC_to_NDC(px[0], py[0], tnr, x, y);
  seg_xform(&x, &y);
  NDC_to_DC(x, y, ix, iy);

  cairo_set_dash(p->cr, p->dashes, 0, 0);

  cairo_move_to(p->cr, ix, iy);

  for (i = 1; i < n; i++)
    {
      WC_to_NDC(px[i], py[i], tnr, x, y);
      seg_xform(&x, &y);
      NDC_to_DC(x, y, ix, iy);
      cairo_line_to(p->cr, ix, iy);
    }

  cairo_close_path(p->cr);

  fl_inter = gkss->asf[10] ? gkss->ints : predef_ints[gkss->findex - 1];

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
      gks_inq_pattern_array(fl_style, gks_pattern);
      size = gks_pattern[0];

      p->patterns = (unsigned char*) gks_realloc(p->patterns,
                                                 size * 8 * sizeof(unsigned char));
      memset(p->patterns, 0, size * 8 * sizeof(unsigned char));

      for (j = 1; j < size + 1; j++)
        {
          for (i = 0; i < 8; i++)
            {
              k = (1 << i) & gks_pattern[j];
              if (!(k))
                {
                  p->patterns[((i + 7) % 8) + ((j - 1 + (size-1)) % size) * 8] =
                    (int) 255 * p->transparency;
                }
            }
        }

      image = cairo_image_surface_create_for_data(p->patterns, format, 8, size,
                                                  stride);

      pattern = cairo_pattern_create_for_surface(image);
      cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);

      cairo_set_source(p->cr, pattern);
    }

  if (fl_inter == GKS_K_INTSTYLE_PATTERN || fl_inter == GKS_K_INTSTYLE_HATCH
      || fl_inter == GKS_K_INTSTYLE_SOLID)
    {
      cairo_fill(p->cr);
    }
  else
    {
      cairo_stroke(p->cr);
    }
}

static
void fillarea(int n, double *px, double *py)
{
  int fl_color;

  fl_color = gkss->asf[12] ? gkss->facoli : 1;
  p->linewidth = gkss->version > 4 ? max(nint((p->width + p->height) * 0.001), 1) : 1;

  set_color(fl_color);

  fill_routine(n, px, py, gkss->cntnr);
}

static
void polyline(int n, double *px, double *py)
{
  int ln_type, ln_color, i;
  double ln_width;
  int width;
  int gks_dashes[10];

  if (n > p->max_points)
    {
      p->points = (cairo_point *) gks_realloc(p->points, n * sizeof(cairo_point));
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
    width = 1;

  p->linewidth = width;
  p->color = ln_color;
  cairo_set_line_width(p->cr, width);
  set_color(ln_color);

  gks_get_dash_list(ln_type, ln_width, gks_dashes);
  for (i = 0; i < gks_dashes[0]; i++)
    p->dashes[i] = gks_dashes[i + 1];
  cairo_set_dash(p->cr, p->dashes, gks_dashes[0], 0);

  gks_set_dev_xform(gkss, p->window, p->viewport);
  gks_emul_polyline(n, px, py, ln_type, gkss->cntnr, move, draw);

  if (p->npoints > 0)
    stroke();
}

static
void symbol_text(int nchars, char *chars)
{
  int i, ic;
  char *temp = (char *) gks_malloc(4);

  for (i = 0; i < nchars; i++) {
    ic = chars[i];
    if ((ic >= 'A'  && ic <= 'Z') || (ic >= 'a' && ic < 'p'))
      ic += 52816;
    else if (ic >= 'p' && ic <= 'z')
      ic += 53008;
    sprintf(temp, "%c%c", (ic>>8), (ic & 255));
    cairo_show_text(p->cr, temp);
  }
  free(temp);
}

static
void text_routine(double x, double y, int nchars, char *chars)
{
  double xrel, yrel, ax, ay;
  double xstart, ystart;
  cairo_text_extents_t extents;
  unsigned char ch, *s, *buf = NULL;
  int i;
  char *str;

  if (!p->use_symbols)
    {
      /* convert latin1 to utf8 */
      s = buf = (unsigned char *) gks_malloc(2 * nchars + 1);
      for (i = 0; i < nchars; i++)
        {
          ch = (unsigned char) chars[i];
          if (ch < 0x80)
            *s++ = chars[i];
          else
            {
              *s++ = 0xc0 | (ch & 0xc0) >> 6;
              *s++ = 0x80 | (ch & 0x3f);
            }
        }
      *s++ = '\0';
      chars = (char *) buf;
      nchars = strlen(chars);
    }

  /* Ugly workaround to avoid Cairo crashes when getting the text extent
     for a string that contains only a single character */
  str = (char *) gks_malloc(nchars + 3);
  strncpy(str, chars, nchars);
  strcat(str, "  ");

  cairo_text_extents(p->cr, str, &extents);
  free(str);

  NDC_to_DC(x, y, xstart, ystart);

  xrel = (extents.width + extents.x_bearing) * xfac[gkss->txal[0]];
  yrel = p->capheight * yfac[gkss->txal[1]];
  CharXform(xrel, yrel, ax, ay);
  xstart += ax;
  ystart -= ay;

  if (p->angle != 0) {
    cairo_translate(p->cr, xstart, ystart);
    cairo_rotate(p->cr, -p->angle*M_PI/180);
    cairo_move_to(p->cr, 0, 0);
    if (p->use_symbols)
      symbol_text(nchars, chars);
    else
      cairo_show_text(p->cr, chars);
    cairo_rotate(p->cr, p->angle*M_PI/180);
    cairo_translate(p->cr, -xstart, -ystart);
  }
  else {
    cairo_move_to(p->cr, xstart, ystart);
    if (p->use_symbols)
      symbol_text(nchars, chars);
    else
      cairo_show_text(p->cr, chars);
  }

  if (!p->use_symbols)
    free(buf);
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
      if (italic && bold)
        cairo_select_font_face(p->cr, fonts[p->family],
                               CAIRO_FONT_SLANT_ITALIC,
                               CAIRO_FONT_WEIGHT_BOLD);
      else if (bold)
        cairo_select_font_face(p->cr, fonts[p->family],
                               CAIRO_FONT_SLANT_NORMAL,
                               CAIRO_FONT_WEIGHT_BOLD);
      else if (italic)
        cairo_select_font_face(p->cr, fonts[p->family],
                               CAIRO_FONT_SLANT_ITALIC,
                               CAIRO_FONT_WEIGHT_NORMAL);
      else
        cairo_select_font_face(p->cr, fonts[p->family],
                               CAIRO_FONT_SLANT_NORMAL,
                               CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size(p->cr, size);
      p->use_symbols = 0;
    }
  else
    p->use_symbols = 1;
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

  cairo_set_dash(p->cr, p->dashes, 0, 0);
  set_color(tx_color);

  if (tx_prec == GKS_K_TEXT_PRECISION_STRING)
    {
      set_font(tx_font);

      WC_to_NDC(px, py, gkss->cntnr, x, y);
      seg_xform(&x, &y);

      text_routine(x, y, nchars, chars);
    }
  else
    {
      p->linewidth = 1;
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
  int red, green, blue, alpha;
  int i, j, ix, iy, ind, rgb;
  int swapx, swapy;
  int stride;
  unsigned char *data;
  cairo_surface_t *image;

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

  swapx = ix1 > ix2;
  swapy = iy1 < iy2;

  stride = cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, width);
  data = (unsigned char *) gks_malloc(stride * height);

  for (j = 0; j < height; j++) {
    iy = dy * j / height;
    if (swapy) {
      iy = dy - 1 - iy;
    }
    for (i = 0; i < width; i++) {
      ix = dx * i / width;
      if (swapx) {
        ix = dx - 1 - ix;
      }
      if (!true_color) {
        ind = colia[iy * dimx + ix];
        red = (int) 255 * p->rgb[ind][0];
        green = (int) 255 * p->rgb[ind][1];
        blue = (int) 255 * p->rgb[ind][2];
        alpha = (int) 255 * p->transparency;
      } else {
        rgb = colia[iy * dimx + ix];
        red = (rgb & 0xff);
        green = (rgb & 0xff00) >> 8;
        blue = (rgb & 0xff0000) >> 16;
        alpha = (rgb & 0xff000000) >> 24;
      }
      // ARGB32 format requires pre-multiplied alpha
      red = red * alpha / 255;
      green = green * alpha / 255;
      blue = blue * alpha / 255;
      data[j * stride + i * 4 + 0] = blue;
      data[j * stride + i * 4 + 1] = green;
      data[j * stride + i * 4 + 2] = red;
      data[j * stride + i * 4 + 3] = alpha;
    }
  }

  image = cairo_image_surface_create_for_data(data, CAIRO_FORMAT_ARGB32,
                                              width, height, stride);
  cairo_set_source_surface(p->cr, image, x, y);
  cairo_paint(p->cr);
  cairo_surface_destroy(image);
  free(data);
}

static
void set_clip_rect(int tnr)
{
  cairo_reset_clip(p->cr);

  if (gkss->clip == GKS_K_CLIP)
    {
      cairo_rectangle(p->cr,
                      p->rect[tnr][0][0], p->rect[tnr][0][1],
                      p->rect[tnr][1][0] - p->rect[tnr][0][0],
                      p->rect[tnr][1][1] - p->rect[tnr][0][1]);
      cairo_clip(p->cr);
    }
}

static
void set_clipping(int index)
{
  gkss->clip = index;
  set_clip_rect(gkss->cntnr);
}

static
void lock(void)
{
#ifndef NO_X11
  if (p->wtype == 141)
    pthread_mutex_lock(&p->mutex);
#endif
}

static
void unlock(void)
{
#ifndef NO_X11
  if (p->wtype == 141)
    pthread_mutex_unlock(&p->mutex);
#endif
}

#ifndef NO_X11

static
void *event_loop(void *arg)
{
  ws_state_list *p = (ws_state_list *) arg;
  XEvent event;

  p->run = 1;
  while (p->run)
    {
      usleep(10000);

      if (idle && p->run)
        {
          if (pthread_mutex_trylock(&p->mutex) == 0)
            {
              while (XPending(p->dpy))
                {
                  XNextEvent(p->dpy, &event);
                  if (event.type == ClientMessage)
                    {
                      if ((Atom) event.xclient.data.l[0] == p->wmDeleteMessage)
                        {
                          pthread_kill(p->master_thread, SIGTERM);
                          p->run = 0;
                        }
                    }
                  else if (event.type == ConfigureNotify)
                    {
                      cairo_xlib_surface_set_size(p->surface,
                                                  event.xconfigure.width,
                                                  event.xconfigure.height);
                    }
                }
              pthread_mutex_unlock(&p->mutex);
            }
        }
    }
  p->done = 1;

  pthread_exit(0);
}

static
void create_window(void)
{
  int screen;
  pthread_attr_t attr;

  if (!(p->dpy = XOpenDisplay(NULL))) {
    gks_perror("Could not open display");
    exit(1);
  }

  screen = DefaultScreen(p->dpy);
  p->win = XCreateSimpleWindow(p->dpy, RootWindow(p->dpy, screen),
                               1, 1, p->width, p->height, 0,
                               BlackPixel(p->dpy, screen), WhitePixel(p->dpy, screen));

  p->master_thread = pthread_self();
  p->wmDeleteMessage = XInternAtom(p->dpy, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(p->dpy, p->win, &p->wmDeleteMessage, 1);

  XStoreName(p->dpy, p->win, "GKS");
  XSelectInput(p->dpy, p->win, StructureNotifyMask | ExposureMask);
  XMapWindow(p->dpy, p->win);

  pthread_mutex_init(&p->mutex, NULL);
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  if (pthread_create(&p->thread, &attr, event_loop, (void *) p))
    perror("pthread_create");
  pthread_attr_destroy(&attr);
}

#endif

static
void open_page(void)
{
  char *env;

  if (p->wtype == 141)
    {
#ifndef NO_X11
      create_window();
      p->surface = cairo_xlib_surface_create(p->dpy, p->win,
                                             DefaultVisual(p->dpy, 0),
                                             p->width, p->height);
#else
      gks_perror("Cairo X11 support not compiled in");
      exit(1);
#endif
    }
  else if (p->wtype == 140 || p->wtype == 150)
    {
      p->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                              p->width, p->height);
    }
  if (p->wtype == 142)
    {
      env = (char *) gks_getenv("GKS_CONID");
      if (!env)
        env = (char *) gks_getenv("GKSconid");

      if (env != NULL)
        sscanf(env, "%lu", (unsigned long *) &p->cr);
      else
        {
          gks_perror("can't obtain Gtk drawable");
          exit(1);
        }
    }
  else
    p->cr = cairo_create(p->surface);
}

static
void close_page(void)
{
  if (p->wtype != 142)
    {
      cairo_destroy(p->cr);
      cairo_surface_destroy(p->surface);
    }
#ifndef NO_X11
  if (p->wtype == 141)
    {
      if (p->run)
        {
          /* pthread_join didn't work for some reason */
          p->done = 0;
          while (!p->done)
            {
              p->run = 0;
              usleep(10000);
            }
        }
      XDestroyWindow(p->dpy, p->win);
      XCloseDisplay(p->dpy);
    }
#endif
}

#define ON_INHEAP 1

typedef struct oct_node_t oct_node_t, *oct_node;

struct oct_node_t {
  int64_t r, g, b; /* sum of all child node colors */
  int count, heap_idx;
  unsigned char n_kids, kid_idx, flags, depth;
  oct_node kids[8], parent;
};

typedef struct {
  int alloc, n;
  oct_node *buf;
} node_heap;

static
int cmp_node(oct_node a, oct_node b)
{
  int ac, bc;

  if (a->n_kids < b->n_kids) return -1;
  if (a->n_kids > b->n_kids) return 1;

  ac = a->count >> a->depth;
  bc = b->count >> b->depth;

  return ac < bc ? -1 : ac > bc;
}

static
void down_heap(node_heap *h, oct_node p)
{
  int n = p->heap_idx, m;

  while (1) {
    m = n * 2;
    if (m >= h->n) break;
    if (m + 1 < h->n && cmp_node(h->buf[m], h->buf[m + 1]) > 0) m++;

    if (cmp_node(p, h->buf[m]) <= 0) break;

    h->buf[n] = h->buf[m];
    h->buf[n]->heap_idx = n;
    n = m;
  }
  h->buf[n] = p;
  p->heap_idx = n;
}

static
void up_heap(node_heap *h, oct_node p)
{
  int n = p->heap_idx;
  oct_node prev;

  while (n > 1) {
    prev = h->buf[n / 2];
    if (cmp_node(p, prev) >= 0) break;

    h->buf[n] = prev;
    prev->heap_idx = n;
    n /= 2;
  }
  h->buf[n] = p;
  p->heap_idx = n;
}

static
void heap_add(node_heap *h, oct_node p)
{
  if ((p->flags & ON_INHEAP)) {
    down_heap(h, p);
    up_heap(h, p);
    return;
  }

  p->flags |= ON_INHEAP;
  if (!h->n) h->n = 1;
  if (h->n >= h->alloc) {
    while (h->n >= h->alloc) h->alloc += 1024;
    h->buf = (oct_node *) gks_realloc(h->buf, sizeof(oct_node) * h->alloc);
  }

  p->heap_idx = h->n;
  h->buf[h->n++] = p;
  up_heap(h, p);
}

static
oct_node pop_heap(node_heap *h)
{
  oct_node ret;

  if (h->n <= 1) return NULL;

  ret = h->buf[1];
  h->buf[1] = h->buf[--h->n];

  h->buf[h->n] = NULL;

  h->buf[1]->heap_idx = 1;
  down_heap(h, h->buf[1]);

  return ret;
}

static
oct_node pool = NULL;

static
oct_node node_new(unsigned char idx, unsigned char depth, oct_node p)
{
  static int len = 0;
  oct_node x, n;

  if (len <= 1) {
    n = (oct_node) gks_malloc(sizeof(oct_node_t) * 2048);
    n->parent = pool;
    pool = n;
    len = 2047;
  }

  x = pool + len--;
  x->kid_idx = idx;
  x->depth = depth;
  x->parent = p;
  if (p) p->n_kids++;

  return x;
}

static
void node_free()
{
  oct_node p;

  while (pool) {
    p = pool->parent;
    free(pool);
    pool = p;
  }
}

static
oct_node node_insert(oct_node root, unsigned char *pix)
{
  unsigned char i, bit, depth = 0;

  for (bit = 1 << 7; ++depth < 8; bit >>= 1) {
    i = !!(pix[1] & bit) * 4 + !!(pix[0] & bit) * 2 + !!(pix[2] & bit);
    if (!root->kids[i])
      root->kids[i] = node_new(i, depth, root);

    root = root->kids[i];
  }

  root->r += pix[0];
  root->g += pix[1];
  root->b += pix[2];
  root->count++;

  return root;
}

static
oct_node node_fold(oct_node p)
{
  oct_node q;

  if (p->n_kids) abort();
  q = p->parent;
  q->count += p->count;

  q->r += p->r;
  q->g += p->g;
  q->b += p->b;
  q->n_kids--;
  q->kids[p->kid_idx] = NULL;

  return q;
}

static
int color_replace(oct_node root, unsigned char *pix)
{
  unsigned char i, bit;

  for (bit = 1 << 7; bit; bit >>= 1) {
    i = !!(pix[1] & bit) * 4 + !!(pix[0] & bit) * 2 + !!(pix[2] & bit);
    if (!root->kids[i]) break;
    root = root->kids[i];
  }

  pix[0] = root->r;
  pix[1] = root->g;
  pix[2] = root->b;

  return root->heap_idx;
}

#define N_COLORS 256

#define WRITE_SIXEL_DATA \
  if (cache == -1) \
    c = 0x3f; \
  else { \
    c = 0x3f + n; \
    if (slots[cache] == 0) { \
      r = palette[cache * 3 - 3] * 100 / 256; \
      g = palette[cache * 3 - 2] * 100 / 256; \
      b = palette[cache * 3 - 1] * 100 / 256; \
      slots[cache] = 1; \
      fprintf(stream, "#%d;2;%d;%d;%d", cache, r, g, b); \
    } \
    fprintf(stream, "#%d", cache); \
  } \
  if (count < 3) \
    for (i = 0; i < count; i++) \
      fprintf(stream, "%c", c); \
  else \
    fprintf(stream, "!%d%c", count, c);

static
void write_sixels(char *path, int width, int height, int *palette, int *data)
{
  int i, slots[257];
  FILE *stream;
  int n, x, y, p, cache, count, c, color;
  int r, g, b;

  for (i = 0; i <= 256; i++)
    slots[i] = 0;

  stream = fopen(path, "w");

  fprintf(stream, "%c%s", 0x1b, "P");
  fprintf(stream, "%d;%d;%dq\"1;1;%d;%d", 7, 1, 75, width, height);

  n = 1;
  for (y = 0; y < height; y++) {
    p = y * width;
    cache = data[p];
    count = 1;
    c = -1;
    for (x = 0; x < width; x++) {
      color = data[p + x];
      if (color == cache)
        count += 1;
      else {
        WRITE_SIXEL_DATA;
        count = 1;
        cache = color;
      }
    }
    if (c != -1 && count > 1) {
      WRITE_SIXEL_DATA;
    }
    if (n == 32) {
      n = 1;
      fprintf(stream, "-");
    } else {
      n <<= 1;
      fprintf(stream, "$");
    }
  }
  fprintf(stream, "%c\\", 0x1b);
  fclose(stream);
}

static
void write_to_six(char *path, int width, int height, unsigned char *data)
{
  unsigned char *pix = data;
  oct_node root, got;
  int i, j;
  node_heap heap = { 0, 0, NULL };
  double c;
  int r, g, b, *palette, *ca;

  root = node_new(0, 0, NULL);
  for (i = 0; i < width * height; i++, pix += 4)
    heap_add(&heap, node_insert(root, pix));

  while (heap.n > N_COLORS + 1)
    heap_add(&heap, node_fold(pop_heap(&heap)));

  palette = (int *) gks_malloc(heap.n * 3 * sizeof(int));
  for (i = 1, j = 0; i < heap.n; i++) {
    got = heap.buf[i];
    c = got->count;
    got->r = got->r / c + 0.5;
    got->g = got->g / c + 0.5;
    got->b = got->b / c + 0.5;
    r = (int) got->r & 0xff;
    g = (int) got->g & 0xff;
    b = (int) got->b & 0xff;
    palette[j++] = r;
    palette[j++] = g;
    palette[j++] = b;
  }

  ca = (int *) gks_malloc(width * height * sizeof(int));
  for (i = 0, pix = data; i < width * height; i++, pix += 4)
    ca[i] = color_replace(root, pix);

  write_sixels(path, width, height, palette, ca);

  node_free();
  free(heap.buf);
}

static
void write_page(void)
{
  char path[MAXPATHLEN];
  unsigned char *data, *pix;
  int width, height, stride;
  double alpha;
  int i, j, k, l, bg[3] = { 255, 255, 255 };

  p->empty = 1;
  p->page_counter++;

  cairo_show_page(p->cr);

  if (p->wtype == 140)
    {
      gks_filepath(path, p->path, "png", p->page_counter, 0);
      cairo_surface_write_to_png(p->surface, path);
    }
#ifndef NO_X11
  else if (p->wtype == 141)
    XSync(p->dpy, False);
#endif
  else if (p->wtype == 150)
    {
      cairo_surface_flush(p->surface);

      data = cairo_image_surface_get_data(p->surface);
      width = cairo_image_surface_get_width(p->surface);
      height = cairo_image_surface_get_height(p->surface);
      stride = cairo_image_surface_get_stride(p->surface);

      pix = (unsigned char *) gks_malloc(width * height * 4);
      l = 0;
      for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
          k = j * stride + i * 4;
          alpha =           data[k + 3] / 255.0;
          pix[l++] = (int) (data[k + 2] * alpha + bg[0] * (1 - alpha) + 0.5);
          pix[l++] = (int) (data[k + 1] * alpha + bg[1] * (1 - alpha) + 0.5);
          pix[l++] = (int) (data[k + 0] * alpha + bg[2] * (1 - alpha) + 0.5);
          pix[l++] = 255;
        }
      }
      gks_filepath(path, p->path, "six", p->page_counter, 0);
      write_to_six(path, width, height, pix);
      free(pix);
    }
}

static
void select_xform(int tnr)
{
  gkss->cntnr = tnr;
  set_clip_rect(tnr);
}

static
void set_transparency(double alpha) {
  p->transparency = alpha;
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

void gks_cairoplugin(
  int fctid, int dx, int dy, int dimx, int *ia,
  int lr1, double *r1, int lr2, double *r2,
  int lc, char *chars, void **ptr)
{
  p = (ws_state_list *) *ptr;

  idle = 0;

  switch (fctid)
    {
    case 2:
      /* open workstation */
      gkss = (gks_state_list_t *) * ptr;

      gks_init_core(gkss);

      p = (ws_state_list *) gks_malloc(sizeof(ws_state_list));

      p->conid = ia[1];
      p->path = chars;
      p->wtype = ia[2];

      if (p->wtype == 140)
        {
          p->mw = 0.28575; p->mh = 0.19685;
          p->w = 6750; p->h = 4650;
          resize(2400, 2400);
        }
      else if (p->wtype == 150)
        {
          p->mw = 0.20320; p->mh = 0.15240;
          p->w = 560; p->h = 420;
          resize(400, 400);
        }
      else
        {
          p->mw = 0.25400; p->mh = 0.19050;
          p->w = 1024; p->h = 768;
          resize(500, 500);
        }

      p->max_points = MAX_POINTS;
      p->points = (cairo_point *) gks_malloc(p->max_points * sizeof(cairo_point));
      p->npoints = 0;

      p->empty = 1;
      p->page_counter = 0;

      p->transparency = 1.0;
      p->linewidth = 1;

      init_colors();

      open_page();

      p->patterns = NULL;

      *ptr = p;
      break;

    case 3:
      /* close workstation */
      if (!p->empty)
        write_page();

      close_page();

      free(p->patterns);
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
          lock();
          cairo_set_source_rgb(p->cr, 255, 255,255);
          cairo_rectangle(p->cr, 0, 0, p->width, p->height);
          cairo_fill(p->cr);
          p->empty = 1;
          unlock();
        }
      break;

    case 8:
      /* update workstation */
      if (ia[1] == GKS_K_PERFORM_FLAG)
        {
          lock();
          if (!p->empty)
            write_page();
          unlock();
        }
      break;

    case 12:
      /* polyline */
      if (p->state == GKS_K_WS_ACTIVE)
        {
          lock();
          polyline(ia[0], r1, r2);
          p->empty = 0;
          unlock();
        }
      break;

    case 13:
      /* polymarker */
      if (p->state == GKS_K_WS_ACTIVE)
        {
          lock();
          polymarker(ia[0], r1, r2);
          p->empty = 0;
          unlock();
        }
      break;

    case 14:
      /* text */
      if (p->state == GKS_K_WS_ACTIVE)
        {
          lock();
          text(r1[0], r2[0], strlen(chars), chars);
          p->empty = 0;
          unlock();
        }
      break;

    case 15:
      /* fill area */
      if (p->state == GKS_K_WS_ACTIVE)
        {
          lock();
          fillarea(ia[0], r1, r2);
          p->empty = 0;
          unlock();
        }
      break;

    case 16:
    case DRAW_IMAGE:
      /* cell array */
      if (p->state == GKS_K_WS_ACTIVE)
        {
          int true_color = fctid == DRAW_IMAGE;

          lock();
          cellarray(r1[0], r1[1], r2[0], r2[1], dx, dy, dimx, ia, true_color);
          p->empty = 0;
          unlock();
        }
      break;

    case 48:
      /* set color representation */
      if (p->state == GKS_K_WS_ACTIVE)
        {
          lock();
          set_color_rep(ia[1], r1[0], r1[1], r1[2]);
          unlock();
        }
      break;

    case 49:
      /* set window */
      lock();
      set_window(gkss->cntnr, r1[0], r1[1], r2[0], r2[1]);
      unlock();
      break;

    case 50:
      /* set viewport */
      lock();
      set_viewport(gkss->cntnr, r1[0], r1[1], r2[0], r2[1]);
      unlock();
      break;

    case 52:
      /* select normalization transformation */
      lock();
      select_xform(ia[0]);
      unlock();
      break;

    case 53:
      /* set clipping inidicator */
      lock();
      set_clipping(ia[0]);
      unlock();
      break;

    case 54:
      /* set workstation window */
      lock();
      p->window[0] = r1[0];
      p->window[1] = r1[1];
      p->window[2] = r2[0];
      p->window[3] = r2[1];

      set_xform();
      init_norm_xform();
      unlock();
      break;

    case 55:
      /* set workstation viewport */
      lock();
      p->viewport[0] = 0;
      p->viewport[1] = r1[1] - r1[0];
      p->viewport[2] = 0;
      p->viewport[3] = r2[1] - r2[0];

      p->width = p->viewport[1] * p->w / p->mw;
      p->height = p->viewport[3] * p->h / p->mh;

      close_page();
      open_page();

      set_xform();
      init_norm_xform();
      set_clip_rect(gkss->cntnr);
      unlock();
      break;

    case 203:
      /* set transparency */
      lock();
      set_transparency(r1[0]);
      unlock();
      break;

    default:
      ;
    }

  idle = 1;
}

#else

void gks_cairoplugin(
  int fctid, int dx, int dy, int dimx, int *ia,
  int lr1, double *r1, int lr2, double *r2,
  int lc, char *chars, void **ptr)
{
  if (fctid == 2)
    {
      gks_perror("Cairo support not compiled in");
      ia[0] = 0;
    }
}

#endif
