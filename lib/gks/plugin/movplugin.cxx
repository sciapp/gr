
#if !defined(NO_AV) && !defined(NO_MUPDF)

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <time.h>

#if !defined(VMS) && !defined(_WIN32)
#include <unistd.h>
#endif

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

DLLEXPORT void gks_movplugin(
  int fctid, int dx, int dy, int dimx, int *i_arr,
  int len_f_arr_1, double *f_arr_1, int len_f_arr_2, double *f_arr_2,
  int len_c_arr, char *c_arr, void **ptr);

#ifdef _WIN32
#ifdef __cplusplus
}
#endif
#endif

#if !defined(NO_AV) && !defined(NO_MUPDF)

#include "vc.h"
#include "gif.h"

#ifdef HAVE_ZLIB
#include <zlib.h>
#else
typedef unsigned char Byte;
typedef unsigned long uLong;
#endif

#define MAX_FONT 31
#define HATCH_STYLE 108
#define PATTERNS 120

#define MEMORY_INCREMENT 32768

#define MAX_OBJECTS 2500
#define MAX_PAGES 250
#define MAX_IMAGES 2500

#define NO_OF_BUFS 10

#define DrawBorder 0

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
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

#define CharXform(phi, xrel, yrel, x, y) \
  x = cos(phi) * (xrel) - sin(phi) * (yrel); \
  y = sin(phi) * (xrel) + cos(phi) * (yrel);

#define nint(a) ((int)(a + 0.5))

#define pdf_obj(p, id) \
  p->byte_offset[id] = p->stream->length; \
  pdf_printf(p->stream, "%ld 0 obj\n", id);

#define pdf_endobj(p)           pdf_printf(p->stream, "endobj\n")
#define pdf_dict(p)             pdf_printf(p->stream, "<<\n")
#define pdf_enddict(p)          pdf_printf(p->stream, ">>\n")
#define pdf_stream(p)           pdf_printf(p->stream, "stream\n")
#define pdf_endstream(p)        pdf_printf(p->stream, "endstream\n")

#define pdf_save(p)             pdf_printf(p->content, "q\n")
#define pdf_restore(p)          pdf_printf(p->content, "Q\n")
#define pdf_clip(p)             pdf_printf(p->content, "W n\n")
#define pdf_moveto(p, x, y)     pdf_printf(p->content, "%.2f %.2f m\n", x, y)
#define pdf_lineto(p, x, y)     pdf_printf(p->content, "%.2f %.2f l\n", x, y)
#define pdf_closepath(p)        pdf_printf(p->content, "h\n")
#define pdf_stroke(p)           pdf_printf(p->content, "S\n")
#define pdf_eofill(p)           pdf_printf(p->content, "f*\n")
#define pdf_point(p, x, y)      pdf_printf(p->content, "%.2f %.2f ", x, y)
#define pdf_curveto(p)          pdf_printf(p->content, "c\n")
#define pdf_setdash(p, dash)    pdf_printf(p->content, "%s 0 d\n", dash)

#define pdf_setlinewidth(p, width) \
  pdf_printf(p->content, "%s w\n", pdf_double(width))

#define pdf_text(p, xorg, yorg, text) \
  pdf_printf(p->content, "BT\n/F%d %d Tf\n%.2f %.2f Td\n(%s) Tj\nET\n", \
  p->font, p->pt, xorg, yorg, text)

#define pdf_setrgbcolor(p, red, green, blue) \
  pdf_printf(p->content, "%s %s %s RG\n", \
    pdf_double(red), pdf_double(green), pdf_double(blue))

#define pdf_setfillcolor(p, red, green, blue) \
  pdf_printf(p->content, "%s %s %s rg\n", \
    pdf_double(red), pdf_double(green), pdf_double(blue))

#define pdf_setalpha(p, alpha) \
  pdf_printf(p->content, "/GS%d gs\n", alpha)

#define PDF ws_state_list

typedef struct PDF_stream_t
  {
    Byte *buffer;
    uLong size, length;
  }
PDF_stream;

typedef struct PDF_image_t
  {
    long object;
    int width, height;
    int *data;
  }
PDF_image;

typedef struct PDF_page_t
  {
    long object, contents, fonts[MAX_FONT];
    double width, height;
    PDF_stream *stream;
    int first_image, last_image;
  }
PDF_page;

typedef struct ws_state_list_t
  {
    int state;
    char *path;
    int fd;
    int wstype;
    double window[4], viewport[4];
    int empty;
    int width, height;
    double a, b, c, d;
    int stroke;
    double lastx, lasty;
    double red[MAX_COLOR], green[MAX_COLOR], blue[MAX_COLOR];
    int color, fillcolor, alpha, ltype, font, size, pt;
    double lwidth, angle;
    PDF_stream *stream;
    long object_number;
    long info, root, outlines, pages;
    long *byte_offset;
    int max_objects;
    PDF_page **page;
    int current_page, max_pages;
    PDF_stream *content;
    int compress;
    int have_alpha[256];
    int pattern;
    int have_pattern[PATTERNS];
    int pattern_id[PATTERNS][2];
    PDF_image **image;
    int images, max_images;
  }
ws_state_list;

static
ws_state_list *p;

static
gks_state_list_t *gkss;

static
double a[MAX_TNR], b[MAX_TNR], c[MAX_TNR], d[MAX_TNR];

static
const char *fonts[MAX_FONT] =
  {
    "Times-Roman", "Times-Italic", "Times-Bold", "Times-BoldItalic",
    "Helvetica", "Helvetica-Oblique", "Helvetica-Bold", "Helvetica-BoldOblique",
    "Courier", "Courier-Oblique", "Courier-Bold", "Courier-BoldOblique",
    "Symbol",
    "Bookman-Light", "Bookman-LightItalic", "Bookman-Demi",
    "Bookman-DemiItalic",
    "NewCenturySchlbk-Roman", "NewCenturySchlbk-Italic",
    "NewCenturySchlbk-Bold", "NewCenturySchlbk-BoldItalic",
    "AvantGarde-Book", "AvantGarde-BookOblique", "AvantGarde-Demi",
    "AvantGarde-DemiOblique",
    "Palatino-Roman", "Palatino-Italic", "Palatino-Bold", "Palatino-BoldItalic",
    "ZapfChancery-MediumItalic", "ZapfDingbats"
  };

static
int flags[MAX_FONT] =
  {
         042,     0142, 01000042, 01000142,
         040,     0140, 01000040, 01000140,
         042,     0142, 01000042, 01000142,
          06,
         042,     0142, 01000042, 01000142,
         042,     0142, 01000042, 01000142,
         040,     0140, 01000040, 01000140,
         042,     0142, 01000042, 01000142,
        0142,       04
  };

static
int stems[MAX_FONT] =
  {
      80,  80, 160, 160,
      80,  80, 160, 160,
      80,  80, 160, 160,
      80,
      80,  80, 160, 160,
      80,  80, 160, 160,
      80,  80, 160, 160,
      80,  80, 160, 160,
      80,  80
  };

static
const char *bboxes[MAX_FONT] =
  {
    "-170 -217 1024 896",
    "-176 -252 990 930",
    "-172 -256 1008 965",
    "-168 -232 1014 894",
    "-174 -220 1001 940",
    "-178 -220 1108 940",
    "-173 -221 1003 936",
    "-177 -221 1107 936",
    "-32 -290 640 795",
    "-85 -290 726 795",
    "-92 -350 700 855",
    "-145 -350 784 855",
    "-180 -293 1090 1010",
    "-188 -251 1266 928",
    "-228 -222 1269 893",
    "-194 -243 1346 934",
    "-231 -220 1388 941",
    "-217 -215 1008 980",
    "-166 -227 1018 968",
    "-166 -221 1000 1007",
    "-170 -220 1151 990",
    "-115 -223 1151 989",
    "-115 -223 1275 989",
    "-121 -251 1248 1025",
    "-121 -251 1281 1025",
    "-166 -283 1072 925",
    "-166 -277 1020 946",
    "-156 -266 1000 966",
    "-166 -281 1073 952",
    "-133 -257 1050 811",
    "-1 -143 981 820"
  };

static
double angles[MAX_FONT] =
  {
        0, -15.5,     0,   -15,
        0,   -12,     0,   -12,
        0,   -12,     0,   -12,
        0,
        0,   -10,     0,   -10,
        0,   -16,     0,   -16,
        0, -10.5,     0, -10.5,
        0,   -10,     0,   -10,
      -14,     0
  };

static
double capheights[MAX_FONT] =
  {
    0.662, 0.660, 0.681, 0.662,
    0.729, 0.729, 0.729, 0.729,
    0.583, 0.583, 0.583, 0.583,
    0.667,
    0.681, 0.681, 0.681, 0.681,
    0.722, 0.722, 0.722, 0.722,
    0.739, 0.739, 0.739, 0.739,
    0.694, 0.693, 0.683, 0.683,
    0.587, 0.692
  };

static
int ascenders[MAX_FONT] =
  {
     682,  684,  670,  682,
     729,  729,  729,  729,
     624,  624,  674,  674,
       0,
     717,  717,  725,  732,
     737,  737,  737,  737,
     739,  739,  739,  739,
     723,  733,  719,  726,
     678,    0
  };

static
int map[32] =
  {
    22,  9,  5, 14, 18, 26, 13,  1,
    24, 11,  7, 16, 20, 28, 13,  3,
    23, 10,  6, 15, 19, 27, 13,  2,
    25, 12,  8, 17, 21, 29, 13,  4
  };

static
int rmap[29] =
  {
     8, 24, 16, 32,
     3, 19, 11, 27,
     2, 18, 10, 26,
    23,
     4, 20, 12, 28,
     5, 21, 13, 29,
     1, 17,  9, 25,
     6, 22, 14, 30
  };

static
char bitmap[PATTERNS][17];

static
int predef_font[] = { 1, 1, 1, -2, -3, -4 };

static
int predef_prec[] = { 0, 1, 2, 2, 2, 2 };

static
int predef_ints[] = { 0, 1, 3, 3, 3 };

static
int predef_styli[] = { 1, 1, 1, 2, 3 };

static
double xfac[4] = { 0, 0, -0.5, -1 };

static
double yfac[6] = { 0, -1.2, -1, -0.5, 0, 0.2 };

static
void fill_routine(int n, double *px, double *py, int tnr);

static
void pdf_to_memory(pdf_t pdf, int page, int width, int height, unsigned char *rgb_image);

static char buf_array[NO_OF_BUFS][20];
static int current_buf = 0;

static
const char *pdf_double(double f)
{
  char *buf = buf_array[(current_buf++) % NO_OF_BUFS];

  if (fabs(f) < 0.00001)
    return "0";

  sprintf(buf, "%.4g", f);
  if (strchr(buf, 'e'))
    {
      if (fabs(f) < 1)
        sprintf(buf, "%1.5f", f);
      else if (fabs(f) < 1000)
        sprintf(buf, "%1.2f", f);
      else
        sprintf(buf, "%1.0f", f);
    }

  return buf;
}

static
char *pdf_calloc(size_t count, size_t size)
{
  char *p;

  p = gks_malloc(count * size);
  if (p == NULL)
    exit(-1);

  return p;
}

static
char *pdf_realloc(void *ptr, size_t size)
{
  char *p;

  p = gks_realloc(ptr, size);
  if (p == NULL)
    exit(-1);

  return p;
}

static
void pdf_memcpy(PDF_stream *p, char *s, size_t n)
{
  if (p->length + n >= p->size)
    {
      while (p->length + n >= p->size)
        p->size += MEMORY_INCREMENT;
      p->buffer = (Byte *) pdf_realloc(p->buffer, p->size);
    }

  memmove(p->buffer + p->length, s, n);
  p->length += n;
}

static
void pdf_printf(PDF_stream *p, const char *args,...)
{
  va_list ap;
  char fmt[BUFSIZ], s[BUFSIZ];

  strcpy(fmt, args);

  va_start(ap, args);
  vsprintf(s, fmt, ap);
  va_end(ap);

  pdf_memcpy(p, s, strlen(s));
}

static
PDF_stream *pdf_alloc_stream(void)
{
  PDF_stream *p;

  p = (PDF_stream *) pdf_calloc(1, sizeof(PDF_stream));
  p->buffer = NULL;
  p->size = p->length = 0;

  return p;
}

static
long pdf_alloc_id(PDF *p)
{
  if (p->object_number >= p->max_objects)
    {
      p->max_objects += MAX_OBJECTS;
      p->byte_offset = (long *) pdf_realloc(p->byte_offset,
                                            p->max_objects * sizeof(long));
    }
  return ++(p->object_number);
}

static
void pdf_open(int fd)
{
  p->fd = fd;

  p->stream = pdf_alloc_stream();

  p->object_number = p->current_page = 0;
  p->max_objects = MAX_OBJECTS;
  p->byte_offset = (long *) pdf_calloc(p->max_objects, sizeof(long));

  p->info = pdf_alloc_id(p);
  p->root = pdf_alloc_id(p);
  p->outlines = pdf_alloc_id(p);

  p->pages = pdf_alloc_id(p);
  p->max_pages = MAX_PAGES;
  p->page = (PDF_page **) pdf_calloc(p->max_pages, sizeof(PDF_page *));

  p->images = 0;
  p->max_images = MAX_IMAGES;
  p->image = (PDF_image **) pdf_calloc(p->max_images, sizeof(PDF_image *));
}

static
PDF_image *pdf_image(PDF *p, int width, int height)
{
  PDF_image *image;

  if (p->images + 1 >= MAX_IMAGES)
    {
      p->max_images += MAX_IMAGES;
      p->image = (PDF_image **) pdf_realloc(
        p->image, p->max_images * sizeof(PDF_image *));
    }

  image = (PDF_image *) pdf_calloc(1, sizeof(PDF_image));

  image->object = pdf_alloc_id(p);
  image->width = width;
  image->height = height;
  image->data = (int *) pdf_calloc(width * height, sizeof(int));

  return image;
}

static
void pdf_page(PDF *p, double height, double width)
{
  PDF_page *page;
  int font;

  if (p->current_page + 1 >= MAX_PAGES)
    {
      p->max_pages += MAX_PAGES;
      p->page = (PDF_page **) pdf_realloc(p->page,
                                          p->max_pages * sizeof(PDF_page *));
    }

  page = (PDF_page *) pdf_calloc(1, sizeof(PDF_page));

  page->object = pdf_alloc_id(p);
  page->contents = pdf_alloc_id(p);
  page->width = width;
  page->height = height;
  page->stream = pdf_alloc_stream();

  p->page[p->current_page++] = page;
  p->content = page->stream;

  for (font = 0; font < MAX_FONT; font++)
    page->fonts[font] = 0;

  page->first_image = page->last_image = p->images;
}

static
void pdf_close(PDF *p)
{
  time_t timer;
  struct tm ltime;
  long start_xref;
  int count, object, font, pattern;
  int image, width, height, length, *rgba, alpha;
  Byte red, green, blue, data[3];
  int mask_id, filter_id, i;
  stroke_data_t s;
  movie_t movie;
  pdf_t pdf;
  frame_t *frames;
  char *env = NULL;
  int framerate = 25;
  char path[MAXPATHLEN];

  pdf_printf(p->stream, "%%PDF-1.%d\n", p->compress ? 2 : 0);
  pdf_printf(p->stream, "%%\344\343\317\322\n");  /* %âãÏÓ\n */

  time(&timer);
  ltime = *localtime(&timer);

  pdf_obj(p, p->info);
  pdf_dict(p);
  pdf_printf(p->stream, "/Creator (GKS)\n");
  pdf_printf(p->stream, "/CreationDate (D:%04d%02d%02d%02d%02d%02d)\n",
             ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday,
             ltime.tm_hour, ltime.tm_min, ltime.tm_sec);
  pdf_printf(p->stream, "/Producer (%s)\n", "GKS 5 PDF driver");
  pdf_enddict(p);
  pdf_endobj(p);

  pdf_obj(p, p->root);
  pdf_dict(p);
  pdf_printf(p->stream, "/Type /Catalog\n");
  pdf_printf(p->stream, "/Pages %ld 0 R\n", p->pages);
  pdf_printf(p->stream, "/Outlines %ld 0 R\n", p->outlines);
  pdf_enddict(p);
  pdf_endobj(p);

  pdf_obj(p, p->outlines);
  pdf_dict(p);
  pdf_printf(p->stream, "/Type /Outlines\n");
  pdf_printf(p->stream, "/Count 0\n");
  pdf_enddict(p);
  pdf_endobj(p);

  pdf_obj(p, p->pages);
  pdf_dict(p);
  pdf_printf(p->stream, "/Type /Pages\n");
  pdf_printf(p->stream, "/Count %d\n", p->current_page);
  pdf_printf(p->stream, "/Kids [");

  for (count = 0; count < p->current_page; count++)
    {
      pdf_printf(p->stream, "%ld 0 R", p->page[count]->object);
      if (count < p->current_page - 1)
        pdf_printf(p->stream, count % 6 ? (char *) " " : (char *) "\n");
    }

  pdf_printf(p->stream, "]\n");
  pdf_enddict(p);
  pdf_endobj(p);

  filter_id = pdf_alloc_id(p);
  pdf_obj(p, filter_id);
  pdf_dict(p);
  pdf_printf(p->stream, "/Length 12\n/Filter/ASCIIHexDecode\n");
  pdf_enddict(p);
  pdf_stream(p);
  pdf_printf(p->stream, "000000ffffff\n");
  pdf_endstream(p);
  for (pattern = 0; pattern < PATTERNS; pattern++)
    {
      if (p->have_pattern[pattern])
        {
          pdf_obj(p, p->pattern_id[pattern][0]);
          pdf_dict(p);
          pdf_printf(p->stream, "/Subtype/Image\n");
          pdf_printf(p->stream, "/Width 8\n");
          pdf_printf(p->stream, "/Height 8\n");
          pdf_printf(p->stream, "/BitsPerComponent 1\n");
          pdf_printf(p->stream, "/ColorSpace [/Indexed/DeviceRGB 1 %d 0 R]\n",
                     filter_id);
          pdf_printf(p->stream, "/Filter/ASCIIHexDecode\n");
          pdf_printf(p->stream, "/Length 16\n");
          pdf_enddict(p);
          pdf_stream(p);
          pdf_printf(p->stream, bitmap[pattern]);
          pdf_printf(p->stream, "\n");
          pdf_endstream(p);
          pdf_endobj(p);

          pdf_obj(p, p->pattern_id[pattern][1]);
          pdf_dict(p);
          pdf_printf(p->stream, "/PatternType 1\n");
          pdf_printf(p->stream, "/PaintType 1\n");
          pdf_printf(p->stream, "/TilingType 1\n");
          pdf_printf(p->stream, "/BBox[0 0 8 8]\n");
          pdf_printf(p->stream, "/XStep 8\n");
          pdf_printf(p->stream, "/YStep 8\n");
          pdf_printf(p->stream, "/Resources ");
          pdf_printf(p->stream, "<<");
          pdf_printf(p->stream,
                  "/ProcSet[/PDF/ImageB/ImageC/ImageI/Text]\n/XObject<<\n");
          pdf_printf(p->stream, "/I%03d %d 0 R\n",
                     pattern, p->pattern_id[pattern][0]);
          pdf_printf(p->stream, ">>>>\n");
          pdf_printf(p->stream, "/Length 44\n");
          pdf_enddict(p);
          pdf_stream(p);
          pdf_printf(p->stream,
                     "q\n1 0 0 1 0 8 cm\n8 0 0 -8 0 0 cm\n/I%03d Do\nQ\n\n",
                     pattern);
          pdf_endstream(p);
          pdf_endobj(p);
        }
    }

  for (count = 0; count < p->current_page; count++)
    {
      PDF_page *page = p->page[count];

      pdf_obj(p, page->object);
      pdf_dict(p);
      pdf_printf(p->stream, "/Type /Page\n");
      pdf_printf(p->stream, "/Parent %ld 0 R\n", p->pages);
      pdf_printf(p->stream, "/Resources << /Font <<");
      for (font = 0; font < MAX_FONT; font++)
        {
          if (page->fonts[font])
            pdf_printf(p->stream, " /F%d %ld 0 R", font, page->fonts[font]);
        }
      pdf_printf(p->stream, " >>\n");

      pdf_printf(p->stream, "/ExtGState <<\n");
      for (alpha = 0; alpha < 256; alpha++)
        {
          if (p->have_alpha[alpha])
            pdf_printf(p->stream, "/GS%d << /CA %g /ca %g >>\n",
                       alpha, alpha / 255.0, alpha / 255.0);
        }
      pdf_printf(p->stream, ">>\n");

      pdf_printf(p->stream, "/Pattern <<\n");
      for (pattern = 0; pattern < PATTERNS; pattern++)
        {
          if (p->have_pattern[pattern])
            {
              pdf_printf(p->stream, "/P%d %d 0 R\n",
                         pattern, p->pattern_id[pattern][1]);
            }
        }
      pdf_printf(p->stream, ">>\n");

      pdf_printf(p->stream, "/XObject <<\n");
      for (image = page->first_image; image < page->last_image; image++)
          pdf_printf(p->stream, "/Im%d %d 0 R\n",
                     image + 1, p->image[image]->object);
      pdf_printf(p->stream, ">>\n>>\n");

      pdf_printf(p->stream, "/MediaBox [0 0 %g %g]\n",
                 page->height, page->width);
      pdf_printf(p->stream, "/Contents %ld 0 R\n", page->contents);
      pdf_enddict(p);
      pdf_endobj(p);

      p->content = page->stream;
      pdf_obj(p, page->contents);
      pdf_dict(p);

#ifdef HAVE_ZLIB
      if (p->compress)
        {
          Byte *buffer;
          uLong length;
          int err;

          length = p->content->length + 1024;
          buffer = (Byte *) pdf_calloc((int) length, 1);
          if ((err = compress(buffer, &length, p->content->buffer,
                              p->content->length)) != Z_OK)
            {
              gks_perror("compression failed (err=%d)", err);
              exit(-1);
            }
          free(p->content->buffer);

          p->content->buffer = buffer;
          p->content->size = p->content->length = length;
          pdf_printf(p->stream, "/Length %ld\n", p->content->length);
          pdf_printf(p->stream, "/Filter [/FlateDecode]\n");
          buffer[p->content->length++] = '\n';
        }
      else
        {
          pdf_printf(p->stream, "/Length %ld\n", p->content->length);
        }
#else
      pdf_printf(p->stream, "/Length %ld\n", p->content->length);
#endif

      pdf_enddict(p);
      pdf_stream(p);
      pdf_memcpy(p->stream, (char *) p->content->buffer, p->content->length);
      pdf_endstream(p);
      pdf_endobj(p);

      for (font = 0; font < MAX_FONT; font++)
        {
          if (page->fonts[font])
            {
              pdf_obj(p, page->fonts[font]);
              pdf_dict(p);
              pdf_printf(p->stream, "/Type /Font\n");
              pdf_printf(p->stream, "/Subtype /Type1\n");
              pdf_printf(p->stream, "/Name /F%d\n", font);
              pdf_printf(p->stream, "/BaseFont /%s\n", fonts[font]);
              pdf_printf(p->stream, "/FirstChar 0\n");
              pdf_printf(p->stream, "/LastChar 255\n");
              pdf_printf(p->stream, "/Widths [");
              for (i = 0; i < 256; i++)
                {
                  gks_lookup_afm(rmap[font], i, &s);
                  pdf_printf(p->stream, "%d ", s.right - s.left);
                }
              pdf_printf(p->stream, "]\n");
              pdf_printf(p->stream, "/FontDescriptor %d 0 R\n",
                         page->fonts[font] + 1);
              if (font != 12)
                pdf_printf(p->stream, "/Encoding /WinAnsiEncoding\n");
              pdf_enddict(p);
              pdf_endobj(p);

              pdf_obj(p, page->fonts[font] + 1);
              pdf_dict(p);
              pdf_printf(p->stream, "/Type /FontDescriptor\n");
              pdf_printf(p->stream, "/FontName /%s\n", fonts[font]);
              pdf_printf(p->stream, "/Flags %d\n", flags[font]);
              pdf_printf(p->stream, "/FontBBox [%s]\n", bboxes[font]);
              pdf_printf(p->stream, "/StemV %d\n", stems[font]);
              pdf_printf(p->stream, "/CapHeight %d\n", s.cap);
              pdf_printf(p->stream, "/Ascent %d\n", ascenders[font]);
              pdf_printf(p->stream, "/Descent %d\n", s.bottom);
              pdf_printf(p->stream, "/ItalicAngle %.1f\n", angles[font]);
              pdf_enddict(p);
              pdf_endobj(p);
           }
        }
      free(p->content->buffer);
    }

  for (image = 0; image < p->images; image++)
    {
      width = p->image[image]->width;
      height = p->image[image]->height;
      length = width * height;
      rgba = p->image[image]->data;

      mask_id = pdf_alloc_id(p);
      pdf_obj(p, mask_id);
      pdf_dict(p);
      pdf_printf(p->stream, "/Type /XObject\n");
      pdf_printf(p->stream, "/Subtype /Image\n");
      pdf_printf(p->stream, "/BitsPerComponent 8\n");
      pdf_printf(p->stream, "/ColorSpace /DeviceGray\n");
      pdf_printf(p->stream, "/Height %d\n",height);
      pdf_printf(p->stream, "/Width %d\n", width);
      pdf_printf(p->stream, "/Length %d\n", length);
      pdf_enddict(p);

      pdf_stream(p);
      for (i = 0; i < length; i++)
        {
          alpha = (*rgba & 0xff000000) >> 24;
          rgba++;
          pdf_memcpy(p->stream, (char *) &alpha, 1);
        }
      pdf_printf(p->stream, "\n");
      pdf_endstream(p);
      pdf_endobj(p);

      rgba = p->image[image]->data;

      pdf_obj(p, p->image[image]->object);
      pdf_dict(p);
      pdf_printf(p->stream, "/Type /XObject\n");
      pdf_printf(p->stream, "/Subtype /Image\n");
      pdf_printf(p->stream, "/BitsPerComponent 8\n");
      pdf_printf(p->stream, "/ColorSpace /DeviceRGB\n");
      pdf_printf(p->stream, "/Height %d\n", height);
      pdf_printf(p->stream, "/Width %d\n", width);
      pdf_printf(p->stream, "/SMask %d 0 R\n", mask_id);
      pdf_printf(p->stream, "/Length %d\n", length * 3);
      pdf_enddict(p);

      pdf_stream(p);
      for (i = 0; i < length; i++)
        {
          red = (*rgba & 0xff);
          green = (*rgba & 0xff00) >> 8;
          blue = (*rgba & 0xff0000) >> 16;
          rgba++;
          data[0] = (Byte) red;
          data[1] = (Byte) green;
          data[2] = (Byte) blue;
          pdf_memcpy(p->stream, (char *) data, 3);
        }
      pdf_printf(p->stream, "\n");
      pdf_endstream(p);
      pdf_endobj(p);
    }

  start_xref = p->stream->length;
  pdf_printf(p->stream, "xref\n");
  pdf_printf(p->stream, "0 %ld\n", p->object_number + 1);
  pdf_printf(p->stream, "0000000000 65535 f \n");
  for (object = 1; object <= p->object_number; object++)
    pdf_printf(p->stream, "%010ld 00000 n \n", p->byte_offset[object]);

  pdf_printf(p->stream, "trailer\n");
  pdf_dict(p);
  pdf_printf(p->stream, "/Size %ld\n", p->object_number + 1);
  pdf_printf(p->stream, "/Root %ld 0 R\n", p->root);
  pdf_printf(p->stream, "/Info %ld 0 R\n", p->info);
  pdf_enddict(p);
  pdf_printf(p->stream, "startxref\n");
  pdf_printf(p->stream, "%ld\n", start_xref);

  pdf_printf(p->stream, "%%%%EOF\n");

  env = (char *) getenv("GKS_FPS");
  if (env != NULL)
    framerate = atoi(env);
  if (framerate <= 0)
    framerate = 25;

  if (p->wstype == 120) {
    gks_filepath(path, p->path, "mov", 0, 0);

    movie = vc_movie_create(path, framerate, 4000000);

    pdf = vc_pdf_from_memory(p->stream->buffer, p->stream->length);
    frames = vc_pdf_to_frames(pdf, p->width, p->height);

    for (i = 0; i <= vc_pdf_get_number_of_pages(pdf) - 1; i++) {
      vc_movie_append_frame(movie, frames[i]);
      vc_frame_free(frames[i]);
    }

    vc_pdf_close(pdf);
    vc_movie_finish(movie);
  } else {
    const char *file_name = "gks.gif";
    unsigned char *rgb_image;
    int delay = 100 / framerate;
    int num_frames;
    gif_writer gw;

    gif_open(&gw, file_name);

    pdf = vc_pdf_from_memory(p->stream->buffer, p->stream->length);
    num_frames = vc_pdf_get_number_of_pages(pdf);
    rgb_image = (unsigned char *) malloc(p->width * p->height * 4 * sizeof(unsigned char));
    assert(rgb_image);

    for (i = 1; i <= num_frames; i++) {
      fprintf(stderr, "\rWriting frame %d/%d ...", i, num_frames);
      pdf_to_memory(pdf, i, p->width, p->height, rgb_image);
      gif_write(&gw, rgb_image, p->width, p->height, FORMAT_RGBA, delay);
    }
    free(rgb_image);

    gif_close(&gw);
    fprintf(stderr, "\rFinished writing %s.\n", file_name);

#ifdef MUPDF_API_VERSION_17
    fz_drop_document(pdf->ctx, pdf->doc);
    fz_drop_context(pdf->ctx);
#else
    fz_close_document(pdf->doc);
    fz_free_context(pdf->ctx);
#endif
    pdf->num_pages = -1;
  }

  free(p->stream->buffer);
}

static void pdf_to_memory(pdf_t pdf, int page, int width, int height, unsigned char *rgb_image) {
  double transx, transy, zoom;
  fz_matrix transform, scale_mat, transl_mat;
  fz_rect rect;
  fz_irect bbox;
  fz_pixmap *pix;
  fz_device *dev;
  fz_page *page_o;
  unsigned char *data;

#ifdef MUPDF_API_VERSION_17
  page_o = fz_load_page(pdf->ctx, pdf->doc, page - 1);
#else
  page_o = fz_load_page(pdf->doc, page - 1);
#endif

  transx = 0;
  transy = 0;
  zoom = 1.0;
  fz_scale(&scale_mat, zoom, zoom);
  fz_translate(&transl_mat, transx, transy);
  fz_concat(&transform, &scale_mat, &transl_mat);

  /*
   * Take the page bounds and transform them by the same matrix that
   * we will use to render the page.
   */

#ifdef MUPDF_API_VERSION_17
  fz_bound_page(pdf->ctx, page_o, &rect);
#else
  fz_bound_page(pdf->doc, page_o, &rect);
#endif
  fz_transform_rect(&rect, &transform);
  fz_round_rect(&bbox, &rect);

  /*
   * Create a blank pixmap to hold the result of rendering. The
   * pixmap bounds used here are the same as the transformed page
   * bounds, so it will contain the entire page.
   */

  pix = fz_new_pixmap(pdf->ctx, fz_device_rgb(pdf->ctx), width, height);
  fz_clear_pixmap_with_value(pdf->ctx, pix, 0xff);

  /*
   * Create a draw device with the pixmap as its target.
   * Run the page with the transform.
   */

  dev = fz_new_draw_device(pdf->ctx, pix);
#ifdef MUPDF_API_VERSION_17
  fz_run_page(pdf->ctx, page_o, dev, &transform, NULL);
#else
  fz_run_page(pdf->doc, page_o, dev, &transform, NULL);
#endif

  data = fz_pixmap_samples(pdf->ctx, pix);
  memmove(rgb_image, data, width * height * 4 * sizeof(unsigned char));

#ifdef MUPDF_API_VERSION_17
  fz_drop_device(pdf->ctx, dev);
  fz_drop_pixmap(pdf->ctx, pix);
  fz_drop_page(pdf->ctx, page_o);
#else
  fz_free_device(dev);
  fz_drop_pixmap(pdf->ctx, pix);
  fz_free_page(pdf->doc, page_o);
#endif
}

static
void pdf_text_ex(PDF *p, double xorg, double yorg, char *text)
{
  double rad, c, s;

  rad = p->angle * M_PI / 180;
  c = cos(rad);
  s = sin(rad);

  pdf_printf(p->content,
             "BT\n/F%d %d Tf\n%s %s %s %s %.2f %.2f Tm\n(%s) Tj\nET\n",
             p->font, p->pt, pdf_double(c), pdf_double(s), pdf_double(-s),
             pdf_double(c), xorg, yorg, text);
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
  double a, b, c, d;

  a = (p->viewport[1] - p->viewport[0]) / (p->window[1] - p->window[0]);
  b = 810 / 0.288;
  p->a = a * b;
  p->b = b * (p->viewport[0] - p->window[0] * a);
  c = (p->viewport[3] - p->viewport[2]) / (p->window[3] - p->window[2]);
  d = 558 / 0.1984;
  p->c = c * d;
  p->d = d * (p->viewport[2] - p->window[2] * c);

  p->width  = nint(p->a * (p->window[1] - p->window[0]));
  p->height = nint(p->c * (p->window[3] - p->window[2]));
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
      p->red[color] = red;
      p->green[color] = green;
      p->blue[color] = blue;
      p->color = p->fillcolor = -1;
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
      p->red[color] = red;
      p->green[color] = green;
      p->blue[color] = blue;
    }
  for (color = 0; color < 256; color++)
    p->have_alpha[color] = 0;
}

static
void create_patterns(void)
{
  int i, j, k;
  int pattern, parray[33];

  for (i = 0; i < PATTERNS; i++)
    {
      pattern = i;
      gks_inq_pattern_array(pattern, parray);
      for (j = 0, k = 1; j < 16; j += 2)
        {
          sprintf(bitmap[i] + j, "%02x", parray[k]);
          if (++k > *parray)
            k = 1;
        }
      bitmap[i][16] = '\0';
      p->have_pattern[i] = 0;
    }
}

static
void init_context(void)
{
  p->stroke = 0;
  p->lastx = p->lasty = -1;

  p->color = p->fillcolor = -1;
  p->alpha = 0xff;
  p->ltype = -999; p->lwidth = -1.0;
  p->font = 1; p->size = 24; p->angle = 0;
  p->pt = nint(p->size / capheights[0]);
}

static
void open_ws(int fd, char *path, int wstype)
{
  p = (ws_state_list *) pdf_calloc(1, sizeof(struct ws_state_list_t));

  p->path = path;
  p->wstype = wstype;

  p->compress = wstype != 101;

  p->window[0] = p->window[2] = 0.0;
  p->window[1] = p->window[3] = 1.0;
  p->viewport[0] = p->viewport[2] = 0;
  p->viewport[1] = p->viewport[3] = 0.1984;
  p->width = p->height = 558;

  p->empty = 1;

  init_context();

  set_xform();

  pdf_open(fd);
}

static
void set_clip(double *clrt)
{
  double x0, x1, y0, y1;

  NDC_to_DC(clrt[0], clrt[2], x0, y0);
  NDC_to_DC(clrt[1], clrt[3], x1, y1);

  pdf_moveto(p, x0, y0);
  pdf_lineto(p, x1, y0);
  pdf_lineto(p, x1, y1);
  pdf_lineto(p, x0, y1);
  pdf_closepath(p);
  pdf_clip(p);
}

static
void set_color(int color)
{
  if (color < MAX_COLOR)
    {
      if (p->color != color)
        {
          pdf_setrgbcolor(p, p->red[color], p->green[color], p->blue[color]);
          p->color = color;
        }
    }
}

static
void set_fillcolor(int color)
{
  if (color < MAX_COLOR)
    {
      if (p->fillcolor != color)
        {
          pdf_setfillcolor(p, p->red[color], p->green[color], p->blue[color]);
          p->fillcolor = color;
        }
    }
}

static
void set_transparency(int alpha)
{
  pdf_setalpha(p, alpha);
  p->alpha = alpha;
  p->have_alpha[alpha] = 1;
}

static
void begin_page(void)
{
  init_context();
  pdf_page(p, p->width, p->height);
  set_clip(p->window);
  p->empty = 0;
}

static
void close_ws(void)
{
  pdf_close(p);

  free(p);
}

static
void clear_ws(void)
{
  p->empty = 1;
}

static
void stroke(void)
{
  if (p->stroke)
    {
      pdf_stroke(p);
      p->stroke = 0;
    }
}

static
void move(double x, double y)
{
  double xdev, ydev;

  stroke();

  NDC_to_DC(x, y, xdev, ydev);
  pdf_moveto(p, xdev, ydev);

  p->lastx = xdev;
  p->lasty = ydev;
}

static
void draw(double x, double y)
{
  double xdev, ydev;

  NDC_to_DC(x, y, xdev, ydev);
  if (xdev != p->lastx || ydev != p->lasty)
    {
      pdf_lineto(p, xdev, ydev);
      p->lastx = xdev;
      p->lasty = ydev;
      p->stroke = 1;
    }
}

static
void line_routine(int n, double *px, double *py, int ltype, int tnr)
{
  int i, j, m;
  double x, y, xdev, ydev;

  m = ltype == DrawBorder ? n + 1 : n;

  for (i = 0; i < m; i++)
    {
      j = i < n ? i : 0;

      WC_to_NDC(px[j], py[j], tnr, x, y);
      seg_xform(&x, &y);
      NDC_to_DC(x, y, xdev, ydev);

      if (i == 0)
        pdf_moveto(p, xdev, ydev);
      else
        pdf_lineto(p, xdev, ydev);
    }

  p->stroke = 1;
  stroke();
}

static
void set_linetype(int ltype, double lwidth)
{
  char dash[80];

  if (gkss->version > 4)
    lwidth *= (p->width + p->height) * 0.001;
  if (p->ltype != ltype || p->lwidth != lwidth)
    {
      gks_get_dash(ltype, lwidth, dash);
      pdf_setdash(p, dash);
      p->ltype = ltype;
    }
}

static
void set_linewidth(double lwidth)
{
  if (gkss->version > 4)
    lwidth *= (p->width + p->height) * 0.001;
  if (p->lwidth != lwidth)
    {
      pdf_setlinewidth(p, lwidth);
      p->lwidth = lwidth;
    }
}

static
void polyline(int n, double *px, double *py)
{
  int ln_type, ln_color;
  double ln_width;

  ln_type = gkss->asf[0] ? gkss->ltype : gkss->lindex;
  ln_width = gkss->asf[1] ? gkss->lwidth : 1;
  ln_color = gkss->asf[2] ? gkss->plcoli : 1;

  set_linetype(ln_type, ln_width);
  set_linewidth(ln_width);
  set_transparency(p->alpha);
  set_color(ln_color);

  gks_set_dev_xform(gkss, p->window, p->viewport);
  gks_emul_polyline(n, px, py, ln_type, gkss->cntnr, move, draw);
  stroke();
}

static
void draw_marker(double xn, double yn, int mtype, double mscale, int mcolor)
{
  int r, curve, i;
  double scale, x, y, xr, yr;
  int pc, op;

#include "marker.h"

  static double cx[4][3] = {
    { 0.5523, 1, 1 },
    { 1, 0.5523, 0 },
    { -0.5523, -1, -1 },
    { -1, -0.5523, 0 }
  };

  static double cy[4][3] = {
    { -1, -0.5523, 0 },
    { 0.5523, 1, 1 },
    { 1, 0.5523, 0 },
    { -0.5523, -1, -1 }
  };

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
        pdf_moveto(p, x, y);
        pdf_lineto(p, x, y);
        pdf_stroke(p);
        break;

      case 2: /* line */
        for (i = 0; i < 2; i++)
        {
          xr =  scale * marker[mtype][pc + 2 * i + 1];
          yr = -scale * marker[mtype][pc + 2 * i + 2];
          seg_xform_rel(&xr, &yr);
          if (i == 0)
            pdf_moveto(p, x - xr, y - yr);
          else
            pdf_lineto(p, x - xr, y - yr);
        }
        pdf_stroke(p);
        pc += 4;
        break;

      case 3: /* polyline */
        for (i = 0; i < marker[mtype][pc + 1]; i++)
        {
          xr =  scale * marker[mtype][pc + 2 + 2 * i];
          yr = -scale * marker[mtype][pc + 3 + 2 * i];
          seg_xform_rel(&xr, &yr);
          if (i == 0)
            pdf_moveto(p, x - xr, y - yr);
          else
            pdf_lineto(p, x - xr, y - yr);
        }
        pdf_stroke(p);
        pc += 1 + 2 * marker[mtype][pc + 1];
        break;

      case 4: /* filled polygon */
      case 5: /* hollow polygon */
        if (op == 5)
          set_fillcolor(0);
        for (i = 0; i < marker[mtype][pc + 1]; i++)
        {
          xr =  scale * marker[mtype][pc + 2 + 2 * i];
          yr = -scale * marker[mtype][pc + 3 + 2 * i];
          seg_xform_rel(&xr, &yr);
          if (i == 0)
            pdf_moveto(p, x - xr, y - yr);
          else
            pdf_lineto(p, x - xr, y - yr);
        }
        pdf_eofill(p);
        pc += 1 + 2 * marker[mtype][pc + 1];
        if (op == 5)
          set_fillcolor(mcolor);
        break;

      case 6: /* arc */
        xr =  0;
        yr =  -r;
        seg_xform_rel(&xr, &yr);
        pdf_moveto(p, x - xr, y - yr);
        for (curve = 0; curve < 4; curve++)
        {
          for (i = 0; i < 3; i++)
          {
            xr = r * cx[curve][i];
            yr = r * cy[curve][i];
            seg_xform_rel(&xr, &yr);
            pdf_point(p, x - xr, y - yr);
          }
          pdf_curveto(p);
        }
        pdf_stroke(p);
        break;

      case 7: /* filled arc */
      case 8: /* hollow arc */
        if (op == 8)
          set_fillcolor(0);
        xr =  0;
        yr =  -r;
        seg_xform_rel(&xr, &yr);
        pdf_moveto(p, x - xr, y - yr);
        for (curve = 0; curve < 4; curve++)
        {
          for (i = 0; i < 3; i++)
          {
            xr = r * cx[curve][i];
            yr = r * cy[curve][i];
            seg_xform_rel(&xr, &yr);
            pdf_point(p, x - xr, y - yr);
          }
          pdf_curveto(p);
        }
        pdf_eofill(p);
        if (op == 8)
          set_fillcolor(mcolor);
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

  set_linetype(GKS_K_LINETYPE_SOLID, mk_size / 2);
  set_linewidth(mk_size / 2);
  set_transparency(p->alpha);
  set_color(mk_color);
  set_fillcolor(mk_color);

  marker_routine(n, px, py, mk_type, 23 * mk_size / 24, mk_color);
}

static
void set_font(int font)
{
  double ux, uy, scale;
  double width, height;
  PDF_page *page = p->page[p->current_page - 1];

  font = abs(font);
  if (font >= 101 && font <= 131)
    font -= 100;
  else if (font >= 1 && font <= 32)
    font = map[font - 1];
  else
    font = 9;

  font--;
  if (!page->fonts[font])
    {
      page->fonts[font] = pdf_alloc_id(p);
      pdf_alloc_id(p);
    }
  p->font = font;

  WC_to_NDC_rel(gkss->chup[0], gkss->chup[1], gkss->cntnr, ux, uy);
  seg_xform_rel(&ux, &uy);

  p->angle = -atan2(ux, uy) * 180 / M_PI;
  if (p->angle < 0)
    p->angle += 360;

  scale = sqrt(gkss->chup[0] * gkss->chup[0] + gkss->chup[1] * gkss->chup[1]);
  ux = gkss->chup[0] / scale * gkss->chh;
  uy = gkss->chup[1] / scale * gkss->chh;
  WC_to_NDC_rel(ux, uy, gkss->cntnr, ux, uy);

  width = 0;
  height = sqrt(ux * ux + uy * uy);
  seg_xform_rel(&width, &height);

  height = sqrt(width * width + height * height);
  p->size = (int) (height * fabs(p->c) + 0.5);
  p->pt = nint(p->size / capheights[p->font]);
}

static
void text_routine(double x, double y, int nchars, char *chars)
{
  char s[BUFSIZ], *cp;
  double xrel, yrel, xorg, yorg;
  int tx_font, tx_prec;
  double phi, ax, ay;
  int width, ch;
  stroke_data_t buffer;
  int i;

  NDC_to_DC(x, y, xorg, yorg);

  tx_font = gkss->asf[6] ? gkss->txfont : predef_font[gkss->tindex - 1];
  tx_prec = gkss->asf[6] ? gkss->txprec : predef_prec[gkss->tindex - 1];

  if (tx_prec == GKS_K_TEXT_PRECISION_STRING)
    {
      width = 0;
      for (i = 0; i < nchars; i++)
        {
          ch = chars[i];
          gks_lookup_afm(tx_font, ch, &buffer);
          width += buffer.right - buffer.left;
        }
      width = (int) (width * p->size / buffer.size);

      phi = p->angle * M_PI / 180;
      xrel = width * xfac[gkss->txal[0]];
      yrel = p->size * yfac[gkss->txal[1]];

      CharXform(phi, xrel, yrel, ax, ay);

      xorg += ax;
      yorg += ay;
    }

  cp = s;
  for (i = 0; i < nchars; i++)
    {
      ch = chars[i];
      if (ch == '(' || ch == ')' || ch == '\\')
        *cp++ = '\\';
      *cp++ = ch;
    }
  *cp = '\0';

  if (fabs(p->angle) > FEPS)
    pdf_text_ex(p, xorg, yorg, s);
  else
    pdf_text(p, xorg, yorg, s);
}

static
void text(double px, double py, int nchars, char *chars)
{
  int tx_font, tx_prec, tx_color;
  double x, y;

  tx_font = gkss->asf[6] ? gkss->txfont : predef_font[gkss->tindex - 1];
  tx_prec = gkss->asf[6] ? gkss->txprec : predef_prec[gkss->tindex - 1];
  tx_color = gkss->asf[9] ? gkss->txcoli : 1;

  set_linetype(GKS_K_LINETYPE_SOLID, 1.0);
  set_linewidth(1.0);
  set_transparency(p->alpha);
  set_color(tx_color);
  set_fillcolor(tx_color);

  if (tx_prec != GKS_K_TEXT_PRECISION_STROKE)
    set_font(tx_font);

  if (tx_prec == GKS_K_TEXT_PRECISION_STRING)
    {
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
void fill_routine(int n, double *px, double *py, int tnr)
{
  int i;
  double x, y, xdev, ydev;

  gks_set_dev_xform(gkss, p->window, p->viewport);

  if (p->pattern)
    pdf_printf(p->content, "/Pattern cs/P%d scn\n", p->pattern);

  for (i = 0; i < n; i++)
    {
      WC_to_NDC(px[i], py[i], tnr, x, y);
      seg_xform(&x, &y);
      NDC_to_DC(x, y, xdev, ydev);

      if (i == 0)
        pdf_moveto(p, xdev, ydev);
      else
        pdf_lineto(p, xdev, ydev);
    }

  if (p->pattern)
    {
      pdf_printf(p->content, "f/Pattern cs/P0 scn\n");

      if (!p->have_pattern[p->pattern])
        {
          p->have_pattern[p->pattern] = 1;
          p->pattern_id[p->pattern][0] = pdf_alloc_id(p);
          p->pattern_id[p->pattern][1] = pdf_alloc_id(p);
        }
      if (!p->have_pattern[0])
        {
          p->have_pattern[0] = 1;
          p->pattern_id[0][0] = pdf_alloc_id(p);
          p->pattern_id[0][1] = pdf_alloc_id(p);
        }
    }
  else
    {
      pdf_eofill(p);
    }
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
      set_linetype(GKS_K_LINETYPE_SOLID, 1.0);
      set_linewidth(1.0);
      set_transparency(p->alpha);
      set_color(fl_color);

      line_routine(n, px, py, DrawBorder, gkss->cntnr);
    }
  else if (fl_inter == GKS_K_INTSTYLE_SOLID)
    {
      set_transparency(p->alpha);
      set_fillcolor(fl_color);

      pdf_save(p);
      set_clip(gkss->viewport[gkss->clip == GKS_K_CLIP ? gkss->cntnr : 0]);

      fill_routine(n, px, py, gkss->cntnr);

      pdf_restore(p);
    }
  else if (fl_inter == GKS_K_INTSTYLE_PATTERN ||
           fl_inter == GKS_K_INTSTYLE_HATCH)
    {
      set_transparency(p->alpha);
      set_fillcolor(fl_color);

      if (fl_inter == GKS_K_INTSTYLE_HATCH)
        fl_style += HATCH_STYLE;
      if (fl_style >= PATTERNS)
        fl_style = 1;
      p->pattern = fl_style;

      pdf_save(p);
      set_clip(gkss->viewport[gkss->clip == GKS_K_CLIP ? gkss->cntnr : 0]);

      fill_routine(n, px, py, gkss->cntnr);

      pdf_restore(p);
    }
}

static
void cellarray(double xmin, double xmax, double ymin, double ymax,
               int dx, int dy, int dimx, int *colia, int true_color)
{
  double x1, y1, x2, y2;
  int x, y, width, height;
  double rx1, rx2, ry1, ry2;
  int i, j, ix, iy, color;
  PDF_image *image;
  int swapx, swapy, count, chars_per_line;
  unsigned char data[3];
  int have_alpha;

  WC_to_NDC(xmin, ymax, gkss->cntnr, x1, y1);
  seg_xform(&x1, &y1);
  NDC_to_DC(x1, y1, rx1, ry1);

  WC_to_NDC(xmax, ymin, gkss->cntnr, x2, y2);
  seg_xform(&x2, &y2);
  NDC_to_DC(x2, y2, rx2, ry2);

  width = (int) (fabs(rx2 - rx1));
  height = (int) (fabs(ry2 - ry1));
  if (width == 0 || height == 0) return;
  x = (int) min(rx1, rx2);
  y = (int) min(ry1, ry2);

  swapx = rx1 > rx2;
  swapy = ry1 > ry2;

  pdf_save(p);
  set_clip(gkss->viewport[gkss->clip == GKS_K_CLIP ? gkss->cntnr : 0]);
  pdf_printf(p->content, "%d 0 0 %d %d %d cm\n", width, height, x, y);

  have_alpha = 0;
  if (true_color)
    {
      for (j = 0; j < dy; j++)
        for (i = 0; i < dx; i++)
          if ((colia[j * dimx + i] & 0xff000000) != 0xff000000)
            {
              have_alpha = 1;
              break;
            }
    }

  if (true_color && have_alpha)
    {
      image = pdf_image(p, dx, dy);
      p->image[p->images++] = image;

      for (j = 0; j < dy; j++)
        {
          iy = swapy ? dy - 1 - j : j;
          for (i = 0; i < dx; i++)
            {
              ix = swapx ? dx - 1 - i : i;
              image->data[j * dx + i] = colia[iy * dimx + ix];
            }
        }

      pdf_printf(p->content, "/Im%d Do\n", p->images);
      p->page[p->current_page - 1]->last_image = p->images;
    }
  else
    {
      pdf_printf(p->content, "BI\n");
      pdf_printf(p->content, "/W %d\n", dx);
      pdf_printf(p->content, "/H %d\n", dy);
      pdf_printf(p->content, "/BPC %d\n", 8);
      pdf_printf(p->content, "/CS /RGB\n");
      pdf_printf(p->content, "/F /AHx\n");
      pdf_printf(p->content, "ID ");

      chars_per_line = 0;
      for (j = 0; j < dy; j++)
        {
          iy = swapy ? dy - 1 - j : j;
          for (i = 0; i < dx; i++)
            {
              ix = swapx ? dx - 1 - i : i;
              color = colia[iy * dimx + ix];
              if (!true_color)
                {
                  data[0] = (Byte) (p->red[color]   * 255);
                  data[1] = (Byte) (p->green[color] * 255);
                  data[2] = (Byte) (p->blue[color]  * 255);
                }
              else
                {
                  data[0] = (Byte) ( color & 0xff           );
                  data[1] = (Byte) ((color & 0xff00)   >>  8);
                  data[2] = (Byte) ((color & 0xff0000) >> 16);
                }
              for (count = 0; count < 3; count++)
                {
                  pdf_printf(p->content, "%02x", data[count]);
                  if ((chars_per_line += 2) >= 64)
                    {
                      pdf_printf(p->content, "\n");
                      chars_per_line = 0;
                    }
                }
            }
        }

      pdf_printf(p->content, ">EI\n");
    }

  pdf_restore(p);
}

void gks_movplugin(
  int fctid, int dx, int dy, int dimx, int *ia,
  int lr1, double *r1, int lr2, double *r2, int lc, char *chars, void **ptr)
{
  p = (ws_state_list *) * ptr;

  switch (fctid)
    {
    case 2:
/* open workstation */
      open_ws(ia[1], chars, ia[2]);
      gkss = (gks_state_list_t *) * ptr;

      init_norm_xform();
      init_colors();
      create_patterns();

      gks_init_core(gkss);

      *ptr = p;
      break;

    case 3:
/* close workstation */
      close_ws();
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
      clear_ws();
      break;

    case 8:
/* update workstation */
      break;

    case 12:
/* polyline */
      if (p->state == GKS_K_WS_ACTIVE)
        {
          if (p->empty)
            begin_page();
          polyline(ia[0], r1, r2);
        }
      break;

    case 13:
/* polymarker */
      if (p->state == GKS_K_WS_ACTIVE)
        {
          if (p->empty)
            begin_page();
          polymarker(ia[0], r1, r2);
        }
      break;

    case 14:
/* text */
      if (p->state == GKS_K_WS_ACTIVE)
        {
          int nchars = strlen(chars);
          if (nchars > 0)
            {
              if (p->empty)
                begin_page();
              text(r1[0], r2[0], nchars, chars);
            }
        }
      break;

    case 15:
/* fill area */
      if (p->state == GKS_K_WS_ACTIVE)
        {
          if (p->empty)
            begin_page();
          fillarea(ia[0], r1, r2);
        }
      break;

    case 16:
    case DRAW_IMAGE:
/* cell array */
      if (p->state == GKS_K_WS_ACTIVE)
        {
          int true_color = fctid == DRAW_IMAGE;
          if (p->empty)
            begin_page();
          cellarray(r1[0], r1[1], r2[0], r2[1], dx, dy, dimx, ia, true_color);
        }
      break;

    case 48:
/* set color representation */
      set_color_rep(ia[1], r1[0], r1[1], r1[2]);
      break;

    case 49:
/* set window */
      set_norm_xform(ia[0], gkss->window[ia[0]], gkss->viewport[ia[0]]);
      break;

    case 50:
/* set viewport */
      set_norm_xform(ia[0], gkss->window[ia[0]], gkss->viewport[ia[0]]);
      break;

    case 54:
/* set workstation window */
      p->window[0] = r1[0];
      p->window[1] = r1[1];
      p->window[2] = r2[0];
      p->window[3] = r2[1];

      set_xform();
      init_norm_xform();
      if (!p->empty)
        set_clip(p->window);
      break;

    case 55:
/* set workstation viewport */
      p->viewport[0] = r1[0];
      p->viewport[1] = r1[1];
      p->viewport[2] = r2[0];
      p->viewport[3] = r2[1];

      set_xform();
      init_norm_xform();
      break;

    case 203:
/* set transparency */
      p->alpha = (int) (r1[0] * 255.0);
      break;

    default:
      ;
    }
}

#else

void gks_movplugin(
  int fctid, int dx, int dy, int dimx, int *i_arr,
  int len_f_arr_1, double *f_arr_1, int len_f_arr_2, double *f_arr_2,
  int len_c_arr, char *c_arr, void **ptr)
{
  if (fctid == 2)
  {
    gks_perror("Movie support not compiled in");
    i_arr[0] = 0;
  }
}

#endif
