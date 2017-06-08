#ifndef NO_QT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

DLLEXPORT void QT_PLUGIN_ENTRY_NAME(
  int fctid, int dx, int dy, int dimx, int *i_arr,
  int len_f_arr_1, double *f_arr_1, int len_f_arr_2, double *f_arr_2,
  int len_c_arr, char *c_arr, void **ptr);

#ifdef _WIN32
#ifdef __cplusplus
}
#endif
#endif

#ifndef NO_QT

#define MAX_POINTS 2048
#define MAX_SELECTIONS 100
#define PATTERNS 120
#define HATCH_STYLE 108

#define DrawBorder 0

#define RESOLVE(arg, type, nbytes) arg = (type *)(s + sp); sp += nbytes

#ifndef M_PI
#define M_PI 3.14159265358979323846
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
  yd = (int) (p->c * (yn) + p->d);

#define DC_to_NDC(xd, yd, xn, yn) \
  xn = ((xd) - p->b) / p->a; \
  yn = ((yd) - p->d) / p->c;

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

typedef struct ws_state_list_t
  {
    gks_display_list_t dl;
    QWidget *widget;
    QPainter *pixmap;
    int state, wtype;
    double mwidth, mheight;
    int width, height, dpiX, dpiY;
    double a, b, c, d;
    double window[4], viewport[4];
    QRect rect[MAX_TNR];
    QColor rgb[MAX_COLOR];
    int transparency;
    QPolygon *points;
    int npoints, max_points;
    QFont *font;
    int family, capheight;
    double alpha, angle;
    QPixmap *pattern[PATTERNS];
    int empty;
  }
ws_state_list;

static
ws_state_list *p;

static
const char *fonts[] = {
  "Times New Roman", "Arial", "Courier", "Open Symbol",
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
  0.694, 0.693, 0.683, 0.683 };

static
int map[32] = {
  22,  9,  5, 14, 18, 26, 13,  1,
  24, 11,  7, 16, 20, 28, 13,  3,
  23, 10,  6, 15, 19, 27, 13,  2,
  25, 12,  8, 17, 21, 29, 13,  4 };

int symbol2utf[256] = {
     0,     1,     2,     3,     4,     5,     6,     7,
     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    18,    20,    21,    22,    23,
    24,    25,    26,    27,    28,    29,    30,    31,
    32,    33,  8704,    35,  8707,    37,    38,  8715,
    40,    41,    42,    43,    44,    45,    46,    47,
    48,    49,    50,    51,    52,    53,    54,    55,
    56,    57,    58,    59,    60,    61,    62,    63,
  8773,   913,   914,   935,   916,   917,   934,   915,
   919,   921,   977,   922,   923,   924,   925,   927,
   928,   920,   929,   931,   932,   933,   962,   937,
   926,   936,   918,    91,  8756,    93,  8869,    95,
  8254,   945,   946,   967,   948,   949,   966,   947,
   951,   953,   981,   954,   955,   956,   957,   959,
   960,   952,   961,   963,   964,   965,   982,   969,
   958,   968,   950,   123,   124,   125,   126,   127,
   128,   129,   130,   131,   132,   133,   134,   135,
   136,   137,   138,   139,   140,   141,   142,   143,
   144,   145,   146,   147,   148,   149,   150,   151,
   152,   153,   154,   155,   156,   157,   158,   159,
   160,   978,  8242,  8804,  8260,  8734,   402,  9827,
  9830,  9829,  9824,  8596,  8592,  8593,  8594,  8595,
   176,   177,  8243,  8805,   215,  8733,  8706,  8226,
   247,  8800,  8801,  8776,  8230,  9116,  9135,  8629,
  8501,  8465,  8476,  8472,  8855,  8853,  8709,  8745,
  8746,  8835,  8839,  8836,  8834,  8838,  8712,  8713,
  8736,  8711,   174,   169,  8482,  8719,  8730,   183,
   172,  8743,  8744,  8660,  8656,  8657,  8658,  8659,
  9674, 12296,   174,   169,  8482,  8721,  9115,  9116,
  9117,  9121,  9116,  9123,  9127,  9128,  9129,  9116,
   240, 12297,  8747,  9127,  9116,  9133,  9131,  9130,
  9120,  9124,  9130,  9126,  9131,  9132,  9133,   255
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
int unused_variable = 0;

static
void set_norm_xform(int tnr, double *wn, double *vp)
{
  int xp1, yp1, xp2, yp2;

  a[tnr] = (vp[1] - vp[0]) / (wn[1] - wn[0]);
  b[tnr] = vp[0] - wn[0] * a[tnr];
  c[tnr] = (vp[3] - vp[2]) / (wn[3] - wn[2]);
  d[tnr] = vp[2] - wn[2] * c[tnr];

  NDC_to_DC(vp[0], vp[3], xp1, yp1);
  NDC_to_DC(vp[1], vp[2], xp2, yp2);

  p->rect[tnr].setCoords(xp1, yp1, xp2, yp2);
}

static
void init_norm_xform(void)
{
  int tnr;

  for (tnr = 0; tnr < MAX_TNR; tnr++)
    set_norm_xform(tnr, gkss->window[tnr], gkss->viewport[tnr]);
}

static
void resize_window(void)
{
  p->width  = nint((p->viewport[1] - p->viewport[0]) / 2.54 * p->dpiX * 100);
  p->height = nint((p->viewport[3] - p->viewport[2]) / 2.54 * p->dpiY * 100);
}

static
void set_xform(void)
{
  p->a = (p->width - 1) / (p->window[1] - p->window[0]);
  p->b = -p->window[0] * p->a;
  p->c = (p->height - 1) / (p->window[2] - p->window[3]);
  p->d = p->height - 1 - p->window[2] * p->c;
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
void set_clip_rect(int tnr)
{
  if (gkss->clip == GKS_K_CLIP)
    p->pixmap->setClipRect(p->rect[tnr]);
  else
    p->pixmap->setClipRect(p->rect[0]);
}

static
void set_color_rep(int color, double red, double green, double blue)
{
  if (color >= 0 && color < MAX_COLOR)
    p->rgb[color].setRgb(nint(red * 255), nint(green * 255), nint(blue * 255));
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
void set_color(int color)
{
  QColor transparent_color(p->rgb[color]);
  transparent_color.setAlpha(p->transparency);
  p->pixmap->setPen(transparent_color);
  p->pixmap->setBrush(transparent_color);
}

static
QPixmap *create_pattern(int pattern)
{
  int parray[33];
  int i, j;
  QPixmap *pm;

  gks_inq_pattern_array(pattern, parray);

  QImage img(8, 8, QImage::Format_Mono);
  img.setColor(0, qRgb(255, 255, 255));
  img.setColor(1, qRgb(0, 0, 0));
  for (i = 0; i < 8; i++)
    for (j = 0; j < 8; j++)
      img.setPixel(i, j, (parray[(j % parray[0]) + 1] >> i) & 0x01 ? 0 : 1);

  pm = new QPixmap(8, 8);
  QPixmap tmp = QPixmap::fromImage(img);
  *pm = tmp;

  return pm;
}

static
void line_routine(int n, double *px, double *py, int linetype, int tnr)
{
  double x, y;
  int i, x0, y0, xi, yi, xim1, yim1;

  WC_to_NDC(px[0], py[0], tnr, x, y);
  seg_xform(&x, &y);
  NDC_to_DC(x, y, x0, y0);

  p->npoints = 0;
  p->points->setPoint(p->npoints++, x0, y0);

  xim1 = x0;
  yim1 = y0;
  for (i = 1; i < n; i++)
    {
      WC_to_NDC(px[i], py[i], tnr, x, y);
      seg_xform(&x, &y);
      NDC_to_DC(x, y, xi, yi);

      if (i == 1 || xi != xim1 || yi != yim1)
        {
          p->points->setPoint(p->npoints++, xi, yi);
          xim1 = xi;
          yim1 = yi;
        }
    }
  if (linetype == 0)
    p->points->setPoint(p->npoints++, x0, y0);

  p->pixmap->drawPolyline(p->points->constData(), p->npoints);
}

static
void polyline(int n, double *px, double *py)
{
  int ln_type, ln_color;
  double ln_width, width;
  int i, list[10];

  if (n > p->max_points)
    {
      p->points->resize(n);
      p->max_points = n;
    }
  ln_type  = gkss->asf[0] ? gkss->ltype  : gkss->lindex;
  ln_width = gkss->asf[1] ? gkss->lwidth : 1;
  ln_color = gkss->asf[2] ? gkss->plcoli : 1;

  if (gkss->version > 4 && n < 0.5 * ln_width * p->height)
  /*                       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
     Note: we don't adjust the linewidth to the window height for lines with a
     large number of points because this leads to performance issues in Qt */
    width = ln_width * (p->width + p->height) * 0.001;
  else
    width = ln_width;
  if (width < 1)
    width = 1;

  if (ln_color <= 0 || ln_color >= MAX_COLOR)
    ln_color = 1;

  QColor transparent_color(p->rgb[ln_color]);
  transparent_color.setAlpha(p->transparency);

  p->pixmap->save();
  p->pixmap->setRenderHint(QPainter::Antialiasing);

  if (ln_type != GKS_K_LINETYPE_SOLID)
    {
      gks_get_dash_list(ln_type, 1.0, list);
      QVector<qreal> dashPattern(list[0]);
      for (i = 0; i < list[0]; i++)
        dashPattern[i] = (double) list[i + 1];

      QPen pen(QPen(transparent_color, width, Qt::CustomDashLine));
      pen.setDashPattern(dashPattern);
      p->pixmap->setPen(pen);
    }
  else
    p->pixmap->setPen(QPen(transparent_color, width, Qt::SolidLine));

  line_routine(n, px, py, ln_type, gkss->cntnr);

  p->pixmap->restore();
}

static
void draw_marker(double xn, double yn, int mtype, double mscale, int mcolor)
{
  int r, d, x, y, i;
  int pc, op;
  double scale, xr, yr;
  QPolygon *points;

#include "marker.h"

  if (gkss->version > 4)
    mscale *= (p->width + p->height) * 0.001;
  r = (int) (3 * mscale);
  d = 2 * r;
  scale = 0.01 * mscale / 3.0;

  xr = r;
  yr = 0;
  seg_xform_rel(&xr, &yr);
  r = nint(sqrt(xr * xr + yr * yr));

  NDC_to_DC(xn, yn, x, y);

  pc = 0;
  mtype = (d > 1) ? mtype + marker_off : marker_off + 1;

  do
    {
      op = marker[mtype][pc];
      switch (op)
        {

        case 1:         /* point */
          p->pixmap->drawPoint(x, y);
          break;

        case 2:         /* line */
          for (i = 0; i < 2; i++)
            {
              xr = scale * marker[mtype][pc + 2 * i + 1];
              yr = -scale * marker[mtype][pc + 2 * i + 2];
              seg_xform_rel(&xr, &yr);
              p->points->setPoint(i, nint(x - xr), nint(y + yr));
            }
          p->pixmap->drawPolyline(p->points->constData(), 2);
          pc += 4;
          break;

        case 3:         /* polygon */
          points = new QPolygon(marker[mtype][pc + 1]);
          for (i = 0; i < marker[mtype][pc + 1]; i++)
            {
              xr = scale * marker[mtype][pc + 2 + 2 * i];
              yr = -scale * marker[mtype][pc + 3 + 2 * i];
              seg_xform_rel(&xr, &yr);
              points->setPoint(i, nint(x - xr), nint(y + yr));
            }
          p->pixmap->drawPolyline(points->constData(), marker[mtype][pc + 1]);
          pc += 1 + 2 * marker[mtype][pc + 1];
          delete points;
          break;

        case 4:         /* filled polygon */
        case 5:         /* hollow polygon */
          points = new QPolygon(marker[mtype][pc + 1]);
          if (op == 5)
            set_color(0);
          p->pixmap->setPen(Qt::NoPen);
          for (i = 0; i < marker[mtype][pc + 1]; i++)
            {
              xr = scale * marker[mtype][pc + 2 + 2 * i];
              yr = -scale * marker[mtype][pc + 3 + 2 * i];
              seg_xform_rel(&xr, &yr);
              points->setPoint(i, nint(x - xr), nint(y + yr));
            }
          p->pixmap->drawPolygon(points->constData(), marker[mtype][pc + 1]);
          pc += 1 + 2 * marker[mtype][pc + 1];
          set_color(mcolor);
          delete points;
          break;

        case 6:         /* arc */
          p->pixmap->drawArc(x - r, y - r, d, d, 0, 360 * 16);
          break;

        case 7:         /* filled arc */
        case 8:         /* hollow arc */
          if (op == 8)
            set_color(0);
          p->pixmap->setPen(Qt::NoPen);
          p->pixmap->drawChord(x - r, y - r, d, d, 0, 360 * 16);
          set_color(mcolor);
          break;
        }
      pc++;
    }
  while (op != 0);
}

static
void marker_routine(
  int n, double *px, double *py, int mtype, double mscale, int mcolor)
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
  double mk_size, ln_width;

  mk_type = gkss->asf[3] ? gkss->mtype : gkss->mindex;
  mk_size = gkss->asf[4] ? gkss->mszsc : 1;
  mk_color = gkss->asf[5] ? gkss->pmcoli : 1;
  if (gkss->version > 4)
    {
      ln_width = (p->width + p->height) * 0.001;
      if (ln_width < 1)
        ln_width = 1;
    }
  else
    ln_width = 1;

  p->pixmap->save();
  p->pixmap->setRenderHint(QPainter::Antialiasing);
  QColor transparent_color(p->rgb[mk_color]);
  transparent_color.setAlpha(p->transparency);
  p->pixmap->setPen(QPen(transparent_color, ln_width, Qt::SolidLine));
  p->pixmap->setBrush(QBrush(transparent_color, Qt::SolidPattern));
  marker_routine(n, px, py, mk_type, mk_size, mk_color);
  p->pixmap->restore();
}

static
void text_routine(double x, double y, int nchars, char *chars)
{
  int i, ch, xstart, ystart, width;
  double xrel, yrel, ax, ay;
  QFontMetrics fm = QFontMetrics(*p->font);
  QString s = QString("");

  for (i = 0; i < nchars; i++)
    {
      ch = chars[i];
      if (ch < 0)
        ch += 256;
      if (p->family == 3)
        ch = symbol2utf[ch];
      s.append(QChar(ch));
    }

  NDC_to_DC(x, y, xstart, ystart);

  width = fm.width(s);
  xrel = width * xfac[gkss->txal[0]];
  yrel = p->capheight * yfac[gkss->txal[1]];
  CharXform(xrel, yrel, ax, ay);
  xstart += (int)ax;
  ystart -= (int)ay;

  if (fabs(p->angle) > FEPS)
    {
      p->pixmap->save();
      p->pixmap->translate(xstart, ystart);
      p->pixmap->rotate(-p->angle);
      p->pixmap->drawText(0, 0, s);
      p->pixmap->restore();
    }
  else
    p->pixmap->drawText(xstart, ystart, s);
}

static
void set_font(int font)
{
  double scale, ux, uy;
  int fontNum, size, bold, italic;
  double width, height, capheight;

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
  p->angle = p->alpha * 180 / M_PI;
  if (p->angle < 0) p->angle += 360;

  scale = sqrt(gkss->chup[0] * gkss->chup[0] + gkss->chup[1] * gkss->chup[1]);
  ux = gkss->chup[0] / scale * gkss->chh;
  uy = gkss->chup[1] / scale * gkss->chh;
  WC_to_NDC_rel(ux, uy, gkss->cntnr, ux, uy);

  width = 0;
  height = sqrt(ux * ux + uy * uy);
  seg_xform_rel(&width, &height);

  height = sqrt(width * width + height * height);
  capheight = height * (fabs(p->c) + 1);
  p->capheight = nint(capheight);

  fontNum = font - 1;
  size = nint(p->capheight / capheights[fontNum]);
  if (size < 1)
    size = 1;
  if (font > 13)
    font += 3;
  p->family = (font - 1) / 4;
  bold = (font % 4 == 1 || font % 4 == 2) ? 0 : 1;
  italic = (font % 4 == 2 || font % 4 == 0);

  p->font->setFamily(fonts[p->family]);
  p->font->setBold(bold);
  p->font->setItalic(italic);
  p->font->setPixelSize(size);

  p->pixmap->setFont(*p->font);
}

static
void fill_routine(int n, double *px, double *py, int tnr);

static
void text(double px, double py, int nchars, char *chars)
{
  int tx_font, tx_prec, tx_color;
  double ln_width, x, y;

  tx_font  = gkss->asf[6] ? gkss->txfont : predef_font[gkss->tindex - 1];
  tx_prec  = gkss->asf[6] ? gkss->txprec : predef_prec[gkss->tindex - 1];
  tx_color = gkss->asf[9] ? gkss->txcoli : 1;
  if (gkss->version > 4)
    {
      ln_width = (p->width + p->height) * 0.001;
      if (ln_width < 1)
        ln_width = 1;
    }
  else
    ln_width = 1;
  if (ln_width < 1)
    ln_width = 1;

  p->pixmap->save();
  p->pixmap->setRenderHint(QPainter::Antialiasing);
  QColor transparent_color(p->rgb[tx_color]);
  transparent_color.setAlpha(p->transparency);
  p->pixmap->setPen(QPen(transparent_color, ln_width, Qt::SolidLine));

  if (tx_prec == GKS_K_TEXT_PRECISION_STRING)
    {
      set_font(tx_font);

      WC_to_NDC(px, py, gkss->cntnr, x, y);
      seg_xform(&x, &y);

      text_routine(x, y, nchars, chars);
    }
  else
    gks_emul_text(px, py, nchars, chars, line_routine, fill_routine);

  p->pixmap->restore();
}

static
void fill_routine(int n, double *px, double *py, int tnr)
{
  int i;
  double x, y;
  int ix, iy;
  QPolygon *points;

  points = new QPolygon(n);
  for (i = 0; i < n; i++)
    {
      WC_to_NDC(px[i], py[i], tnr, x, y);
      seg_xform(&x, &y);
      NDC_to_DC(x, y, ix, iy);
      points->setPoint(i, ix, iy);
    }
  p->pixmap->drawPolygon(points->constData(), n);

  delete points;
}

static
void fillarea(int n, double *px, double *py)
{
  int fl_inter, fl_style, fl_color;
  double ln_width;

  fl_inter = gkss->asf[10] ? gkss->ints : predef_ints[gkss->findex - 1];
  fl_style = gkss->asf[11] ? gkss->styli : predef_styli[gkss->findex - 1];
  fl_color = gkss->asf[12] ? gkss->facoli : 1;
  if (gkss->version > 4)
    {
      ln_width = (p->width + p->height) * 0.001;
      if (ln_width < 1)
        ln_width = 1;
    }
  else
    ln_width = 1;
  if (ln_width < 1)
    ln_width = 1;

  QColor transparent_color(p->rgb[fl_color]);
  transparent_color.setAlpha(p->transparency);

  p->pixmap->save();
  p->pixmap->setRenderHint(QPainter::Antialiasing);

  if (fl_inter == GKS_K_INTSTYLE_HOLLOW)
    {
      p->pixmap->setPen(QPen(transparent_color, ln_width, Qt::SolidLine));
      line_routine(n, px, py, DrawBorder, gkss->cntnr);
    }
  else if (fl_inter == GKS_K_INTSTYLE_SOLID)
    {
      p->pixmap->setPen(Qt::NoPen);
      p->pixmap->setBrush(QBrush(transparent_color, Qt::SolidPattern));
      fill_routine(n, px, py, gkss->cntnr);
    }
  else if (fl_inter == GKS_K_INTSTYLE_PATTERN ||
           fl_inter == GKS_K_INTSTYLE_HATCH)
    {
      if (fl_inter == GKS_K_INTSTYLE_HATCH)
        fl_style += HATCH_STYLE;
      if (fl_style >= PATTERNS)
        fl_style = 1;
      if (p->pattern[fl_style] == NULL)
        p->pattern[fl_style] = create_pattern(fl_style);
      p->pixmap->setPen(Qt::NoPen);
      p->pixmap->setBrush(QBrush(transparent_color, *p->pattern[fl_style]));
      fill_routine(n, px, py, gkss->cntnr);
    }

  p->pixmap->restore();
}

static
void cellarray(
  double xmin, double xmax, double ymin, double ymax,
  int dx, int dy, int dimx, int *colia, int true_color)
{
  double x1, y1, x2, y2;
  int ix1, ix2, iy1, iy2;
  int x, y, width, height;
  int i, j, ix, iy, ind, rgb;
  int swapx, swapy;
  QImage *img;
  QImage::Format format;
  int red, green, blue, alpha;

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

  if (!true_color) {
    format = QImage::Format_RGB32;
  } else {
    format = QImage::Format_ARGB32;
  }
  img = new QImage(width, height, format);

  for (j = 0; j < height; j++)
    {
      iy = dy * j / height;
      if (swapy)
        iy = dy - 1 - iy;
      for (i = 0; i < width; i++)
        {
          ix = dx * i / width;
          if (swapx)
            ix = dx - 1 - ix;
          if (!true_color)
            {
              ind = colia[iy * dimx + ix];
              if (ind < 0)
                ind = 0;
              else if (ind >= MAX_COLOR)
                  ind = MAX_COLOR - 1;
              QColor transparent_color(p->rgb[ind]);
              transparent_color.setAlpha(p->transparency);
              img->setPixel(i, j, transparent_color.rgba());
            }
          else
            {
              rgb = colia[iy * dimx + ix];
              red = (rgb & 0xff);
              green = (rgb & 0xff00) >> 8;
              blue = (rgb & 0xff0000) >> 16;
              alpha = (rgb & 0xff000000) >> 24;
              img->setPixel(i, j, qRgba(red, green, blue, alpha));
            }
        }
    }
  p->pixmap->drawPixmap(x, y, QPixmap::fromImage(*img));

  delete img;
}

static
void interp(char *str)
{
  char *s;
  gks_state_list_t *sl = NULL, saved_gkss;
  int sp = 0, *len, *f;
  int *i_arr = NULL, *dx = NULL, *dy = NULL, *dimx = NULL, *len_c_arr = NULL;
  double *f_arr_1 = NULL, *f_arr_2 = NULL;
  char *c_arr = NULL;
  int i, true_color = 0;

  s = str;

  RESOLVE(len, int, sizeof(int));
  while (*len)
    {
      RESOLVE(f, int, sizeof(int));

      switch (*f)
        {
        case   2:               /* open workstation */
          RESOLVE(sl, gks_state_list_t, sizeof(gks_state_list_t));
          break;

        case  12:               /* polyline */
        case  13:               /* polymarker */
        case  15:               /* fill area */
          RESOLVE(i_arr, int, sizeof(int));
          RESOLVE(f_arr_1, double, i_arr[0] * sizeof(double));
          RESOLVE(f_arr_2, double, i_arr[0] * sizeof(double));
          break;

        case  14:               /* text */
          RESOLVE(f_arr_1, double, sizeof(double));
          RESOLVE(f_arr_2, double, sizeof(double));
          RESOLVE(len_c_arr, int, sizeof(int));
          RESOLVE(c_arr, char, 132);
          break;

        case  16:               /* cell array */
        case 201:               /* draw image */
          RESOLVE(f_arr_1, double, 2 * sizeof(double));
          RESOLVE(f_arr_2, double, 2 * sizeof(double));
          RESOLVE(dx, int, sizeof(int));
          RESOLVE(dy, int, sizeof(int));
          RESOLVE(dimx, int, sizeof(int));
          RESOLVE(i_arr, int, *dimx * *dy * sizeof(int));
          break;

        case  19:               /* set linetype */
        case  21:               /* set polyline color index */
        case  23:               /* set markertype */
        case  25:               /* set polymarker color index */
        case  30:               /* set text color index */
        case  33:               /* set text path */
        case  36:               /* set fillarea interior style */
        case  37:               /* set fillarea style index */
        case  38:               /* set fillarea color index */
        case  52:               /* select normalization transformation */
        case  53:               /* set clipping indicator */
          RESOLVE(i_arr, int, sizeof(int));
          break;

        case  27:               /* set text font and precision */
        case  34:               /* set text alignment */
          RESOLVE(i_arr, int, 2 * sizeof(int));
          break;

        case  20:               /* set linewidth scale factor */
        case  24:               /* set marker size scale factor */
        case  28:               /* set character expansion factor */
        case  29:               /* set character spacing */
        case  31:               /* set character height */
        case 200:               /* set text slant */
        case 203:               /* set transparency */
          RESOLVE(f_arr_1, double, sizeof(double));
          break;

        case  32:               /* set character up vector */
          RESOLVE(f_arr_1, double, sizeof(double));
          RESOLVE(f_arr_2, double, sizeof(double));
          break;

        case  41:               /* set aspect source flags */
          RESOLVE(i_arr, int, 13 * sizeof(int));
          break;

        case  48:               /* set color representation */
          RESOLVE(i_arr, int, sizeof(int));
          RESOLVE(f_arr_1, double, 3 * sizeof(double));
          break;

        case  49:               /* set window */
        case  50:               /* set viewport */
        case  54:               /* set workstation window */
        case  55:               /* set workstation viewport */
          RESOLVE(i_arr, int, sizeof(int));
          RESOLVE(f_arr_1, double, 2 * sizeof(double));
          RESOLVE(f_arr_2, double, 2 * sizeof(double));
          break;

        case 202:               /* set shadow */
          RESOLVE(f_arr_1, double, 3 * sizeof(double));
          break;

        case 204:               /* set coord xform */
          RESOLVE(f_arr_1, double, 6 * sizeof(double));
          break;

        default:
          gks_perror("display list corrupted (len=%d, fctid=%d)", *len, *f);
          exit(1);
        }

      switch (*f)
        {
        case   2:
          memmove(&saved_gkss, gkss, sizeof(gks_state_list_t));
          memmove(gkss, sl, sizeof(gks_state_list_t));

          p->window[0] = p->window[2] = 0.0;
          p->window[1] = p->window[3] = 1.0;

          p->viewport[0] = p->viewport[2] = 0.0;
          p->viewport[1] = p->width  * 2.54 / p->dpiX / 100;
          p->viewport[3] = p->height * 2.54 / p->dpiY / 100;

          set_xform();
          init_norm_xform();
          init_colors();

          gks_init_core(gkss);
          break;

        case  12:
          polyline(i_arr[0], f_arr_1, f_arr_2);
          break;

        case  13:
          polymarker(i_arr[0], f_arr_1, f_arr_2);
          break;

        case  14:
          unused_variable = *len_c_arr;
          text(f_arr_1[0], f_arr_2[0], strlen(c_arr), c_arr);
          break;

        case  15:
          fillarea(i_arr[0], f_arr_1, f_arr_2);
          break;

        case  16:
        case 201:
          true_color = *f == DRAW_IMAGE;
          cellarray(f_arr_1[0], f_arr_1[1], f_arr_2[0], f_arr_2[1],
                    *dx, *dy, *dimx, i_arr, true_color);
          break;

        case  19:
          gkss->ltype = i_arr[0];
          break;

        case  20:
          gkss->lwidth = f_arr_1[0];
          break;

        case  21:
          gkss->plcoli = i_arr[0];
          break;

        case  23:
          gkss->mtype = i_arr[0];
          break;

        case  24:
          gkss->mszsc = f_arr_1[0];
          break;

        case  25:
          gkss->pmcoli = i_arr[0];
          break;

        case  27:
          gkss->txfont = i_arr[0];
          gkss->txprec = i_arr[1];
          break;

        case  28:
          gkss->chxp = f_arr_1[0];
          break;

        case  29:
          gkss->chsp = f_arr_1[0];
          break;

        case  30:
          gkss->txcoli = i_arr[0];
          break;

        case  31:
          gkss->chh = f_arr_1[0];
          break;

        case  32:
          gkss->chup[0] = f_arr_1[0];
          gkss->chup[1] = f_arr_2[0];
          break;

        case  33:
          gkss->txp = i_arr[0];
          break;

        case  34:
          gkss->txal[0] = i_arr[0];
          gkss->txal[1] = i_arr[1];
          break;

        case  36:
          gkss->ints = i_arr[0];
          break;

        case  37:
          gkss->styli = i_arr[0];
          break;

        case  38:
          gkss->facoli = i_arr[0];
          break;

        case  41:
          for (i = 0; i < 13; i++)
            gkss->asf[i] = i_arr[i];
          break;

        case  48:
          set_color_rep(i_arr[0], f_arr_1[0], f_arr_1[1], f_arr_1[2]);
          break;

        case  49:
          gkss->window[*i_arr][0] = f_arr_1[0];
          gkss->window[*i_arr][1] = f_arr_1[1];
          gkss->window[*i_arr][2] = f_arr_2[0];
          gkss->window[*i_arr][3] = f_arr_2[1];
          set_norm_xform(*i_arr, gkss->window[*i_arr], gkss->viewport[*i_arr]);
          gks_set_norm_xform(*i_arr, gkss->window[*i_arr], gkss->viewport[*i_arr]);
          break;

        case  50:
          gkss->viewport[*i_arr][0] = f_arr_1[0];
          gkss->viewport[*i_arr][1] = f_arr_1[1];
          gkss->viewport[*i_arr][2] = f_arr_2[0];
          gkss->viewport[*i_arr][3] = f_arr_2[1];
          set_norm_xform(*i_arr, gkss->window[*i_arr], gkss->viewport[*i_arr]);
          gks_set_norm_xform(*i_arr, gkss->window[*i_arr], gkss->viewport[*i_arr]);

          if (*i_arr == gkss->cntnr)
            set_clip_rect(*i_arr);
          break;

        case  52:
          gkss->cntnr = i_arr[0];
          set_clip_rect(gkss->cntnr);
          break;

        case  53:
          gkss->clip = i_arr[0];
          set_clip_rect(gkss->cntnr);
          break;

        case  54:
          p->window[0] = f_arr_1[0];
          p->window[1] = f_arr_1[1];
          p->window[2] = f_arr_2[0];
          p->window[3] = f_arr_2[1];

          set_xform();
          init_norm_xform();
          break;

        case  55:
          p->viewport[0] = f_arr_1[0];
          p->viewport[1] = f_arr_1[1];
          p->viewport[2] = f_arr_2[0];
          p->viewport[3] = f_arr_2[1];

          resize_window();
          set_xform();
          init_norm_xform();
          break;

        case 200:
          gkss->txslant = f_arr_1[0];
          break;

        case 203:
          p->transparency = (int) (f_arr_1[0] * 255);
        }

      RESOLVE(len, int, sizeof(int));
    }

  memmove(gkss, &saved_gkss, sizeof(gks_state_list_t));
}

static
int get_pixmap(void)
{
  char *env;

  env = (char *) gks_getenv("GKS_CONID");
  if (!env)
    env = (char *) gks_getenv("GKSconid");

  if (env != NULL)
    {
      if (strchr(env, '!') == NULL)
        {
          p->widget = NULL;
          sscanf(env, "%p", (void **) &p->pixmap);
        }
      else
        sscanf(env, "%p!%p", (void **) &p->widget, (void **) &p->pixmap);
    }
  else
      return 1;

  if (p->widget != NULL)
    {
      p->mwidth  = p->widget->widthMM()  * 0.001;
      p->mheight = p->widget->heightMM() * 0.001;
      p->width   = p->widget->width();
      p->height  = p->widget->height();
      p->dpiX    = p->widget->physicalDpiX();
      p->dpiY    = p->widget->physicalDpiY();
    }
  else
    {
      p->mwidth  = p->pixmap->device()->widthMM()  * 0.001;
      p->mheight = p->pixmap->device()->heightMM() * 0.001;
      p->width   = p->pixmap->device()->width();
      p->height  = p->pixmap->device()->height();
      p->dpiX    = p->pixmap->device()->logicalDpiX();
      p->dpiY    = p->pixmap->device()->logicalDpiY();
    }
  return 0;
}

void QT_PLUGIN_ENTRY_NAME(
  int fctid, int dx, int dy, int dimx, int *i_arr,
  int len_f_arr_1, double *f_arr_1, int len_f_arr_2, double *f_arr_2,
  int len_c_arr, char *c_arr, void **ptr)
{
  int i;

  p = (ws_state_list *) *ptr;

  switch (fctid)
    {
    case 2:
      gkss = (gks_state_list_t *) * ptr;
      p = new ws_state_list;

      p->width = p->height = 500;
      p->dpiX = p->dpiY = 100;

      p->font = new QFont();

      p->points = new QPolygon(MAX_POINTS);
      p->npoints = 0;
      p->max_points = MAX_POINTS;

      for (i = 0; i < PATTERNS; i++)
        p->pattern[i] = NULL;

      p->empty = 1;

      p->transparency = 255;

      if (get_pixmap() == 0)
        {
          f_arr_1[0] = p->mwidth;
          f_arr_2[0] = p->mheight;
          i_arr[0] = p->width;
          i_arr[1] = p->height;
        }
      else
        {
          f_arr_1[0] = 0.25400;
          f_arr_2[0] = 0.19050;
          i_arr[0] = 1024;
          i_arr[1] = 768;
        }

      *ptr = p;
      break;

    case 3:
      for (i = 0; i < PATTERNS; i++)
        if (p->pattern[i] != NULL)
          free(p->pattern[i]);

      delete p->points;
      delete p->font;
      delete p;

      p = NULL;
      break;

    case 6:
      /* set display list length to zero */
      memset(p->dl.buffer, 0, sizeof(int));
      p->dl.nbytes = 0;

      p->empty = 1;
      break;

    case 8:
      if (i_arr[1] == GKS_K_PERFORM_FLAG)
        {
          if (get_pixmap() == 0)
            interp(p->dl.buffer);
          else if (!p->empty)
            gks_perror("can't obtain Qt drawable");
        }
      break;

    case  12:  /* polyline */
    case  13:  /* polymarker */
    case  14:  /* text */
    case  15:  /* fill area */
    case  16:  /* cell array */
    case 201:  /* draw image */
      p->empty = 0;
      break;

    default:
      ;
    }

  if (p != NULL)
    gks_dl_write_item(&p->dl,
      fctid, dx, dy, dimx, i_arr, len_f_arr_1, f_arr_1, len_f_arr_2, f_arr_2,
      len_c_arr, c_arr, gkss);
}

#else

void QT_PLUGIN_ENTRY_NAME(
  int fctid, int dx, int dy, int dimx, int *ia,
  int lr1, double *r1, int lr2, double *r2, int lc, char *chars,
  void **ptr)
{
  if (fctid == 2)
  {
    gks_perror(QT_NAME_STRING " support not compiled in");
    ia[0] = 0;
  }
}

#endif

