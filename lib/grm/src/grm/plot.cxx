/* ######################### includes ############################################################################### */

#include <grm/dom_render/graphics_tree/Element.hxx>
#include <grm/dom_render/graphics_tree/Comment.hxx>
#include <grm/dom_render/graphics_tree/util.hxx>
#include <grm/dom_render/render.hxx>

#include <algorithm>
#include <string>
#include <set>

extern "C" {
#include <grm/layout.h>
#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>

#include "base64_int.h"
#include <grm/dump.h>
#include "backtrace_int.h"
#include "event_int.h"
#include "gks.h"
#include "gr.h"
#include "gr3.h"
#include "logging_int.h"
}

#ifndef NO_LIBXML2
#include <libxml/globals.h>
#include <libxml/xmlreader.h>
#elif !defined(NO_EXPAT)
#include <expat.h>
#endif

#include "plot_int.h"
#include "grm/layout.hxx"
#include "utilcpp_int.hxx"

extern "C" {


#include "datatype/double_map_int.h"
#include "datatype/string_array_map_int.h"
#include "datatype/template/map_int.h"
#include "datatype/template/set_int.h"


/* ######################### private interface ###################################################################### */

/* ========================= constants ============================================================================== */

static const std::string SCHEMA_REL_FILEPATH = "share/xml/GRM/grm_graphics_tree_schema.xsd";
static const std::string ENABLE_XML_VALIDATION_ENV_KEY = "GRM_VALIDATE";

/* ========================= datatypes ============================================================================== */

/* ------------------------- args set ------------------------------------------------------------------------------- */

DECLARE_SET_TYPE(args, grm_args_t *)


/* ------------------------- string-to-plot_func map ---------------------------------------------------------------- */

DECLARE_MAP_TYPE(plot_func, plot_func_t)


/* ------------------------- string-to-args_set map ----------------------------------------------------------------- */

DECLARE_MAP_TYPE(args_set, args_set_t *)


#undef DECLARE_SET_TYPE
#undef DECLARE_MAP_TYPE


/* ========================= macros ================================================================================= */

#ifdef isnan
#define is_nan(a) isnan(a)
#else
#define is_nan(x) ((x) != (x))
#endif

/* ------------------------- math ----------------------------------------------------------------------------------- */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef isfinite
#define isfinite(x) ((x) - (x) == (x) - (x))
#endif

/* ------------------------- args get ------------------------------------------------------------------------------- */

#define ARGS_VALUE_ITERATOR_GET(value_it, length, array) \
  if ((value_it)->next(value_it) == nullptr)             \
    {                                                    \
      args_value_iterator_delete(value_it);              \
      return ERROR_INTERNAL;                             \
    }                                                    \
  (length) = (value_it)->array_length;                   \
  (array) = *(double **)(value_it)->value_ptr;

/* ========================= methods ================================================================================ */

/* ------------------------- args set ------------------------------------------------------------------------------- */

DECLARE_SET_METHODS(args)


/* ------------------------- string-to-plot_func map ---------------------------------------------------------------- */

DECLARE_MAP_METHODS(plot_func)


/* ------------------------- string-to-args_set map ----------------------------------------------------------------- */

DECLARE_MAP_METHODS(args_set)


#undef DECLARE_SET_METHODS
#undef DECLARE_MAP_METHODS


/* ######################### internal implementation ################################################################ */

/* ========================= static variables ======================================================================= */

/* ------------------------- plot ----------------------------------------------------------------------------------- */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ general ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static int plot_static_variables_initialized = 0;
const char *plot_hierarchy_names[] = {"figure", "plots", "subplots", "series", nullptr};


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ args ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static grm_args_t *global_root_args = nullptr;
grm_args_t *active_plot_args = nullptr;
static unsigned int active_plot_index = 0;
static int last_merge_plot_id = 0;
static int last_merge_subplot_id = 0;
static int last_merge_series_id = 0;
static bool args_changed_since_last_plot = false;
grid_t *global_grid = nullptr;
static std::shared_ptr<GRM::Render> global_render;
static std::shared_ptr<GRM::Element> global_root;
static std::shared_ptr<GRM::Element> edit_figure;
static std::shared_ptr<GRM::Element> current_dom_element;
static std::shared_ptr<GRM::Element> current_central_region_element;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ event handling ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

event_queue_t *event_queue = nullptr;


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ kind to fmt ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static string_map_entry_t kind_to_fmt[] = {{"line", "xys"},           {"hexbin", "xys"},
                                           {"polar", "xys"},          {"shade", "xys"},
                                           {"stem", "xys"},           {"stairs", "xys"},
                                           {"contour", "xyzc"},       {"contourf", "xyzc"},
                                           {"tricontour", "xyzc"},    {"trisurface", "xyzc"},
                                           {"surface", "xyzc"},       {"wireframe", "xyzc"},
                                           {"plot3", "xyzc"},         {"scatter", "xyzc"},
                                           {"scatter3", "xyzc"},      {"quiver", "xyuv"},
                                           {"heatmap", "xyzc"},       {"hist", "x"},
                                           {"barplot", "y"},          {"isosurface", "c"},
                                           {"imshow", "c"},           {"nonuniformheatmap", "xyzc"},
                                           {"polar_histogram", "x"},  {"pie", "x"},
                                           {"volume", "c"},           {"marginal_heatmap", "xyzc"},
                                           {"polar_heatmap", "xyzc"}, {"nonuniformpolar_heatmap", "xyzc"}};


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ kind to func ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static plot_func_map_entry_t kind_to_func[] = {{"line", plot_line},
                                               {"stairs", plot_stairs},
                                               {"scatter", plot_scatter},
                                               {"quiver", plot_quiver},
                                               {"stem", plot_stem},
                                               {"hist", plot_hist},
                                               {"barplot", plot_barplot},
                                               {"contour", plot_contour},
                                               {"contourf", plot_contourf},
                                               {"hexbin", plot_hexbin},
                                               {"heatmap", plot_heatmap},
                                               {"marginal_heatmap", plot_marginal_heatmap},
                                               {"wireframe", plot_wireframe},
                                               {"surface", plot_surface},
                                               {"plot3", plot_plot3},
                                               {"scatter3", plot_scatter3},
                                               {"imshow", plot_imshow},
                                               {"isosurface", plot_isosurface},
                                               {"polar", plot_polar},
                                               {"trisurface", plot_trisurface},
                                               {"tricontour", plot_tricontour},
                                               {"shade", plot_shade},
                                               {"nonuniformheatmap", plot_heatmap},
                                               {"nonuniformpolar_heatmap", plot_polar_heatmap},
                                               {"polar_histogram", plot_polar_histogram},
                                               {"polar_heatmap", plot_polar_heatmap},
                                               {"pie", plot_pie},
                                               {"volume", plot_volume}};


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ maps ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static double_map_t *meters_per_unit_map = nullptr;
static string_map_t *fmt_map = nullptr;
static plot_func_map_t *plot_func_map = nullptr;
static string_map_t *plot_valid_keys_map = nullptr;
static string_array_map_t *type_map = nullptr;


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plot clear ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

const char *plot_clear_exclude_keys[] = {"array_index", "in_use", nullptr};


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plot merge ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

const char *plot_merge_ignore_keys[] = {"id", "series_id", "subplot_id", "plot_id", "array_index", "in_use", nullptr};
const char *plot_merge_clear_keys[] = {"series", nullptr};


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ kind to func ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static const double_map_entry_t symbol_to_meters_per_unit[] = {
    {"m", 1.0},     {"dm", 0.1},    {"cm", 0.01},  {"mm", 0.001},        {"in", 0.0254},
    {"\"", 0.0254}, {"ft", 0.3048}, {"'", 0.0254}, {"pc", 0.0254 / 6.0}, {"pt", 0.0254 / 72.0},
};


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ text encoding ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static int pre_plot_text_encoding = -1;


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ valid keys ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* IMPORTANT: Every key should only be part of ONE array -> keys can be assigned to the right object, if a user sends a
 * flat object that mixes keys of different hierarchies */

const char *valid_root_keys[] = {"plots", "append_plots", "hold_plots", nullptr};
const char *valid_plot_keys[] = {"clear", "fig_size", "raw", "size", "subplots", "update", nullptr};

const char *valid_subplot_keys[] = {"abs_height",
                                    "abs_width",
                                    "accelerate",
                                    "adjust_x_lim",
                                    "adjust_y_lim",
                                    "adjust_z_lim",
                                    "alpha",
                                    "angle_ticks",
                                    "aspect_ratio",
                                    "background_color",
                                    "bar_color",
                                    "bar_width",
                                    "col",
                                    "col_span",
                                    "colormap",
                                    "fit_parents_height",
                                    "fit_parents_width",
                                    "font",
                                    "font_precision",
                                    "grid_element",
                                    "grplot",
                                    "marginal_heatmap_kind",
                                    "ind_bar_color",
                                    "ind_edge_color",
                                    "ind_edge_width",
                                    "keep_aspect_ratio",
                                    "keep_radii_axes",
                                    "kind",
                                    "labels",
                                    "levels",
                                    "location",
                                    "major_h",
                                    "normalization",
                                    "only_quadratic_aspect_ratio",
                                    "orientation",
                                    "panzoom",
                                    "phi_flip",
                                    "rel_height",
                                    "rel_width",
                                    "resample_method",
                                    "reset_ranges",
                                    "rings",
                                    "rotation",
                                    "row",
                                    "row_span",
                                    "series",
                                    "style",
                                    "subplot",
                                    "tilt",
                                    "title",
                                    "transformation",
                                    "x_bins",
                                    "x_flip",
                                    "x_grid",
                                    "x_label",
                                    "x_lim",
                                    "x_log",
                                    "x_ind",
                                    "x_tick_labels",
                                    "y_bins",
                                    "y_flip",
                                    "y_grid",
                                    "y_label",
                                    "y_lim",
                                    "y_log",
                                    "y_ind",
                                    "y_tick_labels",
                                    "z_flip",
                                    "z_grid",
                                    "z_label",
                                    "z_lim",
                                    "z_log",
                                    nullptr};
const char *valid_series_keys[] = {"a",
                                   "algorithm",
                                   "bin_width",
                                   "bin_edges",
                                   "bin_counts",
                                   "c",
                                   "c_dims",
                                   "c_range",
                                   "clip_negative",
                                   "draw_edges",
                                   "d_min",
                                   "d_max",
                                   "edge_color",
                                   "edge_width",
                                   "error",
                                   "face_color",
                                   "foreground_color",
                                   "indices",
                                   "inner_series",
                                   "int_limits_high",
                                   "int_limits_low",
                                   "isovalue",
                                   "line_spec",
                                   "marker_type",
                                   "num_bins",
                                   "phi_lim",
                                   "rgb",
                                   "r_lim",
                                   "s",
                                   "step_where",
                                   "stairs",
                                   "u",
                                   "v",
                                   "weights",
                                   "x",
                                   "x_colormap",
                                   "x_range",
                                   "y",
                                   "y_colormap",
                                   "y_labels",
                                   "y_range",
                                   "z",
                                   "z_dims",
                                   "z_range",
                                   nullptr};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ valid types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* If multiple formats are supported use `|` as separator
 * Example: "i|s" for supporting both integer and strings */
/* TODO: type for format "s"? */
static string_map_entry_t key_to_formats[] = {{"a", "A"},
                                              {"abs_height", "d"},
                                              {"abs_width", "d"},
                                              {"accelerate", "i"},
                                              {"algorithm", "i|s"},
                                              {"adjust_x_lim", "i"},
                                              {"adjust_y_lim", "i"},
                                              {"adjust_z_lim", "i"},
                                              {"alpha", "d"},
                                              {"aspect_ratio", "d"},
                                              {"append_plots", "i"},
                                              {"background_color", "i"},
                                              {"bar_color", "D|i"},
                                              {"c", "D|I"},
                                              {"c_dims", "I"},
                                              {"c_range", "D"},
                                              {"col", "i|I"},
                                              {"col_span", "i|I"},
                                              {"colormap", "i"},
                                              {"d_min", "d"},
                                              {"d_max", "d"},
                                              {"edge_color", "D|i"},
                                              {"edge_width", "d"},
                                              {"error", "a"},
                                              {"fig_size", "D"},
                                              {"fit_parents_height", "i"},
                                              {"fit_parents_width", "i"},
                                              {"font", "i"},
                                              {"font_precision", "i"},
                                              {"foreground_color", "D"},
                                              {"grid_element", "s"},
                                              {"grplot", "i"},
                                              {"hold_plots", "i"},
                                              {"ind_bar_color", "A"},
                                              {"ind_edge_color", "A"},
                                              {"ind_edge_width", "A"},
                                              {"int_limits_high", "D"},
                                              {"int_limits_low", "D"},
                                              {"isovalue", "d"},
                                              {"keep_aspect_ratio", "i"},
                                              {"kind", "s"},
                                              {"labels", "S"},
                                              {"levels", "i"},
                                              {"line_spec", "s"},
                                              {"location", "i"},
                                              {"marginal_heatmap_kind", "s"},
                                              {"marker_type", "i|D"},
                                              {"num_bins", "i"},
                                              {"only_quadratic_aspect_ratio", "i"},
                                              {"orientation", "s"},
                                              {"panzoom", "D"},
                                              {"raw", "s"},
                                              {"rel_height", "d"},
                                              {"rel_width", "d"},
                                              {"resample_method", "s|i"},
                                              {"reset_ranges", "i"},
                                              {"rotation", "d"},
                                              {"row", "i|I"},
                                              {"row_span", "i|I"},
                                              {"size", "D|I|A"},
                                              {"step_where", "s"},
                                              {"style", "s"},
                                              {"subplot", "D"},
                                              {"tilt", "d"},
                                              {"title", "s"},
                                              {"transformation", "i"},
                                              {"u", "D"},
                                              {"update", "i"},
                                              {"v", "D"},
                                              {"x", "D|I"},
                                              {"x_bins", "i"},
                                              {"x_colormap", "i"},
                                              {"x_flip", "i"},
                                              {"x_grid", "i"},
                                              {"x_label", "s"},
                                              {"x_lim", "D"},
                                              {"x_ind", "i"},
                                              {"x_range", "D"},
                                              {"x_log", "i"},
                                              {"y", "D"},
                                              {"y_bins", "i"},
                                              {"y_colormap", "i"},
                                              {"y_flip", "i"},
                                              {"y_form", "i"},
                                              {"y_grid", "i"},
                                              {"y_label", "s"},
                                              {"y_lim", "D"},
                                              {"y_ind", "i"},
                                              {"y_range", "D"},
                                              {"y_log", "i"},
                                              {"z", "D"},
                                              {"z_dims", "I"},
                                              {"z_flip", "i"},
                                              {"z_grid", "i"},
                                              {"z_label", "s"},
                                              {"z_lim", "D"},
                                              {"z_range", "D"},
                                              {"z_log", "i"}};

/* ========================= functions ============================================================================== */

/* ------------------------- plot ----------------------------------------------------------------------------------- */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ general ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

err_t plot_init_static_variables(void)
{
  err_t error = ERROR_NONE;

  if (!plot_static_variables_initialized)
    {
      logger((stderr, "Initializing static plot variables\n"));
      event_queue = event_queue_new();
      global_root_args = grm_args_new();
      error_cleanup_and_set_error_if(global_root_args == nullptr, ERROR_MALLOC);
      error = plot_init_args_structure(global_root_args, plot_hierarchy_names, 1);
      error_cleanup_if_error;
      plot_set_flag_defaults();
      error_cleanup_and_set_error_if(!grm_args_values(global_root_args, "plots", "a", &active_plot_args),
                                     ERROR_INTERNAL);
      active_plot_index = 1;
      /* initialize global_render and its root */
      global_render = GRM::Render::createRender();
      global_root = global_render->createElement("root");
      global_render->replaceChildren(global_root);
      global_root->setAttribute("_id", 0);
      global_render->setAutoUpdate(false);


      meters_per_unit_map = double_map_new_with_data(array_size(symbol_to_meters_per_unit), symbol_to_meters_per_unit);
      error_cleanup_and_set_error_if(meters_per_unit_map == nullptr, ERROR_MALLOC);
      fmt_map = string_map_new_with_data(array_size(kind_to_fmt), kind_to_fmt);
      error_cleanup_and_set_error_if(fmt_map == nullptr, ERROR_MALLOC);
      plot_func_map = plot_func_map_new_with_data(array_size(kind_to_func), kind_to_func);
      error_cleanup_and_set_error_if(plot_func_map == nullptr, ERROR_MALLOC);
      {
        const char **hierarchy_keys[] = {valid_root_keys, valid_plot_keys, valid_subplot_keys, valid_series_keys,
                                         nullptr};
        const char **hierarchy_names_ptr, ***hierarchy_keys_ptr, **current_key_ptr;
        plot_valid_keys_map = string_map_new(array_size(valid_root_keys) + array_size(valid_plot_keys) +
                                             array_size(valid_subplot_keys) + array_size(valid_series_keys));
        error_cleanup_and_set_error_if(plot_valid_keys_map == nullptr, ERROR_MALLOC);
        hierarchy_keys_ptr = hierarchy_keys;
        hierarchy_names_ptr = plot_hierarchy_names;
        while (*hierarchy_names_ptr != nullptr && *hierarchy_keys_ptr != nullptr)
          {
            current_key_ptr = *hierarchy_keys_ptr;
            while (*current_key_ptr != nullptr)
              {
                string_map_insert(plot_valid_keys_map, *current_key_ptr, *hierarchy_names_ptr);
                ++current_key_ptr;
              }
            ++hierarchy_names_ptr;
            ++hierarchy_keys_ptr;
          }
      }
      type_map = string_array_map_new_from_string_split(array_size(key_to_formats), key_to_formats, '|');
      error_cleanup_and_set_error_if(type_map == nullptr, ERROR_MALLOC);
      install_backtrace_handler_if_enabled();
      plot_static_variables_initialized = 1;
    }
  return ERROR_NONE;

error_cleanup:
  if (global_root_args != nullptr)
    {
      grm_args_delete(global_root_args);
      global_root_args = nullptr;
    }
  if (meters_per_unit_map != nullptr)
    {
      double_map_delete(meters_per_unit_map);
      meters_per_unit_map = nullptr;
    }
  if (fmt_map != nullptr)
    {
      string_map_delete(fmt_map);
      fmt_map = nullptr;
    }
  if (plot_func_map != nullptr)
    {
      plot_func_map_delete(plot_func_map);
      plot_func_map = nullptr;
    }
  if (plot_valid_keys_map != nullptr)
    {
      string_map_delete(plot_valid_keys_map);
      plot_valid_keys_map = nullptr;
    }
  if (type_map != nullptr)
    {
      string_array_map_delete(type_map);
      type_map = nullptr;
    }
  return error;
}

static std::shared_ptr<GRM::Element> getCentralRegion()
{
  auto plot_parent = edit_figure->lastChildElement();
  for (const auto &child : plot_parent->children())
    {
      if (child->localName() == "central_region")
        {
          plot_parent = child;
          break;
        }
      else if (child->localName() == "marginal_heatmap_plot")
        {
          for (const auto &childchild : child->children())
            {
              if (childchild->localName() == "central_region")
                {
                  plot_parent = childchild;
                  break;
                }
            }
        }
    }
  return plot_parent;
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plot arguments ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

err_t plot_merge_args(grm_args_t *args, const grm_args_t *merge_args, const char **hierarchy_name_ptr,
                      uint_map_t *hierarchy_to_id, int hold_always)
{
  static args_set_map_t *key_to_cleared_args = nullptr;
  static int recursion_level = -1;
  int plot_id, subplot_id, series_id;
  int append_plots;
  grm_args_iterator_t *merge_it = nullptr;
  arg_t *arg, *merge_arg;
  grm_args_value_iterator_t *value_it = nullptr, *merge_value_it = nullptr;
  const char **current_hierarchy_name_ptr;
  grm_args_t **args_array, **merge_args_array, *current_args;
  unsigned int i;
  err_t error = ERROR_NONE;

  ++recursion_level;
  if (hierarchy_name_ptr == nullptr)
    {
      hierarchy_name_ptr = plot_hierarchy_names;
    }
  if (hierarchy_to_id == nullptr)
    {
      hierarchy_to_id = uint_map_new(array_size(plot_hierarchy_names));
      cleanup_and_set_error_if(hierarchy_to_id == nullptr, ERROR_MALLOC);
    }
  else
    {
      hierarchy_to_id = uint_map_copy(hierarchy_to_id);
      cleanup_and_set_error_if(hierarchy_to_id == nullptr, ERROR_MALLOC);
    }
  if (key_to_cleared_args == nullptr)
    {
      key_to_cleared_args = args_set_map_new(array_size(plot_merge_clear_keys));
      cleanup_and_set_error_if(hierarchy_to_id == nullptr, ERROR_MALLOC);
    }
  if (!(recursion_level == 0 && grm_args_values(merge_args, "append_plots", "i", &append_plots)))
    {
      grm_args_values(global_root_args, "append_plots", "i", &append_plots);
    }
  get_id_from_args(merge_args, &plot_id, &subplot_id, &series_id);
  if (plot_id > 0)
    {
      uint_map_insert(hierarchy_to_id, "plots", plot_id);
    }
  else
    {
      uint_map_insert_default(hierarchy_to_id, "plots", append_plots ? 0 : active_plot_index);
      uint_map_at(hierarchy_to_id, "plots", (unsigned int *)&plot_id);
      logger((stderr, "Using plot_id \"%u\"\n", plot_id));
    }
  if (subplot_id > 0)
    {
      uint_map_insert(hierarchy_to_id, "subplots", subplot_id);
    }
  else
    {
      uint_map_insert_default(hierarchy_to_id, "subplots", 1);
      uint_map_at(hierarchy_to_id, "subplots", (unsigned int *)&subplot_id);
    }
  if (series_id > 0)
    {
      uint_map_insert(hierarchy_to_id, "series", series_id);
    }
  else
    {
      uint_map_insert_default(hierarchy_to_id, "series", 1);
      uint_map_at(hierarchy_to_id, "series", (unsigned int *)&series_id);
    }
  /* special case: clear the plot container before usage if
   * - it is the first call of `plot_merge_args` AND `hold_always` is `0` AND
   *   - `plot_id` is `1` and `hold_plots` is not set OR
   *   - `hold_plots` is true and no plot will be appended (`plot_id` > 0)
   */
  if (strcmp(*hierarchy_name_ptr, "figure") == 0 && plot_id > 0 && !hold_always)
    {
      int hold_plots_key_available, hold_plots;
      hold_plots_key_available = grm_args_values(args, "hold_plots", "i", &hold_plots);
      if (hold_plots_key_available)
        {
          logger((stderr, "Do%s hold plots\n", hold_plots ? "" : " not"));
        }
      if ((hold_plots_key_available && !hold_plots) || (!hold_plots_key_available && plot_id == 1))
        {
          cleanup_and_set_error_if(!grm_args_values(args, "plots", "A", &args_array), ERROR_INTERNAL);
          current_args = args_array[plot_id - 1];
          grm_args_clear(current_args);
          error = plot_init_args_structure(current_args, hierarchy_name_ptr + 1, 1);
          cleanup_if_error;
          logger((stderr, "Cleared current args\n"));
        }
      else
        {
          logger((stderr, "Held current args\n"));
        }
    }
#ifndef NDEBUG
  if (strcmp(*hierarchy_name_ptr, "figure") == 0 && hold_always)
    {
      logger((stderr, "\"hold_always\" is set\n"));
    }
#endif /* ifndef  */
  merge_it = grm_args_iter(merge_args);
  cleanup_and_set_error_if(merge_it == nullptr, ERROR_MALLOC);
  while ((merge_arg = merge_it->next(merge_it)) != nullptr)
    {
      if (str_equals_any_in_array(merge_arg->key, plot_merge_ignore_keys))
        {
          continue;
        }
      /* First, find the correct hierarchy level where the current merge value belongs to. */
      error = plot_get_args_in_hierarchy(args, hierarchy_name_ptr, merge_arg->key, hierarchy_to_id,
                                         (const grm_args_t **)&current_args, &current_hierarchy_name_ptr);
      if (error == ERROR_PLOT_UNKNOWN_KEY)
        {
          logger((stderr, "WARNING: The key \"%s\" is not assigned to any hierarchy level.\n", merge_arg->key));
        }
      cleanup_if_error;
      if (str_equals_any_in_array(*current_hierarchy_name_ptr, plot_merge_clear_keys))
        {
          int clear_args = 1;
          args_set_t *cleared_args = nullptr;
          if (args_set_map_at(key_to_cleared_args, *current_hierarchy_name_ptr, &cleared_args))
            {
              clear_args = !args_set_contains(cleared_args, current_args);
            }
          if (clear_args)
            {
              logger((stderr, "Perform a clear on the current args container\n"));
              grm_args_clear(current_args);
              if (cleared_args == nullptr)
                {
                  cleared_args = args_set_new(10); /* FIXME: do not use a magic number, use a growable set instead! */
                  cleanup_and_set_error_if(cleared_args == nullptr, ERROR_MALLOC);
                  cleanup_and_set_error_if(
                      !args_set_map_insert(key_to_cleared_args, *current_hierarchy_name_ptr, cleared_args),
                      ERROR_INTERNAL);
                }
              logger((stderr, "Add args container \"%p\" to cleared args with key \"%s\"\n", (void *)current_args,
                      *current_hierarchy_name_ptr));
              cleanup_and_set_error_if(!args_set_add(cleared_args, current_args), ERROR_INTERNAL);
            }
        }
      /* If the current key is a hierarchy key, perform a merge. Otherwise, (else branch) put the value in without a
       * merge.
       */
      if (current_hierarchy_name_ptr[1] != nullptr && strcmp(merge_arg->key, current_hierarchy_name_ptr[1]) == 0)
        {
          /* `args_at` cannot fail in this case because the `args` object was initialized with an empty structure
           * before. If `arg` is nullptr, an internal error occurred. */
          arg = args_at(current_args, merge_arg->key);
          cleanup_and_set_error_if(arg == nullptr, ERROR_INTERNAL);
          value_it = grm_arg_value_iter(arg);
          merge_value_it = grm_arg_value_iter(merge_arg);
          cleanup_and_set_error_if(value_it == nullptr, ERROR_MALLOC);
          cleanup_and_set_error_if(merge_value_it == nullptr, ERROR_MALLOC);
          /* Do not support two-dimensional argument arrays like `nAnA`) -> a loop would be needed with more memory
           * management */
          cleanup_and_set_error_if(value_it->next(value_it) == nullptr, ERROR_MALLOC);
          cleanup_and_set_error_if(merge_value_it->next(merge_value_it) == nullptr, ERROR_MALLOC);
          /* Increase the array size of the internal args array if necessary */
          if (merge_value_it->array_length > value_it->array_length)
            {
              error = plot_init_arg_structure(arg, current_hierarchy_name_ptr, merge_value_it->array_length);
              cleanup_if_error;
              args_value_iterator_delete(value_it);
              value_it = grm_arg_value_iter(arg);
              cleanup_and_set_error_if(value_it == nullptr, ERROR_MALLOC);
              cleanup_and_set_error_if(value_it->next(value_it) == nullptr, ERROR_MALLOC);
              args_array = *(grm_args_t ***)value_it->value_ptr;
            }
          else
            {
              /* The internal args container stores always an array for hierarchy levels */
              args_array = *(grm_args_t ***)value_it->value_ptr;
            }
          if (merge_value_it->is_array)
            {
              merge_args_array = *(grm_args_t ***)merge_value_it->value_ptr;
            }
          else
            {
              merge_args_array = (grm_args_t **)merge_value_it->value_ptr;
            }
          /* Merge array entries pairwise */
          for (i = 0; i < merge_value_it->array_length; ++i)
            {
              logger((stderr, "Perform a recursive merge on key \"%s\", array index \"%d\"\n", merge_arg->key, i));
              error = plot_merge_args(args_array[i], merge_args_array[i], current_hierarchy_name_ptr + 1,
                                      hierarchy_to_id, hold_always);
              cleanup_if_error;
            }
        }
      else
        {
          const char *compatible_format;
          /* Only accept the new value, if it has a compatible type. Convert if possible */
          if ((compatible_format = get_compatible_format(merge_arg->key, merge_arg->value_format)) != nullptr)
            {
              logger((stderr, "Perform a replace on key \"%s\"\n", merge_arg->key));
              if (strlen(merge_arg->value_format) == 1 &&
                  tolower(*merge_arg->value_format) == *merge_arg->value_format &&
                  toupper(*compatible_format) == *compatible_format)
                {
                  /* A single value is given but an array is needed
                   * -> Push the given value as array instead
                   * **IMPORTANT**: In this case the existing `merge_arg` node cannot be shared with `current_args`
                   * (since the stored format does not fit), so the value must be copied (it is owned by the `merge_arg`
                   * node and would be freed twice otherwise). */
                  char array_format[3] = "n";
                  void *copy_buffer;
                  array_format[1] = *compatible_format;
                  logger((stderr, "Convert the given format \"%s\" to an array format \"%s\" \n",
                          merge_arg->value_format, array_format));
                  copy_buffer = copy_value(tolower(*compatible_format), merge_arg->value_ptr);
                  cleanup_and_set_error_if(copy_buffer == nullptr, ERROR_MALLOC);
                  grm_args_push(current_args, merge_arg->key, array_format, 1, copy_buffer);
                  /* x -> copy_buffer -> value, value is now stored and the buffer is not needed any more */
                  free(copy_buffer);
                }
              else
                {
                  /* Otherwise, push without conversion (needed conversion is done later when extracting values with
                   * functions like `grm_args_values`) */
                  error = args_push_arg(current_args, merge_arg);
                  cleanup_if_error;
                }
            }
          else
            {
              logger((stderr, "The type \"%s\" of key \"%s\" was rejected and will not be merged.\n",
                      merge_arg->value_format, merge_arg->key));
            }
        }
    }

cleanup:
  if (recursion_level == 0)
    {
      if (key_to_cleared_args != nullptr)
        {
          args_set_t *cleared_args = nullptr;
          const char **current_key_ptr = plot_merge_clear_keys;
          while (*current_key_ptr != nullptr)
            {
              if (args_set_map_at(key_to_cleared_args, *current_key_ptr, &cleared_args))
                {
                  args_set_delete(cleared_args);
                }
              ++current_key_ptr;
            }
          args_set_map_delete(key_to_cleared_args);
          key_to_cleared_args = nullptr;
        }
    }
  if (hierarchy_to_id != nullptr)
    {
      uint_map_delete(hierarchy_to_id);
      hierarchy_to_id = nullptr;
    }
  if (merge_it != nullptr)
    {
      args_iterator_delete(merge_it);
    }
  if (value_it != nullptr)
    {
      args_value_iterator_delete(value_it);
    }
  if (merge_value_it != nullptr)
    {
      args_value_iterator_delete(merge_value_it);
    }

  --recursion_level;

  return error;
}

err_t plot_init_arg_structure(arg_t *arg, const char **hierarchy_name_ptr, unsigned int next_hierarchy_level_max_id)
{
  grm_args_t **args_array = nullptr;
  unsigned int args_old_array_length;
  unsigned int i;
  err_t error = ERROR_NONE;

  logger((stderr, "Init plot args structure for hierarchy: \"%s\"\n", *hierarchy_name_ptr));

  ++hierarchy_name_ptr;
  if (*hierarchy_name_ptr == nullptr)
    {
      return ERROR_NONE;
    }
  arg_first_value(arg, "A", nullptr, &args_old_array_length);
  if (next_hierarchy_level_max_id <= args_old_array_length)
    {
      return ERROR_NONE;
    }
  logger((stderr, "Increase array for key \"%s\" from %d to %d\n", *hierarchy_name_ptr, args_old_array_length,
          next_hierarchy_level_max_id));
  error = arg_increase_array(arg, next_hierarchy_level_max_id - args_old_array_length);
  return_if_error;
  arg_values(arg, "A", &args_array);
  for (i = args_old_array_length; i < next_hierarchy_level_max_id; ++i)
    {
      args_array[i] = grm_args_new();
      grm_args_push(args_array[i], "array_index", "i", i);
      return_error_if(args_array[i] == nullptr, ERROR_MALLOC);
      error = plot_init_args_structure(args_array[i], hierarchy_name_ptr, 1);
      return_if_error;
      if (strcmp(*hierarchy_name_ptr, "plots") == 0)
        {
          grm_args_push(args_array[i], "in_use", "i", 0);
        }
    }

  return ERROR_NONE;
}

err_t plot_init_args_structure(grm_args_t *args, const char **hierarchy_name_ptr,
                               unsigned int next_hierarchy_level_max_id)
{
  arg_t *arg = nullptr;
  grm_args_t **args_array = nullptr;
  unsigned int i;
  err_t error = ERROR_NONE;

  logger((stderr, "Init plot args structure for hierarchy: \"%s\"\n", *hierarchy_name_ptr));

  ++hierarchy_name_ptr;
  if (*hierarchy_name_ptr == nullptr)
    {
      return ERROR_NONE;
    }
  arg = args_at(args, *hierarchy_name_ptr);
  if (arg == nullptr)
    {
      args_array = static_cast<grm_args_t **>(calloc(next_hierarchy_level_max_id, sizeof(grm_args_t *)));
      error_cleanup_and_set_error_if(args_array == nullptr, ERROR_MALLOC);
      for (i = 0; i < next_hierarchy_level_max_id; ++i)
        {
          args_array[i] = grm_args_new();
          grm_args_push(args_array[i], "array_index", "i", i);
          error_cleanup_and_set_error_if(args_array[i] == nullptr, ERROR_MALLOC);
          error = plot_init_args_structure(args_array[i], hierarchy_name_ptr, 1);
          error_cleanup_if_error;
          if (strcmp(*hierarchy_name_ptr, "plots") == 0)
            {
              grm_args_push(args_array[i], "in_use", "i", 0);
            }
        }
      error_cleanup_if(!grm_args_push(args, *hierarchy_name_ptr, "nA", next_hierarchy_level_max_id, args_array));
      free(args_array);
      args_array = nullptr;
    }
  else
    {
      error = plot_init_arg_structure(arg, hierarchy_name_ptr - 1, next_hierarchy_level_max_id);
      error_cleanup_if_error;
    }

  return ERROR_NONE;

error_cleanup:
  if (args_array != nullptr)
    {
      for (i = 0; i < next_hierarchy_level_max_id; ++i)
        {
          if (args_array[i] != nullptr)
            {
              grm_args_delete(args_array[i]);
            }
        }
      free(args_array);
    }

  return error;
}

int plot_check_for_request(const grm_args_t *args, err_t *error)
{
  const char *request;
  int is_request = 0;

  *error = ERROR_NONE;
  if (grm_args_values(args, "request", "s", &request))
    {
      is_request = 1;
      *error = event_queue_enqueue_request_event(event_queue, request);
    }
  else
    {
      *error = ERROR_PLOT_INVALID_REQUEST;
    }

  return is_request;
}

void plot_set_flag_defaults(void)
{
  /* Use a standalone function for initializing flags instead of `plot_set_attribute_defaults` to guarantee the flags
   * are already set before `grm_plot` is called (important for `grm_merge`) */

  logger((stderr, "Set global flag defaults\n"));

  args_setdefault(global_root_args, "append_plots", "i", ROOT_DEFAULT_APPEND_PLOTS);
}

void plot_set_attribute_defaults(grm_args_t *plot_args)
{
  grm_args_t **current_subplot;

  logger((stderr, "Set plot attribute defaults\n"));

  if (!grm_args_contains(plot_args, "fig_size"))
    {
      // TODO: Remove this default
      args_setdefault(plot_args, "size", "dd", PLOT_DEFAULT_WIDTH, PLOT_DEFAULT_HEIGHT);
    }

  grm_args_values(plot_args, "subplots", "A", &current_subplot);
  while (*current_subplot != nullptr)
    {
      args_setdefault(*current_subplot, "kind", "s", PLOT_DEFAULT_KIND);
      args_setdefault(*current_subplot, "x_grid", "i", PLOT_DEFAULT_XGRID); // This arg is only used in plot.cxx
      args_setdefault(*current_subplot, "y_grid", "i", PLOT_DEFAULT_YGRID); // This arg is only used in plot.cxx
      args_setdefault(*current_subplot, "z_grid", "i", PLOT_DEFAULT_ZGRID); // This arg is only used in plot.cxx

      ++current_subplot;
    }
}

void plot_pre_plot(grm_args_t *plot_args)
{
  int clear;
  int previous_pixel_width, previous_pixel_height;

  logger((stderr, "Pre plot processing\n"));

  plot_set_text_encoding();
  if (grm_args_values(plot_args, "clear", "i", &clear))
    {
      logger((stderr, "Got keyword \"clear\" with value %d\n", clear));
      global_root->setAttribute("clear_ws", clear);
    }

  if (grm_args_values(plot_args, "previous_pixel_size", "ii", &previous_pixel_width, &previous_pixel_height))
    {
      edit_figure->setAttribute("_previous_pixel_width", previous_pixel_width);
      edit_figure->setAttribute("_previous_pixel_height", previous_pixel_height);
    }
}

void plot_set_text_encoding(void)
{
  global_render->setTextEncoding(edit_figure, ENCODING_UTF8);
}

err_t plot_pre_subplot(grm_args_t *subplot_args)
{
  const char *kind;
  double alpha;
  err_t error = ERROR_NONE;
  auto group = (current_dom_element) ? current_dom_element : edit_figure->lastChildElement();

  logger((stderr, "Pre subplot processing\n"));

  grm_args_values(subplot_args, "kind", "s", &kind);
  logger((stderr, "Got keyword \"kind\" with value \"%s\"\n", kind));
  error = plot_store_coordinate_ranges(subplot_args);
  return_if_error;
  plot_process_window(subplot_args);

  plot_process_colormap(subplot_args);
  plot_process_font(subplot_args);
  plot_process_resample_method(subplot_args);

  if (str_equals_any(kind, "polar", "polar_histogram"))
    {
      plot_draw_polar_axes(subplot_args);
    }
  else if (!str_equals_any(kind, "pie", "polar_heatmap", "nonuniformpolar_heatmap"))
    {
      plot_draw_axes(subplot_args, 1);
    }

  if (grm_args_values(subplot_args, "alpha", "d", &alpha))
    {
      group->setAttribute("alpha", alpha);
    }

  return ERROR_NONE;
}

void plot_process_colormap(grm_args_t *subplot_args)
{
  int colormap;
  auto group = edit_figure->lastChildElement();

  if (grm_args_values(subplot_args, "colormap", "i", &colormap)) group->setAttribute("colormap", colormap);
}

void plot_process_font(grm_args_t *subplot_args)
{
  int font, font_precision;
  auto group = edit_figure->lastChildElement();

  if (grm_args_values(subplot_args, "font", "i", &font)) group->setAttribute("font", font);
  if (grm_args_values(subplot_args, "font_precision", "i", &font_precision))
    group->setAttribute("font_precision", font_precision);
}

err_t plot_process_grid_arguments(const grm_args_t *args)
{
  int current_nesting_degree, nesting_degree;
  int *rows, *cols;
  unsigned int rows_length, cols_length;
  int row_span, col_span;
  int *row_spans, *col_spans;
  unsigned int row_spans_length, col_spans_length;
  int rowstart, rowstop, colstart, colstop;
  grm_args_t **current_subplot_args;
  grid_t *current_grid;
  element_t *current_element;

  double *rel_heights, *rel_widths, *abs_heights, *abs_widths, *aspect_ratios;
  int *fit_parents_heights, *fit_parents_widths;
  unsigned int rel_heights_length, rel_widths_length, abs_heights_length, abs_widths_length, aspect_ratios_length,
      fit_parents_heights_length, fit_parents_widths_length;
  err_t error = ERROR_NONE;

  if (global_grid != nullptr)
    {
      grid_delete(global_grid);
    }
  error = grid_new(1, 1, &global_grid);
  return_if_error;
  grm_args_values(active_plot_args, "subplots", "A", &current_subplot_args);
  while (*current_subplot_args != nullptr)
    {
      rows = nullptr, cols = nullptr;
      row_spans = nullptr, col_spans = nullptr;
      row_span = 1, col_span = 1;
      rel_heights = nullptr, rel_widths = nullptr;
      abs_heights = nullptr, abs_widths = nullptr;
      aspect_ratios = nullptr;
      fit_parents_heights = nullptr, fit_parents_widths = nullptr;

      grm_args_first_value(*current_subplot_args, "row", "I", &rows, &rows_length);
      grm_args_first_value(*current_subplot_args, "col", "I", &cols, &cols_length);

      if (rows == nullptr || cols == nullptr)
        {
          rows_length = 0;
          cols_length = 0;
        }

      if (rows_length != cols_length)
        {
          return ERROR_LAYOUT_COMPONENT_LENGTH_MISMATCH;
        }

      grm_args_first_value(*current_subplot_args, "row_span", "I", &row_spans, &row_spans_length);
      grm_args_first_value(*current_subplot_args, "col_span", "I", &col_spans, &col_spans_length);

      if (row_spans == nullptr)
        {
          row_spans = &row_span;
          row_spans_length = 1;
        }
      if (col_spans == nullptr)
        {
          col_spans = &col_span;
          col_spans_length = 1;
        }

      grm_args_first_value(*current_subplot_args, "rel_height", "D", &rel_heights, &rel_heights_length);
      grm_args_first_value(*current_subplot_args, "rel_width", "D", &rel_widths, &rel_widths_length);
      grm_args_first_value(*current_subplot_args, "abs_height", "D", &abs_heights, &abs_heights_length);
      grm_args_first_value(*current_subplot_args, "abs_width", "D", &abs_widths, &abs_widths_length);
      grm_args_first_value(*current_subplot_args, "aspect_ratio", "D", &aspect_ratios, &aspect_ratios_length);
      grm_args_first_value(*current_subplot_args, "fit_parents_height", "I", &fit_parents_heights,
                           &fit_parents_heights_length);
      grm_args_first_value(*current_subplot_args, "fit_parents_width", "I", &fit_parents_widths,
                           &fit_parents_widths_length);

      nesting_degree = (int)rows_length - 1;
      current_grid = global_grid;
      for (current_nesting_degree = 0; current_nesting_degree <= nesting_degree; ++current_nesting_degree)
        {

          rowstart = rows[current_nesting_degree];
          rowstop = (current_nesting_degree >= row_spans_length) ? rowstart + 1
                                                                 : rowstart + row_spans[current_nesting_degree];
          colstart = cols[current_nesting_degree];
          colstop = (current_nesting_degree >= col_spans_length) ? colstart + 1
                                                                 : colstart + col_spans[current_nesting_degree];

          if (rowstart - rowstop == 0 || colstart - colstop == 0)
            {
              break;
            }

          if (nesting_degree == current_nesting_degree)
            {
              error =
                  grid_setElementArgsSlice(rowstart, rowstop, colstart, colstop, *current_subplot_args, current_grid);
              return_if_error;
              error = grid_getElement(rowstart, colstart, current_grid, &current_element);
              return_if_error;
            }
          else
            {
              error = grid_ensureCellsAreGrid(rowstart, rowstop, colstart, colstop, current_grid);
              return_if_error;
              error = grid_getElement(rowstart, colstart, current_grid, (element_t **)&current_grid);
              return_if_error;
              current_element = (element_t *)current_grid;
            }

          if (rel_heights != nullptr && rel_heights_length > current_nesting_degree &&
              rel_heights[current_nesting_degree] != -1)
            {
              error = element_setRelativeHeight(current_element, rel_heights[current_nesting_degree]);
            }
          if (rel_widths != nullptr && rel_widths_length > current_nesting_degree &&
              rel_widths[current_nesting_degree] != -1)
            {
              error = element_setRelativeWidth(current_element, rel_widths[current_nesting_degree]);
            }
          if (abs_heights != nullptr && abs_heights_length > current_nesting_degree &&
              abs_heights[current_nesting_degree] != -1)
            {
              error = element_setAbsHeight(current_element, abs_heights[current_nesting_degree]);
            }
          if (abs_widths != nullptr && abs_widths_length > current_nesting_degree &&
              abs_widths[current_nesting_degree] != -1)
            {
              error = element_setAbsWidth(current_element, abs_widths[current_nesting_degree]);
            }
          if (aspect_ratios != nullptr && aspect_ratios_length > current_nesting_degree &&
              aspect_ratios[current_nesting_degree] != -1)
            {
              error = element_setAspectRatio(current_element, aspect_ratios[current_nesting_degree]);
            }
          if (fit_parents_heights != nullptr && fit_parents_heights_length > current_nesting_degree &&
              fit_parents_heights[current_nesting_degree] != -1)
            {
              element_setFitParentsHeight(current_element, fit_parents_heights[current_nesting_degree]);
            }
          if (fit_parents_widths != nullptr && fit_parents_widths_length > current_nesting_degree &&
              fit_parents_widths[current_nesting_degree] != -1)
            {
              element_setFitParentsWidth(current_element, fit_parents_widths[current_nesting_degree]);
            }
          return_if_error;
        }
      ++current_subplot_args;
    }

  return ERROR_NONE;
}

void plot_process_resample_method(grm_args_t *subplot_args)
{
  int resample_method_flag;
  auto group = edit_figure->lastChildElement();
  auto central_region = getCentralRegion();

  if (!grm_args_values(subplot_args, "resample_method", "i", &resample_method_flag))
    {
      const char *resample_method_str;
      if (grm_args_values(subplot_args, "resample_method", "s", &resample_method_str))
        central_region->setAttribute("resample_method", resample_method_str);
    }
  else
    {
      central_region->setAttribute("resample_method", resample_method_flag);
    }
}

void plot_process_window(grm_args_t *subplot_args)
{
  int scale = 0;
  const char *kind;
  int x_log, y_log, z_log;
  int x_flip, y_flip, z_flip;
  double rotation, tilt;

  auto group = edit_figure->lastChildElement();
  auto central_region = getCentralRegion();

  grm_args_values(subplot_args, "kind", "s", &kind);
  if (grm_args_values(subplot_args, "x_log", "i", &x_log)) group->setAttribute("x_log", x_log);
  if (grm_args_values(subplot_args, "y_log", "i", &y_log)) group->setAttribute("y_log", y_log);
  if (grm_args_values(subplot_args, "z_log", "i", &z_log)) group->setAttribute("z_log", z_log);
  if (grm_args_values(subplot_args, "x_flip", "i", &x_flip)) group->setAttribute("x_flip", x_flip);
  if (grm_args_values(subplot_args, "y_flip", "i", &y_flip)) group->setAttribute("y_flip", y_flip);
  if (grm_args_values(subplot_args, "z_flip", "i", &z_flip)) group->setAttribute("z_flip", z_flip);

  if (str_equals_any(kind, "wireframe", "surface", "plot3", "scatter3", "trisurface", "volume"))
    {
      group->setAttribute("adjust_z_lim", true);
      global_render->setSpace3d(central_region, 30.0, 0.0);
      if (grm_args_values(subplot_args, "rotation", "d", &rotation))
        central_region->setAttribute("space_3d_phi", rotation);
      if (grm_args_values(subplot_args, "tilt", "d", &tilt)) central_region->setAttribute("space_3d_theta", tilt);
    }
  else if (strcmp(kind, "isosurface") == 0)
    {
      global_render->setWindow3d(central_region, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
      global_render->setSpace3d(central_region, 45.0, 2.5);
      if (grm_args_values(subplot_args, "rotation", "d", &rotation))
        central_region->setAttribute("space_3d_phi", rotation);
      if (grm_args_values(subplot_args, "tilt", "d", &tilt)) central_region->setAttribute("space_3d_theta", tilt);
    }

  if (grm_args_values(subplot_args, "scale", "i", scale))
    global_render->setScale(edit_figure->lastChildElement(), scale);
}

err_t plot_store_coordinate_ranges(grm_args_t *subplot_args)
{
  const char *kind;
  err_t error = ERROR_NONE;
  double x_min, x_max, y_min, y_max, z_min, z_max, c_min, c_max;

  auto group = (current_dom_element) ? current_dom_element : edit_figure->lastChildElement();

  if (grm_args_contains(subplot_args, "_original_x_lim"))
    {
      group->setAttribute("original_x_lim", true);
    }

  grm_args_values(subplot_args, "kind", "s", &kind);
  group->setAttribute("kind", kind);

  if (grm_args_values(subplot_args, "x_lim", "dd", &x_min, &x_max))
    {
      group->setAttribute("x_lim_min", x_min);
      group->setAttribute("x_lim_max", x_max);
    }
  if (grm_args_values(subplot_args, "y_lim", "dd", &y_min, &y_max))
    {
      group->setAttribute("y_lim_min", y_min);
      group->setAttribute("y_lim_max", y_max);
    }
  if (grm_args_values(subplot_args, "z_lim", "dd", &z_min, &z_max))
    {
      group->setAttribute("z_lim_min", z_min);
      group->setAttribute("z_lim_max", z_max);
    }
  if (grm_args_values(subplot_args, "c_lim", "dd", &c_min, &c_max))
    {
      group->setAttribute("c_lim_min", c_min);
      group->setAttribute("c_lim_max", c_max);
    }

  return error;
}

void plot_post_plot(grm_args_t *plot_args)
{
  int update;

  logger((stderr, "Post plot processing\n"));

  if (grm_args_values(plot_args, "update", "i", &update))
    {
      logger((stderr, "Got keyword \"update\" with value %d\n", update));
      global_root->setAttribute("update_ws", update);
    }
}

void plot_post_subplot(grm_args_t *subplot_args)
{
  const char *kind;

  logger((stderr, "Post subplot processing\n"));

  grm_args_values(subplot_args, "kind", "s", &kind);
  logger((stderr, "Got keyword \"kind\" with value \"%s\"\n", kind));
  if (grm_args_contains(subplot_args, "labels"))
    {
      if (str_equals_any(kind, "line", "stairs", "scatter", "stem"))
        {
          plot_draw_legend(subplot_args);
        }
      else if (strcmp(kind, "pie") == 0)
        {
          plot_draw_pie_legend(subplot_args);
        }
    }
  if (strcmp(kind, "barplot") == 0)
    {
      plot_draw_axes(subplot_args, 2);
    }
  else if (str_equals_any(kind, "polar_heatmap", "nonuniformpolar_heatmap"))
    {
      plot_draw_polar_axes(subplot_args);
    }
}

err_t plot_get_args_in_hierarchy(grm_args_t *args, const char **hierarchy_name_start_ptr, const char *key,
                                 uint_map_t *hierarchy_to_id, const grm_args_t **found_args,
                                 const char ***found_hierarchy_name_ptr)
{
  const char *key_hierarchy_name, **current_hierarchy_name_ptr;
  grm_args_t *current_args, **args_array;
  arg_t *current_arg;
  unsigned int args_array_length, current_id;

  logger((stderr, "Check hierarchy level for key \"%s\"...\n", key));
  return_error_if(!string_map_at(plot_valid_keys_map, key, static_cast<const char **>(&key_hierarchy_name)),
                  ERROR_PLOT_UNKNOWN_KEY);
  logger((stderr, "... got hierarchy \"%s\"\n", key_hierarchy_name));
  current_hierarchy_name_ptr = hierarchy_name_start_ptr;
  current_args = args;
  if (strcmp(*hierarchy_name_start_ptr, key_hierarchy_name) != 0)
    {
      while (*++current_hierarchy_name_ptr != nullptr)
        {
          current_arg = args_at(current_args, *current_hierarchy_name_ptr);
          return_error_if(current_arg == nullptr, ERROR_INTERNAL);
          arg_first_value(current_arg, "A", &args_array, &args_array_length);
          uint_map_at(hierarchy_to_id, *current_hierarchy_name_ptr, &current_id);
          /* Check for the invalid id 0 because id 0 is set for append mode */
          if (current_id == 0)
            {
              current_id = args_array_length + 1;
              if (strcmp(*current_hierarchy_name_ptr, "plots") == 0)
                {
                  int last_plot_in_use = 0;
                  if (grm_args_values(args_array[args_array_length - 1], "in_use", "i", &last_plot_in_use) &&
                      !last_plot_in_use)
                    {
                      /* Use the last already existing plot if it is still empty */
                      --current_id;
                    }
                }
              logger((stderr, "Append mode, set id to \"%u\"\n", current_id));
              uint_map_insert(hierarchy_to_id, *current_hierarchy_name_ptr, current_id);
            }
          if (current_id > args_array_length)
            {
              plot_init_args_structure(current_args, current_hierarchy_name_ptr - 1, current_id);
              arg_first_value(current_arg, "A", &args_array, &args_array_length);
            }
          current_args = args_array[current_id - 1];
          if (strcmp(*current_hierarchy_name_ptr, "plots") == 0)
            {
              int in_use;
              err_t error = ERROR_NONE;
              grm_args_values(current_args, "in_use", "i", &in_use);
              if (in_use)
                {
                  error = event_queue_enqueue_update_plot_event(event_queue, (int)current_id - 1);
                }
              else
                {
                  error = event_queue_enqueue_new_plot_event(event_queue, (int)current_id - 1);
                }
              return_if_error;
              grm_args_push(current_args, "in_use", "i", 1);
            }
          if (strcmp(*current_hierarchy_name_ptr, key_hierarchy_name) == 0) break;
        }
      return_error_if(*current_hierarchy_name_ptr == nullptr, ERROR_INTERNAL);
    }
  if (found_args != nullptr) *found_args = current_args;
  if (found_hierarchy_name_ptr != nullptr) *found_hierarchy_name_ptr = current_hierarchy_name_ptr;

  return ERROR_NONE;
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plotting ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

err_t plot_line(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  err_t error = ERROR_NONE;
  const char *orientation;
  int marker_type;

  grm_args_values(subplot_args, "series", "A", &current_series);
  std::shared_ptr<GRM::Element> group =
      (current_central_region_element) ? current_central_region_element : getCentralRegion();

  while (*current_series != nullptr)
    {
      double *x = nullptr, *y = nullptr, *int_limits_high = nullptr, *int_limits_low = nullptr;
      unsigned int x_length = 0, y_length = 0, limits_high_num = 0, limits_low_num = 0;
      char *spec;
      double x_min, x_max, y_min, y_max;
      auto subGroup = global_render->createSeries("line");
      group->append(subGroup);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      grm_args_first_value(*current_series, "y", "D", &y, &y_length);

      if (y_length > 0)
        {
          std::vector<double> y_vec(y, y + y_length);
          (*context)["y" + str] = y_vec;
          subGroup->setAttribute("y", "y" + str);
        }
      if (grm_args_first_value(*current_series, "x", "D", &x, &x_length))
        {
          std::vector<double> x_vec(x, x + x_length);
          (*context)["x" + str] = x_vec;
          subGroup->setAttribute("x", "x" + str);
        }

      if (grm_args_values(subplot_args, "orientation", "s", &orientation))
        subGroup->setAttribute("orientation", orientation);

      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          subGroup->setAttribute("x_range_min", x_min);
          subGroup->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          subGroup->setAttribute("y_range_min", y_min);
          subGroup->setAttribute("y_range_max", y_max);
        }

      if (grm_args_values(*current_series, "line_spec", "s", &spec)) subGroup->setAttribute("line_spec", spec);
      if (grm_args_values(*current_series, "marker_type", "i", &marker_type))
        subGroup->setAttribute("marker_type", marker_type);

      // check if there are any attributes for integrals which should be created
      if (grm_args_first_value(*current_series, "int_limits_high", "D", &int_limits_high, &limits_high_num))
        {
          if (grm_args_first_value(*current_series, "int_limits_low", "D", &int_limits_low, &limits_low_num))
            {
              if (limits_low_num != limits_high_num)
                {
                  error = ERROR_PLOT_MISSING_DATA;
                  return_if_error;
                }
              else
                {
                  auto integral_group = global_render->createElement("integral_group");
                  subGroup->append(integral_group);

                  std::vector<double> limits_high_vec(int_limits_high, int_limits_high + limits_high_num);
                  (*context)["int_limits_high" + str] = limits_high_vec;
                  integral_group->setAttribute("int_limits_high", "int_limits_high" + str);

                  std::vector<double> limits_low_vec(int_limits_low, int_limits_low + limits_low_num);
                  (*context)["int_limits_low" + str] = limits_low_vec;
                  integral_group->setAttribute("int_limits_low", "int_limits_low" + str);
                }
            }
          else
            {
              error = ERROR_PLOT_MISSING_DATA;
              return_if_error;
            }
        }

      global_root->setAttribute("_id", ++id);
      error = plot_draw_error_bars(*current_series, x_length);
      return_if_error;
      ++current_series;
    }

  return error;
}

err_t plot_stairs(grm_args_t *subplot_args)
{
  /*
   * Parameters:
   * `x` as double array
   * `y` as double array
   * optional step position `step_where` as string, modes: `pre`, `mid`, `post`, Default: `mid`
   * optional `spec`
   */

  grm_args_t **current_series;
  char *orientation;
  double x_min, x_max, y_min, y_max;
  double *x = nullptr, *y = nullptr, *xi;
  err_t error = ERROR_NONE;

  grm_args_values(subplot_args, "series", "A", &current_series);
  std::shared_ptr<GRM::Element> group =
      (current_central_region_element) ? current_central_region_element : getCentralRegion();

  std::shared_ptr<GRM::Element> element; // declare element here for multiple usages / assignments later
  while (*current_series != nullptr)
    {
      unsigned int x_length, y_length;
      char *spec;
      const char *where;
      auto subGroup = global_render->createSeries("stairs");
      group->append(subGroup);

      if (grm_args_values(subplot_args, "orientation", "s", &orientation))
        subGroup->setAttribute("orientation", orientation);

      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      grm_args_first_value(*current_series, "y", "D", &y, &y_length);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> x_vec(x, x + x_length);
      (*context)["x" + str] = x_vec;
      subGroup->setAttribute("x", "x" + str);

      std::vector<double> y_vec(y, y + y_length);
      (*context)["y" + str] = y_vec;
      subGroup->setAttribute("y", "y" + str);

      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          subGroup->setAttribute("x_range_min", x_min);
          subGroup->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          subGroup->setAttribute("y_range_min", y_min);
          subGroup->setAttribute("y_range_max", y_max);
        }

      if (grm_args_values(*current_series, "line_spec", "s", &spec)) subGroup->setAttribute("line_spec", spec);

      if (grm_args_values(*current_series, "step_where", "s", &where)) subGroup->setAttribute("step_where", where);

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }
  return ERROR_NONE;
}

err_t plot_scatter(grm_args_t *subplot_args)
{
  /*
   * Parameters:
   * `x` as double array
   * `y` as double array
   * optional marker size `z` as double array
   * optional marker color `c` as double array for each marker or as single integer for all markers
   * optional `marker_type` as integer (see: [Marker types](https://gr-framework.org/markertypes.html?highlight=marker))
   */
  grm_args_t **current_series;
  err_t error;
  char *orientation;

  grm_args_values(subplot_args, "series", "A", &current_series);
  std::shared_ptr<GRM::Element> group =
      (current_central_region_element) ? current_central_region_element : getCentralRegion();

  while (*current_series != nullptr)
    {
      auto subGroup = global_render->createSeries("scatter");
      group->append(subGroup);
      if (grm_args_values(subplot_args, "orientation", "s", &orientation))
        subGroup->setAttribute("orientation", orientation);

      double *x = nullptr, *y = nullptr, *z = nullptr, *c = nullptr, c_min, c_max;
      unsigned int x_length, y_length, z_length, c_length;
      int i, c_index = -1, marker_type;
      double x_min, x_max, y_min, y_max;

      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      grm_args_first_value(*current_series, "y", "D", &y, &y_length);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> x_vec(x, x + x_length);
      std::vector<double> y_vec(y, y + y_length);

      (*context)["x" + str] = x_vec;
      subGroup->setAttribute("x", "x" + str);
      (*context)["y" + str] = y_vec;
      subGroup->setAttribute("y", "y" + str);
      if (grm_args_first_value(*current_series, "z", "D", &z, &z_length))
        {
          std::vector<double> z_vec(z, z + z_length);

          (*context)["z" + str] = z_vec;
          subGroup->setAttribute("z", "z" + str);
        }
      if (grm_args_values(*current_series, "marker_type", "i", &marker_type))
        subGroup->setAttribute("marker_type", marker_type);

      if (grm_args_first_value(*current_series, "c", "D", &c, &c_length))
        {
          std::vector<double> c_vec(c, c + c_length);

          (*context)["c" + str] = c_vec;
          subGroup->setAttribute("c", "c" + str);
        }
      if (grm_args_values(*current_series, "c", "i", &c_index))
        {
          subGroup->setAttribute("color_ind", c_index);
        }

      if (z != nullptr || c != nullptr)
        {
          if (grm_args_values(subplot_args, "c_lim", "dd", &c_min, &c_max))
            {
              group->parentElement()->setAttribute("c_lim_min", c_min);
              group->parentElement()->setAttribute("c_lim_max", c_max);
            }
        }

      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          subGroup->setAttribute("x_range_min", x_min);
          subGroup->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          subGroup->setAttribute("y_range_min", y_min);
          subGroup->setAttribute("y_range_max", y_max);
        }

      error = plot_draw_error_bars(*current_series, x_length);
      return_if_error;
      global_root->setAttribute("_id", ++id);
      ++current_series;
    }

  return ERROR_NONE;
}

err_t plot_quiver(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  err_t error = ERROR_NONE;

  std::shared_ptr<GRM::Element> group =
      (current_central_region_element) ? current_central_region_element : getCentralRegion();

  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double *x = nullptr, *y = nullptr, *u = nullptr, *v = nullptr;
      unsigned int x_length, y_length, u_length, v_length;
      double x_min, x_max, y_min, y_max;
      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      grm_args_first_value(*current_series, "y", "D", &y, &y_length);
      grm_args_first_value(*current_series, "u", "D", &u, &u_length);
      grm_args_first_value(*current_series, "v", "D", &v, &v_length);

      std::vector<double> x_vec(x, x + x_length);
      std::vector<double> y_vec(y, y + y_length);
      std::vector<double> u_vec(u, u + x_length * y_length);
      std::vector<double> v_vec(v, v + x_length * y_length);

      int id = (int)global_root->getAttribute("_id");
      std::string str = std::to_string(id);
      auto temp =
          global_render->createQuiver("x" + str, x_vec, "y" + str, y_vec, "u" + str, u_vec, "v" + str, v_vec, 1);

      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          temp->setAttribute("x_range_min", x_min);
          temp->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          temp->setAttribute("y_range_min", y_min);
          temp->setAttribute("y_range_max", y_max);
        }
      group->append(temp);

      global_root->setAttribute("_id", id++);
      ++current_series;
    }
  error = plot_draw_colorbar(subplot_args, 0.0, 256);

  return error;
}

err_t plot_stem(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  std::shared_ptr<GRM::Element> group =
      (current_central_region_element) ? current_central_region_element : getCentralRegion();

  char *orientation;

  grm_args_values(subplot_args, "series", "A", &current_series);

  while (*current_series != nullptr)
    {
      double *x, *y;
      unsigned int x_length, y_length;
      char *spec;
      double y_min, y_max;

      auto subGroup = global_render->createSeries("stem");
      group->append(subGroup);

      if (grm_args_values(subplot_args, "orientation", "s", &orientation))
        subGroup->setAttribute("orientation", orientation);

      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      grm_args_first_value(*current_series, "y", "D", &y, &y_length);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> x_vec(x, x + x_length);
      std::vector<double> y_vec(y, y + x_length);

      (*context)["x" + str] = x_vec;
      subGroup->setAttribute("x", "x" + str);
      (*context)["y" + str] = y_vec;
      subGroup->setAttribute("y", "y" + str);

      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          subGroup->setAttribute("y_range_min", y_min);
          subGroup->setAttribute("y_range_max", y_max);
        }

      if (grm_args_values(*current_series, "line_spec", "s", &spec)) subGroup->setAttribute("line_spec", spec);

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }

  return ERROR_NONE;
}

err_t plot_hist(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  int bar_color_index = 989, i;
  double bar_color_rgb[3] = {-1};
  err_t error = ERROR_NONE;

  std::shared_ptr<GRM::Element> group =
      (current_central_region_element) ? current_central_region_element : getCentralRegion();

  grm_args_values(subplot_args, "series", "A", &current_series);
  grm_args_values(subplot_args, "bar_color", "ddd", &bar_color_rgb[0], &bar_color_rgb[1], &bar_color_rgb[2]);
  grm_args_values(subplot_args, "bar_color", "i", &bar_color_index);

  while (*current_series != nullptr)
    {
      int edge_color_index = 1;
      double edge_color_rgb[3] = {-1};
      double x_min, x_max, bar_width, y_min, y_max;
      double *bins, *x, *weights;
      unsigned int num_bins = 0, x_length, num_weights;
      char *orientation;

      auto subGroup = global_render->createSeries("hist");
      group->append(subGroup);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> bar_color_rgb_vec(bar_color_rgb, bar_color_rgb + 3);
      (*context)["fill_color_rgb" + str] = bar_color_rgb_vec;
      subGroup->setAttribute("fill_color_rgb", "fill_color_rgb" + str);
      subGroup->setAttribute("fill_color_ind", bar_color_index);

      grm_args_values(*current_series, "edge_color", "ddd", &edge_color_rgb[0], &edge_color_rgb[1], &edge_color_rgb[2]);
      grm_args_values(*current_series, "edge_color", "i", &edge_color_index);
      std::vector<double> edge_color_rgb_vec(edge_color_rgb, edge_color_rgb + 3);
      (*context)["line_color_rgb" + str] = edge_color_rgb_vec;
      subGroup->setAttribute("line_color_rgb", "line_color_rgb" + str);
      subGroup->setAttribute("line_color_ind", edge_color_index);

      if (grm_args_first_value(*current_series, "bins", "D", &bins, &num_bins))
        {
          std::vector<double> bins_vec(bins, bins + num_bins);
          (*context)["bins" + str] = bins_vec;
          subGroup->setAttribute("bins", "bins" + str);
        }
      if (num_bins == 0)
        {
          if (grm_args_values(*current_series, "num_bins", "i", &num_bins))
            subGroup->setAttribute("num_bins", (int)num_bins);
        }

      if (grm_args_values(subplot_args, "orientation", "s", &orientation))
        subGroup->setAttribute("orientation", orientation);

      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          subGroup->setAttribute("x_range_min", x_min);
          subGroup->setAttribute("x_range_max", x_max);
        }

      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          subGroup->setAttribute("y_range_min", y_min);
          subGroup->setAttribute("y_range_max", y_max);
        }

      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      std::vector<double> x_vec(x, x + x_length);
      (*context)["x" + str] = x_vec;
      subGroup->setAttribute("x", "x" + str);

      if (grm_args_first_value(*current_series, "weights", "D", &weights, &num_weights))
        {
          std::vector<double> weights_vec(weights, weights + num_weights);
          (*context)["weights" + str] = weights_vec;
          subGroup->setAttribute("weights", "weights" + str);
        }

      bar_width = (x_max - x_min) / num_bins;

      if (grm_args_contains(*current_series, "error"))
        {
          if (num_bins <= 1)
            {
              num_bins = (int)(3.3 * log10((int)x_length) + 0.5) + 1;
            }
          error = plot_draw_error_bars(*current_series, num_bins);
        }
      global_root->setAttribute("_id", ++id);
      ++current_series;
    }

  return error;
}

err_t plot_barplot(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  int bar_color, edge_color;
  double bar_color_rgb[3];
  double edge_color_rgb[3];
  double bar_width, edge_width;
  const char *style;
  int series_index = 0;
  unsigned int i;
  err_t error = ERROR_NONE;
  char *orientation;

  std::shared_ptr<GRM::Element> group =
      (current_central_region_element) ? current_central_region_element : getCentralRegion();

  /* Push attributes on the subplot level to the tree */
  auto context = global_render->getContext();

  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      int inner_series_index;
      double *y = nullptr, *x = nullptr;
      unsigned int y_length = 0, x_length = 0;
      grm_args_t **inner_series = nullptr;
      unsigned int inner_series_length = 0;
      int *c = nullptr;
      unsigned int c_length;
      double *c_rgb = nullptr;
      unsigned int c_rgb_length;
      char **y_labels = nullptr;
      unsigned int y_labels_length = 0;
      std::vector<int> c_vec;
      std::vector<double> c_rgb_vec;
      double x_min, x_max, y_min, y_max;

      auto subGroup = global_render->createSeries("barplot");
      group->append(subGroup);
      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string id_str = std::to_string(id);

      if (grm_args_values(subplot_args, "bar_color", "ddd", &bar_color_rgb[0], &bar_color_rgb[1], &bar_color_rgb[2]))
        {
          std::vector<double> bar_color_rgb_vec(bar_color_rgb, bar_color_rgb + 3);
          (*context)["fill_color_rgb" + id_str] = bar_color_rgb_vec;
          subGroup->setAttribute("fill_color_rgb", "fill_color_rgb" + id_str);
        }
      if (grm_args_values(subplot_args, "bar_color", "i", &bar_color))
        {
          subGroup->setAttribute("fill_color_ind", bar_color);
        }
      if (grm_args_values(subplot_args, "bar_width", "d", &bar_width))
        {
          subGroup->setAttribute("bar_width", bar_width);
        }
      if (grm_args_values(subplot_args, "style", "s", &style))
        {
          subGroup->setAttribute("style", style);
        }
      if (grm_args_values(subplot_args, "orientation", "s", &orientation))
        {
          subGroup->setAttribute("orientation", orientation);
        }

      /* Push attributes on the series level to the tree */
      if (grm_args_values(*current_series, "edge_color", "ddd", &edge_color_rgb[0], &edge_color_rgb[1],
                          &edge_color_rgb[2]))
        {
          std::vector<double> edge_color_rgb_vec(edge_color_rgb, edge_color_rgb + 3);
          (*context)["line_color_rgb" + id_str] = edge_color_rgb_vec;
          subGroup->setAttribute("line_color_rgb", "line_color_rgb" + id_str);
        }
      if (grm_args_values(*current_series, "edge_color", "i", &edge_color))
        {
          subGroup->setAttribute("line_color_ind", edge_color);
        }
      if (grm_args_values(*current_series, "edge_width", "d", &edge_width))
        {
          subGroup->setAttribute("edge_width", edge_width);
        }
      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          subGroup->setAttribute("x_range_min", x_min);
          subGroup->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          subGroup->setAttribute("y_range_min", y_min);
          subGroup->setAttribute("y_range_max", y_max);
        }
      if (grm_args_first_value(*current_series, "y_labels", "S", &y_labels, &y_labels_length))
        {
          std::vector<std::string> y_labels_vec(y_labels, y_labels + y_labels_length);
          (*context)["y_labels" + id_str] = y_labels_vec;
          subGroup->setAttribute("y_labels", "y_labels" + id_str);
        }
      if (grm_args_first_value(*current_series, "c", "I", &c, &c_length))
        {
          c_vec = std::vector<int>(c, c + c_length);
          (*context)["c_ind" + id_str] = c_vec;
          subGroup->setAttribute("color_ind_values", "c_ind" + id_str);
        }
      if (grm_args_first_value(*current_series, "c", "D", &c_rgb, &c_rgb_length))
        {
          c_rgb_vec = std::vector<double>(c_rgb, c_rgb + c_rgb_length);
          (*context)["c_rgb" + id_str] = c_rgb_vec;
          subGroup->setAttribute("color_rgb_values", "c_rgb" + id_str);
        }
      if ((grm_args_first_value(*current_series, "x", "D", &x, &x_length)))
        {
          /* Process data for a flat series (no inner_series) */
          auto x_vec = std::vector<double>(x, x + x_length);
          (*context)["x" + id_str] = x_vec;
          subGroup->setAttribute("x", "x" + id_str);
        }

      std::vector<double> y_vec;
      std::vector<int> indices_vec;
      if ((grm_args_first_value(*current_series, "y", "D", &y, &y_length)))
        {
          /* Process data for a flat series (no inner_series) */
          y_vec = std::vector<double>(y, y + y_length);
          indices_vec = std::vector<int>(y_length, 1);
        }
      else if (grm_args_first_value(*current_series, "inner_series", "A", &inner_series, &inner_series_length))
        {
          /* Flatten inner_series */
          /* Since the data has to be processed the error handling is done here instead of in the renderer */
          if (c != nullptr)
            {
              cleanup_and_set_error_if((c_length < inner_series_length), ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
            }
          if (c_rgb != nullptr)
            {
              cleanup_and_set_error_if((c_rgb_length < inner_series_length * 3), ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
            }

          y_vec = {};
          indices_vec = {};
          c_vec = {};
          c_rgb_vec = {};
          std::vector<std::string> y_labels_vec = {};
          bool inner_y_labels_exists = false;
          bool inner_c_exists = false;
          int cumulative_y_index = 0;
          double *inner_y = nullptr;
          unsigned int inner_y_length = 0;
          int color_save_spot = 1000;

          for (inner_series_index = 0; inner_series_index < inner_series_length; inner_series_index++)
            {
              int *inner_c = nullptr;
              unsigned int inner_c_length;
              double *inner_c_rgb = nullptr;
              unsigned int inner_c_rgb_length;

              /* Retrieve attributes from the inner_series level */
              grm_args_first_value(inner_series[inner_series_index], "y", "D", &inner_y, &inner_y_length);
              if (grm_args_first_value(inner_series[inner_series_index], "c", "I", &inner_c, &inner_c_length))
                {
                  cleanup_and_set_error_if(inner_c_length != inner_y_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
                  cleanup_and_set_error_if(c_rgb != nullptr, ERROR_PLOT_INCOMPATIBLE_ARGUMENTS);
                }
              if (grm_args_first_value(inner_series[inner_series_index], "c", "D", &inner_c_rgb, &inner_c_rgb_length))
                {
                  cleanup_and_set_error_if((inner_c_rgb_length < inner_y_length * 3),
                                           ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
                  cleanup_and_set_error_if(c != nullptr, ERROR_PLOT_INCOMPATIBLE_ARGUMENTS);
                }
              if (y_labels == nullptr)
                {
                  grm_args_first_value(inner_series[inner_series_index], "y_labels", "nS", &y_labels, &y_labels_length);
                }

              /* Push attributes from the inner_series into the corresponding vectors on the series level */
              indices_vec.push_back((int)inner_y_length);
              y_vec.insert(y_vec.end(), inner_y, inner_y + inner_y_length);
              for (i = 0; i < inner_y_length; ++i)
                {
                  if (inner_c != nullptr)
                    {
                      c_vec.push_back(inner_c[i]);
                      inner_c_exists = true;
                    }
                  else if (inner_c_rgb != nullptr)
                    {
                      global_render->setColorRep(subGroup, color_save_spot, inner_c_rgb[i * 3], inner_c_rgb[i * 3 + 1],
                                                 inner_c_rgb[i * 3 + 2]);
                      c_vec.push_back(color_save_spot);
                      ++color_save_spot;
                      inner_c_exists = true;
                    }
                  else
                    {
                      if (c != nullptr)
                        {
                          c_vec.push_back(c[inner_series_index]);
                        }
                      else if (c_rgb != nullptr)
                        {
                          global_render->setColorRep(subGroup, color_save_spot, c_rgb[inner_series_index * 3],
                                                     c_rgb[inner_series_index * 3 + 1],
                                                     c_rgb[inner_series_index * 3 + 2]);
                          c_vec.push_back(color_save_spot);
                          ++color_save_spot;
                        }
                      else
                        {
                          c_vec.push_back(-1);
                        }
                    }

                  cleanup_and_set_error_if(color_save_spot > 1256, ERROR_INTERNAL);

                  if (cumulative_y_index + i < y_labels_length)
                    {
                      y_labels_vec.push_back(std::string(y_labels[cumulative_y_index + i]));
                      inner_y_labels_exists = true;
                    }
                  else
                    {
                      y_labels_vec.push_back(std::string(""));
                    }
                }
              cumulative_y_index += (int)inner_y_length;
            }
          if (inner_y_labels_exists)
            {
              (*context)["y_labels" + id_str] = y_labels_vec;
              subGroup->setAttribute("y_labels", "y_labels" + id_str);
            }
          /* Replace the previously pushed c or c_rgb vector by one containing the data of the inner_series if such
           * data exists */
          if (inner_c_exists)
            {
              (*context)["c_ind" + id_str] = c_vec;
              subGroup->setAttribute("color_ind_values", "c_ind" + id_str);
              subGroup->removeAttribute("color_rgb_values");
            }
        }
      else
        {
          cleanup_and_set_error(ERROR_PLOT_MISSING_DATA);
        }

      (*context)["y" + id_str] = y_vec;
      subGroup->setAttribute("y", "y" + id_str);
      (*context)["indices" + id_str] = indices_vec;
      subGroup->setAttribute("indices", "indices" + id_str);
      subGroup->setAttribute("series_index", series_index);

      cleanup_and_set_error_if(y != nullptr && inner_series != nullptr, ERROR_PLOT_INCOMPATIBLE_ARGUMENTS);

      error = plot_draw_error_bars(*current_series, y_length);

      ++series_index;
      ++current_series;
      global_root->setAttribute("_id", ++id);
    }

cleanup:

  return error;
}

err_t plot_contour(grm_args_t *subplot_args)
{
  int num_levels = 20, major_h;
  grm_args_t **current_series;
  err_t error = ERROR_NONE;

  std::shared_ptr<GRM::Element> group =
      (current_central_region_element) ? current_central_region_element : getCentralRegion();

  bool has_levels = grm_args_values(subplot_args, "levels", "i", &num_levels);
  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double *x, *y, *z;
      unsigned int x_length, y_length, z_length;
      double x_min, x_max, y_min, y_max, z_min, z_max;
      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      grm_args_first_value(*current_series, "y", "D", &y, &y_length);
      grm_args_first_value(*current_series, "z", "D", &z, &z_length);
      auto subGroup = global_render->createSeries("contour");
      group->append(subGroup);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> x_vec(x, x + x_length);
      (*context)["x" + str] = x_vec;
      subGroup->setAttribute("x", "x" + str);

      std::vector<double> y_vec(y, y + y_length);
      (*context)["y" + str] = y_vec;
      subGroup->setAttribute("y", "y" + str);

      std::vector<double> z_vec(z, z + z_length);
      (*context)["z" + str] = z_vec;
      subGroup->setAttribute("z", "z" + str);

      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          subGroup->setAttribute("x_range_min", x_min);
          subGroup->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          subGroup->setAttribute("y_range_min", y_min);
          subGroup->setAttribute("y_range_max", y_max);
        }
      if (grm_args_values(*current_series, "z_range", "dd", &z_min, &z_max))
        {
          subGroup->setAttribute("z_range_min", z_min);
          subGroup->setAttribute("z_range_max", z_max);
        }
      if (grm_args_values(subplot_args, "major_h", "i", &major_h)) subGroup->setAttribute("major_h", major_h);

      if (has_levels) subGroup->setAttribute("levels", num_levels);

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }
  error = plot_draw_colorbar(subplot_args, 0.0, num_levels);

  return error;
}

err_t plot_contourf(grm_args_t *subplot_args)
{
  int num_levels = 20, major_h;
  grm_args_t **current_series;
  err_t error = ERROR_NONE;

  std::shared_ptr<GRM::Element> group =
      (current_central_region_element) ? current_central_region_element : getCentralRegion();

  bool has_levels = grm_args_values(subplot_args, "levels", "i", &num_levels);
  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double *x, *y, *z;
      unsigned int x_length, y_length, z_length;
      double x_min, x_max, y_min, y_max, z_min, z_max;
      auto subGroup = global_render->createSeries("contourf");
      group->append(subGroup);
      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      grm_args_first_value(*current_series, "y", "D", &y, &y_length);
      grm_args_first_value(*current_series, "z", "D", &z, &z_length);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> x_vec(x, x + x_length);
      (*context)["x" + str] = x_vec;
      subGroup->setAttribute("x", "x" + str);

      std::vector<double> y_vec(y, y + y_length);
      (*context)["y" + str] = y_vec;
      subGroup->setAttribute("y", "y" + str);

      std::vector<double> z_vec(z, z + z_length);
      (*context)["z" + str] = z_vec;
      subGroup->setAttribute("z", "z" + str);

      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          subGroup->setAttribute("x_range_min", x_min);
          subGroup->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          subGroup->setAttribute("y_range_min", y_min);
          subGroup->setAttribute("y_range_max", y_max);
        }
      if (grm_args_values(*current_series, "z_range", "dd", &z_min, &z_max))
        {
          subGroup->setAttribute("z_range_min", z_min);
          subGroup->setAttribute("z_range_max", z_max);
        }
      if (grm_args_values(subplot_args, "major_h", "i", &major_h)) subGroup->setAttribute("major_h", major_h);

      if (has_levels) subGroup->setAttribute("levels", num_levels);

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }
  error = plot_draw_colorbar(subplot_args, 0.0, num_levels);

  return error;
}

err_t plot_hexbin(grm_args_t *subplot_args)
{
  grm_args_t **current_series;

  std::shared_ptr<GRM::Element> group =
      (current_central_region_element) ? current_central_region_element : getCentralRegion();

  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double *x, *y;
      unsigned int x_length, y_length;
      int cntmax, nbins;
      double x_min, x_max, y_min, y_max;
      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      grm_args_first_value(*current_series, "y", "D", &y, &y_length);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);

      auto x_vec = std::vector<double>(x, x + x_length);
      auto y_vec = std::vector<double>(y, y + y_length);
      auto subGroup = global_render->createHexbin("x" + str, x_vec, "y" + str, y_vec);
      if (grm_args_values(*current_series, "num_bins", "i", &nbins)) subGroup->setAttribute("num_bins", nbins);
      group->append(subGroup);

      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          subGroup->setAttribute("x_range_min", x_min);
          subGroup->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          subGroup->setAttribute("y_range_min", y_min);
          subGroup->setAttribute("y_range_max", y_max);
        }

      plot_draw_colorbar(subplot_args, 0.0, 256);

      global_root->setAttribute("_id", id++);
      ++current_series;
    }

  return ERROR_NONE;
}

err_t plot_polar_heatmap(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  int z_log = 0;
  unsigned int i, cols, rows, z_length;
  double *x = nullptr, *y = nullptr, *z, x_min, x_max, y_min, y_max, z_min, z_max, c_min, c_max;
  err_t error = ERROR_NONE;
  char *kind;

  std::shared_ptr<GRM::Element> group =
      (current_central_region_element) ? current_central_region_element : getCentralRegion();

  grm_args_values(subplot_args, "series", "A", &current_series);
  grm_args_values(subplot_args, "z_log", "i", &z_log);
  while (*current_series != nullptr)
    {
      auto subGroup = global_render->createSeries("polar_heatmap");
      group->append(subGroup);

      grm_args_first_value(*current_series, "x", "D", &x, &cols);
      grm_args_first_value(*current_series, "y", "D", &y, &rows);
      grm_args_first_value(*current_series, "z", "D", &z, &z_length);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      if (x != nullptr)
        {
          std::vector<double> x_vec(x, x + cols);
          (*context)["x" + str] = x_vec;
          subGroup->setAttribute("x", "x" + str);
        }

      if (y != nullptr)
        {
          std::vector<double> y_vec(y, y + rows);
          (*context)["y" + str] = y_vec;
          subGroup->setAttribute("y", "y" + str);
        }

      std::vector<double> z_vec(z, z + z_length);
      (*context)["z" + str] = z_vec;
      subGroup->setAttribute("z", "z" + str);

      group->parentElement()->setAttribute("z_log", z_log);

      if (x == nullptr && y == nullptr)
        {
          /* If neither `x` nor `y` are given, we need more information about the shape of `z` */
          grm_args_values(*current_series, "z_dims", "ii", &cols, &rows);

          auto z_dims_vec = std::vector<int>{(int)cols, (int)rows};
          auto z_dims_key = "z_dims" + str;
          (*context)[z_dims_key] = z_dims_vec;
          subGroup->setAttribute("z_dims", z_dims_key);
        }
      if (x == nullptr)
        {
          if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
            {
              subGroup->setAttribute("x_range_min", x_min);
              subGroup->setAttribute("x_range_max", x_max);
            }
        }
      if (y == nullptr)
        {
          if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
            {
              subGroup->setAttribute("y_range_min", y_min);
              subGroup->setAttribute("y_range_max", y_max);
            }
        }
      if (grm_args_values(*current_series, "z_range", "dd", &z_min, &z_max))
        {
          subGroup->setAttribute("z_range_min", z_min);
          subGroup->setAttribute("z_range_max", z_max);
        }
      if (grm_args_values(*current_series, "c_range", "dd", &c_min, &c_max))
        {
          subGroup->setAttribute("c_range_min", c_min);
          subGroup->setAttribute("c_range_max", c_max);
        }

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }

  plot_draw_colorbar(subplot_args, PLOT_POLAR_COLORBAR_OFFSET, 256);
  return error;
}

err_t plot_heatmap(grm_args_t *subplot_args)
{
  const char *kind = nullptr;
  grm_args_t **current_series;
  int z_log = 0;
  unsigned int i, cols, rows, z_length;
  double *x = nullptr, *y = nullptr, *z, x_min, x_max, y_min, y_max, z_min, z_max, c_min, c_max;
  err_t error = ERROR_NONE;
  std::shared_ptr<GRM::Element> plot_parent;

  std::shared_ptr<GRM::Element> group =
      (current_central_region_element) ? current_central_region_element : getCentralRegion();

  if (group->parentElement()->localName() == "plot")
    {
      plot_parent = group->parentElement();
    }
  else
    {
      plot_parent = group->parentElement()->parentElement();
    }

  grm_args_values(subplot_args, "series", "A", &current_series);
  grm_args_values(subplot_args, "kind", "s", &kind);
  grm_args_values(subplot_args, "z_log", "i", &z_log);

  if (strcmp(kind, "nonuniformheatmap") == 0)
    group->setAttribute(
        "kind",
        "nonuniform_heatmap"); // temporary workaround for name changes affecting gr-test (gr-test update needed)
  while (*current_series != nullptr)
    {
      x = y = nullptr;
      auto subGroup = global_render->createSeries("heatmap");
      group->append(subGroup);
      grm_args_first_value(*current_series, "x", "D", &x, &cols);
      grm_args_first_value(*current_series, "y", "D", &y, &rows);
      grm_args_first_value(*current_series, "z", "D", &z, &z_length);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      if (x != nullptr)
        {
          std::vector<double> x_vec(x, x + cols);
          (*context)["x" + str] = x_vec;
          subGroup->setAttribute("x", "x" + str);
        }

      if (y != nullptr)
        {
          std::vector<double> y_vec(y, y + rows);
          (*context)["y" + str] = y_vec;
          subGroup->setAttribute("y", "y" + str);
        }

      std::vector<double> z_vec(z, z + z_length);
      (*context)["z" + str] = z_vec;
      subGroup->setAttribute("z", "z" + str);

      plot_parent->setAttribute("z_log", z_log);

      if (x == nullptr && y == nullptr)
        {
          /* If neither `x` nor `y` are given, we need more information about the shape of `z` */
          grm_args_values(*current_series, "z_dims", "ii", &cols, &rows);

          auto z_dims_vec = std::vector<int>{(int)cols, (int)rows};
          auto z_dims_key = "z_dims" + str;
          (*context)[z_dims_key] = z_dims_vec;
          subGroup->setAttribute("z_dims", z_dims_key);
        }
      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          subGroup->setAttribute("x_range_min", x_min);
          subGroup->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          subGroup->setAttribute("y_range_min", y_min);
          subGroup->setAttribute("y_range_max", y_max);
        }
      if (grm_args_values(*current_series, "z_range", "dd", &z_min, &z_max))
        {
          subGroup->setAttribute("z_range_min", z_min);
          subGroup->setAttribute("z_range_max", z_max);
        }
      if (grm_args_values(*current_series, "c_range", "dd", &c_min, &c_max))
        {
          subGroup->setAttribute("c_range_min", c_min);
          subGroup->setAttribute("c_range_max", c_max);
        }

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }

  if (strcmp(kind, "marginal_heatmap") != 0)
    {
      plot_draw_colorbar(subplot_args, 0.0, 256);
    }


  return error;
}

err_t plot_marginal_heatmap(grm_args_t *subplot_args)
{
  int k, x_ind = -1, y_ind = -1, z_log = 0;
  const char *marginal_heatmap_kind = "all";
  err_t error = ERROR_NONE;
  grm_args_t **current_series;
  double *x, *y, *plot;
  unsigned int num_bins_x, num_bins_y, n;

  std::shared_ptr<GRM::Element> group = (current_dom_element) ? current_dom_element : edit_figure->lastChildElement();
  auto subGroup = global_render->querySelectors("marginal_heatmap_plot");
  if (subGroup == nullptr)
    {
      subGroup = global_render->createElement("marginal_heatmap_plot");
      group->append(subGroup);
    }

  grm_args_values(subplot_args, "z_log", "i", &z_log);
  group->setAttribute("z_log", z_log);

  if (grm_args_values(subplot_args, "marginal_heatmap_kind", "s", &marginal_heatmap_kind))
    subGroup->setAttribute("marginal_heatmap_kind", marginal_heatmap_kind);
  if (grm_args_values(subplot_args, "x_ind", "i", &x_ind)) subGroup->setAttribute("x_ind", x_ind);
  if (grm_args_values(subplot_args, "y_ind", "i", &y_ind)) subGroup->setAttribute("y_ind", y_ind);
  subGroup->setAttribute("kind", "marginal_heatmap");

  grm_args_values(subplot_args, "series", "A", &current_series);
  grm_args_first_value(*current_series, "x", "D", &x, &num_bins_x);
  grm_args_first_value(*current_series, "y", "D", &y, &num_bins_y);
  grm_args_first_value(*current_series, "z", "D", &plot, &n);

  int id = static_cast<int>(global_root->getAttribute("_id"));
  std::string str = std::to_string(id);
  auto context = global_render->getContext();

  std::vector<double> x_vec(x, x + num_bins_x);
  (*context)["x" + str] = x_vec;
  subGroup->setAttribute("x", "x" + str);

  std::vector<double> y_vec(y, y + num_bins_y);
  (*context)["y" + str] = y_vec;
  subGroup->setAttribute("y", "y" + str);

  std::vector<double> z_vec(plot, plot + n);
  (*context)["z" + str] = z_vec;
  subGroup->setAttribute("z", "z" + str);

  if (strcmp(marginal_heatmap_kind, "all") == 0)
    {
      const char *algorithm;
      if (grm_args_values(*current_series, "algorithm", "s", &algorithm))
        subGroup->setAttribute("algorithm", algorithm);
    }

  std::shared_ptr<GRM::Element> top_side_region;
  if (!subGroup->querySelectors("side_region[location=\"top\"]"))
    {
      top_side_region = global_render->createSideRegion("top");
      subGroup->append(top_side_region);
    }
  else
    {
      top_side_region = subGroup->querySelectors("side_region[location=\"top\"]");
    }
  top_side_region->setAttribute("marginal_heatmap_side_plot", 1);
  auto right_side_region = global_render->createSideRegion("right");
  right_side_region->setAttribute("marginal_heatmap_side_plot", 1);
  subGroup->append(right_side_region);

  grm_args_push(subplot_args, "kind", "s", "marginal_heatmap");
  global_root->setAttribute("_id", ++id);

  return error;
}

err_t plot_wireframe(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  err_t error = ERROR_NONE;

  std::shared_ptr<GRM::Element> group =
      (current_central_region_element) ? current_central_region_element : getCentralRegion();

  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double *x, *y, *z;
      unsigned int x_length, y_length, z_length;
      double x_min, x_max, y_min, y_max;

      auto subGroup = global_render->createSeries("wireframe");
      group->append(subGroup);

      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      grm_args_first_value(*current_series, "y", "D", &y, &y_length);
      grm_args_first_value(*current_series, "z", "D", &z, &z_length);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> x_vec(x, x + x_length);
      (*context)["x" + str] = x_vec;
      subGroup->setAttribute("x", "x" + str);

      std::vector<double> y_vec(y, y + y_length);
      (*context)["y" + str] = y_vec;
      subGroup->setAttribute("y", "y" + str);

      std::vector<double> z_vec(z, z + z_length);
      (*context)["z" + str] = z_vec;
      subGroup->setAttribute("z", "z" + str);

      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          subGroup->setAttribute("x_range_min", x_min);
          subGroup->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          subGroup->setAttribute("y_range_min", y_min);
          subGroup->setAttribute("y_range_max", y_max);
        }

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }
  plot_draw_axes(subplot_args, 2);

  return error;
}

err_t plot_surface(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  err_t error = ERROR_NONE;
  int accelerate; /* this argument decides if GR3 or GRM is used to plot the surface */
  double xmin, xmax, ymin, ymax;

  std::shared_ptr<GRM::Element> group =
      (current_central_region_element) ? current_central_region_element : getCentralRegion();

  grm_args_values(subplot_args, "series", "A", &current_series);
  bool has_accelerate = grm_args_values(subplot_args, "accelerate", "i", &accelerate);

  while (*current_series != nullptr)
    {
      double *x = nullptr, *y = nullptr, *z = nullptr;
      unsigned int x_length, y_length, z_length;

      auto subGroup = global_render->createSeries("surface");
      group->append(subGroup);
      if (has_accelerate) subGroup->setAttribute("accelerate", accelerate);

      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      grm_args_first_value(*current_series, "y", "D", &y, &y_length);
      grm_args_first_value(*current_series, "z", "D", &z, &z_length);
      grm_args_first_value(*current_series, "z", "D", &z, &z_length);

      if (grm_args_values(*current_series, "z_dims", "ii", &x_length, &y_length))
        {
          int id = static_cast<int>(global_root->getAttribute("_id"));
          std::string str = std::to_string(id);
          auto context = global_render->getContext();
          global_root->setAttribute("_id", ++id);

          auto z_dims_vec = std::vector<int>{(int)x_length, (int)y_length};
          auto z_dims_key = "z_dims" + str;
          (*context)[z_dims_key] = z_dims_vec;
          subGroup->setAttribute("z_dims", z_dims_key);
        }
      if (grm_args_values(*current_series, "x_range", "dd", &xmin, &xmax))
        {
          subGroup->setAttribute("x_range_min", xmin);
          subGroup->setAttribute("x_range_max", xmax);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &ymin, &ymax))
        {
          subGroup->setAttribute("y_range_min", ymin);
          subGroup->setAttribute("y_range_max", ymax);
        }

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      if (x != nullptr)
        {
          std::vector<double> x_vec(x, x + x_length);
          (*context)["x" + str] = x_vec;
          subGroup->setAttribute("x", "x" + str);
        }

      if (y != nullptr)
        {
          std::vector<double> y_vec(y, y + y_length);
          (*context)["y" + str] = y_vec;
          subGroup->setAttribute("y", "y" + str);
        }

      std::vector<double> z_vec(z, z + z_length);
      (*context)["z" + str] = z_vec;
      subGroup->setAttribute("z", "z" + str);

      global_root->setAttribute("_id", id++);
      ++current_series;
    }
  plot_draw_axes(subplot_args, 2);
  plot_draw_colorbar(subplot_args, PLOT_3D_COLORBAR_OFFSET, 256);

  return error;
}

err_t plot_plot3(grm_args_t *subplot_args)
{
  grm_args_t **current_series;

  std::shared_ptr<GRM::Element> group =
      (current_central_region_element) ? current_central_region_element : getCentralRegion();
  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double *x, *y, *z;
      unsigned int x_length, y_length, z_length;
      double x_min, x_max, y_min, y_max, z_min, z_max;
      auto subGroup = global_render->createSeries("plot3");
      group->append(subGroup);
      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      grm_args_first_value(*current_series, "y", "D", &y, &y_length);
      grm_args_first_value(*current_series, "z", "D", &z, &z_length);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> x_vec(x, x + x_length);
      (*context)["x" + str] = x_vec;
      subGroup->setAttribute("x", "x" + str);

      std::vector<double> y_vec(y, y + y_length);
      (*context)["y" + str] = y_vec;
      subGroup->setAttribute("y", "y" + str);

      std::vector<double> z_vec(z, z + z_length);
      (*context)["z" + str] = z_vec;
      subGroup->setAttribute("z", "z" + str);

      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          subGroup->setAttribute("x_range_min", x_min);
          subGroup->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          subGroup->setAttribute("y_range_min", y_min);
          subGroup->setAttribute("y_range_max", y_max);
        }
      if (grm_args_values(*current_series, "z_range", "dd", &z_min, &z_max))
        {
          subGroup->setAttribute("z_range_min", z_min);
          subGroup->setAttribute("z_range_max", z_max);
        }

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }
  plot_draw_axes(subplot_args, 2);

  return ERROR_NONE;
}

err_t plot_scatter3(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  double c_min, c_max;
  unsigned int x_length, y_length, z_length, c_length, i, c_index;
  double *x, *y, *z, *c;

  std::shared_ptr<GRM::Element> group =
      (current_central_region_element) ? current_central_region_element : getCentralRegion();
  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double x_min, x_max, y_min, y_max, z_min, z_max;
      auto subGroup = global_render->createSeries("scatter3");
      group->append(subGroup);
      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      grm_args_first_value(*current_series, "y", "D", &y, &y_length);
      grm_args_first_value(*current_series, "z", "D", &z, &z_length);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> x_vec(x, x + x_length);
      std::vector<double> y_vec(y, y + y_length);
      std::vector<double> z_vec(z, z + z_length);

      (*context)["x" + str] = x_vec;
      subGroup->setAttribute("x", "x" + str);
      (*context)["y" + str] = y_vec;
      subGroup->setAttribute("y", "y" + str);
      (*context)["z" + str] = z_vec;
      subGroup->setAttribute("z", "z" + str);

      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          subGroup->setAttribute("x_range_min", x_min);
          subGroup->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          subGroup->setAttribute("y_range_min", y_min);
          subGroup->setAttribute("y_range_max", y_max);
        }
      if (grm_args_values(*current_series, "z_range", "dd", &z_min, &z_max))
        {
          subGroup->setAttribute("z_range_min", z_min);
          subGroup->setAttribute("z_range_max", z_max);
        }

      if (grm_args_first_value(*current_series, "c", "D", &c, &c_length))
        {
          std::vector<double> c_vec(c, c + c_length);
          (*context)["c" + str] = c_vec;
          subGroup->setAttribute("c", "c" + str);

          if (grm_args_values(subplot_args, "c_lim", "dd", &c_min, &c_max))
            {
              group->parentElement()->setAttribute("c_lim_min", c_min);
              group->parentElement()->setAttribute("c_lim_max", c_max);
            }
        }

      global_root->setAttribute("_id", id++);
      ++current_series;
    }
  plot_draw_axes(subplot_args, 2);

  return ERROR_NONE;
}

err_t plot_imshow(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  double *c_data;
  double c_min, c_max;
  unsigned int c_data_length, i, j, k;
  unsigned int *shape;

  std::shared_ptr<GRM::Element> group =
      (current_central_region_element) ? current_central_region_element : getCentralRegion();

  grm_args_values(subplot_args, "series", "A", &current_series);
  if (grm_args_values(subplot_args, "c_lim", "dd", &c_min, &c_max))
    {
      group->parentElement()->setAttribute("z_lim_min", c_min);
      group->parentElement()->setAttribute("z_lim_max", c_max);
    }
  while (*current_series != nullptr)
    {
      auto subGroup = global_render->createSeries("imshow");
      group->append(subGroup);

      grm_args_first_value(*current_series, "c", "D", &c_data, &c_data_length);
      grm_args_first_value(*current_series, "c_dims", "I", &shape, &i);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> c_data_vec(c_data, c_data + c_data_length);
      std::vector<int> shape_vec(shape, shape + i);

      (*context)["z" + str] = c_data_vec;
      subGroup->setAttribute("z", "z" + str);
      (*context)["z_dims" + str] = shape_vec;
      subGroup->setAttribute("z_dims", "z_dims" + str);

      global_root->setAttribute("_id", id++);
      ++current_series;
    }

  return ERROR_NONE;
}

err_t plot_isosurface(grm_args_t *subplot_args)
{
  /*
   * Possible arguments to pass:
   * double[] `c` with the isodata to be drawn
   * int[3] `c_dims` shape of the c-array
   * optional double `isovalue` of the surface. All values higher or equal to isovalue are seen as inside the object,
   * all values below the isovalue are seen as outside the object.
   * optional double[3] `foreground_color`: color of the surface
   */
  grm_args_t **current_series;
  double *orig_data, *temp_colors;
  unsigned int i, data_length, dims;
  unsigned int *shape;
  double c_min, c_max, isovalue;

  std::shared_ptr<GRM::Element> group =
      (current_central_region_element) ? current_central_region_element : getCentralRegion();

  grm_args_values(subplot_args, "series", "A", &current_series);

  while (*current_series != nullptr)
    {
      auto subGroup = global_render->createSeries("isosurface");
      group->append(subGroup);
      grm_args_first_value(*current_series, "c", "D", &orig_data, &data_length);
      grm_args_first_value(*current_series, "c_dims", "I", &shape, &dims);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> c_data_vec(orig_data, orig_data + data_length);
      std::vector<int> shape_vec(shape, shape + dims);

      (*context)["z" + str] = c_data_vec;
      subGroup->setAttribute("z", "z" + str);
      (*context)["z_dims" + str] = shape_vec;
      subGroup->setAttribute("z_dims", "z_dims" + str);

      if (grm_args_values(*current_series, "isovalue", "d", &isovalue)) subGroup->setAttribute("isovalue", isovalue);
      /*
       * We need to convert the double values to floats, as GR3 expects floats, but an argument can only contain
       * doubles.
       */
      if (grm_args_first_value(*current_series, "foreground_color", "D", &temp_colors, &i))
        {
          std::vector<double> foreground_vec(temp_colors, temp_colors + i);
          (*context)["c_rgb" + str] = foreground_vec;
          subGroup->setAttribute("color_rgb_values", "c_rgb" + str);
        }

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }

  return ERROR_NONE;
}

err_t plot_volume(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  err_t error;

  std::shared_ptr<GRM::Element> group =
      (current_central_region_element) ? current_central_region_element : getCentralRegion();

  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      auto subGroup = global_render->createSeries("volume");
      group->append(subGroup);
      const double *c;
      unsigned int data_length, dims;
      unsigned int *shape;
      int algorithm;
      const char *algorithm_str;
      double d_min, d_max;
      double x_min, x_max, y_min, y_max, z_min, z_max;

      grm_args_first_value(*current_series, "c", "D", &c, &data_length);
      grm_args_first_value(*current_series, "c_dims", "I", &shape, &dims);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> c_data_vec(c, c + data_length);
      std::vector<int> shape_vec(shape, shape + dims);

      (*context)["z" + str] = c_data_vec;
      subGroup->setAttribute("z", "z" + str);
      (*context)["z_dims" + str] = shape_vec;
      subGroup->setAttribute("z_dims", "z_dims" + str);

      if (!grm_args_values(*current_series, "algorithm", "i", &algorithm))
        {
          if (grm_args_values(*current_series, "algorithm", "s", &algorithm_str))
            subGroup->setAttribute("algorithm", algorithm_str);
        }
      else
        {
          subGroup->setAttribute("algorithm", algorithm);
        }

      d_min = d_max = -1.0;
      grm_args_values(*current_series, "d_min", "d", &d_min);
      grm_args_values(*current_series, "d_max", "d", &d_max);
      subGroup->setAttribute("d_min", d_min);
      subGroup->setAttribute("d_max", d_max);

      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          subGroup->setAttribute("x_range_min", x_min);
          subGroup->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          subGroup->setAttribute("y_range_min", y_min);
          subGroup->setAttribute("y_range_max", y_max);
        }
      if (grm_args_values(*current_series, "z_range", "dd", &z_min, &z_max))
        {
          subGroup->setAttribute("z_range_min", z_min);
          subGroup->setAttribute("z_range_max", z_max);
        }

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }

  error = plot_draw_axes(subplot_args, 2);
  return_if_error;
  error = plot_draw_colorbar(subplot_args, PLOT_3D_COLORBAR_OFFSET, 256);
  return_if_error;

  return ERROR_NONE;
}

err_t plot_polar(grm_args_t *subplot_args)
{
  grm_args_t **current_series;

  std::shared_ptr<GRM::Element> group =
      (current_central_region_element) ? current_central_region_element : getCentralRegion();

  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double *rho, *theta;
      double y_min, y_max, x_min, x_max;
      unsigned int rho_length, theta_length;
      char *spec;
      auto subGroup = global_render->createSeries("polar");
      int clip_negative = 0;
      group->append(subGroup);

      grm_args_first_value(*current_series, "x", "D", &theta, &theta_length);
      grm_args_first_value(*current_series, "y", "D", &rho, &rho_length);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> theta_vec(theta, theta + theta_length);
      std::vector<double> rho_vec(rho, rho + rho_length);

      (*context)["x" + str] = theta_vec;
      subGroup->setAttribute("x", "x" + str);
      (*context)["y" + str] = rho_vec;
      subGroup->setAttribute("y", "y" + str);

      if (grm_args_values(*current_series, "line_spec", "s", &spec)) subGroup->setAttribute("spec", spec);
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          subGroup->setAttribute("y_range_min", y_min);
          subGroup->setAttribute("y_range_max", y_max);
        }
      if (grm_args_values(*current_series, "clip_negative", "i", &clip_negative))
        {
          subGroup->setAttribute("clip_negative", clip_negative);
        }
      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          subGroup->setAttribute("x_range_min", x_min);
          subGroup->setAttribute("x_range_max", x_max);
        }

      global_root->setAttribute("_id", id++);
      ++current_series;
    }

  return ERROR_NONE;
}

/*!
 * Plot a polar histogram.
 *
 * \param[in] x (series)
 *            Either a list containing doubles representing angles or ints for bin_counts.
 * \param[in] normalization a string specifying the normalization (subplot)
 *            The default value is "count".
 *          + count: the default normalization.  The height of each bin is the number of observations in each bin.
 *          + probability: The height of each bin is the relative number of observations,
 *            (number of observations in bin/total number of observations).
 *            The sum of the bin heights is 1.
 *          + countdensity: The height of each bin is the number of observations in bin / (width of bin).
 *          + pdf: Probability density function estimate. The height of each bin is
 *            (number of observations in the bin)/(total number of observations * width of bin).
 *            The area of each bin is the relative number of observations. The sum of the bin areas is 1
 *          + cumcount: The height of each bin is the cumulative number of observations in each bin and all
 *            previous bins. The height of the last bin is numel (theta).
 *          + cdf: Cumulative density function estimate. The height of each bin is equal to the cumulative relative
 *            number of observations in the bin and all previous bins. The height of the last bin is 1.
 * \param[in] bin_counts an int which behaves like a boolean (series)
 *            0: the series contains angles.
 *            else: the given series contains the direct values for each bin.
 *            The default value is 0.
 * \param[in] bin_edges a list containing angles that represent the edges of the bins (series)
 *            It is not compatible with nbins or bin_width.
 *            When used with bin_counts: length of bin_edges must equal the length of bin_counts + 1.
 * \param[in] stairs an int which behaves like a boolean (series)
 *            0: everything will be drawn.
 *            else: only the plot outlines will be drawn.
 *            The default value is 0 and stairs with a value unequal 0 is not compatible with colormap.
 * \param[in] colormap a list with two ints which represent a gr colormap (series)
 *            The first int sets the angle colormap and the second one the radius colormap.
 *            Must be between (inclusive) -1 and 47.
 *            -1: no colormap
 * \param[in] draw_edges an int that acts like a boolean (series)
 *            0: bin edges will not be drawn.
 *            else: bin edges will be drawn.
 *            It is only compatible with colormap.
 *            The default value is 0.
 * \param[in] phi_lim a list containing two double values representing start and end angle (series)
 *            The plot will only use the values in the given range and and it will plot bins between those two values.
 * \param[in] phi_flip an integer that represents a boolean (subplot)
 *            0: the angles will be displayed anti clockwise.
 *            else: the angles will be displayed clockwise.
 *            If phi_lim[0] is greater than phi_lim[1], phi_flip will be negated.
 *            The default value is 0.
 * \param[in] r_lim a list containing two double values between (inclusive) 0.0 and 1.0 (series)
 *            0.0 is the center and 1.0 is the outer edge of the plot.
 *            r_lim will limit the bins' start and end height.
 * \param[in] bin_width a double value between (exclusive) 0.0 and 2 * pi setting the bins' width (series)
 *            It is not compatible nbins or bin_edges.
 * \param[in] num_bins an int setting the number of bins (series)
 *            It is not compatible with bin_edges, nbins or bin_counts.
 * \param[in] face_alpha double value between 0.0 and 1.0 inclusive (series)
 *            Sets the opacity of bins.
 *            A value of 1.0 means fully opaque and 0.0 means completely transparent (invisible).
 *            The default value is 0.75.
 * \param[in] face_color sets the fill color of bins (series)
 *            face_color expects an integer between 0 and 1008.
 *            The default value is 989.
 * \param[in] edge_color sets the color of the bin edges (series)
 *            edge_color expects an integer between 0 and 1008.
 *            The default value is 1.
 *
 * */
err_t plot_polar_histogram(grm_args_t *subplot_args)
{
  double *r_lim = nullptr;
  unsigned int dummy;
  int stairs;
  int keep_radii_axes;
  int x_colormap, y_colormap;
  int draw_edges, phi_flip, edge_color, face_color, face_alpha;
  double xrange_min, xrange_max, ylim_min, ylim_max;
  grm_args_t **series;

  std::shared_ptr<GRM::Element> plot_group = edit_figure->lastChildElement();
  std::shared_ptr<GRM::Element> group =
      (current_central_region_element) ? current_central_region_element : getCentralRegion();
  std::shared_ptr<GRM::Element> series_group = global_render->createSeries("polar_histogram");
  group->append(series_group);

  // Call classes -> set attributes and data
  classes_polar_histogram(subplot_args);

  auto context = global_render->getContext();

  int id = static_cast<int>(global_root->getAttribute("_id"));
  std::string str = std::to_string(id);

  grm_args_values(subplot_args, "series", "A", &series);

  /* edge_color */
  if (grm_args_values(*series, "edge_color", "i", &edge_color))
    {
      series_group->setAttribute("line_color_ind", edge_color);
    }

  /* face_color */
  if (grm_args_values(*series, "face_color", "i", &face_color))
    {
      series_group->setAttribute("color_ind", face_color);
    }

  /* face_alpha */
  if (grm_args_values(*series, "face_alpha", "d", &face_alpha))
    {
      series_group->setAttribute("face_alpha", face_alpha);
    }

  if (grm_args_values(subplot_args, "phi_flip", "i", &phi_flip))
    {
      plot_group->setAttribute("phi_flip", phi_flip);
    }

  if (grm_args_values(subplot_args, "keep_radii_axes", "i", &keep_radii_axes))
    {
      plot_group->setAttribute("keep_radii_axes", keep_radii_axes);
    }

  if (grm_args_values(*series, "draw_edges", "i", &draw_edges))
    {
      series_group->setAttribute("draw_edges", draw_edges);
    }

  if (grm_args_values(*series, "stairs", "i", &stairs))
    {
      series_group->setAttribute("stairs", stairs);
    }

  if (grm_args_first_value(*series, "r_lim", "D", &r_lim, &dummy))
    {
      plot_group->setAttribute("r_lim_min", r_lim[0]);
      plot_group->setAttribute("r_lim_max", r_lim[1]);
    }

  if (grm_args_values(subplot_args, "y_lim", "dd", &ylim_min, &ylim_max))
    {
      plot_group->setAttribute("y_lim_min", ylim_min);
      plot_group->setAttribute("y_lim_max", ylim_max);
    }

  if (grm_args_values(*series, "x_range", "dd", &xrange_min, &xrange_max))
    {
      series_group->setAttribute("x_range_min", xrange_min);
      series_group->setAttribute("x_range_max", xrange_max);
    }

  if (grm_args_values(*series, "x_colormap", "i", &x_colormap))
    {
      series_group->setAttribute("x_colormap", x_colormap);
    }
  if (grm_args_values(*series, "y_colormap", "i", &y_colormap))
    {
      series_group->setAttribute("y_colormap", y_colormap);
    }
  global_root->setAttribute("_id", id++);

  return ERROR_NONE;
}

err_t plot_pie(grm_args_t *subplot_args)
{
  grm_args_t *series;
  double *x;
  unsigned int x_length;
  const char *title;
  static unsigned int color_array_length = -1;
  const int *color_indices = nullptr;
  const double *color_rgb_values = nullptr;

  std::shared_ptr<GRM::Element> group =
      (current_central_region_element) ? current_central_region_element : getCentralRegion();

  grm_args_values(subplot_args, "series", "a", &series); /* series exists always */

  auto subGroup = global_render->createSeries("pie");
  group->append(subGroup);

  int id = static_cast<int>(global_root->getAttribute("_id"));
  std::string str = std::to_string(id);
  auto context = global_render->getContext();

  grm_args_first_value(series, "x", "D", &x, &x_length);

  if (x_length > 0)
    {
      std::vector<double> x_vec(x, x + x_length);
      (*context)["x" + str] = x_vec;
      subGroup->setAttribute("x", "x" + str);
    }

  if (grm_args_first_value(series, "c", "I", &color_indices, &color_array_length))
    {
      std::vector<double> color_index_vec(color_indices, color_indices + color_array_length);
      (*context)["c_ind" + str] = color_index_vec;
      subGroup->setAttribute("color_ind_values", "c_ind" + str);
    }
  else if (grm_args_first_value(series, "c", "D", &color_rgb_values, &color_array_length))
    {
      std::vector<double> color_rgb_vec(color_rgb_values, color_rgb_values + color_array_length);
      (*context)["c_rgb" + str] = color_rgb_vec;
      subGroup->setAttribute("color_rgb_values", "c_rgb" + str);
    }
  if (grm_args_values(subplot_args, "title", "s", &title))
    {
      std::shared_ptr<GRM::Element> side_region;
      if (!group->parentElement()->querySelectors("side_region[location=\"top\"]"))
        {
          side_region = global_render->createElement("side_region");
          group->parentElement()->append(side_region);
        }
      else
        {
          side_region = group->parentElement()->querySelectors("side_region[location=\"top\"]");
        }
      side_region->setAttribute("text_content", title);
      side_region->setAttribute("location", "top");
      side_region->setAttribute("text_is_title", true);
    }
  global_root->setAttribute("_id", id++);

  return ERROR_NONE;
}

err_t plot_trisurface(grm_args_t *subplot_args)
{
  grm_args_t **current_series;

  std::shared_ptr<GRM::Element> group =
      (current_central_region_element) ? current_central_region_element : getCentralRegion();
  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double *x, *y, *z;
      unsigned int x_length, y_length, z_length;
      double x_min, x_max, y_min, y_max, z_min, z_max;
      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      grm_args_first_value(*current_series, "y", "D", &y, &y_length);
      grm_args_first_value(*current_series, "z", "D", &z, &z_length);

      int id = (int)global_root->getAttribute("_id");
      std::string str = std::to_string(id);

      std::vector<double> x_vec(x, x + x_length), y_vec(y, y + x_length), z_vec(z, z + x_length);
      auto temp = global_render->createTriSurface("x" + str, x_vec, "y" + str, y_vec, "z" + str, z_vec);

      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          temp->setAttribute("x_range_min", x_min);
          temp->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          temp->setAttribute("y_range_min", y_min);
          temp->setAttribute("y_range_max", y_max);
        }
      if (grm_args_values(*current_series, "z_range", "dd", &z_min, &z_max))
        {
          temp->setAttribute("z_range_min", z_min);
          temp->setAttribute("z_range_max", z_max);
        }

      group->append(temp);

      global_root->setAttribute("_id", id++);
      ++current_series;
    }
  plot_draw_axes(subplot_args, 2);
  plot_draw_colorbar(subplot_args, PLOT_3D_COLORBAR_OFFSET, 256);

  return ERROR_NONE;
}

err_t plot_tricontour(grm_args_t *subplot_args)
{
  double z_min, z_max;
  double *levels;
  int num_levels;
  grm_args_t **current_series;
  int i;

  std::shared_ptr<GRM::Element> group =
      (current_central_region_element) ? current_central_region_element : getCentralRegion();

  bool has_levels = grm_args_values(subplot_args, "levels", "i", &num_levels);
  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double *x, *y, *z;
      unsigned int x_length, y_length, z_length;
      double x_min, x_max, y_min, y_max;
      auto subGroup = global_render->createSeries("tricontour");
      group->append(subGroup);

      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      grm_args_first_value(*current_series, "y", "D", &y, &y_length);
      grm_args_first_value(*current_series, "z", "D", &z, &z_length);

      int id = (int)global_root->getAttribute("_id");
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> x_vec(x, x + x_length);
      (*context)["x" + str] = x_vec;
      subGroup->setAttribute("x", "x" + str);

      std::vector<double> y_vec(y, y + y_length);
      (*context)["y" + str] = y_vec;
      subGroup->setAttribute("y", "y" + str);

      std::vector<double> z_vec(z, z + z_length);
      (*context)["z" + str] = z_vec;
      subGroup->setAttribute("z", "z" + str);

      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          subGroup->setAttribute("x_range_min", x_min);
          subGroup->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          subGroup->setAttribute("y_range_min", y_min);
          subGroup->setAttribute("y_range_max", y_max);
        }
      if (grm_args_values(*current_series, "z_range", "dd", &z_min, &z_max))
        {
          subGroup->setAttribute("z_range_min", z_min);
          subGroup->setAttribute("z_range_max", z_max);
        }

      if (has_levels) subGroup->setAttribute("levels", num_levels);

      global_root->setAttribute("_id", id++);
      ++current_series;
    }
  plot_draw_colorbar(subplot_args, 0.0, 256);

  return ERROR_NONE;
}

err_t plot_shade(grm_args_t *subplot_args)
{
  grm_args_t **current_shader;
  /* char *spec = ""; TODO: read spec from data! */
  int transformation, x_bins, y_bins;
  double *x, *y;
  unsigned int x_length, y_length;
  double x_min, x_max, y_min, y_max;

  std::shared_ptr<GRM::Element> group =
      (current_central_region_element) ? current_central_region_element : getCentralRegion();

  grm_args_values(subplot_args, "series", "A", &current_shader);
  auto subGroup = global_render->createSeries("shade");
  group->append(subGroup);

  grm_args_first_value(*current_shader, "x", "D", &x, &x_length);
  grm_args_first_value(*current_shader, "y", "D", &y, &y_length);

  int id = static_cast<int>(global_root->getAttribute("_id"));
  std::string str = std::to_string(id);
  auto context = global_render->getContext();

  std::vector<double> x_vec(x, x + x_length);
  std::vector<double> y_vec(y, y + y_length);

  (*context)["x" + str] = x_vec;
  subGroup->setAttribute("x", "x" + str);
  (*context)["y" + str] = y_vec;
  subGroup->setAttribute("y", "y" + str);

  if (grm_args_values(subplot_args, "transformation", "i", &transformation))
    {
      subGroup->setAttribute("transformation", transformation);
    }
  if (grm_args_values(subplot_args, "x_bins", "i", &x_bins))
    {
      subGroup->setAttribute("x_bins", x_bins);
    }
  if (grm_args_values(subplot_args, "y_bins", "i", &y_bins))
    {
      subGroup->setAttribute("y_bins", y_bins);
    }

  if (grm_args_values(*current_shader, "x_range", "dd", &x_min, &x_max))
    {
      subGroup->setAttribute("x_range_min", x_min);
      subGroup->setAttribute("x_range_max", x_max);
    }
  if (grm_args_values(*current_shader, "y_range", "dd", &y_min, &y_max))
    {
      subGroup->setAttribute("y_range_min", y_min);
      subGroup->setAttribute("y_range_max", y_max);
    }
  global_root->setAttribute("_id", id++);

  return ERROR_NONE;
}

err_t plot_raw(grm_args_t *plot_args)
{
  const char *base64_data = nullptr;
  char *graphics_data = nullptr;
  err_t error = ERROR_NONE;
  std::vector<int> data_vec;

  cleanup_and_set_error_if(!grm_args_values(plot_args, "raw", "s", &base64_data), ERROR_PLOT_MISSING_DATA);
  graphics_data = base64_decode(nullptr, base64_data, nullptr, &error);
  cleanup_if_error;

  global_root->setAttribute("clear_ws", 1);
  data_vec = std::vector<int>(graphics_data, graphics_data + strlen(graphics_data));
  edit_figure->append(global_render->createDrawGraphics("graphics", data_vec));
  global_root->setAttribute("update_ws", 1);

cleanup:
  if (graphics_data != nullptr)
    {
      free(graphics_data);
    }

  return error;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ auxiliary drawing functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

err_t plot_draw_axes(grm_args_t *args, unsigned int pass)
{
  const char *kind = nullptr;
  int x_grid, y_grid, z_grid;
  char *title;
  char *x_label, *y_label, *z_label;
  std::shared_ptr<GRM::Element> group;
  std::string type = "2d";

  if (!current_central_region_element ||
      current_central_region_element->getElementsByTagName("coordinate_system").empty())
    {
      group = global_render->createElement("coordinate_system");
      if (!current_central_region_element)
        {
          getCentralRegion()->append(group);
        }
      else
        {
          current_central_region_element->append(group);
        }
    }
  else
    {
      group = global_render->getElementsByTagName("coordinate_system")[0];
    }
  grm_args_values(args, "kind", "s", &kind);
  grm_args_values(args, "x_grid", "i", &x_grid);
  grm_args_values(args, "y_grid", "i", &y_grid);

  group->setAttribute("x_grid", x_grid);
  group->setAttribute("y_grid", y_grid);

  if (str_equals_any(kind, "wireframe", "surface", "plot3", "scatter3", "trisurface", "volume", "isosurface"))
    {
      type = "3d";
      grm_args_values(args, "z_grid", "i", &z_grid);
      group->setAttribute("z_grid", z_grid);
      if (strcmp(kind, "isosurface") == 0)
        {
          group->setAttribute("hide", 1);
        }
    }
  else
    {
      if ((strcmp(kind, "barplot") != 0 && strcmp(kind, "imshow") != 0) || pass == 2)
        {
          /* xticklabels */
          char **x_tick_labels = nullptr;
          unsigned int x_tick_labels_length;

          if (grm_args_first_value(args, "x_tick_labels", "S", &x_tick_labels, &x_tick_labels_length))
            {
              std::vector<std::string> x_tick_labels_vec(x_tick_labels, x_tick_labels + x_tick_labels_length);
              int id = static_cast<int>(global_root->getAttribute("_id"));
              std::string key = "x_tick_labels" + std::to_string(id);
              global_root->setAttribute("_id", ++id);
              global_render->setXTickLabels(group, key, x_tick_labels_vec);
            }

          /* y_tick_labels */
          char **y_tick_labels = nullptr;
          unsigned int y_tick_labels_length;

          if (grm_args_first_value(args, "y_tick_labels", "S", &y_tick_labels, &y_tick_labels_length))
            {
              std::vector<std::string> y_tick_labels_vec(y_tick_labels, y_tick_labels + y_tick_labels_length);
              int id = static_cast<int>(global_root->getAttribute("_id"));
              std::string key = "y_tick_labels" + std::to_string(id);
              global_root->setAttribute("_id", ++id);
              global_render->setYTickLabels(group, key, y_tick_labels_vec);
            }
        }
      else if (strcmp(kind, "imshow") == 0)
        {
          group->setAttribute("hide", 1);
        }
    }

  group->setAttribute("plot_type", type);

  if (pass == 1 && grm_args_values(args, "title", "s", &title))
    {
      auto side_region = global_render->createElement("side_region");
      current_central_region_element->parentElement()->append(side_region);
      side_region->setAttribute("text_content", title);
      side_region->setAttribute("location", "top");
      side_region->setAttribute("text_is_title", true);
    }

  if (grm_args_values(args, "x_label", "s", &x_label))
    {
      if (type == "3d")
        {
          group->setAttribute("x_label", x_label);
        }
      else
        {
          if (!current_central_region_element->parentElement()->querySelectors("side_region[location=\"bottom\"]"))
            {
              auto side_region = global_render->createElement("side_region");
              current_central_region_element->parentElement()->append(side_region);
              side_region->setAttribute("text_content", x_label);
              side_region->setAttribute("location", "bottom");
            }
        }
    }
  if (grm_args_values(args, "y_label", "s", &y_label))
    {
      if (type == "3d")
        {
          group->setAttribute("y_label", y_label);
        }
      else
        {
          if (!current_central_region_element->parentElement()->querySelectors("side_region[location=\"left\"]"))
            {
              auto side_region = global_render->createElement("side_region");
              current_central_region_element->parentElement()->append(side_region);
              side_region->setAttribute("text_content", y_label);
              side_region->setAttribute("location", "left");
            }
        }
    }
  if (grm_args_values(args, "z_label", "s", &z_label))
    {
      group->setAttribute("z_label", z_label);
    }

  return ERROR_NONE;
}

err_t plot_draw_polar_axes(grm_args_t *args)
{
  char *kind;
  int angle_ticks;
  int phi_flip = 0;
  const char *norm, *title;
  std::shared_ptr<GRM::Element> group, subGroup;

  group = (current_central_region_element) ? current_central_region_element : getCentralRegion();

  if (global_render->getElementsByTagName("coordinate_system").empty())
    {
      subGroup = global_render->createElement("coordinate_system");
      group->append(subGroup);
    }
  else
    {
      subGroup = global_render->getElementsByTagName("coordinate_system")[0];
    }

  subGroup->setAttribute("plot_type", "polar");

  if (grm_args_values(args, "angle_ticks", "i", &angle_ticks))
    {
      subGroup->setAttribute("angle_ticks", angle_ticks);
    }

  grm_args_values(args, "kind", "s", &kind);

  if (strcmp(kind, "polar_histogram") == 0)
    {
      if (grm_args_values(args, "normalization", "s", &norm))
        {
          subGroup->setAttribute("normalization", norm);
        }
    }

  if (grm_args_values(args, "phi_flip", "i", &phi_flip))
    {
      subGroup->setAttribute("phi_flip", phi_flip);
    }

  if (grm_args_values(args, "title", "s", &title))
    {
      auto side_region = global_render->createElement("side_region");
      current_central_region_element->parentElement()->append(side_region);
      side_region->setAttribute("text_content", title);
      side_region->setAttribute("location", "top");
      side_region->setAttribute("text_is_title", true);
    }

  return ERROR_NONE;
}

err_t plot_draw_legend(grm_args_t *subplot_args)
{
  const char **labels;
  unsigned int num_labels, num_series;
  grm_args_t **current_series;
  int location;

  std::shared_ptr<GRM::Element> group = (current_dom_element) ? current_dom_element : edit_figure->lastChildElement();

  return_error_if(!grm_args_first_value(subplot_args, "labels", "S", &labels, &num_labels), ERROR_PLOT_MISSING_LABELS);
  logger((stderr, "Draw a legend with %d labels\n", num_labels));
  grm_args_first_value(subplot_args, "series", "A", &current_series, &num_series);

  int id = static_cast<int>(global_root->getAttribute("_id"));
  global_root->setAttribute("_id", ++id);

  std::string labels_key = std::to_string(id) + "labels";
  std::string specs_key = std::to_string(id) + "specs";
  std::vector<std::string> labels_vec(labels, labels + num_labels);

  std::vector<std::string> specs_vec;
  while (*current_series != nullptr)
    {
      char *spec;
      if (grm_args_values(*current_series, "line_spec", "s", &spec))
        {
          specs_vec.emplace_back(spec);
        }
      else
        {
          specs_vec.emplace_back("");
        }
      ++current_series;
    }

  auto legend = global_render->createLegend(labels_key, labels_vec, specs_key, specs_vec);
  if (grm_args_values(subplot_args, "location", "i", &location))
    {
      legend->setAttribute("location", location);
    }
  group->append(legend);

  return ERROR_NONE;
}

err_t plot_draw_pie_legend(grm_args_t *subplot_args)
{
  const char **labels;
  unsigned int num_labels;
  grm_args_t *series;

  std::shared_ptr<GRM::Element> group = (current_dom_element) ? current_dom_element : edit_figure->lastChildElement();

  return_error_if(!grm_args_first_value(subplot_args, "labels", "S", &labels, &num_labels), ERROR_PLOT_MISSING_LABELS);
  grm_args_values(subplot_args, "series", "a", &series); /* series exists always */

  int id = static_cast<int>(global_root->getAttribute("_id"));
  global_root->setAttribute("_id", id++);
  std::string labels_key = "labels" + std::to_string(id);
  std::vector<std::string> labels_vec(labels, labels + num_labels);

  auto draw_pie_legend_element = global_render->createPieLegend(labels_key, labels_vec);
  group->append(draw_pie_legend_element);

  return ERROR_NONE;
}

err_t plot_draw_colorbar(grm_args_t *subplot_args, double off, unsigned int colors)
{
  int *data;
  int flip;
  unsigned int i;

  std::shared_ptr<GRM::Element> group = (current_dom_element) ? current_dom_element : edit_figure->lastChildElement();

  auto side_region = global_render->createElement("side_region");
  group->append(side_region);
  auto side_plot_region = global_render->createElement("side_plot_region");
  side_region->append(side_plot_region);
  auto colorbar = global_render->createColorbar(colors);
  side_plot_region->append(colorbar);

  colorbar->setAttribute("x_flip", 0);
  colorbar->setAttribute("y_flip", 0);
  if (grm_args_values(subplot_args, "x_flip", "i", &flip) && flip)
    {
      colorbar->setAttribute("x_flip", flip);
    }
  else if (grm_args_values(subplot_args, "y_flip", "i", &flip) && flip)
    {
      colorbar->setAttribute("y_flip", flip);
    }

  side_region->setAttribute("offset", off + PLOT_DEFAULT_COLORBAR_OFFSET);
  colorbar->setAttribute("max_char_height", PLOT_DEFAULT_COLORBAR_MAX_CHAR_HEIGHT);
  side_region->setAttribute("location", PLOT_DEFAULT_COLORBAR_LOCATION);
  side_region->setAttribute("width", PLOT_DEFAULT_COLORBAR_WIDTH);

  return ERROR_NONE;
}

err_t extract_multi_type_argument(grm_args_t *error_container, const char *key, unsigned int x_length,
                                  unsigned int *downwards_length, unsigned int *upwards_length, double **downwards,
                                  double **upwards, double *downwards_flt, double *upwards_flt)
{
  arg_t *arg_ptr;
  grm_args_value_iterator_t *value_it;
  unsigned int length;
  int i, *ii;

  arg_ptr = args_at(error_container, key);
  if (!arg_ptr)
    {
      return ERROR_NONE;
    }
  if (strcmp(arg_ptr->value_format, "nDnD") == 0)
    {
      value_it = grm_arg_value_iter(arg_ptr);
      ARGS_VALUE_ITERATOR_GET(value_it, *downwards_length, *downwards);
      ARGS_VALUE_ITERATOR_GET(value_it, *upwards_length, *upwards);
      args_value_iterator_delete(value_it);

      return_error_if(*downwards_length != *upwards_length || *downwards_length != x_length,
                      ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
    }
  else if (strcmp(arg_ptr->value_format, "nD") == 0)
    {
      return_error_if(!grm_args_first_value(error_container, key, "D", downwards, downwards_length), ERROR_INTERNAL);
      /* Python encapsulates all single elements into an array */
      if (*downwards_length == 1)
        {
          *downwards_flt = *upwards_flt = **downwards;
          *downwards = nullptr;
          *downwards_length = 0;
          return ERROR_NONE;
        }
      return_error_if(*downwards_length != x_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
      *upwards = *downwards;
      *upwards_length = *downwards_length;
    }
  else if (strcmp(arg_ptr->value_format, "d") == 0)
    {
      return_error_if(!grm_args_values(error_container, key, "d", downwards_flt), ERROR_INTERNAL);
      *upwards_flt = *downwards_flt;
    }
  else if (strcmp(arg_ptr->value_format, "nI") == 0)
    {
      return_error_if(!grm_args_first_value(error_container, key, "nI", &ii, &length), ERROR_INTERNAL);
      return_error_if(length != 1, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
      *upwards_flt = *downwards_flt = (double)ii[0];
    }
  else if (strcmp(arg_ptr->value_format, "i") == 0)
    {
      return_error_if(!grm_args_values(error_container, key, "i", &i), ERROR_INTERNAL);
      *upwards_flt = *downwards_flt = (double)i;
    }
  return ERROR_NONE;
}

err_t plot_draw_error_bars(grm_args_t *series_args, unsigned int x_length)
{
  grm_args_t *error_container;
  arg_t *arg_ptr;
  err_t error;

  double *absolute_upwards = nullptr, *absolute_downwards = nullptr, *relative_upwards = nullptr,
         *relative_downwards = nullptr;
  double absolute_upwards_flt, relative_upwards_flt, absolute_downwards_flt, relative_downwards_flt;
  unsigned int upwards_length, downwards_length, i;
  int color_upwards_cap, color_downwards_cap, color_error_bar;

  std::shared_ptr<GRM::Element> group = (current_central_region_element)
                                            ? current_central_region_element->lastChildElement()
                                            : getCentralRegion()->lastChildElement();

  arg_ptr = args_at(series_args, "error");
  if (!arg_ptr)
    {
      return ERROR_NONE;
    }
  error_container = nullptr;

  auto subGroup = global_render->createElement("error_bars");
  group->append(subGroup);

  int id = static_cast<int>(global_root->getAttribute("_id"));
  std::string str = std::to_string(id);
  global_root->setAttribute("_id", ++id);
  auto context = global_render->getContext();

  if (strcmp(arg_ptr->value_format, "a") == 0 || strcmp(arg_ptr->value_format, "nA") == 0)
    {
      return_error_if(!grm_args_values(series_args, "error", "a", &error_container), ERROR_INTERNAL);

      error = extract_multi_type_argument(error_container, "absolute", x_length, &downwards_length, &upwards_length,
                                          &absolute_downwards, &absolute_upwards, &absolute_downwards_flt,
                                          &absolute_upwards_flt);
      return_if_error;
      error = extract_multi_type_argument(error_container, "relative", x_length, &downwards_length, &upwards_length,
                                          &relative_downwards, &relative_upwards, &relative_downwards_flt,
                                          &relative_upwards_flt);
      return_if_error;
    }
  else
    {
      error = extract_multi_type_argument(series_args, "error", x_length, &downwards_length, &upwards_length,
                                          &absolute_downwards, &absolute_upwards, &absolute_downwards_flt,
                                          &absolute_upwards_flt);
      return_if_error;
    }

  if (absolute_upwards == nullptr && relative_upwards == nullptr && absolute_upwards_flt == FLT_MAX &&
      relative_upwards_flt == FLT_MAX && absolute_downwards == nullptr && relative_downwards == nullptr &&
      absolute_downwards_flt == FLT_MAX && relative_downwards_flt == FLT_MAX)
    {
      return ERROR_PLOT_MISSING_DATA;
    }
  if (absolute_upwards != nullptr)
    {
      std::vector<double> absolute_upwards_vec(absolute_upwards, absolute_upwards + upwards_length);
      (*context)["absolute_upwards" + str] = absolute_upwards_vec;
      subGroup->setAttribute("absolute_upwards", "absolute_upwards" + str);
    }
  if (relative_upwards != nullptr)
    {
      std::vector<double> relative_upwards_vec(relative_upwards, relative_upwards + upwards_length);
      (*context)["relative_upwards" + str] = relative_upwards_vec;
      subGroup->setAttribute("relative_upwards", "relative_upwards" + str);
    }
  if (absolute_downwards != nullptr)
    {
      std::vector<double> absolute_downwards_vec(absolute_downwards, absolute_downwards + downwards_length);
      (*context)["absolute_downwards" + str] = absolute_downwards_vec;
      subGroup->setAttribute("absolute_downwards", "absolute_downwards" + str);
    }
  if (relative_downwards != nullptr)
    {
      std::vector<double> relative_downwards_vec(relative_downwards, relative_downwards + downwards_length);
      (*context)["relative_downwards" + str] = relative_downwards_vec;
      subGroup->setAttribute("relative_downwards", "relative_downwards" + str);
    }
  if (absolute_downwards_flt != FLT_MAX) subGroup->setAttribute("absolute_downwards_flt", absolute_downwards_flt);
  if (relative_downwards_flt != FLT_MAX) subGroup->setAttribute("relative_downwards_flt", relative_downwards_flt);
  if (absolute_upwards_flt != FLT_MAX) subGroup->setAttribute("absolute_upwards_flt", absolute_upwards_flt);
  if (relative_upwards_flt != FLT_MAX) subGroup->setAttribute("relative_upwards_flt", relative_upwards_flt);

  if (error_container != nullptr)
    {
      if (grm_args_values(error_container, "upwards_cap_color", "i", &color_upwards_cap))
        subGroup->setAttribute("upwards_cap_color", color_upwards_cap);
      if (grm_args_values(error_container, "downwards_cap_color", "i", &color_downwards_cap))
        subGroup->setAttribute("downwards_cap_color", color_downwards_cap);
      if (grm_args_values(error_container, "error_bar_color", "i", &color_error_bar))
        subGroup->setAttribute("error_bar_color", color_error_bar);
    }
  subGroup->setAttribute("z_index", 3);

  return ERROR_NONE;
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ util ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

const char *get_compatible_format(const char *key, const char *given_format)
{
  const char **valid_formats;
  char *reduced_given_format;
  const char **current_format_ptr;
  const char *compatible_format = nullptr;
  /* First, get all valid formats */
  if (!string_array_map_at(type_map, key, (char ***)&valid_formats))
    {
      /* If the given key does not exist, there is no type constraint
       * -> simply return the same type that was given */
      return given_format;
    }
  /* Second, filter the given format -> remove `n` chars because they are irrelevant for the following tests */
  reduced_given_format = str_filter(given_format, "n");
  if (reduced_given_format == nullptr)
    {
      debug_print_malloc_error();
      goto cleanup;
    }
  /* Third, iterate over all valid formats and check if
   * - there is an exact match
   * - a single format char is valid,
   *   the type matches (e.g. `d` and `D`) and
   *   a single value / array or multiple single values of the same type ar given (e.g. `dddd`)
   */
  current_format_ptr = valid_formats;
  while (*current_format_ptr != nullptr)
    {
      if (strcmp(*current_format_ptr, reduced_given_format) == 0)
        {
          /* Found an exact match */
          compatible_format = *current_format_ptr;
          break;
        }
      else if ((strlen(*current_format_ptr) == 1) && (tolower(**current_format_ptr) == tolower(*reduced_given_format)))
        {
          /* The current format is a single format of the same base type as the given format */
          if (strlen(reduced_given_format) == 1 ||
              is_homogenous_string_of_char(reduced_given_format, tolower(*reduced_given_format)))
            {
              /* Either a single format of the same type or
               * a homogenous format string of single values of the same type
               * is given */
              compatible_format = *current_format_ptr;
              break;
            }
        }
      ++current_format_ptr;
    }

cleanup:
  free(reduced_given_format);

  /* The previous tests were not successful -> `nullptr` indicates no compatible format */
  return compatible_format;
}

int get_id_from_args(const grm_args_t *args, int *plot_id, int *subplot_id, int *series_id)
{
  const char *combined_id;
  int _plot_id = -1, _subplot_id = 0, _series_id = 0;

  if (grm_args_values(args, "id", "s", &combined_id))
    {
      const char *valid_id_delims = ":.";
      int *id_ptrs[4], **current_id_ptr;
      char *copied_id_str, *current_id_str;
      size_t segment_length;
      int is_last_segment;

      id_ptrs[0] = &_plot_id;
      id_ptrs[1] = &_subplot_id;
      id_ptrs[2] = &_series_id;
      id_ptrs[3] = nullptr;
      if ((copied_id_str = gks_strdup(combined_id)) == nullptr)
        {
          debug_print_malloc_error();
          return 0;
        }

      current_id_ptr = id_ptrs;
      current_id_str = copied_id_str;
      is_last_segment = 0;
      while (*current_id_ptr != nullptr && !is_last_segment)
        {
          segment_length = strcspn(current_id_str, valid_id_delims);
          if (current_id_str[segment_length] == '\0')
            {
              is_last_segment = 1;
            }
          else
            {
              current_id_str[segment_length] = '\0';
            }
          if (*current_id_str != '\0')
            {
              if (!str_to_uint(current_id_str, (unsigned int *)*current_id_ptr))
                {
                  logger((stderr, "Got an invalid id \"%s\"\n", current_id_str));
                }
              else
                {
                  logger((stderr, "Read id: %d\n", **current_id_ptr));
                }
            }
          ++current_id_ptr;
          ++current_id_str;
        }

      free(copied_id_str);
    }
  else
    {
      grm_args_values(args, "plot_id", "i", &_plot_id);
      grm_args_values(args, "subplot_id", "i", &_subplot_id);
      grm_args_values(args, "series_id", "i", &_series_id);
    }
  /* plot id `0` references the first plot object (implicit object) -> handle it as the first plot and shift all ids by
   * one */
  *plot_id = _plot_id + 1;
  *subplot_id = _subplot_id;
  *series_id = _series_id;

  return _plot_id >= 0 || _subplot_id > 0 || _series_id > 0;
}

grm_args_t *get_subplot_from_ndc_point(double x, double y)
{
  grm_args_t **subplot_args;
  const double *viewport;

  grm_args_values(active_plot_args, "subplots", "A", &subplot_args);
  while (*subplot_args != nullptr)
    {
      if (grm_args_values(*subplot_args, "viewport", "D", &viewport))
        {
          if (viewport[0] <= x && x <= viewport[1] && viewport[2] <= y && y <= viewport[3])
            {
              unsigned int array_index;
              grm_args_values(*subplot_args, "array_index", "i", &array_index);
              logger((stderr, "Found subplot id \"%u\" for ndc point (%lf, %lf)\n", array_index + 1, x, y));

              return *subplot_args;
            }
        }
      ++subplot_args;
    }

  return nullptr;
}

grm_args_t *get_subplot_from_ndc_points(unsigned int n, const double *x, const double *y)
{
  grm_args_t *subplot_args;
  unsigned int i;

  for (i = 0, subplot_args = nullptr; i < n && subplot_args == nullptr; ++i)
    {
      subplot_args = get_subplot_from_ndc_point(x[i], y[i]);
    }

  return subplot_args;
}

/*
 * Calculates the classes for polar histogram
 * */
err_t classes_polar_histogram(grm_args_t *subplot_args)
{
  unsigned int num_bins;
  unsigned int length, num_bin_edges, dummy;
  const char *norm = "count";
  double *bin_edges = nullptr, *phi_lim = nullptr, *theta = nullptr;
  double bin_width;
  int is_bin_counts;
  int *bin_counts = nullptr;
  grm_args_t **series;
  err_t error = ERROR_NONE;

  std::shared_ptr<GRM::Element> plot_group = edit_figure->lastChildElement();
  std::shared_ptr<GRM::Element> series_group = (current_central_region_element)
                                                   ? current_central_region_element->lastChildElement()
                                                   : getCentralRegion()->lastChildElement();

  std::shared_ptr<GRM::Context> context = global_render->getContext();


  auto id = static_cast<int>(global_root->getAttribute("_id"));
  global_root->setAttribute("_id", id++);
  auto str = std::to_string(id);

  grm_args_values(subplot_args, "series", "A", &series);

  /* get theta or bin_counts */
  if (grm_args_values(*series, "bin_counts", "i", &is_bin_counts) == 0)
    {
      is_bin_counts = 0;
      grm_args_first_value(*series, "x", "D", &theta, &length);
      std::vector<double> theta_vec(theta, theta + length);
      (*context)["theta" + str] = theta_vec;
      series_group->setAttribute("theta", "theta" + str);
    }
  else
    {
      grm_args_first_value(*series, "x", "I", &bin_counts, &length);
      std::vector<int> bin_counts_vec(bin_counts, bin_counts + length);
      (*context)["bin_counts" + str] = bin_counts_vec;
      series_group->setAttribute("bin_counts", "bin_counts" + str);

      is_bin_counts = 1;
      num_bins = length;
      grm_args_push(*series, "num_bins", "i", num_bins);
      series_group->setAttribute("num_bins", static_cast<int>(num_bins));
    }

  if (grm_args_first_value(*series, "phi_lim", "D", &phi_lim, &dummy))
    {
      plot_group->setAttribute("phi_lim_min", phi_lim[0]);
      plot_group->setAttribute("phi_lim_max", phi_lim[1]);
    }

  /* bin_edges and nbins */
  if (grm_args_first_value(*series, "bin_edges", "D", &bin_edges, &num_bin_edges) == 0)
    {

      if (grm_args_values(*series, "num_bins", "i", &num_bins))
        {
          series_group->setAttribute("num_bins", static_cast<int>(num_bins));
        }
    }
  /* with bin_edges */
  else
    {
      std::vector<double> bin_edges_vec(bin_edges, bin_edges + num_bin_edges);
      (*context)["bin_edges" + str] = bin_edges_vec;
      series_group->setAttribute("bin_edges", "bin_edges" + str);
    }

  if (grm_args_values(subplot_args, "normalization", "s", &norm))
    {
      series_group->setAttribute("norm", norm);
    }

  if (grm_args_values(*series, "bin_width", "d", &bin_width))
    {
      series_group->setAttribute("bin_width", bin_width);
    }

  return error;
}


/* ------------------------- xml ------------------------------------------------------------------------------------ */

#ifndef NO_LIBXML2
#if LIBXML_VERSION >= 21200
static void schema_parse_error_handler(void *has_schema_errors, const xmlError *error)
#else
static void schema_parse_error_handler(void *has_schema_errors, xmlError *error)
#endif
{
  fprintf(stderr, "XML validation error at line %d, column %d: %s", error->line, error->int2, error->message);
  *((bool *)has_schema_errors) = true;
}

err_t validate_graphics_tree(void)
{
  char *gr_dir = get_gr_dir();
  std::string schema_filepath{std::string(gr_dir) + "/" + SCHEMA_REL_FILEPATH};
  free(reinterpret_cast<void *>(gr_dir));
  xmlSchemaParserCtxtPtr schema_parser_ctxt = nullptr;
  xmlSchemaPtr schema = nullptr;
  bool has_schema_errors = false;
  xmlSchemaValidCtxtPtr valid_ctxt = nullptr;
  xmlDocPtr doc = nullptr;
  err_t error = ERROR_NONE;

  xmlInitParser();

  if (!file_exists(schema_filepath.c_str()))
    {
      return ERROR_PARSE_XML_NO_SCHEMA_FILE;
    }
  schema_parser_ctxt = xmlSchemaNewParserCtxt(schema_filepath.c_str());
  cleanup_and_set_error_if(schema_parser_ctxt == nullptr, ERROR_PARSE_XML_INVALID_SCHEMA);
  schema = xmlSchemaParse(schema_parser_ctxt);
  cleanup_and_set_error_if(schema == nullptr, ERROR_PARSE_XML_INVALID_SCHEMA);
  xmlSchemaFreeParserCtxt(schema_parser_ctxt);
  schema_parser_ctxt = nullptr;
  valid_ctxt = xmlSchemaNewValidCtxt(schema);
  doc = xmlReadDoc(BAD_CAST toXML(global_root).c_str(), nullptr, nullptr, XML_PARSE_NOBLANKS);
  cleanup_and_set_error_if(doc == nullptr, ERROR_PARSE_XML_PARSING);
  xmlSchemaSetValidStructuredErrors(valid_ctxt, schema_parse_error_handler, &has_schema_errors);
  xmlSchemaValidateDoc(valid_ctxt, doc);
  cleanup_and_set_error_if(has_schema_errors, ERROR_PARSE_XML_FAILED_SCHEMA_VALIDATION);

cleanup:
  if (doc != nullptr)
    {
      xmlFreeDoc(doc);
    }
  if (valid_ctxt != nullptr)
    {
      xmlSchemaFreeValidCtxt(valid_ctxt);
    }
  if (schema != nullptr)
    {
      xmlSchemaFree(schema);
    }
  if (schema_parser_ctxt != nullptr)
    {
      xmlSchemaFreeParserCtxt(schema_parser_ctxt);
    }
  xmlCleanupParser();

  return error;
}

#endif

int validate_graphics_tree_with_error_messages(void)
{
#ifndef NO_LIBXML2
  err_t validation_error = validate_graphics_tree();
  if (validation_error == ERROR_NONE)
    {
      fprintf(stderr, "The internal graphics tree passed the validity check.\n");
    }
  else if (validation_error == ERROR_PARSE_XML_NO_SCHEMA_FILE)
    {
      fprintf(stderr, "No schema found, XML validation not possible!\n");
    }
  else if (validation_error == ERROR_PARSE_XML_FAILED_SCHEMA_VALIDATION)
    {
      fprintf(stderr, "Schema validation failed!\n");
      return 0;
    }
  else
    {
      fprintf(stderr, "XML validation failed with error \"%d\" (\"%s\")!\n", validation_error,
              error_names[validation_error]);
      return 0;
    }
#else
  fprintf(stderr, "No libxml2 support compiled in, no validation possible!\n");
#endif
  return 1;
}


/* ========================= methods ================================================================================ */

/* ------------------------- args set ------------------------------------------------------------------------------- */

DEFINE_SET_METHODS(args)

int args_set_entry_copy(args_set_entry_t *copy, args_set_const_entry_t entry)
{
  /* discard const because it is necessary to work on the object itself */
  /* TODO create two set types: copy and pointer version */
  *copy = (args_set_entry_t)entry;
  return 1;
}

void args_set_entry_delete(args_set_entry_t entry UNUSED) {}

size_t args_set_entry_hash(args_set_const_entry_t entry)
{
  return (size_t)entry;
}

int args_set_entry_equals(args_set_const_entry_t entry1, args_set_const_entry_t entry2)
{
  return entry1 == entry2;
}

/* ------------------------- string-to-plot_func map ---------------------------------------------------------------- */

DEFINE_MAP_METHODS(plot_func)

int plot_func_map_value_copy(plot_func_t *copy, const plot_func_t value)
{
  *copy = value;

  return 1;
}

void plot_func_map_value_delete(plot_func_t value UNUSED) {}


/* ------------------------- string-to-args_set map ----------------------------------------------------------------- */

DEFINE_MAP_METHODS(args_set)

int args_set_map_value_copy(args_set_t **copy, const args_set_t *value)
{
  /* discard const because it is necessary to work on the object itself */
  /* TODO create two map types: copy and pointer version */
  *copy = (args_set_t *)value;

  return 1;
}

void args_set_map_value_delete(args_set_t *value UNUSED) {}


#undef DEFINE_SET_METHODS
#undef DEFINE_MAP_METHODS


/* ######################### public implementation ################################################################## */

/* ========================= functions ============================================================================== */

/* ------------------------- plot ----------------------------------------------------------------------------------- */

void grm_finalize(void)
{
  if (plot_static_variables_initialized)
    {
      grm_args_delete(global_root_args);
      global_root_args = nullptr;
      active_plot_args = nullptr;
      active_plot_index = 0;
      event_queue_delete(event_queue);
      event_queue = nullptr;
      double_map_delete(meters_per_unit_map);
      meters_per_unit_map = nullptr;
      string_map_delete(fmt_map);
      fmt_map = nullptr;
      plot_func_map_delete(plot_func_map);
      plot_func_map = nullptr;
      string_map_delete(plot_valid_keys_map);
      plot_valid_keys_map = nullptr;
      string_array_map_delete(type_map);
      type_map = nullptr;
      grid_delete(global_grid);
      global_grid = nullptr;
      uninstall_backtrace_handler_if_enabled();
      plot_static_variables_initialized = 0;
    }
  GRM::Render::finalize();
}

int grm_clear(void)
{
  if (plot_init_static_variables() != ERROR_NONE)
    {
      return 0;
    }
  grm_args_clear(active_plot_args);
  if (plot_init_args_structure(active_plot_args, plot_hierarchy_names + 1, 1) != ERROR_NONE)
    {
      return 0;
    }

  return 1;
}

void grm_dump_graphics_tree(FILE *f)
{
  const unsigned int indent = 2;
  fprintf(f, "%s\n", toXML(global_root, GRM::SerializerOptions{std::string(indent, ' ')}).c_str());
}

unsigned int grm_max_plotid(void)
{
  unsigned int args_array_length = 0;

  if (grm_args_first_value(global_root_args, "plots", "A", nullptr, &args_array_length))
    {
      --args_array_length;
    }

  return args_array_length;
}

char *grm_dump_graphics_tree_str(void)
{
  std::string graphics_tree_str = toXML(global_root);
  char *graphics_tree_cstr = new char[graphics_tree_str.length() + 1];
  strcpy(graphics_tree_cstr, graphics_tree_str.c_str());
  return graphics_tree_cstr;
}

#ifndef NO_LIBXML2
int grm_load_graphics_tree(FILE *file)
{
  char *gr_dir = get_gr_dir();
  std::string schema_filepath{std::string(gr_dir) + "/" + SCHEMA_REL_FILEPATH};
  free(reinterpret_cast<void *>(gr_dir));
  bool xml_validation_enabled = false, use_xml_schema = false;
  xmlSchemaParserCtxtPtr schema_parser_ctxt = nullptr;
  xmlSchemaPtr schema = nullptr;
  bool has_schema_errors = false;
  int ret = -1;
  xmlSchemaValidCtxtPtr valid_ctxt = nullptr;
  xmlTextReaderPtr reader = nullptr;
  std::shared_ptr<GRM::Element> insertion_parent, current_gr_element;
  err_t error = ERROR_NONE;

  error = plot_init_static_variables();
  cleanup_if_error;

  xmlInitParser();
  xml_validation_enabled = is_env_variable_enabled(ENABLE_XML_VALIDATION_ENV_KEY.c_str());
  use_xml_schema = xml_validation_enabled && file_exists(schema_filepath.c_str());
  if (use_xml_schema)
    {
      schema_parser_ctxt = xmlSchemaNewParserCtxt(schema_filepath.c_str());
      cleanup_and_set_error_if(schema_parser_ctxt == nullptr, ERROR_PARSE_XML_INVALID_SCHEMA);
      schema = xmlSchemaParse(schema_parser_ctxt);
      cleanup_and_set_error_if(schema == nullptr, ERROR_PARSE_XML_INVALID_SCHEMA);
      xmlSchemaFreeParserCtxt(schema_parser_ctxt);
      schema_parser_ctxt = nullptr;
      valid_ctxt = xmlSchemaNewValidCtxt(schema);
    }
  reader = xmlReaderForFd(fileno(file), nullptr, nullptr, XML_PARSE_NOBLANKS);
  cleanup_and_set_error_if(reader == nullptr, ERROR_PARSE_XML_PARSING);

  if (use_xml_schema)
    {
      xmlTextReaderSchemaValidateCtxt(reader, valid_ctxt, 0);
      xmlSchemaSetValidStructuredErrors(valid_ctxt, schema_parse_error_handler, &has_schema_errors);
    }

  ret = xmlTextReaderRead(reader);
  cleanup_and_set_error_if(has_schema_errors, ERROR_PARSE_XML_FAILED_SCHEMA_VALIDATION);
  global_render->setAutoUpdate(false);
  while (ret == 1)
    {
      xmlNodePtr node = xmlTextReaderCurrentNode(reader);
      int node_type = xmlTextReaderNodeType(reader);
      const xmlChar *node_name = xmlTextReaderConstName(reader);
      if (node_type == XML_READER_TYPE_ELEMENT)
        {
          if (xmlStrEqual(node_name, BAD_CAST "root"))
            {
              global_root = global_render->createElement("root");
              global_render->replaceChildren(global_root);
              insertion_parent = nullptr;
              current_gr_element = global_root;
            }
          else
            {
              current_gr_element = global_render->createElement(reinterpret_cast<const char *>(node_name));
            }
          for (xmlAttrPtr attr = node->properties; attr != nullptr; attr = attr->next)
            {
              const xmlChar *attr_name = attr->name;
              xmlChar *attr_value = xmlNodeListGetString(node->doc, attr->children, 1);

              current_gr_element->setAttribute(reinterpret_cast<const char *>(attr_name),
                                               reinterpret_cast<const char *>(attr_value));
              if (reinterpret_cast<const char *>(attr_name) == "active" &&
                  reinterpret_cast<const char *>(attr_value) == "1")
                global_render->setActiveFigure(current_gr_element);
              xmlFree(reinterpret_cast<void *>(attr_value));
            }
          if (insertion_parent != nullptr)
            {
              insertion_parent->appendChild(current_gr_element);
            }
          if (!xmlTextReaderIsEmptyElement(reader))
            {
              insertion_parent = current_gr_element;
            }
        }
      else if (node_type == XML_READER_TYPE_END_ELEMENT)
        {
          insertion_parent = insertion_parent->parentElement();
        }
      ret = xmlTextReaderRead(reader);
      cleanup_and_set_error_if(has_schema_errors, ERROR_PARSE_XML_FAILED_SCHEMA_VALIDATION);
    }
  edit_figure = global_render->getActiveFigure();
  global_render->setAutoUpdate(true);

  if (ret != 0)
    {
      const xmlError *xml_error = xmlGetLastError();
      logger((stderr, "%s: failed to parse in line %d, col %d. Error %d: %s\n", xml_error->file, xml_error->line,
              xml_error->int2, xml_error->code, xml_error->message));
      cleanup_and_set_error(ERROR_PARSE_XML_PARSING);
    }

cleanup:
  if (reader != nullptr)
    {
      xmlFreeTextReader(reader);
    }
  if (valid_ctxt != nullptr)
    {
      xmlSchemaFreeValidCtxt(valid_ctxt);
    }
  if (schema != nullptr)
    {
      xmlSchemaFree(schema);
    }
  if (schema_parser_ctxt)
    {
      xmlSchemaFreeParserCtxt(schema_parser_ctxt);
    }
  xmlCleanupParser();

  return error == ERROR_NONE;
}
#elif !defined(NO_EXPAT)
static void xml_parse_start_handler(void *data, const XML_Char *tagName, const XML_Char **attr)
{
  auto *insertionParent = (std::shared_ptr<GRM::Element> *)data;
  if (strcmp(tagName, "root") == 0)
    {
      global_root = global_render->createElement("root");
      global_render->replaceChildren(global_root);
      if (attr[0])
        {
          global_root->setAttribute(attr[0], attr[1]);
        }
      (*insertionParent) = global_root;
    }
  else if (strcmp(tagName, "figure") == 0)
    {
      edit_figure = global_render->createElement("figure");
      global_root->append(edit_figure);
    }
  else
    {
      std::shared_ptr<GRM::Element> child = global_render->createElement(tagName);
      for (int i = 0; attr[i]; i += 2)
        {
          child->setAttribute(attr[i], attr[i + 1]);
        }

      (*insertionParent)->appendChild(child);
      *insertionParent = child;
    }
}

static void xml_parse_end_handler(void *data, const char *tagName)
{
  auto currentNode = (std::shared_ptr<GRM::Element> *)data;
  *((std::shared_ptr<GRM::Element> *)data) = (*currentNode)->parentElement();
}

int grm_load_graphics_tree(FILE *file)
{
  std::string xmlstring;
  XML_Parser parser = XML_ParserCreate(nullptr);
  std::shared_ptr<GRM::Element> parentNode;

  std::fseek(file, 0, SEEK_END);
  xmlstring.resize(std::ftell(file));
  std::rewind(file);
  std::fread(&xmlstring[0], 1, xmlstring.size(), file);

  plot_init_static_variables();

  XML_SetUserData(parser, &parentNode);
  XML_SetElementHandler(parser, xml_parse_start_handler, xml_parse_end_handler);

  if (XML_Parse(parser, xmlstring.c_str(), xmlstring.length(), XML_TRUE) == XML_STATUS_ERROR)
    {
      logger((stderr, "Cannot parse XML-String\n"));
      return 0;
    }

  XML_ParserFree(parser);

  return 1;
}
#endif

int grm_merge(const grm_args_t *args)
{
  return grm_merge_extended(args, 0, nullptr);
}

int grm_merge_extended(const grm_args_t *args, int hold, const char *identificator)
{
  err_t error = ERROR_NONE;

  if (plot_init_static_variables() != ERROR_NONE)
    {
      return 0;
    }
  if (args != nullptr)
    {
      if (plot_check_for_request(args, &error))
        {
          // If this is a request, do not process the argument container further
          process_events();
          return error == ERROR_NONE;
        }
      if (plot_merge_args(global_root_args, args, nullptr, nullptr, hold) != ERROR_NONE)
        {
          return 0;
        }
      if (!get_id_from_args(args, &last_merge_plot_id, &last_merge_subplot_id, &last_merge_series_id))
        {
          last_merge_plot_id = 0;
          last_merge_subplot_id = 0;
          last_merge_series_id = 0;
        }
      args_changed_since_last_plot = true;
    }

  process_events();
  event_queue_enqueue_merge_end_event(event_queue, identificator);
  process_events();

  return 1;
}

int grm_merge_hold(const grm_args_t *args)
{
  return grm_merge_extended(args, 1, nullptr);
}

int grm_merge_named(const grm_args_t *args, const char *identificator)
{
  return grm_merge_extended(args, 0, identificator);
}

int plot_process_subplot_args(grm_args_t *subplot_args)
{
  plot_func_t plot_func;
  char *y_label, *x_label, *title, *kind;
  int keep_aspect_ratio, location, adjust_x_lim, adjust_y_lim, only_quadratic_aspect_ratio;
  double *subplot;
  double x_lim_min, x_lim_max, y_lim_min, y_lim_max, z_lim_min, z_lim_max;
  double x_min, x_max, y_min, y_max, z_min, z_max;
  grm_args_t **current_series;
  int grplot = 0;

  std::shared_ptr<GRM::Element> group = (current_dom_element) ? current_dom_element : edit_figure->lastChildElement();
  grm_args_values(subplot_args, "kind", "s", &kind);
  group->setAttribute("kind", kind);
  logger((stderr, "Got keyword \"kind\" with value \"%s\"\n", kind));

  if (plot_pre_subplot(subplot_args) != ERROR_NONE)
    {
      return 0;
    }

  auto central_region = getCentralRegion();
  if (grm_args_values(subplot_args, "keep_aspect_ratio", "i", &keep_aspect_ratio))
    {
      group->setAttribute("keep_aspect_ratio", keep_aspect_ratio);
    }
  if (grm_args_values(subplot_args, "only_quadratic_aspect_ratio", "i", &only_quadratic_aspect_ratio))
    {
      group->setAttribute("only_quadratic_aspect_ratio", only_quadratic_aspect_ratio);
    }
  if (grm_args_values(subplot_args, "location", "i", &location))
    {
      group->setAttribute("location", location);
    }
  if (grm_args_values(subplot_args, "subplot", "D", &subplot))
    {
      group->setAttribute("plot_x_min", subplot[0]);
      group->setAttribute("plot_x_max", subplot[1]);
      group->setAttribute("plot_y_min", subplot[2]);
      group->setAttribute("plot_y_max", subplot[3]);
    }

  if (grm_args_values(subplot_args, "x_lim", "dd", &x_lim_min, &x_lim_max))
    {
      group->setAttribute("x_lim_min", x_lim_min);
      group->setAttribute("x_lim_max", x_lim_max);
    }
  if (grm_args_values(subplot_args, "y_lim", "dd", &y_lim_min, &y_lim_max))
    {
      group->setAttribute("y_lim_min", y_lim_min);
      group->setAttribute("y_lim_max", y_lim_max);
    }
  if (grm_args_values(subplot_args, "z_lim", "dd", &z_lim_min, &z_lim_max))
    {
      group->setAttribute("z_lim_min", z_lim_min);
      group->setAttribute("z_lim_max", z_lim_max);
    }

  if (grm_args_values(subplot_args, "adjust_x_lim", "i", &adjust_x_lim))
    {
      group->setAttribute("adjust_x_lim", adjust_x_lim);
    }
  if (grm_args_values(subplot_args, "adjust_y_lim", "i", &adjust_y_lim))
    {
      group->setAttribute("adjust_y_lim", adjust_y_lim);
    }

  if (grm_args_values(subplot_args, "grplot", "i", &grplot))
    {
      group->setAttribute("grplot", grplot);
    }

  if (!plot_func_map_at(plot_func_map, kind, &plot_func))
    {
      return 0;
    }
  if (plot_func(subplot_args) != ERROR_NONE)
    {
      return 0;
    }
  plot_post_subplot(subplot_args);
  return 1;
}

int grm_plot(const grm_args_t *args) // TODO: rename this method so the name displays the functionality better
{
  grm_args_t **current_subplot_args;
  grm::Grid *currentGrid;
  plot_func_t plot_func;
  int fig_size_x, fig_size_y, tmp_size_i[2];
  double tmp_size_d[2];
  grm_args_ptr_t tmp_size_a[2];
  const char *tmp_size_s[2];
  std::string vars[2] = {"x", "y"};
  double default_size[2] = {PLOT_DEFAULT_WIDTH, PLOT_DEFAULT_HEIGHT};
  bool hold_figures = false, append_figures = false, figure_id_given = false;

  if (!grm_merge(args)) return 0;
  int figure_id = active_plot_index - 1; // the container is 1 based, the  DOM-tree 0

  if (!args_changed_since_last_plot && global_render->documentElement() &&
      (global_render->documentElement()->hasChildNodes() ||
       global_render->documentElement()->hasAttribute("_removed_children")))
    {
      /*
       * Use `grm_render` instead of `global_render->render()` because `grm_render` has additional checks in debug mode
       */
      grm_render();
      return 1;
    }
  else
    {
      int temp;
      global_render->setAutoUpdate(false);
      if (grm_args_values(global_root_args, "hold_plots", "i", &temp))
        {
          hold_figures = temp;
        }
      if (grm_args_values(global_root_args, "append_plots", "i", &temp))
        {
          append_figures = temp;
        }

      /* if plot_id is set - plot_id is getting stored different than other attributes inside the container */
      if (last_merge_plot_id > 0)
        {
          figure_id = last_merge_plot_id - 1;
          figure_id_given = true;
        }

      /* check if given figure_id (even default 0) already exists in the render */
      auto figure_element = global_root->querySelectors("[figure_id=figure" + std::to_string(figure_id) + "]");

      auto last_figure = global_root->hasChildNodes() ? global_root->children().back() : nullptr;
      if (append_figures && !figure_id_given)
        {
          if (last_figure != nullptr && !last_figure->hasChildNodes())
            {
              auto figure_id_str = static_cast<std::string>(last_figure->getAttribute("figure_id"));
              figure_id = std::stoi(figure_id_str.substr(6)); // Remove a `figure` prefix before converting
              last_figure->remove();
            }
          else
            {
              /* also set a not given figure_id for identification */
              figure_id = get_free_id_from_figure_elements();
            }
          edit_figure = global_render->createElement("figure");
          global_root->append(edit_figure);
          edit_figure->setAttribute("figure_id", "figure" + std::to_string(figure_id));
        }
      else
        {
          if (figure_element != nullptr) figure_element->remove();
          edit_figure = global_render->createElement("figure");
          global_root->append(edit_figure);
          edit_figure->setAttribute("figure_id", "figure" + std::to_string(figure_id));
        }
      current_dom_element = nullptr;
      current_central_region_element = nullptr;
    }

  if (grm_args_values(active_plot_args, "raw", "s", &current_subplot_args))
    {
      plot_raw(active_plot_args);
      global_render->setActiveFigure(edit_figure);
      grm_render();
    }
  else
    {
      /*
       * active_plot_args is wrong, because this will lead to this error: we edit f.e. figure0 with the data from the
       * edit_figure figure1, but we actually want to edit figure0 with the container-data that belong to figure0 and
       * set figure1 afterwards as active
       */
      grm_args_t **args_array = nullptr;
      unsigned int args_array_length = 0;
      if (!grm_args_first_value(global_root_args, "plots", "A", &args_array, &args_array_length)) return 0;
      _grm_args_t *edit_plot_args = args_array[figure_id];

      if (!edit_figure->hasChildNodes() || !hold_figures || (append_figures && !figure_id_given))
        plot_set_attribute_defaults(edit_plot_args);

      if (grm_args_values(edit_plot_args, "size", "dd", &tmp_size_d[0], &tmp_size_d[1]))
        {
          for (int i = 0; i < 2; ++i)
            {
              edit_figure->setAttribute("size_" + vars[i], tmp_size_d[i]);
              edit_figure->setAttribute("size_" + vars[i] + "_type", "double");
              edit_figure->setAttribute("size_" + vars[i] + "_unit", "px");
            }
        }
      if (grm_args_values(edit_plot_args, "size", "ii", &tmp_size_i[0], &tmp_size_i[1]))
        {
          for (int i = 0; i < 2; ++i)
            {
              edit_figure->setAttribute("size_" + vars[i], tmp_size_i[i]);
              edit_figure->setAttribute("size_" + vars[i] + "_type", "int");
              edit_figure->setAttribute("size_" + vars[i] + "_unit", "px");
            }
        }
      if (grm_args_values(edit_plot_args, "size", "aa", &tmp_size_a[0], &tmp_size_a[1]))
        {
          for (int i = 0; i < 2; ++i)
            {
              if (grm_args_values(tmp_size_a[i], "unit", "s", &tmp_size_s[i]))
                {
                  edit_figure->setAttribute("size_" + vars[i] + "_unit", tmp_size_s[i]);
                }
              if (grm_args_values(tmp_size_a[i], "value", "i", &tmp_size_i[i]))
                {
                  edit_figure->setAttribute("size_" + vars[i] + "_type", "int");
                  edit_figure->setAttribute("size_" + vars[i], tmp_size_i[i]);
                }
              else if (grm_args_values(tmp_size_a[i], "value", "d", &tmp_size_d[i]))
                {
                  edit_figure->setAttribute("size_" + vars[i] + "_type", "double");
                  edit_figure->setAttribute("size_" + vars[i], tmp_size_d[i]);
                }
              else
                {
                  /* If no value is given, fall back to default value */
                  for (int j = 0; j < 2; ++j)
                    {
                      edit_figure->setAttribute("size_" + vars[j], default_size[j]);
                      edit_figure->setAttribute("size_" + vars[j] + "_type", "double");
                      edit_figure->setAttribute("size_" + vars[j] + "_unit", "px");
                    }
                  return 0;
                }
            }
        }
      if (grm_args_values(edit_plot_args, "fig_size", "dd", &fig_size_x, &fig_size_y))
        {
          edit_figure->setAttribute("fig_size_x", fig_size_x);
          edit_figure->setAttribute("fig_size_y", fig_size_y);
        }
      if (!edit_figure->hasChildNodes() || !hold_figures || (append_figures && !figure_id_given))
        {
          if (plot_process_grid_arguments(edit_plot_args) != ERROR_NONE) return 0;
        }
      currentGrid = reinterpret_cast<grm::Grid *>(global_grid);
      int nrows = currentGrid->getNRows();
      int ncols = currentGrid->getNCols();

      plot_pre_plot(edit_plot_args);
      grm_args_values(edit_plot_args, "subplots", "A", &current_subplot_args);
      if (!edit_figure->hasChildNodes() || (append_figures && !figure_id_given))
        {
          int plot_id = 0;
          if (!(nrows == 1 && ncols == 1 &&
                currentGrid->getElement(0, 0) == nullptr)) // Check if Grid arguments in container
            {
              auto gridDomElement = global_render->createLayoutGrid(*currentGrid);
              edit_figure->append(gridDomElement);

              if (!grm_iterate_grid(currentGrid, gridDomElement, plot_id)) return 0;
            }
          else
            {
              logger((stderr, "No grid elements inside the container structure\n"));
              while (*current_subplot_args != nullptr)
                {
                  grm_args_t **series;
                  if (grm_args_values(*current_subplot_args, "series", "A", &series))
                    {
                      const char *kind;
                      auto plot = global_render->createPlot(plot_id);
                      auto central_region = global_render->createCentralRegion();
                      edit_figure->append(plot);
                      grm_args_values(*current_subplot_args, "kind", "s", &kind);
                      if (strcmp(kind, "marginal_heatmap") == 0)
                        {
                          auto marginal_heatmap = global_render->createElement("marginal_heatmap_plot");
                          plot->append(marginal_heatmap);
                          marginal_heatmap->append(central_region);
                        }
                      else
                        {
                          plot->append(central_region);
                        }
                      current_dom_element = plot;
                      current_central_region_element = central_region;
                    }
                  else
                    {
                      current_dom_element = edit_figure->firstChildElement();
                      for (const auto &child : current_dom_element->children())
                        {
                          if (child->localName() == "central_region")
                            {
                              current_central_region_element = child;
                              break;
                            }
                        }
                    }
                  if (!plot_process_subplot_args(*current_subplot_args)) return 0;
                  ++plot_id;
                  ++current_subplot_args;
                }
            }
          plot_post_plot(edit_plot_args);
        }
      edit_figure = global_root->querySelectors("[figure_id=figure" + std::to_string(active_plot_index - 1) + "]");
      global_render->setActiveFigure(edit_figure);
      global_render->render();
      global_render->setAutoUpdate(true);
    }

  process_events();

  last_merge_plot_id = 0;
  last_merge_subplot_id = 0;
  last_merge_series_id = 0;

#ifndef NDEBUG
  logger((stderr, "root args after \"grm_plot\" (active_plot_index: %d):\n", active_plot_index - 1));
  if (logger_enabled())
    {
      grm_dump(global_root_args, stderr);
    }
  if (is_env_variable_enabled(ENABLE_XML_VALIDATION_ENV_KEY.c_str()) || logger_enabled())
    {
      return validate_graphics_tree_with_error_messages();
    }

#endif

  args_changed_since_last_plot = false;

  return 1;
}

int grm_render(void)
{
  global_render->render();
#ifndef NDEBUG
  if (is_env_variable_enabled(ENABLE_XML_VALIDATION_ENV_KEY.c_str()) || logger_enabled())
    {
      return validate_graphics_tree_with_error_messages();
    }
#endif
  return 1;
}

int grm_process_tree(void)
{
  global_render->process_tree();
#ifndef NDEBUG
  if (is_env_variable_enabled(ENABLE_XML_VALIDATION_ENV_KEY.c_str()) || logger_enabled())
    {
      return validate_graphics_tree_with_error_messages();
    }
#endif
  return 1;
}

int grm_export(const char *file_path)
{
  gr_beginprint(const_cast<char *>(file_path));
  int return_value = grm_plot(nullptr);
  gr_endprint();
  return return_value;
}

int grm_switch(unsigned int id)
{
  grm_args_t **args_array = nullptr;
  unsigned int args_array_length = 0;

  auto figure_element = global_root->querySelectors("[figure_id=figure" + std::to_string(id) + "]");
  if (figure_element == nullptr)
    {
      /* it is a new figure_id, but only with grm_switch will it be really active
       * edit_figure is only set on this for creating the needed child_elements
       * without grm_switch it will not be rendered */
      bool auto_update;
      edit_figure = global_render->createElement("figure");
      global_root->append(edit_figure);
      global_render->getAutoUpdate(&auto_update);
      global_render->setAutoUpdate(false);
      edit_figure->setAttribute("figure_id", "figure" + std::to_string(id));
      global_render->setAutoUpdate(auto_update);
      global_render->setActiveFigure(edit_figure);
    }
  else
    {
      edit_figure = figure_element;
      global_render->setActiveFigure(edit_figure);
    }

  if (plot_init_static_variables() != ERROR_NONE) return 0;
  if (plot_init_args_structure(global_root_args, plot_hierarchy_names, id + 1) != ERROR_NONE) return 0;
  if (!grm_args_first_value(global_root_args, "plots", "A", &args_array, &args_array_length)) return 0;
  if (id + 1 > args_array_length) return 0;

  active_plot_index = id + 1;
  active_plot_args = args_array[id];

  return 1;
}

} /* end of extern "C" block */

/* ========================= c++ ==================================================================================== */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ c++ libxml util ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifndef NO_LIBXML2
std::shared_ptr<GRM::Document> grm_load_graphics_tree_schema(void)
{
  char *gr_dir = get_gr_dir();
  std::string schema_filepath{std::string(gr_dir) + "/" + SCHEMA_REL_FILEPATH};
  free(reinterpret_cast<void *>(gr_dir));
  int ret = -1;
  xmlSchemaValidCtxtPtr valid_ctxt = nullptr;
  xmlTextReaderPtr reader = nullptr;
  std::shared_ptr<GRM::Document> document;
  std::shared_ptr<GRM::Element> insertion_parent, current_element;
  err_t error = ERROR_NONE;
  FILE *schema_file;

  error = plot_init_static_variables();
  cleanup_if_error;

  schema_file = fopen(schema_filepath.c_str(), "r");
  cleanup_and_set_error_if(schema_file == nullptr, ERROR_PARSE_XML_NO_SCHEMA_FILE);
  xmlInitParser();
  reader = xmlReaderForFd(fileno(schema_file), nullptr, nullptr, XML_PARSE_NOBLANKS);
  cleanup_and_set_error_if(reader == nullptr, ERROR_PARSE_XML_PARSING);
  ret = xmlTextReaderRead(reader);
  document = GRM::createDocument();
  while (ret == 1)
    {
      xmlNodePtr node = xmlTextReaderCurrentNode(reader);
      int node_type = xmlTextReaderNodeType(reader);
      const xmlChar *node_name = xmlTextReaderConstName(reader);
      if (node_type == XML_READER_TYPE_ELEMENT)
        {
          current_element = document->createElement(reinterpret_cast<const char *>(node_name));
          for (xmlAttrPtr attr = node->properties; attr != nullptr; attr = attr->next)
            {
              const xmlChar *attr_name = attr->name;
              xmlChar *attr_value = xmlNodeListGetString(node->doc, attr->children, 1);

              current_element->setAttribute(reinterpret_cast<const char *>(attr_name),
                                            reinterpret_cast<const char *>(attr_value));
              xmlFree(reinterpret_cast<void *>(attr_value));
            }
          if (insertion_parent != nullptr)
            {
              insertion_parent->append(current_element);
            }
          else
            {
              document->append(current_element);
            }
          if (!xmlTextReaderIsEmptyElement(reader))
            {
              insertion_parent = current_element;
            }
        }
      else if (node_type == XML_READER_TYPE_END_ELEMENT)
        {
          insertion_parent = insertion_parent->parentElement();
        }
      ret = xmlTextReaderRead(reader);
    }

  if (ret != 0)
    {
      const xmlError *xml_error = xmlGetLastError();
      logger((stderr, "%s: failed to parse in line %d, col %d. Error %d: %s\n", xml_error->file, xml_error->line,
              xml_error->int2, xml_error->code, xml_error->message));
      cleanup_and_set_error(ERROR_PARSE_XML_PARSING);
    }

cleanup:
  if (reader != nullptr)
    {
      xmlFreeTextReader(reader);
    }
  if (valid_ctxt != nullptr)
    {
      xmlSchemaFreeValidCtxt(valid_ctxt);
    }
  xmlCleanupParser();
  if (schema_file != nullptr)
    {
      fclose(schema_file);
    }

  return (error == ERROR_NONE) ? document : nullptr;
}
#endif

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ c++ util ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int grm_iterate_grid(grm::Grid *grid, const std::shared_ptr<GRM::Element> &parentDomElement, int plotId)
{
  std::set<grm::GridElement *> processedGridElements;

  auto rows = grid->getRows();
  auto elementsToPosition = grid->getElementToPosition();

  for (const auto &row : rows)
    {
      for (const auto &element : row)
        {
          if (!processedGridElements.count(element))
            {
              processedGridElements.insert(element);
              auto slice = elementsToPosition.at(element);
              if (!grm_plot_helper(element, slice, parentDomElement, plotId++)) return 0;
            }
        }
    }
  return 1;
}


int grm_plot_helper(grm::GridElement *gridElement, grm::Slice *slice,
                    const std::shared_ptr<GRM::Element> &parentDomElement, int plotId)
{
  plot_func_t plot_func;

  if (gridElement == nullptr)
    {
      std::cout << "Error: gridElement is nullptr\n";
      return 0;
    }

  if (!gridElement->isGrid())
    {
      grm_args_t **current_subplot_args = &gridElement->subplot_args;
      auto layoutGridElement = global_render->createLayoutGridElement(*gridElement, *slice);
      parentDomElement->append(layoutGridElement);
      auto plot = global_render->createPlot(plotId);
      auto central_region = global_render->createCentralRegion();
      layoutGridElement->append(plot);
      plot->append(central_region);
      current_dom_element = plot;
      current_central_region_element = central_region;

      if (!plot_process_subplot_args(*current_subplot_args)) return 0;
    }
  else
    {
      auto *currentGrid = reinterpret_cast<grm::Grid *>(gridElement);

      auto gridDomElement = global_render->createLayoutGrid(*currentGrid);
      gridDomElement->setAttribute("start_row", slice->rowStart);
      gridDomElement->setAttribute("stop_row", slice->rowStop);
      gridDomElement->setAttribute("start_col", slice->colStart);
      gridDomElement->setAttribute("stop_col", slice->colStop);
      parentDomElement->append(gridDomElement);

      if (!grm_iterate_grid(currentGrid, gridDomElement, plotId)) return 0;
    }
  return 1;
}

std::shared_ptr<GRM::Element> grm_get_document_root()
{
  return global_root;
}

std::shared_ptr<GRM::Render> grm_get_render()
{
  return global_render;
}

int get_free_id_from_figure_elements()
{
  std::vector<std::string> given_ids;
  for (auto &fig : global_root->children())
    {
      given_ids.push_back(static_cast<std::string>(fig->getAttribute("figure_id")));
    }
  int free_id = 0;
  while (true)
    {
      if (std::count(given_ids.begin(), given_ids.end(), "figure" + std::to_string(free_id)) > 0)
        ++free_id;
      else
        return free_id;
    }
}

std::shared_ptr<GRM::Element> get_subplot_from_ndc_point_using_dom_helper(std::shared_ptr<GRM::Element> element,
                                                                          double x, double y)
{
  bool elementIsSubplotGroup =
      (element->hasAttribute("plot_group") && static_cast<int>(element->getAttribute("plot_group")));

  if (element->localName() == "layout_grid_element" || elementIsSubplotGroup)
    {
      double viewport[4];
      viewport[0] = static_cast<double>(element->getAttribute("viewport_x_min"));
      viewport[1] = static_cast<double>(element->getAttribute("viewport_x_max"));
      viewport[2] = static_cast<double>(element->getAttribute("viewport_y_min"));
      viewport[3] = static_cast<double>(element->getAttribute("viewport_y_max"));
      if (elementIsSubplotGroup)
        {
          auto central_region = element->querySelectors("central_region");
          viewport[0] = static_cast<double>(central_region->getAttribute("viewport_x_min"));
          viewport[1] = static_cast<double>(central_region->getAttribute("viewport_x_max"));
          viewport[2] = static_cast<double>(central_region->getAttribute("viewport_y_min"));
          viewport[3] = static_cast<double>(central_region->getAttribute("viewport_y_max"));
        }
      if (viewport[0] <= x && x <= viewport[1] && viewport[2] <= y && y <= viewport[3])
        {
          return element;
        }
    }
  if (element->localName() == "layout_grid")
    {
      for (const auto &child : element->children())
        {
          std::shared_ptr<GRM::Element> subplot_element = get_subplot_from_ndc_point_using_dom_helper(child, x, y);
          if (subplot_element != nullptr)
            {
              return subplot_element;
            }
        }
    }

  return nullptr;
}

std::shared_ptr<GRM::Element> get_subplot_from_ndc_point_using_dom(double x, double y)
{
  edit_figure = global_render->getActiveFigure();
  if (edit_figure->hasChildNodes())
    {
      for (const auto &child : edit_figure->children())
        {
          std::shared_ptr<GRM::Element> subplot_element = get_subplot_from_ndc_point_using_dom_helper(child, x, y);
          if (subplot_element != nullptr)
            {
              return subplot_element;
            }
        }
    }

  return nullptr;
}

std::shared_ptr<GRM::Element> get_subplot_from_ndc_points_using_dom(unsigned int n, const double *x, const double *y)
{
  unsigned int i;
  std::shared_ptr<GRM::Element> subplot_element;

  for (i = 0, subplot_element = nullptr; i < n && subplot_element == nullptr; ++i)
    {
      subplot_element = get_subplot_from_ndc_point_using_dom(x[i], y[i]);
    }

  return subplot_element;
}

void grm_set_attribute_on_all_subplots_helper(std::shared_ptr<GRM::Element> element, std::string attribute, int value)
{
  bool elementIsSubplotGroup =
      (element->hasAttribute("plot_group") && static_cast<int>(element->getAttribute("plot_group")));

  if (element->localName() == "layout_grid_element" || elementIsSubplotGroup)
    {
      element->setAttribute(attribute, value);
    }
  if (element->localName() == "layout_grid")
    {
      for (const auto &child : element->children())
        {
          grm_set_attribute_on_all_subplots_helper(child, attribute, value);
        }
    }
}

void grm_set_attribute_on_all_subplots(std::string attribute, int value)
{
  if (global_root->hasChildNodes())
    {
      for (const auto &child : global_root->children())
        {
          grm_set_attribute_on_all_subplots_helper(child, attribute, value);
        }
    }
}

int get_focus_and_factor_from_dom(const int x1, const int y1, const int x2, const int y2, const int keep_aspect_ratio,
                                  double *factor_x, double *factor_y, double *focus_x, double *focus_y,
                                  std::shared_ptr<GRM::Element> &subplot_element)
{
  double ndc_box_x[4], ndc_box_y[4], viewport[4], wswindow[4];
  double ndc_left, ndc_top, ndc_right, ndc_bottom;
  int width, height, max_width_height;

  GRM::Render::getFigureSize(&width, &height, nullptr, nullptr);
  max_width_height = grm_max(width, height);

  if (x1 <= x2)
    {
      ndc_left = (double)x1 / max_width_height;
      ndc_right = (double)x2 / max_width_height;
    }
  else
    {
      ndc_left = (double)x2 / max_width_height;
      ndc_right = (double)x1 / max_width_height;
    }
  if (y1 <= y2)
    {
      ndc_top = (double)(height - y1) / max_width_height;
      ndc_bottom = (double)(height - y2) / max_width_height;
    }
  else
    {
      ndc_top = (double)(height - y2) / max_width_height;
      ndc_bottom = (double)(height - y1) / max_width_height;
    }

  ndc_box_x[0] = ndc_left;
  ndc_box_y[0] = ndc_bottom;
  ndc_box_x[1] = ndc_right;
  ndc_box_y[1] = ndc_bottom;
  ndc_box_x[2] = ndc_left;
  ndc_box_y[2] = ndc_top;
  ndc_box_x[3] = ndc_right;
  ndc_box_y[3] = ndc_top;
  subplot_element = get_subplot_from_ndc_points_using_dom(array_size(ndc_box_x), ndc_box_x, ndc_box_y);
  if (subplot_element == nullptr)
    {
      return 0;
    }
  auto central_region = subplot_element->querySelectors("central_region");
  viewport[0] = static_cast<double>(central_region->getAttribute("viewport_x_min"));
  viewport[1] = static_cast<double>(central_region->getAttribute("viewport_x_max"));
  viewport[2] = static_cast<double>(central_region->getAttribute("viewport_y_min"));
  viewport[3] = static_cast<double>(central_region->getAttribute("viewport_y_max"));
  wswindow[0] = static_cast<double>(subplot_element->parentElement()->getAttribute("ws_window_x_min"));
  wswindow[1] = static_cast<double>(subplot_element->parentElement()->getAttribute("ws_window_x_max"));
  wswindow[2] = static_cast<double>(subplot_element->parentElement()->getAttribute("ws_window_y_min"));
  wswindow[3] = static_cast<double>(subplot_element->parentElement()->getAttribute("ws_window_y_max"));

  *factor_x = abs(x1 - x2) / (width * (viewport[1] - viewport[0]) / (wswindow[1] - wswindow[0]));
  *factor_y = abs(y1 - y2) / (height * (viewport[3] - viewport[2]) / (wswindow[3] - wswindow[2]));
  if (keep_aspect_ratio)
    {
      if (*factor_x <= *factor_y)
        {
          *factor_x = *factor_y;
          if (x1 > x2)
            {
              ndc_left = ndc_right - *factor_x * (viewport[1] - viewport[0]);
            }
        }
      else
        {
          *factor_y = *factor_x;
          if (y1 > y2)
            {
              ndc_top = ndc_bottom + *factor_y * (viewport[3] - viewport[2]);
            }
        }
    }
  *focus_x = (ndc_left - *factor_x * viewport[0]) / (1 - *factor_x) - (viewport[0] + viewport[1]) / 2.0;
  *focus_y = (ndc_top - *factor_y * viewport[3]) / (1 - *factor_y) - (viewport[2] + viewport[3]) / 2.0;
  return 1;
}

bool grm_validate(void)
{
#ifndef NO_LIBXML2
  err_t validation_error = validate_graphics_tree();
  return (validation_error == ERROR_NONE);
#endif
  return false;
}
