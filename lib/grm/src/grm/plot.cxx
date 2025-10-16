/* ######################### includes ############################################################################### */

#include <grm/dom_render/graphics_tree/Element.hxx>
#include <grm/dom_render/graphics_tree/Comment.hxx>
#include <grm/dom_render/graphics_tree/util.hxx>
#include <grm/dom_render/render.hxx>

#include <algorithm>
#include <array>
#include <fstream>
#include <functional>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <variant>

#ifdef __unix__
#include <unistd.h>
#endif

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
#include "json_int.h"
#include "bson_int.h"
#include "logging_int.h"
}
#include "utilcpp_int.hxx"

#ifndef NO_XERCES_C
#include <xercesc/sax/InputSource.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>

#include <xercesc/parsers/SAX2XMLReaderImpl.hpp>

#include <xercesc/util/BinInputStream.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/util/TransService.hpp>

#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/framework/psvi/PSVIAttributeList.hpp>
#include <xercesc/framework/psvi/PSVIHandler.hpp>
#include <xercesc/framework/psvi/XSAttributeDeclaration.hpp>
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
static const std::string PRIVATE_SCHEMA_REL_FILEPATH = "share/xml/GRM/grm_graphics_tree_private_schema.xsd";
static const std::string FULL_SCHEMA_FILENAME = "grm_graphics_tree_full_schema.xsd";
static const std::string ENABLE_XML_VALIDATION_ENV_KEY = "GRM_VALIDATE";

/* ========================= datatypes ============================================================================== */

/* ------------------------- args set ------------------------------------------------------------------------------- */

DECLARE_SET_TYPE(Args, grm_args_t *)


/* ------------------------- string-to-plot_func map ---------------------------------------------------------------- */

DECLARE_MAP_TYPE(PlotFunc, PlotFunc)


/* ------------------------- string-to-args_set map ----------------------------------------------------------------- */

DECLARE_MAP_TYPE(ArgsSet, ArgsSet *)


#undef DECLARE_SET_TYPE
#undef DECLARE_MAP_TYPE


/* ========================= macros ================================================================================= */

/* ------------------------- args get ------------------------------------------------------------------------------- */

#define argsValueIteratorGet(value_it, length, array) \
  if ((value_it)->next(value_it) == nullptr)          \
    {                                                 \
      argsValueIteratorDelete(value_it);              \
      return GRM_ERROR_INTERNAL;                      \
    }                                                 \
  (length) = (value_it)->array_length;                \
  (array) = *(double **)(value_it)->value_ptr;

/* ========================= methods ================================================================================ */

/* ------------------------- args set ------------------------------------------------------------------------------- */

DECLARE_SET_METHODS(Args, args)


/* ------------------------- string-to-plot_func map ---------------------------------------------------------------- */

DECLARE_MAP_METHODS(PlotFunc, plotFunc)


/* ------------------------- string-to-args_set map ----------------------------------------------------------------- */

DECLARE_MAP_METHODS(ArgsSet, argsSet)


#undef DECLARE_SET_METHODS
#undef DECLARE_MAP_METHODS


/* ######################### internal implementation ################################################################ */

/* ========================= static variables ======================================================================= */

/* ------------------------- dump ----------------------------------------------------------------------------------- */

/*!
 * \brief List of attribute names in the graphics tree which will be exported as-is on XML dumps. Backup attributes
 * (`_*_org) are ignored for these attribute names.
 */
const static std::unordered_set<std::string_view> RESTORE_BACKUP_FORMAT_EXCLUDES = {
    "space_3d_phi",
    "space_3d_theta",
};

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
grm_grid_t *global_grid = nullptr;
static std::shared_ptr<GRM::Render> global_render;
static std::shared_ptr<GRM::Element> global_root;
static std::shared_ptr<GRM::Element> edit_figure;
static std::weak_ptr<GRM::Element> current_dom_element;
static std::weak_ptr<GRM::Element> current_central_region_element;
static int error_code = GRM_ERROR_NONE;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ event handling ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

EventQueue *event_queue = nullptr;


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ kind to fmt ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static StringMapEntry kind_to_fmt[] = {
    {"line", "xys"},
    {"hexbin", "xys"},
    {"polar_line", "thetars"},
    {"shade", "xys"},
    {"stem", "xys"},
    {"stairs", "xys"},
    {"contour", "xyzc"},
    {"contourf", "xyzc"},
    {"tricontour", "xyzc"},
    {"trisurface", "xyzc"},
    {"surface", "xyzc"},
    {"wireframe", "xyzc"},
    {"line3", "xyzc"},
    {"scatter", "xyzc"},
    {"scatter3", "xyzc"},
    {"quiver", "xyuv"},
    {"heatmap", "xyzc"},
    {"histogram", "x"},
    {"barplot", "y"},
    {"isosurface", "c"},
    {"imshow", "c"},
    {"nonuniform_heatmap", "xyzc"},
    {"polar_histogram", "theta"},
    {"pie", "x"},
    {"volume", "c"},
    {"marginal_heatmap", "xyzc"},
    {"polar_heatmap", "thetarzc"},
    {"nonuniform_polar_heatmap", "thetarzc"},
    {"polar_scatter", "thetars"},
};


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ kind to func ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static PlotFuncMapEntry kind_to_func[] = {{"line", plotLine},
                                          {"stairs", plotStairs},
                                          {"scatter", plotScatter},
                                          {"quiver", plotQuiver},
                                          {"stem", plotStem},
                                          {"histogram", plotHistogram},
                                          {"barplot", plotBarplot},
                                          {"contour", plotContour},
                                          {"contourf", plotContourf},
                                          {"hexbin", plotHexbin},
                                          {"heatmap", plotHeatmap},
                                          {"marginal_heatmap", plotMarginalHeatmap},
                                          {"wireframe", plotWireframe},
                                          {"surface", plotSurface},
                                          {"line3", plotLine3},
                                          {"scatter3", plotScatter3},
                                          {"imshow", plotImshow},
                                          {"isosurface", plotIsosurface},
                                          {"polar_line", plotPolarLine},
                                          {"polar_scatter", plotPolarScatter},
                                          {"trisurface", plotTrisurface},
                                          {"tricontour", plotTricontour},
                                          {"shade", plotShade},
                                          {"nonuniform_heatmap", plotHeatmap},
                                          {"nonuniform_polar_heatmap", plotPolarHeatmap},
                                          {"polar_histogram", plotPolarHistogram},
                                          {"polar_heatmap", plotPolarHeatmap},
                                          {"pie", plotPie},
                                          {"volume", plotVolume}};


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ maps ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static DoubleMap *meters_per_unit_map = nullptr;
static StringMap *fmt_map = nullptr;
static PlotFuncMap *plot_func_map = nullptr;
static StringMap *plot_valid_keys_map = nullptr;
static StringArrayMap *type_map = nullptr;


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plot clear ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

const char *plot_clear_exclude_keys[] = {"array_index", "in_use", nullptr};


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plot merge ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

const char *plot_merge_ignore_keys[] = {"id", "series_id", "subplot_id", "plot_id", "array_index", "in_use", nullptr};
const char *plot_merge_clear_keys[] = {"series", nullptr};


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ kind to func ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static const DoubleMapEntry SYMBOL_TO_METERS_PER_UNIT[] = {
    {"m", 1.0},     {"dm", 0.1},    {"cm", 0.01},  {"mm", 0.001},        {"in", 0.0254},
    {"\"", 0.0254}, {"ft", 0.3048}, {"'", 0.0254}, {"pc", 0.0254 / 6.0}, {"pt", 0.0254 / 72.0},
};


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ text encoding ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static int pre_plot_text_encoding = -1;


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ valid keys ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* IMPORTANT: Every key should only be part of ONE array -> keys can be assigned to the right object, if a user sends a
 * flat object that mixes keys of different hierarchies */

const char *valid_root_keys[] = {"plots", "append_plots", "hold_plots", nullptr};
const char *valid_plot_keys[] = {"clear", "raw", "size", "subplots", "update", nullptr};

const char *valid_subplot_keys[] = {"abs_height",
                                    "abs_width",
                                    "accelerate",
                                    "adjust_x_lim",
                                    "adjust_y_lim",
                                    "adjust_z_lim",
                                    "alpha",
                                    "angle_ticks",
                                    "aspect_ratio",
                                    "axes_mod",
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
                                    "keep_aspect_ratio",
                                    "keep_radii_axes",
                                    "kind",
                                    "levels",
                                    "location",
                                    "major_h",
                                    "normalization",
                                    "only_square_aspect_ratio",
                                    "orientation",
                                    "panzoom",
                                    "r_lim",
                                    "r_log",
                                    "rel_height",
                                    "rel_width",
                                    "resample_method",
                                    "reset_ranges",
                                    "rotation",
                                    "row",
                                    "row_span",
                                    "series",
                                    "style",
                                    "subplot",
                                    "theta_flip",
                                    "theta_lim",
                                    "theta_log",
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
                                    "y_bins",
                                    "y_flip",
                                    "y_grid",
                                    "y_label",
                                    "y_lim",
                                    "y_line_pos",
                                    "y_log",
                                    "y_ind",
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
                                   "border_width",
                                   "border_color_ind",
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
                                   "error_bar_style",
                                   "face_color",
                                   "fill_int_style",
                                   "fill_style",
                                   "foreground_color",
                                   "indices",
                                   "inner_series",
                                   "int_limits_high",
                                   "int_limits_low",
                                   "isovalue",
                                   "label",
                                   "labels",
                                   "line_color_ind",
                                   "line_spec",
                                   "line_type",
                                   "line_width",
                                   "marker_color_ind",
                                   "marker_size",
                                   "marker_type",
                                   "num_bins",
                                   "r",
                                   "ref_x_axis_location",
                                   "ref_y_axis_location",
                                   "rgb",
                                   "r_colormap",
                                   "r_range",
                                   "s",
                                   "step_where",
                                   "stairs",
                                   "theta",
                                   "theta_colormap",
                                   "theta_data_lim",
                                   "theta_range",
                                   "transparency",
                                   "u",
                                   "v",
                                   "weights",
                                   "x",
                                   "x_range",
                                   "y",
                                   "y_labels",
                                   "y_line_pos",
                                   "y_range",
                                   "z",
                                   "z_dims",
                                   "z_range",
                                   nullptr};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ valid types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* If multiple formats are supported use `|` as separator
 * Example: "i|s" for supporting both integer and strings */
/* TODO: type for format "s"? */
static StringMapEntry key_to_formats[] = {{"a", "A"},
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
                                          {"axes_mod", "a"},
                                          {"background_color", "i"},
                                          {"bar_color", "D|i"},
                                          {"border_color_ind", "i"},
                                          {"border_width", "d"},
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
                                          {"error_bar_style", "i"},
                                          {"fill_int_style", "i"},
                                          {"fill_style", "i"},
                                          {"fit_parents_height", "i"},
                                          {"fit_parents_width", "i"},
                                          {"font", "i"},
                                          {"font_precision", "i"},
                                          {"foreground_color", "D"},
                                          {"grid_element", "s"},
                                          {"grplot", "i"},
                                          {"hold_plots", "i"},
                                          {"int_limits_high", "D"},
                                          {"int_limits_low", "D"},
                                          {"isovalue", "d"},
                                          {"keep_aspect_ratio", "i"},
                                          {"kind", "s"},
                                          {"label", "s"},
                                          {"labels", "S"},
                                          {"levels", "i"},
                                          {"line_color_ind", "i"},
                                          {"line_spec", "s"},
                                          {"line_type", "i"},
                                          {"line_width", "d"},
                                          {"location", "i"},
                                          {"marginal_heatmap_kind", "s"},
                                          {"marker_color_ind", "i"},
                                          {"marker_size", "d"},
                                          {"marker_type", "i|D"},
                                          {"num_bins", "i"},
                                          {"only_square_aspect_ratio", "i"},
                                          {"orientation", "s"},
                                          {"panzoom", "D"},
                                          {"r", "D|I"},
                                          {"r_colormap", "i"},
                                          {"r_lim", "D"},
                                          {"r_range", "D"},
                                          {"raw", "s"},
                                          {"ref_x_axis_location", "s"},
                                          {"ref_y_axis_location", "s"},
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
                                          {"theta", "D|I"},
                                          {"theta_colormap", "i"},
                                          {"theta_lim", "D"},
                                          {"theta_range", "D"},
                                          {"tilt", "d"},
                                          {"title", "s"},
                                          {"transformation", "i"},
                                          {"transparency", "d"},
                                          {"u", "D"},
                                          {"update", "i"},
                                          {"v", "D"},
                                          {"x", "D|I"},
                                          {"x_bins", "i"},
                                          {"x_flip", "i"},
                                          {"x_grid", "i"},
                                          {"x_label", "s"},
                                          {"x_lim", "D"},
                                          {"x_log", "i"},
                                          {"x_ind", "i"},
                                          {"x_range", "D"},
                                          {"y", "D"},
                                          {"y_bins", "i"},
                                          {"y_flip", "i"},
                                          {"y_form", "i"},
                                          {"y_grid", "i"},
                                          {"y_label", "s"},
                                          {"y_lim", "D"},
                                          {"y_line_pos", "d"},
                                          {"y_log", "i"},
                                          {"y_ind", "i"},
                                          {"y_range", "D"},
                                          {"z", "D"},
                                          {"z_dims", "I"},
                                          {"z_flip", "i"},
                                          {"z_grid", "i"},
                                          {"z_label", "s"},
                                          {"z_lim", "D"},
                                          {"z_range", "D"},
                                          {"z_log", "i"}};

/* ------------------------- util ----------------------------------------------------------------------------------- */

static const char *grm_tmp_dir = NULL;


/* ========================= functions ============================================================================== */

/* ------------------------- plot ----------------------------------------------------------------------------------- */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ general ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

grm_error_t plotInitStaticVariables(void)
{
  grm_error_t error = GRM_ERROR_NONE;

  if (!plot_static_variables_initialized)
    {
      logger((stderr, "Initializing static plot variables\n"));
      event_queue = eventQueueNew();
      global_root_args = grm_args_new();
      errorCleanupAndSetErrorIf(global_root_args == nullptr, GRM_ERROR_MALLOC);
      error = plotInitArgsStructure(global_root_args, plot_hierarchy_names, 1);
      errorCleanupIfError;
      plotSetFlagDefaults();
      errorCleanupAndSetErrorIf(!grm_args_values(global_root_args, "plots", "a", &active_plot_args),
                                GRM_ERROR_INTERNAL);
      active_plot_index = 1;
      /* initialize global_render and its root */
      global_render = GRM::Render::createRender();
      global_root = global_render->createElement("root");
      global_render->replaceChildren(global_root);
      global_root->setAttribute("_id", 0);
      global_render->setAutoUpdate(false);

      meters_per_unit_map = doubleMapNewWithData(arraySize(SYMBOL_TO_METERS_PER_UNIT), SYMBOL_TO_METERS_PER_UNIT);
      errorCleanupAndSetErrorIf(meters_per_unit_map == nullptr, GRM_ERROR_MALLOC);
      fmt_map = stringMapNewWithData(arraySize(kind_to_fmt), kind_to_fmt);
      errorCleanupAndSetErrorIf(fmt_map == nullptr, GRM_ERROR_MALLOC);
      plot_func_map = plotFuncMapNewWithData(arraySize(kind_to_func), kind_to_func);
      errorCleanupAndSetErrorIf(plot_func_map == nullptr, GRM_ERROR_MALLOC);
      {
        const char **hierarchy_keys[] = {valid_root_keys, valid_plot_keys, valid_subplot_keys, valid_series_keys,
                                         nullptr};
        const char **hierarchy_names_ptr, ***hierarchy_keys_ptr, **current_key_ptr;
        plot_valid_keys_map = stringMapNew(arraySize(valid_root_keys) + arraySize(valid_plot_keys) +
                                           arraySize(valid_subplot_keys) + arraySize(valid_series_keys));
        errorCleanupAndSetErrorIf(plot_valid_keys_map == nullptr, GRM_ERROR_MALLOC);
        hierarchy_keys_ptr = hierarchy_keys;
        hierarchy_names_ptr = plot_hierarchy_names;
        while (*hierarchy_names_ptr != nullptr && *hierarchy_keys_ptr != nullptr)
          {
            current_key_ptr = *hierarchy_keys_ptr;
            while (*current_key_ptr != nullptr)
              {
                stringMapInsert(plot_valid_keys_map, *current_key_ptr, *hierarchy_names_ptr);
                ++current_key_ptr;
              }
            ++hierarchy_names_ptr;
            ++hierarchy_keys_ptr;
          }
      }
      type_map = stringArrayMapNewFromStringSplit(arraySize(key_to_formats), key_to_formats, '|');
      errorCleanupAndSetErrorIf(type_map == nullptr, GRM_ERROR_MALLOC);
      grm_tmp_dir = createTmpDir();
      errorCleanupAndSetErrorIf(grm_tmp_dir == nullptr, GRM_ERROR_TMP_DIR_CREATION);
      installBacktraceHandlerIfEnabled();
      plot_static_variables_initialized = 1;
    }
  return GRM_ERROR_NONE;

error_cleanup:
  if (global_root_args != nullptr)
    {
      grm_args_delete(global_root_args);
      global_root_args = nullptr;
    }
  if (meters_per_unit_map != nullptr)
    {
      doubleMapDelete(meters_per_unit_map);
      meters_per_unit_map = nullptr;
    }
  if (fmt_map != nullptr)
    {
      stringMapDelete(fmt_map);
      fmt_map = nullptr;
    }
  if (plot_func_map != nullptr)
    {
      plotFuncMapDelete(plot_func_map);
      plot_func_map = nullptr;
    }
  if (plot_valid_keys_map != nullptr)
    {
      stringMapDelete(plot_valid_keys_map);
      plot_valid_keys_map = nullptr;
    }
  if (type_map != nullptr)
    {
      stringArrayMapDelete(type_map);
      type_map = nullptr;
    }
  error_code = error;
  return error;
}

static std::shared_ptr<GRM::Element> getCentralRegion()
{
  auto plot_parent = edit_figure->lastChildElement();
  plot_parent = plot_parent->querySelectors("plot");
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

grm_error_t plotMergeArgs(grm_args_t *args, const grm_args_t *merge_args, const char **hierarchy_name_ptr,
                          UintMap *hierarchy_to_id, int hold_always)
{
  static ArgsSetMap *key_to_cleared_args = nullptr;
  static int recursion_level = -1;
  int plot_id = 0, subplot_id = 0, series_id = 0;
  int append_plots;
  grm_args_iterator_t *merge_it = nullptr;
  grm_arg_t *arg, *merge_arg;
  grm_args_value_iterator_t *value_it = nullptr, *merge_value_it = nullptr;
  const char **current_hierarchy_name_ptr;
  grm_args_t **args_array, **merge_args_array, *current_args;
  unsigned int i;
  grm_error_t error = GRM_ERROR_NONE;

  ++recursion_level;
  if (hierarchy_name_ptr == nullptr) hierarchy_name_ptr = plot_hierarchy_names;
  if (hierarchy_to_id == nullptr)
    {
      hierarchy_to_id = uintMapNew(arraySize(plot_hierarchy_names));
      cleanupAndSetErrorIf(hierarchy_to_id == nullptr, GRM_ERROR_MALLOC);
    }
  else
    {
      hierarchy_to_id = uintMapCopy(hierarchy_to_id);
      cleanupAndSetErrorIf(hierarchy_to_id == nullptr, GRM_ERROR_MALLOC);
    }
  if (key_to_cleared_args == nullptr)
    {
      key_to_cleared_args = argsSetMapNew(arraySize(plot_merge_clear_keys));
      cleanupAndSetErrorIf(hierarchy_to_id == nullptr, GRM_ERROR_MALLOC);
    }
  if (!(recursion_level == 0 && grm_args_values(merge_args, "append_plots", "i", &append_plots)))
    {
      grm_args_values(global_root_args, "append_plots", "i", &append_plots);
    }
  getIdFromArgs(merge_args, &plot_id, &subplot_id, &series_id);
  if (plot_id > 0)
    {
      uintMapInsert(hierarchy_to_id, "plots", plot_id);
    }
  else
    {
      uintMapInsertDefault(hierarchy_to_id, "plots", append_plots ? 0 : active_plot_index);
      uintMapAt(hierarchy_to_id, "plots", (unsigned int *)&plot_id);
      logger((stderr, "Using plot_id \"%u\"\n", plot_id));
    }
  if (subplot_id > 0)
    {
      uintMapInsert(hierarchy_to_id, "subplots", subplot_id);
    }
  else
    {
      uintMapInsertDefault(hierarchy_to_id, "subplots", 1);
      uintMapAt(hierarchy_to_id, "subplots", (unsigned int *)&subplot_id);
    }
  if (series_id > 0)
    {
      uintMapInsert(hierarchy_to_id, "series", series_id);
    }
  else
    {
      uintMapInsertDefault(hierarchy_to_id, "series", 1);
      uintMapAt(hierarchy_to_id, "series", (unsigned int *)&series_id);
    }
  /* special case: clear the plot container before usage if
   * - it is the first call of `plotMergeArgs` AND `hold_always` is `0` AND
   *   - `hold_plots` is `0` OR
   *   - `hold_plots` is not set AND `plot_id` is `1`
   */
  if (strcmp(*hierarchy_name_ptr, "figure") == 0 && !hold_always)
    {
      int hold_plots_key_available, hold_plots;
      hold_plots_key_available = grm_args_values(merge_args, "hold_plots", "i", &hold_plots) ||
                                 grm_args_values(args, "hold_plots", "i", &hold_plots);
#ifndef NDEBUG
      if (hold_plots_key_available)
        {
          logger((stderr, "Do%s hold plots\n", hold_plots ? "" : " not"));
          if (grm_args_contains(merge_args, "hold_plots"))
            logger((stderr, "\"hold_plots\" key read from \"merge_args\"\n"));
          else
            logger((stderr, "\"hold_plots\" key read from \"args\"\n"));
        }
#endif
      if ((hold_plots_key_available && !hold_plots) || (!hold_plots_key_available && plot_id == 1))
        {
          cleanupAndSetErrorIf(!grm_args_values(args, "plots", "A", &args_array), GRM_ERROR_INTERNAL);
          current_args = args_array[plot_id - 1];
          grm_args_clear(current_args);
          error = plotInitArgsStructure(current_args, hierarchy_name_ptr + 1, 1);
          cleanupIfError;
          logger((stderr, "Cleared current args\n"));
        }
      else
        {
          logger((stderr, "Held current args\n"));
        }
    }
#ifndef NDEBUG
  if (strcmp(*hierarchy_name_ptr, "figure") == 0 && hold_always) logger((stderr, "\"hold_always\" is set\n"));
#endif /* ifndef  */
  merge_it = grm_args_iter(merge_args);
  cleanupAndSetErrorIf(merge_it == nullptr, GRM_ERROR_MALLOC);
  while ((merge_arg = merge_it->next(merge_it)) != nullptr)
    {
      if (strEqualsAnyInArray(merge_arg->key, plot_merge_ignore_keys)) continue;
      /* First, find the correct hierarchy level where the current merge value belongs to. */
      error = plotGetArgsInHierarchy(args, hierarchy_name_ptr, merge_arg->key, hierarchy_to_id,
                                     (const grm_args_t **)&current_args, &current_hierarchy_name_ptr);
      if (error == GRM_ERROR_PLOT_UNKNOWN_KEY)
        {
          logger((stderr, "WARNING: The key \"%s\" is not assigned to any hierarchy level.\n", merge_arg->key));
        }
      cleanupIfError;
      if (strEqualsAnyInArray(*current_hierarchy_name_ptr, plot_merge_clear_keys))
        {
          int clear_args = 1;
          ArgsSet *cleared_args = nullptr;
          if (argsSetMapAt(key_to_cleared_args, *current_hierarchy_name_ptr, &cleared_args))
            {
              clear_args = !argsSetContains(cleared_args, current_args);
            }
          if (clear_args)
            {
              logger((stderr, "Perform a clear on the current args container\n"));
              grm_args_clear(current_args);
              if (cleared_args == nullptr)
                {
                  cleared_args = argsSetNew(100); /* FIXME: do not use a magic number, use a growable set instead! */
                  cleanupAndSetErrorIf(cleared_args == nullptr, GRM_ERROR_MALLOC);
                  cleanupAndSetErrorIf(
                      !argsSetMapInsert(key_to_cleared_args, *current_hierarchy_name_ptr, cleared_args),
                      GRM_ERROR_INTERNAL);
                }
              logger((stderr, "Add args container \"%p\" to cleared args with key \"%s\"\n", (void *)current_args,
                      *current_hierarchy_name_ptr));
              cleanupAndSetErrorIf(!argsSetAdd(cleared_args, current_args), GRM_ERROR_INTERNAL);
            }
        }
      /* If the current key is a hierarchy key, perform a merge. Otherwise, (else branch) put the value in without a
       * merge.
       */
      if (current_hierarchy_name_ptr[1] != nullptr && strcmp(merge_arg->key, current_hierarchy_name_ptr[1]) == 0)
        {
          /* `argsAt` cannot fail in this case because the `args` object was initialized with an empty structure
           * before. If `arg` is nullptr, an internal error occurred. */
          arg = argsAt(current_args, merge_arg->key);
          cleanupAndSetErrorIf(arg == nullptr, GRM_ERROR_INTERNAL);
          value_it = grm_arg_value_iter(arg);
          merge_value_it = grm_arg_value_iter(merge_arg);
          cleanupAndSetErrorIf(value_it == nullptr, GRM_ERROR_MALLOC);
          cleanupAndSetErrorIf(merge_value_it == nullptr, GRM_ERROR_MALLOC);
          /* Do not support two-dimensional argument arrays like `nAnA`) -> a loop would be needed with more memory
           * management */
          cleanupAndSetErrorIf(value_it->next(value_it) == nullptr, GRM_ERROR_MALLOC);
          cleanupAndSetErrorIf(merge_value_it->next(merge_value_it) == nullptr, GRM_ERROR_MALLOC);
          /* Increase the array size of the internal args array if necessary */
          if (merge_value_it->array_length > value_it->array_length)
            {
              error = plotInitArgStructure(arg, current_hierarchy_name_ptr, merge_value_it->array_length);
              cleanupIfError;
              argsValueIteratorDelete(value_it);
              value_it = grm_arg_value_iter(arg);
              cleanupAndSetErrorIf(value_it == nullptr, GRM_ERROR_MALLOC);
              cleanupAndSetErrorIf(value_it->next(value_it) == nullptr, GRM_ERROR_MALLOC);
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
              error = plotMergeArgs(args_array[i], merge_args_array[i], current_hierarchy_name_ptr + 1, hierarchy_to_id,
                                    hold_always);
              cleanupIfError;
            }
        }
      else
        {
          const char *compatible_format;
          /* Only accept the new value, if it has a compatible type. Convert if possible */
          if ((compatible_format = getCompatibleFormat(merge_arg->key, merge_arg->value_format)) != nullptr)
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
                  copy_buffer = copyValue(tolower(*compatible_format), merge_arg->value_ptr);
                  cleanupAndSetErrorIf(copy_buffer == nullptr, GRM_ERROR_MALLOC);
                  grm_args_push(current_args, merge_arg->key, array_format, 1, copy_buffer);
                  /* x -> copy_buffer -> value, value is now stored and the buffer is not needed any more */
                  free(copy_buffer);
                }
              else
                {
                  /* Otherwise, push without conversion (needed conversion is done later when extracting values with
                   * functions like `grm_args_values`) */
                  error = argsPushArg(current_args, merge_arg);
                  cleanupIfError;
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
          ArgsSet *cleared_args = nullptr;
          const char **current_key_ptr = plot_merge_clear_keys;
          while (*current_key_ptr != nullptr)
            {
              if (argsSetMapAt(key_to_cleared_args, *current_key_ptr, &cleared_args)) argsSetDelete(cleared_args);
              ++current_key_ptr;
            }
          argsSetMapDelete(key_to_cleared_args);
          key_to_cleared_args = nullptr;
        }
    }
  if (hierarchy_to_id != nullptr)
    {
      uintMapDelete(hierarchy_to_id);
      hierarchy_to_id = nullptr;
    }
  if (merge_it != nullptr) argsIteratorDelete(merge_it);
  if (value_it != nullptr) argsValueIteratorDelete(value_it);
  if (merge_value_it != nullptr) argsValueIteratorDelete(merge_value_it);

  --recursion_level;

  error_code = error;
  return error;
}

grm_error_t plotInitArgStructure(grm_arg_t *arg, const char **hierarchy_name_ptr,
                                 unsigned int next_hierarchy_level_max_id)
{
  grm_args_t **args_array = nullptr;
  unsigned int args_old_array_length;
  unsigned int i;
  grm_error_t error = GRM_ERROR_NONE;

  logger((stderr, "Init plot args structure for hierarchy: \"%s\"\n", *hierarchy_name_ptr));

  ++hierarchy_name_ptr;
  if (*hierarchy_name_ptr == nullptr) return GRM_ERROR_NONE;
  argFirstValue(arg, "A", nullptr, &args_old_array_length);
  if (next_hierarchy_level_max_id <= args_old_array_length) return GRM_ERROR_NONE;
  logger((stderr, "Increase array for key \"%s\" from %d to %d\n", *hierarchy_name_ptr, args_old_array_length,
          next_hierarchy_level_max_id));
  error = argIncreaseArray(arg, next_hierarchy_level_max_id - args_old_array_length);
  error_code = error;
  returnIfError;
  argValues(arg, "A", &args_array);
  for (i = args_old_array_length; i < next_hierarchy_level_max_id; ++i)
    {
      args_array[i] = grm_args_new();
      grm_args_push(args_array[i], "array_index", "i", i);

      if (args_array[i] == nullptr)
        {
          error = GRM_ERROR_MALLOC;
          error_code = error;
          returnIfError;
        }
      error = plotInitArgsStructure(args_array[i], hierarchy_name_ptr, 1);
      error_code = error;
      returnIfError;
      if (strcmp(*hierarchy_name_ptr, "plots") == 0) grm_args_push(args_array[i], "in_use", "i", 0);
    }

  return GRM_ERROR_NONE;
}

grm_error_t plotInitArgsStructure(grm_args_t *args, const char **hierarchy_name_ptr,
                                  unsigned int next_hierarchy_level_max_id)
{
  grm_arg_t *arg = nullptr;
  grm_args_t **args_array = nullptr;
  unsigned int i;
  grm_error_t error = GRM_ERROR_NONE;

  logger((stderr, "Init plot args structure for hierarchy: \"%s\"\n", *hierarchy_name_ptr));

  ++hierarchy_name_ptr;
  if (*hierarchy_name_ptr == nullptr) return GRM_ERROR_NONE;
  arg = argsAt(args, *hierarchy_name_ptr);
  if (arg == nullptr)
    {
      args_array = static_cast<grm_args_t **>(calloc(next_hierarchy_level_max_id, sizeof(grm_args_t *)));
      errorCleanupAndSetErrorIf(args_array == nullptr, GRM_ERROR_MALLOC);
      for (i = 0; i < next_hierarchy_level_max_id; ++i)
        {
          args_array[i] = grm_args_new();
          grm_args_push(args_array[i], "array_index", "i", i);
          errorCleanupAndSetErrorIf(args_array[i] == nullptr, GRM_ERROR_MALLOC);
          error = plotInitArgsStructure(args_array[i], hierarchy_name_ptr, 1);
          errorCleanupIfError;
          if (strcmp(*hierarchy_name_ptr, "plots") == 0) grm_args_push(args_array[i], "in_use", "i", 0);
        }
      errorCleanupIf(!grm_args_push(args, *hierarchy_name_ptr, "nA", next_hierarchy_level_max_id, args_array));
      free(args_array);
      args_array = nullptr;
    }
  else
    {
      error = plotInitArgStructure(arg, hierarchy_name_ptr - 1, next_hierarchy_level_max_id);
      errorCleanupIfError;
    }

  return GRM_ERROR_NONE;

error_cleanup:
  if (args_array != nullptr)
    {
      for (i = 0; i < next_hierarchy_level_max_id; ++i)
        {
          if (args_array[i] != nullptr) grm_args_delete(args_array[i]);
        }
      free(args_array);
    }

  error_code = error;
  return error;
}

int plotCheckForRequest(const grm_args_t *args, grm_error_t *error)
{
  const char *request;
  int is_request = 0;

  *error = GRM_ERROR_NONE;
  if (grm_args_values(args, "request", "s", &request))
    {
      is_request = 1;
      *error = eventQueueEnqueueRequestEvent(event_queue, request);
    }
  else
    {
      *error = GRM_ERROR_PLOT_INVALID_REQUEST;
      error_code = GRM_ERROR_PLOT_INVALID_REQUEST;
    }

  return is_request;
}

void plotSetFlagDefaults(void)
{
  /* Use a standalone function for initializing flags instead of `plotSetAttributeDefaults` to guarantee the flags
   * are already set before `grm_plot` is called (important for `grm_merge`) */

  logger((stderr, "Set global flag defaults\n"));

  argsSetDefault(global_root_args, "append_plots", "i", ROOT_DEFAULT_APPEND_PLOTS);
}

void plotSetAttributeDefaults(grm_args_t *plot_args)
{
  grm_args_t **current_subplot;

  logger((stderr, "Set plot attribute defaults\n"));

  argsSetDefault(plot_args, "size", "dd", PLOT_DEFAULT_WIDTH, PLOT_DEFAULT_HEIGHT);

  grm_args_values(plot_args, "subplots", "A", &current_subplot);
  while (*current_subplot != nullptr)
    {
      argsSetDefault(*current_subplot, "kind", "s", PLOT_DEFAULT_KIND);
      argsSetDefault(*current_subplot, "x_grid", "i", PLOT_DEFAULT_XGRID); // This arg is only used in plot.cxx
      argsSetDefault(*current_subplot, "y_grid", "i", PLOT_DEFAULT_YGRID); // This arg is only used in plot.cxx
      argsSetDefault(*current_subplot, "z_grid", "i", PLOT_DEFAULT_ZGRID); // This arg is only used in plot.cxx

      ++current_subplot;
    }
}

void plotPrePlot(grm_args_t *plot_args)
{
  int clear;
  int previous_pixel_width, previous_pixel_height;

  logger((stderr, "Pre plot processing\n"));

  plotSetTextEncoding();
  if (grm_args_values(plot_args, "clear", "i", &clear))
    {
      logger((stderr, "Got keyword \"clear\" with value %d\n", clear));
      global_root->setAttribute("_clear_ws", clear);
    }

  if (grm_args_values(plot_args, "previous_pixel_size", "ii", &previous_pixel_width, &previous_pixel_height))
    {
      edit_figure->setAttribute("_previous_pixel_width", previous_pixel_width);
      edit_figure->setAttribute("_previous_pixel_height", previous_pixel_height);
    }
}

void plotSetTextEncoding(void)
{
  global_render->setTextEncoding(edit_figure, ENCODING_UTF8);
}

grm_error_t plotPreSubplot(grm_args_t *subplot_args)
{
  const char *kind;
  double alpha;
  grm_error_t error = GRM_ERROR_NONE;
  auto group = (!current_dom_element.expired()) ? current_dom_element.lock() : edit_figure->lastChildElement();

  logger((stderr, "Pre subplot processing\n"));

  grm_args_values(subplot_args, "kind", "s", &kind);
  if (strcmp(kind, "hist") == 0)
    {
      kind = "histogram";
      grm_args_push(subplot_args, "kind", "s", kind);
    }
  else if (strcmp(kind, "plot3") == 0)
    {
      kind = "line3";
      grm_args_push(subplot_args, "kind", "s", kind);
    }
  logger((stderr, "Got keyword \"kind\" with value \"%s\"\n", kind));
  if (!strEqualsAny(kind, "nonuniform_polar_heatmap", "polar_heatmap", "polar_histogram", "polar_line", "polar_scatter",
                    "wireframe", "surface", "line3", "scatter3", "trisurface", "volume", "isosurface", "barplot",
                    "contour", "contourf", "heatmap", "hexbin", "histogram", "line", "quiver", "scatter", "shade",
                    "stairs", "stem", "marginal_heatmap", "imshow", "tricontour", "pie"))
    {
      error = GRM_ERROR_PLOT_UNKNOWN_KIND;
      error_code = error;
      returnIfError;
    }
  error = plotStoreCoordinateRanges(subplot_args);
  error_code = error;
  returnIfError;
  plotProcessWindow(subplot_args);

  plotProcessColormap(subplot_args);
  plotProcessFont(subplot_args);
  plotProcessResampleMethod(subplot_args);

  if (strEqualsAny(kind, "polar_line", "polar_scatter", "polar_histogram"))
    {
      plotDrawPolarAxes(subplot_args);
    }
  else if (!strEqualsAny(kind, "pie", "polar_heatmap", "nonuniform_polar_heatmap"))
    {
      plotDrawAxes(subplot_args, 1);
    }

  return GRM_ERROR_NONE;
}

void plotProcessColormap(grm_args_t *subplot_args)
{
  int colormap;
  auto group = edit_figure->lastChildElement();

  if (grm_args_values(subplot_args, "colormap", "i", &colormap)) group->setAttribute("colormap", colormap);
}

void plotProcessFont(grm_args_t *subplot_args)
{
  int font, font_precision;
  auto group = edit_figure->lastChildElement();

  if (grm_args_values(subplot_args, "font", "i", &font)) group->setAttribute("font", font);
  if (grm_args_values(subplot_args, "font_precision", "i", &font_precision))
    group->setAttribute("font_precision", font_precision);
}

grm_error_t plotProcessGridArguments(const grm_args_t *args)
{
  int current_nesting_degree, nesting_degree;
  int *rows, *cols;
  unsigned int rows_length, cols_length;
  int row_span, col_span;
  int *row_spans, *col_spans;
  unsigned int row_spans_length, col_spans_length;
  int rowstart, rowstop, colstart, colstop;
  grm_args_t **current_subplot_args;
  grm_grid_t *current_grid;
  grm_element_t *current_element;

  double *rel_heights, *rel_widths, *abs_heights, *abs_widths, *aspect_ratios;
  int *fit_parents_heights, *fit_parents_widths;
  unsigned int rel_heights_length, rel_widths_length, abs_heights_length, abs_widths_length, aspect_ratios_length,
      fit_parents_heights_length, fit_parents_widths_length;
  grm_error_t error = GRM_ERROR_NONE;

  if (global_grid != nullptr) grm_grid_delete(global_grid);
  error = grm_grid_new(1, 1, &global_grid);
  error_code = error;
  returnIfError;
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
          error_code = GRM_ERROR_LAYOUT_COMPONENT_LENGTH_MISMATCH;
          return GRM_ERROR_LAYOUT_COMPONENT_LENGTH_MISMATCH;
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

          if (rowstart - rowstop == 0 || colstart - colstop == 0) break;

          if (nesting_degree == current_nesting_degree)
            {
              error = grm_grid_set_element_args_slice(rowstart, rowstop, colstart, colstop, *current_subplot_args,
                                                      current_grid);
              error_code = error;
              returnIfError;
              error = grm_grid_get_element(rowstart, colstart, current_grid, &current_element);
              error_code = error;
              returnIfError;
            }
          else
            {
              error = grm_grid_ensure_cells_are_grid(rowstart, rowstop, colstart, colstop, current_grid);
              error_code = error;
              returnIfError;
              error = grm_grid_get_element(rowstart, colstart, current_grid, (grm_element_t **)&current_grid);
              error_code = error;
              returnIfError;
              current_element = (grm_element_t *)current_grid;
            }

          if (rel_heights != nullptr && rel_heights_length > current_nesting_degree &&
              rel_heights[current_nesting_degree] != -1)
            {
              error = grm_element_set_relative_height(current_element, rel_heights[current_nesting_degree]);
              error_code = error;
            }
          if (rel_widths != nullptr && rel_widths_length > current_nesting_degree &&
              rel_widths[current_nesting_degree] != -1)
            {
              error = grm_element_set_relative_width(current_element, rel_widths[current_nesting_degree]);
              error_code = error;
            }
          if (abs_heights != nullptr && abs_heights_length > current_nesting_degree &&
              abs_heights[current_nesting_degree] != -1)
            {
              error = grm_element_set_abs_height(current_element, abs_heights[current_nesting_degree]);
              error_code = error;
            }
          if (abs_widths != nullptr && abs_widths_length > current_nesting_degree &&
              abs_widths[current_nesting_degree] != -1)
            {
              error = grm_element_set_abs_width(current_element, abs_widths[current_nesting_degree]);
              error_code = error;
            }
          if (aspect_ratios != nullptr && aspect_ratios_length > current_nesting_degree &&
              aspect_ratios[current_nesting_degree] != -1)
            {
              error = grm_element_set_aspect_ratio(current_element, aspect_ratios[current_nesting_degree]);
              error_code = error;
            }
          if (fit_parents_heights != nullptr && fit_parents_heights_length > current_nesting_degree &&
              fit_parents_heights[current_nesting_degree] != -1)
            {
              grm_element_set_fit_parents_height(current_element, fit_parents_heights[current_nesting_degree]);
            }
          if (fit_parents_widths != nullptr && fit_parents_widths_length > current_nesting_degree &&
              fit_parents_widths[current_nesting_degree] != -1)
            {
              grm_element_set_fit_parents_width(current_element, fit_parents_widths[current_nesting_degree]);
            }
          returnIfError;
        }
      ++current_subplot_args;
    }

  return GRM_ERROR_NONE;
}

void plotProcessResampleMethod(grm_args_t *subplot_args)
{
  int resample_method_flag;
  auto group = edit_figure->lastChildElement();
  auto central_region =
      (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();

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

void plotProcessWindow(grm_args_t *subplot_args)
{
  int scale = 0;
  const char *kind, *orientation = PLOT_DEFAULT_ORIENTATION;
  int x_log, y_log, z_log, theta_log, r_log;
  int x_flip, y_flip, z_flip;
  double rotation, tilt;

  auto group = edit_figure->lastChildElement();
  auto central_region =
      (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();

  grm_args_values(subplot_args, "kind", "s", &kind);
  if (strcmp(kind, "hist") == 0)
    {
      kind = "histogram";
      grm_args_push(subplot_args, "kind", "s", kind);
    }
  else if (strcmp(kind, "plot3") == 0)
    {
      kind = "line3";
      grm_args_push(subplot_args, "kind", "s", kind);
    }
  auto plot = strcmp(kind, "marginal_heatmap") != 0 ? central_region->parentElement()
                                                    : central_region->parentElement()->parentElement();
  if (strEqualsAny(kind, "polar_line", "polar_scatter", "polar_heatmap", "polar_histogram", "nonuniform_polar_heatmap"))
    {
      if (grm_args_values(subplot_args, "theta_log", "i", &theta_log)) plot->setAttribute("theta_log", theta_log);
      if (grm_args_values(subplot_args, "r_log", "i", &r_log)) plot->setAttribute("r_log", r_log);
    }
  else
    {
      if (grm_args_values(subplot_args, "x_log", "i", &x_log)) plot->setAttribute("x_log", x_log);
      if (grm_args_values(subplot_args, "y_log", "i", &y_log)) plot->setAttribute("y_log", y_log);
    }
  if (grm_args_values(subplot_args, "z_log", "i", &z_log)) plot->setAttribute("z_log", z_log);
  if (grm_args_values(subplot_args, "x_flip", "i", &x_flip)) plot->setAttribute("x_flip", x_flip);
  if (grm_args_values(subplot_args, "y_flip", "i", &y_flip)) plot->setAttribute("y_flip", y_flip);
  if (grm_args_values(subplot_args, "z_flip", "i", &z_flip)) plot->setAttribute("z_flip", z_flip);

  if (strEqualsAny(kind, "wireframe", "surface", "line3", "scatter3", "trisurface", "volume"))
    {
      plot->setAttribute("adjust_z_lim", true);
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

  if (grm_args_values(subplot_args, "orientation", "s", &orientation))
    central_region->setAttribute("orientation", orientation);
  if (grm_args_values(subplot_args, "scale", "i", scale)) global_render->setScale(plot, scale);
}

grm_error_t plotStoreCoordinateRanges(grm_args_t *subplot_args)
{
  const char *kind;
  grm_error_t error = GRM_ERROR_NONE;
  double x_min, x_max, y_min, y_max, z_min, z_max, c_min, c_max, theta_min, theta_max, r_min, r_max;

  auto group = (!current_dom_element.expired()) ? current_dom_element.lock() : edit_figure->lastChildElement();

  if (grm_args_contains(subplot_args, "_original_x_lim")) group->setAttribute("original_x_lim", true);

  grm_args_values(subplot_args, "kind", "s", &kind);
  if (strcmp(kind, "hist") == 0)
    {
      kind = "histogram";
      grm_args_push(subplot_args, "kind", "s", kind);
    }
  else if (strcmp(kind, "plot3") == 0)
    {
      kind = "line3";
      grm_args_push(subplot_args, "kind", "s", kind);
    }
  group->setAttribute("_kind", kind);

  if (grm_args_values(subplot_args, "c_lim", "dd", &c_min, &c_max))
    {
      group->setAttribute("c_lim_min", c_min);
      group->setAttribute("c_lim_max", c_max);
    }
  if (strEqualsAny(kind, "polar_line", "polar_scatter", "polar_heatmap", "polar_histogram", "nonuniform_polar_heatmap"))
    {
      if (grm_args_values(subplot_args, "theta_lim", "dd", &theta_min, &theta_max))
        {
          group->setAttribute("theta_lim_min", theta_min);
          group->setAttribute("theta_lim_max", theta_max);
        }
      if (grm_args_values(subplot_args, "r_lim", "dd", &r_min, &r_max))
        {
          group->setAttribute("r_lim_min", r_min);
          group->setAttribute("r_lim_max", r_max);
        }
    }
  else
    {
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
    }

  if (grm_args_values(subplot_args, "z_lim", "dd", &z_min, &z_max))
    {
      group->setAttribute("z_lim_min", z_min);
      group->setAttribute("z_lim_max", z_max);
    }

  return error;
}

void plotPostPlot(grm_args_t *plot_args)
{
  int update;

  logger((stderr, "Post plot processing\n"));

  if (grm_args_values(plot_args, "update", "i", &update))
    {
      logger((stderr, "Got keyword \"update\" with value %d\n", update));
      global_root->setAttribute("_update_ws", update);
    }
}

void plotPostSubplot(grm_args_t *subplot_args)
{
  const char *kind;

  logger((stderr, "Post subplot processing\n"));

  grm_args_values(subplot_args, "kind", "s", &kind);
  logger((stderr, "Got keyword \"kind\" with value \"%s\"\n", kind));
  if (strcmp(kind, "barplot") == 0)
    {
      plotDrawAxes(subplot_args, 2);
    }
  else if (strEqualsAny(kind, "polar_heatmap", "nonuniform_polar_heatmap"))
    {
      plotDrawPolarAxes(subplot_args);
    }
}

grm_error_t plotGetArgsInHierarchy(grm_args_t *args, const char **hierarchy_name_start_ptr, const char *key,
                                   UintMap *hierarchy_to_id, const grm_args_t **found_args,
                                   const char ***found_hierarchy_name_ptr)
{
  const char *key_hierarchy_name, **current_hierarchy_name_ptr;
  grm_args_t *current_args, **args_array;
  grm_arg_t *current_arg;
  grm_error_t error;
  unsigned int args_array_length, current_id;

  logger((stderr, "Check hierarchy level for key \"%s\"...\n", key));
  if (!stringMapAt(plot_valid_keys_map, key, static_cast<const char **>(&key_hierarchy_name)))
    {
      error = GRM_ERROR_PLOT_UNKNOWN_KEY;
      error_code = error;
      returnIfError;
    }
  logger((stderr, "... got hierarchy \"%s\"\n", key_hierarchy_name));
  current_hierarchy_name_ptr = hierarchy_name_start_ptr;
  current_args = args;
  if (strcmp(*hierarchy_name_start_ptr, key_hierarchy_name) != 0)
    {
      while (*++current_hierarchy_name_ptr != nullptr)
        {
          current_arg = argsAt(current_args, *current_hierarchy_name_ptr);
          if (current_arg == nullptr)
            {
              error = GRM_ERROR_INTERNAL;
              error_code = error;
              returnIfError;
            }
          argFirstValue(current_arg, "A", &args_array, &args_array_length);
          uintMapAt(hierarchy_to_id, *current_hierarchy_name_ptr, &current_id);
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
              uintMapInsert(hierarchy_to_id, *current_hierarchy_name_ptr, current_id);
            }
          if (current_id > args_array_length)
            {
              plotInitArgsStructure(current_args, current_hierarchy_name_ptr - 1, current_id);
              argFirstValue(current_arg, "A", &args_array, &args_array_length);
            }
          current_args = args_array[current_id - 1];
          if (strcmp(*current_hierarchy_name_ptr, "plots") == 0)
            {
              int in_use;
              grm_error_t error = GRM_ERROR_NONE;
              grm_args_values(current_args, "in_use", "i", &in_use);
              if (in_use)
                {
                  error = eventQueueEnqueueUpdatePlotEvent(event_queue, (int)current_id - 1);
                }
              else
                {
                  error = eventQueueEnqueueNewPlotEvent(event_queue, (int)current_id - 1);
                }
              error_code = error;
              returnIfError;
              grm_args_push(current_args, "in_use", "i", 1);
            }
          if (strcmp(*current_hierarchy_name_ptr, key_hierarchy_name) == 0) break;
        }
      if (*current_hierarchy_name_ptr == nullptr)
        {
          error = GRM_ERROR_INTERNAL;
          error_code = error;
          returnIfError;
        }
    }
  if (found_args != nullptr) *found_args = current_args;
  if (found_hierarchy_name_ptr != nullptr) *found_hierarchy_name_ptr = current_hierarchy_name_ptr;

  return GRM_ERROR_NONE;
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plotting ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

grm_error_t plotLine(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  grm_error_t error = GRM_ERROR_NONE;
  int marker_type;

  grm_args_values(subplot_args, "series", "A", &current_series);
  auto group = (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();

  while (*current_series != nullptr)
    {
      double *x = nullptr, *y = nullptr, *int_limits_high = nullptr, *int_limits_low = nullptr;
      unsigned int x_length = 0, y_length = 0, limits_high_num = 0, limits_low_num = 0;
      char *spec, *label;
      double x_min, x_max, y_min, y_max;
      const char *x_axis_ref, *y_axis_ref;
      double line_width, border_width, marker_size;
      int line_color_ind, line_type, border_color_ind, marker_color_ind;
      std::string prefix = "";
      auto sub_group = global_render->createSeries("line");
      group->append(sub_group);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      grm_args_first_value(*current_series, "y", "D", &y, &y_length);

      if (grm_args_values(*current_series, "label", "s", &label))
        {
          prefix = std::string(label) + "_";
          sub_group->setAttribute("label", label);
          plotDrawLegend(subplot_args);
        }

      if (y_length > 0)
        {
          std::vector<double> y_vec(y, y + y_length);
          (*context)[prefix + "y" + str] = y_vec;
          sub_group->setAttribute("y", prefix + "y" + str);
        }
      if (grm_args_first_value(*current_series, "x", "D", &x, &x_length))
        {
          std::vector<double> x_vec(x, x + x_length);
          (*context)[prefix + "x" + str] = x_vec;
          sub_group->setAttribute("x", prefix + "x" + str);
        }

      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          sub_group->setAttribute("x_range_min", x_min);
          sub_group->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          sub_group->setAttribute("y_range_min", y_min);
          sub_group->setAttribute("y_range_max", y_max);
        }

      if (grm_args_values(*current_series, "ref_x_axis_location", "s", &x_axis_ref))
        sub_group->setAttribute("ref_x_axis_location", x_axis_ref);
      if (grm_args_values(*current_series, "ref_y_axis_location", "s", &y_axis_ref))
        sub_group->setAttribute("ref_y_axis_location", y_axis_ref);

      if (grm_args_values(*current_series, "line_spec", "s", &spec)) sub_group->setAttribute("line_spec", spec);
      if (grm_args_values(*current_series, "line_width", "d", &line_width))
        sub_group->setAttribute("line_width", line_width);
      if (grm_args_values(*current_series, "line_type", "i", &line_type))
        sub_group->setAttribute("line_type", line_type);
      if (grm_args_values(*current_series, "line_color_ind", "i", &line_color_ind))
        sub_group->setAttribute("line_color_ind", line_color_ind);
      if (grm_args_values(*current_series, "marker_type", "i", &marker_type))
        sub_group->setAttribute("marker_type", marker_type);
      if (grm_args_values(*current_series, "marker_size", "d", &marker_size))
        sub_group->setAttribute("marker_size", marker_size);
      if (grm_args_values(*current_series, "marker_color_ind", "i", &marker_color_ind))
        sub_group->setAttribute("marker_color_ind", marker_color_ind);
      if (grm_args_values(*current_series, "border_color_ind", "i", &border_color_ind))
        sub_group->setAttribute("border_color_ind", border_color_ind);
      if (grm_args_values(*current_series, "border_width", "d", &border_width))
        sub_group->setAttribute("border_width", border_width);

      // check if there are any attributes for integrals which should be created
      if (grm_args_first_value(*current_series, "int_limits_high", "D", &int_limits_high, &limits_high_num))
        {
          if (grm_args_first_value(*current_series, "int_limits_low", "D", &int_limits_low, &limits_low_num))
            {
              if (limits_low_num != limits_high_num)
                {
                  error = GRM_ERROR_PLOT_MISSING_DATA;
                  error_code = error;
                  returnIfError;
                }
              else
                {
                  auto integral_group = global_render->createElement("integral_group");
                  sub_group->append(integral_group);

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
              error = GRM_ERROR_PLOT_MISSING_DATA;
              error_code = error;
              returnIfError;
            }
        }

      global_root->setAttribute("_id", ++id);
      error = plotDrawErrorBars(*current_series, x_length);
      error_code = error;
      returnIfError;
      ++current_series;
    }

  return error;
}

grm_error_t plotStairs(grm_args_t *subplot_args)
{
  /*
   * Parameters:
   * `x` as double array
   * `y` as double array
   * optional step position `step_where` as string, modes: `pre`, `mid`, `post`, Default: `mid`
   * optional `spec`
   */

  grm_args_t **current_series;
  double x_min, x_max, y_min, y_max;
  double *x = nullptr, *y = nullptr;

  grm_args_values(subplot_args, "series", "A", &current_series);
  auto group = (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();

  std::shared_ptr<GRM::Element> element; // declare element here for multiple usages / assignments later
  while (*current_series != nullptr)
    {
      unsigned int x_length, y_length;
      char *spec, *label;
      const char *where;
      double y_line_pos;
      const char *x_axis_ref, *y_axis_ref;
      double line_width;
      int line_color_ind, line_type;
      std::string prefix = "";
      auto sub_group = global_render->createSeries("stairs");
      group->append(sub_group);

      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      grm_args_first_value(*current_series, "y", "D", &y, &y_length);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      if (grm_args_values(*current_series, "label", "s", &label))
        {
          prefix = std::string(label) + "_";
          sub_group->setAttribute("label", label);
          plotDrawLegend(subplot_args);
        }

      std::vector<double> x_vec(x, x + x_length);
      (*context)[prefix + "x" + str] = x_vec;
      sub_group->setAttribute("x", prefix + "x" + str);

      std::vector<double> y_vec(y, y + y_length);
      (*context)[prefix + "y" + str] = y_vec;
      sub_group->setAttribute("y", prefix + "y" + str);

      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          sub_group->setAttribute("x_range_min", x_min);
          sub_group->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          sub_group->setAttribute("y_range_min", y_min);
          sub_group->setAttribute("y_range_max", y_max);
        }

      if (grm_args_values(*current_series, "ref_x_axis_location", "s", &x_axis_ref))
        sub_group->setAttribute("ref_x_axis_location", x_axis_ref);
      if (grm_args_values(*current_series, "ref_y_axis_location", "s", &y_axis_ref))
        sub_group->setAttribute("ref_y_axis_location", y_axis_ref);

      if (grm_args_values(*current_series, "y_line_pos", "d", &y_line_pos))
        group->parentElement()->setAttribute("_y_line_pos", y_line_pos);

      if (grm_args_values(*current_series, "line_spec", "s", &spec)) sub_group->setAttribute("line_spec", spec);
      if (grm_args_values(*current_series, "line_width", "d", &line_width))
        sub_group->setAttribute("line_width", line_width);
      if (grm_args_values(*current_series, "line_type", "i", &line_type))
        sub_group->setAttribute("line_type", line_type);
      if (grm_args_values(*current_series, "line_color_ind", "i", &line_color_ind))
        sub_group->setAttribute("line_color_ind", line_color_ind);
      if (grm_args_values(*current_series, "step_where", "s", &where)) sub_group->setAttribute("step_where", where);

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }
  return GRM_ERROR_NONE;
}

grm_error_t plotScatter(grm_args_t *subplot_args)
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
  grm_error_t error;

  grm_args_values(subplot_args, "series", "A", &current_series);
  auto group = (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();

  while (*current_series != nullptr)
    {
      auto sub_group = global_render->createSeries("scatter");
      group->append(sub_group);

      double *x = nullptr, *y = nullptr, *z = nullptr, *c = nullptr, c_min, c_max;
      unsigned int x_length, y_length, z_length, c_length;
      int c_index = -1, marker_type;
      double x_min, x_max, y_min, y_max;
      const char *x_axis_ref, *y_axis_ref;
      char *label;
      double marker_size, border_width;
      int marker_color_ind, border_color_ind;
      std::string prefix = "";

      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      grm_args_first_value(*current_series, "y", "D", &y, &y_length);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> x_vec(x, x + x_length);
      std::vector<double> y_vec(y, y + y_length);

      if (grm_args_values(*current_series, "label", "s", &label))
        {
          prefix = std::string(label) + "_";
          sub_group->setAttribute("label", label);
          plotDrawLegend(subplot_args);
        }

      (*context)[prefix + "x" + str] = x_vec;
      sub_group->setAttribute("x", prefix + "x" + str);
      (*context)[prefix + "y" + str] = y_vec;
      sub_group->setAttribute("y", prefix + "y" + str);
      if (grm_args_first_value(*current_series, "z", "D", &z, &z_length))
        {
          std::vector<double> z_vec(z, z + z_length);

          (*context)[prefix + "z" + str] = z_vec;
          sub_group->setAttribute("z", prefix + "z" + str);
        }
      if (grm_args_values(*current_series, "marker_type", "i", &marker_type))
        sub_group->setAttribute("marker_type", marker_type);
      if (grm_args_values(*current_series, "marker_size", "d", &marker_size))
        sub_group->setAttribute("marker_size", marker_size);
      if (grm_args_values(*current_series, "marker_color_ind", "i", &marker_color_ind))
        sub_group->setAttribute("marker_color_ind", marker_color_ind);
      if (grm_args_values(*current_series, "border_color_ind", "i", &border_color_ind))
        sub_group->setAttribute("border_color_ind", border_color_ind);
      if (grm_args_values(*current_series, "border_width", "d", &border_width))
        sub_group->setAttribute("border_width", border_width);

      if (grm_args_first_value(*current_series, "c", "D", &c, &c_length))
        {
          std::vector<double> c_vec(c, c + c_length);

          (*context)[prefix + "c" + str] = c_vec;
          sub_group->setAttribute("c", prefix + "c" + str);
        }
      if (grm_args_values(*current_series, "c", "i", &c_index))
        {
          sub_group->setAttribute("marker_color_ind", c_index);
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
          sub_group->setAttribute("x_range_min", x_min);
          sub_group->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          sub_group->setAttribute("y_range_min", y_min);
          sub_group->setAttribute("y_range_max", y_max);
        }

      if (grm_args_values(*current_series, "ref_x_axis_location", "s", &x_axis_ref))
        sub_group->setAttribute("ref_x_axis_location", x_axis_ref);
      if (grm_args_values(*current_series, "ref_y_axis_location", "s", &y_axis_ref))
        sub_group->setAttribute("ref_y_axis_location", y_axis_ref);

      error = plotDrawErrorBars(*current_series, x_length);
      error_code = error;
      returnIfError;
      global_root->setAttribute("_id", ++id);
      ++current_series;
    }

  return GRM_ERROR_NONE;
}

grm_error_t plotQuiver(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  grm_error_t error = GRM_ERROR_NONE;

  auto group = (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();

  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double *x = nullptr, *y = nullptr, *u = nullptr, *v = nullptr;
      unsigned int x_length, y_length, u_length, v_length;
      double x_min, x_max, y_min, y_max;
      const char *x_axis_ref, *y_axis_ref;
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
          global_render->createQuiver("x" + str, x_vec, "y" + str, y_vec, "u" + str, u_vec, "v" + str, v_vec, true);

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

      if (grm_args_values(*current_series, "ref_x_axis_location", "s", &x_axis_ref))
        temp->setAttribute("ref_x_axis_location", x_axis_ref);
      if (grm_args_values(*current_series, "ref_y_axis_location", "s", &y_axis_ref))
        temp->setAttribute("ref_y_axis_location", y_axis_ref);

      group->append(temp);
      global_root->setAttribute("_id", ++id);
      ++current_series;
    }
  error = plotDrawColorbar(subplot_args, 0.0, 256);
  error_code = error;

  return error;
}

grm_error_t plotStem(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  auto group = (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();

  grm_args_values(subplot_args, "series", "A", &current_series);

  while (*current_series != nullptr)
    {
      double *x, *y;
      unsigned int x_length, y_length;
      char *spec, *label;
      double y_min, y_max, y_line_pos;
      const char *x_axis_ref, *y_axis_ref;
      double line_width;
      int line_type, line_color_ind;
      std::string prefix;

      auto sub_group = global_render->createSeries("stem");
      group->append(sub_group);

      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      grm_args_first_value(*current_series, "y", "D", &y, &y_length);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> x_vec(x, x + x_length);
      std::vector<double> y_vec(y, y + x_length);

      if (grm_args_values(*current_series, "label", "s", &label))
        {
          prefix = std::string(label) + "_";
          sub_group->setAttribute("label", label);
          plotDrawLegend(subplot_args);
        }

      (*context)[prefix + "x" + str] = x_vec;
      sub_group->setAttribute("x", prefix + "x" + str);
      (*context)[prefix + "y" + str] = y_vec;
      sub_group->setAttribute("y", prefix + "y" + str);

      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          sub_group->setAttribute("y_range_min", y_min);
          sub_group->setAttribute("y_range_max", y_max);
        }

      if (grm_args_values(*current_series, "ref_x_axis_location", "s", &x_axis_ref))
        sub_group->setAttribute("ref_x_axis_location", x_axis_ref);
      if (grm_args_values(*current_series, "ref_y_axis_location", "s", &y_axis_ref))
        sub_group->setAttribute("ref_y_axis_location", y_axis_ref);

      if (grm_args_values(*current_series, "y_line_pos", "d", &y_line_pos))
        group->parentElement()->setAttribute("_y_line_pos", y_line_pos);

      if (grm_args_values(*current_series, "line_spec", "s", &spec)) sub_group->setAttribute("line_spec", spec);
      if (grm_args_values(*current_series, "line_width", "d", &line_width))
        sub_group->setAttribute("line_width", line_width);
      if (grm_args_values(*current_series, "line_type", "i", &line_type))
        sub_group->setAttribute("line_type", line_type);
      if (grm_args_values(*current_series, "line_color_ind", "i", &line_color_ind))
        sub_group->setAttribute("line_color_ind", line_color_ind);

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }

  return GRM_ERROR_NONE;
}

grm_error_t plotHistogram(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  int bar_color_index = 989;
  double bar_color_rgb[3] = {-1};
  grm_error_t error = GRM_ERROR_NONE;

  auto group = (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();

  grm_args_values(subplot_args, "series", "A", &current_series);

  while (*current_series != nullptr)
    {
      int edge_color_index = 1;
      double edge_color_rgb[3] = {-1};
      double x_min, x_max, y_min, y_max, y_line_pos;
      double *bins, *x, *weights;
      unsigned int num_bins = 0, x_length, num_weights;
      double transparency;
      const char *x_axis_ref, *y_axis_ref;
      int fill_style, fill_int_style;

      auto sub_group = global_render->createSeries("histogram");
      group->append(sub_group);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      if (grm_args_values(subplot_args, "bar_color", "ddd", &bar_color_rgb[0], &bar_color_rgb[1], &bar_color_rgb[2]))
        {
          std::vector<double> bar_color_rgb_vec(bar_color_rgb, bar_color_rgb + 3);
          (*context)["fill_color_rgb" + str] = bar_color_rgb_vec;
          sub_group->setAttribute("fill_color_rgb", "fill_color_rgb" + str);
        }
      if (grm_args_values(subplot_args, "bar_color", "i", &bar_color_index))
        sub_group->setAttribute("fill_color_ind", bar_color_index);
      if (grm_args_values(subplot_args, "fill_style", "i", &fill_style))
        sub_group->setAttribute("fill_style", fill_style);
      if (grm_args_values(subplot_args, "fill_int_style", "i", &fill_int_style))
        sub_group->setAttribute("fill_int_style", fill_int_style);

      if (grm_args_values(*current_series, "edge_color", "ddd", &edge_color_rgb[0], &edge_color_rgb[1],
                          &edge_color_rgb[2]))
        {
          std::vector<double> edge_color_rgb_vec(edge_color_rgb, edge_color_rgb + 3);
          (*context)["line_color_rgb" + str] = edge_color_rgb_vec;
          sub_group->setAttribute("line_color_rgb", "line_color_rgb" + str);
        }
      if (grm_args_values(*current_series, "edge_color", "i", &edge_color_index))
        sub_group->setAttribute("line_color_ind", edge_color_index);

      if (grm_args_first_value(*current_series, "bins", "D", &bins, &num_bins))
        {
          std::vector<double> bins_vec(bins, bins + num_bins);
          (*context)["bins" + str] = bins_vec;
          sub_group->setAttribute("bins", "bins" + str);
        }
      if (num_bins == 0)
        {
          if (grm_args_values(*current_series, "num_bins", "i", &num_bins))
            sub_group->setAttribute("num_bins", (int)num_bins);
        }

      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          sub_group->setAttribute("x_range_min", x_min);
          sub_group->setAttribute("x_range_max", x_max);
        }

      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          sub_group->setAttribute("y_range_min", y_min);
          sub_group->setAttribute("y_range_max", y_max);
        }

      if (grm_args_values(*current_series, "ref_x_axis_location", "s", &x_axis_ref))
        sub_group->setAttribute("ref_x_axis_location", x_axis_ref);
      if (grm_args_values(*current_series, "ref_y_axis_location", "s", &y_axis_ref))
        sub_group->setAttribute("ref_y_axis_location", y_axis_ref);

      if (grm_args_values(*current_series, "y_line_pos", "d", &y_line_pos))
        group->parentElement()->setAttribute("_y_line_pos", y_line_pos);

      if (grm_args_values(*current_series, "transparency", "d", &transparency))
        {
          sub_group->setAttribute("transparency", transparency);
        }

      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      std::vector<double> x_vec(x, x + x_length);
      (*context)["x" + str] = x_vec;
      sub_group->setAttribute("x", "x" + str);

      if (grm_args_first_value(*current_series, "weights", "D", &weights, &num_weights))
        {
          std::vector<double> weights_vec(weights, weights + num_weights);
          (*context)["weights" + str] = weights_vec;
          sub_group->setAttribute("weights", "weights" + str);
        }

      if (grm_args_contains(*current_series, "error"))
        {
          if (num_bins <= 1) num_bins = (int)(3.3 * log10((int)x_length) + 0.5) + 1;
          error = plotDrawErrorBars(*current_series, num_bins);
          error_code = error;
        }
      global_root->setAttribute("_id", ++id);
      ++current_series;
    }

  return error;
}

grm_error_t plotBarplot(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  int bar_color, edge_color;
  double bar_color_rgb[3];
  double edge_color_rgb[3];
  double bar_width, edge_width;
  const char *style;
  int series_index = 0;
  unsigned int i;
  grm_error_t error = GRM_ERROR_NONE;

  auto group = (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();

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
      double x_min, x_max, y_min, y_max, y_line_pos;
      double transparency;
      const char *x_axis_ref, *y_axis_ref;
      int fill_style, fill_int_style;

      auto sub_group = global_render->createSeries("barplot");
      group->append(sub_group);
      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string id_str = std::to_string(id);

      if (grm_args_values(subplot_args, "bar_color", "ddd", &bar_color_rgb[0], &bar_color_rgb[1], &bar_color_rgb[2]))
        {
          std::vector<double> bar_color_rgb_vec(bar_color_rgb, bar_color_rgb + 3);
          (*context)["fill_color_rgb" + id_str] = bar_color_rgb_vec;
          sub_group->setAttribute("fill_color_rgb", "fill_color_rgb" + id_str);
        }
      if (grm_args_values(subplot_args, "bar_color", "i", &bar_color))
        sub_group->setAttribute("fill_color_ind", bar_color);
      if (grm_args_values(subplot_args, "fill_style", "i", &fill_style))
        sub_group->setAttribute("fill_style", fill_style);
      if (grm_args_values(subplot_args, "fill_int_style", "i", &fill_int_style))
        sub_group->setAttribute("fill_int_style", fill_int_style);
      if (grm_args_values(subplot_args, "bar_width", "d", &bar_width)) sub_group->setAttribute("bar_width", bar_width);
      if (grm_args_values(subplot_args, "style", "s", &style)) sub_group->setAttribute("style", style);
      if (grm_args_values(*current_series, "transparency", "d", &transparency))
        sub_group->setAttribute("transparency", transparency);

      /* Push attributes on the series level to the tree */
      if (grm_args_values(*current_series, "edge_color", "ddd", &edge_color_rgb[0], &edge_color_rgb[1],
                          &edge_color_rgb[2]))
        {
          std::vector<double> edge_color_rgb_vec(edge_color_rgb, edge_color_rgb + 3);
          (*context)["line_color_rgb" + id_str] = edge_color_rgb_vec;
          sub_group->setAttribute("line_color_rgb", "line_color_rgb" + id_str);
        }
      if (grm_args_values(*current_series, "edge_color", "i", &edge_color))
        sub_group->setAttribute("line_color_ind", edge_color);
      if (grm_args_values(*current_series, "edge_width", "d", &edge_width))
        sub_group->setAttribute("edge_width", edge_width);
      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          sub_group->setAttribute("x_range_min", x_min);
          sub_group->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          sub_group->setAttribute("y_range_min", y_min);
          sub_group->setAttribute("y_range_max", y_max);
        }

      if (grm_args_values(*current_series, "ref_x_axis_location", "s", &x_axis_ref))
        sub_group->setAttribute("ref_x_axis_location", x_axis_ref);
      if (grm_args_values(*current_series, "ref_y_axis_location", "s", &y_axis_ref))
        sub_group->setAttribute("ref_y_axis_location", y_axis_ref);

      if (grm_args_values(*current_series, "y_line_pos", "d", &y_line_pos))
        group->parentElement()->setAttribute("_y_line_pos", y_line_pos);
      if (grm_args_first_value(*current_series, "y_labels", "S", &y_labels, &y_labels_length))
        {
          std::vector<std::string> y_labels_vec(y_labels, y_labels + y_labels_length);
          (*context)["y_labels" + id_str] = y_labels_vec;
          sub_group->setAttribute("y_labels", "y_labels" + id_str);
        }
      if (grm_args_first_value(*current_series, "c", "I", &c, &c_length))
        {
          c_vec = std::vector<int>(c, c + c_length);
          (*context)["c_ind" + id_str] = c_vec;
          sub_group->setAttribute("color_ind_values", "c_ind" + id_str);
        }
      if (grm_args_first_value(*current_series, "c", "D", &c_rgb, &c_rgb_length))
        {
          c_rgb_vec = std::vector<double>(c_rgb, c_rgb + c_rgb_length);
          (*context)["c_rgb" + id_str] = c_rgb_vec;
          sub_group->setAttribute("color_rgb_values", "c_rgb" + id_str);
        }
      if ((grm_args_first_value(*current_series, "x", "D", &x, &x_length)))
        {
          /* Process data for a flat series (no inner_series) */
          auto x_vec = std::vector<double>(x, x + x_length);
          (*context)["x" + id_str] = x_vec;
          sub_group->setAttribute("x", "x" + id_str);
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
            cleanupAndSetErrorIf((c_length < inner_series_length), GRM_ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
          if (c_rgb != nullptr)
            cleanupAndSetErrorIf((c_rgb_length < inner_series_length * 3), GRM_ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);

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
                  cleanupAndSetErrorIf(inner_c_length != inner_y_length, GRM_ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
                  cleanupAndSetErrorIf(c_rgb != nullptr, GRM_ERROR_PLOT_INCOMPATIBLE_ARGUMENTS);
                }
              if (grm_args_first_value(inner_series[inner_series_index], "c", "D", &inner_c_rgb, &inner_c_rgb_length))
                {
                  cleanupAndSetErrorIf((inner_c_rgb_length < inner_y_length * 3),
                                       GRM_ERROR_PLOT_COMPONENT_LENGTH_MISMATCH);
                  cleanupAndSetErrorIf(c != nullptr, GRM_ERROR_PLOT_INCOMPATIBLE_ARGUMENTS);
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
                      global_render->setColorRep(sub_group, color_save_spot, inner_c_rgb[i * 3], inner_c_rgb[i * 3 + 1],
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
                          global_render->setColorRep(sub_group, color_save_spot, c_rgb[inner_series_index * 3],
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

                  cleanupAndSetErrorIf(color_save_spot > 1256, GRM_ERROR_INTERNAL);

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
              sub_group->setAttribute("y_labels", "y_labels" + id_str);
            }
          /* Replace the previously pushed c or c_rgb vector by one containing the data of the inner_series if such
           * data exists */
          if (inner_c_exists)
            {
              (*context)["c_ind" + id_str] = c_vec;
              sub_group->setAttribute("color_ind_values", "c_ind" + id_str);
              sub_group->removeAttribute("color_rgb_values");
            }
        }
      else
        {
          cleanupAndSetError(GRM_ERROR_PLOT_MISSING_DATA);
        }

      (*context)["y" + id_str] = y_vec;
      sub_group->setAttribute("y", "y" + id_str);
      (*context)["indices" + id_str] = indices_vec;
      sub_group->setAttribute("indices", "indices" + id_str);
      sub_group->setAttribute("series_index", series_index);

      cleanupAndSetErrorIf(y != nullptr && inner_series != nullptr, GRM_ERROR_PLOT_INCOMPATIBLE_ARGUMENTS);

      error = plotDrawErrorBars(*current_series, y_length);

      ++series_index;
      ++current_series;
      global_root->setAttribute("_id", ++id);
    }

cleanup:
  error_code = error;

  return error;
}

grm_error_t plotContour(grm_args_t *subplot_args)
{
  int num_levels = 20, major_h;
  grm_args_t **current_series;
  grm_error_t error = GRM_ERROR_NONE;

  auto group = (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();

  bool has_levels = grm_args_values(subplot_args, "levels", "i", &num_levels);
  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double *x, *y, *z;
      unsigned int x_length, y_length, z_length;
      double x_min, x_max, y_min, y_max, z_min, z_max;
      const char *x_axis_ref, *y_axis_ref;
      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      grm_args_first_value(*current_series, "y", "D", &y, &y_length);
      grm_args_first_value(*current_series, "z", "D", &z, &z_length);
      auto sub_group = global_render->createSeries("contour");
      group->append(sub_group);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> x_vec(x, x + x_length);
      (*context)["x" + str] = x_vec;
      sub_group->setAttribute("x", "x" + str);

      std::vector<double> y_vec(y, y + y_length);
      (*context)["y" + str] = y_vec;
      sub_group->setAttribute("y", "y" + str);

      std::vector<double> z_vec(z, z + z_length);
      (*context)["z" + str] = z_vec;
      sub_group->setAttribute("z", "z" + str);

      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          sub_group->setAttribute("x_range_min", x_min);
          sub_group->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          sub_group->setAttribute("y_range_min", y_min);
          sub_group->setAttribute("y_range_max", y_max);
        }
      if (grm_args_values(*current_series, "z_range", "dd", &z_min, &z_max))
        {
          sub_group->setAttribute("z_range_min", z_min);
          sub_group->setAttribute("z_range_max", z_max);
        }

      if (grm_args_values(*current_series, "ref_x_axis_location", "s", &x_axis_ref))
        sub_group->setAttribute("ref_x_axis_location", x_axis_ref);
      if (grm_args_values(*current_series, "ref_y_axis_location", "s", &y_axis_ref))
        sub_group->setAttribute("ref_y_axis_location", y_axis_ref);

      if (grm_args_values(subplot_args, "major_h", "i", &major_h)) sub_group->setAttribute("major_h", major_h);
      if (has_levels) sub_group->setAttribute("levels", num_levels);

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }
  error = plotDrawColorbar(subplot_args, 0.0, num_levels);
  error_code = error;

  return error;
}

grm_error_t plotContourf(grm_args_t *subplot_args)
{
  int num_levels = 20, major_h;
  grm_args_t **current_series;
  grm_error_t error = GRM_ERROR_NONE;

  auto group = (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();

  bool has_levels = grm_args_values(subplot_args, "levels", "i", &num_levels);
  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double *x, *y, *z;
      unsigned int x_length, y_length, z_length;
      double x_min, x_max, y_min, y_max, z_min, z_max;
      const char *x_axis_ref, *y_axis_ref;
      auto sub_group = global_render->createSeries("contourf");
      group->append(sub_group);
      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      grm_args_first_value(*current_series, "y", "D", &y, &y_length);
      grm_args_first_value(*current_series, "z", "D", &z, &z_length);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> x_vec(x, x + x_length);
      (*context)["x" + str] = x_vec;
      sub_group->setAttribute("x", "x" + str);

      std::vector<double> y_vec(y, y + y_length);
      (*context)["y" + str] = y_vec;
      sub_group->setAttribute("y", "y" + str);

      std::vector<double> z_vec(z, z + z_length);
      (*context)["z" + str] = z_vec;
      sub_group->setAttribute("z", "z" + str);

      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          sub_group->setAttribute("x_range_min", x_min);
          sub_group->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          sub_group->setAttribute("y_range_min", y_min);
          sub_group->setAttribute("y_range_max", y_max);
        }
      if (grm_args_values(*current_series, "z_range", "dd", &z_min, &z_max))
        {
          sub_group->setAttribute("z_range_min", z_min);
          sub_group->setAttribute("z_range_max", z_max);
        }

      if (grm_args_values(*current_series, "ref_x_axis_location", "s", &x_axis_ref))
        sub_group->setAttribute("ref_x_axis_location", x_axis_ref);
      if (grm_args_values(*current_series, "ref_y_axis_location", "s", &y_axis_ref))
        sub_group->setAttribute("ref_y_axis_location", y_axis_ref);

      if (grm_args_values(subplot_args, "major_h", "i", &major_h)) sub_group->setAttribute("major_h", major_h);
      if (has_levels) sub_group->setAttribute("levels", num_levels);

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }
  error = plotDrawColorbar(subplot_args, 0.0, num_levels);
  error_code = error;

  return error;
}

grm_error_t plotHexbin(grm_args_t *subplot_args)
{
  grm_args_t **current_series;

  auto group = (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();

  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double *x, *y;
      unsigned int x_length, y_length;
      int nbins;
      double x_min, x_max, y_min, y_max;
      const char *x_axis_ref, *y_axis_ref;
      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      grm_args_first_value(*current_series, "y", "D", &y, &y_length);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);

      auto x_vec = std::vector<double>(x, x + x_length);
      auto y_vec = std::vector<double>(y, y + y_length);
      auto sub_group = global_render->createHexbin("x" + str, x_vec, "y" + str, y_vec);
      if (grm_args_values(*current_series, "num_bins", "i", &nbins)) sub_group->setAttribute("num_bins", nbins);
      group->append(sub_group);

      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          sub_group->setAttribute("x_range_min", x_min);
          sub_group->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          sub_group->setAttribute("y_range_min", y_min);
          sub_group->setAttribute("y_range_max", y_max);
        }

      if (grm_args_values(*current_series, "ref_x_axis_location", "s", &x_axis_ref))
        sub_group->setAttribute("ref_x_axis_location", x_axis_ref);
      if (grm_args_values(*current_series, "ref_y_axis_location", "s", &y_axis_ref))
        sub_group->setAttribute("ref_y_axis_location", y_axis_ref);

      plotDrawColorbar(subplot_args, 0.0, 256);

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }

  return GRM_ERROR_NONE;
}

grm_error_t plotPolarHeatmap(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  int z_log = 0;
  unsigned int cols, rows, z_length;
  double *theta = nullptr, *r = nullptr, *z, theta_min, theta_max, r_min, r_max, z_min, z_max, c_min, c_max;
  grm_error_t error = GRM_ERROR_NONE;

  auto group = (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();

  grm_args_values(subplot_args, "series", "A", &current_series);
  grm_args_values(subplot_args, "z_log", "i", &z_log);
  while (*current_series != nullptr)
    {
      auto sub_group = global_render->createSeries("polar_heatmap");
      group->append(sub_group);

      grm_args_first_value(*current_series, "theta", "D", &theta, &cols);
      grm_args_first_value(*current_series, "r", "D", &r, &rows);
      grm_args_first_value(*current_series, "z", "D", &z, &z_length);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      if (theta != nullptr)
        {
          std::vector<double> theta_vec(theta, theta + cols);
          (*context)["theta" + str] = theta_vec;
          sub_group->setAttribute("theta", "theta" + str);
        }

      if (r != nullptr)
        {
          std::vector<double> r_vec(r, r + rows);
          (*context)["r" + str] = r_vec;
          sub_group->setAttribute("r", "r" + str);
        }

      std::vector<double> z_vec(z, z + z_length);
      (*context)["z" + str] = z_vec;
      sub_group->setAttribute("z", "z" + str);

      group->parentElement()->setAttribute("z_log", z_log);

      if (theta == nullptr && r == nullptr)
        {
          /* If neither `theta` nor `r` are given, we need more information about the shape of `z` */
          grm_args_values(*current_series, "z_dims", "ii", &cols, &rows);

          auto z_dims_vec = std::vector<int>{(int)cols, (int)rows};
          auto z_dims_key = "z_dims" + str;
          (*context)[z_dims_key] = z_dims_vec;
          sub_group->setAttribute("z_dims", z_dims_key);
        }
      if (theta == nullptr)
        {
          if (grm_args_values(*current_series, "theta_range", "dd", &theta_min, &theta_max))
            {
              sub_group->setAttribute("theta_range_min", theta_min);
              sub_group->setAttribute("theta_range_max", theta_max);
            }
        }
      if (r == nullptr)
        {
          if (grm_args_values(*current_series, "r_range", "dd", &r_min, &r_max))
            {
              sub_group->setAttribute("r_range_min", r_min);
              sub_group->setAttribute("r_range_max", r_max);
            }
        }
      if (grm_args_values(*current_series, "z_range", "dd", &z_min, &z_max))
        {
          sub_group->setAttribute("z_range_min", z_min);
          sub_group->setAttribute("z_range_max", z_max);
        }
      if (grm_args_values(*current_series, "c_range", "dd", &c_min, &c_max))
        {
          sub_group->setAttribute("c_range_min", c_min);
          sub_group->setAttribute("c_range_max", c_max);
        }

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }

  plotDrawColorbar(subplot_args, PLOT_POLAR_COLORBAR_OFFSET, 256);
  return error;
}

grm_error_t plotHeatmap(grm_args_t *subplot_args)
{
  const char *kind = nullptr;
  grm_args_t **current_series;
  int z_log = 0;
  unsigned int cols, rows, z_length;
  double *x = nullptr, *y = nullptr, *z, x_min, x_max, y_min, y_max, z_min, z_max, c_min, c_max;
  grm_error_t error = GRM_ERROR_NONE;
  std::shared_ptr<GRM::Element> plot_parent;

  auto group = (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();

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
  while (*current_series != nullptr)
    {
      const char *x_axis_ref, *y_axis_ref;
      x = y = nullptr;
      auto sub_group = global_render->createSeries("heatmap");
      group->append(sub_group);
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
          sub_group->setAttribute("x", "x" + str);
        }

      if (y != nullptr)
        {
          std::vector<double> y_vec(y, y + rows);
          (*context)["y" + str] = y_vec;
          sub_group->setAttribute("y", "y" + str);
        }

      std::vector<double> z_vec(z, z + z_length);
      (*context)["z" + str] = z_vec;
      sub_group->setAttribute("z", "z" + str);

      plot_parent->setAttribute("z_log", z_log);

      if (x == nullptr && y == nullptr)
        {
          /* If neither `x` nor `y` are given, we need more information about the shape of `z` */
          grm_args_values(*current_series, "z_dims", "ii", &cols, &rows);

          auto z_dims_vec = std::vector<int>{(int)cols, (int)rows};
          auto z_dims_key = "z_dims" + str;
          (*context)[z_dims_key] = z_dims_vec;
          sub_group->setAttribute("z_dims", z_dims_key);
        }
      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          sub_group->setAttribute("x_range_min", x_min);
          sub_group->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          sub_group->setAttribute("y_range_min", y_min);
          sub_group->setAttribute("y_range_max", y_max);
        }
      if (grm_args_values(*current_series, "z_range", "dd", &z_min, &z_max))
        {
          sub_group->setAttribute("z_range_min", z_min);
          sub_group->setAttribute("z_range_max", z_max);
        }
      if (grm_args_values(*current_series, "c_range", "dd", &c_min, &c_max))
        {
          sub_group->setAttribute("c_range_min", c_min);
          sub_group->setAttribute("c_range_max", c_max);
        }

      if (grm_args_values(*current_series, "ref_x_axis_location", "s", &x_axis_ref))
        sub_group->setAttribute("ref_x_axis_location", x_axis_ref);
      if (grm_args_values(*current_series, "ref_y_axis_location", "s", &y_axis_ref))
        sub_group->setAttribute("ref_y_axis_location", y_axis_ref);

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }

  if (strcmp(kind, "marginal_heatmap") != 0) plotDrawColorbar(subplot_args, 0.0, 256);

  return error;
}

grm_error_t plotMarginalHeatmap(grm_args_t *subplot_args)
{
  int x_ind = -1, y_ind = -1, z_log = 0;
  const char *marginal_heatmap_kind = "all";
  grm_error_t error = GRM_ERROR_NONE;
  grm_args_t **current_series;
  double *x, *y, *plot;
  unsigned int num_bins_x, num_bins_y, n;

  auto group = (!current_dom_element.expired()) ? current_dom_element.lock() : edit_figure->lastChildElement();
  auto sub_group = group->querySelectors("marginal_heatmap_plot");
  if (sub_group == nullptr)
    {
      sub_group = global_render->createElement("marginal_heatmap_plot");
      group->append(sub_group);
    }

  grm_args_values(subplot_args, "z_log", "i", &z_log);
  group->setAttribute("z_log", z_log);

  if (grm_args_values(subplot_args, "marginal_heatmap_kind", "s", &marginal_heatmap_kind))
    sub_group->setAttribute("marginal_heatmap_kind", marginal_heatmap_kind);
  if (grm_args_values(subplot_args, "x_ind", "i", &x_ind)) sub_group->setAttribute("x_ind", x_ind);
  if (grm_args_values(subplot_args, "y_ind", "i", &y_ind)) sub_group->setAttribute("y_ind", y_ind);
  sub_group->setAttribute("kind", "marginal_heatmap");

  grm_args_values(subplot_args, "series", "A", &current_series);
  grm_args_first_value(*current_series, "x", "D", &x, &num_bins_x);
  grm_args_first_value(*current_series, "y", "D", &y, &num_bins_y);
  grm_args_first_value(*current_series, "z", "D", &plot, &n);

  int id = static_cast<int>(global_root->getAttribute("_id"));
  std::string str = std::to_string(id);
  auto context = global_render->getContext();

  std::vector<double> x_vec(x, x + num_bins_x);
  (*context)["x" + str] = x_vec;
  sub_group->setAttribute("x", "x" + str);

  std::vector<double> y_vec(y, y + num_bins_y);
  (*context)["y" + str] = y_vec;
  sub_group->setAttribute("y", "y" + str);

  std::vector<double> z_vec(plot, plot + n);
  (*context)["z" + str] = z_vec;
  sub_group->setAttribute("z", "z" + str);

  if (strcmp(marginal_heatmap_kind, "all") == 0)
    {
      const char *algorithm;
      if (grm_args_values(*current_series, "algorithm", "s", &algorithm))
        sub_group->setAttribute("algorithm", algorithm);
    }

  std::shared_ptr<GRM::Element> top_side_region;
  if (!sub_group->querySelectors("side_region[location=\"top\"]"))
    {
      top_side_region = global_render->createSideRegion("top");
      sub_group->append(top_side_region);
    }
  else
    {
      top_side_region = sub_group->querySelectors("side_region[location=\"top\"]");
    }
  top_side_region->setAttribute("marginal_heatmap_side_plot", 1);
  auto right_side_region = global_render->createSideRegion("right");
  right_side_region->setAttribute("marginal_heatmap_side_plot", 1);
  sub_group->append(right_side_region);

  grm_args_push(subplot_args, "kind", "s", "marginal_heatmap");
  global_root->setAttribute("_id", ++id);

  return error;
}

grm_error_t plotWireframe(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  grm_error_t error = GRM_ERROR_NONE;

  auto group = (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();

  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double *x, *y, *z;
      unsigned int x_length, y_length, z_length;
      double x_min, x_max, y_min, y_max;

      auto sub_group = global_render->createSeries("wireframe");
      group->append(sub_group);

      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      grm_args_first_value(*current_series, "y", "D", &y, &y_length);
      grm_args_first_value(*current_series, "z", "D", &z, &z_length);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> x_vec(x, x + x_length);
      (*context)["x" + str] = x_vec;
      sub_group->setAttribute("x", "x" + str);

      std::vector<double> y_vec(y, y + y_length);
      (*context)["y" + str] = y_vec;
      sub_group->setAttribute("y", "y" + str);

      std::vector<double> z_vec(z, z + z_length);
      (*context)["z" + str] = z_vec;
      sub_group->setAttribute("z", "z" + str);

      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          sub_group->setAttribute("x_range_min", x_min);
          sub_group->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          sub_group->setAttribute("y_range_min", y_min);
          sub_group->setAttribute("y_range_max", y_max);
        }

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }
  plotDrawAxes(subplot_args, 2);

  return error;
}

grm_error_t plotSurface(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  grm_error_t error = GRM_ERROR_NONE;
  int accelerate; /* this argument decides if GR3 or GRM is used to plot the surface */
  double xmin, xmax, ymin, ymax;

  auto group = (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();

  grm_args_values(subplot_args, "series", "A", &current_series);
  bool has_accelerate = grm_args_values(subplot_args, "accelerate", "i", &accelerate);

  while (*current_series != nullptr)
    {
      double *x = nullptr, *y = nullptr, *z = nullptr;
      unsigned int x_length, y_length, z_length;

      auto sub_group = global_render->createSeries("surface");
      group->append(sub_group);
      if (has_accelerate) sub_group->setAttribute("accelerate", accelerate);

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
          sub_group->setAttribute("z_dims", z_dims_key);
        }
      if (grm_args_values(*current_series, "x_range", "dd", &xmin, &xmax))
        {
          sub_group->setAttribute("x_range_min", xmin);
          sub_group->setAttribute("x_range_max", xmax);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &ymin, &ymax))
        {
          sub_group->setAttribute("y_range_min", ymin);
          sub_group->setAttribute("y_range_max", ymax);
        }

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      if (x != nullptr)
        {
          std::vector<double> x_vec(x, x + x_length);
          (*context)["x" + str] = x_vec;
          sub_group->setAttribute("x", "x" + str);
        }

      if (y != nullptr)
        {
          std::vector<double> y_vec(y, y + y_length);
          (*context)["y" + str] = y_vec;
          sub_group->setAttribute("y", "y" + str);
        }

      std::vector<double> z_vec(z, z + z_length);
      (*context)["z" + str] = z_vec;
      sub_group->setAttribute("z", "z" + str);

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }
  plotDrawAxes(subplot_args, 2);
  plotDrawColorbar(subplot_args, PLOT_3D_COLORBAR_OFFSET, 256);

  return error;
}

grm_error_t plotLine3(grm_args_t *subplot_args)
{
  grm_args_t **current_series;

  auto group = (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();
  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double *x, *y, *z;
      unsigned int x_length, y_length, z_length;
      double x_min, x_max, y_min, y_max, z_min, z_max;
      char *label;
      std::string prefix = "";
      auto sub_group = global_render->createSeries("line3");
      group->append(sub_group);
      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      grm_args_first_value(*current_series, "y", "D", &y, &y_length);
      grm_args_first_value(*current_series, "z", "D", &z, &z_length);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      if (grm_args_values(*current_series, "label", "s", &label))
        {
          prefix = std::string(label) + "_";
          sub_group->setAttribute("label", label);
          plotDrawLegend(subplot_args);
        }

      std::vector<double> x_vec(x, x + x_length);
      (*context)[prefix + "x" + str] = x_vec;
      sub_group->setAttribute("x", prefix + "x" + str);

      std::vector<double> y_vec(y, y + y_length);
      (*context)[prefix + "y" + str] = y_vec;
      sub_group->setAttribute("y", prefix + "y" + str);

      std::vector<double> z_vec(z, z + z_length);
      (*context)[prefix + "z" + str] = z_vec;
      sub_group->setAttribute("z", prefix + "z" + str);

      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          sub_group->setAttribute("x_range_min", x_min);
          sub_group->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          sub_group->setAttribute("y_range_min", y_min);
          sub_group->setAttribute("y_range_max", y_max);
        }
      if (grm_args_values(*current_series, "z_range", "dd", &z_min, &z_max))
        {
          sub_group->setAttribute("z_range_min", z_min);
          sub_group->setAttribute("z_range_max", z_max);
        }

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }
  plotDrawAxes(subplot_args, 2);

  return GRM_ERROR_NONE;
}

grm_error_t plotScatter3(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  double c_min, c_max;
  unsigned int x_length, y_length, z_length, c_length;
  double *x, *y, *z, *c;

  auto group = (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();
  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double x_min, x_max, y_min, y_max, z_min, z_max;
      char *label;
      std::string prefix = "";
      auto sub_group = global_render->createSeries("scatter3");
      group->append(sub_group);
      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      grm_args_first_value(*current_series, "y", "D", &y, &y_length);
      grm_args_first_value(*current_series, "z", "D", &z, &z_length);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> x_vec(x, x + x_length);
      std::vector<double> y_vec(y, y + y_length);
      std::vector<double> z_vec(z, z + z_length);

      if (grm_args_values(*current_series, "label", "s", &label))
        {
          prefix = std::string(label) + "_";
          sub_group->setAttribute("label", label);
          plotDrawLegend(subplot_args);
        }

      (*context)[prefix + "x" + str] = x_vec;
      sub_group->setAttribute("x", prefix + "x" + str);
      (*context)[prefix + "y" + str] = y_vec;
      sub_group->setAttribute("y", prefix + "y" + str);
      (*context)[prefix + "z" + str] = z_vec;
      sub_group->setAttribute("z", prefix + "z" + str);

      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          sub_group->setAttribute("x_range_min", x_min);
          sub_group->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          sub_group->setAttribute("y_range_min", y_min);
          sub_group->setAttribute("y_range_max", y_max);
        }
      if (grm_args_values(*current_series, "z_range", "dd", &z_min, &z_max))
        {
          sub_group->setAttribute("z_range_min", z_min);
          sub_group->setAttribute("z_range_max", z_max);
        }

      if (grm_args_first_value(*current_series, "c", "D", &c, &c_length))
        {
          std::vector<double> c_vec(c, c + c_length);
          (*context)[prefix + "c" + str] = c_vec;
          sub_group->setAttribute("c", prefix + "c" + str);

          if (grm_args_values(subplot_args, "c_lim", "dd", &c_min, &c_max))
            {
              group->parentElement()->setAttribute("c_lim_min", c_min);
              group->parentElement()->setAttribute("c_lim_max", c_max);
            }
        }

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }
  plotDrawAxes(subplot_args, 2);

  return GRM_ERROR_NONE;
}

grm_error_t plotImshow(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  double *c_data;
  double c_min, c_max;
  unsigned int c_data_length, i;
  unsigned int *shape;

  auto group = (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();

  grm_args_values(subplot_args, "series", "A", &current_series);
  if (grm_args_values(subplot_args, "c_lim", "dd", &c_min, &c_max))
    {
      group->parentElement()->setAttribute("z_lim_min", c_min);
      group->parentElement()->setAttribute("z_lim_max", c_max);
    }
  while (*current_series != nullptr)
    {
      auto sub_group = global_render->createSeries("imshow");
      group->append(sub_group);

      grm_args_first_value(*current_series, "c", "D", &c_data, &c_data_length);
      grm_args_first_value(*current_series, "c_dims", "I", &shape, &i);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> c_data_vec(c_data, c_data + c_data_length);
      std::vector<int> shape_vec(shape, shape + i);

      (*context)["z" + str] = c_data_vec;
      sub_group->setAttribute("z", "z" + str);
      (*context)["z_dims" + str] = shape_vec;
      sub_group->setAttribute("z_dims", "z_dims" + str);

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }

  return GRM_ERROR_NONE;
}

grm_error_t plotIsosurface(grm_args_t *subplot_args)
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
  double isovalue;

  auto group = (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();

  grm_args_values(subplot_args, "series", "A", &current_series);

  while (*current_series != nullptr)
    {
      auto sub_group = global_render->createSeries("isosurface");
      group->append(sub_group);
      grm_args_first_value(*current_series, "c", "D", &orig_data, &data_length);
      grm_args_first_value(*current_series, "c_dims", "I", &shape, &dims);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> c_data_vec(orig_data, orig_data + data_length);
      std::vector<int> shape_vec(shape, shape + dims);

      (*context)["z" + str] = c_data_vec;
      sub_group->setAttribute("z", "z" + str);
      (*context)["z_dims" + str] = shape_vec;
      sub_group->setAttribute("z_dims", "z_dims" + str);

      if (grm_args_values(*current_series, "isovalue", "d", &isovalue)) sub_group->setAttribute("isovalue", isovalue);
      /*
       * We need to convert the double values to floats, as GR3 expects floats, but an argument can only contain
       * doubles.
       */
      if (grm_args_first_value(*current_series, "foreground_color", "D", &temp_colors, &i))
        {
          std::vector<double> foreground_vec(temp_colors, temp_colors + i);
          (*context)["c_rgb" + str] = foreground_vec;
          sub_group->setAttribute("color_rgb_values", "c_rgb" + str);
        }

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }

  return GRM_ERROR_NONE;
}

grm_error_t plotVolume(grm_args_t *subplot_args)
{
  grm_args_t **current_series;
  grm_error_t error;

  auto group = (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();

  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      auto sub_group = global_render->createSeries("volume");
      group->append(sub_group);
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
      sub_group->setAttribute("z", "z" + str);
      (*context)["z_dims" + str] = shape_vec;
      sub_group->setAttribute("z_dims", "z_dims" + str);

      if (!grm_args_values(*current_series, "algorithm", "i", &algorithm))
        {
          if (grm_args_values(*current_series, "algorithm", "s", &algorithm_str))
            sub_group->setAttribute("algorithm", algorithm_str);
        }
      else
        {
          sub_group->setAttribute("algorithm", algorithm);
        }

      d_min = d_max = -1.0;
      grm_args_values(*current_series, "d_min", "d", &d_min);
      grm_args_values(*current_series, "d_max", "d", &d_max);
      sub_group->setAttribute("d_min", d_min);
      sub_group->setAttribute("d_max", d_max);

      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          sub_group->setAttribute("x_range_min", x_min);
          sub_group->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          sub_group->setAttribute("y_range_min", y_min);
          sub_group->setAttribute("y_range_max", y_max);
        }
      if (grm_args_values(*current_series, "z_range", "dd", &z_min, &z_max))
        {
          sub_group->setAttribute("z_range_min", z_min);
          sub_group->setAttribute("z_range_max", z_max);
        }

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }

  error = plotDrawAxes(subplot_args, 2);
  error_code = error;
  returnIfError;
  error = plotDrawColorbar(subplot_args, PLOT_3D_COLORBAR_OFFSET, 256);
  error_code = error;
  returnIfError;

  return GRM_ERROR_NONE;
}

grm_error_t plotPolarLine(grm_args_t *subplot_args)
{
  grm_args_t **current_series;

  auto group = (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();

  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double *theta, *r;
      double theta_min, theta_max, r_min, r_max;
      unsigned int theta_length, r_length;
      char *spec, *label;
      auto sub_group = global_render->createSeries("polar_line");
      int clip_negative = 0, marker_type;
      double line_width, border_width, marker_size;
      int line_type, line_color_ind, border_color_ind, marker_color_ind;
      std::string prefix = "";
      group->append(sub_group);

      grm_args_first_value(*current_series, "theta", "D", &theta, &theta_length);
      grm_args_first_value(*current_series, "r", "D", &r, &r_length);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> theta_vec(theta, theta + theta_length);
      std::vector<double> r_vec(r, r + r_length);

      if (grm_args_values(*current_series, "label", "s", &label))
        {
          prefix = std::string(label) + "_";
          sub_group->setAttribute("label", label);
          plotDrawLegend(subplot_args);
        }

      (*context)[prefix + "theta" + str] = theta_vec;
      sub_group->setAttribute("theta", prefix + "theta" + str);
      (*context)[prefix + "r" + str] = r_vec;
      sub_group->setAttribute("r", prefix + "r" + str);

      if (grm_args_values(*current_series, "r_range", "dd", &r_min, &r_max))
        {
          sub_group->setAttribute("r_range_min", r_min);
          sub_group->setAttribute("r_range_max", r_max);
        }
      if (grm_args_values(*current_series, "theta_range", "dd", &theta_min, &theta_max))
        {
          sub_group->setAttribute("theta_range_min", theta_min);
          sub_group->setAttribute("theta_range_max", theta_max);
        }
      if (grm_args_values(*current_series, "clip_negative", "i", &clip_negative))
        {
          sub_group->setAttribute("clip_negative", clip_negative);
        }

      if (grm_args_values(*current_series, "line_spec", "s", &spec)) sub_group->setAttribute("line_spec", spec);
      if (grm_args_values(*current_series, "line_width", "d", &line_width))
        sub_group->setAttribute("line_width", line_width);
      if (grm_args_values(*current_series, "line_type", "i", &line_type))
        sub_group->setAttribute("line_type", line_type);
      if (grm_args_values(*current_series, "line_color_ind", "i", &line_color_ind))
        sub_group->setAttribute("line_color_ind", line_color_ind);
      if (grm_args_values(*current_series, "marker_type", "i", &marker_type))
        sub_group->setAttribute("marker_type", marker_type);
      if (grm_args_values(*current_series, "marker_size", "d", &marker_size))
        sub_group->setAttribute("marker_size", marker_size);
      if (grm_args_values(*current_series, "marker_color_ind", "i", &marker_color_ind))
        sub_group->setAttribute("marker_color_ind", marker_color_ind);
      if (grm_args_values(*current_series, "border_color_ind", "i", &border_color_ind))
        sub_group->setAttribute("border_color_ind", border_color_ind);
      if (grm_args_values(*current_series, "border_width", "d", &border_width))
        sub_group->setAttribute("border_width", border_width);

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }

  return GRM_ERROR_NONE;
}

grm_error_t plotPolarScatter(grm_args_t *subplot_args)
{
  grm_args_t **current_series;

  auto group = (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();

  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double *theta, *r;
      double theta_min, theta_max, r_min, r_max;
      unsigned int theta_length, r_length;
      char *label;
      auto sub_group = global_render->createSeries("polar_scatter");
      int clip_negative = 0, marker_type;
      std::string prefix = "";
      group->append(sub_group);

      grm_args_first_value(*current_series, "theta", "D", &theta, &theta_length);
      grm_args_first_value(*current_series, "r", "D", &r, &r_length);

      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      if (grm_args_values(*current_series, "label", "s", &label))
        {
          prefix = std::string(label) + "_";
          sub_group->setAttribute("label", label);
          plotDrawLegend(subplot_args);
        }

      std::vector<double> theta_vec(theta, theta + theta_length);
      std::vector<double> r_vec(r, r + r_length);

      (*context)[prefix + "theta" + str] = theta_vec;
      sub_group->setAttribute("theta", prefix + "theta" + str);
      (*context)[prefix + "r" + str] = r_vec;
      sub_group->setAttribute("r", prefix + "r" + str);

      if (grm_args_values(*current_series, "r_range", "dd", &r_min, &r_max))
        {
          sub_group->setAttribute("r_range_min", r_min);
          sub_group->setAttribute("r_range_max", r_max);
        }
      if (grm_args_values(*current_series, "theta_range", "dd", &theta_min, &theta_max))
        {
          sub_group->setAttribute("theta_range_min", theta_min);
          sub_group->setAttribute("theta_range_max", theta_max);
        }
      if (grm_args_values(*current_series, "clip_negative", "i", &clip_negative))
        {
          sub_group->setAttribute("clip_negative", clip_negative);
        }

      if (grm_args_values(*current_series, "marker_type", "i", &marker_type))
        sub_group->setAttribute("marker_type", marker_type);

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }

  return GRM_ERROR_NONE;
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
 *            previous bins. The height of the last bin is numel (x).
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
 * \param[in] theta_lim a list containing two double values representing start and end angle (series)
 *            The plot will only use the values in the given range and and it will plot bins between those two values.
 * \param[in] theta_flip an integer that represents a boolean (subplot)
 *            0: the angles will be displayed anti clockwise.
 *            else: the angles will be displayed clockwise.
 *            If theta_lim[0] is greater than theta_lim[1], theta_flip will be negated.
 *            The default value is 0.
 * \param[in] r_lim a list containing two double values between (inclusive) 0.0 and 1.0 (series)
 *            0.0 is the center and 1.0 is the outer edge of the plot.
 *            r_lim will limit the bins' start and end height.
 * \param[in] bin_width a double value between (exclusive) 0.0 and 2 * pi setting the bins' width (series)
 *            It is not compatible nbins or bin_edges.
 * \param[in] num_bins an int setting the number of bins (series)
 *            It is not compatible with bin_edges, nbins or bin_counts.
 * \param[in] transparency double value between 0.0 and 1.0 inclusive (series)
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
grm_error_t plotPolarHistogram(grm_args_t *subplot_args)
{
  int stairs;
  int keep_radii_axes;
  int theta_colormap, r_colormap;
  int draw_edges, theta_flip, edge_color, face_color;
  double transparency;
  double theta_range_min, theta_range_max, r_lim_min, r_lim_max;
  grm_args_t **series;
  int fill_style, fill_int_style;

  std::shared_ptr<GRM::Element> plot_group = edit_figure->lastChildElement();
  auto group = (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();
  std::shared_ptr<GRM::Element> series_group = global_render->createSeries("polar_histogram");
  group->append(series_group);

  // Call counts -> set attributes and data
  countsPolarHistogram(subplot_args);

  auto context = global_render->getContext();

  int id = static_cast<int>(global_root->getAttribute("_id"));
  std::string str = std::to_string(id);

  grm_args_values(subplot_args, "series", "A", &series);

  /* edge_color */
  if (grm_args_values(*series, "edge_color", "i", &edge_color))
    series_group->setAttribute("line_color_ind", edge_color);
  /* face_color */
  if (grm_args_values(*series, "face_color", "i", &face_color))
    series_group->setAttribute("fill_color_ind", face_color);
  if (grm_args_values(subplot_args, "fill_style", "i", &fill_style))
    series_group->setAttribute("fill_style", fill_style);
  if (grm_args_values(subplot_args, "fill_int_style", "i", &fill_int_style))
    series_group->setAttribute("fill_int_style", fill_int_style);
  /* transparency */
  if (grm_args_values(*series, "transparency", "d", &transparency))
    series_group->setAttribute("transparency", transparency);
  if (grm_args_values(subplot_args, "theta_flip", "i", &theta_flip)) plot_group->setAttribute("theta_flip", theta_flip);
  if (grm_args_values(subplot_args, "keep_radii_axes", "i", &keep_radii_axes))
    plot_group->setAttribute("keep_radii_axes", keep_radii_axes);
  if (grm_args_values(*series, "draw_edges", "i", &draw_edges)) series_group->setAttribute("draw_edges", draw_edges);
  if (grm_args_values(*series, "stairs", "i", &stairs)) series_group->setAttribute("stairs", stairs);

  if (grm_args_values(subplot_args, "r_lim", "dd", &r_lim_min, &r_lim_max))
    {
      plot_group->setAttribute("r_lim_min", r_lim_min);
      plot_group->setAttribute("r_lim_max", r_lim_max);
    }

  if (grm_args_values(*series, "theta_range", "dd", &theta_range_min, &theta_range_max))
    {
      series_group->setAttribute("theta_range_min", theta_range_min);
      series_group->setAttribute("theta_range_max", theta_range_max);
    }

  if (grm_args_values(*series, "theta_colormap", "i", &theta_colormap))
    series_group->setAttribute("theta_colormap", theta_colormap);
  if (grm_args_values(*series, "r_colormap", "i", &r_colormap)) series_group->setAttribute("r_colormap", r_colormap);
  global_root->setAttribute("_id", ++id);

  return GRM_ERROR_NONE;
}

grm_error_t plotPie(grm_args_t *subplot_args)
{
  grm_args_t *series;
  double *x;
  unsigned int x_length, num_labels;
  const char *title;
  static unsigned int color_array_length = -1;
  const int *color_indices = nullptr;
  const double *color_rgb_values = nullptr;
  const char **labels;

  auto group = (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();

  grm_args_values(subplot_args, "series", "a", &series); /* series exists always */

  auto sub_group = global_render->createSeries("pie");
  group->append(sub_group);

  int id = static_cast<int>(global_root->getAttribute("_id"));
  std::string str = std::to_string(id);
  auto context = global_render->getContext();

  grm_args_first_value(series, "x", "D", &x, &x_length);

  if (x_length > 0)
    {
      std::vector<double> x_vec(x, x + x_length);
      (*context)["x" + str] = x_vec;
      sub_group->setAttribute("x", "x" + str);
    }

  if (grm_args_first_value(series, "c", "I", &color_indices, &color_array_length))
    {
      std::vector<double> color_index_vec(color_indices, color_indices + color_array_length);
      (*context)["c_ind" + str] = color_index_vec;
      sub_group->setAttribute("color_ind_values", "c_ind" + str);
    }
  else if (grm_args_first_value(series, "c", "D", &color_rgb_values, &color_array_length))
    {
      std::vector<double> color_rgb_vec(color_rgb_values, color_rgb_values + color_array_length);
      (*context)["c_rgb" + str] = color_rgb_vec;
      sub_group->setAttribute("color_rgb_values", "c_rgb" + str);
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
  if (grm_args_first_value(series, "labels", "nS", &labels, &num_labels))
    {
      std::string labels_key = "labels" + std::to_string(id);
      std::vector<std::string> labels_vec(labels, labels + num_labels);

      (*context)["labels" + str] = labels_vec;
      sub_group->setAttribute("labels", "labels" + str);
      plotDrawLegend(subplot_args);
    }
  global_root->setAttribute("_id", ++id);

  return GRM_ERROR_NONE;
}

grm_error_t plotTrisurface(grm_args_t *subplot_args)
{
  grm_args_t **current_series;

  auto group = (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();
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

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }
  plotDrawAxes(subplot_args, 2);
  plotDrawColorbar(subplot_args, PLOT_3D_COLORBAR_OFFSET, 256);

  return GRM_ERROR_NONE;
}

grm_error_t plotTricontour(grm_args_t *subplot_args)
{
  double z_min, z_max;
  int num_levels;
  grm_args_t **current_series;

  auto group = (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();

  bool has_levels = grm_args_values(subplot_args, "levels", "i", &num_levels);
  grm_args_values(subplot_args, "series", "A", &current_series);
  while (*current_series != nullptr)
    {
      double *x, *y, *z;
      unsigned int x_length, y_length, z_length;
      double x_min, x_max, y_min, y_max;
      auto sub_group = global_render->createSeries("tricontour");
      group->append(sub_group);

      grm_args_first_value(*current_series, "x", "D", &x, &x_length);
      grm_args_first_value(*current_series, "y", "D", &y, &y_length);
      grm_args_first_value(*current_series, "z", "D", &z, &z_length);

      int id = (int)global_root->getAttribute("_id");
      std::string str = std::to_string(id);
      auto context = global_render->getContext();

      std::vector<double> x_vec(x, x + x_length);
      (*context)["x" + str] = x_vec;
      sub_group->setAttribute("x", "x" + str);

      std::vector<double> y_vec(y, y + y_length);
      (*context)["y" + str] = y_vec;
      sub_group->setAttribute("y", "y" + str);

      std::vector<double> z_vec(z, z + z_length);
      (*context)["z" + str] = z_vec;
      sub_group->setAttribute("z", "z" + str);

      if (grm_args_values(*current_series, "x_range", "dd", &x_min, &x_max))
        {
          sub_group->setAttribute("x_range_min", x_min);
          sub_group->setAttribute("x_range_max", x_max);
        }
      if (grm_args_values(*current_series, "y_range", "dd", &y_min, &y_max))
        {
          sub_group->setAttribute("y_range_min", y_min);
          sub_group->setAttribute("y_range_max", y_max);
        }
      if (grm_args_values(*current_series, "z_range", "dd", &z_min, &z_max))
        {
          sub_group->setAttribute("z_range_min", z_min);
          sub_group->setAttribute("z_range_max", z_max);
        }

      if (has_levels) sub_group->setAttribute("levels", num_levels);

      global_root->setAttribute("_id", ++id);
      ++current_series;
    }
  plotDrawColorbar(subplot_args, 0.0, 256);

  return GRM_ERROR_NONE;
}

grm_error_t plotShade(grm_args_t *subplot_args)
{
  grm_args_t **current_shader;
  /* char *spec = ""; TODO: read spec from data! */
  int transformation, x_bins, y_bins;
  double *x, *y;
  unsigned int x_length, y_length;
  double x_min, x_max, y_min, y_max;

  auto group = (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();

  grm_args_values(subplot_args, "series", "A", &current_shader);
  auto sub_group = global_render->createSeries("shade");
  group->append(sub_group);

  grm_args_first_value(*current_shader, "x", "D", &x, &x_length);
  grm_args_first_value(*current_shader, "y", "D", &y, &y_length);

  int id = static_cast<int>(global_root->getAttribute("_id"));
  std::string str = std::to_string(id);
  auto context = global_render->getContext();

  std::vector<double> x_vec(x, x + x_length);
  std::vector<double> y_vec(y, y + y_length);

  (*context)["x" + str] = x_vec;
  sub_group->setAttribute("x", "x" + str);
  (*context)["y" + str] = y_vec;
  sub_group->setAttribute("y", "y" + str);

  if (grm_args_values(subplot_args, "transformation", "i", &transformation))
    {
      sub_group->setAttribute("transformation", transformation);
    }
  if (grm_args_values(subplot_args, "x_bins", "i", &x_bins)) sub_group->setAttribute("x_bins", x_bins);
  if (grm_args_values(subplot_args, "y_bins", "i", &y_bins)) sub_group->setAttribute("y_bins", y_bins);

  if (grm_args_values(*current_shader, "x_range", "dd", &x_min, &x_max))
    {
      sub_group->setAttribute("x_range_min", x_min);
      sub_group->setAttribute("x_range_max", x_max);
    }
  if (grm_args_values(*current_shader, "y_range", "dd", &y_min, &y_max))
    {
      sub_group->setAttribute("y_range_min", y_min);
      sub_group->setAttribute("y_range_max", y_max);
    }
  global_root->setAttribute("_id", ++id);

  return GRM_ERROR_NONE;
}

grm_error_t plotRaw(grm_args_t *plot_args)
{
  const char *base64_data = nullptr;
  char *graphics_data = nullptr;
  grm_error_t error = GRM_ERROR_NONE;
  std::vector<int> data_vec;

  cleanupAndSetErrorIf(!grm_args_values(plot_args, "raw", "s", &base64_data), GRM_ERROR_PLOT_MISSING_DATA);
  graphics_data = base64Decode(nullptr, base64_data, nullptr, &error);
  cleanupIfError;

  global_root->setAttribute("_clear_ws", 1);
  data_vec = std::vector<int>(graphics_data, graphics_data + strlen(graphics_data));
  edit_figure->append(global_render->createDrawGraphics("graphics", data_vec));
  global_root->setAttribute("_update_ws", 1);

cleanup:
  if (graphics_data != nullptr) free(graphics_data);
  error_code = error;

  return error;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ auxiliary drawing functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

grm_error_t plotDrawAxes(grm_args_t *args, unsigned int pass)
{
  const char *kind = nullptr;
  int x_grid, y_grid, z_grid;
  char *title;
  char *x_label, *y_label, *z_label;
  std::shared_ptr<GRM::Element> group;
  std::string type = "2d";

  auto current_central_region_element_locked = current_central_region_element.lock();

  if (!current_central_region_element_locked ||
      current_central_region_element_locked->getElementsByTagName("coordinate_system").empty())
    {
      group = global_render->createElement("coordinate_system");
      if (!current_central_region_element_locked)
        {
          getCentralRegion()->append(group);
        }
      else
        {
          current_central_region_element_locked->append(group);
        }
    }
  else
    {
      if (current_central_region_element_locked)
        group = current_central_region_element.lock()->getElementsByTagName("coordinate_system")[0];
      else
        group = getCentralRegion()->getElementsByTagName("coordinate_system")[0];
    }
  grm_args_values(args, "kind", "s", &kind);
  grm_args_values(args, "x_grid", "i", &x_grid);
  grm_args_values(args, "y_grid", "i", &y_grid);

  group->setAttribute("x_grid", x_grid);
  group->setAttribute("y_grid", y_grid);

  if (strEqualsAny(kind, "wireframe", "surface", "line3", "scatter3", "trisurface", "volume", "isosurface"))
    {
      type = "3d";
      grm_args_values(args, "z_grid", "i", &z_grid);
      group->setAttribute("z_grid", z_grid);
      if (strcmp(kind, "isosurface") == 0) group->setAttribute("hide", 1);
    }
  else
    {
      if (strcmp(kind, "imshow") == 0) group->setAttribute("hide", 1);
    }

  group->setAttribute("plot_type", type);

  if (pass == 1 && grm_args_values(args, "title", "s", &title))
    {
      auto side_region = global_render->createElement("side_region");
      current_central_region_element_locked->parentElement()->append(side_region);
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
          if (!current_central_region_element_locked->parentElement()->querySelectors(
                  "side_region[location=\"bottom\"]"))
            {
              auto side_region = global_render->createElement("side_region");
              current_central_region_element_locked->parentElement()->append(side_region);
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
          if (!current_central_region_element_locked->parentElement()->querySelectors("side_region[location=\"left\"]"))
            {
              auto side_region = global_render->createElement("side_region");
              current_central_region_element_locked->parentElement()->append(side_region);
              side_region->setAttribute("text_content", y_label);
              side_region->setAttribute("location", "left");
            }
        }
    }
  if (grm_args_values(args, "z_label", "s", &z_label))
    {
      group->setAttribute("z_label", z_label);
    }

  /* axes modifications */
  grm_args_t *axes_mod = nullptr;
  auto tick_modification_map = global_render->getTickModificationMap();

  if (grm_args_values(args, "axes_mod", "a", &axes_mod))
    {
      grm_args_iterator_t *axis_it = grm_args_iter(axes_mod);
      grm_arg_t *axis_arg;
      int axis_id;
      while ((axis_arg = axis_it->next(axis_it)) != nullptr)
        {
          logger((stderr, "Got axis name \"%s\" in \"axes_mod\"\n", axis_arg->key));
          if (!strEqualsAny(axis_arg->key, "x", "y"))
            {
              logger((stderr, "Ignoring invalid axis name \"%s\" in \"axes_mod\"\n", axis_arg->key));
              continue;
            }
          if (strcmp(axis_arg->key, "x") == 0)
            axis_id = global_render->getAxisId() + 1;
          else if (strcmp(axis_arg->key, "y") == 0)
            axis_id = global_render->getAxisId();

          grm_args_t **axis_mods;
          if (!grm_args_values(axes_mod, axis_arg->key, "A", &axis_mods))
            {
              logger((stderr, "Expected an array of sub containers for axis \"%s\" in \"axes_mod\"\n", axis_arg->key));
              continue;
            }
          grm_args_t **current_axis_mod = axis_mods;
          while (*current_axis_mod != nullptr)
            {
              double tick_value;
              if (!grm_args_values(*current_axis_mod, "tick_value", "d", &tick_value))
                {
                  int int_tick_value;
                  if (grm_args_values(*current_axis_mod, "tick_value", "i", &int_tick_value))
                    {
                      tick_value = int_tick_value;
                    }
                  else
                    {
                      logger((stderr, "Expected a number (integer or double) for \"tick_value\".\n"));
                      ++current_axis_mod;
                      continue;
                    }
                }
              logger((stderr, "Got tick value \"%lf\" for axis \"%s\"\n", tick_value, axis_arg->key));
              grm_args_iterator_t *tick_it = grm_args_iter(*current_axis_mod);
              grm_arg_t *tick_arg;
              while ((tick_arg = tick_it->next(tick_it)) != nullptr)
                {
                  // `tick_value` was read before, so ignore it in this loop
                  if (strcmp(tick_arg->key, "tick_value") == 0) continue;
                  logger((stderr, "Next tick_arg: \"%s\" on axis \"%s\"\n", tick_arg->key, axis_arg->key));
                  if (strcmp(tick_arg->key, "tick_color") == 0)
                    {
                      if (strcmp(tick_arg->value_format, "i") != 0)
                        {
                          logger((stderr, "Invalid value format \"%s\" for axis modification \"%s\", expected \"i\"\n",
                                  tick_arg->value_format, tick_arg->key));
                          continue;
                        }
                      int tick_color;
                      tick_color = *reinterpret_cast<int *>(tick_arg->value_ptr);

                      if ((*tick_modification_map)[axis_id][tick_value].count("line_color_ind") > 0)
                        (*tick_modification_map)[axis_id][tick_value]["line_color_ind"] = GRM::Value(tick_color);
                      else
                        (*tick_modification_map)[axis_id][tick_value].emplace("line_color_ind", tick_color);
                      logger((stderr, "Got tick_color \"%i\"\n", tick_color));
                    }
                  else if (strcmp(tick_arg->key, "line_spec") == 0)
                    {
                      if (strcmp(tick_arg->value_format, "s") != 0)
                        {
                          logger((stderr, "Invalid value format \"%s\" for axis modification \"%s\", expected \"s\"\n",
                                  tick_arg->value_format, tick_arg->key));
                          continue;
                        }
                      const char *line_spec;
                      line_spec = *reinterpret_cast<const char **>(tick_arg->value_ptr);

                      if ((*tick_modification_map)[axis_id][tick_value].count("line_spec") > 0)
                        (*tick_modification_map)[axis_id][tick_value]["line_spec"] = GRM::Value(line_spec);
                      else
                        (*tick_modification_map)[axis_id][tick_value].emplace("line_spec", line_spec);
                      logger((stderr, "Got line_spec \"%s\"\n", line_spec));
                    }
                  else if (strcmp(tick_arg->key, "tick_length") == 0)
                    {
                      if (strcmp(tick_arg->value_format, "d") != 0)
                        {
                          logger((stderr, "Invalid value format \"%s\" for axis modification \"%s\", expected \"d\"\n",
                                  tick_arg->value_format, tick_arg->key));
                          continue;
                        }
                      double tick_length;
                      tick_length = *reinterpret_cast<double *>(tick_arg->value_ptr);

                      if ((*tick_modification_map)[axis_id][tick_value].count("tick_size") > 0)
                        (*tick_modification_map)[axis_id][tick_value]["tick_size"] = GRM::Value(tick_length);
                      else
                        (*tick_modification_map)[axis_id][tick_value].emplace("tick_size", tick_length);
                      logger((stderr, "Got tick_length \"%lf\"\n", tick_length));
                    }
                  else if (strcmp(tick_arg->key, "text_align_horizontal") == 0)
                    {
                      if (!strEqualsAny(tick_arg->value_format, "s", "i"))
                        {
                          logger((stderr,
                                  "Invalid value format \"%s\" for axis modification \"%s\", expected \"s\" or "
                                  "\"i\"\n",
                                  tick_arg->value_format, tick_arg->key));
                          continue;
                        }
                      if (strcmp(tick_arg->value_format, "s") == 0)
                        {
                          const char *text_align_horizontal;
                          text_align_horizontal = *reinterpret_cast<const char **>(tick_arg->value_ptr);

                          if ((*tick_modification_map)[axis_id][tick_value].count("text_align_horizontal") > 0)
                            (*tick_modification_map)[axis_id][tick_value]["text_align_horizontal"] =
                                GRM::Value(text_align_horizontal);
                          else
                            (*tick_modification_map)[axis_id][tick_value].emplace("text_align_horizontal",
                                                                                  text_align_horizontal);
                          logger((stderr, "Got text_align_horizontal \"%s\"\n", text_align_horizontal));
                        }
                      else
                        {
                          int text_align_horizontal;
                          text_align_horizontal = *reinterpret_cast<int *>(tick_arg->value_ptr);

                          if ((*tick_modification_map)[axis_id][tick_value].count("text_align_horizontal") > 0)
                            (*tick_modification_map)[axis_id][tick_value]["text_align_horizontal"] =
                                GRM::Value(text_align_horizontal);
                          else
                            (*tick_modification_map)[axis_id][tick_value].emplace("text_align_horizontal",
                                                                                  text_align_horizontal);
                          logger((stderr, "Got text_align_horizontal \"%i\"\n", text_align_horizontal));
                        }
                    }
                  else if (strcmp(tick_arg->key, "text_align_vertical") == 0)
                    {
                      if (!strEqualsAny(tick_arg->value_format, "s", "i"))
                        {
                          logger((stderr,
                                  "Invalid value format \"%s\" for axis modification \"%s\", expected \"s\" or "
                                  "\"i\"\n",
                                  tick_arg->value_format, tick_arg->key));
                          continue;
                        }
                      if (strcmp(tick_arg->value_format, "s") == 0)
                        {
                          const char *text_align_vertical;
                          text_align_vertical = *reinterpret_cast<const char **>(tick_arg->value_ptr);

                          if ((*tick_modification_map)[axis_id][tick_value].count("text_align_vertical") > 0)
                            (*tick_modification_map)[axis_id][tick_value]["text_align_vertical"] =
                                GRM::Value(text_align_vertical);
                          else
                            (*tick_modification_map)[axis_id][tick_value].emplace("text_align_vertical",
                                                                                  text_align_vertical);
                          logger((stderr, "Got text_align_vertical \"%s\"\n", text_align_vertical));
                        }
                      else
                        {
                          int text_align_vertical;
                          text_align_vertical = *reinterpret_cast<int *>(tick_arg->value_ptr);

                          if ((*tick_modification_map)[axis_id][tick_value].count("text_align_vertical") > 0)
                            (*tick_modification_map)[axis_id][tick_value]["text_align_vertical"] =
                                GRM::Value(text_align_vertical);
                          else
                            (*tick_modification_map)[axis_id][tick_value].emplace("text_align_vertical",
                                                                                  text_align_vertical);
                          logger((stderr, "Got text_align_vertical \"%i\"\n", text_align_vertical));
                        }
                    }
                  else if (strcmp(tick_arg->key, "tick_label") == 0)
                    {
                      if (strcmp(tick_arg->value_format, "s") != 0)
                        {
                          logger((stderr, "Invalid value format \"%s\" for axis modification \"%s\", expected \"s\"\n",
                                  tick_arg->value_format, tick_arg->key));
                          continue;
                        }
                      const char *tick_label;
                      tick_label = *reinterpret_cast<const char **>(tick_arg->value_ptr);

                      if ((*tick_modification_map)[axis_id][tick_value].count("tick_label") > 0)
                        (*tick_modification_map)[axis_id][tick_value]["tick_label"] = GRM::Value(tick_label);
                      else
                        (*tick_modification_map)[axis_id][tick_value].emplace("tick_label", tick_label);
                      logger((stderr, "Got tick_label \"%s\"\n", tick_label));
                    }
                  else if (strcmp(tick_arg->key, "tick_width") == 0)
                    {
                      if (strcmp(tick_arg->value_format, "d") != 0)
                        {
                          logger((stderr, "Invalid value format \"%s\" for axis modification \"%s\", expected \"d\"\n",
                                  tick_arg->value_format, tick_arg->key));
                          continue;
                        }
                      double tick_width;
                      tick_width = *reinterpret_cast<double *>(tick_arg->value_ptr);

                      if ((*tick_modification_map)[axis_id][tick_value].count("tick_width") > 0)
                        (*tick_modification_map)[axis_id][tick_value]["tick_width"] = GRM::Value(tick_width);
                      else
                        (*tick_modification_map)[axis_id][tick_value].emplace("tick_width", tick_width);
                      logger((stderr, "Got tick_width \"%lf\"\n", tick_width));
                    }
                  else if (strcmp(tick_arg->key, "new_tick_value") == 0)
                    {
                      if (strcmp(tick_arg->value_format, "d") != 0)
                        {
                          logger((stderr, "Invalid value format \"%s\" for axis modification \"%s\", expected \"d\"\n",
                                  tick_arg->value_format, tick_arg->key));
                          continue;
                        }
                      double new_tick_value;
                      new_tick_value = *reinterpret_cast<double *>(tick_arg->value_ptr);

                      if ((*tick_modification_map)[axis_id][tick_value].count("value") > 0)
                        (*tick_modification_map)[axis_id][tick_value]["value"] = GRM::Value(new_tick_value);
                      else
                        (*tick_modification_map)[axis_id][tick_value].emplace("value", new_tick_value);
                      logger((stderr, "Got new_tick_value \"%lf\"\n", new_tick_value));
                    }
                  else if (strcmp(tick_arg->key, "tick_is_major") == 0)
                    {
                      if (strcmp(tick_arg->value_format, "i") != 0)
                        {
                          logger((stderr, "Invalid value format \"%s\" for axis modification \"%s\", expected \"i\"\n",
                                  tick_arg->value_format, tick_arg->key));
                          continue;
                        }
                      int tick_is_major;
                      tick_is_major = *reinterpret_cast<int *>(tick_arg->value_ptr);

                      if ((*tick_modification_map)[axis_id][tick_value].count("is_major") > 0)
                        (*tick_modification_map)[axis_id][tick_value]["is_major"] = GRM::Value(tick_is_major);
                      else
                        (*tick_modification_map)[axis_id][tick_value].emplace("is_major", tick_is_major);
                      logger((stderr, "Got tick_is_major \"%i\"\n", tick_is_major));
                    }
                  else if (strcmp(tick_arg->key, "tick_label_color") == 0)
                    {
                      if (strcmp(tick_arg->value_format, "i") != 0)
                        {
                          logger((stderr, "Invalid value format \"%s\" for axis modification \"%s\", expected \"i\"\n",
                                  tick_arg->value_format, tick_arg->key));
                          continue;
                        }
                      int tick_label_color;
                      tick_label_color = *reinterpret_cast<int *>(tick_arg->value_ptr);

                      if ((*tick_modification_map)[axis_id][tick_value].count("text_color_ind") > 0)
                        (*tick_modification_map)[axis_id][tick_value]["text_color_ind"] = GRM::Value(tick_label_color);
                      else
                        (*tick_modification_map)[axis_id][tick_value].emplace("text_color_ind", tick_label_color);
                      logger((stderr, "Got tick_label_color \"%i\"\n", tick_label_color));
                    }
                  else if (strcmp(tick_arg->key, "font") == 0)
                    {
                      if (!strEqualsAny(tick_arg->value_format, "s", "i"))
                        {
                          logger((stderr,
                                  "Invalid value format \"%s\" for axis modification \"%s\", expected \"s\" or "
                                  "\"i\"\n",
                                  tick_arg->value_format, tick_arg->key));
                          continue;
                        }
                      if (strcmp(tick_arg->value_format, "s") == 0)
                        {
                          const char *font;
                          font = *reinterpret_cast<const char **>(tick_arg->value_ptr);

                          if ((*tick_modification_map)[axis_id][tick_value].count("font") > 0)
                            (*tick_modification_map)[axis_id][tick_value]["font"] = GRM::Value(font);
                          else
                            (*tick_modification_map)[axis_id][tick_value].emplace("font", font);
                          logger((stderr, "Got font \"%s\"\n", font));
                        }
                      else
                        {
                          int font;
                          font = *reinterpret_cast<int *>(tick_arg->value_ptr);

                          if ((*tick_modification_map)[axis_id][tick_value].count("font") > 0)
                            (*tick_modification_map)[axis_id][tick_value]["font"] = GRM::Value(font);
                          else
                            (*tick_modification_map)[axis_id][tick_value].emplace("font", font);
                          logger((stderr, "Got font \"%i\"\n", font));
                        }
                    }
                  else if (strcmp(tick_arg->key, "font_precision") == 0)
                    {
                      if (!strEqualsAny(tick_arg->value_format, "s", "i"))
                        {
                          logger((stderr,
                                  "Invalid value format \"%s\" for axis modification \"%s\", expected \"s\" or "
                                  "\"i\"\n",
                                  tick_arg->value_format, tick_arg->key));
                          continue;
                        }
                      if (strcmp(tick_arg->value_format, "s") == 0)
                        {
                          const char *font_precision;
                          font_precision = *reinterpret_cast<const char **>(tick_arg->value_ptr);

                          if ((*tick_modification_map)[axis_id][tick_value].count("font_precision") > 0)
                            (*tick_modification_map)[axis_id][tick_value]["font_precision"] =
                                GRM::Value(font_precision);
                          else
                            (*tick_modification_map)[axis_id][tick_value].emplace("font_precision", font_precision);
                          logger((stderr, "Got font_precision \"%s\"\n", font_precision));
                        }
                      else
                        {
                          int font_precision;
                          font_precision = *reinterpret_cast<int *>(tick_arg->value_ptr);

                          if ((*tick_modification_map)[axis_id][tick_value].count("font_precision") > 0)
                            (*tick_modification_map)[axis_id][tick_value]["font_precision"] =
                                GRM::Value(font_precision);
                          else
                            (*tick_modification_map)[axis_id][tick_value].emplace("font_precision", font_precision);
                          logger((stderr, "Got font_precision \"%i\"\n", font_precision));
                        }
                    }
                  else if (strcmp(tick_arg->key, "scientific_format") == 0)
                    {
                      if (strcmp(tick_arg->value_format, "i") != 0)
                        {
                          logger((stderr, "Invalid value format \"%s\" for axis modification \"%s\", expected \"i\"\n",
                                  tick_arg->value_format, tick_arg->key));
                          continue;
                        }
                      int scientific_format;
                      scientific_format = *reinterpret_cast<int *>(tick_arg->value_ptr);

                      if ((*tick_modification_map)[axis_id][tick_value].count("scientific_format") > 0)
                        (*tick_modification_map)[axis_id][tick_value]["scientific_format"] =
                            GRM::Value(scientific_format);
                      else
                        (*tick_modification_map)[axis_id][tick_value].emplace("scientific_format", scientific_format);
                      logger((stderr, "Got scientific_format \"%i\"\n", scientific_format));
                    }
                  else
                    {
                      logger((stderr, "Ignoring unknown axis modification \"%s\" for tick \"%lf\" on axis \"%s\"\n",
                              tick_arg->key, tick_value, axis_arg->key));
                    }
                }
              argsIteratorDelete(tick_it);
              ++current_axis_mod;
            }
        }
      argsIteratorDelete(axis_it);
    }

  return GRM_ERROR_NONE;
}

grm_error_t plotDrawPolarAxes(grm_args_t *args)
{
  char *kind;
  int angle_ticks, theta_flip = 0;
  const char *norm, *title;
  std::shared_ptr<GRM::Element> group, sub_group;

  group = (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();
  auto current_central_region_element_locked = current_central_region_element.lock();

  if (!current_central_region_element_locked ||
      current_central_region_element_locked->getElementsByTagName("coordinate_system").empty())
    {
      sub_group = global_render->createElement("coordinate_system");
      group->append(sub_group);
    }
  else
    {
      if (current_central_region_element_locked)
        sub_group = current_central_region_element_locked->getElementsByTagName("coordinate_system")[0];
      else
        sub_group = getCentralRegion()->getElementsByTagName("coordinate_system")[0];
    }

  sub_group->setAttribute("plot_type", "polar");

  if (grm_args_values(args, "angle_ticks", "i", &angle_ticks)) sub_group->setAttribute("angle_ticks", angle_ticks);

  grm_args_values(args, "kind", "s", &kind);

  if (grm_args_values(args, "theta_flip", "i", &theta_flip)) sub_group->setAttribute("theta_flip", theta_flip);

  if (grm_args_values(args, "title", "s", &title))
    {
      auto side_region = global_render->createElement("side_region");
      group->parentElement()->append(side_region);
      side_region->setAttribute("text_content", title);
      side_region->setAttribute("location", "top");
      side_region->setAttribute("text_is_title", true);
    }

  return GRM_ERROR_NONE;
}

grm_error_t plotDrawLegend(grm_args_t *subplot_args)
{
  int location;

  auto group = (!current_dom_element.expired()) ? current_dom_element.lock() : edit_figure->lastChildElement();

  auto legend = group->querySelectors("legend");
  legend = global_render->createLegend(legend);
  if (grm_args_values(subplot_args, "location", "i", &location)) legend->setAttribute("location", location);
  group->append(legend);

  return GRM_ERROR_NONE;
}

grm_error_t plotDrawColorbar(grm_args_t *subplot_args, double off, unsigned int colors)
{
  int flip;

  auto group = (!current_dom_element.expired()) ? current_dom_element.lock() : edit_figure->lastChildElement();

  auto side_region = global_render->createElement("side_region");
  group->append(side_region);
  auto side_plot_region = global_render->createElement("side_plot_region");
  side_region->append(side_plot_region);
  auto colorbar = global_render->createColorbar(colors);
  side_plot_region->append(colorbar);

  colorbar->setAttribute("x_flip", 0);
  colorbar->setAttribute("y_flip", 0);
  if (grm_args_values(subplot_args, "x_flip", "i", &flip) && flip) colorbar->setAttribute("x_flip", flip);
  if (grm_args_values(subplot_args, "y_flip", "i", &flip) && flip) colorbar->setAttribute("y_flip", flip);

  side_region->setAttribute("offset", off + PLOT_DEFAULT_COLORBAR_OFFSET);
  colorbar->setAttribute("char_height", PLOT_DEFAULT_COLORBAR_CHAR_HEIGHT);
  side_region->setAttribute("location", PLOT_DEFAULT_COLORBAR_LOCATION);
  side_region->setAttribute("width", PLOT_DEFAULT_COLORBAR_WIDTH);

  return GRM_ERROR_NONE;
}

grm_error_t extractMultiTypeArgument(grm_args_t *error_container, const char *key, unsigned int x_length,
                                     unsigned int *downwards_length, unsigned int *upwards_length, double **downwards,
                                     double **upwards, double *downwards_flt, double *upwards_flt)
{
  grm_arg_t *arg_ptr;
  grm_args_value_iterator_t *value_it;
  grm_error_t error = GRM_ERROR_NONE;
  unsigned int length;
  int i, *ii;

  arg_ptr = argsAt(error_container, key);
  if (!arg_ptr) return GRM_ERROR_NONE;
  if (strcmp(arg_ptr->value_format, "nDnD") == 0)
    {
      value_it = grm_arg_value_iter(arg_ptr);
      argsValueIteratorGet(value_it, *downwards_length, *downwards);
      argsValueIteratorGet(value_it, *upwards_length, *upwards);
      argsValueIteratorDelete(value_it);

      if (*downwards_length != *upwards_length || *downwards_length != x_length)
        {
          error = GRM_ERROR_PLOT_COMPONENT_LENGTH_MISMATCH;
          error_code = error;
          returnIfError;
        }
    }
  else if (strcmp(arg_ptr->value_format, "nD") == 0)
    {
      if (!grm_args_first_value(error_container, key, "D", downwards, downwards_length))
        {
          error = GRM_ERROR_INTERNAL;
          error_code = error;
          returnIfError;
        }
      /* Python encapsulates all single elements into an array */
      if (*downwards_length == 1)
        {
          *downwards_flt = *upwards_flt = **downwards;
          *downwards = nullptr;
          *downwards_length = 0;
          return GRM_ERROR_NONE;
        }
      if (*downwards_length != x_length)
        {
          error = GRM_ERROR_PLOT_COMPONENT_LENGTH_MISMATCH;
          error_code = error;
          returnIfError;
        }
      *upwards = *downwards;
      *upwards_length = *downwards_length;
    }
  else if (strcmp(arg_ptr->value_format, "d") == 0)
    {
      if (!grm_args_values(error_container, key, "d", downwards_flt))
        {
          error = GRM_ERROR_INTERNAL;
          error_code = error;
          returnIfError;
        }
      *upwards_flt = *downwards_flt;
    }
  else if (strcmp(arg_ptr->value_format, "nI") == 0)
    {
      if (!grm_args_first_value(error_container, key, "nI", &ii, &length))
        {
          error = GRM_ERROR_INTERNAL;
          error_code = error;
          returnIfError;
        }
      if (length != 1)
        {
          error = GRM_ERROR_PLOT_COMPONENT_LENGTH_MISMATCH;
          error_code = error;
          returnIfError;
        }
      *upwards_flt = *downwards_flt = (double)ii[0];
    }
  else if (strcmp(arg_ptr->value_format, "i") == 0)
    {
      if (!grm_args_values(error_container, key, "i", &i))
        {
          error = GRM_ERROR_INTERNAL;
          error_code = error;
          returnIfError;
        }
      *upwards_flt = *downwards_flt = (double)i;
    }
  return GRM_ERROR_NONE;
}

grm_error_t plotDrawErrorBars(grm_args_t *series_args, unsigned int x_length)
{
  grm_args_t *error_container;
  grm_arg_t *arg_ptr;
  grm_error_t error;
  int error_bar_style;

  double *absolute_upwards = nullptr, *absolute_downwards = nullptr, *relative_upwards = nullptr,
         *relative_downwards = nullptr;
  double absolute_upwards_flt, relative_upwards_flt, absolute_downwards_flt, relative_downwards_flt;
  unsigned int upwards_length, downwards_length, i;
  int color_upwards_cap, color_downwards_cap, color_error_bar;

  auto group = (!current_central_region_element.expired()) ? current_central_region_element.lock()->lastChildElement()
                                                           : getCentralRegion()->lastChildElement();

  arg_ptr = argsAt(series_args, "error");
  if (!arg_ptr) return GRM_ERROR_NONE;
  error_container = nullptr;

  auto sub_group = global_render->createElement("error_bars");
  group->append(sub_group);

  int id = static_cast<int>(global_root->getAttribute("_id"));
  std::string str = std::to_string(id);
  global_root->setAttribute("_id", ++id);
  auto context = global_render->getContext();

  if (strcmp(arg_ptr->value_format, "a") == 0 || strcmp(arg_ptr->value_format, "nA") == 0)
    {
      if (!grm_args_values(series_args, "error", "a", &error_container))
        {
          error = GRM_ERROR_INTERNAL;
          error_code = error;
          returnIfError;
        }

      error = extractMultiTypeArgument(error_container, "absolute", x_length, &downwards_length, &upwards_length,
                                       &absolute_downwards, &absolute_upwards, &absolute_downwards_flt,
                                       &absolute_upwards_flt);
      error_code = error;
      returnIfError;
      error = extractMultiTypeArgument(error_container, "relative", x_length, &downwards_length, &upwards_length,
                                       &relative_downwards, &relative_upwards, &relative_downwards_flt,
                                       &relative_upwards_flt);
      error_code = error;
      returnIfError;
    }
  else
    {
      error = extractMultiTypeArgument(series_args, "error", x_length, &downwards_length, &upwards_length,
                                       &absolute_downwards, &absolute_upwards, &absolute_downwards_flt,
                                       &absolute_upwards_flt);
      error_code = error;
      returnIfError;
    }

  if (absolute_upwards == nullptr && relative_upwards == nullptr && absolute_upwards_flt == FLT_MAX &&
      relative_upwards_flt == FLT_MAX && absolute_downwards == nullptr && relative_downwards == nullptr &&
      absolute_downwards_flt == FLT_MAX && relative_downwards_flt == FLT_MAX)
    {
      error_code = GRM_ERROR_PLOT_MISSING_DATA;
      return GRM_ERROR_PLOT_MISSING_DATA;
    }
  if (absolute_upwards != nullptr)
    {
      std::vector<double> absolute_upwards_vec(absolute_upwards, absolute_upwards + upwards_length);
      (*context)["absolute_upwards" + str] = absolute_upwards_vec;
      sub_group->setAttribute("absolute_upwards", "absolute_upwards" + str);
    }
  if (relative_upwards != nullptr)
    {
      std::vector<double> relative_upwards_vec(relative_upwards, relative_upwards + upwards_length);
      (*context)["relative_upwards" + str] = relative_upwards_vec;
      sub_group->setAttribute("relative_upwards", "relative_upwards" + str);
    }
  if (absolute_downwards != nullptr)
    {
      std::vector<double> absolute_downwards_vec(absolute_downwards, absolute_downwards + downwards_length);
      (*context)["absolute_downwards" + str] = absolute_downwards_vec;
      sub_group->setAttribute("absolute_downwards", "absolute_downwards" + str);
    }
  if (relative_downwards != nullptr)
    {
      std::vector<double> relative_downwards_vec(relative_downwards, relative_downwards + downwards_length);
      (*context)["relative_downwards" + str] = relative_downwards_vec;
      sub_group->setAttribute("relative_downwards", "relative_downwards" + str);
    }
  if (absolute_downwards_flt != FLT_MAX) sub_group->setAttribute("absolute_downwards_flt", absolute_downwards_flt);
  if (relative_downwards_flt != FLT_MAX) sub_group->setAttribute("relative_downwards_flt", relative_downwards_flt);
  if (absolute_upwards_flt != FLT_MAX) sub_group->setAttribute("absolute_upwards_flt", absolute_upwards_flt);
  if (relative_upwards_flt != FLT_MAX) sub_group->setAttribute("relative_upwards_flt", relative_upwards_flt);

  if (grm_args_values(series_args, "error_bar_style", "i", &error_bar_style))
    {
      sub_group->setAttribute("error_bar_style", error_bar_style);
    }

  if (error_container != nullptr)
    {
      if (grm_args_values(error_container, "upwards_cap_color", "i", &color_upwards_cap))
        sub_group->setAttribute("upwards_cap_color", color_upwards_cap);
      if (grm_args_values(error_container, "downwards_cap_color", "i", &color_downwards_cap))
        sub_group->setAttribute("downwards_cap_color", color_downwards_cap);
      if (grm_args_values(error_container, "error_bar_color", "i", &color_error_bar))
        sub_group->setAttribute("error_bar_color", color_error_bar);
    }
  sub_group->setAttribute("z_index", 3);

  return GRM_ERROR_NONE;
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ util ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

const char *getCompatibleFormat(const char *key, const char *given_format)
{
  const char **valid_formats;
  char *reduced_given_format;
  const char **current_format_ptr;
  const char *compatible_format = nullptr;
  /* First, get all valid formats */
  if (!stringArrayMapAt(type_map, key, (char ***)&valid_formats))
    {
      /* If the given key does not exist, there is no type constraint
       * -> simply return the same type that was given */
      return given_format;
    }
  /* Second, filter the given format -> remove `n` chars because they are irrelevant for the following tests */
  reduced_given_format = strFilter(given_format, "n");
  if (reduced_given_format == nullptr)
    {
      debugPrintMallocError();
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
              isHomogenousStringOfChar(reduced_given_format, tolower(*reduced_given_format)))
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

int getIdFromArgs(const grm_args_t *args, int *plot_id_ptr, int *subplot_id_ptr, int *series_id_ptr)
{
  const char *combined_id;
  int plot_id = -1, subplot_id = 0, series_id = 0;

  if (grm_args_values(args, "id", "s", &combined_id))
    {
      const char *valid_id_delims = ":.";
      int *id_ptrs[4], **current_id_ptr;
      char *copied_id_str, *current_id_str;
      size_t segment_length;
      int is_last_segment;

      id_ptrs[0] = &plot_id;
      id_ptrs[1] = &subplot_id;
      id_ptrs[2] = &series_id;
      id_ptrs[3] = nullptr;
      if ((copied_id_str = gks_strdup(combined_id)) == nullptr)
        {
          debugPrintMallocError();
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
              if (!strToUint(current_id_str, (unsigned int *)*current_id_ptr))
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
      grm_args_values(args, "plot_id", "i", &plot_id);
      grm_args_values(args, "subplot_id", "i", &subplot_id);
      grm_args_values(args, "series_id", "i", &series_id);
    }
  /* plot id `0` references the first plot object (implicit object) -> handle it as the first plot and shift all ids by
   * one */
  *plot_id_ptr = plot_id + 1;
  *subplot_id_ptr = subplot_id;
  *series_id_ptr = series_id;

  return plot_id >= 0 || subplot_id > 0 || series_id > 0;
}

grm_args_t *getSubplotFromNdcPoint(double x, double y)
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

grm_args_t *getSubplotFromNdcPoints(unsigned int n, const double *x, const double *y)
{
  grm_args_t *subplot_args;
  unsigned int i;

  for (i = 0, subplot_args = nullptr; i < n && subplot_args == nullptr; ++i)
    {
      subplot_args = getSubplotFromNdcPoint(x[i], y[i]);
    }

  return subplot_args;
}

/*
 * Calculates the counts for polar histogram
 * */
grm_error_t countsPolarHistogram(grm_args_t *subplot_args)
{
  unsigned int num_bins;
  unsigned int length, num_bin_edges, dummy;
  const char *norm = "count";
  double *bin_edges = nullptr, *theta_data_lim = nullptr, *theta = nullptr;
  double bin_width;
  int is_bin_counts;
  int *bin_counts = nullptr;
  grm_args_t **series;
  grm_error_t error = GRM_ERROR_NONE;

  auto plot_group = edit_figure->lastChildElement();
  auto series_group = (!current_central_region_element.expired())
                          ? current_central_region_element.lock()->lastChildElement()
                          : getCentralRegion()->lastChildElement();

  auto context = global_render->getContext();


  auto id = static_cast<int>(global_root->getAttribute("_id"));
  global_root->setAttribute("_id", ++id);
  auto str = std::to_string(id);

  grm_args_values(subplot_args, "series", "A", &series);

  /* get x or bin_counts */
  if (grm_args_values(*series, "bin_counts", "i", &is_bin_counts) == 0)
    {
      is_bin_counts = 0;
      grm_args_first_value(*series, "theta", "D", &theta, &length);
      std::vector<double> theta_vec(theta, theta + length);
      (*context)["theta" + str] = theta_vec;
      series_group->setAttribute("theta", "theta" + str);
    }
  else
    {
      grm_args_first_value(*series, "theta", "I", &bin_counts, &length);
      std::vector<int> bin_counts_vec(bin_counts, bin_counts + length);
      (*context)["bin_counts" + str] = bin_counts_vec;
      series_group->setAttribute("bin_counts", "bin_counts" + str);

      is_bin_counts = 1;
      num_bins = length;
      grm_args_push(*series, "num_bins", "i", num_bins);
      series_group->setAttribute("num_bins", static_cast<int>(num_bins));
    }

  if (grm_args_first_value(*series, "theta_data_lim", "D", &theta_data_lim, &dummy))
    {
      series_group->setAttribute("theta_data_lim_min", theta_data_lim[0]);
      series_group->setAttribute("theta_data_lim_max", theta_data_lim[1]);
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

  if (grm_args_values(subplot_args, "normalization", "s", &norm)) series_group->setAttribute("norm", norm);
  if (grm_args_values(*series, "bin_width", "d", &bin_width)) series_group->setAttribute("bin_width", bin_width);

  return error;
}

int grm_get_error_code()
{
  return error_code;
}
} /* end of extern "C" */

/* ------------------------- dump ----------------------------------------------------------------------------------- */

/*!
 * \brief Dump the current GRM context object into a file object.
 *
 * \param[in] f The file object, the serialized context will be written to.
 * \param[in] dump_encoding The encoding of the serialized context. The context is exported as JSON or BSON, but an
 *                          additional encoding step can be necessary to embed the JSON/BSON document into a file (like
 *                          an XML document). Possible values are:
 *                          - `DUMP_JSON_PLAIN`: Export as JSON and do not apply any encoding.
 *                          - `DUMP_JSON_ESCAPE_DOUBLE_MINUS`: Export as JSON and escape double minus signs (`--`) in
 *                                                             the JSON document by replacing them with `-\-`. This can
 *                                                             be used to embed the output into an XML comment.
 *                          - `DUMP_JSON_BASE64`: Export a Base64 encoded JSON document.
 *                          - `DUMP_BSON_BASE64`: Export a Base64 encoded BSON document.
 * \param[in] context_keys_to_discard The context keys which will be excluded in the dump.
 */
void dumpContext(FILE *f, DumpEncoding dump_encoding, const std::unordered_set<std::string> *context_keys_to_discard)
{
  char *base64_str = dumpContextStr(dump_encoding, context_keys_to_discard);
  fprintf(f, "%s", base64_str);
  free(base64_str);
}

/*!
 * \brief Dump the current GRM context object into a string.
 *
 * \param[in] dump_encoding The encoding of the serialized context. See `dumpContext` for details.
 * \param[in] context_keys_to_discard The context keys which will be excluded in the dump.
 * \return A C-string containing the serialized context. The caller is responsible for freeing the returned string.
 */
char *dumpContextStr(DumpEncoding dump_encoding, const std::unordered_set<std::string> *context_keys_to_discard)
{
  auto memwriter = memwriterNew();
  if (memwriter == nullptr)
    {
      debugPrintMallocError();
      return nullptr;
    }
  auto context = global_render->getContext();

  auto write_callback = (dump_encoding != DUMP_BSON_BASE64) ? toJsonWrite : toBsonWrite;
  write_callback(memwriter, "o(");
  for (auto item : *context)
    {
      std::visit(
          GRM::Overloaded{[&memwriter, &context_keys_to_discard, &write_callback](
                              std::reference_wrapper<std::pair<const std::string, std::vector<double>>> pair_ref) {
                            if (context_keys_to_discard->find(pair_ref.get().first) != context_keys_to_discard->end())
                              return;
                            std::stringstream format_stream;
                            format_stream << pair_ref.get().first << ":nD";
                            write_callback(memwriter, format_stream.str().c_str(), pair_ref.get().second.size(),
                                           pair_ref.get().second.data());
                          },
                          [&memwriter, &context_keys_to_discard, &write_callback](
                              std::reference_wrapper<std::pair<const std::string, std::vector<int>>> pair_ref) {
                            if (context_keys_to_discard->find(pair_ref.get().first) != context_keys_to_discard->end())
                              return;
                            std::stringstream format_stream;
                            format_stream << pair_ref.get().first << ":nI";
                            write_callback(memwriter, format_stream.str().c_str(), pair_ref.get().second.size(),
                                           pair_ref.get().second.data());
                          },
                          [&memwriter, &context_keys_to_discard, &write_callback](
                              std::reference_wrapper<std::pair<const std::string, std::vector<std::string>>> pair_ref) {
                            if (context_keys_to_discard->find(pair_ref.get().first) != context_keys_to_discard->end())
                              return;
                            std::stringstream format_stream;
                            format_stream << pair_ref.get().first << ":nS";
                            std::vector<const char *> c_strings;
                            c_strings.reserve(pair_ref.get().second.size());
                            for (const auto &str : pair_ref.get().second)
                              {
                                c_strings.push_back(str.c_str());
                              }
                            write_callback(memwriter, format_stream.str().c_str(), pair_ref.get().second.size(),
                                           c_strings.data());
                          }},
          item);
    }
  write_callback(memwriter, ")");
  char *encoded_string = nullptr;
  switch (dump_encoding)
    {
      grm_error_t error;

    case DUMP_JSON_ESCAPE_DOUBLE_MINUS:
      encoded_string = strdup(escapeDoubleMinus(memwriterBuf(memwriter)).c_str());
      break;
    case DUMP_JSON_BASE64:
    case DUMP_BSON_BASE64:
      encoded_string = base64Encode(nullptr, memwriterBuf(memwriter), memwriterSize(memwriter), &error);
      error_code = error;
      if (error != GRM_ERROR_NONE) logger((stderr, "Got error \"%d\" (\"%s\")!\n", error, grm_error_names[error]));
      break;
    default:
      // Plain JSON
      encoded_string = strdup(memwriterBuf(memwriter));
    }
  if (encoded_string == nullptr) debugPrintMallocError();

  memwriterDelete(memwriter);
  return encoded_string;
}

/*!
 * \brief Dump the current GRM context object into an XML comment.
 *
 * This function generates a special XML comment and embeds the context object as a JSON document. In order to avoid a
 * broken XML comment, double minus signs (`--`) in the JSON document are escaped by replacing them with `-\-`.
 *
 * \param[in] f The file object, the comment will be written into.
 * \param[in] context_keys_to_discard The context keys which will be excluded in the dump.
 */
void dumpContextAsXmlComment(FILE *f, const std::unordered_set<std::string> *context_keys_to_discard)
{
#ifndef NDEBUG
  // TODO: Undo
  // auto dump_encoding = DUMP_JSON_ESCAPE_DOUBLE_MINUS;
  auto dump_encoding = DUMP_BSON_BASE64;
#else
  auto dump_encoding = DUMP_BSON_BASE64;
#endif
  fprintf(f, "<!-- __grm_context__: ");
  dumpContext(f, dump_encoding, context_keys_to_discard);
  fprintf(f, " -->\n");
}

/*!
 * \brief Dump the current GRM context object into an XML comment. See `dumpContextAsXmlComment` for details.
 *
 * \param[in] context_keys_to_discard The context keys which will be excluded in the dump.
 * \return A C-string containing the XML comment. The caller is responsible for freeing the returned string.
 */
char *dumpContextAsXmlCommentStr(const std::unordered_set<std::string> *context_keys_to_discard)
{
  char *encoded_json_str = nullptr;
  size_t escaped_json_strlen;
  char *xml_comment = nullptr;

#ifndef NDEBUG
  // TODO: Undo
  // auto dump_encoding = DUMP_JSON_ESCAPE_DOUBLE_MINUS;
  auto dump_encoding = DUMP_BSON_BASE64;
#else
  auto dump_encoding = DUMP_BSON_BASE64;
#endif
  encoded_json_str = dumpContextStr(dump_encoding, context_keys_to_discard);
  cleanupIf(encoded_json_str == nullptr);
  escaped_json_strlen = strlen(encoded_json_str);
  /* 27 = strlen("<!-- __grm_context__:  -->") + 1 (`\0`) */
  xml_comment = static_cast<char *>(malloc(escaped_json_strlen + 27));
  cleanupIf(xml_comment == nullptr);
  strcpy(xml_comment, "<!-- __grm_context__: ");
  strcpy(xml_comment + 22, encoded_json_str);
  strcpy(xml_comment + 22 + escaped_json_strlen, " -->");
  xml_comment[escaped_json_strlen + 26] = '\0';

cleanup:
  free(encoded_json_str);

  return xml_comment;
}


/* ------------------------- load ----------------------------------------------------------------------------------- */

namespace internal
{

/*!
 * \brief Helper template function to avoid code duplication in `loadContextStr`.
 *
 * All arguments of this function are simply passed through from `loadContextStr`.
 *
 * \tparam T The type of the value which is read from the context argument container.
 * \tparam U The type of the value which is stored in the GRM context.
 */
template <typename T, typename U>
static void putValueIntoContext(grm_arg_t *context_arg, grm_args_value_iterator_t *context_arg_value_it,
                                GRM::Context &context)
{
  if (context_arg_value_it->is_array)
    {
      T *value_array = *static_cast<T **>(context_arg_value_it->value_ptr);
      context[context_arg->key] = std::vector<U>(value_array, value_array + context_arg_value_it->array_length);
    }
  else
    {
      T value = *static_cast<T *>(context_arg_value_it->value_ptr);
      context[context_arg->key] = std::vector<U>{value};
    }
};
} // namespace internal

/*!
 * \brief Load a GRM context object from a JSON string
 *
 * \param[in] context The context object to load into
 * \param[in] context_str The serialized context string to deserialize
 * \param[in] dump_encoding The encoding of the context string. Set to `DUMP_AUTO_DETECT` to detect the encoding.
 *                          WARNING: Auto-detection may not be reliable. Currently, only Base64 and double minus escape
 *                          formats can be detected. Plain JSON is always detected as double minus escape format.
 * \throw std::runtime_error
 */
void loadContextStr(GRM::Context &context, const std::string &context_str, DumpEncoding dump_encoding)
{
  const char *serialized_context;
  std::string serialized_context_tmp;
  if (dump_encoding == DUMP_AUTO_DETECT)
    {
      if (context_str[0] == '{')
        {
          dump_encoding = DUMP_JSON_ESCAPE_DOUBLE_MINUS;
        }
      else if (context_str[0] == 'e' && context_str[1] == 'y')
        {
          // Base64 encoded `{"` is `ey`
          dump_encoding = DUMP_JSON_BASE64;
        }
      else
        {
          dump_encoding = DUMP_BSON_BASE64;
        }
    }
  switch (dump_encoding)
    {
    case DUMP_JSON_ESCAPE_DOUBLE_MINUS:
      serialized_context_tmp = unescapeDoubleMinus(context_str);
      serialized_context = serialized_context_tmp.c_str();
      break;
    case DUMP_JSON_BASE64:
    case DUMP_BSON_BASE64:
      grm_error_t error;
      serialized_context = base64Decode(nullptr, context_str.c_str(), nullptr, &error);
      if (error != GRM_ERROR_NONE)
        {
          error_code = error;
          std::stringstream error_description;
          error_description << "error \"" << error << "\" (\"" << grm_error_names[error] << "\")";
          logger((stderr, "Got %s!\n", error_description.str().c_str()));
          // TODO: Throw a custom exception type when `plot.cxx` has a better C++ interface
          throw std::runtime_error("Failed to decode base64 context string (" + error_description.str() + ")");
        }
      break;
    default:
      // Plain JSON
      serialized_context = context_str.c_str();
    }
  auto context_args = grm_args_new();
  if (context_args == nullptr) throw std::runtime_error("Failed to create context args object");
  if (dump_encoding != DUMP_BSON_BASE64)
    {
      fromJsonRead(context_args, serialized_context);
    }
  else
    {
      fromBsonRead(context_args, serialized_context);
    }
  auto context_args_it = grm_args_iter(context_args);
  grm_arg_t *context_arg;
  while ((context_arg = context_args_it->next(context_args_it)))
    {
      auto context_arg_value_it = grm_arg_value_iter(context_arg);
      while (context_arg_value_it->next(context_arg_value_it) != NULL)
        {
          switch (context_arg_value_it->format)
            {
            case 'i':
              internal::putValueIntoContext<int, int>(context_arg, context_arg_value_it, context);
              break;
            case 'd':
              internal::putValueIntoContext<double, double>(context_arg, context_arg_value_it, context);
              break;
            case 's':
              internal::putValueIntoContext<char *, std::string>(context_arg, context_arg_value_it, context);
              break;
            }
        }
    }
}


/* ------------------------- xml ------------------------------------------------------------------------------------ */

#ifndef NO_XERCES_C
namespace XERCES_CPP_NAMESPACE
{

/*!
 * \brief A helper class to encode Xerces strings in UTF-8.
 *
 * This class takes a Xerces string and encodes it in UTF-8. By overloading the `<<` operator, the encoding result can
 * be used with C++ streams directly.
 */
class TranscodeToUtf8Str : public TranscodeToStr
{
public:
  TranscodeToUtf8Str(const XMLCh *in) : TranscodeToStr(in, "UTF-8") {}
};

inline std::ostream &operator<<(std::ostream &target, const TranscodeToStr &to_dump)
{
  target << to_dump.str();
  return target;
}

/*!
 * \brief Another helper class to encode Xerces strings in another encoding.
 *
 * `TranscodeToUtf8Str` is meant to be used with temporary objects in a stream expression. In contrast, objects of
 * `XMLStringBuffer` are meant to be used with permanent objects to avoid the overhead of constructing new objects for
 * every string encoding operation.
 */
class XMLStringBuffer : public XMLFormatter, private XMLFormatTarget
{
public:
  /*!
   * \brief Construct a new XMLStringBuffer object.
   *
   * \param[in] encoding The encoding to apply to Xerces strings, e.g. "UTF-8".
   */
  XMLStringBuffer(const char *encoding) : XMLFormatter(encoding, this) {}

  void writeChars(const XMLByte *const to_write, const XMLSize_t count, XMLFormatter *const formatter) override
  {
    out_buffer_.write((char *)to_write, (int)count);
  }

  /*!
   * \brief Encode a given Xerces string and return the result.
   *
   * \param[in] chars Xerces string to encode.
   * \return The encoded string.
   */
  std::string encode(std::optional<const XMLCh *> chars = std::nullopt)
  {
    if (chars) *this << *chars;
    std::string out = out_buffer_.str();
    out_buffer_.str("");
    return out;
  }

private:
  std::stringstream out_buffer_;
};


/*!
 * \brief A helper class for `FileInputSource` which manages the file reading.
 *
 * This class reads XML data from a FILE handle and decodes obfuscated internal attributes on the fly.
 */
class FileBinInputStream : public BinInputStream
{
public:
  explicit FileBinInputStream(FILE *file) : file_(file) {}

  [[nodiscard]] XMLFilePos curPos() const override { return cur_pos_; }

  /*!
   * \brief Read the requested amoutn of bytes from the wrapped XML file handle, or less if EOF is reached.
   *
   * \param[out] to_fill The buffer which is filled with the read bytes. Must be allocated by the user.
   * \param[in] max_to_read The maximum amount of bytes to read from the file. The buffer must be large enough.
   * \return The real amount of read bytes.
   */
  [[nodiscard]] XMLSize_t readBytes(XMLByte *const to_fill, const XMLSize_t max_to_read) override
  {
    size_t max_to_read_from_file = grm_max(0L, static_cast<long>(max_to_read) - static_cast<long>(buffer_.size()));
    buffer_.resize(buffer_.size() + max_to_read_from_file);
    auto read_from_file =
        fread(buffer_.data() + buffer_.size() - max_to_read_from_file, sizeof(char), max_to_read_from_file, file_);
    buffer_.resize(buffer_.size() - max_to_read_from_file + read_from_file);
    size_t look_ahead_pos = 0;
    while (true)
      {
        auto buffer_view = std::string_view(buffer_.data(), buffer_.size());
        look_ahead_pos = buffer_view.find(look_ahead_prefix_, look_ahead_pos);
        if (look_ahead_pos == std::string_view::npos)
          {
            look_ahead_pos = endsWithAnySubPrefix(buffer_view, look_ahead_prefix_);
          }

        if (look_ahead_pos != std::string_view::npos && lookAhead(buffer_, look_ahead_pos))
          {
            buffer_ = transformLookAheadBuffer(buffer_, look_ahead_pos);
          }
        else
          {
            break;
          }
      }
    size_t bytes_to_copy = grm_min(max_to_read, buffer_.size());
    std::copy(std::begin(buffer_), std::begin(buffer_) + static_cast<long>(bytes_to_copy), to_fill);
    buffer_.erase(std::begin(buffer_), std::begin(buffer_) + static_cast<long>(bytes_to_copy));

    cur_pos_ += static_cast<long>(bytes_to_copy);
    return bytes_to_copy;
  }

  [[nodiscard]] const XMLCh *getContentType() const override { return nullptr; }

protected:
  /*!
   * \brief Look ahead in the opened file to find the end of the next obfuscated internal attribute.
   *
   * \param[inout] buffer The buffer to check for an internal attribute. If it only contains a part of an internal
   *                      attribute, read more bytes from the file and append them to the buffer.
   * \param[in] look_ahead_pos The position to start the search for the next internal attribute from.
   * \return If a complete internal attribute was found.
   */
  [[nodiscard]] bool lookAhead(std::vector<char> &buffer, size_t look_ahead_pos)
  {
    const size_t chunk_size = 100;
    size_t buffer_view_start = look_ahead_pos;
    auto buffer_view = std::string_view(buffer.data(), buffer.size()).substr(buffer_view_start);
    int delimiter_count = 0;
    bool successful = false;

    auto read_from_file_at_least_once = false;
    while (true)
      {
        bool delimiter_found;
        if (delimiter_count == 0)
          {
            if ((delimiter_found = startsWith(buffer_view, look_ahead_prefix_)))
              {
                buffer_view_start += look_ahead_prefix_.size();
                buffer_view = buffer_view.substr(look_ahead_prefix_.size());
                ++delimiter_count;
              }
            else if (read_from_file_at_least_once)
              {
                // In this case the first look ahead operation could not complete the searched prefix -> abort now
                break;
              }
          }
        if (delimiter_count == 1)
          {
            if ((delimiter_found = buffer_view.find(attribute_delimiter) != std::string_view::npos)) ++delimiter_count;
          }
        if (delimiter_count == 2)
          {
            successful = true;
            break;
          }
        buffer.resize(buffer.size() + chunk_size);
        auto read_bytes = fread(buffer.data() + buffer.size() - chunk_size, sizeof(char), chunk_size, file_);
        read_from_file_at_least_once = true;
        buffer.resize(buffer.size() - chunk_size + read_bytes);
        if (read_bytes == 0) break;
        buffer_view = std::string_view(buffer.data(), buffer.size()).substr(buffer_view_start);
      }

    return successful;
  }

  /*!
   * \brief Decode the next internal attribute in the given look ahead buffer and return the result as new buffer.
   *
   * \param[in] look_ahead_buffer The buffer to operate on.
   * \param[in] start_index An optional start_index to start the search for the next internal attribute from.
   * \return A buffer containing the decoded internal attribute and the rest of the passed look ahead buffer.
   */
  [[nodiscard]] std::vector<char> transformLookAheadBuffer(const std::vector<char> &look_ahead_buffer,
                                                           size_t start_index = 0) const
  {
    auto look_ahead_buffer_view =
        std::string_view(look_ahead_buffer.data(), look_ahead_buffer.size()).substr(start_index);
    auto value_start_pos = look_ahead_buffer_view.find(attribute_delimiter) + 1;
    auto value_end_pos = look_ahead_buffer_view.find(attribute_delimiter, value_start_pos);
    auto attribute_value = std::string(look_ahead_buffer_view.substr(value_start_pos, value_end_pos - value_start_pos));
    grm_error_t error = GRM_ERROR_NONE;
    auto bson_data = std::unique_ptr<char, void (*)(void *)>(
        base64Decode(nullptr, attribute_value.c_str(), nullptr, &error), std::free);
    if (error != GRM_ERROR_NONE)
      {
        error_code = error;
        logger((stderr, "Got error \"%d\" (\"%s\")!\n", error, grm_error_names[error]));
        throw std::runtime_error("Got error \"" + std::to_string(error) + "\" (\"" + grm_error_names[error] + "\")!");
      }

    auto internal_args = std::unique_ptr<grm_args_t, void (*)(grm_args_t *)>(grm_args_new(), grm_args_delete);
    error = fromBsonRead(internal_args.get(), bson_data.get());
    if (error != GRM_ERROR_NONE)
      {
        error_code = error;
        logger((stderr, "Got error \"%d\" (\"%s\")!\n", error, grm_error_names[error]));
        throw std::runtime_error("Got error \"" + std::to_string(error) + "\" (\"" + grm_error_names[error] + "\")!");
      }
    auto args_it = std::unique_ptr<grm_args_iterator_t, void (*)(grm_args_iterator_t *)>(
        grm_args_iter(internal_args.get()), argsIteratorDelete);
    grm_arg_t *arg;
    auto transformed_attribute_value = std::stringstream();
    bool first_iteration = true;
    while ((arg = args_it->next(args_it.get())) != nullptr)
      {
        /* Check if there is already content in the string stream (after first iteration) */
        if (!first_iteration) transformed_attribute_value << " ";
        transformed_attribute_value << arg->key << "=" << attribute_delimiter;
        if (*arg->value_format)
          {
            auto value_it = std::unique_ptr<grm_args_value_iterator_t, void (*)(grm_args_value_iterator_t *)>(
                grm_arg_value_iter(arg), argsValueIteratorDelete);
            while (value_it->next(value_it.get()) != nullptr)
              {
                /*
                 * Decoded attributes must not
                 * - be arrays
                 * - be nested
                 * - be of single char type
                 * In these cases there is some internal error.
                 */
                assert(!value_it->is_array && value_it->format != 'a' && value_it->format != 'c');
                switch (value_it->format)
                  {
                  case 'i':
                    transformed_attribute_value << *static_cast<int *>(value_it->value_ptr);
                    break;
                  case 'd':
                    {
                      auto double_as_string = std::to_string(*static_cast<double *>(value_it->value_ptr));
                      if (double_as_string == "inf" || double_as_string == "-inf")
                        {
                          std::transform(std::begin(double_as_string), std::end(double_as_string),
                                         std::begin(double_as_string), [](auto c) { return std::toupper(c); });
                        }
                      else if (double_as_string == "nan")
                        {
                          double_as_string = "NaN";
                        }
                      transformed_attribute_value << double_as_string;
                    }
                    break;
                  case 's':
                    transformed_attribute_value << *static_cast<char **>(value_it->value_ptr);
                    break;
                  default:
                    /* This branch should never be reached */
                    error_code = GRM_ERROR_INTERNAL;
                    logger((stderr, "Internal error!\n"));
                    assert(false);
                  }
              }
          }
        transformed_attribute_value << attribute_delimiter;
        first_iteration = false;
      }

    auto transformed_look_ahead_buffer = std::vector<char>();
    transformed_look_ahead_buffer.reserve(start_index + transformed_attribute_value.tellp() +
                                          look_ahead_buffer_view.size() - value_end_pos - 1);
    transformed_look_ahead_buffer.insert(std::end(transformed_look_ahead_buffer), std::begin(look_ahead_buffer),
                                         std::begin(look_ahead_buffer) + static_cast<long>(start_index));
    transformed_look_ahead_buffer.insert(std::end(transformed_look_ahead_buffer),
                                         std::istreambuf_iterator<char>(transformed_attribute_value),
                                         std::istreambuf_iterator<char>());
    transformed_look_ahead_buffer.insert(std::end(transformed_look_ahead_buffer),
                                         std::begin(look_ahead_buffer_view) + value_end_pos + 1,
                                         std::end(look_ahead_buffer_view));
    return transformed_look_ahead_buffer;
  }

private:
  const char attribute_delimiter = '"';
  const std::string look_ahead_prefix_ = "internal=" + std::string(1, attribute_delimiter);
  std::vector<char> buffer_;
  XMLFilePos cur_pos_ = 0;
  FILE *file_;
};

/*!
 * \brief An adapter class to read XML data from a file.
 */
class FileInputSource : public InputSource
{
public:
  FileInputSource(FILE *file)
      : file_(file), recovered_filename_(recoverFilename()),
        system_id_transcoder_(reinterpret_cast<const XMLByte *>(recovered_filename_.c_str()),
                              recovered_filename_.length(), "UTF-8")
  {
  }

  BinInputStream *makeStream() const override { return new FileBinInputStream(file_); }

  const XMLCh *getSystemId() const override { return system_id_transcoder_.str(); }

private:
  /*!
   * \brief Recover the filename from a file descriptor.
   *
   * Xerces assigns a system id to input sources to make them distinguishable from each other. Thus, this function tries
   * to recover the filename from the given FILE object to have a suitable system id. Currently, this is only possible
   * on Unix systems with mounted `/proc` filesystem.
   *
   * \return The recovered filename if found, otherwise `<unknown>`.
   */
  std::string recoverFilename() const
  {
#ifdef __unix__
    std::stringstream proc_link_stream;
    const unsigned int maxsize = 4096;
    std::array<char, maxsize> filename;
    ssize_t readlink_bytes;

    proc_link_stream << "/proc/self/fd/" << fileno(file_);
    auto proc_link{proc_link_stream.str()};
    readlink_bytes = readlink(proc_link.c_str(), filename.data(), maxsize);
    filename[readlink_bytes] = '\0';
    if (readlink_bytes >= 0)
      {
        filename[readlink_bytes] = '\0';
        return filename.data();
      }
#endif
    return "<unknown>";
  }

  FILE *file_;
  std::string recovered_filename_;
  TranscodeFromStr system_id_transcoder_;
};

/*!
 * \brief A helper class for `StringInputSource` which reads data from a string.
 */
class StringInputStream : public BinInputStream
{
public:
  StringInputStream(const std::string &str) : str_(str), cur_pos_(0) {}

  XMLFilePos curPos() const override { return cur_pos_; }

  XMLSize_t readBytes(XMLByte *const to_fill, const XMLSize_t max_to_read) override
  {
    auto str_view = std::string_view(str_).substr(cur_pos_, max_to_read);
    memcpy(to_fill, str_view.data(), str_view.length());
    cur_pos_ += str_view.length();
    return str_view.length();
  }

  const XMLCh *getContentType() const override { return nullptr; }

private:
  const std::string &str_;
  long cur_pos_;
};

/*!
 * \brief An adapter class to read XML data from a string.
 */
class StringInputSource : public InputSource
{
public:
  StringInputSource(const std::string &str)
      : str_(str), system_id_transcoder_(reinterpret_cast<const XMLByte *>("<in-memory-string>"),
                                         strlen("<in-memory-string>"), "UTF-8")
  {
  }

  StringInputSource(std::string &&str)
      : str_(std::move(str)), system_id_transcoder_(reinterpret_cast<const XMLByte *>("<in-memory-string>"),
                                                    strlen("<in-memory-string>"), "UTF-8")
  {
  }

  BinInputStream *makeStream() const override { return new StringInputStream(str_); }

  const XMLCh *getSystemId() const override { return system_id_transcoder_.str(); }

private:
  std::string str_;
  TranscodeFromStr system_id_transcoder_;
};

/*!
 * \brief A class which manages error reporting for a SAX parser.
 */
class SaxErrorHandler : public ErrorHandler
{
public:
  SaxErrorHandler() = default;

  SaxErrorHandler(const std::string &schema_filepath) : schema_filepath_(schema_filepath), schema_invalid_(false) {}

  void warning(const SAXParseException &e) override
  {
    std::cerr << "\nWarning at file " << TranscodeToUtf8Str(e.getSystemId()) << ", line " << e.getLineNumber()
              << ", char " << e.getColumnNumber() << "\n  Message: " << TranscodeToUtf8Str(e.getMessage()) << std::endl;
  }

  void error(const SAXParseException &e) override
  {
    std::cerr << "\nError at file " << TranscodeToUtf8Str(e.getSystemId()) << ", line " << e.getLineNumber()
              << ", char " << e.getColumnNumber() << "\n  Message: " << TranscodeToUtf8Str(e.getMessage()) << std::endl;
  }

  void fatalError(const SAXParseException &e) override
  {
    auto system_id{TranscodeToUtf8Str(e.getSystemId())};
    std::cerr << "\nFatal Error at file " << system_id << ", line " << e.getLineNumber() << ", char "
              << e.getColumnNumber() << "\n  Message: " << TranscodeToUtf8Str(e.getMessage()) << std::endl;
    if (std::string(reinterpret_cast<const char *>(system_id.str())) == schema_filepath_) schema_invalid_ = true;
  }

  void resetErrors() override
  {
    if (schema_filepath_) schema_invalid_ = false;
  }

  std::optional<bool> schemaInvalid() const { return schema_invalid_; }

private:
  std::optional<std::string> schema_filepath_;
  std::optional<bool> schema_invalid_;
};

/*!
 * \brief The core class to handle SAX parsing of XML encoded graphics trees.
 *
 * This class inherits from all parser types that are needed to handle the whole parsing process.
 */
class GraphicsTreeParseHandler : public DefaultHandler, public SaxErrorHandler, public PSVIHandler
{
public:
  GraphicsTreeParseHandler(GRM::Context &context) : context_(context) {}
  ~GraphicsTreeParseHandler() {}

  void startDocument() override {}

  void endDocument() override {}

  /*!
   * \brief Handle the start of an XML tag in the document.
   *
   * This function creates a new GRM tree element and records all attributes which need to be inserted into the element.
   * All further processing is delayed until the PSVI handler is called since it has access to the XML schema and the
   * types of all attributes.
   */
  void startElement(const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname,
                    const Attributes &attributes) override
  {
    const std::string node_name = encode(qname);

    if (node_name == "root")
      {
        global_root = global_render->createElement("root");
        global_render->replaceChildren(global_root);
        current_element_ = global_root;
        insertion_parent_ = nullptr;
      }
    else
      {
        current_element_ = global_render->createElement(node_name);
      }

    XMLSize_t attribute_count = attributes.getLength();
    current_attributes_.clear();
    current_attributes_.reserve(attribute_count);
    for (XMLSize_t i = 0; i < attribute_count; i++)
      {
        current_attributes_.push_back({encode(attributes.getQName(i)), encode(attributes.getValue(i))});
      }
  }

  /*!
   * \brief Handle a closing XML tag.
   *
   * This function navigates one level up in the graphics tree hierarchy and sets this value as the new insertion
   * parent.
   */
  void endElement(const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname) override
  {
    insertion_parent_ = insertion_parent_->parentElement();
  }

  /*!
   * \brief Process comments in the XML document.
   *
   * This function parses special GRM comments to load a serialized context.
   */
  void comment(const XMLCh *const chars, const XMLSize_t length) override
  {
    std::string comment{encode(chars)};
    std::string_view comment_view{comment};
    comment_view = trim(comment_view);
    if (startsWith(comment_view, "__grm_context__:"))
      {
        comment_view.remove_prefix(16);
        comment_view = lTrim(comment_view);
        loadContextStr(context_, std::string(comment_view), DUMP_AUTO_DETECT);
      }
  }

  /*!
   * \brief Process the types of all attributes of a new XML tag.
   *
   * This handler is called after `startElement`. The type information in this method together with the previously
   * recorded element data can be used to create a new element in the graphics tree.
   */
  void handleAttributesPSVI(const XMLCh *const local_name, const XMLCh *const uri,
                            PSVIAttributeList *psvi_attributes) override
  {
    XMLSize_t attribute_count = psvi_attributes->getLength();
    for (XMLSize_t i = 0; i < attribute_count; i++)
      {
        auto attribute_declaration = psvi_attributes->getAttributePSVIAtIndex(i)->getAttributeDeclaration();
        if (attribute_declaration == nullptr) continue;

        const std::string &attribute_name = current_attributes_[i].first;
        const std::string &attribute_value = current_attributes_[i].second;
        assert(attribute_name == encode(attribute_declaration->getName()));
        const std::string attribute_type = encode(attribute_declaration->getTypeDefinition()->getName());

        std::vector<std::string> attribute_types;
        if (attribute_type == "strint")
          {
            attribute_types = {"integer", "string"};
          }
        else
          {
            attribute_types = {attribute_type};
          }

        for (const auto &attribute_type : attribute_types)
          {
            try
              {
                if (attribute_type == "integer")
                  {
                    current_element_->setAttribute(attribute_name, std::stoi(attribute_value));
                  }
                else if (attribute_type == "double")
                  {
                    current_element_->setAttribute(attribute_name, std::stod(attribute_value));
                  }
                else
                  {
                    current_element_->setAttribute(attribute_name, attribute_value);
                  }
                break;
              }
            catch (const std::invalid_argument &)
              {
              }
          }
        if (attribute_name == "active" && attribute_value == "1") global_render->setActiveFigure(current_element_);
      }

    if (insertion_parent_ != nullptr) insertion_parent_->appendChild(current_element_);
    insertion_parent_ = current_element_;
  }

  void handleElementPSVI(const XMLCh *const local_name, const XMLCh *const uri, PSVIElement *element_info) override {}

  void handlePartialElementPSVI(const XMLCh *const local_name, const XMLCh *const uri,
                                PSVIElement *element_info) override
  {
  }

  void warning(const SAXParseException &e) override { SaxErrorHandler::warning(e); }

  void error(const SAXParseException &e) override { SaxErrorHandler::error(e); }

  void fatalError(const SAXParseException &e) override { SaxErrorHandler::fatalError(e); }

  void resetErrors() override { SaxErrorHandler::resetErrors(); }

private:
  std::string encode(std::optional<const XMLCh *> chars = std::nullopt) { return xml_buffer_.encode(chars); }

  XMLStringBuffer xml_buffer_{"UTF-8"};
  GRM::Context &context_;
  std::shared_ptr<GRM::Element> insertion_parent_, current_element_;
  std::vector<std::pair<std::string, std::string>> current_attributes_;
};

/*!
 * \brief The core class to handle SAX parsing of XML schemas.
 *
 * This class inherits from all parser types that are needed to handle the whole parsing process.
 */
class SchemaParseHandler : public DefaultHandler, public SaxErrorHandler
{
public:
  SchemaParseHandler(GRM::Document &document, GRM::Document *document_to_be_merged = nullptr)
      : document_(document), document_to_be_merged_(document_to_be_merged)
  {
  }
  ~SchemaParseHandler() {}

  void startDocument() override {}

  void endDocument() override {}

  /*!
   * \brief Handle the start of an XML tag in the document.
   *
   * This function creates a new GRM tree element and adds all read attributes to the element.
   */
  void startElement(const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname,
                    const Attributes &attributes) override
  {
    const std::string node_name = encode(qname);

    if (node_name == "xs:schema")
      {
        current_gr_element_ = document_.createElement(node_name);
        document_.replaceChildren(current_gr_element_);
        insertion_parent_ = nullptr;
      }
    else
      {
        current_gr_element_ = document_.createElement(node_name);
      }

    XMLSize_t attribute_count = attributes.getLength();
    for (XMLSize_t i = 0; i < attribute_count; i++)
      {
        current_gr_element_->setAttribute(encode(attributes.getQName(i)), encode(attributes.getValue(i)));
      }
    if (node_name == "xs:schema")
      {
        // xmlns namespace attributes are not handled as attributes in this method, so add the namespace explicitly.
        current_gr_element_->setAttribute("xmlns:xs", "http://www.w3.org/2001/XMLSchema");
        current_gr_element_->setAttribute("xmlns:vc", "http://www.w3.org/2007/XMLSchema-versioning");
      }
    if (insertion_parent_ != nullptr) insertion_parent_->appendChild(current_gr_element_);
    insertion_parent_ = current_gr_element_;
  }

  /*!
   * \brief Handle a closing XML tag.
   *
   * This function navigates one level up in the graphics tree hierarchy and sets this value as the new insertion
   * parent.
   */
  void endElement(const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname) override
  {
    current_gr_element_ = insertion_parent_;
    if (document_to_be_merged_ != nullptr)
      {
        if (current_gr_element_->localName() == "xs:element")
          {
            auto element_name_attribute = current_gr_element_->getAttribute("name");
            if (element_name_attribute.isString())
              {
                std::stringstream selector;
                /*
                 * It would be better to use `xs:element[name="<name>"]` as selector, but `:element` is detected as
                 * pseudo class in the selector although it part of the name. Thus, use only `[name="<name>"]` and check
                 * if any result is an `xs:element`.
                 */
                selector << "[name=\"" << static_cast<std::string>(element_name_attribute) << "\"]";
                std::shared_ptr<GRM::Element> element_to_be_merged;
                for (const auto &matched_element : document_to_be_merged_->querySelectorsAll(selector.str()))
                  {
                    if (matched_element->localName() == "xs:element")
                      {
                        element_to_be_merged = matched_element;
                        break;
                      }
                  }
                if (element_to_be_merged) mergeElementsImpl(*current_gr_element_, *element_to_be_merged);
              }
          }
        else if (current_gr_element_->localName() == "xs:schema")
          {
            /*
             * At this point, the end of the schema document is reached. Append all attribute groups of the other
             * document now.
             */
            for (const auto &element : document_to_be_merged_->documentElement()->children())
              {
                if (element->localName() != "xs:attributeGroup") continue;
                current_gr_element_->appendChild(element);
              }
          }
      }
    insertion_parent_ = insertion_parent_->parentElement();
  }

  void warning(const SAXParseException &e) override { SaxErrorHandler::warning(e); }

  void error(const SAXParseException &e) override { SaxErrorHandler::error(e); }

  void fatalError(const SAXParseException &e) override { SaxErrorHandler::fatalError(e); }

  void resetErrors() override { SaxErrorHandler::resetErrors(); }

protected:
  static void mergeElementsImpl(GRM::Element &element, GRM::Element &element_to_be_merged)
  {
    const std::unordered_set<std::string> element_merge_whitelist{"xs:complexType"};
    for (auto &merge_child : element_to_be_merged.children())
      {
        auto found_child = false;
        if (element_merge_whitelist.find(merge_child->localName()) != element_merge_whitelist.end())
          {
            for (const auto &child : element.children())
              {
                if (child->localName() == merge_child->localName() && child->hasChildNodes() &&
                    merge_child->hasChildNodes())
                  {
                    mergeElementsImpl(*child, *merge_child);
                    found_child = true;
                    break;
                  }
              }
          }
        if (!found_child) element.appendChild(merge_child);
      }
  }

private:
  std::string encode(std::optional<const XMLCh *> chars = std::nullopt) { return xml_buffer_.encode(chars); }

  XMLStringBuffer xml_buffer_{"UTF-8"};
  GRM::Document &document_;
  GRM::Document *document_to_be_merged_;
  std::shared_ptr<GRM::Element> insertion_parent_, current_gr_element_;
};
} // namespace XERCES_CPP_NAMESPACE

extern "C" {

/*!
 * \brief Load a graphics tree from an XML file.
 *
 * \param[in] file The file object to parse from.
 * \return 1 on success, 0 on failure.
 */
int grm_load_graphics_tree(FILE *file)
{
  using namespace XERCES_CPP_NAMESPACE;

  if (plotInitStaticVariables() != GRM_ERROR_NONE) return 0;

  gr_setscale(0); // TODO: Check why scale is not restored after a render call in `render.cxx` automatically

  std::string schema_filepath{getMergedSchemaFilepath()};

  try
    {
      XMLPlatformUtils::Initialize();
    }
  catch (const XMLException &e)
    {
      std::cerr << "Error during initialization! :\n" << TranscodeToUtf8Str(e.getMessage()) << std::endl;
      return 0;
    }

  bool auto_update;
  global_render->getAutoUpdate(&auto_update);
  global_render->setAutoUpdate(false);

  XMLSize_t error_count = 0;
  {
    auto parser =
        std::unique_ptr<SAX2XMLReaderImpl>(static_cast<SAX2XMLReaderImpl *>(XMLReaderFactory::createXMLReader()));

    // Activate validation
    parser->setFeature(XMLUni::fgSAX2CoreValidation, true);
    parser->setFeature(XMLUni::fgXercesDynamic, false);
    parser->setFeature(XMLUni::fgXercesSchema, true);
    parser->setFeature(XMLUni::fgXercesSchemaFullChecking, true);
    auto schema_filepath_transcoder =
        TranscodeFromStr(reinterpret_cast<const XMLByte *>(schema_filepath.c_str()), schema_filepath.length(), "UTF-8");
    parser->setProperty(XMLUni::fgXercesSchemaExternalNoNameSpaceSchemaLocation,
                        (void *)schema_filepath_transcoder.str());

    try
      {
        GraphicsTreeParseHandler handler(*global_render->getContext());
        parser->setPSVIHandler(&handler);
        parser->setContentHandler(&handler);
        parser->setLexicalHandler(&handler);
        parser->setErrorHandler(static_cast<SaxErrorHandler *>(&handler));
        parser->parse(FileInputSource(file));
        error_count = parser->getErrorCount();
      }
    catch (const OutOfMemoryException &)
      {
        std::cerr << "OutOfMemoryException" << std::endl;
      }
    catch (const XMLException &e)
      {
        std::cerr << "\nAn error occurred\n  Error: " << TranscodeToUtf8Str(e.getMessage()) << "\n" << std::endl;
      }

  } // `parser` must be freed before `XMLPlatformUtils::Terminate()` is called

  XMLPlatformUtils::Terminate();

  edit_figure = global_render->getActiveFigure();
  global_render->setAutoUpdate(auto_update);

  return error_count == 0;
}
}

/*!
 * \brief Validate the currently loaded grapics tree against the internal XML schema definition.
 *
 * \return GRM_ERROR_NONE on success, an error code on failure.
 */
grm_error_t validateGraphicsTree(bool include_private_attributes)
{
  using namespace XERCES_CPP_NAMESPACE;

  auto schema_filepath{include_private_attributes ? getMergedSchemaFilepath()
                                                  : (std::string(getGrDir()) + PATH_SEPARATOR + SCHEMA_REL_FILEPATH)};
  if (!fileExists(schema_filepath.c_str()))
    {
      error_code = GRM_ERROR_PARSE_XML_NO_SCHEMA_FILE;
      return GRM_ERROR_PARSE_XML_NO_SCHEMA_FILE;
    }

  try
    {
      XMLPlatformUtils::Initialize();
    }
  catch (const XMLException &e)
    {
      std::cerr << "Error during initialization! :\n" << TranscodeToUtf8Str(e.getMessage()) << std::endl;
      error_code = GRM_ERROR_PARSE_XML_PARSING;
      return GRM_ERROR_PARSE_XML_PARSING;
    }

  XMLSize_t error_count = 0;
  bool schema_invalid = false;
  {
    auto parser =
        std::unique_ptr<SAX2XMLReaderImpl>(static_cast<SAX2XMLReaderImpl *>(XMLReaderFactory::createXMLReader()));

    // Activate validation
    parser->setFeature(XMLUni::fgSAX2CoreValidation, true);
    parser->setFeature(XMLUni::fgXercesDynamic, false);
    parser->setFeature(XMLUni::fgXercesSchema, true);
    parser->setFeature(XMLUni::fgXercesSchemaFullChecking, true);
    auto schema_filepath_transcoder =
        TranscodeFromStr(reinterpret_cast<const XMLByte *>(schema_filepath.c_str()), schema_filepath.length(), "UTF-8");
    parser->setProperty(XMLUni::fgXercesSchemaExternalNoNameSpaceSchemaLocation,
                        (void *)schema_filepath_transcoder.str());

    try
      {
        SaxErrorHandler error_handler(schema_filepath);
        parser->setErrorHandler(&error_handler);
        parser->parse(StringInputSource(toXML(
            global_root, GRM::SerializerOptions{"", include_private_attributes
                                                        ? GRM::SerializerOptions::InternalAttributesFormat::PLAIN
                                                        : GRM::SerializerOptions::InternalAttributesFormat::NONE})));
        error_count = parser->getErrorCount();
        schema_invalid = error_handler.schemaInvalid().value();
      }
    catch (const OutOfMemoryException &)
      {
        std::cerr << "OutOfMemoryException" << std::endl;
      }
    catch (const XMLException &e)
      {
        std::cerr << "\nAn error occurred\n  Error: " << TranscodeToUtf8Str(e.getMessage()) << "\n" << std::endl;
      }

  } // `parser` must be freed before `XMLPlatformUtils::Terminate()` is called

  XMLPlatformUtils::Terminate();

  return schema_invalid ? GRM_ERROR_PARSE_XML_INVALID_SCHEMA
                        : ((error_count == 0) ? GRM_ERROR_NONE : GRM_ERROR_PARSE_XML_FAILED_SCHEMA_VALIDATION);
}

#endif

/*!
 * \brief Validate the currently loaded graphics tree and print error messages to stderr.
 *
 * This is a helper function to make `validate_graphics_tree` more convenient to use.
 *
 * \return 1 on success, 0 on failure.
 */
bool validateGraphicsTreeWithErrorMessages()
{
#ifndef NO_XERCES_C
  grm_error_t validation_error = validateGraphicsTree(true);
  error_code = validation_error;
  if (validation_error == GRM_ERROR_NONE)
    {
      fprintf(stderr, "The internal graphics tree passed the validity check.\n");
    }
  else if (validation_error == GRM_ERROR_PARSE_XML_NO_SCHEMA_FILE)
    {
      fprintf(stderr, "No schema found, XML validation not possible!\n");
    }
  else if (validation_error == GRM_ERROR_PARSE_XML_FAILED_SCHEMA_VALIDATION)
    {
      fprintf(stderr, "Schema validation failed!\n");
      return 0;
    }
  else
    {
      fprintf(stderr, "XML validation failed with error \"%d\" (\"%s\")!\n", validation_error,
              grm_error_names[validation_error]);
      return 0;
    }
#else
  fprintf(stderr, "No Xerces-C++ support compiled in, no validation possible!\n");
#endif
  return 1;
}


#ifndef NO_XERCES_C
std::string getMergedSchemaFilepath()
{
  if (plotInitStaticVariables() != GRM_ERROR_NONE)
    {
      throw std::runtime_error("Initialization of static plot variables failed.");
    }

  std::string schema_filepath{std::string(grm_tmp_dir) + PATH_SEPARATOR + FULL_SCHEMA_FILENAME};
  if (!fileExists(schema_filepath.c_str()))
    {
      const unsigned int indent = 2;
      auto merged_schema = grm_load_graphics_tree_schema(true);
      std::ofstream schema_file{schema_filepath};
      schema_file << toXML(merged_schema, GRM::SerializerOptions{std::string(indent, ' ')});
    }

  return schema_filepath;
}
#endif

/* ========================= methods ================================================================================ */

/* ------------------------- args set ------------------------------------------------------------------------------- */

extern "C" {

DEFINE_SET_METHODS(Args, args)

int argsSetEntryCopy(ArgsSetEntry *copy, ArgsSetConstEntry entry)
{
  /* discard const because it is necessary to work on the object itself */
  /* TODO create two set types: copy and pointer version */
  *copy = (ArgsSetEntry)entry;
  return 1;
}

void argsSetEntryDelete(ArgsSetEntry entry UNUSED) {}

size_t argsSetEntryHash(ArgsSetConstEntry entry)
{
  return (size_t)entry;
}

int argsSetEntryEquals(ArgsSetConstEntry entry1, ArgsSetConstEntry entry2)
{
  return entry1 == entry2;
}

/* ------------------------- string-to-plot_func map ---------------------------------------------------------------- */

DEFINE_MAP_METHODS(PlotFunc, plotFunc)

int plotFuncMapValueCopy(PlotFunc *copy, const PlotFunc value)
{
  *copy = value;

  return 1;
}

void plotFuncMapValueDelete(PlotFunc value UNUSED) {}


/* ------------------------- string-to-args_set map ----------------------------------------------------------------- */

DEFINE_MAP_METHODS(ArgsSet, argsSet)

int argsSetMapValueCopy(ArgsSet **copy, const ArgsSet *value)
{
  /* discard const because it is necessary to work on the object itself */
  /* TODO create two map types: copy and pointer version */
  *copy = (ArgsSet *)value;

  return 1;
}

void argsSetMapValueDelete(ArgsSet *value UNUSED) {}


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
      eventQueueDelete(event_queue);
      event_queue = nullptr;
      doubleMapDelete(meters_per_unit_map);
      meters_per_unit_map = nullptr;
      stringMapDelete(fmt_map);
      fmt_map = nullptr;
      plotFuncMapDelete(plot_func_map);
      plot_func_map = nullptr;
      stringMapDelete(plot_valid_keys_map);
      plot_valid_keys_map = nullptr;
      stringArrayMapDelete(type_map);
      type_map = nullptr;
      grm_grid_delete(global_grid);
      global_grid = nullptr;
      deleteTmpDir();
      uninstallBacktraceHandlerIfEnabled();
      plot_static_variables_initialized = 0;
    }
  GRM::Render::finalize();
}

int grm_clear(void)
{
  if (plotInitStaticVariables() != GRM_ERROR_NONE) return 0;
  grm_args_clear(active_plot_args);
  if (plotInitArgsStructure(active_plot_args, plot_hierarchy_names + 1, 1) != GRM_ERROR_NONE) return 0;

  return 1;
}

namespace internal
{
/*!
 * \brief Restore backup attributes on the graphics tree when dumped with the `toXML` function.
 *
 * Use objects of this class as functor to filter attributes on the graphics tree which have a backup attribute. Rename
 * backup attributes to match the names of the deleted attributes. This filter is needed to discard tranformed context
 * data and to export the original data when the graphics tree is saved to an XML file.
 */
class RestoreBackupAttributeFilter
{
public:
  /*!
   * \brief The overloaded ()-operator to provide functor functionality.
   *
   * \param[in] attribute_name The name of attribute to filter.
   * \param[in] element The element the currently processed attribute belongs to.
   * \param[out] new_attribute_name Can be used to set a modified name for the current attribute. Leave unset or set to
   *                                `std::nullopt` to keep the current attribute name.
   * \return `true` if the attribute should be kept, `false` otherwise.
   */
  bool operator()(const std::string &attribute_name, const GRM::Element &element,
                  std::optional<std::string> &new_attribute_name)
  {
    if (attribute_name.empty()) return false;

    if (attribute_name[0] == '_')
      {
        std::optional<std::string_view> original_attribute_name = isBackupAttributeFor(attribute_name);
        if (original_attribute_name &&
            RESTORE_BACKUP_FORMAT_EXCLUDES.find(*original_attribute_name) == RESTORE_BACKUP_FORMAT_EXCLUDES.end())
          {
            new_attribute_name = *original_attribute_name;
          }
        // highlighted is an attribute which is just used for a specific view inside the grplot editor mode which
        // shouldnt be exported
        if (attribute_name == "_highlighted") return false;
        return true;
      }

    if (RESTORE_BACKUP_FORMAT_EXCLUDES.find(attribute_name) == RESTORE_BACKUP_FORMAT_EXCLUDES.end())
      {
        std::stringstream potential_backup_attribute_name_stream;
        potential_backup_attribute_name_stream << "_" << attribute_name << "_org";
        auto potential_backup_attribute_name = potential_backup_attribute_name_stream.str();
        if (element.hasAttribute(potential_backup_attribute_name))
          {
            if (element.getAttribute(attribute_name) != element.getAttribute(potential_backup_attribute_name) &&
                strEqualsAny(attribute_name, "r", "theta", "x", "y", "z"))
              {
                context_keys_to_discard_.insert(static_cast<std::string>(element.getAttribute(attribute_name)));
              }
            return false;
          }
      }
    return true;
  }

  /*!
   * \brief Get the set of context keys which should be discarded when saving the graphics tree to an XML file.
   */
  const std::unordered_set<std::string> &contextKeysToDiscard() const { return context_keys_to_discard_; }

private:
  std::unordered_set<std::string> context_keys_to_discard_;
};

/*!
 * \brief Discard attributes based on attribute name and graphics tree element.
 *
 * \param[in] attribute_name The attribute name to check.
 * \param[in] element The element the currently processed attribute belongs to.
 * \return `true` if the attribute should be kept, `false` otherwise.
 */
static bool discardAttributeFilter(const std::string &attribute_name, const GRM::Element &element)
{
  return !startsWith(attribute_name, "_bbox");
}
} // namespace internal

void grm_dump_graphics_tree(FILE *f)
{
  internal::RestoreBackupAttributeFilter restore_backup_attribute_filter;
  const unsigned int indent = 2;
  // Use a lambda around `restore_backup_attribute_filter` to make sure it is used by reference.
  fprintf(f, "%s",
          toXML(global_root,
                GRM::SerializerOptions{std::string(indent, ' '),
                                       GRM::SerializerOptions::InternalAttributesFormat::OBFUSCATED},
                [&restore_backup_attribute_filter](const std::string &attribute_name, const GRM::Element &element,
                                                   std::optional<std::string> &new_attribute_name) -> bool {
                  return restore_backup_attribute_filter(attribute_name, element, new_attribute_name) &&
                         internal::discardAttributeFilter(attribute_name, element);
                })
              .c_str());
  dumpContextAsXmlComment(f, &restore_backup_attribute_filter.contextKeysToDiscard());
}

unsigned int grm_max_plot_id(void)
{
  unsigned int args_array_length = 0;

  if (grm_args_first_value(global_root_args, "plots", "A", nullptr, &args_array_length)) --args_array_length;

  return args_array_length;
}

char *grm_dump_graphics_tree_str(void)
{
  internal::RestoreBackupAttributeFilter restore_backup_attribute_filter;
  // Use a lambda around `restore_backup_attribute_filter` to make sure it is used by reference.
  std::string graphics_tree_str =
      toXML(global_root, GRM::SerializerOptions{"", GRM::SerializerOptions::InternalAttributesFormat::OBFUSCATED},
            [&restore_backup_attribute_filter](const std::string &attribute_name, const GRM::Element &element,
                                               std::optional<std::string> &new_attribute_name) -> bool {
              return restore_backup_attribute_filter(attribute_name, element, new_attribute_name) &&
                     internal::discardAttributeFilter(attribute_name, element);
            });
  char *context_cstr = dumpContextAsXmlCommentStr(&restore_backup_attribute_filter.contextKeysToDiscard());
  char *graphics_tree_with_context_cstr =
      static_cast<char *>(malloc(graphics_tree_str.length() + strlen(context_cstr) + 1));
  strcpy(graphics_tree_with_context_cstr, graphics_tree_str.c_str());
  strcpy(graphics_tree_with_context_cstr + graphics_tree_str.length(), context_cstr);
  free(context_cstr);
  return graphics_tree_with_context_cstr;
}

int grm_merge(const grm_args_t *args)
{
  return grm_merge_extended(args, 0, nullptr);
}

int grm_merge_extended(const grm_args_t *args, int hold, const char *identificator)
{
  grm_error_t error = GRM_ERROR_NONE;

  if (plotInitStaticVariables() != GRM_ERROR_NONE) return 0;
  if (args != nullptr)
    {
      if (plotCheckForRequest(args, &error))
        {
          // If this is a request, do not process the argument container further
          processEvents();
          error_code = error;
          return error == GRM_ERROR_NONE;
        }
      error = plotMergeArgs(global_root_args, args, nullptr, nullptr, hold);
      if (error != GRM_ERROR_NONE)
        {
          error_code = error;
          return 0;
        }
      if (!getIdFromArgs(args, &last_merge_plot_id, &last_merge_subplot_id, &last_merge_series_id))
        {
          last_merge_plot_id = 0;
          last_merge_subplot_id = 0;
          last_merge_series_id = 0;
        }
      args_changed_since_last_plot = true;
    }

  processEvents();
  eventQueueEnqueueMergeEndEvent(event_queue, identificator);
  processEvents();

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

int plotProcessSubplotArgs(grm_args_t *subplot_args)
{
  PlotFunc plot_func;
  char *kind;
  int keep_aspect_ratio, location, adjust_x_lim, adjust_y_lim, only_square_aspect_ratio;
  double *subplot;
  double x_lim_min, x_lim_max, y_lim_min, y_lim_max, z_lim_min, z_lim_max, theta_lim_min, theta_lim_max, r_lim_min,
      r_lim_max;
  int grplot = 0;

  auto group = (!current_dom_element.expired()) ? current_dom_element.lock() : edit_figure->lastChildElement();
  grm_args_values(subplot_args, "kind", "s", &kind);
  if (strcmp(kind, "hist") == 0)
    {
      kind = (char *)"histogram";
      grm_args_push(subplot_args, "kind", "s", kind);
    }
  else if (strcmp(kind, "plot3") == 0)
    {
      kind = (char *)"line3";
      grm_args_push(subplot_args, "kind", "s", kind);
    }
  group->setAttribute("_kind", kind);
  logger((stderr, "Got keyword \"kind\" with value \"%s\"\n", kind));

  if ((error_code = plotPreSubplot(subplot_args)) != GRM_ERROR_NONE) return 0;

  auto central_region =
      (!current_central_region_element.expired()) ? current_central_region_element.lock() : getCentralRegion();
  if (grm_args_values(subplot_args, "keep_aspect_ratio", "i", &keep_aspect_ratio))
    {
      group->setAttribute("keep_aspect_ratio", keep_aspect_ratio);
    }
  if (grm_args_values(subplot_args, "only_square_aspect_ratio", "i", &only_square_aspect_ratio))
    {
      group->setAttribute("only_square_aspect_ratio", only_square_aspect_ratio);
    }
  if (grm_args_values(subplot_args, "location", "i", &location))
    {
      group->setAttribute("location", location);
    }
  if (grm_args_values(subplot_args, "subplot", "D", &subplot))
    {
      group->setAttribute("viewport_normalized_x_min", subplot[0]);
      group->setAttribute("viewport_normalized_x_max", subplot[1]);
      group->setAttribute("viewport_normalized_y_min", subplot[2]);
      group->setAttribute("viewport_normalized_y_max", subplot[3]);
      group->setAttribute("_viewport_normalized_x_min_org", subplot[0]);
      group->setAttribute("_viewport_normalized_x_max_org", subplot[1]);
      group->setAttribute("_viewport_normalized_y_min_org", subplot[2]);
      group->setAttribute("_viewport_normalized_y_max_org", subplot[3]);
    }

  if (strEqualsAny(kind, "polar_line", "polar_scatter", "polar_heatmap", "polar_histogram", "nonuniform_polar_heatmap"))
    {
      if (grm_args_values(subplot_args, "theta_lim", "dd", &theta_lim_min, &theta_lim_max))
        {
          group->setAttribute("theta_lim_min", theta_lim_min);
          group->setAttribute("theta_lim_max", theta_lim_max);
        }
      if (grm_args_values(subplot_args, "r_lim", "dd", &r_lim_min, &r_lim_max))
        {
          group->setAttribute("r_lim_min", r_lim_min);
          group->setAttribute("r_lim_max", r_lim_max);
        }
    }
  else
    {
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

  if (grm_args_values(subplot_args, "grplot", "i", &grplot)) group->setAttribute("grplot", grplot);

  if (!plotFuncMapAt(plot_func_map, kind, &plot_func)) return 0;
  if ((error_code = plot_func(subplot_args)) != GRM_ERROR_NONE) return 0;

  plotPostSubplot(subplot_args);
  return 1;
}

int grm_plot(const grm_args_t *args) // TODO: rename this method so the name displays the functionality better
{
  grm_args_t **current_subplot_args;
  GRM::Grid *current_grid;
  int tmp_size_i[2];
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
  int temp;
  global_render->setAutoUpdate(false);
  if (grm_args_values(global_root_args, "hold_plots", "i", &temp)) hold_figures = temp;
  if (grm_args_values(global_root_args, "append_plots", "i", &temp)) append_figures = temp;

  /* if plot_id is set - plot_id is getting stored different than other attributes inside the container */
  if (last_merge_plot_id > 0)
    {
      figure_id = last_merge_plot_id - 1;
      figure_id_given = true;
    }

  /* check if given figure_id (even default 0) already exists in the render */
  auto figure_element = global_root->querySelectors("[_figure_id=figure" + std::to_string(figure_id) + "]");

  auto last_figure = global_root->hasChildNodes() ? global_root->children().back() : nullptr;
  if (append_figures && !figure_id_given)
    {
      if (last_figure != nullptr && !last_figure->hasChildNodes())
        {
          auto figure_id_str = static_cast<std::string>(last_figure->getAttribute("_figure_id"));
          figure_id = std::stoi(figure_id_str.substr(6)); // Remove a `figure` prefix before converting
          last_figure->remove();
        }
      else
        {
          /* also set a not given figure_id for identification */
          figure_id = getFreeIdFromFigureElements();
        }
      edit_figure = global_render->createElement("figure");
      global_root->append(edit_figure);
      edit_figure->setAttribute("_figure_id", "figure" + std::to_string(figure_id));
    }
  else
    {
      if (figure_element != nullptr) figure_element->remove();
      edit_figure = global_render->createElement("figure");
      global_root->append(edit_figure);
      edit_figure->setAttribute("_figure_id", "figure" + std::to_string(figure_id));
    }
  current_dom_element.reset();
  current_central_region_element.reset();

  if (grm_args_values(active_plot_args, "raw", "s", &current_subplot_args))
    {
      plotRaw(active_plot_args);
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
      grm_args_t *edit_plot_args = args_array[figure_id];

      if (!edit_figure->hasChildNodes() || !hold_figures || (append_figures && !figure_id_given))
        plotSetAttributeDefaults(edit_plot_args);

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
      if (!edit_figure->hasChildNodes() || !hold_figures || (append_figures && !figure_id_given))
        {
          if ((error_code = plotProcessGridArguments(edit_plot_args)) != GRM_ERROR_NONE) return 0;
        }
      current_grid = reinterpret_cast<GRM::Grid *>(global_grid);
      int nrows = current_grid->getNRows();
      int ncols = current_grid->getNCols();

      plotPrePlot(edit_plot_args);
      grm_args_values(edit_plot_args, "subplots", "A", &current_subplot_args);
      if (!edit_figure->hasChildNodes() || (append_figures && !figure_id_given))
        {
          int plot_id = 0;
          if (!(nrows == 1 && ncols == 1 &&
                current_grid->getElement(0, 0) == nullptr)) // Check if Grid arguments in container
            {
              auto grid_dom_element = global_render->createLayoutGrid(*current_grid);
              edit_figure->append(grid_dom_element);

              if (!grm_iterate_grid(current_grid, grid_dom_element, plot_id)) return 0;
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
                      for (const auto &child : current_dom_element.lock()->children())
                        {
                          if (child->localName() == "central_region")
                            {
                              current_central_region_element = child;
                              break;
                            }
                        }
                    }
                  if (!plotProcessSubplotArgs(*current_subplot_args)) return 0;
                  ++plot_id;
                  ++current_subplot_args;
                }
            }
          plotPostPlot(edit_plot_args);
        }
      edit_figure = global_root->querySelectors("[_figure_id=figure" + std::to_string(active_plot_index - 1) + "]");
      global_render->setActiveFigure(edit_figure);
      global_render->render();
      global_render->setAutoUpdate(true);
    }

  processEvents();

  last_merge_plot_id = 0;
  last_merge_subplot_id = 0;
  last_merge_series_id = 0;

#ifndef NDEBUG
  logger((stderr, "root args after \"grm_plot\" (active_plot_index: %d):\n", active_plot_index - 1));
  if (loggerEnabled()) grm_dump(global_root_args, stderr);
  if (isEnvVariableEnabled(ENABLE_XML_VALIDATION_ENV_KEY.c_str()) || loggerEnabled())
    return validateGraphicsTreeWithErrorMessages();

#endif

  args_changed_since_last_plot = false;

  return 1;
}

int grm_render(void)
{
  global_render->render();
#ifndef NDEBUG
  if (isEnvVariableEnabled(ENABLE_XML_VALIDATION_ENV_KEY.c_str()) || loggerEnabled())
    return validateGraphicsTreeWithErrorMessages();
#endif
  return 1;
}

int grm_process_tree(void)
{
  global_render->processTree();
#ifndef NDEBUG
  if (isEnvVariableEnabled(ENABLE_XML_VALIDATION_ENV_KEY.c_str()) || loggerEnabled())
    return validateGraphicsTreeWithErrorMessages();
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

  auto figure_element = global_root->querySelectors("[_figure_id=figure" + std::to_string(id) + "]");
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
      edit_figure->setAttribute("_figure_id", "figure" + std::to_string(id));
      global_render->setAutoUpdate(auto_update);
      global_render->setActiveFigure(edit_figure);
    }
  else
    {
      edit_figure = figure_element;
      global_render->setActiveFigure(edit_figure);
    }

  if ((error_code = plotInitStaticVariables()) != GRM_ERROR_NONE) return 0;
  if ((error_code = plotInitArgsStructure(global_root_args, plot_hierarchy_names, id + 1)) != GRM_ERROR_NONE) return 0;
  if (!grm_args_first_value(global_root_args, "plots", "A", &args_array, &args_array_length)) return 0;
  if (id + 1 > args_array_length) return 0;

  active_plot_index = id + 1;
  active_plot_args = args_array[id];

  return 1;
}

int grm_validate(void)
{
#ifndef NO_XERCES_C
  grm_error_t validation_error = validateGraphicsTree();
  error_code = validation_error;
  return (validation_error == GRM_ERROR_NONE);
#endif
  return 0;
}
} /* end of extern "C" */

/* ========================= c++ ==================================================================================== */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ c++ xerces util ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifndef NO_XERCES_C
std::shared_ptr<GRM::Document> grm_load_graphics_tree_schema(bool with_private_attributes)
{
  using namespace XERCES_CPP_NAMESPACE;

  const std::string gr_dir{getGrDir()};
  const std::string schema_filepath{gr_dir + PATH_SEPARATOR + SCHEMA_REL_FILEPATH};
  const std::string private_schema_filepath{gr_dir + PATH_SEPARATOR + PRIVATE_SCHEMA_REL_FILEPATH};

  try
    {
      XMLPlatformUtils::Initialize();
    }
  catch (const XMLException &e)
    {
      std::cerr << "Error during initialization! :\n" << TranscodeToUtf8Str(e.getMessage()) << std::endl;
      return nullptr;
    }

  bool auto_update;
  global_render->getAutoUpdate(&auto_update);
  global_render->setAutoUpdate(false);

  std::shared_ptr<GRM::Document> private_schema_document;
  XMLSize_t error_count = 0;
  if (with_private_attributes)
    {
      private_schema_document = GRM::createDocument();
      auto parser =
          std::unique_ptr<SAX2XMLReaderImpl>(static_cast<SAX2XMLReaderImpl *>(XMLReaderFactory::createXMLReader()));

      // Deactivate validation since there is no schema to validate the schema
      parser->setFeature(XMLUni::fgSAX2CoreValidation, false);
      parser->setFeature(XMLUni::fgXercesDynamic, false);
      parser->setFeature(XMLUni::fgXercesSchema, false);
      parser->setFeature(XMLUni::fgXercesSchemaFullChecking, false);

      try
        {
          SchemaParseHandler handler(*private_schema_document);
          parser->setContentHandler(&handler);
          parser->setErrorHandler(static_cast<SaxErrorHandler *>(&handler));
          parser->parse(private_schema_filepath.c_str());
          error_count = parser->getErrorCount();
        }
      catch (const OutOfMemoryException &)
        {
          std::cerr << "OutOfMemoryException" << std::endl;
        }
      catch (const XMLException &e)
        {
          std::cerr << "\nAn error occurred\n  Error: " << TranscodeToUtf8Str(e.getMessage()) << "\n" << std::endl;
        }
    }

  std::shared_ptr<GRM::Document> schema_document;
  if (error_count == 0)
    {
      error_count = 0;
      schema_document = GRM::createDocument();
      {
        auto parser =
            std::unique_ptr<SAX2XMLReaderImpl>(static_cast<SAX2XMLReaderImpl *>(XMLReaderFactory::createXMLReader()));

        // Deactivate validation since there is no schema to validate the schema
        parser->setFeature(XMLUni::fgSAX2CoreValidation, false);
        parser->setFeature(XMLUni::fgXercesDynamic, false);
        parser->setFeature(XMLUni::fgXercesSchema, false);
        parser->setFeature(XMLUni::fgXercesSchemaFullChecking, false);

        try
          {
            SchemaParseHandler handler(*schema_document,
                                       with_private_attributes ? private_schema_document.get() : nullptr);
            parser->setContentHandler(&handler);
            parser->setErrorHandler(static_cast<SaxErrorHandler *>(&handler));
            parser->parse(schema_filepath.c_str());
            error_count = parser->getErrorCount();
          }
        catch (const OutOfMemoryException &)
          {
            std::cerr << "OutOfMemoryException" << std::endl;
          }
        catch (const XMLException &e)
          {
            std::cerr << "\nAn error occurred\n  Error: " << TranscodeToUtf8Str(e.getMessage()) << "\n" << std::endl;
          }

      } // `parser` must be freed before `XMLPlatformUtils::Terminate()` is called
    }

  XMLPlatformUtils::Terminate();

  global_render->setAutoUpdate(auto_update);

  return (error_count == 0) ? schema_document : nullptr;
}
#endif

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ c++ util ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int grm_iterate_grid(GRM::Grid *grid, const std::shared_ptr<GRM::Element> &parent_dom_element, int plot_id)
{
  std::set<GRM::GridElement *> processed_grid_elements;

  auto rows = grid->getRows();
  auto elements_to_position = grid->getElementToPosition();

  for (const auto &row : rows)
    {
      for (const auto &element : row)
        {
          if (!processed_grid_elements.count(element) && element != nullptr)
            {
              processed_grid_elements.insert(element);
              auto slice = elements_to_position.at(element);
              if (!grm_plot_helper(element, slice, parent_dom_element, plot_id++)) return 0;
            }
        }
    }
  return 1;
}


int grm_plot_helper(GRM::GridElement *grid_element, GRM::Slice *slice,
                    const std::shared_ptr<GRM::Element> &parent_dom_element, int plot_id)
{
  if (grid_element == nullptr)
    {
      std::cout << "Error: grid element is nullptr\n";
      return 0;
    }

  if (!grid_element->isGrid())
    {
      const char *kind;
      grm_args_t **current_subplot_args = &grid_element->plot_args;
      auto layout_grid_element = global_render->createLayoutGridElement(*grid_element, *slice);
      parent_dom_element->append(layout_grid_element);
      auto plot = global_render->createPlot(plot_id);
      auto central_region = global_render->createCentralRegion();
      layout_grid_element->append(plot);
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

      if (!plotProcessSubplotArgs(*current_subplot_args)) return 0;
    }
  else
    {
      auto *current_grid = reinterpret_cast<GRM::Grid *>(grid_element);

      auto grid_dom_element = global_render->createLayoutGrid(*current_grid);
      grid_dom_element->setAttribute("start_row", slice->row_start);
      grid_dom_element->setAttribute("stop_row", slice->row_stop);
      grid_dom_element->setAttribute("start_col", slice->col_start);
      grid_dom_element->setAttribute("stop_col", slice->col_stop);
      parent_dom_element->append(grid_dom_element);

      if (!grm_iterate_grid(current_grid, grid_dom_element, plot_id)) return 0;
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

int getFreeIdFromFigureElements()
{
  std::vector<std::string> given_ids;
  for (auto &fig : global_root->children())
    {
      given_ids.push_back(static_cast<std::string>(fig->getAttribute("_figure_id")));
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

std::shared_ptr<GRM::Element> getSubplotFromNdcPointUsingDomHelper(std::shared_ptr<GRM::Element> element, double x,
                                                                   double y)
{
  bool element_is_plot_group =
      (element->hasAttribute("plot_group") && static_cast<int>(element->getAttribute("plot_group")));

  if (element_is_plot_group)
    {
      double viewport[4];
      auto central_region = element->querySelectors("central_region");
      if (!GRM::Render::getViewport(central_region, &viewport[0], &viewport[1], &viewport[2], &viewport[3]))
        throw NotFoundError("Central region doesn't have a viewport but it should.\n");
      if (viewport[0] <= x && x <= viewport[1] && viewport[2] <= y && y <= viewport[3]) return element;
    }
  if (element->localName() == "layout_grid" || element->localName() == "layout_grid_element")
    {
      for (const auto &child : element->children())
        {
          std::shared_ptr<GRM::Element> subplot_element = getSubplotFromNdcPointUsingDomHelper(child, x, y);
          if (subplot_element != nullptr) return subplot_element;
        }
    }

  return nullptr;
}

std::shared_ptr<GRM::Element> grm_get_subplot_from_ndc_point_using_dom(double x, double y)
{
  edit_figure = global_render->getActiveFigure();
  if (edit_figure->hasChildNodes())
    {
      for (const auto &child : edit_figure->children())
        {
          std::shared_ptr<GRM::Element> subplot_element = getSubplotFromNdcPointUsingDomHelper(child, x, y);
          if (subplot_element != nullptr) return subplot_element;
        }
    }

  return nullptr;
}

std::shared_ptr<GRM::Element> grm_get_subplot_from_ndc_points_using_dom(unsigned int n, const double *x,
                                                                        const double *y)
{
  unsigned int i;
  std::shared_ptr<GRM::Element> subplot_element;

  for (i = 0, subplot_element = nullptr; i < n && subplot_element == nullptr; ++i)
    {
      subplot_element = grm_get_subplot_from_ndc_point_using_dom(x[i], y[i]);
    }

  return subplot_element;
}

void grm_set_attribute_on_all_subplots_helper(std::shared_ptr<GRM::Element> element, std::string attribute, int value)
{
  bool element_is_subplot_group =
      (element->hasAttribute("plot_group") && static_cast<int>(element->getAttribute("plot_group")));

  if (element->localName() == "layout_grid_element" || element_is_subplot_group)
    element->setAttribute(attribute, value);
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

int grm_get_focus_and_factor_from_dom(const int x1, const int y1, const int x2, const int y2,
                                      const int keep_aspect_ratio, double *factor_x, double *factor_y, double *focus_x,
                                      double *focus_y, std::shared_ptr<GRM::Element> &subplot_element)
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
  subplot_element = grm_get_subplot_from_ndc_points_using_dom(arraySize(ndc_box_x), ndc_box_x, ndc_box_y);
  if (subplot_element == nullptr) return 0;

  auto central_region = subplot_element->querySelectors("central_region");
  if (!GRM::Render::getViewport(central_region, &viewport[0], &viewport[1], &viewport[2], &viewport[3]))
    throw NotFoundError("Central region doesn't have a viewport but it should.\n");
  wswindow[0] = static_cast<double>(edit_figure->getAttribute("ws_window_x_min"));
  wswindow[1] = static_cast<double>(edit_figure->getAttribute("ws_window_x_max"));
  wswindow[2] = static_cast<double>(edit_figure->getAttribute("ws_window_y_min"));
  wswindow[3] = static_cast<double>(edit_figure->getAttribute("ws_window_y_max"));

  *factor_x = abs(x1 - x2) / (width * (viewport[1] - viewport[0]) / (wswindow[1] - wswindow[0]));
  *factor_y = abs(y1 - y2) / (height * (viewport[3] - viewport[2]) / (wswindow[3] - wswindow[2]));
  if (keep_aspect_ratio)
    {
      if (*factor_x <= *factor_y)
        {
          *factor_x = *factor_y;
          if (x1 > x2) ndc_left = ndc_right - *factor_x * (viewport[1] - viewport[0]);
        }
      else
        {
          *factor_y = *factor_x;
          if (y1 > y2) ndc_top = ndc_bottom + *factor_y * (viewport[3] - viewport[2]);
        }
    }
  *focus_x = (ndc_left - *factor_x * viewport[0]) / (1 - *factor_x) - (viewport[0] + viewport[1]) / 2.0;
  *focus_y = (ndc_top - *factor_y * viewport[3]) / (1 - *factor_y) - (viewport[2] + viewport[3]) / 2.0;
  return 1;
}

std::map<std::string, std::list<std::string>> grm_get_context_data()
{
  std::map<std::string, std::list<std::string>> context_data;
  auto context = global_render->getContext();
  for (auto item : *context)
    {
      std::visit(
          GRM::Overloaded{
              [&context_data](std::reference_wrapper<std::pair<const std::string, std::vector<double>>> pair_ref) {
                for (int row = 0; row < pair_ref.get().second.size(); row++)
                  {
                    context_data[pair_ref.get().first.c_str()].emplace_back(
                        std::to_string(pair_ref.get().second.data()[row]));
                  }
              },
              [&context_data](std::reference_wrapper<std::pair<const std::string, std::vector<int>>> pair_ref) {
                for (int row = 0; row < pair_ref.get().second.size(); row++)
                  {
                    context_data[pair_ref.get().first.c_str()].emplace_back(
                        std::to_string(pair_ref.get().second.data()[row]));
                  }
              },
              [&context_data](std::reference_wrapper<std::pair<const std::string, std::vector<std::string>>> pair_ref) {
                for (int row = 0; row < pair_ref.get().second.size(); row++)
                  {
                    context_data[pair_ref.get().first.c_str()].emplace_back(pair_ref.get().second.data()[row]);
                  }
              }},
          item);
    }
  return context_data;
}
