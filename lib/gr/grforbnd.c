
#include <stdlib.h>
#include <string.h>

#include "gr.h"

#if defined (_WIN32) && !defined (__GNUC__)
#define STDCALL __stdcall
#else
#define STDCALL
#endif

#define FORTRAN(a) STDCALL a##_

void FORTRAN(gr_opengks)(void)
{
  gr_opengks();
}

void FORTRAN(gr_closegks)(void)
{
  gr_closegks();
}

void FORTRAN(gr_inqdspsize)(
  double *mwidth, double *mheight, int *width, int *height)
{
  gr_inqdspsize(mwidth, mheight, width, height);
}

void FORTRAN(gr_openws)(
  int *workstation_id, char *connection, int *type,
  unsigned short connection_len)
{
  char *_connection;

  _connection = (char *) calloc(1, sizeof(char) * connection_len);
  strncpy(_connection, connection, connection_len);

  gr_openws(*workstation_id, _connection, *type);

  free(_connection);
}

void FORTRAN(gr_closews)(int *workstation_id)
{
  gr_closews(*workstation_id);
}

void FORTRAN(gr_activatews)(int *workstation_id)
{
  gr_activatews(*workstation_id);
}

void FORTRAN(gr_deactivatews)(int *workstation_id)
{
  gr_deactivatews(*workstation_id);
}

void FORTRAN(gr_clearws)(void)
{
  gr_clearws();
}

void FORTRAN(gr_updatews)(void)
{
  gr_updatews();
}

void FORTRAN(gr_polyline)(int *n, double *x, double *y)
{
  gr_polyline(*n, x, y);
}

void FORTRAN(gr_polymarker)(int *n, double *x, double *y)
{
  gr_polymarker(*n, x, y);
}

void FORTRAN(gr_text)(
  double *x, double *y, char *string, unsigned char string_len)
{
  char *_string;

  _string = (char *) calloc(1, sizeof(char) * string_len);
  strncpy(_string, string, string_len);

  gr_text(*x, *y, _string);

  free(_string);
}

void FORTRAN(gr_inqtext)(
  double *x, double *y, char *string, double *tbx, double *tby,
  unsigned short string_len)
{
  char *_string;

  _string = (char *) calloc(1, sizeof(char) * string_len);
  strncpy(_string, string, string_len);

  gr_inqtext(*x, *y, _string, tbx, tby);

  free(_string);
}

void FORTRAN(gr_fillarea)(int *n, double *x, double *y)
{
  gr_fillarea(*n, x, y);
}

void FORTRAN(gr_cellarray)(
  double *xmin, double *xmax, double *ymin, double *ymax,
  int *dimx, int *dimy, int *scol, int *srow, int *ncol, int *nrow, int *color)
{
  gr_cellarray(*xmin, *xmax, *ymin, *ymax,
               *dimx, *dimy, *scol, *srow, *ncol, *nrow, color);
}

void FORTRAN(gr_spline)(int *n, double *x, double *y, int *m, int *method)
{
  gr_spline(*n, x, y, *m, *method);
}

void FORTRAN(gr_gridit)(
  int *nd, double *xd, double *yd, double *zd,
  int *nx, int *ny, double *x, double *y, double *z)
{
  gr_gridit(*nd, xd, yd, zd, *nx, *ny, x, y, z);
}

void FORTRAN(gr_setlinetype)(int *type)
{
  gr_setlinetype(*type);
}

void FORTRAN(gr_setlinewidth)(double *width)
{
  gr_setlinewidth(*width);
}

void FORTRAN(gr_setlinecolorind)(int *color)
{
  gr_setlinecolorind(*color);
}

void FORTRAN(gr_setmarkertype)(int *type)
{
  gr_setmarkertype(*type);
}

void FORTRAN(gr_setmarkersize)(double *size)
{
  gr_setmarkersize(*size);
}

void FORTRAN(gr_setmarkercolorind)(int *color)
{
  gr_setmarkercolorind(*color);
}

void FORTRAN(gr_settextfontprec)(int *font, int *precision)
{
  gr_settextfontprec(*font, *precision);
}

void FORTRAN(gr_setcharexpan)(double *factor)
{
  gr_setcharexpan(*factor);
}

void FORTRAN(gr_setcharspace)(double *spacing)
{
  gr_setcharspace(*spacing);
}

void FORTRAN(gr_settextcolorind)(int *color)
{
  gr_settextcolorind(*color);
}

void FORTRAN(gr_setcharheight)(double *height)
{
  gr_setcharheight(*height);
}

void FORTRAN(gr_setcharup)(double *ux, double *uy)
{
  gr_setcharup(*ux, *uy);
}

void FORTRAN(gr_settextpath)(int *path)
{
  gr_settextpath(*path);
}

void FORTRAN(gr_settextalign)(int *horizontal, int *vertical)
{
  gr_settextalign(*horizontal, *vertical);
}

void FORTRAN(gr_setfillintstyle)(int *style)
{
  gr_setfillintstyle(*style);
}

void FORTRAN(gr_setfillstyle)(int *index)
{
  gr_setfillstyle(*index);
}

void FORTRAN(gr_setfillcolorind)(int *color)
{
  gr_setfillcolorind(*color);
}

void FORTRAN(gr_setcolorrep)(int *index, double *red, double *green, double *blue)
{
  gr_setcolorrep(*index, *red, *green, *blue);
}

void FORTRAN(gr_setwindow)(double *xmin, double *xmax, double *ymin, double *ymax)
{
  gr_setwindow(*xmin, *xmax, *ymin, *ymax);
}

void FORTRAN(gr_inqwindow)(double *xmin, double *xmax, double *ymin, double *ymax)
{
  gr_inqwindow(xmin, xmax, ymin, ymax);
}

void FORTRAN(gr_setviewport)(double *xmin, double *xmax, double *ymin, double *ymax)
{
  gr_setviewport(*xmin, *xmax, *ymin, *ymax);
}

void FORTRAN(gr_inqviewport)(double *xmin, double *xmax, double *ymin, double *ymax)
{
  gr_inqviewport(xmin, xmax, ymin, ymax);
}

void FORTRAN(gr_selntran)(int *transform)
{
  gr_selntran(*transform);
}

void FORTRAN(gr_setclip)(int *indicator)
{
  gr_setclip(*indicator);
}

void FORTRAN(gr_setwswindow)(
  double *xmin, double *xmax, double *ymin, double *ymax)
{
  gr_setwswindow(*xmin, *xmax, *ymin, *ymax);
}

void FORTRAN(gr_setwsviewport)(
  double *xmin, double *xmax, double *ymin, double *ymax)
{
  gr_setwsviewport(*xmin, *xmax, *ymin, *ymax);
}

void FORTRAN(gr_createseg)(int *segment)
{
  gr_createseg(*segment);
}

void FORTRAN(gr_copysegws)(int *segment)
{
  gr_copysegws(*segment);
}

void FORTRAN(gr_redrawsegws)(void)
{
  gr_redrawsegws();
}

void FORTRAN(gr_setsegtran)(
  int *segment, double *fx, double *fy, double *transx, double *transy, double *phi,
  double *scalex, double *scaley)
{
  gr_setsegtran(*segment, *fx, *fy, *transx, *transy, *phi, *scalex, *scaley);
}

void FORTRAN(gr_closeseg)(void)
{
  gr_closeseg();
}

void FORTRAN(gr_emergencyclosegks)(void)
{
  gr_emergencyclosegks();
}

void FORTRAN(gr_updategks)(void)
{
  gr_updategks();
}

int FORTRAN(gr_setspace)(double *zmin, double *zmax, int *rotation, int *tilt)
{
  return gr_setspace(*zmin, *zmax, *rotation, *tilt);
}

void FORTRAN(gr_inqspace)(double *zmin, double *zmax, int *rotation, int *tilt)
{
  gr_inqspace(zmin, zmax, rotation, tilt);
}

int FORTRAN(gr_setscale)(int *options)
{
  return gr_setscale(*options);
}

void FORTRAN(gr_inqscale)(int *options)
{
  gr_inqscale(options);
}

int FORTRAN(gr_textext)(
  double *x, double *y, char *string, unsigned short string_len)
{
  char *_string;
  int result;

  _string = (char *) calloc(1, sizeof(char) * string_len);
  strncpy(_string, string, string_len);

  result = gr_textext(*x, *y, _string);

  free(_string);

  return result;
}

void FORTRAN(gr_inqtextext)(
  double *x, double *y, char *string, double *tbx, double *tby,
  unsigned short string_len)
{
  char *_string;

  _string = (char *) calloc(1, sizeof(char) * string_len);
  strncpy(_string, string, string_len);

  gr_inqtextext(*x, *y, _string, tbx, tby);

  free(_string);
}

void FORTRAN(gr_axes)(
  double *x_tick, double *y_tick, double *x_org, double *y_org,
  int *major_x, int *major_y, double *tick_size)
{
  gr_axes(*x_tick, *y_tick, *x_org, *y_org, *major_x, *major_y, *tick_size);
}

void FORTRAN(gr_grid)(
  double *x_tick, double *y_tick, double *x_org, double *y_org,
  int *major_x, int *major_y)
{
  gr_grid(*x_tick, *y_tick, *x_org, *y_org, *major_x, *major_y);
}

void FORTRAN(gr_grid3d)(
  double *x_tick, double *y_tick, double *z_tick,
  double *x_org, double *y_org, double *z_org,
  int *major_x, int *major_y, int *major_z)
{
  gr_grid3d(*x_tick, *y_tick, *z_tick, *x_org, *y_org, *z_org,
            *major_x, *major_y, *major_z);
}

void FORTRAN(gr_verrorbars)(int *n, double *px, double *py, double *e1, double *e2)
{
  gr_verrorbars(*n, px, py, e1, e2);
}

void FORTRAN(gr_herrorbars)(int *n, double *px, double *py, double *e1, double *e2)
{
  gr_herrorbars(*n, px, py, e1, e2);
}

void FORTRAN(gr_polyline3d)(int *n, double *px, double *py, double *pz)
{
  gr_polyline3d(*n, px, py, pz);
}

void FORTRAN(gr_polymarker3d)(int *n, double *px, double *py, double *pz)
{
  gr_polymarker3d(*n, px, py, pz);
}

void FORTRAN(gr_axes3d)(
  double *x_tick, double *y_tick, double *z_tick,
  double *x_org, double *y_org, double *z_org,
  int *major_x, int *major_y, int *major_z, double *tick_size)
{
  gr_axes3d(*x_tick, *y_tick, *z_tick, *x_org, *y_org, *z_org,
  	    *major_x, *major_y, *major_z, *tick_size);
}

void FORTRAN(gr_titles3d)(
  char *x_title, char *y_title, char *z_title,
  unsigned short x_title_len, unsigned short y_title_len,
  unsigned short z_title_len)
{
  char *_x_title, *_y_title, *_z_title;

  _x_title = (char *) calloc(1, sizeof(char) * x_title_len);
  _y_title = (char *) calloc(1, sizeof(char) * y_title_len);
  _z_title = (char *) calloc(1, sizeof(char) * z_title_len);

  strncpy(_x_title, x_title, x_title_len);
  strncpy(_y_title, y_title, y_title_len);
  strncpy(_z_title, z_title, z_title_len);

  gr_titles3d(_x_title, _y_title, _z_title);

  free(_z_title);
  free(_y_title);
  free(_x_title);
}

void FORTRAN(gr_surface)(
  int *nx, int *ny, double *px, double *py, double *pz, int *option)
{
  gr_surface(*nx, *ny, px, py, pz, *option);
}

void FORTRAN(gr_contour)(
  int *nx, int *ny, int *nh, double *px, double *py, double *h, double *pz,
  int *major_h)
{
  gr_contour(*nx, *ny, *nh, px, py, h, pz, *major_h);
}

void FORTRAN(gr_tricontour)(
  int *npoints, double *x, double *y, double *z, int *nlevels, double *levels)
{
  gr_tricontour(*npoints, x, y, z, *nlevels, levels);
}

int FORTRAN(gr_hexbin)(
  int *n, double *x, double *y, int *nbins)
{
  return gr_hexbin(*n, x, y, *nbins);
}

void FORTRAN(gr_setcolormap)(int *index)
{
  gr_setcolormap(*index);
}

void FORTRAN(gr_colorbar)(void)
{
  gr_colorbar();
}

void FORTRAN(gr_inqcolor)(int *color, int *rgb)
{
  gr_inqcolor(*color, rgb);
}

double FORTRAN(gr_tick)(double *amin, double *amax)
{
  return gr_tick(*amin, *amax);
}

int FORTRAN(gr_validaterange)(double amin, double amax)
{
  return gr_validaterange(amin, amax);
}

void FORTRAN(gr_adjustrange)(double *amin, double *amax)
{
  gr_adjustrange(amin, amax);
}

void FORTRAN(gr_beginprint)(char *pathname, unsigned short pathname_len)
{
  char *_pathname;

  _pathname = (char *) calloc(1, sizeof(char) * pathname_len);
  strncpy(_pathname, pathname, pathname_len);

  gr_beginprint(_pathname);

  free(_pathname);
}

void FORTRAN(gr_beginprintext)(
  char *pathname, char *mode, char *format, char *orientation,
  unsigned short pathname_len, unsigned short mode_len,
  unsigned short format_len, unsigned short orientation_len)
{
  char *_pathname, *_mode, *_format, *_orientation;

  _pathname = (char *) calloc(1, sizeof(char) * pathname_len);
  _mode = (char *) calloc(1, sizeof(char) * mode_len);
  _format = (char *) calloc(1, sizeof(char) * format_len);
  _orientation = (char *) calloc(1, sizeof(char) * orientation_len);

  strncpy(_pathname, pathname, pathname_len);
  strncpy(_mode, mode, mode_len);
  strncpy(_format, format, format_len);
  strncpy(_orientation, orientation, orientation_len);

  gr_beginprintext(_pathname, _mode, _format, _orientation);

  free(_orientation);
  free(_format);
  free(_mode);
  free(_pathname);
}

void FORTRAN(gr_endprint)(void)
{
  gr_endprint();
}

void FORTRAN(gr_ndctowc)(double *x, double *y)
{
  gr_ndctowc(x, y);
}

void FORTRAN(gr_wctondc)(double *x, double *y)
{
  gr_wctondc(x, y);
}

void FORTRAN(gr_wc3towc)(double *x, double *y, double *z)
{
  gr_wc3towc(x, y, z);
}

void FORTRAN(gr_drawrect)(double *xmin, double *xmax, double *ymin, double *ymax)
{
  gr_drawrect(*xmin, *xmax, *ymin, *ymax);
}

void FORTRAN(gr_fillrect)(double *xmin, double *xmax, double *ymin, double *ymax)
{
  gr_fillrect(*xmin, *xmax, *ymin, *ymax);
}

void FORTRAN(gr_drawarc)(
  double *xmin, double *xmax, double *ymin, double *ymax, int *a1, int *a2)
{
  gr_drawarc(*xmin, *xmax, *ymin, *ymax, *a1, *a2);
}

void FORTRAN(gr_fillarc)(
  double *xmin, double *xmax, double *ymin, double *ymax, int *a1, int *a2)
{
  gr_fillarc(*xmin, *xmax, *ymin, *ymax, *a1, *a2);
}

void FORTRAN(gr_setarrowstyle)(int *style)
{
  gr_setarrowstyle(*style);
}

void FORTRAN(gr_setarrowsize)(double *size)
{
  gr_setarrowsize(*size);
}

void FORTRAN(gr_drawarrow)(double *x1, double *y1, double *x2, double *y2)
{
  gr_drawarrow(*x1, *y1, *x2, *y2);
}

void FORTRAN(gr_begingraphics)(char *pathname, unsigned short pathname_len)
{
  char *_pathname;

  _pathname = (char *) calloc(1, sizeof(char) * pathname_len);
  strncpy(_pathname, pathname, pathname_len);

  gr_begingraphics(_pathname);

  free(_pathname);
}

void FORTRAN(gr_endgraphics)(void)
{
  gr_endgraphics();
}

void FORTRAN(gr_mathtex)(
  double *x, double *y, char *string, unsigned char string_len)
{
  char *_string;

  _string = (char *) calloc(1, sizeof(char) * string_len);
  strncpy(_string, string, string_len);

  gr_mathtex(*x, *y, _string);

  free(_string);
}

