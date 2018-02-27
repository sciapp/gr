#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <libpng16/png.h>

typedef struct JS_point_t
{
  double x, y;
} JS_point;

#ifdef __cplusplus
#define extern "C" {
#endif

extern void js_stroke(int n, JS_point *points, unsigned char color[4], int linewidth);
extern void js_text(double x, double y, int n, char *chars, int height, double top, double angle, int bold, int italic, int align, int valign, int font, unsigned char rgb[4]);
extern void js_cellarray(int x, int y, int width, int height, int colia[]);
extern void js_line_routine(int n, double *px, double *py, int linetype, int fill, int width, unsigned char *rgb);
extern void js_point(double x, double y, unsigned char *rgb);
extern void js_line(double x1, double y1, double x2, double y2, unsigned char *rgb);
extern void js_circle(double x, double y, double r, int fill, unsigned char *rgb);
extern void js_fill_routine(int n, double *px, double *py, unsigned char *color);
extern void js_pattern_routine(int n, double *px, double *py, int *rgb);
extern void js_clip_path(int x, int y, int width, int height);
extern void js_reset_clipping(void);
extern void js_clear(void);

#ifdef __cplusplus
#define }
#endif

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
  int conid, state, wtype;
  double a, b, c, d;
  double window[4], viewport[4];
  unsigned char rgb[MAX_COLOR][4];
  int width, height;
  int color, linewidth;
  double angle;
  int family, capheight;
  int pattern, have_pattern[PATTERNS];
  JS_point *points;
  int npoints, max_points;
  int empty, page_counter, offset;
  double cxl[MAX_TNR], cxr[MAX_TNR], cyb[MAX_TNR], cyt[MAX_TNR];
  int cx[MAX_TNR], cy[MAX_TNR], cwidth[MAX_TNR], cheight[MAX_TNR];
  int clip_index, path_index, path_counter;
  double alpha;
}
ws_state_list;

static
ws_state_list *p;

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


static void set_clip_path(int tnr);

static
void set_norm_xform(int tnr, double *wn, double *vp)
{
  a[tnr] = (vp[1] - vp[0]) / (wn[1] - wn[0]);
  b[tnr] = vp[0] - wn[0] * a[tnr];
  c[tnr] = (vp[3] - vp[2]) / (wn[3] - wn[2]);
  d[tnr] = vp[2] - wn[2] * c[tnr];
    
  NDC_to_DC(vp[0], vp[3], p->cxl[tnr], p->cyt[tnr]);
  NDC_to_DC(vp[1], vp[2], p->cxr[tnr], p->cyb[tnr]);
  p->cxr[tnr] += 1;
  p->cyb[tnr] += 1;
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
      p->rgb[color][0]=red*255;
      p->rgb[color][1]=green*255;
      p->rgb[color][2]=blue*255;
      p->rgb[color][3]=255;
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
void init_clippaths(void)
{
  p->path_counter = 0;
  p->clip_index = 0;
  for (int i = 0; i < MAX_TNR; i++)
    {
      p->cx[i] = p->cy[i] = -1;
      p->cwidth[i] = p->cheight[i] = 0;
    }
  set_clip_path(0);
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
  int r, i;
  double scale, x, y, xr, yr;
  int pc, op, fill;
  double x1, x2, y1, y2;
  double *px, *py;
  int color;
    
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
    
  do
    {
      op = marker[mtype][pc];
      switch (op)
        {
        case 1:		/* point */
          js_point(x, y, p->rgb[mcolor]);
          break;
                
        case 2:		/* line */
          for (i = 0; i < 2; i++)
            {
              xr = scale * marker[mtype][pc + 2 * i + 1];
              yr = -scale * marker[mtype][pc + 2 * i + 2];
              seg_xform_rel(&xr, &yr);
              if (i == 0){
                x1 = x - xr;
                y1 = y + yr;
              }
              else{
                x2 = x - xr;
                y2 = y + yr;
              }
            }
          pc += 4;
          js_line(x1, y1, x2, y2, p->rgb[mcolor]);
          break;
                
        case 3:		/* polyline */
          px = malloc(marker[mtype][pc + 1]*sizeof(double));
          py = malloc(marker[mtype][pc + 1]*sizeof(double));
          for (i = 0; i < marker[mtype][pc + 1]; i++)
            {
              xr = scale * marker[mtype][pc + 2 + 2 * i];
              yr = -scale * marker[mtype][pc + 3 + 2 * i];
              seg_xform_rel(&xr, &yr);
              px[i] = x - xr;
              py[i] = y + yr;
            }
          js_line_routine(marker[mtype][pc + 1], px, py, 1, 0, 1, p->rgb[mcolor]);
          pc += 1 + 2 * marker[mtype][pc + 1];
          free(px);
          free(py);
          break;
                
        case 4:		/* filled polygon */
        case 5:		/* hollow polygon */
          if (op == 5){
            fill =0;
          }
          else{
            fill=1;
          }
          px= malloc(marker[mtype][pc + 1]*sizeof(double));
          py= malloc(marker[mtype][pc + 1]*sizeof(double));
          for (i = 0; i < marker[mtype][pc + 1]; i++)
            {
              xr = scale * marker[mtype][pc + 2 + 2 * i];
              yr = -scale * marker[mtype][pc + 3 + 2 * i];
              seg_xform_rel(&xr, &yr);
              px[i] = x-xr;
              py[i] = y+yr;
            }
          js_line_routine(marker[mtype][pc + 1], px, py, 0, fill, 1, p->rgb[mcolor]);
          free(px);
          free(py);
          pc += 1 + 2 * marker[mtype][pc + 1];
          if (op == 5)
            p->color = mcolor;
          break;
                
        case 6:		/* arc */
          js_circle(x, y, r, 0, p->rgb[mcolor]);
          break;
                
        case 7:		/* filled arc */
        case 8:		/* hollow arc */
          if (op == 8){
            js_circle(x, y, r, 0, p->rgb[mcolor]);
          }
          else{
            js_circle(x, y, r, 1, p->rgb[mcolor]);
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
  register int i, draw;
    
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
  unsigned char color[4];
  color[0] = p->rgb[p->color][0];
  color[1] = p->rgb[p->color][1];
  color[2] = p->rgb[p->color][2];
  color[3] = p->alpha;
    
  js_stroke(p->npoints,p->points, color, p->linewidth);
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
  unsigned char color[4];
  color[0] = p->rgb[p->color][0];
  color[1] = p->rgb[p->color][1];
  color[2] = p->rgb[p->color][2];
  color[3] = p->alpha;
    
  js_line_routine(n, px, py, linetype, 0, 1, color);
    
}

static
void line_routine_with_transform(int n, double *px, double *py, int linetype, int tnr)
{
  double x, y, *ix, *iy;
  
  ix = malloc(n*sizeof(double));
  iy = malloc(n*sizeof(double));
  for (int i = 0; i < n; i++)
    {
      WC_to_NDC(px[i], py[i], gkss->cntnr, x, y);
      seg_xform(&x, &y);
      NDC_to_DC(x, y, ix[i], iy[i]);
    }
  line_routine(n, ix, iy, linetype, tnr);
  free(ix);
  free(iy);
}

static
void fill_routine(int n, double *px, double *py, int tnr)
{
  unsigned char color[4];
  
  color[0] = p->rgb[p->color][0];
  color[1] = p->rgb[p->color][1];
  color[2] = p->rgb[p->color][2];
  color[3] = p->alpha;
  js_fill_routine(n, px, py, color);
}

static
void fillarea(int n, double *px, double *py)
{
  int fl_inter, fl_style, fl_color, col, col2;
  unsigned char color[4];
  double x, y, *ix, *iy;
  int parray[33];
  int *patp;
    
  fl_inter = gkss->asf[10] ? gkss->ints : predef_ints[gkss->findex - 1];
  fl_style = gkss->asf[11] ? gkss->styli : predef_styli[gkss->findex - 1];
  fl_color = gkss->asf[12] ? gkss->facoli : 1;
    
  p->pattern = 0;
  if (fl_inter == GKS_K_INTSTYLE_HOLLOW)
    {
      p->color = fl_color;
      ix = malloc(n*sizeof(double));
      iy = malloc(n*sizeof(double));
      for (int i = 0; i < n; i++)
        {
          WC_to_NDC(px[i], py[i], gkss->cntnr, x, y);
          seg_xform(&x, &y);
          NDC_to_DC(x, y, ix[i], iy[i]);
        }
      color[0] = p->rgb[p->color][0];
      color[1] = p->rgb[p->color][1];
      color[2] = p->rgb[p->color][2];
      color[3] = p->alpha;
      js_line_routine(n, ix, iy, 0, 0, 1, color);
      free(ix);
      free(iy);
    }
  else if (fl_inter == GKS_K_INTSTYLE_SOLID)
    {
      p->color = fl_color;
      color[0] = p->rgb[p->color][0];
      color[1] = p->rgb[p->color][1];
      color[2] = p->rgb[p->color][2];
      color[3] = p->alpha;
      ix = malloc(n*sizeof(double));
      iy = malloc(n*sizeof(double));
      for (int i = 0; i < n; i++)
        {
          WC_to_NDC(px[i], py[i], gkss->cntnr, x, y);
          seg_xform(&x, &y);
          NDC_to_DC(x, y, ix[i], iy[i]);
        }
      js_line_routine(n, ix, iy, 0, 1, 1, color);
      free(ix);
      free(iy);
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
      gks_inq_pattern_array(fl_style, parray);
      col =  (255 << 0) + (255 << 8) + (255 << 16)+ (255 << 24);
      col2 = (0 << 0) + (0<< 8) + (0 << 16)+ (255 << 24);
      patp = malloc(64*sizeof(int));
      for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
          patp[i*8+j]= (((parray[(j % parray[0]) + 1] >> i) & 0x01) ? col : col2);
      ix = malloc(n*sizeof(double));
      iy = malloc(n*sizeof(double));
      for (int i = 0; i < n; i++)
        {
          WC_to_NDC(px[i], py[i], gkss->cntnr, x, y);
          seg_xform(&x, &y);
          NDC_to_DC(x, y, ix[i], iy[i]);
        }
      js_pattern_routine(n, ix, iy, patp);
      free(ix);
      free(iy);
      free(patp);
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
      p->points = (JS_point *) realloc(p->points, n * sizeof(JS_point));
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
    
  gks_set_dev_xform(gkss, p->window, p->viewport);
  line_routine_with_transform(n, px, py, ln_type, gkss->cntnr);
    
  if (p->npoints > 0)
    stroke();
    
}

static
void text(double px, double py, int nchars, char *chars)
{
  int tx_font, tx_prec, tx_color;
  double x, y, xstart, ystart;
  int ch, font;
  double xrel, yrel, ax, ay;
    
  double scale, ux, uy, angle;
  int size;
  double capheight, width, height;
  int bold, italic, boit;
    
  tx_font = gkss->asf[6] ? gkss->txfont : predef_font[gkss->tindex - 1];
  tx_prec = gkss->asf[6] ? gkss->txprec : predef_prec[gkss->tindex - 1];
  tx_color = gkss->asf[9] ? gkss->txcoli : 1;
    
  p->color = tx_color;
    
  if (tx_prec == GKS_K_TEXT_PRECISION_STRING)
    {
      font = tx_font;
        
      font = abs(font);
      if (font >= 101 && font <= 129)
        font -= 100;
      else if (font >= 1 && font <= 32)
        font = map[font - 1];
      else
        font = 9;
        
      WC_to_NDC_rel(gkss->chup[0], gkss->chup[1], gkss->cntnr, ux, uy);
      seg_xform_rel(&ux, &uy);
        
      angle = -atan2(ux, uy) * 180 / M_PI;
      if (angle < 0)
        angle += 360;
      p->angle = -angle;
        
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
        
        
      WC_to_NDC(px, py, gkss->cntnr, x, y);
      seg_xform(&x, &y);
      NDC_to_DC(x, y, xstart, ystart);
        
      width = 0;
      height = p->capheight;
        
      if (gkss->txal[0] == GKS_K_TEXT_HALIGN_CENTER){
        js_text(xstart, ystart, nchars, chars, size, gkss->chh, p->angle, bold, italic, 1, gkss->txal[1], p->family, p->rgb[p->color]);
      }
      else if (gkss->txal[0] == GKS_K_TEXT_HALIGN_RIGHT){
        js_text(xstart, ystart, nchars, chars, size, gkss->chh, p->angle, bold, italic, 2, gkss->txal[1], p->family, p->rgb[p->color]);
      }
      else{
        js_text(xstart, ystart, nchars, chars, size, gkss->chh, p->angle, bold, italic, 0, gkss->txal[1], p->family, p->rgb[p->color]);
      }
    }
  else
    gks_emul_text(px, py, nchars, chars, line_routine_with_transform, fill_routine);
}

static
void cellarray(double xmin, double xmax, double ymin, double ymax,
               int dx, int dy, int dimx, int *colia, int true_color)
{
  double x1, y1, x2, y2;
  int ix1, ix2, iy1, iy2;
  int x, y, width, height;
  register int i, j, ix, iy, ind;
  int swapx, swapy, *tmpptr;
    
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
    
  tmpptr = (int *) gks_malloc(dx * dy * sizeof(int));
  for (i = 0; i < dx; i++)
    for (j = 0; j < dy; j++)
      {
        ix = swapx ? dx - i - 1 : i;
        iy = swapy ? dy - j - 1 : j;
        tmpptr[iy * dx + ix] = colia[j * dimx + i];
      }
    
  if (dx != width || dy != height)
    {
      colia = gks_resize(tmpptr, dx, dy, width, height);
      free(tmpptr);
    }
  else
    colia = tmpptr;
    
  if (!true_color)
    {
      for (i = 0; i < width * height; i++)
        {
          ind = colia[i];
          if (ind >= MAX_COLOR)
            ind = MAX_COLOR - 1;
          colia[i] =  (int)(p->rgb[ind][0] << 0) +
            ((int)(p->rgb[ind][1]) << 8) +
            ((int)(p->rgb[ind][2]) << 16)+
            ((int)(p->rgb[ind][3]) << 24);
        }
    }
  js_cellarray(x, y, width, height, colia);
  free(colia);
}

static
void set_clip_path(int tnr)
{
  int x, y, width, height;
  int i, found = 0, index;
    
  if (gkss->clip == GKS_K_CLIP)
    {
      x = (int) p->cxl[tnr];
      y = (int) p->cyt[tnr];
      width = (int) (p->cxr[tnr] - p->cxl[tnr]);
      height = (int) (p->cyb[tnr] - p->cyt[tnr]);
    }
  else
    {
      js_reset_clipping();
      return;
    }
    
  for (i = 0; i < p->clip_index && !found; i++)
    {
      if (x == p->cx[i] && y == p->cy[i] &&
          width == p->cwidth[i] && height == p->cheight[i])
        {
          found = 1;
          index = i;
        }
    }
  if (found)
    {
      p->path_index = index;
    }
  else
    {
      if (p->clip_index < MAX_TNR)
        {
          p->cx[p->clip_index] = x;
          p->cy[p->clip_index] = y;
          p->cwidth[p->clip_index] = width;
          p->cheight[p->clip_index] = height;
          p->path_index = p->clip_index;
            
          js_clip_path(x, y, width, height);
          p->clip_index++;
        }
      else
        {
          js_clip_path(x, y, width, height);
          p->path_index = p->path_counter++;
        }
    }
}


void gks_drv_js(int fctid, int dx, int dy, int dimx, int *ia,
                int lr1, double *r1, int lr2, double *r2,
                int lc, char *chars, void **ptr)
{
  register int i;
    
  p = (ws_state_list *) *ptr;
    
  switch (fctid)
    {
      /* open workstation */
    case 2:
      gkss = (gks_state_list_t *) * ptr;
            
      gks_init_core(gkss);
            
      p = (ws_state_list *) calloc(1, sizeof(ws_state_list));
            
      p->conid = ia[1];
            
      p->height = 500;
      p->width = 500;
      p->window[0] = p->window[2] = 0.0;
      p->window[1] = p->window[3] = 1.0;
      p->viewport[0] = p->viewport[2] = 0;
      p->viewport[1] = (double) p->width * MWIDTH / WIDTH;
      p->viewport[3] = (double) p->height * MHEIGHT / HEIGHT;
            
      p->max_points = MAX_POINTS;
      p->points = (JS_point *) gks_malloc(p->max_points * sizeof(JS_point));
      p->npoints = 0;
            
      p->empty = 1;
      p->page_counter = 0;
      p->offset = 0;
            
      p->alpha = 1.0;
            
      set_xform();
      init_norm_xform();
      init_colors();
      init_clippaths();
            
      for (i = 0; i < PATTERNS; i++)
        p->have_pattern[i] = 0;
            
      *ptr = p;
      break;
            
      /* close workstation */
    case 3:
      if (!p->empty)
            
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
          js_clear();
          init_clippaths();
        }
      break;
            
      /* update workstation */
    case 8:
      break;
            
      /* polyline */
    case 12:
      if (p->state == GKS_K_WS_ACTIVE)
        {
          polyline(ia[0], r1, r2);
          p->empty = 0;
        }
      break;
            
      /* polymarker */
    case 13:
      if (p->state == GKS_K_WS_ACTIVE)
        {
          polymarker(ia[0], r1, r2);
          p->empty = 0;
        }
      break;
            
      /* text */
    case 14:
      if (p->state == GKS_K_WS_ACTIVE)
        {
          text(r1[0], r2[0], strlen(chars), chars);
          p->empty = 0;
        }
      break;
            
      /* fill area */
    case 15:
      if (p->state == GKS_K_WS_ACTIVE)
        {
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
      if (*ia == gkss->cntnr)
        set_clip_path(*ia);
      break;
            
    case 52:
      /* select normalization transformation */
    case 53:
      /* set clipping inidicator */
      set_clip_path(gkss->cntnr);
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
            
    case 203:
      /* set transparency */
      p->alpha = r1[0];
      break;
            
    default:
      ;
    }
}
