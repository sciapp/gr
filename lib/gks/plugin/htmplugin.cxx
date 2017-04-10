
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <libpng16/png.h>

#ifndef _WIN32
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <unistd.h>
#else
#include <windows.h>
#include <io.h>
#endif

#define PORT 8410

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

DLLEXPORT void gks_htmplugin(
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
int img_count = 0;

static
gks_state_list_t *gkss;

typedef unsigned char Byte;
typedef unsigned long uLong;

typedef struct HTM_stream_t
{
  char *buffer;
  size_t size, length;
}
HTM_stream;

typedef struct HTM_point_t
{
  int x, y;
}
HTM_point;

typedef struct ws_state_list_t
{
  int conid, state, wtype;
  double a, b, c, d;
  double window[4], viewport[4];
  int rgb[MAX_COLOR][3];
  double transparency;
  int width, height;
  int color, linewidth;
  double alpha, angle;
  int family, capheight;
  int pattern, have_pattern[PATTERNS];
  HTM_stream *stream, *footer;
  HTM_point *points;
  int npoints, max_points;
  double rect[MAX_TNR][2][2];
  char font[256];
  int valign, halign;
}
ws_state_list;

static
ws_state_list *p;

struct mem_encode
{
  char *buffer;
  size_t size;
};

static
int map[32] = {
  22, 9, 5, 14, 18, 26, 13, 1,
  24, 11, 7, 16, 20, 28, 13, 3,
  23, 10, 6, 15, 19, 27, 13, 2,
  25, 12, 8, 17, 21, 29, 13, 4
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
const char *fonts[] = {
  "Times New Roman", "Arial", "Courier", "Symbol",
  "Bookman Old Style", "Century Schoolbook", "Century Gothic", "Book Antiqua"
};

static
int predef_ints[] = { 0, 1, 3, 3, 3 };

static
int predef_styli[] = { 1, 1, 1, 2, 3 };

static
int predef_font[] = { 1, 1, 1, -2, -3, -4 };

static
int predef_prec[] = { 0, 1, 2, 2, 2, 2 };

static
double a[MAX_TNR], b[MAX_TNR], c[MAX_TNR], d[MAX_TNR];

static
int connect_socket()
{
  int s;
  struct hostent *hp;
  struct sockaddr_in sin;
  int opt;

#if defined(_WIN32) && !defined(__GNUC__)
  WORD wVersionRequested = MAKEWORD(1, 1);
  WSADATA wsaData;

  if (WSAStartup(wVersionRequested, &wsaData) != 0)
    {
      fprintf(stderr, "Can't find a usable WinSock DLL\n");
      return -1;
    }
#endif

  s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (s == -1) {
    perror("socket");
    return -1;
  }

  opt = 1;
#ifdef SO_REUSEADDR
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));
#endif

  if ((hp = gethostbyname("127.0.0.1")) == 0) {
    perror("gethostbyname");
    return -1;
  }

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
  sin.sin_port = htons(PORT);

  if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) == -1) {
    perror("connect");
    return -1;
  }

  return s;
}

static
int send_socket(int s, char *buf, int size)
{
  int sent, n = 0;

  for (sent = 0; sent < size; sent += n) {
    if ((n = send(s, buf + sent, size - sent, 0)) == -1) {
      perror("send");
      return -1;
    }
  }

  return sent;
}

static
int close_socket(int s)
{
  close(s);
#if defined(_WIN32) && !defined(__GNUC__)
  WSACleanup();
#endif
  return 0;
}

static
void htm_memcpy(HTM_stream *p, char *s, size_t n)
{
  if (p->length + n >= p->size)
    {
      while (p->length + n >= p->size)
        p->size += MEMORY_INCREMENT;
      p->buffer = (char *) gks_realloc(p->buffer, p->size);
    }

  memmove(p->buffer + p->length, s, n);
  p->length += n;
}

static
void htm_write(const char *args, ...)
{
  va_list ap;
  char fmt[BUFSIZ], s[BUFSIZ];

  strcpy(fmt, args);

  va_start(ap, args);
  vsprintf (s, fmt, ap);
  va_end(ap);

  htm_memcpy(p->stream, s, strlen(s));
}

static
void htm_write_footer(const char *args, ...)
{
  va_list ap;
  char fmt[BUFSIZ], s[BUFSIZ];

  strcpy(fmt, args);

  va_start(ap, args);
  vsprintf (s, fmt, ap);
  va_end(ap);

  htm_memcpy(p->footer, s, strlen(s));
}

static
HTM_stream *htm_alloc_stream(void)
{
  HTM_stream *p;

  p = (HTM_stream *) calloc(1, sizeof(HTM_stream));
  p->buffer = NULL;
  p->size = p->length = 0;

  return p;
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
void draw_line(double x1, double y1, double x2, double y2)
{
  htm_write("c.moveTo(%.3f, %.3f);\n", x1, y1);
  htm_write("c.lineTo(%.3f, %.3f);\n", x2, y2);
  htm_write("c.stroke();\n");
}

static
void draw_point(double x, double y)
{
  htm_write("c.fillRect(%.3f, %.3f, 1, 1);\n", x, y);
}

static
void draw_marker(double xn, double yn, int mtype, double mscale)
{
  int r;
  double x, y;
  double scale, xr, yr, x1, y1, x2, y2;
  int i, pc, op;

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
        case 1:             /* point */
          draw_point(x, y);
        case 2:             /* line */
          x1 = scale * marker[mtype][pc + 1];
          y1 = scale * marker[mtype][pc + 2];
          seg_xform_rel(&x1, &y1);

          x2 = scale * marker[mtype][pc + 2 + 1];
          y2 = scale * marker[mtype][pc + 2 + 2];
          seg_xform_rel(&x2, &y2);

          draw_line(x-x1, y-y1, x-x2, y-y2);

          pc += 4;
          break;

        case 3:             /* polyline */
        case 4:             /* filled polygon */
        case 5:             /* hollow polygon */
          xr = scale * marker[mtype][pc + 2];
          yr = scale * marker[mtype][pc + 3];
          seg_xform_rel(&xr, &yr);

          htm_write("c.moveTo(%.3f, %.3f);\n", xr, yr);
          htm_write("c.beginPath();\n");

          for (i = 1; i < marker[mtype][pc + 1]; i++)
            {
              xr = scale * marker[mtype][pc + 2 + 2 * i];
              yr = scale * marker[mtype][pc + 3 + 2 * i];
              seg_xform_rel(&xr, &yr);
              htm_write("c.lineTo(%.3f, %.3f);\n", x-xr, y-yr);
            }

          htm_write("c.closePath();\n");

          if (op == 4)
            htm_write("c.fill();\n");
          else
            htm_write("c.stroke();\n");

          pc += 1 + 2 * marker[mtype][pc + 1];
          break;

        case 6:             /* arc */
        case 7:             /* filled arc */
        case 8:             /* hollow arc */
          htm_write("c.beginPath();\n");
          htm_write("c.arc(%.3f, %.3f, %d, %.3f, %.3f, 1);\n",
                    x, y, r, 0.0, 2 * M_PI);
          htm_write("c.closePath();\n");

          if (op == 7)
            htm_write("c.fill();\n");
          else
            htm_write("c.stroke();\n");
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
  int mk_type, mk_color, ln_width;
  double mk_size;
  double x, y;
  int i;

  mk_type = gkss->asf[3] ? gkss->mtype : gkss->mindex;
  mk_size = gkss->asf[4] ? gkss->mszsc : 1;
  mk_color = gkss->asf[5] ? gkss->pmcoli : 1;

  htm_write("set_dashes(c, []);\n");
  htm_write("c.fillStyle = \"rgba(%d,%d,%d,%f)\";\n", p->rgb[mk_color][0],
            p->rgb[mk_color][1], p->rgb[mk_color][2], p->transparency);
  htm_write("c.strokeStyle = \"rgba(%d,%d,%d,%f)\";\n", p->rgb[mk_color][0],
            p->rgb[mk_color][1], p->rgb[mk_color][2], p->transparency);

  ln_width = gkss->version > 4 ? max(1, nint(p->height/500.0)) : 1;
  if (p->linewidth != ln_width)
    {
      p->linewidth = ln_width;
      htm_write("c.lineWidth = %d;\n", ln_width);
    }

  for (i = 0; i < n; i++)
    {
      WC_to_NDC(px[i], py[i], gkss->cntnr, x, y);
      seg_xform(&x, &y);

      draw_marker(x, y, mk_type, mk_size);
    }
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
  p->a = (p->width - 1) / (p->window[1] - p->window[0]);
  p->b = -p->window[0] * p->a;
  p->c = (p->height - 1) / (p->window[2] - p->window[3]);
  p->d = p->height - 1 - p->window[2] * p->c;
}

static
void line_routine(int n, double *px, double *py, int linetype, int tnr)
{
  double x, y, xi, yi;
  int i;

  htm_write("c.beginPath();\n");

  WC_to_NDC(px[0], py[0], tnr, x, y);
  seg_xform(&x, &y);
  NDC_to_DC(x, y, xi, yi);

  htm_write("c.moveTo(%.3f, %.3f);\n", xi, yi);

  for (i = 1; i < n; i++)
    {
      WC_to_NDC(px[i], py[i], tnr, x, y);
      seg_xform(&x, &y);
      NDC_to_DC(x, y, xi, yi);

      htm_write("c.lineTo(%.3f, %.3f);\n", xi, yi);
    }
  htm_write("c.stroke();\n");
}

void
mem_png_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
  struct mem_encode* mem_png = (struct mem_encode*)png_get_io_ptr(png_ptr);
  size_t nsize = mem_png->size + length;
  if(mem_png->buffer)
    mem_png->buffer = (char *)gks_realloc(mem_png->buffer, nsize);
  else
    mem_png->buffer = (char *)gks_malloc(nsize);

  if(!mem_png->buffer)
    png_error(png_ptr, "Write Error");

  memmove(mem_png->buffer + mem_png->size, data, length);
  mem_png->size += length;
}


static
void image_routine(double xmin, double xmax, double ymin, double ymax,
                   int dx, int dy, int dimx, int *colia, int true_color)
{
  double x1, y1, x2, y2, x, y;
  int ix1, ix2, iy1, iy2, ci, i;
  int width, height, j;
  int red, green, blue,alpha;
  int ix, iy, rgb;
  int swapx, swapy;
  png_byte bit_depth = 8;
  png_byte color_type = PNG_COLOR_TYPE_RGBA;
  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep *row_pointers;
  char *enc_png, line[80];
  size_t enc_png_len;
  struct mem_encode io_ptr;
  int length;
  char *data_uri;

  io_ptr.buffer = NULL;
  io_ptr.size = 0;

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

  row_pointers = (png_bytep *) gks_malloc(sizeof(png_bytep) * height);
  for (j = 0; j < height; ++j)
    {
      row_pointers[j] = (png_byte *) gks_malloc(width * 4);
    }
  for (j = 0; j < height; j++)
    {
      png_byte *row = row_pointers[j];
      iy = dy * j / height;
      if (swapy)
        {
          iy = dy - 1 - iy;
        }
      for (i = 0; i < width; i++)
        {
          png_byte *ptr = &(row[i * 4]);
          ix = dx * i / width;
          if (swapx)
            {
              ix = dx - 1 - ix;
            }
          if (!true_color)
            {
              ci = colia[iy * dimx + ix];
              red = p->rgb[ci][0];
              green = p->rgb[ci][1];
              blue = p->rgb[ci][2];
              alpha = (int) 255 * p->transparency;
            }
          else
            {
              rgb = colia[iy * dimx + ix];
              red = (rgb & 0xff);
              green = (rgb & 0xff00) >> 8;
              blue = (rgb & 0xff0000) >> 16;
              alpha = (rgb & 0xff000000) >>24;
            }
          ptr[0] = red;
          ptr[1] = green;
          ptr[2] = blue;
          ptr[3] = alpha;
        }
    }

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  info_ptr = png_create_info_struct(png_ptr);
  png_set_write_fn(png_ptr, &io_ptr, mem_png_write_data, NULL);
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

  enc_png_len = io_ptr.size * 4 / 3 + 4;
  enc_png = (char *) gks_malloc(enc_png_len);
  gks_base64((unsigned char *)io_ptr.buffer, io_ptr.size, enc_png, enc_png_len);
  free(io_ptr.buffer);
  io_ptr.size = 0;

  length = 22 + strlen(enc_png);
  data_uri = (char*) gks_malloc(length);
  strcpy(data_uri, "data:image/png;base64,");
  i = j = 0;
  while (enc_png[j])
    {
      line[i++] = enc_png[j++];
      if (i == 76 || enc_png[j] == '\0')
        {
          line[i] = '\0';
          strcat(data_uri, line);
          i = 0;
        }
    }
  free(enc_png);
  img_count++;
  htm_write("var imageObj%d = new Image();\n", img_count);

  htm_write("imageObj%d.src = \"", img_count);
  htm_memcpy(p->stream, data_uri, length);
  htm_write("\";");

  htm_write("imageObj%d.onload = function() {\n", img_count);
  htm_write("c.drawImage(imageObj%d, %.3f, %.3f);\n", img_count, x, y);

  htm_write_footer("};\n");
  free(data_uri);
}

static
void cellarray(double xmin, double xmax, double ymin, double ymax,
               int dx, int dy, int dimx, int *colia)
{
  image_routine(xmin, xmax, ymin, ymax, dx, dy, dimx, colia, 0);
}

static
void draw_image(double xmin, double xmax, double ymin, double ymax,
                int dx, int dy, int dimx, int *colia)
{
  image_routine(xmin, xmax, ymin, ymax, dx, dy, dimx, colia, 1);
}

static
void set_color_rep(int color, double red, double green, double blue)
{
  if (color >= 0 && color < MAX_COLOR)
    {
      p->rgb[color][0] = (int) (red * 255);
      p->rgb[color][1] = (int) (green * 255);
      p->rgb[color][2] = (int) (blue * 255);
    }
}

static
void set_clip_rect(int tnr)
{
  htm_write("c.restore();\n");
  htm_write("c.save();\n");
  htm_write("c.beginPath();");
  htm_write("c.rect(%.3f, %.3f, %.3f, %.3f);\n",
            p->rect[tnr][0][0], p->rect[tnr][0][1],
            p->rect[tnr][1][0] - p->rect[tnr][0][0],
            p->rect[tnr][1][1] - p->rect[tnr][0][1]);
  htm_write("c.clip();\n");
}

static
void set_clipping(int clip)
{
  gkss->clip = clip;
  set_clip_rect(gkss->cntnr);
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

int reverse(int value)
{
  unsigned char c = value;
  unsigned char result = 0x00;
  int i, j;

  for (i = 0, j = 7; i < 8; i++, j--)
    {
      if (c & (1 << i))
        {
          result |= (1 << j);
        }
    }

  return result;
}

static
void fill_routine(int n, double *px, double *py, int tnr)
{
  int i, j, k;
  double x, y, ix, iy;
  int fl_inter, fl_style;
  int pattern[33], size;

  htm_write("c.beginPath();\n");

  WC_to_NDC(px[0], py[0], tnr, x, y);
  seg_xform(&x, &y);
  NDC_to_DC(x, y, ix, iy);

  htm_write("set_dashes(c, []);\n");
  htm_write("c.moveTo(%.3f, %.3f);\n", ix, iy);
  for (i = 1; i < n; i++)
    {
      WC_to_NDC(px[i], py[i], tnr, x, y);
      seg_xform(&x, &y);
      NDC_to_DC(x, y, ix, iy);
      htm_write("c.lineTo(%.3f, %.3f);\n", ix, iy);
    }
  htm_write("c.closePath();\n");

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
      gks_inq_pattern_array(fl_style, pattern);
      size = pattern[0];
      htm_write("var pcan = document.createElement(\"canvas\");\n");
      htm_write("pcan.width = 8;\n");
      htm_write("pcan.height = %d;\n", size);
      htm_write("var pctx = pcan.getContext(\"2d\");\n");
      htm_write("c.fillStyle = \"rgba(%d,%d,%d,%f)\";\n",
                p->rgb[0], p->rgb[1], p->rgb[2], p->transparency);
      for (j = 1; j < size+1; j++)
        {
          for (i = 0; i < 8; i++)
            {
              k = (1 << i) & pattern[j];
              if (!(k))
                {
                  htm_write("pctx.rect(%d, %d, 1, 1);\n", (i+7) % 8,
                            (j - 1 + (size -1)) % size);
                }
            }
        }
      htm_write("pctx.fill();\n");
      htm_write("var pattern = c.createPattern(pcan, \"repeat\");\n");
      htm_write("c.fillStyle = pattern;\n");
      htm_write("c.fill();\n");
    }
  else if (fl_inter == GKS_K_INTSTYLE_SOLID)
    {
      htm_write("c.fill();\n");
    }
  else
    {
      htm_write("c.stroke();\n");
    }
}

static
void fillarea(int n, double *px, double *py)
{
  int fl_color, ln_width;

  fl_color = gkss->asf[12] ? gkss->facoli : 1;
  ln_width = gkss->version > 4 ? max(nint(p->height/500.0), 1) : 1;

  if (p->linewidth != ln_width)
    {
      p->linewidth = ln_width;
      htm_write("c.lineWidth = %d;\n", ln_width);
    }

  htm_write("c.fillStyle = \"rgba(%d,%d,%d,%f)\";\n", p->rgb[fl_color][0],
            p->rgb[fl_color][1], p->rgb[fl_color][2], p->transparency);
  fill_routine(n, px, py, gkss->cntnr);
}

static
void set_font(int font)
{
  double scale, ux, uy;
  int size;
  double width, height, capheight;
  int bold, italic;
  char font_str[200], tmp[15];

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
  if (p->alpha < 0)
    p->alpha += 2*M_PI;

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

  size = nint(p->capheight / capheights[font-1]);
  if (font > 13)
    font += 3;
  p->family = (font - 1) / 4;
  bold = (font % 4 == 1 || font % 4 == 2) ? 0 : 1;
  italic = (font % 4 == 2 || font % 4 == 0);

  *font_str = '\0';
  if (bold)
    {
      strcat(font_str, "bold ");
    }
  if (italic)
    {
      strcat(font_str, "italic ");
    }
  sprintf(tmp, "%d", size);
  strcat(font_str, tmp);
  strcat(font_str, "px ");
  strcat(font_str, fonts[p->family]);

  htm_write("c.font = \"%s\";\n", font_str);
}

static
void text_routine(double x, double y, int nchars, char *chars)
{
  double xstart, ystart;

  NDC_to_DC(x, y, xstart, ystart);

  if (p->halign != gkss->txal[0])
    {
      if (gkss->txal[0] == GKS_K_TEXT_HALIGN_CENTER)
        htm_write("c.textAlign = \"center\";\n");
      else if (gkss->txal[0] == GKS_K_TEXT_HALIGN_RIGHT)
        htm_write("c.textAlign = \"right\";\n");
      else
        htm_write("c.textAlign = \"left\";\n");
    }

  if (p->valign != gkss->txal[1])
    {
      p->valign = gkss->txal[1];
      if (gkss->txal[1] == GKS_K_TEXT_VALIGN_TOP)
        htm_write("c.textBaseLine = \"top\";\n");
      else if (gkss->txal[1] == GKS_K_TEXT_VALIGN_CAP)
        htm_write("c.textBaseLine = \"hanging\";\n");
      else if (gkss->txal[1] == GKS_K_TEXT_VALIGN_HALF)
        htm_write("c.textBaseLine = \"middle\";\n");
      else if (gkss->txal[1] == GKS_K_TEXT_VALIGN_BOTTOM)
        htm_write("c.textBaseLine = \"bottom\";\n");
      else
        htm_write("c.textBaseLine = \"alphabetic\";\n");
    }

  if (p->alpha > 0)
    {
      htm_write("c.save();\n");
      htm_write("c.translate(%.3f, %.3f);\n", xstart, ystart);
      htm_write("c.rotate(%.3f);\n", ((-1)*p->alpha));
      htm_write("c.fillText(\"%s\", 0, 0);\n", chars);
      htm_write("c.restore();\n");
    }
  else
    {
      htm_write("c.fillText(\"%s\", %.3f, %.3f);\n", chars, xstart, ystart);
    }
}

static
void text(double px, double py, int nchars, char *chars)
{
  int tx_font, tx_prec, tx_color;
  double x, y;

  tx_font = gkss->asf[6] ? gkss->txfont : predef_font[gkss->tindex - 1];
  tx_prec = gkss->asf[6] ? gkss->txprec : predef_prec[gkss->tindex - 1];
  tx_color = gkss->asf[9] ? gkss->txcoli : 1;

  htm_write("c.fillStyle = \"rgba(%d,%d,%d,%f)\";\n", p->rgb[tx_color][0],
            p->rgb[tx_color][1], p->rgb[tx_color][2], p->transparency);
  htm_write("c.strokeStyle = \"rgba(%d,%d,%d,%f)\";\n", p->rgb[tx_color][0],
            p->rgb[tx_color][1], p->rgb[tx_color][2], p->transparency);

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
void polyline(int n, double *px, double *py)
{
  int ln_type, ln_color, width;
  double ln_width;
  double x, y, ix, iy;
  int i, dashes[10];

  ln_type = gkss->asf[0] ? gkss->ltype : gkss->lindex;
  ln_width = gkss->asf[1] ? gkss->lwidth : 1;
  ln_color = gkss->asf[2] ? gkss->plcoli : 1;

  if (gkss->version > 4)
    width = nint(ln_width * (p->width + p->height) * 0.001);
  else
    width = nint(ln_width);
  if (ln_color <= 0 || ln_color >= MAX_COLOR)
    ln_color = 1;
  if (width < 1)
    width = 1;

  htm_write("c.strokeStyle = \"rgba(%d,%d,%d,%f)\";\n", p->rgb[ln_color][0],
            p->rgb[ln_color][1], p->rgb[ln_color][2], p->transparency);
  if (p->linewidth != width)
    {
      p->linewidth = width;
      htm_write("c.lineWidth = %d;\n", width);
    }

  htm_write("c.beginPath();\n");
  gks_get_dash_list(ln_type, ln_width, dashes);
  htm_write("set_dashes(c, [");
  for (i = 1; i <= dashes[0]; i++)
    {
      htm_write("%d, ", dashes[i]);
    }
  htm_write("]);\n");

  WC_to_NDC(px[0], py[0], gkss->cntnr, x, y);
  seg_xform(&x, &y);
  NDC_to_DC(x, y, ix, iy);

  htm_write("c.moveTo(%.3f, %.3f);\n", ix, iy);
  for (i = 1; i < n; i++)
    {
      WC_to_NDC(px[i], py[i], gkss->cntnr, x, y);
      seg_xform(&x, &y);
      NDC_to_DC(x, y, ix, iy);
      htm_write("c.lineTo(%.3f, %.3f);\n", ix, iy);
    }
  htm_write("c.stroke();\n");
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
  htm_write("c.globalAlpha = %f;\n",alpha);
}

static
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

  set_xform();
  set_norm_xform(tnr, gkss->window[tnr], gkss->viewport[tnr]);
  gks_set_norm_xform(tnr, gkss->window[tnr], gkss->viewport[tnr]);
  if (tnr == gkss->cntnr) {
    set_clip_rect(tnr);
  }
}

static
void write_page(void)
{
  if (p->conid >= 0)
    {
      htm_memcpy(p->stream, p->footer->buffer, p->footer->length);
      p->footer->length = 0;
      send_socket(p->conid, (char *) &p->stream->length, sizeof(int));
      send_socket(p->conid, p->stream->buffer, p->stream->length);

      p->stream->length = 0;
    } else {
    gks_perror("can't open socket");
    perror("open");
  }
}

void resize_window(void)
{
  p->width = nint((p->viewport[1] - p->viewport[0]) / MWIDTH * WIDTH);
  p->height = nint((p->viewport[3] - p->viewport[2]) / MHEIGHT * HEIGHT);
}

void gks_htmplugin(
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

      if ((p->conid = connect_socket()) < 0) {
        printf("%d", p->conid);
        gks_perror("can't open socket");
        perror("open");
      }

      p->height = 500;
      p->width = 500;
      p->window[0] = p->window[2] = 0.0;
      p->window[1] = p->window[3] = 1.0;
      p->viewport[0] = p->viewport[2] = 0;
      p->viewport[1] = (double) p->width * MWIDTH / WIDTH;
      p->viewport[3] = (double) p->height * MHEIGHT / HEIGHT;

      p->stream = htm_alloc_stream();
      p->footer = htm_alloc_stream();

      p->max_points = MAX_POINTS;
      p->points = (HTM_point *) gks_malloc(p->max_points * sizeof(HTM_point));
      p->npoints = 0;

      p->transparency  = 1.0;

      set_xform();
      init_norm_xform();
      init_colors();

      for (i = 0; i < PATTERNS; i++)
        p->have_pattern[i] = 0;

      *ptr = p;
      break;

    case 3:
      /* close workstation */
      if (p->stream->length > 0)
        write_page();

      if (p->conid != 0)
        close_socket(p->conid);

      free(p->stream->buffer);
      free(p->footer->buffer);
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
      break;

    case 8:
      /* update workstation */
      if (p->stream->length > 0)
        write_page();
      img_count = 0;
      break;

    case 12:
      /* polyline */
      if (p->state == GKS_K_WS_ACTIVE)
        polyline(ia[0], r1, r2);
      break;

    case 13:
      /* polymarker */
      if (p->state == GKS_K_WS_ACTIVE)
        polymarker(ia[0], r1, r2);
      break;

    case 14:
      /* text */
      if (p->state == GKS_K_WS_ACTIVE)
        text(r1[0], r2[0], strlen(chars), chars);
      break;

    case 15:
      /* fill area */
      if (p->state == GKS_K_WS_ACTIVE)
        fillarea(ia[0], r1, r2);
      break;

    case 16:
      /* cell array */
      if (p->state == GKS_K_WS_ACTIVE)
        cellarray(r1[0], r1[1], r2[0], r2[1], dx, dy, dimx, ia);
      break;

    case 48:
      /* set color representation */
      if (p->state == GKS_K_WS_ACTIVE)
        set_color_rep(ia[1], r1[0], r1[1], r1[2]);
      break;

    case 49:
      /* set window */
      if (p->state == GKS_K_WS_ACTIVE)
        set_window(gkss->cntnr, r1[0], r1[1], r2[0], r2[1]);
      break;

    case 50:
      /* set viewport */
      if (p->state == GKS_K_WS_ACTIVE)
        set_viewport(gkss->cntnr, r1[0], r1[1], r2[0], r2[1]);
      break;

    case 52:
      /* select normalization transformation */
      if (p->state == GKS_K_WS_ACTIVE)
        select_xform(ia[0]);
      break;

    case 53:
      /* set clipping inidicator */
      if (p->state == GKS_K_WS_ACTIVE)
        set_clipping(ia[0]);
      break;

    case 201:
      /* draw_image */
      if (p->state == GKS_K_WS_ACTIVE)
        draw_image(r1[0], r1[1], r2[0], r2[1], dx, dy, dimx, ia);
      break;

    case 203:
      /* set transparency */
      if (p->state == GKS_K_WS_ACTIVE)
        set_transparency(r1[0]);
      break;
    }
}
