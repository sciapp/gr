#ifndef GRM_PLOT_INT_H_INCLUDED
#define GRM_PLOT_INT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#include "args_int.h"
#include "datatype/uint_map_int.h"
#include "grm/error.h"
#include "event_int.h"

#ifdef __cplusplus
}

#include <grm/dom_render/graphics_tree/Element.hxx>

#include <grm/plot.h>
extern "C" {
#else
#include <grm/plot.h>
#endif


/* ######################### internal interface ##################################################################### */

/* ========================= global varibales ======================================================================= */

/* ------------------------- plot ----------------------------------------------------------------------------------- */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ args ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

extern grm_args_t *active_plot_args;


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ event handling ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

extern event_queue_t *event_queue;


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plot clear ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

extern const char *plot_clear_exclude_keys[];


/* ========================= macros ================================================================================= */

/* ------------------------- plot ----------------------------------------------------------------------------------- */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plot arguments ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#define ROOT_DEFAULT_APPEND_PLOTS 0
#define PLOT_DEFAULT_WIDTH 600.0
#define PLOT_DEFAULT_HEIGHT 450.0
#define PLOT_DEFAULT_KIND "line"
#define PLOT_DEFAULT_SPEC ""
#define PLOT_DEFAULT_CLEAR 1
#define PLOT_DEFAULT_UPDATE 1
#define PLOT_DEFAULT_LOCATION 1
#define PLOT_DEFAULT_SUBPLOT_MIN_X 0.0
#define PLOT_DEFAULT_SUBPLOT_MAX_X 1.0
#define PLOT_DEFAULT_SUBPLOT_MIN_Y 0.0
#define PLOT_DEFAULT_SUBPLOT_MAX_Y 1.0
#define PLOT_DEFAULT_XLOG 0
#define PLOT_DEFAULT_YLOG 0
#define PLOT_DEFAULT_ZLOG 0
#define PLOT_DEFAULT_XFLIP 0
#define PLOT_DEFAULT_YFLIP 0
#define PLOT_DEFAULT_ZFLIP 0
#define PLOT_DEFAULT_XGRID 1
#define PLOT_DEFAULT_YGRID 1
#define PLOT_DEFAULT_ZGRID 1
#define PLOT_DEFAULT_RESAMPLE_METHOD GKS_K_RESAMPLE_DEFAULT
#define PLOT_DEFAULT_ADJUST_XLIM 1
#define PLOT_DEFAULT_ADJUST_YLIM 1
#define PLOT_DEFAULT_ADJUST_ZLIM 1
#define PLOT_DEFAULT_COLORMAP 44                                 /* VIRIDIS */
#define PLOT_DEFAULT_FONT 232                                    /* CMUSerif-Math */
#define PLOT_DEFAULT_FONT_PRECISION GKS_K_TEXT_PRECISION_OUTLINE /* hardware font rendering */
#define PLOT_DEFAULT_ROTATION 40.0
#define PLOT_DEFAULT_TILT 60.0
#define PLOT_DEFAULT_KEEP_ASPECT_RATIO 0
#define PLOT_DEFAULT_XLABEL ""
#define PLOT_DEFAULT_YLABEL ""
#define PLOT_DEFAULT_ZLABEL ""
#define PLOT_DEFAULT_STEP_WHERE "mid"
#define PLOT_DEFAULT_CONTOUR_LEVELS 20
#define PLOT_DEFAULT_HEXBIN_NBINS 40
#define PLOT_DEFAULT_TRICONT_LEVELS 20
#define PLOT_DEFAULT_VOLUME_ALGORITHM GR_VOLUME_EMISSION
#define PLOT_DEFAULT_ORIENTATION "horizontal"
#define SERIES_DEFAULT_SPEC ""
#define PLOT_POLAR_AXES_TEXT_BUFFER 40
#define PLOT_CONTOUR_GRIDIT_N 200
#define PLOT_WIREFRAME_GRIDIT_N 50
#define PLOT_SURFACE_GRIDIT_N 200


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ util ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#define PLOT_CUSTOM_COLOR_INDEX 979


/* ========================= datatypes ============================================================================== */

/* ------------------------- plot ----------------------------------------------------------------------------------- */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ kind to func ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

typedef err_t (*plot_func_t)(grm_args_t *args);


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ options ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

typedef enum
{
  GR_OPTION_X_LOG = 1 << 0,
  GR_OPTION_Y_LOG = 1 << 1,
  GR_OPTION_Z_LOG = 1 << 2,
  GR_OPTION_FLIP_X = 1 << 3,
  GR_OPTION_FLIP_Y = 1 << 4,
  GR_OPTION_FLIP_Z = 1 << 5,
  GR_OPTION_LINES = 0,
  GR_OPTION_MESH = 1,
  GR_OPTION_FILLED_MESH = 2,
  GR_OPTION_Z_SHADED_MESH = 3,
  GR_OPTION_COLORED_MESH = 4,
  GR_OPTION_CELL_ARRAY = 5,
  GR_OPTION_SHADED_MESH = 6,
  GR_OPTION_3D_MESH = 7
} gr_option_t;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ util ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

typedef enum
{
  GR_COLOR_RESET = 0,
  GR_COLOR_LINE = 1 << 0,
  GR_COLOR_MARKER = 1 << 1,
  GR_COLOR_FILL = 1 << 2,
  GR_COLOR_TEXT = 1 << 3,
  GR_COLOR_BORDER = 1 << 4
} gr_color_type_t;


/* ========================= functions ============================================================================== */

/* ------------------------- plot ----------------------------------------------------------------------------------- */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ general ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

err_t plot_init_static_variables(void);


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plot arguments ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

err_t plot_merge_args(grm_args_t *args, const grm_args_t *merge_args, const char **hierarchy_name_ptr,
                      uint_map_t *hierarchy_to_id, int hold_always);
err_t plot_init_arg_structure(arg_t *arg, const char **hierarchy_name_ptr, unsigned int next_hierarchy_level_max_id);
err_t plot_init_args_structure(grm_args_t *args, const char **hierarchy_name_ptr,
                               unsigned int next_hierarchy_level_max_id);
int plot_check_for_request(const grm_args_t *args, err_t *error);
void plot_set_flag_defaults(void);
void plot_set_attribute_defaults(grm_args_t *subplot_args);
void plot_pre_plot(grm_args_t *plot_args);
void plot_set_text_encoding(void);
void plot_process_wswindow_wsviewport(grm_args_t *plot_args);
err_t plot_pre_subplot(grm_args_t *subplot_args);
int plot_process_subplot_args(grm_args_t *subplot_args);
void plot_process_colormap(grm_args_t *subplot_args);
void plot_process_font(grm_args_t *subplot_args);
err_t plot_process_grid_arguments(const grm_args_t *args);
void plot_process_resample_method(grm_args_t *subplot_args);
void plot_process_viewport(grm_args_t *subplot_args);
void plot_process_window(grm_args_t *subplot_args);
err_t plot_store_coordinate_ranges(grm_args_t *subplot_args);
void plot_post_plot(grm_args_t *plot_args);
void plot_restore_text_encoding(void);
void plot_post_subplot(grm_args_t *subplot_args);
err_t plot_get_args_in_hierarchy(grm_args_t *args, const char **hierarchy_name_start_ptr, const char *key,
                                 uint_map_t *hierarchy_to_id, const grm_args_t **found_args,
                                 const char ***found_hierarchy_ptr);


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plotting ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

err_t plot_line(grm_args_t *subplot_args);
err_t plot_stairs(grm_args_t *subplot_args);
err_t plot_scatter(grm_args_t *subplot_args);
err_t plot_quiver(grm_args_t *subplot_args);
err_t plot_stem(grm_args_t *subplot_args);
err_t plot_hist(grm_args_t *subplot_args);
err_t plot_barplot(grm_args_t *subplot_args);
err_t plot_contour(grm_args_t *subplot_args);
err_t plot_contourf(grm_args_t *subplot_args);
err_t plot_hexbin(grm_args_t *subplot_args);
err_t plot_heatmap(grm_args_t *subplot_args);
err_t plot_polar_heatmap(grm_args_t *subplot_args);
err_t plot_marginalheatmap(grm_args_t *subplot_args);
err_t plot_wireframe(grm_args_t *subplot_args);
err_t plot_surface(grm_args_t *subplot_args);
err_t plot_plot3(grm_args_t *subplot_args);
err_t plot_scatter3(grm_args_t *subplot_args);
err_t plot_imshow(grm_args_t *subplot_args);
err_t plot_isosurface(grm_args_t *subplot_args);
err_t plot_volume(grm_args_t *subplot_args);
err_t plot_polar(grm_args_t *subplot_args);
err_t plot_polar_histogram(grm_args_t *subplot_args);
err_t plot_pie(grm_args_t *subplot_args);
err_t plot_trisurf(grm_args_t *subplot_args);
err_t plot_tricont(grm_args_t *subplot_args);
err_t plot_shade(grm_args_t *subplot_args);
err_t plot_raw(grm_args_t *subplot_args);
err_t plot_polar_heatmap(grm_args_t *subplot_args);


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ auxiliary drawing functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

err_t plot_draw_axes(grm_args_t *args, unsigned int pass);
err_t plot_draw_polar_axes(grm_args_t *args);
err_t plot_draw_legend(grm_args_t *args);
err_t plot_draw_pie_legend(grm_args_t *args);
err_t plot_draw_colorbar(grm_args_t *args, double off, unsigned int colors);
err_t plot_draw_errorbars(grm_args_t *series_args, double *x, unsigned int x_length, double *y, const char *kind);


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ util ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

double find_max_step(unsigned int n, const double *x);
double *normalize(unsigned int n, const double *x);
unsigned int *normalize_int(unsigned int n, const double *x, unsigned int sum);
const char *next_fmt_key(const char *fmt) UNUSED;
const char *get_compatible_format(const char *key, const char *given_format);
int get_id_from_args(const grm_args_t *args, int *plot_id, int *subplot_id, int *series_id);
int get_figure_size(const grm_args_t *plot_args, int *pixel_width, int *pixel_height, double *metric_width,
                    double *metric_height);
int get_focus_and_factor(const int top, const int right, const int bottom, const int left, const int keep_aspect_ratio,
                         double *factor_x, double *factor_y, double *focus_x, double *focus_y,
                         grm_args_t **subplot_args);
grm_args_t *get_subplot_from_ndc_point(double x, double y);
grm_args_t *get_subplot_from_ndc_points(unsigned int n, const double *x, const double *y);
double *moivre(double r, int x, int n);
double *listcomprehension(double count, double (*pFunction)(double), double *pDouble, int num, int start,
                          double *result);
int *create_colormap(int x, int y, int size);
err_t classes_polar_histogram(grm_args_t *subplot_args);
double get_lightness_from_rbg(double r, double g, double b);
void set_text_color_for_background(double r, double g, double b);
void draw_xticklabel(double x1, double x2, const char *label, double available_width);
void set_next_color(const grm_args_t *args, const char *key, gr_color_type_t color_type);
double auto_tick(double amin, double amax);


#ifdef __cplusplus
}
int set_next_color(const grm_args_t *args, const char *key, gr_color_type_t color_type,
                   const std::shared_ptr<GRM::Element> &element);
// void set_nect_color(std::optional<std::vector<int>> color_indices, std::optional<std::vector<double>>
// color_rgb_values,
//                     const std::string &key, gr_color_type_t color_type, const std::shared_ptr<GRM::Element>
//                     &element);

void set_text_color_for_background(double r, double g, double b, const std::shared_ptr<GRM::Element> &element);
void draw_xticklabel(double x1, double x2, const char *label, double available_width,
                     const std::shared_ptr<GRM::Element> &element);

double auto_tick_rings_polar(double rmax, int &rings, const std::string &norm);


#endif
#endif /* ifndef GRM_PLOT_INT_H_INCLUDED */
