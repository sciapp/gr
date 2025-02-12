#ifndef _GR_H_
#define _GR_H_

#ifdef _WIN32

#define HAVE_BOOLEAN

#include <windows.h> /* required for all Windows applications */
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
  GR_PROJECTION_DEFAULT,
  GR_PROJECTION_ORTHOGRAPHIC,
  GR_PROJECTION_PERSPECTIVE
} projection_type_t;

typedef enum
{
  GR_VOLUME_WITHOUT_BORDER,
  GR_VOLUME_WITH_BORDER
} volume_border_calculation_t;

typedef enum
{
  GR_VOLUME_EMISSION,
  GR_VOLUME_ABSORPTION,
  GR_VOLUME_MIP
} volume_rendering_model_t;

typedef enum
{
  GR_TEXT_USE_WC = 1u << 0u,
  GR_TEXT_ENABLE_INLINE_MATH = 1u << 1u
} textx_option_t;

typedef enum
{
  GR_2PASS_CLEANUP = 1,
  GR_2PASS_RENDER
} f2pass_option_t;

#define GR_MAX_CONTEXT 8192

#define GR_DEFAULT_MATH_FONT 232

typedef enum
{
  GR_OPTION_X_LOG = 1u << 0u,
  GR_OPTION_Y_LOG = 1u << 1u,
  GR_OPTION_Z_LOG = 1u << 2u,

  GR_OPTION_FLIP_X = 1u << 3u,
  GR_OPTION_FLIP_Y = 1u << 4u,
  GR_OPTION_FLIP_Z = 1u << 5u,

  /* for use within GR.jl */
  GR_OPTION_X_LOG2 = 1u << 6u,
  GR_OPTION_Y_LOG2 = 1u << 7u,
  GR_OPTION_Z_LOG2 = 1u << 8u,
  GR_OPTION_X_LN = 1u << 9u,
  GR_OPTION_Y_LN = 1u << 10u,
  GR_OPTION_Z_LN = 1u << 11u
} scale_option_t;

typedef enum
{
  GR_SPEC_LINE = 1u << 0u,
  GR_SPEC_MARKER = 1u << 1u,
  GR_SPEC_COLOR = 1u << 2u
} linespec_t;

typedef enum
{
  GR_OPTION_LINES,
  GR_OPTION_MESH,
  GR_OPTION_FILLED_MESH,
  GR_OPTION_Z_SHADED_MESH,
  GR_OPTION_COLORED_MESH,
  GR_OPTION_CELL_ARRAY,
  GR_OPTION_SHADED_MESH,
  GR_OPTION_3D_MESH /* for GR3 */
} surface_option_t;

typedef enum
{
  GR_MODEL_RGB,
  GR_MODEL_HSV
} color_model_t;

typedef enum
{
  GR_STOP,
  GR_MOVETO,
  GR_LINETO,
  GR_CURVE3,
  GR_CURVE4,
  GR_CLOSEPOLY = 0x4f
} path_code_t;

typedef enum
{
  GR_XFORM_BOOLEAN,
  GR_XFORM_LINEAR,
  GR_XFORM_LOG,
  GR_XFORM_LOGLOG,
  GR_XFORM_CUBIC,
  GR_XFORM_EQUALIZED
} xform_types_t;

typedef enum
{
  GR_INTERP2_NEAREST,
  GR_INTERP2_LINEAR,
  GR_INTERP2_SPLINE,
  GR_INTERP2_CUBIC
} interp2_method_t;

typedef struct
{
  double x, y;
} vertex_t;

/*! Three-dimensional coordinate */
typedef struct
{
  double x, y, z;
} point3d_t;

/*! Data point for `gr_volume_nogrid` */
typedef struct
{
  point3d_t pt; /*!< Coordinates of data point */
  double data;  /*!< Intensity of data point */
} data_point3d_t;

/*! Provides optional extra data for `gr_volume_interp_gauss` */
typedef struct
{
  double sqrt_det;                                 /*!< Square root of determinant of covariance matrix */
  point3d_t gauss_sig_1, gauss_sig_2, gauss_sig_3; /*!< \f$\Sigma^{-\frac{1}{2}}\f$ encoded as three column vectors */
} gauss_t;

/*! Provides optional extra data for `gr_volume_interp_tri_linear` */
typedef struct
{
  double grid_x_re; /*!< Reciproke of interpolation kernel extent in x-direction */
  double grid_y_re; /*!< Reciproke of interpolation kernel extent in y-direction */
  double grid_z_re; /*!< Reciproke of interpolation kernel extent in z-direction */
} tri_linear_t;

typedef struct cpubasedvolume_2pass_priv cpubasedvolume_2pass_priv_t;
typedef struct
{
  double dmin;
  double dmax;
  int action;
  cpubasedvolume_2pass_priv_t *priv;
} cpubasedvolume_2pass_t;

typedef struct hexbin_2pass_priv hexbin_2pass_priv_t;
typedef struct
{
  int nc;
  int cntmax;
  int action;
  hexbin_2pass_priv_t *priv;
} hexbin_2pass_t;

typedef struct
{
  int scientific;
  int decimal_digits;
} format_reference_t;

typedef struct
{
  double value;
  int is_major;
} tick_t;

typedef struct
{
  double tick;
  char *label;
  double width;
} tick_label_t;

typedef struct
{
  double min, max;
  double tick, org;
  double position;
  int major_count;
  int num_ticks;
  tick_t *ticks;
  double tick_size;
  int num_tick_labels;
  tick_label_t *tick_labels;
  double label_position;
  int draw_axis_line;
  int label_orientation;
} axis_t;

#define GR_AXES_SIMPLE_AXES (1 << 0)
#define GR_AXES_TWIN_AXES (1 << 1)
#define GR_AXES_WITH_GRID (1 << 2)

DLLEXPORT void gr_initgr(void);
DLLEXPORT int gr_debug(void);
DLLEXPORT void gr_opengks(void);
DLLEXPORT void gr_closegks(void);
DLLEXPORT void gr_inqdspsize(double *, double *, int *, int *);
DLLEXPORT void gr_openws(int, char *, int);
DLLEXPORT void gr_closews(int);
DLLEXPORT void gr_activatews(int);
DLLEXPORT void gr_deactivatews(int);
DLLEXPORT void gr_configurews(void);
DLLEXPORT void gr_clearws(void);
DLLEXPORT void gr_updatews(void);
DLLEXPORT void gr_polyline(int, double *, double *);
DLLEXPORT void gr_polymarker(int, double *, double *);
DLLEXPORT void gr_text(double, double, char *);
DLLEXPORT void gr_textx(double, double, char *, int);
DLLEXPORT void gr_inqtext(double, double, char *, double *, double *);
DLLEXPORT void gr_inqtextx(double, double, char *, int, double *, double *);
DLLEXPORT void gr_fillarea(int, double *, double *);
DLLEXPORT void gr_cellarray(double, double, double, double, int, int, int, int, int, int, int *);
DLLEXPORT void gr_nonuniformcellarray(double *, double *, int, int, int, int, int, int, int *);
DLLEXPORT void gr_polarcellarray(double, double, double, double, double, double, int, int, int, int, int, int, int *);
DLLEXPORT void gr_nonuniformpolarcellarray(double, double, double *, double *, int, int, int, int, int, int, int *);
DLLEXPORT void gr_gdp(int, double *, double *, int, int, int *);
DLLEXPORT void gr_spline(int, double *, double *, int, int);
DLLEXPORT void gr_gridit(int, double *, double *, double *, int, int, double *, double *, double *);
DLLEXPORT void gr_setlinetype(int);
DLLEXPORT void gr_inqlinetype(int *);
DLLEXPORT void gr_setlinewidth(double);
DLLEXPORT void gr_inqlinewidth(double *);
DLLEXPORT void gr_setlinecolorind(int);
DLLEXPORT void gr_inqlinecolorind(int *);
DLLEXPORT void gr_setmarkertype(int);
DLLEXPORT void gr_inqmarkertype(int *);
DLLEXPORT void gr_setmarkersize(double);
DLLEXPORT void gr_inqmarkersize(double *);
DLLEXPORT void gr_setmarkercolorind(int);
DLLEXPORT void gr_inqmarkercolorind(int *);
DLLEXPORT void gr_settextfontprec(int, int);
DLLEXPORT void gr_setcharexpan(double);
DLLEXPORT void gr_setcharspace(double);
DLLEXPORT void gr_settextcolorind(int);
DLLEXPORT void gr_inqtextcolorind(int *);
DLLEXPORT void gr_setcharheight(double);
DLLEXPORT void gr_setwscharheight(double chh, double height);
DLLEXPORT void gr_inqcharheight(double *);
DLLEXPORT void gr_setcharup(double, double);
DLLEXPORT void gr_settextpath(int);
DLLEXPORT void gr_settextalign(int, int);
DLLEXPORT void gr_setfillintstyle(int);
DLLEXPORT void gr_inqfillintstyle(int *);
DLLEXPORT void gr_setfillstyle(int);
DLLEXPORT void gr_inqfillstyle(int *);
DLLEXPORT void gr_setfillcolorind(int);
DLLEXPORT void gr_inqfillcolorind(int *);
DLLEXPORT void gr_setnominalsize(double);
DLLEXPORT void gr_inqnominalsize(double *);
DLLEXPORT void gr_setcolorrep(int, double, double, double);
DLLEXPORT void gr_setwindow(double, double, double, double);
DLLEXPORT void gr_inqwindow(double *, double *, double *, double *);
DLLEXPORT void gr_setviewport(double, double, double, double);
DLLEXPORT void gr_inqviewport(double *, double *, double *, double *);
DLLEXPORT void gr_selntran(int);
DLLEXPORT void gr_setclip(int);
DLLEXPORT void gr_setwswindow(double, double, double, double);
DLLEXPORT void gr_setwsviewport(double, double, double, double);
DLLEXPORT void gr_createseg(int);
DLLEXPORT void gr_copysegws(int);
DLLEXPORT void gr_redrawsegws(void);
DLLEXPORT void gr_setsegtran(int, double, double, double, double, double, double, double);
DLLEXPORT void gr_closeseg(void);
DLLEXPORT void gr_samplelocator(double *, double *, int *);
DLLEXPORT void gr_emergencyclosegks(void);
DLLEXPORT void gr_updategks(void);
DLLEXPORT int gr_setspace(double, double, int, int);
DLLEXPORT void gr_inqspace(double *, double *, int *, int *);
DLLEXPORT int gr_setscale(int);
DLLEXPORT void gr_inqscale(int *);
DLLEXPORT int gr_textext(double, double, char *);
DLLEXPORT void gr_inqtextext(double, double, char *, double *, double *);
DLLEXPORT void gr_setscientificformat(int);
DLLEXPORT void gr_axes(double, double, double, double, int, int, double);
DLLEXPORT void gr_axeslbl(double, double, double, double, int, int, double,
                          void (*)(double, double, const char *, double),
                          void (*)(double, double, const char *, double));
DLLEXPORT void gr_axis(char, axis_t *);
DLLEXPORT void gr_drawaxis(char, axis_t *);
DLLEXPORT void gr_drawaxes(axis_t *, axis_t *, int);
DLLEXPORT void gr_freeaxis(axis_t *);
DLLEXPORT void gr_grid(double, double, double, double, int, int);
DLLEXPORT void gr_grid3d(double, double, double, double, double, double, int, int, int);
DLLEXPORT void gr_verrorbars(int, double *, double *, double *, double *);
DLLEXPORT void gr_herrorbars(int, double *, double *, double *, double *);
DLLEXPORT void gr_polyline3d(int, double *, double *, double *);
DLLEXPORT void gr_polymarker3d(int, double *, double *, double *);
DLLEXPORT void gr_axes3d(double, double, double, double, double, double, int, int, int, double);
DLLEXPORT void gr_titles3d(char *, char *, char *);
DLLEXPORT void gr_settitles3d(char *, char *, char *);
DLLEXPORT void gr_surface(int, int, double *, double *, double *, int);
DLLEXPORT void gr_contour(int, int, int, double *, double *, double *, double *, int);
DLLEXPORT void gr_contourf(int, int, int, double *, double *, double *, double *, int);
DLLEXPORT void gr_tricontour(int, double *, double *, double *, int, double *);
DLLEXPORT int gr_hexbin(int, double *, double *, int);
DLLEXPORT const hexbin_2pass_t *gr_hexbin_2pass(int, double *, double *, int, const hexbin_2pass_t *);
DLLEXPORT void gr_setcolormap(int);
DLLEXPORT void gr_inqcolormap(int *);
DLLEXPORT void gr_setcolormapfromrgb(int n, double *r, double *g, double *b, double *x);
DLLEXPORT void gr_inqcolormapinds(int *, int *);
DLLEXPORT void gr_colorbar(void);
DLLEXPORT void gr_inqcolor(int, int *);
DLLEXPORT int gr_inqcolorfromrgb(double, double, double);
DLLEXPORT void gr_hsvtorgb(double h, double s, double v, double *r, double *g, double *b);
DLLEXPORT double gr_tick(double, double);
DLLEXPORT int gr_validaterange(double, double);
DLLEXPORT void gr_adjustlimits(double *, double *);
DLLEXPORT void gr_adjustrange(double *, double *);
DLLEXPORT void gr_beginprint(char *);
DLLEXPORT void gr_beginprintext(char *, char *, char *, char *);
DLLEXPORT void gr_endprint(void);
DLLEXPORT void gr_ndctowc(double *, double *);
DLLEXPORT void gr_wctondc(double *, double *);
DLLEXPORT void gr_wc3towc(double *, double *, double *);
DLLEXPORT void gr_drawrect(double, double, double, double);
DLLEXPORT void gr_fillrect(double, double, double, double);
DLLEXPORT void gr_drawarc(double, double, double, double, double, double);
DLLEXPORT void gr_fillarc(double, double, double, double, double, double);
DLLEXPORT void gr_drawpath(int, vertex_t *, unsigned char *, int);
DLLEXPORT void gr_setarrowstyle(int);
DLLEXPORT void gr_setarrowsize(double);
DLLEXPORT void gr_drawarrow(double, double, double, double);
DLLEXPORT int gr_readimage(char *, int *, int *, int **);
DLLEXPORT void gr_drawimage(double, double, double, double, int, int, int *, int);
DLLEXPORT int gr_importgraphics(char *);
DLLEXPORT void gr_setshadow(double, double, double);
DLLEXPORT void gr_settransparency(double);
DLLEXPORT void gr_inqtransparency(double *);
DLLEXPORT void gr_setcoordxform(double[3][2]);
DLLEXPORT void gr_begingraphics(char *);
DLLEXPORT void gr_endgraphics(void);
DLLEXPORT char *gr_getgraphics(void);
DLLEXPORT int gr_drawgraphics(char *);
DLLEXPORT int gr_startlistener(void);
DLLEXPORT void gr_mathtex(double, double, char *);
DLLEXPORT void gr_inqmathtex(double, double, char *, double *, double *);
DLLEXPORT void gr_mathtex3d(double, double, double, char *, int);
DLLEXPORT void gr_inqmathtex3d(double, double, double, char *, int, double *, double *, double *, double *);
DLLEXPORT void gr_beginselection(int, int);
DLLEXPORT void gr_endselection(void);
DLLEXPORT void gr_setbboxcallback(int, void (*)(int, double, double, double, double));
DLLEXPORT void gr_cancelbboxcallback(void);
DLLEXPORT void gr_moveselection(double, double);
DLLEXPORT void gr_resizeselection(int, double, double);
DLLEXPORT void gr_inqbbox(double *, double *, double *, double *);
DLLEXPORT void gr_setbackground(void);
DLLEXPORT void gr_clearbackground(void);
DLLEXPORT double gr_precision(void);
DLLEXPORT int gr_text_maxsize(void);
DLLEXPORT void gr_setregenflags(int);
DLLEXPORT int gr_inqregenflags(void);
DLLEXPORT void gr_savestate(void);
DLLEXPORT void gr_restorestate(void);
DLLEXPORT void gr_savecontext(int);
DLLEXPORT void gr_selectcontext(int);
DLLEXPORT void gr_destroycontext(int);
DLLEXPORT void gr_unselectcontext(void);
DLLEXPORT int gr_uselinespec(char *);
DLLEXPORT void gr_delaunay(int, const double *, const double *, int *, int **);
DLLEXPORT void gr_reducepoints(int, const double *, const double *, int, double *, double *);
DLLEXPORT void gr_trisurface(int, double *, double *, double *);
DLLEXPORT void gr_gradient(int, int, double *, double *, double *, double *, double *);
DLLEXPORT void gr_quiver(int, int, double *, double *, double *, double *, int);
DLLEXPORT void gr_interp2(int nx, int ny, const double *x, const double *y, const double *z, int nxq, int nyq,
                          const double *xq, const double *yq, double *zq, interp2_method_t method, double extrapval);
DLLEXPORT const char *gr_version(void);
DLLEXPORT void gr_shade(int, double *, double *, int, int, double *, int, int, int *);
DLLEXPORT void gr_shadepoints(int, double *, double *, int, int, int);
DLLEXPORT void gr_shadelines(int, double *, double *, int, int, int);
DLLEXPORT void gr_panzoom(double, double, double, double, double *, double *, double *, double *);
DLLEXPORT int gr_findboundary(int, double *, double *, double, double (*)(double, double), int, int *);
DLLEXPORT void gr_setresamplemethod(unsigned int);
DLLEXPORT void gr_inqresamplemethod(unsigned int *);
DLLEXPORT void gr_path(int, double *, double *, const char *);
DLLEXPORT void gr_setborderwidth(double);
DLLEXPORT void gr_inqborderwidth(double *);
DLLEXPORT void gr_setbordercolorind(int);
DLLEXPORT void gr_inqbordercolorind(int *);
DLLEXPORT void gr_selectclipxform(int);
DLLEXPORT void gr_inqclipxform(int *);
DLLEXPORT void gr_setprojectiontype(int);
DLLEXPORT void gr_inqprojectiontype(int *);
DLLEXPORT void gr_setperspectiveprojection(double, double, double);
DLLEXPORT void gr_inqperspectiveprojection(double *, double *, double *);
DLLEXPORT void gr_settransformationparameters(double, double, double, double, double, double, double, double, double);
DLLEXPORT void gr_inqtransformationparameters(double *, double *, double *, double *, double *, double *, double *,
                                              double *, double *);
DLLEXPORT void gr_setorthographicprojection(double, double, double, double, double, double);
DLLEXPORT void gr_inqorthographicprojection(double *, double *, double *, double *, double *, double *);
DLLEXPORT void gr_camerainteraction(double, double, double, double);
DLLEXPORT void gr_setwindow3d(double, double, double, double, double, double);
DLLEXPORT void gr_inqwindow3d(double *, double *, double *, double *, double *, double *);
DLLEXPORT void gr_setscalefactors3d(double, double, double);
DLLEXPORT void gr_inqscalefactors3d(double *, double *, double *);
DLLEXPORT void gr_setspace3d(double, double, double, double);
DLLEXPORT void gr_inqspace3d(int *, double *, double *, double *, double *);
DLLEXPORT void gr_text3d(double, double, double, char *, int axis);
DLLEXPORT void gr_inqtext3d(double, double, double, char *, int axis, double *, double *);
DLLEXPORT void gr_settextencoding(int);
DLLEXPORT void gr_inqtextencoding(int *);
DLLEXPORT void gr_loadfont(char *, int *);
DLLEXPORT void gr_setcallback(char *(*)(const char *));
DLLEXPORT void gr_setthreadnumber(int);
DLLEXPORT void gr_setpicturesizeforvolume(int, int);
DLLEXPORT void gr_setvolumebordercalculation(int);
DLLEXPORT void gr_setapproximativecalculation(int);
DLLEXPORT void gr_inqvolumeflags(int *, int *, int *, int *, int *);
DLLEXPORT void gr_cpubasedvolume(int, int, int, double *, int, double *, double *, double *, double *);
DLLEXPORT const cpubasedvolume_2pass_t *gr_cpubasedvolume_2pass(int, int, int, double *, int, double *, double *,
                                                                double *, double *, const cpubasedvolume_2pass_t *);
DLLEXPORT void gr_inqvpsize(int *, int *, double *);
DLLEXPORT void gr_polygonmesh3d(int, const double *, const double *, const double *, int, const int *, const int *);

typedef double (*kernel_f)(const data_point3d_t *, const void *, const point3d_t *, const point3d_t *);
typedef double (*radius_f)(const data_point3d_t *, const void *);

DLLEXPORT void gr_volume_nogrid(unsigned long, const data_point3d_t *, const void *, int, kernel_f, double *, double *,
                                double, radius_f);

DLLEXPORT void gr_volume_interp_tri_linear_init(double, double, double);
DLLEXPORT void gr_volume_interp_gauss_init(double, double *);
DLLEXPORT double gr_volume_interp_tri_linear(const data_point3d_t *, const void *, const point3d_t *,
                                             const point3d_t *);
DLLEXPORT double gr_volume_interp_gauss(const data_point3d_t *, const void *, const point3d_t *, const point3d_t *);
DLLEXPORT void gr_setmathfont(int font);
DLLEXPORT void gr_inqmathfont(int *font);
DLLEXPORT void gr_setclipregion(int region);
DLLEXPORT void gr_inqclipregion(int *region);
DLLEXPORT void gr_setclipsector(double start_angle, double end_angle);
DLLEXPORT void gr_inqclipsector(double *start_angle, double *end_angle);
DLLEXPORT void gr_settextoffset(double xoff, double yoff);
DLLEXPORT char *gr_ftoa(char *string, double value, format_reference_t *reference);
DLLEXPORT void gr_getformat(format_reference_t *result, double origin, double min, double max, double tick_width,
                            int major);

#ifdef __cplusplus
}
#endif

#endif
