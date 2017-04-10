#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <QtGlobal>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#endif
#include <qpainter.h>
#include <qpixmap.h>
#include <qimage.h>

#include "gkswidget.h"
#include "gks.h"
#include "gkscore.h"

#define PORT 8410
#define SIZE 262144

#define MAXCONN 10

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

#define WC_to_NDC(xw, yw, tnr, xn, yn)          \
  xn = a[tnr] * (xw) + b[tnr];                  \
  yn = c[tnr] * (yw) + d[tnr]

#define WC_to_NDC_rel(xw, yw, tnr, xn, yn)      \
  xn = a[tnr] * (xw);                           \
  yn = c[tnr] * (yw)

#define NDC_to_WC(xw, yw, tnr, xn, yn)          \
  xn = ((xw) - b[tnr]) / a[tnr];                \
  yn = ((yw) - d[tnr]) / c[tnr]

#define NDC_to_DC(xn, yn, xd, yd)               \
  xd = (int) (p->a * (xn) + p->b);              \
  yd = (int) (p->c * (yn) + p->d);

#define DC_to_NDC(xd, yd, xn, yn)               \
  xn = ((xd) - p->b) / p->a;                    \
  yn = ((yd) - p->d) / p->c;

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

int GKSWidget::lastWidgetNumber = 0;

static
gks_state_list_t gkss_, *gkss;

static
double a[MAX_TNR], b[MAX_TNR], c[MAX_TNR], d[MAX_TNR];

typedef struct ws_state_list_t
  {
    QPixmap *pm;
    QPainter *pixmap;
    int state, wtype;
    int width, height;
    int saved_width, saved_height;
    double a, b, c, d;
    double window[4], viewport[4];
    QRect rect[MAX_TNR];
    QColor rgb[MAX_COLOR];
    QPolygon *points;
    int npoints, max_points;
    QFont *font;
    int family, capheight;
    double alpha, angle;
    QPixmap *pattern[PATTERNS];
    int trans_x, trans_y;
    bool selecting, have_selection;
    QPoint start;
    QRect area;
  }
ws_state_list;

static
ws_state_list p_, *p;

static
GKSWidget *activeWidget;

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

static
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

  gks_set_norm_xform(tnr, wn, vp);
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
  p->pixmap->setPen(p->rgb[color]);
  p->pixmap->setBrush(p->rgb[color]);
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
void create_window(ws_state_list *p)
{
  int i;

  p->pm = new QPixmap(p->width, p->height);
  p->pm->fill(Qt::white);

  p->pixmap = new QPainter(p->pm);
  p->pixmap->setClipRect(0, 0, p->width, p->height);

  p->font = new QFont();

  p->points = new QPolygon(MAX_POINTS);
  p->npoints = 0;
  p->max_points = MAX_POINTS;

  for (i = 0; i < PATTERNS; i++)
    p->pattern[i] = NULL;
}

static
void resize_window(void)
{
  int width, height;
  QWidget *parentWidget;

  width  = nint((p->viewport[1] - p->viewport[0]) / 2.54 *
                activeWidget->physicalDpiX() * 100);
  height = nint((p->viewport[3] - p->viewport[2]) / 2.54 *
                activeWidget->physicalDpiY() * 100);

  if (p->width != width || p->height != height)
    {
      parentWidget = activeWidget->parentWidget();
      parentWidget->resize(width + parentWidget->width() - p->width,
                           height + parentWidget->height() - p->height);

      p->width = width;
      p->height = height;
    }
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

  p->pixmap->save();
  p->pixmap->setRenderHint(QPainter::Antialiasing);

  if (ln_type != GKS_K_LINETYPE_SOLID)
    {
      gks_get_dash_list(ln_type, 1.0, list);
      QVector<qreal> dashPattern(list[0]);
      for (i = 0; i < list[0]; i++)
        dashPattern[i] = (double) list[i + 1];

      QPen pen(QPen(p->rgb[ln_color], width, Qt::CustomDashLine));
      pen.setDashPattern(dashPattern);
      p->pixmap->setPen(pen);
    }
  else
    p->pixmap->setPen(QPen(p->rgb[ln_color], width, Qt::SolidLine));

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
          for (i = 0; i < marker[mtype][pc + 1]; i++)
            {
              xr = scale * marker[mtype][pc + 2 + 2 * i];
              yr = -scale * marker[mtype][pc + 3 + 2 * i];
              seg_xform_rel(&xr, &yr);
              points->setPoint(i, nint(x - xr), nint(y + yr));
            }
          p->pixmap->drawPolygon(points->constData(), marker[mtype][pc + 1]);
          pc += 1 + 2 * marker[mtype][pc + 1];
          if (op == 5)
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
          p->pixmap->drawChord(x - r, y - r, d, d, 0, 360 * 16);
          if (op == 8)
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
  p->pixmap->setPen(QPen(p->rgb[mk_color], ln_width, Qt::SolidLine));
  p->pixmap->setBrush(QBrush(p->rgb[mk_color], Qt::SolidPattern));
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
  p->pixmap->setPen(QPen(p->rgb[tx_color], ln_width, Qt::SolidLine));

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

  p->pixmap->save();
  p->pixmap->setRenderHint(QPainter::Antialiasing);

  if (fl_inter == GKS_K_INTSTYLE_HOLLOW)
    {
      p->pixmap->setPen(QPen(p->rgb[fl_color], ln_width, Qt::SolidLine));
      line_routine(n, px, py, DrawBorder, gkss->cntnr);
    }
  else if (fl_inter == GKS_K_INTSTYLE_SOLID)
    {
      p->pixmap->setPen(Qt::NoPen);
      p->pixmap->setBrush(QBrush(p->rgb[fl_color], Qt::SolidPattern));
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
      p->pixmap->setBrush(QBrush(p->rgb[fl_color], *p->pattern[fl_style]));
      fill_routine(n, px, py, gkss->cntnr);
    }

  p->pixmap->restore();
}

static
void adjust_cellarray(
  double *qx, double *qy, double *rx, double *ry,
  int *scol, int *srow, int *ncol, int *nrow, int dimx, int dimy)
{
  double xmin, xmax, ymin, ymax, tmp, dx, dy;
  double left, bottom, right, top;

  WC_to_NDC(*qx, *qy, gkss->cntnr, xmin, ymax);
  WC_to_NDC(*rx, *ry, gkss->cntnr, xmax, ymin);

  if (p->have_selection)
    {
      DC_to_NDC(p->area.left(), p->area.right(), left, right);
      DC_to_NDC(p->area.bottom(), p->area.top(), bottom, top);
    }
  else
    {
      left = 0;
      right = 1;
      bottom = 0;
      top = 1;
    }

  if (*rx < *qx)
    {
      tmp = xmax; xmax = xmin; xmin = tmp;
    }
  if (*ry < *qy)
    {
      tmp = ymax; ymax = ymin; ymin = tmp;
    }

  dx = (xmax - xmin) / *ncol;
  dy = (ymax - ymin) / *nrow;

  while (xmin + dx < left && *ncol > 0)
    {
      xmin += dx;
      *scol += 1;
      *ncol -= 1;
      if (xmin >= xmax || *scol + *ncol - 1 > dimx)
        *ncol = 0;
    }
  while (xmax - dx > right && *ncol > 0)
    {
      xmax -= dx;
      *ncol -= 1;
      if (xmin >= xmax)
        *ncol = 0;
    }

  while (ymin + dy < bottom && *ncol > 0 && *nrow > 0)
    {
      ymin += dy;
      *srow += 1;
      *nrow -= 1;
      if (ymin >= ymax || *srow + *nrow - 1 > dimy)
        *nrow = 0;
    }
  while (ymax - dy > top && *ncol > 0 && *nrow > 0)
    {
      ymax -= dy;
      *nrow -= 1;
      if (ymin >= ymax)
        *nrow = 0;
    }

  if (xmax - xmin > 3 || ymax - ymin > 3)
    *ncol = *nrow = 0;

  if (*rx < *qx)
    {
      tmp = xmax; xmax = xmin; xmin = tmp;
    }
  if (*ry < *qy)
    {
      tmp = ymax; ymax = ymin; ymin = tmp;
    }

  NDC_to_WC(xmin, ymax, gkss->cntnr, *qx, *qy);
  NDC_to_WC(xmax, ymin, gkss->cntnr, *rx, *ry);
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
  int red, green, blue;

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

  img = new QImage(width, height, QImage::Format_RGB32);
  if (img->isNull())
    {
      gks_perror("memory for %dx%d QImage cannot be allocated", width, height);
      exit(1);
    }

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
              img->setPixel(i, j, p->rgb[ind].rgb());
            }
          else
            {
              rgb = colia[iy * dimx + ix];
              red = (rgb & 0xff);
              green = (rgb & 0xff00) >> 8;
              blue = (rgb & 0xff0000) >> 16;
              img->setPixel(i, j, qRgb(red, green, blue));
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
  int scol, srow, ncol, nrow;
  double qx, qy, rx, ry;

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
          gkss = &gkss_;
          p = &p_;

          memmove(&saved_gkss, gkss, sizeof(gks_state_list_t));
          memmove(gkss, sl, sizeof(gks_state_list_t));

          p->window[0] = p->window[2] = 0.0;
          p->window[1] = p->window[3] = 1.0;

          p->viewport[0] = p->viewport[2] = 0.0;
          p->viewport[1] = p->width  * 2.54 / activeWidget->physicalDpiX() / 100;
          p->viewport[3] = p->height * 2.54 / activeWidget->physicalDpiY() / 100;

          p->selecting = false;
          p->have_selection = false;

          gks_init_core(gkss);

          set_xform();
          init_norm_xform();
          init_colors();

          gkss->fontfile = gks_open_font();
          break;

        case   3:
          if (gkss)
            gks_close_font(gkss->fontfile);

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

          qx = f_arr_1[0];
          qy = f_arr_2[0];
          rx = f_arr_1[1];
          ry = f_arr_2[1];
          scol = 1;
          srow = 1;
          ncol = *dx;
          nrow = *dy;
          adjust_cellarray(&qx, &qy, &rx, &ry,
                           &scol, &srow, &ncol, &nrow, *dimx, *dy);

          true_color = *f == DRAW_IMAGE;
          cellarray(qx, rx, qy, ry,
                    ncol, nrow, *dimx, i_arr + (srow - 1) * *dimx + scol - 1,
                    true_color);
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
          break;

        case  50:
          gkss->viewport[*i_arr][0] = f_arr_1[0];
          gkss->viewport[*i_arr][1] = f_arr_1[1];
          gkss->viewport[*i_arr][2] = f_arr_2[0];
          gkss->viewport[*i_arr][3] = f_arr_2[1];
          set_norm_xform(*i_arr, gkss->window[*i_arr], gkss->viewport[*i_arr]);

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
        }

      RESOLVE(len, int, sizeof(int));
    }

  if (gkss)
    gks_close_font(gkss->fontfile);

  memmove(gkss, &saved_gkss, sizeof(gks_state_list_t));
}


GKSQtWindow::GKSQtWindow(QWidget *parent)
  : QMainWindow(parent)
{
  mdiArea = new QMdiArea;
  mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  mdiArea->setOption(QMdiArea::DontMaximizeSubWindowOnActivation);
  mdiArea->setTabPosition(QTabWidget::North);
  setCentralWidget(mdiArea);

  connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow *)),
          this, SLOT(updateMenus()));

  windowMapper = new QSignalMapper(this);
  connect(windowMapper, SIGNAL(mapped(QWidget *)),
          this, SLOT(setActiveSubWindow(QWidget *)));

  dl = NULL;
  activeWidget = NULL;

  server = new GKSServer();
  connect(server, SIGNAL(openWindow()), this, SLOT(openWindow()));
  connect(server, SIGNAL(data(char *)), this, SLOT(interpret(char *)));

  p = &p_;
  p->width = p->height = 500;

  supportedFileFmtList = QImageWriter::supportedImageFormats();
  rotation  = 0;
  rotateBy = 90.0;
  numWidgets = 0;

  create_window(p);
  createMenubar();
  createToolbar();

  setWindowTitle(tr("GKS Qt"));
  setUnifiedTitleAndToolBarOnMac(true);

  QSize disp  = QApplication::desktop()->screenGeometry().size();
  resize(qMin(1000, qRound(disp.width()*0.8)), qMin(750, qRound(disp.height()*0.8)));
}

GKSQtWindow::~GKSQtWindow()
{
}

void GKSQtWindow::setActiveSubWindow(QWidget *window)
{
  if (!window)
    return;
  mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
}

void GKSQtWindow::updateMenuWindow()
{
  menuWindow->clear();
  menuWindow->addAction(actionMinimize);
  menuWindow->addAction(actionClose);
  menuWindow->addAction(actionCloseAll);
  menuWindow->addSeparator();
  menuWindow->addAction(actionTabbedView);
  menuWindow->addMenu(menuTabPosition);
  menuWindow->addAction(actionTile);
  menuWindow->addAction(actionCascade);
  menuWindow->addSeparator();
  menuWindow->addAction(actionNext);
  menuWindow->addAction(actionPrevious);
  menuWindow->addSeparator();

  QList<QMdiSubWindow *> win_list = mdiArea->subWindowList();
  for (int i=0; i<win_list.size(); i++) {
    GKSWidget *win = qobject_cast<GKSWidget *>(win_list.at(i)->widget());
    QAction *act = menuWindow->addAction(win->windowTitle());
    act->setCheckable(true);
    act->setChecked(win == activeMdiChild());
    connect(act, SIGNAL(triggered()), windowMapper, SLOT(map()));
    windowMapper->setMapping(act, win_list.at(i));
  }
}

void GKSQtWindow::updateMenus()
{
  bool hasGKSWidget = (activeMdiChild() != 0);

  actionSave_As->setEnabled(hasGKSWidget);
  actionPrint->setEnabled(hasGKSWidget);
  actionRotate_by_90->setEnabled(hasGKSWidget);
  actionMinimize->setEnabled(hasGKSWidget);
  actionClose->setEnabled(hasGKSWidget);
  actionCloseAll->setEnabled(hasGKSWidget);
  actionTile->setEnabled(hasGKSWidget);
  actionCascade->setEnabled(hasGKSWidget);
  actionNext->setEnabled(hasGKSWidget);
  actionPrevious->setEnabled(hasGKSWidget);
}

GKSWidget *GKSQtWindow::activeMdiChild()
{
  if (QMdiSubWindow *activeSubWindow = mdiArea->activeSubWindow()) {
    mdiArea->update();
    return qobject_cast<GKSWidget *>(activeSubWindow->widget());
  }
  return 0;
}

void GKSQtWindow::createToolbar()
{
  toolBarFile = addToolBar("menuFile");
  toolBarFile->setToolButtonStyle(Qt::ToolButtonIconOnly);
  toolBarFile->addAction(actionSave_As);
  toolBarFile->addAction(actionPrint);
  toolBarFile->addSeparator();

  toolBarEdit = addToolBar("menuEdit");
  toolBarEdit->setToolButtonStyle(Qt::ToolButtonIconOnly);
  toolBarEdit->addAction(actionRotate_by_90);
  toolBarEdit->addSeparator();

  toolBarWindow = addToolBar(QString::fromUtf8("menuWindow"));
  toolBarWindow->setToolButtonStyle(Qt::ToolButtonIconOnly);
  toolBarWindow->addAction(actionClose);
  toolBarWindow->addSeparator();
  toolBarWindow->addAction(actionNext);
  toolBarWindow->addAction(actionPrevious);

  QAction *act = toolBarFile->toggleViewAction();
  act->setText("File toolbar");
  menuView->addAction(act);
  act = toolBarEdit->toggleViewAction();
  act->setText("Edit toolbar");
  menuView->addAction(act);
  act = toolBarWindow->toggleViewAction();
  act->setText("Window toolbar");
  menuView->addAction(act);
}

void GKSQtWindow::createMenubar()
{
  menuBar = new QMenuBar(this);

  menuBar->setObjectName(QString::fromUtf8("menuBar"));
  this->setMenuBar(menuBar);

  actionSave_As = new QAction(this);
  actionSave_As->setObjectName(QString::fromUtf8("actionSave_As"));
  actionSave_As->setText(QString::fromUtf8("Save As ..."));
  actionSave_As->setShortcut(QKeySequence(QKeySequence::SaveAs));
  actionSave_As->setIcon(QIcon(":/images/save.png"));
  actionSave_As->setIconText(actionSave_As->text());

  actionPage_Setup = new QAction(this);
  actionPage_Setup->setObjectName(QString::fromUtf8("actionPage_Setup"));
  actionPage_Setup->setText(QString::fromUtf8("Page Setup"));

  actionPrint = new QAction(this);
  actionPrint->setObjectName(QString::fromUtf8("actionPrint"));
  actionPrint->setText(QString::fromUtf8("Print ..."));
  actionPrint->setShortcut(QKeySequence(QKeySequence::Print));
  actionPrint->setIcon(QIcon(":/images/fileprint.png"));
  actionPrint->setIconText(actionPrint->text());

  actionQuitGKSQt = new QAction(this);
  actionQuitGKSQt->setObjectName(QString::fromUtf8("actionQuitGKSQt"));
  actionQuitGKSQt->setText(QString::fromUtf8("Quit GKSQt"));
  actionQuitGKSQt->setShortcut(QKeySequence(QKeySequence::Quit));

  menuFile = new QMenu(menuBar);
  menuFile->setObjectName(QString::fromUtf8("menuFile"));
  menuFile->setTitle(QString::fromUtf8("File"));
  menuBar->addMenu(menuFile);
  menuFile->addAction(actionSave_As);
  menuFile->addSeparator();
  menuFile->addAction(actionPage_Setup);
  menuFile->addAction(actionPrint);
  menuFile->addSeparator();
  menuFile->addAction(actionQuitGKSQt);

  actionCut = new QAction(this);
  actionCut->setObjectName(QString::fromUtf8("actionCut"));
  actionCut->setText(QString::fromUtf8("Cut"));
  actionCut->setShortcut(QKeySequence(QKeySequence::Cut));
  actionCut->setEnabled(false);
  actionCut->setIcon(QIcon(":/images/cut.png"));
  actionCut->setIconText(actionCut->text());

  actionCopy = new QAction(this);
  actionCopy->setObjectName(QString::fromUtf8("actionCopy"));
  actionCopy->setText(QString::fromUtf8("Copy"));
  actionCopy->setShortcut(QKeySequence(QKeySequence::Copy));
  actionCopy->setEnabled(false);
  actionCopy->setIcon(QIcon(":/images/copy.png"));
  actionCopy->setIconText(actionCopy->text());

  actionPaste = new QAction(this);
  actionPaste->setObjectName(QString::fromUtf8("actionPaste"));
  actionPaste->setText(QString::fromUtf8("Paste"));
  actionPaste->setShortcut(QKeySequence(QKeySequence::Paste));
  actionPaste->setEnabled(false);
  actionPaste->setIcon(QIcon(":/images/paste.png"));
  actionPaste->setIconText(actionPaste->text());

  actionKeep_on_Display = new QAction(this);
  actionKeep_on_Display->setObjectName(QString::fromUtf8("actionKeep_on_Display"));
  actionKeep_on_Display->setText(QString::fromUtf8("Keep on Display"));
  actionKeep_on_Display->setShortcut(QKeySequence("Ctrl+K"));
  actionKeep_on_Display->setCheckable(true);
  actionKeep_on_Display->setChecked(true);
  server->setKeepOnDisplay(actionKeep_on_Display->isChecked());

  actionRotate_by_90 = new QAction(this);
  actionRotate_by_90->setObjectName(QString::fromUtf8("actionRotate_by_90"));
  actionRotate_by_90->setText(QString::fromUtf8("Rotate by %1").arg(rotateBy)+QString(QChar(0x00b0)));
  actionRotate_by_90->setShortcut(QKeySequence("Ctrl+R"));
  actionRotate_by_90->setIcon(QIcon(":/images/rotateright.png"));
  actionRotate_by_90->setIconText(actionRotate_by_90->text());

  actionSpecial_Characters = new QAction(this);
  actionSpecial_Characters->setObjectName(QString::fromUtf8("actionSpecial_Characters"));
  actionSpecial_Characters->setText(QString::fromUtf8("Special Characters ..."));
  actionSpecial_Characters->setShortcut(QKeySequence("Ctrl+Alt+T"));
  actionSpecial_Characters->setEnabled(false);

  menuEdit = new QMenu(menuBar);
  menuEdit->setObjectName(QString::fromUtf8("menuEdit"));
  menuEdit->setTitle(QString::fromUtf8("Edit"));
  menuBar->addMenu(menuEdit);
  menuEdit->addAction(actionCut);
  menuEdit->addAction(actionCopy);
  menuEdit->addAction(actionPaste);
  menuEdit->addAction(actionKeep_on_Display);
  menuEdit->addSeparator();
  menuEdit->addAction(actionRotate_by_90);
  menuEdit->addSeparator();
  menuEdit->addAction(actionSpecial_Characters);

  actionMinimize = new QAction(this);
  actionMinimize->setObjectName(QString::fromUtf8("actionMinimize"));
  actionMinimize->setText(QString::fromUtf8("Minimize"));
  actionMinimize->setShortcut(QKeySequence("Ctrl+M"));

  actionClose = new QAction(this);
  actionClose->setObjectName(QString::fromUtf8("actionClose"));
  actionClose->setText(QString::fromUtf8("Close"));
  actionClose->setShortcut(QKeySequence(QKeySequence::Close));
  actionClose->setIcon(QIcon(":/images/close.png"));
  actionClose->setIconText(actionClose->text());

  actionCloseAll = new QAction(this);
  actionCloseAll->setObjectName(QString::fromUtf8("actionCloseAll"));
  actionCloseAll->setText(QString::fromUtf8("Close all"));

  actionTabbedView = new QAction(this);
  actionTabbedView->setObjectName(QString::fromUtf8("actionTabbedView"));
  actionTabbedView->setText(QString::fromUtf8("TabbedView"));
  actionTabbedView->setCheckable(true);
  actionTabbedView->setChecked(false);

  menuTabPosition = new QMenu(this);
  menuTabPosition->setObjectName(QString::fromUtf8("TabPosition"));
  menuTabPosition->setTitle(QString::fromUtf8("TabPosition"));
  menuTabPosition->setEnabled(false);

  groupTabPosition = new QActionGroup(this);
  actionTabPositionNorth = new QAction(this);
  actionTabPositionEast = new QAction(this);
  actionTabPositionSouth = new QAction(this);
  actionTabPositionWest = new QAction(this);
  actionTabPositionNorth->setObjectName(QString::fromUtf8("actionTabPositionNorth"));
  actionTabPositionNorth->setText(QString::fromUtf8("Top"));
  actionTabPositionNorth->setCheckable(true);
  actionTabPositionNorth->setChecked(true);
  actionTabPositionEast->setObjectName(QString::fromUtf8("actionTabPositionEast"));
  actionTabPositionEast->setText(QString::fromUtf8("Right"));
  actionTabPositionEast->setCheckable(true);
  actionTabPositionSouth->setObjectName(QString::fromUtf8("actionTabPositionSouth"));
  actionTabPositionSouth->setText(QString::fromUtf8("Bottom"));
  actionTabPositionSouth->setCheckable(true);
  actionTabPositionWest->setObjectName(QString::fromUtf8("actionTabPositionWest"));
  actionTabPositionWest->setText(QString::fromUtf8("Left"));
  actionTabPositionWest->setCheckable(true);

  groupTabPosition->addAction(actionTabPositionNorth);
  groupTabPosition->addAction(actionTabPositionEast);
  groupTabPosition->addAction(actionTabPositionSouth);
  groupTabPosition->addAction(actionTabPositionWest);

  menuTabPosition->addAction(actionTabPositionNorth);
  menuTabPosition->addAction(actionTabPositionEast);
  menuTabPosition->addAction(actionTabPositionSouth);
  menuTabPosition->addAction(actionTabPositionWest);

  actionTile = new QAction(this);
  actionTile->setObjectName(QString::fromUtf8("actionTile"));
  actionTile->setText(QString::fromUtf8("Tile"));

  actionCascade = new QAction(this);
  actionCascade->setObjectName(QString::fromUtf8("actionCascade"));
  actionCascade->setText(QString::fromUtf8("Cascade"));

  actionNext = new QAction(this);
  actionNext->setObjectName(QString::fromUtf8("actionNext"));
  actionNext->setText(QString::fromUtf8("Next"));
  actionNext->setShortcuts(QKeySequence::NextChild);
  actionNext->setIcon(QIcon(":/images/next.png"));
  actionNext->setIconText(actionNext->text());

  actionPrevious = new QAction(this);
  actionPrevious->setObjectName(QString::fromUtf8("actionPrevious"));
  actionPrevious->setText(QString::fromUtf8("Previous"));
  actionPrevious->setShortcuts(QKeySequence::PreviousChild);
  actionPrevious->setIcon(QIcon(":/images/previous.png"));
  actionPrevious->setIconText(actionPrevious->text());


  menuWindow = new QMenu(menuBar);
  menuWindow->setObjectName(QString::fromUtf8("menuWindow"));
  menuWindow->setTitle(QString::fromUtf8("Window"));
  menuBar->addMenu(menuWindow);
  updateMenuWindow();
  connect(menuWindow, SIGNAL(aboutToShow()), this, SLOT(updateMenuWindow()));

  actionAbout_Qt = new QAction(this);
  actionAbout_Qt->setObjectName(QString::fromUtf8("actionAbout_Qt"));
  actionAbout_Qt->setText(QString::fromUtf8("About Qt"));

  actionGKSQt_Help = new QAction(this);
  actionGKSQt_Help->setObjectName(QString::fromUtf8("actionGKSQt_Help"));
  actionGKSQt_Help->setText(QString::fromUtf8("GKSQt Help"));

  menuView = new  QMenu(menuBar);
  menuView->setObjectName(QString::fromUtf8("menuView"));
  menuView->setTitle(QString::fromUtf8("View"));
  menuBar->addMenu(menuView);

  menuHelp = new QMenu(menuBar);
  menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
  menuHelp->setTitle(QString::fromUtf8("Help"));
  menuBar->addMenu(menuHelp);
  menuHelp->addAction(actionAbout_Qt);
  menuHelp->addAction(actionGKSQt_Help);

  updateMenus();

  printer  = new QPrinter();
  savePath = QDir::currentPath();

  QMetaObject::connectSlotsByName(this);
}

void GKSQtWindow::openWindow()
{
  numWidgets++;

  GKSWidget *widget = new GKSWidget(NULL, Qt::Window);
  activeWidget = widget;

  widget->setAttribute(Qt::WA_DeleteOnClose);
  mdiArea->addSubWindow(widget, Qt::Window);

  widget->setWidgetNumber(numWidgets);
  widget->setLastWidgetNumber(numWidgets);
  widget->setRotation(rotation);
  widget->setRotateBy(rotateBy);
  widget->show();
}

void GKSQtWindow::interpret(char *dl)
{
  int sp = 0, *len;
  char *s;

  s = dl;

  RESOLVE(len, int, sizeof(int));

  activeWidget->setDisplayList(dl);
  activeWidget->show();
  activeWidget->repaint();
}

void GKSQtWindow::on_actionQuitGKSQt_triggered()
{
  qApp->closeAllWindows();
}

void GKSQtWindow::on_actionClose_triggered()
{
  mdiArea->closeActiveSubWindow();
}

void GKSQtWindow::on_actionCloseAll_triggered()
{
  mdiArea->closeAllSubWindows();
}

void GKSQtWindow::on_actionSave_As_triggered()
{
  activeWidget = qobject_cast<GKSWidget *>(mdiArea->activeSubWindow()->widget());
  if (activeWidget == NULL) {
    QMessageBox::warning(this, QString(tr("Save as ...")),
                         QString(tr("No data to save")));
    return;
  }

  QString caption = QString("Save as ... Filedialog");
  QFileDialog *fd = new QFileDialog(this, caption, savePath);
  fd->setFileMode(QFileDialog::AnyFile);
  fd->setAcceptMode(QFileDialog::AcceptSave);
  fd->setLabelText(QFileDialog::FileName, QString("Save As:"));
  fd->setLabelText(QFileDialog::FileType, QString("Save figure as:"));

  int i = 0;
  QStringList nameFilters;
  foreach (const QByteArray &sff, supportedFileFmtList)
    {
      ++i;
      nameFilters << sff;
    }

  fd->setNameFilters(nameFilters);

  fd->selectFile(QString("GKSQt"));
  fd->selectFile(QString("gksqt"));
  if (fd->exec() == QDialog::Accepted) {
    QString selType      = fd->selectedNameFilter();
    QStringList selFiles = fd->selectedFiles();
    if (!selFiles.isEmpty()) {
      savePath = fd->directory().dirName();
      SaveFileAs(selFiles.first()+QString(".")+selType);
    }
  }
}

void GKSQtWindow::on_actionPage_Setup_triggered()
{
  printer->setOutputFormat(QPrinter::NativeFormat);
  QPageSetupDialog *pgDialog = new QPageSetupDialog(printer, this);
  pgDialog->exec();
}

void GKSQtWindow::on_actionPrint_triggered()
{
  int rc;
  activeWidget = qobject_cast<GKSWidget *>(mdiArea->activeSubWindow()->widget());
  if (activeWidget == NULL) {
    QMessageBox::warning(this, QString(tr("print ...")),
                         QString(tr("No data to print")));
    return;
  }

  printer->setOutputFormat(QPrinter::NativeFormat);
  QPrintDialog *prtDialog = new QPrintDialog(printer,this);
  rc = prtDialog->exec();
  if (rc == QDialog::Accepted) {
    QPainter painter(printer);
    if (activeWidget != NULL) {
      int rot = activeWidget->getRotation();
      if (rot > 0) {
        qreal angle = rot * rotateBy;
        QMatrix matrix;
        qreal x = painter.window().width() / 2.0;
        qreal y = painter.window().height() / 2.0;
        matrix.translate(x, y);
        matrix.rotate(angle);
        matrix.translate(-x, -y);
        painter.setMatrix(matrix);
      }

      QPixmap *pm = new QPixmap(* (activeWidget->getPixmap()));
      painter.drawPixmap(10, 100, *pm);
    }
  }
}

void GKSQtWindow::on_actionCut_triggered()
{
}

void GKSQtWindow::on_actionCopy_triggered()
{
}

void GKSQtWindow::on_actionPaste_triggered()
{
}

void GKSQtWindow::on_actionKeep_on_Display_triggered()
{
  server->setKeepOnDisplay(actionKeep_on_Display->isChecked());
}

void GKSQtWindow::on_actionRotate_by_90_triggered()
{
  activeWidget = qobject_cast<GKSWidget *>(mdiArea->activeSubWindow()->widget());
  if (activeWidget != NULL) {
    activeWidget->rotate();
    activeWidget->update();
  }
}

void GKSQtWindow::on_actionSpecial_Characters_triggered()
{
  QStringList args;
  args << QString("-a") << QString("/System/Library/Input Methods/CharacterPalette.app");
  QProcess::execute(QString("open"), args);
}

void GKSQtWindow::on_actionMinimize_triggered()
{
  activeWidget = qobject_cast<GKSWidget *>(mdiArea->activeSubWindow()->widget());
  if (activeWidget != NULL) activeWidget->showMinimized();
}

void GKSQtWindow::on_actionTabbedView_triggered() {
  if (actionTabbedView->isChecked()) {
    mdiArea->setViewMode(QMdiArea::TabbedView);
  } else {
    mdiArea->setViewMode(QMdiArea::SubWindowView);
  }
  menuTabPosition->setEnabled(actionTabbedView->isChecked());
}

void  GKSQtWindow::on_actionTabPositionNorth_triggered()
{
  mdiArea->setTabPosition(QTabWidget::North);
}

void  GKSQtWindow::on_actionTabPositionEast_triggered()
{
  mdiArea->setTabPosition(QTabWidget::East);
}

void  GKSQtWindow::on_actionTabPositionSouth_triggered()
{
  mdiArea->setTabPosition(QTabWidget::South);
}

void  GKSQtWindow::on_actionTabPositionWest_triggered()
{
  mdiArea->setTabPosition(QTabWidget::West);
}

void GKSQtWindow::on_actionTile_triggered()
{
  mdiArea->tileSubWindows();
}

void GKSQtWindow::on_actionCascade_triggered()
{
  mdiArea->cascadeSubWindows();
}

void GKSQtWindow::on_actionNext_triggered()
{
  mdiArea->activateNextSubWindow();
}

void GKSQtWindow::on_actionPrevious_triggered()
{
  mdiArea->activatePreviousSubWindow();
}

void GKSQtWindow::on_actionAbout_Qt_triggered()
{
  QApplication::aboutQt();
}

void GKSQtWindow::on_actionGKSQt_Help_triggered()
{
}

void GKSQtWindow::SaveFileAs (const QString fname)
{
  activeWidget = qobject_cast<GKSWidget *>(mdiArea->activeSubWindow()->widget());

  if (activeWidget == NULL) {
    QMessageBox::warning(this, QString(tr("Save as ...")),
                         QString(tr("No data to save")));
    return;
  }

  QFileInfo *fi = new QFileInfo(fname);
  bool fmtOk = supportedFileFmtList.contains(fi->suffix().toLatin1());

  if (fmtOk) {
    bool ok = false;
    int quality = QInputDialog::getInt(this, QString("Enter quality"),
                                       QString("Quality [%1,%2]:").arg(1).arg(100), 100, 1, 100, 1, &ok);

    QPixmap *pm = new QPixmap(* (activeWidget->getPixmap()));
    pm->save(fname, 0, quality);
  } else {
    QMessageBox::warning(this, QString(tr("Save as ...")),
                         QString(tr("File format not supported: %1")).arg(fi->suffix()));
  }
}

GKSWidget::GKSWidget(QWidget *parent, Qt::WindowFlags f)
  : QWidget(parent,f)
{
  dl = NULL;

  p = &p_;
  p->width = p->height = 500;
  p->saved_width = p->width;
  p->saved_height = p->height;
  p->trans_x = p->trans_y = 0;

  first = true;
  create_window(p);

  layout = new QVBoxLayout(this);
  layout->setContentsMargins(0,0,0,0);
  sb = new QStatusBar;
  sb_label = new QLabel;
  sb_label->setAutoFillBackground(true);
  sb->setSizeGripEnabled(false);
  sb->setFixedHeight(20);
  sb->addWidget(sb_label);

  rubberBand = NULL;
  spacer = new QSpacerItem(p->width, p->height, QSizePolicy::MinimumExpanding,
                           QSizePolicy::MinimumExpanding);

  layout->addSpacerItem(spacer);
  layout->addWidget(sb);
  setMouseTracking(true);
  setLayout(layout);

  rotation = 0;
  rotateBy = 90.0;
  widgetNumber = 1;
}

GKSWidget::~GKSWidget() {
}

void GKSWidget::setWidgetNumber(int number)
{
  widgetNumber = number;
  this->setWindowTitle(QString("GKS Window %1").arg(widgetNumber));
}

void GKSWidget::setLastWidgetNumber(int number)
{
  lastWidgetNumber = number;
}

int GKSWidget::getWidgetNumber()
{
  return widgetNumber;
}

int GKSWidget::getLastWidgetNumber()
{
  return lastWidgetNumber;
}

QPixmap* GKSWidget::getPixmap()
{
  return pm;
}

void GKSWidget::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  activeWidget = this;

  if (dl)
    {
      if (rotation > 0) {
        qreal angle = rotation * rotateBy;
        QMatrix matrix;
        qreal x = painter.window().width() / 2.0;
        qreal y = painter.window().height() / 2.0;
        matrix.translate(x, y);
        matrix.rotate(angle);
        matrix.translate(-x, -y);
        painter.setMatrix(matrix);
      }

      if (first) {
        p->pm->fill(Qt::white);
        interp(dl);
        pm = new QPixmap(* (p->pm));
        first = false;
      } else {
        if (widgetNumber == GKSWidget::getLastWidgetNumber()) {
          p->pm->fill(Qt::white);
          interp(dl);
          pm = new QPixmap(* (p->pm));
        } else {
        }
      }

      painter.translate(p->trans_x, p->trans_y);
      if (p->selecting) {
        painter.drawPixmap(QPoint(0, 0), *pm);
      }
      else {
        painter.setClipRect(0, 0, p->width, p->height);
        painter.drawPixmap(QPoint(0, 0), *pm);
      }
    }
}

void GKSWidget::resizeEvent(QResizeEvent *e)
{
  double fac;

  p->selecting = false;

  fac = min((double) max(1., this->spacer->geometry().width()) / p->width,
            (double) max(1., this->spacer->geometry().height()) / p->width);
  p->width = nint(p->width * fac);
  p->height = nint(p->height * fac);
  p->saved_width = p->width;
  p->saved_height = p->height;

  delete p->pixmap;
  delete p->pm;

  p->pm = new QPixmap(p->width, p->height);
  p->pm->fill(Qt::white);

  p->pixmap = new QPainter(p->pm);
  p->pixmap->setClipRect(0, 0, p->width, p->height);

  p->viewport[0] = 0;
  p->viewport[1] = p->width * 2.54 / activeWidget->physicalDpiX() * 0.01;
  p->viewport[2] = 0;
  p->viewport[3] = p->height * 2.54 / activeWidget->physicalDpiY() * 0.01;

  resize_window();
  set_xform();

  p->trans_x = (this->spacer->geometry().width() - p->pm->width()) / 2;
  p->trans_y = (this->spacer->geometry().height() - p->pm->height()) / 2;
}

void GKSWidget::mouseMoveEvent(QMouseEvent *e) {
  double x, xf;
  double y, yf;
  int xi = e->pos().x() - p->trans_x;
  int yi = e->pos().y() - p->trans_y;

  DC_to_NDC(xi, yi, x, y);
  NDC_to_WC(x, y, 1, xf, yf);

  this->sb_label->setText(QString(tr("(%1, %2)")).arg(xf).arg(yf));
  if (e->buttons() == Qt::LeftButton) {
    rubberBand->setGeometry(QRect(p->start, e->pos()).normalized());
    update();
  }
}

void GKSWidget::mousePressEvent(QMouseEvent *e) {
  if (p->selecting) {
    p->selecting = false;
    p->width = p->saved_width;
    p->height = p->saved_height;

    p->viewport[0] = 0;
    p->viewport[1] = p->width * 2.54 / activeWidget->physicalDpiX() * 0.01;
    p->viewport[2] = 0;
    p->viewport[3] = p->height * 2.54 / activeWidget->physicalDpiY() * 0.01;

    resize_window();
    set_xform();

    delete p->pixmap;
    delete p->pm;

    p->pm = new QPixmap(p->width, p->height);
    p->pm->fill(Qt::white);

    p->pixmap = new QPainter(p->pm);
    p->pixmap->setClipRect(0, 0, p->width, p->height);
  }

  if (e->buttons() == Qt::LeftButton) {
    leftButton = true;
    p->start = e->pos();
    if (!rubberBand)
      rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    rubberBand->setGeometry(QRect(p->start, QSize()));
    rubberBand->show();
  }
  if (e->buttons() == Qt::RightButton) {
    leftButton = false;
  }
}

void GKSWidget::mouseReleaseEvent(QMouseEvent *e) {
  double fac;

  if (leftButton) {
    p->selecting = false;

    p->area = QRect(0, 0, p->width, p->height).intersected(rubberBand->geometry());
    p->have_selection = true;
    p->area.translate(-p->trans_x, -p->trans_y);

    if (!(p->area.isNull())) {
      fac = min((double) p->width / max(1, p->area.width()),
                (double) p->height / max(1, p->area.height()));

      p->saved_width = p->width;
      p->saved_height = p->height;
      p->width = nint(p->width * fac);
      p->height = nint(p->height * fac);

      rubberBand->hide();
      p->area.setRect(nint(p->area.x() * fac), nint(p->area.y() * fac),
                      nint(p->area.width() * fac), nint(p->area.height() * fac));

      p->pixmap->setClipRect(p->area);
      p->pixmap->translate(-p->area.left(), -p->area.top());

      p->viewport[0] = 0;
      p->viewport[1] = p->width * 2.54 / activeWidget->physicalDpiX() * 0.01;
      p->viewport[2] = 0;
      p->viewport[3] = p->height * 2.54 / activeWidget->physicalDpiY() * 0.01;

      resize_window();
      set_xform();
    }
    this->repaint();
  }
}

void GKSWidget::rotate()
{
  rotation += 1;
  if (rotation >= qRound(360.0/rotateBy)) rotation = 0;
}

void GKSWidget::setRotation(const int val)
{
  rotation = val;
}

int GKSWidget::getRotation() {
  return rotation;
}

void GKSWidget::setRotateBy(const qreal val)
{
  rotateBy = val;
}

void GKSWidget::setDisplayList(char *val)
{
  dl = val;
}
