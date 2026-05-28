#ifndef GR_DOM_RENDER_H
#define GR_DOM_RENDER_H

#ifdef __cplusplus
extern "C" {
#endif
#include "../util.h"
#include "../layout.h"
#include "graphics_tree/graphics_tree_c.h"

typedef struct grm_context grm_context_t;
typedef struct grm_inner_context grm_inner_context_t;
typedef struct grm_creator grm_creator_t;
typedef struct grm_group_mask grm_group_mask_t;
typedef struct grm_render grm_render_t;
typedef struct grm_manage_z_index grm_manage_z_index_t;
typedef struct grm_coordinate_space grm_coordinate_space_t;

/* =============================== casts ============================================================================ */
/* ------------------------------- string to int -------------------------------------------------------------------- */

GRM_EXPORT int grm_algorithm_string_to_int(const char *algorithm_str);
GRM_EXPORT int grm_colormap_string_to_int(const char *colormap_str);
GRM_EXPORT int grm_font_string_to_int(const char *font_str);
GRM_EXPORT int grm_font_precision_string_to_int(const char *ont_precision_str);
GRM_EXPORT int grm_line_type_string_to_int(const char *line_type_str);
GRM_EXPORT int grm_location_string_to_int(const char *location_str);
GRM_EXPORT int grm_x_axis_location_string_to_int(const char *location_str);
GRM_EXPORT int grm_y_axis_location_string_to_int(const char *location_str);
GRM_EXPORT int grm_marker_type_string_to_int(const char *marker_type_str);
GRM_EXPORT int grm_color_model_string_to_int(const char *color_model_str);
GRM_EXPORT int grm_scientific_format_string_to_int(const char *scientific_format_str);
GRM_EXPORT int grm_text_align_horizontal_string_to_int(const char *text_align_horizontal_str);
GRM_EXPORT int grm_text_align_vertical_string_to_int(const char *text_align_vertical_str);
GRM_EXPORT int grm_text_encoding_string_to_int(const char *text_encoding_str);
GRM_EXPORT int grm_tick_orientation_string_to_int(const char *tick_orientation_str);
GRM_EXPORT int grm_error_bar_style_string_to_int(const char *error_bar_stylr_str);
GRM_EXPORT int grm_clip_region_string_to_int(const char *error_bar_stylr_str);
GRM_EXPORT int grm_resample_method_string_to_int(const char *error_bar_stylr_str);
GRM_EXPORT int grm_fill_style_string_to_int(const char *fill_style_str);
GRM_EXPORT int grm_fill_int_style_string_to_int(const char *fill_int_style_str);
GRM_EXPORT int grm_transformation_string_to_int(const char *transformation_str);
GRM_EXPORT int grm_label_orientation_string_to_int(const char *label_orientation_str);
GRM_EXPORT int grm_world_coordinates_string_to_int(const char *world_coordinates_str);

/* ------------------------------- int to string ---------------------------------------------------------------------*/

GRM_EXPORT const char *grm_algorithm_int_to_string(int algorithm);
GRM_EXPORT const char *grm_colormap_int_to_string(int colormap);
GRM_EXPORT const char *grm_font_int_to_string(int font);
GRM_EXPORT const char *grm_font_precision_int_to_string(int font_precision);
GRM_EXPORT const char *grm_line_type_int_to_string(int line_type);
GRM_EXPORT const char *grm_location_int_to_string(int location);
GRM_EXPORT const char *grm_x_axis_location_int_to_string(int location);
GRM_EXPORT const char *grm_y_axis_location_int_to_string(int location);
GRM_EXPORT const char *grm_marker_type_int_to_string(int marker_type);
GRM_EXPORT const char *grm_color_model_int_to_string(int color_model);
GRM_EXPORT const char *grm_scientific_format_int_to_string(int scientific_format);
GRM_EXPORT const char *grm_text_align_horizontal_int_to_string(int text_align_horizontal);
GRM_EXPORT const char *grm_text_align_vertical_int_to_string(int text_align_vertical);
GRM_EXPORT const char *grm_text_encoding_int_to_string(int text_encoding);
GRM_EXPORT const char *grm_tick_orientation_int_to_string(int tick_orientation);
GRM_EXPORT const char *grm_error_bar_style_int_to_string(int error_bar_style);
GRM_EXPORT const char *grm_clip_region_int_to_string(int error_bar_style);
GRM_EXPORT const char *grm_resample_method_int_to_string(int error_bar_style);
GRM_EXPORT const char *grm_fill_style_int_to_string(int fill_style);
GRM_EXPORT const char *grm_fill_int_style_int_to_string(int fill_int_style);
GRM_EXPORT const char *grm_transformation_int_to_string(int transformation);
GRM_EXPORT const char *grm_label_orientation_int_to_string(int label_orientation);
GRM_EXPORT const char *grm_world_coordinates_int_to_string(int world_coordinates);

/* ------------------------------- get functions ---------------------------------------------------------------------*/

GRM_EXPORT void grm_get_size_units(const char **return_value);
GRM_EXPORT void grm_get_colormaps(const char **return_value);
GRM_EXPORT void grm_get_fonts(const char **return_value);
GRM_EXPORT void grm_get_font_precisions(const char **return_value);
GRM_EXPORT void grm_get_line_types(const char **return_value);
GRM_EXPORT void grm_get_locations(const char **return_value);
GRM_EXPORT void grm_get_x_axis_locations(const char **return_value);
GRM_EXPORT void grm_get_y_axis_locations(const char **return_value);
GRM_EXPORT void grm_get_marker_types(const char **return_value);
GRM_EXPORT void grm_get_text_align_horizontal(const char **return_value);
GRM_EXPORT void grm_get_text_align_vertical(const char **return_value);
GRM_EXPORT void grm_get_algorithm(const char **return_value);
GRM_EXPORT void grm_get_color_model(const char **return_value);
GRM_EXPORT void grm_get_context_attributes(const char **return_value);
GRM_EXPORT void grm_get_fill_styles(const char **return_value);
GRM_EXPORT void grm_get_fill_int_styles(const char **return_value);
GRM_EXPORT void grm_get_transformation(const char **return_value);

/* =============================== context ========================================================================== */

GRM_EXPORT void grm_new_inner_context(grm_context_t *context, const char *key, grm_inner_context_t **a_inner_context);
GRM_EXPORT void grm_new_inner_context_const(const grm_context_t *context, const char *key,
                                            grm_inner_context_t **a_inner_context);
GRM_EXPORT void grm_inner_context_set_int(grm_inner_context_t *inner_context, int *value, unsigned int value_length);
GRM_EXPORT void grm_inner_context_set_double(grm_inner_context_t *inner_context, double *value,
                                             unsigned int value_length);
GRM_EXPORT void grm_inner_context_set_string(grm_inner_context_t *inner_context, const char **value,
                                             unsigned int value_length);
GRM_EXPORT int grm_inner_context_int_used(grm_inner_context_t *inner_context);
GRM_EXPORT int grm_inner_context_double_used(grm_inner_context_t *inner_context);
GRM_EXPORT int grm_inner_context_string_used(grm_inner_context_t *inner_context);
GRM_EXPORT void grm_inner_context_delete_key(grm_inner_context_t *inner_context, const char *key);
GRM_EXPORT void grm_inner_context_use_context_key(grm_inner_context_t *inner_context, const char *key,
                                                  const char *old_key);
GRM_EXPORT void grm_inner_context_decrement_key(grm_inner_context_t *inner_context, const char *key);
GRM_EXPORT void grm_new_context(grm_context_t **a_context);

/* =============================== creator ========================================================================== */
/* ------------------------------- create functions ----------------------------------------------------------------*/

GRM_EXPORT grm_creator_t *grm_create_creator(grm_context_t *context);

GRM_EXPORT grm_element_t *grm_create_plot(int plot_id, grm_element_t *ext_element, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_central_region(grm_element_t *ext_element, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_polymarkers(const char *x_key, double *x, int x_length, const char *y_key,
                                                 double *y, int y_length, grm_context_t *ext_context, int marker_type,
                                                 double marker_size, int marker_color_ind, grm_element_t *ext_element,
                                                 grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_polymarker(double x, double y, int marker_type, double marker_size,
                                                int marker_colorind, grm_element_t *ext_element,
                                                grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_polylines(const char *x_key, double *x, int x_length, const char *y_key, double *y,
                                               int y_length, grm_context_t *ext_context, int line_type,
                                               double line_width, int line_colorind, grm_element_t *ext_element,
                                               grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_polyline(double x1, double x2, double y1, double y2, int line_type,
                                              double line_width, int line_colorind, grm_element_t *ext_element,
                                              grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_text(double x, double y, const char *text, grm_coordinate_space_t *space,
                                          grm_element_t *ext_element, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_fill_area(const char *x_key, double *x, int x_length, const char *y_key, double *y,
                                               int y_length, grm_context_t *ext_context, int fill_int_style,
                                               int fill_style, int fill_color_ind, grm_element_t *ext_element,
                                               grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_cell_array(double xmin, double xmax, double ymin, double ymax, int dimx, int dimy,
                                                int scol, int srow, int ncol, int nrow, const char *color_key,
                                                int *color, int color_length, grm_context_t *ext_context,
                                                grm_element_t *ext_element, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_non_uniform_polar_cell_array(
    double x_org, double y_org, const char *theta_key, double *theta, int theta_length, const char *r_key, double *r,
    int r_length, int dim_theta, int dim_r, int s_col, int s_row, int n_col, int n_row, const char *color_key,
    int *color, int color_length, grm_context_t *ext_context, grm_element_t *ext_element, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_polar_cell_array(double x_org, double y_org, double theta_min, double theta_max,
                                                      double r_min, double r_max, int dim_theta, int dim_r, int s_col,
                                                      int s_row, int n_col, int n_row, const char *color_key,
                                                      int *color, int color_length, grm_context_t *ext_context,
                                                      grm_element_t *ext_element, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_empty_axis(grm_element_t *ext_element, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_axis(double min_val, double max_val, double tick, double org, double pos,
                                          int major_count, int num_ticks, int num_tick_labels, double tick_size,
                                          int tick_orientation, double label_pos, grm_element_t *ext_element,
                                          grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_tick_group(int is_major, const char *tick_label, double value, double width,
                                                grm_element_t *ext_element, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_tick(int is_major, double value, grm_element_t *ext_element,
                                          grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_grid_line(int is_major, double value, grm_element_t *ext_element,
                                               grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_legend(const grm_element_t *ext_element, grm_context_t *ext_context,
                                            grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_pie_segment(const double start_angle, const double end_angle, const char *text,
                                                 const int color_index, grm_element_t *ext_element,
                                                 grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_bar(const double x1, const double x2, const double y1, const double y2,
                                         const int bar_color_index, const int edge_color_index,
                                         const char *bar_color_rgb, const char *edge_color_rgb, const double linewidth,
                                         const char *text, grm_element_t *xt_element, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_series(const char *name, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_draw_image(double xmin, double ymin, double xmax, double ymax, int width,
                                                int height, const char *data_key, int *data, int data_length, int model,
                                                grm_context_t *ext_context, grm_element_t *ext_element,
                                                grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_draw_arc(double xmin, double xmax, double ymin, double ymax, double start_angle,
                                              double end_angle, grm_element_t *ext_element, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_fill_arc(double xmin, double xmax, double ymin, double ymax, double a1, double a2,
                                              int fill_int_style, int fill_style, int fill_color_ind,
                                              grm_element_t *ext_element, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_draw_rect(double xmin, double xmax, double ymin, double ymax,
                                               grm_element_t *ext_element, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_fill_rect(double xmin, double xmax, double ymin, double ymax, int fill_int_style,
                                               int fill_style, int fill_color_ind, grm_element_t *ext_element,
                                               grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_quiver(const char *x_key, double *x, int x_length, const char *y_key, double *y,
                                            int y_length, const char *u_key, double *u, int u_length, const char *v_key,
                                            double *v, int v_length, int colored, grm_context_t *ext_context,
                                            grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_hexbin(const char *x_key, double *x, int x_length, const char *y_key, double *y,
                                            int y_length, grm_context_t *ext_context, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_colorbar(unsigned int num_color_values, grm_context_t *ext_context,
                                              grm_element_t *ext_element, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_non_uniform_cell_array(const char *x_key, double *x, int x_length,
                                                            const char *y_key, double *y, int y_length, int dimx,
                                                            int dimy, int scol, int srow, int ncol, int nrow,
                                                            const char *color_key, int *color, int color_length,
                                                            grm_context_t *ext_context, grm_element_t *ext_element,
                                                            grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_grid_3d(double x_tick, double y_tick, double z_tick, double x_org, double y_org,
                                             double z_org, int major_x, int major_y, int major_z,
                                             grm_element_t *ext_element, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_empty_grid_3d(int x_grid, int y_grid, int z_grid, grm_element_t *ext_element,
                                                   grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_axes_3d(double x_tick, double y_tick, double z_tick, double x_org, double y_org,
                                             double z_org, int major_x, int major_y, int major_z, int tick_orientation,
                                             grm_element_t *ext_element, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_empty_axes_3d(int tick_orientation, grm_element_t *ext_element,
                                                   grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_polyline_3d(const char *x_key, double *x, int x_length, const char *y_key,
                                                 double *y, int y_length, const char *z_key, double *z, int z_length,
                                                 grm_context_t *ext_context, grm_element_t *ext_element,
                                                 grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_polymarker_3d(const char *x_key, double *x, int x_length, const char *y_key,
                                                   double *y, int y_length, const char *z_key, double *z, int z_length,
                                                   grm_context_t *ext_context, grm_element_t *ext_element,
                                                   grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_draw_graphics(const char *data_key, int *data, int data_length,
                                                   grm_context_t *ext_context, grm_element_t *ext_element,
                                                   grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_tri_surface(const char *px_key, double *px, int px_length, const char *py_key,
                                                 double *py, int py_length, const char *pz_key, double *pz,
                                                 int pz_length, grm_context_t *ext_context, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_titles_3d(const char *x_label, const char *y_label, const char *z_label,
                                               grm_element_t *ext_element, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_layout_grid(grm_grid_t *grid, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_layout_grid_element(grm_element_t *grid_element, grm_slice_t *slice,
                                                         grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_panzoom(double x, double y, double xzoom, double yzoom, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_polar_bar(double count, int class_nr, grm_element_t *ext_element,
                                               grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_error_bar(double error_bar_x, double error_bar_y_min, double error_bar_y_max,
                                               int color_error_bar, grm_element_t *ext_element, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_integral(double int_lim_low, double int_lim_high, grm_element_t *ext_element,
                                              grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_side_region(const char *location, grm_element_t *ext_element,
                                                 grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_text_region(grm_element_t *ext_element, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_side_plot_region(grm_element_t *ext_element, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_radial_axes(grm_element_t *ext_element, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_theta_axes(grm_element_t *ext_element, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_angle_line(double x, double y, const char *angle_label, grm_element_t *ext_element,
                                                grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_arc_grid_line(double value, grm_element_t *ext_element, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_overlay(grm_element_t *ext_element, grm_creator_t *creator);
GRM_EXPORT grm_element_t *grm_create_overlay_element(double x, double y, const char *type, grm_element_t *ext_element,
                                                     grm_creator_t *creator);

/* ---------------------------------- get functions ------------------------------------------------------------------*/

GRM_EXPORT int grm_creator_get_axis_id(grm_creator_t *creator);

/* =============================== group mask ======================================================================= */

GRM_EXPORT grm_element_t *grm_get_document_root_c(void);

/* =============================== process attributes =============================================================== */

GRM_EXPORT void grm_process_limits(grm_element_t *element);
GRM_EXPORT void grm_process_window(grm_element_t *element);

/* =============================== render =========================================================================== */

GRM_EXPORT void grm_render_add_valid_context_key(const char *key);
GRM_EXPORT const grm_group_mask_t *grm_render_get_group_mask();
GRM_EXPORT void grm_render_caller();
GRM_EXPORT void grm_render_update_context_attribute(grm_element_t *element, const char *attr, grm_value_t *old_value);
GRM_EXPORT void grm_delete_context_attribute(grm_element_t *element);
GRM_EXPORT void grm_cleanup_element(grm_element_t *element);
GRM_EXPORT grm_render_t *grm_create_render();

/* ------------------------------- setter functions ----------------------------------------------------------------*/

GRM_EXPORT void grm_render_set_clip_region(grm_element_t *element, int region, grm_render_t *render);
GRM_EXPORT void grm_render_set_viewport(grm_element_t *element, double xmin, double xmax, double ymin, double ymax,
                                        grm_render_t *render);
GRM_EXPORT void grm_render_set_ws_viewport(grm_element_t *element, double xmin, double xmax, double ymin, double ymax,
                                           grm_render_t *render);
GRM_EXPORT void grm_render_set_window(grm_element_t *element, double xmin, double xmax, double ymin, double ymax,
                                      grm_render_t *render);
GRM_EXPORT void grm_render_set_ws_window(grm_element_t *element, double xmin, double xmax, double ymin, double ymax,
                                         grm_render_t *render);
GRM_EXPORT void grm_render_set_marker_type(grm_element_t *element, int type, grm_render_t *render);
GRM_EXPORT void grm_render_set_marker_types(grm_element_t *element, const char *types_key, int *types, int types_length,
                                            grm_context_t *ext_context, grm_render_t *render);
GRM_EXPORT void grm_render_set_marker_sizes(grm_element_t *element, const char *sizes_key, double *sizes,
                                            int sizes_length, grm_context_t *ext_context, grm_render_t *render);
GRM_EXPORT void grm_render_set_marker_size(grm_element_t *element, double size, grm_render_t *render);
GRM_EXPORT void grm_render_set_marker_color_inds(grm_element_t *element, const char *colorinds_key, int *colorinds,
                                                 int colorinds_length, grm_context_t *ext_context,
                                                 grm_render_t *render);
GRM_EXPORT void grm_render_set_marker_color_ind(const grm_element_t *element, int color, grm_render_t *render);
GRM_EXPORT void grm_render_set_line_types(grm_element_t *element, const char *types_key, int *types, int types_length,
                                          grm_context_t *ext_context, grm_render_t *render);
GRM_EXPORT void grm_render_set_line_type(grm_element_t *element, int type, grm_render_t *render);
GRM_EXPORT void grm_render_set_line_widths(grm_element_t *element, const char *widths_key, double *widths,
                                           int widths_length, grm_context_t *ext_context, grm_render_t *render);
GRM_EXPORT void grm_render_set_line_width(grm_element_t *element, double width, grm_render_t *render);
GRM_EXPORT void grm_render_set_line_color_ind(grm_element_t *element, const char *colorinds_key, int *colorinds,
                                              int colorinds_length, grm_context_t *ext_context, grm_render_t *render);
GRM_EXPORT void grm_render_set_line_color_inds(grm_element_t *element, int color, grm_render_t *render);
GRM_EXPORT void grm_render_set_char_up(grm_element_t *element, double ux, double uy, grm_render_t *render);
GRM_EXPORT void grm_render_set_text_align(grm_element_t *element, int horizontal, int vertical, grm_render_t *render);
GRM_EXPORT void grm_render_set_text_width_and_height(grm_element_t *element, double width, double height,
                                                     grm_render_t *render);
GRM_EXPORT void grm_render_set_color_rep(grm_element_t *element, int index, double red, double green, double blue,
                                         grm_render_t *render);
GRM_EXPORT void grm_render_set_line_spec(grm_element_t *element, const char *spec, grm_render_t *render);
GRM_EXPORT void grm_render_set_fill_int_style(grm_element_t *element, int index, grm_render_t *render);
GRM_EXPORT void grm_render_set_fill_color_ind(grm_element_t *element, int color, grm_render_t *render);
GRM_EXPORT void grm_render_set_fill_style(grm_element_t *element, int index, grm_render_t *render);
GRM_EXPORT void grm_render_set_scale(grm_element_t *element, int scale, grm_render_t *render);
GRM_EXPORT void grm_render_set_window_3d(grm_element_t *element, double xmin, double xmax, double ymin, double ymax,
                                         double zmin, double zmax, grm_render_t *render);
GRM_EXPORT void grm_render_set_space_3d(grm_element_t *element, double fov, double camera_distance,
                                        grm_render_t *render);
GRM_EXPORT void grm_render_set_space(grm_element_t *element, double zmin, double zmax, int rotation, int tilt,
                                     grm_render_t *render);
GRM_EXPORT void grm_render_set_select_specific_xform(grm_element_t *element, int transform, grm_render_t *render);
GRM_EXPORT void grm_render_set_text_color_ind(grm_element_t *element, int index, grm_render_t *render);
GRM_EXPORT void grm_render_set_border_color_ind(grm_element_t *element, int index, grm_render_t *render);
GRM_EXPORT void grm_render_set_border_width(grm_element_t *element, double width, grm_render_t *render);
GRM_EXPORT void grm_render_set_char_height(grm_element_t *element, double height, grm_render_t *render);
GRM_EXPORT void grm_render_set_transparency(grm_element_t *element, double transparency, grm_render_t *render);
GRM_EXPORT void grm_render_set_resample_method(grm_element_t *element, int resample, grm_render_t *render);
GRM_EXPORT void grm_render_set_text_encoding(grm_element_t *element, int encoding, grm_render_t *render);
GRM_EXPORT void grm_render_set_viewport_normalized(grm_element_t *element, double xmin, double xmax, double ymin,
                                                   double ymax, grm_render_t *render);
GRM_EXPORT void grm_render_set_origin_position(grm_element_t *element, const char *x_org_pos, const char *y_org_pos,
                                               grm_render_t *render);
GRM_EXPORT void grm_render_set_origin_position_3d(grm_element_t *element, const char *x_org_pos, const char *y_org_pos,
                                                  const char *z_org_pos, grm_render_t *render);
GRM_EXPORT void grm_render_set_gr3_light_parameters(grm_element_t *element, double ambient, double diffuse,
                                                    double specular, double specular_power, grm_render_t *render);
GRM_EXPORT void grm_render_set_auto_update(int update, grm_render_t *render);
GRM_EXPORT void grm_render_set_enable_editor(int update, grm_render_t *render);
GRM_EXPORT void grm_render_set_active_figure(grm_element_t *element, grm_render_t *render);
GRM_EXPORT void grm_render_set_first_call(int call, grm_render_t *render);
GRM_EXPORT void grm_render_set_previous_scatter_marker_type(int *previous_scatter_marker_type, grm_render_t *render);
GRM_EXPORT void grm_render_set_previous_line_marker_type(int *previous_line_marker_type, grm_render_t *render);

/* ------------------------------- getter functions ----------------------------------------------------------------*/

GRM_EXPORT grm_element_t *grm_render_get_active_figure(grm_render_t *render);
GRM_EXPORT void grm_render_get_auto_update(int *update, grm_render_t *render);
GRM_EXPORT void grm_render_get_enable_editor(int *update, grm_render_t *render);
GRM_EXPORT int grm_render_get_first_call(grm_render_t *render);
GRM_EXPORT int grm_render_get_z_queue_is_being_rendered(grm_render_t *render);
GRM_EXPORT grm_manage_z_index_t *grm_render_get_z_index_manager(grm_render_t *render);
GRM_EXPORT int grm_render_get_redraw_ws(grm_render_t *render);
GRM_EXPORT int grm_render_get_highlighted_attr_exist(grm_render_t *render);
GRM_EXPORT int *grm_render_get_previous_scatter_marker_type(grm_render_t *render);
GRM_EXPORT int *grm_render_get_previous_line_marker_type(grm_render_t *render);
GRM_EXPORT int grm_render_get_viewport(grm_element_t *element, double *xmin, double *xmax, double *ymin, double *ymax,
                                       grm_render_t *render);
GRM_EXPORT grm_context_t *grm_render_get_render_context(grm_render_t *render);
GRM_EXPORT void grm_render_c(grm_render_t *render);                                   // render doc and render context
GRM_EXPORT void grm_render_context(grm_context_t *ext_context, grm_render_t *render); // render doc and external context
GRM_EXPORT void grm_render_document(grm_document_t *document, grm_render_t *render);  // external doc and render context
GRM_EXPORT void grm_render_both(grm_document_t *document, grm_context_t *ext_context,
                                grm_render_t *render); // external doc and external context; could be static
GRM_EXPORT void grm_render_process_tree(grm_render_t *render);
GRM_EXPORT void grm_render_mask_highlight(grm_element_t *highlighted_element,
                                          void (*image_callback)(int, unsigned int, unsigned int, unsigned int,
                                                                 unsigned int, unsigned int *),
                                          grm_render_t *render);
GRM_EXPORT void grm_render_finalize(grm_render_t *render);
GRM_EXPORT grm_context_t *grm_render_get_context(grm_render_t *render);
GRM_EXPORT const char **grm_render_get_default_and_tooltip(grm_element_t *element, const char *attribute_name,
                                                           grm_render_t *render);

/* ----------------------- history -------------------------------------------------------------------------------- */

GRM_EXPORT const char *grm_render_initialize_history(grm_render_t *render);

/* =============================== render util ====================================================================== */

GRM_EXPORT void grm_render_get_figure_size(int *pixel_width, int *pixel_height, double *metric_width,
                                           double *metric_height);
GRM_EXPORT void grm_render_calculate_char_height(grm_element_t *element);

/* =============================== updater ========================================================================== */

GRM_EXPORT void grm_update_filter(grm_element_t *element, const char *attr, const char *value);

#ifdef __cplusplus
}
#endif
#endif // GR_DOM_RENDER_H
