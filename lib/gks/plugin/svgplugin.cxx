
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

#define DrawBorder 0

#define TMP_NAME "gks_svg.tmp"

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

DLLEXPORT void gks_svgplugin(
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
  xd = (p->a * (xn) + p->b); \
  yd = (p->c * (yn) + p->d)

#define CharXform(xrel, yrel, x, y) \
  x = cos(p->phi) * (xrel) - sin(p->phi) * (yrel); \
  y = sin(p->phi) * (xrel) + cos(p->phi) * (yrel);

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

typedef struct SVG_stream_t
{
  Byte *buffer;
  uLong size, length;
}
SVG_stream;

typedef struct SVG_point_t
{
  double x, y;
}
SVG_point;

typedef struct ws_state_list_t
{
  int conid, state, wtype;
  char *path;
  double a, b, c, d;
  double window[4], viewport[4];
  char rgb[MAX_COLOR][7];
  int width, height;
  int color;
  double linewidth;
  double phi, angle;
  int family, capheight;
  int pattern, have_pattern[PATTERNS];
  SVG_stream *stream;
  SVG_point *points;
  int npoints, max_points;
  int empty, page_counter, offset;
  int cx[MAX_TNR], cy[MAX_TNR], cwidth[MAX_TNR], cheight[MAX_TNR];
  int clip_index, path_index, path_counter;
  double transparency;
}
ws_state_list;

static
ws_state_list *p;

static
const char *fonts[] = {
  "Times New Roman,TimesNewRoman,Times,Baskerville,Georgia,serif",
  "Arial,Helvetica Neue,Helvetica,sans-serif",
  "Courier,Courier New,Lucida Sans Typewriter,Lucida Typewriter,monospace",
  "Symbol",
  "Bookman Old Style,Bookman,Georgia,serif",
  "Century Schoolbook,Georgia,serif",
  "Century Gothic,CenturyGothic,AppleGothic,sans-serif",
  "Book Antiqua,Palatino,Palatino Linotype,Palatino LT STD,Georgia,serif"
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
void svg_memcpy(SVG_stream *p, char *s, size_t n)
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
void svg_printf(SVG_stream *p, const char *args, ...)
{
  va_list ap;
  char fmt[BUFSIZ], s[BUFSIZ];

  strcpy(fmt, args);

  va_start(ap, args);
  vsprintf(s, fmt, ap);
  va_end(ap);

  svg_memcpy(p, s, strlen(s));
}

static
SVG_stream *svg_alloc_stream(void)
{
  SVG_stream *p;

  p = (SVG_stream *) calloc(1, sizeof(SVG_stream));
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
void init_clippaths(void)
{
  p->path_counter = MAX_TNR;
  p->clip_index = 0;
  for (int i = 0; i < MAX_TNR; i++)
    {
      p->cx[i] = p->cy[i] = -1;
      p->cwidth[i] = p->cheight[i] = 0;
    }
}

static
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
void create_pattern(void)
{
  int i, j, height;
  int parray[33];
  png_byte bit_depth = 1;
  png_byte color_type = PNG_COLOR_TYPE_GRAY;
  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep *row_pointers;
  FILE *stream;

  stream = fopen(TMP_NAME, "wb");

  row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * 8);
  for (i = 0; i < 8; i++)
    {
      row_pointers[i] = (png_byte *) malloc(sizeof(png_byte));
    }
  gks_inq_pattern_array(p->pattern, parray);
  height = (*parray == 32) ? 16 : (*parray == 4) ? 8 : *parray;
  for (j = *parray; j < height; j++)
    {
      parray[j + 1] = parray[j % *parray + 1];
    }
  for (j = 0; j < 8; ++j)
    {
      png_byte *row = row_pointers[j];
      png_byte *ptr = row;
      *ptr = reverse(parray[j + 1]);
    }

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  info_ptr = png_create_info_struct(png_ptr);
  png_init_io(png_ptr, stream);
  png_set_IHDR(png_ptr, info_ptr, 8, 8, bit_depth, color_type,
	       PNG_FILTER_TYPE_BASE, PNG_COMPRESSION_TYPE_BASE,
	       PNG_FILTER_TYPE_BASE);
  png_write_info(png_ptr, info_ptr);
  png_write_image(png_ptr, row_pointers);
  png_write_end(png_ptr, NULL);
  fclose(stream);
  for (j = 0; j < 8; ++j)
    {
      free(row_pointers[j]);
    }
  free(row_pointers);
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
  int pc, op;

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
	  svg_printf(p->stream,
		     "<line clip-path=\"url(#clip%02d)\" x1=\"%g\" y1=\"%g\" "
		     "x2=\"%g\" y2=\"%g\" style=\"stroke:#%s; stroke-opacity:%g\"/>\n",
		     p->path_index, x, y, x + 1, y, p->rgb[mcolor], p->transparency);
	  break;

	case 2:		/* line */
	  for (i = 0; i < 2; i++)
	    {
	      xr = scale * marker[mtype][pc + 2 * i + 1];
	      yr = -scale * marker[mtype][pc + 2 * i + 2];
	      seg_xform_rel(&xr, &yr);
	      if (i == 0)
		svg_printf(p->stream,
			   "<line clip-path=\"url(#clip%02d)\" x1="
			   "\"%g\" y1=\"%g\" ", p->path_index,
		           x - xr, y - yr);
	      else
		svg_printf(p->stream, "x2=\"%g\" y2=\"%g\" "
                           "style=\"stroke:#%s; stroke-opacity:%g\"/>\n",
		           x - xr, y - yr, p->rgb[mcolor], p->transparency);
	    }
	  pc += 4;
	  break;

	case 3:		/* polyline */
	  svg_printf(p->stream,
		     "<polyline clip-path=\"url(#clip%02d)\" style=\"stroke:"
		     "#%s; stroke-opacity:%g; fill:none\" points=\"\n  ", p->path_index,
		     p->rgb[mcolor], p->transparency);
	  for (i = 0; i < marker[mtype][pc + 1]; i++)
	    {
	      xr = scale * marker[mtype][pc + 2 + 2 * i];
	      yr = -scale * marker[mtype][pc + 3 + 2 * i];
	      seg_xform_rel(&xr, &yr);
	      svg_printf(p->stream, "%g,%g ", x - xr, y + yr);
	      if (!((i + 1) % 10))
		{
		  svg_printf(p->stream, "\n  ");
		}
	    }
	  svg_printf(p->stream, "\n  \"/>\n");
	  pc += 1 + 2 * marker[mtype][pc + 1];
	  break;

	case 4:		/* filled polygon */
	case 5:		/* hollow polygon */
	  if (op == 5)
	    svg_printf(p->stream,
		       "<polygon clip-path=\"url(#clip%02d)\" "
		       "style=\"fill:#%s; fill-opacity:%g\" points=\"",
                       p->path_index, p->rgb[0], p->transparency);
	  else
	    svg_printf(p->stream,
		       "<polygon clip-path=\"url(#clip%02d)\" "
		       "style=\"fill:#%s; fill-opacity:%g\" points=\"\n  ",
                       p->path_index, p->rgb[mcolor], p->transparency);
	  for (i = 0; i < marker[mtype][pc + 1]; i++)
	    {
	      xr = scale * marker[mtype][pc + 2 + 2 * i];
	      yr = -scale * marker[mtype][pc + 3 + 2 * i];
	      seg_xform_rel(&xr, &yr);
	      svg_printf(p->stream, "%g,%g ", x - xr, y + yr);
	      if (!((i + 1) % 10))
		{
		  svg_printf(p->stream, "\n  ");
		}
	    }
	  svg_printf(p->stream, "\n  \"/>\n");
	  pc += 1 + 2 * marker[mtype][pc + 1];
	  if (op == 5)
	    p->color = mcolor;
	  break;

	case 6:		/* arc */
	  svg_printf(p->stream,
		     "<circle clip-path=\"url(#clip%02d)\" style=\"fill:none; "
		     "stroke:#%s; stroke-opacity:%g\" "
                     "cx=\"%g\" cy=\"%g\" r=\"%d\"/>\n",
		     p->path_index, p->rgb[mcolor], p->transparency, x, y, r);
	  break;

	case 7:		/* filled arc */
	case 8:		/* hollow arc */
	  if (op == 8)
	    svg_printf(p->stream,
		       "<circle clip-path=\"url(#clip%02d)\" style=\"fill:none; "
		       "stroke:#%s; stroke-opacity:%g\" "
                       "cx=\"%g\" cy=\"%g\" r=\"%d\"/>\n",
		       p->path_index, p->rgb[0], p->transparency, x, y, r);
	  else
	    svg_printf(p->stream,
		       "<circle clip-path=\"url(#clip%02d)\" style=\"fill:#%s; "
		       "stroke:none; fill-opacity:%g\" "
                       "cx=\"%g\" cy=\"%g\" r=\"%d\"/>\n",
		       p->path_index, p->rgb[mcolor], p->transparency, x, y, r);
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

  svg_printf(p->stream, "<polyline clip-path=\"url(#clip%02d)\" style=\""
	     "stroke:#%s; stroke-width:%g; stroke-opacity:%g; fill:none\" ",
	     p->path_index, p->rgb[p->color], p->linewidth, p->transparency);
  svg_printf(p->stream, "points=\"\n  ");
  for (i = 0; i < p->npoints; i++)
    {
      svg_printf(p->stream, "%g,%g ", p->points[i].x, p->points[i].y);
      if (!((i + 1) % 10))
	{
	  svg_printf(p->stream, "\n  ");
	}
    }
  svg_printf(p->stream, "\n  \"/>\n");

  p->npoints = 0;
}

static
void line_routine(int n, double *px, double *py, int linetype, int tnr)
{
  double x, y;
  int i, len;
  double x0, y0, xi, yi, xim1, yim1;
  int dash_list[10];
  char s[100], buf[20];

  WC_to_NDC(px[0], py[0], tnr, x, y);
  seg_xform(&x, &y);
  NDC_to_DC(x, y, x0, y0);

  svg_printf(p->stream, "<polyline clip-path=\"url(#clip%02d)\" style=\""
	     "stroke:#%s; stroke-width:%g; stroke-opacity:%g; fill:none\" ",
	     p->path_index, p->rgb[p->color], p->linewidth, p->transparency);
  if (linetype < 0 || linetype > 1)
    {
      gks_get_dash_list(linetype, 0.5 * p->linewidth, dash_list);
      len = dash_list[0];
      *s = '\0';
      for (i = 1; i <= len; i++)
        {
          sprintf(buf, "%d%s", dash_list[i], i < len ? ", " : "");
          strcat(s, buf);
        }
      svg_printf(p->stream, "stroke-dasharray=\"%s\" ", s);
    }
  svg_printf(p->stream, "points=\"\n  %g,%g ", x0, y0);

  xim1 = x0;
  yim1 = y0;
  for (i = 1; i < n; i++)
    {
      WC_to_NDC(px[i], py[i], tnr, x, y);
      seg_xform(&x, &y);
      NDC_to_DC(x, y, xi, yi);

      if (i == 1 || xi != xim1 || yi != yim1)
	{
	  svg_printf(p->stream, "%g,%g ", xi, yi);
	  xim1 = xi;
	  yim1 = yi;
	}
      if (!((i + 1) % 10))
	{
	  svg_printf(p->stream, "\n  ");
	}
    }
  if (linetype == 0)
    svg_printf(p->stream, "%g,%g", x0, y0);
  svg_printf(p->stream, "\n  \"/>\n");
}

static
char *base64_stream(const char *path)
{
  FILE *stream;
  struct stat buf;
  unsigned char *src;
  char *dest;
  size_t srclen, destlen;

  if ((stream = fopen(path, "rb")) == NULL)
    {
      gks_perror("can't open image file");
      perror("fopen");
      return NULL;
    }

  fstat(fileno(stream), &buf);
  srclen = buf.st_size;
  src = (unsigned char *) gks_malloc(srclen);

  if (fread(src, srclen, 1, stream) == 0)
    {
      gks_perror("can't read image file");
      perror("fread");
      return NULL;
    }
  fclose(stream);

  destlen = buf.st_size * 4 / 3 + 4;
  dest = (char *) gks_malloc(destlen);

  gks_base64(src, srclen, dest, destlen);

  free(src);

  return dest;
}

static
void fill_routine(int n, double *px, double *py, int tnr)
{
  int i, j;
  double x, y, ix, iy;
  char *s, line[80];

  if (p->pattern && !p->have_pattern[p->pattern])
    {
      create_pattern();
      p->have_pattern[p->pattern] = 1;
      svg_printf(p->stream,
		"<defs>\n  <pattern id=\"pattern%d\" patternUnits=\"userSpaceOn"
		"Use\" x=\"0\" y=\"0\" width=\"8\" height=\"8\">\n"
		"<image width=\"8\" height=\"8\" xlink:href=\"data:;base64,\n",\
		p->pattern + 1);
      s = base64_stream(TMP_NAME);
      remove(TMP_NAME);
      i = j = 0;
      while (s[j])
	{
	  line[i++] = s[j++];
	  if (i == 76 || s[j] == '\0')
	    {
	      line[i] = '\0';
	      svg_printf(p->stream, "%s\n", line);
	      i = 0;
	    }
	}
      free(s);
      svg_printf(p->stream, "\"/>\n  </pattern>\n</defs>\n");
    }

  svg_printf(p->stream, "<polygon clip-path=\"url(#clip%02d)\" points=\"\n",
	     p->path_index);
  for (i = 0; i < n; i++)
    {
      WC_to_NDC(px[i], py[i], tnr, x, y);
      seg_xform(&x, &y);
      NDC_to_DC(x, y, ix, iy);

      svg_printf(p->stream, "%g,%g ", ix, iy);
      if (!((i + 1) % 10))
	{
	  svg_printf(p->stream, "\n  ");
	}
    }
  if (p->pattern)
    svg_printf(p->stream, "\n  \" fill=\"url(#pattern%d)\"", p->pattern + 1);
  else
    svg_printf(p->stream, "\n  \" fill=\"#%s\" fill-opacity=\"%g\"",
               p->rgb[p->color], p->transparency);
  svg_printf(p->stream, "/>\n");
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
  double ln_width, width;

  if (n > p->max_points)
  {
    p->points = (SVG_point *) realloc(p->points, n * sizeof(SVG_point));
    p->max_points = n;
  }

  ln_type = gkss->asf[0] ? gkss->ltype : gkss->lindex;
  ln_width = gkss->asf[1] ? gkss->lwidth : 1;
  ln_color = gkss->asf[2] ? gkss->plcoli : 1;

  if (gkss->version > 4)
    width = ln_width * (p->width + p->height) * 0.001;
  else
    width = ln_width;

  p->linewidth = width;
  p->color = ln_color;

  line_routine(n, px, py, ln_type, gkss->cntnr);

  if (p->npoints > 0)
    stroke();
}

static
void text_routine(double x, double y, int nchars, char *chars)
{
  double xstart, ystart;
  int width, height, ch;
  double xrel, yrel, ax, ay;
  char utf[4];
  size_t len;

  NDC_to_DC(x, y, xstart, ystart);

  width = 0;
  height = p->capheight;

  xrel = width * xfac[gkss->txal[0]];
  yrel = height * yfac[gkss->txal[1]];
  CharXform(xrel, yrel, ax, ay);
  xstart += ax;
  ystart -= ay;

  if (gkss->txal[0] == GKS_K_TEXT_HALIGN_CENTER)
    svg_printf(p->stream, "text-anchor:middle;");
  else if (gkss->txal[0] == GKS_K_TEXT_HALIGN_RIGHT)
    svg_printf(p->stream, "text-anchor:end;");
  else
    svg_printf(p->stream, "text-anchor:start;");
  svg_printf(p->stream, "\" ");
  svg_printf(p->stream, "transform=\"rotate(%.4g", p->angle);
  svg_printf(p->stream, ", %g, %g)\" ", xstart, ystart);
  svg_printf(p->stream, "x=\"%g\" y=\"%g\"", xstart, ystart);
  svg_printf(p->stream, ">");

  for (int i = 0; i < nchars; ++i)
    {
      ch = (chars[i] < 0) ? chars[i] + 256 : chars[i];
      switch (ch)
	{
	case '&':
	  svg_printf(p->stream, "&amp;");
	  break;
	case '<':
	  svg_printf(p->stream, "&lt;");
	  break;
	case '>':
	  svg_printf(p->stream, "&gt;");
	  break;
	case '"':
	  svg_printf(p->stream, "&quot;");
	  break;
	case '\'':
	  svg_printf(p->stream, "&apos;");
	  break;
	default:
	  if (p->family == 3)
	    gks_symbol2utf(ch, utf, &len);
          else
	    gks_iso2utf(ch, utf, &len);
	  utf[len] = '\0';
          svg_printf(p->stream, "%s", utf);
	  break;
	}
    }
  svg_printf(p->stream, "<");
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

  p->phi = -atan2(ux, uy);
  angle = p->phi * 180 / M_PI;
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

  svg_printf(p->stream, "font-family:%s; font-size:%d; ",
             fonts[p->family], size);
  if (bold && italic)
    svg_printf(p->stream, "font-style:italic; font-weight:bold; ");
  else if (italic)
    svg_printf(p->stream, "font-style:italic; ");
  else if (bold)
    svg_printf(p->stream, "font-weight:bold; ");
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
      svg_printf(p->stream,
		 "<g clip-path=\"url(#clip%02d)\">\n<text style=\""
		 "fill:#%s; fill-opacity:%g; ", p->path_index, p->rgb[tx_color],
                 p->transparency);
      set_font(tx_font);

      WC_to_NDC(px, py, gkss->cntnr, x, y);
      seg_xform(&x, &y);

      text_routine(x, y, nchars, chars);
      svg_printf(p->stream, "/text>\n</g>\n");
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
  double x1, y1, x2, y2;
  double ix1, ix2, iy1, iy2;
  int x, y, width, height;
  int red, green, blue, alpha;
  int i, j, ix, iy, ind, rgb;
  int swapx, swapy;
  png_byte bit_depth = 8;
  png_byte color_type = PNG_COLOR_TYPE_RGBA;
  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep *row_pointers;
  FILE *stream;
  char *s, line[80];

  WC_to_NDC(xmin, ymax, gkss->cntnr, x1, y1);
  seg_xform(&x1, &y1);
  NDC_to_DC(x1, y1, ix1, iy1);

  WC_to_NDC(xmax, ymin, gkss->cntnr, x2, y2);
  seg_xform(&x2, &y2);
  NDC_to_DC(x2, y2, ix2, iy2);

  width = nint(fabs(ix2 - ix1));
  height = nint(fabs(iy2 - iy1));
  if (width == 0 || height == 0) return;
  x = nint(min(ix1, ix2));
  y = nint(min(iy1, iy2));

  if ((stream = fopen(TMP_NAME, "wb")) == NULL)
    {
      gks_perror("can't open temporary file");
      perror("open");
      return;
    }

  swapx = ix1 > ix2;
  swapy = iy1 < iy2;

  row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * height);
  for (j = 0; j < height; ++j)
    {
      row_pointers[j] = (png_byte *) malloc(width * 4);
    }
  for (j = 0; j < height; j++)
    {
      png_byte *row = row_pointers[j];
      iy = dy * j / height;
      if (swapy)
	iy = dy - 1 - iy;
      for (i = 0; i < width; i++)
	{
	  png_byte *ptr = &(row[i * 4]);
	  ix = dx * i / width;
	  if (swapx)
	    ix = dx - 1 - ix;
	  if (!true_color)
	    {
	      ind = colia[iy * dimx + ix];
	      sscanf(p->rgb[ind], "%02x%02x%02x", &red, &green, &blue);
              alpha = 0xff;
	    }
	  else
	    {
	      rgb   = colia[iy * dimx + ix];
	      red   = (rgb & 0xff);
	      green = (rgb & 0xff00) >> 8;
	      blue  = (rgb & 0xff0000) >> 16;
	      alpha = (rgb & 0xff000000) >> 24;
	    }
	  ptr[0] = red;
	  ptr[1] = green;
	  ptr[2] = blue;
	  ptr[3] = alpha;
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

  s = base64_stream(TMP_NAME);
  remove(TMP_NAME);
  svg_printf(p->stream,
	     "<g clip-path=\"url(#clip%02d)\">\n"
	     "<image width=\"%d\" height=\"%d\" xlink:href=\"data:;base64,\n",
	     p->path_index, width, height);
  i = j = 0;
  while (s[j])
    {
      line[i++] = s[j++];
      if (i == 76 || s[j] == '\0')
	{
	  line[i] = '\0';
	  svg_printf(p->stream, "%s\n", line);
	  i = 0;
	}
    }
  svg_printf(p->stream, "\" transform=\"translate(%d, %d)\"/>\n</g>\n", x, y);
  free(s);
}

static
void set_clip_path(int tnr)
{
  double *vp;
  int x, y, width, height;
  double cxl, cxr, cyb, cyt;
  int i, found = 0, index;

  if (gkss->clip == GKS_K_CLIP)
    vp = gkss->viewport[tnr];
  else
    vp = gkss->viewport[0];

  NDC_to_DC(vp[0], vp[2], cxl, cyb);
  NDC_to_DC(vp[1], vp[3], cxr, cyt);

  x = (int) cxl;
  y = (int) cyt;
  width = nint(cxr - cxl);
  height = nint(cyb - cyt);

  x = max(0, x);
  width = min(p->width, width + 1);
  y = max(0, y);
  height = min(p->height, height + 1);

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
	  svg_printf(p->stream,
		     "<defs>\n  <clipPath id=\"clip%02d\">\n    <rect"
		     " x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\"/>\n  </clip"
		     "Path>\n</defs>\n", p->path_index, x, y, width,
		     height);
	  p->clip_index++;
	}
      else
	{
	  svg_printf(p->stream,
		     "<defs>\n  <clipPath id=\"clip%02d\">\n    <rect"
		     " x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\"/>\n  </clip"
		     "Path>\n</defs>\n", p->path_counter, x, y, width,
		     height);
	  p->path_index = p->path_counter++;
	}
    }
}

static
void write_page(void)
{
  char path[MAXPATHLEN], buf[256];
  int fd;

  p->page_counter++;

  if (p->conid == 0)
    {
      gks_filepath(path, p->path, "svg", p->page_counter, 0);
      fd = gks_open_file(path, "w");
    }
  else
    fd = p->conid;

  if (fd >= 0)
    {
      sprintf(buf, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
              "<svg xmlns=\"http://www.w3.org/2000/svg\" "
	      "xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
	      "width=\"%d\" height=\"%d\" viewBox=\"0 0 %d %d\">\n",
	     p->width, p->height, p->width, p->height);
      gks_write_file(fd, buf, strlen(buf));
      gks_write_file(fd, p->stream->buffer, p->stream->length);
      sprintf(buf, "</svg>\n");
      gks_write_file(fd, buf, strlen(buf));
      if (fd != p->conid)
        gks_close_file(fd);

      p->stream->length = 0;
    }
  else
    {
      gks_perror("can't open SVG file");
      perror("open");
    }
}

#ifndef EMSCRIPTEN
void gks_svgplugin(
#else
void gks_drv_js(
#endif
  int fctid, int dx, int dy, int dimx, int *ia,
  int lr1, double *r1, int lr2, double *r2,
  int lc, char *chars, void **ptr)
{
  int i;

  p = (ws_state_list *) *ptr;

  switch (fctid)
    {
/* open workstation */
    case 2:
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

      p->stream = svg_alloc_stream();

      p->max_points = MAX_POINTS;
      p->points = (SVG_point *) gks_malloc(p->max_points * sizeof(SVG_point));
      p->npoints = 0;

      p->empty = 1;
      p->page_counter = 0;
      p->offset = 0;

      p->linewidth = 1.0;
      p->transparency = 1.0;

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
      p->transparency = r1[0];
      break;

    default:
      ;
    }
}
