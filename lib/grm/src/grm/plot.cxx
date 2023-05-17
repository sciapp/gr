/* ######################### includes ############################################################################### */

#include <grm/dom_render/graphics_tree/Document.hxx>
#include <grm/dom_render/graphics_tree/Element.hxx>
#include <grm/dom_render/graphics_tree/Comment.hxx>
#include <grm/dom_render/graphics_tree/util.hxx>
#include <grm/dom_render/render.hxx>

#include <string>

extern "C" {

#include <grm/layout.h>
#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <cm.h>

#include "base64_int.h"
#include <grm/dump.h>
#include "event_int.h"
#include "gks.h"
#include "gr.h"
#include "gr3.h"
#include "logging_int.h"

#ifndef NO_EXPAT
#include <expat.h>
#endif
}

#include "plot_int.h"
#include "grm/layout.hxx"

extern "C" {

#include "util_int.h"

#include "datatype/double_map_int.h"
#include "datatype/string_map_int.h"
#include "datatype/string_array_map_int.h"
#include "datatype/template/map_int.h"
#include "datatype/template/set_int.h"


/* ######################### private interface ###################################################################### */

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
  if (value_it->next(value_it) == nullptr)               \
    {                                                    \
      args_value_iterator_delete(value_it);              \
      return ERROR_INTERNAL;                             \
    }                                                    \
  length = value_it->array_length;                       \
  array = *(double **)value_it->value_ptr;

/* ------------------------- re-implementation of x_lin/x_log ------------------------------------------------------- */

#define X_FLIP_IF(x, scale_options, xmin, xmax) \
  (GR_OPTION_FLIP_X & scale_options ? xmin + xmax : 0) + (GR_OPTION_FLIP_X & scale_options ? -1 : 1) * x

#define X_LIN(x, scale_options, xmin, xmax, a, b) \
  X_FLIP_IF((GR_OPTION_X_LOG & scale_options ? (x > 0 ? a * log10(x) + b : -FLT_MAX) : x), scale_options, xmin, xmax)

#define X_LOG(x, scale_options, xmin, xmax, a, b)                                                             \
  (GR_OPTION_X_LOG & scale_options ? (pow(10.0, (double)((X_FLIP_IF(x, scale_options, xmin, xmax) - b) / a))) \
                                   : X_FLIP_IF(x, scale_options, xmin, xmax))

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
static int plot_scatter_markertypes[] = {
    GKS_K_MARKERTYPE_SOLID_CIRCLE,   GKS_K_MARKERTYPE_SOLID_TRI_UP, GKS_K_MARKERTYPE_SOLID_TRI_DOWN,
    GKS_K_MARKERTYPE_SOLID_SQUARE,   GKS_K_MARKERTYPE_SOLID_BOWTIE, GKS_K_MARKERTYPE_SOLID_HGLASS,
    GKS_K_MARKERTYPE_SOLID_DIAMOND,  GKS_K_MARKERTYPE_SOLID_STAR,   GKS_K_MARKERTYPE_SOLID_TRI_RIGHT,
    GKS_K_MARKERTYPE_SOLID_TRI_LEFT, GKS_K_MARKERTYPE_SOLID_PLUS,   GKS_K_MARKERTYPE_PENTAGON,
    GKS_K_MARKERTYPE_HEXAGON,        GKS_K_MARKERTYPE_HEPTAGON,     GKS_K_MARKERTYPE_OCTAGON,
    GKS_K_MARKERTYPE_STAR_4,         GKS_K_MARKERTYPE_STAR_5,       GKS_K_MARKERTYPE_STAR_6,
    GKS_K_MARKERTYPE_STAR_7,         GKS_K_MARKERTYPE_STAR_8,       GKS_K_MARKERTYPE_VLINE,
    GKS_K_MARKERTYPE_HLINE,          GKS_K_MARKERTYPE_OMARK,        INT_MAX};


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ args ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static grm_args_t *global_root_args = nullptr;
grm_args_t *active_plot_args = nullptr;
static unsigned int active_plot_index = 0;
grid_t *global_grid = nullptr;
static std::shared_ptr<GRM::Render> global_render;
static std::shared_ptr<GRM::Element> global_root;
static std::shared_ptr<GRM::Element> currentDomElement;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ event handling ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

event_queue_t *event_queue = nullptr;


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ kind to fmt ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static string_map_entry_t kind_to_fmt[] = {{"line", "xys"},           {"hexbin", "xys"},
                                           {"polar", "xys"},          {"shade", "xys"},
                                           {"stem", "xys"},           {"stairs", "xys"},
                                           {"contour", "xyzc"},       {"contourf", "xyzc"},
                                           {"tricont", "xyzc"},       {"trisurf", "xyzc"},
                                           {"surface", "xyzc"},       {"wireframe", "xyzc"},
                                           {"plot3", "xyzc"},         {"scatter", "xyzc"},
                                           {"scatter3", "xyzc"},      {"quiver", "xyuv"},
                                           {"heatmap", "xyzc"},       {"hist", "x"},
                                           {"barplot", "y"},          {"isosurface", "c"},
                                           {"imshow", "c"},           {"nonuniformheatmap", "xyzc"},
                                           {"polar_histogram", "x"},  {"pie", "x"},
                                           {"volume", "c"},           {"marginalheatmap", "xyzc"},
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
                                               {"marginalheatmap", plot_marginalheatmap},
                                               {"wireframe", plot_wireframe},
                                               {"surface", plot_surface},
                                               {"plot3", plot_plot3},
                                               {"scatter3", plot_scatter3},
                                               {"imshow", plot_imshow},
                                               {"isosurface", plot_isosurface},
                                               {"polar", plot_polar},
                                               {"trisurf", plot_trisurf},
                                               {"tricont", plot_tricont},
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
const char *valid_plot_keys[] = {"clear", "figsize", "raw", "size", "subplots", "update", nullptr};

const char *valid_subplot_keys[] = {"abs_height",
                                    "abs_width",
                                    "accelerate",
                                    "adjust_xlim",
                                    "adjust_ylim",
                                    "adjust_zlim",
                                    "alpha",
                                    "angle_ticks",
                                    "aspect_ratio",
                                    "backgroundcolor",
                                    "bar_color",
                                    "bar_width",
                                    "col",
                                    "colspan",
                                    "colormap",
                                    "fit_parents_height",
                                    "fit_parents_width",
                                    "font",
                                    "font_precision",
                                    "grid_element",
                                    "grplot",
                                    "marginalheatmap_kind",
                                    "ind_bar_color",
                                    "ind_edge_color",
                                    "ind_edge_width",
                                    "keep_aspect_ratio",
                                    "kind",
                                    "labels",
                                    "levels",
                                    "location",
                                    "normalization",
                                    "orientation",
                                    "panzoom",
                                    "phiflip",
                                    "rel_height",
                                    "rel_width",
                                    "resample_method",
                                    "reset_ranges",
                                    "rings",
                                    "rotation",
                                    "row",
                                    "rowspan",
                                    "series",
                                    "style",
                                    "subplot",
                                    "tilt",
                                    "title",
                                    "xbins",
                                    "xflip",
                                    "xform",
                                    "xgrid",
                                    "xlabel",
                                    "xlim",
                                    "xlog",
                                    "xind",
                                    "xticklabels",
                                    "ybins",
                                    "yflip",
                                    "ygrid",
                                    "ylabel",
                                    "ylim",
                                    "ylog",
                                    "yind",
                                    "zflip",
                                    "zgrid",
                                    "zlabel",
                                    "zlim",
                                    "zlog",
                                    nullptr};
const char *valid_series_keys[] = {
    "a",          "algorithm",    "bin_width",  "bin_edges",  "bin_counts", "c",      "c_dims",     "crange",
    "draw_edges", "dmin",         "dmax",       "edge_color", "edge_width", "error",  "face_color", "foreground_color",
    "indices",    "inner_series", "isovalue",   "markertype", "nbins",      "philim", "rgb",        "rlim",
    "s",          "spec",         "step_where", "stairs",     "u",          "v",      "weights",    "x",
    "xcolormap",  "xrange",       "y",          "ycolormap",  "ylabels",    "yrange", "z",          "z_dims",
    "zrange",     nullptr};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ valid types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* If multiple formats are supported use `|` as separator
 * Example: "i|s" for supporting both integer and strings */
/* TODO: type for format "s"? */
static string_map_entry_t key_to_formats[] = {{"a", "A"},
                                              {"abs_height", "d"},
                                              {"abs_width", "d"},
                                              {"accelerate", "i"},
                                              {"algorithm", "i|s"},
                                              {"adjust_xlim", "i"},
                                              {"adjust_ylim", "i"},
                                              {"adjust_zlim", "i"},
                                              {"alpha", "d"},
                                              {"aspect_ratio", "d"},
                                              {"append_plots", "i"},
                                              {"backgroundcolor", "i"},
                                              {"bar_color", "D|i"},
                                              {"c", "D|I"},
                                              {"c_dims", "I"},
                                              {"crange", "D"},
                                              {"col", "i|I"},
                                              {"colspan", "i|I"},
                                              {"colormap", "i"},
                                              {"dmin", "d"},
                                              {"dmax", "d"},
                                              {"edge_color", "D|i"},
                                              {"edge_width", "d"},
                                              {"error", "a"},
                                              {"figsize", "D"},
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
                                              {"isovalue", "d"},
                                              {"keep_aspect_ratio", "i"},
                                              {"kind", "s"},
                                              {"labels", "S"},
                                              {"levels", "i"},
                                              {"location", "i"},
                                              {"markertype", "i|D"},
                                              {"nbins", "i"},
                                              {"orientation", "s"},
                                              {"panzoom", "D"},
                                              {"raw", "s"},
                                              {"rel_height", "d"},
                                              {"rel_width", "d"},
                                              {"resample_method", "s|i"},
                                              {"reset_ranges", "i"},
                                              {"rotation", "d"},
                                              {"row", "i|I"},
                                              {"rowspan", "i|I"},
                                              {"size", "D|I|A"},
                                              {"spec", "s"},
                                              {"step_where", "s"},
                                              {"style", "s"},
                                              {"subplot", "D"},
                                              {"tilt", "d"},
                                              {"title", "s"},
                                              {"marginalheatmap_kind", "s"},
                                              {"u", "D"},
                                              {"update", "i"},
                                              {"v", "D"},
                                              {"x", "D|I"},
                                              {"xbins", "i"},
                                              {"xcolormap", "i"},
                                              {"xflip", "i"},
                                              {"xform", "i"},
                                              {"xgrid", "i"},
                                              {"xlabel", "s"},
                                              {"xlim", "D"},
                                              {"xind", "i"},
                                              {"xrange", "D"},
                                              {"xlog", "i"},
                                              {"y", "D"},
                                              {"ybins", "i"},
                                              {"ycolormap", "i"},
                                              {"yflip", "i"},
                                              {"yform", "i"},
                                              {"ygrid", "i"},
                                              {"ylabel", "s"},
                                              {"ylim", "D"},
                                              {"yind", "i"},
                                              {"yrange", "D"},
                                              {"ylog", "i"},
                                              {"z", "D"},
                                              {"z_dims", "I"},
                                              {"zflip", "i"},
                                              {"zgrid", "i"},
                                              {"zlabel", "s"},
                                              {"zlim", "D"},
                                              {"zrange", "D"},
                                              {"zlog", "i"}};

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
      global_render = GRM::Render::createRender();
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


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plot arguments ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

err_t plot_merge_args(grm_args_t *args, const grm_args_t *merge_args, const char **hierarchy_name_ptr,
                      uint_map_t *hierarchy_to_id, int hold_always)
{
  static args_set_map_t *key_to_cleared_args = nullptr;
  static int recursion_level = -1;
  int plot_id, subplot_id, series_id;
  int append_plots;
  args_iterator_t *merge_it = nullptr;
  arg_t *arg, *merge_arg;
  args_value_iterator_t *value_it = nullptr, *merge_value_it = nullptr;
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
  grm_args_values(global_root_args, "append_plots", "i", &append_plots);
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
  merge_it = args_iter(merge_args);
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
      /* If the current key is a hierarchy key, perform a merge. Otherwise (else branch) put the value in without a
       * merge.
       */
      if (current_hierarchy_name_ptr[1] != nullptr && strcmp(merge_arg->key, current_hierarchy_name_ptr[1]) == 0)
        {
          /* `args_at` cannot fail in this case because the `args` object was initialized with an empty structure
           * before. If `arg` is nullptr, an internal error occurred. */
          arg = args_at(current_args, merge_arg->key);
          cleanup_and_set_error_if(arg == nullptr, ERROR_INTERNAL);
          value_it = arg_value_iter(arg);
          merge_value_it = arg_value_iter(merge_arg);
          cleanup_and_set_error_if(value_it == nullptr, ERROR_MALLOC);
          cleanup_and_set_error_if(merge_value_it == nullptr, ERROR_MALLOC);
          /* Do not support two dimensional argument arrays like `nAnA`) -> a loop would be needed with more memory
           * management */
          cleanup_and_set_error_if(value_it->next(value_it) == nullptr, ERROR_MALLOC);
          cleanup_and_set_error_if(merge_value_it->next(merge_value_it) == nullptr, ERROR_MALLOC);
          /* Increase the array size of the internal args array if necessary */
          if (merge_value_it->array_length > value_it->array_length)
            {
              error = plot_init_arg_structure(arg, current_hierarchy_name_ptr, merge_value_it->array_length);
              cleanup_if_error;
              args_value_iterator_delete(value_it);
              value_it = arg_value_iter(arg);
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
  const char *kind;
  grm_args_t **current_subplot, **current_series;
  double garbage0, garbage1;

  logger((stderr, "Set plot attribute defaults\n"));

  args_setdefault(plot_args, "clear", "i", PLOT_DEFAULT_CLEAR);
  args_setdefault(plot_args, "update", "i", PLOT_DEFAULT_UPDATE);
  if (!grm_args_contains(plot_args, "figsize"))
    {
      args_setdefault(plot_args, "size", "dd", PLOT_DEFAULT_WIDTH, PLOT_DEFAULT_HEIGHT);
    }

  grm_args_values(plot_args, "subplots", "A", &current_subplot);
  while (*current_subplot != nullptr)
    {
      args_setdefault(*current_subplot, "kind", "s", PLOT_DEFAULT_KIND);
      grm_args_values(*current_subplot, "kind", "s", &kind);
      if (grm_args_contains(*current_subplot, "labels"))
        {
          args_setdefault(*current_subplot, "location", "i", PLOT_DEFAULT_LOCATION);
        }
      args_setdefault(*current_subplot, "subplot", "dddd", PLOT_DEFAULT_SUBPLOT_MIN_X, PLOT_DEFAULT_SUBPLOT_MAX_X,
                      PLOT_DEFAULT_SUBPLOT_MIN_Y, PLOT_DEFAULT_SUBPLOT_MAX_Y);
      args_setdefault(*current_subplot, "xlog", "i", PLOT_DEFAULT_XLOG);
      args_setdefault(*current_subplot, "ylog", "i", PLOT_DEFAULT_YLOG);
      args_setdefault(*current_subplot, "zlog", "i", PLOT_DEFAULT_ZLOG);
      args_setdefault(*current_subplot, "xflip", "i", PLOT_DEFAULT_XFLIP);
      args_setdefault(*current_subplot, "yflip", "i", PLOT_DEFAULT_YFLIP);
      args_setdefault(*current_subplot, "zflip", "i", PLOT_DEFAULT_ZFLIP);
      args_setdefault(*current_subplot, "xgrid", "i", PLOT_DEFAULT_XGRID);
      args_setdefault(*current_subplot, "ygrid", "i", PLOT_DEFAULT_YGRID);
      args_setdefault(*current_subplot, "zgrid", "i", PLOT_DEFAULT_ZGRID);
      args_setdefault(*current_subplot, "resample_method", "i", PLOT_DEFAULT_RESAMPLE_METHOD);
      if (str_equals_any(kind, 2, "heatmap", "marginalheatmap"))
        {
          args_setdefault(*current_subplot, "adjust_xlim", "i", 0);
          args_setdefault(*current_subplot, "adjust_ylim", "i", 0);
        }
      else
        {
          args_setdefault(
              *current_subplot, "adjust_xlim", "i",
              (grm_args_values(*current_subplot, "xlim", "dd", &garbage0, &garbage1) ? 0 : PLOT_DEFAULT_ADJUST_XLIM));
          args_setdefault(
              *current_subplot, "adjust_ylim", "i",
              (grm_args_values(*current_subplot, "ylim", "dd", &garbage0, &garbage1) ? 0 : PLOT_DEFAULT_ADJUST_YLIM));
          args_setdefault(
              *current_subplot, "adjust_zlim", "i",
              (grm_args_values(*current_subplot, "zlim", "dd", &garbage0, &garbage1) ? 0 : PLOT_DEFAULT_ADJUST_ZLIM));
        }
      args_setdefault(*current_subplot, "colormap", "i", PLOT_DEFAULT_COLORMAP);
      args_setdefault(*current_subplot, "font", "i", PLOT_DEFAULT_FONT);
      args_setdefault(*current_subplot, "font_precision", "i", PLOT_DEFAULT_FONT_PRECISION);
      args_setdefault(*current_subplot, "rotation", "d", PLOT_DEFAULT_ROTATION);
      args_setdefault(*current_subplot, "tilt", "d", PLOT_DEFAULT_TILT);
      args_setdefault(*current_subplot, "keep_aspect_ratio", "i", PLOT_DEFAULT_KEEP_ASPECT_RATIO);

      if (str_equals_any(kind, 2, "contour", "contourf"))
        {
          args_setdefault(*current_subplot, "levels", "i", PLOT_DEFAULT_CONTOUR_LEVELS);
        }
      else if (strcmp(kind, "tricont") == 0)
        {
          args_setdefault(*current_subplot, "levels", "i", PLOT_DEFAULT_TRICONT_LEVELS);
        }
      else if (strcmp(kind, "marginalheatmap") == 0)
        {
          args_setdefault(*current_subplot, "xind", "i", -1);
          args_setdefault(*current_subplot, "yind", "i", -1);
          args_setdefault(*current_subplot, "marginalheatmap_kind", "s", "all");
        }
      else if (str_equals_any(kind, 1, "surface"))
        {
          args_setdefault(*current_subplot, "accelerate", "i", 1);
        }
      if (str_equals_any(kind, 6, "barplot", "hist", "line", "scatter", "stairs", "stem"))
        {
          args_setdefault(*current_subplot, "orientation", "s", PLOT_DEFAULT_ORIENTATION);
        }

      grm_args_values(*current_subplot, "series", "A", &current_series);
      while (*current_series != nullptr)
        {
          args_setdefault(*current_series, "spec", "s", SERIES_DEFAULT_SPEC);
          if (strcmp(kind, "stairs") == 0)
            {
              args_setdefault(*current_series, "step_where", "s", PLOT_DEFAULT_STEP_WHERE);
            }
          else if (strcmp(kind, "hexbin") == 0)
            {
              args_setdefault(*current_series, "nbins", "i", PLOT_DEFAULT_HEXBIN_NBINS);
            }
          else if (strcmp(kind, "volume") == 0)
            {
              args_setdefault(*current_series, "algorithm", "i", PLOT_DEFAULT_VOLUME_ALGORITHM);
            }
          else if (strcmp(kind, "marginalheatmap") == 0)
            {
              args_setdefault(*current_series, "algorithm", "s", "sum");
            }
          ++current_series;
        }
      ++current_subplot;
    }
}

void plot_pre_plot(grm_args_t *plot_args)
{
  int clear;

  logger((stderr, "Pre plot processing\n"));

  plot_set_text_encoding();
  grm_args_values(plot_args, "clear", "i", &clear);
  logger((stderr, "Got keyword \"clear\" with value %d\n", clear));
  global_root->setAttribute("clearws", clear);
  plot_process_wswindow_wsviewport(plot_args);
}

void plot_set_text_encoding(void)
{
  global_render->setTextEncoding(global_root, ENCODING_UTF8);
}

void plot_process_wswindow_wsviewport(grm_args_t *plot_args)
{
  int pixel_width, pixel_height;
  int previous_pixel_width, previous_pixel_height;
  double metric_width, metric_height;
  double aspect_ratio_ws_pixel, aspect_ratio_ws_metric;
  double wsviewport[4] = {0.0, 0.0, 0.0, 0.0};
  double wswindow[4] = {0.0, 0.0, 0.0, 0.0};

  // set wswindow/wsviewport on root
  auto group = global_root;

  get_figure_size(plot_args, &pixel_width, &pixel_height, &metric_width, &metric_height);

  if (!grm_args_values(plot_args, "previous_pixel_size", "ii", &previous_pixel_width, &previous_pixel_height) ||
      (previous_pixel_width != pixel_width || previous_pixel_height != pixel_height))
    {
      /* TODO: handle error return value? */
      event_queue_enqueue_size_event(event_queue, active_plot_index - 1, pixel_width, pixel_height);
    }

  aspect_ratio_ws_pixel = (double)pixel_width / pixel_height;
  aspect_ratio_ws_metric = metric_width / metric_height;
  if (aspect_ratio_ws_pixel > 1)
    {
      wsviewport[1] = metric_width;
      wsviewport[3] = metric_width / aspect_ratio_ws_metric;
      wswindow[1] = 1.0;
      wswindow[3] = 1.0 / (aspect_ratio_ws_pixel);
    }
  else
    {
      wsviewport[1] = metric_height * aspect_ratio_ws_metric;
      wsviewport[3] = metric_height;
      wswindow[1] = aspect_ratio_ws_pixel;
      wswindow[3] = 1.0;
    }
  global_render->setWSViewport(group, wsviewport[0], wsviewport[1], wsviewport[2], wsviewport[3]);
  global_render->setWSWindow(group, wswindow[0], wswindow[1], wswindow[2], wswindow[3]);

  grm_args_push(plot_args, "wswindow", "dddd", wswindow[0], wswindow[1], wswindow[2], wswindow[3]);
  grm_args_push(plot_args, "wsviewport", "dddd", wsviewport[0], wsviewport[1], wsviewport[2], wsviewport[3]);
  grm_args_push(plot_args, "previous_pixel_size", "ii", pixel_width, pixel_height);

  logger((stderr, "Stored wswindow (%lf, %lf, %lf, %lf)\n", wswindow[0], wswindow[1], wswindow[2], wswindow[3]));
  logger(
      (stderr, "Stored wsviewport (%lf, %lf, %lf, %lf)\n", wsviewport[0], wsviewport[1], wsviewport[2], wsviewport[3]));
}

err_t plot_pre_subplot(grm_args_t *subplot_args)
{
  const char *kind;
  double alpha;
  err_t error = ERROR_NONE;

  logger((stderr, "Pre subplot processing\n"));

  grm_args_values(subplot_args, "kind", "s", &kind);
  logger((stderr, "Got keyword \"kind\" with value \"%s\"\n", kind));
  error = plot_store_coordinate_ranges(subplot_args);
  return_if_error;
  plot_process_window(subplot_args);

  plot_process_colormap(subplot_args);
  plot_process_font(subplot_args);
  plot_process_resample_method(subplot_args);

  if (str_equals_any(kind, 2, "polar", "polar_histogram"))
    {
      plot_draw_polar_axes(subplot_args);
    }
  else if (!str_equals_any(kind, 5, "imshow", "isosurface", "pie", "polar_heatmap", "nonuniformpolar_heatmap"))
    {
      plot_draw_axes(subplot_args, 1);
    }

  gr_uselinespec(const_cast<char *>(" "));

  gr_savestate();
  if (grm_args_values(subplot_args, "alpha", "d", &alpha))
    {
      gr_settransparency(alpha);
    }

  return ERROR_NONE;
}

void plot_process_colormap(grm_args_t *subplot_args)
{
  int colormap;

  if (grm_args_values(subplot_args, "colormap", "i", &colormap))
    {
      gr_setcolormap(colormap);
    }
  /* TODO: Implement other datatypes for `colormap` */
}

void plot_process_font(grm_args_t *subplot_args)
{
  int font, font_precision;

  /* `font` and `font_precision` are always set */
  if (grm_args_values(subplot_args, "font", "i", &font) &&
      grm_args_values(subplot_args, "font_precision", "i", &font_precision))
    {
      logger((stderr, "Using font: %d with precision %d\n", font, font_precision));
      gr_settextfontprec(font, font_precision);
    }
  /* TODO: Implement other datatypes for `font` and `font_precision` */
}

err_t plot_process_grid_arguments(const grm_args_t *args)
{
  int current_nesting_degree, nesting_degree;
  int *rows, *cols;
  unsigned int rows_length, cols_length;
  int rowspan, colspan;
  int *rowspans, *colspans;
  unsigned int rowspans_length, colspans_length;
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
      rowspans = nullptr, colspans = nullptr;
      rowspan = 1, colspan = 1;
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

      grm_args_first_value(*current_subplot_args, "rowspan", "I", &rowspans, &rowspans_length);
      grm_args_first_value(*current_subplot_args, "colspan", "I", &colspans, &colspans_length);

      if (rowspans == nullptr)
        {
          rowspans = &rowspan;
          rowspans_length = 1;
        }
      if (colspans == nullptr)
        {
          colspans = &colspan;
          colspans_length = 1;
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

      nesting_degree = rows_length - 1;
      current_grid = global_grid;
      for (current_nesting_degree = 0; current_nesting_degree <= nesting_degree; ++current_nesting_degree)
        {

          rowstart = rows[current_nesting_degree];
          rowstop =
              (current_nesting_degree >= rowspans_length) ? rowstart + 1 : rowstart + rowspans[current_nesting_degree];
          colstart = cols[current_nesting_degree];
          colstop =
              (current_nesting_degree >= colspans_length) ? colstart + 1 : colstart + colspans[current_nesting_degree];

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
  unsigned int resample_method_flag;
  if (!grm_args_values(subplot_args, "resample_method", "i", &resample_method_flag))
    {
      const char *resample_method_str;
      grm_args_values(subplot_args, "resample_method", "s", &resample_method_str);
      if (strcmp(resample_method_str, "nearest") == 0)
        {
          resample_method_flag = GKS_K_RESAMPLE_NEAREST;
        }
      else if (strcmp(resample_method_str, "linear") == 0)
        {
          resample_method_flag = GKS_K_RESAMPLE_LINEAR;
        }
      else if (strcmp(resample_method_str, "lanczos") == 0)
        {
          resample_method_flag = GKS_K_RESAMPLE_LANCZOS;
        }
      else
        {
          resample_method_flag = GKS_K_RESAMPLE_DEFAULT;
        }
    }
  gr_setresamplemethod(resample_method_flag);
}

static void legend_size(grm_args_t *subplot_args, double *w, double *h)
{
  double tbx[4], tby[4];
  const char **labels, **current_label;
  unsigned int num_labels;

  *w = 0;
  *h = 0;
  if (grm_args_first_value(subplot_args, "labels", "S", &labels, &num_labels))
    {
      for (current_label = labels; *current_label != nullptr; ++current_label)
        {
          gr_inqtext(0, 0, *(char **)current_label, tbx, tby);
          *w = grm_max(*w, tbx[2] - tbx[0]);
          *h += grm_max(tby[2] - tby[0], 0.03);
        }
    }
}

void plot_process_window(grm_args_t *subplot_args)
{
  int scale = 0;
  const char *kind;
  int xlog, ylog, zlog;
  int xflip, yflip, zflip;
  double x_min, x_max, y_min, y_max, z_min, z_max;
  double rotation, tilt;

  auto group = global_root->lastChildElement();

  grm_args_values(subplot_args, "kind", "s", &kind);
  grm_args_values(subplot_args, "xlog", "i", &xlog);
  grm_args_values(subplot_args, "ylog", "i", &ylog);
  grm_args_values(subplot_args, "zlog", "i", &zlog);
  grm_args_values(subplot_args, "xflip", "i", &xflip);
  grm_args_values(subplot_args, "yflip", "i", &yflip);
  grm_args_values(subplot_args, "zflip", "i", &zflip);

  if (!str_equals_any(kind, 5, "pie", "polar", "polar_histogram", "polar_heatmap", "nonuniformpolar_heatmap"))
    {
      scale |= xlog ? GR_OPTION_X_LOG : 0;
      scale |= ylog ? GR_OPTION_Y_LOG : 0;
      scale |= zlog ? GR_OPTION_Z_LOG : 0;
      scale |= xflip ? GR_OPTION_FLIP_X : 0;
      scale |= yflip ? GR_OPTION_FLIP_Y : 0;
      scale |= zflip ? GR_OPTION_FLIP_Z : 0;
    }

  grm_args_values(subplot_args, "_xlim", "dd", &x_min, &x_max);
  grm_args_values(subplot_args, "_ylim", "dd", &y_min, &y_max);
  group->setAttribute("limits", true);
  group->setAttribute("lim_xmin", x_min);
  group->setAttribute("lim_xmax", x_max);
  group->setAttribute("lim_ymin", y_min);
  group->setAttribute("lim_ymax", y_max);

  if (grm_args_values(subplot_args, "_zlim", "dd", &z_min, &z_max))
    {
      group->setAttribute("lim_zmin", z_min);
      group->setAttribute("lim_zmax", z_max);
    }

  if (str_equals_any(kind, 6, "wireframe", "surface", "plot3", "scatter3", "trisurf", "volume"))
    {
      group->setAttribute("adjust_zlim", true);
      grm_args_values(subplot_args, "rotation", "d", &rotation);
      grm_args_values(subplot_args, "tilt", "d", &tilt);
      global_render->setSpace3d(group, rotation, tilt, 30.0, 0.0);
    }
  else if (strcmp(kind, "isosurface") == 0)
    {
      grm_args_values(subplot_args, "rotation", "d", &rotation);
      grm_args_values(subplot_args, "tilt", "d", &tilt);
      global_render->setWindow3d(group, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
      global_render->setSpace3d(group, rotation, tilt, 45.0, 2.5);
    }

  grm_args_push(subplot_args, "scale", "i", scale);
  global_render->setScale(global_root->lastChildElement(), scale);
}

err_t plot_store_coordinate_ranges(grm_args_t *subplot_args)
{
  const char *kind;
  const char *style = "";
  const char *fmt;
  grm_args_t **current_series;
  unsigned int series_count;
  grm_args_t **inner_series;
  unsigned int inner_series_count;
  const char *data_component_names[] = {"x", "y", "z", "c", nullptr};
  const char **current_component_name;
  double *current_component = nullptr;
  unsigned int current_point_count = 0;
  struct
  {
    const char *subplot;
    const char *series;
  } * current_range_keys,
      range_keys[] = {{"xlim", "xrange"}, {"ylim", "yrange"}, {"zlim", "zrange"}, {"clim", "crange"}};
  double *bins = nullptr;
  unsigned int i;
  err_t error = ERROR_NONE;

  logger((stderr, "Storing coordinate ranges\n"));

  /* If a pan and/or zoom was performed before, do not overwrite limits
   * -> the user fully controls limits by interaction */
  if (grm_args_contains(subplot_args, "_original_xlim"))
    {
      logger((stderr, "Panzoom active, do not modify limits...\n"));
      return ERROR_NONE;
    }

  grm_args_values(subplot_args, "kind", "s", &kind);
  grm_args_values(subplot_args, "style", "s", &style);
  cleanup_and_set_error_if(!string_map_at(fmt_map, kind, static_cast<const char **>(&fmt)), ERROR_PLOT_UNKNOWN_KIND);
  if (!str_equals_any(kind, 2, "pie", "polar_histogram"))
    {
      current_component_name = data_component_names;
      current_range_keys = range_keys;
      while (*current_component_name != nullptr)
        {
          double min_component = DBL_MAX;
          double max_component = -DBL_MAX;
          double step = -DBL_MAX;
          if (strchr(fmt, **current_component_name) == nullptr)
            {
              ++current_range_keys;
              ++current_component_name;
              continue;
            }
          /* Heatmaps need calculated range keys, so run the calculation even if limits are given */
          if (!grm_args_contains(subplot_args, current_range_keys->subplot) ||
              str_equals_any(kind, 3, "heatmap", "marginalheatmap", "polar_heatmap"))
            {
              grm_args_first_value(subplot_args, "series", "A", &current_series, &series_count);
              while (*current_series != nullptr)
                {
                  double current_min_component = DBL_MAX, current_max_component = -DBL_MAX;
                  if (!grm_args_values(*current_series, current_range_keys->series, "dd", &current_min_component,
                                       &current_max_component))
                    {
                      if (grm_args_first_value(*current_series, *current_component_name, "D", &current_component,
                                               &current_point_count))
                        {
                          if (strcmp(style, "stacked") == 0)
                            {
                              current_max_component = 0.0;
                              current_min_component = 0.0;
                              for (i = 0; i < current_point_count; i++)
                                {
                                  if (current_component[i] > 0)
                                    {
                                      current_max_component += current_component[i];
                                    }
                                  else
                                    {
                                      current_min_component += current_component[i];
                                    }
                                }
                            }
                          else
                            {
                              if (strcmp(kind, "barplot") == 0)
                                {
                                  current_min_component = 0.0;
                                  current_max_component = 0.0;
                                }
                              for (i = 0; i < current_point_count; i++)
                                {
                                  if (!is_nan(current_component[i]))
                                    {
                                      current_min_component = grm_min(current_component[i], current_min_component);
                                      current_max_component = grm_max(current_component[i], current_max_component);
                                    }
                                }
                            }
                        }
                      /* TODO: Add more plot types which can omit `x` */
                      else if (str_equals_any(kind, 1, "line") && strcmp(*current_component_name, "x") == 0)
                        {
                          double *y;
                          unsigned int y_length;
                          cleanup_and_set_error_if(!grm_args_first_value(*current_series, "y", "D", &y, &y_length),
                                                   ERROR_PLOT_MISSING_DATA);
                          current_min_component = 0.0;
                          current_max_component = y_length - 1;
                        }
                      else if (str_equals_any(kind, 4, "heatmap", "marginalheatmap", "polar_heatmap", "surface") &&
                               str_equals_any(*current_component_name, 2, "x", "y"))
                        {
                          /* in this case `x` or `y` (or both) are missing
                           * -> set the current grm_min/max_component to the dimensions of `z`
                           *    (shifted by half a unit to center color blocks) */
                          const char *other_component_name = (strcmp(*current_component_name, "x") == 0) ? "y" : "x";
                          double *other_component;
                          unsigned int other_point_count;
                          if (grm_args_first_value(*current_series, other_component_name, "D", &other_component,
                                                   &other_point_count))
                            {
                              /* The other component is given -> the missing dimension can be calculated */
                              double *z;
                              unsigned int z_length;
                              cleanup_and_set_error_if(!grm_args_first_value(*current_series, "z", "D", &z, &z_length),
                                                       ERROR_PLOT_MISSING_DATA);
                              current_point_count = z_length / other_point_count;
                            }
                          else
                            {
                              /* A heatmap/surface without `x` and `y` values
                               * -> dimensions can only be read from `z_dims` */
                              int rows, cols;
                              cleanup_and_set_error_if(!grm_args_values(*current_series, "z_dims", "ii", &rows, &cols),
                                                       ERROR_PLOT_MISSING_DIMENSIONS);
                              current_point_count = (strcmp(*current_component_name, "x") == 0) ? cols : rows;
                            }
                          current_min_component = 0.5;
                          current_max_component = current_point_count + 0.5;
                        }
                      else if (grm_args_first_value(*current_series, "inner_series", "nA", &inner_series,
                                                    &inner_series_count))
                        {
                          while (*inner_series != nullptr)
                            {
                              if (grm_args_first_value(*inner_series, *current_component_name, "D", &current_component,
                                                       &current_point_count))
                                {
                                  current_max_component = 0;
                                  current_min_component = 0;
                                  for (i = 0; i < current_point_count; i++)
                                    {
                                      if (!is_nan(current_component[i]))
                                        {

                                          if (current_component[i] > 0)
                                            {
                                              current_max_component += current_component[i];
                                            }
                                          else
                                            {
                                              current_min_component += current_component[i];
                                            }
                                        }
                                    }
                                  max_component = grm_max(current_max_component, max_component);
                                  min_component = grm_min(current_min_component, min_component);
                                }
                              inner_series++;
                            }
                        }
                    }
                  if (current_min_component != DBL_MAX && current_max_component != -DBL_MAX)
                    {
                      grm_args_push(*current_series, current_range_keys->series, "dd", current_min_component,
                                    current_max_component);
                    }
                  min_component = grm_min(current_min_component, min_component);
                  max_component = grm_max(current_max_component, max_component);
                  ++current_series;
                }
            }
          if (grm_args_values(subplot_args, current_range_keys->subplot, "dd", &min_component, &max_component))
            {
              grm_args_push(subplot_args, private_name(current_range_keys->subplot), "dd", min_component,
                            max_component);
            }
          else if (min_component != DBL_MAX && max_component != -DBL_MAX)
            {
              if (strcmp(kind, "quiver") == 0)
                {
                  step = grm_max(find_max_step(current_point_count, current_component), step);
                  if (step > 0.0)
                    {
                      min_component -= step;
                      max_component += step;
                    }
                }
              grm_args_push(subplot_args, private_name(current_range_keys->subplot), "dd", min_component,
                            max_component);
            }
          ++current_range_keys;
          ++current_component_name;
        }
    }
  else if (strcmp(kind, "polar_histogram") == 0)
    {

      // todo: xlim ylim in render???
      auto temp = global_root->lastChildElement();
      //      temp->setAttribute("")
      grm_args_push(subplot_args, "_xlim", "dd", -1.0, 1.0);
      grm_args_push(subplot_args, "_ylim", "dd", -1.0, 1.0);
    }

  /* For quiver plots use u^2 + v^2 as z value */
  if (strcmp(kind, "quiver") == 0)
    {
      double min_component = DBL_MAX;
      double max_component = -DBL_MAX;
      if (!grm_args_values(subplot_args, "zlim", "dd", &min_component, &max_component))
        {
          grm_args_values(subplot_args, "series", "A", &current_series);
          while (*current_series != nullptr)
            {
              double current_min_component = DBL_MAX;
              double current_max_component = -DBL_MAX;
              if (!grm_args_values(*current_series, "zrange", "dd", &current_min_component, &current_max_component))
                {
                  double *u, *v;
                  unsigned int u_length, v_length;
                  cleanup_and_set_error_if(!grm_args_first_value(*current_series, "u", "D", &u, &u_length),
                                           ERROR_PLOT_MISSING_DATA);
                  cleanup_and_set_error_if(!grm_args_first_value(*current_series, "v", "D", &v, &v_length),
                                           ERROR_PLOT_MISSING_DATA);
                  cleanup_and_set_error_if(u_length != v_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
                  for (i = 0; i < u_length; i++)
                    {
                      double z = u[i] * u[i] + v[i] * v[i];
                      current_min_component = grm_min(z, current_min_component);
                      current_max_component = grm_max(z, current_max_component);
                    }
                  current_min_component = sqrt(current_min_component);
                  current_max_component = sqrt(current_max_component);
                }
              min_component = grm_min(current_min_component, min_component);
              max_component = grm_max(current_max_component, max_component);
              ++current_series;
            }
        }
      grm_args_push(subplot_args, "_zlim", "dd", min_component, max_component);
    }
  else if (str_equals_any(kind, 3, "imshow", "isosurface", "volume"))
    {
      /* Iterate over `x` and `y` range keys (and `z` depending on `kind`) */
      current_range_keys = range_keys;
      for (i = 0; i < (strcmp(kind, "imshow") == 0 ? 2 : 3); i++)
        {
          double min_component = (strcmp(kind, "imshow") == 0 ? 0.0 : -1.0);
          double max_component = 1.0;
          grm_args_values(subplot_args, current_range_keys->subplot, "dd", &min_component, &max_component);
          grm_args_push(subplot_args, private_name(current_range_keys->subplot), "dd", min_component, max_component);
          ++current_range_keys;
        }
    }
  else if (strcmp(kind, "barplot") == 0)
    {
      double x_min = 0.0, x_max = -DBL_MAX, y_min = DBL_MAX, y_max = -DBL_MAX;
      char *orientation;

      grm_args_values(subplot_args, "orientation", "s", &orientation);
      if (!grm_args_values(subplot_args, "xlim", "dd", &x_min, &x_max))
        {
          double xmin, xmax, ymin, ymax;
          if (str_equals_any(style, 2, "lined", "stacked"))
            {
              x_max = series_count + 1;
            }
          else
            {
              grm_args_values(subplot_args, "series", "A", &current_series);
              while (*current_series != nullptr)
                {
                  double *y;
                  grm_args_first_value(*current_series, "y", "D", &y, &current_point_count);
                  x_max = grm_max(current_point_count + 1, x_max);
                  ++current_series;
                }
            }

          grm_args_values(subplot_args, "series", "A", &current_series);
          while (*current_series != nullptr)
            {
              double *y;
              grm_args_first_value(*current_series, "y", "D", &y, &current_point_count);

              if (grm_args_values(*current_series, "xrange", "dd", &xmin, &xmax))
                {
                  double step_x = (xmax - xmin) / (current_point_count - 1);
                  if (!str_equals_any(style, 2, "lined", "stacked"))
                    {
                      x_min = xmin - step_x;
                      x_max = xmax + step_x;
                    }
                  else
                    {
                      x_min = xmin - (x_max - 1);
                      x_max = xmin + (x_max - 1);
                    }
                }

              if (grm_args_values(*current_series, "yrange", "dd", &ymin, &ymax))
                {
                  y_min = grm_min(y_min, ymin);
                  if (strcmp(style, "stacked") == 0)
                    {
                      double tmp_ymax;
                      tmp_ymax = ymin;
                      for (i = 0; i < current_point_count; i++)
                        {
                          if (y_min < 0)
                            {
                              tmp_ymax += fabs(y[i]);
                            }
                          else
                            {
                              tmp_ymax += y[i] - y_min;
                            }
                        }
                      y_max = grm_max(y_max, tmp_ymax);
                    }
                  else
                    {
                      y_max = grm_max(y_max, ymax);
                    }
                }
              else
                {
                  grm_args_values(*current_series, "ylim", "dd", &y_min, &ymax);
                }
              ++current_series;
            }
        }

      if (strcmp(orientation, "horizontal") == 0)
        {
          grm_args_push(subplot_args, "_xlim", "dd", x_min, x_max);
          grm_args_push(subplot_args, "_ylim", "dd", y_min, y_max);
        }
      else
        {
          grm_args_push(subplot_args, "_xlim", "dd", y_min, y_max);
          grm_args_push(subplot_args, "_ylim", "dd", x_min, x_max);
        }
    }
  else if (strcmp(kind, "hist") == 0)
    {
      double x_min = 0.0, x_max = 0.0, y_min = 0.0, y_max = 0.0;
      char *orientation;
      int is_horizontal;
      double xmin, xmax;

      if (!grm_args_values(subplot_args, "ylim", "dd", &y_min, &y_max))
        {
          grm_args_values(subplot_args, "series", "A", &current_series);
          grm_args_values(subplot_args, "orientation", "s", &orientation);
          is_horizontal = strcmp(orientation, "horizontal") == 0;
          while (*current_series != nullptr)
            {
              double current_y_min = DBL_MAX, current_y_max = -DBL_MAX;
              {
                double *x = nullptr, *weights = nullptr;
                unsigned int num_bins = 0, num_weights;
                cleanup_and_set_error_if(!grm_args_first_value(*current_series, "x", "D", &x, &current_point_count),
                                         ERROR_PLOT_MISSING_DATA);
                grm_args_values(*current_series, "nbins", "i", &num_bins);
                grm_args_first_value(*current_series, "weights", "D", &weights, &num_weights);
                if (weights != nullptr)
                  {
                    cleanup_and_set_error_if(current_point_count != num_weights, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
                  }
                if (num_bins <= 1)
                  {
                    num_bins = (int)(3.3 * log10(current_point_count) + 0.5) + 1;
                  }
                bins = static_cast<double *>(malloc(num_bins * sizeof(double)));
                cleanup_and_set_error_if(bins == nullptr, ERROR_MALLOC);
                bin_data(current_point_count, x, num_bins, bins, weights);
                for (i = 0; i < num_bins; i++)
                  {
                    current_y_min = grm_min(current_y_min, bins[i]);
                    current_y_max = grm_max(current_y_max, bins[i]);
                  }
                grm_args_push(*current_series, "bins", "nD", num_bins, bins);
                free(bins);
                bins = nullptr;
              }
              y_min = grm_min(current_y_min, y_min);
              y_max = grm_max(current_y_max, y_max);
              x_max = current_point_count - 1;
              if (grm_args_values(*current_series, "yrange", "dd", &current_y_min, &current_y_max))
                {
                  y_min = current_y_min;
                  y_max = current_y_max;
                }
              if (grm_args_values(*current_series, "xrange", "dd", &xmin, &xmax))
                {
                  x_min = xmin;
                  x_max = xmax;
                }
              current_series++;
            }
          if (is_horizontal)
            {
              grm_args_push(subplot_args, "_xlim", "dd", x_min, x_max);
              grm_args_push(subplot_args, "_ylim", "dd", y_min, y_max);
            }
          else
            {
              grm_args_push(subplot_args, "_xlim", "dd", y_min, y_max);
              grm_args_push(subplot_args, "_ylim", "dd", x_min, x_max);
            }
        }
    }
  else if (strcmp(kind, "polar_histogram") == 0)
    {
      // todo: move series/data dependent code to render.cxx
      //      double r_max;
      //      error = classes_polar_histogram(subplot_args, &r_max);
      //      cleanup_if_error;
      //      global_root->lastChildElement()->setAttribute("r_max", r_max);
      //      grm_args_push(subplot_args, "r_max", "d", r_max);
    }
  else if (str_equals_any(kind, 2, "stem", "stairs"))
    {
      double x_min = 0.0, x_max = 0.0, y_min = 0.0, y_max = 0.0;
      char *orientation;
      int is_horizontal;

      grm_args_values(subplot_args, "series", "A", &current_series);
      grm_args_values(subplot_args, "orientation", "s", &orientation);
      is_horizontal = strcmp(orientation, "horizontal") == 0;
      while (*current_series != nullptr)
        {
          if (grm_args_values(*current_series, "xrange", "dd", &x_min, &x_max))
            {
              if (is_horizontal)
                grm_args_push(subplot_args, "_xlim", "dd", x_min, x_max);
              else
                grm_args_push(subplot_args, "_ylim", "dd", x_min, x_max);
            }
          if (grm_args_values(*current_series, "yrange", "dd", &y_min, &y_max))
            {
              if (is_horizontal)
                grm_args_push(subplot_args, "_ylim", "dd", y_min, y_max);
              else
                grm_args_push(subplot_args, "_xlim", "dd", y_min, y_max);
            }
          current_series++;
        }
    }

cleanup:
  free(bins);

  return error;
}

void plot_post_plot(grm_args_t *plot_args)
{
  int update;

  logger((stderr, "Post plot processing\n"));

  grm_args_values(plot_args, "update", "i", &update);
  logger((stderr, "Got keyword \"update\" with value %d\n", update));
  global_root->setAttribute("updatews", update);
  plot_restore_text_encoding();
}

void plot_restore_text_encoding(void)
{
  gr_inqtextencoding(&pre_plot_text_encoding);
  if (pre_plot_text_encoding >= 0)
    {
      gr_settextencoding(pre_plot_text_encoding);
      pre_plot_text_encoding = -1;
    }
}

void plot_post_subplot(grm_args_t *subplot_args)
{
  const char *kind;

  logger((stderr, "Post subplot processing\n"));

  gr_restorestate();
  grm_args_values(subplot_args, "kind", "s", &kind);
  logger((stderr, "Got keyword \"kind\" with value \"%s\"\n", kind));
  if (grm_args_contains(subplot_args, "labels"))
    {
      if (str_equals_any(kind, 4, "line", "stairs", "scatter", "stem"))
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
  else if (str_equals_any(kind, 2, "polar_heatmap", "nonuniformpolar_heatmap"))
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
                  error = event_queue_enqueue_update_plot_event(event_queue, current_id - 1);
                }
              else
                {
                  error = event_queue_enqueue_new_plot_event(event_queue, current_id - 1);
                }
              return_if_error;
              grm_args_push(current_args, "in_use", "i", 1);
            }
          if (strcmp(*current_hierarchy_name_ptr, key_hierarchy_name) == 0)
            {
              break;
            }
        }
      return_error_if(*current_hierarchy_name_ptr == nullptr, ERROR_INTERNAL);
    }
  if (found_args != nullptr)
    {
      *found_args = current_args;
    }
  if (found_hierarchy_name_ptr != nullptr)
    {
      *found_hierarchy_name_ptr = current_hierarchy_name_ptr;
    }

  return ERROR_NONE;
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plotting ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

err_t plot_line(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  err_t error = ERROR_NONE;
  const char *kind, *orientation;

  grm_args_values(subplot_args, "series", "A", &current_series);
  grm_args_values(subplot_args, "kind", "s", &kind);
  grm_args_values(subplot_args, "orientation", "s", &orientation);
  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();
  group->setAttribute("name", "line");

  while (*current_series != nullptr)
    {
      double *x = nullptr, *y = nullptr;
      unsigned int x_length = 0, y_length = 0;
      char *spec;
      auto subGroup = global_render->createSeries("line");
      group->append(subGroup);

      int id = static_cast<int>(global_root->getAttribute("id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      cleanup_and_set_error_if(!grm_args_first_value(*current_series, "y", "D", &y, &y_length),
                               ERROR_PLOT_MISSING_DATA);

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
          cleanup_and_set_error_if(x_length != y_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
        }

      subGroup->setAttribute("orientation", orientation);
      grm_args_values(*current_series, "spec", "s", &spec); /* `spec` is always set */
      subGroup->setAttribute("spec", spec);

      global_root->setAttribute("id", ++id);
      grm_args_push(*current_series, "orientation", "s", orientation);
      error = plot_draw_errorbars(*current_series, x, x_length, y, kind);
      cleanup_if_error;
      ++current_series;

    cleanup:
      x = y = nullptr;
      x_length = y_length = 0;

      if (error != ERROR_NONE)
        {
          break;
        }
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
  char *kind, *orientation;
  double xmin, xmax, ymin, ymax;
  double *x = nullptr, *y = nullptr, *xi;
  int is_vertical;
  err_t error = ERROR_NONE;

  grm_args_values(subplot_args, "series", "A", &current_series);
  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();
  if (!global_root->lastChildElement()->hasAttribute("name")) group->setAttribute("name", "stairs");

  grm_args_values(subplot_args, "kind", "s", &kind);
  grm_args_values(subplot_args, "orientation", "s", &orientation);
  is_vertical = strcmp(orientation, "vertical") == 0;

  std::shared_ptr<GRM::Element> element; // declare element here for multiple usages / assignments later
  while (*current_series != nullptr)
    {
      unsigned int x_length, y_length;
      char *spec;
      const char *where;
      auto subGroup = global_render->createSeries("stairs");
      group->append(subGroup);

      subGroup->setAttribute("kind", kind);
      subGroup->setAttribute("orientation", orientation);

      return_error_if(!grm_args_first_value(*current_series, "x", "D", &x, &x_length) && x_length < 1,
                      ERROR_PLOT_MISSING_DATA);
      return_error_if(!grm_args_first_value(*current_series, "y", "D", &y, &y_length), ERROR_PLOT_MISSING_DATA);

      int id = static_cast<int>(global_root->getAttribute("id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> x_vec(x, x + x_length);
      (*context)["x" + str] = x_vec;
      subGroup->setAttribute("x", "x" + str);

      std::vector<double> y_vec(y, y + y_length);
      (*context)["y" + str] = y_vec;
      subGroup->setAttribute("y", "y" + str);

      if (subGroup->parentElement()->hasAttribute("marginalheatmap_kind"))
        {
          double y_max = 0, *plot;
          unsigned int n = 0;

          grm_args_values(*current_series, "xrange", "dd", &xmin, &xmax);
          global_root->lastChildElement()->setAttribute("xrange_min", xmin);
          global_root->lastChildElement()->setAttribute("xrange_max", xmax);
          grm_args_values(*current_series, "yrange", "dd", &ymin, &ymax);
          global_root->lastChildElement()->setAttribute("yrange_min", ymin);
          global_root->lastChildElement()->setAttribute("yrange_max", ymax);
          grm_args_first_value(*current_series, "z", "D", &plot, &n);

          std::vector<double> z_vec(plot, plot + n);
          (*context)["z" + str] = z_vec;
          subGroup->setAttribute("z", "z" + str);
        }
      else
        {
          return_error_if(x_length != y_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
        }

      grm_args_values(*current_series, "spec", "s", &spec); /* `spec` is always set */
      subGroup->setAttribute("spec", spec);

      if (grm_args_values(*current_series, "step_where", "s", &where)) subGroup->setAttribute("step_where", where);

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
   * optional `markertype` as integer (see: [Marker types](https://gr-framework.org/markertypes.html?highlight=marker))
   */
  grm_args_t **current_series;
  err_t error;
  char *kind;
  char *orientation;
  int *previous_marker_type = plot_scatter_markertypes;

  grm_args_values(subplot_args, "orientation", "s", &orientation);
  grm_args_values(subplot_args, "series", "A", &current_series);
  grm_args_values(subplot_args, "kind", "s", &kind);
  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();
  group->setAttribute("name", "scatter");

  while (*current_series != nullptr)
    {
      auto subGroup = global_render->createSeries("scatter");
      group->append(subGroup);
      subGroup->setAttribute("orientation", orientation);

      double *x = nullptr, *y = nullptr, *z = nullptr, *c = nullptr, c_min, c_max;
      unsigned int x_length, y_length, z_length, c_length;
      int i, c_index = -1, markertype;

      return_error_if(!grm_args_first_value(*current_series, "x", "D", &x, &x_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(!grm_args_first_value(*current_series, "y", "D", &y, &y_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(x_length != y_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);

      int id = static_cast<int>(global_root->getAttribute("id"));
      std::string str = std::to_string(id);
      global_root->setAttribute("id", ++id);
      auto context = global_render->getContext();

      std::vector<double> x_vec(x, x + x_length);
      std::vector<double> y_vec(y, y + y_length);

      (*context)["x" + str] = x_vec;
      subGroup->setAttribute("x", "x" + str);
      (*context)["y" + str] = y_vec;
      subGroup->setAttribute("y", "y" + str);
      if (grm_args_first_value(*current_series, "z", "D", &z, &z_length))
        {
          return_error_if(x_length != z_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);

          std::vector<double> z_vec(z, z + z_length);

          (*context)["z" + str] = z_vec;
          subGroup->setAttribute("z", "z" + str);
        }
      if (grm_args_values(*current_series, "markertype", "i", &markertype))
        {
          subGroup->setAttribute("markertype", markertype);
        }
      else
        {
          subGroup->setAttribute("markertype", *previous_marker_type++);
          if (*previous_marker_type == INT_MAX)
            {
              previous_marker_type = plot_scatter_markertypes;
            }
        }

      if (grm_args_first_value(*current_series, "c", "D", &c, &c_length))
        {
          std::vector<double> c_vec(c, c + c_length);

          (*context)["c" + str] = c_vec;
          subGroup->setAttribute("c", "c" + str);
        }
      if (grm_args_values(*current_series, "c", "i", &c_index))
        {
          subGroup->setAttribute("c_index", c_index);
        }

      if (z != nullptr || c != nullptr)
        {
          grm_args_values(subplot_args, "_clim", "dd", &c_min, &c_max);
          subGroup->setAttribute("c_min", c_min);
          subGroup->setAttribute("c_max", c_max);
        }

      grm_args_push(*current_series, "orientation", "s", orientation);
      error = plot_draw_errorbars(*current_series, x, x_length, y, kind);
      return_if_error;
      ++current_series;
    }

  return ERROR_NONE;
}

err_t plot_quiver(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  err_t error = ERROR_NONE;

  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();
  group->setAttribute("name", "quiver");

  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double *x = nullptr, *y = nullptr, *u = nullptr, *v = nullptr;
      unsigned int x_length, y_length, u_length, v_length;
      return_error_if(!grm_args_first_value(*current_series, "x", "D", &x, &x_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(!grm_args_first_value(*current_series, "y", "D", &y, &y_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(!grm_args_first_value(*current_series, "u", "D", &u, &u_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(!grm_args_first_value(*current_series, "v", "D", &v, &v_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(x_length * y_length != u_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
      return_error_if(x_length * y_length != v_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);

      std::vector<double> x_vec(x, x + x_length);
      std::vector<double> y_vec(y, y + y_length);
      std::vector<double> u_vec(u, u + x_length * y_length);
      std::vector<double> v_vec(v, v + x_length * y_length);

      int id = (int)global_root->getAttribute("id");
      std::string str = std::to_string(id);
      global_root->setAttribute("id", id + 1);
      group->append(
          global_render->createQuiver("x" + str, x_vec, "y" + str, y_vec, "u" + str, u_vec, "v" + str, v_vec, 1));

      ++current_series;
    }
  error = plot_draw_colorbar(subplot_args, 0.0, 256);

  return error;
}

err_t plot_stem(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();

  group->setAttribute("name", "stem");
  char *orientation;

  grm_args_values(subplot_args, "series", "A", &current_series);
  grm_args_values(subplot_args, "orientation", "s", &orientation);

  while (*current_series != nullptr)
    {
      double *x, *y;
      unsigned int x_length, y_length;
      char *spec;
      double y_min, y_max;

      auto subGroup = global_render->createSeries("stem");
      group->append(subGroup);

      subGroup->setAttribute("orientation", orientation);

      return_error_if(!grm_args_first_value(*current_series, "x", "D", &x, &x_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(!grm_args_first_value(*current_series, "y", "D", &y, &y_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(x_length != y_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);

      int id = static_cast<int>(global_root->getAttribute("id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> x_vec(x, x + x_length);
      std::vector<double> y_vec(y, y + x_length);

      (*context)["x" + str] = x_vec;
      subGroup->setAttribute("x", "x" + str);
      (*context)["y" + str] = y_vec;
      subGroup->setAttribute("y", "y" + str);

      if (grm_args_values(*current_series, "yrange", "dd", &y_min, &y_max)) group->setAttribute("yrange_min", y_min);

      grm_args_values(*current_series, "spec", "s", &spec);
      subGroup->setAttribute("spec", spec);

      global_root->setAttribute("id", ++id);
      ++current_series;
    }

  return ERROR_NONE;
}

err_t plot_hist(grm_args_t *subplot_args)
{
  char *kind;
  grm_args_t **current_series;
  double *bar_centers = nullptr;
  int bar_color_index = 989, i;
  double bar_color_rgb[3] = {-1};
  err_t error = ERROR_NONE;

  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();
  if (!global_root->lastChildElement()->hasAttribute("name"))
    global_root->lastChildElement()->setAttribute("name", "hist");

  grm_args_values(subplot_args, "kind", "s", &kind);
  grm_args_values(subplot_args, "series", "A", &current_series);
  grm_args_values(subplot_args, "bar_color", "ddd", &bar_color_rgb[0], &bar_color_rgb[1], &bar_color_rgb[2]);
  grm_args_values(subplot_args, "bar_color", "i", &bar_color_index);

  while (*current_series != nullptr)
    {
      int edge_color_index = 1;
      double edge_color_rgb[3] = {-1};
      double x_min, x_max, bar_width, y_min, y_max;
      double *bins;
      unsigned int num_bins;
      char *orientation;
      int is_horizontal;

      auto subGroup = global_render->createSeries("hist");
      group->append(subGroup);

      int id = static_cast<int>(global_root->getAttribute("id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> bar_color_rgb_vec(bar_color_rgb, bar_color_rgb + 3);
      (*context)["bar_color_rgb" + str] = bar_color_rgb_vec;
      subGroup->setAttribute("bar_color_rgb", "bar_color_rgb" + str);
      subGroup->setAttribute("bar_color_index", bar_color_index);

      grm_args_values(*current_series, "edge_color", "ddd", &edge_color_rgb[0], &edge_color_rgb[1], &edge_color_rgb[2]);
      grm_args_values(*current_series, "edge_color", "i", &edge_color_index);
      std::vector<double> edge_color_rgb_vec(edge_color_rgb, edge_color_rgb + 3);
      (*context)["edge_color_rgb" + str] = edge_color_rgb_vec;
      subGroup->setAttribute("edge_color_rgb", "edge_color_rgb" + str);
      subGroup->setAttribute("edge_color_index", edge_color_index);

      grm_args_first_value(*current_series, "bins", "D", &bins, &num_bins);
      std::vector<double> bins_vec(bins, bins + num_bins);
      (*context)["bins" + str] = bins_vec;
      subGroup->setAttribute("bins", "bins" + str);

      grm_args_values(subplot_args, "orientation", "s", &orientation);
      subGroup->setAttribute("orientation", orientation);

      grm_args_values(*current_series, "xrange", "dd", &x_min, &x_max);
      global_root->lastChildElement()->setAttribute("xrange_min", x_min);
      global_root->lastChildElement()->setAttribute("xrange_max", x_max);

      grm_args_values(*current_series, "yrange", "dd", &y_min, &y_max);
      global_root->lastChildElement()->setAttribute("yrange_min", y_min);

      bar_width = (x_max - x_min) / num_bins;

      // TODO: Move this into render
      if (grm_args_contains(*current_series, "error"))
        {
          bar_centers = static_cast<double *>(malloc(num_bins * sizeof(double)));
          cleanup_and_set_error_if(bar_centers == nullptr, ERROR_MALLOC);
          linspace(x_min + 0.5 * bar_width, x_max - 0.5 * bar_width, num_bins, bar_centers);
          grm_args_push(*current_series, "orientation", "s", orientation);
          error = plot_draw_errorbars(*current_series, bar_centers, num_bins, bins, kind);
          cleanup_if_error;
          free(bar_centers);
          bar_centers = nullptr;
        }
      global_root->setAttribute("id", ++id);
      ++current_series;
    }

cleanup:
  free(bar_centers);

  return error;
}

err_t plot_barplot(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  grm_args_t **inner_series;
  unsigned int inner_series_length;

  /* Default */
  int bar_color = 989, edge_color = 1;
  double bar_color_rgb[3] = {-1};
  double edge_color_rgb[3] = {-1};
  double bar_width = 0.8, edge_width = 1.0, bar_shift = 1;
  const char *style = "default";
  double *y;
  unsigned int y_length;
  unsigned int fixed_y_length = 0;
  grm_args_t **ind_bar_color = nullptr;
  double(*pos_ind_bar_color)[3] = nullptr;
  grm_args_t **ind_edge_color = nullptr;
  double(*pos_ind_edge_color)[3] = nullptr;
  grm_args_t **ind_edge_width = nullptr;
  double *pos_ind_edge_width = nullptr;
  int series_index = 0;
  double wfac;
  int len_std_colors = 20;
  int std_colors[20] = {989, 982, 980, 981, 996, 983, 995, 988, 986, 990,
                        991, 984, 992, 993, 994, 987, 985, 997, 998, 999};
  int color_save_spot = 1000;
  int change_bar_color = 0;
  int change_edge_color = 0;
  int change_edge_width = 0;
  unsigned int i;
  err_t error = ERROR_NONE;
  double *y_lightness = nullptr;
  char *orientation;
  int is_vertical;

  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();
  gr_savestate();
  group->setAttribute("name", "barplot");

  grm_args_values(subplot_args, "series", "A", &current_series);
  grm_args_values(subplot_args, "bar_color", "ddd", &bar_color_rgb[0], &bar_color_rgb[1], &bar_color_rgb[2]);
  grm_args_values(subplot_args, "bar_color", "i", &bar_color);
  grm_args_values(subplot_args, "bar_width", "d", &bar_width);
  grm_args_values(subplot_args, "style", "s", &style);
  grm_args_values(subplot_args, "orientation", "s", &orientation);

  is_vertical = strcmp(orientation, "vertical") == 0;

  if (bar_color_rgb[0] != -1)
    {
      for (i = 0; i < 3; i++)
        {
          cleanup_and_set_error_if((bar_color_rgb[i] > 1 || bar_color_rgb[i] < 0), ERROR_PLOT_OUT_OF_RANGE);
        }
    }

  /* ind_parameter */
  /* determine the length of y */
  while (*current_series != nullptr)
    {
      if (!grm_args_first_value(*current_series, "y", "D", &y, &y_length))
        {
          cleanup_and_set_error_if(
              !grm_args_first_value(*current_series, "inner_series", "A", &inner_series, &inner_series_length),
              ERROR_PLOT_MISSING_DATA);
          y_length = inner_series_length;
        }
      fixed_y_length = grm_max(y_length, fixed_y_length);
      ++current_series;
    }

  /* ind_bar_color */
  if (grm_args_values(subplot_args, "ind_bar_color", "A", &ind_bar_color))
    {
      pos_ind_bar_color = static_cast<double(*)[3]>(malloc(3 * fixed_y_length * sizeof(double)));
      cleanup_and_set_error_if(pos_ind_bar_color == nullptr, ERROR_MALLOC);
      change_bar_color = 1;
      for (i = 0; i < fixed_y_length; ++i)
        {
          pos_ind_bar_color[i][0] = -1;
        }
      while (*ind_bar_color != nullptr)
        {
          int *indices = nullptr;
          unsigned int indices_length;
          int index;
          double rgb[3];
          unsigned int j;
          cleanup_and_set_error_if(!(grm_args_first_value(*ind_bar_color, "indices", "I", &indices, &indices_length) ||
                                     grm_args_values(*ind_bar_color, "indices", "i", &index)),
                                   ERROR_PLOT_MISSING_DATA);
          cleanup_and_set_error_if(!grm_args_values(*ind_bar_color, "rgb", "ddd", &rgb[0], &rgb[1], &rgb[2]),
                                   ERROR_PLOT_MISSING_DATA);
          for (j = 0; j < 3; j++)
            {
              cleanup_and_set_error_if((rgb[j] > 1 || rgb[j] < 0), ERROR_PLOT_OUT_OF_RANGE);
            }

          if (indices != nullptr)
            {
              for (j = 0; j < indices_length; ++j)
                {
                  cleanup_and_set_error_if(indices[j] - 1 >= (int)fixed_y_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
                  pos_ind_bar_color[indices[j] - 1][0] = rgb[0];
                  pos_ind_bar_color[indices[j] - 1][1] = rgb[1];
                  pos_ind_bar_color[indices[j] - 1][2] = rgb[2];
                }
            }
          else
            {
              cleanup_and_set_error_if(index - 1 >= (int)fixed_y_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
              pos_ind_bar_color[index - 1][0] = rgb[0];
              pos_ind_bar_color[index - 1][1] = rgb[1];
              pos_ind_bar_color[index - 1][2] = rgb[2];
            }
          ind_bar_color++;
        }
    }

  /* ind_edge_color */
  if (grm_args_values(subplot_args, "ind_edge_color", "A", &ind_edge_color))
    {
      pos_ind_edge_color = static_cast<double(*)[3]>(malloc(3 * fixed_y_length * sizeof(double)));
      cleanup_and_set_error_if(pos_ind_edge_color == nullptr, ERROR_MALLOC);
      change_edge_color = 1;
      for (i = 0; i < fixed_y_length; ++i)
        {
          pos_ind_edge_color[i][0] = -1;
        }
      while (*ind_edge_color != nullptr)
        {
          int *indices = nullptr;
          unsigned int indices_length;
          int index;
          double rgb[3];
          unsigned int j;
          cleanup_and_set_error_if(!(grm_args_first_value(*ind_edge_color, "indices", "I", &indices, &indices_length) ||
                                     grm_args_values(*ind_edge_color, "indices", "i", &index)),
                                   ERROR_PLOT_MISSING_DATA);
          cleanup_and_set_error_if(!grm_args_values(*ind_edge_color, "rgb", "ddd", &rgb[0], &rgb[1], &rgb[2]),
                                   ERROR_PLOT_MISSING_DATA);
          for (j = 0; j < 3; j++)
            {
              cleanup_and_set_error_if((rgb[j] > 1 || rgb[j] < 0), ERROR_PLOT_OUT_OF_RANGE);
            }

          if (indices != nullptr)
            {
              for (j = 0; j < indices_length; ++j)
                {
                  cleanup_and_set_error_if(indices[j] - 1 >= (int)fixed_y_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
                  pos_ind_edge_color[indices[j] - 1][0] = rgb[0];
                  pos_ind_edge_color[indices[j] - 1][1] = rgb[1];
                  pos_ind_edge_color[indices[j] - 1][2] = rgb[2];
                }
            }
          else
            {
              cleanup_and_set_error_if(index - 1 >= (int)fixed_y_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
              pos_ind_edge_color[index - 1][0] = rgb[0];
              pos_ind_edge_color[index - 1][1] = rgb[1];
              pos_ind_edge_color[index - 1][2] = rgb[2];
            }
          ind_edge_color++;
        }
    }

  /* ind_edge_width */
  if (grm_args_values(subplot_args, "ind_edge_width", "A", &ind_edge_width))
    {
      pos_ind_edge_width = static_cast<double *>(malloc(sizeof(double) * fixed_y_length));
      cleanup_and_set_error_if(pos_ind_edge_width == nullptr, ERROR_MALLOC);
      for (i = 0; i < fixed_y_length; ++i)
        {
          pos_ind_edge_width[i] = -1;
        }
      change_edge_width = 1;
      while (*ind_edge_width != nullptr)
        {
          int *indices = nullptr;
          unsigned int indices_length;
          int index;
          double width;
          unsigned int j;
          cleanup_and_set_error_if(!(grm_args_first_value(*ind_edge_width, "indices", "I", &indices, &indices_length) ||
                                     grm_args_values(*ind_edge_width, "indices", "i", &index)),
                                   ERROR_PLOT_MISSING_DATA);
          cleanup_and_set_error_if(!grm_args_values(*ind_edge_width, "width", "d", &width), ERROR_PLOT_MISSING_DATA);
          cleanup_and_set_error_if(width < 0, ERROR_PLOT_OUT_OF_RANGE);

          if (indices != nullptr)
            {
              for (j = 0; j < indices_length; ++j)
                {
                  cleanup_and_set_error_if(indices[j] - 1 >= (int)fixed_y_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
                  pos_ind_edge_width[indices[j] - 1] = width;
                }
            }
          else
            {
              cleanup_and_set_error_if(index - 1 >= (int)fixed_y_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
              pos_ind_edge_width[index - 1] = width;
            }
          ind_edge_width++;
        }
    }

  grm_args_values(subplot_args, "series", "A", &current_series);
  wfac = 0.9 * bar_width;
  while (*current_series != nullptr)
    {
      /* Init */
      int inner_series_index;
      double *y = nullptr;
      unsigned int y_length = 0;
      grm_args_t **inner_series = nullptr;
      unsigned int inner_series_length = 0;
      int *c = nullptr;
      unsigned int c_length;
      double *c_rgb = nullptr;
      unsigned int c_rgb_length;
      char **ylabels = nullptr;
      unsigned int ylabels_left = 0;
      unsigned int ylabels_length = 0;
      unsigned int y_lightness_to_get = 0;
      unsigned char rgb[sizeof(int)];
      int use_y_notations_from_inner_series = 1;
      double Y;
      int color;
      /* Style Varianz */
      double pos_vertical_change = 0;
      double neg_vertical_change = 0;
      double x1, x2, y1, y2;
      double x_min = 0, x_max, y_min = 0, y_max;

      auto subGroup = global_render->createSeries("bar_series");
      group->append(subGroup);

      grm_args_values(*current_series, "edge_color", "ddd", &edge_color_rgb[0], &edge_color_rgb[1], &edge_color_rgb[2]);
      grm_args_values(*current_series, "edge_color", "i", &edge_color);
      global_render->setTextAlign(subGroup, 2, 3);
      global_render->selectClipXForm(subGroup, 1);

      if (edge_color_rgb[0] != -1)
        {
          for (i = 0; i < 3; i++)
            {
              cleanup_and_set_error_if((edge_color_rgb[i] > 1 || edge_color_rgb[i] < 0), ERROR_PLOT_OUT_OF_RANGE);
            }
        }
      grm_args_values(*current_series, "edge_width", "d", &edge_width);

      grm_args_first_value(*current_series, "c", "I", &c, &c_length);
      grm_args_first_value(*current_series, "c", "D", &c_rgb, &c_rgb_length);
      grm_args_first_value(*current_series, "ylabels", "S", &ylabels, &ylabels_length);
      ylabels_left = ylabels_length;
      y_lightness_to_get = ylabels_length;

      cleanup_and_set_error_if(
          !(grm_args_first_value(*current_series, "y", "D", &y, &y_length) ||
            (grm_args_first_value(*current_series, "inner_series", "A", &inner_series, &inner_series_length))),
          ERROR_PLOT_MISSING_DATA);

      if (grm_args_values(*current_series, "xrange", "dd", &x_min, &x_max) &&
          grm_args_values(*current_series, "yrange", "dd", &y_min, &y_max))
        {
          if (!grm_args_values(subplot_args, "bar_width", "d", &bar_width))
            {
              bar_width = (x_max - x_min) / (y_length - 1.0);
              bar_shift = (x_max - x_min) / (y_length - 1.0);
              x_min -= 1; // in the later calculation there is allways a +1 in combination with x
              wfac = 0.9 * bar_width;
            }
        }

      cleanup_and_set_error_if(strcmp(style, "lined") && inner_series != nullptr, ERROR_UNSUPPORTED_OPERATION);
      cleanup_and_set_error_if(y != nullptr && inner_series != nullptr, ERROR_PLOT_INCOMPATIBLE_ARGUMENTS);
      if (c != nullptr)
        {
          cleanup_and_set_error_if((c_length < y_length) && (c_length < inner_series_length),
                                   ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
        }
      if (c_rgb != nullptr)
        {
          cleanup_and_set_error_if((c_rgb_length < y_length * 3) && (c_rgb_length < inner_series_length * 3),
                                   ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
          for (i = 0; i < y_length * 3; i++)
            {
              cleanup_and_set_error_if((c_rgb[i] > 1 || c_rgb[i] < 0), ERROR_PLOT_OUT_OF_RANGE);
            }
        }
      if (ylabels != nullptr)
        {
          y_lightness = static_cast<double *>(malloc(sizeof(double) * y_lightness_to_get));
          use_y_notations_from_inner_series = 0;
        }

      global_render->setFillIntStyle(subGroup, 1);
      global_render->setFillColorInd(subGroup, bar_color);

      if (bar_color_rgb[0] != -1)
        {
          global_render->setColorRep(subGroup, color_save_spot, bar_color_rgb[0], bar_color_rgb[1], bar_color_rgb[2]);
          bar_color = color_save_spot;
          global_render->setFillColorInd(subGroup, bar_color);
        }
      /* Draw Bar */
      for (i = 0; i < y_length; i++)
        {
          y1 = y_min;
          y2 = y[i];

          if (strcmp(style, "default") == 0)
            {
              x1 = (i * bar_shift) + 1 - 0.5 * bar_width;
              x2 = (i * bar_shift) + 1 + 0.5 * bar_width;
            }
          else if (strcmp(style, "stacked") == 0)
            {
              x1 = series_index + 1 - 0.5 * bar_width;
              x2 = series_index + 1 + 0.5 * bar_width;
              if (y[i] > 0)
                {
                  y1 = ((i == 0) ? y_min : 0) + pos_vertical_change;
                  pos_vertical_change += y[i] - ((i > 0) ? y_min : 0);
                  y2 = pos_vertical_change;
                }
              else
                {
                  y1 = ((i == 0) ? y_min : 0) + neg_vertical_change;
                  neg_vertical_change += y[i] - ((i > 0) ? y_min : 0);
                  y2 = neg_vertical_change;
                }
            }
          else if (strcmp(style, "lined") == 0)
            {
              bar_width = wfac / y_length;
              x1 = series_index + 1 - 0.5 * wfac + bar_width * i;
              x2 = series_index + 1 - 0.5 * wfac + bar_width + bar_width * i;
            }


          auto temp = global_render->createFillRect(x1, x2, y1, y2);
          subGroup->append(temp);

          if (strcmp(style, "default") != 0)
            {
              int color_index = i % len_std_colors;
              global_render->setFillColorInd(temp, std_colors[color_index]);
            }
          if (c != nullptr)
            {
              global_render->setFillColorInd(temp, c[i]);
            }
          else if (c_rgb != nullptr)
            {
              global_render->setColorRep(temp, color_save_spot, c_rgb[i * 3], c_rgb[i * 3 + 1], c_rgb[i * 3 + 2]);
              global_render->setFillColorInd(temp, color_save_spot);
            }
          else if (change_bar_color)
            {
              if (*pos_ind_bar_color[i] != -1)
                {
                  global_render->setColorRep(temp, color_save_spot, pos_ind_bar_color[i][0], pos_ind_bar_color[i][1],
                                             pos_ind_bar_color[i][2]);
                  global_render->setFillColorInd(temp, color_save_spot);
                }
            }
          if (y_lightness_to_get > 0 && y_lightness != nullptr)
            {
              if (temp->hasAttribute("fillcolorind"))
                {
                  color = (int)temp->getAttribute("fillcolorind");
                }
              else if (subGroup->hasAttribute("fillcolorind"))
                {
                  color = (int)subGroup->getAttribute("fillcolorind");
                }

              gr_inqcolor(color, (int *)rgb);
              Y = (0.2126729 * rgb[0] / 255 + 0.7151522 * rgb[1] / 255 + 0.0721750 * rgb[2] / 255);
              y_lightness[i] = 116 * pow(Y / 100, 1.0 / 3) - 16;
              --y_lightness_to_get;
            }
        }

      pos_vertical_change = 0;
      neg_vertical_change = 0;
      /* Draw Edge */
      for (i = 0; i < y_length; i++)
        {
          if (strcmp(style, "default") == 0)
            {
              x1 = (i * bar_shift) + 1 - 0.5 * bar_width;
              x2 = (i * bar_shift) + 1 + 0.5 * bar_width;
              y1 = y_min;
              y2 = y[i];
            }
          if (strcmp(style, "stacked") == 0)
            {
              x1 = series_index + 1 - 0.5 * bar_width;
              x2 = series_index + 1 + 0.5 * bar_width;
              if (y[i] > 0)
                {
                  y1 = ((i == 0) ? y_min : 0) + pos_vertical_change;
                  pos_vertical_change += y[i] - ((i > 0) ? y_min : 0);
                  y2 = pos_vertical_change;
                }
              else
                {
                  y1 = ((i == 0) ? y_min : 0) + neg_vertical_change;
                  neg_vertical_change += y[i] - ((i > 0) ? y_min : 0);
                  y2 = neg_vertical_change;
                }
            }
          if (strcmp(style, "lined") == 0)
            {
              bar_width = wfac / y_length;
              x1 = series_index + 1 - 0.5 * wfac + bar_width * i;
              x2 = series_index + 1 - 0.5 * wfac + bar_width + bar_width * i;
              y1 = y_min;
              y2 = y[i];
            }

          std::shared_ptr<GRM::Element> temp;

          if (is_vertical)
            {
              temp = global_render->createDrawRect(y1, y2, x1, x2);
              subGroup->append(temp);
            }
          else
            {
              temp = global_render->createDrawRect(x1, x2, y1, y2);
              subGroup->append(temp);
            }

          global_render->setLineWidth(temp, edge_width);
          if (change_edge_width)
            {
              if (pos_ind_edge_width[i] != -1)
                {
                  double width = pos_ind_edge_width[i];
                  global_render->setLineWidth(temp, width);
                }
            }
          if (edge_color_rgb[0] != -1)
            {
              global_render->setColorRep(temp, color_save_spot, edge_color_rgb[0], edge_color_rgb[1],
                                         edge_color_rgb[2]);
              edge_color = color_save_spot;
            }
          global_render->setLineColorInd(temp, edge_color);
          if (change_edge_color)
            {
              if (*pos_ind_edge_color[i] != -1)
                {
                  global_render->setColorRep(temp, color_save_spot, pos_ind_edge_color[i][0], pos_ind_edge_color[i][1],
                                             pos_ind_edge_color[i][2]);
                  global_render->setLineColorInd(temp, color_save_spot);
                }
            }
        }

      pos_vertical_change = 0;
      neg_vertical_change = 0;
      double width, height, available_width, available_height, x_text, y_text;
      double tbx[4], tby[4];

      /* Draw ylabels */
      if (ylabels != nullptr)
        {
          for (i = 0; i < y_length; i++)
            {
              if (strcmp(style, "default") == 0)
                {
                  x1 = (i * bar_shift) + 1 - 0.5 * bar_width;
                  x2 = (i * bar_shift) + 1 + 0.5 * bar_width;
                  y1 = y_min;
                  y2 = y[i];
                }
              if (strcmp(style, "stacked") == 0)
                {
                  x1 = series_index + 1 - 0.5 * bar_width;
                  x2 = series_index + 1 + 0.5 * bar_width;
                  if (y[i] > 0)
                    {
                      y1 = ((i == 0) ? y_min : 0) + pos_vertical_change;
                      pos_vertical_change += y[i] - ((i > 0) ? y_min : 0);
                      y2 = pos_vertical_change;
                    }
                  else
                    {
                      y1 = ((i == 0) ? y_min : 0) + neg_vertical_change;
                      neg_vertical_change += y[i] - ((i > 0) ? y_min : 0);
                      y2 = neg_vertical_change;
                    }
                }
              if (strcmp(style, "lined") == 0)
                {
                  bar_width = wfac / y_length;
                  x1 = series_index + 1 - 0.5 * wfac + bar_width * i;
                  x2 = series_index + 1 - 0.5 * wfac + bar_width + bar_width * i;
                  y1 = y_min;
                  y2 = y[i];
                }

              if (ylabels_left > 0)
                {
                  available_width = x2 - x1;
                  available_height = y2 - y1;
                  x_text = (x1 + x2) / 2;
                  y_text = (y1 + y2) / 2;

                  std::shared_ptr<GRM::Element> temp =
                      global_render->createText(x_text, y_text, ylabels[i], CoordinateSpace::WC);
                  global_render->setTextAlign(temp, 2, 3);
                  global_render->setTextWidthAndHeight(temp, available_width, available_height);
                  if (y_lightness[i] < 0.4)
                    {
                      global_render->setTextColorInd(temp, 0);
                    }
                  else
                    {
                      global_render->setTextColorInd(temp, 1);
                    }
                  subGroup->append(temp);
                  --ylabels_left;
                }
            }
        }

      /* Draw inner_series */
      for (inner_series_index = 0; inner_series_index < inner_series_length; inner_series_index++)
        {
          /* Draw bars from inner_series */
          int *inner_c = nullptr;
          unsigned int inner_c_length;
          double *inner_c_rgb = nullptr;
          unsigned int inner_c_rgb_length;
          auto inner_group = global_render->createGroup("bar_inner_series");
          subGroup->append(inner_group);

          global_render->setFillColorInd(inner_group, std_colors[inner_series_index % len_std_colors]);
          grm_args_first_value(inner_series[inner_series_index], "y", "D", &y, &y_length);
          bar_width = wfac / fixed_y_length;
          if (c != nullptr)
            {
              global_render->setFillColorInd(inner_group, c[inner_series_index]);
            }
          else if (c_rgb != nullptr)
            {
              global_render->setColorRep(inner_group, color_save_spot, c_rgb[inner_series_index * 3],
                                         c_rgb[inner_series_index * 3 + 1], c_rgb[inner_series_index * 3 + 2]);
              global_render->setFillColorInd(inner_group, color_save_spot);
            }
          else if (change_bar_color)
            {
              if (*pos_ind_bar_color[inner_series_index] != -1)
                {
                  global_render->setColorRep(inner_group, color_save_spot, pos_ind_bar_color[inner_series_index][0],
                                             pos_ind_bar_color[inner_series_index][1],
                                             pos_ind_bar_color[inner_series_index][2]);
                  global_render->setFillColorInd(inner_group, color_save_spot);
                }
            }
          if (grm_args_first_value(inner_series[inner_series_index], "c", "I", &inner_c, &inner_c_length))
            {
              cleanup_and_set_error_if(inner_c_length != y_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
            }
          if (grm_args_first_value(inner_series[inner_series_index], "c", "D", &inner_c_rgb, &inner_c_rgb_length))
            {
              cleanup_and_set_error_if((inner_c_rgb_length < y_length * 3), ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
              for (i = 0; i < y_length * 3; i++)
                {
                  cleanup_and_set_error_if((inner_c_rgb[i] > 1 || inner_c_rgb[i] < 0), ERROR_PLOT_OUT_OF_RANGE);
                }
            }
          if (ylabels == nullptr)
            {
              grm_args_first_value(inner_series[inner_series_index], "ylabels", "nS", &ylabels, &ylabels_length);
              ylabels_left = ylabels_length;
              y_lightness_to_get = ylabels_length;
              y_lightness = static_cast<double *>(malloc(sizeof(double) * y_lightness_to_get));
            }
          for (i = 0; i < y_length; i++)
            {
              x1 = series_index + 1 - 0.5 * wfac + bar_width * inner_series_index;
              x2 = series_index + 1 - 0.5 * wfac + bar_width + bar_width * inner_series_index;
              if (y[i] > 0)
                {
                  y1 = ((i == 0) ? y_min : 0) + pos_vertical_change;
                  pos_vertical_change += y[i] - ((i > 0) ? y_min : 0);
                  y2 = pos_vertical_change;
                }
              else
                {
                  y1 = ((i == 0) ? y_min : 0) + neg_vertical_change;
                  neg_vertical_change += y[i] - ((i > 0) ? y_min : 0);
                  y2 = neg_vertical_change;
                }
              auto temp = global_render->createFillRect(x1, x2, y1, y2);
              inner_group->append(temp);

              if (inner_c != nullptr)
                {
                  global_render->setFillColorInd(temp, inner_c[i]);
                }
              if (inner_c_rgb != nullptr)
                {
                  global_render->setColorRep(temp, color_save_spot, inner_c_rgb[i * 3], inner_c_rgb[i * 3 + 1],
                                             inner_c_rgb[i * 3 + 2]);
                  global_render->setFillColorInd(temp, color_save_spot);
                }

              if (y_lightness_to_get > 0 && y_lightness != nullptr)
                {
                  if (temp->hasAttribute("fillcolorind"))
                    {
                      color = (int)temp->getAttribute("fillcolorind");
                    }
                  else if (subGroup->hasAttribute("fillcolorind"))
                    {
                      color = (int)inner_group->getAttribute("fillcolorind");
                    }
                  gr_inqcolor(color, (int *)rgb);
                  Y = (0.2126729 * rgb[0] / 255 + 0.7151522 * rgb[1] / 255 + 0.0721750 * rgb[2] / 255);
                  y_lightness[ylabels_length - y_lightness_to_get] = 116 * pow(Y / 100, 1.0 / 3) - 16;
                  --y_lightness_to_get;
                }
            }
          pos_vertical_change = 0;
          neg_vertical_change = 0;


          auto inner_edges = global_render->createGroup("bar_inner_edges");
          inner_group->append(inner_edges);

          /* Draw edges from inner_series */
          global_render->setLineWidth(inner_edges, edge_width);
          if (change_edge_width)
            {
              if (pos_ind_edge_width[inner_series_index] != -1)
                {
                  double width = pos_ind_edge_width[inner_series_index];
                  global_render->setLineWidth(inner_edges, width);
                }
            }
          if (edge_color_rgb[0] != -1)
            {
              global_render->setColorRep(inner_edges, color_save_spot, edge_color_rgb[0], edge_color_rgb[1],
                                         edge_color_rgb[2]);
              edge_color = color_save_spot;
            }
          global_render->setLineColorInd(inner_edges, edge_color);
          if (change_edge_color)
            {
              if (*pos_ind_edge_color[inner_series_index] != -1)
                {
                  global_render->setColorRep(inner_edges, color_save_spot, pos_ind_edge_color[inner_series_index][0],
                                             pos_ind_edge_color[inner_series_index][1],
                                             pos_ind_edge_color[inner_series_index][2]);
                  global_render->setLineColorInd(inner_edges, color_save_spot);
                }
            }

          for (i = 0; i < y_length; i++)
            {
              x1 = series_index + 1 - 0.5 * wfac + bar_width * inner_series_index;
              x2 = series_index + 1 - 0.5 * wfac + bar_width + bar_width * inner_series_index;
              if (y[i] > 0)
                {
                  y1 = ((i == 0) ? y_min : 0) + pos_vertical_change;
                  pos_vertical_change += y[i] - ((i > 0) ? y_min : 0);
                  y2 = pos_vertical_change;
                }
              else
                {
                  y1 = ((i == 0) ? y_min : 0) + neg_vertical_change;
                  neg_vertical_change += y[i] - ((i > 0) ? y_min : 0);
                  y2 = neg_vertical_change;
                }
              x1 += x_min;
              x2 += x_min;

              std::shared_ptr<GRM::Element> temp;
              if (is_vertical)
                {
                  temp = global_render->createDrawRect(y1, y2, x1, x2);
                  inner_edges->append(temp);
                }
              else
                {
                  temp = global_render->createDrawRect(x1, x2, y1, y2);
                  inner_edges->append(temp);
                }
              global_render->setFillColorInd(temp, std_colors[inner_series_index % len_std_colors]);
            }
          pos_vertical_change = 0;
          neg_vertical_change = 0;

          /* Draw ynotations from inner_series */
          if (ylabels != nullptr)
            {
              for (i = 0; i < y_length; i++)
                {
                  x1 = series_index + 1 - 0.5 * wfac + bar_width * inner_series_index;
                  x2 = series_index + 1 - 0.5 * wfac + bar_width + bar_width * inner_series_index;
                  if (y[i] > 0)
                    {
                      y1 = ((i == 0) ? y_min : 0) + pos_vertical_change;
                      pos_vertical_change += y[i] - ((i > 0) ? y_min : 0);
                      y2 = pos_vertical_change;
                    }
                  else
                    {
                      y1 = ((i == 0) ? y_min : 0) + neg_vertical_change;
                      neg_vertical_change += y[i] - ((i > 0) ? y_min : 0);
                      y2 = neg_vertical_change;
                    }

                  if (ylabels_left > 0)
                    {
                      available_width = x2 - x1;
                      available_height = y2 - y1;
                      x_text = (x1 + x2) / 2;
                      y_text = (y1 + y2) / 2;

                      auto label_elem = global_render->createText(
                          x_text, y_text, ylabels[ylabels_length - ylabels_left], CoordinateSpace::WC);
                      global_render->setTextAlign(label_elem, 2, 3);
                      global_render->setTextWidthAndHeight(label_elem, available_width, available_height);
                      inner_group->append(label_elem);

                      if (y_lightness[ylabels_length - ylabels_left] < 0.4)
                        {
                          global_render->setTextColorInd(label_elem, 0);
                        }
                      else
                        {
                          global_render->setTextColorInd(label_elem, 1);
                        }
                      --ylabels_left;
                    }
                }
            }
          y_length = 0;
          pos_vertical_change = 0;
          neg_vertical_change = 0;

          if (use_y_notations_from_inner_series)
            {
              free(y_lightness);
              y_lightness = nullptr;
              ylabels = nullptr;
            }
        }

      if (y_lightness != nullptr)
        {
          free(y_lightness);
          y_lightness = nullptr;
        }

      if (grm_args_contains(*current_series, "error"))
        {
          grm_args_t **curr_series;
          grm_args_values(subplot_args, "series", "A", &curr_series);
          if (*curr_series != nullptr)
            {
              double *bar_centers;
              double *x, *err_x;
              unsigned int x_length;
              grm_args_first_value(*curr_series, "x", "D", &x, &x_length);
              bar_centers = static_cast<double *>(malloc(x_length * sizeof(double)));
              cleanup_and_set_error_if(bar_centers == nullptr, ERROR_MALLOC);
              if (strcmp(style, "default") == 0)
                {
                  linspace(x_min + 1, x_length, x_length, bar_centers);
                }
              else if (strcmp(style, "lined") == 0)
                {
                  for (i = 0; i < y_length; i++)
                    {
                      bar_width = wfac / y_length;
                      x1 = x_min + series_index + 1 - 0.5 * wfac + bar_width * i;
                      x2 = x_min + series_index + 1 - 0.5 * wfac + bar_width + bar_width * i;
                      bar_centers[i] = (x1 + x2) / 2.0;
                    }
                  x_length = y_length;
                }
              else
                {
                  for (i = 0; i < y_length; i++)
                    {
                      x1 = x_min + series_index + 1 - 0.5 * bar_width;
                      x2 = x_min + series_index + 1 + 0.5 * bar_width;
                      bar_centers[i] = (x1 + x2) / 2.0;
                    }
                  x_length = y_length;
                }
              grm_args_push(*current_series, "orientation", "s", orientation);
              error = plot_draw_errorbars(*current_series, bar_centers, x_length, y, "barplot");
              cleanup_if_error;
              free(bar_centers);
              bar_centers = nullptr;
            }
        }
      series_index++;
      ++current_series;
    }
  gr_restorestate();


cleanup:
  if (pos_ind_bar_color != nullptr)
    {
      free(pos_ind_bar_color);
    }
  if (pos_ind_edge_color != nullptr)
    {
      free(pos_ind_edge_color);
    }
  if (pos_ind_edge_width != nullptr)
    {
      free(pos_ind_edge_width);
    }
  if (y_lightness != nullptr)
    {
      free(y_lightness);
    }
  return error;
}

err_t plot_contour(grm_args_t *subplot_args)
{
  int num_levels;
  double *h;
  grm_args_t **current_series;
  int i;
  err_t error = ERROR_NONE;

  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();
  group->setAttribute("name", "contour");

  grm_args_values(subplot_args, "levels", "i", &num_levels);
  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double *x, *y, *z;
      unsigned int x_length, y_length, z_length;
      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      grm_args_first_value(*current_series, "y", "D", &y, &y_length);
      grm_args_first_value(*current_series, "z", "D", &z, &z_length);
      auto subGroup = global_render->createSeries("contour");
      group->append(subGroup);

      int id = static_cast<int>(global_root->getAttribute("id"));
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

      subGroup->setAttribute("levels", num_levels);

      global_root->setAttribute("id", ++id);
      ++current_series;
    }
  error = plot_draw_colorbar(subplot_args, 0.0, num_levels);

  return error;
}

err_t plot_contourf(grm_args_t *subplot_args)
{
  int num_levels, scale;
  double *h;
  grm_args_t **current_series;
  int i;
  err_t error = ERROR_NONE;

  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();
  group->setAttribute("name", "contourf");

  grm_args_values(subplot_args, "levels", "i", &num_levels);
  grm_args_values(subplot_args, "scale", "i", &scale);
  global_render->setScale(group, scale);
  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double *x, *y, *z;
      unsigned int x_length, y_length, z_length;
      auto subGroup = global_render->createSeries("contourf");
      group->append(subGroup);
      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      grm_args_first_value(*current_series, "y", "D", &y, &y_length);
      grm_args_first_value(*current_series, "z", "D", &z, &z_length);

      int id = static_cast<int>(global_root->getAttribute("id"));
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

      subGroup->setAttribute("levels", num_levels);

      global_root->setAttribute("id", ++id);
      ++current_series;
    }
  error = plot_draw_colorbar(subplot_args, 0.0, num_levels);

  return error;
}

err_t plot_hexbin(grm_args_t *subplot_args)
{
  grm_args_t **current_series;

  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();
  group->setAttribute("name", "hexbin");

  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double *x, *y;
      unsigned int x_length, y_length;
      int cntmax, nbins;
      return_error_if(!grm_args_first_value(*current_series, "x", "D", &x, &x_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(!grm_args_first_value(*current_series, "y", "D", &y, &y_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(x_length != y_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
      grm_args_values(*current_series, "nbins", "i", &nbins);

      int id_int = static_cast<int>(global_root->getAttribute("id"));
      global_root->setAttribute("id", ++id_int);
      std::string id = std::to_string(id_int);

      auto x_vec = std::vector<double>(x, x + x_length);
      auto y_vec = std::vector<double>(y, y + y_length);
      auto hexbin = global_render->createHexbin("x" + id, x_vec, "y" + id, y_vec, nbins);
      group->append(hexbin);

      currentDomElement = hexbin; /* so that the colorbar will be a child of the hexbin_series */
      plot_draw_colorbar(subplot_args, 0.0, 256);

      ++current_series;
    }

  return ERROR_NONE;
}

err_t plot_polar_heatmap(grm_args_t *subplot_args)
{
  const char *kind = nullptr;
  grm_args_t **current_series;
  int zlog = 0;
  unsigned int i, cols, rows, z_length;
  double *x = nullptr, *y = nullptr, *z, x_min, x_max, y_min, y_max, z_min, z_max, c_min, c_max;
  err_t error = ERROR_NONE;

  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();
  group->setAttribute("name", "polar_heatmap");

  grm_args_values(subplot_args, "series", "A", &current_series);
  grm_args_values(subplot_args, "kind", "s", &kind);
  grm_args_values(subplot_args, "zlog", "i", &zlog);
  while (*current_series != nullptr)
    {
      int is_uniform_heatmap;
      auto subGroup = global_render->createSeries("polar_heatmap");
      group->append(subGroup);

      grm_args_first_value(*current_series, "x", "D", &x, &cols);
      grm_args_first_value(*current_series, "y", "D", &y, &rows);
      is_uniform_heatmap = is_equidistant_array(cols, x) && is_equidistant_array(rows, y);
      if (strcmp(kind, "nonuniformpolar_heatmap") == 0) is_uniform_heatmap = 0;
      return_error_if(!is_uniform_heatmap && (x == nullptr || y == nullptr), ERROR_PLOT_MISSING_DATA);
      return_error_if(!grm_args_first_value(*current_series, "z", "D", &z, &z_length), ERROR_PLOT_MISSING_DATA);

      int id = static_cast<int>(global_root->getAttribute("id"));
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

      group->setAttribute("zlog", zlog);
      group->setAttribute("kind", kind);

      if (x == nullptr && y == nullptr)
        {
          /* If neither `x` nor `y` are given, we need more information about the shape of `z` */
          return_error_if(!grm_args_values(*current_series, "z_dims", "ii", &rows, &cols),
                          ERROR_PLOT_MISSING_DIMENSIONS);
          subGroup->setAttribute("zdims_min", (int)rows);
          subGroup->setAttribute("zdims_max", (int)cols);
        }
      if (x == nullptr)
        {
          grm_args_values(*current_series, "xrange", "dd", &x_min, &x_max);
          group->setAttribute("xrange_min", x_min);
          group->setAttribute("xrange_max", x_max);
        }
      if (y == nullptr)
        {
          grm_args_values(*current_series, "yrange", "dd", &y_min, &y_max);
          group->setAttribute("yrange_min", y_min);
          group->setAttribute("yrange_max", y_max);
        }
      grm_args_values(*current_series, "zrange", "dd", &z_min, &z_max);
      group->setAttribute("zrange_min", z_min);
      group->setAttribute("zrange_max", z_max);
      if (grm_args_values(*current_series, "crange", "dd", &c_min, &c_max))
        {
          subGroup->setAttribute("crange_min", c_min);
          subGroup->setAttribute("crange_max", c_max);
        }

      global_root->setAttribute("id", ++id);
      ++current_series;
    }

  plot_draw_colorbar(subplot_args, 0.025, 256);
  return error;
}

err_t plot_heatmap(grm_args_t *subplot_args)
{
  const char *kind = nullptr;
  grm_args_t **current_series;
  int zlog = 0;
  unsigned int i, cols, rows, z_length;
  double *x = nullptr, *y = nullptr, *z, x_min, x_max, y_min, y_max, z_min, z_max, c_min, c_max;
  err_t error = ERROR_NONE;
  std::shared_ptr<GRM::Element> plot_parent;

  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();
  if (!group->hasAttribute("name")) group->setAttribute("name", "heatmap");

  if (group->localName() == "plot")
    {
      plot_parent = group;
    }
  else
    {
      plot_parent = group->parentElement();
    }

  grm_args_values(subplot_args, "series", "A", &current_series);
  grm_args_values(subplot_args, "kind", "s", &kind);
  grm_args_values(subplot_args, "zlog", "i", &zlog);
  while (*current_series != nullptr)
    {
      int is_uniform_heatmap;
      x = y = nullptr;
      auto subGroup = global_render->createSeries("heatmap");
      group->append(subGroup);
      grm_args_first_value(*current_series, "x", "D", &x, &cols);
      grm_args_first_value(*current_series, "y", "D", &y, &rows);
      is_uniform_heatmap =
          (x == nullptr || is_equidistant_array(cols, x)) && (y == nullptr || is_equidistant_array(rows, y));
      return_error_if(!is_uniform_heatmap && (x == nullptr || y == nullptr), ERROR_PLOT_MISSING_DATA);
      return_error_if(!grm_args_first_value(*current_series, "z", "D", &z, &z_length), ERROR_PLOT_MISSING_DATA);

      int id = static_cast<int>(global_root->getAttribute("id"));
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

      plot_parent->setAttribute("zlog", zlog);

      if (x == nullptr && y == nullptr)
        {
          /* If neither `x` nor `y` are given, we need more information about the shape of `z` */
          return_error_if(!grm_args_values(*current_series, "z_dims", "ii", &rows, &cols),
                          ERROR_PLOT_MISSING_DIMENSIONS);
          subGroup->setAttribute("zdims_min", (int)rows);
          subGroup->setAttribute("zdims_max", (int)cols);
        }
      if (x == nullptr)
        {
          grm_args_values(*current_series, "xrange", "dd", &x_min, &x_max);
          plot_parent->setAttribute("xrange_min", x_min);
          plot_parent->setAttribute("xrange_max", x_max);
        }
      if (y == nullptr)
        {
          grm_args_values(*current_series, "yrange", "dd", &y_min, &y_max);
          plot_parent->setAttribute("yrange_min", y_min);
          plot_parent->setAttribute("yrange_max", y_max);
        }
      grm_args_values(*current_series, "zrange", "dd", &z_min, &z_max);
      plot_parent->setAttribute("zrange_min", z_min);
      plot_parent->setAttribute("zrange_max", z_max);
      if (grm_args_values(*current_series, "crange", "dd", &c_min, &c_max))
        {
          subGroup->setAttribute("crange_min", c_min);
          subGroup->setAttribute("crange_max", c_max);
        }
      global_root->setAttribute("id", ++id);

      ++current_series;
    }

  if (strcmp(kind, "marginalheatmap") != 0)
    {
      gr_setlinecolorind(1);
      plot_draw_colorbar(subplot_args, 0.0, 256);
    }

  return error;
}

err_t plot_marginalheatmap(grm_args_t *subplot_args)
{
  double c_min, c_max;
  int flip, options, xind, yind;
  unsigned int i, j, k;
  grm_args_t **current_series;
  char *algorithm, *marginalheatmap_kind;
  double *bins = nullptr;
  unsigned int num_bins_x = 0, num_bins_y = 0, n = 0;
  double *xi, *yi, *plot;
  err_t error = ERROR_NONE;
  std::string mkind;

  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();
  group->setAttribute("name", "marginalheatmap");
  auto subGroup = global_render->createSeries("marginalheatmap_series");
  group->append(subGroup);
  currentDomElement = subGroup;

  plot_heatmap(subplot_args);

  grm_args_values(subplot_args, "marginalheatmap_kind", "s", &marginalheatmap_kind);
  grm_args_values(subplot_args, "xind", "i", &xind);
  grm_args_values(subplot_args, "yind", "i", &yind);

  subGroup->setAttribute("marginalheatmap_kind", marginalheatmap_kind);
  subGroup->setAttribute("xind", xind);
  subGroup->setAttribute("yind", yind);

  for (k = 0; k < 2; k++)
    {
      double x_min, x_max, y_min, y_max, value, bin_max = 0;

      grm_args_values(subplot_args, "series", "A", &current_series);
      grm_args_values(*current_series, "algorithm", "s", &algorithm);
      grm_args_values(*current_series, "xrange", "dd", &x_min, &x_max);
      grm_args_values(*current_series, "yrange", "dd", &y_min, &y_max);
      if (!grm_args_values(subplot_args, "_clim", "dd", &c_min, &c_max))
        {
          cleanup_and_set_error_if(!grm_args_values(subplot_args, "_zlim", "dd", &c_min, &c_max),
                                   ERROR_PLOT_MISSING_DATA);
          group->setAttribute("lim_zmin", c_min);
          group->setAttribute("lim_zmax", c_max);
        }
      else
        {
          group->setAttribute("lim_cmin", c_min);
          group->setAttribute("lim_cmax", c_max);
        }

      grm_args_first_value(*current_series, "x", "D", &xi, &num_bins_x);
      grm_args_first_value(*current_series, "y", "D", &yi, &num_bins_y);
      grm_args_first_value(*current_series, "z", "D", &plot, &n);

      if (strcmp(marginalheatmap_kind, "all") == 0)
        {
          unsigned int x_len = num_bins_x, y_len = num_bins_y;

          bins = static_cast<double *>(malloc(((k == 0) ? num_bins_y : num_bins_x) * sizeof(double)));
          cleanup_and_set_error_if(bins == nullptr, ERROR_MALLOC);
          grm_args_push(subplot_args, "kind", "s", "hist");

          for (i = 0; i < ((k == 0) ? num_bins_y : num_bins_x); i++)
            {
              bins[i] = 0;
            }
          for (i = 0; i < y_len; i++)
            {
              for (j = 0; j < x_len; j++)
                {
                  value = (grm_isnan(plot[i * num_bins_x + j])) ? 0 : plot[i * num_bins_x + j];
                  if (strcmp(algorithm, "sum") == 0)
                    {
                      bins[(k == 0) ? i : j] += value;
                    }
                  else if (strcmp(algorithm, "max") == 0)
                    {
                      bins[(k == 0) ? i : j] = grm_max(bins[(k == 0) ? i : j], value);
                    }
                }
              if (k == 0)
                {
                  bin_max = grm_max(bin_max, bins[i]);
                }
            }
          if (k == 1)
            {
              for (i = 0; i < x_len; i++)
                {
                  bin_max = grm_max(bin_max, bins[i]);
                }
            }
          for (i = 0; i < ((k == 0) ? y_len : x_len); i++)
            {
              bins[i] = (bin_max == 0) ? 0 : bins[i] / bin_max * (c_max / 15);
            }

          grm_args_push(*current_series, "bins", "nD", ((k == 0) ? num_bins_y : num_bins_x), bins);

          free(bins);
          bins = nullptr;
        }

      if (grm_args_values(subplot_args, "xflip", "i", &flip) && flip)
        {
          subGroup->setAttribute("gr_option_flip_y", 1);
          subGroup->setAttribute("gr_option_flip_x", 0);
        }
      else if (grm_args_values(subplot_args, "yflip", "i", &flip) && flip)
        {
          subGroup->setAttribute("gr_option_flip_y", 0);
          subGroup->setAttribute("gr_option_flip_x", 0);
        }
      else
        {
          subGroup->setAttribute("gr_option_flip_x", 0);
        }

      if (k == 0)
        {
          grm_args_push(subplot_args, "orientation", "s", "vertical");
        }
      else
        {
          grm_args_push(subplot_args, "orientation", "s", "horizontal");
        }

      if (strcmp(marginalheatmap_kind, "all") == 0)
        {
          plot_hist(subplot_args);
        }
      else if (strcmp(marginalheatmap_kind, "line") == 0 && xind != -1 && yind != -1)
        {
          plot_stairs(subplot_args);
        }
    }
  grm_args_push(subplot_args, "kind", "s", "marginalheatmap");

cleanup:
  free(bins);
  currentDomElement = nullptr;

  return error;
}

err_t plot_wireframe(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  err_t error = ERROR_NONE;

  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();
  group->setAttribute("name", "wireframe");

  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double *x, *y, *z;
      unsigned int x_length, y_length, z_length;

      auto subGroup = global_render->createSeries("wireframe");
      group->append(subGroup);

      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      grm_args_first_value(*current_series, "y", "D", &y, &y_length);
      grm_args_first_value(*current_series, "z", "D", &z, &z_length);

      int id = static_cast<int>(global_root->getAttribute("id"));
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

      global_root->setAttribute("id", ++id);
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

  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();
  group->setAttribute("name", "surface");

  grm_args_values(subplot_args, "series", "A", &current_series);
  grm_args_values(subplot_args, "accelerate", "i", &accelerate);

  while (*current_series != nullptr)
    {
      double *x = nullptr, *y = nullptr, *z = nullptr;
      unsigned int x_length, y_length, z_length;

      auto subGroup = global_render->createSeries("surface");
      group->append(subGroup);
      subGroup->setAttribute("accelerate", accelerate);

      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      grm_args_first_value(*current_series, "y", "D", &y, &y_length);
      grm_args_first_value(*current_series, "z", "D", &z, &z_length);
      return_error_if(!grm_args_first_value(*current_series, "z", "D", &z, &z_length), ERROR_PLOT_MISSING_DATA);

      if (grm_args_values(*current_series, "z_dims", "ii", &y_length, &x_length))
        {
          subGroup->setAttribute("zdims_min", (int)x_length);
          subGroup->setAttribute("zdims_max", (int)y_length);
        }
      if (grm_args_values(*current_series, "xrange", "dd", &xmin, &xmax))
        {
          group->setAttribute("xrange_min", xmin);
          group->setAttribute("xrange_max", xmax);
        }
      if (grm_args_values(*current_series, "yrange", "dd", &ymin, &ymax))
        {
          group->setAttribute("yrange_min", ymin);
          group->setAttribute("yrange_max", ymax);
        }

      int id_int = static_cast<int>(global_root->getAttribute("id"));
      global_root->setAttribute("id", ++id_int);
      std::string str = std::to_string(id_int);
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

      ++current_series;
    }
  plot_draw_axes(subplot_args, 2);
  plot_draw_colorbar(subplot_args, 0.05, 256);

  return error;
}

err_t plot_plot3(grm_args_t *subplot_args)
{
  grm_args_t **current_series;

  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();
  group->setAttribute("name", "plot3");
  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double *x, *y, *z;
      unsigned int x_length, y_length, z_length;
      auto subGroup = global_render->createSeries("plot3");
      group->append(subGroup);
      return_error_if(!grm_args_first_value(*current_series, "x", "D", &x, &x_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(!grm_args_first_value(*current_series, "y", "D", &y, &y_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(!grm_args_first_value(*current_series, "z", "D", &z, &z_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(x_length != y_length || x_length != z_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);

      int id_int = static_cast<int>(global_root->getAttribute("id"));
      std::string str = std::to_string(id_int);
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

      global_root->setAttribute("id", ++id_int);
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

  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();
  group->setAttribute("name", "scatter3");
  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      auto subGroup = global_render->createSeries("scatter3");
      group->append(subGroup);
      return_error_if(!grm_args_first_value(*current_series, "x", "D", &x, &x_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(!grm_args_first_value(*current_series, "y", "D", &y, &y_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(!grm_args_first_value(*current_series, "z", "D", &z, &z_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(x_length != y_length || x_length != z_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);

      int id = static_cast<int>(global_root->getAttribute("id"));
      std::string str = std::to_string(id);
      global_root->setAttribute("id", ++id);
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

      if (grm_args_first_value(*current_series, "c", "D", &c, &c_length))
        {
          std::vector<double> c_vec(c, c + c_length);
          (*context)["c" + str] = c_vec;
          subGroup->setAttribute("c", "c" + str);

          grm_args_values(subplot_args, "_clim", "dd", &c_min, &c_max);
          subGroup->setAttribute("c_min", c_min);
          subGroup->setAttribute("c_max", c_max);
        }
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
  int *img_data;
  unsigned int *shape;
  int grplot = 0;

  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();
  group->setAttribute("name", "imshow");

  grm_args_values(subplot_args, "series", "A", &current_series);
  grm_args_values(subplot_args, "grplot", "i", &grplot);
  return_error_if(!grm_args_values(subplot_args, "_clim", "dd", &c_min, &c_max), ERROR_PLOT_MISSING_DATA);
  while (*current_series != nullptr)
    {
      auto subGroup = global_render->createSeries("imshow");
      group->append(subGroup);
      group->setAttribute("grplot", grplot);
      subGroup->setAttribute("c_min", c_min);
      subGroup->setAttribute("c_max", c_max);

      return_error_if(!grm_args_first_value(*current_series, "c", "D", &c_data, &c_data_length),
                      ERROR_PLOT_MISSING_DATA);
      return_error_if(!grm_args_first_value(*current_series, "c_dims", "I", &shape, &i), ERROR_PLOT_MISSING_DATA);
      return_error_if(i != 2, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
      return_error_if(shape[0] * shape[1] != c_data_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);

      int id = static_cast<int>(global_root->getAttribute("id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> c_data_vec(c_data, c_data + c_data_length);
      std::vector<double> shape_vec(shape, shape + i);

      (*context)["c" + str] = c_data_vec;
      subGroup->setAttribute("c", "c" + str);
      (*context)["cdims" + str] = shape_vec;
      subGroup->setAttribute("cdims", "cdims" + str);

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
  int strides[3];
  double c_min, c_max, isovalue;
  float foreground_colors[3];
  float *conv_data = nullptr;

  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();
  group->setAttribute("name", "isosurface");

  grm_args_values(subplot_args, "series", "A", &current_series);

  while (*current_series != nullptr)
    {
      auto subGroup = global_render->createSeries("isosurface");
      group->append(subGroup);
      return_error_if(!grm_args_first_value(*current_series, "c", "D", &orig_data, &data_length),
                      ERROR_PLOT_MISSING_DATA);
      return_error_if(!grm_args_first_value(*current_series, "c_dims", "I", &shape, &dims), ERROR_PLOT_MISSING_DATA);
      return_error_if(dims != 3, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
      return_error_if(shape[0] * shape[1] * shape[2] != data_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
      return_error_if(data_length <= 0, ERROR_PLOT_MISSING_DATA);

      int id = static_cast<int>(global_root->getAttribute("id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> c_data_vec(orig_data, orig_data + data_length);
      std::vector<int> shape_vec(shape, shape + dims);

      (*context)["c" + str] = c_data_vec;
      subGroup->setAttribute("c", "c" + str);
      (*context)["c_dims" + str] = shape_vec;
      subGroup->setAttribute("c_dims", "c_dims" + str);

      if (grm_args_values(*current_series, "isovalue", "d", &isovalue)) subGroup->setAttribute("isovalue", isovalue);
      /*
       * We need to convert the double values to floats, as GR3 expects floats, but an argument can only contain
       * doubles.
       */
      if (grm_args_first_value(*current_series, "foreground_color", "D", &temp_colors, &i))
        {
          return_error_if(i != 3, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
          std::vector<double> foreground_vec(temp_colors, temp_colors + i);
          (*context)["foreground_color" + str] = foreground_vec;
          subGroup->setAttribute("foreground_color", "foreground_color" + str);
        }

      global_root->setAttribute("id", ++id);
      ++current_series;
    }

  return ERROR_NONE;
}

err_t plot_volume(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  double dlim[2] = {INFINITY, -INFINITY};
  err_t error;

  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();
  group->setAttribute("name", "volume");

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
      double dmin, dmax;

      return_error_if(!grm_args_first_value(*current_series, "c", "D", &c, &data_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(!grm_args_first_value(*current_series, "c_dims", "I", &shape, &dims), ERROR_PLOT_MISSING_DATA);
      return_error_if(dims != 3, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
      return_error_if(shape[0] * shape[1] * shape[2] != data_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
      return_error_if(data_length <= 0, ERROR_PLOT_MISSING_DATA);

      int id = static_cast<int>(global_root->getAttribute("id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> c_data_vec(c, c + data_length);
      std::vector<int> shape_vec(shape, shape + dims);

      (*context)["c" + str] = c_data_vec;
      subGroup->setAttribute("c", "c" + str);
      (*context)["c_dims" + str] = shape_vec;
      subGroup->setAttribute("c_dims", "c_dims" + str);

      if (!grm_args_values(*current_series, "algorithm", "i", &algorithm))
        {
          if (grm_args_values(*current_series, "algorithm", "s", &algorithm_str))
            {
              if (strcmp(algorithm_str, "emission") == 0)
                {
                  algorithm = GR_VOLUME_EMISSION;
                }
              else if (strcmp(algorithm_str, "absorption") == 0)
                {
                  algorithm = GR_VOLUME_ABSORPTION;
                }
              else if (str_equals_any(algorithm_str, 2, "mip", "maximum"))
                {
                  algorithm = GR_VOLUME_MIP;
                }
              else
                {
                  logger((stderr, "Got unknown volume algorithm \"%s\"\n", algorithm_str));
                  return ERROR_PLOT_UNKNOWN_ALGORITHM;
                }
            }
          else
            {
              logger((stderr, "No volume algorithm given! Aborting the volume routine\n"));
              return ERROR_PLOT_MISSING_ALGORITHM;
            }
        }
      if (algorithm != GR_VOLUME_ABSORPTION && algorithm != GR_VOLUME_EMISSION && algorithm != GR_VOLUME_MIP)
        {
          logger((stderr, "Got unknown volume algorithm \"%d\"\n", algorithm));
          return ERROR_PLOT_UNKNOWN_ALGORITHM;
        }
      subGroup->setAttribute("algorithm", algorithm);

      dmin = dmax = -1.0;
      grm_args_values(*current_series, "dmin", "d", &dmin);
      grm_args_values(*current_series, "dmax", "d", &dmax);
      subGroup->setAttribute("dmin", dmin);
      subGroup->setAttribute("dmax", dmax);

      global_root->setAttribute("id", ++id);
      ++current_series;
    }

  logger((stderr, "dmin, dmax: (%lf, %lf)\n", dlim[0], dlim[1]));
  grm_args_push(subplot_args, "_clim", "dd", dlim[0], dlim[1]);
  group->setAttribute("dlim_min", dlim[0]);
  group->setAttribute("dlim_max", dlim[1]);

  error = plot_draw_axes(subplot_args, 2);
  return_if_error;
  error = plot_draw_colorbar(subplot_args, 0.05, 256);
  return_if_error;

  return ERROR_NONE;
}

err_t plot_polar(grm_args_t *subplot_args)
{
  double r_min, r_max;
  grm_args_t **current_series;

  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();
  group->setAttribute("name", "polar");

  grm_args_values(subplot_args, "_ylim", "dd", &r_min, &r_max);
  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double *rho, *theta;
      unsigned int rho_length, theta_length;
      char *spec;
      auto subGroup = global_render->createSeries("polar");
      group->append(subGroup);

      subGroup->setAttribute("r_min", r_min);
      subGroup->setAttribute("r_max", r_max);

      return_error_if(!grm_args_first_value(*current_series, "x", "D", &theta, &theta_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(!grm_args_first_value(*current_series, "y", "D", &rho, &rho_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(rho_length != theta_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);

      int id = static_cast<int>(global_root->getAttribute("id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> theta_vec(theta, theta + theta_length);
      std::vector<double> rho_vec(rho, rho + rho_length);

      (*context)["x" + str] = theta_vec;
      subGroup->setAttribute("x", "x" + str);
      (*context)["y" + str] = rho_vec;
      subGroup->setAttribute("y", "y" + str);

      grm_args_values(*current_series, "spec", "s", &spec); /* `spec` is always set */
      subGroup->setAttribute("spec", spec);
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
 * \param[in] philim a list containing two double values representing start and end angle (series)
 *            The plot will only use the values in the given range and and it will plot bins between those two values.
 * \param[in] phiflip an integer that represents a boolean (subplot)
 *            0: the angles will be displayed anti clockwise.
 *            else: the angles will be displayed clockwise.
 *            If philim[0] is greater than philim[1], phiflip will be negated.
 *            The default value is 0.
 * \param[in] rlim a list containing two double values between (inclusive) 0.0 and 1.0 (series)
 *            0.0 is the center and 1.0 is the outer edge of the plot.
 *            rlim will limit the bins' start and end height.
 * \param[in] bin_width a double value between (exclusive) 0.0 and 2 * pi setting the bins' width (series)
 *            It is not compatible nbins or bin_edges.
 * \param[in] nbins an int setting the number of bins (series)
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
  unsigned int resample;
  unsigned int num_bins;
  unsigned int length;
  double max;
  double *rlim = nullptr;
  unsigned int dummy;
  int stairs;
  double r_min = 0.0;
  double r_max = 1.0;
  int xcolormap;
  int ycolormap;
  int *colormap = nullptr;
  int draw_edges = 0;
  int phiflip = 0;
  int edge_color = 1;
  int face_color = 989;
  double face_alpha = 0.75;
  grm_args_t **series;
  int *bin_counts = nullptr;
  err_t error = ERROR_NONE;

  std::shared_ptr<GRM::Element> group = global_root->lastChildElement();
  group->setAttribute("kind", "polar_histogram");

  // Call classes -> set attributes and data
  classes_polar_histogram(subplot_args);

  auto context = global_render->getContext();

  int id = static_cast<int>(global_root->getAttribute("id"));
  global_root->setAttribute("id", id + 1);
  std::string str = std::to_string(id);

  group->setAttribute("name", "polar_histogram");
  grm_args_values(subplot_args, "series", "A", &series);

  gr_inqresamplemethod(&resample);
  group->setAttribute("original_resample", static_cast<int>(resample));

  /* edge_color */
  if (grm_args_values(*series, "edge_color", "i", &edge_color) == 0)
    {
      edge_color = 1;
    }
  group->setAttribute("edge_color", edge_color);

  /* face_color */
  if (grm_args_values(*series, "face_color", "i", &face_color) == 0)
    {
      face_color = 989;
    }
  group->setAttribute("face_color", face_color);

  /* face_alpha */
  if (grm_args_values(*series, "face_alpha", "d", &face_alpha) == 0)
    {
      face_alpha = 0.75;
    }
  group->setAttribute("face_alpha", face_alpha);

  if (grm_args_values(subplot_args, "phiflip", "i", &phiflip) == 0)
    {
      phiflip = 0;
    }
  group->setAttribute("phiflip", phiflip);

  if (grm_args_values(*series, "draw_edges", "i", &draw_edges) == 0)
    {
      draw_edges = 0;
    }
  group->setAttribute("draw_edges", draw_edges);


  if (grm_args_values(*series, "stairs", "i", &stairs) == 0)
    {
      stairs = 0;
    }
  group->setAttribute("stairs", stairs);

  if (grm_args_first_value(*series, "rlim", "D", &rlim, &dummy) == 0)
    {
      rlim = nullptr;
    }
  else
    {
      group->setAttribute("rlim", true);
      group->setAttribute("rlim0", rlim[0]);
      group->setAttribute("rlim1", rlim[1]);
    }

  if (grm_args_values(*series, "xcolormap", "i", &xcolormap))
    {
      group->setAttribute("xcolormap", xcolormap);
    }
  if (grm_args_values(*series, "ycolormap", "i", &ycolormap))
    {
      group->setAttribute("ycolormap", ycolormap);
    }

  return ERROR_NONE;
}

err_t plot_pie(grm_args_t *subplot_args)
{
  grm_args_t *series;
  double *x;
  unsigned int x_length;
  int color_index;
  unsigned char color_rgb[4];
  const char *title;
  err_t error = ERROR_NONE;
  static unsigned int color_array_length = -1;
  const int *color_indices = nullptr;
  const double *color_rgb_values = nullptr;
  std::string color_indices_key;
  std::string color_rgb_values_key;
  std::vector<int> color_indices_vec;
  std::vector<double> color_rgb_values_vec;

  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();
  group->setAttribute("name", "pie");

  grm_args_values(subplot_args, "series", "a", &series); /* series exists always */

  auto subGroup = global_render->createSeries("pie");
  group->append(subGroup);

  int id = static_cast<int>(global_root->getAttribute("id"));
  global_root->setAttribute("id", id + 1);
  std::string str = std::to_string(id);
  auto context = global_render->getContext();

  return_error_if(!grm_args_first_value(series, "x", "D", &x, &x_length), ERROR_PLOT_MISSING_DATA);

  if (x_length > 0)
    {
      std::vector<double> x_vec(x, x + x_length);
      (*context)["x" + str] = x_vec;
      subGroup->setAttribute("x", "x" + str);
    }

  if (grm_args_first_value(series, "c", "I", &color_indices, &color_array_length))
    {
      std::vector<double> color_indec_vec(color_indices, color_indices + color_array_length);
      (*context)["color_indices" + str] = color_indec_vec;
      subGroup->setAttribute("color_indices", "color_indices" + str);
    }
  else if (grm_args_first_value(series, "c", "D", &color_rgb_values, &color_array_length))
    {
      std::vector<double> color_rgb_vec(color_rgb_values, color_rgb_values + color_array_length);
      (*context)["color_rgb_values" + str] = color_rgb_vec;
      subGroup->setAttribute("color_rgb_values", "color_rgb_values" + str);
    }
  if (grm_args_values(subplot_args, "title", "s", &title))
    {
      group->setAttribute("title", title);
    }

  return error;
}

err_t plot_trisurf(grm_args_t *subplot_args)
{
  grm_args_t **current_series;

  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();
  group->setAttribute("name", "trisurf");
  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double *x, *y, *z;
      unsigned int x_length, y_length, z_length;
      return_error_if(!grm_args_first_value(*current_series, "x", "D", &x, &x_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(!grm_args_first_value(*current_series, "y", "D", &y, &y_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(!grm_args_first_value(*current_series, "z", "D", &z, &z_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(x_length != y_length || x_length != z_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);

      int id = (int)global_root->getAttribute("id");
      global_root->setAttribute("id", id + 1);
      std::string str = std::to_string(id);

      std::vector<double> x_vec(x, x + x_length), y_vec(y, y + x_length), z_vec(z, z + x_length);
      auto temp = global_render->createTriSurface("px" + str, x_vec, "py" + str, y_vec, "pz" + str, z_vec);
      group->append(temp);
      ++current_series;
    }
  plot_draw_axes(subplot_args, 2);
  plot_draw_colorbar(subplot_args, 0.05, 256);

  return ERROR_NONE;
}

err_t plot_tricont(grm_args_t *subplot_args)
{
  double z_min, z_max;
  double *levels;
  int num_levels;
  grm_args_t **current_series;
  int i;

  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();
  group->setAttribute("name", "tricont");

  grm_args_values(subplot_args, "levels", "i", &num_levels);
  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double *x, *y, *z;
      unsigned int x_length, y_length, z_length;
      auto subGroup = global_render->createSeries("tricontour");
      group->append(subGroup);

      return_error_if(!grm_args_first_value(*current_series, "x", "D", &x, &x_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(!grm_args_first_value(*current_series, "y", "D", &y, &y_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(!grm_args_first_value(*current_series, "z", "D", &z, &z_length), ERROR_PLOT_MISSING_DATA);
      return_error_if(x_length != y_length || x_length != z_length, ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);

      int id = (int)global_root->getAttribute("id");
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> x_vec(x, x + x_length);
      (*context)["px" + str] = x_vec;
      subGroup->setAttribute("px", "px" + str);

      std::vector<double> y_vec(y, y + y_length);
      (*context)["py" + str] = y_vec;
      subGroup->setAttribute("py", "py" + str);

      std::vector<double> z_vec(z, z + z_length);
      (*context)["pz" + str] = z_vec;
      subGroup->setAttribute("pz", "pz" + str);

      subGroup->setAttribute("levels", num_levels);

      global_root->setAttribute("id", id + 1);
      ++current_series;
    }
  plot_draw_colorbar(subplot_args, 0.0, 256);

  return ERROR_NONE;
}

err_t plot_shade(grm_args_t *subplot_args)
{
  grm_args_t **current_shader;
  /* char *spec = ""; TODO: read spec from data! */
  int xform, xbins, ybins;
  double *x, *y;
  unsigned int x_length, y_length;

  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();
  group->setAttribute("name", "shade");

  grm_args_values(subplot_args, "series", "A", &current_shader);
  auto subGroup = global_render->createSeries("shade");
  group->append(subGroup);

  grm_args_first_value(*current_shader, "x", "D", &x, &x_length);
  grm_args_first_value(*current_shader, "y", "D", &y, &y_length);

  int id = static_cast<int>(global_root->getAttribute("id"));
  std::string str = std::to_string(id);
  global_root->setAttribute("id", ++id);
  auto context = global_render->getContext();

  std::vector<double> x_vec(x, x + x_length);
  std::vector<double> y_vec(y, y + y_length);

  (*context)["x" + str] = x_vec;
  subGroup->setAttribute("x", "x" + str);
  (*context)["y" + str] = y_vec;
  subGroup->setAttribute("y", "y" + str);

  if (!grm_args_values(subplot_args, "xform", "i", &xform))
    {
      xform = 5;
    }
  if (!grm_args_values(subplot_args, "xbins", "i", &xbins))
    {
      xbins = 1200;
    }
  if (!grm_args_values(subplot_args, "ybins", "i", &ybins))
    {
      ybins = 1200;
    }
  subGroup->setAttribute("xform", xform);
  subGroup->setAttribute("xbins", xbins);
  subGroup->setAttribute("ybins", ybins);

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

  global_root->setAttribute("clearws", 1);
  data_vec = std::vector<int>(graphics_data, graphics_data + strlen(graphics_data));
  global_root->append(global_render->createDrawGraphics("graphics", data_vec));
  global_root->setAttribute("updatews", 1);

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
  int x_grid;
  int y_grid;
  int z_grid;
  char *title;
  char *x_label, *y_label, *z_label;
  int tick_orientation = 1;
  int keep_aspect_ratio;
  std::shared_ptr<GRM::Element> group;

  if (global_render->getElementsByTagName("coordinate_system").empty())
    {
      group = global_render->createElement("coordinate_system");
      if (!currentDomElement)
        {
          global_root->lastChildElement()->append(group);
        }
      else
        {
          currentDomElement->append(group);
        }
    }
  else
    {
      group = global_render->getElementsByTagName("coordinate_system")[0];
    }
  grm_args_values(args, "kind", "s", &kind);
  grm_args_values(args, "xgrid", "i", &x_grid);
  grm_args_values(args, "ygrid", "i", &y_grid);
  grm_args_values(args, "keep_aspect_ratio", "i", &keep_aspect_ratio); // todo needed?

  global_render->setLineColorInd(group, 1);
  global_render->setLineWidth(group, 1);

  if (str_equals_any(kind, 6, "wireframe", "surface", "plot3", "scatter3", "trisurf", "volume"))
    {
      grm_args_values(args, "zgrid", "i", &z_grid);
      if (pass == 1)
        {
          auto grid3d = global_render->createEmptyGrid3d(x_grid, false, z_grid);
          global_render->setOriginPosition3d(grid3d, "low", "high", "low");
          grid3d->setAttribute("x_major", 2);
          grid3d->setAttribute("y_major", 0);
          grid3d->setAttribute("z_major", 2);
          group->append(grid3d);
          grid3d = global_render->createEmptyGrid3d(false, y_grid, false);
          global_render->setOriginPosition3d(grid3d, "low", "high", "low");
          grid3d->setAttribute("x_major", 0);
          grid3d->setAttribute("y_major", 2);
          grid3d->setAttribute("z_major", 0);
          group->append(grid3d);
        }
      else
        {
          auto axes3d = global_render->createEmptyAxes3d(-tick_orientation);
          global_render->setOriginPosition3d(axes3d, "low", "low", "low");
          axes3d->setAttribute("y_tick", 0);
          axes3d->setAttribute("y_major", 0);
          axes3d->setAttribute("z_index", 2);
          group->append(axes3d);
          axes3d = global_render->createEmptyAxes3d(tick_orientation);
          global_render->setOriginPosition3d(axes3d, "high", "low", "low");
          axes3d->setAttribute("x_tick", 0);
          axes3d->setAttribute("z_tick", 0);
          axes3d->setAttribute("x_major", 0);
          axes3d->setAttribute("z_major", 0);
          axes3d->setAttribute("z_index", 2);
          group->append(axes3d);
        }
    }
  else
    {
      if (str_equals_any(kind, 3, "heatmap", "shade", "marginalheatmap"))
        {
          tick_orientation = -1;
        }
      if (!str_equals_any(kind, 1, "shade"))
        {
          if (pass == 1 || strcmp(kind, "barplot") != 0)
            {
              auto grid = global_render->createEmptyGrid(x_grid, y_grid);
              grid->setAttribute("x_org", 0);
              grid->setAttribute("y_org", 0);
              group->append(grid);
            }
        }
      if (strcmp(kind, "barplot") != 0 || pass == 2)
        {
          auto axes = global_render->createEmptyAxes(tick_orientation);
          global_render->setOriginPosition(axes, "low", "low");
          if (pass == 2) axes->setAttribute("z_index", 2);
          group->append(axes);
          axes = global_render->createEmptyAxes(-tick_orientation);
          global_render->setOriginPosition(axes, "high", "high");
          if (pass == 2) axes->setAttribute("z_index", 2);
          group->append(axes);

          if (strcmp("barplot", kind) == 0)
            {
              /* xticklabels */
              char **xticklabels = nullptr;
              unsigned int xticklabels_length;
              double y_min, y_max;

              grm_args_values(args, "_ylim", "dd", &y_min, &y_max);

              if (grm_args_first_value(args, "xticklabels", "S", &xticklabels, &xticklabels_length))
                {
                  std::vector<std::string> xticklabels_vec(xticklabels, xticklabels + xticklabels_length);
                  int id = static_cast<int>(global_root->getAttribute("id"));
                  std::string key = "xticklabels" + std::to_string(id);
                  global_root->setAttribute("id", ++id);
                  global_render->setXTickLabels(group, key, xticklabels_vec);
                }

              /* negative values */
              if (y_min < 0)
                {
                  group->append(global_render->createYLine());
                }
            }
        }
    }

  if (pass == 1 && grm_args_values(args, "title", "s", &title))
    {
      group->parentElement()->setAttribute("title", title);
    }

  if (str_equals_any(kind, 6, "wireframe", "surface", "plot3", "scatter3", "trisurf", "volume"))
    {
      std::string xlabel, ylabel, zlabel;
      bool title3d = false;
      if (grm_args_values(args, "xlabel", "s", &x_label))
        {
          xlabel = x_label;
          title3d = true;
        }
      if (grm_args_values(args, "ylabel", "s", &y_label))
        {
          ylabel = y_label;
          title3d = true;
        }
      if (grm_args_values(args, "zlabel", "s", &z_label))
        {
          zlabel = z_label;
          title3d = true;
        }
      if (pass == 2 && title3d)
        {
          auto title3d = global_render->createTitles3d(xlabel, ylabel, zlabel);
          title3d->setAttribute("z_index", 2);
          group->append(title3d);
        }
    }
  else
    {
      if (grm_args_values(args, "xlabel", "s", &x_label))
        {
          for (const auto &axes : group->getElementsByTagName("axes"))
            {
              if (static_cast<std::string>(axes->getAttribute("x_org_pos")) == "low")
                axes->setAttribute("xlabel", x_label);
            }
        }
      if (grm_args_values(args, "ylabel", "s", &y_label))
        {
          for (const auto &axes : group->getElementsByTagName("axes"))
            {
              if (static_cast<std::string>(axes->getAttribute("x_org_pos")) == "low")
                axes->setAttribute("ylabel", y_label);
            }
        }
    }

  return ERROR_NONE;
}

err_t plot_draw_polar_axes(grm_args_t *args)
{
  double diag;
  double charheight;
  double r_min = 0.0, r_max = -1.0;
  double tick;
  double x[2], y[2];
  int i, n;
  double alpha;
  char text_buffer[PLOT_POLAR_AXES_TEXT_BUFFER];
  char *kind;
  int angle_ticks, rings;
  int phiflip = 0;
  double interval;
  const char *norm, *title;
  std::shared_ptr<GRM::Element> group, subGroup;

  if (global_render->getElementsByTagName("coordinate_system").empty())
    {
      group = global_render->createElement("coordinate_system");
      if (!currentDomElement)
        {
          global_root->lastChildElement()->append(group);
        }
      else
        {
          currentDomElement->append(group);
        }
    }
  else
    {
      group = global_render->getElementsByTagName("coordinate_system")[0];
    }

  if (grm_args_values(args, "angle_ticks", "i", &angle_ticks) == 0)
    {
      angle_ticks = 8;
    }

  grm_args_values(args, "kind", "s", &kind);

  if (strcmp(kind, "polar_histogram") == 0)
    {
      if (grm_args_values(args, "normalization", "s", &norm) == 0) norm = "count";
    }

  if (grm_args_values(args, "phiflip", "i", &phiflip) == 0) phiflip = 0;

  if (!grm_args_values(args, "title", "s", &title)) title = "";
  group->parentElement()->setAttribute("title", title);

  if (strcmp(kind, "polar_histogram") == 0)
    {
      // todo: move series/data dependent code into the renderer.
      subGroup = global_render->createDrawPolarAxes(angle_ticks, kind, phiflip, norm, 1.0);
      //      grm_args_values(args, "r_max", "d", &r_max);
      //      group->setAttribute("r_max", r_max);
    }
  else
    {
      subGroup = global_render->createDrawPolarAxes(angle_ticks, kind, phiflip, "");
    }
  group->append(subGroup);
  return ERROR_NONE;
}

err_t plot_draw_legend(grm_args_t *subplot_args)
{
  const char **labels, **current_label;
  unsigned int num_labels, num_series;
  grm_args_t **current_series;
  int location;
  double px, py, w, h;
  double tbx[4], tby[4];
  double legend_symbol_x[2], legend_symbol_y[2];
  int i;

  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();

  return_error_if(!grm_args_first_value(subplot_args, "labels", "S", &labels, &num_labels), ERROR_PLOT_MISSING_LABELS);
  logger((stderr, "Draw a legend with %d labels\n", num_labels));
  grm_args_values(subplot_args, "location", "i", &location);
  grm_args_first_value(subplot_args, "series", "A", &current_series, &num_series);

  int id = static_cast<int>(global_root->getAttribute("id"));
  global_root->setAttribute("id", ++id);

  std::string labels_key = std::to_string(id) + "labels";
  std::string specs_key = std::to_string(id) + "specs";
  std::vector<std::string> labels_vec(labels, labels + num_labels);

  std::vector<std::string> specs_vec;
  while (*current_series != nullptr)
    {
      char *spec;
      grm_args_values(*current_series, "spec", "s", &spec); /* `spec` is always set */
      specs_vec.push_back(std::string(spec));
      ++current_series;
    }

  auto subGroup = global_render->createLegend(labels_key, labels_vec, location, specs_key, specs_vec);
  group->append(subGroup);

  return ERROR_NONE;
}

err_t plot_draw_pie_legend(grm_args_t *subplot_args)
{
  const char **labels;
  unsigned int num_labels;
  grm_args_t *series;

  static const int *color_indices = nullptr;
  static const double *color_rgb_values = nullptr;
  static unsigned int color_array_length = -1;
  int useFallback = 0;

  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();

  return_error_if(!grm_args_first_value(subplot_args, "labels", "S", &labels, &num_labels), ERROR_PLOT_MISSING_LABELS);
  grm_args_values(subplot_args, "series", "a", &series); /* series exists always */

  int id = static_cast<int>(global_root->getAttribute("id"));
  global_root->setAttribute("id", id + 1);
  std::string labels_key = "labels" + std::to_string(id);
  std::vector<std::string> labels_vec(labels, labels + num_labels);

  auto drawPieLegendElement = global_render->createPieLegend(labels_key, labels_vec);
  group->append(drawPieLegendElement);

  return ERROR_NONE;
}

err_t plot_draw_colorbar(grm_args_t *subplot_args, double off, unsigned int colors)
{
  double c_min, c_max;
  int *data;
  int scale, flip, options;
  unsigned int i;

  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();

  auto colorbar = global_render->createColorbar(colors);
  group->append(colorbar);

  colorbar->setAttribute("flip", 1);
  colorbar->setAttribute("xflip", 0);
  colorbar->setAttribute("yflip", 0);
  if (grm_args_values(subplot_args, "xflip", "i", &flip) && flip)
    {
      colorbar->setAttribute("xflip", flip);
    }
  else if (grm_args_values(subplot_args, "yflip", "i", &flip) && flip)
    {
      colorbar->setAttribute("yflip", flip);
    }

  colorbar->setAttribute("offset", off + 0.02);
  colorbar->setAttribute("width", 0.03);
  colorbar->setAttribute("colorbar_position", true);

  colorbar->setAttribute("diag_factor", 0.016);
  colorbar->setAttribute("max_charheight", 0.012);
  colorbar->setAttribute("relative_charheight", true);

  return ERROR_NONE;
}

err_t extract_multi_type_argument(grm_args_t *error_container, const char *key, unsigned int x_length,
                                  unsigned int *downwards_length, unsigned int *upwards_length, double **downwards,
                                  double **upwards, double *downwards_flt, double *upwards_flt)
{
  arg_t *arg_ptr;
  args_value_iterator_t *value_it;
  unsigned int length;
  int i, *ii;

  arg_ptr = args_at(error_container, key);
  if (!arg_ptr)
    {
      return ERROR_NONE;
    }
  if (strcmp(arg_ptr->value_format, "nDnD") == 0)
    {
      value_it = arg_value_iter(arg_ptr);
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

err_t plot_draw_errorbars(grm_args_t *series_args, double *x, unsigned int x_length, double *y, const char *kind)
{
  grm_args_t *error_container;
  arg_t *arg_ptr;
  err_t error;

  double *absolute_upwards = nullptr, *absolute_downwards = nullptr, *relative_upwards = nullptr,
         *relative_downwards = nullptr;
  double absolute_upwards_flt, relative_upwards_flt, absolute_downwards_flt, relative_downwards_flt;
  unsigned int upwards_length, downwards_length, i;
  int color_upwardscap, color_downwardscap, color_errorbar;

  std::shared_ptr<GRM::Element> group =
      (currentDomElement) ? currentDomElement->lastChildElement() : global_root->lastChildElement()->lastChildElement();

  arg_ptr = args_at(series_args, "error");
  if (!arg_ptr)
    {
      return ERROR_NONE;
    }
  error_container = nullptr;

  auto subGroup = global_render->createElement("errorbars");
  group->append(subGroup);

  int id = static_cast<int>(global_root->getAttribute("id"));
  std::string str = std::to_string(id);
  global_root->setAttribute("id", ++id);
  auto context = global_render->getContext();

  std::vector<double> x_vec(x, x + x_length);
  std::vector<double> y_vec(y, y + x_length);

  (*context)["x" + str] = x_vec;
  subGroup->setAttribute("x", "x" + str);
  (*context)["y" + str] = y_vec;
  subGroup->setAttribute("y", "y" + str);

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
      if (grm_args_values(error_container, "upwardscap_color", "i", &color_upwardscap))
        subGroup->setAttribute("upwardscap_color", color_upwardscap);
      if (grm_args_values(error_container, "downwardscap_color", "i", &color_downwardscap))
        subGroup->setAttribute("downwardscap_color", color_downwardscap);
      if (grm_args_values(error_container, "errorbar_color", "i", &color_errorbar))
        subGroup->setAttribute("errorbar_color", color_errorbar);
    }

  return ERROR_NONE;
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ util ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

double find_max_step(unsigned int n, const double *x)
{
  double max_step = 0.0;
  unsigned int i;

  if (n < 2)
    {
      return 0.0;
    }
  for (i = 1; i < n; ++i)
    {
      max_step = grm_max(x[i] - x[i - 1], max_step);
    }

  return max_step;
}

const char *next_fmt_key(const char *kind)
{
  static const char *saved_fmt = nullptr;
  static char fmt_key[2] = {0, 0};

  if (kind != nullptr)
    {
      string_map_at(fmt_map, kind, static_cast<const char **>(&saved_fmt));
    }
  if (saved_fmt == nullptr)
    {
      return nullptr;
    }
  fmt_key[0] = *saved_fmt;
  if (*saved_fmt != '\0')
    {
      ++saved_fmt;
    }

  return fmt_key;
}

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

  return _plot_id > 0 || _subplot_id > 0 || _series_id > 0;
}

int get_figure_size(const grm_args_t *plot_args, int *pixel_width, int *pixel_height, double *metric_width,
                    double *metric_height)
{
  double display_metric_width, display_metric_height;
  int display_pixel_width, display_pixel_height;
  double dpm[2], dpi[2];
  int tmp_size_i[2], pixel_size[2];
  double tmp_size_d[2], metric_size[2];
  grm_args_ptr_t tmp_size_a[2];
  const char *tmp_size_s[2];
  int i;

  if (plot_args == nullptr)
    {
      plot_args = active_plot_args;
    }

#ifdef __EMSCRIPTEN__
  display_metric_width = 0.16384;
  display_metric_height = 0.12288;
  display_pixel_width = 640;
  display_pixel_height = 480;
#else
  gr_inqdspsize(&display_metric_width, &display_metric_height, &display_pixel_width, &display_pixel_height);
#endif
  dpm[0] = display_pixel_width / display_metric_width;
  dpm[1] = display_pixel_height / display_metric_height;
  dpi[0] = dpm[0] * 0.0254;
  dpi[1] = dpm[1] * 0.0254;

  /* TODO: Overwork this calculation */
  if (grm_args_values(plot_args, "figsize", "dd", &tmp_size_d[0], &tmp_size_d[1]))
    {
      for (i = 0; i < 2; ++i)
        {
          pixel_size[i] = (int)grm_round(tmp_size_d[i] * dpi[i]);
          metric_size[i] = tmp_size_d[i] / 0.0254;
        }
    }
  else if (grm_args_values(plot_args, "size", "dd", &tmp_size_d[0], &tmp_size_d[1]))
    {
      for (i = 0; i < 2; ++i)
        {
          pixel_size[i] = (int)grm_round(tmp_size_d[i]);
          metric_size[i] = tmp_size_d[i] / dpm[i];
        }
    }
  else if (grm_args_values(plot_args, "size", "ii", &tmp_size_i[0], &tmp_size_i[1]))
    {
      for (i = 0; i < 2; ++i)
        {
          pixel_size[i] = tmp_size_i[i];
          metric_size[i] = tmp_size_i[i] / dpm[i];
        }
    }
  else if (grm_args_values(plot_args, "size", "aa", &tmp_size_a[0], &tmp_size_a[1]))
    {
      for (i = 0; i < 2; ++i)
        {
          double pixels_per_unit = 1;
          if (grm_args_values(tmp_size_a[i], "unit", "s", &tmp_size_s[i]))
            {
              if (strcmp(tmp_size_s[i], "px") != 0)
                {
                  double meters_per_unit;
                  if (double_map_at(meters_per_unit_map, tmp_size_s[i], &meters_per_unit))
                    {
                      pixels_per_unit = meters_per_unit * dpm[i];
                    }
                  else
                    {
                      debug_print_error(("The unit %s is unknown.\n", tmp_size_s[i]));
                    }
                }
            }
          if (grm_args_values(tmp_size_a[i], "value", "i", &tmp_size_i[i]))
            {
              tmp_size_d[i] = tmp_size_i[i] * pixels_per_unit;
            }
          else if (grm_args_values(tmp_size_a[i], "value", "d", &tmp_size_d[i]))
            {
              tmp_size_d[i] = tmp_size_d[i] * pixels_per_unit;
            }
          else
            {
              /* If no value is given, fall back to default value */
              return 0;
            }
          pixel_size[i] = (int)grm_round(tmp_size_d[i]);
          metric_size[i] = tmp_size_d[i] / dpm[i];
        }
    }
  else
    {
      /* If this branch is executed, there is an internal error (size has a default value if not set by the user) */
      return 0;
    }

  logger((stderr, "figure pixel size: (%d, %d)\n", pixel_size[0], pixel_size[1]));
  logger((stderr, "figure metric size: (%f, %f)\n", metric_size[0], metric_size[1]));
  logger((stderr, "device dpi: (%lf, %lf)\n", dpi[0], dpi[1]));

  if (pixel_width != nullptr)
    {
      *pixel_width = pixel_size[0];
    }
  if (pixel_height != nullptr)
    {
      *pixel_height = pixel_size[1];
    }
  if (metric_width != nullptr)
    {
      *metric_width = metric_size[0];
    }
  if (metric_height != nullptr)
    {
      *metric_height = metric_size[1];
    }

  return 1;
}

int get_focus_and_factor(const int x1, const int y1, const int x2, const int y2, const int keep_aspect_ratio,
                         double *factor_x, double *factor_y, double *focus_x, double *focus_y,
                         grm_args_t **subplot_args)
{
  double ndc_box_x[4], ndc_box_y[4];
  double ndc_left, ndc_top, ndc_right, ndc_bottom;
  const double *wswindow, *viewport;
  int width, height, max_width_height;

  get_figure_size(nullptr, &width, &height, nullptr, nullptr);
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
  *subplot_args = get_subplot_from_ndc_points(array_size(ndc_box_x), ndc_box_x, ndc_box_y);
  if (*subplot_args == nullptr)
    {
      return 0;
    }
  grm_args_values(*subplot_args, "viewport", "D", &viewport);
  grm_args_values(active_plot_args, "wswindow", "D", &wswindow);

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

double *moivre(double r, int x, int n)
{
  double *result = static_cast<double *>(malloc(2 * sizeof(double)));
  if (result != nullptr)
    {
      if (n != 0)
        {
          *result = pow(r, (1.0 / n)) * (cos(2.0 * x * M_PI / n));
          *(result + 1) = pow(r, (1.0 / n)) * (sin(2.0 * x * M_PI / n));
        }
      else
        {
          *result = 1.0;
          *(result + 1) = 0.0;
        }
    }
  return result;
}


/* like python list comprehension [factor * func(element) for element in list] saves values in result starting at start
 * index */

double *listcomprehension(double factor, double (*pFunction)(double), double *list, int num, int start, double *result)
{
  int i;
  if (result == nullptr)
    {
      result = static_cast<double *>(malloc(num * sizeof(double)));
      if (result == nullptr)
        {
          return nullptr;
        }
    }

  for (i = 0; i < num; ++i)
    {
      result[start] = factor * (*pFunction)(list[i]);
      start++;
    }

  return result;
}

/*
 * mixes gr colormaps with size = size * size. If x and or y < 0
 * */
int *create_colormap(int x, int y, int size)
{
  int r, g, b, a;
  int outer, inner;
  int r1, g1, b1;
  int r2, g2, b2;
  int *colormap = nullptr;
  if (x > 47 || y > 47)
    {
      logger((stderr, "values for the keyword \"colormap\" can not be greater than 47\n"));
      return nullptr;
    }

  colormap = static_cast<int *>(malloc(size * size * sizeof(int)));
  if (colormap == nullptr)
    {
      debug_print_malloc_error();
      return nullptr;
    }
  if (x >= 0 && y < 0)
    {
      for (outer = 0; outer < size; outer++)
        {
          for (inner = 0; inner < size; inner++)
            {
              a = 255;
              r = ((cmap_h[x][(int)(inner * 255.0 / size)] >> 16) & 0xff);
              g = ((cmap_h[x][(int)(inner * 255.0 / size)] >> 8) & 0xff);
              b = (cmap_h[x][(int)(inner * 255.0 / size)] & 0xff);

              colormap[outer * size + inner] = (a << 24) + (b << 16) + (g << 8) + (r);
            }
        }
      return colormap;
    }

  if (x < 0 && y >= 0)
    {
      gr_setcolormap(y);
      for (outer = 0; outer < size; outer++)
        {
          for (inner = 0; inner < size; inner++)
            {
              a = 255;
              r = ((cmap_h[y][(int)(inner * 255.0 / size)] >> 16) & 0xff);
              g = ((cmap_h[y][(int)(inner * 255.0 / size)] >> 8) & 0xff);
              b = (cmap_h[y][(int)(inner * 255.0 / size)] & 0xff);

              colormap[inner * size + outer] = (a << 24) + (b << 16) + (g << 8) + (r);
            }
        }


      return colormap;
    }

  if ((x >= 0 && y >= 0) || (x < 0 && y < 0))
    {
      if (x < 0 && y < 0)
        {
          x = y = 0;
        }
      gr_setcolormap(x);
      for (outer = 0; outer < size; outer++)
        {
          for (inner = 0; inner < size; inner++)
            {
              a = 255;
              r1 = ((cmap_h[x][(int)(inner * 255.0 / size)] >> 16) & 0xff);
              g1 = ((cmap_h[x][(int)(inner * 255.0 / size)] >> 8) & 0xff);
              b1 = (cmap_h[x][(int)(inner * 255.0 / size)] & 0xff);

              r2 = ((cmap_h[y][(int)(outer * 255.0 / size)] >> 16) & 0xff);
              g2 = ((cmap_h[y][(int)(outer * 255.0 / size)] >> 8) & 0xff);
              b2 = (cmap_h[y][(int)(outer * 255.0 / size)] & 0xff);

              colormap[outer * size + inner] =
                  (a << 24) + (((b1 + b2) / 2) << 16) + (((g1 + g2) / 2) << 8) + ((r1 + r2) / 2);
            }
        }
      return colormap;
    }
  return nullptr;
}


/*
 * Calculates the classes for polar histogram
 * */
err_t classes_polar_histogram(grm_args_t *subplot_args)
{
  unsigned int num_bins;
  double *theta = nullptr;
  unsigned int length;
  const char *norm;
  double *classes = nullptr;

  double interval;
  double start;
  double *p;
  double max;
  double temp_max;

  double *bin_edges = nullptr;
  double *bin_edges_buf = nullptr;
  unsigned int num_bin_edges;

  double bin_width;

  double *bin_widths = nullptr;

  int is_bin_counts;
  int *bin_counts = nullptr;

  double false_ = -1;

  double *philim = nullptr;
  unsigned int dummy;

  double *new_theta = nullptr;
  double *new_edges = nullptr;

  grm_args_t **series;

  err_t error = ERROR_NONE;

  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();
  std::shared_ptr<GRM::Context> context = global_render->getContext();

  group->setAttribute("polar_histogram_classes", true);

  int id = static_cast<int>(global_root->getAttribute("id"));
  global_root->setAttribute("id", id + 1);
  auto str = std::to_string(id);

  //! store id string in group for later usages in render.cxx
  group->setAttribute("id", str);

  grm_args_values(subplot_args, "series", "A", &series);

  /* get theta or bin_counts */
  if (grm_args_values(*series, "bin_counts", "i", &is_bin_counts) == 0)
    {
      is_bin_counts = 0;
      grm_args_first_value(*series, "x", "D", &theta, &length);
      std::vector<double> theta_vec(theta, theta + length);
      (*context)["theta" + str] = theta_vec;
      group->setAttribute("theta", "theta" + str);
    }
  else
    {
      grm_args_first_value(*series, "x", "I", &bin_counts, &length);
      std::vector<int> bin_counts_vec(bin_counts, bin_counts + length);
      (*context)["bin_counts" + str] = bin_counts_vec;
      group->setAttribute("bin_counts", "bin_counts" + str);

      is_bin_counts = 1;
      num_bins = length;
      grm_args_push(*series, "nbins", "i", num_bins);
      group->setAttribute("nbins", static_cast<int>(num_bins));
    }

  if (grm_args_first_value(*series, "philim", "D", &philim, &dummy))
    {
      int phiflip;
      grm_args_values(subplot_args, "phiflip", "i", &phiflip);
      group->setAttribute("phiflip", phiflip);
      group->setAttribute("philim", true);
      group->setAttribute("phimin", philim[0]);
      group->setAttribute("phimax", philim[1]);
    }


  /* bin_edges and nbins */
  if (grm_args_first_value(*series, "bin_edges", "D", &bin_edges, &num_bin_edges) == 0)
    {

      if (grm_args_values(*series, "nbins", "i", &num_bins))
        {
          group->setAttribute("nbins", static_cast<int>(num_bins));
        }
    }
  /* with bin_edges */
  else
    {
      std::vector<double> bin_edges_vec(bin_edges, bin_edges + num_bin_edges);
      (*context)["bin_edges" + str] = bin_edges_vec;
      group->setAttribute("bin_edges", "bin_edges" + str);
    }

  if (grm_args_values(subplot_args, "normalization", "s", &norm) == 0)
    {
      norm = "count";
    }
  group->setAttribute("normalization", norm);

  if (grm_args_values(*series, "bin_width", "d", &bin_width))
    {
      group->setAttribute("bin_width", bin_width);
    }

  return error;
}

/*!
 * Convert an RGB triple to a luminance value following the CCIR 601 format.
 *
 * \param[in] r The red component of the RGB triple in the range [0.0, 1.0].
 * \param[in] g The green component of the RGB triple in the range [0.0, 1.0].
 * \param[in] b The blue component of the RGB triple in the range [0.0, 1.0].
 * \return The luminance of the given RGB triple in the range [0.0, 1.0].
 */
double get_lightness_from_rbg(double r, double g, double b)
{
  return 0.299 * r + 0.587 * g + 0.114 * b;
}

/*!
 * Set the text color either to black or white depending on the given background color.
 *
 * \param[in] r The red component of the RGB triple in the range [0.0, 1.0].
 * \param[in] g The green component of the RGB triple in the range [0.0, 1.0].
 * \param[in] b The blue component of the RGB triple in the range [0.0, 1.0].
 * \return The luminance of the given RGB triple.
 */
void set_text_color_for_background(double r, double g, double b)
{
  double color_lightness;

  color_lightness = get_lightness_from_rbg(r, g, b);
  if (color_lightness < 0.4)
    {
      gr_settextcolorind(0);
    }
  else
    {
      gr_settextcolorind(1);
    }
}


/*!
 * Draw an xticklabel at the specified position while trying to stay in the available space.
 * Every space character (' ') is seen as a possible position to break the label into the next line.
 * The label will not break into the next line when enough space is available.
 * If a label or a part of it does not fit in the available space but doesnt have a space character to break it up
 * it will get drawn anyway.
 *
 * \param[in] x1 The X coordinate of starting position of the label.
 * \param[in] x2 The Y coordinate of starting position of the label.
 * \param[in] label The label to be drawn.
 * \param[in] available_width The available space in X direction around the starting position.
 */
void draw_xticklabel(double x1, double x2, const char *label, double available_width)
{
  char new_label[256];
  int breakpoint_positions[128];
  int cur_num_breakpoints = 0;
  int i = 0;
  int cur_start = 0;
  double tbx[4], tby[4];
  double width;
  double charheight;

  gr_inqcharheight(&charheight);


  for (i = 0; i == 0 || label[i - 1] != '\0'; ++i)
    {
      if (label[i] == ' ' || label[i] == '\0')
        {
          /* calculate width of the next part of the label to be drawn */
          new_label[i] = '\0';
          gr_inqtext(x1, x2, new_label + cur_start, tbx, tby);
          gr_wctondc(&tbx[0], &tby[0]);
          gr_wctondc(&tbx[2], &tby[2]);
          width = tbx[2] - tbx[0];
          new_label[i] = ' ';

          /* add possible breakpoint */
          breakpoint_positions[cur_num_breakpoints++] = i;

          if (width > available_width)
            {
              /* part is too big but doesnt have a breakpoint in it */
              if (cur_num_breakpoints == 1)
                {
                  new_label[i] = '\0';
                  gr_text(x1, x2, new_label + cur_start);

                  cur_start = i + 1;
                  cur_num_breakpoints = 0;
                }
              /* part is too big and has breakpoints in it */
              else
                {
                  /* break label at last breakpoint that still allowed the text to fit */
                  new_label[breakpoint_positions[cur_num_breakpoints - 2]] = '\0';
                  gr_text(x1, x2, new_label + cur_start);

                  cur_start = breakpoint_positions[cur_num_breakpoints - 2] + 1;
                  breakpoint_positions[0] = breakpoint_positions[cur_num_breakpoints - 1];
                  cur_num_breakpoints = 1;
                }
              x2 -= charheight * 1.5;
            }
        }
      else
        {
          new_label[i] = label[i];
        }
    }

  /* 0-terminate the new label */
  new_label[i] = '\0';

  /* draw the rest */
  gr_text(x1, x2, new_label + cur_start);
}

double auto_tick(double amin, double amax)
{
  double tick_size[] = {5.0, 2.0, 1.0, 0.5, 0.2, 0.1, 0.05, 0.02, 0.01};
  double scale, tick;
  int i, n;

  scale = pow(10.0, (int)(log10(amax - amin)));
  tick = 1.0;
  for (i = 0; i < 9; i++)
    {
      n = (amax - amin) / scale / tick_size[i];
      if (n > 7)
        {
          tick = tick_size[i - 1];
          break;
        }
    }
  tick *= scale;
  return tick;
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
      plot_static_variables_initialized = 0;
      grid_delete(global_grid);
      global_grid = nullptr;
    }
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

#ifndef NO_EXPAT
static void xml_parse_start_handler(void *data, const XML_Char *tagName, const XML_Char **attr)
{
  std::shared_ptr<GRM::Element> *insertionParent = (std::shared_ptr<GRM::Element> *)data;
  if (strcmp(tagName, "figure") == 0)
    {
      global_root = global_render->createElement("figure");
      global_render->replaceChildren(global_root);
      if (attr[0])
        {
          global_root->setAttribute(attr[0], attr[1]);
        }
      (*insertionParent) = global_root;
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

void grm_load_graphics_tree(FILE *file)
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
      return;
    }

  XML_ParserFree(parser);
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
  char *ylabel, *xlabel, *title, *kind;
  int keep_aspect_ratio, location, adjust_xlim, adjust_ylim;
  double *subplot;

  std::shared_ptr<GRM::Element> group = (currentDomElement) ? currentDomElement : global_root->lastChildElement();
  grm_args_values(subplot_args, "kind", "s", &kind);
  group->setAttribute("kind", kind);
  logger((stderr, "Got keyword \"kind\" with value \"%s\"\n", kind));


  if (plot_pre_subplot(subplot_args) != ERROR_NONE)
    {
      return 0;
    }

  if (grm_args_values(subplot_args, "ylabel", "s", &ylabel))
    {
      group->setAttribute("ylabel_margin", 1);
    }
  if (grm_args_values(subplot_args, "xlabel", "s", &xlabel))
    {
      group->setAttribute("xlabel_margin", 1);
    }
  if (grm_args_values(subplot_args, "title", "s", &title))
    {
      group->setAttribute("title_margin", 1);
    }
  if (grm_args_values(subplot_args, "keep_aspect_ratio", "i", &keep_aspect_ratio))
    {
      group->setAttribute("keep_aspect_ratio", keep_aspect_ratio);
    }
  if (grm_args_values(subplot_args, "location", "i", &location))
    {
      group->setAttribute("location", location);
    }
  if (grm_args_values(subplot_args, "subplot", "D", &subplot))
    {
      group->setAttribute("subplot", true);
      group->setAttribute("subplot_xmin", subplot[0]);
      group->setAttribute("subplot_xmax", subplot[1]);
      group->setAttribute("subplot_ymin", subplot[2]);
      group->setAttribute("subplot_ymax", subplot[3]);
    }
  if (grm_args_values(subplot_args, "adjust_xlim", "i", &adjust_xlim))
    {
      group->setAttribute("adjust_xlim", adjust_xlim);
    }
  if (grm_args_values(subplot_args, "adjust_ylim", "i", &adjust_ylim))
    {
      group->setAttribute("adjust_ylim", adjust_ylim);
    }

  if (!plot_func_map_at(plot_func_map, kind, &plot_func))
    {
      return 0;
    }
  if (plot_func(subplot_args) != ERROR_NONE)
    {
      return 0;
    };
  plot_post_subplot(subplot_args);
  return 1;
}

int grm_plot(const grm_args_t *args)
{
  grm_args_t **current_subplot_args;
  grm::Grid *currentGrid;
  plot_func_t plot_func;
  const char *kind = nullptr;
  int figsize_x, figsize_y, tmp_size_i[2];
  double tmp_size_d[2];
  grm_args_ptr_t tmp_size_a[2];
  const char *tmp_size_s[2];
  std::string vars[2] = {"x", "y"};
  double default_size[2] = {PLOT_DEFAULT_WIDTH, PLOT_DEFAULT_HEIGHT};

  if (!grm_merge(args))
    {
      return 0;
    }

  if (args == nullptr && global_render->documentElement())
    {
      global_render->render();
      return 1;
    }
  else
    {
      global_render = GRM::Render::createRender();
      global_root = global_render->createElement("figure");
      global_render->replaceChildren(global_root);
      global_root->setAttribute("id", 0);
      currentDomElement = nullptr;
    }

  if (grm_args_values(active_plot_args, "raw", "s", &current_subplot_args))
    {
      plot_raw(active_plot_args);
      global_render->render();
    }
  else
    {
      plot_set_attribute_defaults(active_plot_args);

      if (grm_args_values(active_plot_args, "size", "dd", &tmp_size_d[0], &tmp_size_d[1]))
        {
          global_root->setAttribute("size", true);
          for (int i = 0; i < 2; ++i)
            {
              global_root->setAttribute("size_" + vars[i], tmp_size_d[i]);
              global_root->setAttribute("size_" + vars[i] + "_type", "double");
              global_root->setAttribute("size_" + vars[i] + "_unit", "px");
            }
        }
      if (grm_args_values(active_plot_args, "size", "ii", &tmp_size_i[0], &tmp_size_i[1]))
        {
          global_root->setAttribute("size", true);
          for (int i = 0; i < 2; ++i)
            {
              global_root->setAttribute("size_" + vars[i], tmp_size_i[i]);
              global_root->setAttribute("size_" + vars[i] + "_type", "int");
              global_root->setAttribute("size_" + vars[i] + "_unit", "px");
            }
        }
      if (grm_args_values(active_plot_args, "size", "aa", &tmp_size_a[0], &tmp_size_a[1]))
        {
          global_root->setAttribute("size", true);
          for (int i = 0; i < 2; ++i)
            {
              if (grm_args_values(tmp_size_a[i], "unit", "s", &tmp_size_s[i]))
                {
                  global_root->setAttribute("size_" + vars[i] + "_unit", tmp_size_s[i]);
                }
              if (grm_args_values(tmp_size_a[i], "value", "i", &tmp_size_i[i]))
                {
                  global_root->setAttribute("size_" + vars[i] + "_type", "int");
                  global_root->setAttribute("size_" + vars[i], tmp_size_i[i]);
                }
              else if (grm_args_values(tmp_size_a[i], "value", "d", &tmp_size_d[i]))
                {
                  global_root->setAttribute("size_" + vars[i] + "_type", "double");
                  global_root->setAttribute("size_" + vars[i], tmp_size_d[i]);
                }
              else
                {
                  /* If no value is given, fall back to default value */
                  for (int i = 0; i < 2; ++i)
                    {
                      global_root->setAttribute("size_" + vars[i], default_size[i]);
                      global_root->setAttribute("size_" + vars[i] + "_type", "double");
                      global_root->setAttribute("size_" + vars[i] + "_unit", "px");
                    }
                  return 0;
                }
            }
        }
      if (grm_args_values(active_plot_args, "figsize", "dd", &figsize_x, &figsize_y))
        {
          global_root->setAttribute("figsize", true);
          global_root->setAttribute("figsize_x", figsize_x);
          global_root->setAttribute("figsize_y", figsize_y);
        }
      if (plot_process_grid_arguments(active_plot_args) != ERROR_NONE)
        {
          return 0;
        }

      currentGrid = reinterpret_cast<grm::Grid *>(global_grid);
      int nrows = currentGrid->getNRows();
      int ncols = currentGrid->getNCols();

      plot_pre_plot(active_plot_args);
      grm_args_values(active_plot_args, "subplots", "A", &current_subplot_args);

      if (!(nrows == 1 && ncols == 1 &&
            currentGrid->getElement(0, 0) == nullptr)) // Check if Grid arguments in container
        {
          auto gridDomElement = global_render->createLayoutGrid(*currentGrid);
          global_root->append(gridDomElement);

          for (auto const &elementToSlice : currentGrid->getElementToPosition())
            {
              grm_plot_helper(elementToSlice.first, elementToSlice.second, gridDomElement);
            }
        }
      else
        {
          std::cout << "No grid elements\n";
          while (*current_subplot_args != nullptr)
            {
              auto group = global_render->createElement("plot");
              group->setAttribute("subplotGroup", true);
              global_root->append(group);
              currentDomElement = group;
              if (!plot_process_subplot_args(*current_subplot_args))
                {
                  return 0;
                }
              ++current_subplot_args;
            }
        }

      plot_post_plot(active_plot_args);
      global_render->render();
    }

  process_events();

#ifndef NDEBUG
  logger((stderr, "root args after \"grm_plot\" (active_plot_index: %d):\n", active_plot_index - 1));
  if (logger_enabled())
    {
      grm_dump(global_root_args, stderr);
      grm_dump_graphics_tree(stderr);
    }
#endif

  return 1;
}

void grm_render(void)
{
  global_render->render();
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

  if (plot_init_static_variables() != ERROR_NONE)
    {
      return 0;
    }

  if (plot_init_args_structure(global_root_args, plot_hierarchy_names, id + 1) != ERROR_NONE)
    {
      return 0;
    }
  if (!grm_args_first_value(global_root_args, "plots", "A", &args_array, &args_array_length))
    {
      return 0;
    }
  if (id + 1 > args_array_length)
    {
      return 0;
    }

  active_plot_index = id + 1;
  active_plot_args = args_array[id];

  return 1;
}

} /* end of extern "C" block */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ c++ util ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


int grm_plot_helper(grm::GridElement *gridElement, grm::Slice *slice,
                    const std::shared_ptr<GRM::Element> &parentDomElement)
{
  plot_func_t plot_func;
  const char *kind = nullptr;

  if (gridElement == nullptr)
    {
      std::cout << "Error: gridElement is nullptr\n";
      return 0;
    }

  if (!gridElement->isGrid())
    {
      grm_args_t **current_subplot_args = &gridElement->subplot_args;
      auto subplotGroup = global_render->createLayoutGridElement(*gridElement, *slice);
      currentDomElement = subplotGroup;
      parentDomElement->append(subplotGroup);

      if (!plot_process_subplot_args(*current_subplot_args))
        {
          return 0;
        }
    }
  else
    {
      grm::Grid *currentGrid = reinterpret_cast<grm::Grid *>(gridElement);

      auto gridDomElement = global_render->createLayoutGrid(*currentGrid);
      gridDomElement->setAttribute("rowStart", slice->rowStart);
      gridDomElement->setAttribute("rowStop", slice->rowStop);
      gridDomElement->setAttribute("colStart", slice->colStart);
      gridDomElement->setAttribute("colStop", slice->colStop);
      parentDomElement->append(gridDomElement);

      for (auto const &elementToSlice : currentGrid->getElementToPosition())
        {
          grm_plot_helper(elementToSlice.first, elementToSlice.second, gridDomElement);
        }
    }
  return 0;
}

std::shared_ptr<GRM::Element> grm_get_document_root(void)
{
  return global_root;
}

std::shared_ptr<GRM::Render> grm_get_render(void)
{
  return global_render;
}

std::shared_ptr<GRM::Element> get_subplot_from_ndc_point_using_dom_helper(std::shared_ptr<GRM::Element> element,
                                                                          double x, double y)
{
  bool elementIsSubplotGroup =
      (element->hasAttribute("subplotGroup") && static_cast<int>(element->getAttribute("subplotGroup")));

  if (element->localName() == "layout_gridelement" || elementIsSubplotGroup)
    {
      double viewport[4];
      viewport[0] = static_cast<double>(element->getAttribute("viewport_xmin"));
      viewport[1] = static_cast<double>(element->getAttribute("viewport_xmax"));
      viewport[2] = static_cast<double>(element->getAttribute("viewport_ymin"));
      viewport[3] = static_cast<double>(element->getAttribute("viewport_ymax"));
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
  if (global_root->hasChildNodes())
    {
      for (const auto &child : global_root->children())
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
      (element->hasAttribute("subplotGroup") && static_cast<int>(element->getAttribute("subplotGroup")));

  if (element->localName() == "layout_gridelement" || elementIsSubplotGroup)
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
                                  std::shared_ptr<GRM::Element> subplot_element)
{
  double ndc_box_x[4], ndc_box_y[4], viewport[4], wswindow[4];
  double ndc_left, ndc_top, ndc_right, ndc_bottom;
  int width, height, max_width_height;

  get_figure_size(nullptr, &width, &height, nullptr, nullptr);
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
  viewport[0] = static_cast<double>(subplot_element->getAttribute("viewport_xmin"));
  viewport[1] = static_cast<double>(subplot_element->getAttribute("viewport_xmax"));
  viewport[2] = static_cast<double>(subplot_element->getAttribute("viewport_ymin"));
  viewport[3] = static_cast<double>(subplot_element->getAttribute("viewport_ymax"));
  wswindow[0] = static_cast<double>(subplot_element->parentElement()->getAttribute("wswindow_xmin"));
  wswindow[1] = static_cast<double>(subplot_element->parentElement()->getAttribute("wswindow_xmax"));
  wswindow[2] = static_cast<double>(subplot_element->parentElement()->getAttribute("wswindow_ymin"));
  wswindow[3] = static_cast<double>(subplot_element->parentElement()->getAttribute("wswindow_ymax"));

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

/*!
 * Set the text color either to black or white depending on the given background color.
 *
 * \param[in] r The red component of the RGB triple in the range [0.0, 1.0].
 * \param[in] g The green component of the RGB triple in the range [0.0, 1.0].
 * \param[in] b The blue component of the RGB triple in the range [0.0, 1.0].
 * \return The luminance of the given RGB triple.
 */
void set_text_color_for_background(double r, double g, double b, const std::shared_ptr<GRM::Element> &element)
{
  double color_lightness;

  color_lightness = get_lightness_from_rbg(r, g, b);
  if (color_lightness < 0.4)
    {
      global_render->setTextColorInd(element, 0);
    }
  else
    {
      global_render->setTextColorInd(element, 1);
    }
}

double auto_tick_rings_polar(double rmax, int &rings, const std::string &norm)
{
  double scale;
  bool decimal = false;

  std::vector<int> largeRings = {6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
  std::vector<int> normalRings = {3, 4, 5, 6, 7};

  std::vector<int> *whichVector;

  // -1 --> auto rings
  if (rings == -1)
    {
      if (norm == "cdf")
        {
          rings = 4;
          return 1.0 / rings;
        }

      if (rmax > 20)
        {
          whichVector = &largeRings;
        }
      else
        {
          whichVector = &normalRings;
        }
      scale = ceil(abs(log10(rmax)));
      if (rmax < 1.0)
        {
          decimal = true;
          rmax = static_cast<int>(ceil(rmax * pow(10.0, scale)));
        }

      while (true)
        {
          for (int i : *whichVector)
            {
              if (static_cast<int>(rmax) % i == 0)
                {
                  if (decimal)
                    {
                      rmax = rmax / pow(10.0, scale);
                    }
                  rings = i;
                  return rmax / rings;
                }
            }
          // rmax not divisible by whichVector
          ++rmax;
        }
    }

  // given rings
  if (norm == "cdf")
    {
      return 1.0 / rings;
    }

  if (rmax > rings)
    {
      return (static_cast<int>(rmax) + (rings - (static_cast<int>(rmax) % rings))) / rings;
    }
  else if (rmax > (rings * 0.6))
    {
      // returns rings / rings -> 1.0 so that rmax = rings * tick -> rings. Number of rings is rmax then
      return 1.0;
    }
  scale = ceil(abs(log10(rmax)));
  rmax = static_cast<int>(rmax * pow(10.0, scale));
  if (static_cast<int>(rmax) % rings == 0)
    {
      rmax = rmax / pow(10.0, scale);
      return rmax / rings;
    }
  rmax += rings - (static_cast<int>(rmax) % rings);
  rmax = rmax / pow(10.0, scale);

  return rmax / rings;
}
