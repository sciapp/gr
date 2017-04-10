
#include <stdio.h>

#if defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1050

#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include <Carbon/Carbon.h>

#include "gks.h"
#include "gkscore.h"

#define PATTERNS 120
#define HATCH_STYLE 108

#define MWIDTH 0.381

#define DrawBorder 0

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#define WC_to_NDC(xw, yw, tnr, xn, yn) \
  xn = a[tnr] * (xw) + b[tnr]; \
  yn = c[tnr] * (yw) + d[tnr]

#define WC_to_NDC_rel(xw, yw, tnr, xn, yn) \
  xn = a[tnr] * (xw); \
  yn = c[tnr] * (yw)

#define NDC_to_DC(xn, yn, xd, yd) \
  xd = p->a * (xn) + p->b; \
  yd = p->c * (yn) + p->d

#define DC_to_NDC(xd, yd, xn, yn) \
    xn = ((xd) - p->b) / p->a; \
    yn = ((yd) - p->d) / p->c

#define CharXform(xrel, yrel, x, y) \
  x = cos_f[p->path] * (xrel) - sin_f[p->path] * (yrel); \
  y = sin_f[p->path] * (xrel) + cos_f[p->path] * (yrel);

#define nint(a) ((int)(a + 0.5))

typedef struct ws_state_list_t
  {
    pthread_t thread;
    pthread_mutex_t mutex;
    WindowRef win;
    GrafPtr port;
    int run;
    int state, wtype;
    int width, height;
    int swidth, sheight;
    double a, b, c, d;
    double window[4], viewport[4];
    Rect rect[MAX_TNR];
    RGBColor rgb[MAX_COLOR];
    int family, path, capheight;
    Pattern pattern[PATTERNS], *pat;
  }
ws_state_list;

static
ws_state_list *p;

static
char *fonts[] = {
  "Times", "Helvetica", "Courier", "Symbol",
  "Bookman Old Style", "Century Schoolbook", "Century Gothic", "Book Antiqua" };

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
int iso2mac[256] = {
    0,   1,   2,   3,   4,   5,   6,   7,
    8,   9,  10,  11,  12,  13,  14,  15,
   16,  17,  18,  19,  20,  21,  22,  23,
   24,  25,  26,  27,  28,  29,  30,  31,
   32,  33,  34,  35,  36,  37,  38,  39,
   40,  41,  42,  43,  44,  45,  46,  47,
   48,  49,  50,  51,  52,  53,  54,  55,
   56,  57,  58,  59,  60,  61,  62,  63,
   64,  65,  66,  67,  68,  69,  70,  71,
   72,  73,  74,  75,  76,  77,  78,  79,
   80,  81,  82,  83,  84,  85,  86,  87,
   88,  89,  90,  91,  92,  93,  94,  95,
   96,  97,  98,  99, 100, 101, 102, 103,
  104, 105, 106, 107, 108, 109, 110, 111,
  112, 113, 114, 115, 116, 117, 118, 119,
  120, 121, 122, 123, 124, 125, 126, 127,
  128, 129, 130, 131, 132, 133, 134, 135,
  136, 137, 138, 139, 140, 141, 142, 143,
  144, 145, 146, 147, 148, 149, 150, 151,
  152, 153, 154, 155, 156, 157, 158, 159,
  202, 193, 162, 163,  32, 180,  32, 164,
  172, 169, 187, 199, 194,  32, 168, 248,
  161, 177,  32,  32, 171, 181, 166, 225,
  252,  32, 188, 200,  32,  32,  32, 192,
  203, 231, 229, 204, 128, 129, 174, 130,
  233, 131, 230, 232, 237, 234, 235, 236,
   32, 132, 241, 238, 239, 205, 133,  32,
  175, 244, 242, 243, 134,  32,  32, 167,
  136, 135, 137, 139, 138, 140, 190, 141,
  143, 142, 144, 145, 147, 146, 148, 149,
   32, 150, 152, 151, 153, 155, 154, 214,
  191, 157, 156, 158, 159,  32,  32, 216 };

static
gks_state_list_t *gkss;

static
double a[MAX_TNR], b[MAX_TNR], c[MAX_TNR], d[MAX_TNR];

static
double xfac[4] = { 0, 0, -0.5, -1 };

static
double yfac[6] = { 0, -1.2, -1, -0.5, 0, 0.2 };

static
double sin_f[] = { 0, 1, 0, -1 };

static
double cos_f[] = { 1, 0, -1, 0 };

static
int map[32] = {
  22,  9,  5, 14, 18, 26, 13,  1,
  24, 11,  7, 16, 20, 28, 13,  3,
  23, 10,  6, 15, 19, 27, 13,  2,
  25, 12,  8, 17, 21, 29, 13,  4 };

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
  Rect *rect = &p->rect[tnr];

  a[tnr] = (vp[1] - vp[0]) / (wn[1] - wn[0]);
  b[tnr] = vp[0] - wn[0] * a[tnr];
  c[tnr] = (vp[3] - vp[2]) / (wn[3] - wn[2]);
  d[tnr] = vp[2] - wn[2] * c[tnr];

  NDC_to_DC(vp[0], vp[3], rect->left, rect->top);
  NDC_to_DC(vp[1], vp[2], rect->right, rect->bottom);
  rect->right  += 1;
  rect->bottom += 1;
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
void set_color_rep(int color, double red, double green, double blue)
{
  if (color >= 0 && color < MAX_COLOR)
    {
      p->rgb[color].red = nint(red * 65535);
      p->rgb[color].green = nint(green * 65535);
      p->rgb[color].blue = nint(blue * 65535);
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
void set_color(int color)
{
  RGBForeColor(&p->rgb[color]);
}

static
void create_patterns(void)
{
  int i, j, height;
  int pattern, parray[33];

  for (i = 0; i < PATTERNS; i++)
    {
      pattern = i;
      gks_inq_pattern_array(pattern, parray);
      height = (*parray == 32) ? 16 : (*parray == 4) ? 8 : *parray;
      for (j = 0; j < height; j++)
        p->pattern[i].pat[height - j - 1] = ~parray[(j % *parray) + 1];
    }
}

static
void set_clip_rect(int tnr)
{
  Rect *rect;

  if (gkss->clip == GKS_K_CLIP)
    rect = p->rect + tnr;
  else
    rect = p->rect;

  ClipRect(rect);
}

static
void clear_ws(void)
{
  Rect rect;

  set_clip_rect(0);
  set_color(0);

  GetWindowPortBounds(p->win, &rect);
  PaintRect(&rect);

  set_clip_rect(gkss->cntnr);
}

static
void create_window(ws_state_list *p)
{
  Rect screenRect, wRect;
  CGrafPtr screenPort;

  screenPort = CreateNewPort();
  GetPortBounds(screenPort, &screenRect);
  DisposePort(screenPort);

  p->width = p->height = 500;
  p->swidth = screenRect.right - screenRect.left;
  p->sheight = screenRect.bottom - screenRect.top;

  p->window[0] = p->window[2] = 0.0;
  p->window[1] = p->window[3] = 1.0;
  p->viewport[0] = p->viewport[2] = 0;
  p->viewport[1] = (double)p->width * MWIDTH / p->swidth;
  p->viewport[3] = (double)p->height * MWIDTH / p->sheight;

  SetRect(&wRect, 0, 0, p->width, p->height);
  CreateNewWindow(
    kDocumentWindowClass,
    kWindowStandardDocumentAttributes | kWindowStandardHandlerAttribute,
    &wRect, &p->win);

  SetWindowTitleWithCFString(p->win, CFSTR("GKS 5"));
  ChangeWindowAttributes(p->win, 0,
                         kWindowCloseBoxAttribute | kWindowResizableAttribute);
  RepositionWindow(p->win, NULL, kWindowCascadeOnMainScreen);
  ShowWindow(p->win);

  GetWindowPortBounds(p->win, &wRect);
  ClipRect(&wRect);

  set_xform();
}

static
void resize_window(void)
{
  double max_width, max_height;
  int width, height;
  Rect wRect;

  max_width = MWIDTH;
  max_height = max_width * p->sheight / p->swidth;

  gks_fit_ws_viewport(p->viewport, max_width, max_height, 0.0075);
  width = nint((p->viewport[1] - p->viewport[0]) / max_width * p->swidth);
  height = nint((p->viewport[3] - p->viewport[2]) / max_height * p->sheight);

  if (p->width != width || p->height != height)
    {
      p->width = width;
      p->height = height;

      SizeWindow(p->win, width, height, TRUE);

      GetWindowPortBounds(p->win, &wRect);
      ClipRect(&wRect);
    }
}

static
void *exec(void *arg)
{
  ws_state_list *p = (ws_state_list *) arg;
  EventRecord event;

  create_window(p);

  init_norm_xform();
  init_colors();
  create_patterns();

  p->run = 1;
  while (p->run)
    {
      pthread_mutex_lock(&p->mutex);
      while (EventAvail(everyEvent, &event))
	{
	  GetNextEvent(everyEvent, &event);
	  if (event.what == kHighLevelEvent)
	    AEProcessAppleEvent(&event);
	}
      pthread_mutex_unlock(&p->mutex);
      usleep(10000);
    }
  DisposeWindow(p->win);

  pthread_exit(0);

  return 0;
}

static
void move_to(double x, double y)
{
  int ix, iy;

  NDC_to_DC(x, y, ix, iy);
  MoveTo(ix, iy);
}

static
void line_to(double x, double y)
{
  int ix, iy;

  NDC_to_DC(x, y, ix, iy);
  LineTo(ix, iy);
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
  int i, x0, y0, xi, yi, xim1, yim1;

  WC_to_NDC(px[0], py[0], tnr, x, y);
  seg_xform(&x, &y);
  NDC_to_DC(x, y, x0, y0);

  MoveTo(x0, y0);
  xim1 = x0;
  yim1 = y0;
  for (i = 1; i < n; i++)
    {
      WC_to_NDC(px[i], py[i], tnr, x, y);
      seg_xform(&x, &y);
      NDC_to_DC(x, y, xi, yi);

      if (i == 1 || xi != xim1 || yi != yim1)
	{
	  Line(xi - xim1, yi - yim1);
	  xim1 = xi;
	  yim1 = yi;
	}
    }
  if (linetype == 0)
    LineTo(x0, y0);
}

static
void polyline(int n, double *px, double *py)
{
  int ln_type, ln_color;
  double ln_width;
  int width;

  ln_type  = gkss->asf[0] ? gkss->ltype  : gkss->lindex;
  ln_width = gkss->asf[1] ? gkss->lwidth : 1;
  ln_color = gkss->asf[2] ? gkss->plcoli : 1;

  if (gkss->version > 4)
    width = nint(ln_width * (p->width + p->height) * 0.001);
  else
    width = nint(ln_width);
  if (width < 1)
    width = 1;

  PenSize(width, width);
  set_color(ln_color);

  gks_set_dev_xform(gkss, p->window, p->viewport);
  gks_emul_polyline(n, px, py, ln_type, gkss->cntnr, move, draw);
}

static
void draw_marker(double xn, double yn, int mtype, double mscale, int mcolor)
{
  int r, x, y, i;
  double scale, xr, yr;
  int pc, op;
  PolyHandle poly;
  Rect rect;

#include "marker.h"

  if (gkss->version > 4)
    mscale *= (p->width + p->height) * 0.001;
  r = (int)(3 * mscale);
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
      case 1: /* point */
	MoveTo(x, y);
	LineTo(x, y);
	break;

      case 2: /* line */
	for (i = 0; i < 2; i++)
        {
          xr =  scale * marker[mtype][pc + 2 * i + 1];
          yr = -scale * marker[mtype][pc + 2 * i + 2];
          seg_xform_rel(&xr, &yr);
          if (i == 0)
	    MoveTo(x - xr, y + yr);
	  else
	    LineTo(x - xr, y + yr);
        }
	pc += 4;
	break;

      case 3: /* polyline */
	for (i = 0; i < marker[mtype][pc + 1]; i++)
	{
	  xr =  scale * marker[mtype][pc + 2 + 2 * i];
	  yr = -scale * marker[mtype][pc + 3 + 2 * i];
          seg_xform_rel(&xr, &yr);
          if (i == 0)
	    MoveTo(x - xr, y + yr);
	  else
	    LineTo(x - xr, y + yr);
	}
	pc += 1 + 2 * marker[mtype][pc + 1];
	break;

      case 4: /* filled polygon */
      case 5: /* hollow polygon */
	poly = OpenPoly();
	if (op == 5)
	  set_color(0);
	for (i = 0; i < marker[mtype][pc + 1]; i++)
	{
	  xr =  scale * marker[mtype][pc + 2 + 2 * i];
	  yr = -scale * marker[mtype][pc + 3 + 2 * i];
          seg_xform_rel(&xr, &yr);
          if (i == 0)
	    MoveTo(x - xr, y + yr);
	  else
	    LineTo(x - xr, y + yr);
	}
	ClosePoly();
	PaintPoly(poly);
	KillPoly(poly);
        pc += 1 + 2 * marker[mtype][pc + 1];
	if (op == 5)
	  set_color(mcolor);
	break;

      case 6: /* arc */
	rect.top = y - r;
	rect.left = x - r;
	rect.bottom = y + r;
	rect.right = x + r;
	FrameArc(&rect, 0, 360);
	break;

      case 7: /* filled arc */
      case 8: /* hollow arc */
	if (op == 8)
	  set_color(0);
	rect.top = y - r;
	rect.left = x - r;
	rect.bottom = y + r;
	rect.right = x + r;
	PaintArc(&rect, 0, 360);
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
  double mk_size;

  mk_type  = gkss->asf[3] ? gkss->mtype  : gkss->mindex;
  mk_size  = gkss->asf[4] ? gkss->mszsc  : 1;
  mk_color = gkss->asf[5] ? gkss->pmcoli : 1;

  PenSize(1, 1);
  set_color(mk_color);

  marker_routine(n, px, py, mk_type, mk_size, mk_color);
}

static
void draw_string(int x, int y, int width, int height, Str255 chars)
{
  FontInfo info;
  GWorldPtr fromworld, toworld;
  CGrafPtr onport, saveport;
  GDHandle savedevice;
  int txFont, txFace, txSize;
  Rect fromrect, torect, onrect;
  PixMapHandle frompix, topix, onpix;
  int *from, *to, from_width, to_width;
  int descent, w = 0, h = 0;
  int i, j, ii = 0, jj = 0;

  if (p->path != 0)
    {
      GetFontInfo(&info);
      height += 8;
      descent = info.descent;

      switch (p->path)
	{
	  case 1: x -= height-descent; y -= width; w = height; h = width; break;
	  case 2: x -= width; y -= descent; w = width; h = height; break;
	  case 3: x -= descent; w = height; h = width; break;
	}

      GetGWorld(&saveport, &savedevice);
      txFont = GetPortTextFont(saveport);
      txFace = GetPortTextFace(saveport);
      txSize = GetPortTextSize(saveport);

      onport = p->port;
      onpix = GetPortPixMap(onport);

      fromrect.left = 0;
      fromrect.right = width;
      fromrect.top = 0;
      fromrect.bottom = height;
      NewGWorld(&fromworld, 32, &fromrect, NULL, NULL, 0);
      SetGWorld(fromworld, NULL);
	
      frompix = GetGWorldPixMap(fromworld);
      from = (int *) GetPixBaseAddr(frompix);
      from_width = GetPixRowBytes(frompix) / 4;

      LockPixels(frompix);

      EraseRect(&fromrect);
      MoveTo(0, height - descent);
      TextFont(txFont);
      TextFace(txFace);
      TextSize(txSize);
      DrawString(chars);

      torect.left = 0;
      torect.right = w;
      torect.top = 0;
      torect.bottom = h;
      NewGWorld(&toworld, 32, &torect, NULL, NULL, 0);

      topix = GetGWorldPixMap(toworld);
      to = (int *) GetPixBaseAddr(topix);
      to_width = GetPixRowBytes(topix) / 4;

      LockPixels(topix);
      for (i = 0; i < width; i++)
	{
	  for (j = 0; j < height; j++)
	    {
	      switch (p->path)
		{
		  case 1: ii = j; jj = h - i - 1; break;
		  case 2: ii = w - i - 1; jj = h - j - 1; break;
		  case 3: ii = w - j - 1; jj = i; break;
		}
	      to[jj * to_width + ii] = from[j * from_width + i];
	    }
	}
      UnlockPixels(topix);
      UnlockPixels(frompix);

      onrect.left = x;
      onrect.right = x + w;
      onrect.top = y;
      onrect.bottom = y + h;

      SetGWorld(saveport, savedevice);

      SetPort(onport);
      LockPixels(topix);
      CopyBits(
	(BitMap *) *topix, (BitMap *) *onpix, &torect, &onrect, srcOr, NULL);
      UnlockPixels(topix);
	
      DisposeGWorld(toworld);
      DisposeGWorld(fromworld);
    }
  else
    {
      MoveTo(x, y);
      DrawString(chars);
    }
}

static
void text_routine(double x, double y, int nchars, char *chars)
{
  int i, ch, xstart, ystart, width, height;
  double xrel, yrel, ax, ay;
  unsigned char *s;
  Str255 dst;

  s = (unsigned char *) malloc(nchars + 1);
  for (i = 0; i < nchars; i++)
    {
      ch = chars[i];
      if (ch < 0)
	ch += 256;
      s[i] = p->family == 3 ? ch : iso2mac[ch];
    }
  s[nchars] = '\0';
  CopyCStringToPascal((char *) s, dst);

  NDC_to_DC(x, y, xstart, ystart);

  width = StringWidth(dst);
  height = p->capheight;

  xrel = width * xfac[gkss->txal[0]];
  yrel = p->capheight * yfac[gkss->txal[1]];
  CharXform(xrel, yrel, ax, ay);
  xstart += (int)ax;
  ystart -= (int)ay;

  draw_string(xstart, ystart, width, height, dst);

  free(s);
}

static
void set_font(int font)
{
  double rad, scale, ux, uy;
  int family, size, angle;
  double width, height, capheight;
  StyleParameter face;
  Str255 name;

  font = abs(font);
  if (font >= 101 && font <= 129)
    font -= 100;
  else if (font >= 1 && font <= 32)
    font = map[font - 1];
  else
    font = 9;

  WC_to_NDC_rel(gkss->chup[0], gkss->chup[1], gkss->cntnr, ux, uy);
  seg_xform_rel(&ux, &uy);

  rad = -atan2(ux, uy);
  angle = (int)(rad * 180 / M_PI + 0.5);
  if (angle < 0) angle += 360;
  p->path = ((angle + 45) / 90) % 4;

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
  p->family = (font - 1) / 4;
  face = (font % 4 == 1 || font % 4 == 2) ? normal : bold;
  if (font % 4 == 2 || font % 4 == 0)
    face |= italic;

  CopyCStringToPascal(fonts[p->family], name);
  family = FMGetFontFamilyFromName(name);
  if (family != kInvalidFontFamily)
    {
      TextFont(family);
      TextFace(face);
      TextSize(size);
    }
  else
    gks_perror("invalid font family (%s)", fonts[p->family]);
}

static
void fill_routine(int n, double *px, double *py, int tnr);

static
void text(double px, double py, int nchars, char *chars)
{
  int tx_font, tx_prec, tx_color;
  double x, y;

  tx_font  = gkss->asf[6] ? gkss->txfont : predef_font[gkss->tindex - 1];
  tx_prec  = gkss->asf[6] ? gkss->txprec : predef_prec[gkss->tindex - 1];
  tx_color = gkss->asf[9] ? gkss->txcoli : 1;

  PenSize(1, 1);
  set_color(tx_color);

  if (tx_prec == GKS_K_TEXT_PRECISION_STRING)
    {
      set_font(tx_font);

      WC_to_NDC(px, py, gkss->cntnr, x, y);
      seg_xform(&x, &y);

      text_routine(x, y, nchars, chars);
    }
  else
    gks_emul_text(px, py, nchars, chars, line_routine, fill_routine);
}

static
void fill_routine(int n, double *px, double *py, int tnr)
{
  PolyHandle poly;
  int i;
  double x, y;
  int ix, iy;

  poly = OpenPoly();
  for (i = 0; i < n; i++)
  {
    WC_to_NDC(px[i], py[i], tnr, x, y);
    seg_xform(&x, &y);
    NDC_to_DC(x, y, ix, iy);

    if (i == 0)
      MoveTo(ix, iy);
    else
      LineTo(ix, iy);
  }
  ClosePoly();
  if (p->pat)
    FillPoly(poly, p->pat);
  else
    PaintPoly(poly);
  KillPoly(poly);
}

static
void fillarea(int n, double *px, double *py)
{
  int fl_inter, fl_style, fl_color;

  fl_inter = gkss->asf[10] ? gkss->ints   : predef_ints[gkss->findex - 1];
  fl_style = gkss->asf[11] ? gkss->styli  : predef_styli[gkss->findex - 1];
  fl_color = gkss->asf[12] ? gkss->facoli : 1;

  p->pat = NULL;
  if (fl_inter == GKS_K_INTSTYLE_HOLLOW)
    {
      PenSize(1, 1);
      set_color(fl_color);
      line_routine(n, px, py, DrawBorder, gkss->cntnr);
    }
  else if (fl_inter == GKS_K_INTSTYLE_SOLID)
    {
      set_color(fl_color);
      fill_routine(n, px, py, gkss->cntnr);
    }
  else if (fl_inter == GKS_K_INTSTYLE_PATTERN ||
    fl_inter == GKS_K_INTSTYLE_HATCH)
    {
      set_color(fl_color);
      if (fl_inter == GKS_K_INTSTYLE_HATCH)
	fl_style += HATCH_STYLE;
      if (fl_style >= PATTERNS)
	fl_style = 1;
      p->pat = p->pattern + fl_style;
      fill_routine(n, px, py, gkss->cntnr);
    }
}

static
void cellarray(
  double xmin, double xmax, double ymin, double ymax,
  int dx, int dy, int dimx, int *colia, int true_color)
{
  double x1, y1, x2, y2;
  int ix1, ix2, iy1, iy2;
  int x, y, width, height;
  int i, j, ix, iy, ind;
  int swapx, swapy;
  RGBColor color;
  int rgb, red, green, blue;

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
	  color.red = p->rgb[ind].red;
	  color.green = p->rgb[ind].green;
	  color.blue = p->rgb[ind].blue;
	}
      else
	{
	  rgb = colia[iy * dimx + ix];
	  red = (rgb & 0xff);
	  green = (rgb & 0xff00) >> 8;
	  blue = (rgb & 0xff0000) >> 16;
	  color.red = nint(red * 256);
	  color.green = nint(green * 256);
	  color.blue = nint(blue * 256);
	}
      SetCPixel(x + i, y + j, &color);
    }
  }
}

static
void crosshair(int mousex, int mousey)
{
  PenMode(srcXor);

  MoveTo(mousex, 0); LineTo(mousex, p->height);
  MoveTo(0, mousey); LineTo(p->width, mousey);
  QDFlushPortBuffer(p->port, NULL);

  PenMode(srcCopy);
}

static
void get_mouse(Point *mouse, EventModifiers *modifiers)
{
  EventRecord event;

  EventAvail(0, &event);

  *mouse = event.where;
  *modifiers = event.modifiers;

  GlobalToLocal(mouse);
}

static
void get_pointer(double *x, double *y, int *state)
{
  Point mouse;
  EventModifiers modifiers;
  Rect rect;
  int mousex, mousey;

  GetWindowPortBounds(p->win, &rect);

  GetMouse(&mouse);
  mousex = mouse.h;
  mousey = mouse.v;
  crosshair(mousex, mousey);

  *state = GKS_K_STATUS_OK;
  do
    {  
      get_mouse(&mouse, &modifiers);

      if (mousex != mouse.h || mousey != mouse.v)
	{
	  crosshair(mousex, mousey);
	  mousex = mouse.h;
	  mousey = mouse.v;
	  crosshair(mousex, mousey);
	}
      usleep(10000);

      if (!(modifiers & 0x80) && modifiers & 0xff00)
	{
	  *state = GKS_K_STATUS_NONE;
	  break;
	}
    }
  while (modifiers);

  crosshair(mousex, mousey);

  DC_to_NDC(mousex, mousey, *x, *y);
}

void gks_drv_mac(
  int fctid, int dx, int dy, int dimx, int *ia,
  int lr1, double *r1, int lr2, double *r2, int lc, char *chars,
  void **ptr)
{
  p = (ws_state_list *) *ptr;

  switch (fctid)
    {
/* open workstation */
    case 2:
      gkss = (gks_state_list_t *) *ptr;

      p = (ws_state_list *) calloc(1, sizeof(ws_state_list));

      if (pthread_mutex_init(&p->mutex, NULL))
	{
	  perror("pthread_mutex_init");
	  exit(-1);
	}
      p->run = 0;
      if (pthread_create(&p->thread, NULL, exec, (void *) p))
	{
	  perror("pthread_create");
	  exit(-1);
	}

      while (!p->run)
	usleep(10000);

      p->port = GetWindowPort(p->win);
      SetPort(p->port);

      *ptr = p;
      break;

/* close workstation */
    case 3:
      p->run = 0;
      pthread_join(p->thread, NULL);
      pthread_mutex_destroy(&p->mutex);

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
      clear_ws();
      break;

/* update workstation */
    case 8:
      pthread_mutex_lock(&p->mutex);
      QDFlushPortBuffer(p->port, NULL);
      pthread_mutex_unlock(&p->mutex);
      break;

/* polyline */
    case 12:
      if (p->state == GKS_K_WS_ACTIVE)
	{
	  pthread_mutex_lock(&p->mutex);
	  LockPortBits(p->port);
	  polyline(ia[0], r1, r2);
	  UnlockPortBits(p->port);
	  pthread_mutex_unlock(&p->mutex);
	}
      break;

/* polymarker */
    case 13:
      if (p->state == GKS_K_WS_ACTIVE)
	{
	  pthread_mutex_lock(&p->mutex);
	  LockPortBits(p->port);
	  polymarker(ia[0], r1, r2);
	  UnlockPortBits(p->port);
	  pthread_mutex_unlock(&p->mutex);
	}
      break;

/* text */
    case 14:
      if (p->state == GKS_K_WS_ACTIVE)
	{
	  pthread_mutex_lock(&p->mutex);
	  LockPortBits(p->port);
	  text(r1[0], r2[0], strlen(chars), chars);
	  UnlockPortBits(p->port);
	  pthread_mutex_unlock(&p->mutex);
	}
      break;

/* fill area */
    case 15:
      if (p->state == GKS_K_WS_ACTIVE)
	{
	  pthread_mutex_lock(&p->mutex);
	  LockPortBits(p->port);
	  fillarea(ia[0], r1, r2);
	  UnlockPortBits(p->port);
	  pthread_mutex_unlock(&p->mutex);
	}
      break;

/* cell array */
    case 16:
    case DRAW_IMAGE:
      if (p->state == GKS_K_WS_ACTIVE)
	{
	  int true_color = fctid == DRAW_IMAGE;

	  pthread_mutex_lock(&p->mutex);
	  LockPortBits(p->port);
	  cellarray(r1[0], r1[1], r2[0], r2[1], dx, dy, dimx, ia, true_color);
	  UnlockPortBits(p->port);
	  pthread_mutex_unlock(&p->mutex);
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
        set_clip_rect(*ia);
      break;

    case 52:
/* select normalization transformation */
    case 53:
/* set clipping inidicator */
      set_clip_rect(gkss->cntnr);
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

      if (p->state == GKS_K_WS_ACTIVE)
	{
	  pthread_mutex_lock(&p->mutex);
	  LockPortBits(p->port);
	  resize_window();
	  set_xform();
	  init_norm_xform();
	  UnlockPortBits(p->port);
	  pthread_mutex_unlock(&p->mutex);
	}
      break;

/* request locator */
    case 81:
      if (p->state == GKS_K_WS_ACTIVE)
	{
	  pthread_mutex_lock(&p->mutex);
	  QDFlushPortBuffer(p->port, NULL);
	  get_pointer(r1, r2, &ia[0]);
	  pthread_mutex_unlock(&p->mutex);
	}
      break;

    default:
      ;
    }
}

#else

#include "gks.h"
#include "gkscore.h"

void gks_drv_mac(
  int fctid, int dx, int dy, int dimx, int *ia,
  int lr1, double *r1, int lr2, double *r2, int lc, char *chars, void **ptr)
{
  if (fctid == 2)
  {
#if defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ >= 1060
    gks_perror("Quickdraw is deprecated\nConsider using the Quartz driver");
#else
    gks_perror("Carbon support not compiled in");
#endif
    ia[0] = 0;
  }
}

#endif
