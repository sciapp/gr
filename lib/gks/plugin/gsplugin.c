#ifdef __unix__
#define _POSIX_C_SOURCE 200112L
#endif

#ifndef NO_GS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define __PROTOTYPES__

#ifdef __cplusplus
#define GSDLLEXPORT extern "C"
#endif

#ifdef _WIN32
#define __WINDOWS__
#endif

#include "ghostscript/iapi.h"
#include "ghostscript/gdevdsp.h"
#include "ghostscript/ierrors.h"

#if DISPLAY_VERSION_MAJOR > 1
#define gs_main_instance void
#endif

#endif

#include "gks.h"
#include "gkscore.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32

#include <windows.h>
#ifndef DLLEXPORT
#define DLLEXPORT __declspec(dllexport)
#endif

#endif

DLLEXPORT void gks_gsplugin(int fctid, int dx, int dy, int dimx, int *i_arr, int len_f_arr_1, double *f_arr_1,
                            int len_f_arr_2, double *f_arr_2, int len_c_arr, char *c_arr, void **ptr);

#ifdef __cplusplus
}
#endif

#ifndef GKS_UNUSED
#define GKS_UNUSED(x) (void)(x)
#endif

#ifndef NO_GS

#if !defined(VMS) && !defined(_WIN32)
#include <unistd.h>
#endif

#ifdef hpux
#include <sys/utsname.h>
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#define NINT(a) (int)((a) + 0.5)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#define SIZE_INCREMENT 32768

#define PATTERNS 120
#define NUM_GS_ARGS 8

#define LLX 0
#define LLY 0

#define WC_to_NDC(xw, yw, tnr, xn, yn) \
  xn = a[tnr] * (xw) + b[tnr];         \
  yn = c[tnr] * (yw) + d[tnr]

#define WC_to_NDC_rel(xw, yw, tnr, xn, yn) \
  xn = a[tnr] * (xw);                      \
  yn = c[tnr] * (yw)

#define NDC_to_DC(xn, yn, xd, yd) \
  xd = p->a * (xn) + p->b;        \
  yd = p->c * (yn) + p->d

#define nint(a) ((int)(a + 0.5))

static gks_state_list_t *gkss;

static double a[MAX_TNR], b[MAX_TNR], c[MAX_TNR], d[MAX_TNR];

static const char *show[] = {"lj", "lj", "ct", "rj"};
static double yfac[] = {0., -1.2, -1.0, -0.5, 0., 0.2};

static int predef_font[] = {1, 1, 1, -2, -3, -4};
static int predef_prec[] = {0, 1, 2, 2, 2, 2};
static int predef_ints[] = {0, 1, 3, 3, 3};
static int predef_styli[] = {1, 1, 1, 2, 3};

static int map[] = {22, 9,  5, 14, 18, 26, 13, 1, 24, 11, 7, 16, 20, 28, 13, 3,
                    23, 10, 6, 15, 19, 27, 13, 2, 25, 12, 8, 17, 21, 29, 13, 4};

static double caps[] = {0.662, 0.653, 0.676, 0.669, 0.718, 0.718, 0.718, 0.718, 0.562, 0.562, 0.562,
                        0.562, 0.667, 0.681, 0.681, 0.681, 0.681, 0.722, 0.722, 0.722, 0.722, 0.740,
                        0.740, 0.740, 0.740, 0.692, 0.692, 0.681, 0.681, 0.587, 0.692};

static const char *fonts[] = {"Times-Roman",
                              "Times-Italic",
                              "Times-Bold",
                              "Times-BoldItalic",
                              "Helvetica",
                              "Helvetica-Oblique",
                              "Helvetica-Bold",
                              "Helvetica-BoldOblique",
                              "Courier",
                              "Courier-Oblique",
                              "Courier-Bold",
                              "Courier-BoldOblique",
                              "Symbol",
                              "Bookman-Light",
                              "Bookman-LightItalic",
                              "Bookman-Demi",
                              "Bookman-DemiItalic",
                              "NewCenturySchlbk-Roman",
                              "NewCenturySchlbk-Italic",
                              "NewCenturySchlbk-Bold",
                              "NewCenturySchlbk-BoldItalic",
                              "AvantGarde-Book",
                              "AvantGarde-BookOblique",
                              "AvantGarde-Demi",
                              "AvantGarde-DemiOblique",
                              "Palatino-Roman",
                              "Palatino-Italic",
                              "Palatino-Bold",
                              "Palatino-BoldItalic",
                              "ZapfChancery-MediumItalic",
                              "ZapfDingbats"};

static const char *dc[3][3] = {{"F", "E", "D"}, {"G", "I", "C"}, {"H", "A", "B"}};

typedef struct ws_state_list_t
{
  int conid, gs_dev, wtype, state;
  char *path;
  int empty, init, pages, page_counter;

  int ix, iy;
  double a, b, c, d, e, f, g, h, mw, mh;
  int ytrans, res;
  double magstep;
  int stroke, limit, np;

  double red[MAX_COLOR + 1], green[MAX_COLOR + 1], blue[MAX_COLOR + 1];
  int color, fcol;

  double ysize;

  int len, size, column, saved_len, saved_column;
  char *buffer;

  unsigned char ascii85_buffer[10];
  char a85line[100];
  long a85offset;

  double window[4], viewpt[4];

  int ltype;
  double cwidth, csize, cangle, cheight;
  int font;

  gs_main_instance *gs_instance;
  int gs_argc;
  char *gs_argv[NUM_GS_ARGS];
  int gs_position;

  double width, height, nominal_size;
} ws_state_list;

static ws_state_list *p;

static void set_norm_xform(int tnr, double *wn, double *vp)
{
  a[tnr] = (vp[1] - vp[0]) / (wn[1] - wn[0]);
  b[tnr] = vp[0] - wn[0] * a[tnr];
  c[tnr] = (vp[3] - vp[2]) / (wn[3] - wn[2]);
  d[tnr] = vp[2] - wn[2] * c[tnr];
}

static void init_norm_xform(void)
{
  int tnr;

  for (tnr = 0; tnr < MAX_TNR; tnr++) set_norm_xform(tnr, gkss->window[tnr], gkss->viewport[tnr]);
}

static void seg_xform(double *x, double *y)
{
  double xx;

  xx = *x * gkss->mat[0][0] + *y * gkss->mat[0][1] + gkss->mat[2][0];
  *y = *x * gkss->mat[1][0] + *y * gkss->mat[1][1] + gkss->mat[2][1];
  *x = xx;
}

static void seg_xform_rel(double *x, double *y)
{
  double xx;

  xx = *x * gkss->mat[0][0] + *y * gkss->mat[0][1];
  *y = *x * gkss->mat[1][0] + *y * gkss->mat[1][1];
  *x = xx;
}

static int lastop(const char *op)
{
  int len;

  len = strlen(op);
  if (len < p->len)
    return strncmp(p->buffer + p->len - len, op, len);
  else
    return -1;
}

static void packb(const char *buff)
{
  int len, i;

  len = strlen(buff);

  p->saved_len = p->len;
  p->saved_column = p->column;

  if (buff[0] == '%')
    {
      if (p->column != 0)
        {
          p->buffer[p->len++] = '\n';
          p->column = 0;
        }
    }
  else if (len > 78 - p->column)
    {
      if (p->len != 0)
        {
          p->buffer[p->len++] = '\n';
          p->column = 0;
        }
    }

  if (len + 2 > p->size - p->len)
    {
      p->size += SIZE_INCREMENT;
      p->buffer = (char *)realloc(p->buffer, p->size);
    }

  if (p->column != 0)
    {
      p->buffer[p->len++] = ' ';
      p->column++;
    }

  for (i = 0; i < len; i++)
    {
      p->buffer[p->len++] = buff[i];
      p->column++;
    }

  if (buff[0] == '%')
    {
      p->buffer[p->len++] = '\n';
      p->column = 0;
    }
}

static char *Ascii85Tuple(unsigned char *data)
{
  static char tuple[6];
  long i, x;
  unsigned long code, quantum;

  code = ((((unsigned long)data[0] << 8) | (unsigned long)data[1]) << 16) | ((unsigned long)data[2] << 8) |
         (unsigned long)data[3];
  if (code == 0L)
    {
      tuple[0] = 'z';
      tuple[1] = '\0';
      return (tuple);
    }
  quantum = 85UL * 85UL * 85UL * 85UL;
  for (i = 0; i < 4; i++)
    {
      x = (long)(code / quantum);
      code -= quantum * x;
      tuple[i] = (char)(x + (int)'!');
      quantum /= 85L;
    }
  tuple[4] = (char)((code % 85L) + (int)'!');
  tuple[5] = '\0';
  return (tuple);
}

static void Ascii85Initialize(void)
{
  p->a85offset = 0L;
  p->a85line[0] = '\0';
}

static void Ascii85Flush(void)
{
  char *tuple;

  packb(p->a85line);
  if (p->a85offset > 0)
    {
      p->ascii85_buffer[p->a85offset] = '\0';
      p->ascii85_buffer[p->a85offset + 1] = '\0';
      p->ascii85_buffer[p->a85offset + 2] = '\0';
      tuple = Ascii85Tuple(p->ascii85_buffer);
      packb(*tuple == 'z' ? (char *)"!!!!" : tuple);
    }
  packb("~>");
}

static void Ascii85Encode(unsigned char code)
{
  long n, i = 0;
  char *q;
  unsigned char *c;
  char b[100];

  p->ascii85_buffer[p->a85offset] = code;
  p->a85offset++;
  if (p->a85offset < 4) return;
  c = p->ascii85_buffer;
  for (n = p->a85offset; n >= 4; n -= 4)
    {
      for (q = Ascii85Tuple(c); *q; q++) b[i++] = *q;
      c += 8;
    }
  p->a85offset = n;
  c -= 4;
  b[i] = '\0';
  strcat(p->a85line, b);
  if (strlen(p->a85line) >= 75)
    {
      packb(p->a85line);
      p->a85line[0] = '\0';
    }
  for (n = 0; n < 4; n++) p->ascii85_buffer[n] = (*c++);
}

static unsigned int LZWEncodeImage(unsigned int number_pixels, unsigned char *pixels)
{
#define LZWClr 256UL /* Clear Table Marker */
#define LZWEod 257UL /* End of Data marker */
#define OutputCode(code)                                    \
  {                                                         \
    accumulator += code << (32 - code_width - number_bits); \
    number_bits += code_width;                              \
    while (number_bits >= 8)                                \
      {                                                     \
        Ascii85Encode((unsigned char)(accumulator >> 24));  \
        accumulator = accumulator << 8;                     \
        number_bits -= 8;                                   \
      }                                                     \
  }

  typedef struct _TableType
  {
    long prefix, suffix, next;
  } TableType;

  long index;
  long i;
  TableType *table;
  unsigned long accumulator, number_bits, code_width, last_code, next_index;

  /*
   * Allocate string table.
   */

  table = (TableType *)malloc((1 << 12) * sizeof(*table));
  if (table == (TableType *)NULL) return (0);

  /*
   * Initialize variables.
   */
  accumulator = 0;
  code_width = 9;
  number_bits = 0;
  last_code = 0;
  Ascii85Initialize();
  OutputCode(LZWClr);
  for (index = 0; index < 256; index++)
    {
      table[index].prefix = (-1);
      table[index].suffix = (short)index;
      table[index].next = (-1);
    }
  next_index = LZWEod + 1;
  code_width = 9;
  last_code = (unsigned long)pixels[0];
  for (i = 1; i < (long)number_pixels; i++)
    {
      /*
       * Find string.
       */
      index = (long)last_code;
      while (index != -1)
        if ((table[index].prefix != (long)last_code) || (table[index].suffix != (long)pixels[i]))
          index = table[index].next;
        else
          {
            last_code = (unsigned long)index;
            break;
          }
      if (last_code != (unsigned long)index)
        {
          /*
           * Add string.
           */
          OutputCode(last_code);
          table[next_index].prefix = (long)last_code;
          table[next_index].suffix = (short)pixels[i];
          table[next_index].next = table[last_code].next;
          table[last_code].next = (long)next_index;
          next_index++;
          /*
           * Did we just move up to next bit width?
           */
          if ((next_index >> code_width) != 0)
            {
              code_width++;
              if (code_width > 12)
                {
                  /*
                   * Did we overflow the max bit width?
                   */
                  code_width--;
                  OutputCode(LZWClr);
                  for (index = 0; index < 256; index++)
                    {
                      table[index].prefix = (-1);
                      table[index].suffix = index;
                      table[index].next = (-1);
                    }
                  next_index = LZWEod + 1;
                  code_width = 9;
                }
            }
          last_code = (unsigned long)pixels[i];
        }
    }
  /*
   * Flush tables.
   */
  OutputCode(last_code);
  OutputCode(LZWEod);
  if (number_bits != 0) Ascii85Encode((unsigned char)(accumulator >> 24));

  Ascii85Flush();
  free(table);

  return (0);
}

static void set_xform(double *wn, double *vp)
{
  p->e = (vp[1] - vp[0]) / (wn[1] - wn[0]);
  p->f = (6750 - 1) / 0.28575;
  p->h = (4650 - 1) / 0.19685;
  p->g = (vp[3] - vp[2]) / (wn[3] - wn[2]);

  p->a = p->e * p->f;
  p->b = p->f * (vp[0] - wn[0] * p->e);
  p->c = p->g * p->h;
  p->d = p->h * (vp[2] - wn[2] * p->g);

  p->mw = p->a * (wn[1] - wn[0]);
  p->mh = p->c * (wn[3] - wn[2]);

  p->width = p->a;
  p->height = p->c;
  p->nominal_size = MIN(p->width, p->height) / 500.0 * 72 / 600;
  if (gkss->nominal_size > 0) p->nominal_size *= gkss->nominal_size;

  p->stroke = 0;
}

static void bounding_box(int landscape, double magstep)
{
  char buffer[50];
  int ix1, ix2, iy1, iy2;
  double magn;

  if (fabs(magstep) > FEPS)
    magn = pow(1.2, magstep);
  else
    magn = 1.0;

  ix1 = LLX;
  iy1 = LLY;
  ix2 = ix1 + NINT((landscape ? p->mh : p->mw) * 72 / 600 * magn);
  iy2 = iy1 + NINT((landscape ? p->mw : p->mh) * 72 / 600 * magn);
  if (gkss->version < 5)
    p->ytrans = landscape ? iy2 : iy1;
  else
    p->ytrans = -ix2;
  p->res = (ix2 - ix1) + (iy2 - iy1);

  snprintf(buffer, 50, "%%%%BoundingBox: %d %d %d %d", ix1, iy1, ix2, iy2);
  packb(buffer);
  if (gkss->version > 4)
    {
      snprintf(buffer, 50, "%%%%Orientation: %s", landscape ? "Landscape" : "Portrait");
      packb(buffer);
    }
}

static void move(double x, double y)
{
  char buffer[50];

  p->ix = NINT(p->a * x + p->b);
  p->iy = NINT(p->c * y + p->d);

  if (p->stroke)
    {
      packb("sk");
      p->stroke = 0;
    }
  snprintf(buffer, 50, "np %d %d m", p->ix, p->iy);
  packb(buffer);
  p->np = 1;
}

static void draw(double x, double y)
{
  char buffer[50];
  int jx, jy, rx, ry;

  jx = p->ix;
  jy = p->iy;
  p->ix = NINT(p->a * x + p->b);
  p->iy = NINT(p->c * y + p->d);

  if (p->np == 1 || p->ix != jx || p->iy != jy)
    {
      rx = p->ix - jx;
      ry = p->iy - jy;
      if (abs(rx) > 1 || abs(ry) > 1)
        {
          snprintf(buffer, 50, "%d %d rl", rx, ry);
          packb(buffer);
        }
      else
        packb(dc[rx + 1][ry + 1]);
      p->np++;

      if (p->limit)
        {
          if (p->np == p->limit)
            {
              packb("sk");
              p->stroke = 0;
              snprintf(buffer, 50, "%d %d m", p->ix, p->iy);
              packb(buffer);
              p->np = 1;
            }
          else
            p->stroke = 1;
        }
    }
}

static void moveto(double x, double y)
{
  char buffer[20];

  p->ix = NINT(x);
  p->iy = NINT(y);

  snprintf(buffer, 20, "%d %d m", p->ix, p->iy);
  packb(buffer);
}

static void amoveto(double angle, double x, double y)
{
  char buffer[30];

  p->ix = NINT(x);
  p->iy = NINT(y);

  snprintf(buffer, 30, "%.4g %d %d am", angle, p->ix, p->iy);
  packb(buffer);
}

static void set_linetype(int ltype, double lwidth)
{
  char buffer[100], dash[80];

  if (ltype != p->ltype || (fabs(lwidth - p->cwidth) > FEPS))
    {
      p->ltype = ltype;
      gks_get_dash(ltype, lwidth * 8, dash);
      snprintf(buffer, 100, "%s 0 setdash", dash);
      packb(buffer);
    }
}

static void set_linewidth(double width)
{
  char buffer[20];

  if (fabs(width - p->cwidth) > FEPS)
    {
      p->cwidth = fabs(width);
      snprintf(buffer, 20, "%.4g lw", p->cwidth * 600 / 72 * p->nominal_size);
      packb(buffer);
    }
}

static void set_markersize(double size)
{
  char buffer[20];

  if (fabs(size - p->csize) > FEPS)
    {
      p->csize = fabs(size);
      snprintf(buffer, 20, "%.4g ms", p->csize * p->nominal_size);
      packb(buffer);
    }
}

static void set_markerangle(double angle)
{
  char buffer[20];

  if (fabs(angle - p->cangle) > FEPS)
    {
      p->cangle = fabs(angle);
      snprintf(buffer, 20, "%.4g ma", p->cangle);
      packb(buffer);
    }
}

static void gkinfo(int *nchars, char *chars, size_t size)
{
  char *date, host[100];
  const char *user;
  time_t elapsed_time;
#ifdef hpux
  struct utsname utsname;
#endif
#ifdef _WIN32
  char lpBuffer[100];
  DWORD nSize = 100;
#endif

  time(&elapsed_time);
  date = ctime(&elapsed_time);

#ifndef _WIN32
  user = (char *)gks_getenv("USER");
#else
  if (GetUserName(lpBuffer, &nSize) != 0)
    {
      user = lpBuffer;
      lpBuffer[nSize] = '\0';
    }
  else
    user = NULL;
#endif
  if (user == NULL) user = "(?)";

#ifdef VMS
  strcpy(host, (char *)gks_getenv("SYS$NODE"));
#else
#ifdef hpux
  uname(&utsname);
  strcpy(host, utsname.nodename);
#else
#if defined(OS2) || (defined(_WIN32) && !defined(__GNUC__))
  strcpy(host, "(unknown)"); /* FIXME */
#else
  gethostname(host, 100);
#endif /* _WIN32 */
#endif /* hpux */
#endif /* VMS */

  strtok(date, "\n");
  strtok(host, ".");

  snprintf(chars, size, "%s  by user  %s @ %s", date, user, host);
  *nchars = strlen(chars);
}

static void ps_header(void)
{
  int nchars;
  char info[150], buffer[200];

  gkinfo(&nchars, info, 150);
  packb("%!PS-Adobe-2.0");
  if (nchars > 0)
    {
      snprintf(buffer, 200, "\
%%%%Creator: %s, GKS 5 PostScript Device Handler",
               info + 35);
      packb(buffer);
      info[24] = '\0';
      snprintf(buffer, 200, "%%%%+CreationDate: %s", info);
      packb(buffer);
    }
  else
    packb("%%Creator: GKS 5 PostScript Device Handler");
  packb("%%+Copyright @ 1993-2007, J.Heinen");
  snprintf(buffer, 200, "%%%%Pages: 1");
  packb(buffer);
}

static void set_color(int color, int wtype)
{
  char buffer[50];
  double grey;
  int index;

  if (color < MAX_COLOR)
    {
      if (color != p->color)
        {
          if (lastop("sc") == 0)
            {
              p->len = p->saved_len;
              p->column = p->saved_column;
            }
          index = abs(color);
          if (wtype % 2)
            {
              grey = 0.3 * p->red[index] + 0.59 * p->green[index] + 0.11 * p->blue[index];
              snprintf(buffer, 50, "%.4g sg", grey);
              packb(buffer);
            }
          else
            {
              snprintf(buffer, 50, "%.4g %.4g %.4g sc", p->red[index], p->green[index], p->blue[index]);
              packb(buffer);
            }
          p->color = index;
        }
    }
}

static void set_foreground(int color, int wtype)
{
  char buffer[50];
  int index;
  double grey;

  if (color < MAX_COLOR)
    {
      if (color != p->fcol)
        {
          index = abs(color);
          if (wtype % 2)
            {
              grey = 0.3 * p->red[index] + 0.59 * p->green[index] + 0.11 * p->blue[index];
              snprintf(buffer, 50, "/fg {%.4g sg} def", grey);
              packb(buffer);
            }
          else
            {
              snprintf(buffer, 50, "/fg {%.4g %.4g %.4g sc} def", p->red[index], p->green[index], p->blue[index]);
              packb(buffer);
            }
          p->fcol = index;
        }
      if (color != p->color)
        {
          index = abs(color);
          packb("fg");
          p->color = index;
        }
    }
}

static void set_background(int wtype)
{
  char buffer[50];
  double grey;

  if (wtype % 2)
    {
      grey = 0.3 * p->red[0] + 0.59 * p->green[0] + 0.11 * p->blue[0];
      snprintf(buffer, 50, "/bg {%.4g sg} def", grey);
      packb(buffer);
    }
  else
    {
      snprintf(buffer, 50, "/bg {%.4g %.4g %.4g sc} def", p->red[0], p->green[0], p->blue[0]);
      packb(buffer);
    }
}

static void update(void)
{
  if (p->column != 0)
    {
      p->buffer[p->len++] = '\n';
      p->column = 0;
    }
}

static void set_clip_rect(int tnr)
{
  int i, j;
  double *clrt, cx1, cy1, cx2, cy2;
  char buffer[120];

  if (gkss->clip_tnr != 0)
    tnr = gkss->clip_tnr;
  else if (gkss->clip == GKS_K_NOCLIP)
    tnr = 0;

  clrt = gkss->viewport[tnr];
  i = clrt[0] < clrt[1] ? 0 : 1;
  j = clrt[2] < clrt[3] ? 2 : 3;

  NDC_to_DC(clrt[i], clrt[j], cx1, cy1);
  NDC_to_DC(clrt[1 - i], clrt[5 - j], cx2, cy2);

  if (gkss->clip_region == GKS_K_REGION_ELLIPSE && tnr != 0)
    {
      double x, y, rx, ry;

      x = 0.5 * (cx1 + cx2);
      y = 0.5 * (cy1 + cy2);
      rx = 0.5 * (cx2 - cx1);
      ry = 0.5 * (cy2 - cy1);

      snprintf(buffer, 120, "np %.2f %.2f m %.2f %.2f l %.2f %.2f %.2f %.2f %.2f %.2f ellipse clip", x, y,
               x + rx * cos(gkss->clip_start_angle * M_PI / 180), y + ry * sin(gkss->clip_start_angle * M_PI / 180), x,
               y, rx, ry, gkss->clip_start_angle, gkss->clip_end_angle);
    }
  else
    {
      int ix1, ix2, iy1, iy2;

      ix1 = ((int)cx1) - 2;
      iy1 = ((int)cy1) - 2;
      ix2 = NINT(cx2) + 2;
      iy2 = NINT(cy2) + 2;

      snprintf(buffer, 120, "np %d %d m %d %d l %d %d l %d %d l cp clip", ix1, iy1, ix1, iy2, ix2, iy2, ix2, iy1);
    }
  packb(buffer);
}

static void set_font(int font)
{

  double scale, w, h, ux, uy, chh;
  char buffer[200];
  int size;

  scale = sqrt(gkss->chup[0] * gkss->chup[0] + gkss->chup[1] * gkss->chup[1]);
  ux = gkss->chup[0] / scale * gkss->chh;
  uy = gkss->chup[1] / scale * gkss->chh;
  WC_to_NDC_rel(ux, uy, gkss->cntnr, ux, uy);

  w = 0;
  h = sqrt(ux * ux + uy * uy);
  seg_xform_rel(&w, &h);

  chh = sqrt(w * w + h * h);

  if ((font != p->font) || (fabs(chh - p->cheight) > FEPS))
    {
      p->font = abs(font);
      p->cheight = fabs(chh);

      if (p->font >= 101 && p->font <= 131)
        font = p->font - 101;
      else if (p->font >= 1 && p->font <= 32)
        font = map[p->font - 1] - 1;
      else
        font = 8;

      p->ysize = p->cheight * p->height;
      size = MIN(MAX((int)(p->ysize / caps[font]), 1), 7200);

      if (font != 12 && font != 29 && font != 30)
        {
          snprintf(buffer, 200, "gsave /%s_ ISOLatin1Encoding", fonts[font]);
          packb(buffer);
          snprintf(buffer, 200, "/%s encodefont pop grestore", fonts[font]);
          packb(buffer);
          snprintf(buffer, 200, "/%s_ findfont %d scalefont setfont", fonts[font], size);
          packb(buffer);
        }
      else
        {
          snprintf(buffer, 200, "/%s findfont %d scalefont setfont", fonts[font], size);
          packb(buffer);
        }
    }
}

static void get_magstep(double *magstep, int *dpi)
{
  char *env;

  if ((env = (char *)gks_getenv("GKS_MAGSTEP")) != NULL)
    *magstep = atof(env);
  else
    *magstep = 0;

  *dpi = 75;
}

static void ps_init(int *pages)
{
  int dpi, landscape;
  int pa[33], i, j = 1, k;
  char str[17], buffer[100];

  ps_header();

  landscape = p->wtype >= 63;

  if (gkss->version < 5)
    if (!landscape) landscape = p->viewpt[1] - p->viewpt[0] > 0.19685;

  if (*pages == 0)
    {
      get_magstep(&p->magstep, &dpi);

      bounding_box(landscape, p->magstep);
      packb("%%EndComments");
      packb("%%BeginProcSet: GKS 5");

      packb("save /GKS_save exch def");
      packb("/GKS_dict 150 dict def GKS_dict begin");

      packb("/in {72 mul} def");
      packb("/np {newpath} def");
      packb("/cp {closepath} def");
      packb("/m {moveto} def");
      packb("/l {lineto} def");
      packb("/c {curveto} def");
      packb("/A {1 0 rlineto} def");
      packb("/B {1 1 rlineto} def");
      packb("/C {0 1 rlineto} def");
      packb("/D {-1 1 rlineto} def");
      packb("/E {-1 0 rlineto} def");
      packb("/F {-1 -1 rlineto} def");
      packb("/G {0 -1 rlineto} def");
      packb("/H {1 -1 rlineto} def");
      packb("/I {1 0 rlineto -1 0 rlineto} def");
      packb("/am {np gsave translate rotate 0 0 m} def");
      packb("/gs {gsave} def");
      packb("/gr {grestore} def");
      packb("/rm {rmoveto} def");
      packb("/srm {rxy rx s mul ry s mul rm} def");
      packb("/rl {rlineto} def");
      packb("/srl {rxy rx s mul ry s mul rl} def");
      packb("/sk {stroke} def");
      packb("/csk {closepath stroke} def");
      packb("/fi {closepath eofill} def");
      packb("/gi {closepath fill} def");
      packb("/el {/endangle exch def /startangle exch def\
 /yrad exch def /xrad exch def /y exch def /x exch def\
 /savematrix matrix currentmatrix def x y translate xrad yrad scale\
 0 0 1 startangle endangle arc savematrix setmatrix} def");
      packb("/eln {/endangle exch def /startangle exch def\
 /yrad exch def /xrad exch def /y exch def /x exch def\
 /savematrix matrix currentmatrix def x y translate xrad yrad scale\
 0 0 1 startangle endangle arcn savematrix setmatrix} def");
      packb("/sg {setgray} def");
      packb("/sc {setrgbcolor} def");
      packb("/fg {0 sg} def");

      set_background(p->wtype);

      packb("/lw {setlinewidth} def");
      packb("/ms {/s exch def} def");
      packb("/ma {/a exch def} def");
      packb("/ct {dup stringwidth pop 2 div neg 0 rmoveto show} def");
      packb("/rj {dup stringwidth pop neg 0 rmoveto show} def");
      packb("/lj {show} def");
      packb("/xy {/y exch def /x exch def} def");
      packb("/rxy {/ry exch def /rx exch def} def");
      packb("/sxy {gsave xy x y translate a rotate x neg y neg translate} def");
      packb("/dt {xy np fg x y s 0 360 arc fi} def");
      packb("/pl {sxy np x y m fg -24 0 srl 48 0 srl\
 -24 0 srl 0 24 srl 0 -48 srl sk gr} def");
      packb("/as {np x y m 0.0 22.5 srm 5.0 -17.5 srl 17.5 2.5 srl -15.0 -10.0\
 srl 7.5 -17.5 srl -15.0 12.5 srl -15.0 -12.5 srl 7.5 17.5 srl -15.0 10.0 srl\
 17.5 -2.5 srl 5.0 17.5 srl} def");
      packb("/fas {sxy as cp gs fg fill gr bc sk gr} def");
      packb("/dc {sxy np x y m fg -24 24 srl 48 -48 srl\
 -24 24 srl -24 -24 srl 48 48 srl");
      packb("sk gr} def");
      packb("/hl {sxy np x y m fg -24 0 srm 48 0 srl sk gr} def");
      packb("/vl {sxy np x y m fg 0 -24 srm 0 48 srl sk gr} def");
      packb("/e5 {np x y m 0 25 srm 23.775 -17.275 srl -9.075 -27.95 srl\
 -29.4 0 srl -9.075 27.95 srl} def");
      packb("/e6 {np x y m 0 -25 srm 21.65 12.5 srl 0 25 srl -21.65 12.5 srl\
 -21.65 -12.5 srl 0 -25 srl} def");
      packb("/e7 {np x y m 0 25 srm 19.55 -9.425 srl 4.825 -21.15 srl\
 -13.525 -16.95 srl -21.7 0 srl -13.525 16.95 srl 4.825 21.15 srl} def");
      packb("/e8 {np x y m 0 -25 srm 17.675 7.325 srl 7.325 17.675 srl\
 -7.325 17.675 srl -17.675 7.325 srl -17.675 -7.325 srl -7.325 -17.675 srl\
 7.325 -17.675 srl} def");
      packb("/s4 {np x y m 0 -25 srm 7.075 17.925 srl 17.925 7.075 srl\
 -17.925 7.075 srl -7.075 17.925 srl -7.075 -17.925 srl -17.925 -7.075 srl\
 17.925 -7.075 srl} def");
      packb("/s5 {np x y m 0 25 srm 5.875 -16.9 srl 17.9 -0.375 srl\
 -14.275 -10.825 srl 5.2 -17.125 srl -14.7 10.225 srl -14.7 -10.225 srl\
 5.2 17.125 srl -14.275 10.825 srl 17.9 0.375 srl} def");
      packb("/s6 {np x y m 0 -25 srm 5 16.35 srl 16.65 -3.85 srl\
 -11.65 12.5 srl 11.65 12.5 srl -16.65 -3.85 srl -5 16.35 srl -5 -16.35 srl\
 -16.65 3.85 srl 11.65 -12.5 srl -11.65 -12.5 srl 16.65 3.85 srl} def");
      packb("/s7 {np x y m 0 25 srm 4.35 -16 srl 15.2 6.575 srl\
 -9.8 -13.35 srl 14.625 -7.8 srl -16.55 -0.65 srl 3.025 -16.3 srl\
 -10.85 12.525 srl -10.85 -12.525 srl 3.025 16.3 srl -16.55 0.65 srl\
 14.625 7.8 srl -9.8 13.35 srl 15.2 -6.575 srl} def");
      packb("/s8 {np x y m 0 -25 srm 3.825 15.75 srl 13.85 -8.425 srl\
 -8.425 13.85 srl 15.75 3.825 srl -15.75 3.825 srl 8.425 13.85 srl\
 -13.85 -8.425 srl -3.825 15.75 srl -3.825 -15.75 srl -13.85 8.425 srl\
 8.425 -13.85 srl -15.75 -3.825 srl 15.75 -3.825 srl -8.425 -13.85 srl\
 13.85 8.425 srl} def");
      packb("/ed5 {sxy e5 cp gs fg fill gr bc sk gr} def");
      packb("/ed6 {sxy e6 cp gs fg fill gr bc sk gr} def");
      packb("/ed7 {sxy e7 cp gs fg fill gr bc sk gr} def");
      packb("/ed8 {sxy e8 cp gs fg fill gr bc sk gr} def");
      packb("/st4 {sxy s4 cp gs fg fill gr bc sk gr} def");
      packb("/st5 {sxy s5 cp gs fg fill gr bc sk gr} def");
      packb("/st6 {sxy s6 cp gs fg fill gr bc sk gr} def");
      packb("/st7 {sxy s7 cp gs fg fill gr bc sk gr} def");
      packb("/st8 {sxy s8 cp gs fg fill gr bc sk gr} def");
      packb("/sq {np x y m 0 24 srm 24 0 srl 0 -48 srl\
 -48 0 srl 0 48 srl 24 0 srl} def");
      packb("/nsq {sxy bg sq fi fg sq csk gr} def");
      packb("/fsq {sxy sq cp gs fg fill gr bc sk gr} def");
      packb("/ci {np x y 24 s mul 0 360 arc} def");
      packb("/nci {xy bg ci fi fg ci sk} def");
      packb("/fci {sxy ci cp gs fg fill gr bc sk gr} def");
      packb("/tu {np x y m 0 24 srm -24 -48 srl 48 0 srl -24 48 srl} def");
      packb("/ntu {sxy bg tu fi fg tu csk gr} def");
      packb("/ftu {sxy tu cp gs fg fill gr bc sk gr} def");
      packb("/td {np x y m 0 -24 srm -24 48 srl 48 0 srl -24 -48 srl} def");
      packb("/ntd {sxy bg td fi fg td csk gr} def");
      packb("/ftd {sxy td cp gs fg fill gr bc sk gr} def");
      packb("/dm {np x y m 0 24 srm -24 -24 srl\
 24 -24 srl 24 24 srl -24 24 srl} def");
      packb("/ndm {sxy bg dm fi fg dm csk gr} def");
      packb("/fdm {sxy dm cp gs fg fill gr bc sk gr} def");
      packb("/bt {np x y m -30 24 srl 0 -48 srl\
 60 48 srl 0 -48 srl -30 24 srl} def");
      packb("/nbt {sxy bg bt fi fg bt csk gr} def");
      packb("/fbt {sxy bt cp gs fg fill gr bc sk gr} def");
      packb("/hg {np x y m -24 30 srl 48 0 srl\
 -48 -60 srl 48 0 srl -24 30 srl} def");
      packb("/nhg {sxy bg hg fi fg hg csk gr} def");
      packb("/fhg {sxy hg cp gs fg fill gr bc sk gr} def");
      packb("/st {sxy bg as fi fg as csk gr} def");
      packb("/fst {fas} def");
      packb("/ast {sxy np x y m fg 0 25 srl sk np x y m 25 7.5 srl sk\
 np x y m 15 -25 srl sk np x y m -15 -25 srl sk\
 np x y m -25 7.5 srl sk} def");
      packb("/tus {np x y m 0 28 srm -24 -42 srl 48 0 srl -24 42 srl} def");
      packb("/tds {np x y m 0 -28 srm -24 42 srl 48 0 srl -24 -42 srl} def");
      packb("/tud {sxy bg tus fi bg tds fi fg tus csk fg tds csk gr} def");
      packb("/tl {np x y m -14 0 srm 42 -24 srl 0 48 srl -42 -24 srl} def");
      packb("/ftl {sxy tl cp gs fg fill gr bc sk gr} def");
      packb("/tr {np x y m 28 0 srm -42 -24 srl 0 48 srl 42 -24 srl} def");
      packb("/ftr {sxy tr cp gs fg fill gr bc sk gr} def");
      packb("/opl {np x y m 0 24 srm 8 0 srl\
 0 -16 srl 16 0 srl 0 -16 srl -16 0 srl");
      packb("0 -16 srl -16 0 srl 0 16 srl -16 0 srl\
 0 16 srl 16 0 srl 0 16 srl 8 0 srl} def");
      packb("/npl {sxy bg opl fi fg opl csk gr} def");
      packb("/fpl {sxy opl cp gs fg fill gr bc sk gr} def");
      packb("/om {np x y m 0 24 srm 16 0 srl\
 8 -8 srl 0 -32 srl -8 -8 srl -32 0 srl");
      packb("-8 8 srl 0 32 srl 8 8 srl 16 0 srl} def");
      packb("/nom {sxy bg om fi fg om csk gr} def");

      for (i = 0; i < PATTERNS; i++)
        {
          gks_inq_pattern_array(i, pa);
          for (j = *pa; j < ((*pa == 32) ? 16 : (*pa == 4) ? 8 : *pa); j++)
            {
              pa[j + 1] = pa[j % *pa + 1];
            }
          for (k = 0, j = 1; j < 9; j++, k += 2)
            {
              snprintf(str + k, 17 - k, "%02x", pa[j]);
            }
          snprintf(buffer, 100, "/pat%d << /PaintType 2 /PatternType 1 /TilingType 1\
 /BBox [0 0 1 1] /XStep 1",
                   i);
          packb(buffer);
          snprintf(buffer, 100, "/YStep 1 /PaintProc {pop 8 8 false [8 0 0 8 0 0] \
{<%s>} imagemask}",
                   str);
          packb(buffer);
          packb(">> [0 8 -8 0 0 0] makepattern def");
        }

      packb("/OF /findfont load def");
      packb("/findfont {dup GKS_dict exch known");
      packb("{GKS_dict exch get}");
      packb("if GKS_dict /OF get exec} def");
      packb("mark");
      packb("/ISOLatin1Encoding 8#000 1 8#001 {StandardEncoding exch get} for");
      packb("/emdash /endash 8#004 1 8#025 {StandardEncoding exch get} for");
      packb("/quotedblleft /quotedblright 8#030 1 8#054\
 {StandardEncoding exch get} for");
      packb("/minus 8#056 1 8#217 {StandardEncoding exch get} for");
      packb("/dotlessi 8#301 1 8#317 {StandardEncoding exch get} for");
      packb("/space/exclamdown/cent/sterling/currency/yen/brokenbar/section");
      packb("/dieresis/copyright/ordfeminine/guillemotleft\
/logicalnot/hyphen/registered");
      packb("/macron/degree/plusminus/twosuperior\
/threesuperior/acute/mu/paragraph");
      packb("/periodcentered/cedilla/onesuperior/ordmasculine\
/guillemotright/onequarter");
      packb("/onehalf/threequarters/questiondown/Agrave\
/Aacute/Acircumflex/Atilde");
      packb("/Adieresis/Aring/AE/Ccedilla/Egrave/Eacute\
/Ecircumflex/Edieresis/Igrave");
      packb("/Iacute/Icircumflex/Idieresis/Eth/Ntilde/Ograve\
/Oacute/Ocircumflex/Otilde");
      packb("/Odieresis/multiply/Oslash/Ugrave/Uacute\
/Ucircumflex/Udieresis/Yacute/Thorn");
      packb("/germandbls/agrave/aacute/acircumflex/atilde\
/adieresis/aring/ae/ccedilla");
      packb("/egrave/eacute/ecircumflex/edieresis/igrave\
/iacute/icircumflex/idieresis");
      packb("/eth/ntilde/ograve/oacute/ocircumflex/otilde\
/odieresis/divide/oslash/ugrave");
      packb("/uacute/ucircumflex/udieresis/yacute/thorn\
/ydieresis");
      packb("256 array astore def cleartomark");
      packb("/encodefont {findfont dup maxlength dict begin");
      packb("{1 index /FID ne {def} {pop pop} ifelse} forall");
      packb("/Encoding exch def dup");
      packb("/FontName exch def currentdict");
      packb("definefont end} def");
      packb("/ellipse {\
 /endangle exch def\
 /startangle exch def\
 /yrad exch def\
 /xrad exch def\
 /y exch def\
 /x exch def\
 /savematrix matrix currentmatrix def\
 x y translate xrad yrad scale 0 0 1 startangle endangle arc\
 savematrix setmatrix\
} def");
      packb("end");

      packb("%%EndProcSet");
      packb("%%EndProlog");
    }

  (*pages)++;
  snprintf(buffer, 100, "%%%%Page: %d %d", *pages, *pages);
  packb(buffer);

  packb("%%BeginPageSetup");
  packb("GKS_dict begin save /psl exch def");

  if (landscape)
    {
      if (gkss->version < 5)
        snprintf(buffer, 100, "%d %d translate -90 rotate", LLX, p->ytrans);
      else
        snprintf(buffer, 100, "90 rotate %d %d translate", LLX, p->ytrans);
      packb(buffer);
    }
  else
    {
      snprintf(buffer, 100, "%d %d translate", LLX, LLY);
      packb(buffer);
    }
  if (fabs(p->magstep) > FEPS)
    {
      snprintf(buffer, 100, "%.4g 1 in 600 div mul dup scale", pow(1.2, p->magstep));
      packb(buffer);
    }
  else
    packb("1 in 600 div dup scale");

  set_color(-1, p->wtype);
  set_foreground(-1, p->wtype);
  packb("1 setlinecap 1 setlinejoin");
  set_linewidth(-1.0);
  set_markersize(-1.0);
  packb("0 ma");
  set_font(-1);
  packb("%%EndPageSetup");
  update();
}

static void end_page(int pages)
{
  char buffer[30];

  snprintf(buffer, 30, "%%%%EndPage: %d %d", pages, pages);
  packb(buffer);
}


static void set_colortable(void)
{
  int i;

  for (i = 0; i < MAX_COLOR; i++) gks_inq_rgb(i, p->red + i, p->green + i, p->blue + i);
  p->color = -1;
}

static void set_color_rep(int color, double red, double green, double blue)
{
  if (color >= 0 && color < MAX_COLOR)
    {
      p->red[color] = red;
      p->green[color] = green;
      p->blue[color] = blue;
      p->color = -1;
    }
}

static void query_color(int index, unsigned char **buf, int wtype)
{
  double grey;

  index = FIX_COLORIND(index);

  if (wtype % 2)
    {
      grey = 0.3 * p->red[index] + 0.59 * p->green[index] + 0.11 * p->blue[index];
      **buf = (char)NINT(grey * 255);
      (*buf)++;
    }
  else
    {
      **buf = (char)NINT(p->red[index] * 255);
      (*buf)++;
      **buf = (char)NINT(p->green[index] * 255);
      (*buf)++;
      **buf = (char)NINT(p->blue[index] * 255);
      (*buf)++;
    }
}

static void rgb2color(unsigned int rgb, unsigned char **buf, int wtype)
{
  int r, g, b, a;
  double grey;

  r = (rgb & 0xff);
  g = (rgb & 0xff00) >> 8;
  b = (rgb & 0xff0000) >> 16;
  a = (rgb & 0xff000000) >> 24;
  if (a == 0)
    {
      r = g = b = 0xff;
    }
  if (wtype % 2)
    {
      grey = 0.3 * r / 255.0 + 0.59 * g / 255.0 + 0.11 * b / 255.0;
      **buf = (char)NINT(grey * 255);
      (*buf)++;
    }
  else
    {
      **buf = (char)NINT(r);
      (*buf)++;
      **buf = (char)NINT(g);
      (*buf)++;
      **buf = (char)NINT(b);
      (*buf)++;
    }
}

static void set_connection(int conid, char *path, int wtype)
{
  p->conid = conid;
  p->path = path;
  p->gs_dev = wtype != 301 ? wtype : 0;
  p->wtype = 62;

  p->window[0] = 0;
  p->window[1] = 1;
  p->window[2] = 0;
  p->window[3] = 1;

  p->viewpt[0] = 0;
  p->viewpt[1] = 0.19685;
  p->viewpt[2] = 0;
  p->viewpt[3] = p->viewpt[1];

  set_xform(p->window, p->viewpt);

  p->pages = 0;
  p->init = 0;
  p->empty = 1;
  p->page_counter = 0;
  p->color = 1;
  p->len = p->column = p->saved_len = p->saved_column = 0;
  p->font = 0;
  p->ltype = GKS_K_LINETYPE_SOLID;
  p->cwidth = p->csize = p->cangle = p->cheight = 0.0;
}

static void marker_routine(double x, double y, int marker)
{
  double dx, dy;
  char buffer[50];
  static const char *macro[] = {"nom", " hl", " vl", "st8", "st7", "st6", "st5", "st4", "ed8", "ed7",
                                "ed6", "ed5", "fpl", "npl", "ftl", "ftr", "tud", "fst", " st", "fdm",
                                "ndm", "fhg", "nhg", "fbt", "nbt", "fsq", "nsq", "ftd", "ntd", "ftu",
                                "ntu", "fci", " dt", " dt", " pl", "ast", "nci", " dc"};

  NDC_to_DC(x, y, dx, dy);

  p->ix = NINT(dx);
  p->iy = NINT(dy);
  snprintf(buffer, 50, "%d %d %s", p->ix, p->iy, macro[marker + 32]);
  packb(buffer);
}

static void cell_array(double xmin, double xmax, double ymin, double ymax, int dx, int dy, int dimx, int *colia,
                       int wtype, int true_color)
{
  char buffer[100];
  unsigned char *buf, *bufP;
  double x1, x2, y1, y2;
  int w, h, x, y;

  int i, j, ci, len, swap = 0;
  int tnr;

  tnr = gkss->cntnr;

  WC_to_NDC(xmin, ymax, tnr, x1, y1);
  seg_xform(&x1, &y1);
  NDC_to_DC(x1, y1, x1, y1);

  WC_to_NDC(xmax, ymin, tnr, x2, y2);
  seg_xform(&x2, &y2);
  NDC_to_DC(x2, y2, x2, y2);

  w = (int)fabs(x2 - x1);
  h = (int)fabs(y2 - y1);
  if (w == 0 || h == 0) return;
  x = (int)MIN(x1, x2);
  y = (int)MIN(y1, y2);

  packb("gsave");

  set_clip_rect(gkss->cntnr);

  packb("/RawData currentfile /ASCII85Decode filter def");
  packb("/Data RawData << >> /LZWDecode filter def");

  snprintf(buffer, 100, "%d %d translate", x, y);
  packb(buffer);

  snprintf(buffer, 100, "%d %d scale", w, h);
  packb(buffer);

  snprintf(buffer, 100, "/Device%s setcolorspace", wtype % 2 == 0 ? "RGB" : "Gray");
  packb(buffer);

  if (x1 > x2) swap = 1;
  if (y1 > y2) swap += 2;

  packb("{ << /ImageType 1");

  snprintf(buffer, 100, "/Width %d /Height %d", dx, dy);
  packb(buffer);
  if (swap == 0)
    snprintf(buffer, 100, "/ImageMatrix [%d 0 0 -%d 0 %d]", dx, dy, dy);
  else if (swap == 1)
    snprintf(buffer, 100, "/ImageMatrix [-%d 0 0 -%d %d %d]", dx, dy, dx, dy);
  else if (swap == 2)
    snprintf(buffer, 100, "/ImageMatrix [%d 0 0 %d 0 0]", dx, dy);
  else
    snprintf(buffer, 100, "/ImageMatrix [-%d 0 0 %d %d 0]", dx, dy, dx);
  packb(buffer);

  snprintf(buffer, 100, "/DataSource Data /BitsPerComponent 8 /Decode [0 1%s]", wtype % 2 == 0 ? " 0 1 0 1" : "");
  packb(buffer);

  packb(">> image Data closefile RawData flushfile } exec");

  len = dx * dy;
  if (wtype % 2 == 0) len = len * 3;

  buf = (unsigned char *)malloc(len);
  bufP = buf;
  for (j = 0; j < dy; j++)
    {
      for (i = 0; i < dx; i++)
        {
          ci = colia[j * dimx + i];
          if (!true_color)
            query_color(ci, &bufP, wtype);
          else
            rgb2color((unsigned int)ci, &bufP, wtype);
        }
    }
  LZWEncodeImage(len, buf);
  free(buf);

  packb("grestore");
}

static void text_routine(double *x, double *y, int nchars, char *chars)
{
  int i, j;
  double ux, uy, yrel, angle, phi;
  double xorg, yorg;
  int alh, alv, ic;
  char str[500], buffer[510];
  int prec;
  char *latin1_str = gks_malloc(nchars + 1);

  gks_utf82latin1(chars, latin1_str);
  chars = latin1_str;
  nchars = strlen(chars);

  NDC_to_DC(*x, *y, xorg, yorg);

  prec = gkss->asf[6] ? gkss->txprec : predef_prec[gkss->tindex - 1];
  alh = gkss->txal[0];
  alv = gkss->txal[1];

  WC_to_NDC_rel(gkss->chup[0], gkss->chup[1], gkss->cntnr, ux, uy);
  seg_xform_rel(&ux, &uy);
  angle = -atan2(ux, uy) * 180.0 / M_PI;

  if (prec == GKS_K_TEXT_PRECISION_STRING)
    {
      phi = angle / 180.0 * M_PI;
      yrel = p->ysize * yfac[alv - GKS_K_TEXT_VALIGN_NORMAL];
      xorg -= yrel * sin(phi);
      yorg += yrel * cos(phi);
    }

  if (fabs(angle) > FEPS)
    amoveto(angle, xorg, yorg);
  else
    moveto(xorg, yorg);

  for (i = 0, j = 0; i < nchars; i++)
    {
      ic = chars[i];
      if (ic < 0) ic += 256;
      if (ic < 127)
        {
          if (strchr("()\\", ic) != NULL) str[j++] = '\\';
          str[j++] = chars[i];
        }
      else
        {
          snprintf(str + j, 500 - j, "\\%03o", ic);
          j += 4;
        }
      str[j] = '\0';
    }
  snprintf(buffer, 510, "(%s) %s", str, show[alh + GKS_K_TEXT_HALIGN_NORMAL]);
  packb(buffer);
  if (fabs(angle) > FEPS) packb("gr");

  gks_free(latin1_str);
}

static void fill_routine(int n, double *px, double *py, int tnr)
{
  double x, y;
  char buffer[50];
  int i, jx, jy, rx, ry, nan_found = 0;

  packb("gsave");

  set_clip_rect(gkss->cntnr);

  WC_to_NDC(px[0], py[0], tnr, x, y);
  NDC_to_DC(x, y, p->ix, p->iy);

  snprintf(buffer, 50, "np %d %d m", p->ix, p->iy);
  packb(buffer);
  p->np = 1;

  for (i = 1; i < n; i++)
    {
      jx = p->ix;
      jy = p->iy;
      WC_to_NDC(px[i], py[i], tnr, x, y);
      NDC_to_DC(x, y, p->ix, p->iy);

      if (i == 1 || p->ix != jx || p->iy != jy)
        {
          rx = p->ix - jx;
          ry = p->iy - jy;
          if (abs(rx) > 1 || abs(ry) > 1)
            {
              if (px[i] != px[i] && py[i] != py[i])
                {
                  nan_found = 1;
                  continue;
                }
              if (nan_found)
                {
                  snprintf(buffer, 50, "%d %d m", p->ix, p->iy);
                  nan_found = 0;
                }
              else
                {
                  snprintf(buffer, 50, "%d %d rl", rx, ry);
                }
              packb(buffer);
            }
          else
            packb(dc[rx + 1][ry + 1]);
          p->np++;
        }
    }

  if (p->np > 2) packb("fi");

  packb("grestore");
}

static void fillpattern_routine(int n, double *px, double *py, int tnr, int pattern)
{
  char buffer[100];

  snprintf(buffer, 100, "gs [/Pattern /Device%s] setcolorspace %.4g %.4g %.4g pat%d setcolor",
           p->wtype % 2 == 0 ? "RGB" : "Gray", p->red[p->color], p->green[p->color], p->blue[p->color], pattern);
  packb(buffer);

  fill_routine(n, px, py, tnr);
  packb("gr");
}

static void line_routine(int n, double *px, double *py, int ltype, int tnr)
{
  p->limit = 1000;
  gks_emul_polyline(n, px, py, ltype, tnr, move, draw);
  if (p->stroke)
    {
      packb("sk");
      p->stroke = 0;
    }
}

static void to_DC(int n, double *x, double *y)
{
  int i;
  double xn, yn;

  for (i = 0; i < n; i++)
    {
      WC_to_NDC(x[i], y[i], gkss->cntnr, xn, yn);
      seg_xform(&xn, &yn);
      NDC_to_DC(xn, yn, x[i], y[i]);
    }
}

static void draw_path(int n, double *px, double *py, int nc, int *codes)
{
  char buffer[100];
  int i, j, np;
  double x[3], y[3], w, h, a1, a2;
  double cur_x = 0, cur_y = 0, start_x = 0, start_y = 0;
  double x1, y1, x2, y2;
  GKS_UNUSED(n);

  snprintf(buffer, 100, "np ");
  packb(buffer);
  np = 0;

  j = 0;
  for (i = 0; i < nc; ++i)
    {
      switch (codes[i])
        {
        case 'M':
        case 'm':
          x[0] = px[j];
          y[0] = py[j];
          if (codes[i] == 'm')
            {
              x[0] += cur_x;
              y[0] += cur_y;
            }
          start_x = cur_x = x[0];
          start_y = cur_y = y[0];
          to_DC(1, x, y);
          snprintf(buffer, 100, "%s%.2f %.2f m", np ? "np " : "", x[0], y[0]);
          packb(buffer);
          np = 0;
          j += 1;
          break;
        case 'L':
        case 'l':
          x[0] = px[j];
          y[0] = py[j];
          if (codes[i] == 'l')
            {
              x[0] += cur_x;
              y[0] += cur_y;
            }
          cur_x = x[0];
          cur_y = y[0];
          to_DC(1, x, y);
          snprintf(buffer, 100, "%.2f %.2f l", x[0], y[0]);
          packb(buffer);
          j += 1;
          break;
        case 'Q':
        case 'q':
          x[0] = px[j];
          y[0] = py[j];
          if (codes[i] == 'q')
            {
              x[0] += cur_x;
              y[0] += cur_y;
            }
          x[1] = px[j + 1];
          y[1] = py[j + 1];
          if (codes[i] == 'q')
            {
              x[1] += cur_x;
              y[1] += cur_y;
            }
          x[2] = cur_x;
          y[2] = cur_y;
          cur_x = x[1];
          cur_y = y[1];
          to_DC(3, x, y);
          x1 = x[2] + (2.0 / 3.0) * (x[0] - x[2]);
          y1 = y[2] + (2.0 / 3.0) * (y[0] - y[2]);
          x2 = x[1] + (2.0 / 3.0) * (x[0] - x[1]);
          y2 = y[1] + (2.0 / 3.0) * (y[0] - y[1]);
          snprintf(buffer, 100, "%.2f %.2f %.2f %.2f %.2f %.2f c", x1, y1, x2, y2, x[1], y[1]);
          packb(buffer);
          j += 2;
          break;
        case 'C':
        case 'c':
          x[0] = px[j];
          y[0] = py[j];
          if (codes[i] == 'c')
            {
              x[0] += cur_x;
              y[0] += cur_y;
            }
          x[1] = px[j + 1];
          y[1] = py[j + 1];
          if (codes[i] == 'c')
            {
              x[1] += cur_x;
              y[1] += cur_y;
            }
          x[2] = px[j + 2];
          y[2] = py[j + 2];
          if (codes[i] == 'c')
            {
              x[2] += cur_x;
              y[2] += cur_y;
            }
          cur_x = x[2];
          cur_y = y[2];
          to_DC(3, x, y);
          snprintf(buffer, 100, "%.2f %.2f %.2f %.2f %.2f %.2f c", x[0], y[0], x[1], y[1], x[2], y[2]);
          packb(buffer);
          j += 3;
          break;
        case 'A':
        case 'a':
          {
            double rx, ry, cx, cy;
            rx = fabs(px[j]);
            ry = fabs(py[j]);
            a1 = px[j + 1];
            a2 = py[j + 1];
            cx = cur_x - rx * cos(a1);
            cy = cur_y - ry * sin(a1);
            x[0] = cx;
            y[0] = cy;
            x[1] = cx + rx;
            y[1] = cy + ry;
            cur_x = cx + rx * cos(a2);
            cur_y = cy + ry * sin(a2);
          }
          to_DC(2, x, y);
          w = x[1] - x[0];
          h = y[1] - y[0];
          a1 *= 180 / M_PI;
          a2 *= 180 / M_PI;
          if (a2 < a1)
            {
              snprintf(buffer, 100, "%.2f %.2f %.2f %.2f %.2f %.2f eln", x[0], y[0], w, h, a1, a2);
            }
          else
            {
              snprintf(buffer, 100, "%.2f %.2f %.2f %.2f %.2f %.2f el", x[0], y[0], w, h, a1, a2);
            }
          packb(buffer);
          j += 3;
          break;
        case 's':
          snprintf(buffer, 100, "cp");
          cur_x = start_x;
          cur_y = start_y;
          packb(buffer);
          /* fall through */
        case 'S':
          set_linewidth(gkss->bwidth);
          snprintf(buffer, 100, "%.4g %.4g %.4g sc sk", p->red[gkss->bcoli], p->green[gkss->bcoli],
                   p->blue[gkss->bcoli]);
          packb(buffer);
          np = 1;
          break;
        case 'f':
        case 'g':
          snprintf(buffer, 100, "%.4g %.4g %.4g sc %ci", p->red[gkss->facoli], p->green[gkss->facoli],
                   p->blue[gkss->facoli], codes[i]);
          cur_x = start_x;
          cur_y = start_y;
          packb(buffer);
          np = 1;
          break;
        case 'F':
        case 'G':
          snprintf(buffer, 100, "gs %.4g %.4g %.4g sc %ci gr", p->red[gkss->facoli], p->green[gkss->facoli],
                   p->blue[gkss->facoli], codes[i] + 32);
          packb(buffer);
          set_linewidth(gkss->bwidth);
          snprintf(buffer, 100, "%.4g %.4g %.4g sc csk", p->red[gkss->bcoli], p->green[gkss->bcoli],
                   p->blue[gkss->bcoli]);
          cur_x = start_x;
          cur_y = start_y;
          packb(buffer);
          np = 1;
          break;
        case 'Z':
          snprintf(buffer, 100, "cp");
          cur_x = start_x;
          cur_y = start_y;
          np = 0;
          break;
        case '\0':
          break;
        default:
          gks_perror("invalid path code ('%c')", codes[i]);
          exit(1);
        }
    }
}

static void draw_lines(int n, double *px, double *py, int *attributes)
{
  int i, j = 0, rgba, ln_color = MAX_COLOR;
  double x, y;
  int xim1, yim1, xi, yi;
  double line_width;
  char buffer[50];

  WC_to_NDC(px[0], py[0], gkss->cntnr, x, y);
  seg_xform(&x, &y);
  NDC_to_DC(x, y, xi, yi);

  for (i = 1; i < n; i++)
    {
      xim1 = xi;
      yim1 = yi;
      WC_to_NDC(px[i], py[i], gkss->cntnr, x, y);
      seg_xform(&x, &y);
      NDC_to_DC(x, y, xi, yi);

      line_width = 0.001 * attributes[j++];
      rgba = attributes[j++];
      p->red[ln_color] = (rgba & 0xff) / 255.0;
      p->green[ln_color] = ((rgba >> 8) & 0xff) / 255.0;
      p->blue[ln_color] = ((rgba >> 16) & 0xff) / 255.0;

      set_linewidth(line_width);
      set_color(-ln_color, p->wtype);

      snprintf(buffer, 50, "%d %d m %d %d l sk", xim1, yim1, xi, yi);
      packb(buffer);
    }
}

static void set_bordercolor(int wtype)
{
  char buffer[50];

  if (wtype % 2)
    {
      snprintf(buffer, 50, "/bc {%.4g sg} def",
               0.3 * p->red[gkss->bcoli] + 0.59 * p->green[gkss->bcoli] + 0.11 * p->blue[gkss->bcoli]);
    }
  else
    {
      snprintf(buffer, 50, "/bc {%.4g %.4g %.4g sc} def", p->red[gkss->bcoli], p->green[gkss->bcoli],
               p->blue[gkss->bcoli]);
    }

  packb(buffer);
}

static void draw_markers(int n, double *px, double *py, int *attributes)
{
  int i, j = 0, rgba;
  int mk_type, mk_color = MAX_COLOR;
  double mk_size, x, y;

  mk_type = gkss->asf[3] ? gkss->mtype : gkss->mindex;

  set_bordercolor(p->wtype);

  for (i = 0; i < n; i++)
    {
      WC_to_NDC(px[i], py[i], gkss->cntnr, x, y);
      seg_xform(&x, &y);

      mk_size = 0.001 * attributes[j++];
      rgba = attributes[j++];
      p->red[mk_color] = (rgba & 0xff) / 255.0;
      p->green[mk_color] = ((rgba >> 8) & 0xff) / 255.0;
      p->blue[mk_color] = ((rgba >> 16) & 0xff) / 255.0;

      set_markersize(mk_size);
      set_foreground(-mk_color, p->wtype);

      marker_routine(x, y, mk_type);
    }
}

static void draw_triangles(int n, double *px, double *py, int ntri, int *tri)
{
  double x, y;
  int i, j, k, rgba, ln_color = MAX_COLOR;
  double tri_x[3], tri_y[3];
  char buffer[200];
  GKS_UNUSED(n);

  j = 0;
  for (i = 0; i < ntri / 4; ++i)
    {
      for (k = 0; k < 3; ++k)
        {
          WC_to_NDC(px[tri[j] - 1], py[tri[j] - 1], gkss->cntnr, x, y);
          seg_xform(&x, &y);
          NDC_to_DC(x, y, tri_x[k], tri_y[k]);
          j++;
        }

      rgba = tri[j++];
      p->red[ln_color] = (rgba & 0xff) / 255.0;
      p->green[ln_color] = ((rgba >> 8) & 0xff) / 255.0;
      p->blue[ln_color] = ((rgba >> 16) & 0xff) / 255.0;

      packb("np");
      set_linewidth(gkss->lwidth);
      set_color(-ln_color, p->wtype);

      snprintf(buffer, 200, "%.2f %.2f m %.2f %.2f l %.2f %.2f l csk", tri_x[0], tri_y[0], tri_x[1], tri_y[1], tri_x[2],
               tri_y[2]);
      packb(buffer);
    }
}

static void fill_polygons(int n, double *px, double *py, int nply, int *ply)
{
  double x, y, xd, yd;
  int j, k, len, fl_color = MAX_COLOR;
  unsigned int rgba;
  char buffer[50];
  GKS_UNUSED(n);

  j = 0;
  while (j < nply)
    {
      len = ply[j++];

      packb("np");
      for (k = 0; k < len; ++k)
        {
          WC_to_NDC(px[ply[j] - 1], py[ply[j] - 1], gkss->cntnr, x, y);
          seg_xform(&x, &y);
          NDC_to_DC(x, y, xd, yd);
          j++;

          if (k == 0)
            {
              snprintf(buffer, 50, "%.2f %.2f m", xd, yd);
            }
          else
            {
              snprintf(buffer, 50, "%.2f %.2f l", xd, yd);
            }
          packb(buffer);
        }

      rgba = (unsigned int)ply[j++];
      p->red[fl_color] = (rgba & 0xff) / 255.0;
      p->green[fl_color] = ((rgba >> 8) & 0xff) / 255.0;
      p->blue[fl_color] = ((rgba >> 16) & 0xff) / 255.0;

      packb("cp gs");
      set_color(-fl_color, p->wtype);
      packb("fi gr");

      snprintf(buffer, 50, "%.4g %.4g %.4g sc", p->red[gkss->bcoli], p->green[gkss->bcoli], p->blue[gkss->bcoli]);
      packb(buffer);
      set_linewidth(gkss->bwidth);
      packb("sk");
    }
}

static void gdp(int n, double *px, double *py, int primid, int nc, int *codes)
{
  switch (primid)
    {
    case GKS_K_GDP_DRAW_PATH:
      draw_path(n, px, py, nc, codes);
      break;
    case GKS_K_GDP_DRAW_LINES:
      draw_lines(n, px, py, codes);
      break;
    case GKS_K_GDP_DRAW_MARKERS:
      draw_markers(n, px, py, codes);
      break;
    case GKS_K_GDP_DRAW_TRIANGLES:
      draw_triangles(n, px, py, nc, codes);
      break;
    case GKS_K_GDP_FILL_POLYGONS:
      fill_polygons(n, px, py, nc, codes);
      break;
    default:
      gks_perror("invalid drawing primitive ('%d')", primid);
      exit(1);
    }
}

static int GSDLLCALL gsdll_stdin(void *instance, char *buf, int len)
{
  int ch;
  int count = 0;
  GKS_UNUSED(instance);

  while (count < len)
    {
      ch = p->buffer[count + p->gs_position];
      if (ch == '\0')
        {
          return 0;
        }
      *buf++ = ch;
      ++count;
      if (ch == '\n')
        {
          break;
        }
    }
  p->gs_position += count;
  return count;
}

static int GSDLLCALL gsdll_stdout(void *instance, const char *buf, int len)
{
  GKS_UNUSED(instance);
  GKS_UNUSED(buf);
  return len;
}

static int GSDLLCALL gsdll_stderr(void *instance, const char *buf, int len)
{
  GKS_UNUSED(instance);
  GKS_UNUSED(buf);
  return len;
}

static void init_arguments(void)
{
  char path[MAXPATHLEN];
  const char *device = "jpeg", *type = ".jpg";
  int i;

  p->page_counter++;

  switch (p->gs_dev)
    {
    case 320:
      device = "bmp256";
      type = "bmp";
      break;
    case 321:
      device = "jpeg";
      type = "jpg";
      break;
    case 322:
      device = "pngalpha";
      type = "png";
      break;
    case 323:
      device = "tiff24nc";
      type = "tif";
      break;
    }

  gks_filepath(path, p->path, type, p->page_counter, 0);

  p->gs_argc = NUM_GS_ARGS;
  for (i = 0; i < NUM_GS_ARGS; ++i)
    {
      p->gs_argv[i] = (char *)malloc(MAXPATHLEN * sizeof(char));
    }

#ifdef _WIN32
  snprintf(p->gs_argv[0], MAXPATHLEN * sizeof(char), "gswin32c");
#else
  snprintf(p->gs_argv[0], MAXPATHLEN * sizeof(char), "gs");
#endif
  snprintf(p->gs_argv[1], MAXPATHLEN * sizeof(char), "-sDEVICE=%s", device);
  snprintf(p->gs_argv[2], MAXPATHLEN * sizeof(char), "-g%dx%d", (int)(p->viewpt[1] * 100.0 * 600.0 / 2.54),
           (int)(p->viewpt[3] * 100.0 * 600.0 / 2.54));
  snprintf(p->gs_argv[3], MAXPATHLEN * sizeof(char), "-r600x600");
  snprintf(p->gs_argv[4], MAXPATHLEN * sizeof(char), "-sOutputFile=%s", path);
  snprintf(p->gs_argv[5], MAXPATHLEN * sizeof(char), "-dGraphicsAlphaBits=4");
  snprintf(p->gs_argv[6], MAXPATHLEN * sizeof(char), "-dTextAlphaBits=4");
  snprintf(p->gs_argv[7], MAXPATHLEN * sizeof(char), "-");
}

static void free_arguments(void)
{
  int i;

  for (i = 0; i < NUM_GS_ARGS; ++i)
    {
      free(p->gs_argv[i]);
    }
}

static void gs(void)
{
  init_arguments();
  p->gs_position = 0;

  gsapi_new_instance(&p->gs_instance, NULL);
  gsapi_set_stdio(p->gs_instance, gsdll_stdin, gsdll_stdout, gsdll_stderr);
  gsapi_init_with_args(p->gs_instance, p->gs_argc, p->gs_argv);
  gsapi_exit(p->gs_instance);
  gsapi_delete_instance(p->gs_instance);

  free_arguments();
}

void gks_gsplugin(int fctid, int dx, int dy, int dimx, int *ia, int lr1, double *r1, int lr2, double *r2, int lc,
                  char *chars, void **ptr)
{
  int style, color, pattern, ltype;
  double yres, width, size, x, y, angle;
  int font, tnr, prec;
  int nchars;
  GKS_UNUSED(lr1);
  GKS_UNUSED(lr2);
  GKS_UNUSED(lc);

  p = (ws_state_list *)*ptr;

  switch (fctid)
    {
      /* open workstation */
    case 2:
      gkss = (gks_state_list_t *)*ptr;
      gks_init_core(gkss);

      p = (ws_state_list *)calloc(1, sizeof(struct ws_state_list_t));

      p->size = SIZE_INCREMENT;
      p->buffer = (char *)calloc(1, p->size);

      init_norm_xform();
      set_connection(ia[1], chars, ia[2]);
      set_colortable();

      *ptr = p;
      break;

      /* close workstation */
    case 3:
      if (p->init)
        {
          if (!p->empty) packb("showpage");
          packb("psl restore end % GKS_dict");
          end_page(p->pages);
          packb("%%Trailer");
          packb("GKS_save restore");
        }
      if (p->pages == 0) packb("%%Trailer");
      update();

      if (!p->empty) gs();

      free(p->buffer);
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
      p->empty = 1;
      p->init = 0;
      p->pages = 0;
      p->len = p->column = 0;
      break;

      /* update workstation */
    case 8:
      if (ia[1] & GKS_K_WRITE_PAGE_FLAG)
        {
          if (p->init)
            {
              if (!p->empty)
                {
                  packb("showpage");
                  p->empty = 1;
                }
              packb("psl restore end % GKS_dict");
              end_page(p->pages);

              gs();

              p->init = 0;
              p->pages = 0;
              p->len = p->column = 0;
            }
        }
      break;

      /* polyline */
    case 12:
      if (p->state == GKS_K_WS_ACTIVE)
        {
          if (!p->init)
            {
              ps_init(&p->pages);
              p->init = 1;
            }
          tnr = gkss->cntnr;
          gks_set_dev_xform(gkss, p->window, p->viewpt);
          ltype = gkss->asf[0] ? gkss->ltype : gkss->lindex;
          width = gkss->asf[1] ? gkss->lwidth : 1;
          color = gkss->asf[2] ? gkss->plcoli : 1;
          if (ltype != GKS_K_LINETYPE_SOLID) set_linetype(ltype, width);
          set_linewidth(width);
          set_color(color, p->wtype);
          packb("gsave");
          set_clip_rect(gkss->cntnr);
          line_routine(ia[0], r1, r2, ltype, tnr);
          packb("grestore");
          if (ltype != GKS_K_LINETYPE_SOLID) set_linetype(GKS_K_LINETYPE_SOLID, 1.0);
          p->empty = 0;
        }
      break;

      /* polymarker */
    case 13:
      if (p->state == GKS_K_WS_ACTIVE)
        {
          if (!p->init)
            {
              ps_init(&p->pages);
              p->init = 1;
            }
          gks_set_dev_xform(gkss, p->window, p->viewpt);
          size = gkss->asf[4] ? gkss->mszsc : 1;
          x = 0.0;
          y = 1.0;
          seg_xform_rel(&x, &y);
          size *= sqrt(x * x + y * y);
          set_markersize(size);
          angle = -atan2(x, y) * 180.0 / M_PI;
          set_markerangle(angle);
          color = gkss->asf[5] ? gkss->pmcoli : 1;
          set_foreground(color, p->wtype);
          packb("gsave");
          set_clip_rect(gkss->cntnr);
          set_linewidth(gkss->bwidth);
          set_bordercolor(p->wtype);
          packb("0 setlinejoin");
          gks_emul_polymarker(ia[0], r1, r2, marker_routine);
          packb("1 setlinejoin");
          packb("grestore");
          p->empty = 0;
        }
      break;

      /* text */
    case 14:
      if (p->state == GKS_K_WS_ACTIVE)
        {
          if (!p->init)
            {
              ps_init(&p->pages);
              p->init = 1;
            }
          tnr = gkss->cntnr;
          gks_set_dev_xform(gkss, p->window, p->viewpt);
          font = gkss->asf[6] ? gkss->txfont : predef_font[gkss->tindex - 1];
          prec = gkss->asf[6] ? gkss->txprec : predef_prec[gkss->tindex - 1];
          if (prec != GKS_K_TEXT_PRECISION_STROKE)
            set_font(font);
          else
            set_linewidth(1.0);
          color = gkss->asf[9] ? gkss->txcoli : 1;
          set_color(color, p->wtype);
          packb("gsave");
          set_clip_rect(gkss->cntnr);
          nchars = strlen(chars);
          if (prec == GKS_K_TEXT_PRECISION_STRING)
            {
              double px, py;
              WC_to_NDC(*r1, *r2, tnr, px, py);
              seg_xform(&px, &py);
              text_routine(&px, &py, nchars, chars);
            }
          else
            {
              gks_emul_text(r1[0], r2[0], nchars, chars, line_routine, fill_routine);
            }
          packb("grestore");
          p->empty = 0;
        }
      break;

      /* fill area */
    case 15:
      if (p->state == GKS_K_WS_ACTIVE)
        {
          if (!p->init)
            {
              ps_init(&p->pages);
              p->init = 1;
            }
          tnr = gkss->cntnr;
          gks_set_dev_xform(gkss, p->window, p->viewpt);
          style = gkss->asf[10] ? gkss->ints : predef_ints[gkss->findex - 1];
          color = gkss->asf[12] ? gkss->facoli : 1;
          set_color(color, p->wtype);
          set_linewidth(gkss->bwidth);
          packb("gsave");
          set_clip_rect(gkss->cntnr);
          if (style == GKS_K_INTSTYLE_SOLID)
            fill_routine(ia[0], r1, r2, tnr);
          else if (style == GKS_K_INTSTYLE_PATTERN)
            {
              pattern = gkss->asf[11] ? gkss->styli : predef_styli[gkss->findex - 1];
              fillpattern_routine(ia[0], r1, r2, tnr, pattern);
            }
          else
            {
              yres = 1.0 / 4650.0;
              gks_emul_fillarea(ia[0], r1, r2, tnr, line_routine, yres);
            }
          packb("grestore");
          p->empty = 0;
        }
      break;

      /* cell array */
    case 16:
    case DRAW_IMAGE:
      if (p->state == GKS_K_WS_ACTIVE)
        {
          int true_color = fctid == DRAW_IMAGE;

          if (!p->init)
            {
              ps_init(&p->pages);
              p->init = 1;
            }
          gks_set_dev_xform(gkss, p->window, p->viewpt);
          packb("gsave");
          set_clip_rect(gkss->cntnr);
          cell_array(r1[0], r1[1], r2[0], r2[1], dx, dy, dimx, ia, p->wtype, true_color);
          packb("grestore");
          p->empty = 0;
        }
      break;

      /* GDP */
    case 17:
      if (p->state == GKS_K_WS_ACTIVE)
        {
          if (!p->init)
            {
              ps_init(&p->pages);
              p->init = 1;
            }
          gks_set_dev_xform(gkss, p->window, p->viewpt);
          packb("gsave");
          set_clip_rect(gkss->cntnr);
          gdp(ia[0], r1, r2, ia[1], ia[2], ia + 3);
          packb("grestore");
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
      set_xform(p->window, p->viewpt);
      init_norm_xform();
      break;

      /* set workstation viewport */
    case 55:
      p->viewpt[0] = r1[0];
      p->viewpt[1] = r1[1];
      p->viewpt[2] = r2[0];
      p->viewpt[3] = r2[1];
      set_xform(p->window, p->viewpt);
      init_norm_xform();
      break;

      /* set nominal size */
    case 109:
      p->nominal_size = MIN(p->width, p->height) / 500.0 * 72 / 600;
      if (gkss->nominal_size > 0) p->nominal_size *= gkss->nominal_size;
      break;

    default:;
    }
}

#else

void gks_gsplugin(int fctid, int dx, int dy, int dimx, int *ia, int lr1, double *r1, int lr2, double *r2, int lc,
                  char *chars, void **ptr)
{
  GKS_UNUSED(dx);
  GKS_UNUSED(dy);
  GKS_UNUSED(dimx);
  GKS_UNUSED(ia);
  GKS_UNUSED(lr1);
  GKS_UNUSED(r1);
  GKS_UNUSED(lr2);
  GKS_UNUSED(r2);
  GKS_UNUSED(lc);
  GKS_UNUSED(chars);
  GKS_UNUSED(ptr);

  if (fctid == 2)
    {
      gks_perror("Ghostscript support not compiled in");
      ia[0] = 0;
    }
}

#endif
