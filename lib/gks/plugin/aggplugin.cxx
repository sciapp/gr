#include "gks.h"
#include "gkscore.h"

#ifndef GKS_UNUSED
#define GKS_UNUSED(x) (void)(x)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32

#include <windows.h>
#ifndef DLLEXPORT
#define DLLEXPORT __declspec(dllexport)
#endif

#endif

DLLEXPORT void gks_aggplugin(int fctid, int dx, int dy, int dimx, int *i_arr, int len_f_arr_1, double *f_arr_1,
                             int len_f_arr_2, double *f_arr_2, int len_c_arr, char *c_arr, void **ptr);

#ifdef __cplusplus
}
#endif

#ifdef NO_AGG

void gks_aggplugin(int fctid, int dx, int dy, int dimx, int *i_arr, int len_f_arr_1, double *f_arr_1, int len_f_arr_2,
                   double *f_arr_2, int len_c_arr, char *c_arr, void **ptr)
{
  GKS_UNUSED(dx);
  GKS_UNUSED(dy);
  GKS_UNUSED(dimx);
  GKS_UNUSED(i_arr);
  GKS_UNUSED(len_f_arr_1);
  GKS_UNUSED(f_arr_1);
  GKS_UNUSED(len_f_arr_2);
  GKS_UNUSED(f_arr_2);
  GKS_UNUSED(len_c_arr);
  GKS_UNUSED(c_arr);
  GKS_UNUSED(ptr);

  if (fctid == 2)
    {
      gks_perror("Agg support not compiled in");
      i_arr[0] = 0;
    }
}

#else

#include <agg_path_storage.h>
#include <agg_conv_stroke.h>
#include <agg_conv_curve.h>
#include <agg_conv_dash.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_renderer_base.h>
#include <agg_renderer_scanline.h>
#include <agg_scanline_p.h>
#include <agg_span_allocator.h>
#include <agg_image_accessors.h>
#include <agg_span_pattern_rgba.h>

#include <png.h>
#include <jpeglib.h>

#define PATTERNS 120
#define HATCH_STYLE 108
#define MAXPATHLEN 1024

typedef agg::pixfmt_alpha_blend_rgba<agg::blender_rgba<agg::rgba8, agg::order_bgra>, agg::rendering_buffer> pix_fmt_t;
typedef agg::renderer_base<pix_fmt_t> renderer_base_t;
typedef agg::rasterizer_scanline_aa<agg::rasterizer_sl_clip_dbl> rasterizer_t;
typedef agg::scanline_p8 scanline_p8_t;
typedef agg::renderer_scanline_aa_solid<renderer_base_t> renderer_aa_t;
typedef agg::path_storage path_t;
typedef agg::conv_curve<agg::path_storage> conv_curve_t;
typedef agg::conv_stroke<agg::conv_curve<agg::path_storage>> conv_stroke_t;
typedef agg::conv_dash<agg::conv_curve<agg::path_storage>> conv_dash_t;

struct ws_state_list
{
  double mw{}, mh{};
  int w{}, h{}, dpi{};
  int state{}, wtype{}, current_page_written{}, page_counter{};
  double linewidth{}, nominal_size{};
  int width{}, height{};
  int color{};
  double a{}, b{}, c{}, d{};
  double window[4]{}, viewport[4]{};
  double rect[MAX_TNR][2][2]{};

  char *file_path{};
  double rgb[MAX_COLOR + 1][3]{};
  double transparency{};

  int *mem{};
  bool mem_resizable{};
  char mem_format{'a'};

  agg::rendering_buffer render_buffer;
  pix_fmt_t pix_fmt;
  renderer_base_t renderer;
  unsigned char *image_buffer{};
  rasterizer_t rasterizer{1 << 14};
  scanline_p8_t scanline;
  renderer_aa_t renderer_aa;
  path_t path;
  conv_curve_t curve{path};
  conv_stroke_t stroke{curve};
  agg::rgba8 fill_col, stroke_col;
};

static gks_state_list_t *gkss;

static double a[MAX_TNR], b[MAX_TNR], c[MAX_TNR], d[MAX_TNR];
static ws_state_list *p;

static const int predef_prec[] = {0, 1, 2, 2, 2, 2};
static const int predef_ints[] = {0, 1, 3, 3, 3};
static const int predef_styli[] = {1, 1, 1, 2, 3};

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

inline void WC_to_NDC(double xw, double yw, int tnr, double &xn, double &yn)
{
  xn = a[tnr] * xw + b[tnr];
  yn = c[tnr] * yw + d[tnr];
}

inline void WC_to_NDC_rel(double xw, double yw, int tnr, double &xn, double &yn)
{
  xn = a[tnr] * xw;
  yn = c[tnr] * yw;
}

inline void NDC_to_DC(double xn, double yn, double &xd, double &yd)
{
  xd = p->a * xn + p->b;
  yd = p->c * yn + p->d;
}

inline void NDC_to_DC(double xn, double yn, int &xd, int &yd)
{
  double x, y;
  NDC_to_DC(xn, yn, x, y);
  xd = (int)x;
  yd = (int)y;
}

inline void DC_to_NDC(double xd, double yd, double &xn, double &yn)
{
  xn = (xd - p->b) / p->a;
  yn = (yd - p->d) / p->c;
}

static void set_norm_xform(int tnr, const double *wn, const double *vp)
{
  a[tnr] = (vp[1] - vp[0]) / (wn[1] - wn[0]);
  b[tnr] = vp[0] - wn[0] * a[tnr];
  c[tnr] = (vp[3] - vp[2]) / (wn[3] - wn[2]);
  d[tnr] = vp[2] - wn[2] * c[tnr];

  NDC_to_DC(vp[0], vp[3], p->rect[tnr][0][0], p->rect[tnr][0][1]);
  NDC_to_DC(vp[1], vp[2], p->rect[tnr][1][0], p->rect[tnr][1][1]);
}

static void init_norm_xform()
{
  for (int tnr = 0; tnr < MAX_TNR; tnr++)
    {
      set_norm_xform(tnr, gkss->window[tnr], gkss->viewport[tnr]);
    }
}

static void set_xform()
{
  p->a = p->width / (p->window[1] - p->window[0]);
  p->b = -p->window[0] * p->a;
  p->c = p->height / (p->window[2] - p->window[3]);
  p->d = p->height - p->window[2] * p->c;
}

static void seg_xform(double &x, double &y)
{
  double tmp = x * gkss->mat[0][0] + y * gkss->mat[0][1] + gkss->mat[2][0];
  y = x * gkss->mat[1][0] + y * gkss->mat[1][1] + gkss->mat[2][1];
  x = tmp;
}

static void seg_xform_rel(double &x, double &y)
{
  double tmp = x * gkss->mat[0][0] + y * gkss->mat[0][1];
  y = x * gkss->mat[1][0] + y * gkss->mat[1][1];
  x = tmp;
}

static void set_clip_rect(int tnr)
{
  p->renderer.reset_clipping(true);

  if (gkss->clip_tnr != 0 || gkss->clip == GKS_K_CLIP)
    {
      if (gkss->clip_tnr != 0)
        {
          tnr = gkss->clip_tnr;
        }
      p->renderer.clip_box((int)p->rect[tnr][0][0], (int)p->rect[tnr][0][1], (int)p->rect[tnr][1][0],
                           (int)p->rect[tnr][1][1]);
    }
}

static void set_clipping(int index)
{
  gkss->clip = index;
  set_clip_rect(gkss->cntnr);
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

static void select_xform(int tnr)
{
  gkss->cntnr = tnr;
  set_clip_rect(tnr);
}

static void set_transparency(double alpha)
{
  p->transparency = alpha;
}

static void set_viewport(int tnr, double xmin, double xmax, double ymin, double ymax)
{
  gkss->viewport[tnr][0] = xmin;
  gkss->viewport[tnr][1] = xmax;
  gkss->viewport[tnr][2] = ymin;
  gkss->viewport[tnr][3] = ymax;

  set_norm_xform(tnr, gkss->window[tnr], gkss->viewport[tnr]);
  gks_set_norm_xform(tnr, gkss->window[tnr], gkss->viewport[tnr]);
  if (tnr == gkss->cntnr)
    {
      set_clip_rect(tnr);
    }
}

static void set_color_rep(int color, double red, double green, double blue)
{
  if (color >= 0 && color < MAX_COLOR)
    {
      p->rgb[color][0] = red;
      p->rgb[color][1] = green;
      p->rgb[color][2] = blue;
    }
}

static void init_colors()
{
  int color;
  double red, green, blue;

  for (color = 0; color < MAX_COLOR; color++)
    {
      gks_inq_rgb(color, &red, &green, &blue);
      set_color_rep(color, red, green, blue);
    }
}

static void open_page()
{
  set_xform();
  init_norm_xform();

  p->image_buffer = new unsigned char[p->width * p->height * pix_fmt_t::pix_width];
  p->render_buffer = agg::rendering_buffer(p->image_buffer, p->width, p->height, p->width * pix_fmt_t::pix_width);
  p->pix_fmt = pix_fmt_t(p->render_buffer);
  p->renderer = renderer_base_t(p->pix_fmt);
  p->renderer.clear(agg::rgba(0, 0, 0, 0));
  p->renderer_aa = renderer_aa_t(p->renderer);
  p->stroke.line_cap(agg::butt_cap);
  p->stroke.line_join(agg::round_join);
  p->transparency = 1;
}

static void write_page()
{
  char path[MAXPATHLEN];

  p->current_page_written = 1;
  p->page_counter++;

  if (p->wtype == 170)
    {
      /* PPM (p6) output */
      gks_filepath(path, p->file_path, "ppm", p->page_counter, 0);

      FILE *fd = fopen(path, "wb");
      if (fd)
        {
          fprintf(fd, "P6 %d %d 255 ", p->width, p->height);
          for (int i = 0; i < p->width * p->height; i++)
            {
              int alpha = p->image_buffer[4 * i + 3];
              for (int j = 2; j >= 0; j--)
                {
                  fputc(255 - alpha + p->image_buffer[4 * i + j], fd);
                }
            }
          fclose(fd);
        }
    }
  else if (p->wtype == 171)
    {
      /* PNG output */
      png_structp png_ptr;
      png_infop info_ptr;
      png_bytepp row_pointers;

      gks_filepath(path, p->file_path, "png", p->page_counter, 0);
      FILE *fd = fopen(path, "wb");

      png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
      if (!png_ptr)
        {
          fclose(fd);
          gks_perror("Cannot create PNG write struct.");
        }
      info_ptr = png_create_info_struct(png_ptr);
      if (!info_ptr)
        {
          fclose(fd);
          png_destroy_write_struct(&png_ptr, (png_infopp) nullptr);
          gks_perror("Cannot create PNG info struct.");
        }
      png_init_io(png_ptr, fd);
      png_set_IHDR(png_ptr, info_ptr, p->width, p->height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                   PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

      row_pointers = new png_bytep[p->height];
      for (int i = 0; i < p->height; i++)
        {
          row_pointers[i] = p->render_buffer.row_ptr(i);
        }
      png_set_rows(png_ptr, info_ptr, row_pointers);
      png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_BGR, nullptr);

      png_destroy_write_struct(&png_ptr, &info_ptr);
      fclose(fd);
      delete[] row_pointers;
    }
  else if (p->wtype == 172)
    {
      /* JPG output */
      auto *row = new unsigned char[p->width * 3];
      jpeg_compress_struct cinfo = {};
      jpeg_error_mgr jerr = {};

      gks_filepath(path, p->file_path, "jpg", p->page_counter, 0);
      FILE *fd = fopen(path, "wb");

      cinfo.err = jpeg_std_error(&jerr);
      jpeg_create_compress(&cinfo);
      jpeg_stdio_dest(&cinfo, fd);

      cinfo.image_width = p->width;
      cinfo.image_height = p->height;
      cinfo.input_components = 3;
      cinfo.in_color_space = JCS_RGB;
      jpeg_set_defaults(&cinfo);
      jpeg_set_quality(&cinfo, 100, TRUE);

      jpeg_start_compress(&cinfo, TRUE);
      while (cinfo.next_scanline < cinfo.image_height)
        {
          for (int i = 0; i < p->width; i++)
            {
              int alpha = p->render_buffer.row_ptr((int)cinfo.next_scanline)[i * 4 + 3];
              for (int j = 0; j < 3; j++)
                {
                  row[i * 3 + j] = 255 - alpha + p->render_buffer.row_ptr((int)cinfo.next_scanline)[i * 4 + (2 - j)];
                }
            }
          jpeg_write_scanlines(&cinfo, &row, 1);
        }
      jpeg_finish_compress(&cinfo);
      jpeg_destroy_compress(&cinfo);
      fclose(fd);
      delete[] row;
    }
  else if (p->wtype == 173)
    {
      /* Memory output */
      unsigned char *mem;
      if (p->mem_resizable)
        {
          unsigned char **mem_ptr_ptr;
          int *mem_info_ptr = (int *)p->mem;
          mem_ptr_ptr = (unsigned char **)(mem_info_ptr + 3);
          mem_info_ptr[0] = p->width;
          mem_info_ptr[1] = p->height;
          *mem_ptr_ptr = (unsigned char *)gks_realloc(*mem_ptr_ptr, p->width * p->height * pix_fmt_t::pix_width);
          mem = *mem_ptr_ptr;
        }
      else
        {
          mem = reinterpret_cast<unsigned char *>(p->mem);
        }
      if (p->mem_format == 'a')
        {
          for (int j = 0; j < p->height; j++)
            {
              for (int i = 0; i < p->width; i++)
                {
                  /* Reverse alpha pre-multiplication */
                  double alpha = p->image_buffer[j * p->width * 4 + i * 4 + 3];
                  double red = p->image_buffer[j * p->width * 4 + i * 4 + 2] * 255.0 / alpha;
                  double green = p->image_buffer[j * p->width * 4 + i * 4 + 1] * 255.0 / alpha;
                  double blue = p->image_buffer[j * p->width * 4 + i * 4 + 0] * 255.0 / alpha;
                  if (red > 255)
                    {
                      red = 255;
                    }
                  if (green > 255)
                    {
                      green = 255;
                    }
                  if (blue > 255)
                    {
                      blue = 255;
                    }
                  mem[j * p->width * 4 + i * 4 + 0] = (unsigned char)red;
                  mem[j * p->width * 4 + i * 4 + 1] = (unsigned char)green;
                  mem[j * p->width * 4 + i * 4 + 2] = (unsigned char)blue;
                  mem[j * p->width * 4 + i * 4 + 3] = (unsigned char)alpha;
                }
            }
        }
      else if (p->mem_format == 'r')
        {
          memcpy(mem, p->image_buffer, p->width * p->height * pix_fmt_t::pix_width);
        }
      else
        {
          fprintf(stderr, "GKS: Invalid memory format %c\n", p->mem_format);
        }
    }
}

static void close_page()
{
  p->renderer.reset_clipping(true);
  delete[] p->image_buffer;
}

static void fill_path(agg::path_storage &path, bool winding_rule = false)
{
  path.close_polygon();
  p->rasterizer.reset();
  p->rasterizer.add_path(p->curve);
  p->renderer_aa.color(p->fill_col);
  p->rasterizer.filling_rule(winding_rule ? agg::fill_non_zero : agg::fill_even_odd);
  agg::render_scanlines(p->rasterizer, p->scanline, p->renderer_aa);
  p->rasterizer.filling_rule(agg::fill_non_zero);
  p->path.remove_all();
}

static void stroke_path(agg::path_storage &path, bool close = true)
{
  if (close)
    {
      path.close_polygon();
    }
  p->rasterizer.reset();
  p->rasterizer.add_path(p->stroke);
  p->renderer_aa.color(p->stroke_col);
  agg::render_scanlines(p->rasterizer, p->scanline, p->renderer_aa);
  p->path.remove_all();
}

static void fill_stroke_path(agg::path_storage &path, bool winding_rule = false)
{
  path.close_polygon();
  p->rasterizer.reset();
  p->rasterizer.add_path(p->curve);
  p->renderer_aa.color(p->fill_col);
  p->rasterizer.filling_rule(winding_rule ? agg::fill_non_zero : agg::fill_even_odd);
  agg::render_scanlines(p->rasterizer, p->scanline, p->renderer_aa);
  p->rasterizer.filling_rule(agg::fill_non_zero);
  p->rasterizer.reset();
  p->rasterizer.add_path(p->stroke);
  p->renderer_aa.color(p->stroke_col);
  agg::render_scanlines(p->rasterizer, p->scanline, p->renderer_aa);
  p->path.remove_all();
}

static void text_routine(double x, double y, int nchars, char *chars)
{
  int i, j;
  int width = p->width;
  int height = p->height;
  int px, py;
  unsigned char *alpha_pixels;
  unsigned char *bgra_pixels;
  double red, green, blue;

  NDC_to_DC(x, y, px, py);
  py = p->height - py;

  alpha_pixels = gks_ft_get_bitmap(&px, &py, &width, &height, gkss, chars, nchars);
  bgra_pixels = new unsigned char[pix_fmt_t::pix_width * height * width];

  gks_inq_rgb(p->color, &red, &green, &blue);
  for (i = 0; i < height; i++)
    {
      for (j = 0; j < width; j++)
        {
          double alpha = alpha_pixels[i * width + j] / 255.0;
          p->renderer.blend_pixel(px + j, p->height - py - height + i, agg::rgba(red, green, blue, alpha),
                                  agg::cover_full);
        }
    }
  delete[] bgra_pixels;
  gks_free(alpha_pixels);
}

static void line_routine(int n, double *px, double *py, int linetype, int tnr)
{
  double x, y, x0, y0, xi, yi;
  int gks_dashes[10], i;

  WC_to_NDC(px[0], py[0], tnr, x, y);
  seg_xform(x, y);
  NDC_to_DC(x, y, x0, y0);
  p->path.move_to(x0, y0);
  for (i = 1; i < n; i++)
    {
      WC_to_NDC(px[i], py[i], tnr, x, y);
      seg_xform(x, y);
      NDC_to_DC(x, y, xi, yi);
      p->path.line_to(xi, yi);
    }
  p->stroke.width(p->linewidth);
  p->stroke_col = agg::rgba(p->rgb[p->color][0], p->rgb[p->color][1], p->rgb[p->color][2], p->transparency);
  if (linetype != GKS_K_LINETYPE_SOLID)
    {
      conv_dash_t dashes(p->curve);
      gks_get_dash_list(linetype, p->linewidth, gks_dashes);
      for (i = 0; i < gks_dashes[0]; i += 2)
        {
          dashes.add_dash(gks_dashes[i + 1], gks_dashes[i + 2]);
        }
      p->rasterizer.reset();
      agg::conv_stroke<agg::conv_dash<agg::conv_curve<agg::path_storage>>> stroke(dashes);
      stroke.width(p->linewidth);
      p->rasterizer.add_path(stroke);
      p->renderer_aa.color(p->stroke_col);
      agg::render_scanlines(p->rasterizer, p->scanline, p->renderer_aa);
      p->path.remove_all();
    }
  else
    {
      stroke_path(p->path, false);
    }
}

static void fill_routine(int n, double *px, double *py, int tnr)
{
  int i, j, k;
  double x, y, ix, iy;
  int fl_inter, fl_style, fl_color, size;
  int gks_pattern[33];

  WC_to_NDC(px[0], py[0], tnr, x, y);
  seg_xform(x, y);
  NDC_to_DC(x, y, ix, iy);

  p->path.move_to(ix, iy);

  int nan_found = 0;
  for (i = 1; i < n; i++)
    {
      WC_to_NDC(px[i], py[i], tnr, x, y);
      seg_xform(x, y);
      NDC_to_DC(x, y, ix, iy);
      if (px[i] != px[i] || py[i] != py[i])
        {
          nan_found = 1;
          continue;
        }
      if (nan_found)
        {
          nan_found = 0;
          p->path.move_to(ix, iy);
        }
      else
        {
          p->path.line_to(ix, iy);
        }
    }

  fl_inter = gkss->asf[10] ? gkss->ints : predef_ints[gkss->findex - 1];
  fl_color = gkss->asf[12] ? gkss->facoli : 1;

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

      typedef agg::wrap_mode_repeat wrap_x_type;
      typedef agg::wrap_mode_repeat wrap_y_type;
      typedef agg::image_accessor_wrap<agg::pixfmt_rgba32, wrap_x_type, wrap_y_type> img_source_type;
      typedef agg::span_pattern_rgba<img_source_type> span_gen_type;

      auto *m_pattern = new agg::int8u[8 * size * pix_fmt_t::pix_width];
      agg::rendering_buffer m_pattern_rbuf(m_pattern, 8, size, 8 * pix_fmt_t::pix_width);
      agg::pixfmt_rgba32 img_pixf(m_pattern_rbuf);

      for (j = 1; j < size + 1; j++)
        {
          for (i = 0; i < 8; i++)
            {
              k = (1 << i) & gks_pattern[j];
              if (!(k))
                {
                  img_pixf.copy_pixel(i, j - 1,
                                      agg::rgba(p->rgb[fl_color][0] * p->transparency,
                                                p->rgb[fl_color][1] * p->transparency,
                                                p->rgb[fl_color][2] * p->transparency, p->transparency));
                }
              else
                {
                  img_pixf.copy_pixel(i, j - 1, agg::rgba(0, 0, 0, 0));
                }
            }
        }

      agg::span_allocator<agg::rgba8> sa;
      img_source_type img_src(img_pixf);
      span_gen_type sg(img_src, 0, 0);

      p->path.close_polygon();
      p->rasterizer.reset();
      p->rasterizer.add_path(p->path);
      agg::render_scanlines_aa(p->rasterizer, p->scanline, p->renderer, sa, sg);
      p->path.remove_all();
      return;
    }

  if (fl_inter == GKS_K_INTSTYLE_SOLID)
    {
      p->fill_col = agg::rgba(p->rgb[p->color][0], p->rgb[p->color][1], p->rgb[p->color][2], p->transparency);
      fill_path(p->path);
    }
  else
    {
      p->rasterizer.filling_rule(agg::fill_non_zero);
      p->stroke.width(p->linewidth);
      p->stroke.line_cap(agg::round_cap);
      p->stroke.line_join(agg::round_join);
      p->stroke_col = agg::rgba(p->rgb[p->color][0], p->rgb[p->color][1], p->rgb[p->color][2], p->transparency);
      stroke_path(p->path, true);
    }
}

static void text(double px, double py, int nchars, char *chars)
{
  int tx_prec, tx_color;
  double x, y;

  tx_prec = gkss->asf[6] ? gkss->txprec : predef_prec[gkss->tindex - 1];
  tx_color = gkss->asf[9] ? gkss->txcoli : 1;

  p->color = tx_color;

  if (tx_prec == GKS_K_TEXT_PRECISION_STRING)
    {
      WC_to_NDC(px, py, gkss->cntnr, x, y);
      seg_xform(x, y);

      text_routine(x, y, nchars, chars);
    }
  else
    {
      p->linewidth = p->nominal_size;
      gks_emul_text(px, py, nchars, chars, line_routine, fill_routine);
    }
}

static void polyline(int n, double *px, double *py)
{
  int ln_type, ln_color;
  double ln_width;

  ln_type = gkss->asf[0] ? gkss->ltype : gkss->lindex;
  ln_width = gkss->asf[1] ? gkss->lwidth : 1;
  ln_color = gkss->asf[2] ? gkss->plcoli : 1;

  p->linewidth = ln_width * p->nominal_size;
  p->color = ln_color;

  gks_set_dev_xform(gkss, p->window, p->viewport);

  line_routine(n, px, py, ln_type, gkss->cntnr);
}

static void draw_marker(double xn, double yn, int mtype, double mscale, int mcolor)
{
  double x, y;
  double scale, xr, yr, x1, x2, y1, y2;
  int pc, op, i, color;
  double r;

#include "marker.h"

  mscale *= p->nominal_size;
  r = 3 * mscale;
  scale = 0.01 * mscale / 3.0;

  xr = r;
  yr = 0;
  seg_xform_rel(xr, yr);
  r = sqrt(xr * xr + yr * yr);
  NDC_to_DC(xn, yn, x, y);

  pc = 0;
  mtype = (r > 0) ? mtype + marker_off : marker_off + 1;

  p->stroke_col = p->fill_col = agg::rgba(p->rgb[mcolor][0], p->rgb[mcolor][1], p->rgb[mcolor][2], p->transparency);
  p->stroke.width(gkss->bwidth * p->nominal_size);

  do
    {
      op = marker[mtype][pc];
      switch (op)
        {
        case 1: /* point */
          p->path.move_to(x, y);
          p->path.arc_rel(p->nominal_size / 2, p->nominal_size / 2, 0, false, false, 0, -p->nominal_size);
          p->path.arc_rel(p->nominal_size / 2, p->nominal_size / 2, 0, false, false, 0, p->nominal_size);
          fill_path(p->path);
          break;

        case 2: /* line */
          {
            p->stroke.width(max(gkss->bwidth, gkss->lwidth) * p->nominal_size);

            x1 = scale * marker[mtype][pc + 1];
            y1 = scale * marker[mtype][pc + 2];
            seg_xform_rel(x1, y1);

            x2 = scale * marker[mtype][pc + 2 + 1];
            y2 = scale * marker[mtype][pc + 2 + 2];
            seg_xform_rel(x2, y2);

            p->path.move_to(x - x1, y - y1);
            p->path.line_to(x - x2, y - y2);
            stroke_path(p->path, false);
            pc += 4;
          }
          break;

        case 3: /* polyline */
        case 9: /* border polyline */
          if (op == 3 || gkss->bwidth > 0)
            {
              color = op == 3 ? mcolor : gkss->bcoli;
              for (i = 0; i < marker[mtype][pc + 1]; i++)
                {
                  xr = scale * marker[mtype][pc + 2 + 2 * i];
                  yr = -scale * marker[mtype][pc + 3 + 2 * i];
                  seg_xform_rel(xr, yr);

                  if (i == 0)
                    p->path.move_to(x - xr, y + yr);
                  else
                    p->path.line_to(x - xr, y + yr);
                }

              p->stroke_col = agg::rgba(p->rgb[color][0], p->rgb[color][1], p->rgb[color][2], p->transparency);
              stroke_path(p->path);
            }
          pc += 1 + 2 * marker[mtype][pc + 1];
          break;

        case 4: /* filled polygon */
        case 5: /* hollow polygon */
          {
            color = op == 4 ? mcolor : 0;
            p->fill_col = agg::rgba(p->rgb[color][0], p->rgb[color][1], p->rgb[color][2], p->transparency);
            for (i = 0; i < marker[mtype][pc + 1]; i++)
              {
                xr = scale * marker[mtype][pc + 2 + 2 * i];
                yr = -scale * marker[mtype][pc + 3 + 2 * i];
                seg_xform_rel(xr, yr);

                if (i == 0)
                  p->path.move_to(x - xr, y + yr);
                else
                  p->path.line_to(x - xr, y + yr);
              }

            if (op == 4 && gkss->bcoli != gkss->pmcoli && gkss->bwidth > 0)
              {
                p->stroke_col =
                    agg::rgba(p->rgb[gkss->bcoli][0], p->rgb[gkss->bcoli][1], p->rgb[gkss->bcoli][2], p->transparency);
                fill_stroke_path(p->path);
              }
            else
              {
                fill_path(p->path);
              }
            pc += 1 + 2 * marker[mtype][pc + 1];
          }
          break;

        case 6: /* arc */
          p->path.move_to(x, y + r);
          p->path.arc_rel(r, r, 0, false, false, 0, -2 * r);
          p->path.arc_rel(r, r, 0, false, false, 0, 2 * r);

          p->stroke.width(max(gkss->bwidth, gkss->lwidth) * p->nominal_size);
          stroke_path(p->path, true);
          break;

        case 7: /* filled arc */
        case 8: /* hollow arc */
          color = op == 7 ? mcolor : 0;
          p->fill_col = agg::rgba(p->rgb[color][0], p->rgb[color][1], p->rgb[color][2], p->transparency);

          p->path.move_to(x, y + r);
          p->path.arc_rel(r, r, 0, false, false, 0, -2 * r);
          p->path.arc_rel(r, r, 0, false, false, 0, 2 * r);

          if (op == 7 && gkss->bcoli != gkss->pmcoli && gkss->bwidth > 0)
            {
              p->stroke_col =
                  agg::rgba(p->rgb[gkss->bcoli][0], p->rgb[gkss->bcoli][1], p->rgb[gkss->bcoli][2], p->transparency);
              fill_stroke_path(p->path);
            }
          else
            {
              fill_path(p->path);
            }
          break;

        default:
          break;
        }
      pc++;
    }
  while (op != 0);
}

static void polymarker(int n, const double *px, const double *py)
{
  int mk_type, mk_color, i;
  double mk_size, x, y;

  mk_type = gkss->asf[3] ? gkss->mtype : gkss->mindex;
  mk_size = gkss->asf[4] ? gkss->mszsc : 1;
  mk_color = gkss->asf[5] ? gkss->pmcoli : 1;

  for (i = 0; i < n; i++)
    {
      WC_to_NDC(px[i], py[i], gkss->cntnr, x, y);
      seg_xform(x, y);
      draw_marker(x, y, mk_type, mk_size, mk_color);
    }
}

static void fillarea(int n, double *px, double *py)
{
  p->linewidth = gkss->bwidth * p->nominal_size;
  p->color = gkss->asf[12] ? gkss->facoli : 1;

  p->rasterizer.filling_rule(agg::fill_even_odd);
  fill_routine(n, px, py, gkss->cntnr);
  p->rasterizer.filling_rule(agg::fill_non_zero);
}

static void cellarray(double xmin, double xmax, double ymin, double ymax, int dx, int dy, int dimx, int *colia,
                      int true_color)
{
  double x1, y1, x2, y2;
  int x, y;
  int ix1, ix2, iy1, iy2;
  int width, height;
  int i, j, ix, iy, ind;
  int swapx, swapy;
  unsigned char *data;

  WC_to_NDC(xmin, ymax, gkss->cntnr, x1, y1);
  seg_xform(x1, y1);
  NDC_to_DC(x1, y1, ix1, iy1);

  WC_to_NDC(xmax, ymin, gkss->cntnr, x2, y2);
  seg_xform(x2, y2);
  NDC_to_DC(x2, y2, ix2, iy2);

  width = abs(ix2 - ix1);
  height = abs(iy2 - iy1);
  if (width == 0 || height == 0) return;
  x = min(ix1, ix2);
  y = min(iy1, iy2);

  swapx = ix1 > ix2;
  swapy = iy1 < iy2;

  if (true_color)
    {
      data = new unsigned char[width * height * pix_fmt_t::pix_width];
      gks_resample((unsigned char *)colia, data, (size_t)dx, (size_t)dy, (size_t)width, (size_t)height, (size_t)dimx,
                   swapx, swapy, gkss->resample_method);
      for (i = width - 1; i >= 0; i--)
        {
          for (j = height - 1; j >= 0; j--)
            {
              int red, green, blue, alpha;
              red = data[(j * width + i) * pix_fmt_t::pix_width + 0];
              green = data[(j * width + i) * pix_fmt_t::pix_width + 1];
              blue = data[(j * width + i) * pix_fmt_t::pix_width + 2];
              alpha = (int)(data[(j * width + i) * pix_fmt_t::pix_width + 3] * p->transparency);

              agg::rgba8 col = agg::rgba8(red, green, blue, alpha);
              p->renderer.blend_pixel(x + i, y + j, col, agg::cover_full);
            }
        }
      delete[] data;
    }
  else
    {
      for (j = 0; j < height; j++)
        {
          iy = dy * j / height;
          if (swapy)
            {
              iy = dy - 1 - iy;
            }
          for (i = 0; i < width; i++)
            {
              double red, green, blue, alpha;
              ix = dx * i / width;
              if (swapx)
                {
                  ix = dx - 1 - ix;
                }
              ind = colia[iy * dimx + ix];
              ind = FIX_COLORIND(ind);
              alpha = p->transparency;
              red = alpha * p->rgb[ind][0];
              green = alpha * p->rgb[ind][1];
              blue = alpha * p->rgb[ind][2];

              agg::rgba col = agg::rgba(red, green, blue, alpha);
              p->renderer.blend_pixel(x + i, y + j, col, agg::cover_full);
            }
        }
    }
}

static void to_DC(int n, double *x, double *y)
{
  int i;
  double xn, yn;

  for (i = 0; i < n; i++)
    {
      WC_to_NDC(x[i], y[i], gkss->cntnr, xn, yn);
      seg_xform(xn, yn);
      NDC_to_DC(xn, yn, x[i], y[i]);
    }
}

static void draw_path(int n, double *px, double *py, int nc, int *codes)
{
  int i, j;
  double x[3], y[3], w, h, a1, a2;
  double cur_x = 0, cur_y = 0, start_x = 0, start_y = 0;
  int large_arc_flag, sweep_flag;
  GKS_UNUSED(n);

  p->stroke.width(gkss->bwidth * p->nominal_size);

  p->fill_col = agg::rgba(p->rgb[gkss->facoli][0], p->rgb[gkss->facoli][1], p->rgb[gkss->facoli][2], p->transparency);
  p->stroke_col = agg::rgba(p->rgb[gkss->bcoli][0], p->rgb[gkss->bcoli][1], p->rgb[gkss->bcoli][2], p->transparency);

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
          cur_x = x[0];
          cur_y = y[0];
          to_DC(1, x, y);
          start_x = x[0];
          start_y = y[0];
          p->path.move_to(x[0], y[0]);
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
          p->path.line_to(x[0], y[0]);
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
          cur_x = x[1];
          cur_y = y[1];
          to_DC(2, x, y);
          p->path.curve3(x[0], y[0], x[1], y[1]);
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
          p->path.curve4(x[0], y[0], x[1], y[1], x[2], y[2]);
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
            x[0] = cx - rx;
            y[0] = cy - ry;
            x[1] = cx + rx;
            y[1] = cy + ry;
            cur_x = cx + rx * cos(a2);
            cur_y = cy + ry * sin(a2);
            x[2] = cur_x;
            y[2] = cur_y;
          }
          to_DC(3, x, y);

          w = (x[1] - x[0]) * 0.5;
          h = (y[1] - y[0]) * 0.5;

          sweep_flag = a1 > a2;
          while (fabs(a2 - a1) >= 2 * agg::pi)
            {
              double mx, my;
              if (sweep_flag)
                {
                  a1 -= agg::pi;
                }
              else
                {
                  a1 += agg::pi;
                }
              mx = x[0] + w + cos(a1) * w;
              my = y[0] + h + sin(a1) * h;
              p->path.arc_to(w, h, 0, true, sweep_flag, mx, my);
            }
          large_arc_flag = fabs(a2 - a1) >= agg::pi;
          p->path.arc_to(w, h, 0, large_arc_flag, sweep_flag, x[2], y[2]);
          j += 3;
          break;
        case 'S': /* stroke */
          {
            stroke_path(p->path, false);
          }
          break;
        case 's': /* close and stroke */
          {
            stroke_path(p->path, true);
          }
          break;
        case 'f': /* fill (even-odd) */
        case 'g': /* fill (winding) */
          {
            fill_path(p->path, codes[i] == 'g');
          }
          break;
        case 'F': /* fill (even-odd) and stroke */
        case 'G': /* fill (winding) and stroke */
          {
            fill_stroke_path(p->path, codes[i] == 'G');
          }
          break;
        case 'Z': /* close */
          p->path.line_to(start_x, start_y);
          p->path.close_polygon();
        case '\0':
          break;
        default:
          gks_perror("invalid path code ('%c')", codes[i]);
          exit(1);
        }
    }
}

static void draw_lines(int n, const double *px, const double *py, const int *attributes)
{
  int i, j = 0, rgba, line_color = MAX_COLOR;
  double x, y, xim1, yim1, xi, yi;

  WC_to_NDC(px[0], py[0], gkss->cntnr, x, y);
  seg_xform(x, y);
  NDC_to_DC(x, y, xi, yi);

  for (i = 1; i < n; i++)
    {
      xim1 = xi;
      yim1 = yi;
      WC_to_NDC(px[i], py[i], gkss->cntnr, x, y);
      seg_xform(x, y);
      NDC_to_DC(x, y, xi, yi);

      p->stroke.line_cap(agg::round_cap);
      p->stroke.width(0.001 * attributes[j++] * p->nominal_size);
      rgba = attributes[j++];
      p->rgb[line_color][0] = (rgba & 0xff) / 255.0;
      p->rgb[line_color][1] = ((rgba >> 8) & 0xff) / 255.0;
      p->rgb[line_color][2] = ((rgba >> 16) & 0xff) / 255.0;

      p->stroke_col = agg::rgba(p->rgb[line_color][0], p->rgb[line_color][1], p->rgb[line_color][2], p->transparency);

      p->path.move_to(xim1, yim1);
      p->path.line_to(xi, yi);
      stroke_path(p->path, false);
      p->stroke.line_cap(agg::butt_cap);
    }
}

static void draw_markers(int n, const double *px, const double *py, const int *attributes)
{
  int i, j = 0, rgba;
  int mk_type, mk_color = MAX_COLOR;
  double mk_size, x, y;

  mk_type = gkss->asf[3] ? gkss->mtype : gkss->mindex;

  for (i = 0; i < n; i++)
    {
      WC_to_NDC(px[i], py[i], gkss->cntnr, x, y);
      seg_xform(x, y);

      mk_size = 0.001 * attributes[j++];
      rgba = attributes[j++];
      p->rgb[mk_color][0] = (rgba & 0xff) / 255.0;
      p->rgb[mk_color][1] = ((rgba >> 8) & 0xff) / 255.0;
      p->rgb[mk_color][2] = ((rgba >> 16) & 0xff) / 255.0;
      draw_marker(x, y, mk_type, mk_size, mk_color);
    }
}

static void draw_triangles(int n, const double *px, const double *py, int ntri, const int *tri)
{
  double x, y;
  int i, j, k, rgba, line_color = MAX_COLOR;
  agg::vertex_d triangle[3];
  auto points = new agg::vertex_d[n];

  for (i = 0; i < n; ++i)
    {
      WC_to_NDC(px[i], py[i], gkss->cntnr, x, y);
      seg_xform(x, y);
      NDC_to_DC(x, y, points[i].x, points[i].y);
    }

  j = 0;
  p->stroke.line_cap(agg::round_cap);
  p->stroke.line_join(agg::round_join);
  p->stroke.width(gkss->lwidth * p->nominal_size);

  for (i = 0; i < ntri / 4; ++i)
    {
      for (k = 0; k < 3; ++k)
        {
          triangle[k].x = points[tri[j] - 1].x;
          triangle[k].y = points[tri[j] - 1].y;
          j++;
        }

      rgba = tri[j++];
      p->rgb[line_color][0] = (rgba & 0xff) / 255.0;
      p->rgb[line_color][1] = ((rgba >> 8) & 0xff) / 255.0;
      p->rgb[line_color][2] = ((rgba >> 16) & 0xff) / 255.0;

      p->stroke_col = agg::rgba(p->rgb[line_color][0], p->rgb[line_color][1], p->rgb[line_color][2], p->transparency);

      p->path.move_to(triangle[0].x, triangle[0].y);
      for (k = 1; k < 3; k++)
        {
          p->path.line_to(triangle[k].x, triangle[k].y);
        }
      stroke_path(p->path, true);
    }

  p->stroke.line_cap(agg::butt_cap);
  delete[] points;
}

static void fill_polygons(int n, const double *px, const double *py, int nply, const int *ply)
{
  double x, y;
  int i, j, k, len, fill_color = MAX_COLOR;
  unsigned int rgba;
  auto points = new agg::vertex_d[n];

  for (i = 0; i < n; ++i)
    {
      WC_to_NDC(px[i], py[i], gkss->cntnr, x, y);
      seg_xform(x, y);
      NDC_to_DC(x, y, points[i].x, points[i].y);
    }

  j = 0;
  while (j < nply)
    {
      len = ply[j++];
      p->path.move_to(points[ply[j] - 1].x, points[ply[j] - 1].y);
      j++;
      for (k = 1; k < len; k++)
        {
          p->path.line_to(points[ply[j] - 1].x, points[ply[j] - 1].y);
          j++;
        }

      rgba = (unsigned int)ply[j++];
      p->rgb[fill_color][0] = (rgba & 0xff) / 255.0;
      p->rgb[fill_color][1] = ((rgba >> 8) & 0xff) / 255.0;
      p->rgb[fill_color][2] = ((rgba >> 16) & 0xff) / 255.0;
      p->transparency = ((rgba >> 24) & 0xff) / 255.0;

      p->fill_col = agg::rgba(p->rgb[fill_color][0], p->rgb[fill_color][1], p->rgb[fill_color][2], p->transparency);
      p->stroke_col =
          agg::rgba(p->rgb[gkss->bcoli][0], p->rgb[gkss->bcoli][1], p->rgb[gkss->bcoli][2], p->transparency);

      p->stroke.line_join(agg::round_join);
      p->stroke.width(gkss->bwidth * p->nominal_size);
      fill_stroke_path(p->path);
    }
  delete[] points;
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

void gks_aggplugin(int fctid, int dx, int dy, int dimx, int *i_arr, int len_f_arr_1, double *f_arr_1, int len_f_arr_2,
                   double *f_arr_2, int len_c_arr, char *c_arr, void **ptr)
{
  GKS_UNUSED(len_c_arr);
  GKS_UNUSED(len_f_arr_1);
  GKS_UNUSED(len_f_arr_2);

  p = (ws_state_list *)*ptr;

  switch (fctid)
    {
    case 2:
      gkss = (gks_state_list_t *)*ptr;
      gks_init_core(gkss);

      p = new ws_state_list;
      p->wtype = i_arr[2];
      p->file_path = c_arr;
      p->page_counter = 0;

      if (p->wtype == 170 || p->wtype == 171 || p->wtype == 172)
        {
          p->mw = 0.28575;
          p->mh = 0.19685;
          p->w = 6750;
          p->h = 4650;
          p->dpi = 600;
          p->width = 2400;
          p->height = 2400;
        }
      else if (p->wtype == 173)
        {
          int width = 0;
          int height = 0;
          int symbols_read;
          int characters_read = 0;
          void *mem_ptr = nullptr;

          if (!c_arr)
            {
              fprintf(stderr, "Missing mem path. Expected !<width>x<height>@<pointer>.mem\n");
              exit(1);
            }
          symbols_read = sscanf(c_arr, "!resizable@%p.mem%n", &mem_ptr, &characters_read);
          if (symbols_read == 1 && (c_arr[characters_read] == 0 || c_arr[characters_read] == ':') && mem_ptr != nullptr)
            {
              p->mem_resizable = true;
              width = ((int *)mem_ptr)[0];
              height = ((int *)mem_ptr)[1];
              p->dpi = ((int *)mem_ptr)[2];
              if (width <= 0 || height <= 0 || p->dpi <= 0)
                {
                  width = 2400;
                  height = 2400;
                  p->dpi = 600;
                }
            }
          else
            {
              p->mem_resizable = false;
              symbols_read = sscanf(c_arr, "!%dx%d@%p.mem%n", &width, &height, &mem_ptr, &characters_read);
              if (symbols_read != 3 || (c_arr[characters_read] != 0 && c_arr[characters_read] != ':') || width <= 0 ||
                  height <= 0 || mem_ptr == nullptr)
                {
                  fprintf(stderr, "Failed to parse mem path. Expected !<width>x<height>@<pointer>.mem, but found %s\n",
                          c_arr);
                  exit(1);
                }
            }
          if (c_arr[characters_read] == ':')
            {
              if (c_arr[characters_read + 1] != 0 && c_arr[characters_read + 2] == 0)
                {
                  p->mem_format = c_arr[characters_read + 1];
                }
              else
                {
                  fprintf(stderr, "Failed to parse mem format.\n");
                }
            }

          p->width = width;
          p->height = height;
          p->mem = (int *)mem_ptr;

          p->w = 6750;
          p->h = 4650;
          p->mw = p->w * 2.54 / 100 / p->dpi;
          p->mh = p->h * 2.54 / 100 / p->dpi;
        }

      p->window[0] = p->window[2] = 0.0;
      p->window[1] = p->window[3] = 1.0;
      p->viewport[0] = p->viewport[2] = 0;
      p->viewport[1] = (double)p->width * p->mw / p->w;
      p->viewport[3] = (double)p->height * p->mh / p->h;
      if (gkss->nominal_size > 0)
        {
          p->nominal_size = p->dpi / 100 * gkss->nominal_size;
        }
      else
        {
          p->nominal_size = min(p->width, p->height) / 500.0;
        }

      init_colors();
      open_page();

      *ptr = p;
      break;

    case 3:
      /* close workstation */
      if (!p->current_page_written)
        {
          write_page();
        }

      close_page();
      delete p;
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
      p->renderer.reset_clipping(true);
      p->renderer.clear(agg::rgba(0, 0, 0, 0));
      break;

    case 8:
      /* update workstation */
      if (i_arr[1] & GKS_K_WRITE_PAGE_FLAG)
        {
          write_page();
        }
      break;

    case 12:
      /* polyline */
      if (p->state == GKS_K_WS_ACTIVE)
        {
          polyline(i_arr[0], f_arr_1, f_arr_2);
          p->current_page_written = 0;
        }
      break;

    case 13:
      /* polymarker */
      if (p->state == GKS_K_WS_ACTIVE)
        {
          polymarker(i_arr[0], f_arr_1, f_arr_2);
          p->current_page_written = 0;
        }
      break;

    case 14:
      /* text */
      if (p->state == GKS_K_WS_ACTIVE)
        {
          text(f_arr_1[0], f_arr_2[0], (int)strlen(c_arr), c_arr);
          p->current_page_written = 0;
        }
      break;

    case 15:
      /* fill area */
      if (p->state == GKS_K_WS_ACTIVE)
        {
          fillarea(i_arr[0], f_arr_1, f_arr_2);
          p->current_page_written = 0;
        }
      break;

    case 16:
    case DRAW_IMAGE:
      /* cell array */
      if (p->state == GKS_K_WS_ACTIVE)
        {
          int true_color = fctid == DRAW_IMAGE;
          cellarray(f_arr_1[0], f_arr_1[1], f_arr_2[0], f_arr_2[1], dx, dy, dimx, i_arr, true_color);
          p->current_page_written = 0;
        }
      break;

    case 17:
      if (p->state == GKS_K_WS_ACTIVE)
        {
          gdp(i_arr[0], f_arr_1, f_arr_2, i_arr[1], i_arr[2], i_arr + 3);
          p->current_page_written = 0;
        }
      break;

    case 48:
      /* set color representation */
      if (p->state == GKS_K_WS_ACTIVE)
        {
          set_color_rep(i_arr[1], f_arr_1[0], f_arr_1[1], f_arr_1[2]);
        }
      break;

    case 49:
      /* set window */
      set_window(i_arr[0], f_arr_1[0], f_arr_1[1], f_arr_2[0], f_arr_2[1]);
      break;

    case 50:
      /* set viewport */
      set_viewport(i_arr[0], f_arr_1[0], f_arr_1[1], f_arr_2[0], f_arr_2[1]);
      break;

    case 52:
      /* select normalization transformation */
      select_xform(i_arr[0]);
      break;

    case 53:
      /* set clipping inidicator */
      set_clipping(i_arr[0]);
      break;

    case 54:
      /* set workstation window */
      p->window[0] = f_arr_1[0];
      p->window[1] = f_arr_1[1];
      p->window[2] = f_arr_2[0];
      p->window[3] = f_arr_2[1];

      set_xform();
      init_norm_xform();
      break;

    case 55:
      /* set workstation viewport */
      if (p->viewport[0] != 0 || p->viewport[1] != f_arr_1[1] - f_arr_1[0] || p->viewport[2] != 0 ||
          p->viewport[3] != f_arr_2[1] - f_arr_2[0])
        {
          p->viewport[0] = 0;
          p->viewport[1] = f_arr_1[1] - f_arr_1[0];
          p->viewport[2] = 0;
          p->viewport[3] = f_arr_2[1] - f_arr_2[0];

          p->width = (int)(p->viewport[1] * p->w / p->mw);
          p->height = (int)(p->viewport[3] * p->h / p->mh);
          if (gkss->nominal_size > 0)
            {
              p->nominal_size = p->dpi / 100 * gkss->nominal_size;
            }
          else
            {
              p->nominal_size = min(p->width, p->height) / 500.0;
            }

          close_page();
          open_page();
        }
      break;

    case 203:
      /* set transparency */
      set_transparency(f_arr_1[0]);
      break;

    default:
      break;
    }
}

#endif
