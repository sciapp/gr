#ifdef _WIN32
/*
 * Headers on Windows can define `min` and `max` as macros which causes
 * problem when using `std::min` and `std::max`
 * -> Define `NOMINMAX` to prevent the definition of these macros
 */
#define NOMINMAX
#endif

#define _USE_MATH_DEFINES


#include <functional>
#include <vector>
#include <array>
#include <set>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <cfloat>
#include <climits>
#include <grm/dom_render/graphics_tree/Element.hxx>
#include <grm/dom_render/graphics_tree/Document.hxx>
#include <grm/dom_render/graphics_tree/Value.hxx>
#include <grm/dom_render/graphics_tree/util.hxx>
#include <grm/dom_render/render.hxx>
#include <grm/dom_render/NotFoundError.hxx>
#include <grm/dom_render/context.hxx>
#include "gks.h"
#include "gr.h"
#include "gr3.h"
#include "grm/layout.hxx"
#include "grm/plot_int.h"
#include <cm.h>
#include "grm/utilcpp_int.hxx"
#include "grm/dom_render/ManageZIndex.hxx"
#include "grm/dom_render/Drawable.hxx"
#include "grm/dom_render/ManageGRContextIds.hxx"
#include "grm/dom_render/ManageCustomColorIndex.hxx"
extern "C" {
#include "grm/datatype/string_map_int.h"
}

/* ------------------------- re-implementation of x_lin/x_log ------------------------------------------------------- */

#define X_FLIP_IF(x, scale_options, xmin, xmax) \
  ((GR_OPTION_FLIP_X & (scale_options) ? (xmin) + (xmax) : 0) + (GR_OPTION_FLIP_X & (scale_options) ? -1 : 1) * (x))

#define X_LIN(x, scale_options, xmin, xmax, a, b)                                                                 \
  X_FLIP_IF((GR_OPTION_X_LOG & (scale_options) ? ((x) > 0 ? (a)*log10(x) + (b) : -FLT_MAX) : (x)), scale_options, \
            xmin, xmax)

#define X_LOG(x, scale_options, xmin, xmax, a, b)                                                                   \
  (GR_OPTION_X_LOG & (scale_options) ? (pow(10.0, (double)((X_FLIP_IF(x, scale_options, xmin, xmax) - (b)) / (a)))) \
                                     : X_FLIP_IF(x, scale_options, xmin, xmax))

std::shared_ptr<GRM::Element> global_root;
std::shared_ptr<GRM::Element> active_figure;
std::shared_ptr<GRM::Render> global_render;
std::priority_queue<std::shared_ptr<Drawable>, std::vector<std::shared_ptr<Drawable>>, CompareZIndex> z_queue;
bool z_queue_is_being_rendered = false;
std::map<std::shared_ptr<GRM::Element>, int> parent_to_context;
ManageGRContextIds gr_context_id_manager;
ManageZIndex z_index_manager;
ManageCustomColorIndex custom_color_index_manager;

//! This vector is used for storing element types which children get processed. Other types' children will be ignored
static std::set<std::string> parent_types = {
    "axes_text_group",
    "axis",
    "bar",
    "central_region",
    "colorbar",
    "coordinate_system",
    "error_bar",
    "error_bars",
    "figure",
    "integral",
    "integral_group",
    "label",
    "labels_group",
    "layout_grid",
    "layout_grid_element",
    "legend",
    "pie_segment",
    "plot",
    "polar_axes",
    "polar_bar",
    "marginal_heatmap_plot",
    "root",
    "series_barplot",
    "series_contour",
    "series_contourf",
    "series_heatmap",
    "series_hexbin",
    "series_hist",
    "series_imshow",
    "series_isosurface",
    "series_line",
    "series_nonuniform_heatmap",
    "series_nonuniform_polar_heatmap",
    "series_pie",
    "series_plot3",
    "series_polar",
    "series_polar_heatmap",
    "series_polar_histogram",
    "series_quiver",
    "series_scatter",
    "series_scatter3",
    "series_shade",
    "series_stairs",
    "series_stem",
    "series_surface",
    "series_tricontour",
    "series_trisurface",
    "series_volume",
    "series_wireframe",
    "side_region",
    "side_plot_region",
    "text_region",
    "tick_group",
};

static std::set<std::string> drawable_types = {
    "axes_3d",       "cellarray",
    "draw_arc",      "draw_graphics",
    "draw_image",    "draw_rect",
    "fill_arc",      "fill_area",
    "fill_rect",     "grid",
    "grid_3d",       "isosurface_render",
    "layout_grid",   "layout_grid_element",
    "legend",        "nonuniform_cellarray",
    "panzoom",       "polyline",
    "polyline_3d",   "polymarker",
    "polymarker_3d", "text",
    "tick",          "titles_3d",
};

static std::set<std::string> drawable_kinds = {
    "contour", "contourf", "hexbin", "isosurface", "quiver", "shade", "surface", "tricontour", "trisurface", "volume",
};

static std::set<std::string> valid_context_keys = {"absolute_downwards",
                                                   "absolute_upwards",
                                                   "bin_counts",
                                                   "bin_edges",
                                                   "bin_widths",
                                                   "bins",
                                                   "c",
                                                   "c_rgb",
                                                   "c_ind",
                                                   "classes",
                                                   "color_ind_values",
                                                   "color_rgb_values",
                                                   "data",
                                                   "directions",
                                                   "fill_color_rgb",
                                                   "indices",
                                                   "int_limits_high",
                                                   "int_limits_low",
                                                   "labels",
                                                   "line_color_rgb",
                                                   "line_color_indices",
                                                   "line_types",
                                                   "line_widths",
                                                   "marker_color_indices",
                                                   "marker_sizes",
                                                   "marker_types",
                                                   "phi",
                                                   "positions",
                                                   "px",
                                                   "py",
                                                   "pz",
                                                   "r",
                                                   "relative_downwards",
                                                   "relative_upwards",
                                                   "scales",
                                                   "specs",
                                                   "theta",
                                                   "u",
                                                   "ups",
                                                   "v",
                                                   "weights",
                                                   "x",
                                                   "xi",
                                                   "y",
                                                   "y_labels",
                                                   "z",
                                                   "z_dims"};

static std::set<std::string> polar_kinds = {
    "polar",
    "polar_histogram",
    "polar_heatmap",
    "nonuniformpolar_heatmap",
};

static std::set<std::string> kinds_3d = {
    "wireframe", "surface", "plot3", "scatter3", "trisurface", "volume", "isosurface",
};

static std::set<std::string> kinds_with_possible_label = {
    "barplot",          "contour", "contourf", "heatmap", "hexbin", "hist", "line",
    "marginal_heatmap", "quiver",  "scatter",  "shade",   "stairs", "stem", "tricontour",
};

static std::map<std::string, double> symbol_to_meters_per_unit{
    {"m", 1.0},     {"dm", 0.1},    {"cm", 0.01},  {"mm", 0.001},        {"in", 0.0254},
    {"\"", 0.0254}, {"ft", 0.3048}, {"'", 0.0254}, {"pc", 0.0254 / 6.0}, {"pt", 0.0254 / 72.0},
};

static int plot_scatter_markertypes[] = {
    GKS_K_MARKERTYPE_SOLID_CIRCLE,   GKS_K_MARKERTYPE_SOLID_TRI_UP, GKS_K_MARKERTYPE_SOLID_TRI_DOWN,
    GKS_K_MARKERTYPE_SOLID_SQUARE,   GKS_K_MARKERTYPE_SOLID_BOWTIE, GKS_K_MARKERTYPE_SOLID_HGLASS,
    GKS_K_MARKERTYPE_SOLID_DIAMOND,  GKS_K_MARKERTYPE_SOLID_STAR,   GKS_K_MARKERTYPE_SOLID_TRI_RIGHT,
    GKS_K_MARKERTYPE_SOLID_TRI_LEFT, GKS_K_MARKERTYPE_SOLID_PLUS,   GKS_K_MARKERTYPE_PENTAGON,
    GKS_K_MARKERTYPE_HEXAGON,        GKS_K_MARKERTYPE_HEPTAGON,     GKS_K_MARKERTYPE_OCTAGON,
    GKS_K_MARKERTYPE_STAR_4,         GKS_K_MARKERTYPE_STAR_5,       GKS_K_MARKERTYPE_STAR_6,
    GKS_K_MARKERTYPE_STAR_7,         GKS_K_MARKERTYPE_STAR_8,       GKS_K_MARKERTYPE_VLINE,
    GKS_K_MARKERTYPE_HLINE,          GKS_K_MARKERTYPE_OMARK,        INT_MAX};
static int *previous_scatter_marker_type = plot_scatter_markertypes;
static int *previous_line_marker_type = plot_scatter_markertypes;

static int bounding_id = 0, axis_id = 0;
static bool automatic_update = false;
static bool redraw_ws = false;
static std::map<int, std::shared_ptr<GRM::Element>> bounding_map;
static std::map<int, std::map<double, std::map<std::string, GRM::Value>>> tick_modification_map;

static string_map_entry_t kind_to_fmt[] = {{"line", "xys"},           {"hexbin", "xys"},
                                           {"polar", "xys"},          {"shade", "xys"},
                                           {"stem", "xys"},           {"stairs", "xys"},
                                           {"contour", "xyzc"},       {"contourf", "xyzc"},
                                           {"tricontour", "xyzc"},    {"trisurface", "xyzc"},
                                           {"surface", "xyzc"},       {"wireframe", "xyzc"},
                                           {"plot3", "xyzc"},         {"scatter", "xyzc"},
                                           {"scatter3", "xyzc"},      {"quiver", "xyuv"},
                                           {"heatmap", "xyzc"},       {"hist", "x"},
                                           {"barplot", "y"},          {"isosurface", "z"},
                                           {"imshow", "z"},           {"nonuniform_heatmap", "xyzc"},
                                           {"polar_histogram", "x"},  {"pie", "x"},
                                           {"volume", "z"},           {"marginal_heatmap", "xyzc"},
                                           {"polar_heatmap", "xyzc"}, {"nonuniform_polar_heatmap", "xyzc"}};

static string_map_t *fmt_map = string_map_new_with_data(array_size(kind_to_fmt), kind_to_fmt);

enum class del_values
{
  update_without_default = 0,
  update_with_default = 1,
  recreate_own_children = 2,
  recreate_all_children = 3
};

static std::map<std::string, int> colormap_string_to_int{
    {"uniform", 0},       {"temperature", 1}, {"grayscale", 2},   {"glowing", 3},    {"rainbowlike", 4},
    {"geologic", 5},      {"greenscale", 6},  {"cyanscale", 7},   {"bluescale", 8},  {"magentascale", 9},
    {"redscale", 10},     {"flame", 11},      {"brownscale", 12}, {"pilatus", 13},   {"autumn", 14},
    {"bone", 15},         {"cool", 16},       {"copper", 17},     {"gray", 18},      {"hot", 19},
    {"hsv", 20},          {"jet", 21},        {"pink", 22},       {"spectral", 23},  {"spring", 24},
    {"summer", 25},       {"winter", 26},     {"gist_earth", 27}, {"gist_heat", 28}, {"gist_ncar", 29},
    {"gist_rainbow", 30}, {"gist_stern", 31}, {"afmhot", 32},     {"brg", 33},       {"bwr", 34},
    {"coolwarm", 35},     {"cmrmap", 36},     {"cubehelix", 37},  {"gnuplot", 38},   {"gnuplot2", 39},
    {"ocean", 40},        {"rainbow", 41},    {"seismic", 42},    {"terrain", 43},   {"viridis", 44},
    {"inferno", 45},      {"plasma", 46},     {"magma", 47},
};
static std::map<std::string, int> font_string_to_int{
    {"times_roman", 101},
    {"times_italic", 102},
    {"times_bold", 103},
    {"times_bolditalic", 104},
    {"helvetica", 105},
    {"helvetica_oblique", 106},
    {"helvetica_bold", 107},
    {"helvetica_boldoblique", 108},
    {"courier", 109},
    {"courier_oblique", 110},
    {"courier_bold", 111},
    {"courier_boldoblique", 112},
    {"symbol", 113},
    {"bookman_light", 114},
    {"bookman_lightitalic", 115},
    {"bookman_demi", 116},
    {"bookman_demiitalic", 117},
    {"newcenturyschlbk_roman", 118},
    {"newcenturyschlbk_italic", 119},
    {"newcenturyschlbk_bold", 120},
    {"newcenturyschlbk_bolditalic", 121},
    {"avantgarde_book", 122},
    {"avantgarde_bookoblique", 123},
    {"avantgarde_demi", 124},
    {"avantgarde_demioblique", 125},
    {"palantino_roman", 126},
    {"palantino_italic", 127},
    {"palantino_bold", 128},
    {"palantino_bolditalic", 129},
    {"zapfchancery_mediumitalic", 130},
    {"zapfdingbats", 131},
    {"computermodern", 232},
    {"dejavusans", 233},
};
static std::map<std::string, int> font_precision_string_to_int{
    {"string", 0},
    {"char", 1},
    {"stroke", 2},
    {"outline", 3},
};
static std::map<std::string, int> line_type_string_to_int{
    {"solid", 1},        {"dashed", 2},      {"dotted", 3},      {"dashed_dotted", 4},
    {"dash_2_dot", -1},  {"dash_3_dot", -2}, {"long_dash", -3},  {"long_short_dash", -4},
    {"spaced_dash", -5}, {"spaced_dot", -6}, {"double_dot", -7}, {"triple_dot", -8},
};
static std::map<std::string, int> location_string_to_int{
    {"top_right", 1},
    {"top_left", 2},
    {"bottom_left", 3},
    {"bottom_right", 4},
    {"mid_right", 5},
    {"mid_left", 6},
    {"mid_right", 7},
    {"mid_bottom", 8},
    {"mid_top", 9},
    {"central", 10},
    {"outside_window_top_right", 11},
    {"outside_window_mid_right", 12},
    {"outside_window_bottom_right", 13},
};
static std::map<std::string, int> marker_type_string_to_int{
    {"dot", 1},
    {"plus", 2},
    {"asterisk", 3},
    {"circle", 4},
    {"diagonal_cross", 5},
    {"solid_circle", -1},
    {"triangle_up", -2},
    {"solid_tri_up", -3},
    {"triangle_down", -4},
    {"solid_tri_down", -5},
    {"square", -6},
    {"solid_square", -7},
    {"bowtie", -8},
    {"solid_bowtie", -9},
    {"hglass", -10},
    {"solid_hglass", -11},
    {"diamond", -12},
    {"solid_diamond", -13},
    {"star", -14},
    {"solid_star", -15},
    {"tri_up_down", -16},
    {"solid_tri_right", -17},
    {"solid_tri_left", -18},
    {"hollow_plus", -19},
    {"solid_plus", -20},
    {"pentagon", -21},
    {"hexagon", -22},
    {"heptagon", -23},
    {"octagon", -24},
    {"star_4", -25},
    {"star_5", -26},
    {"star_6", -27},
    {"star_7", -28},
    {"star_8", -29},
    {"vline", -30},
    {"hline", -31},
    {"omark", -32},
};
static std::map<std::string, int> scientific_format_string_to_int{
    {"textex", 2},
    {"mathtex", 3},
};
static std::map<std::string, int> text_align_horizontal_string_to_int{
    {"normal", 0},
    {"left", 1},
    {"center", 2},
    {"right", 3},
};
static std::map<std::string, int> text_align_vertical_string_to_int{
    {"normal", 0}, {"top", 1}, {"cap", 2}, {"half", 3}, {"base", 4}, {"bottom", 5},
};
static std::map<std::string, int> algorithm_string_to_int{
    {"emission", GR_VOLUME_EMISSION},
    {"absorption", GR_VOLUME_ABSORPTION},
    {"mip", GR_VOLUME_MIP},
    {"maximum", GR_VOLUME_MIP},
};
static std::map<std::string, int> model_string_to_int{
    {"rgb", 0},
    {"hsv", 1},
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ static function header ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static void processTickGroup(const std::shared_ptr<GRM::Element> &element,
                             const std::shared_ptr<GRM::Context> &context);

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ utility functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static void getPlotParent(std::shared_ptr<GRM::Element> &element)
{
  auto plot_parent = element;
  while (plot_parent->localName() != "plot")
    {
      plot_parent = plot_parent->parentElement();
    }
  element = plot_parent;
}

static bool isUniformData(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  std::string x_key, y_key, kind;
  kind = static_cast<std::string>(element->getAttribute("kind"));
  if (str_equals_any(kind, "line", "scatter", "pie", "polar", "polar_histogram", "polar_heatmap", "imshow", "hist",
                     "barplot", "stem", "stairs") ||
      std::find(kinds_3d.begin(), kinds_3d.end(), kind) != kinds_3d.end())
    return false;

  if (kind == "heatmap" && (!element->hasAttribute("x") || !element->hasAttribute("y")))
    {
      auto z_key = static_cast<std::string>(element->getAttribute("z"));
      std::vector<double> z_vec = GRM::get<std::vector<double>>((*context)[z_key]);
      if (!element->hasAttribute("x") && !element->hasAttribute("y"))
        {
          auto z_dims_key = static_cast<std::string>(element->getAttribute("z_dims"));
          auto z_dims_vec = GRM::get<std::vector<int>>((*context)[z_dims_key]);
          return z_dims_vec[0] == z_dims_vec[1];
        }
      else if (!element->hasAttribute("x"))
        {
          y_key = static_cast<std::string>(element->getAttribute("y"));
          std::vector<double> y_vec = GRM::get<std::vector<double>>((*context)[y_key]);
          return z_vec.size() / y_vec.size() == y_vec.size();
        }
      else if (!element->hasAttribute("y"))
        {
          x_key = static_cast<std::string>(element->getAttribute("x"));
          std::vector<double> x_vec = GRM::get<std::vector<double>>((*context)[x_key]);
          return z_vec.size() / x_vec.size() == x_vec.size();
        }
    }
  x_key = static_cast<std::string>(element->getAttribute("x"));
  if (kind == "hist")
    {
      y_key = static_cast<std::string>(element->getAttribute("weights"));
    }
  else
    {
      y_key = static_cast<std::string>(element->getAttribute("y"));
    }

  std::vector<double> x_vec = GRM::get<std::vector<double>>((*context)[x_key]);
  std::vector<double> y_vec = GRM::get<std::vector<double>>((*context)[y_key]);
  return x_vec.size() == y_vec.size();
}

static double getMaxViewport(const std::shared_ptr<GRM::Element> &element, bool x)
{
  double max_vp;
  int pixel_width, pixel_height;
  double metric_width, metric_height;
  auto plot_element = global_root->querySelectors("plot");

  GRM::Render::getFigureSize(&pixel_width, &pixel_height, &metric_width, &metric_height);
  auto aspect_ratio_ws = metric_width / metric_height;
  auto max_width_height = grm_max(pixel_width, pixel_height);

  if (plot_element == nullptr) return 1;
  if (x)
    {
      max_vp = (aspect_ratio_ws < 1) ? static_cast<double>(plot_element->getAttribute("_viewport_x_max_org")) : 1;
      if (!str_equals_any(element->localName(), "legend", "side_region", "text_region", "side_plot_region") &&
          element->hasAttribute("_bbox_x_max"))
        {
          max_vp -= abs(static_cast<double>(element->getAttribute("_viewport_x_max_org")) -
                        static_cast<double>(element->getAttribute("_bbox_x_max")) / max_width_height);
        }
    }
  else
    {
      max_vp = (aspect_ratio_ws > 1) ? static_cast<double>(plot_element->getAttribute("_viewport_y_max_org")) : 1;
      if (!str_equals_any(element->localName(), "legend", "marginal_heatmap_plot", "plot", "side_region",
                          "side_plot_region", "text_region") &&
          element->hasAttribute("_bbox_y_max")) // TODO: Exclude plot leads to problem with different aspect ration -
                                                // need to fix bboxes to fix this issue here
        {
          max_vp -= abs(static_cast<double>(element->getAttribute("_viewport_y_max_org")) -
                        static_cast<double>(element->getAttribute("_bbox_y_max")) / max_width_height);
        }
    }
  return max_vp;
}

static double getMinViewport(const std::shared_ptr<GRM::Element> &element, bool x)
{
  double min_vp = 0.0;
  int pixel_width, pixel_height;

  GRM::Render::getFigureSize(&pixel_width, &pixel_height, nullptr, nullptr);
  auto max_width_height = grm_max(pixel_width, pixel_height);

  if (x)
    {
      if (!str_equals_any(element->localName(), "legend", "side_region", "text_region", "side_plot_region") &&
          element->hasAttribute("_bbox_x_min"))
        {
          min_vp += abs(static_cast<double>(element->getAttribute("_viewport_x_min_org")) -
                        static_cast<double>(element->getAttribute("_bbox_x_min")) / max_width_height);
        }
    }
  else
    {
      if (!str_equals_any(element->localName(), "legend", "side_region", "text_region", "side_plot_region") &&
          element->hasAttribute("_bbox_y_min"))
        {
          min_vp += abs(static_cast<double>(element->getAttribute("_viewport_y_min_org")) -
                        static_cast<double>(element->getAttribute("_bbox_y_min")) / max_width_height);
        }
    }
  return min_vp;
}

static std::tuple<double, int> getColorbarAttributes(std::string kind, std::shared_ptr<GRM::Element> plot)
{
  double offset = 0.0;
  int colors = 256;

  if (kind == "contour")
    {
      std::shared_ptr<GRM::Element> series = plot->querySelectors("series_contour");
      if (series && series->hasAttribute("levels"))
        colors = static_cast<int>(series->getAttribute("levels"));
      else
        colors = PLOT_DEFAULT_CONTOUR_LEVELS;
    }
  if (kind == "contourf")
    {
      std::shared_ptr<GRM::Element> series = plot->querySelectors("series_contourf");
      if (series && series->hasAttribute("levels"))
        colors = static_cast<int>(series->getAttribute("levels"));
      else
        colors = PLOT_DEFAULT_CONTOUR_LEVELS;
    }
  if (kind == "polar_heatmap")
    {
      offset = PLOT_POLAR_COLORBAR_OFFSET;
    }
  if (kind == "surface" || kind == "volume" || kind == "trisurface")
    {
      offset = PLOT_3D_COLORBAR_OFFSET;
    }
  return {offset, colors};
}

static double getLightness(int color)
{
  unsigned char rgb[sizeof(int)];

  gr_inqcolor(color, (int *)rgb);
  double y = (0.2126729 * rgb[0] / 255 + 0.7151522 * rgb[1] / 255 + 0.0721750 * rgb[2] / 255);
  return 116 * pow(y / 100, 1.0 / 3) - 16;
}

// transform single coordinate (like x or y) into range (and or log scale)
// first transform into range and then log scale
static double transformCoordinate(double value, double vmin, double vmax, double rangeMin, double rangeMax,
                                  int logScale = 0)
{
  if (logScale)
    {
      double transVal, B, A; // A, B are the min and max after the y_log transformation
      if (!(rangeMin == 0.0 && rangeMax == 0.0))
        {
          transVal = (rangeMax - rangeMin) * (value - vmin) / (vmax - vmin) + rangeMin;
          B = rangeMax;
          A = rangeMin;
        }
      else
        {
          transVal = value;
          B = vmax;
          A = vmin;
        }
      if (transVal == 0.0) return 0;

      // negative radii -> abs -> log scale -> make negative again
      int sign = 1;
      if (transVal < 0)
        {
          transVal = abs(transVal);
          sign = -1;
        }

      double b = B / log10(vmax / vmin);
      double a = A - b * log10(vmin);
      double temp = sign * (a + b * log10(transVal));
      return sign * (a + b * log10(transVal));
    }
  return (rangeMax - rangeMin) * (value - vmin) / (vmax - vmin) + rangeMin;
}

static void transformCoordinatesVector(std::vector<double> &coords, double vmin, double vmax, double rangeMin,
                                       double rangeMax, int logScale = 0)
{
  for (auto &coord : coords)
    {
      coord = transformCoordinate(coord, vmin, vmax, rangeMin, rangeMax, logScale);
    }
}


static void resetOldBoundingBoxes(const std::shared_ptr<GRM::Element> &element)
{
  if (getenv("GRDISPLAY") && strcmp(getenv("GRDISPLAY"), "edit") == 0)
    {
      element->setAttribute("_bbox_id", -1);
      element->removeAttribute("_bbox_x_min");
      element->removeAttribute("_bbox_x_max");
      element->removeAttribute("_bbox_y_min");
      element->removeAttribute("_bbox_y_max");
    }
}

static void clearOldChildren(del_values *del, const std::shared_ptr<GRM::Element> &element)
{
  /* clear all old children of an element when del is 2 or 3, in the special case where no children exist del gets
   * manipulated so that new children will be created in caller functions*/
  if (*del != del_values::update_without_default && *del != del_values::update_with_default)
    {
      for (const auto &child : element->children())
        {
          if (*del == del_values::recreate_own_children)
            {
              if (child->hasAttribute("_child_id"))
                {
                  child->remove();
                }
              else if (element->localName() == "marginal_heatmap_plot")
                {
                  for (const auto &real_child : child->children())
                    {
                      if (real_child->hasAttribute("_child_id")) real_child->remove();
                      if (real_child->localName() == "side_plot_region")
                        {
                          for (const auto &side_plot_child : real_child->children())
                            {
                              if (side_plot_child->hasAttribute("_child_id")) side_plot_child->remove();
                            }
                        }
                    }
                }
            }
          else if (*del == del_values::recreate_all_children)
            {
              if (!(element->localName() == "marginal_heatmap_plot" &&
                    (child->localName() != "central_region" || child->localName() != "side_region")))
                child->remove();
            }
        }
    }
  else if (!element->hasChildNodes())
    *del = del_values::recreate_own_children;
  else
    {
      bool only_children_created_from_attributes = true;
      bool only_error_child = true;
      bool only_non_marginal_heatmap_children = true;
      bool only_side_plot_region_child = true;
      /* types of children the coordinate system can have that are created from attributes */
      std::vector<std::string> coordinate_system_children = {"titles_3d"};
      for (const auto &child : element->children())
        {
          if (element->localName() == "coordinate_system" &&
              std::find(coordinate_system_children.begin(), coordinate_system_children.end(), child->localName()) ==
                  coordinate_system_children.end())
            {
              only_children_created_from_attributes = false;
              break;
            }
          if (element->localName() == "marginal_heatmap_plot")
            {
              if (child->localName() == "side_region" && child->hasAttribute("marginal_heatmap_side_plot"))
                {
                  for (const auto &side_region_child : child->children())
                    {
                      if (side_region_child->localName() != "text_region")
                        {
                          only_non_marginal_heatmap_children = false;
                          break;
                        }
                    }
                }
              else if (child->localName() == "central_region")
                {
                  for (const auto &central_region_child : child->children())
                    {
                      if (central_region_child->localName() == "series_heatmap")
                        only_non_marginal_heatmap_children = false;
                    }
                }
            }
          if (element->localName() == "side_region")
            {
              for (const auto &side_region_child : element->children())
                {
                  if (side_region_child->localName() == "text_region") only_side_plot_region_child = false;
                }
            }
          if (child->localName() != "error_bars" && child->localName() != "integral_group" &&
              element->localName() != "coordinate_system" && element->localName() != "marginal_heatmap_plot")
            {
              only_error_child = false;
              break;
            }
        }
      if (element->localName() == "coordinate_system" && only_children_created_from_attributes)
        *del = del_values::recreate_own_children;
      if (starts_with(element->localName(), "series_") && only_error_child) *del = del_values::recreate_own_children;
      if (element->localName() == "marginal_heatmap_plot" && only_non_marginal_heatmap_children)
        *del = del_values::recreate_own_children;
      if (element->localName() == "side_region" && only_side_plot_region_child)
        *del = del_values::recreate_own_children;
    }
}

static void legendSize(const std::shared_ptr<GRM::Element> &element, double *w, double *h)
{
  double tbx[4], tby[4];
  unsigned int num_labels;
  std::vector<std::string> labels;

  *w = 0;
  *h = 0;

  if (auto render = std::dynamic_pointer_cast<GRM::Render>(element->ownerDocument()))
    {
      auto context = render->getContext();
      auto key = static_cast<std::string>(element->getAttribute("labels"));
      labels = GRM::get<std::vector<std::string>>((*context)[key]);
    }

  for (auto current_label : labels)
    {
      gr_inqtext(0, 0, current_label.data(), tbx, tby);
      *w = grm_max(*w, tbx[2] - tbx[0]);
      *h += grm_max(tby[2] - tby[0], 0.03);
    }
}

static void legendSize(const std::vector<std::string> &labels, double *w, double *h)
{
  double tbx[4], tby[4];
  unsigned int num_labels;

  *w = 0;
  *h = 0;

  if (!labels.empty())
    {
      for (auto current_label : labels)
        {
          gr_inqtext(0, 0, current_label.data(), tbx, tby);
          *w = grm_max(*w, tbx[2] - tbx[0]);
          *h += grm_max(tby[2] - tby[0], 0.03);
        }
    }
}

static void sidePlotMargin(const std::shared_ptr<GRM::Element> side_region, double *margin, double inc,
                           bool aspect_ratio_scale, double aspect_ratio_ws, double start_aspect_ratio_ws)
{
  if (side_region->querySelectors("side_plot_region") ||
      (side_region->hasAttribute("marginal_heatmap_side_plot") &&
       static_cast<int>(side_region->getAttribute("marginal_heatmap_side_plot"))))
    {
      *margin += inc;
      if (aspect_ratio_scale)
        {
          if (aspect_ratio_ws > start_aspect_ratio_ws)
            {
              *margin /= (start_aspect_ratio_ws / aspect_ratio_ws);
            }
          else
            {
              if (aspect_ratio_ws < 1) *margin /= aspect_ratio_ws;
            }
        }
    }
}

static void capSidePlotMarginInNonKeepAspectRatio(const std::shared_ptr<GRM::Element> side_region, double *margin,
                                                  std::string kind)
{
  // TODO: Overwork max value workaround
  if (side_region->querySelectors("side_plot_region"))
    {
      if (str_equals_any(kind, "surface", "volume", "trisurface"))
        {
          *margin = grm_max(0.12, *margin);
        }
      else
        {
          *margin = grm_max(0.075, *margin);
        }
    }
}

static void calculateCentralRegionMarginOrDiagFactor(const std::shared_ptr<GRM::Element> element, double *vp_x_min,
                                                     double *vp_x_max, double *vp_y_min, double *vp_y_max,
                                                     bool diag_factor = false)
{
  bool left_text_margin = false, right_text_margin = false, bottom_text_margin = false, top_text_margin = false;
  bool top_text_is_title = false;
  std::string kind;
  bool keep_aspect_ratio, uniform_data = true, only_quadratic_aspect_ratio = false;
  double metric_width, metric_height;
  double aspect_ratio_ws, start_aspect_ratio_ws;
  double vp0, vp1, vp2, vp3;
  double left_margin = 0.0, right_margin = 0.0, bottom_margin = 0.0, top_margin = 0.0;
  double viewport[4] = {0.0, 0.0, 0.0, 0.0};
  std::shared_ptr<GRM::Element> plot_parent = element, left_side_region, right_side_region, bottom_side_region,
                                top_side_region;

  auto render = grm_get_render();
  getPlotParent(plot_parent);

  kind = static_cast<std::string>(plot_parent->getAttribute("kind"));
  keep_aspect_ratio = static_cast<int>(plot_parent->getAttribute("keep_aspect_ratio"));
  only_quadratic_aspect_ratio = static_cast<int>(plot_parent->getAttribute("only_quadratic_aspect_ratio"));

  left_side_region = plot_parent->querySelectors("side_region[location=\"left\"]");
  right_side_region = plot_parent->querySelectors("side_region[location=\"right\"]");
  bottom_side_region = plot_parent->querySelectors("side_region[location=\"bottom\"]");
  top_side_region = plot_parent->querySelectors("side_region[location=\"top\"]");
  if (left_side_region && left_side_region->hasAttribute("text_content")) left_text_margin = true;
  if (right_side_region && right_side_region->hasAttribute("text_content")) right_text_margin = true;
  if (bottom_side_region && bottom_side_region->hasAttribute("text_content")) bottom_text_margin = true;
  if (top_side_region && top_side_region->hasAttribute("text_content")) top_text_margin = true;
  if (top_side_region && top_side_region->hasAttribute("text_is_title"))
    top_text_is_title = top_text_margin && static_cast<int>(top_side_region->getAttribute("text_is_title"));

  for (const auto &series : element->children())
    {
      if (!starts_with(series->localName(), "series_")) continue;
      uniform_data = isUniformData(series, render->getContext());
      if (!uniform_data) break;
    }
  if (kind == "marginal_heatmap" && uniform_data)
    uniform_data = isUniformData(element->parentElement(), render->getContext());

  GRM::Render::getFigureSize(nullptr, nullptr, &metric_width, &metric_height);
  aspect_ratio_ws = metric_width / metric_height;
  start_aspect_ratio_ws = static_cast<double>(plot_parent->getAttribute("_start_aspect_ratio"));

  if (keep_aspect_ratio && (!only_quadratic_aspect_ratio || (only_quadratic_aspect_ratio && !uniform_data)) &&
      !diag_factor && kind != "imshow")
    {
      if (aspect_ratio_ws > start_aspect_ratio_ws)
        {
          auto x_min = *vp_x_min * (start_aspect_ratio_ws / aspect_ratio_ws);
          auto x_max = *vp_x_max * (start_aspect_ratio_ws / aspect_ratio_ws);
          auto diff = 0.5 * ((*vp_x_max - *vp_x_min) - (x_max - x_min));
          *vp_x_min += diff;
          *vp_x_max -= diff;
        }
      else
        {
          auto y_min = *vp_y_min / (start_aspect_ratio_ws / aspect_ratio_ws);
          auto y_max = *vp_y_max / (start_aspect_ratio_ws / aspect_ratio_ws);
          auto diff = 0.5 * ((*vp_y_max - *vp_y_min) - (y_max - y_min));
          *vp_y_min += diff;
          *vp_y_max -= diff;
        }
    }
  else if (keep_aspect_ratio && uniform_data && only_quadratic_aspect_ratio && !diag_factor && kind != "imshow")
    {
      if (aspect_ratio_ws > 1)
        {
          double border = 0.5 * (*vp_x_max - *vp_x_min) * (1.0 - 1.0 / aspect_ratio_ws);
          *vp_x_min += border;
          *vp_x_max -= border;
        }
      else if (aspect_ratio_ws <= 1)
        {
          double border = 0.5 * (*vp_y_max - *vp_y_min) * (1.0 - aspect_ratio_ws);
          *vp_y_min += border;
          *vp_y_max -= border;
        }
    }

  if (str_equals_any(kind, "wireframe", "surface", "plot3", "scatter3", "trisurface", "volume"))
    {
      double extent;

      extent = grm_min(*vp_x_max - *vp_x_min, *vp_y_max - *vp_y_min);
      vp0 = 0.5 * (*vp_x_min + *vp_x_max - extent);
      vp1 = 0.5 * (*vp_x_min + *vp_x_max + extent);
      vp2 = 0.5 * (*vp_y_min + *vp_y_max - extent);
      vp3 = 0.5 * (*vp_y_min + *vp_y_max + extent);
    }
  else
    {
      vp0 = *vp_x_min;
      vp1 = *vp_x_max;
      vp2 = *vp_y_min;
      vp3 = *vp_y_max;
    }

  // margin respects colorbar and sideplot in the specific side_region
  // TODO: respect individual size defined by user
  sidePlotMargin(left_side_region, &left_margin, (vp1 - vp0) * 0.1, (keep_aspect_ratio && !diag_factor),
                 aspect_ratio_ws, start_aspect_ratio_ws);
  sidePlotMargin(right_side_region, &right_margin, (vp1 - vp0) * 0.1, (keep_aspect_ratio && !diag_factor),
                 aspect_ratio_ws, start_aspect_ratio_ws);
  sidePlotMargin(bottom_side_region, &bottom_margin, (vp3 - vp2) * 0.1, (keep_aspect_ratio && !diag_factor),
                 aspect_ratio_ws, start_aspect_ratio_ws);
  sidePlotMargin(top_side_region, &top_margin, (vp3 - vp2) * 0.1, (keep_aspect_ratio && !diag_factor), aspect_ratio_ws,
                 start_aspect_ratio_ws);

  // in the non keep_aspect_ratio case the viewport vp0 - vp3 can be too small for the resulting side_plot; use a
  // predefined maximum in these cases
  if (kind != "marginal_heatmap" && !keep_aspect_ratio)
    {
      capSidePlotMarginInNonKeepAspectRatio(left_side_region, &left_margin, kind);
      capSidePlotMarginInNonKeepAspectRatio(right_side_region, &right_margin, kind);
      capSidePlotMarginInNonKeepAspectRatio(bottom_side_region, &bottom_margin, kind);
      capSidePlotMarginInNonKeepAspectRatio(top_side_region, &top_margin, kind);
    }

  // margin respects text in the specific side_region
  if (left_text_margin) left_margin = 0.05;
  if (right_text_margin) right_margin = 0.05;
  if (bottom_text_margin) bottom_margin = 0.05;

  // calculate text impact for top_margin and adjust all margins if defined by attributes
  if (kind == "marginal_heatmap")
    {
      top_margin += (right_margin - top_margin) + (top_text_margin ? top_text_is_title ? 0.075 : 0.05 : 0.025);

      if (keep_aspect_ratio && uniform_data && only_quadratic_aspect_ratio)
        {
          if (bottom_margin != left_margin)
            {
              bottom_margin = grm_max(left_margin, bottom_margin);
              left_margin = bottom_margin;
            }
          if (right_margin > top_margin)
            {
              top_margin += (0.975 - top_margin) - (0.95 - right_margin);
            }
          else
            {
              right_margin += (0.95 - right_margin) - (0.975 - top_margin);
            }
        }
    }
  else
    {
      top_margin = (top_text_margin ? top_text_is_title ? 0.075 : 0.05 : 0.0);
      if (keep_aspect_ratio && uniform_data && only_quadratic_aspect_ratio)
        {
          if (bottom_margin != left_margin)
            {
              bottom_margin = grm_max(left_margin, bottom_margin);
              left_margin = bottom_margin;
            }
          right_margin += top_margin;
          if (right_margin > top_margin)
            {
              auto diff = (0.975 - top_margin) - (0.95 - right_margin);
              top_margin += 0.5 * diff;
              bottom_margin += 0.5 * diff;
            }
          else
            {
              auto diff = (0.95 - right_margin) - (0.975 - top_margin);
              right_margin += 0.5 * diff;
              left_margin += 0.5 * diff;
            }
        }
    }
  if (kind == "imshow")
    {
      double w, h, x_min, x_max, y_min, y_max;

      auto cols = static_cast<int>(element->getAttribute("cols"));
      auto rows = static_cast<int>(element->getAttribute("rows"));

      h = (double)rows / (double)cols * (vp1 - vp0);
      w = (double)cols / (double)rows * (vp3 - vp2);

      x_min = grm_max(0.5 * (vp0 + vp1 - w), vp0);
      x_max = grm_min(0.5 * (vp0 + vp1 + w), vp1);
      y_min = grm_max(0.5 * (vp3 + vp2 - h), vp2);
      y_max = grm_min(0.5 * (vp3 + vp2 + h), vp3);

      left_margin = (x_min == vp0) ? -0.075 : (x_min - vp0) / (vp1 - vp0) - 0.075;
      right_margin = (x_max == vp1) ? -0.05 : 0.95 - (x_max - vp0) / (vp1 - vp0);
      bottom_margin = (y_min == vp2) ? -0.075 : (y_min - vp2) / (vp3 - vp2) - 0.075;
      top_margin = (y_max == vp3) ? -0.025 : 0.975 - (y_max - vp2) / (vp3 - vp2);
    }

  viewport[0] = vp0 + (0.075 + left_margin) * (vp1 - vp0);
  viewport[1] = vp0 + (0.95 - right_margin) * (vp1 - vp0);
  viewport[2] = vp2 + (0.075 + bottom_margin) * (vp3 - vp2);
  viewport[3] = vp2 + (0.975 - top_margin) * (vp3 - vp2);

  if (str_equals_any(kind, "line", "stairs", "scatter", "stem"))
    {
      double w, h;
      int location = PLOT_DEFAULT_LOCATION;
      if (element->hasAttribute("location"))
        {
          if (element->getAttribute("location").isInt())
            {
              location = static_cast<int>(element->getAttribute("location"));
            }
          else if (element->getAttribute("location").isString())
            {
              location = locationStringToInt(static_cast<std::string>(element->getAttribute("location")));
            }
        }
      else
        {
          element->setAttribute("location", location);
        }

      if (location == 11 || location == 12 || location == 13)
        {
          legendSize(element, &w, &h);
          viewport[1] -= w + 0.1;
        }
    }

  if (str_equals_any(kind, "pie", "polar", "polar_histogram", "polar_heatmap", "nonuniformpolar_heatmap"))
    {
      double x_center, y_center, r;

      x_center = 0.5 * (viewport[0] + viewport[1]);
      y_center = 0.5 * (viewport[2] + viewport[3]);
      r = 0.45 * grm_min(viewport[1] - viewport[0], viewport[3] - viewport[2]);
      if (top_text_margin)
        {
          r *= 0.975;
          y_center -= 0.025 * r;
        }
      viewport[0] = x_center - r;
      viewport[1] = x_center + r;
      viewport[2] = y_center - r;
      viewport[3] = y_center + r;
    }
  *vp_x_min = viewport[0];
  *vp_x_max = viewport[1];
  *vp_y_min = viewport[2];
  *vp_y_max = viewport[3];
}

static void setViewportForSideRegionElements(const std::shared_ptr<GRM::Element> &element, double offset, double width,
                                             bool uniform_data)
{
  double viewport[4];
  std::string location = PLOT_DEFAULT_SIDEREGION_LOCATION;
  double max_vp, min_vp;
  double offset_rel, width_rel;
  double metric_width, metric_height, start_aspect_ratio_ws;
  bool keep_aspect_ratio = false, only_quadratic_aspect_ratio = false;
  std::shared_ptr<GRM::Element> plot_parent = element, central_region, side_region = element;
  getPlotParent(plot_parent);

  central_region = plot_parent->querySelectors("central_region");
  if (element->localName() != "side_region") side_region = element->parentElement();

  viewport[0] = static_cast<double>(central_region->getAttribute("_viewport_x_min_org"));
  viewport[1] = static_cast<double>(central_region->getAttribute("_viewport_x_max_org"));
  viewport[2] = static_cast<double>(central_region->getAttribute("_viewport_y_min_org"));
  viewport[3] = static_cast<double>(central_region->getAttribute("_viewport_y_max_org"));
  keep_aspect_ratio = static_cast<int>(plot_parent->getAttribute("keep_aspect_ratio"));
  only_quadratic_aspect_ratio = static_cast<int>(plot_parent->getAttribute("only_quadratic_aspect_ratio"));
  start_aspect_ratio_ws = static_cast<double>(plot_parent->getAttribute("_start_aspect_ratio"));
  location = static_cast<std::string>(side_region->getAttribute("location"));

  GRM::Render::getFigureSize(nullptr, nullptr, &metric_width, &metric_height);
  auto aspect_ratio_ws = metric_width / metric_height;
  double diag_factor = std::sqrt((viewport[1] - viewport[0]) * (viewport[1] - viewport[0]) +
                                 (viewport[3] - viewport[2]) * (viewport[3] - viewport[2]));
  if (!element->hasAttribute("_default_diag_factor"))
    element->setAttribute("_default_diag_factor",
                          ((DEFAULT_ASPECT_RATIO_FOR_SCALING) *
                           (start_aspect_ratio_ws <= 1 ? start_aspect_ratio_ws : (1.0 / start_aspect_ratio_ws))) /
                              diag_factor);

  // special case for keep_aspect_ratio with uniform data which can lead to smaller plots
  if ((keep_aspect_ratio && uniform_data && only_quadratic_aspect_ratio) || !keep_aspect_ratio)
    {
      if (!element->hasAttribute("_offset_set_by_user")) offset *= DEFAULT_ASPECT_RATIO_FOR_SCALING;
      if (!element->hasAttribute("_width_set_by_user")) width *= DEFAULT_ASPECT_RATIO_FOR_SCALING;
      if (aspect_ratio_ws <= 1)
        {
          offset_rel = offset * aspect_ratio_ws;
          width_rel = width * aspect_ratio_ws;
        }
      else
        {
          offset_rel = offset / aspect_ratio_ws;
          width_rel = width / aspect_ratio_ws;
        }
    }
  else
    {
      auto default_diag_factor = static_cast<double>(element->getAttribute("_default_diag_factor"));
      offset_rel = offset * diag_factor * default_diag_factor;
      width_rel = width * diag_factor * default_diag_factor;
    }

  if (location == "right")
    {
      max_vp = getMaxViewport(element, true);
      global_render->setViewport(element, viewport[1] + offset_rel,
                                 grm_min(viewport[1] + offset_rel + width_rel, max_vp), viewport[2], viewport[3]);
      element->setAttribute("_viewport_x_min_org", viewport[1] + offset_rel);
      element->setAttribute("_viewport_x_max_org", grm_min(viewport[1] + offset_rel + width_rel, max_vp));
      element->setAttribute("_viewport_y_min_org", viewport[2]);
      element->setAttribute("_viewport_y_max_org", viewport[3]);
    }
  else if (location == "left")
    {
      min_vp = getMinViewport(element, true);
      global_render->setViewport(element, grm_max(viewport[0] - (offset_rel + width_rel), min_vp), viewport[0],
                                 viewport[2], viewport[3]);
      element->setAttribute("_viewport_x_min_org", grm_max(viewport[0] - (offset_rel + width_rel), min_vp));
      element->setAttribute("_viewport_x_max_org", viewport[0]);
      element->setAttribute("_viewport_y_min_org", viewport[2]);
      element->setAttribute("_viewport_y_max_org", viewport[3]);
    }
  else if (location == "top")
    {
      max_vp = getMaxViewport(element, false);
      global_render->setViewport(element, viewport[0], viewport[1], viewport[3] + offset_rel,
                                 grm_min(viewport[3] + offset_rel + width_rel, max_vp));
      element->setAttribute("_viewport_x_min_org", viewport[0]);
      element->setAttribute("_viewport_x_max_org", viewport[1]);
      element->setAttribute("_viewport_y_min_org", viewport[3] + offset_rel);
      element->setAttribute("_viewport_y_max_org", grm_min(viewport[3] + offset_rel + width_rel, max_vp));
    }
  else if (location == "bottom")
    {
      min_vp = getMinViewport(element, false);
      global_render->setViewport(element, viewport[0], viewport[1],
                                 grm_max(viewport[2] - (offset_rel + width_rel), min_vp), viewport[2]);
      element->setAttribute("_viewport_x_min_org", viewport[0]);
      element->setAttribute("_viewport_x_max_org", viewport[1]);
      element->setAttribute("_viewport_y_min_org", grm_max(viewport[2] - (offset_rel + width_rel), min_vp));
      element->setAttribute("_viewport_y_max_org", viewport[2]);
    }
}

static void calculateViewport(const std::shared_ptr<GRM::Element> &element)
{
  auto render = grm_get_render();
  if (element->localName() == "central_region")
    {
      double vp[4];
      std::shared_ptr<GRM::Element> plot_parent = element;
      getPlotParent(plot_parent);

      vp[0] = static_cast<double>(element->parentElement()->getAttribute("viewport_x_min"));
      vp[1] = static_cast<double>(element->parentElement()->getAttribute("viewport_x_max"));
      vp[2] = static_cast<double>(element->parentElement()->getAttribute("viewport_y_min"));
      vp[3] = static_cast<double>(element->parentElement()->getAttribute("viewport_y_max"));

      calculateCentralRegionMarginOrDiagFactor(element, &vp[0], &vp[1], &vp[2], &vp[3], false);

      render->setViewport(element, vp[0], vp[1], vp[2], vp[3]);
      element->setAttribute("_viewport_x_min_org", vp[0]);
      element->setAttribute("_viewport_x_max_org", vp[1]);
      element->setAttribute("_viewport_y_min_org", vp[2]);
      element->setAttribute("_viewport_y_max_org", vp[3]);
    }
  else if (element->localName() == "plot")
    {
      double vp[4];
      double metric_width, metric_height;
      double aspect_ratio_ws;
      std::string kind;

      /* when grids are being used for layouting the subplot information is stored in the parent of the plot */
      if (element->parentElement()->localName() == "layout_grid_element")
        {
          vp[0] = static_cast<double>(element->parentElement()->getAttribute("plot_x_min"));
          vp[1] = static_cast<double>(element->parentElement()->getAttribute("plot_x_max"));
          vp[2] = static_cast<double>(element->parentElement()->getAttribute("plot_y_min"));
          vp[3] = static_cast<double>(element->parentElement()->getAttribute("plot_y_max"));
        }
      else
        {
          vp[0] = static_cast<double>(element->getAttribute("plot_x_min"));
          vp[1] = static_cast<double>(element->getAttribute("plot_x_max"));
          vp[2] = static_cast<double>(element->getAttribute("plot_y_min"));
          vp[3] = static_cast<double>(element->getAttribute("plot_y_max"));
        }
      kind = static_cast<std::string>(element->getAttribute("kind"));

      GRM::Render::getFigureSize(nullptr, nullptr, &metric_width, &metric_height);
      aspect_ratio_ws = metric_width / metric_height;
      if (aspect_ratio_ws > 1)
        {
          vp[2] /= aspect_ratio_ws;
          vp[3] /= aspect_ratio_ws;
        }
      else
        {
          vp[0] *= aspect_ratio_ws;
          vp[1] *= aspect_ratio_ws;
        }
      if (!element->hasAttribute("_start_aspect_ratio")) element->setAttribute("_start_aspect_ratio", aspect_ratio_ws);

      render->setViewport(element, vp[0], vp[1], vp[2], vp[3]);
      element->setAttribute("_viewport_x_min_org", vp[0]);
      element->setAttribute("_viewport_x_max_org", vp[1]);
      element->setAttribute("_viewport_y_min_org", vp[2]);
      element->setAttribute("_viewport_y_max_org", vp[3]);
    }
  else if (element->localName() == "side_region")
    {
      double plot_viewport[4];
      double offset = PLOT_DEFAULT_SIDEREGION_OFFSET, width = PLOT_DEFAULT_SIDEREGION_WIDTH;
      std::string location = PLOT_DEFAULT_SIDEREGION_LOCATION, kind;
      double max_vp, min_vp;
      double offset_rel, width_rel;
      double metric_width, metric_height;
      bool keep_aspect_ratio = false, uniform_data = true, only_quadratic_aspect_ratio = false;

      auto plot_parent = element;
      getPlotParent(plot_parent);

      if (element->hasAttribute("location")) location = static_cast<std::string>(element->getAttribute("location"));

      // is set cause processElement is run for every Element even when only attributes gets processed; so the
      // calculateViewport call from the plot element which causes the calculation of the central_region viewport is
      // also processed
      plot_viewport[0] = static_cast<double>(plot_parent->getAttribute("plot_x_min"));
      plot_viewport[1] = static_cast<double>(plot_parent->getAttribute("plot_x_max"));
      plot_viewport[2] =
          static_cast<double>(plot_parent->getAttribute("plot_y_min")) / (DEFAULT_ASPECT_RATIO_FOR_SCALING);
      plot_viewport[3] =
          static_cast<double>(plot_parent->getAttribute("plot_y_max")) / (DEFAULT_ASPECT_RATIO_FOR_SCALING);
      keep_aspect_ratio = static_cast<int>(plot_parent->getAttribute("keep_aspect_ratio"));
      only_quadratic_aspect_ratio = static_cast<int>(plot_parent->getAttribute("only_quadratic_aspect_ratio"));
      kind = static_cast<std::string>(plot_parent->getAttribute("kind"));

      if (keep_aspect_ratio && only_quadratic_aspect_ratio)
        {
          for (const auto &series : plot_parent->querySelectors("central_region")->children())
            {
              if (!starts_with(series->localName(), "series_")) continue;
              uniform_data = isUniformData(series, render->getContext());
              if (!uniform_data) break;
            }
          if (kind == "marginal_heatmap" && uniform_data)
            uniform_data = isUniformData(plot_parent->children()[0], render->getContext());
          if (uniform_data)
            {
              double border =
                  0.5 * (plot_viewport[1] - plot_viewport[0]) * (1.0 - 1.0 / (DEFAULT_ASPECT_RATIO_FOR_SCALING));
              plot_viewport[0] += border;
              plot_viewport[1] -= border;
            }
        }

      if (element->hasAttribute("offset") && !element->hasAttribute("marginal_heatmap_side_plot"))
        offset = static_cast<double>(element->getAttribute("offset"));
      if (element->hasAttribute("width") && !element->hasAttribute("marginal_heatmap_side_plot"))
        width = static_cast<double>(element->getAttribute("width"));

      // side_region only has a text_region child - no offset or width is needed
      if (element->querySelectors("side_plot_region") == nullptr &&
          !element->hasAttribute("marginal_heatmap_side_plot"))
        {
          offset = 0.0;
          width = 0.0;
        }

      if (!element->hasAttribute("_offset_set_by_user")) element->setAttribute("offset", offset);
      if (!element->hasAttribute("_width_set_by_user")) element->setAttribute("width", width);

      // apply text width to the side_region
      if (kind != "imshow")
        {
          if (location == "top" && element->hasAttribute("text_content") &&
              !element->hasAttribute("marginal_heatmap_side_plot"))
            {
              width += (0.025 + ((element->hasAttribute("text_is_title") &&
                                  static_cast<int>(element->getAttribute("text_is_title")))
                                     ? 0.075
                                     : 0.05)) *
                       (plot_viewport[3] - plot_viewport[2]);
            }
          if (location == "left" && element->hasAttribute("text_content"))
            {
              width += (0.075 + 0.05) * (plot_viewport[1] - plot_viewport[0]);
            }
          if (location == "bottom" && element->hasAttribute("text_content"))
            {
              width += (0.075 + 0.05) * (plot_viewport[3] - plot_viewport[2]);
            }
          if (location == "right" && element->hasAttribute("text_content"))
            {
              width += (0.075 + 0.05) * (plot_viewport[1] - plot_viewport[0]);
            }
        }

      setViewportForSideRegionElements(element, offset, width, uniform_data);
    }
  else if (element->localName() == "text_region")
    {
      double width = 0.0, offset = 0.0;
      double plot_viewport[4];
      std::string kind, location;
      auto plot_parent = element;
      getPlotParent(plot_parent);

      plot_viewport[0] = static_cast<double>(plot_parent->getAttribute("plot_x_min"));
      plot_viewport[1] = static_cast<double>(plot_parent->getAttribute("plot_x_max"));
      plot_viewport[2] =
          static_cast<double>(plot_parent->getAttribute("plot_y_min")) / (DEFAULT_ASPECT_RATIO_FOR_SCALING);
      plot_viewport[3] =
          static_cast<double>(plot_parent->getAttribute("plot_y_max")) / (DEFAULT_ASPECT_RATIO_FOR_SCALING);
      location = static_cast<std::string>(element->parentElement()->getAttribute("location"));
      kind = static_cast<std::string>(plot_parent->getAttribute("kind"));

      // apply text width to the side_region
      if (kind != "imshow")
        {
          if (location == "top")
            {
              if (!element->parentElement()->hasAttribute("marginal_heatmap_side_plot"))
                {
                  width += (0.025 + ((element->parentElement()->hasAttribute("text_is_title") &&
                                      static_cast<int>(element->parentElement()->getAttribute("text_is_title")))
                                         ? 0.075
                                         : 0.05)) *
                           (plot_viewport[3] - plot_viewport[2]);
                }
              if (element->parentElement()->hasAttribute("offset"))
                offset = static_cast<double>(element->parentElement()->getAttribute("offset"));
              if (element->parentElement()->hasAttribute("width"))
                offset += static_cast<double>(element->parentElement()->getAttribute("width"));
            }
          if (location == "left")
            {
              width += (0.075 + 0.05) * (plot_viewport[1] - plot_viewport[0]);
            }
          if (location == "bottom")
            {
              width += (0.075 + 0.05) * (plot_viewport[3] - plot_viewport[2]);
            }
          if (location == "right")
            {
              width += (0.075 + 0.05) * (plot_viewport[1] - plot_viewport[0]);
            }
        }

      setViewportForSideRegionElements(element, offset, width, false);
    }
  else if (element->localName() == "side_plot_region")
    {
      double plot_viewport[4];
      double offset = PLOT_DEFAULT_SIDEREGION_OFFSET, width = PLOT_DEFAULT_SIDEREGION_WIDTH;
      std::string kind, location;
      bool keep_aspect_ratio = false, uniform_data = true, only_quadratic_aspect_ratio = false;
      auto plot_parent = element;
      getPlotParent(plot_parent);

      plot_viewport[0] = static_cast<double>(plot_parent->getAttribute("plot_x_min"));
      plot_viewport[1] = static_cast<double>(plot_parent->getAttribute("plot_x_max"));
      plot_viewport[2] =
          static_cast<double>(plot_parent->getAttribute("plot_y_min")) / (DEFAULT_ASPECT_RATIO_FOR_SCALING);
      plot_viewport[3] =
          static_cast<double>(plot_parent->getAttribute("plot_y_max")) / (DEFAULT_ASPECT_RATIO_FOR_SCALING);
      location = static_cast<std::string>(element->parentElement()->getAttribute("location"));
      keep_aspect_ratio = static_cast<int>(plot_parent->getAttribute("keep_aspect_ratio"));
      only_quadratic_aspect_ratio = static_cast<int>(plot_parent->getAttribute("only_quadratic_aspect_ratio"));
      kind = static_cast<std::string>(plot_parent->getAttribute("kind"));

      if (keep_aspect_ratio && only_quadratic_aspect_ratio)
        {
          for (const auto &series : plot_parent->querySelectors("central_region")->children())
            {
              if (!starts_with(series->localName(), "series_")) continue;
              uniform_data = isUniformData(series, render->getContext());
              if (!uniform_data) break;
            }
          if (kind == "marginal_heatmap" && uniform_data)
            uniform_data = isUniformData(plot_parent->children()[0], render->getContext());
          if (uniform_data)
            {
              double border =
                  0.5 * (plot_viewport[1] - plot_viewport[0]) * (1.0 - 1.0 / (DEFAULT_ASPECT_RATIO_FOR_SCALING));
              plot_viewport[0] += border;
              plot_viewport[1] -= border;
            }
        }

      if (element->parentElement()->hasAttribute("offset"))
        offset = static_cast<double>(element->parentElement()->getAttribute("offset"));
      if (element->parentElement()->hasAttribute("width"))
        width = static_cast<double>(element->parentElement()->getAttribute("width"));

      setViewportForSideRegionElements(element, offset, width, false);
    }
  else if (element->localName() == "colorbar") // TODO: adjust this calculation when texts are included in side_region
    {
      auto vp_x_min = static_cast<double>(element->parentElement()->getAttribute("viewport_x_min"));
      auto vp_x_max = static_cast<double>(element->parentElement()->getAttribute("viewport_x_max"));
      auto vp_y_min = static_cast<double>(element->parentElement()->getAttribute("viewport_y_min"));
      auto vp_y_max = static_cast<double>(element->parentElement()->getAttribute("viewport_y_max"));
      render->setViewport(element, vp_x_min, vp_x_max, vp_y_min, vp_y_max);
      element->setAttribute("_viewport_x_min_org", vp_x_min);
      element->setAttribute("_viewport_x_max_org", vp_x_max);
      element->setAttribute("_viewport_y_min_org", vp_y_min);
      element->setAttribute("_viewport_y_max_org", vp_y_max);
    }
  else if (element->localName() == "marginal_heatmap_plot")
    {
      auto vp_x_min = static_cast<double>(element->parentElement()->getAttribute("viewport_x_min"));
      auto vp_x_max = static_cast<double>(element->parentElement()->getAttribute("viewport_x_max"));
      auto vp_y_min = static_cast<double>(element->parentElement()->getAttribute("viewport_y_min"));
      auto vp_y_max = static_cast<double>(element->parentElement()->getAttribute("viewport_y_max"));
      render->setViewport(element, vp_x_min, vp_x_max, vp_y_min, vp_y_max);
      element->setAttribute("_viewport_x_min_org", vp_x_min);
      element->setAttribute("_viewport_x_max_org", vp_x_max);
      element->setAttribute("_viewport_y_min_org", vp_y_min);
      element->setAttribute("_viewport_y_max_org", vp_y_max);
    }
  else if (element->localName() == "legend")
    {
      int location = PLOT_DEFAULT_LOCATION;
      double px, py, w, h;
      double viewport[4];
      double vp_x_min, vp_x_max, vp_y_min, vp_y_max;
      double scale_factor = 1.0, start_aspect_ratio_ws;
      const std::shared_ptr<GRM::Context> &context = render->getContext();
      std::string kind, labels_key = static_cast<std::string>(element->getAttribute("labels"));
      auto labels = GRM::get<std::vector<std::string>>((*context)[labels_key]);
      std::shared_ptr<GRM::Element> central_region;
      bool keep_aspect_ratio = false;

      for (const auto &child : element->parentElement()->children())
        {
          if (child->localName() == "central_region")
            {
              central_region = child;
              break;
            }
        }

      viewport[0] = static_cast<double>(central_region->getAttribute("_viewport_x_min_org"));
      viewport[1] = static_cast<double>(central_region->getAttribute("_viewport_x_max_org"));
      viewport[2] = static_cast<double>(central_region->getAttribute("_viewport_y_min_org"));
      viewport[3] = static_cast<double>(central_region->getAttribute("_viewport_y_max_org"));

      if (element->hasAttribute("location"))
        {
          if (element->getAttribute("location").isInt())
            {
              location = static_cast<int>(element->getAttribute("location"));
            }
          else if (element->getAttribute("location").isString())
            {
              location = locationStringToInt(static_cast<std::string>(element->getAttribute("location")));
            }
        }
      else
        {
          element->setAttribute("location", location);
        }
      keep_aspect_ratio = static_cast<int>(element->parentElement()->getAttribute("keep_aspect_ratio"));
      start_aspect_ratio_ws = static_cast<double>(element->parentElement()->getAttribute("_start_aspect_ratio"));
      kind = static_cast<std::string>(element->parentElement()->getAttribute("kind"));

      if (!keep_aspect_ratio)
        {
          double metric_width, metric_height;
          GRM::Render::getFigureSize(nullptr, nullptr, &metric_width, &metric_height);
          auto aspect_ratio_ws = metric_width / metric_height;

          scale_factor *= DEFAULT_ASPECT_RATIO_FOR_SCALING;
          scale_factor *= (aspect_ratio_ws <= 1) ? aspect_ratio_ws : 1.0 / aspect_ratio_ws;
        }
      else
        {
          double diag_factor = std::sqrt((viewport[1] - viewport[0]) * (viewport[1] - viewport[0]) +
                                         (viewport[3] - viewport[2]) * (viewport[3] - viewport[2]));
          if (!element->hasAttribute("_default_diag_factor"))
            element->setAttribute(
                "_default_diag_factor",
                ((DEFAULT_ASPECT_RATIO_FOR_SCALING) *
                 (start_aspect_ratio_ws <= 1 ? start_aspect_ratio_ws : (1.0 / start_aspect_ratio_ws))) /
                    diag_factor);
          auto default_diag_factor = static_cast<double>(element->getAttribute("_default_diag_factor"));

          scale_factor = diag_factor * default_diag_factor;
        }
      element->setAttribute("_scale_factor", scale_factor);

      if (kind != "pie")
        {
          legendSize(labels, &w, &h);
          if (element->hasAttribute("_start_w"))
            {
              w = static_cast<double>(element->getAttribute("_start_w"));
            }
          else
            {
              element->setAttribute("_start_w", w);
            }
          if (element->hasAttribute("_start_h"))
            {
              h = static_cast<double>(element->getAttribute("_start_h"));
            }
          else
            {
              element->setAttribute("_start_h", h);
            }

          if (int_equals_any(location, 3, 11, 12, 13))
            {
              px = viewport[1] + 0.11 * scale_factor;
            }
          else if (int_equals_any(location, 3, 8, 9, 10))
            {
              px = 0.5 * (viewport[0] + viewport[1] - (w - 0.05) * scale_factor);
            }
          else if (int_equals_any(location, 3, 2, 3, 6))
            {
              px = viewport[0] + 0.11 * scale_factor;
            }
          else
            {
              px = viewport[1] - (0.05 + w) * scale_factor;
            }
          if (int_equals_any(location, 5, 5, 6, 7, 10, 12))
            {
              py = 0.5 * (viewport[2] + viewport[3] + h * scale_factor) - 0.03 * scale_factor;
            }
          else if (location == 13)
            {
              py = viewport[2] + h * scale_factor;
            }
          else if (int_equals_any(location, 3, 3, 4, 8))
            {
              py = viewport[2] + (h + 0.03) * scale_factor;
            }
          else if (location == 11)
            {
              py = viewport[3] - 0.03 * scale_factor;
            }
          else
            {
              py = viewport[3] - 0.06 * scale_factor;
            }
          vp_x_min = px - 0.08 * scale_factor;
          vp_x_max = px + (w + 0.02) * scale_factor;
          vp_y_min = py - h * scale_factor;
          vp_y_max = py + 0.03 * scale_factor;
        }
      else
        {
          double tbx[4], tby[4];
          int num_labels = labels.size();

          w = 0;
          h = 0;
          for (auto current_label : labels)
            {
              gr_inqtext(0, 0, current_label.data(), tbx, tby);
              w += tbx[2] - tbx[0];
              h = grm_max(h, tby[2] - tby[0]);
            }
          w += num_labels * 0.03 + (num_labels - 1) * 0.02;

          if (element->hasAttribute("_start_w"))
            {
              w = static_cast<double>(element->getAttribute("_start_w"));
            }
          else
            {
              element->setAttribute("_start_w", w);
            }
          if (element->hasAttribute("_start_h"))
            {
              h = static_cast<double>(element->getAttribute("_start_h"));
            }
          else
            {
              element->setAttribute("_start_h", h);
            }

          px = 0.5 * (viewport[0] + viewport[1] - w * scale_factor);
          py = viewport[2] - 0.75 * h * scale_factor;

          vp_x_min = px - 0.02 * scale_factor;
          vp_x_max = px + (w + 0.02) * scale_factor;
          vp_y_min = py - (0.5 * h + 0.02) * scale_factor;
          vp_y_max = py + (0.5 * h + 0.02) * scale_factor;
        }

      render->setViewport(element, vp_x_min, vp_x_max, vp_y_min, vp_y_max);
      element->setAttribute("_viewport_x_min_org", vp_x_min);
      element->setAttribute("_viewport_x_max_org", vp_x_max);
      element->setAttribute("_viewport_y_min_org", vp_y_min);
      element->setAttribute("_viewport_y_max_org", vp_y_max);
    }
  GRM::Render::processViewport(element);
}

static void applyMoveTransformation(const std::shared_ptr<GRM::Element> &element)
{
  double w[4], vp[4], vp_org[4];
  double x_shift = 0, y_shift = 0;
  double x_scale = 1, y_scale = 1;
  bool disable_x_trans = false, disable_y_trans = false, any_xform = false; // only for wc
  std::shared_ptr<GRM::Element> parent_element = element->parentElement();
  std::string post_fix = "_wc";
  std::vector<std::string> ndc_transformation_elems = {
      "figure",
      "plot",
      "colorbar",
      "labels_group",
      "titles_3d",
      "text",
      "layout_grid_element",
      "layout_grid",
      "central_region",
      "side_region",
      "marginal_heatmap_plot",
      "legend",
      "axis",
  };

  if (std::find(ndc_transformation_elems.begin(), ndc_transformation_elems.end(), element->localName()) !=
      ndc_transformation_elems.end())
    post_fix = "_ndc";

  if (element->hasAttribute("x_scale" + post_fix))
    {
      x_scale = static_cast<double>(element->getAttribute("x_scale" + post_fix));
      any_xform = true;
    }
  if (element->hasAttribute("y_scale" + post_fix))
    {
      y_scale = static_cast<double>(element->getAttribute("y_scale" + post_fix));
      any_xform = true;
    }
  if (element->hasAttribute("x_shift" + post_fix))
    {
      x_shift = static_cast<double>(element->getAttribute("x_shift" + post_fix));
      any_xform = true;
    }
  if (element->hasAttribute("y_shift" + post_fix))
    {
      y_shift = static_cast<double>(element->getAttribute("y_shift" + post_fix));
      any_xform = true;
    }
  if (element->hasAttribute("disable_x_trans"))
    disable_x_trans = static_cast<int>(element->getAttribute("disable_x_trans"));
  if (element->hasAttribute("disable_y_trans"))
    disable_y_trans = static_cast<int>(element->getAttribute("disable_y_trans"));

  // get parent transformation when an element doesn't have an own in wc case
  if (!any_xform && post_fix == "_wc")
    {
      while (parent_element->localName() != "root" && !any_xform)
        {
          if (parent_element->hasAttribute("x_scale" + post_fix))
            {
              x_scale = static_cast<double>(parent_element->getAttribute("x_scale" + post_fix));
              any_xform = true;
            }
          if (parent_element->hasAttribute("y_scale" + post_fix))
            {
              y_scale = static_cast<double>(parent_element->getAttribute("y_scale" + post_fix));
              any_xform = true;
            }
          if (parent_element->hasAttribute("x_shift" + post_fix))
            {
              x_shift = static_cast<double>(parent_element->getAttribute("x_shift" + post_fix));
              any_xform = true;
            }
          if (parent_element->hasAttribute("y_shift" + post_fix))
            {
              y_shift = static_cast<double>(parent_element->getAttribute("y_shift" + post_fix));
              any_xform = true;
            }
          if (parent_element->hasAttribute("disable_x_trans"))
            disable_x_trans = static_cast<int>(parent_element->getAttribute("disable_x_trans"));
          if (parent_element->hasAttribute("disable_y_trans"))
            disable_y_trans = static_cast<int>(parent_element->getAttribute("disable_y_trans"));

          parent_element = parent_element->parentElement();
        }
    }

  if (post_fix == "_ndc")
    {
      // elements in ndc space gets transformed in ndc space which is equal to changing their viewport
      double diff;
      double vp_border_x_min = 0.0, vp_border_x_max, vp_border_y_min = 0.0, vp_border_y_max;
      bool private_shift = false;

      if (element->hasAttribute("viewport_x_min"))
        {
          vp_org[0] = static_cast<double>(element->getAttribute("_viewport_x_min_org"));
          vp_org[1] = static_cast<double>(element->getAttribute("_viewport_x_max_org"));
          vp_org[2] = static_cast<double>(element->getAttribute("_viewport_y_min_org"));
          vp_org[3] = static_cast<double>(element->getAttribute("_viewport_y_max_org"));
        }
      else
        {
          gr_inqviewport(&vp_org[0], &vp_org[1], &vp_org[2], &vp_org[3]);
        }

      // apply viewport changes defined by the user via setAttribute first
      if (element->hasAttribute("_x_min_shift"))
        {
          vp_org[0] += static_cast<double>(element->getAttribute("_x_min_shift"));
          private_shift = true;
        }
      if (element->hasAttribute("_x_max_shift"))
        {
          vp_org[1] += static_cast<double>(element->getAttribute("_x_max_shift"));
          private_shift = true;
        }
      if (element->hasAttribute("_y_min_shift"))
        {
          vp_org[2] += static_cast<double>(element->getAttribute("_y_min_shift"));
          private_shift = true;
        }
      if (element->hasAttribute("_y_max_shift"))
        {
          vp_org[3] += static_cast<double>(element->getAttribute("_y_max_shift"));
          private_shift = true;
        }

      if (element->localName() == "text" && (x_shift != 0 || y_shift != 0)) gr_settextoffset(x_shift, -y_shift);

      // when the element contains an axes the max viewport must be smaller than normal to respect the axes
      vp_border_x_min = getMinViewport(element, true);
      vp_border_x_max = getMaxViewport(element, true);
      vp_border_y_min = getMinViewport(element, false);
      vp_border_y_max = getMaxViewport(element, false);

      if (private_shift || (x_shift != 0 || x_scale != 1 || y_shift != 0 || y_scale != 1))
        {
          // calculate viewport changes in x-direction
          vp[0] = vp_org[0] / x_scale + x_shift;
          vp[1] = vp_org[1] / x_scale + x_shift;
          diff = grm_min(vp[1] - vp[0], vp_border_x_max - vp_border_x_min);

          // the viewport cant leave the [vp_border_x_min, vp_border_x_max] space
          if (vp[0] < vp_border_x_min)
            {
              vp[0] = vp_border_x_min;
              vp[1] = vp_border_x_min + diff;
            }
          if (vp[1] > vp_border_x_max)
            {
              vp[0] = vp_border_x_max - diff;
              vp[1] = vp_border_x_max;
            }

          // calculate viewport changes in y-direction
          vp[2] = vp_org[2] / y_scale - y_shift;
          vp[3] = vp_org[3] / y_scale - y_shift;
          diff = grm_min(vp_border_y_max - vp_border_y_min, vp[3] - vp[2]);

          // the viewport cant leave the [vp_border_y_min, vp_border_y_max] space
          if (vp[2] < vp_border_y_min)
            {
              vp[3] = vp_border_y_min + diff;
              vp[2] = vp_border_y_min;
            }
          if (vp[3] > vp_border_y_max)
            {
              vp[2] = vp_border_y_max - diff;
              vp[3] = vp_border_y_max;
            }

          bool old_state = automatic_update;
          automatic_update = false;
          grm_get_render()->setViewport(element, vp[0], vp[1], vp[2], vp[3]);
          grm_get_render()->processViewport(element);
          automatic_update = old_state;
        }
    }
  else if (x_shift != 0 || x_scale != 1 || y_shift != 0 || y_scale != 1)
    {
      // elements in world space gets transformed in world space which is equal to changing their window
      gr_inqwindow(&w[0], &w[1], &w[2], &w[3]);
      if (!disable_x_trans)
        {
          w[0] = w[0] / x_scale - x_shift;
          w[1] = w[1] / x_scale - x_shift;
        }
      if (!disable_y_trans)
        {
          w[2] = w[2] / y_scale - y_shift;
          w[3] = w[3] / y_scale - y_shift;
        }
      if (w[1] - w[0] > 0.0 && w[3] - w[2] > 0.0) gr_setwindow(w[0], w[1], w[2], w[3]);
    }
}

static std::string getLocalName(const std::shared_ptr<GRM::Element> &element)
{
  std::string local_name = element->localName();
  if (starts_with(element->localName(), "series")) local_name = "series";
  return local_name;
}

static bool isDrawable(const std::shared_ptr<GRM::Element> &element)
{
  auto local_name = getLocalName(element);
  if (drawable_types.find(local_name) != drawable_types.end())
    {
      return true;
    }
  if (local_name == "series")
    {
      auto kind = static_cast<std::string>(element->getAttribute("kind"));
      if (drawable_kinds.find(kind) != drawable_kinds.end())
        {
          return true;
        }
    }
  return false;
}

int algorithmStringToInt(const std::string &algorithm_str)
{
  if (algorithm_string_to_int.count(algorithm_str))
    return algorithm_string_to_int[algorithm_str];
  else
    {
      logger((stderr, "Got unknown volume algorithm \"%s\"\n", algorithm_str.c_str()));
      throw std::logic_error("For volume series the given algorithm is unknown.\n");
    }
}

std::string algorithmIntToString(int algorithm)
{
  for (auto const &map_elem : algorithm_string_to_int)
    {
      if (map_elem.second == algorithm)
        {
          return map_elem.first;
        }
    }
  logger((stderr, "Got unknown volume algorithm \"%i\"\n", algorithm));
  throw std::logic_error("For volume series the given algorithm is unknown.\n");
}

int scientificFormatStringToInt(const std::string &scientific_format_str)
{
  if (scientific_format_string_to_int.count(scientific_format_str))
    return scientific_format_string_to_int[scientific_format_str];
  else
    {
      logger((stderr, "Got unknown scientific_format \"%s\"\n", scientific_format_str.c_str()));
      throw std::logic_error("Given scientific_format is unknown.\n");
    }
}

std::string scientificFormatIntToString(int scientific_format)
{
  for (auto const &map_elem : scientific_format_string_to_int)
    {
      if (map_elem.second == scientific_format)
        {
          return map_elem.first;
        }
    }
  logger((stderr, "Got unknown scientific_format \"%i\"\n", scientific_format));
  throw std::logic_error("Given scientific_format is unknown.\n");
}

int getVolumeAlgorithm(const std::shared_ptr<GRM::Element> &element)
{
  int algorithm;
  std::string algorithm_str;

  if (element->getAttribute("algorithm").isInt())
    {
      algorithm = static_cast<int>(element->getAttribute("algorithm"));
    }
  else if (element->getAttribute("algorithm").isString())
    {
      algorithm_str = static_cast<std::string>(element->getAttribute("algorithm"));
      algorithm = algorithmStringToInt(algorithm_str);
    }
  else
    {
      throw NotFoundError("Volume series is missing attribute algorithm.\n");
    }
  return algorithm;
}

PushDrawableToZQueue::PushDrawableToZQueue(
    std::function<void(const std::shared_ptr<GRM::Element> &, const std::shared_ptr<GRM::Context> &)> draw_function)
    : drawFunction(draw_function)
{
  ;
}

void PushDrawableToZQueue::operator()(const std::shared_ptr<GRM::Element> &element,
                                      const std::shared_ptr<GRM::Context> &context)
{
  int context_id;
  auto parent = element->parentElement();

  if (auto search = parent_to_context.find(parent); search != parent_to_context.end())
    {
      context_id = search->second;
    }
  else
    {
      context_id = gr_context_id_manager.getUnusedGRContextId();
      gr_savecontext(context_id);
      parent_to_context[parent] = context_id;
    }
  auto drawable =
      std::shared_ptr<Drawable>(new Drawable(element, context, context_id, z_index_manager.getZIndex(), drawFunction));
  drawable->insertionIndex = (int)z_queue.size();
  custom_color_index_manager.savecontext(context_id);
  z_queue.push(drawable);
}

static double autoTick(double amin, double amax)
{
  double tick_size[] = {5.0, 2.0, 1.0, 0.5, 0.2, 0.1, 0.05, 0.02, 0.01};
  double scale, tick;
  int i, n;

  scale = pow(10.0, (int)(log10(amax - amin)));
  tick = 1.0;
  for (i = 0; i < 9; i++)
    {
      n = (int)((amax - amin) / scale / tick_size[i]);
      if (n > 7)
        {
          tick = tick_size[i - 1];
          break;
        }
    }
  tick *= scale;
  return tick;
}

static double autoTickRingsPolar(double rmax, int &rings, const std::string &norm, int Y_log = 0)
{
  // todo ylog with other cases! e.g. polarhistogram with "cdf" max is always 1
  double scale;
  bool is_decimal = false;
  std::vector<int> *whichVector;
  std::vector<int> largeRings = {6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
  std::vector<int> normalRings = {3, 4, 5, 6, 7};

  // define good decimal rmax values and a fitting number of rings
  // todo
  std::map<double, int> decimalMap{{0.1, 4}};

  // -1 --> auto rings
  if (rings == -1)
    {
      if (norm == "cdf")
        {
          rings = 4;
          return 1.0 / rings;
        }

      whichVector = (rmax > 20) ? &largeRings : &normalRings;

      // todo: negative scales
      // todo: rmin is needed for negative scales
      scale = ceil(abs(log10(rmax)));

      // todo: values near 1... epsilon environment?
      if (rmax != 1.0)
        {
          scale *= log10(rmax) / abs(log10(rmax));
        }

      if (rmax <= 1.0)
        {
          is_decimal = true;

          //          rmax = static_cast<int>(ceil(rmax * pow(10.0, scale)));

          // todo: for decimalrings <= 1.0 don't make rmax bigger than 1.0
          while (true)
            {
              for (int i : *whichVector)
                {
                  if (static_cast<int>(ceil(rmax)) % i == 0)
                    {
                      if (is_decimal)
                        {
                          rmax = rmax / pow(10.0, scale);
                        }
                      rings = i;
                      return rmax / rings;
                    }
                }
              // rmax not divisible by whichVector
              ++rmax;
              if (rmax > 1.0)
                {
                  rings = 4;
                  return 1.0 / rings;
                }
            }
        } // end decimal case <= 1.0
      else if (rmax < (*whichVector)[0])
        {
          // if rmax is bigger than 1.0 but smaller than the smallest amount of rings
          for (int i : *whichVector)
            {
              if (static_cast<int>((rmax * 100)) % i == 0)
                {
                  rings = i;
                  return floor(rmax * 100) / 100 / i;
                }
            }
        }

      while (true)
        {
          for (int i : *whichVector)
            {
              if (static_cast<int>(ceil(rmax)) % i == 0)
                {
                  if (is_decimal) rmax = rmax / pow(10.0, scale);
                  rings = i;
                  return rmax / rings;
                }
            }
          // rmax not divisible by whichVector
          ++rmax;
        }
    }

  // given rings
  if (norm == "cdf") return 1.0 / rings;

  if (rmax > rings)
    {
      return (static_cast<int>(ceil(rmax)) + (rings - (static_cast<int>(rmax) % rings))) / rings;
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

static void clearAxisAttributes(const std::shared_ptr<GRM::Element> &axis)
{
  if (axis->hasAttribute("min_value")) axis->removeAttribute("min_value");
  if (axis->hasAttribute("max_value")) axis->removeAttribute("max_value");
  if (axis->hasAttribute("org")) axis->removeAttribute("org");
  if (axis->hasAttribute("pos")) axis->removeAttribute("pos");
  if (axis->hasAttribute("tick")) axis->removeAttribute("tick");
  if (axis->hasAttribute("major_count")) axis->removeAttribute("major_count");
  if (axis->hasAttribute("tick_size")) axis->removeAttribute("tick_size");
  if (axis->hasAttribute("_tick_size_org")) axis->removeAttribute("_tick_size_org");
  if (axis->hasAttribute("tick_orientation")) axis->removeAttribute("tick_orientation");
}

/*!
 * Convert an RGB triple to a luminance value following the CCIR 601 format.
 *
 * \param[in] r The red component of the RGB triple in the range [0.0, 1.0].
 * \param[in] g The green component of the RGB triple in the range [0.0, 1.0].
 * \param[in] b The blue component of the RGB triple in the range [0.0, 1.0].
 * \return The luminance of the given RGB triple in the range [0.0, 1.0].
 */
static double getLightnessFromRGB(double r, double g, double b)
{
  return 0.299 * r + 0.587 * g + 0.114 * b;
}

/*
 * mixes gr color maps with size = size * size. If x and or y < 0
 */
void createColormap(int x, int y, int size, std::vector<int> &colormap)
{
  int r, g, b, a;
  int outer, inner;
  int r1, g1, b1;
  int r2, g2, b2;
  if (x > 47 || y > 47) logger((stderr, "values for the keyword \"colormap\" can not be greater than 47\n"));

  colormap.resize(size * size);
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
    }
  else if ((x >= 0 && y >= 0) || (x < 0 && y < 0))
    {
      if (x < 0 && y < 0) x = y = 0;
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
    }
}

static void markerHelper(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context,
                         const std::string &str)
{
  /*!
   * Helper function for marker functions using vectors for marker parameters
   *
   * \param[in] element The GRM::Element that contains all marker attributes and data keys. If element's parent is a
   * group element it may fallback to its marker attributes
   * \param[in] context The GRM::Context that contains the actual data
   * \param[in] str The std::string that specifies what GRM Routine should be called (polymarker)
   *
   */
  std::vector<int> type, color_ind;
  std::vector<double> size;
  std::string x_key, y_key, z_key;
  int skip_color_ind = -1000;
  auto parent = element->parentElement();
  bool group = parent_types.count(parent->localName());
  auto attr = element->getAttribute("marker_types");

  if (attr.isString())
    {
      type = GRM::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
    }
  else if (group)
    {
      attr = parent->getAttribute("marker_types");
      if (attr.isString())
        {
          type = GRM::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
        }
    }

  attr = element->getAttribute("marker_color_indices");
  if (attr.isString())
    {
      color_ind = GRM::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
    }
  else if (group)
    {
      attr = parent->getAttribute("marker_color_indices");
      if (attr.isString())
        {
          color_ind = GRM::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
        }
    }

  attr = element->getAttribute("marker_sizes");
  if (attr.isString())
    {
      size = GRM::get<std::vector<double>>((*context)[static_cast<std::string>(attr)]);
    }
  else if (group)
    {
      attr = parent->getAttribute("marker_sizes");
      if (attr.isString())
        {
          size = GRM::get<std::vector<double>>((*context)[static_cast<std::string>(attr)]);
        }
    }

  x_key = static_cast<std::string>(element->getAttribute("x"));
  y_key = static_cast<std::string>(element->getAttribute("y"));
  if (element->hasAttribute("z")) z_key = static_cast<std::string>(element->getAttribute("z"));

  std::vector<double> x_vec = GRM::get<std::vector<double>>((*context)[x_key]);
  std::vector<double> y_vec = GRM::get<std::vector<double>>((*context)[y_key]);
  std::vector<double> z_vec;
  if (auto z_ptr = GRM::get_if<std::vector<double>>((*context)[z_key])) z_vec = *z_ptr;

  auto n = std::min<int>((int)x_vec.size(), (int)y_vec.size());

  for (int i = 0; i < n; ++i)
    {
      // fallback to the last element when lists are too short
      if (!type.empty())
        {
          if (type.size() > i)
            {
              gr_setmarkertype(type[i]);
            }
          else
            {
              gr_setmarkertype(type.back());
            }
        }
      if (!color_ind.empty())
        {
          if (color_ind.size() > i)
            {
              if (color_ind[i] == skip_color_ind)
                {
                  continue;
                }
              gr_setmarkercolorind(color_ind[i]);
            }
          else
            {
              if (color_ind.back() == skip_color_ind)
                {
                  continue;
                }
              gr_setmarkercolorind(color_ind.back());
            }
        }
      if (!size.empty())
        {
          if (size.size() > i)
            {
              gr_setmarkersize(size[i]);
            }
          else
            {
              gr_setmarkersize(size.back());
            }
        }

      applyMoveTransformation(element);
      if (str == "polymarker")
        {
          if (redraw_ws) gr_polymarker(1, (double *)&(x_vec[i]), (double *)&(y_vec[i]));
        }
      else if (str == "polymarker_3d")
        {
          if (redraw_ws) gr_polymarker3d(1, (double *)&(x_vec[i]), (double *)&(y_vec[i]), (double *)&(z_vec[i]));
        }
    }
}

static void lineHelper(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context,
                       const std::string &str)
{
  /*!
   * Helper function for line functions using vectors for line parameters
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   * \param[in] str The std::string that specifies what GRM Routine should be called (polyline)
   *
   *
   */
  std::vector<int> type, color_ind;
  std::vector<double> width;
  std::string x_key, y_key, z_key;

  auto parent = element->parentElement();
  bool group = parent_types.count(parent->localName());

  auto attr = element->getAttribute("line_types");
  if (attr.isString())
    {
      type = GRM::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
    }
  else if (group)
    {
      attr = parent->getAttribute("line_types");
      if (attr.isString())
        {
          type = GRM::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
        }
    }

  attr = element->getAttribute("line_color_indices");
  if (attr.isString())
    {
      color_ind = GRM::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
    }
  else if (group)
    {
      attr = parent->getAttribute("line_color_indices");
      if (attr.isString())
        {
          color_ind = GRM::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
        }
    }

  attr = element->getAttribute("line_widths");
  if (attr.isString())
    {
      width = GRM::get<std::vector<double>>((*context)[static_cast<std::string>(attr)]);
    }
  else if (group)
    {
      attr = parent->getAttribute("line_widths");
      if (attr.isString())
        {
          width = GRM::get<std::vector<double>>((*context)[static_cast<std::string>(attr)]);
        }
    }

  x_key = static_cast<std::string>(element->getAttribute("x"));
  y_key = static_cast<std::string>(element->getAttribute("y"));
  if (element->hasAttribute("z")) z_key = static_cast<std::string>(element->getAttribute("z"));

  std::vector<double> x_vec = GRM::get<std::vector<double>>((*context)[x_key]);
  std::vector<double> y_vec = GRM::get<std::vector<double>>((*context)[y_key]);
  std::vector<double> z_vec;

  if (auto z_ptr = GRM::get_if<std::vector<double>>((*context)[z_key])) z_vec = *z_ptr;

  auto n = std::min<int>((int)x_vec.size(), (int)y_vec.size());
  for (int i = 0; i < n; ++i)
    {
      if (!type.empty())
        {
          if (type.size() > i)
            {
              gr_setlinetype(type[i]);
            }
          else
            {
              gr_setlinetype(type.back());
            }
        }
      if (!color_ind.empty())
        {
          if (color_ind.size() > i)
            {
              gr_setlinecolorind(color_ind[i]);
            }
          else
            {
              gr_setlinecolorind(color_ind.back());
            }
        }
      if (!width.empty())
        {
          if (width.size() > i)
            {
              gr_setlinewidth(width[i]);
            }
          else
            {
              gr_setlinewidth(width.back());
            }
        }

      applyMoveTransformation(element);
      if (str == "polyline")
        {
          if (redraw_ws) gr_polyline(2, (double *)&(x_vec[i]), (double *)&(y_vec[i]));
        }
      else if (str == "polyline_3d")
        {
          if (redraw_ws) gr_polyline3d(2, (double *)&(x_vec[i]), (double *)&(y_vec[i]), (double *)&(z_vec[i]));
        }
    }
}

static std::shared_ptr<GRM::Element> getSubplotElement(const std::shared_ptr<GRM::Element> &element)
{
  auto ancestor = element;

  while (ancestor->localName() != "figure")
    {
      bool ancestor_is_subplot_group =
          (ancestor->hasAttribute("plot_group") && static_cast<int>(ancestor->getAttribute("plot_group")));
      if (ancestor->localName() == "layout_grid_element" || ancestor_is_subplot_group)
        {
          return ancestor;
        }
      else
        {
          ancestor = ancestor->parentElement();
        }
    }
  return nullptr;
}

static void getTickSize(const std::shared_ptr<GRM::Element> &element, double &tick_size)
{
  if (element->hasAttribute("tick_size") && element->parentElement()->localName() == "colorbar")
    {
      double plot_viewport[2], tick_size_rel;
      double metric_width, metric_height;
      bool keep_aspect_ratio = false, uniform_data = true, only_quadratic_aspect_ratio = false;
      auto plot_parent = element->parentElement();
      getPlotParent(plot_parent);

      plot_viewport[0] = static_cast<double>(plot_parent->getAttribute("_viewport_x_min_org"));
      plot_viewport[1] = static_cast<double>(plot_parent->getAttribute("_viewport_x_max_org"));

      GRM::Render::getFigureSize(nullptr, nullptr, &metric_width, &metric_height);
      auto aspect_ratio_ws = metric_width / metric_height;

      if (element->hasAttribute("_tick_size_set_by_user"))
        {
          tick_size = static_cast<double>(element->getAttribute("_tick_size_set_by_user"));
        }
      else
        {
          if (element->hasAttribute("_tick_size_org"))
            {
              tick_size = static_cast<double>(element->getAttribute("_tick_size_org"));
            }
          else
            {
              tick_size = static_cast<double>(element->getAttribute("tick_size"));
              element->setAttribute("_tick_size_org", tick_size);
            }
        }

      keep_aspect_ratio = static_cast<int>(plot_parent->getAttribute("keep_aspect_ratio"));
      only_quadratic_aspect_ratio = static_cast<int>(plot_parent->getAttribute("only_quadratic_aspect_ratio"));
      // special case for keep_aspect_ratio with uniform data which can lead to smaller plots
      if (keep_aspect_ratio && only_quadratic_aspect_ratio)
        {
          auto render = grm_get_render();
          auto kind = static_cast<std::string>(plot_parent->getAttribute("kind"));
          for (const auto &series : plot_parent->querySelectors("central_region")->children())
            {
              if (!starts_with(series->localName(), "series_")) continue;
              uniform_data = isUniformData(series, render->getContext());
              if (!uniform_data) break;
            }
          if (kind == "marginal_heatmap" && uniform_data)
            uniform_data = isUniformData(plot_parent->children()[0], render->getContext());
        }

      if ((keep_aspect_ratio && uniform_data && only_quadratic_aspect_ratio) || !keep_aspect_ratio)
        {
          if (aspect_ratio_ws <= 1)
            {
              tick_size_rel = tick_size * aspect_ratio_ws;
            }
          else
            {
              tick_size_rel = tick_size / aspect_ratio_ws;
            }
          if (!element->hasAttribute("_tick_size_set_by_user")) tick_size_rel *= DEFAULT_ASPECT_RATIO_FOR_SCALING;
        }
      else
        {
          double viewport[4];
          double default_diag_factor;
          std::shared_ptr<GRM::Element> central_region, central_region_parent;
          auto kind = static_cast<std::string>(plot_parent->getAttribute("kind"));

          central_region_parent = plot_parent;
          if (kind == "marginal_heatmap") central_region_parent = plot_parent->children()[0];
          for (const auto &child : central_region_parent->children())
            {
              if (child->localName() == "central_region")
                {
                  central_region = child;
                  break;
                }
            }

          viewport[0] = static_cast<double>(central_region->getAttribute("viewport_x_min"));
          viewport[1] = static_cast<double>(central_region->getAttribute("viewport_x_max"));
          viewport[2] = static_cast<double>(central_region->getAttribute("viewport_y_min"));
          viewport[3] = static_cast<double>(central_region->getAttribute("viewport_y_max"));
          auto start_aspect_ratio_ws = static_cast<double>(plot_parent->getAttribute("_start_aspect_ratio"));
          auto diag_factor = std::sqrt((viewport[1] - viewport[0]) * (viewport[1] - viewport[0]) +
                                       (viewport[3] - viewport[2]) * (viewport[3] - viewport[2]));
          if (element->hasAttribute("_default_diag_factor"))
            {
              default_diag_factor = static_cast<double>(element->getAttribute("_default_diag_factor"));
            }
          else
            {
              default_diag_factor =
                  ((DEFAULT_ASPECT_RATIO_FOR_SCALING) *
                   (start_aspect_ratio_ws <= 1 ? start_aspect_ratio_ws : (1.0 / start_aspect_ratio_ws))) /
                  diag_factor;
              element->setAttribute("_default_diag_factor", default_diag_factor);
            }
          tick_size_rel = tick_size * diag_factor * default_diag_factor;
        }
      tick_size = tick_size_rel;
    }
  else
    {
      double viewport[4];
      auto plot_element = getSubplotElement(element);
      std::shared_ptr<GRM::Element> central_region, central_region_parent;
      auto kind = static_cast<std::string>(plot_element->getAttribute("kind"));

      central_region_parent = plot_element;
      if (kind == "marginal_heatmap") central_region_parent = plot_element->children()[0];
      for (const auto &child : central_region_parent->children())
        {
          if (child->localName() == "central_region")
            {
              central_region = child;
              break;
            }
        }

      viewport[0] = static_cast<double>(central_region->getAttribute("viewport_x_min"));
      viewport[1] = static_cast<double>(central_region->getAttribute("viewport_x_max"));
      viewport[2] = static_cast<double>(central_region->getAttribute("viewport_y_min"));
      viewport[3] = static_cast<double>(central_region->getAttribute("viewport_y_max"));

      double diag = std::sqrt((viewport[1] - viewport[0]) * (viewport[1] - viewport[0]) +
                              (viewport[3] - viewport[2]) * (viewport[3] - viewport[2]));

      tick_size = PLOT_DEFAULT_AXES_TICK_SIZE * diag;
    }
}

static void getMajorCount(const std::shared_ptr<GRM::Element> &element, const std::string &kind, int &major_count)
{
  if (element->hasAttribute("major"))
    {
      major_count = static_cast<int>(element->getAttribute("major"));
    }
  else
    {
      if (str_equals_any(kind, "wireframe", "surface", "plot3", "scatter3", "polar", "trisurface",
                         "polar_heatmap", "nonuniform_polar_heatmap", "volume"))
        {
          major_count = 2;
        }
      else
        {
          major_count = 5;
        }
    }
}

static void getAxesInformation(const std::shared_ptr<GRM::Element> &element, const std::string &x_org_pos,
                               const std::string &y_org_pos, double &x_org, double &y_org, int &x_major, int &y_major,
                               double &x_tick, double &y_tick)
{
  double x_org_low, x_org_high;
  double y_org_low, y_org_high;
  int major_count;
  std::shared_ptr<GRM::Element> central_region, central_region_parent;

  auto subplot_element = getSubplotElement(element);
  auto kind = static_cast<std::string>(subplot_element->getAttribute("kind"));

  central_region_parent = subplot_element;
  if (kind == "marginal_heatmap") central_region_parent = subplot_element->children()[0];
  for (const auto &child : central_region_parent->children())
    {
      if (child->localName() == "central_region")
        {
          central_region = child;
          break;
        }
    }

  auto scale = static_cast<int>(subplot_element->getAttribute("scale"));
  auto xmin = static_cast<double>(central_region->getAttribute("window_x_min"));
  auto xmax = static_cast<double>(central_region->getAttribute("window_x_max"));
  auto ymin = static_cast<double>(central_region->getAttribute("window_y_min"));
  auto ymax = static_cast<double>(central_region->getAttribute("window_y_max"));

  getMajorCount(element, kind, major_count);

  if (scale & GR_OPTION_X_LOG)
    {
      x_major = 1;
    }
  else
    {
      if (element->hasAttribute("x_major"))
        {
          x_major = static_cast<int>(element->getAttribute("x_major"));
        }
      else
        {
          if (kind == "barplot")
            {
              bool problematic_bar_num = false;
              auto context = global_render->getContext();
              auto barplots = central_region->querySelectorsAll("series_barplot");
              for (const auto &barplot : barplots)
                {
                  if (!barplot->hasAttribute("style") ||
                      static_cast<std::string>(barplot->getAttribute("style")) == "default")
                    {
                      auto y_key = static_cast<std::string>(barplot->getAttribute("y"));
                      std::vector<double> y_vec = GRM::get<std::vector<double>>((*context)[y_key]);
                      if (size(y_vec) > 20)
                        {
                          problematic_bar_num = true;
                          break;
                        }
                    }
                }
              x_major = problematic_bar_num ? major_count : 1;
            }
          else
            {
              x_major = major_count;
            }
        }
      element->setAttribute("x_major", x_major);
    }

  if (scale & GR_OPTION_X_LOG &&
      !(element->hasAttribute("x_tick") && static_cast<std::string>(element->getAttribute("name")) == "colorbar"))
    {
      x_tick = 1;
    }
  else
    {
      if (element->hasAttribute("x_tick"))
        {
          x_tick = static_cast<double>(element->getAttribute("x_tick"));
        }
      else
        {
          if (kind == "barplot")
            {
              x_tick = 1;
            }
          else
            {
              if (x_major != 0)
                {
                  x_tick = autoTick(xmin, xmax) / x_major;
                }
              else
                {
                  x_tick = 1;
                }
            }
        }
    }

  if (scale & GR_OPTION_FLIP_X &&
      !(element->hasAttribute("x_org") && (static_cast<std::string>(element->getAttribute("name")) == "colorbar" ||
                                           element->localName() == "grid" || element->localName() == "grid3d")))
    {
      x_org_low = xmax;
      x_org_high = xmin;
      if (x_org_pos == "low")
        {
          x_org = x_org_low;
        }
      else
        {
          x_org = x_org_high;
          x_major = -x_major;
        }
    }
  else
    {
      if (element->hasAttribute("x_org"))
        {
          x_org = static_cast<double>(element->getAttribute("x_org"));
        }
      else
        {
          x_org_low = xmin;
          x_org_high = xmax;
          if (x_org_pos == "low")
            {
              x_org = x_org_low;
            }
          else
            {
              x_org = x_org_high;
              x_major = -x_major;
            }
        }
    }

  if (scale & GR_OPTION_Y_LOG)
    {
      y_major = 1;
    }
  else
    {
      if (element->hasAttribute("y_major"))
        {
          y_major = static_cast<int>(element->getAttribute("y_major"));
        }
      else
        {
          y_major = major_count;
          element->setAttribute("y_major", y_major);
        }
    }

  if (scale & GR_OPTION_Y_LOG &&
      !((element->localName() == "axes_3d" || static_cast<std::string>(element->getAttribute("name")) == "colorbar") &&
        element->hasAttribute("y_tick")))
    {
      y_tick = 1;
    }
  else
    {
      if (element->hasAttribute("y_tick"))
        {
          y_tick = static_cast<double>(element->getAttribute("y_tick"));
        }
      else
        {
          if (y_major != 0)
            {
              y_tick = autoTick(ymin, ymax) / y_major;
            }
          else
            {
              y_tick = 1;
            }
        }
    }

  if (scale & GR_OPTION_FLIP_Y &&
      !(element->hasAttribute("y_org") && (static_cast<std::string>(element->getAttribute("name")) == "colorbar" ||
                                           element->localName() == "grid" || element->localName() == "grid3d")))
    {
      y_org_low = ymax;
      y_org_high = ymin;
      if (y_org_pos == "low")
        {
          y_org = y_org_low;
        }
      else
        {
          y_org = y_org_high;
          y_major = -y_major;
        }
    }
  else
    {
      if (element->hasAttribute("y_org"))
        {
          y_org = static_cast<double>(element->getAttribute("y_org"));
        }
      else
        {
          y_org_low = ymin;
          y_org_high = ymax;
          if (y_org_pos == "low")
            {
              y_org = y_org_low;
            }
          else
            {
              y_org = y_org_high;
              y_major = -y_major;
            }
        }
    }
}

static void getAxes3dInformation(const std::shared_ptr<GRM::Element> &element, const std::string &x_org_pos,
                                 const std::string &y_org_pos, const std::string &z_org_pos, double &x_org,
                                 double &y_org, double &z_org, int &x_major, int &y_major, int &z_major, double &x_tick,
                                 double &y_tick, double &z_tick)
{
  getAxesInformation(element, x_org_pos, y_org_pos, x_org, y_org, x_major, y_major, x_tick, y_tick);

  double z_org_low, z_org_high;
  int major_count;
  std::shared_ptr<GRM::Element> central_region;

  auto draw_axes_group = element->parentElement();
  auto subplot_element = getSubplotElement(element);
  for (const auto &child : subplot_element->children()) // don't need special case for marginal_heatmap cause it's 3d
    {
      if (child->localName() == "central_region")
        {
          central_region = child;
          break;
        }
    }

  auto kind = static_cast<std::string>(subplot_element->getAttribute("kind"));
  auto scale = static_cast<int>(subplot_element->getAttribute("scale"));
  auto zmin = static_cast<double>(central_region->getAttribute("window_z_min"));
  auto zmax = static_cast<double>(central_region->getAttribute("window_z_max"));

  getMajorCount(element, kind, major_count);

  if (scale & GR_OPTION_Z_LOG)
    {
      z_major = 1;
    }
  else
    {
      if (element->hasAttribute("z_major"))
        {
          z_major = static_cast<int>(element->getAttribute("z_major"));
        }
      else
        {
          z_major = major_count;
        }
    }

  if (scale & GR_OPTION_Z_LOG && !(element->localName() == "axes_3d" && element->hasAttribute("z_tick")))
    {
      z_tick = 1;
    }
  else
    {
      if (element->hasAttribute("z_tick"))
        {
          z_tick = static_cast<double>(element->getAttribute("z_tick"));
        }
      else
        {
          if (z_major != 0)
            {
              z_tick = autoTick(zmin, zmax) / z_major;
            }
          else
            {
              z_tick = 1;
            }
        }
    }

  if (scale & GR_OPTION_FLIP_Z)
    {
      z_org_low = zmax;
      z_org_high = zmin;
      if (z_org_pos == "low")
        {
          z_org = z_org_low;
        }
      else
        {
          z_org = z_org_high;
          z_major = -z_major;
        }
    }
  else
    {
      if (element->hasAttribute("z_org"))
        {
          z_org = static_cast<double>(element->getAttribute("z_org"));
        }
      else
        {
          z_org_low = zmin;
          z_org_high = zmax;
          if (z_org_pos == "low")
            {
              z_org = z_org_low;
            }
          else
            {
              z_org = z_org_high;
              z_major = -z_major;
            }
        }
    }
}

void GRM::Render::getFigureSize(int *pixel_width, int *pixel_height, double *metric_width, double *metric_height)
{
  double display_metric_width, display_metric_height;
  int display_pixel_width, display_pixel_height;
  double dpm[2], dpi[2];
  int tmp_size_i[2], pixel_size[2];
  double tmp_size_d[2], metric_size[2];
  int i;
  std::string size_unit, size_type;
  std::array<std::string, 2> vars = {"x", "y"};
  std::array<double, 2> default_size = {PLOT_DEFAULT_WIDTH, PLOT_DEFAULT_HEIGHT};
  std::shared_ptr<GRM::Element> figure = active_figure;

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
  if (figure->hasAttribute("fig_size_x") && figure->hasAttribute("fig_size_y"))
    {
      tmp_size_d[0] = static_cast<double>(figure->getAttribute("fig_size_x"));
      tmp_size_d[1] = static_cast<double>(figure->getAttribute("fig_size_y"));
      for (i = 0; i < 2; ++i)
        {
          pixel_size[i] = (int)grm_round(tmp_size_d[i] * dpi[i]);
          metric_size[i] = tmp_size_d[i] / 0.0254;
        }
    }
  else if (figure->hasAttribute("size_x") && figure->hasAttribute("size_y"))
    {
      for (i = 0; i < 2; ++i)
        {
          size_unit = static_cast<std::string>(figure->getAttribute("size_" + vars[i] + "_unit"));
          size_type = static_cast<std::string>(figure->getAttribute("size_" + vars[i] + "_type"));
          if (size_unit.empty()) size_unit = "px";
          tmp_size_d[i] = default_size[i];

          if (size_type == "double" || size_type == "int")
            {
              tmp_size_d[i] = static_cast<double>(figure->getAttribute("size_" + vars[i]));
              auto meters_per_unit_iter = symbol_to_meters_per_unit.find(size_unit);
              if (meters_per_unit_iter != symbol_to_meters_per_unit.end())
                {
                  double meters_per_unit = meters_per_unit_iter->second;
                  double pixels_per_unit = meters_per_unit * dpm[i];

                  tmp_size_d[i] *= pixels_per_unit;
                }
            }
          pixel_size[i] = (int)grm_round(tmp_size_d[i]);
          metric_size[i] = tmp_size_d[i] / dpm[i];
        }
    }
  else
    {
      pixel_size[0] = (int)grm_round(PLOT_DEFAULT_WIDTH);
      pixel_size[1] = (int)grm_round(PLOT_DEFAULT_HEIGHT);
      metric_size[0] = PLOT_DEFAULT_WIDTH / dpm[0];
      metric_size[1] = PLOT_DEFAULT_HEIGHT / dpm[1];
    }

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
}

static void setNextColor(gr_color_type_t color_type, std::vector<int> &color_indices,
                         std::vector<double> &color_rgb_values, const std::shared_ptr<GRM::Element> &element)
{
  // TODO: is this method really needed? Cant it be replaced by set_next_color?
  const static std::vector<int> fallback_color_indices{989, 982, 980, 981, 996, 983, 995, 988, 986, 990,
                                                       991, 984, 992, 993, 994, 987, 985, 997, 998, 999};
  static double saved_color[3];
  static int last_array_index = -1;
  static unsigned int color_array_length = -1;
  int current_array_index = last_array_index + 1;
  int color_index = 0;
  int reset = (color_type == GR_COLOR_RESET);
  int gks_err_ind = GKS_K_NO_ERROR;

  if (reset)
    {
      last_array_index = -1;
      color_array_length = -1;
      return;
    }

  if (color_indices.empty() && color_rgb_values.empty())
    {
      color_indices = fallback_color_indices;
    }

  if (last_array_index < 0 && !color_rgb_values.empty())
    {
      gks_inq_color_rep(1, PLOT_CUSTOM_COLOR_INDEX, GKS_K_VALUE_SET, &gks_err_ind, &saved_color[0], &saved_color[1],
                        &saved_color[2]);
    }

  current_array_index %= color_array_length;

  if (!color_indices.empty())
    {
      color_index = color_indices[current_array_index];
      last_array_index = current_array_index;
    }
  else if (!color_rgb_values.empty())
    {
      color_index = PLOT_CUSTOM_COLOR_INDEX;
      global_render->setColorRep(element, PLOT_CUSTOM_COLOR_INDEX, color_rgb_values[current_array_index],
                                 color_rgb_values[current_array_index + 1], color_rgb_values[current_array_index + 2]);
      last_array_index = current_array_index + 2;
    }

  if (color_type & GR_COLOR_LINE)
    {
      global_render->setLineColorInd(element, color_index);
    }
  if (color_type & GR_COLOR_MARKER)
    {
      global_render->setMarkerColorInd(element, color_index);
    }
  if (color_type & GR_COLOR_FILL)
    {
      global_render->setFillColorInd(element, color_index);
    }
  if (color_type & GR_COLOR_TEXT)
    {
      global_render->setTextColorInd(element, color_index);
    }
  if (color_type & GR_COLOR_BORDER)
    {
      global_render->setBorderColorInd(element, color_index);
    }
}

void receiverFunction(int id, double x_min, double x_max, double y_min, double y_max)
{
  if (!(x_min == DBL_MAX || x_max == -DBL_MAX || y_min == DBL_MAX || y_max == -DBL_MAX))
    {
      bounding_map[id]->setAttribute("_bbox_id", id);
      bounding_map[id]->setAttribute("_bbox_x_min", x_min);
      bounding_map[id]->setAttribute("_bbox_x_max", x_max);
      bounding_map[id]->setAttribute("_bbox_y_min", y_min);
      bounding_map[id]->setAttribute("_bbox_y_max", y_max);
    }
}

static bool getLimitsForColorbar(const std::shared_ptr<GRM::Element> &element, double &c_min, double &c_max)
{
  bool limits_found = true;
  auto plot_parent = element->parentElement();
  getPlotParent(plot_parent);

  if (!std::isnan(static_cast<double>(plot_parent->getAttribute("_c_lim_min"))) &&
      !std::isnan(static_cast<double>(plot_parent->getAttribute("_c_lim_max"))))
    {
      c_min = static_cast<double>(plot_parent->getAttribute("_c_lim_min"));
      c_max = static_cast<double>(plot_parent->getAttribute("_c_lim_max"));
    }
  else if (!std::isnan(static_cast<double>(plot_parent->getAttribute("_z_lim_min"))) &&
           !std::isnan(static_cast<double>(plot_parent->getAttribute("_z_lim_max"))))
    {
      c_min = static_cast<double>(plot_parent->getAttribute("_z_lim_min"));
      c_max = static_cast<double>(plot_parent->getAttribute("_z_lim_max"));
    }
  else
    {
      limits_found = false;
    }

  return limits_found;
}

static double findMaxStep(unsigned int n, std::vector<double> x)
{
  double max_step = 0.0;
  unsigned int i;

  if (n >= 2)
    {
      for (i = 1; i < n; ++i)
        {
          max_step = grm_max(x[i] - x[i - 1], max_step);
        }
    }

  return max_step;
}

static void extendErrorBars(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context,
                            std::vector<double> x, std::vector<double> y)
{
  auto id = static_cast<int>(global_root->getAttribute("_id"));
  auto str = std::to_string(id);
  global_root->setAttribute("_id", ++id);

  (*context)["x" + str] = x;
  element->setAttribute("x", "x" + str);
  (*context)["y" + str] = y;
  element->setAttribute("y", "y" + str);
}

std::vector<std::string> getSizeUnits()
{
  std::vector<std::string> size_units;
  size_units.reserve(symbol_to_meters_per_unit.size());
  for (auto const &imap : symbol_to_meters_per_unit) size_units.push_back(imap.first);
  return size_units;
}

std::vector<std::string> getColormaps()
{
  std::vector<std::string> colormaps;
  colormaps.reserve(colormap_string_to_int.size());
  for (auto const &imap : colormap_string_to_int) colormaps.push_back(imap.first);
  return colormaps;
}

std::vector<std::string> getFonts()
{
  std::vector<std::string> fonts;
  fonts.reserve(font_string_to_int.size());
  for (auto const &imap : font_string_to_int) fonts.push_back(imap.first);
  return fonts;
}

std::vector<std::string> getFontPrecisions()
{
  std::vector<std::string> font_precisions;
  font_precisions.reserve(font_precision_string_to_int.size());
  for (auto const &imap : font_precision_string_to_int) font_precisions.push_back(imap.first);
  return font_precisions;
}

std::vector<std::string> getLineTypes()
{
  std::vector<std::string> line_types;
  line_types.reserve(line_type_string_to_int.size());
  for (auto const &imap : line_type_string_to_int) line_types.push_back(imap.first);
  return line_types;
}

std::vector<std::string> getLocations()
{
  std::vector<std::string> locations;
  locations.reserve(location_string_to_int.size());
  for (auto const &imap : location_string_to_int) locations.push_back(imap.first);
  return locations;
}

std::vector<std::string> getMarkerTypes()
{
  std::vector<std::string> marker_types;
  marker_types.reserve(marker_type_string_to_int.size());
  for (auto const &imap : marker_type_string_to_int) marker_types.push_back(imap.first);
  return marker_types;
}

std::vector<std::string> getTextAlignHorizontal()
{
  std::vector<std::string> text_align_horizontal;
  text_align_horizontal.reserve(text_align_horizontal_string_to_int.size());
  for (auto const &imap : text_align_horizontal_string_to_int) text_align_horizontal.push_back(imap.first);
  return text_align_horizontal;
}

std::vector<std::string> getTextAlignVertical()
{
  std::vector<std::string> text_align_vertical;
  text_align_vertical.reserve(text_align_vertical_string_to_int.size());
  for (auto const &imap : text_align_vertical_string_to_int) text_align_vertical.push_back(imap.first);
  return text_align_vertical;
}

std::vector<std::string> getAlgorithm()
{
  std::vector<std::string> algorithm;
  algorithm.reserve(algorithm_string_to_int.size());
  for (auto const &imap : algorithm_string_to_int) algorithm.push_back(imap.first);
  return algorithm;
}

std::vector<std::string> getModel()
{
  std::vector<std::string> model;
  model.reserve(model_string_to_int.size());
  for (auto const &imap : model_string_to_int) model.push_back(imap.first);
  return model;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ attribute processing functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static void processAlpha(const std::shared_ptr<GRM::Element> &element)
{
  gr_settransparency(static_cast<double>(element->getAttribute("alpha")));
}

static void processBorderColorInd(const std::shared_ptr<GRM::Element> &element)
{
  gr_setbordercolorind(static_cast<int>(element->getAttribute("border_color_ind")));
}

static void processMarginalHeatmapSidePlot(const std::shared_ptr<GRM::Element> &element)
{
  double viewport[4], window[4];
  double x_min, x_max, y_min, y_max, c_max;
  auto kind = static_cast<std::string>(element->getAttribute("kind"));

  if (element->parentElement()->localName() == "marginal_heatmap_plot" &&
      element->parentElement()->hasAttribute("marginal_heatmap_kind"))
    {
      auto location = static_cast<std::string>(element->getAttribute("location"));
      auto plot_group = element->parentElement();
      getPlotParent(plot_group);
      applyMoveTransformation(element);
      viewport[0] = static_cast<double>(element->getAttribute("viewport_x_min"));
      viewport[1] = static_cast<double>(element->getAttribute("viewport_x_max"));
      viewport[2] = static_cast<double>(element->getAttribute("viewport_y_min"));
      viewport[3] = static_cast<double>(element->getAttribute("viewport_y_max"));
      x_min = static_cast<double>(plot_group->getAttribute("_x_lim_min"));
      x_max = static_cast<double>(plot_group->getAttribute("_x_lim_max"));
      y_min = static_cast<double>(plot_group->getAttribute("_y_lim_min"));
      y_max = static_cast<double>(plot_group->getAttribute("_y_lim_max"));
      if (!std::isnan(static_cast<double>(plot_group->getAttribute("_c_lim_max"))))
        {
          c_max = static_cast<double>(plot_group->getAttribute("_c_lim_max"));
        }
      else
        {
          c_max = static_cast<double>(plot_group->getAttribute("_z_lim_max"));
        }

      if (element->hasAttribute("window_x_min")) window[0] = static_cast<double>(element->getAttribute("window_x_min"));
      if (element->hasAttribute("window_x_max")) window[1] = static_cast<double>(element->getAttribute("window_x_max"));
      if (element->hasAttribute("window_y_min")) window[2] = static_cast<double>(element->getAttribute("window_y_min"));
      if (element->hasAttribute("window_y_max")) window[3] = static_cast<double>(element->getAttribute("window_y_max"));
      if (location == "right")
        {
          window[0] = 0.0;
          window[1] = c_max / 10;
          window[2] = y_min;
          window[3] = y_max;
        }
      else if (location == "top")
        {
          window[0] = x_min;
          window[1] = x_max;
          window[2] = 0.0;
          window[3] = c_max / 10;
        }
      grm_get_render()->setWindow(element, window[0], window[1], window[2], window[3]);
      grm_get_render()->processWindow(element);
      calculateViewport(element);
      applyMoveTransformation(element);
    }
}

static void processCharExpan(const std::shared_ptr<GRM::Element> &element)
{
  gr_setcharexpan(static_cast<double>(element->getAttribute("char_expan")));
}

static void processCharHeight(const std::shared_ptr<GRM::Element> &element)
{
  gr_setcharheight(static_cast<double>(element->getAttribute("char_height")));
}

static void processCharSpace(const std::shared_ptr<GRM::Element> &element)
{
  gr_setcharspace(static_cast<double>(element->getAttribute("char_space")));
}

static void processCharUp(const std::shared_ptr<GRM::Element> &element)
{
  gr_setcharup(static_cast<double>(element->getAttribute("char_up_x")),
               static_cast<double>(element->getAttribute("char_up_y")));
}

static void processClipTransformation(const std::shared_ptr<GRM::Element> &element)
{
  gr_selectclipxform(static_cast<int>(element->getAttribute("clip_transformation")));
}

int colormapStringToInt(const std::string &colormap_str)
{
  return colormap_string_to_int[colormap_str];
}

std::string colormapIntToString(int colormap)
{
  for (auto const &map_elem : colormap_string_to_int)
    {
      if (map_elem.second == colormap)
        {
          return map_elem.first;
        }
    }
}

static void processColormap(const std::shared_ptr<GRM::Element> &element)
{
  int colormap;
  if (element->getAttribute("colormap").isInt())
    {
      colormap = static_cast<int>(element->getAttribute("colormap"));
    }
  else if (element->getAttribute("colormap").isString())
    {
      colormap = colormapStringToInt(static_cast<std::string>(element->getAttribute("colormap")));
    }

  gr_setcolormap(colormap);
}

static void processColorRep(const std::shared_ptr<GRM::Element> &element, const std::string attribute)
{
  int index, hex_int;
  double red, green, blue;
  std::string name, hex_string;
  std::stringstream string_stream;

  auto end = attribute.find('.');
  index = std::stoi(attribute.substr(end + 1, attribute.size()));

  hex_int = 0;
  hex_string = static_cast<std::string>(element->getAttribute(attribute));
  string_stream << std::hex << hex_string;
  string_stream >> hex_int;

  red = ((hex_int >> 16) & 0xFF) / 255.0;
  green = ((hex_int >> 8) & 0xFF) / 255.0;
  blue = ((hex_int)&0xFF) / 255.0;

  gr_setcolorrep(index, red, green, blue);
}

static void processColorReps(const std::shared_ptr<GRM::Element> &element)
{
  for (auto &attr : element->getAttributeNames())
    {
      auto start = 0U;
      auto end = attr.find('.');
      if (attr.substr(start, end) == "colorrep")
        {
          processColorRep(element, attr);
        }
    }
}

static void processFillColorInd(const std::shared_ptr<GRM::Element> &element)
{
  gr_setfillcolorind(static_cast<int>(element->getAttribute("fill_color_ind")));
}

int fillIntStyleStringToInt(const std::string &fill_int_style_str)
{
  if (fill_int_style_str == "hollow")
    return 0;
  else if (fill_int_style_str == "solid")
    return 1;
  else if (fill_int_style_str == "pattern")
    return 2;
  else if (fill_int_style_str == "hatch")
    return 3;
  else if (fill_int_style_str == "solid_with_border")
    return 4;
  else
    {
      logger((stderr, "Got unknown fill_int_style \"%s\"\n", fill_int_style_str.c_str()));
      throw std::logic_error("The given fill_int_style is unknown.\n");
    }
}

std::string fillIntStyleIntToString(int fill_int_style)
{
  if (fill_int_style == 0)
    return "hollow";
  else if (fill_int_style == 1)
    return "solid";
  else if (fill_int_style == 2)
    return "pattern";
  else if (fill_int_style == 3)
    return "hatch";
  else if (fill_int_style == 4)
    return "solid_with_border";
  else
    {
      logger((stderr, "Got unknown fill_int_style \"%i\"\n", fill_int_style));
      throw std::logic_error("The given fill_int_style is unknown.\n");
    }
}

static void processFillIntStyle(const std::shared_ptr<GRM::Element> &element)
{
  int fill_int_style;
  if (element->getAttribute("fill_int_style").isInt())
    {
      fill_int_style = static_cast<int>(element->getAttribute("fill_int_style"));
    }
  else if (element->getAttribute("fill_int_style").isString())
    {
      fill_int_style = fillIntStyleStringToInt(static_cast<std::string>(element->getAttribute("fill_int_style")));
    }
  gr_setfillintstyle(fill_int_style);
}

int fillStyleStringToInt(const std::string &fill_style_str)
{
  if (fill_style_str == "hollow")
    return 0;
  else if (fill_style_str == "solid")
    return 1;
  else if (fill_style_str == "pattern")
    return 2;
  else if (fill_style_str == "hatch")
    return 3;
  else if (fill_style_str == "solid_with_border")
    return 4;
  else
    {
      logger((stderr, "Got unknown fill_style \"%s\"\n", fill_style_str.c_str()));
      throw std::logic_error("The given fill_style is unknown.\n");
    }
}

std::string fillStyleIntToString(int fill_style)
{
  if (fill_style == 0)
    return "hollow";
  else if (fill_style == 1)
    return "solid";
  else if (fill_style == 2)
    return "pattern";
  else if (fill_style == 3)
    return "hatch";
  else if (fill_style == 4)
    return "solid_with_border";
  else
    {
      logger((stderr, "Got unknown fill_style \"%i\"\n", fill_style));
      throw std::logic_error("The given fill_style is unknown.\n");
    }
}

static void processFillStyle(const std::shared_ptr<GRM::Element> &element)
{
  int fill_style;
  if (element->getAttribute("fill_style").isInt())
    {
      fill_style = static_cast<int>(element->getAttribute("fill_int_style"));
    }
  else if (element->getAttribute("fill_style").isString())
    {
      fill_style = fillStyleStringToInt(static_cast<std::string>(element->getAttribute("fill_int_style")));
    }
  gr_setfillstyle(fill_style);
}

static void processFlip(const std::shared_ptr<GRM::Element> &element)
{
  int options;
  bool x_flip = static_cast<int>(element->getAttribute("x_flip"));
  bool y_flip = static_cast<int>(element->getAttribute("y_flip"));

  gr_inqscale(&options);
  if (x_flip)
    {
      options = options | GR_OPTION_FLIP_X;
    }
  else
    {
      options = options & ~GR_OPTION_FLIP_X;
    }
  if (y_flip)
    {
      options = options | GR_OPTION_FLIP_Y;
    }
  else
    {
      options = options & ~GR_OPTION_FLIP_Y;
    }
  gr_setscale(options);
}

int fontStringToInt(const std::string &font_str)
{
  return font_string_to_int[font_str];
}

std::string fontIntToString(int font)
{
  for (auto const &map_elem : font_string_to_int)
    {
      if (map_elem.second == font)
        {
          return map_elem.first;
        }
    }
}

int fontPrecisionStringToInt(const std::string &font_precision_str)
{
  return font_precision_string_to_int[font_precision_str];
}

std::string fontPrecisionIntToString(int font_precision)
{
  for (auto const &map_elem : font_precision_string_to_int)
    {
      if (map_elem.second == font_precision)
        {
          return map_elem.first;
        }
    }
}

static void processFont(const std::shared_ptr<GRM::Element> &element)
{
  int font = PLOT_DEFAULT_FONT, font_precision = PLOT_DEFAULT_FONT_PRECISION;

  /* `font` and `font_precision` are always set */
  if (element->hasAttribute("font_precision"))
    {
      if (element->getAttribute("font_precision").isInt())
        {
          font_precision = static_cast<int>(element->getAttribute("font_precision"));
        }
      else if (element->getAttribute("font_precision").isString())
        {
          font_precision = fontPrecisionStringToInt(static_cast<std::string>(element->getAttribute("font_precision")));
        }
    }
  else
    {
      logger((stderr, "Use default font precision\n"));
    }
  if (element->hasAttribute("font"))
    {
      if (element->getAttribute("font").isInt())
        {
          font = static_cast<int>(element->getAttribute("font"));
        }
      else if (element->getAttribute("font").isString())
        {
          font = fontStringToInt(static_cast<std::string>(element->getAttribute("font")));
        }
    }
  else
    {
      logger((stderr, "Use default font\n"));
    }
  logger((stderr, "Using font: %d with precision %d\n", font, font_precision));
  gr_settextfontprec(font, font_precision);
  /* TODO: Implement other datatypes for `font` and `font_precision` */
}

static void processIntegral(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  double int_lim_low = 0, int_lim_high;
  std::vector<double> x_vec, y_vec, f1, f2;
  int x_length;
  del_values del = del_values::update_without_default;
  int child_id = 0, id, i;
  std::shared_ptr<GRM::Element> fill_area, left_border, right_border;
  std::string str;
  auto subplot_element = getSubplotElement(element);
  auto series_element = element->parentElement()->parentElement();
  double x_shift = 0;
  double x1, x2;

  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  if (element->hasAttribute("int_lim_low"))
    {
      int_lim_low = static_cast<double>(element->getAttribute("int_lim_low"));
    }
  /* is always given from definition */
  int_lim_high = static_cast<double>(element->getAttribute("int_lim_high"));

  /* if there is a shift defined for x, get the value to shift the x-values in later calculation */
  fill_area = element->querySelectors("fill_area[_child_id=" + std::to_string(child_id) + "]");
  if (fill_area != nullptr && fill_area->hasAttribute("x_shift_wc") &&
      !(fill_area->hasAttribute("disable_x_trans") && static_cast<int>(fill_area->getAttribute("disable_x_trans"))))
    {
      x_shift = static_cast<double>(fill_area->getAttribute("x_shift_wc"));
      int_lim_low += x_shift;
      int_lim_high += x_shift;

      /* trigger an event with the new integral size so that the user can use it for calculations */
      event_queue_enqueue_integral_update_event(event_queue, int_lim_low, int_lim_high);
    }
  else if ((fill_area == nullptr || (fill_area != nullptr && !fill_area->hasAttribute("x_shift_wc"))) &&
           element->hasAttribute("x_shift_wc") &&
           !(element->hasAttribute("disable_x_trans") && static_cast<int>(element->getAttribute("disable_x_trans"))))
    {
      x_shift = static_cast<double>(element->getAttribute("x_shift_wc"));
      int_lim_low += x_shift;
      int_lim_high += x_shift;

      /* trigger an event with the new integral size so that the user can use it for calculations */
      event_queue_enqueue_integral_update_event(event_queue, int_lim_low, int_lim_high);
    }

  if (int_lim_high < int_lim_low)
    {
      fprintf(stderr, "Integral low limit is greater than the high limit. The limits gets swapped.\n");
      auto tmp = int_lim_high;
      int_lim_high = int_lim_low;
      int_lim_low = tmp;
    }

  auto y = static_cast<std::string>(series_element->getAttribute("y"));
  y_vec = GRM::get<std::vector<double>>((*context)[y]);

  auto x = static_cast<std::string>(series_element->getAttribute("x"));
  x_vec = GRM::get<std::vector<double>>((*context)[x]);
  x_length = (int)x_vec.size();

  /* get all points for the fill area from the current line */
  x1 = (int_lim_low < x_vec[0]) ? x_vec[0] - x_shift : int_lim_low - x_shift;
  f1.push_back(x1);
  f2.push_back((static_cast<int>(subplot_element->getAttribute("y_log"))) ? 1 : 0);
  for (i = 0; i < x_length; i++)
    {
      if (grm_isnan(x_vec[i]) || grm_isnan(y_vec[i])) continue;
      if (x_vec[i] < int_lim_low && x_vec[i + 1] > int_lim_low)
        {
          f1.push_back(int_lim_low - x_shift);
          f2.push_back(y_vec[i] + (1 - abs(x_vec[i + 1] - int_lim_low)) * (y_vec[i + 1] - y_vec[i]));
        }
      if (x_vec[i] >= int_lim_low && x_vec[i] <= int_lim_high)
        {
          f1.push_back(x_vec[i] - x_shift);
          f2.push_back(y_vec[i]);
        }
      if (x_vec[i] < int_lim_high && x_vec[i + 1] > int_lim_high)
        {
          f1.push_back(int_lim_high - x_shift);
          f2.push_back(y_vec[i] + (1 - abs(x_vec[i + 1] - int_lim_high)) * (y_vec[i + 1] - y_vec[i]));
        }
    }
  x2 = (x_vec[x_length - 1] < int_lim_high) ? x_vec[x_length - 1] - x_shift : int_lim_high - x_shift;
  f1.push_back(x2);
  f2.push_back((static_cast<int>(subplot_element->getAttribute("y_log"))) ? 1 : 0);

  id = static_cast<int>(global_root->getAttribute("_id"));
  global_root->setAttribute("_id", id + 1);
  str = std::to_string(id);

  if ((del != del_values::update_without_default && del != del_values::update_with_default))
    {
      fill_area = global_render->createFillArea("x" + str, f1, "y" + str, f2);
      fill_area->setAttribute("_child_id", child_id++);
      element->append(fill_area);
    }
  else
    {
      fill_area = element->querySelectors("fill_area[_child_id=" + std::to_string(child_id++) + "]");
      if (fill_area != nullptr)
        global_render->createFillArea("x" + str, f1, "y" + str, f2, nullptr, 0, 0, -1, fill_area);
    }
  if (fill_area != nullptr)
    {
      int fill_color_ind = 989, fill_int_style = 2;
      /* when there is a color defined on the series element use it */
      if (series_element->hasAttribute("line_color_ind"))
        fill_color_ind = static_cast<int>(series_element->getAttribute("line_color_ind"));
      /* when there is a color defined on the integral_group element use it */
      if (element->parentElement()->hasAttribute("fill_color_ind"))
        fill_color_ind = static_cast<int>(element->parentElement()->getAttribute("fill_color_ind"));
      /* when there is a color defined on the polyline use it no matter if there was a color defined on the series */
      for (const auto &elem : series_element->querySelectorsAll("polyline[_child_id=0]"))
        {
          if (elem->hasAttribute("line_color_ind"))
            fill_color_ind = static_cast<int>(elem->getAttribute("line_color_ind"));
        }
      /* color on the integral element has the highest priority */
      if (element->hasAttribute("fill_color_ind"))
        fill_color_ind = static_cast<int>(element->getAttribute("fill_color_ind"));
      fill_area->setAttribute("fill_color_ind", fill_color_ind);
      /* when there is a fill int style defined on the integral_group element use it */
      if (element->parentElement()->hasAttribute("fill_int_style"))
        fill_int_style = static_cast<int>(element->parentElement()->getAttribute("fill_int_style"));
      if (element->hasAttribute("fill_int_style"))
        fill_int_style = static_cast<int>(element->getAttribute("fill_int_style"));
      fill_area->setAttribute("fill_int_style", fill_int_style);
      fill_area->setAttribute("name", "integral");
    }

  x1 += x_shift;
  if ((del != del_values::update_without_default && del != del_values::update_with_default))
    {
      left_border = global_render->createPolyline(x1, x1, 0, f2[1]);
      left_border->setAttribute("_child_id", child_id++);
      element->append(left_border);
    }
  else
    {
      left_border = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
      if (left_border != nullptr) global_render->createPolyline(x1, x1, 0, f2[1], 0, 0.0, 0, left_border);
    }
  if (left_border != nullptr)
    {
      int line_color_ind = 1;
      double alpha = 0;
      if (element->hasAttribute("line_color_ind"))
        line_color_ind = static_cast<int>(element->getAttribute("line_color_ind"));
      left_border->setAttribute("line_color_ind", line_color_ind);
      left_border->setAttribute("name", "integral_left");
      if (left_border->hasAttribute("alpha")) alpha = static_cast<double>(left_border->getAttribute("alpha"));
      left_border->setAttribute("alpha", alpha);
    }

  x2 += x_shift;
  if ((del != del_values::update_without_default && del != del_values::update_with_default))
    {
      right_border = global_render->createPolyline(x2, x2, 0, f2[f2.size() - 2]);
      right_border->setAttribute("_child_id", child_id++);
      element->append(right_border);
    }
  else
    {
      right_border = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
      if (right_border != nullptr) global_render->createPolyline(x2, x2, 0, f2[f2.size() - 2], 0, 0.0, 0, right_border);
    }
  if (right_border != nullptr)
    {
      int line_color_ind = 1;
      double alpha = 0;
      if (element->hasAttribute("line_color_ind"))
        line_color_ind = static_cast<int>(element->getAttribute("line_color_ind"));
      right_border->setAttribute("line_color_ind", line_color_ind);
      right_border->setAttribute("name", "integral_right");
      if (right_border->hasAttribute("alpha")) alpha = static_cast<double>(right_border->getAttribute("alpha"));
      right_border->setAttribute("alpha", alpha);
    }
}

static void processIntegralGroup(const std::shared_ptr<GRM::Element> &element,
                                 const std::shared_ptr<GRM::Context> &context)
{
  std::vector<double> int_limits_high_vec, int_limits_low_vec;
  int limits_high_num = 0, limits_low_num = 0;
  del_values del = del_values::update_without_default;
  int child_id = 0, id, i;
  std::shared_ptr<GRM::Element> integral;
  std::string str;
  auto subplot_element = getSubplotElement(element);

  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  /* the following 2 attributes are required, so they must be set */
  if (!element->hasAttribute("int_limits_high")) throw NotFoundError("Missing required attribute int_limits_high");
  auto limits_high_key = static_cast<std::string>(element->getAttribute("int_limits_high"));
  int_limits_high_vec = GRM::get<std::vector<double>>((*context)[limits_high_key]);
  limits_high_num = (int)int_limits_high_vec.size();

  if (!element->hasAttribute("int_limits_low")) throw NotFoundError("Missing required attribute int_limits_low");
  auto limits_low_key = static_cast<std::string>(element->getAttribute("int_limits_low"));
  int_limits_low_vec = GRM::get<std::vector<double>>((*context)[limits_low_key]);
  limits_low_num = (int)int_limits_low_vec.size();

  if (limits_low_num != limits_high_num) throw std::length_error("Both limits must have the same number of arguments");

  /* create or update all the children */
  for (i = 0; i < limits_low_num; i++)
    {
      if ((del != del_values::update_without_default && del != del_values::update_with_default))
        {
          integral = global_render->createIntegral(int_limits_low_vec[i], int_limits_high_vec[i]);
          integral->setAttribute("_child_id", child_id++);
          element->append(integral);
        }
      else
        {
          integral = element->querySelectors("integral[_child_id=" + std::to_string(child_id++) + "]");
          if (integral != nullptr)
            global_render->createIntegral(int_limits_low_vec[i], int_limits_high_vec[i], integral);
        }
    }
}

static void processMarginalHeatmapKind(const std::shared_ptr<GRM::Element> &element)
{
  auto mkind = static_cast<std::string>(element->getAttribute("marginal_heatmap_kind"));
  for (const auto &side_region : element->children())
    {
      if (!side_region->hasAttribute("marginal_heatmap_side_plot") ||
          !side_region->querySelectors("side_plot_region") ||
          static_cast<int>(element->getAttribute("_delete_children")) >= 2)
        continue;
      if (mkind == "line")
        {
          for (const auto &series : side_region->querySelectors("side_plot_region")->children())
            {
              // when processing all elements the first side_region has a series with xi while the second side_regions
              // wasn't processed yet so the series doesn't has the xi attribute; so we skip this side_region/series
              if (!series->hasAttribute("xi")) continue;
              int i, x_offset = 0, y_offset = 0;
              auto x_ind = static_cast<int>(element->getAttribute("x_ind"));
              auto y_ind = static_cast<int>(element->getAttribute("y_ind"));
              if (x_ind == -1 || y_ind == -1)
                {
                  series->remove();
                  continue;
                }
              double y_max = 0;
              std::shared_ptr<GRM::Context> context;
              std::shared_ptr<GRM::Render> render;
              std::shared_ptr<GRM::Element> line_elem, marker_elem;

              render = std::dynamic_pointer_cast<GRM::Render>(side_region->ownerDocument());
              if (!render)
                {
                  throw NotFoundError("Render-document not found for element\n");
                }
              context = render->getContext();
              auto plot_group = element->parentElement();
              getPlotParent(plot_group);

              auto location = static_cast<std::string>(side_region->getAttribute("location"));
              auto c_max = static_cast<double>(plot_group->getAttribute("_z_lim_max"));
              auto xmin = static_cast<double>(element->getAttribute("x_range_min"));
              auto xmax = static_cast<double>(element->getAttribute("x_range_max"));
              auto ymin = static_cast<double>(element->getAttribute("y_range_min"));
              auto ymax = static_cast<double>(element->getAttribute("y_range_max"));

              auto z = GRM::get<std::vector<double>>((*context)[static_cast<std::string>(element->getAttribute("z"))]);
              auto y = GRM::get<std::vector<double>>((*context)[static_cast<std::string>(element->getAttribute("y"))]);
              auto xi = GRM::get<std::vector<double>>((*context)[static_cast<std::string>(series->getAttribute("xi"))]);
              auto x = GRM::get<std::vector<double>>((*context)[static_cast<std::string>(element->getAttribute("x"))]);
              auto y_length = (int)y.size();
              auto x_length = (int)xi.size();

              if (plot_group->hasAttribute("x_log") && static_cast<int>(plot_group->getAttribute("x_log")))
                x_offset = (int)x.size() - x_length;
              if (plot_group->hasAttribute("y_log") && static_cast<int>(plot_group->getAttribute("y_log")))
                {
                  for (i = 0; i < y_length; i++)
                    {
                      if (grm_isnan(y[i])) y_offset += 1;
                    }
                }

              // plot step in marginal
              for (i = 0; i < ((location == "right") ? (y_length - y_offset) : x_length); i++)
                {
                  if (location == "right")
                    {
                      y[i] = std::isnan(z[(x_ind + x_offset) + (i + y_offset) * (x_length + x_offset)])
                                 ? 0
                                 : z[(x_ind + x_offset) + (i + y_offset) * (x_length + x_offset)];
                      y_max = grm_max(y_max, y[i]);
                    }
                  else
                    {
                      y[i] = std::isnan(z[(x_length + x_offset) * (y_ind + y_offset) + (i + x_offset)])
                                 ? 0
                                 : z[(x_length + x_offset) * (y_ind + y_offset) + (i + x_offset)];
                      y_max = grm_max(y_max, y[i]);
                    }
                }
              for (i = 0; i < ((location == "right") ? (y_length - y_offset) : x_length); i++)
                {
                  y[i] = y[i] / y_max * (c_max / 15);
                  xi[i] = x[i + x_offset] + ((location == "right") ? ymin : xmin);
                }

              double x_pos, y_pos;
              unsigned int len = (location == "right") ? (y_length - y_offset) : x_length;
              std::vector<double> x_step_boundaries(2 * len);
              std::vector<double> y_step_values(2 * len);

              x_step_boundaries[0] = (location == "right") ? ymin : xmin;
              for (i = 2; i < 2 * len; i += 2)
                {
                  x_step_boundaries[i - 1] = x_step_boundaries[i] =
                      x_step_boundaries[0] + (i / 2) * ((location == "right") ? (ymax - ymin) : (xmax - xmin)) / len;
                }
              x_step_boundaries[2 * len - 1] = (location == "right") ? ymax : xmax;
              y_step_values[0] = y[0];
              for (i = 2; i < 2 * len; i += 2)
                {
                  y_step_values[i - 1] = y[i / 2 - 1];
                  y_step_values[i] = y[i / 2];
                }
              y_step_values[2 * len - 1] = y[len - 1];

              auto id = static_cast<int>(global_root->getAttribute("_id"));
              global_root->setAttribute("_id", id + 1);
              auto id_str = std::to_string(id);

              // special case where it shouldn't be necessary to use the delete attribute from the children
              if (!series->hasChildNodes())
                {
                  if (location == "right")
                    {
                      line_elem =
                          global_render->createPolyline("x" + id_str, y_step_values, "y" + id_str, x_step_boundaries);
                      x_pos = (x_step_boundaries[y_ind * 2] + x_step_boundaries[y_ind * 2 + 1]) / 2;
                      y_pos = y[y_ind];
                      marker_elem = global_render->createPolymarker(y_pos, x_pos);
                    }
                  else
                    {
                      line_elem =
                          global_render->createPolyline("x" + id_str, x_step_boundaries, "y" + id_str, y_step_values);
                      x_pos = (x_step_boundaries[x_ind * 2] + x_step_boundaries[x_ind * 2 + 1]) / 2;
                      y_pos = y[x_ind];
                      marker_elem = global_render->createPolymarker(x_pos, y_pos);
                    }

                  global_render->setLineColorInd(line_elem, 989);
                  global_render->setMarkerColorInd(marker_elem, 2);
                  global_render->setMarkerType(marker_elem, -1);
                  global_render->setMarkerSize(marker_elem,
                                               1.5 * (len / ((location == "right") ? (ymax - ymin) : (xmax - xmin))));

                  marker_elem->setAttribute("name", "line");
                  line_elem->setAttribute("name", "line");
                  series->append(marker_elem);
                  series->append(line_elem);
                  marker_elem->setAttribute("z_index", 2);
                }
              else
                {
                  for (const auto &child : series->children())
                    {
                      if (child->localName() == "polyline")
                        {
                          std::string x_key = "x" + id_str;
                          std::string y_key = "y" + id_str;
                          if (location == "right")
                            {
                              (*context)[x_key] = y_step_values;
                              (*context)[y_key] = x_step_boundaries;
                            }
                          else
                            {
                              (*context)[y_key] = y_step_values;
                              (*context)[x_key] = x_step_boundaries;
                            }
                          child->setAttribute("x", x_key);
                          child->setAttribute("y", y_key);
                        }
                      else if (child->localName() == "polymarker")
                        {
                          if (location == "right")
                            {
                              x_pos = (x_step_boundaries[y_ind * 2] + x_step_boundaries[y_ind * 2 + 1]) / 2;
                              y_pos = y[y_ind];
                              child->setAttribute("x", y_pos);
                              child->setAttribute("y", x_pos);
                            }
                          else
                            {
                              x_pos = (x_step_boundaries[x_ind * 2] + x_step_boundaries[x_ind * 2 + 1]) / 2;
                              y_pos = y[x_ind];
                              child->setAttribute("x", x_pos);
                              child->setAttribute("y", y_pos);
                            }
                          global_render->setMarkerSize(
                              child, 1.5 * (len / ((location == "right") ? (ymax - ymin) : (xmax - xmin))));
                        }
                    }
                }
            }
        }
      else if (mkind == "all")
        {
          for (const auto &series : side_region->querySelectors("side_plot_region")->children())
            {
              int cnt = 0;
              auto x_ind = static_cast<int>(element->getAttribute("x_ind"));
              auto y_ind = static_cast<int>(element->getAttribute("y_ind"));

              if (x_ind == -1 && y_ind == -1)
                {
                  for (const auto &bar : series->children())
                    {
                      for (const auto &rect : bar->children())
                        {
                          if (rect->hasAttribute("line_color_ind")) continue;
                          rect->setAttribute("fill_color_ind", 989);
                        }
                    }
                  continue;
                }

              bool is_horizontal = static_cast<std::string>(series->getAttribute("orientation")) == "horizontal";
              std::vector<std::shared_ptr<GRM::Element>> bar_groups = series->children();

              if ((is_horizontal && x_ind == -1) || (!is_horizontal && y_ind == -1))
                {
                  continue;
                }
              if ((is_horizontal ? x_ind : y_ind) >= bar_groups.size())
                {
                  continue;
                }

              for (const auto &group : bar_groups)
                {
                  for (const auto &rect : group->children())
                    {
                      if (rect->hasAttribute("line_color_ind")) continue;
                      if (cnt == (is_horizontal ? x_ind : y_ind))
                        {
                          rect->setAttribute("fill_color_ind", 2);
                        }
                      else
                        {
                          rect->setAttribute("fill_color_ind", 989);
                        }
                      cnt += 1;
                    }
                }
            }
        }
    }
}

static void processResetRotation(const std::shared_ptr<GRM::Element> &element)
{
  if (element->hasAttribute("_space_3d_phi_org") && element->hasAttribute("_space_3d_theta_org"))
    {
      auto phi = static_cast<double>(element->getAttribute("_space_3d_phi_org"));
      auto theta = static_cast<double>(element->getAttribute("_space_3d_theta_org"));
      element->setAttribute("space_3d_phi", phi);
      element->setAttribute("space_3d_theta", theta);
    }
  element->removeAttribute("reset_rotation");
}

void GRM::Render::processLimits(const std::shared_ptr<GRM::Element> &element)
{
  /*!
   * processing function for gr_window
   *
   * \param[in] element The GRM::Element that contains the attributes
   */
  int adjust_x_lim, adjust_y_lim;
  int scale = 0;
  bool plot_reset_ranges = false;
  std::shared_ptr<GRM::Element> central_region;
  auto kind = static_cast<std::string>(element->getAttribute("kind"));
  gr_inqscale(&scale);

  // todo: log scales not for polar types?
  if (kind != "pie" && kind != "polar" && kind != "polar_histogram" && kind != "polar_heatmap" &&
      kind != "nonuniform_polar_heatmap")
    {
      scale |= static_cast<int>(element->getAttribute("x_log")) ? GR_OPTION_X_LOG : 0;
      scale |= static_cast<int>(element->getAttribute("y_log")) ? GR_OPTION_Y_LOG : 0;
      scale |= static_cast<int>(element->getAttribute("z_log")) ? GR_OPTION_Z_LOG : 0;
      scale |= static_cast<int>(element->getAttribute("x_flip")) ? GR_OPTION_FLIP_X : 0;
      scale |= static_cast<int>(element->getAttribute("y_flip")) ? GR_OPTION_FLIP_Y : 0;
      scale |= static_cast<int>(element->getAttribute("z_flip")) ? GR_OPTION_FLIP_Z : 0;
    }
  element->setAttribute("scale", scale);

  auto xmin = static_cast<double>(element->getAttribute("_x_lim_min"));
  auto xmax = static_cast<double>(element->getAttribute("_x_lim_max"));
  auto ymin = static_cast<double>(element->getAttribute("_y_lim_min"));
  auto ymax = static_cast<double>(element->getAttribute("_y_lim_max"));

  if (element->hasAttribute("reset_ranges") && static_cast<int>(element->getAttribute("reset_ranges")))
    {
      if (element->hasAttribute("_original_x_min") && element->hasAttribute("_original_x_max") &&
          element->hasAttribute("_original_y_min") && element->hasAttribute("_original_y_max") &&
          element->hasAttribute("_original_adjust_x_lim") && element->hasAttribute("_original_adjust_y_lim"))
        {
          xmin = static_cast<double>(element->getAttribute("_original_x_min"));
          xmax = static_cast<double>(element->getAttribute("_original_x_max"));
          ymin = static_cast<double>(element->getAttribute("_original_y_min"));
          ymax = static_cast<double>(element->getAttribute("_original_y_max"));
          adjust_x_lim = static_cast<int>(element->getAttribute("_original_adjust_x_lim"));
          adjust_y_lim = static_cast<int>(element->getAttribute("_original_adjust_y_lim"));
          element->setAttribute("adjust_x_lim", adjust_x_lim);
          element->setAttribute("adjust_y_lim", adjust_y_lim);
          element->removeAttribute("_original_x_lim");
          element->removeAttribute("_original_x_min");
          element->removeAttribute("_original_x_max");
          element->removeAttribute("_original_y_lim");
          element->removeAttribute("_original_y_min");
          element->removeAttribute("_original_y_max");
          element->removeAttribute("_original_adjust_x_lim");
          element->removeAttribute("_original_adjust_y_lim");
        }
      plot_reset_ranges = true;
      element->removeAttribute("reset_ranges");
    }
  // reset rotation
  for (const auto &plot_child : element->children())
    {
      if (plot_child->localName() == "marginal_heatmap_plot")
        {
          for (const auto &child : plot_child->children())
            {
              if (child->localName() == "central_region")
                {
                  central_region = child;
                  if (central_region->hasAttribute("reset_rotation")) processResetRotation(central_region);
                  break;
                }
            }
        }
      if (plot_child->localName() == "central_region")
        {
          central_region = plot_child;
          if (central_region->hasAttribute("reset_rotation")) processResetRotation(central_region);
          processWindow(central_region);
          processScale(element);
          break;
        }
    }

  if (element->hasAttribute("panzoom") && static_cast<int>(element->getAttribute("panzoom")))
    {
      gr_savestate();

      // when possible set the viewport of central_region, cause that's the one which is used to determinate the panzoom
      // before it gets set on the tree
      if (central_region->hasAttribute("viewport_x_min"))
        {
          auto viewport_x_min = static_cast<double>(central_region->getAttribute("viewport_x_min"));
          auto viewport_x_max = static_cast<double>(central_region->getAttribute("viewport_x_max"));
          auto viewport_y_min = static_cast<double>(central_region->getAttribute("viewport_y_min"));
          auto viewport_y_max = static_cast<double>(central_region->getAttribute("viewport_y_max"));
          gr_setviewport(viewport_x_min, viewport_x_max, viewport_y_min, viewport_y_max);
        }

      if (!element->hasAttribute("_original_x_lim"))
        {
          element->setAttribute("_original_x_min", xmin);
          element->setAttribute("_original_x_max", xmax);
          element->setAttribute("_original_x_lim", true);
          adjust_x_lim = static_cast<int>(element->getAttribute("adjust_x_lim"));
          element->setAttribute("_original_adjust_x_lim", adjust_x_lim);
          element->setAttribute("adjust_x_lim", 0);
        }
      if (!element->hasAttribute("_original_y_lim"))
        {
          element->setAttribute("_original_y_min", ymin);
          element->setAttribute("_original_y_max", ymax);
          element->setAttribute("_original_y_lim", true);
          adjust_y_lim = static_cast<int>(element->getAttribute("adjust_y_lim"));
          element->setAttribute("_original_adjust_y_lim", adjust_y_lim);
          element->setAttribute("adjust_y_lim", 0);
        }
      auto panzoom_element = element->getElementsByTagName("panzoom")[0];
      auto x = static_cast<double>(panzoom_element->getAttribute("x"));
      auto y = static_cast<double>(panzoom_element->getAttribute("y"));
      auto x_zoom = static_cast<double>(panzoom_element->getAttribute("x_zoom"));
      auto y_zoom = static_cast<double>(panzoom_element->getAttribute("y_zoom"));

      /* Ensure the correct window is set in GRM */
      bool window_exists =
          (central_region->hasAttribute("window_x_min") && central_region->hasAttribute("window_x_max") &&
           central_region->hasAttribute("window_y_min") && central_region->hasAttribute("window_y_max"));
      if (window_exists)
        {
          auto stored_window_xmin = static_cast<double>(central_region->getAttribute("window_x_min"));
          auto stored_window_xmax = static_cast<double>(central_region->getAttribute("window_x_max"));
          auto stored_window_ymin = static_cast<double>(central_region->getAttribute("window_y_min"));
          auto stored_window_ymax = static_cast<double>(central_region->getAttribute("window_y_max"));

          if (stored_window_xmax - stored_window_xmin > 0.0 && stored_window_ymax - stored_window_ymin > 0.0)
            gr_setwindow(stored_window_xmin, stored_window_xmax, stored_window_ymin, stored_window_ymax);
        }
      else
        {
          throw NotFoundError("Window not found\n");
        }

      gr_panzoom(x, y, x_zoom, y_zoom, &xmin, &xmax, &ymin, &ymax);

      element->removeAttribute("panzoom");
      element->removeChild(panzoom_element);
      central_region->setAttribute("_zoomed", true);

      for (const auto &child : central_region->children())
        {
          if (starts_with(child->localName(), "series_"))
            {
              resetOldBoundingBoxes(child);
            }
        }
      gr_restorestate();
    }

  element->setAttribute("_x_lim_min", xmin);
  element->setAttribute("_x_lim_max", xmax);
  element->setAttribute("_y_lim_min", ymin);
  element->setAttribute("_y_lim_max", ymax);

  if (!(scale & GR_OPTION_X_LOG))
    {
      adjust_x_lim = static_cast<int>(element->getAttribute("adjust_x_lim"));
      if (adjust_x_lim)
        {
          logger((stderr, "_x_lim before \"gr_adjustlimits\": (%lf, %lf)\n", xmin, xmax));
          gr_adjustlimits(&xmin, &xmax);
          logger((stderr, "_x_lim after \"gr_adjustlimits\": (%lf, %lf)\n", xmin, xmax));
        }
    }

  if (!(scale & GR_OPTION_Y_LOG))
    {
      adjust_y_lim = static_cast<int>(element->getAttribute("adjust_y_lim"));
      if (adjust_y_lim)
        {
          logger((stderr, "_y_lim before \"gr_adjustlimits\": (%lf, %lf)\n", ymin, ymax));
          gr_adjustlimits(&ymin, &ymax);
          logger((stderr, "_y_lim after \"gr_adjustlimits\": (%lf, %lf)\n", ymin, ymax));
        }
    }

  if (str_equals_any(kind, "wireframe", "surface", "plot3", "scatter3", "trisurface", "volume", "isosurface"))
    {
      auto zmin = static_cast<double>(element->getAttribute("_z_lim_min"));
      auto zmax = static_cast<double>(element->getAttribute("_z_lim_max"));
      if (zmax > 0)
        {
          if (!(scale & GR_OPTION_Z_LOG))
            {
              auto adjust_z_lim = static_cast<int>(element->hasAttribute("adjust_z_lim"));
              if (adjust_z_lim)
                {
                  logger((stderr, "_z_lim before \"gr_adjustlimits\": (%lf, %lf)\n", zmin, zmax));
                  gr_adjustlimits(&zmin, &zmax);
                  logger((stderr, "_z_lim after \"gr_adjustlimits\": (%lf, %lf)\n", zmin, zmax));
                }
            }
          logger((stderr, "Storing window 3d (%lf, %lf, %lf, %lf, %lf, %lf)\n", xmin, xmax, ymin, ymax, zmin, zmax));
          global_render->setWindow3d(central_region, xmin, xmax, ymin, ymax, zmin, zmax);
        }
    }
  else
    {
      logger((stderr, "Storing window (%lf, %lf, %lf, %lf)\n", xmin, xmax, ymin, ymax));
      global_render->setWindow(central_region, xmin, xmax, ymin, ymax);
    }
  if (plot_reset_ranges) central_region->setAttribute("_zoomed", true);
  processWindow(central_region);
  processScale(element);
}

static void processLineColorInd(const std::shared_ptr<GRM::Element> &element)
{
  gr_setlinecolorind(static_cast<int>(element->getAttribute("line_color_ind")));
}

static void processLineSpec(const std::shared_ptr<GRM::Element> &element)
{
  if (element->localName() != "series_line" && element->localName() != "series_stairs")
    gr_uselinespec((static_cast<std::string>(element->getAttribute("line_spec"))).data());
}

int lineTypeStringToInt(const std::string &line_type_str)
{
  return line_type_string_to_int[line_type_str];
}

std::string lineTypeIntToString(int line_type)
{
  for (auto const &map_elem : line_type_string_to_int)
    {
      if (map_elem.second == line_type)
        {
          return map_elem.first;
        }
    }
}

static void processLineType(const std::shared_ptr<GRM::Element> &element)
{
  int line_type;
  if (element->getAttribute("line_type").isInt())
    {
      line_type = static_cast<int>(element->getAttribute("line_type"));
    }
  else if (element->getAttribute("line_type").isString())
    {
      line_type = lineTypeStringToInt(static_cast<std::string>(element->getAttribute("line_type")));
    }
  gr_setlinetype(line_type);
}

static void processLineWidth(const std::shared_ptr<GRM::Element> &element)
{
  gr_setlinewidth(static_cast<double>(element->getAttribute("line_width")));
}

static void processMarkerColorInd(const std::shared_ptr<GRM::Element> &element)
{
  gr_setmarkercolorind(static_cast<int>(element->getAttribute("marker_color_ind")));
}

static void processMarkerSize(const std::shared_ptr<GRM::Element> &element)
{
  gr_setmarkersize(static_cast<double>(element->getAttribute("marker_size")));
}

int markerTypeStringToInt(const std::string &marker_type_str)
{
  return marker_type_string_to_int[marker_type_str];
}

std::string markerTypeIntToString(int marker_type)
{
  for (auto const &map_elem : marker_type_string_to_int)
    {
      if (map_elem.second == marker_type)
        {
          return map_elem.first;
        }
    }
}

static void processMarkerType(const std::shared_ptr<GRM::Element> &element)
{
  int marker_type;
  if (element->getAttribute("marker_type").isInt())
    {
      marker_type = static_cast<int>(element->getAttribute("marker_type"));
    }
  else if (element->getAttribute("marker_type").isString())
    {
      marker_type = markerTypeStringToInt(static_cast<std::string>(element->getAttribute("marker_type")));
    }
  gr_setmarkertype(marker_type);
}

int projectionTypeStringToInt(const std::string &projection_type_str)
{
  if (projection_type_str == "default")
    return 0;
  else if (projection_type_str == "orthographic")
    return 1;
  else if (projection_type_str == "perspective")
    return 2;
}

std::string projectionTypeIntToString(int projection_type)
{
  if (projection_type == 0)
    return "default";
  else if (projection_type == 1)
    return "orthographic";
  else if (projection_type == 2)
    return "perspective";
}

static void processProjectionType(const std::shared_ptr<GRM::Element> &element)
{
  int projection_type;
  if (element->getAttribute("projection_type").isInt())
    {
      projection_type = static_cast<int>(element->getAttribute("projection_type"));
    }
  else if (element->getAttribute("projection_type").isString())
    {
      projection_type = projectionTypeStringToInt(static_cast<std::string>(element->getAttribute("projection_type")));
    }
  gr_setprojectiontype(projection_type);
}

static void processRelativeCharHeight(const std::shared_ptr<GRM::Element> &element)
{
  double viewport[4], subplot_viewport[4];
  auto plot_element = getSubplotElement(element);
  double char_height, max_char_height, max_char_height_rel;
  std::shared_ptr<GRM::Element> central_region_parent, subplot_parent;
  auto kind = static_cast<std::string>(plot_element->getAttribute("kind"));
  double diag_factor;
  double metric_width, metric_height;
  bool multiple_plots = false, uniform_data = true, keep_aspect_ratio = false, only_quadratic_aspect_ratio = false;

  subplot_parent = (plot_element->parentElement()->localName() == "layout_grid_element")
                       ? subplot_parent = plot_element->parentElement()
                       : plot_element;
  central_region_parent = plot_element;
  if (kind == "marginal_heatmap") central_region_parent = plot_element->children()[0];

  if (!element->parentElement()->hasAttribute("viewport_x_min") ||
      !element->parentElement()->hasAttribute("viewport_x_max") ||
      !element->parentElement()->hasAttribute("viewport_y_min") ||
      !element->parentElement()->hasAttribute("viewport_y_max"))
    {
      throw NotFoundError("Viewport not found\n");
    }
  viewport[0] = static_cast<double>(element->parentElement()->getAttribute("viewport_x_min"));
  viewport[1] = static_cast<double>(element->parentElement()->getAttribute("viewport_x_max"));
  viewport[2] = static_cast<double>(element->parentElement()->getAttribute("viewport_y_min"));
  viewport[3] = static_cast<double>(element->parentElement()->getAttribute("viewport_y_max"));

  // is always set otherwise the method wouldn't be called
  max_char_height = static_cast<double>(element->getAttribute("max_char_height"));
  keep_aspect_ratio = static_cast<int>(plot_element->getAttribute("keep_aspect_ratio"));
  only_quadratic_aspect_ratio = static_cast<int>(plot_element->getAttribute("only_quadratic_aspect_ratio"));

  subplot_viewport[0] = static_cast<double>(subplot_parent->getAttribute("plot_x_min"));
  subplot_viewport[1] = static_cast<double>(subplot_parent->getAttribute("plot_x_max"));
  subplot_viewport[2] =
      static_cast<double>(subplot_parent->getAttribute("plot_y_min")) / (DEFAULT_ASPECT_RATIO_FOR_SCALING);
  subplot_viewport[3] =
      static_cast<double>(subplot_parent->getAttribute("plot_y_max")) / (DEFAULT_ASPECT_RATIO_FOR_SCALING);

  GRM::Render::getFigureSize(nullptr, nullptr, &metric_width, &metric_height);
  auto aspect_ratio_ws = metric_width / metric_height;

  // special case for keep_aspect_ratio with uniform data which can lead to smaller plots
  if (keep_aspect_ratio && only_quadratic_aspect_ratio)
    {
      auto render = grm_get_render();
      for (const auto &series : plot_element->querySelectors("central_region")->children())
        {
          if (!starts_with(series->localName(), "series_")) continue;
          uniform_data = isUniformData(series, render->getContext());
          if (!uniform_data) break;
        }
      if (kind == "marginal_heatmap" && uniform_data)
        uniform_data = isUniformData(plot_element->children()[0], render->getContext());
      if (uniform_data)
        {
          double border =
              0.5 * (subplot_viewport[1] - subplot_viewport[0]) * (1.0 - 1.0 / (DEFAULT_ASPECT_RATIO_FOR_SCALING));
          subplot_viewport[0] += border;
          subplot_viewport[1] -= border;
        }
    }

  if ((keep_aspect_ratio && uniform_data && only_quadratic_aspect_ratio) || !keep_aspect_ratio)
    {
      // calculate the diagonal viewport size of the default viewport with the fix aspect_ratio 4/3
      calculateCentralRegionMarginOrDiagFactor(element, &subplot_viewport[0], &subplot_viewport[1],
                                               &subplot_viewport[2], &subplot_viewport[3], true);
      diag_factor =
          std::sqrt((subplot_viewport[1] - subplot_viewport[0]) * (subplot_viewport[1] - subplot_viewport[0]) +
                    (subplot_viewport[3] - subplot_viewport[2]) * (subplot_viewport[3] - subplot_viewport[2]));
    }
  else
    {
      double default_diag_factor;
      diag_factor = std::sqrt((viewport[1] - viewport[0]) * (viewport[1] - viewport[0]) +
                              (viewport[3] - viewport[2]) * (viewport[3] - viewport[2]));
      if (element->hasAttribute("_default_diag_factor"))
        {
          default_diag_factor = static_cast<double>(element->getAttribute("_default_diag_factor"));
        }
      else
        {
          calculateCentralRegionMarginOrDiagFactor(element, &subplot_viewport[0], &subplot_viewport[1],
                                                   &subplot_viewport[2], &subplot_viewport[3], true);
          double plot_diag_factor =
              std::sqrt((subplot_viewport[1] - subplot_viewport[0]) * (subplot_viewport[1] - subplot_viewport[0]) +
                        (subplot_viewport[3] - subplot_viewport[2]) * (subplot_viewport[3] - subplot_viewport[2]));
          auto start_aspect_ratio_ws = static_cast<double>(plot_element->getAttribute("_start_aspect_ratio"));
          default_diag_factor = ((DEFAULT_ASPECT_RATIO_FOR_SCALING) *
                                 (start_aspect_ratio_ws <= 1 ? start_aspect_ratio_ws : (1.0 / start_aspect_ratio_ws))) *
                                (plot_diag_factor / diag_factor);
          element->setAttribute("_default_diag_factor", default_diag_factor);
        }
      diag_factor *= default_diag_factor;
    }
  if (!element->hasAttribute("diag_factor")) element->setAttribute("diag_factor", diag_factor);

  if ((keep_aspect_ratio && uniform_data && only_quadratic_aspect_ratio) || !keep_aspect_ratio)
    {
      if (!element->hasAttribute("_max_char_height_set_by_user")) max_char_height *= DEFAULT_ASPECT_RATIO_FOR_SCALING;
      if (aspect_ratio_ws <= 1)
        {
          max_char_height_rel = max_char_height * aspect_ratio_ws;
        }
      else
        {
          max_char_height_rel = max_char_height / aspect_ratio_ws;
        }
    }
  else
    {
      max_char_height_rel = max_char_height;
    }

  gr_setcharheight(max_char_height_rel * diag_factor);
}

static void processResampleMethod(const std::shared_ptr<GRM::Element> &element)
{
  unsigned int resample_method_flag;
  if (!element->getAttribute("resample_method").isInt())
    {
      auto resample_method_str = static_cast<std::string>(element->getAttribute("resample_method"));

      if (resample_method_str == "nearest")
        {
          resample_method_flag = GKS_K_RESAMPLE_NEAREST;
        }
      else if (resample_method_str == "linear")
        {
          resample_method_flag = GKS_K_RESAMPLE_LINEAR;
        }
      else if (resample_method_str == "lanczos")
        {
          resample_method_flag = GKS_K_RESAMPLE_LANCZOS;
        }
      else
        {
          resample_method_flag = GKS_K_RESAMPLE_DEFAULT;
        }
    }
  else
    {
      resample_method_flag = static_cast<int>(element->getAttribute("resample_method"));
    }
  gr_setresamplemethod(resample_method_flag);
}

void GRM::Render::processScale(const std::shared_ptr<GRM::Element> &element)
{
  gr_setscale(static_cast<int>(element->getAttribute("scale")));
}

static void processSelectSpecificXform(const std::shared_ptr<GRM::Element> &element)
{
  gr_selntran(static_cast<int>(element->getAttribute("select_specific_xform")));
}

static void processSpace(const std::shared_ptr<GRM::Element> &element)
{
  auto zmin = static_cast<double>(element->getAttribute("space_z_min"));
  auto zmax = static_cast<double>(element->getAttribute("space_z_max"));
  auto rotation = static_cast<int>(element->getAttribute("space_rotation"));
  auto tilt = static_cast<int>(element->getAttribute("space_tilt"));

  gr_setspace(zmin, zmax, rotation, tilt);
}

static void processSpace3d(const std::shared_ptr<GRM::Element> &element)
{
  double phi = PLOT_DEFAULT_ROTATION, theta = PLOT_DEFAULT_TILT, fov, camera_distance;

  if (element->hasAttribute("space_3d_phi"))
    {
      phi = static_cast<double>(element->getAttribute("space_3d_phi"));
    }
  else
    {
      element->setAttribute("space_3d_phi", phi);
    }
  if (element->hasAttribute("space_3d_theta"))
    {
      theta = static_cast<double>(element->getAttribute("space_3d_theta"));
    }
  else
    {
      element->setAttribute("space_3d_theta", theta);
    }
  fov = static_cast<double>(element->getAttribute("space_3d_fov"));
  camera_distance = static_cast<double>(element->getAttribute("space_3d_camera_distance"));

  gr_setspace3d(-phi, theta, fov, camera_distance);
}

int locationStringToInt(const std::string &location_str)
{
  return location_string_to_int[location_str];
}

std::string locationIntToString(int location)
{
  for (auto const &map_elem : location_string_to_int)
    {
      if (map_elem.second == location)
        {
          return map_elem.first;
        }
    }
}

int modelStringToInt(const std::string &model_str)
{
  return model_string_to_int[model_str];
}

std::string modelIntToString(int model)
{
  for (auto const &map_elem : model_string_to_int)
    {
      if (map_elem.second == model)
        {
          return map_elem.first;
        }
    }
}

int textAlignHorizontalStringToInt(const std::string &text_align_horizontal_str)
{
  return text_align_horizontal_string_to_int[text_align_horizontal_str];
}

std::string textAlignHorizontalIntToString(int text_align_horizontal)
{
  for (auto const &map_elem : text_align_horizontal_string_to_int)
    {
      if (map_elem.second == text_align_horizontal)
        {
          return map_elem.first;
        }
    }
}

int textAlignVerticalStringToInt(const std::string &text_align_vertical_str)
{
  return text_align_vertical_string_to_int[text_align_vertical_str];
}

std::string textAlignVerticalIntToString(int text_align_vertical)
{
  for (auto const &map_elem : text_align_vertical_string_to_int)
    {
      if (map_elem.second == text_align_vertical)
        {
          return map_elem.first;
        }
    }
}

static void processTextAlign(const std::shared_ptr<GRM::Element> &element)
{
  int text_align_vertical, text_align_horizontal;
  if (element->getAttribute("text_align_vertical").isInt())
    {
      text_align_vertical = static_cast<int>(element->getAttribute("text_align_vertical"));
    }
  else if (element->getAttribute("text_align_vertical").isString())
    {
      text_align_vertical =
          textAlignVerticalStringToInt(static_cast<std::string>(element->getAttribute("text_align_vertical")));
    }
  if (element->getAttribute("text_align_horizontal").isInt())
    {
      text_align_horizontal = static_cast<int>(element->getAttribute("text_align_horizontal"));
    }
  else if (element->getAttribute("text_align_horizontal").isString())
    {
      text_align_horizontal =
          textAlignHorizontalStringToInt(static_cast<std::string>(element->getAttribute("text_align_horizontal")));
    }
  gr_settextalign(text_align_horizontal, text_align_vertical);
}

static void processTextColorForBackground(const std::shared_ptr<GRM::Element> &element)
/*  The set_text_color_for_background function used in plot.cxx now as an attribute function
    It is now possible to inquire colors during runtime -> No colors are given as parameters
    The new color is set on `element`
    There are no params apart from element
    \param[in] element The GRM::Element the color should be set in. Also contains other attributes which may function as
 parameters

    Attributes as Parameters (with prefix "stcfb-"):
    plot: for which plot it is used: right now only pie plot
 */
{
  int color_ind, inq_color;
  unsigned char color_rgb[4];
  std::string plot = "pie";
  std::shared_ptr<GRM::Element> plot_parent = element;
  getPlotParent(plot_parent);

  if (!static_cast<int>(element->getAttribute("set_text_color_for_background"))) return;
  if (element->hasAttribute("stcfb_plot"))
    {
      plot = static_cast<std::string>(element->getAttribute("stcfb_plot"));
    }

  if (plot == "pie" && static_cast<std::string>(plot_parent->getAttribute("kind")) == "pie")
    {
      double r, g, b;
      double color_lightness;
      std::shared_ptr<GRM::Render> render;

      render = std::dynamic_pointer_cast<GRM::Render>(element->ownerDocument());
      if (!render)
        {
          throw NotFoundError("Render-document not found for element\n");
        }
      if (element->hasAttribute("color_ind"))
        {
          color_ind = static_cast<int>(element->getAttribute("color_ind"));
        }
      else
        {
          gr_inqfillcolorind(&color_ind);
        }
      gr_inqcolor(color_ind, (int *)color_rgb);

      r = color_rgb[0] / 255.0;
      g = color_rgb[1] / 255.0;
      b = color_rgb[2] / 255.0;

      color_lightness = getLightnessFromRGB(r, g, b);
      if (color_lightness < 0.4)
        {
          gr_settextcolorind(0);
          element->setAttribute("text_color_ind", 0);
        }
      else
        {
          gr_settextcolorind(1);
          element->setAttribute("text_color_ind", 1);
        }
    }
}

static void processTextColorInd(const std::shared_ptr<GRM::Element> &element)
{

  gr_settextcolorind(static_cast<int>(element->getAttribute("text_color_ind")));
}

int textEncodingStringToInt(const std::string &text_encoding_str)
{
  if (text_encoding_str == "latin1")
    return 300;
  else if (text_encoding_str == "utf8")
    return 301;
}

std::string textEncodingIntToString(int text_encoding)
{
  if (text_encoding == 300)
    return "latin1";
  else if (text_encoding == 301)
    return "utf8";
}

static void processTextEncoding(const std::shared_ptr<GRM::Element> &element)
{
  int text_encoding;
  if (element->getAttribute("text_encoding").isInt())
    {
      text_encoding = static_cast<int>(element->getAttribute("text_encoding"));
    }
  else if (element->getAttribute("text_encoding").isString())
    {
      text_encoding = textEncodingStringToInt(static_cast<std::string>(element->getAttribute("text_encoding")));
    }
  gr_settextencoding(text_encoding);
}

int tickOrientationStringToInt(const std::string &tick_orientation_str)
{
  if (tick_orientation_str == "up")
    return 1;
  else if (tick_orientation_str == "down")
    return -1;
}

std::string tickOrientationIntToString(int tick_orientation)
{
  if (tick_orientation > 0)
    return "up";
  else if (tick_orientation < 0)
    return "down";
}

static void processTransparency(const std::shared_ptr<GRM::Element> &element)
{
  gr_settransparency(static_cast<double>(element->getAttribute("transparency")));
}

static void axisArgumentsConvertedIntoTickGroups(tick_t *ticks, tick_label_t *tick_labels,
                                                 const std::shared_ptr<GRM::Element> &axis, del_values del)
{
  int child_id = 1, label_ind = 0;
  std::shared_ptr<GRM::Element> tick_group;
  auto num_ticks = static_cast<int>(axis->getAttribute("num_ticks"));
  auto num_labels = static_cast<int>(axis->getAttribute("num_tick_labels"));
  auto axis_type = static_cast<std::string>(axis->getAttribute("axis_type"));

  if (static_cast<int>(axis->getAttribute("mirrored_axis"))) child_id += 1;
  for (int i = 0; i < num_ticks; i++)
    {
      std::string label = "";
      double width = 0.0;
      if (label_ind < num_labels && tick_labels[label_ind].tick == ticks[i].value)
        {
          if (tick_labels[label_ind].label) label = tick_labels[label_ind].label;
          if (tick_labels[label_ind].width) width = tick_labels[label_ind].width;
          label_ind += 1;
        }

      if (del != del_values::update_without_default && del != del_values::update_with_default)
        {
          tick_group = global_render->createTickGroup(ticks[i].is_major, label, ticks[i].value, width);
          tick_group->setAttribute("_child_id", child_id++);
          axis->append(tick_group);
        }
      else
        {
          tick_group = axis->querySelectors("tick_group[_child_id=" + std::to_string(child_id++) + "]");
          if (tick_group != nullptr)
            tick_group = global_render->createTickGroup(ticks[i].is_major, label, ticks[i].value, width, tick_group);
        }
    }
}

static void processAxis(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for axes
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double x_tick, x_org;
  double y_tick, y_org;
  int x_major, y_major, major_count = 1;
  int tick_orientation = 1, child_id = 0;
  double tick_size = NAN, tick = NAN;
  double min_val = NAN, max_val = NAN;
  double org = NAN, pos = NAN;
  double line_x_min = 0.0, line_x_max = 0.0, line_y_min = 0.0, line_y_max = 0.0;
  std::string x_org_pos = PLOT_DEFAULT_ORG_POS, y_org_pos = PLOT_DEFAULT_ORG_POS;
  std::shared_ptr<GRM::Element> plot_parent = element, line, axis_elem = element, central_region;
  double window[4], old_window[4] = {NAN, NAN, NAN, NAN};
  bool mirrored_axis = MIRRORED_AXIS_DEFAULT, x_flip = false, y_flip = false;
  std::string kind, axis_type;
  del_values del = del_values::update_without_default;
  int scientific_format = SCIENTIFIC_FORMAT_OPTION, scale = 0;

  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);
  getAxesInformation(element, x_org_pos, y_org_pos, x_org, y_org, x_major, y_major, x_tick, y_tick);
  getPlotParent(plot_parent);

  window[0] = static_cast<double>(element->parentElement()->parentElement()->getAttribute("window_x_min"));
  window[1] = static_cast<double>(element->parentElement()->parentElement()->getAttribute("window_x_max"));
  window[2] = static_cast<double>(element->parentElement()->parentElement()->getAttribute("window_y_min"));
  window[3] = static_cast<double>(element->parentElement()->parentElement()->getAttribute("window_y_max"));
  if (element->hasAttribute("_window_old_x_min"))
    old_window[0] = static_cast<double>(element->getAttribute("_window_old_x_min"));
  if (element->hasAttribute("_window_old_x_max"))
    old_window[1] = static_cast<double>(element->getAttribute("_window_old_x_max"));
  if (element->hasAttribute("_window_old_y_min"))
    old_window[2] = static_cast<double>(element->getAttribute("_window_old_y_min"));
  if (element->hasAttribute("_window_old_y_max"))
    old_window[3] = static_cast<double>(element->getAttribute("_window_old_y_max"));
  element->setAttribute("_window_old_x_min", window[0]);
  element->setAttribute("_window_old_x_max", window[1]);
  element->setAttribute("_window_old_y_min", window[2]);
  element->setAttribute("_window_old_y_max", window[3]);

  kind = static_cast<std::string>(plot_parent->getAttribute("kind"));
  if (element->hasAttribute("mirrored_axis")) mirrored_axis = static_cast<int>(element->getAttribute("mirrored_axis"));
  axis_type = static_cast<std::string>(element->getAttribute("axis_type"));
  if (element->hasAttribute("scientific_format"))
    scientific_format = static_cast<int>(element->getAttribute("scientific_format"));
  if (plot_parent->hasAttribute("font")) processFont(plot_parent);
  if (plot_parent->hasAttribute("x_flip")) x_flip = static_cast<int>(plot_parent->getAttribute("x_flip"));
  if (plot_parent->hasAttribute("y_flip")) y_flip = static_cast<int>(plot_parent->getAttribute("y_flip"));

  if (element->hasAttribute("scale"))
    {
      global_render->processScale(element);
      scale = static_cast<int>(element->getAttribute("scale"));
    }

  if (axis_type == "x")
    {
      min_val = window[0];
      max_val = window[1];
      tick = x_tick;
      pos = window[2];
      major_count = x_major;
    }
  else if (axis_type == "y")
    {
      min_val = window[2];
      max_val = window[3];
      tick = y_tick;
      pos = window[0];
      major_count = y_major;
    }
  getTickSize(element, tick_size); // GRM calculated tick_size

  if (element->parentElement()->localName() == "colorbar" ||
      (axis_type == "x" && ((std::isnan(old_window[0]) || old_window[0] == window[0]) &&
                            (std::isnan(old_window[1]) || old_window[1] == window[1]))) ||
      (axis_type == "y" && ((std::isnan(old_window[2]) || old_window[2] == window[2]) &&
                            (std::isnan(old_window[3]) || old_window[3] == window[3]))))
    {
      if (element->hasAttribute("min_value")) min_val = static_cast<double>(element->getAttribute("min_value"));
      if (element->hasAttribute("max_value")) max_val = static_cast<double>(element->getAttribute("max_value"));
      if (element->hasAttribute("org")) org = static_cast<double>(element->getAttribute("org"));
      if (element->hasAttribute("pos")) pos = static_cast<double>(element->getAttribute("pos"));
      if (element->hasAttribute("tick")) tick = static_cast<double>(element->getAttribute("tick"));
      if (element->hasAttribute("major_count")) major_count = static_cast<int>(element->getAttribute("major_count"));
      if (element->hasAttribute("tick_orientation"))
        tick_orientation = static_cast<int>(element->getAttribute("tick_orientation"));
    }

  // special cases for x- and y-flip
  if ((scale & GR_OPTION_FLIP_X || x_flip) && axis_type == "y") pos = window[1];
  if ((scale & GR_OPTION_FLIP_Y || y_flip) && axis_type == "x") pos = window[3];

  // axis
  if (element->parentElement()->localName() != "colorbar")
    {
      central_region = element->parentElement()->parentElement();
      // ticks need to flipped cause a heatmap or shade series is part of the central_region
      for (const auto &series : central_region->children())
        {
          if (starts_with(series->localName(), "series_") &&
              str_equals_any(series->localName(), "series_contourf", "series_heatmap", "series_shade"))
            {
              tick_orientation = -1;
              break;
            }
        }
    }
  else
    {
      if ((scale & GR_OPTION_FLIP_X || x_flip) && axis_type == "y") pos = window[0];
      processFlip(element->parentElement());
    }
  tick_size *= tick_orientation;
  axis_t axis = {min_val, max_val, tick, org, pos, major_count, 0, nullptr, tick_size, 0, nullptr, NAN, 1};
  if (axis_type == "x")
    gr_axis('X', &axis);
  else if (axis_type == "y")
    gr_axis('Y', &axis);
  tick_orientation = axis.tick_size < 0 ? -1 : 1;

  axis_elem = global_render->createAxis(axis.min, axis.max, axis.tick, axis.org, axis.position, axis.major_count,
                                        axis.num_ticks, axis.num_tick_labels, abs(tick_size), tick_orientation,
                                        axis.label_position, axis_elem);
  if (del != del_values::update_without_default)
    {
      if (!axis_elem->hasAttribute("draw_grid")) axis_elem->setAttribute("draw_grid", true);
      if (kind == "barplot")
        {
          bool only_barplot = true;
          std::string orientation = PLOT_DEFAULT_ORIENTATION;
          auto barplot = plot_parent->querySelectors("series_barplot");
          for (const auto &series : central_region->children())
            {
              if (starts_with(series->localName(), "series_") && series->localName() != "series_barplot")
                {
                  only_barplot = false;
                  break;
                }
            }

          if (only_barplot)
            {
              if (barplot != nullptr) orientation = static_cast<std::string>(barplot->getAttribute("orientation"));
              if (axis_type == "x" && orientation == "horizontal") axis_elem->setAttribute("draw_grid", false);
              if (axis_type == "y" && orientation == "vertical") axis_elem->setAttribute("draw_grid", false);
            }
        }
      if (kind == "shade") axis_elem->setAttribute("draw_grid", false);
      axis_elem->setAttribute("mirrored_axis", mirrored_axis);
    }
  // create tick_group elements
  if (!element->querySelectors("tick_group") ||
      (axis_type == "x" && ((!std::isnan(old_window[0]) && old_window[0] != window[0]) ||
                            (!std::isnan(old_window[1]) && old_window[1] != window[1]))) ||
      (axis_type == "y" && ((!std::isnan(old_window[2]) && old_window[2] != window[2]) ||
                            (!std::isnan(old_window[3]) && old_window[3] != window[3]))))
    {
      for (const auto &child : element->children())
        {
          if (child->localName() != "polyline" && child->hasAttribute("_child_id")) child->remove();
        }
      element->setAttribute("scientific_format", scientific_format);
      axisArgumentsConvertedIntoTickGroups(axis.ticks, axis.tick_labels, axis_elem, del_values::recreate_own_children);
    }
  // polyline for axis-line
  if (axis_type == "x")
    {
      line_x_min = axis.min;
      line_x_max = axis.max;
      line_y_min = line_y_max = axis.position;
    }
  else if (axis_type == "y")
    {
      line_x_min = line_x_max = axis.position;
      line_y_min = axis.min;
      line_y_max = axis.max;
    }
  if (del != del_values::update_without_default && del != del_values::update_with_default)
    {
      line = global_render->createPolyline(line_x_min, line_x_max, line_y_min, line_y_max);
      line->setAttribute("_child_id", 0);
      axis_elem->append(line);
    }
  else
    {
      line = element->querySelectors("polyline[_child_id=0]");
      if (line != nullptr)
        global_render->createPolyline(line_x_min, line_x_max, line_y_min, line_y_max, 0, 0.0, 0, line);
    }
  if (line != nullptr && del != del_values::update_without_default)
    {
      int z_index = axis_type == "x" ? 0 : -2;
      line->setAttribute("name", axis_type + "-axis-line");
      line->setAttribute("z_index", element->parentElement()->localName() == "colorbar" ? 2 : z_index);
    }
  if (axis_type == "x")
    {
      line_y_min = line_y_max = window[3];
      if (scale & GR_OPTION_FLIP_Y || y_flip) line_y_min = line_y_max = window[2];
    }
  else if (axis_type == "y")
    {
      line_x_min = line_x_max = window[1];
      if (scale & GR_OPTION_FLIP_X || x_flip) line_x_min = line_x_max = window[0];
    }
  if (mirrored_axis)
    {
      if (del != del_values::update_without_default && del != del_values::update_with_default)
        {
          line = global_render->createPolyline(line_x_min, line_x_max, line_y_min, line_y_max);
          line->setAttribute("_child_id", 1);
          axis_elem->append(line);
        }
      else
        {
          line = element->querySelectors("polyline[_child_id=1]");
          if (line != nullptr)
            global_render->createPolyline(line_x_min, line_x_max, line_y_min, line_y_max, 0, 0.0, 0, line);
        }
      if (line != nullptr && del != del_values::update_without_default)
        {
          line->setAttribute("name", axis_type + "-axis-line mirrored");
          line->setAttribute("z_index", axis_type == "x" ? -1 : -3);
        }
    }

  applyMoveTransformation(element);
  gr_freeaxis(&axis);
}

void GRM::Render::processWindow(const std::shared_ptr<GRM::Element> &element)
{
  auto xmin = static_cast<double>(element->getAttribute("window_x_min"));
  auto xmax = static_cast<double>(element->getAttribute("window_x_max"));
  auto ymin = static_cast<double>(element->getAttribute("window_y_min"));
  auto ymax = static_cast<double>(element->getAttribute("window_y_max"));
  if (element->localName() == "central_region")
    {
      std::shared_ptr<GRM::Element> plot_element = element;
      getPlotParent(plot_element);
      auto kind = static_cast<std::string>(plot_element->getAttribute("kind"));

      if (kind != "pie")
        {
          if (xmax - xmin > 0.0 && ymax - ymin > 0.0) gr_setwindow(xmin, xmax, ymin, ymax);
        }
      if (str_equals_any(kind, "wireframe", "surface", "plot3", "scatter3", "trisurface", "volume", "isosurface"))
        {
          auto zmin = static_cast<double>(element->getAttribute("window_z_min"));
          auto zmax = static_cast<double>(element->getAttribute("window_z_max"));

          gr_setwindow3d(xmin, xmax, ymin, ymax, zmin, zmax);
        }
      if (element->hasAttribute("_zoomed") && static_cast<int>(element->getAttribute("_zoomed")))
        {
          for (const auto axis : element->querySelectorsAll("axis"))
            {
              clearAxisAttributes(axis);
              processAxis(axis, global_render->context);
            }
          element->setAttribute("_zoomed", false);
        }
    }
  else
    {
      if (xmax - xmin > 0.0 && ymax - ymin > 0.0) gr_setwindow(xmin, xmax, ymin, ymax);
    }
}

static void processWSViewport(const std::shared_ptr<GRM::Element> &element)
{
  /*!
   * processing function for gr_wsviewport
   *
   * \param[in] element The GRM::Element that contains the attributes
   */
  double ws_viewport[4];
  ws_viewport[0] = static_cast<double>(element->getAttribute("ws_viewport_x_min"));
  ws_viewport[1] = static_cast<double>(element->getAttribute("ws_viewport_x_max"));
  ws_viewport[2] = static_cast<double>(element->getAttribute("ws_viewport_y_min"));
  ws_viewport[3] = static_cast<double>(element->getAttribute("ws_viewport_y_max"));

  gr_setwsviewport(ws_viewport[0], ws_viewport[1], ws_viewport[2], ws_viewport[3]);
}

static void processWSWindow(const std::shared_ptr<GRM::Element> &element)
{
  double ws_window[4];
  ws_window[0] = static_cast<double>(element->getAttribute("ws_window_x_min"));
  ws_window[1] = static_cast<double>(element->getAttribute("ws_window_x_max"));
  ws_window[2] = static_cast<double>(element->getAttribute("ws_window_y_min"));
  ws_window[3] = static_cast<double>(element->getAttribute("ws_window_y_max"));

  gr_setwswindow(ws_window[0], ws_window[1], ws_window[2], ws_window[3]);
}

void GRM::Render::processViewport(const std::shared_ptr<GRM::Element> &element)
{
  /*!
   * processing function for viewport
   *
   * \param[in] element The GRM::Element that contains the attributes
   */
  double viewport[4];

  viewport[0] = static_cast<double>(element->getAttribute("viewport_x_min"));
  viewport[1] = static_cast<double>(element->getAttribute("viewport_x_max"));
  viewport[2] = static_cast<double>(element->getAttribute("viewport_y_min"));
  viewport[3] = static_cast<double>(element->getAttribute("viewport_y_max"));

  // TODO: Change this workaround when all elements with viewports really have a valid viewport
  if (viewport[1] - viewport[0] > 0.0 && viewport[3] - viewport[2] > 0.0)
    {
      gr_setviewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    }
}

void GRM::Render::calculateCharHeight(const std::shared_ptr<GRM::Element> &element)
{
  /*!
   * processing function for gr_viewport
   *
   * \param[in] element The GRM::Element that contains the attributes
   */
  double viewport[4];
  double subplot_viewport[4]; // the subplot vp is the figure vp unless there are more plots inside a figure
  double char_height, diag_factor;
  std::string kind;
  double metric_width, metric_height;
  bool keep_aspect_ratio = false, uniform_data = true, only_quadratic_aspect_ratio = false;
  std::shared_ptr<GRM::Element> plot_parent = element, subplot_parent;
  getPlotParent(plot_parent);

  subplot_parent = (plot_parent->parentElement()->localName() == "layout_grid_element")
                       ? subplot_parent = plot_parent->parentElement()
                       : plot_parent;

  viewport[0] = static_cast<double>(element->getAttribute("viewport_x_min"));
  viewport[1] = static_cast<double>(element->getAttribute("viewport_x_max"));
  viewport[2] = static_cast<double>(element->getAttribute("viewport_y_min"));
  viewport[3] = static_cast<double>(element->getAttribute("viewport_y_max"));
  subplot_viewport[0] = static_cast<double>(subplot_parent->getAttribute("plot_x_min"));
  subplot_viewport[1] = static_cast<double>(subplot_parent->getAttribute("plot_x_max"));
  subplot_viewport[2] =
      static_cast<double>(subplot_parent->getAttribute("plot_y_min")) / (DEFAULT_ASPECT_RATIO_FOR_SCALING);
  subplot_viewport[3] =
      static_cast<double>(subplot_parent->getAttribute("plot_y_max")) / (DEFAULT_ASPECT_RATIO_FOR_SCALING);
  kind = static_cast<std::string>(plot_parent->getAttribute("kind"));
  keep_aspect_ratio = static_cast<int>(plot_parent->getAttribute("keep_aspect_ratio"));
  only_quadratic_aspect_ratio = static_cast<int>(plot_parent->getAttribute("only_quadratic_aspect_ratio"));

  GRM::Render::getFigureSize(nullptr, nullptr, &metric_width, &metric_height);
  auto aspect_ratio_ws = metric_width / metric_height;

  // special case for keep_aspect_ratio with uniform data which can lead to smaller plots
  if (keep_aspect_ratio && only_quadratic_aspect_ratio)
    {
      auto render = grm_get_render();
      for (const auto &series : plot_parent->querySelectors("central_region")->children())
        {
          if (!starts_with(series->localName(), "series_")) continue;
          uniform_data = isUniformData(series, render->getContext());
          if (!uniform_data) break;
        }
      if (kind == "marginal_heatmap" && uniform_data)
        uniform_data = isUniformData(plot_parent->children()[0], render->getContext());
      if (uniform_data)
        {
          double border =
              0.5 * (subplot_viewport[1] - subplot_viewport[0]) * (1.0 - 1.0 / (DEFAULT_ASPECT_RATIO_FOR_SCALING));
          subplot_viewport[0] += border;
          subplot_viewport[1] -= border;
        }
    }

  if ((keep_aspect_ratio && uniform_data && only_quadratic_aspect_ratio) || !keep_aspect_ratio)
    {
      // calculate the diagonal viewport size of the default viewport with the fix aspect_ratio 4/3
      calculateCentralRegionMarginOrDiagFactor(element, &subplot_viewport[0], &subplot_viewport[1],
                                               &subplot_viewport[2], &subplot_viewport[3], true);
      diag_factor =
          std::sqrt((subplot_viewport[1] - subplot_viewport[0]) * (subplot_viewport[1] - subplot_viewport[0]) +
                    (subplot_viewport[3] - subplot_viewport[2]) * (subplot_viewport[3] - subplot_viewport[2]));
    }
  else
    {
      diag_factor = std::sqrt((viewport[1] - viewport[0]) * (viewport[1] - viewport[0]) +
                              (viewport[3] - viewport[2]) * (viewport[3] - viewport[2]));
      if (!element->hasAttribute("_default_diag_factor"))
        {
          calculateCentralRegionMarginOrDiagFactor(element, &subplot_viewport[0], &subplot_viewport[1],
                                                   &subplot_viewport[2], &subplot_viewport[3], true);
          double plot_diag_factor =
              std::sqrt((subplot_viewport[1] - subplot_viewport[0]) * (subplot_viewport[1] - subplot_viewport[0]) +
                        (subplot_viewport[3] - subplot_viewport[2]) * (subplot_viewport[3] - subplot_viewport[2]));
          auto start_aspect_ratio_ws = static_cast<double>(plot_parent->getAttribute("_start_aspect_ratio"));
          double default_diag_factor =
              ((DEFAULT_ASPECT_RATIO_FOR_SCALING) *
               (start_aspect_ratio_ws <= 1 ? start_aspect_ratio_ws : (1.0 / start_aspect_ratio_ws))) *
              (plot_diag_factor / diag_factor);
          element->setAttribute("_default_diag_factor", default_diag_factor);
        }
      diag_factor *= static_cast<double>(element->getAttribute("_default_diag_factor"));
    }

  if (!element->hasAttribute("diag_factor")) element->setAttribute("diag_factor", diag_factor);

  if (str_equals_any(kind, "wireframe", "surface", "plot3", "scatter3", "trisurface", "volume"))
    {
      char_height = PLOT_3D_CHAR_HEIGHT;
    }
  else if (str_equals_any(kind, "polar", "polar_histogram", "polar_heatmap", "nonuniformpolar_heatmap"))
    {
      char_height = PLOT_POLAR_CHAR_HEIGHT;
    }
  else
    {
      char_height = PLOT_2D_CHAR_HEIGHT;
    }
  char_height *= diag_factor;

  if ((keep_aspect_ratio && uniform_data && only_quadratic_aspect_ratio) || !keep_aspect_ratio)
    {
      if (aspect_ratio_ws > 1)
        {
          char_height /= aspect_ratio_ws;
        }
      else
        {
          char_height *= aspect_ratio_ws;
        }
      if (!element->hasAttribute("_char_height_set_by_user")) char_height *= DEFAULT_ASPECT_RATIO_FOR_SCALING;
    }

  plot_parent->setAttribute("char_height", char_height);
  processCharHeight(plot_parent);
}

static void processZIndex(const std::shared_ptr<GRM::Element> &element)
{
  if (!z_queue_is_being_rendered)
    {
      auto z_index = static_cast<int>(element->getAttribute("z_index"));
      z_index_manager.setZIndex(z_index);
    }
}

static void processBackgroundColor(const std::shared_ptr<GRM::Element> &element)
{
  if (element->hasAttribute("background_color"))
    {
      double vp[4];
      double metric_width, metric_height;
      double aspect_ratio_ws;
      std::shared_ptr<GRM::Element> plot_elem = element;
      getPlotParent(plot_elem);

      vp[0] = static_cast<double>(plot_elem->getAttribute("plot_x_min"));
      vp[1] = static_cast<double>(plot_elem->getAttribute("plot_x_max"));
      vp[2] = static_cast<double>(plot_elem->getAttribute("plot_y_min"));
      vp[3] = static_cast<double>(plot_elem->getAttribute("plot_y_max"));

      GRM::Render::getFigureSize(nullptr, nullptr, &metric_width, &metric_height);
      aspect_ratio_ws = metric_width / metric_height;

      auto background_color_index = static_cast<int>(element->getAttribute("background_color"));
      gr_savestate();
      gr_selntran(0);
      gr_setfillintstyle(GKS_K_INTSTYLE_SOLID);
      gr_setfillcolorind(background_color_index);
      if (aspect_ratio_ws > 1)
        {
          if (redraw_ws) gr_fillrect(vp[0], vp[1], vp[2] / aspect_ratio_ws, vp[3] / aspect_ratio_ws);
        }
      else
        {
          if (redraw_ws) gr_fillrect(vp[0] * aspect_ratio_ws, vp[1] * aspect_ratio_ws, vp[2], vp[3]);
        }
      gr_selntran(1);
      gr_restorestate();
    }
}

void GRM::Render::processAttributes(const std::shared_ptr<GRM::Element> &element)
{
  /*!
   * processing function for all kinds of attributes
   *
   * \param[in] element The GRM::Element containing attributes
   */

  // Map used for processing all kinds of attributes
  static std::map<std::string, std::function<void(const std::shared_ptr<GRM::Element> &)>> attrStringToFunc{
      {std::string("alpha"), processAlpha},
      {std::string("background_color"), processBackgroundColor},
      {std::string("border_color_ind"), processBorderColorInd},
      {std::string("marginal_heatmap_side_plot"), processMarginalHeatmapSidePlot},
      {std::string("char_expan"), processCharExpan},
      {std::string("char_space"), processCharSpace},
      {std::string("char_up_x"), processCharUp}, // the x element can be used cause both must be set
      {std::string("clip_transformation"), processClipTransformation},
      {std::string("colormap"), processColormap},
      {std::string("fill_color_ind"), processFillColorInd},
      {std::string("fill_int_style"), processFillIntStyle},
      {std::string("fill_style"), processFillStyle},
      {std::string("font"), processFont},
      {std::string("line_color_ind"), processLineColorInd},
      {std::string("line_spec"), processLineSpec},
      {std::string("line_type"), processLineType},
      {std::string("line_width"), processLineWidth},
      {std::string("marginal_heatmap_kind"), processMarginalHeatmapKind},
      {std::string("marker_color_ind"), processMarkerColorInd},
      {std::string("marker_size"), processMarkerSize},
      {std::string("marker_type"), processMarkerType},
      {std::string("projection_type"), processProjectionType},
      {std::string("max_char_height"), processRelativeCharHeight},
      {std::string("resample_method"), processResampleMethod},
      {std::string("reset_rotation"), processResetRotation},
      {std::string("select_specific_xform"), processSelectSpecificXform},
      {std::string("space"), processSpace},
      {std::string("space_3d_fov"), processSpace3d},          // the fov element can be used cause both must be set
      {std::string("text_align_vertical"), processTextAlign}, // the alignment in both directions is set
      {std::string("text_color_ind"), processTextColorInd},
      {std::string("text_encoding"), processTextEncoding},
      {std::string("viewport"), processViewport},
      {std::string("ws_viewport_x_min"),
       processWSViewport},                               // the xmin element can be used here cause all 4 are required
      {std::string("ws_window_x_min"), processWSWindow}, // the xmin element can be used here cause all 4 are required
      {std::string("x_flip"), processFlip},              // y_flip is also set
      {std::string("z_index"), processZIndex},
  };

  static std::map<std::string, std::function<void(const std::shared_ptr<GRM::Element> &)>> attrStringToFuncPost{
      /* This map contains functions for attributes that should be called after some attributes have been processed
       * already. These functions can contain e.g. inquire function calls for colors.
       * */
      {std::string("transparency"), processTransparency},
      {std::string("set_text_color_for_background"), processTextColorForBackground},
  };

  static std::map<std::string, std::function<void(const std::shared_ptr<GRM::Element> &, const std::string attribute)>>
      multiAttrStringToFunc{
          /* This map contains functions for attributes of which an element can hold more than one e.g. colorrep
           * */
          {std::string("colorrep"), processColorRep},
      };

  for (const auto &attribute : element->getAttributeNames())
    {
      auto start = 0U;
      auto end = attribute.find('.');
      if (end != std::string::npos)
        {
          /* element can hold more than one attribute of this kind */
          auto attributeKind = attribute.substr(start, end);
          if (multiAttrStringToFunc.find(attributeKind) != multiAttrStringToFunc.end())
            {
              multiAttrStringToFunc[attributeKind](element, attribute);
            }
        }
      else if (attrStringToFunc.find(attribute) != attrStringToFunc.end())
        {
          attrStringToFunc[attribute](element);
        }
    }

  for (auto &attribute : element->getAttributeNames())
    /*
     * Post process attribute run
     */
    {
      if (attrStringToFuncPost.find(attribute) != attrStringToFuncPost.end())
        {
          attrStringToFuncPost[attribute](element);
        }
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ element processing functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static void drawYLine(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  double window[4];
  double ymin;
  double line_x1, line_x2, line_y1, line_y2;
  std::shared_ptr<GRM::Element> series = nullptr, line, central_region, central_region_parent;
  std::string orientation = PLOT_DEFAULT_ORIENTATION;
  del_values del = del_values::update_without_default;

  auto subplot_element = getSubplotElement(element);
  auto kind = static_cast<std::string>(subplot_element->getAttribute("kind"));

  central_region_parent = subplot_element;
  if (kind == "marginal_heatmap") central_region_parent = subplot_element->children()[0];
  for (const auto &child : central_region_parent->children())
    {
      if (child->localName() == "central_region")
        {
          central_region = child;
          break;
        }
    }

  window[0] = static_cast<double>(central_region->getAttribute("window_x_min"));
  window[1] = static_cast<double>(central_region->getAttribute("window_x_max"));
  window[2] = static_cast<double>(central_region->getAttribute("window_y_min"));
  window[3] = static_cast<double>(central_region->getAttribute("window_y_max"));

  for (const auto &child : central_region->children())
    {
      if (child->localName() != "series_barplot" && child->localName() != "series_stem") continue;
      series = child;
      break;
    }

  if (series != nullptr)
    {
      if (series->hasAttribute("orientation"))
        {
          orientation = static_cast<std::string>(series->getAttribute("orientation"));
        }
      else
        {
          series->setAttribute("orientation", orientation);
        }

      if (series->hasAttribute("y_range_min")) ymin = static_cast<double>(series->getAttribute("y_range_min"));
    }
  if (series != nullptr && series->localName() == "series_barplot" && ymin < 0) ymin = 0;

  bool is_vertical = orientation == "vertical";

  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  line = element->querySelectors("[name=\"y_line\"]");

  if (is_vertical)
    {
      line_x1 = ymin;
      line_x2 = ymin;
      line_y1 = window[2];
      line_y2 = window[3];
    }
  else
    {
      line_x1 = window[0];
      line_x2 = window[1];
      line_y1 = ymin;
      line_y2 = ymin;
    }

  if (del != del_values::update_without_default && del != del_values::update_with_default || line == nullptr)
    {
      line = global_render->createPolyline(line_x1, line_x2, line_y1, line_y2, 0, 0.0, 1);
      element->append(line);
    }
  else if (line != nullptr)
    {
      global_render->createPolyline(line_x1, line_x2, line_y1, line_y2, 0, 0.0, 1, line);
    }
  if (line != nullptr)
    {
      line->setAttribute("name", "y_line");
      line->setAttribute("z_index", 4);
    }
}

static void processAxes3d(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for axes 3d
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double x_tick, y_tick, z_tick;
  double x_org, y_org, z_org;
  int x_major, y_major, z_major;
  int tick_orientation = 1;
  double tick_size;
  std::string x_org_pos = PLOT_DEFAULT_ORG_POS, y_org_pos = PLOT_DEFAULT_ORG_POS, z_org_pos = PLOT_DEFAULT_ORG_POS;

  if (element->hasAttribute("x_org_pos")) x_org_pos = static_cast<std::string>(element->getAttribute("x_org_pos"));
  if (element->hasAttribute("y_org_pos")) y_org_pos = static_cast<std::string>(element->getAttribute("y_org_pos"));
  if (element->hasAttribute("z_org_pos")) z_org_pos = static_cast<std::string>(element->getAttribute("z_org_pos"));

  getAxes3dInformation(element, x_org_pos, y_org_pos, z_org_pos, x_org, y_org, z_org, x_major, y_major, z_major, x_tick,
                       y_tick, z_tick);

  auto subplot_element = getSubplotElement(element);

  if (element->hasAttribute("tick_orientation"))
    {
      tick_orientation = static_cast<int>(element->getAttribute("tick_orientation"));
    }

  getTickSize(element, tick_size);
  tick_size *= tick_orientation;
  applyMoveTransformation(element);

  if (redraw_ws) gr_axes3d(x_tick, y_tick, z_tick, x_org, y_org, z_org, x_major, y_major, z_major, tick_size);
}

static void processCellArray(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for cellArray
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  auto xmin = static_cast<double>(element->getAttribute("x_min"));
  auto xmax = static_cast<double>(element->getAttribute("x_max"));
  auto ymin = static_cast<double>(element->getAttribute("y_min"));
  auto ymax = static_cast<double>(element->getAttribute("y_max"));
  auto dimx = static_cast<int>(element->getAttribute("x_dim"));
  auto dimy = static_cast<int>(element->getAttribute("y_dim"));
  auto scol = static_cast<int>(element->getAttribute("start_col"));
  auto srow = static_cast<int>(element->getAttribute("start_row"));
  auto ncol = static_cast<int>(element->getAttribute("num_col"));
  auto nrow = static_cast<int>(element->getAttribute("num_row"));
  auto color = static_cast<std::string>(element->getAttribute("color_ind_values"));
  applyMoveTransformation(element);
  if (redraw_ws)
    gr_cellarray(xmin, xmax, ymin, ymax, dimx, dimy, scol, srow, ncol, nrow,
                 (int *)&(GRM::get<std::vector<int>>((*context)[color])[0]));
}

static void processColorbar(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  double c_min, c_max;
  int data, i, options;
  int z_log = 0;
  int child_id = 0;
  del_values del = del_values::update_without_default;
  std::shared_ptr<GRM::Element> cell_array = nullptr, axis = nullptr;
  auto colors = static_cast<int>(element->getAttribute("color_ind"));

  if (!getLimitsForColorbar(element, c_min, c_max))
    {
      auto subplot_element = getSubplotElement(element);
      if (!getLimitsForColorbar(subplot_element, c_min, c_max))
        {
          throw NotFoundError("Missing limits\n");
        }
    }

  auto plot_parent = element->parentElement();
  getPlotParent(plot_parent);
  z_log = static_cast<int>(plot_parent->getAttribute("z_log"));
  global_render->setWindow(element->parentElement(), 0.0, 1.0, c_min, c_max);
  gr_setwindow(0.0, 1.0, c_min, c_max);

  calculateViewport(element);
  applyMoveTransformation(element);

  /* create cell array */
  std::vector<int> data_vec;
  for (i = 0; i < colors; ++i)
    {
      data = 1000 + (int)((255.0 * i) / (colors - 1) + 0.5);
      data_vec.push_back(data);
    }
  auto id = static_cast<int>(global_root->getAttribute("_id"));
  auto str = std::to_string(id);
  global_root->setAttribute("_id", id + 1);

  /* clear old child nodes */
  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  if (del != del_values::update_without_default && del != del_values::update_with_default)
    {
      cell_array =
          global_render->createCellArray(0, 1, c_max, c_min, 1, colors, 1, 1, 1, colors, "data" + str, data_vec);
      cell_array->setAttribute("_child_id", 0);
      element->append(cell_array);
    }
  else
    {
      cell_array = element->querySelectors("cellarray[_child_id=0]");
      if (cell_array != nullptr)
        global_render->createCellArray(0, 1, c_max, c_min, 1, colors, 1, 1, 1, colors, "data" + str, data_vec, context,
                                       cell_array);
    }
  if (cell_array != nullptr) cell_array->setAttribute("name", "colorbar");

  /* create axes */
  gr_inqscale(&options);
  if (options & GR_OPTION_Z_LOG || z_log)
    {
      axis_t y_axis = {c_min, c_max, 2, c_min, 1, 1, 0, nullptr, NAN, 0, nullptr, NAN, 1};
      gr_axis('Y', &y_axis);

      if ((del != del_values::update_without_default && del != del_values::update_with_default && axis == nullptr) ||
          !element->hasChildNodes())
        {
          axis = global_render->createAxis(y_axis.min, y_axis.max, y_axis.tick, y_axis.org, y_axis.position,
                                           y_axis.major_count, y_axis.num_ticks, y_axis.num_tick_labels,
                                           abs(y_axis.tick_size), y_axis.tick_size > 0 ? 1 : -1, y_axis.label_position);
          axis->setAttribute("_child_id", 1);
          global_render->setLineColorInd(axis, 1);
          element->append(axis);
        }
      else if (axis != nullptr)
        {
          axis = element->querySelectors("axis[_child_id=1]");
          if (axis != nullptr)
            {
              auto tick_size = y_axis.tick_size;
              if (axis->hasAttribute("tick_size")) tick_size = static_cast<double>(axis->getAttribute("tick_size"));
              if (axis->hasAttribute("_tick_size_set_by_user"))
                tick_size = static_cast<double>(axis->getAttribute("_tick_size_set_by_user"));

              global_render->createAxis(y_axis.min, y_axis.max, y_axis.tick, y_axis.org, y_axis.position,
                                        y_axis.major_count, y_axis.num_ticks, y_axis.num_tick_labels, abs(tick_size),
                                        tick_size > 0 ? 1 : -1, y_axis.label_position, axis);
            }
        }
      if (axis != nullptr)
        {
          global_render->setScale(axis, GR_OPTION_Y_LOG);
          global_render->processScale(axis);
          axis->setAttribute("name", "colorbar y-axis");
          axis->setAttribute("axis_type", "y");
          axis->setAttribute("draw_grid", false);
          axis->setAttribute("mirrored_axis", false);
          if (del == del_values::update_without_default) axis->setAttribute("min_value", c_min);
        }
      gr_freeaxis(&y_axis);
    }
  else
    {
      double c_tick = autoTick(c_min, c_max);
      axis_t y_axis = {c_min, c_max, c_tick, c_min, 1, 1, 0, nullptr, NAN, 0, nullptr, NAN, 1};
      gr_axis('Y', &y_axis);

      if (del != del_values::update_without_default && del != del_values::update_with_default)
        {
          axis = global_render->createAxis(y_axis.min, y_axis.max, y_axis.tick, y_axis.org, y_axis.position,
                                           y_axis.major_count, y_axis.num_ticks, y_axis.num_tick_labels,
                                           abs(y_axis.tick_size), y_axis.tick_size > 0 ? 1 : -1, y_axis.label_position);
          axis->setAttribute("_child_id", 1);
          global_render->setLineColorInd(axis, 1);
          element->append(axis);
        }
      else
        {
          axis = element->querySelectors("axis[_child_id=1]");
          if (axis != nullptr)
            {
              auto tick_size = y_axis.tick_size;
              if (axis->hasAttribute("tick_size")) tick_size = static_cast<double>(axis->getAttribute("tick_size"));
              if (axis->hasAttribute("_tick_size_set_by_user"))
                tick_size = static_cast<double>(axis->getAttribute("_tick_size_set_by_user"));

              global_render->createAxis(y_axis.min, y_axis.max, y_axis.tick, y_axis.org, y_axis.position,
                                        y_axis.major_count, y_axis.num_ticks, y_axis.num_tick_labels, abs(tick_size),
                                        tick_size > 0 ? 1 : -1, y_axis.label_position, axis);
            }
        }
      if (axis != nullptr)
        {
          axis->setAttribute("scale", 0);
          if (del == del_values::update_without_default)
            {
              axis->setAttribute("tick", c_tick);
              axis->setAttribute("min_value", c_min);
            }
        }
      processFlip(element);
      gr_freeaxis(&y_axis);
    }
  if (axis != nullptr)
    {
      if (!axis->hasAttribute("_tick_size_set_by_user"))
        axis->setAttribute("tick_size", PLOT_DEFAULT_COLORBAR_TICK_SIZE);
      else
        axis->setAttribute("tick_size", static_cast<double>(axis->getAttribute("_tick_size_set_by_user")));
      if (del != del_values::update_without_default)
        {
          axis->setAttribute("name", "colorbar y-axis");
          axis->setAttribute("axis_type", "y");
          axis->setAttribute("draw_grid", false);
          axis->setAttribute("mirrored_axis", false);
        }
    }
  applyMoveTransformation(element);
}

static void processBarplot(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for barplot
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */

  /* subplot level */
  int bar_color = 989, edge_color = 1;
  std::vector<double> bar_color_rgb = {-1, -1, -1};
  std::vector<double> edge_color_rgb = {-1, -1, -1};
  double bar_width = 0.8, edge_width = 1.0, bar_shift = 1;
  std::string style = "default", orientation = PLOT_DEFAULT_ORIENTATION;
  double wfac;
  int len_std_colors = 20;
  int std_colors[20] = {989, 982, 980, 981, 996, 983, 995, 988, 986, 990,
                        991, 984, 992, 993, 994, 987, 985, 997, 998, 999};
  int color_save_spot = PLOT_CUSTOM_COLOR_INDEX;
  unsigned int i;

  /* series level */
  unsigned int y_length, c_length, c_rgb_length;
  std::vector<int> c;
  std::vector<double> c_rgb;
  std::vector<std::string> ylabels;
  unsigned int ylabels_left = 0, ylabels_length = 0;
  /* style variance */
  double pos_vertical_change = 0, neg_vertical_change = 0;
  double x1, x2, y1, y2;
  double x_min = 0, x_max, y_min = 0;
  bool is_vertical;
  bool inner_series, inner_c = false, inner_c_rgb = false;
  del_values del = del_values::update_without_default;
  int child_id = 0;

  /* clear old bars */
  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  /* retrieve attributes from the subplot level */
  auto plot_parent = element->parentElement();
  getPlotParent(plot_parent);
  auto series_index = static_cast<int>(element->getAttribute("series_index"));
  auto fixed_y_length = static_cast<int>(plot_parent->getAttribute("max_y_length"));

  if (element->hasAttribute("fill_color_rgb"))
    {
      auto bar_color_rgb_key = static_cast<std::string>(element->getAttribute("fill_color_rgb"));
      bar_color_rgb = GRM::get<std::vector<double>>((*context)[bar_color_rgb_key]);
    }
  if (element->hasAttribute("bar_width")) bar_width = static_cast<double>(element->getAttribute("bar_width"));
  if (element->hasAttribute("style"))
    {
      style = static_cast<std::string>(element->getAttribute("style"));
    }
  else
    {
      element->setAttribute("style", style);
    }
  if (element->hasAttribute("orientation"))
    {
      orientation = static_cast<std::string>(element->getAttribute("orientation"));
    }
  else
    {
      element->setAttribute("orientation", orientation);
    }

  is_vertical = orientation == "vertical";

  if (bar_color_rgb[0] != -1)
    {
      for (i = 0; i < 3; i++)
        {
          if (bar_color_rgb[i] > 1 || bar_color_rgb[i] < 0)
            throw std::out_of_range("For barplot series bar_color_rgb must be inside [0, 1].\n");
        }
    }

  /* retrieve attributes form the series level */
  if (!element->hasAttribute("y")) throw NotFoundError("Barplot series is missing y.\n");

  auto y_key = static_cast<std::string>(element->getAttribute("y"));
  std::vector<double> y_vec = GRM::get<std::vector<double>>((*context)[y_key]);
  y_length = size(y_vec);

  if (!element->hasAttribute("indices")) throw NotFoundError("Barplot series is missing indices\n");
  auto indices = static_cast<std::string>(element->getAttribute("indices"));
  std::vector<int> indices_vec = GRM::get<std::vector<int>>((*context)[indices]);

  inner_series = size(indices_vec) != y_length;

  wfac = 0.9 * bar_width;

  if (element->hasAttribute("line_color_rgb"))
    {
      auto edge_color_rgb_key = static_cast<std::string>(element->getAttribute("line_color_rgb"));
      edge_color_rgb = GRM::get<std::vector<double>>((*context)[edge_color_rgb_key]);
    }
  if (element->hasAttribute("line_color_ind")) edge_color = static_cast<int>(element->getAttribute("line_color_ind"));
  if (element->hasAttribute("edge_width")) edge_width = static_cast<double>(element->getAttribute("edge_width"));
  global_render->setTextAlign(element, 2, 3);
  global_render->selectClipXForm(element, 1);

  if (edge_color_rgb[0] != -1)
    {
      for (i = 0; i < 3; i++)
        {
          if (edge_color_rgb[i] > 1 || edge_color_rgb[i] < 0)
            throw std::out_of_range("For barplot series edge_color_rgb must be inside [0, 1].\n");
        }
    }

  if (element->hasAttribute("color_ind_values"))
    {
      auto c_key = static_cast<std::string>(element->getAttribute("color_ind_values"));
      c = GRM::get<std::vector<int>>((*context)[c_key]);
      c_length = size(c);
    }
  if (element->hasAttribute("color_rgb_values"))
    {
      auto c_rgb_key = static_cast<std::string>(element->getAttribute("color_rgb_values"));
      c_rgb = GRM::get<std::vector<double>>((*context)[c_rgb_key]);
      c_rgb_length = size(c_rgb);
    }
  if (element->hasAttribute("y_labels"))
    {
      auto ylabels_key = static_cast<std::string>(element->getAttribute("y_labels"));
      ylabels = GRM::get<std::vector<std::string>>((*context)[ylabels_key]);
      ylabels_length = size(ylabels);

      ylabels_left = ylabels_length;
    }

  if (element->hasAttribute("x_range_min") && element->hasAttribute("x_range_max") &&
      element->hasAttribute("y_range_min"))
    {
      x_min = static_cast<double>(element->getAttribute("x_range_min"));
      x_max = static_cast<double>(element->getAttribute("x_range_max"));
      y_min = static_cast<double>(element->getAttribute("y_range_min"));
      if (!element->hasAttribute("bar_width"))
        {
          bar_width = (x_max - x_min) / (y_length - 1.0);
          bar_shift = (x_max - x_min) / (y_length - 1.0);
          x_min -= 1; // in the later calculation there is always a +1 in combination with x
          wfac = 0.9 * bar_width;
        }
    }

  if (style != "lined" && inner_series) throw TypeError("Unsupported operation for barplot series.\n");
  if (!c.empty())
    {
      if (!inner_series && (c_length < y_length))
        throw std::length_error("For a barplot series c_length must be >= y_length.\n");
      if (inner_series)
        {
          if (c_length == y_length)
            {
              inner_c = true;
            }
          else if (c_length != size(indices_vec))
            {
              throw std::length_error("For a barplot series c_length must be >= y_length.\n");
            }
        }
    }
  if (!c_rgb.empty())
    {
      if (!inner_series && (c_rgb_length < y_length * 3))
        throw std::length_error("For a barplot series c_rgb_length must be >= y_length * 3.\n");
      if (inner_series)
        {
          if (c_rgb_length == y_length * 3)
            {
              inner_c_rgb = true;
            }
          else if (c_rgb_length != size(indices_vec) * 3)
            {
              throw std::length_error("For a barplot series c_rgb_length must be >= y_length * 3\n");
            }
        }
      for (i = 0; i < y_length * 3; i++)
        {
          if ((c_rgb[i] > 1 || c_rgb[i] < 0) && c_rgb[i] != -1)
            throw std::out_of_range("For barplot series c_rgb must be inside [0, 1] or -1.\n");
        }
    }

  global_render->setFillIntStyle(element, 1);
  processFillIntStyle(element);
  if (!element->hasAttribute("fill_color_ind"))
    {
      global_render->setFillColorInd(element, bar_color);
    }
  else
    {
      bar_color = static_cast<int>(element->getAttribute("fill_color_ind"));
    }
  processFillColorInd(element);

  /* overrides bar_color */
  if (bar_color_rgb[0] != -1)
    {
      global_render->setColorRep(element, color_save_spot, bar_color_rgb[0], bar_color_rgb[1], bar_color_rgb[2]);
      processColorReps(element);
      bar_color = color_save_spot;
      global_render->setFillColorInd(element, bar_color);
      processFillColorInd(element);
    }
  if (!inner_series)
    {
      /* Draw Bar */
      for (i = 0; i < y_length; i++)
        {
          y1 = y_min;
          y2 = y_vec[i];

          if (style == "default")
            {
              x1 = (i * bar_shift) + 1 - 0.5 * bar_width;
              x2 = (i * bar_shift) + 1 + 0.5 * bar_width;
            }
          else if (style == "stacked")
            {
              x1 = series_index + 1 - 0.5 * bar_width;
              x2 = series_index + 1 + 0.5 * bar_width;
              if (y_vec[i] > 0)
                {
                  y1 = ((i == 0) ? y_min : 0) + pos_vertical_change;
                  pos_vertical_change += y_vec[i] - ((i > 0) ? y_min : 0);
                  y2 = pos_vertical_change;
                }
              else
                {
                  y1 = ((i == 0) ? y_min : 0) + neg_vertical_change;
                  neg_vertical_change += y_vec[i] - ((i > 0) ? y_min : 0);
                  y2 = neg_vertical_change;
                }
            }
          else if (style == "lined")
            {
              bar_width = wfac / y_length;
              x1 = series_index + 1 - 0.5 * wfac + bar_width * i;
              x2 = series_index + 1 - 0.5 * wfac + bar_width + bar_width * i;
            }
          x1 += x_min;
          x2 += x_min;

          if (is_vertical)
            {
              double tmp1 = x1, tmp2 = x2;
              x1 = y1, x2 = y2;
              y1 = tmp1, y2 = tmp2;
            }

          int fillcolorind = -1;
          std::vector<double> bar_fillcolor_rgb, edge_fillcolor_rgb;
          std::string bar_color_rgb_key, edge_color_rgb_key;
          std::shared_ptr<GRM::Element> bar;
          auto id = static_cast<int>(global_root->getAttribute("_id"));
          auto str = std::to_string(id);

          /* attributes for fill_rect */
          if (style != "default")
            {
              fillcolorind = std_colors[i % len_std_colors];
            }
          if (!c.empty() && c[i] != -1)
            {
              fillcolorind = c[i];
            }
          else if (!c_rgb.empty() && c_rgb[i * 3] != -1)
            {
              bar_fillcolor_rgb = std::vector<double>{c_rgb[i * 3], c_rgb[i * 3 + 1], c_rgb[i * 3 + 2]};
              bar_color_rgb_key = "fill_color_rgb" + str;
              (*context)[bar_color_rgb_key] = bar_fillcolor_rgb;
            }

          if (fillcolorind == -1)
            fillcolorind = element->hasAttribute("fill_color_ind")
                               ? static_cast<int>(element->getAttribute("fill_color_ind"))
                               : 989;

          /* Colorrep for draw_rect */
          if (edge_color_rgb[0] != -1)
            {
              edge_fillcolor_rgb = std::vector<double>{edge_color_rgb[0], edge_color_rgb[1], edge_color_rgb[2]};
              edge_color_rgb_key = "line_color_rgb" + str;
              (*context)[edge_color_rgb_key] = edge_fillcolor_rgb;
            }

          /* Create bars */
          if (del != del_values::update_without_default && del != del_values::update_with_default)
            {
              bar = global_render->createBar(x1, x2, y1, y2, fillcolorind, edge_color, bar_color_rgb_key,
                                             edge_color_rgb_key, edge_width, "");
              bar->setAttribute("_child_id", child_id++);
              element->append(bar);
            }
          else
            {
              bar = element->querySelectors("bar[_child_id=" + std::to_string(child_id++) + "]");
              if (bar != nullptr)
                global_render->createBar(x1, x2, y1, y2, fillcolorind, edge_color, bar_color_rgb_key,
                                         edge_color_rgb_key, edge_width, "", bar);
            }

          /* Draw y-notations */
          if (!ylabels.empty() && ylabels_left > 0)
            {
              if (bar != nullptr) bar->setAttribute("text", ylabels[i]);
              --ylabels_left;
            }
          global_root->setAttribute("_id", ++id);
        }
    }
  else
    {
      /* edge has the same with and color for every inner series */
      global_render->setLineWidth(element, edge_width);
      if (edge_color_rgb[0] != -1)
        {
          global_render->setColorRep(element, color_save_spot, edge_color_rgb[0], edge_color_rgb[1], edge_color_rgb[2]);
          processFillColorInd(element);
          edge_color = color_save_spot;
        }
      global_render->setLineColorInd(element, edge_color);
      element->setAttribute("line_color_ind", edge_color);
      processLineWidth(element);
      processLineColorInd(element);

      int inner_y_start_index = 0;
      /* Draw inner_series */
      for (int inner_series_index = 0; inner_series_index < size(indices_vec); inner_series_index++)
        {
          /* Draw bars from inner_series */
          int inner_y_length = indices_vec[inner_series_index];
          std::vector<double> inner_y_vec(y_vec.begin() + inner_y_start_index,
                                          y_vec.begin() + inner_y_start_index + inner_y_length);
          bar_width = wfac / fixed_y_length;

          for (i = 0; i < inner_y_length; i++)
            {
              x1 = series_index + 1 - 0.5 * wfac + bar_width * inner_series_index;
              x2 = series_index + 1 - 0.5 * wfac + bar_width + bar_width * inner_series_index;
              if (inner_y_vec[i] > 0)
                {
                  y1 = ((i == 0) ? y_min : 0) + pos_vertical_change;
                  pos_vertical_change += inner_y_vec[i] - ((i > 0) ? y_min : 0);
                  y2 = pos_vertical_change;
                }
              else
                {
                  y1 = ((i == 0) ? y_min : 0) + neg_vertical_change;
                  neg_vertical_change += inner_y_vec[i] - ((i > 0) ? y_min : 0);
                  y2 = neg_vertical_change;
                }
              x1 += x_min;
              x2 += x_min;

              if (is_vertical)
                {
                  double tmp1 = x1, tmp2 = x2;
                  x1 = y1, x2 = y2;
                  y1 = tmp1, y2 = tmp2;
                }

              int fillcolorind = -1;
              std::vector<double> bar_fillcolor_rgb, edge_fillcolor_rgb;
              std::string bar_color_rgb_key, edge_color_rgb_key;
              std::shared_ptr<GRM::Element> bar;
              auto id = static_cast<int>(global_root->getAttribute("_id"));
              auto str = std::to_string(id);

              /* attributes for fill_rect */
              if (!c.empty() && !inner_c && c[inner_series_index] != -1)
                {
                  fillcolorind = c[inner_series_index];
                }
              if (!c_rgb.empty() && !inner_c_rgb && c_rgb[inner_series_index * 3] != -1)
                {
                  bar_fillcolor_rgb =
                      std::vector<double>{c_rgb[inner_series_index * 3], c_rgb[inner_series_index * 3 + 1],
                                          c_rgb[inner_series_index * 3 + 2]};
                  bar_color_rgb_key = "fill_color_rgb" + str;
                  (*context)[bar_color_rgb_key] = bar_fillcolor_rgb;
                }
              if (inner_c && c[inner_y_start_index + i] != -1)
                {
                  fillcolorind = c[inner_y_start_index + i];
                }
              if (inner_c_rgb && c_rgb[(inner_y_start_index + i) * 3] != -1)
                {
                  bar_fillcolor_rgb = std::vector<double>{c_rgb[(inner_y_start_index + i) * 3],
                                                          c_rgb[(inner_y_start_index + i) * 3 + 1],
                                                          c_rgb[(inner_y_start_index + i) * 3 + 2]};
                  bar_color_rgb_key = "fill_color_rgb" + str;
                  (*context)[bar_color_rgb_key] = bar_fillcolor_rgb;
                }
              if (fillcolorind == -1) fillcolorind = std_colors[inner_series_index % len_std_colors];

              /* Create bars */
              if (del != del_values::update_without_default && del != del_values::update_with_default)
                {
                  bar = global_render->createBar(x1, x2, y1, y2, fillcolorind, edge_color, bar_color_rgb_key,
                                                 edge_color_rgb_key, edge_width, "");
                  bar->setAttribute("_child_id", child_id++);
                  element->append(bar);
                }
              else
                {
                  bar = element->querySelectors("bar[_child_id=" + std::to_string(child_id++) + "]");
                  if (bar != nullptr)
                    global_render->createBar(x1, x2, y1, y2, fillcolorind, edge_color, bar_color_rgb_key,
                                             edge_color_rgb_key, edge_width, "", bar);
                }

              /* Draw y-notations from inner_series */
              if (!ylabels.empty() && ylabels_left > 0)
                {
                  if (bar != nullptr) bar->setAttribute("text", ylabels[ylabels_length - ylabels_left]);
                  --ylabels_left;
                }
              global_root->setAttribute("_id", ++id);
            }
          pos_vertical_change = 0;
          neg_vertical_change = 0;
          y_length = 0;
          inner_y_start_index += inner_y_length;
        }
    }
  element->setAttribute("line_color_ind", edge_color);
  processLineColorInd(element);

  // error_bar handling
  for (const auto &child : element->children())
    {
      if (child->localName() == "error_bars")
        {
          std::vector<double> bar_centers;
          bar_width = wfac / y_length;
          for (i = 0; i < y_length; i++)
            {
              if (style == "default")
                {
                  x1 = x_min + (i * bar_shift) + 1 - 0.5 * bar_width;
                  x2 = x_min + (i * bar_shift) + 1 + 0.5 * bar_width;
                }
              else if (style == "lined")
                {
                  x1 = x_min + series_index + 1 - 0.5 * wfac + bar_width * i;
                  x2 = x_min + series_index + 1 - 0.5 * wfac + bar_width + bar_width * i;
                }
              else
                {
                  x1 = x_min + series_index + 1 - 0.5 * bar_width;
                  x2 = x_min + series_index + 1 + 0.5 * bar_width;
                }
              bar_centers.push_back((x1 + x2) / 2.0);
            }
          extendErrorBars(child, context, bar_centers, y_vec);
        }
    }
}

static void processContour(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for contour
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double z_min, z_max;
  int num_levels = PLOT_DEFAULT_CONTOUR_LEVELS;
  int i;
  unsigned int x_length, y_length, z_length;
  std::vector<double> x_vec, y_vec, z_vec;
  std::vector<double> px_vec, py_vec, pz_vec;
  int major_h = PLOT_DEFAULT_CONTOUR_MAJOR_H;
  auto plot_parent = element->parentElement();
  getPlotParent(plot_parent);
  z_min = element->hasAttribute("z_min") ? static_cast<double>(element->getAttribute("z_min"))
                                         : static_cast<double>(plot_parent->getAttribute("_z_lim_min"));
  z_max = element->hasAttribute("z_max") ? static_cast<double>(element->getAttribute("z_max"))
                                         : static_cast<double>(plot_parent->getAttribute("_z_lim_max"));
  if (element->hasAttribute("levels"))
    {
      num_levels = static_cast<int>(element->getAttribute("levels"));
    }
  else
    {
      element->setAttribute("levels", num_levels);
    }
  if (element->hasAttribute("major_h"))
    {
      major_h = static_cast<int>(element->getAttribute("major_h"));
    }

  gr_setprojectiontype(0);
  gr_setspace(z_min, z_max, 0, 90);

  std::vector<double> h(num_levels);

  if (!element->hasAttribute("px") || !element->hasAttribute("py") || !element->hasAttribute("pz"))
    {
      if (!element->hasAttribute("x")) throw NotFoundError("Contour series is missing required attribute x-data.\n");
      auto x = static_cast<std::string>(element->getAttribute("x"));
      if (!element->hasAttribute("y")) throw NotFoundError("Contour series is missing required attribute y-data.\n");
      auto y = static_cast<std::string>(element->getAttribute("y"));
      if (!element->hasAttribute("z")) throw NotFoundError("Contour series is missing required attribute z-data.\n");
      auto z = static_cast<std::string>(element->getAttribute("z"));

      x_vec = GRM::get<std::vector<double>>((*context)[x]);
      y_vec = GRM::get<std::vector<double>>((*context)[y]);
      z_vec = GRM::get<std::vector<double>>((*context)[z]);
      x_length = x_vec.size();
      y_length = y_vec.size();
      z_length = z_vec.size();

      auto id = static_cast<int>(global_root->getAttribute("_id"));
      global_root->setAttribute("_id", id + 1);
      auto str = std::to_string(id);

      if (x_length == y_length && x_length == z_length)
        {
          std::vector<double> gridit_x_vec(PLOT_CONTOUR_GRIDIT_N);
          std::vector<double> gridit_y_vec(PLOT_CONTOUR_GRIDIT_N);
          std::vector<double> gridit_z_vec(PLOT_CONTOUR_GRIDIT_N * PLOT_CONTOUR_GRIDIT_N);

          double *gridit_x = &(gridit_x_vec[0]);
          double *gridit_y = &(gridit_y_vec[0]);
          double *gridit_z = &(gridit_z_vec[0]);
          double *x_p = &(x_vec[0]);
          double *y_p = &(y_vec[0]);
          double *z_p = &(z_vec[0]);

          gr_gridit((int)x_length, x_p, y_p, z_p, PLOT_CONTOUR_GRIDIT_N, PLOT_CONTOUR_GRIDIT_N, gridit_x, gridit_y,
                    gridit_z);
          for (i = 0; i < PLOT_CONTOUR_GRIDIT_N * PLOT_CONTOUR_GRIDIT_N; i++)
            {
              z_min = grm_min(gridit_z[i], z_min);
              z_max = grm_max(gridit_z[i], z_max);
            }
          element->setAttribute("z_min", z_min);
          element->setAttribute("z_max", z_max);

          global_render->setSpace(element->parentElement(), z_min, z_max, 0,
                                  90); // not plot_parent because it should be now on central_region
          processSpace(element->parentElement());

          px_vec = std::vector<double>(gridit_x, gridit_x + PLOT_CONTOUR_GRIDIT_N);
          py_vec = std::vector<double>(gridit_y, gridit_y + PLOT_CONTOUR_GRIDIT_N);
          pz_vec = std::vector<double>(gridit_z, gridit_z + PLOT_CONTOUR_GRIDIT_N * PLOT_CONTOUR_GRIDIT_N);
        }
      else
        {
          if (x_length * y_length != z_length)
            throw std::length_error("For contour series x_length * y_length must be z_length.\n");

          px_vec = x_vec;
          py_vec = y_vec;
          pz_vec = z_vec;
        }

      (*context)["px" + str] = px_vec;
      element->setAttribute("px", "px" + str);
      (*context)["py" + str] = py_vec;
      element->setAttribute("py", "py" + str);
      (*context)["pz" + str] = pz_vec;
      element->setAttribute("pz", "pz" + str);
    }
  else
    {
      auto px = static_cast<std::string>(element->getAttribute("px"));
      auto py = static_cast<std::string>(element->getAttribute("py"));
      auto pz = static_cast<std::string>(element->getAttribute("pz"));

      px_vec = GRM::get<std::vector<double>>((*context)[px]);
      py_vec = GRM::get<std::vector<double>>((*context)[py]);
      pz_vec = GRM::get<std::vector<double>>((*context)[pz]);
    }

  for (i = 0; i < num_levels; ++i)
    {
      h[i] = z_min + (1.0 * i) / num_levels * (z_max - z_min);
    }

  auto nx = (int)px_vec.size();
  auto ny = (int)py_vec.size();

  double *px_p = &(px_vec[0]);
  double *py_p = &(py_vec[0]);
  double *h_p = &(h[0]);
  double *pz_p = &(pz_vec[0]);
  applyMoveTransformation(element);

  if (redraw_ws) gr_contour(nx, ny, num_levels, px_p, py_p, h_p, pz_p, major_h);
}

static void processContourf(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for contourf
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double z_min, z_max;
  int num_levels = PLOT_DEFAULT_CONTOUR_LEVELS;
  int i;
  unsigned int x_length, y_length, z_length;
  std::vector<double> x_vec, y_vec, z_vec;
  std::vector<double> px_vec, py_vec, pz_vec;
  int major_h = PLOT_DEFAULT_CONTOURF_MAJOR_H;
  auto plot_parent = element->parentElement();
  getPlotParent(plot_parent);
  z_min = element->hasAttribute("z_min") ? static_cast<double>(element->getAttribute("z_min"))
                                         : static_cast<double>(plot_parent->getAttribute("_z_lim_min"));
  z_max = element->hasAttribute("z_max") ? static_cast<double>(element->getAttribute("z_max"))
                                         : static_cast<double>(plot_parent->getAttribute("_z_lim_max"));
  if (element->hasAttribute("levels"))
    {
      num_levels = static_cast<int>(element->getAttribute("levels"));
    }
  else
    {
      element->setAttribute("levels", num_levels);
    }
  if (element->hasAttribute("major_h"))
    {
      major_h = static_cast<int>(element->getAttribute("major_h"));
    }

  gr_setprojectiontype(0);
  gr_setspace(z_min, z_max, 0, 90);

  std::vector<double> h(num_levels);

  if (!element->hasAttribute("px") || !element->hasAttribute("py") || !element->hasAttribute("pz"))
    {
      if (!element->hasAttribute("x")) throw NotFoundError("Contourf series is missing required attribute x-data.\n");
      auto x = static_cast<std::string>(element->getAttribute("x"));
      if (!element->hasAttribute("y")) throw NotFoundError("Contourf series is missing required attribute y-data.\n");
      auto y = static_cast<std::string>(element->getAttribute("y"));
      if (!element->hasAttribute("z")) throw NotFoundError("Contourf series is missing required attribute z-data.\n");
      auto z = static_cast<std::string>(element->getAttribute("z"));

      x_vec = GRM::get<std::vector<double>>((*context)[x]);
      y_vec = GRM::get<std::vector<double>>((*context)[y]);
      z_vec = GRM::get<std::vector<double>>((*context)[z]);
      x_length = x_vec.size();
      y_length = y_vec.size();
      z_length = z_vec.size();

      auto id = static_cast<int>(global_root->getAttribute("_id"));
      global_root->setAttribute("_id", id + 1);
      auto str = std::to_string(id);

      gr_setlinecolorind(1);
      global_render->setLineColorInd(element, 1);

      if (x_length == y_length && x_length == z_length)
        {
          std::vector<double> gridit_x_vec(PLOT_CONTOUR_GRIDIT_N);
          std::vector<double> gridit_y_vec(PLOT_CONTOUR_GRIDIT_N);
          std::vector<double> gridit_z_vec(PLOT_CONTOUR_GRIDIT_N * PLOT_CONTOUR_GRIDIT_N);

          double *gridit_x = &(gridit_x_vec[0]);
          double *gridit_y = &(gridit_y_vec[0]);
          double *gridit_z = &(gridit_z_vec[0]);
          double *x_p = &(x_vec[0]);
          double *y_p = &(y_vec[0]);
          double *z_p = &(z_vec[0]);

          gr_gridit((int)x_length, x_p, y_p, z_p, PLOT_CONTOUR_GRIDIT_N, PLOT_CONTOUR_GRIDIT_N, gridit_x, gridit_y,
                    gridit_z);
          for (i = 0; i < PLOT_CONTOUR_GRIDIT_N * PLOT_CONTOUR_GRIDIT_N; i++)
            {
              z_min = grm_min(gridit_z[i], z_min);
              z_max = grm_max(gridit_z[i], z_max);
            }
          element->setAttribute("z_min", z_min);
          element->setAttribute("z_max", z_max);

          global_render->setLineColorInd(element, 989);
          global_render->setSpace(element->parentElement(), z_min, z_max, 0, 90); // central_region
          processSpace(element->parentElement());

          px_vec = std::vector<double>(gridit_x, gridit_x + PLOT_CONTOUR_GRIDIT_N);
          py_vec = std::vector<double>(gridit_y, gridit_y + PLOT_CONTOUR_GRIDIT_N);
          pz_vec = std::vector<double>(gridit_z, gridit_z + PLOT_CONTOUR_GRIDIT_N * PLOT_CONTOUR_GRIDIT_N);
        }
      else
        {
          if (x_length * y_length != z_length)
            throw std::length_error("For contourf series x_length * y_length must be z_length.\n");

          global_render->setLineColorInd(element, 989);

          px_vec = x_vec;
          py_vec = y_vec;
          pz_vec = z_vec;
        }

      (*context)["px" + str] = px_vec;
      element->setAttribute("px", "px" + str);
      (*context)["py" + str] = py_vec;
      element->setAttribute("py", "py" + str);
      (*context)["pz" + str] = pz_vec;
      element->setAttribute("pz", "pz" + str);
      processLineColorInd(element);
    }
  else
    {
      auto px = static_cast<std::string>(element->getAttribute("px"));
      auto py = static_cast<std::string>(element->getAttribute("py"));
      auto pz = static_cast<std::string>(element->getAttribute("pz"));

      px_vec = GRM::get<std::vector<double>>((*context)[px]);
      py_vec = GRM::get<std::vector<double>>((*context)[py]);
      pz_vec = GRM::get<std::vector<double>>((*context)[pz]);
    }

  for (i = 0; i < num_levels; ++i)
    {
      h[i] = z_min + (1.0 * i) / num_levels * (z_max - z_min);
    }

  auto nx = (int)px_vec.size();
  auto ny = (int)py_vec.size();

  double *px_p = &(px_vec[0]);
  double *py_p = &(py_vec[0]);
  double *h_p = &(h[0]);
  double *pz_p = &(pz_vec[0]);
  applyMoveTransformation(element);

  if (redraw_ws) gr_contourf(nx, ny, num_levels, px_p, py_p, h_p, pz_p, major_h);
}

static void processDrawArc(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for draw_arc
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  auto x_min = static_cast<double>(element->getAttribute("x_min"));
  auto x_max = static_cast<double>(element->getAttribute("x_max"));
  auto y_min = static_cast<double>(element->getAttribute("y_min"));
  auto y_max = static_cast<double>(element->getAttribute("y_max"));
  auto start_angle = static_cast<double>(element->getAttribute("start_angle"));
  auto end_angle = static_cast<double>(element->getAttribute("end_angle"));
  applyMoveTransformation(element);
  if (redraw_ws) gr_drawarc(x_min, x_max, y_min, y_max, start_angle, end_angle);
}

static void processDrawGraphics(const std::shared_ptr<GRM::Element> &element,
                                const std::shared_ptr<GRM::Context> &context)
{
  std::vector<char> char_vec;
  auto key = static_cast<std::string>(element->getAttribute("data"));
  auto data_vec = GRM::get<std::vector<int>>((*context)[key]);

  char_vec.reserve(data_vec.size());
  for (int i : data_vec)
    {
      char_vec.push_back((char)i);
    }
  char *data_p = &(char_vec[0]);
  applyMoveTransformation(element);

  if (redraw_ws) gr_drawgraphics(data_p);
}

static void processDrawImage(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for drawImage
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  int model;
  auto x_min = static_cast<double>(element->getAttribute("x_min"));
  auto x_max = static_cast<double>(element->getAttribute("x_max"));
  auto y_min = static_cast<double>(element->getAttribute("y_min"));
  auto y_max = static_cast<double>(element->getAttribute("y_max"));
  auto width = static_cast<int>(element->getAttribute("width"));
  auto height = static_cast<int>(element->getAttribute("height"));
  auto data = static_cast<std::string>(element->getAttribute("data"));
  if (element->getAttribute("model").isInt())
    {
      model = static_cast<int>(element->getAttribute("model"));
    }
  else if (element->getAttribute("model").isString())
    {
      model = modelStringToInt(static_cast<std::string>(element->getAttribute("model")));
    }
  applyMoveTransformation(element);
  if (redraw_ws)
    gr_drawimage(x_min, x_max, y_max, y_min, width, height, (int *)&(GRM::get<std::vector<int>>((*context)[data])[0]),
                 model);
}

static void processErrorBars(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  std::string orientation = PLOT_DEFAULT_ORIENTATION, kind;
  bool is_horizontal;
  std::vector<double> absolute_upwards_vec, absolute_downwards_vec, relative_upwards_vec, relative_downwards_vec;
  std::string absolute_upwards, absolute_downwards, relative_upwards, relative_downwards;
  double absolute_upwards_flt, relative_upwards_flt, absolute_downwards_flt, relative_downwards_flt;
  int scale_options, color_upwards_cap, color_downwards_cap, color_error_bar;
  double marker_size, x_min, x_max, y_min, y_max, tick, a, b, e_upwards, e_downwards, x_value;
  double line_x[2], line_y[2];
  std::vector<double> x_vec, y_vec;
  unsigned int x_length;
  std::string x_key, y_key;
  std::shared_ptr<GRM::Element> series;
  del_values del = del_values::update_without_default;
  int child_id = 0;

  absolute_upwards_flt = absolute_downwards_flt = relative_upwards_flt = relative_downwards_flt = FLT_MAX;
  if (static_cast<std::string>(element->parentElement()->parentElement()->localName()) == "central_region")
    {
      series = element->parentElement();
    }
  else
    {
      series = element->parentElement()->parentElement(); // marginal heatmap
    }

  if (!element->hasAttribute("x")) throw NotFoundError("Error-bars are missing required attribute x-data.\n");
  x_key = static_cast<std::string>(element->getAttribute("x"));
  if (!element->hasAttribute("y")) throw NotFoundError("Error-bars are missing required attribute y-data.\n");
  y_key = static_cast<std::string>(element->getAttribute("y"));

  x_vec = GRM::get<std::vector<double>>((*context)[x_key]);
  y_vec = GRM::get<std::vector<double>>((*context)[y_key]);
  x_length = x_vec.size();
  kind = static_cast<std::string>(series->parentElement()->getAttribute("kind"));
  if (element->parentElement()->hasAttribute("orientation"))
    {
      orientation = static_cast<std::string>(element->parentElement()->getAttribute("orientation"));
    }
  else
    {
      element->parentElement()->setAttribute("orientation", orientation);
    }

  if (!element->hasAttribute("absolute_downwards") && !element->hasAttribute("relative_downwards"))
    throw NotFoundError("Error-bars are missing required attribute downwards.\n");
  if (!element->hasAttribute("absolute_upwards") && !element->hasAttribute("relative_upwards"))
    throw NotFoundError("Error-bars are missing required attribute upwards.\n");
  if (element->hasAttribute("absolute_downwards"))
    {
      absolute_downwards = static_cast<std::string>(element->getAttribute("absolute_downwards"));
      absolute_downwards_vec = GRM::get<std::vector<double>>((*context)[absolute_downwards]);
    }
  if (element->hasAttribute("relative_downwards"))
    {
      relative_downwards = static_cast<std::string>(element->getAttribute("relative_downwards"));
      relative_downwards_vec = GRM::get<std::vector<double>>((*context)[relative_downwards]);
    }
  if (element->hasAttribute("absolute_upwards"))
    {
      absolute_upwards = static_cast<std::string>(element->getAttribute("absolute_upwards"));
      absolute_upwards_vec = GRM::get<std::vector<double>>((*context)[absolute_upwards]);
    }
  if (element->hasAttribute("relative_upwards"))
    {
      relative_upwards = static_cast<std::string>(element->getAttribute("relative_upwards"));
      relative_upwards_vec = GRM::get<std::vector<double>>((*context)[relative_upwards]);
    }
  if (element->hasAttribute("absolute_downwards_flt"))
    absolute_downwards_flt = static_cast<double>(element->getAttribute("absolute_downwards_flt"));
  if (element->hasAttribute("absolute_upwards_flt"))
    absolute_upwards_flt = static_cast<double>(element->getAttribute("absolute_upwards_flt"));
  if (element->hasAttribute("relative_downwards_flt"))
    relative_downwards_flt = static_cast<double>(element->getAttribute("relative_downwards_flt"));
  if (element->hasAttribute("relative_upwards_flt"))
    relative_upwards_flt = static_cast<double>(element->getAttribute("relative_upwards_flt"));

  if (absolute_upwards_vec.empty() && relative_upwards_vec.empty() && absolute_upwards_flt == FLT_MAX &&
      relative_upwards_flt == FLT_MAX && absolute_downwards_vec.empty() && relative_downwards_vec.empty() &&
      absolute_downwards_flt == FLT_MAX && relative_downwards_flt == FLT_MAX)
    {
      throw NotFoundError("Error-bar is missing required error-data.");
    }

  is_horizontal = orientation == "horizontal";

  /* Getting GRM options and sizes. See gr_verrorbars. */
  gr_savestate();
  gr_inqmarkersize(&marker_size);
  gr_inqwindow(&x_min, &x_max, &y_min, &y_max);
  gr_inqscale(&scale_options);
  tick = marker_size * 0.0075 * (x_max - x_min);
  a = (x_max - x_min) / log10(x_max / x_min);
  b = x_min - a * log10(x_min);

  gr_inqlinecolorind(&color_error_bar);
  // special case for barplot
  if (kind == "barplot") color_error_bar = static_cast<int>(element->parentElement()->getAttribute("line_color_ind"));
  color_upwards_cap = color_downwards_cap = color_error_bar;
  if (element->hasAttribute("upwards_cap_color"))
    color_upwards_cap = static_cast<int>(element->getAttribute("upwards_cap_color"));
  if (element->hasAttribute("downwards_cap_color"))
    color_downwards_cap = static_cast<int>(element->getAttribute("downwards_cap_color"));
  if (element->hasAttribute("error_bar_color"))
    color_error_bar = static_cast<int>(element->getAttribute("error_bar_color"));

  /* clear old lines */
  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  /* Actual drawing of bars */
  e_upwards = e_downwards = FLT_MAX;
  for (int i = 0; i < x_length; i++)
    {
      std::shared_ptr<GRM::Element> error_bar;
      if (!absolute_upwards.empty() || !relative_upwards.empty() || absolute_upwards_flt != FLT_MAX ||
          relative_upwards_flt != FLT_MAX)
        {
          e_upwards = y_vec[i] * (1. + (!relative_upwards.empty()
                                            ? relative_upwards_vec[i]
                                            : (relative_upwards_flt != FLT_MAX ? relative_upwards_flt : 0))) +
                      (!absolute_upwards.empty() ? absolute_upwards_vec[i]
                                                 : (absolute_upwards_flt != FLT_MAX ? absolute_upwards_flt : 0.));
        }

      if (!absolute_downwards.empty() || !relative_downwards.empty() || absolute_downwards_flt != FLT_MAX ||
          relative_downwards_flt != FLT_MAX)
        {
          e_downwards =
              y_vec[i] * (1. - (!relative_downwards.empty()
                                    ? relative_downwards_vec[i]
                                    : (relative_downwards_flt != FLT_MAX ? relative_downwards_flt : 0))) -
              (!absolute_downwards.empty() ? absolute_downwards_vec[i]
                                           : (absolute_downwards_flt != FLT_MAX ? absolute_downwards_flt : 0.));
        }

      /* See gr_verrorbars for reference */
      x_value = x_vec[i];
      line_x[0] = X_LOG(X_LIN(x_value - tick, scale_options, x_min, x_max, a, b), scale_options, x_min, x_max, a, b);
      line_x[1] = X_LOG(X_LIN(x_value + tick, scale_options, x_min, x_max, a, b), scale_options, x_min, x_max, a, b);

      if (!is_horizontal)
        {
          double tmp1, tmp2;
          tmp1 = line_x[0], tmp2 = line_x[1];
          line_x[0] = line_y[0], line_x[1] = line_y[1];
          line_y[0] = tmp1, line_y[1] = tmp2;
        }

      if (color_error_bar >= 0)
        {
          line_y[0] = e_upwards != FLT_MAX ? e_upwards : y_vec[i];
          line_y[1] = e_downwards != FLT_MAX ? e_downwards : y_vec[i];
          if (del != del_values::update_without_default && del != del_values::update_with_default)
            {
              error_bar = global_render->createErrorBar(x_value, line_y[0], line_y[1], color_error_bar);
              error_bar->setAttribute("_child_id", child_id++);
              element->append(error_bar);
            }
          else
            {
              error_bar = element->querySelectors("error_bar[_child_id=" + std::to_string(child_id++) + "]");
              if (error_bar != nullptr)
                global_render->createErrorBar(x_value, line_y[0], line_y[1], color_error_bar, error_bar);
            }

          if (error_bar != nullptr)
            {
              if (e_upwards != FLT_MAX)
                {
                  error_bar->setAttribute("e_upwards", e_upwards);
                  error_bar->setAttribute("upwards_cap_color", color_upwards_cap);
                }
              if (e_downwards != FLT_MAX)
                {
                  error_bar->setAttribute("e_downwards", e_downwards);
                  error_bar->setAttribute("downwards_cap_color", color_downwards_cap);
                }
              if (e_downwards != FLT_MAX || e_upwards != FLT_MAX)
                {
                  error_bar->setAttribute("cap_x_min", line_x[0]);
                  error_bar->setAttribute("cap_x_max", line_x[1]);
                }
            }
        }
    }
  gr_restorestate();
}

static void processErrorBar(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  double cap_x_min, cap_x_max, e_upwards = FLT_MAX, e_downwards = FLT_MAX;
  double error_bar_x, error_bar_y_min, error_bar_y_max;
  int color_upwards_cap = 0, color_downwards_cap = 0, color_error_bar;
  std::shared_ptr<GRM::Element> line;
  del_values del = del_values::update_without_default;
  int child_id = 0;

  /* clear old lines */
  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  error_bar_x = static_cast<double>(element->getAttribute("error_bar_x"));
  error_bar_y_min = static_cast<double>(element->getAttribute("error_bar_y_min"));
  error_bar_y_max = static_cast<double>(element->getAttribute("error_bar_y_max"));
  color_error_bar = static_cast<int>(element->getAttribute("error_bar_color"));

  if (element->hasAttribute("cap_x_min")) cap_x_min = static_cast<double>(element->getAttribute("cap_x_min"));
  if (element->hasAttribute("cap_x_max")) cap_x_max = static_cast<double>(element->getAttribute("cap_x_max"));
  if (element->hasAttribute("e_upwards")) e_upwards = static_cast<double>(element->getAttribute("e_upwards"));
  if (element->hasAttribute("e_downwards")) e_downwards = static_cast<double>(element->getAttribute("e_downwards"));
  if (element->hasAttribute("upwards_cap_color"))
    color_upwards_cap = static_cast<int>(element->getAttribute("upwards_cap_color"));
  if (element->hasAttribute("downwards_cap_color"))
    color_downwards_cap = static_cast<int>(element->getAttribute("downwards_cap_color"));

  if (e_upwards != FLT_MAX && color_upwards_cap >= 0)
    {
      if (del != del_values::update_without_default && del != del_values::update_with_default)
        {
          line = global_render->createPolyline(cap_x_min, cap_x_max, e_upwards, e_upwards, 0, 0.0, color_upwards_cap);
          line->setAttribute("_child_id", child_id++);
          element->append(line);
        }
      else
        {
          line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
          if (line != nullptr)
            global_render->createPolyline(cap_x_min, cap_x_max, e_upwards, e_upwards, 0, 0.0, color_upwards_cap, line);
        }
    }

  if (e_downwards != FLT_MAX && color_downwards_cap >= 0)
    {
      if (del != del_values::update_without_default && del != del_values::update_with_default)
        {
          line = global_render->createPolyline(cap_x_min, cap_x_max, e_downwards, e_downwards, 0, 0.0,
                                               color_downwards_cap);
          line->setAttribute("_child_id", child_id++);
          element->append(line);
        }
      else
        {
          line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
          if (line != nullptr)
            global_render->createPolyline(cap_x_min, cap_x_max, e_downwards, e_downwards, 0, 0.0, color_downwards_cap,
                                          line);
        }
    }

  if (color_error_bar >= 0)
    {
      if (del != del_values::update_without_default && del != del_values::update_with_default)
        {
          line = global_render->createPolyline(error_bar_x, error_bar_x, error_bar_y_min, error_bar_y_max, 0, 0.0,
                                               color_error_bar);
          line->setAttribute("_child_id", child_id++);
          element->append(line);
        }
      else
        {
          line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
          if (line != nullptr)
            global_render->createPolyline(error_bar_x, error_bar_x, error_bar_y_min, error_bar_y_max, 0, 0.0,
                                          color_error_bar, line);
        }
    }
}

static void processIsosurface(const std::shared_ptr<GRM::Element> &element,
                              const std::shared_ptr<GRM::Context> &context)
{
  std::vector<double> z_vec, temp_colors;
  unsigned int i, z_length, dims;
  int strides[3];
  double c_min, c_max, isovalue = 0.5;
  float foreground_colors[3] = {0.0, 0.5, 0.8};

  if (!element->hasAttribute("z")) throw NotFoundError("Isosurface series is missing required attribute z-data.\n");
  auto z_key = static_cast<std::string>(element->getAttribute("z"));
  z_vec = GRM::get<std::vector<double>>((*context)[z_key]);
  z_length = z_vec.size();

  if (!element->hasAttribute("z_dims"))
    throw NotFoundError("Isosurface series is missing required attribute z_dims.\n");
  auto z_dims_key = static_cast<std::string>(element->getAttribute("z_dims"));
  auto z_dims_vec = GRM::get<std::vector<int>>((*context)[z_dims_key]);
  dims = z_dims_vec.size();

  if (dims != 3) throw std::length_error("For isosurface series the size of z_dims has to be 3.\n");
  if (z_dims_vec[0] * z_dims_vec[1] * z_dims_vec[2] != z_length)
    throw std::length_error("For isosurface series shape[0] * shape[1] * shape[2] must be c_length.\n");
  if (z_length <= 0) throw NotFoundError("For isosurface series the size of c has to be greater than 0.\n");

  if (element->hasAttribute("isovalue")) isovalue = static_cast<double>(element->getAttribute("isovalue"));
  element->setAttribute("isovalue", isovalue);
  /* We need to convert the double values to floats, as GR3 expects floats, but an argument can only contain doubles. */
  if (element->hasAttribute("color_rgb_values"))
    {
      auto temp_c = static_cast<std::string>(element->getAttribute("color_rgb_values"));
      temp_colors = GRM::get<std::vector<double>>((*context)[temp_c]);
      i = temp_colors.size();
      if (i != 3) throw std::length_error("For isosurface series the foreground colors must have size 3.\n");
      while (i-- > 0)
        {
          foreground_colors[i] = (float)temp_colors[i];
        }
    }
  logger((stderr, "Colors; %f %f %f\n", foreground_colors[0], foreground_colors[1], foreground_colors[2]));

  /* Check if any value is finite in array, also calculation of real min and max */
  c_min = c_max = z_vec[0];
  for (i = 0; i < z_length; ++i)
    {
      if (std::isfinite(z_vec[i]))
        {
          if (grm_isnan(c_min) || c_min > z_vec[i])
            {
              c_min = z_vec[i];
            }
          if (grm_isnan(c_max) || c_max < z_vec[i])
            {
              c_max = z_vec[i];
            }
        }
    }
  if (c_min == c_max || !std::isfinite(c_min) || !std::isfinite(c_max))
    throw NotFoundError("For isosurface series the given c-data isn't enough.\n");

  logger((stderr, "c_min %lf c_max %lf isovalue %lf\n ", c_min, c_max, isovalue));
  std::vector<float> conv_data(z_vec.begin(), z_vec.end());

  strides[0] = z_dims_vec[1] * z_dims_vec[2];
  strides[1] = z_dims_vec[2];
  strides[2] = 1;

  global_render->setGR3LightParameters(element, 0.2, 0.8, 0.7, 128);

  float ambient = (float)static_cast<double>(element->getAttribute("ambient"));
  float diffuse = (float)static_cast<double>(element->getAttribute("diffuse"));
  float specular = (float)static_cast<double>(element->getAttribute("specular"));
  float specular_power = (float)static_cast<double>(element->getAttribute("specular_power"));

  float *data = &(conv_data[0]);
  float light_parameters[4];

  gr3_clear();

  /* Save and restore original light parameters */
  gr3_getlightparameters(&light_parameters[0], &light_parameters[1], &light_parameters[2], &light_parameters[3]);
  gr3_setlightparameters(ambient, diffuse, specular, specular_power);

  if (redraw_ws)
    gr3_isosurface(z_dims_vec[0], z_dims_vec[1], z_dims_vec[2], data, (float)isovalue, foreground_colors, strides);

  gr3_setlightparameters(light_parameters[0], light_parameters[1], light_parameters[2], light_parameters[3]);
}

static void processLegend(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  double tbx[4], tby[4];
  std::shared_ptr<GRM::Render> render;
  std::string labels_key = static_cast<std::string>(element->getAttribute("labels"));
  auto labels = GRM::get<std::vector<std::string>>((*context)[labels_key]);
  del_values del = del_values::update_without_default;
  int child_id = 0;
  auto plot_parent = element->parentElement();
  getPlotParent(plot_parent);

  std::shared_ptr<GRM::Element> central_region, central_region_parent;
  auto kind = static_cast<std::string>(plot_parent->getAttribute("kind"));

  central_region_parent = plot_parent;
  if (kind == "marginal_heatmap") central_region_parent = plot_parent->children()[0];
  for (const auto &child : central_region_parent->children())
    {
      if (child->localName() == "central_region")
        {
          central_region = child;
          break;
        }
    }

  render = std::dynamic_pointer_cast<GRM::Render>(element->ownerDocument());
  if (!render)
    {
      throw NotFoundError("No render-document found for element\n");
    }

  calculateViewport(element);
  applyMoveTransformation(element);

  if (static_cast<std::string>(plot_parent->getAttribute("kind")) != "pie")
    {
      double legend_symbol_x[2], legend_symbol_y[2];
      int i, legend_elems = 0;
      double viewport[4];
      std::shared_ptr<GRM::Element> fr, dr;

      gr_savestate();

      auto specs_key = static_cast<std::string>(element->getAttribute("specs"));
      std::vector<std::string> specs = GRM::get<std::vector<std::string>>((*context)[specs_key]);
      auto scale_factor = static_cast<double>(element->getAttribute("_scale_factor"));

      viewport[0] = static_cast<double>(element->getAttribute("viewport_x_min"));
      viewport[1] = static_cast<double>(element->getAttribute("viewport_x_max"));
      viewport[2] = static_cast<double>(element->getAttribute("viewport_y_min"));
      viewport[3] = static_cast<double>(element->getAttribute("viewport_y_max"));

      del = del_values(static_cast<int>(element->getAttribute("_delete_children")));

      /* get the amount of series which should be displayed inside the legend */
      for (const auto &series : element->parentElement()->children()) // central_region children
        {
          if (series->localName() != "series_line" && series->localName() != "series_scatter") continue;
          for (const auto &child : series->children())
            {
              if (child->localName() != "polyline" && child->localName() != "polymarker") continue;
              legend_elems += 1;
            }
        }
      if (element->hasAttribute("_legend_elems"))
        {
          /* the amount has changed - all legend children have to recreated cause its unknown which is new or gone */
          if (static_cast<int>(element->getAttribute("_legend_elems")) != legend_elems)
            {
              del = (del == del_values::recreate_all_children) ? del_values::recreate_all_children
                                                               : del_values::recreate_own_children;
              element->setAttribute("_legend_elems", legend_elems);
            }
        }
      else
        {
          element->setAttribute("_legend_elems", legend_elems);
        }

      /* clear old child nodes */
      clearOldChildren(&del, element);

      gr_selntran(1);

      render->setSelectSpecificXform(element, 0);
      render->setScale(element, 0);

      if (del != del_values::update_without_default && del != del_values::update_with_default)
        {
          fr = render->createFillRect(viewport[0], viewport[1], viewport[3], viewport[2]);
          fr->setAttribute("_child_id", child_id++);
          element->append(fr);
        }
      else
        {
          fr = element->querySelectors("fill_rect[_child_id=" + std::to_string(child_id++) + "]");
          if (fr != nullptr) render->createFillRect(viewport[0], viewport[1], viewport[3], viewport[2], 0, 0, -1, fr);
        }

      render->setFillIntStyle(element, GKS_K_INTSTYLE_SOLID);
      render->setFillColorInd(element, 0);

      if (del != del_values::update_without_default && del != del_values::update_with_default)
        {
          dr = render->createDrawRect(viewport[0], viewport[1], viewport[3], viewport[2]);
          dr->setAttribute("_child_id", child_id++);
          element->append(dr);
        }
      else
        {
          dr = element->querySelectors("draw_rect[_child_id=" + std::to_string(child_id++) + "]");
          if (dr != nullptr) render->createDrawRect(viewport[0], viewport[1], viewport[3], viewport[2], dr);
        }

      if (dr != nullptr && del != del_values::update_without_default)
        {
          render->setLineType(dr, GKS_K_INTSTYLE_SOLID);
          render->setLineColorInd(dr, 1);
          render->setLineWidth(dr, 1);
        }

      i = 0;
      render->setLineSpec(element, const_cast<char *>(" "));

      int spec_i = 0;
      for (const auto &plot_child : element->parentElement()->children()) // central_region childs
        {
          if (plot_child->localName() != "central_region") continue;
          for (const auto &series : plot_child->children())
            {
              int mask;
              double dy;

              if (series->localName() != "series_line" && series->localName() != "series_scatter") continue;

              if (i <= labels.size())
                {
                  gr_inqtext(0, 0, labels[i].data(), tbx, tby);
                  dy = grm_max((tby[2] - tby[0]) - 0.03 * scale_factor, 0);
                  viewport[3] -= 0.5 * dy;
                }

              gr_savestate();
              mask = gr_uselinespec(specs[spec_i].data());
              gr_restorestate();

              if (int_equals_any(mask, 5, 0, 1, 3, 4, 5))
                {
                  legend_symbol_x[0] = viewport[0] + 0.01 * scale_factor;
                  legend_symbol_x[1] = viewport[0] + 0.07 * scale_factor;
                  legend_symbol_y[0] = viewport[3] - 0.03 * scale_factor;
                  legend_symbol_y[1] = viewport[3] - 0.03 * scale_factor;
                  for (const auto &child : series->children())
                    {
                      std::shared_ptr<GRM::Element> pl;
                      if (child->localName() == "polyline")
                        {
                          if (del != del_values::update_without_default && del != del_values::update_with_default)
                            {
                              pl = render->createPolyline(legend_symbol_x[0], legend_symbol_x[1], legend_symbol_y[0],
                                                          legend_symbol_y[1]);
                              pl->setAttribute("_child_id", child_id++);
                              element->append(pl);
                            }
                          else
                            {
                              pl = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
                              if (pl != nullptr)
                                render->createPolyline(legend_symbol_x[0], legend_symbol_x[1], legend_symbol_y[0],
                                                       legend_symbol_y[1], 0, 0.0, 0, pl);
                            }
                          if (pl != nullptr)
                            {
                              render->setLineSpec(pl, specs[spec_i]);
                              if (child->hasAttribute("line_color_ind"))
                                {
                                  render->setLineColorInd(pl, static_cast<int>(child->getAttribute("line_color_ind")));
                                }
                              else
                                {
                                  render->setLineColorInd(pl, static_cast<int>(series->getAttribute("line_color_ind")));
                                }
                            }
                        }
                      else if (child->localName() == "polymarker")
                        {
                          int markertype;
                          if (child->hasAttribute("marker_type"))
                            {
                              markertype = static_cast<int>(child->getAttribute("marker_type"));
                            }
                          else
                            {
                              markertype = static_cast<int>(series->getAttribute("marker_type"));
                            }
                          if (del != del_values::update_without_default && del != del_values::update_with_default)
                            {
                              pl = render->createPolymarker(legend_symbol_x[0] + 0.02 * scale_factor,
                                                            legend_symbol_y[0], markertype);
                              pl->setAttribute("_child_id", child_id++);
                              element->append(pl);
                            }
                          else
                            {
                              pl = element->querySelectors("polymarker[_child_id=" + std::to_string(child_id++) + "]");
                              if (pl != nullptr)
                                render->createPolymarker(legend_symbol_x[0] + 0.02 * scale_factor, legend_symbol_y[0],
                                                         markertype, 0.0, 0, pl);
                            }
                          if (pl != nullptr)
                            {
                              render->setMarkerColorInd(
                                  pl, (series->hasAttribute("marker_color_ind")
                                           ? static_cast<int>(series->getAttribute("marker_color_ind"))
                                           : 989));
                              processMarkerColorInd(pl);
                            }
                        }
                    }
                }
              if (mask & 2)
                {
                  legend_symbol_x[0] = viewport[0] + 0.02 * scale_factor;
                  legend_symbol_x[1] = viewport[0] + 0.06 * scale_factor;
                  legend_symbol_y[0] = viewport[3] - 0.03 * scale_factor;
                  legend_symbol_y[1] = viewport[3] - 0.03 * scale_factor;
                  for (const auto &child : series->children())
                    {
                      std::shared_ptr<GRM::Element> pl;
                      if (child->localName() == "polyline")
                        {
                          if (del != del_values::update_without_default && del != del_values::update_with_default)
                            {
                              pl = render->createPolyline(legend_symbol_x[0], legend_symbol_x[1], legend_symbol_y[0],
                                                          legend_symbol_y[1]);
                              pl->setAttribute("_child_id", child_id++);
                              element->append(pl);
                            }
                          else
                            {
                              pl = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
                              if (pl != nullptr)
                                render->createPolyline(legend_symbol_x[0], legend_symbol_x[1], legend_symbol_y[0],
                                                       legend_symbol_y[1], 0, 0.0, 0, pl);
                            }
                          if (pl != nullptr)
                            {
                              render->setLineSpec(pl, specs[spec_i]);
                              if (child->hasAttribute("line_color_ind"))
                                {
                                  render->setLineColorInd(pl, static_cast<int>(child->getAttribute("line_color_ind")));
                                }
                              else
                                {
                                  render->setLineColorInd(pl, static_cast<int>(series->getAttribute("line_color_ind")));
                                }
                            }
                        }
                      else if (child->localName() == "polymarker")
                        {
                          int markertype;
                          if (child->hasAttribute("marker_type"))
                            {
                              markertype = static_cast<int>(child->getAttribute("marker_type"));
                            }
                          else
                            {
                              markertype = static_cast<int>(series->getAttribute("marker_type"));
                            }
                          if (del != del_values::update_without_default && del != del_values::update_with_default)
                            {
                              pl = render->createPolymarker(legend_symbol_x[0] + 0.02 * scale_factor,
                                                            legend_symbol_y[0], markertype);
                              pl->setAttribute("_child_id", child_id++);
                              element->append(pl);
                            }
                          else
                            {
                              pl = element->querySelectors("polymarker[_child_id=" + std::to_string(child_id++) + "]");
                              if (pl != nullptr)
                                render->createPolymarker(legend_symbol_x[0] + 0.02 * scale_factor, legend_symbol_y[0],
                                                         markertype, 0.0, 0, pl);
                            }
                          if (pl != nullptr)
                            {
                              render->setMarkerColorInd(
                                  pl, (series->hasAttribute("marker_color_ind")
                                           ? static_cast<int>(series->getAttribute("marker_color_ind"))
                                           : 989));
                              processMarkerColorInd(pl);
                            }
                        }
                    }
                }
              if (i < labels.size())
                {
                  std::shared_ptr<GRM::Element> tx;
                  if (del != del_values::update_without_default && del != del_values::update_with_default)
                    {
                      tx = render->createText(viewport[0] + 0.08 * scale_factor, viewport[3] - 0.03 * scale_factor,
                                              labels[i].data());
                      tx->setAttribute("_child_id", child_id++);
                      element->append(tx);
                    }
                  else
                    {
                      tx = element->querySelectors("text[_child_id=" + std::to_string(child_id++) + "]");
                      if (tx != nullptr)
                        render->createText(viewport[0] + 0.08 * scale_factor, viewport[3] - 0.03 * scale_factor,
                                           labels[i].data(), CoordinateSpace::NDC, tx);
                    }
                  if (tx != nullptr && del != del_values::update_without_default)
                    {
                      render->setTextAlign(tx, GKS_K_TEXT_HALIGN_LEFT, GKS_K_TEXT_VALIGN_HALF);
                    }
                  viewport[3] -= 0.5 * dy;
                  i += 1;
                }
              viewport[3] -= 0.03 * scale_factor;
              spec_i += 1;
            }
        }
      gr_restorestate();

      processLineSpec(element);
    }
  else
    {
      std::shared_ptr<GRM::Element> fr, dr, text;
      int label_child_id = 0;
      double viewport[4];

      /* clear child nodes */
      del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
      clearOldChildren(&del, element);

      auto scale_factor = static_cast<double>(element->getAttribute("_scale_factor"));
      auto h = static_cast<double>(element->getAttribute("_start_h"));

      viewport[0] = static_cast<double>(element->getAttribute("viewport_x_min"));
      viewport[1] = static_cast<double>(element->getAttribute("viewport_x_max"));
      viewport[2] = static_cast<double>(element->getAttribute("viewport_y_min"));
      viewport[3] = static_cast<double>(element->getAttribute("viewport_y_max"));

      gr_selntran(1);

      render->setSelectSpecificXform(element, 0);
      render->setScale(element, 0);

      if (del != del_values::update_without_default && del != del_values::update_with_default)
        {
          fr = render->createFillRect(viewport[0], viewport[1], viewport[2], viewport[3]);
          fr->setAttribute("_child_id", child_id++);
          element->append(fr);
        }
      else
        {
          fr = element->querySelectors("fill_rect[_child_id=" + std::to_string(child_id++) + "]");
          if (fr != nullptr) render->createFillRect(viewport[0], viewport[1], viewport[2], viewport[3], 0, 0, -1, fr);
        }

      render->setFillIntStyle(element, GKS_K_INTSTYLE_SOLID);
      render->setFillColorInd(element, 0);

      if (del != del_values::update_without_default && del != del_values::update_with_default)
        {
          dr = render->createDrawRect(viewport[0], viewport[1], viewport[2], viewport[3]);
          dr->setAttribute("_child_id", child_id++);
          element->append(dr);
        }
      else
        {
          dr = element->querySelectors("draw_rect[_child_id=" + std::to_string(child_id++) + "]");
          if (dr != nullptr) render->createDrawRect(viewport[0], viewport[1], viewport[2], viewport[3], dr);
        }

      render->setLineType(element, GKS_K_INTSTYLE_SOLID);
      render->setLineColorInd(element, 1);
      render->setLineWidth(element, 1);

      auto labels_group = element->querySelectors("labels_group");
      if (labels_group == nullptr)
        {
          labels_group = render->createElement("labels_group");
          element->append(labels_group);
        }
      render->setTextAlign(labels_group, GKS_K_TEXT_HALIGN_LEFT, GKS_K_TEXT_VALIGN_HALF);

      for (auto &current_label : labels)
        {
          std::shared_ptr<GRM::Element> label_elem;
          if (del != del_values::update_without_default && del != del_values::update_with_default)
            {
              label_elem = render->createElement("label");
              label_elem->setAttribute("_child_id", label_child_id++);
              labels_group->append(label_elem);
            }
          else
            {
              label_elem = labels_group->querySelectors("label[_child_id=" + std::to_string(label_child_id++) + "]");
            }
          if (label_elem != nullptr)
            {
              if (del != del_values::update_without_default && del != del_values::update_with_default)
                {
                  fr = render->createFillRect(viewport[0] + 0.02 * scale_factor, viewport[0] + 0.04 * scale_factor,
                                              viewport[2] + (0.5 * h + 0.01) * scale_factor,
                                              viewport[2] + (0.5 * h + 0.03) * scale_factor);
                  fr->setAttribute("_child_id", 0);
                  label_elem->append(fr);
                }
              else
                {
                  fr = label_elem->querySelectors("fill_rect[_child_id=0]");
                  if (fr != nullptr)
                    render->createFillRect(viewport[0] + 0.02 * scale_factor, viewport[0] + 0.04 * scale_factor,
                                           viewport[2] + (0.5 * h + 0.01) * scale_factor,
                                           viewport[2] + (0.5 * h + 0.03) * scale_factor, 0, 0, -1, fr);
                }
              if (fr != nullptr)
                {
                  for (const auto &plot_child : element->parentElement()->children()) // central_region childs
                    {
                      if (plot_child->localName() != "central_region") continue;
                      for (const auto &child : plot_child->children())
                        {
                          if (child->localName() == "series_pie")
                            {
                              std::shared_ptr<GRM::Element> pie_segment;
                              pie_segment = child->querySelectors(
                                  "pie_segment[_child_id=" + std::to_string(label_child_id - 1) + "]");
                              if (pie_segment != nullptr)
                                {
                                  int color_ind = static_cast<int>(pie_segment->getAttribute("fill_color_ind"));
                                  auto color_rep = static_cast<std::string>(
                                      pie_segment->getAttribute("colorrep." + std::to_string(color_ind)));
                                  fr->setAttribute("fill_color_ind", color_ind);
                                  if (!color_rep.empty())
                                    fr->setAttribute("colorrep." + std::to_string(color_ind), color_rep);
                                }
                              break;
                            }
                        }
                    }
                }

              if (del != del_values::update_without_default && del != del_values::update_with_default)
                {
                  dr = render->createDrawRect(viewport[0] + 0.02 * scale_factor, viewport[0] + 0.04 * scale_factor,
                                              viewport[2] + (0.5 * h + 0.01) * scale_factor,
                                              viewport[2] + (0.5 * h + 0.03) * scale_factor);
                  dr->setAttribute("_child_id", 1);
                  label_elem->append(dr);
                }
              else
                {
                  dr = label_elem->querySelectors("draw_rect[_child_id=1]");
                  if (dr != nullptr)
                    render->createDrawRect(viewport[0] + 0.02 * scale_factor, viewport[0] + 0.04 * scale_factor,
                                           viewport[2] + (0.5 * h + 0.01) * scale_factor,
                                           viewport[2] + (0.5 * h + 0.03) * scale_factor, dr);
                }
              if (del != del_values::update_without_default && del != del_values::update_with_default)
                {
                  text = render->createText(viewport[0] + 0.05 * scale_factor,
                                            viewport[2] + (0.5 * h + 0.02) * scale_factor, current_label);
                  text->setAttribute("_child_id", 2);
                  label_elem->append(text);
                }
              else
                {
                  text = label_elem->querySelectors("text[_child_id=2]");
                  if (text != nullptr)
                    render->createText(viewport[0] + 0.05 * scale_factor, viewport[2] + (0.5 * h + 0.02) * scale_factor,
                                       current_label, CoordinateSpace::NDC, text);
                }

              gr_inqtext(0, 0, current_label.data(), tbx, tby);
              viewport[0] += tbx[2] - tbx[0] + 0.05 * scale_factor;
            }
        }

      processLineColorInd(element);
      processLineWidth(element);
      processLineType(element);
    }
  processSelectSpecificXform(element);
  GRM::Render::processScale(element);
  processFillIntStyle(element);
  processFillColorInd(element);
}

static void processPolarAxes(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  double viewport[4];
  double r_min, r_max;
  double min_scale = 0, max_scale; // used for y_log (with negative exponents)
  double tick;
  double x[2], y[2];
  int i, n;
  double alpha;
  char text_buffer[PLOT_POLAR_AXES_TEXT_BUFFER];
  std::string kind;
  int angle_ticks, rings;
  bool phiflip = false;
  double interval;
  std::string title, norm;
  del_values del = del_values::update_without_default;
  int child_id = 0;
  int y_log = 0;
  double y_lim_min, y_lim_max;

  std::shared_ptr<GRM::Render> render;
  std::shared_ptr<GRM::Element> central_region;

  render = std::dynamic_pointer_cast<GRM::Render>(element->ownerDocument());
  if (!render)
    {
      throw NotFoundError("No render-document for element found\n");
    }

  auto subplotElement = getSubplotElement(element);
  for (const auto &child : subplotElement->children()) // no special case for marginal_heatmap cause the plot is polar
    {
      if (child->localName() == "central_region")
        {
          central_region = child;
          break;
        }
    }

  viewport[0] = static_cast<double>(central_region->getAttribute("viewport_x_min"));
  viewport[1] = static_cast<double>(central_region->getAttribute("viewport_x_max"));
  viewport[2] = static_cast<double>(central_region->getAttribute("viewport_y_min"));
  viewport[3] = static_cast<double>(central_region->getAttribute("viewport_y_max"));

  if (subplotElement->hasAttribute("y_log"))
    {
      y_log = static_cast<int>(subplotElement->getAttribute("y_log"));
    }

  kind = static_cast<std::string>(subplotElement->getAttribute("kind"));

  // these kinds need r_min and r_max and y_lim_min and y_lim_max (if given)
  if (kind == "polar_heatmap" || kind == "nonuniform_polar_heatmap" || kind == "polar")
    {
      r_min = static_cast<double>(subplotElement->getAttribute("r_min"));
      if (kind == "uniform_polar_heatmap")
        {
          r_min = 0.0; // todo: uniform polar heatmap always starts at 0.0?
        }
      r_max = static_cast<double>(subplotElement->getAttribute("r_max"));
      if (subplotElement->hasAttribute("y_lim_max") && subplotElement->hasAttribute("y_lim_min"))
        {
          y_lim_min = static_cast<double>(subplotElement->getAttribute("y_lim_min"));
          y_lim_max = static_cast<double>(subplotElement->getAttribute("y_lim_max"));
        }
    }
  else
    {
      r_min = static_cast<double>(subplotElement->getAttribute("y_lim_min"));
      r_max = static_cast<double>(subplotElement->getAttribute("y_lim_max"));
    }

  // if y_log and y_lim_min == 0 --> set y_lim_min = 10^-1 per default todo: what if y_lim_max is 10^-x [x > 1]?
  if (y_log && y_lim_min == 0.0)
    {
      y_lim_min = pow(10, -1);
      subplotElement->setAttribute("y_lim_min", y_lim_min);
    }

  angle_ticks = static_cast<int>(element->getAttribute("angle_ticks"));

  render->setLineType(element, GKS_K_LINETYPE_SOLID);

  // for polar(no ylims) and polar_histogram (no ylims or if keep_radii_axes is given)
  if ((kind == "polar_histogram" &&
       (!subplotElement->hasAttribute("y_lim_max") || subplotElement->hasAttribute("keep_radii_axes"))) ||
      (kind == "polar" && !subplotElement->hasAttribute("y_lim_max")))
    {
      auto max = static_cast<double>(central_region->getAttribute("r_max"));
      rings = (element->hasAttribute("rings")) ? static_cast<int>(element->getAttribute("rings")) : -1;


      if (kind == "polar_histogram")
        {
          max = static_cast<double>(central_region->getAttribute("r_max"));
          norm = static_cast<std::string>(element->getAttribute("norm"));
          r_min = 0.0;
          tick = auto_tick_rings_polar(max, rings, norm, y_log);
        }
      else if (kind == "polar" && !y_log)
        {
          r_min = 0.0;
          auto seriesElement = central_region->querySelectors("series_polar");
          max = static_cast<double>(seriesElement->getAttribute("y_range_max"));
          tick = auto_tick_rings_polar(max, rings, "", y_log);
        }

      if (kind == "polar" && y_log)
        {
          // todo: what if r_max == 0.0; what error should be thrown?
          max_scale = ceil(abs(log10(r_max)));
          if (max_scale != 0.0) // add signum of max_scale if not 0.0
            {
              max_scale *= log10(r_max) / abs(log10(r_max));
            }

          if (r_min > 0.0)
            {
              min_scale = ceil(abs(log10(r_min)));
              if (min_scale != 0.0)
                {
                  min_scale *= (log10(r_min) / abs(log10(r_min)));
                }
            }
          else
            {
              if (max_scale <= 0) min_scale = max_scale - 5;
            }

          // todo: smart ring calculation for y_log especially with large differences in magnitudes
          rings = abs(abs(max_scale) - abs(min_scale));
          central_region->setAttribute("rings", rings);
          // overwrite r_max and r_min because of rounded scales?
          central_region->setAttribute("r_max", pow(10, max_scale));
          central_region->setAttribute("r_min", pow(10, min_scale));
        }
      else
        {
          central_region->setAttribute("tick", tick);
          max = tick * rings;
          central_region->setAttribute("r_max", max);
          central_region->setAttribute("rings", rings);
        }
    }
  else
    {
      // with given ylims
      if (y_log)
        {
          // todo: set y_lim_min to 10^x if y_lim_min == 0; x depends on the magnitude of y_lim_max...
          max_scale = ceil(abs(log10(y_lim_max)));
          if (max_scale != 0.0)
            {
              max_scale *= log10(y_lim_max) / abs(log10(y_lim_max));
            }

          if (y_lim_min <= 0.0)
            {
              if (r_min > 0.0)
                {
                  min_scale = ceil(abs(log10(r_min))) * (log10(r_min) / abs(log10(r_min)));
                }
              else
                {
                  min_scale = (max_scale <= 0) ? max_scale - 5 : 0;
                }
              y_lim_min = pow(10, min_scale);
              subplotElement->setAttribute("y_lim_min", y_lim_min);
            }
          else
            {
              min_scale = ceil(abs(log10(y_lim_min)));
              if (min_scale != 0.0)
                {
                  min_scale *= (log10(y_lim_min) / abs(log10(y_lim_min)));
                }
            }

          rings = grm_min(static_cast<int>(max_scale - min_scale), 12); // number of rings should not exceed 12?
          // todo what if number of rings is different than the magnitude difference?
          element->setAttribute("rings", rings);
        }
      else
        {
          rings = (element->hasAttribute("rings")) ? static_cast<int>(element->getAttribute("rings"))
                                                   : grm_max(4, (int)(r_max - r_min));
          // todo better rings calculation when ylim is given
          element->setAttribute("rings", rings);

          if (subplotElement->hasAttribute("y_lim_max") && subplotElement->hasAttribute("y_lim_min"))
            {
              tick = (r_max - r_min) / rings;
            }
          else if (element->hasAttribute("tick"))
            {
              tick = static_cast<double>(element->getAttribute("tick"));
            }
          else
            {
              tick = autoTickRingsPolar(r_max, rings, norm);
              subplotElement->setAttribute("r_max", tick * rings);
              subplotElement->setAttribute("rings", rings);
            }
        }
    }

  /* clear old polar_axes_elements */
  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  n = rings;
  phiflip = static_cast<int>(subplotElement->getAttribute("phi_flip"));

  // Draw rings
  for (i = 0; i <= n; i++)
    {
      std::shared_ptr<GRM::Element> arc;
      double r = 1.0 / n * i;
      if (del != del_values::update_without_default && del != del_values::update_with_default)
        {
          arc = render->createDrawArc(-r, r, -r, r, 0, 360);
          arc->setAttribute("_child_id", child_id++);
          element->append(arc);
        }
      else
        {
          arc = element->querySelectors("draw_arc[_child_id=" + std::to_string(child_id++) + "]");
          if (arc != nullptr) render->createDrawArc(-r, r, -r, r, 0, 360, arc);
        }
      if (arc != nullptr) arc->setAttribute("name", "polar_axes");
      if (i % 2 == 0)
        {
          if (i > 0)
            {
              if (arc != nullptr && del != del_values::update_without_default) render->setLineColorInd(arc, 88);
            }
        }
      else
        {
          if (arc != nullptr && del != del_values::update_without_default) render->setLineColorInd(arc, 90);
        }
    }

  // Draw sectorlines
  interval = 360.0 / angle_ticks;
  for (alpha = 0.0; alpha < 360; alpha += interval)
    {
      std::shared_ptr<GRM::Element> line, text;
      x[0] = std::cos(alpha * M_PI / 180.0);
      y[0] = std::sin(alpha * M_PI / 180.0);
      x[1] = 0.0;
      y[1] = 0.0;

      if (del != del_values::update_without_default && del != del_values::update_with_default)
        {
          line = render->createPolyline(x[0], x[1], y[0], y[1]);
          line->setAttribute("_child_id", child_id++);
          element->append(line);
        }
      else
        {
          line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
          if (line != nullptr) render->createPolyline(x[0], x[1], y[0], y[1], 0, 0.0, 0, line);
        }
      if (line != nullptr && del != del_values::update_without_default) render->setLineColorInd(line, 88);
      if (line != nullptr) line->setAttribute("name", "polar_axes");

      x[0] *= 1.1;
      y[0] *= 1.1;
      if (!phiflip)
        {
          snprintf(text_buffer, PLOT_POLAR_AXES_TEXT_BUFFER, "%d\xc2\xb0", (int)grm_round(alpha));
        }
      else
        {
          if (alpha == 0.0)
            snprintf(text_buffer, PLOT_POLAR_AXES_TEXT_BUFFER, "%d\xc2\xb0", 0);
          else
            snprintf(text_buffer, PLOT_POLAR_AXES_TEXT_BUFFER, "%d\xc2\xb0", 330 - (int)grm_round(alpha - interval));
        }
      if (del != del_values::update_without_default && del != del_values::update_with_default)
        {
          text = render->createText(x[0], y[0], text_buffer, CoordinateSpace::WC);
          text->setAttribute("_child_id", child_id++);
          element->append(text);
        }
      else
        {
          text = element->querySelectors("text[_child_id=" + std::to_string(child_id++) + "]");
          if (text != nullptr) render->createText(x[0], y[0], text_buffer, CoordinateSpace::WC, text);
        }
      if (text != nullptr) text->setAttribute("name", "polar_axes");
      if (text != nullptr && del != del_values::update_without_default)
        render->setTextAlign(text, GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_HALF);
    }

  // Draw Text
  render->setTextAlign(element, GKS_K_TEXT_HALIGN_LEFT, GKS_K_TEXT_VALIGN_HALF);
  std::shared_ptr<GRM::Element> axes_text_group = element->querySelectors("axes_text_group");
  if (axes_text_group == nullptr)
    {
      axes_text_group = render->createElement("axes_text_group");
      element->append(axes_text_group);
    }

  for (i = 0; i <= n; i++)
    {
      std::shared_ptr<GRM::Element> text;
      double r = 1.0 / n * i;
      if (i % 2 == 0 || i == n)
        {
          x[0] = 0.05;
          y[0] = r;
          if (y_log) // ylog uses the exponential notation
            {
              // todo: if magnitude difference != rings -> i * n + ...
              if (subplotElement->hasAttribute("y_lim_max")) // todo:ylog with ylims
                {
                  snprintf(text_buffer, PLOT_POLAR_AXES_TEXT_BUFFER, "%.1e", pow(10, min_scale + i));
                }
              else // no ylims
                {
                  snprintf(text_buffer, PLOT_POLAR_AXES_TEXT_BUFFER, "%.1e", pow(10, min_scale + i));
                }
            }
          else // no y_log
            {
              snprintf(text_buffer, PLOT_POLAR_AXES_TEXT_BUFFER, "%.1lf", r_min + tick * i);
            }

          if (del != del_values::update_without_default && del != del_values::update_with_default)
            {
              text = render->createText(x[0], y[0], text_buffer, CoordinateSpace::WC);
              text->setAttribute("_child_id", child_id++);
              axes_text_group->append(text);
            }
          else
            {
              text = element->querySelectors("text[_child_id=" + std::to_string(child_id++) + "]");
              if (text != nullptr) render->createText(x[0], y[0], text_buffer, CoordinateSpace::WC, text);
            }
        }
    }
  if (element->hasAttribute("char_height"))
    {
      processCharHeight(element);
    }
  processLineType(element);
  processTextAlign(element);
}

static void processDrawRect(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for draw_arc
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  auto x_min = static_cast<double>(element->getAttribute("x_min"));
  auto x_max = static_cast<double>(element->getAttribute("x_max"));
  auto y_min = static_cast<double>(element->getAttribute("y_min"));
  auto y_max = static_cast<double>(element->getAttribute("y_max"));
  applyMoveTransformation(element);
  if (redraw_ws) gr_drawrect(x_min, x_max, y_min, y_max);
}

static void processFillArc(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for draw_arc
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  auto x_min = static_cast<double>(element->getAttribute("x_min"));
  auto x_max = static_cast<double>(element->getAttribute("x_max"));
  auto y_min = static_cast<double>(element->getAttribute("y_min"));
  auto y_max = static_cast<double>(element->getAttribute("y_max"));
  auto start_angle = static_cast<double>(element->getAttribute("start_angle"));
  auto end_angle = static_cast<double>(element->getAttribute("end_angle"));
  applyMoveTransformation(element);
  if (redraw_ws) gr_fillarc(x_min, x_max, y_min, y_max, start_angle, end_angle);
}

static void processFillRect(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for draw_arc
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  auto x_min = static_cast<double>(element->getAttribute("x_min"));
  auto x_max = static_cast<double>(element->getAttribute("x_max"));
  auto y_min = static_cast<double>(element->getAttribute("y_min"));
  auto y_max = static_cast<double>(element->getAttribute("y_max"));
  applyMoveTransformation(element);
  if (redraw_ws) gr_fillrect(x_min, x_max, y_min, y_max);
}

static void processFillArea(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for fillArea
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));

  std::vector<double> x_vec = GRM::get<std::vector<double>>((*context)[x]);
  std::vector<double> y_vec = GRM::get<std::vector<double>>((*context)[y]);

  int n = std::min<int>((int)x_vec.size(), (int)y_vec.size());
  applyMoveTransformation(element);

  if (redraw_ws) gr_fillarea(n, (double *)&(x_vec[0]), (double *)&(y_vec[0]));
}

static void processGrid3d(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for grid 3d
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double x_tick, y_tick, z_tick;
  double x_org, y_org, z_org;
  int x_major, y_major, z_major;
  std::string x_org_pos = PLOT_DEFAULT_ORG_POS, y_org_pos = PLOT_DEFAULT_ORG_POS, z_org_pos = PLOT_DEFAULT_ORG_POS;
  if (element->hasAttribute("x_org_pos")) x_org_pos = static_cast<std::string>(element->getAttribute("x_org_pos"));
  if (element->hasAttribute("y_org_pos")) y_org_pos = static_cast<std::string>(element->getAttribute("y_org_pos"));
  if (element->hasAttribute("z_org_pos")) z_org_pos = static_cast<std::string>(element->getAttribute("z_org_pos"));

  getAxes3dInformation(element, x_org_pos, y_org_pos, z_org_pos, x_org, y_org, z_org, x_major, y_major, z_major, x_tick,
                       y_tick, z_tick);
  applyMoveTransformation(element);

  if (redraw_ws) gr_grid3d(x_tick, y_tick, z_tick, x_org, y_org, z_org, abs(x_major), abs(y_major), abs(z_major));
}

static void processGridLine(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  std::shared_ptr<GRM::Element> axis_elem = element->parentElement()->parentElement();
  std::shared_ptr<GRM::Element> plot_parent = element;
  getPlotParent(plot_parent);

  auto coordinate_system = plot_parent->querySelectors("coordinate_system");
  bool hide =
      (coordinate_system->hasAttribute("hide")) ? static_cast<int>(coordinate_system->getAttribute("hide")) : false;
  auto coordinate_system_type = static_cast<std::string>(coordinate_system->getAttribute("plot_type"));
  auto axis_type = static_cast<std::string>(axis_elem->getAttribute("axis_type"));
  auto min_val = static_cast<double>(axis_elem->getAttribute("min_value"));
  auto max_val = static_cast<double>(axis_elem->getAttribute("max_value"));
  auto org = static_cast<double>(axis_elem->getAttribute("org"));
  auto pos = static_cast<double>(axis_elem->getAttribute("pos"));
  auto tick = static_cast<double>(axis_elem->getAttribute("tick"));
  auto major_count = static_cast<int>(axis_elem->getAttribute("major_count"));
  auto value = static_cast<double>(element->getAttribute("value"));
  auto is_major = static_cast<int>(element->getAttribute("is_major"));

  tick_t g = {value, is_major};
  axis_t grid = {min_val, max_val, tick, org, pos, major_count, 1, &g, 0.0, 0, nullptr, NAN, false};
  if (redraw_ws && !hide && (coordinate_system_type == "2d" || axis_elem->parentElement()->localName() == "colorbar"))
    {
      if (axis_type == "x")
        {
          gr_drawaxes(&grid, nullptr, 4);
        }
      else
        {
          gr_drawaxes(nullptr, &grid, 4);
        }
    }
}

static void processHeatmap(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for heatmap
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */

  int icmap[256], i, j;
  unsigned int cols, rows, z_length;
  double x_min, x_max, y_min, y_max, z_min, z_max, c_min, c_max, zv;
  bool is_uniform_heatmap;
  std::shared_ptr<GRM::Element> plot_parent;
  std::shared_ptr<GRM::Element> element_context = element;
  std::vector<int> data, rgba;
  std::vector<double> x_vec, y_vec, z_vec;
  del_values del = del_values::update_without_default;
  int child_id = 0;
  int x_offset = 0, y_offset = 0;

  if (element->parentElement()->parentElement()->localName() == "plot")
    {
      plot_parent = element->parentElement()->parentElement();
    }
  else
    {
      plot_parent = element->parentElement();
      getPlotParent(plot_parent);
      for (const auto &children : plot_parent->children())
        {
          if (children->localName() == "marginal_heatmap_plot")
            {
              element_context = children;
              break;
            }
        }
    }

  if (element_context->hasAttribute("x"))
    {
      auto x = static_cast<std::string>(element_context->getAttribute("x"));
      x_vec = GRM::get<std::vector<double>>((*context)[x]);
      cols = x_vec.size();

      if (static_cast<int>(plot_parent->getAttribute("x_log")))
        {
          for (i = 0; i < cols; i++)
            {
              if (grm_isnan(x_vec[i]))
                {
                  x_vec.erase(x_vec.begin(), x_vec.begin() + 1);
                  x_offset += 1;
                }
              else
                {
                  break;
                }
            }
          cols = x_vec.size();
        }
    }
  if (element_context->hasAttribute("y"))
    {
      auto y = static_cast<std::string>(element_context->getAttribute("y"));
      y_vec = GRM::get<std::vector<double>>((*context)[y]);
      rows = y_vec.size();

      if (static_cast<int>(plot_parent->getAttribute("y_log")))
        {
          for (i = 0; i < rows; i++)
            {
              if (grm_isnan(y_vec[i]))
                {
                  y_vec.erase(y_vec.begin(), y_vec.begin() + 1);
                  y_offset += 1;
                }
              else
                {
                  break;
                }
            }
          rows = y_vec.size();
        }
    }

  if (!element_context->hasAttribute("z"))
    throw NotFoundError("Heatmap series is missing required attribute z-data.\n");
  auto z = static_cast<std::string>(element_context->getAttribute("z"));
  z_vec = GRM::get<std::vector<double>>((*context)[z]);
  z_length = z_vec.size();

  if (x_offset > 0 || y_offset > 0)
    {
      std::vector<double> new_z_vec(rows * cols);
      z_length = rows * cols;

      for (i = 0; i < rows; i++)
        {
          for (j = 0; j < cols; j++)
            {
              new_z_vec[j + i * cols] = z_vec[(x_offset + j) + (y_offset + i) * (cols + x_offset)];
            }
        }
      z_vec = new_z_vec;
    }

  if (x_vec.empty() && y_vec.empty())
    {
      /* If neither `x` nor `y` are given, we need more information about the shape of `z` */
      if (!element->hasAttribute("z_dims"))
        throw NotFoundError("Heatmap series is missing required attribute z_dims.\n");
      auto z_dims_key = static_cast<std::string>(element->getAttribute("z_dims"));
      auto z_dims_vec = GRM::get<std::vector<int>>((*context)[z_dims_key]);
      cols = z_dims_vec[0];
      rows = z_dims_vec[1];
    }
  else if (x_vec.empty())
    {
      cols = z_length / rows;
    }
  else if (y_vec.empty())
    {
      rows = z_length / cols;
    }

  is_uniform_heatmap = (x_vec.empty() || is_equidistant_array(cols, &(x_vec[0]))) &&
                       (y_vec.empty() || is_equidistant_array(rows, &(y_vec[0])));
  if (!is_uniform_heatmap && (x_vec.empty() || y_vec.empty()))
    throw NotFoundError("Heatmap series is missing x- or y-data or the data has to be uniform.\n");

  auto id = static_cast<int>(global_root->getAttribute("_id"));
  global_root->setAttribute("_id", id + 1);
  auto str = std::to_string(id);
  if (x_vec.empty())
    {
      std::vector<double> x_vec_tmp;
      x_min = static_cast<double>(element->getAttribute("x_range_min"));
      x_max = static_cast<double>(element->getAttribute("x_range_max"));
      linspace(x_min, x_max, (int)cols, x_vec_tmp);
      (*context)["x" + str] = x_vec_tmp;
      element->setAttribute("x", "x" + str);
    }
  else
    {
      for (j = 0; j < cols; j++)
        {
          if (!grm_isnan(x_vec[j]))
            {
              x_min = x_vec[j];
              break;
            }
        }
      x_max = x_vec[cols - 1];
    }
  if (y_vec.empty())
    {
      std::vector<double> y_vec_tmp;
      y_min = static_cast<double>(element->getAttribute("y_range_min"));
      y_max = static_cast<double>(element->getAttribute("y_range_max"));
      linspace(y_min, y_max, (int)rows, y_vec_tmp);
      (*context)["y" + str] = y_vec_tmp;
      element->setAttribute("y", "y" + str);
    }
  else
    {
      for (j = 0; j < rows; j++)
        {
          if (!grm_isnan(y_vec[j]))
            {
              y_min = y_vec[j];
              break;
            }
        }
      y_max = y_vec[rows - 1];
    }
  if (element_context->hasAttribute("marginal_heatmap_kind"))
    {
      z_min = static_cast<double>(element_context->getAttribute("z_range_min"));
      z_max = static_cast<double>(element_context->getAttribute("z_range_max"));
    }
  else
    {
      z_min = static_cast<double>(element->getAttribute("z_range_min"));
      z_max = static_cast<double>(element->getAttribute("z_range_max"));
    }
  if (!element->hasAttribute("c_range_min") || !element->hasAttribute("c_range_max"))
    {
      c_min = z_min;
      c_max = z_max;
    }
  else
    {
      c_min = static_cast<double>(element->getAttribute("c_range_min"));
      c_max = static_cast<double>(element->getAttribute("c_range_max"));
    }

  if (!is_uniform_heatmap)
    {
      --cols;
      --rows;
    }
  for (i = 0; i < 256; i++)
    {
      gr_inqcolor(1000 + i, icmap + i);
    }

  data = std::vector<int>(rows * cols);
  if (z_max > z_min)
    {
      for (i = 0; i < cols * rows; i++)
        {
          zv = z_vec[i];

          if (zv > z_max || zv < z_min || grm_isnan(zv))
            {
              data[i] = -1;
            }
          else
            {
              data[i] = (int)((zv - c_min) / (c_max - c_min) * 255 + 0.5);
              if (data[i] >= 255)
                {
                  data[i] = 255;
                }
              else if (data[i] < 0)
                {
                  data[i] = 0;
                }
            }
        }
    }
  else
    {
      for (i = 0; i < cols * rows; i++)
        {
          data[i] = 0;
        }
    }
  rgba = std::vector<int>(rows * cols);

  /* clear old heatmaps */
  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  if (is_uniform_heatmap)
    {
      for (i = 0; i < rows * cols; i++)
        {
          if (data[i] == -1)
            {
              rgba[i] = 0;
            }
          else
            {
              rgba[i] = (255 << 24) + icmap[data[i]];
            }
        }

      std::shared_ptr<GRM::Element> cellarray;
      if (del != del_values::update_without_default && del != del_values::update_with_default)
        {
          cellarray =
              global_render->createDrawImage(x_min, y_min, x_max, y_max, (int)cols, (int)rows, "rgba" + str, rgba, 0);
          cellarray->setAttribute("_child_id", child_id++);
          element->append(cellarray);
        }
      else
        {
          cellarray = element->querySelectors("draw_image[_child_id=" + std::to_string(child_id++) + "]");
          if (cellarray != nullptr)
            global_render->createDrawImage(x_min, y_min, x_max, y_max, (int)cols, (int)rows, "rgba" + str, rgba, 0,
                                           nullptr, cellarray);
        }
    }
  else
    {
      for (i = 0; i < rows * cols; i++)
        {
          if (data[i] == -1)
            {
              rgba[i] = 1256 + 1; /* Invalid color index -> gr_nonuniformcellarray draws a transparent rectangle */
            }
          else
            {
              rgba[i] = data[i] + 1000;
            }
        }

      std::shared_ptr<GRM::Element> cellarray;
      if (del != del_values::update_without_default && del != del_values::update_with_default)
        {
          cellarray =
              global_render->createNonUniformCellArray("x" + str, x_vec, "y" + str, y_vec, (int)cols, (int)rows, 1, 1,
                                                       (int)cols, (int)rows, "color_ind_values" + str, rgba);
          cellarray->setAttribute("_child_id", child_id++);
          element->append(cellarray);
        }
      else
        {
          cellarray = element->querySelectors("nonuniformcellarray[_child_id=" + std::to_string(child_id++) + "]");
          if (cellarray != nullptr)
            global_render->createNonUniformCellArray("x" + str, x_vec, "y" + str, y_vec, (int)cols, (int)rows, 1, 1,
                                                     (int)cols, (int)rows, "color_ind_values" + str, rgba, nullptr,
                                                     cellarray);
        }
    }
}

static void hexbin(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for hexbin
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));
  auto nbins = static_cast<int>(element->getAttribute("num_bins"));

  double *x_p = &(GRM::get<std::vector<double>>((*context)[x])[0]);
  double *y_p = &(GRM::get<std::vector<double>>((*context)[y])[0]);

  std::vector<double> x_vec = GRM::get<std::vector<double>>((*context)[x]);
  std::vector<double> y_vec = GRM::get<std::vector<double>>((*context)[y]);
  auto x_length = (int)x_vec.size();

  if (element->hasAttribute("_hexbin_context_address"))
    {
      auto address = static_cast<std::string>(element->getAttribute("_hexbin_context_address"));
      long hex_address = stol(address, nullptr, 16);
      const hexbin_2pass_t *hexbinContext = (hexbin_2pass_t *)hex_address;
      bool cleanup = hexbinContext->action & GR_2PASS_CLEANUP;
      if (redraw_ws) gr_hexbin_2pass(x_length, x_p, y_p, nbins, hexbinContext);
      if (cleanup)
        {
          element->removeAttribute("_hexbin_context_address");
        }
    }
  else
    {
      applyMoveTransformation(element);
      if (redraw_ws) gr_hexbin(x_length, x_p, y_p, nbins);
    }
}

static void processHexbin(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  int nbins = PLOT_DEFAULT_HEXBIN_NBINS;

  if (!element->hasAttribute("x")) throw NotFoundError("Hexbin series is missing required attribute x-data.\n");
  auto x = static_cast<std::string>(element->getAttribute("x"));
  if (!element->hasAttribute("y")) throw NotFoundError("Hexbin series is missing required attribute y-data.\n");
  auto y = static_cast<std::string>(element->getAttribute("y"));
  if (element->hasAttribute("num_bins"))
    {
      nbins = static_cast<int>(element->getAttribute("num_bins"));
    }
  else
    {
      element->setAttribute("num_bins", nbins);
    }

  double *x_p = &(GRM::get<std::vector<double>>((*context)[x])[0]);
  double *y_p = &(GRM::get<std::vector<double>>((*context)[y])[0]);

  std::vector<double> x_vec = GRM::get<std::vector<double>>((*context)[x]);
  std::vector<double> y_vec = GRM::get<std::vector<double>>((*context)[y]);
  auto x_length = (int)x_vec.size();
  auto y_length = (int)y_vec.size();
  if (x_length != y_length) throw std::length_error("For Hexbin x- and y-data must have the same size\n.");

  const hexbin_2pass_t *hexbinContext = gr_hexbin_2pass(x_length, x_p, y_p, nbins, nullptr);
  double c_min = 0.0, c_max = hexbinContext->cntmax;
  std::ostringstream get_address;
  get_address << hexbinContext;
  element->setAttribute("_hexbin_context_address", get_address.str());

  auto plot_parent = element->parentElement();
  getPlotParent(plot_parent);
  plot_parent->setAttribute("_c_lim_min", c_min);
  plot_parent->setAttribute("_c_lim_max", c_max);
  if (redraw_ws)
    {
      PushDrawableToZQueue pushHexbinToZQueue(hexbin);
      pushHexbinToZQueue(element, context);
    }
}

static void histBins(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  double *tmp_bins;
  std::vector<double> x, weights;
  unsigned int num_bins = 0, num_weights;

  if (!element->hasAttribute("x")) throw NotFoundError("Hist series is missing required attribute x-data.\n");
  auto key = static_cast<std::string>(element->getAttribute("x"));
  x = GRM::get<std::vector<double>>((*context)[key]);
  auto current_point_count = (int)x.size();

  if (element->hasAttribute("num_bins")) num_bins = static_cast<int>(element->getAttribute("num_bins"));
  if (element->hasAttribute("weights"))
    {
      auto weights_key = static_cast<std::string>(element->getAttribute("weights"));
      weights = GRM::get<std::vector<double>>((*context)[weights_key]);
      num_weights = weights.size();
    }
  if (!weights.empty() && current_point_count != num_weights)
    throw std::length_error("For hist series the size of data and weights must be the same.\n");
  if (num_bins <= 1)
    {
      num_bins = (int)(3.3 * log10(current_point_count) + 0.5) + 1;
    }
  auto bins = std::vector<double>(num_bins);
  double *x_p = &(x[0]);
  double *weights_p = (weights.empty()) ? nullptr : &(weights[0]);
  tmp_bins = &(bins[0]);
  bin_data(current_point_count, x_p, num_bins, tmp_bins, weights_p);
  std::vector<double> tmp(tmp_bins, tmp_bins + num_bins);

  auto id = static_cast<int>(global_root->getAttribute("_id"));
  auto str = std::to_string(id);
  (*context)["bins" + str] = tmp;
  element->setAttribute("bins", "bins" + str);
  global_root->setAttribute("_id", ++id);
}

static void processHist(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for hist
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */

  int bar_color_index = 989, i;
  std::vector<double> bar_color_rgb_vec = {-1, -1, -1};
  std::shared_ptr<GRM::Element> plot_parent;
  del_values del = del_values::update_without_default;
  int child_id = 0;
  int edge_color_index = 1;
  std::vector<double> edge_color_rgb_vec = {-1, -1, -1};
  double x_min, x_max, bar_width, y_min, y_max;
  std::vector<double> bins_vec;
  unsigned int num_bins;
  std::string orientation = PLOT_DEFAULT_ORIENTATION;
  bool is_horizontal;

  if (element->hasAttribute("fill_color_rgb"))
    {
      auto bar_color_rgb = static_cast<std::string>(element->getAttribute("fill_color_rgb"));
      bar_color_rgb_vec = GRM::get<std::vector<double>>((*context)[bar_color_rgb]);
    }

  if (element->hasAttribute("fill_color_ind"))
    bar_color_index = static_cast<int>(element->getAttribute("fill_color_ind"));

  plot_parent = element->parentElement();
  getPlotParent(plot_parent);

  if (bar_color_rgb_vec[0] != -1)
    {
      for (i = 0; i < 3; i++)
        {
          if (bar_color_rgb_vec[i] > 1 || bar_color_rgb_vec[i] < 0)
            throw std::out_of_range("For hist series bar_color_rgb must be inside [0, 1].\n");
        }
      bar_color_index = 1000;
      global_render->setColorRep(element, bar_color_index, bar_color_rgb_vec[0], bar_color_rgb_vec[1],
                                 bar_color_rgb_vec[2]);
      // processColorRep has to be manually triggered.
      processColorReps(element);
    }

  if (element->hasAttribute("line_color_rgb"))
    {
      auto edge_color_rgb = static_cast<std::string>(element->getAttribute("line_color_rgb"));
      edge_color_rgb_vec = GRM::get<std::vector<double>>((*context)[edge_color_rgb]);
    }

  if (element->hasAttribute("line_color_ind"))
    edge_color_index = static_cast<int>(element->getAttribute("line_color_ind"));
  if (edge_color_rgb_vec[0] != -1)
    {
      for (i = 0; i < 3; i++)
        {
          if (edge_color_rgb_vec[i] > 1 || edge_color_rgb_vec[i] < 0)
            throw std::out_of_range("For hist series edge_color_rgb must be inside [0, 1].\n");
        }
      edge_color_index = 1001;
      global_render->setColorRep(element, edge_color_index, edge_color_rgb_vec[0], edge_color_rgb_vec[1],
                                 edge_color_rgb_vec[2]);
      processColorReps(element);
    }

  if (!element->hasAttribute("bins")) histBins(element, context);
  auto bins = static_cast<std::string>(element->getAttribute("bins"));
  bins_vec = GRM::get<std::vector<double>>((*context)[bins]);
  num_bins = bins_vec.size();

  if (element->hasAttribute("orientation"))
    {
      orientation = static_cast<std::string>(element->getAttribute("orientation"));
    }
  else
    {
      element->setAttribute("orientation", orientation);
    }
  is_horizontal = orientation == "horizontal";

  x_min = static_cast<double>(element->getAttribute("x_range_min"));
  x_max = static_cast<double>(element->getAttribute("x_range_max"));
  y_min = static_cast<double>(element->getAttribute("y_range_min"));
  y_max = static_cast<double>(element->getAttribute("y_range_max"));
  if (std::isnan(y_min)) y_min = 0.0;

  if (element->parentElement()->parentElement()->hasAttribute("marginal_heatmap_side_plot"))
    {
      std::shared_ptr<GRM::Element> marginal_heatmap;
      for (const auto &children : plot_parent->children())
        {
          if (children->localName() == "marginal_heatmap_plot")
            {
              marginal_heatmap = children;
              break;
            }
        }
      if (marginal_heatmap->hasAttribute("x_range_min"))
        x_min = static_cast<double>(marginal_heatmap->getAttribute("x_range_min"));
      if (marginal_heatmap->hasAttribute("x_range_max"))
        x_max = static_cast<double>(marginal_heatmap->getAttribute("x_range_max"));
      if (marginal_heatmap->hasAttribute("y_range_min"))
        y_min = static_cast<double>(marginal_heatmap->getAttribute("y_range_min"));
      if (marginal_heatmap->hasAttribute("y_range_max"))
        y_max = static_cast<double>(marginal_heatmap->getAttribute("y_range_max"));
      processMarginalHeatmapSidePlot(element->parentElement());
      processMarginalHeatmapKind(marginal_heatmap);

      if (orientation == "vertical")
        {
          double tmp_min = x_min, tmp_max = x_max;

          x_min = y_min;
          x_max = y_max;
          y_min = tmp_min;
          y_max = tmp_max;
        }
      y_min = 0.0;
    }

  bar_width = (x_max - x_min) / num_bins;

  /* clear old bars */
  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  for (i = 1; i < num_bins + 1; ++i)
    {
      double x = x_min + (i - 1) * bar_width;
      std::shared_ptr<GRM::Element> bar;

      if (is_horizontal)
        {
          if (del != del_values::update_without_default && del != del_values::update_with_default)
            {
              bar =
                  global_render->createBar(x, x + bar_width, y_min, bins_vec[i - 1], bar_color_index, edge_color_index);
              bar->setAttribute("_child_id", child_id++);
              element->append(bar);
            }
          else
            {
              bar = element->querySelectors("bar[_child_id=" + std::to_string(child_id++) + "]");
              if (bar != nullptr)
                global_render->createBar(x, x + bar_width, y_min, bins_vec[i - 1], bar_color_index, edge_color_index,
                                         "", "", -1, "", bar);
            }
        }
      else
        {
          if (del != del_values::update_without_default && del != del_values::update_with_default)
            {
              bar =
                  global_render->createBar(y_min, bins_vec[i - 1], x, x + bar_width, bar_color_index, edge_color_index);
              bar->setAttribute("_child_id", child_id++);
              element->append(bar);
            }
          else
            {
              bar = element->querySelectors("bar[_child_id=" + std::to_string(child_id++) + "]");
              if (bar != nullptr)
                global_render->createBar(y_min, bins_vec[i - 1], x, x + bar_width, bar_color_index, edge_color_index,
                                         "", "", -1, "", bar);
            }
        }
    }

  // error_bar handling
  for (const auto &child : element->children())
    {
      if (child->localName() == "error_bars")
        {
          std::vector<double> bar_centers(num_bins);
          linspace(x_min + 0.5 * bar_width, x_max - 0.5 * bar_width, (int)num_bins, bar_centers);
          extendErrorBars(child, context, bar_centers, bins_vec);
        }
    }
}

static void processBar(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  bool is_horizontal;
  double x1, x2, y1, y2;
  int bar_color_index, edge_color_index, text_color_index = -1, color_save_spot = PLOT_CUSTOM_COLOR_INDEX;
  std::shared_ptr<GRM::Element> fill_rect, draw_rect, text_elem;
  std::string orientation = PLOT_DEFAULT_ORIENTATION, text;
  del_values del = del_values::update_without_default;
  double line_width = NAN, y_lightness = NAN;
  std::vector<double> bar_color_rgb, edge_color_rgb;
  int child_id = 0;

  x1 = static_cast<double>(element->getAttribute("x1"));
  x2 = static_cast<double>(element->getAttribute("x2"));
  y1 = static_cast<double>(element->getAttribute("y1"));
  y2 = static_cast<double>(element->getAttribute("y2"));
  bar_color_index = static_cast<int>(element->getAttribute("fill_color_ind"));
  edge_color_index = static_cast<int>(element->getAttribute("line_color_ind"));
  if (element->hasAttribute("text")) text = static_cast<std::string>(element->getAttribute("text"));
  if (element->hasAttribute("line_width")) line_width = static_cast<double>(element->getAttribute("line_width"));

  if (element->hasAttribute("fill_color_rgb"))
    {
      auto bar_color_rgb_key = static_cast<std::string>(element->getAttribute("fill_color_rgb"));
      bar_color_rgb = GRM::get<std::vector<double>>((*context)[bar_color_rgb_key]);
    }
  if (element->hasAttribute("line_color_rgb"))
    {
      auto edge_color_rgb_key = static_cast<std::string>(element->getAttribute("line_color_rgb"));
      edge_color_rgb = GRM::get<std::vector<double>>((*context)[edge_color_rgb_key]);
    }

  /* clear old rects */
  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  if (del != del_values::update_without_default && del != del_values::update_with_default)
    {
      fill_rect = global_render->createFillRect(x1, x2, y1, y2);
      fill_rect->setAttribute("_child_id", child_id++);
      element->append(fill_rect);
    }
  else
    {
      fill_rect = element->querySelectors("fill_rect[_child_id=" + std::to_string(child_id++) + "]");
      if (fill_rect != nullptr) global_render->createFillRect(x1, x2, y1, y2, 0, 0, -1, fill_rect);
    }
  if (fill_rect != nullptr)
    {
      global_render->setFillIntStyle(fill_rect, GKS_K_INTSTYLE_SOLID);

      if (!bar_color_rgb.empty() && bar_color_rgb[0] != -1)
        {
          global_render->setColorRep(fill_rect, color_save_spot, bar_color_rgb[0], bar_color_rgb[1], bar_color_rgb[2]);
          bar_color_index = color_save_spot;
          processColorReps(fill_rect);
        }
      global_render->setFillColorInd(fill_rect, bar_color_index);

      if (!text.empty())
        {
          int color;
          if (bar_color_index != -1)
            {
              color = bar_color_index;
            }
          else if (element->hasAttribute("fill_color_ind"))
            {
              color = (int)element->getAttribute("fill_color_ind");
            }
          y_lightness = getLightness(color);
        }
    }

  if (del != del_values::update_without_default && del != del_values::update_with_default)
    {
      draw_rect = global_render->createDrawRect(x1, x2, y1, y2);
      draw_rect->setAttribute("_child_id", child_id++);
      element->append(draw_rect);
    }
  else
    {
      draw_rect = element->querySelectors("draw_rect[_child_id=" + std::to_string(child_id++) + "]");
      if (draw_rect != nullptr) global_render->createDrawRect(x1, x2, y1, y2, draw_rect);
    }
  if (draw_rect != nullptr)
    {
      draw_rect->setAttribute("z_index", 2);

      if (!edge_color_rgb.empty() && edge_color_rgb[0] != -1)
        {
          global_render->setColorRep(draw_rect, color_save_spot - 1, edge_color_rgb[0], edge_color_rgb[1],
                                     edge_color_rgb[2]);
          edge_color_index = color_save_spot - 1;
        }
      if (element->parentElement()->localName() == "series_barplot")
        element->parentElement()->setAttribute("line_color_ind", edge_color_index);
      global_render->setLineColorInd(draw_rect, edge_color_index);
      processLineColorInd(draw_rect);
      if (!std::isnan(line_width)) global_render->setLineWidth(draw_rect, line_width);
    }

  if (!text.empty())
    {
      if (del != del_values::update_without_default && del != del_values::update_with_default)
        {
          text_elem = global_render->createText((x1 + x2) / 2, (y1 + y2) / 2, text, CoordinateSpace::WC);
          text_elem->setAttribute("_child_id", child_id++);
          element->append(text_elem);
        }
      else
        {
          text_elem = element->querySelectors("text[_child_id=" + std::to_string(child_id++) + "]");
          if (text_elem != nullptr)
            global_render->createText((x1 + x2) / 2, (y1 + y2) / 2, text, CoordinateSpace::WC, text_elem);
        }
      if (text_elem != nullptr)
        {
          text_elem->setAttribute("z_index", 2);
          global_render->setTextAlign(text_elem, 2, 3);
          global_render->setTextWidthAndHeight(text_elem, x2 - x1, y2 - y1);
          if (!std::isnan(y_lightness)) global_render->setTextColorInd(text_elem, (y_lightness < 0.4) ? 0 : 1);
        }
    }
}

static void processIsosurfaceRender(const std::shared_ptr<GRM::Element> &element,
                                    const std::shared_ptr<GRM::Context> &context)
{
  double viewport[4];
  double x_min, x_max, y_min, y_max;
  int fig_width, fig_height;
  int subplot_width, subplot_height;
  int drawable_type;

  drawable_type = static_cast<int>(element->getAttribute("drawable_type"));

  gr_inqviewport(&viewport[0], &viewport[1], &viewport[2], &viewport[3]);

  x_min = viewport[0];
  x_max = viewport[1];
  y_min = viewport[2];
  y_max = viewport[3];

  GRM::Render::getFigureSize(&fig_width, &fig_height, nullptr, nullptr);
  subplot_width = (int)(grm_max(fig_width, fig_height) * (x_max - x_min));
  subplot_height = (int)(grm_max(fig_width, fig_height) * (y_max - y_min));

  logger((stderr, "viewport: (%lf, %lf, %lf, %lf)\n", x_min, x_max, y_min, y_max));
  logger((stderr, "viewport ratio: %lf\n", (x_min - x_max) / (y_min - y_max)));
  logger((stderr, "subplot size: (%d, %d)\n", subplot_width, subplot_height));
  logger((stderr, "subplot ratio: %lf\n", ((double)subplot_width / (double)subplot_height)));

  gr3_drawimage((float)x_min, (float)x_max, (float)y_min, (float)y_max, subplot_width, subplot_height,
                GR3_DRAWABLE_GKS);
}

static void processLayoutGrid(const std::shared_ptr<GRM::Element> &element,
                              const std::shared_ptr<GRM::Context> &context)
{
  double plot_x_min, plot_x_max, plot_y_min, plot_y_max;
  plot_x_min = static_cast<double>(element->getAttribute("plot_x_min"));
  plot_x_max = static_cast<double>(element->getAttribute("plot_x_max"));
  plot_y_min = static_cast<double>(element->getAttribute("plot_y_min"));
  plot_y_max = static_cast<double>(element->getAttribute("plot_y_max"));

  applyMoveTransformation(element);
  gr_setviewport(plot_x_min, plot_x_max, plot_y_min, plot_y_max);
}

static void processNonUniformPolarCellArray(const std::shared_ptr<GRM::Element> &element,
                                            const std::shared_ptr<GRM::Context> &context)
{
  auto x_org = static_cast<double>(element->getAttribute("x_org"));
  auto y_org = static_cast<double>(element->getAttribute("y_org"));
  auto phi_key = static_cast<std::string>(element->getAttribute("phi"));
  auto r_key = static_cast<std::string>(element->getAttribute("r"));
  auto dimr = static_cast<int>(element->getAttribute("r_dim"));
  auto dimphi = static_cast<int>(element->getAttribute("phi_dim"));
  auto scol = static_cast<int>(element->getAttribute("start_col"));
  auto srow = static_cast<int>(element->getAttribute("start_row"));
  auto ncol = static_cast<int>(element->getAttribute("num_col"));
  auto nrow = static_cast<int>(element->getAttribute("num_row"));
  auto color_key = static_cast<std::string>(element->getAttribute("color_ind_values"));

  auto r_vec = GRM::get<std::vector<double>>((*context)[r_key]);
  auto phi_vec = GRM::get<std::vector<double>>((*context)[phi_key]);
  auto color_vec = GRM::get<std::vector<int>>((*context)[color_key]);

  double *phi = &(phi_vec[0]);
  double *r = &(r_vec[0]);
  int *color = &(color_vec[0]);
  applyMoveTransformation(element);

  if (redraw_ws) gr_nonuniformpolarcellarray(x_org, y_org, phi, r, dimphi, dimr, scol, srow, ncol, nrow, color);
}

static void processNonuniformcellarray(const std::shared_ptr<GRM::Element> &element,
                                       const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for nonuniformcellarray
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));

  auto dimx = static_cast<int>(element->getAttribute("x_dim"));
  auto dimy = static_cast<int>(element->getAttribute("y_dim"));
  auto scol = static_cast<int>(element->getAttribute("start_col"));
  auto srow = static_cast<int>(element->getAttribute("start_row"));
  auto ncol = static_cast<int>(element->getAttribute("num_col"));
  auto nrow = static_cast<int>(element->getAttribute("num_row"));
  auto color = static_cast<std::string>(element->getAttribute("color_ind_values"));

  auto x_p = (double *)&(GRM::get<std::vector<double>>((*context)[x])[0]);
  auto y_p = (double *)&(GRM::get<std::vector<double>>((*context)[y])[0]);

  auto color_p = (int *)&(GRM::get<std::vector<int>>((*context)[color])[0]);
  applyMoveTransformation(element);
  if (redraw_ws) gr_nonuniformcellarray(x_p, y_p, dimx, dimy, scol, srow, ncol, nrow, color_p);
}

static void processPanzoom(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  ; /* panzoom is being processed in the processLimits routine */
}

static void processPolarCellArray(const std::shared_ptr<GRM::Element> &element,
                                  const std::shared_ptr<GRM::Context> &context)
{
  auto x_org = static_cast<double>(element->getAttribute("x_org"));
  auto y_org = static_cast<double>(element->getAttribute("y_org"));
  auto phimin = static_cast<double>(element->getAttribute("phi_min"));
  auto phimax = static_cast<double>(element->getAttribute("phi_max"));
  auto rmin = static_cast<double>(element->getAttribute("r_min"));
  auto rmax = static_cast<double>(element->getAttribute("r_max"));
  auto dimr = static_cast<int>(element->getAttribute("r_dim"));
  auto dimphi = static_cast<int>(element->getAttribute("phi_dim"));
  auto scol = static_cast<int>(element->getAttribute("start_col"));
  auto srow = static_cast<int>(element->getAttribute("start_row"));
  auto ncol = static_cast<int>(element->getAttribute("num_col"));
  auto nrow = static_cast<int>(element->getAttribute("num_row"));
  auto color_key = static_cast<std::string>(element->getAttribute("color_ind_values"));

  auto color_vec = GRM::get<std::vector<int>>((*context)[color_key]);
  int *color = &(color_vec[0]);
  applyMoveTransformation(element);

  if (redraw_ws)
    gr_polarcellarray(x_org, y_org, phimin, phimax, rmin, rmax, dimphi, dimr, scol, srow, ncol, nrow, color);
}

static void processPolyline(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing Function for polyline
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  applyMoveTransformation(element);

  auto name = static_cast<std::string>(element->getAttribute("name"));

  if (starts_with(name, "x-axis-line") || starts_with(name, "y-axis-line")) gr_setclip(0);
  if (element->getAttribute("x").isString() && element->getAttribute("y").isString())
    {
      auto x = static_cast<std::string>(element->getAttribute("x"));
      auto y = static_cast<std::string>(element->getAttribute("y"));

      std::vector<double> x_vec = GRM::get<std::vector<double>>((*context)[x]);
      std::vector<double> y_vec = GRM::get<std::vector<double>>((*context)[y]);

      auto n = std::min<int>((int)x_vec.size(), (int)y_vec.size());
      auto group = element->parentElement();
      if ((element->hasAttribute("line_types") || element->hasAttribute("line_widths") ||
           element->hasAttribute("line_color_indices")) ||
          ((parent_types.count(group->localName())) &&
           (group->hasAttribute("line_types") || group->hasAttribute("line_widths") ||
            group->hasAttribute("line_color_indices"))))
        {
          lineHelper(element, context, "polyline");
        }
      else if (redraw_ws)
        gr_polyline(n, (double *)&(x_vec[0]), (double *)&(y_vec[0]));
    }
  else if (element->getAttribute("x1").isDouble() && element->getAttribute("x2").isDouble() &&
           element->getAttribute("y1").isDouble() && element->getAttribute("y2").isDouble())
    {
      auto x1 = static_cast<double>(element->getAttribute("x1"));
      auto x2 = static_cast<double>(element->getAttribute("x2"));
      auto y1 = static_cast<double>(element->getAttribute("y1"));
      auto y2 = static_cast<double>(element->getAttribute("y2"));
      double x[2] = {x1, x2};
      double y[2] = {y1, y2};

      if (redraw_ws) gr_polyline(2, x, y);
    }
  if (starts_with(name, "x-axis-line") || starts_with(name, "y-axis-line")) gr_setclip(1);
}

static void processPolyline3d(const std::shared_ptr<GRM::Element> &element,
                              const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for polyline 3d
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));
  auto z = static_cast<std::string>(element->getAttribute("z"));

  std::vector<double> x_vec = GRM::get<std::vector<double>>((*context)[x]);
  std::vector<double> y_vec = GRM::get<std::vector<double>>((*context)[y]);
  std::vector<double> z_vec = GRM::get<std::vector<double>>((*context)[z]);

  double *x_p = &(x_vec[0]);
  double *y_p = &(y_vec[0]);
  double *z_p = &(z_vec[0]);
  auto group = element->parentElement();

  applyMoveTransformation(element);
  if ((element->hasAttribute("line_types") || element->hasAttribute("line_widths") ||
       element->hasAttribute("line_color_indices")) ||
      ((parent_types.count(group->localName())) &&
       (group->hasAttribute("line_types") || group->hasAttribute("line_widths") ||
        group->hasAttribute("line_color_indices"))))
    {
      lineHelper(element, context, "polyline_3d");
    }
  else
    {
      if (redraw_ws) gr_polyline3d((int)x_vec.size(), x_p, y_p, z_p);
    }
}

static void processPolymarker(const std::shared_ptr<GRM::Element> &element,
                              const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for polymarker
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  applyMoveTransformation(element);
  if (element->getAttribute("x").isString() && element->getAttribute("y").isString())
    {
      auto x = static_cast<std::string>(element->getAttribute("x"));
      auto y = static_cast<std::string>(element->getAttribute("y"));

      std::vector<double> x_vec = GRM::get<std::vector<double>>((*context)[x]);
      std::vector<double> y_vec = GRM::get<std::vector<double>>((*context)[y]);

      auto n = std::min<int>((int)x_vec.size(), (int)y_vec.size());
      auto group = element->parentElement();
      if ((element->hasAttribute("marker_types") || element->hasAttribute("marker_sizes") ||
           element->hasAttribute("marker_color_indices")) ||
          (parent_types.count(group->localName()) &&
           (group->hasAttribute("marker_types") || group->hasAttribute("marker_sizes") ||
            group->hasAttribute("marker_color_indices"))))
        {
          markerHelper(element, context, "polymarker");
        }
      else
        {
          if (redraw_ws) gr_polymarker(n, (double *)&(x_vec[0]), (double *)&(y_vec[0]));
        }
    }
  else if (element->getAttribute("x").isDouble() && element->getAttribute("y").isDouble())
    {
      auto x = static_cast<double>(element->getAttribute("x"));
      auto y = static_cast<double>(element->getAttribute("y"));
      if (redraw_ws) gr_polymarker(1, &x, &y);
    }
}

static void processPolymarker3d(const std::shared_ptr<GRM::Element> &element,
                                const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for polymarker 3d
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));
  auto z = static_cast<std::string>(element->getAttribute("z"));

  std::vector<double> x_vec = GRM::get<std::vector<double>>((*context)[x]);
  std::vector<double> y_vec = GRM::get<std::vector<double>>((*context)[y]);
  std::vector<double> z_vec = GRM::get<std::vector<double>>((*context)[z]);

  double *x_p = &(x_vec[0]);
  double *y_p = &(y_vec[0]);
  double *z_p = &(z_vec[0]);

  auto group = element->parentElement();
  applyMoveTransformation(element);
  if ((element->hasAttribute("marker_types") || element->hasAttribute("marker_sizes") ||
       element->hasAttribute("marker_color_indices")) ||
      (parent_types.count(group->localName()) &&
       (group->hasAttribute("marker_types") || group->hasAttribute("marker_sizes") ||
        group->hasAttribute("marker_color_indices"))))
    {
      markerHelper(element, context, "polymarker_3d");
    }
  else
    {
      if (redraw_ws) gr_polymarker3d((int)x_vec.size(), x_p, y_p, z_p);
    }
}

static void processQuiver(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for quiver
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  if (!element->hasAttribute("x")) throw NotFoundError("Quiver series is missing required attribute x-data.\n");
  auto x = static_cast<std::string>(element->getAttribute("x"));
  if (!element->hasAttribute("y")) throw NotFoundError("Quiver series is missing required attribute y-data.\n");
  auto y = static_cast<std::string>(element->getAttribute("y"));
  if (!element->hasAttribute("u")) throw NotFoundError("Quiver series is missing required attribute u-data.\n");
  auto u = static_cast<std::string>(element->getAttribute("u"));
  if (!element->hasAttribute("v")) throw NotFoundError("Quiver series is missing required attribute v-data.\n");
  auto v = static_cast<std::string>(element->getAttribute("v"));
  auto color = static_cast<int>(element->getAttribute("color_ind"));

  std::vector<double> x_vec = GRM::get<std::vector<double>>((*context)[x]);
  std::vector<double> y_vec = GRM::get<std::vector<double>>((*context)[y]);
  std::vector<double> u_vec = GRM::get<std::vector<double>>((*context)[u]);
  std::vector<double> v_vec = GRM::get<std::vector<double>>((*context)[v]);
  auto x_length = (int)x_vec.size();
  auto y_length = (int)y_vec.size();
  auto u_length = (int)u_vec.size();
  auto v_length = (int)v_vec.size();

  if (x_length * y_length != u_length)
    throw std::length_error("For quiver series x_length * y_length must be u_length.\n");
  if (x_length * y_length != v_length)
    throw std::length_error("For quiver series x_length * y_length must be v_length.\n");

  double *x_p = &(x_vec[0]);
  double *y_p = &(y_vec[0]);
  double *u_p = &(GRM::get<std::vector<double>>((*context)[u])[0]);
  double *v_p = &(GRM::get<std::vector<double>>((*context)[v])[0]);
  applyMoveTransformation(element);

  if (redraw_ws) gr_quiver(x_length, y_length, x_p, y_p, u_p, v_p, color);
}

static void processPolar(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for polar
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double r_min, r_max, tick;
  double y_lim_min, y_lim_max, yrange_min, yrange_max, xrange_min, xrange_max;
  double theta_min, theta_max;

  int n;
  bool transform_radii = false, transform_angles = false, ylim = false, clip_negative = false;
  unsigned int rho_length, theta_length;
  std::string line_spec = SERIES_DEFAULT_SPEC;
  unsigned int i, index;
  std::vector<double> theta_vec, rho_vec;
  auto plot_parent = element->parentElement();
  del_values del = del_values::update_without_default;
  int child_id = 0;
  int y_log = 0;
  std::vector<unsigned int> indices_vec;


  getPlotParent(plot_parent);
  if (plot_parent->hasAttribute("y_lim_max") && plot_parent->hasAttribute("y_lim_min"))
    {
      y_lim_min = static_cast<double>(plot_parent->getAttribute("y_lim_min"));
      y_lim_max = static_cast<double>(plot_parent->getAttribute("y_lim_max"));
      ylim = true;
    }
  if (element->hasAttribute("y_range_min") && element->hasAttribute("y_range_max"))
    {
      transform_radii = true;
      yrange_min = static_cast<double>(element->getAttribute("y_range_min"));
      yrange_max = static_cast<double>(element->getAttribute("y_range_max"));
    }
  else
    {
      r_max = static_cast<double>(plot_parent->getAttribute("r_max"));
      r_min = 0.0;
    }
  if (plot_parent->hasAttribute("y_log"))
    {
      y_log = static_cast<int>(plot_parent->getAttribute("y_log"));
    }

  // xranges are sometimes given, calculated by the min and max of the angles x
  if (element->hasAttribute("x_range_min") && element->hasAttribute("x_range_max"))
    {
      transform_angles = true;
      xrange_min = static_cast<double>(element->getAttribute("x_range_min"));
      xrange_max = static_cast<double>(element->getAttribute("x_range_max"));
      if (xrange_max > 2 * M_PI)
        {
          // convert from degrees to radians
          xrange_max = xrange_max * M_PI / 180.0;
          xrange_min = xrange_min * M_PI / 180.0;
        }
    }

  if (element->hasAttribute("line_spec"))
    {
      line_spec = static_cast<std::string>(element->getAttribute("line_spec"));
    }
  else
    {
      element->setAttribute("line_spec", line_spec);
    }

  if (element->hasAttribute("clip_negative"))
    {
      clip_negative = static_cast<int>(element->getAttribute("clip_negative"));
    }

  if (!element->hasAttribute("x")) throw NotFoundError("Polar series is missing required attribute x-data (theta).\n");
  auto x_key = static_cast<std::string>(element->getAttribute("x"));
  if (!element->hasAttribute("y")) throw NotFoundError("Polar series is missing required attribute y-data (rho).\n");
  auto y_key = static_cast<std::string>(element->getAttribute("y"));
  theta_vec = GRM::get<std::vector<double>>((*context)[x_key]);
  rho_vec = GRM::get<std::vector<double>>((*context)[y_key]);
  theta_length = theta_vec.size();
  rho_length = rho_vec.size();

  // negative radii are clipped before the transformation into user specified yrange (also when y_log is given)
  if (clip_negative || y_log)
    {
      for (unsigned int ind = 0; ind < theta_vec.size(); ++ind)
        {
          if (rho_vec[ind] < 0) indices_vec.insert(indices_vec.begin(), ind);
        }

      for (auto ind : indices_vec)
        {
          rho_vec.erase(rho_vec.begin() + ind);
          theta_vec.erase(theta_vec.begin() + ind);
        }
      indices_vec.clear();
    }

  r_min = *std::min_element(rho_vec.begin(), rho_vec.end());
  r_max = *std::max_element(rho_vec.begin(), rho_vec.end());

  theta_min = *std::min_element(theta_vec.begin(), theta_vec.end());
  theta_max = *std::max_element(theta_vec.begin(), theta_vec.end());

  if (theta_min == xrange_min && theta_max == xrange_max) transform_angles = false;

  if (r_min == yrange_min && r_max == yrange_max) transform_radii = false;
  if (!ylim)
    {
      y_lim_max = r_max;
      y_lim_min = r_min;
    }

  if (rho_length != theta_length)
    throw std::length_error("For polar series y(rho)- and x(theta)-data must have the same size.\n");

  std::vector<double> x(rho_length), std::vector<double> y(rho_length);

  // transform angles into user specified xranges if given
  if (transform_angles)
    {
      transformCoordinatesVector(theta_vec, theta_min, theta_max, xrange_min, xrange_max);
    }

  index = 0;
  // transform radii into yrange if given or log scale
  if (transform_radii || y_log)
    {
      for (i = 0; i < rho_length; ++i)
        {
          double current_rho;
          double temp_rho = rho_vec[i];
          // skip nan data
          if (std::isnan(rho_vec[i]))
            {
              continue;
            }

          if (ylim)
            {
              // todo: skipping here causes problems for y_range with y_lim. Maybe this is not needed?
              //              if (temp_rho < 0.0) continue;
            }

          if (y_log && !transform_radii)
            {
              current_rho = transformCoordinate(temp_rho, y_lim_min, y_lim_max, 0.0, 0.0, y_log);
            }
          else
            {
              // todo: yrange transformation with y_log? Maybe transform the vector before
              current_rho = transformCoordinate(temp_rho, r_min, r_max, yrange_min, yrange_max, y_log);
            }

          if (ylim && y_lim_min > 0.0 && current_rho < 0.0)
            {
              current_rho = 0.0;
            }
          else if (ylim && temp_rho > y_lim_max) // todo: polar clipping for > ylim_max is wip by Josef
            {
              // this is just some temporary fix for the clipping
              continue;
            }

          current_rho /= y_lim_max;
          x[index] = current_rho * cos(theta_vec[index]);
          y[index] = current_rho * sin(theta_vec[index]);
          ++index;
        }
    }
  else // no radii_transformation and no y_log
    {
      for (i = 0; i < rho_length; ++i)
        {
          if (rho_vec[i] < 0)
            {
              // iterate over rho_vec and for each negative value add 180 degrees in radian to the corresponding value
              // in theta_vec and make the rho_vec value positive
              theta_vec[i] += M_PI;
              // if theta_vec[i] is bigger than 2 * PI, subtract 2 * PI todo: what if its bigger than 4 * PI?
              if (theta_vec[i] > 2 * M_PI)
                {
                  theta_vec[i] -= 2 * M_PI;
                }
              rho_vec[i] = -rho_vec[i];
            }

          // subtract ylim_min from rho_vec if ylim is true
          double current_rho;
          if (ylim)
            {
              current_rho = (rho_vec[i] - y_lim_min) / y_lim_max;
              // just set negative radii to 0.0 if ylim_min > 0.0
              if (current_rho < 0.0 && y_lim_min > 0.0) current_rho = 0.0;
            }
          else
            {
              current_rho = rho_vec[i] / y_lim_max;
            }
          x[index] = current_rho * cos(theta_vec[index]);
          y[index] = current_rho * sin(theta_vec[index]);
          ++index;
        }
    }

  // resize x and y
  x.resize(index);
  y.resize(index);


  auto id = static_cast<int>(global_root->getAttribute("_id"));
  global_root->setAttribute("_id", id + 1);

  /* clear old polylines */
  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  std::shared_ptr<GRM::Element> line;
  if (del != del_values::update_without_default && del != del_values::update_with_default)
    {
      line = global_render->createPolyline("x" + std::to_string(id), x, "y" + std::to_string(id), y);
      line->setAttribute("_child_id", child_id++);
      element->append(line);
    }
  else
    {
      line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
      if (line != nullptr)
        global_render->createPolyline("x" + std::to_string(id), x, "y" + std::to_string(id), y, nullptr, 0, 0.0, 0,
                                      line);
    }
}

static void processPolarHeatmap(const std::shared_ptr<GRM::Element> &element,
                                const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for polar_heatmap
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */

  std::string kind;
  int icmap[256];
  unsigned int i, cols, rows, z_length;
  double x_min, x_max, y_min, y_max, z_min, z_max, c_min, c_max, zv;
  double xrange_min, xrange_max, yrange_min, yrange_max, zrange_min, zrange_max;
  bool is_uniform_heatmap;
  std::vector<int> data;
  std::vector<double> x_vec, y_vec, z_vec;
  del_values del = del_values::update_without_default;
  int child_id = 0;
  bool zrange = false, transform = false;
  double convert = 1.0;
  std::shared_ptr<GRM::Element> central_region;
  auto plot_parent = element->parentElement();
  getPlotParent(plot_parent);

  for (const auto &child : plot_parent->children())
    {
      if (child->localName() == "central_region")
        {
          central_region = child;
          break;
        }
    }

  kind = static_cast<std::string>(plot_parent->getAttribute("kind"));

  if (element->hasAttribute("x"))
    {
      auto x = static_cast<std::string>(element->getAttribute("x"));
      x_vec = GRM::get<std::vector<double>>((*context)[x]);
      cols = x_vec.size();
    }
  if (element->hasAttribute("y"))
    {
      auto y = static_cast<std::string>(element->getAttribute("y"));
      y_vec = GRM::get<std::vector<double>>((*context)[y]);
      rows = y_vec.size();
    }

  if (element->hasAttribute("x_range_min") && element->hasAttribute("x_range_max"))
    {
      transform = true;
      xrange_min = static_cast<double>(element->getAttribute("x_range_min"));
      xrange_max = static_cast<double>(element->getAttribute("x_range_max"));
      if (xrange_max <= 2 * M_PI) convert = 180.0 / M_PI;
    }
  if (element->hasAttribute("y_range_min") && element->hasAttribute("y_range_max"))
    {
      transform = true;
      yrange_min = static_cast<double>(element->getAttribute("y_range_min"));
      yrange_max = static_cast<double>(element->getAttribute("y_range_max"));
    }
  if (element->hasAttribute("zrange_min") && element->hasAttribute("zrange_max"))
    {
      zrange_min = static_cast<double>(element->getAttribute("zrange_min"));
      zrange = true;
      zrange_max = static_cast<double>(element->getAttribute("zrange_max"));
    }

  if (!element->hasAttribute("z")) throw NotFoundError("Polar-heatmap series is missing required attribute z-data.\n");
  auto z = static_cast<std::string>(element->getAttribute("z"));
  z_vec = GRM::get<std::vector<double>>((*context)[z]);
  z_length = z_vec.size();

  if (x_vec.empty() && y_vec.empty())
    {
      /* If neither `x` nor `y` are given, we need more information about the shape of `z` */
      if (!element->hasAttribute("z_dims"))
        throw NotFoundError("Polar-heatmap series is missing required attribute z_dims.\n");
      auto z_dims_key = static_cast<std::string>(element->getAttribute("z_dims"));
      auto z_dims_vec = GRM::get<std::vector<int>>((*context)[z_dims_key]);
      cols = z_dims_vec[0]; // todo: is this switched?
      rows = z_dims_vec[1];
    }
  else if (x_vec.empty())
    {
      cols = z_length / rows;
    }
  else if (y_vec.empty())
    {
      rows = z_length / cols;
    }

  is_uniform_heatmap = is_equidistant_array(cols, x_vec.data()) && is_equidistant_array(rows, y_vec.data());
  if (kind == "nonuniform_polar_heatmap") is_uniform_heatmap = false;

  if (!is_uniform_heatmap && (x_vec.empty() || y_vec.empty()))
    throw NotFoundError("Polar-heatmap series is missing x- or y-data or the data has to be uniform.\n");

  if (x_vec.empty())
    {
      x_min = static_cast<double>(element->getAttribute("x_range_min"));
      x_max = static_cast<double>(element->getAttribute("x_range_max"));
    }
  else
    {
      x_min = x_vec[0];
      x_max = x_vec[cols - 1];
    }
  if (y_vec.empty())
    {
      y_min = static_cast<double>(element->getAttribute("y_range_min"));
      y_max = static_cast<double>(element->getAttribute("y_range_max"));
    }
  else
    {
      y_min = y_vec[0];
      y_max = y_vec[rows - 1];
    }
  if (y_min > 0.0) is_uniform_heatmap = 0; /* when y range min > 0 for example */

  // Check if coordinate transformations are needed and then transform if needed
  if (transform && ((!x_vec.empty() && (x_vec[0] < xrange_min || x_vec[-1] > xrange_max)) ||
                    (!y_vec.empty() && (y_vec[0] < yrange_min || y_vec[-1] > yrange_max))))
    {
      is_uniform_heatmap = 0;
      int col, row;

      if (x_vec.empty())
        {
          x_vec.resize(cols);
          for (col = 0; col < cols; ++col)
            {
              x_vec[col] = transformCoordinate(col / (cols - 1.0) * 360.0, 0.0, 360.0, xrange_min * convert,
                                               xrange_max * convert);
            }
        }
      else
        {
          transformCoordinatesVector(x_vec, x_min, x_max, xrange_min * convert, xrange_max * convert);
        }
      if (y_vec.empty())
        {
          y_vec.resize(rows);
          for (row = 0; row < rows; ++row)
            {
              y_vec[row] = transformCoordinate(row / (rows - 1.0), 0.0, 1.0, yrange_min, yrange_max);
            }
        }
      else
        {
          transformCoordinatesVector(y_vec, y_min, y_max, yrange_min, yrange_max);
        }
    }

  if (zrange)
    {
      int elem;
      double min_val = *std::min_element(z_vec.begin(), z_vec.end());
      double max_val = *std::max_element(z_vec.begin(), z_vec.end());

      for (elem = 0; elem < rows * cols; ++elem)
        {
          z_vec[elem] = zrange_min + (zrange_max - zrange_min) * (z_vec[elem] - min_val) / (max_val - min_val);
        }
    }

  z_min = static_cast<double>(element->getAttribute("z_range_min"));
  z_max = static_cast<double>(element->getAttribute("z_range_max"));
  if (!element->hasAttribute("c_range_min") || !element->hasAttribute("c_range_max"))
    {
      c_min = z_min;
      c_max = z_max;
    }
  else
    {
      c_min = static_cast<double>(element->getAttribute("c_range_min"));
      c_max = static_cast<double>(element->getAttribute("c_range_max"));
    }

  for (i = 0; i < 256; i++)
    {
      gr_inqcolor(1000 + i, icmap + i);
    }

  data = std::vector<int>(rows * cols);
  if (z_max > z_min)
    {
      for (i = 0; i < cols * rows; i++)
        {
          zv = z_vec[i];

          if (zv > z_max || zv < z_min || grm_isnan(zv))
            {
              data[i] = -1;
            }
          else
            {
              data[i] = 1000 + (int)(255.0 * (zv - c_min) / (c_max - c_min) + 0.5);
              if (data[i] >= 1255)
                {
                  data[i] = 1255;
                }
              else if (data[i] < 1000)
                {
                  data[i] = 1000;
                }
            }
        }
    }
  else
    {
      for (i = 0; i < cols * rows; i++)
        {
          data[i] = 0;
        }
    }

  auto id = static_cast<int>(global_root->getAttribute("_id"));
  global_root->setAttribute("_id", id + 1);
  auto str = std::to_string(id);

  /* clear old polar_heatmaps */
  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  std::shared_ptr<GRM::Element> polar_cellarray;
  if (is_uniform_heatmap)
    {
      if (del != del_values::update_without_default && del != del_values::update_with_default)
        {
          polar_cellarray = global_render->createPolarCellArray(0, 0, 0, 360, 0, 1, (int)cols, (int)rows, 1, 1,
                                                                (int)cols, (int)rows, "color_ind_values" + str, data);
          polar_cellarray->setAttribute("_child_id", child_id++);
          element->append(polar_cellarray);
        }
      else
        {
          polar_cellarray = element->querySelectors("polarcellarray[_child_id=" + std::to_string(child_id++) + "]");
          if (polar_cellarray != nullptr)
            global_render->createPolarCellArray(0, 0, 0, 360, 0, 1, (int)cols, (int)rows, 1, 1, (int)cols, (int)rows,
                                                "color_ind_values" + str, data, nullptr, polar_cellarray);
        }
    }
  else
    {
      y_min = static_cast<double>(central_region->getAttribute("window_y_min"));
      y_max = static_cast<double>(central_region->getAttribute("window_y_max"));

      std::vector<double> rho(rows), phi(cols);
      if (x_vec[cols - 1] <= 2 * M_PI)
        {
          convert = 180.0 / M_PI;
        }
      for (i = 0; i < ((cols > rows) ? cols : rows); ++i)
        {
          if (i < cols)
            {
              phi[i] = (x_vec[i]) * convert;
            }
          if (i < rows)
            {
              rho[i] = (y_vec[i] / y_max);
            }
        }

      if (del != del_values::update_without_default && del != del_values::update_with_default)
        {
          polar_cellarray = global_render->createNonUniformPolarCellArray(0, 0, "phi" + str, phi, "rho" + str, rho,
                                                                          (int)-cols, (int)-rows, 1, 1, (int)cols,
                                                                          (int)rows, "color_ind_values" + str, data);
          polar_cellarray->setAttribute("_child_id", child_id++);
          element->append(polar_cellarray);
        }
      else
        {
          polar_cellarray =
              element->querySelectors("nonuniform_polarcellarray[_child_id=" + std::to_string(child_id++) + "]");
          if (polar_cellarray != nullptr)
            global_render->createNonUniformPolarCellArray(0, 0, "phi" + str, phi, "rho" + str, rho, (int)-cols,
                                                          (int)-rows, 1, 1, (int)cols, (int)rows,
                                                          "color_ind_values" + str, data, nullptr, polar_cellarray);
        }
    }
}

static void preBarplot(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  std::vector<int> indices_vec;
  int max_y_length = 0;
  for (const auto &series : element->querySelectorsAll("series_barplot"))
    {
      if (!series->hasAttribute("indices"))
        {
          if (!series->hasAttribute("y")) throw NotFoundError("Barplot series is missing indices\n");
          auto y_key = static_cast<std::string>(series->getAttribute("y"));
          auto y_vec = GRM::get<std::vector<double>>((*context)[y_key]);
          indices_vec = std::vector<int>(y_vec.size(), 1);
          auto id = static_cast<int>(global_root->getAttribute("_id"));
          auto id_str = std::to_string(id);

          (*context)["indices" + id_str] = indices_vec;
          series->setAttribute("indices", "indices" + id_str);
          global_root->setAttribute("_id", ++id);
        }
      else
        {
          auto indices_key = static_cast<std::string>(series->getAttribute("indices"));
          indices_vec = GRM::get<std::vector<int>>((*context)[indices_key]);
        }
      auto cur_y_length = (int)indices_vec.size();
      max_y_length = grm_max(cur_y_length, max_y_length);
    }
  element->setAttribute("max_y_length", max_y_length);
}

static void prePolarHistogram(const std::shared_ptr<GRM::Element> &element,
                              const std::shared_ptr<GRM::Context> &context)
{
  unsigned int num_bins, length, num_bin_edges, dummy;
  std::vector<double> theta;
  std::string norm = "count";
  std::vector<int> classes, bin_counts;
  double interval, start, max, temp_max, bin_width, xrange_min, xrange_max;
  double *p, *phi_lim = nullptr;
  int max_observations = 0, total_observations = 0;
  std::vector<double> bin_edges, bin_widths;
  bool is_bin_counts = false;
  double phi_lim_arr[2];
  std::vector<double> new_theta, new_edges;

  // element is the plot element -> get the first series with polar_histogram
  auto series_list = element->querySelectorsAll("series_polar_histogram");
  std::shared_ptr<GRM::Element> group = series_list[0];
  // element = plot_group for better readability
  const std::shared_ptr<GRM::Element> &plot_group = element;

  // define keys for later usages
  auto id = static_cast<int>(global_root->getAttribute("_id"));
  auto str = std::to_string(id);
  global_root->setAttribute("_id", id + 1);
  std::string bin_widths_key = "bin_widths" + str, bin_edges_key = "bin_edges" + str, classes_key = "classes" + str;

  if (group->hasAttribute("bin_counts"))
    {
      is_bin_counts = true;
      auto bin_counts_key = static_cast<std::string>(group->getAttribute("bin_counts"));
      bin_counts = GRM::get<std::vector<int>>((*context)[bin_counts_key]);

      length = bin_counts.size();
      num_bins = length;
      group->setAttribute("num_bins", static_cast<int>(num_bins));
    }
  else if (group->hasAttribute("theta"))
    {
      auto theta_key = static_cast<std::string>(group->getAttribute("theta"));
      theta = GRM::get<std::vector<double>>((*context)[theta_key]);
      length = theta.size();

      if (group->hasAttribute("x_range_min") && group->hasAttribute("x_range_max"))
        {
          xrange_min = static_cast<double>(group->getAttribute("x_range_min"));
          xrange_max = static_cast<double>(group->getAttribute("x_range_max"));
          // convert xrange_min and max to radian if xrange_max > 2 * M_PI
          if (xrange_max > 2 * M_PI)
            {
              xrange_min = xrange_min / 180.0 * M_PI;
              xrange_max = xrange_max / 180.0 * M_PI;
            }
          if (xrange_min > xrange_max)
            {
              std::swap(xrange_min, xrange_max);
              group->setAttribute("x_range_flip", 1);
            }

          double theta_min = *std::min_element(theta.begin(), theta.end());
          double theta_max = *std::max_element(theta.begin(), theta.end());
          transformCoordinatesVector(theta, theta_min, theta_max, xrange_min, xrange_max);
        }
    }
  else
    {
      throw NotFoundError("Polar histogram series is missing data theta or bincounts\n");
    }

  if (plot_group->hasAttribute("phi_lim_min") || plot_group->hasAttribute("phi_lim_max"))
    {
      phi_lim = phi_lim_arr;
      phi_lim[0] = static_cast<double>(plot_group->getAttribute("phi_lim_min"));
      phi_lim[1] = static_cast<double>(plot_group->getAttribute("phi_lim_max"));

      if (phi_lim[1] < phi_lim[0])
        {
          std::swap(phi_lim[0], phi_lim[1]);
          plot_group->setAttribute("phi_flip", 1);
        }
      if (phi_lim[0] < 0.0 || phi_lim[1] > 2 * M_PI) logger((stderr, "\"phi_lim\" must be between 0 and 2 * pi\n"));
      plot_group->setAttribute("phi_lim_min", phi_lim[0]);
      plot_group->setAttribute("phi_lim_max", phi_lim[1]);
    }

  /* bin_edges and num_bins */
  if (!group->hasAttribute("bin_edges"))
    {
      if (!group->hasAttribute("num_bins"))
        {
          num_bins = grm_min(12, (int)(length / 2.0) - 1);
          group->setAttribute("num_bins", static_cast<int>(num_bins));
        }
      else
        {
          num_bins = static_cast<int>(group->getAttribute("num_bins"));
          if (num_bins <= 0 || num_bins > 200)
            {
              num_bins = grm_min(12, (int)(length / 2.0) - 1);
              group->setAttribute("num_bins", static_cast<int>(num_bins));
            }
        }
      // check phi_lim again
      if (phi_lim == nullptr)
        num_bin_edges = 0;
      else
        {
          // if phi_lim is given, it will create equidistant bin_edges from phi_min to phi_max
          bin_edges.resize(num_bins + 1);
          linspace(phi_lim[0], phi_lim[1], (int)num_bins + 1, bin_edges);
          num_bin_edges = num_bins + 1;
          (*context)[bin_edges_key] = bin_edges;
          group->setAttribute("bin_edges", bin_edges_key);
        }
    }
  else /* with bin_edges */
    {
      int cnt = 0;

      bin_edges_key = static_cast<std::string>(group->getAttribute("bin_edges"));
      bin_edges = GRM::get<std::vector<double>>((*context)[bin_edges_key]);
      num_bin_edges = bin_edges.size();

      /* filter bin_edges */
      new_edges.resize(num_bin_edges);
      for (int i = 0; i < num_bin_edges; ++i)
        {
          if (phi_lim == nullptr) /* no phi_lim */
            {
              if (0.0 <= bin_edges[i] && bin_edges[i] <= 2 * M_PI)
                {
                  new_edges[cnt++] = bin_edges[i];
                }
              else
                {
                  logger((stderr, "Only values between 0 and 2 * pi allowed\n"));
                }
            }
          else
            {
              if (phi_lim[0] <= bin_edges[i] && bin_edges[i] <= phi_lim[1])
                {
                  new_edges[cnt++] = bin_edges[i];
                }
            }
        }
      if (num_bin_edges > cnt)
        {
          num_bin_edges = cnt;
          bin_edges.resize(cnt);
        }
      else
        {
          bin_edges = new_edges;
        }
      if (phi_lim == nullptr) /* no phi_lim */
        {
          num_bins = num_bin_edges - 1;
          group->setAttribute("num_bins", static_cast<int>(num_bins));
        }
      else /* with phi_lim and bin_edges */
        {
          if (num_bin_edges == 1)
            {
              logger((stderr, "Given \"phi_lim\" and given \"bin_edges\" are not compatible --> filtered "
                              "\"len(bin_edges) == 1\"\n"));
            }
          else
            {
              num_bins = num_bin_edges - 1;
              group->setAttribute("num_bins", static_cast<int>(num_bins));
              group->setAttribute("bin_edges", bin_edges_key);
              (*context)[bin_edges_key] = bin_edges;
            }
        }
    }

  if (group->hasAttribute("norm"))
    {
      norm = static_cast<std::string>(group->getAttribute("norm"));
      if (!str_equals_any(norm, "count", "countdensity", "pdf", "probability", "cumcount", "cdf"))
        {
          logger((stderr, "Got keyword \"norm\"  with invalid value \"%s\"\n", norm.c_str()));
        }
    }

  if (!group->hasAttribute("bin_width"))
    {
      if (num_bin_edges > 0)
        {
          bin_widths.resize(num_bins + 1);
          for (int i = 1; i <= num_bin_edges - 1; ++i)
            {
              bin_widths[i - 1] = bin_edges[i] - bin_edges[i - 1];
            }
          group->setAttribute("bin_widths", bin_widths_key);
          (*context)[bin_widths_key] = bin_widths;
        }
      else
        {
          bin_width = 2 * M_PI / num_bins;
          group->setAttribute("bin_width", bin_width);
        }
    }
  else /* bin_width is given */
    {
      int n = 0;

      bin_width = static_cast<double>(group->getAttribute("bin_width"));

      if (num_bin_edges > 0 && phi_lim == nullptr)
        {
          logger((stderr, "\"bin_width\" is not compatible with \"bin_edges\"\n"));

          bin_widths.resize(num_bins);

          for (int i = 1; i <= num_bin_edges - 1; ++i)
            {
              bin_widths[i - 1] = bin_edges[i] - bin_edges[i - 1];
            }
          group->setAttribute("bin_widths", bin_widths_key);
          (*context)[bin_widths_key] = bin_widths;
        }

      if (bin_width <= 0 || bin_width > 2 * M_PI)
        logger((stderr, "\"bin_width\" must be between 0 (exclusive) and 2 * pi\n"));

      /* with phi_lim (with bin_width) */
      if (phi_lim != nullptr)
        {
          if (phi_lim[1] - phi_lim[0] < bin_width)
            {
              logger((stderr, "The given \"phi_lim\" range does not work with the given \"bin_width\"\n"));
            }
          else
            {
              n = (int)((phi_lim[1] - phi_lim[0]) / bin_width);
              if (is_bin_counts)
                {
                  if (num_bins > n)
                    logger((stderr, "\"bin_width\" does not work with this specific \"bin_count\". \"nbins\" do not "
                                    "fit \"bin_width\"\n"));
                  n = (int)num_bins;
                }
              bin_edges.resize(n + 1);
              linspace(phi_lim[0], n * bin_width, n + 1, bin_edges);
            }
        }
      else /* without phi_lim */
        {
          if ((int)(2 * M_PI / bin_width) > 200)
            {
              n = 200;
              bin_width = 2 * M_PI / n;
            }
          n = (int)(2 * M_PI / bin_width);
          if (is_bin_counts)
            {
              if (num_bins > n)
                logger((stderr, "\"bin_width\" does not work with this specific \"bin_count\". \"nbins\" do not fit "
                                "\"bin_width\"\n"));
              n = (int)num_bins;
            }
          bin_edges.resize(n + 1);
          linspace(0.0, n * bin_width, n + 1, bin_edges);
        }
      group->setAttribute("num_bins", n);
      num_bin_edges = n + 1;
      num_bins = n;
      group->setAttribute("bin_edges", bin_edges_key);
      (*context)[bin_edges_key] = bin_edges;
      group->setAttribute("bin_width", bin_width);
      bin_widths.resize(num_bins);

      for (int i = 0; i < num_bins; ++i)
        {
          bin_widths[i] = bin_width;
        }
      group->setAttribute("bin_widths", bin_widths_key);
      (*context)[bin_widths_key] = bin_widths;
    }

  /* is_bin_counts */
  if (is_bin_counts)
    {
      double temp_max_bc = 0.0;
      int i, j, total = 0, prev = 0;

      if (num_bin_edges > 0 && num_bins != num_bin_edges - 1)
        {
          logger((stderr, "Number of bin_edges must be number of bin_counts + 1\n"));
        }

      total = std::accumulate(bin_counts.begin(), bin_counts.end(), 0);
      for (i = 0; i < num_bins; ++i)
        {
          // temp_max_bc is a potential maximum for all bins respecting the given norm
          if (num_bin_edges > 0) bin_width = bin_widths[i];

          if (norm == "pdf" && bin_counts[i] * 1.0 / (total * bin_width) > temp_max_bc)
            temp_max_bc = bin_counts[i] * 1.0 / (total * bin_width);
          else if (norm == "countdensity" && bin_counts[i] * 1.0 / (bin_width) > temp_max_bc)
            temp_max_bc = bin_counts[i] * 1.0 / (bin_width);
          else if (bin_counts[i] > temp_max_bc)
            temp_max_bc = bin_counts[i];
        }

      classes.resize(num_bins);

      // bin_counts is affected by cumulative norms --> bin_counts are summed in later bins
      if (str_equals_any(norm, "cdf", "cumcount"))
        {
          for (i = 0; i < num_bins; ++i)
            {
              classes[i] = bin_counts[i];
              if (i != 0) classes[i] += classes[i - 1];
            }
        }
      else
        {
          classes = bin_counts;
        }

      group->setAttribute("classes", classes_key);
      (*context)[classes_key] = classes;
      group->setAttribute("total", total);

      if (norm == "probability")
        max = temp_max_bc * 1.0 / total;
      else if (norm == "cdf")
        max = 1.0;
      else if (norm == "cumcount")
        max = total * 1.0;
      else
        max = temp_max_bc;
    }
  else /* no is_bin_counts */
    {
      max = 0.0;
      classes.resize(num_bins);

      // prepare bin_edges
      if (num_bin_edges == 0) // no bin_edges --> create bin_edges for uniform code later
        {
          // linspace the bin_edges
          bin_edges.resize(num_bins + 1);
          linspace(0.0, 2 * M_PI, (int)num_bins + 1, bin_edges);
        }
      else // bin_edges given
        {
          // filter theta
          double edge_min = bin_edges[0], edge_max = bin_edges[num_bin_edges - 1];

          auto it = std::remove_if(theta.begin(), theta.end(), [edge_min, edge_max](double angle) {
            return (angle < edge_min || angle > edge_max);
          });
          theta.erase(it, theta.end());
          length = theta.size();
        }

      // calc classes
      for (int x = 0; x < num_bins; ++x)
        {
          int observations = 0;

          // iterate theta --> filter angles for current bin
          for (int y = 0; y < length; ++y)
            {
              if (bin_edges[x] <= theta[y] && theta[y] < bin_edges[x + 1]) ++observations;
            }

          // differentiate between cumulative and non-cumulative norms
          classes[x] = observations;
          if (x != 0 && str_equals_any(norm, "cdf", "cumcount")) classes[x] += classes[x - 1];
          // update the total number of observations; used for some norms
          total_observations += observations;
        }

      // get maximum number of observation from all bins
      max_observations = *std::max_element(classes.begin(), classes.end());

      group->setAttribute("classes", classes_key);
      (*context)[classes_key] = classes;
      group->setAttribute("total", total_observations);

      // calculate the maximum from maxObservations respecting the norms
      if (num_bin_edges == 0 && norm == "pdf") // no given bin_edges
        {
          max = max_observations * 1.0 / (total_observations * bin_width);
        }
      else if (num_bin_edges != 0 && str_equals_any(norm, "pdf", "countdensity")) // calc maximum with given bin_edges
        {
          for (int i = 0; i < num_bins; ++i)
            {
              // temporary maximum respecting norm
              temp_max = classes[i];
              if (norm == "pdf")
                temp_max /= total_observations * bin_widths[i];
              else if (norm == "countdensity")
                temp_max /= bin_widths[i];

              if (temp_max > max) max = temp_max;
            }
        }
      else if (str_equals_any(norm, "probability", "cdf"))
        {
          max = max_observations * 1.0 / total_observations;
        }
      else
        {
          max = max_observations * 1.0;
        }
    } /* end classes and maximum */
  // set r_max (radius_max) in parent for later usages in polar_axes and polar_histogram
  group->parentElement()->setAttribute("r_max", max);
}

static void processPolarHistogram(const std::shared_ptr<GRM::Element> &element,
                                  const std::shared_ptr<GRM::Context> &context)
{
  unsigned int num_bins, num_bin_edges = 0;
  int edge_color = 1, face_color = 989;
  int total_observations = 0;
  int xcolormap = -2, ycolormap = -2;
  int child_id = 0;
  double face_alpha = 0.75, bin_width = -1.0, max;
  double r_min = 0.0, r_max = 1.0;
  double *r_lim = nullptr, *phi_lim = nullptr;
  double phi_lim_arr[2];
  double ylim_min, ylim_max;
  bool draw_edges = false, stairs = false, phiflip = false, keep_radii_axes = false, ylim = false, colormaps = false;
  std::string norm = "count";
  std::vector<double> r_lim_vec;
  std::vector<double> bin_edges, bin_widths;
  std::vector<double> rectlist;
  std::vector<int> classes;
  std::shared_ptr<GRM::Element> plot_group = element->parentElement();
  getPlotParent(plot_group);
  del_values del = del_values::update_without_default;

  auto classes_key = static_cast<std::string>(element->getAttribute("classes"));
  classes = GRM::get<std::vector<int>>((*context)[classes_key]);

  /* clear old polar-histogram children */
  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  if (element->hasAttribute("line_color_ind")) edge_color = static_cast<int>(element->getAttribute("line_color_ind"));
  if (element->hasAttribute("color_ind")) face_color = static_cast<int>(element->getAttribute("color_ind"));
  if (element->hasAttribute("face_alpha")) face_alpha = static_cast<double>(element->getAttribute("face_alpha"));
  if (element->hasAttribute("norm")) norm = static_cast<std::string>(element->getAttribute("norm"));
  if (plot_group->hasAttribute("phi_flip")) phiflip = static_cast<int>(plot_group->getAttribute("phi_flip"));
  if (element->hasAttribute("draw_edges")) draw_edges = static_cast<int>(element->getAttribute("draw_edges"));

  num_bins = static_cast<int>(element->getAttribute("num_bins"));
  max = static_cast<double>(element->parentElement()->getAttribute("r_max"));
  total_observations = static_cast<int>(element->getAttribute("total"));
  global_render->setTransparency(element, face_alpha);
  processTransparency(element);

  if (plot_group->hasAttribute("y_lim_min") && (plot_group->hasAttribute("y_lim_max")))
    {
      ylim = true;
      ylim_min = static_cast<double>(plot_group->getAttribute("y_lim_min"));
      ylim_max = static_cast<double>(plot_group->getAttribute("y_lim_max"));

      if (plot_group->hasAttribute("keep_radii_axes"))
        {
          keep_radii_axes = static_cast<int>(plot_group->getAttribute("keep_radii_axes"));
        }
    }
  if (plot_group->hasAttribute("phi_lim_min") || plot_group->hasAttribute("phi_lim_max"))
    {
      phi_lim = phi_lim_arr;
      phi_lim[0] = static_cast<double>(plot_group->getAttribute("phi_lim_min"));
      phi_lim[1] = static_cast<double>(plot_group->getAttribute("phi_lim_max"));
    }

  if (!element->hasAttribute("bin_edges"))
    {
      if (element->hasAttribute("bin_width")) bin_width = static_cast<double>(element->getAttribute("bin_width"));
    }
  else
    {
      auto bin_edges_key = static_cast<std::string>(element->getAttribute("bin_edges"));
      bin_edges = GRM::get<std::vector<double>>((*context)[bin_edges_key]);
      num_bin_edges = bin_edges.size();

      auto bin_widths_key = static_cast<std::string>(element->getAttribute("bin_widths"));
      bin_widths = GRM::get<std::vector<double>>((*context)[bin_widths_key]);
      num_bins = bin_widths.size();
    }

  if (element->hasAttribute("stairs"))
    {
      /* Set default stairs line color width and alpha values */
      if (!element->hasAttribute("face_alpha")) element->setAttribute("face_alpha", 1.0);

      stairs = static_cast<int>(element->getAttribute("stairs"));
      if (stairs)
        {
          rectlist.resize(num_bins);
        }
    }

  if (phiflip) std::reverse(classes.begin(), classes.end());

  /* if phiflip and bin_edges are given --> invert the angles */
  if (phiflip && num_bin_edges > 0)
    {
      int u;
      std::vector<double> temp(num_bin_edges), temp2(num_bins);

      for (u = 0; u < num_bin_edges; u++)
        {
          temp[u] = 2 * M_PI - bin_edges[num_bin_edges - 1 - u];
        }
      for (u = (int)num_bins - 1; u >= 0; --u)
        {
          temp2[u] = bin_widths[num_bins - 1 - u];
        }
      bin_widths = temp2;
      bin_edges = temp;
    }

  // Special colormap case! Dont iterate through every bar because of the 2000x2000 drawimage size!
  if (!(element->hasAttribute("x_colormap") && element->hasAttribute("y_colormap")))
    {
      if (draw_edges) logger((stderr, "\"draw_edges\" can only be used with colormap\n"));
    }
  else
    {
      xcolormap = static_cast<int>(element->getAttribute("x_colormap"));
      ycolormap = static_cast<int>(element->getAttribute("y_colormap"));
      colormaps = true;
    }

  // Iterate through the classes and create for every bar polarbar
  for (int class_nr = 0; class_nr < classes.size();
       ++class_nr) // main loop used for each bar (and arc in stairs; but not the lines in stairs
    {
      double count = classes[class_nr];

      // adjust count according to the given normalization
      if (str_equals_any(norm, "probability", "cdf"))
        {
          count /= total_observations;
        }
      else if (norm == "pdf")
        {
          if (num_bin_edges == 0)
            {
              count /= total_observations * bin_width;
            }
          else
            {
              count /= (total_observations * bin_widths[class_nr]);
            }
        }
      else if (norm == "countdensity")
        {
          if (num_bin_edges == 0)
            {
              count /= bin_width;
            }
          else
            {
              count /= bin_widths[class_nr];
            }
        }

      // no stairs
      if (!stairs)
        {
          std::shared_ptr<GRM::Element> polar_bar;

          if (del != del_values::update_without_default && del != del_values::update_with_default)
            {
              polar_bar = global_render->createPolarBar(count, class_nr);
              polar_bar->setAttribute("_child_id", child_id++);
              element->append(polar_bar);
            }
          else
            {
              polar_bar = element->querySelectors("polar_bar[_child_id=" + std::to_string(child_id++) + "]");
              if (polar_bar != nullptr) global_render->createPolarBar(count, class_nr, polar_bar);
            }

          if (polar_bar != nullptr)
            {
              if (bin_width != -1) polar_bar->setAttribute("bin_width", bin_width);
              if (norm != "count") polar_bar->setAttribute("norm", norm);
              if (phiflip) polar_bar->setAttribute("phi_flip", phiflip);
              if (draw_edges) polar_bar->setAttribute("draw_edges", draw_edges);
              if (edge_color != 1) polar_bar->setAttribute("line_color_ind", edge_color);
              if (face_color != 989) polar_bar->setAttribute("color_ind", face_color);
              if (xcolormap != -2) polar_bar->setAttribute("x_colormap", xcolormap);
              if (ycolormap != -2) polar_bar->setAttribute("y_colormap", ycolormap);
              if (!bin_widths.empty()) polar_bar->setAttribute("bin_widths", bin_widths[class_nr]);
              if (!bin_edges.empty())
                {
                  auto id = static_cast<int>(global_root->getAttribute("_id"));
                  auto str = std::to_string(id);
                  global_root->setAttribute("_id", id + 1);

                  auto bin_edges_vec = std::vector<double>{bin_edges[class_nr], bin_edges[class_nr + 1]};
                  auto bin_edges_key = "bin_edges" + str;
                  (*context)[bin_edges_key] = bin_edges_vec;
                  polar_bar->setAttribute("bin_edges", bin_edges_key);
                }
            }
        }
      else if (!draw_edges && (xcolormap == -2 && ycolormap == -2)) /* stairs without draw_edges (not compatible) */
        {
          // this is for drawing the arcs in stairs.
          double r, rect;
          std::complex<double> complex1, complex2;
          const double convert = 180.0 / M_PI;
          double edge_width = 2.3; /* only for stairs */
          bool draw_inner = true;

          global_render->setFillColorInd(element, 1);
          global_render->setLineColorInd(element, edge_color);
          global_render->setLineWidth(element, edge_width);
          processLineColorInd(element);
          processFillColorInd(element);
          processLineWidth(element);

          /* perform calculations for later usages, this r is used for complex calculations */
          if (keep_radii_axes && ylim)
            {
              r = pow((count / max), num_bins * 2);
              if (r > pow(ylim_max / max, num_bins * 2))
                {
                  r = pow(ylim_max / max, num_bins * 2);
                }
            }
          else if (ylim)
            {
              // trim count to ylim_max if higher
              if (count > ylim_max)
                {
                  count = ylim_max;
                }
              count -= ylim_min;
              if (count < 0.0) count = 0.0;
              r = pow((count / (ylim_max - ylim_min)), num_bins * 2);
            }
          else
            {
              r = pow((count / max), (num_bins * 2));
            }

          complex1 = moivre(r, (2 * class_nr), (int)num_bins * 2);
          complex2 = moivre(r, (2 * class_nr + 2), ((int)num_bins * 2));
          rect = sqrt(pow(real(complex1), 2) + pow(imag(complex1), 2));

          double start_angle, end_angle;
          if (num_bin_edges)
            {
              start_angle = bin_edges[class_nr] * convert;
              end_angle = bin_edges[class_nr + 1] * convert;
            }
          else
            {
              start_angle = class_nr * (360.0 / num_bins);
              end_angle = (class_nr + 1) * (360 / num_bins);
            }

          std::shared_ptr<GRM::Element> arc;
          double arc_pos;

          if (ylim)
            {
              if (keep_radii_axes)
                {
                  if (count < ylim_min)
                    rectlist[class_nr] = ylim_min / ylim_max;
                  else if (rect > r_max)
                    rectlist[class_nr] = ylim_max;
                  else
                    rectlist[class_nr] = rect;

                  auto complex_min = moivre(pow(ylim_min / ylim_max, num_bins * 2), (2 * class_nr), (int)num_bins * 2);
                  arc_pos = sqrt(pow(real(complex_min), 2) + pow(imag(complex_min), 2));
                  if (count < ylim_min) arc_pos = 0.0;
                }
              else
                {
                  if (count < ylim_min)
                    rectlist[class_nr] = 0.0;
                  else if (count > ylim_max)
                    rectlist[class_nr] = 1.0; // 1.0 equals ylim_max (when no keep_radii_axes is set)
                  else
                    rectlist[class_nr] = rect;

                  arc_pos = 0.0;
                }

              // this is the outer arc
              if ((count > 0 && !keep_radii_axes) || (count > ylim_min && keep_radii_axes))
                {
                  if (del != del_values::update_without_default && del != del_values::update_with_default)
                    {
                      arc = global_render->createDrawArc(-grm_min(rect, r_max), grm_min(rect, r_max),
                                                         -grm_min(rect, r_max), grm_min(rect, r_max), start_angle,
                                                         end_angle);
                      arc->setAttribute("_child_id", child_id++);
                      element->append(arc);
                    }
                  else
                    {
                      arc = element->querySelectors("draw_arc[_child_id=" + std::to_string(child_id++) + "]");
                      if (arc != nullptr)
                        global_render->createDrawArc(-grm_min(rect, r_max), grm_min(rect, r_max), -grm_min(rect, r_max),
                                                     grm_min(rect, r_max), start_angle, end_angle, arc);
                    }
                }
              else
                {
                  draw_inner = false;
                }
            }
          else /* no rlim */
            {
              rectlist[class_nr] = rect;
              if (class_nr == classes.size()) break;
              arc_pos = rect;
            }

          // these are the inner arcs with ylim and the normal_arcs without ylim, only if its higher than ylim_min
          if (draw_inner)
            {
              if (del != del_values::update_without_default && del != del_values::update_with_default)
                {
                  arc = global_render->createDrawArc(-arc_pos, arc_pos, -arc_pos, arc_pos, start_angle, end_angle);
                  arc->setAttribute("_child_id", child_id++);
                  element->append(arc);
                }
              else
                {
                  arc = element->querySelectors("draw_arc[_child_id=" + std::to_string(child_id++) + "]");
                  if (arc != nullptr)
                    global_render->createDrawArc(-arc_pos, arc_pos, -arc_pos, arc_pos, start_angle, end_angle, arc);
                }
            }
        }
    } /* end of classes for loop */

  // this is for drawing the stairs straight lines.
  if (stairs && !draw_edges && (xcolormap == -2 && ycolormap == -2))
    {
      std::shared_ptr<GRM::Element> line;
      double line_x[2], line_y[2];

      if (ylim)
        {
          double startx, starty;

          // startx/y is the coordinate for minimum radius (ylim_min)
          std::vector<double> angles_vec;

          if (num_bin_edges != 0)
            {
              startx = grm_max(rectlist[0] * cos(bin_edges[0]), r_min * cos(bin_edges[0]));
              starty = grm_max(rectlist[0] * sin(bin_edges[0]), r_min * sin(bin_edges[0]));
              angles_vec = bin_edges;
            }
          else
            {
              startx = grm_max(rectlist[0] * cos(2 * M_PI / num_bins * 0), ylim_min / ylim_max * cos(0.0));
              starty = grm_max(rectlist[0] * sin(2 * M_PI / num_bins * 0), ylim_min / ylim_max * sin(0.0));
              linspace(0.0, 2 * M_PI, classes.size() + 1, angles_vec);
            }

          for (int x = 0; x < classes.size(); ++x)
            {
              line_x[0] = startx;
              line_x[1] = rectlist[x] * cos(angles_vec[x]);
              line_y[0] = starty;
              line_y[1] = rectlist[x] * sin(angles_vec[x]);

              startx = rectlist[x] * cos(angles_vec[x + 1]);
              starty = rectlist[x] * sin(angles_vec[x + 1]);

              if ((!phiflip && (!((angles_vec[0] > 0.0 && angles_vec[0] < 0.001) &&
                                  angles_vec[angles_vec.size() - 1] > 1.96 * M_PI) ||
                                x > 0)) ||
                  ((angles_vec[0] > 1.96 * M_PI &&
                    !(angles_vec[angles_vec.size() - 1] > 0.0 && angles_vec[angles_vec.size() - 1] < 0.001)) ||
                   x > 0))
                {
                  if (del != del_values::update_without_default && del != del_values::update_with_default)
                    {
                      line = global_render->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1]);
                      line->setAttribute("_child_id", child_id++);
                      element->append(line);
                    }
                  else
                    {
                      line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
                      if (line != nullptr)
                        global_render->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1], 0, 0.0, 0, line);
                    }
                }
            }

          // draw a line when it is not a full circle
          if (!(angles_vec[0] == 0.0 && angles_vec[angles_vec.size() - 1] > 1.96 * M_PI))
            {
              line_x[0] = ylim_min / ylim_max * cos(angles_vec[0]);
              line_x[1] = rectlist[0] * cos(angles_vec[0]);
              line_y[0] = ylim_min / ylim_max * sin(angles_vec[0]);
              line_y[1] = rectlist[0] * sin(angles_vec[0]);

              if (del != del_values::update_without_default && del != del_values::update_with_default)
                {
                  line = global_render->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1]);
                  line->setAttribute("_child_id", child_id++);
                  element->append(line);
                }
              else
                {
                  line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
                  if (line != nullptr)
                    global_render->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1], 0, 0.0, 0, line);
                }
            }

          if (angles_vec[0] == 0.0 && angles_vec[angles_vec.size() - 1] > 1.96 * M_PI)
            {
              line_x[0] = rectlist[0] * cos(angles_vec[0]);
              line_x[1] = rectlist[angles_vec.size() - 2] * cos(angles_vec[angles_vec.size() - 1]);
              line_y[0] = rectlist[0] * sin(angles_vec[0]);
              line_y[1] = rectlist[angles_vec.size() - 2] * sin(angles_vec[angles_vec.size() - 1]);
            }
          else
            {
              line_x[0] = rectlist[angles_vec.size() - 2] * cos(angles_vec[angles_vec.size() - 1]);
              line_x[1] = ylim_min / ylim_max * cos(angles_vec[angles_vec.size() - 1]);
              line_y[0] = rectlist[angles_vec.size() - 2] * sin(angles_vec[angles_vec.size() - 1]);
              line_y[1] = ylim_min / ylim_max * sin(angles_vec[angles_vec.size() - 1]);
            }
        } // end of ylim case
      else
        { // without ylims
          double startx = 0.0, starty = 0.0;

          std::vector<double> angles_vec;

          if (num_bin_edges != 0)
            {
              startx = grm_max(rectlist[0] * cos(bin_edges[0]), r_min * cos(bin_edges[0]));
              starty = grm_max(rectlist[0] * sin(bin_edges[0]), r_min * sin(bin_edges[0]));
              angles_vec = bin_edges;
            }
          else
            {
              startx = grm_max(rectlist[0] * cos(2 * M_PI / num_bins * 0), ylim_min / ylim_max * cos(0.0));
              starty = grm_max(rectlist[0] * sin(2 * M_PI / num_bins * 0), ylim_min / ylim_max * sin(0.0));
              linspace(0.0, 2 * M_PI, classes.size() + 1, angles_vec);
            }

          for (int x = 0; x < angles_vec.size() - 1; ++x)
            {
              line_x[0] = startx;
              line_x[1] = rectlist[x] * cos(angles_vec[x]);
              line_y[0] = starty;
              line_y[1] = rectlist[x] * sin(angles_vec[x]);

              startx = rectlist[x] * cos(angles_vec[x + 1]);
              starty = rectlist[x] * sin(angles_vec[x + 1]);

              if (!(angles_vec[0] == 0.0 && angles_vec[angles_vec.size() - 1] > 1.96 * M_PI) || x > 0)
                {
                  if (del != del_values::update_without_default && del != del_values::update_with_default)
                    {
                      line = global_render->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1]);
                      line->setAttribute("_child_id", child_id++);
                      element->append(line);
                    }
                  else
                    {
                      line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
                      if (line != nullptr)
                        global_render->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1], 0, 0.0, 0, line);
                    }
                }
            }

          if (angles_vec[0] == 0.0 && angles_vec[angles_vec.size() - 1] > 1.96 * M_PI)
            {
              line_x[0] = rectlist[0] * cos(angles_vec[0]);
              line_x[1] = startx;
              line_y[0] = rectlist[0] * sin(angles_vec[0]);
              line_y[1] = starty;
            }
          else
            {
              line_x[0] = rectlist[angles_vec.size() - 2] * cos(angles_vec[angles_vec.size() - 1]);
              line_x[1] = 0.0;
              line_y[0] = rectlist[angles_vec.size() - 2] * sin(angles_vec[angles_vec.size() - 1]);
              line_y[1] = 0.0;
            }
        } // end of no ylim case

      if (del != del_values::update_without_default && del != del_values::update_with_default)
        {
          line = global_render->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1]);
          line->setAttribute("_child_id", child_id++);
          element->append(line);
        }
      else
        {
          line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
          if (line != nullptr)
            global_render->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1], 0, 0.0, 0, line);
        }
    }
}

static void processPolarBar(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  unsigned int resample;
  double r, rect;
  double y_lim_min, y_lim_max;
  std::complex<double> complex1;
  std::vector<double> bin_edges;
  const double convert = 180.0 / M_PI;
  std::vector<double> f1, f2, arc_2_x, arc_2_y;
  std::vector<double> phi_vec;
  int child_id = 0;
  int x_colormap = -2, y_colormap = -2;
  double count, bin_width = -1.0;
  double max; // todo rename max -> radius_max?
  int num_bins, num_bin_edges = 0, class_nr;
  std::string norm = "count", str;
  bool phiflip = false, draw_edges = false, keep_radii_axes = false, y_lim = false, is_colormap = false;
  int edge_color = 1, face_color = 989;
  std::shared_ptr<GRM::Element> plot_elem = element->parentElement();
  getPlotParent(plot_elem);
  del_values del = del_values::update_without_default;

  /* clear old polar-histogram children */
  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  // class_nr is used for the position of the bar in the histogram
  class_nr = static_cast<int>(element->getAttribute("class_nr"));
  // count is already converted by normalization!
  count = static_cast<double>(element->getAttribute("count"));

  if (element->hasAttribute("bin_width")) bin_width = static_cast<double>(element->getAttribute("bin_width"));
  if (element->hasAttribute("norm")) norm = static_cast<std::string>(element->getAttribute("norm"));

  if (element->hasAttribute("phi_flip")) phiflip = static_cast<int>(element->getAttribute("phi_flip"));
  if (element->hasAttribute("draw_edges")) draw_edges = static_cast<int>(element->getAttribute("draw_edges"));
  if (element->hasAttribute("line_color_ind")) edge_color = static_cast<int>(element->getAttribute("line_color_ind"));
  if (element->hasAttribute("color_ind")) face_color = static_cast<int>(element->getAttribute("color_ind"));

  if (element->hasAttribute("x_colormap"))
    {
      x_colormap = static_cast<int>(element->getAttribute("x_colormap"));
      is_colormap = true;
    }
  if (element->hasAttribute("y_colormap"))
    {
      y_colormap = static_cast<int>(element->getAttribute("y_colormap"));
      is_colormap = true;
    }

  if (element->hasAttribute("bin_edges"))
    {
      auto bin_edges_key = static_cast<std::string>(element->getAttribute("bin_edges"));
      bin_edges = GRM::get<std::vector<double>>((*context)[bin_edges_key]);
      num_bin_edges = (int)bin_edges.size();
    }

  num_bins = static_cast<int>(element->parentElement()->getAttribute("num_bins"));

  if (element->parentElement()->hasAttribute("bin_widths"))
    {
      auto bin_widths_key = static_cast<std::string>(element->parentElement()->getAttribute("bin_widths"));
      auto bin_widths_vec = GRM::get<std::vector<double>>((*context)[bin_widths_key]);
      num_bins = (int)bin_widths_vec.size();
      bin_width = bin_widths_vec[class_nr];
    }

  // no ylims -> max = ylim_max; with ylims -> max = max_count of series
  for (const auto &child : plot_elem->children())
    {
      if (child->localName() == "central_region")
        {
          max = static_cast<double>(child->getAttribute("r_max"));
          break;
        }
    }

  if (plot_elem->hasAttribute("y_lim_min") && plot_elem->hasAttribute("y_lim_max"))
    {
      y_lim = true;
      y_lim_min = static_cast<double>(plot_elem->getAttribute("y_lim_min"));
      y_lim_max = static_cast<double>(plot_elem->getAttribute("y_lim_max"));
      if (plot_elem->hasAttribute("keep_radii_axes"))
        {
          // if true: max = true radius axes maximum!
          keep_radii_axes = static_cast<int>(plot_elem->getAttribute("keep_radii_axes"));
        }
    }
  else
    {
      y_lim_min = 0.0;
      y_lim_max = max;
    }

  // creates an image for draw_image
  if (is_colormap)
    {
      if (-1 > x_colormap || x_colormap > 47 || y_colormap < -1 || y_colormap > 47)
        {
          logger((stderr, "The value for keyword \"colormap\" must contain two integer between -1 and 47\n"));
        }
      else
        {
          std::shared_ptr<GRM::Element> drawImage;
          const int colormap_size = 500, image_size = 2000;
          // todo: maybe use dynamic image_size when using interactions or maybe calculate a smaller rectangle in image
          // todo: only one bar per rectangle -> no image_size * image_size iterations
          // todo: or maybe don't iterate through image_size * image_size matrix
          // todo: instead calculate the the coordinates and iterate through these coordinates somehow like in python
          double radius, angle, max_radius, count_radius;
          double start_angle, end_angle;
          double ylim_min_radius, ylim_max_radius;
          int total = 0;
          double norm_factor = 1;
          double original_count = count;
          std::vector<int> lineardata, colormap;
          int id = (int)global_root->getAttribute("_id");

          global_root->setAttribute("_id", id + 1);
          const std::string &str = std::to_string(id);

          lineardata.resize(image_size * image_size);

          createColormap(x_colormap, y_colormap, colormap_size, colormap);

          if (num_bin_edges == 0)
            {
              start_angle = M_PI * 2 / ((int)num_bins) * class_nr;
              end_angle = M_PI * 2 / ((int)num_bins) * (class_nr + 1);
            }
          else
            {
              start_angle = bin_edges[class_nr];
              end_angle = bin_edges[class_nr + 1];
            }

          max_radius = image_size / 2;

          total = static_cast<int>(element->getAttribute("total"));

          if (str_equals_any(norm.c_str(), 2, "probability", "cdf"))
            norm_factor = total;
          else if (num_bin_edges == 0 && norm == "pdf")
            norm_factor = total * bin_width;
          else if (num_bin_edges == 0 && norm == "countdensity")
            norm_factor = bin_width;

          if (!keep_radii_axes)
            {
              if (count > y_lim_max)
                {
                  count = y_lim_max;
                }
              count -= y_lim_min;
            }
          else
            {
              // if keep_radii_axes-> max = true y axis maximum!
              ylim_min_radius = y_lim_min / max * max_radius;
              ylim_max_radius = grm_min(y_lim_max, max) / max * max_radius;
            }

          if (norm == "pdf" && num_bin_edges > 0)
            norm_factor = total * bin_width;
          else if (norm == "countdensity" && num_bin_edges > 0)
            norm_factor = bin_width;

          // calculate the radius of the bar with height count
          if (keep_radii_axes)
            {
              count_radius = (grm_round((count * 1.0 / norm_factor / max * max_radius) * 100) / 100);
            }
          else
            {
              count_radius = (grm_round((count / norm_factor / (y_lim_max - y_lim_min) * max_radius) * 100) / 100);
            }

          // go through every point in the image and check if its inside of a bar
          for (int y = 0; y < image_size; y++)
            {
              for (int x = 0; x < image_size; x++)
                {
                  radius = sqrt(pow(x - max_radius, 2) + pow(y - max_radius, 2));
                  radius = grm_round(radius * 100) / 100;
                  angle = atan2(y - max_radius, x - max_radius);

                  if (angle < 0) angle += M_PI * 2;
                  if (!phiflip) angle = 2 * M_PI - angle;

                  if (angle > start_angle && angle <= end_angle)
                    {

                      // todo: this could be done more efficiently if the if keep_radii_axes condition is
                      // todo: moved outside the loop. One less condition per iteration. But less readable
                      if (keep_radii_axes)
                        {
                          if (radius <= count_radius && radius <= ylim_max_radius && radius > ylim_min_radius)
                            {
                              lineardata[y * image_size + x] = colormap
                                  [(int)(radius / (max_radius * pow(2, 0.5)) * (colormap_size - 1)) * colormap_size +
                                   grm_max(grm_min((int)(angle / (2 * M_PI) * colormap_size), colormap_size - 1), 0)];
                            }
                        }
                      else
                        {
                          if ((grm_round(radius * 100) / 100) <= count_radius && radius <= max_radius && count > 0.0)
                            {
                              lineardata[y * image_size + x] = colormap
                                  [(int)(radius / (max_radius * pow(2, 0.5)) * (colormap_size - 1)) * colormap_size +
                                   grm_max(grm_min((int)(angle / (2 * M_PI) * colormap_size), colormap_size - 1), 0)];
                            }
                        }

                    } /* end angle check */

                } /* end x loop*/
            }     /* end y loop */


          /* save resample method and reset because it isn't restored with gr_restorestate */
          if (del != del_values::update_without_default && del != del_values::update_with_default)
            {
              drawImage = global_render->createDrawImage(-1.0, 1.0, 1.0, -1.0, image_size, image_size, "data" + str,
                                                         lineardata, 0);
              drawImage->setAttribute("_child_id", child_id++);
              element->append(drawImage);
            }
          else
            {
              drawImage = element->querySelectors("drawimage[_child_id=" + std::to_string(child_id++) + "]");
              if (drawImage != nullptr)
                global_render->createDrawImage(-1.0, 1.0, 1.0, -1.0, image_size, image_size, "data" + str, lineardata,
                                               0, nullptr, drawImage);
            }
          unsigned int resample;
          gr_inqresamplemethod(&resample);
          if (drawImage != nullptr) global_render->setResampleMethod(drawImage, static_cast<int>(0x2020202));
          lineardata.clear();
          colormap.clear();
          count = original_count;
        }
    } // end of colormaps

  // substract ylim_min from count for ylim_min > 0.0
  if (!keep_radii_axes)
    {
      // trim count to ylim_max if higher
      if (count > y_lim_max)
        {
          count = y_lim_max;
        }
      count -= y_lim_min;
    }

  /* perform calculations for later usages, this r is used for complex calculations */
  if (keep_radii_axes)
    {
      r = pow((count / max), num_bins * 2);
      if (r > pow(y_lim_max / max, num_bins * 2))
        {
          r = pow(y_lim_max / max, num_bins * 2);
        }
    }
  else
    {
      r = pow((count / (y_lim_max - y_lim_min)), num_bins * 2);
    }

  complex1 = moivre(r, 2 * class_nr, (int)num_bins * 2);

  // drawarc rectangle
  rect = sqrt(pow(real(complex1), 2) + pow(imag(complex1), 2));


  // this does not affect rect
  if (y_lim)
    {

      // this r is used directly for the radii of each drawarc
      if (keep_radii_axes)
        {
          r = count / max;
          if (r > y_lim_max / max) r = y_lim_max / max;
        }
      else
        {
          r = count / (y_lim_max - y_lim_min);
          if (r > y_lim_max) r = 1.0;
        }
    }

  // if keep_radii_axes is given, then arcs can not be easily drawn (because of the lower arc [donut shaped]), so
  // additional calculations are needed for arcs and lines
  if (!keep_radii_axes) /* no keep_radii_axes given*/
    {
      std::shared_ptr<GRM::Element> arc, draw_arc;
      double start_angle, end_angle;

      if (num_bin_edges != 0.0)
        {
          start_angle = bin_edges[0] * convert;
          end_angle = bin_edges[1] * convert;
        }
      else
        {
          start_angle = class_nr * (360.0 / num_bins);
          end_angle = (class_nr + 1) * (360.0 / num_bins);
        }
      if (!draw_edges)
        {
          if (del != del_values::update_without_default && del != del_values::update_with_default)
            {
              arc = global_render->createFillArc(-rect, rect, -rect, rect, start_angle, end_angle);
              arc->setAttribute("_child_id", child_id++);
              element->append(arc);
            }
          else
            {
              arc = element->querySelectors("fill_arc[_child_id=" + std::to_string(child_id++) + "]");
              if (arc != nullptr)
                global_render->createFillArc(-rect, rect, -rect, rect, start_angle, end_angle, 0, 0, -1, arc);
            }

          if (arc != nullptr)
            {
              global_render->setFillIntStyle(arc, 1);
              global_render->setFillColorInd(arc, face_color);
            }
        }

      if (del != del_values::update_without_default && del != del_values::update_with_default)
        {
          draw_arc = global_render->createFillArc(-rect, rect, -rect, rect, start_angle, end_angle);
          draw_arc->setAttribute("_child_id", child_id++);
          element->append(draw_arc);
        }
      else
        {
          draw_arc = element->querySelectors("fill_arc[_child_id=" + std::to_string(child_id++) + "]");
          if (draw_arc != nullptr)
            global_render->createFillArc(-rect, rect, -rect, rect, start_angle, end_angle, 0, 0, -1, draw_arc);
        }
      if (draw_arc != nullptr)
        {
          global_render->setFillIntStyle(draw_arc, 0);
          global_render->setFillColorInd(draw_arc, edge_color);
          draw_arc->setAttribute("z_index", 2);
        }
    }  /* end of no keep_radii_axes given*/
  else /* keep_radii_axes is given so extra calculations are needed */
    {
      int i, num_angle;
      double start_angle, end_angle;
      std::shared_ptr<GRM::Element> area;
      int id = (int)global_root->getAttribute("_id");

      if ((count > 0.0 && !keep_radii_axes) ||
          (count > y_lim_min && keep_radii_axes)) // check if original count (count + ylim_min) is larger than ylim_min
        {
          global_root->setAttribute("_id", id + 1);
          str = std::to_string(id);

          if (num_bin_edges != 0.0)
            {
              start_angle = bin_edges[0];
              end_angle = bin_edges[1];
            }
          else
            {
              start_angle = class_nr * (360.0 / num_bins) / convert;
              end_angle = (class_nr + 1) * (360.0 / num_bins) / convert;
            }

          // determine number of angles for arc approximations
          num_angle = (int)((end_angle - start_angle) / (0.2 / convert));
          phi_vec.resize(num_angle);
          linspace(start_angle, end_angle, num_angle, phi_vec);

          // 4 because of the 4 corner coordinates and 2 * num_angle for the arc approximations, top and
          // bottom
          f1.resize(4 + 2 * num_angle);
          /* line_1_x[0] and [1]*/
          f1[0] = cos(start_angle) * y_lim_min / max;
          f1[1] = rect * cos(start_angle);

          /* arc_1_x */
          listcomprehension(r, cos, phi_vec, num_angle, 2, f1);

          /* reversed line_2_x [0] and [1] */
          f1[2 + num_angle + 1] = cos(end_angle) * y_lim_min / max;
          f1[2 + num_angle] = rect * cos(end_angle);
          /* reversed arc_2_x */
          if (keep_radii_axes)
            {
              listcomprehension(y_lim_min / max, cos, phi_vec, num_angle, 0, arc_2_x);
            }
          else
            {
              listcomprehension(0.0, cos, phi_vec, num_angle, 0, arc_2_x);
            }

          for (i = 0; i < num_angle; ++i)
            {
              f1[2 + num_angle + 2 + i] = arc_2_x[num_angle - 1 - i];
            }
          arc_2_x.clear();

          f2.resize(4 + 2 * num_angle);
          /* line_1_y[0] and [1] */
          f2[0] = y_lim_min / max * sin(start_angle);
          f2[1] = rect * sin(start_angle);

          /*arc_1_y */
          listcomprehension(r, sin, phi_vec, num_angle, 2, f2);
          /* reversed line_2_y [0] and [1] */
          f2[2 + num_angle + 1] = y_lim_min / max * sin(end_angle);
          f2[2 + num_angle] = rect * sin(end_angle);
          /* reversed arc_2_y */
          if (keep_radii_axes)
            {
              listcomprehension(y_lim_min / max, sin, phi_vec, num_angle, 0, arc_2_y);
            }
          else
            {
              listcomprehension(0.0, sin, phi_vec, num_angle, 0, arc_2_y);
            }
          for (i = 0; i < num_angle; ++i)
            {
              f2[2 + num_angle + 2 + i] = arc_2_y[num_angle - 1 - i];
            }
          arc_2_y.clear();

          if (!draw_edges)
            {
              // with rlim gr_fillarc cant be used because it will always draw from the origin
              // instead use gr_fillarea and approximate line segment with calculations from above.
              if (del != del_values::update_without_default && del != del_values::update_with_default)
                {
                  area = global_render->createFillArea("x" + str, f1, "y" + str, f2);
                  area->setAttribute("_child_id", child_id++);
                  element->append(area);
                }
              else
                {
                  area = element->querySelectors("fill_area[_child_id=" + std::to_string(child_id++) + "]");
                  if (area != nullptr)
                    global_render->createFillArea("x" + str, f1, "y" + str, f2, nullptr, 0, 0, -1, area);
                }

              if (area != nullptr)
                {
                  global_render->setFillColorInd(area, face_color);
                  global_render->setFillIntStyle(area, 1);
                }
            }

          // draw_area more likely
          if (del != del_values::update_without_default && del != del_values::update_with_default)
            {
              area = global_render->createFillArea("x" + str, f1, "y" + str, f2);
              area->setAttribute("_child_id", child_id++);
              element->append(area);
            }
          else
            {
              area = element->querySelectors("fill_area[_child_id=" + std::to_string(child_id++) + "]");
              if (area != nullptr) global_render->createFillArea("x" + str, f1, "y" + str, f2, nullptr, 0, 0, -1, area);
            }
          if (area != nullptr)
            {
              global_render->setFillColorInd(area, edge_color);
              global_render->setFillIntStyle(area, 0);
              area->setAttribute("z_index", 2);
            }

          /* clean up vectors for next iteration */
          phi_vec.clear();
          f1.clear();
          f2.clear();
        }
    } /* end of ylims given case */
}


static void processScatter(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for scatter
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  std::string orientation = PLOT_DEFAULT_ORIENTATION;
  double c_min, c_max;
  unsigned int x_length, y_length, z_length, c_length;
  int i, c_index = -1;
  std::vector<int> marker_color_inds_vec;
  std::vector<double> marker_sizes_vec;
  std::vector<double> x_vec, y_vec, z_vec, c_vec;
  del_values del = del_values::update_without_default;
  int child_id = 0;
  std::shared_ptr<GRM::Element> marker;

  if (!element->hasAttribute("x")) throw NotFoundError("Scatter series is missing required attribute x-data.\n");
  auto x = static_cast<std::string>(element->getAttribute("x"));
  if (!element->hasAttribute("y")) throw NotFoundError("Scatter series is missing required attribute y-data.\n");
  auto y = static_cast<std::string>(element->getAttribute("y"));
  x_vec = GRM::get<std::vector<double>>((*context)[x]);
  y_vec = GRM::get<std::vector<double>>((*context)[y]);
  x_length = x_vec.size();
  y_length = y_vec.size();
  if (x_length != y_length) throw std::length_error("For scatter series x- and y-data must have the same size.\n");

  if (element->hasAttribute("z"))
    {
      auto z = static_cast<std::string>(element->getAttribute("z"));
      z_vec = GRM::get<std::vector<double>>((*context)[z]);
      z_length = z_vec.size();
      if (x_length != z_length) throw std::length_error("For scatter series x- and z-data must have the same size.\n");
    }
  if (element->hasAttribute("c"))
    {
      auto c = static_cast<std::string>(element->getAttribute("c"));
      c_vec = GRM::get<std::vector<double>>((*context)[c]);
      c_length = c_vec.size();
    }
  if (element->hasAttribute("orientation"))
    {
      orientation = static_cast<std::string>(element->getAttribute("orientation"));
    }
  else
    {
      element->setAttribute("orientation", orientation);
    }
  auto is_horizontal = orientation == "horizontal";

  if (!element->hasAttribute("marker_type"))
    {
      element->setAttribute("marker_type", *previous_scatter_marker_type++);
      if (*previous_scatter_marker_type == INT_MAX)
        {
          previous_scatter_marker_type = plot_scatter_markertypes;
        }
    }
  processMarkerType(element);

  if (c_vec.empty() && element->hasAttribute("color_ind"))
    {
      c_index = static_cast<int>(element->getAttribute("color_ind"));
      if (c_index < 0)
        {
          logger((stderr, "Invalid scatter color %d, using 0 instead\n", c_index));
          c_index = 0;
        }
      else if (c_index > 255)
        {
          logger((stderr, "Invalid scatter color %d, using 255 instead\n", c_index));
          c_index = 255;
        }
    }

  /* clear old marker */
  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  if (!z_vec.empty() || !c_vec.empty())
    {
      auto plot_parent = element->parentElement();
      getPlotParent(plot_parent);

      c_min = static_cast<double>(plot_parent->getAttribute("_c_lim_min"));
      c_max = static_cast<double>(plot_parent->getAttribute("_c_lim_max"));

      for (i = 0; i < x_length; i++)
        {
          if (!z_vec.empty())
            {
              if (i < z_length)
                {
                  marker_sizes_vec.push_back(z_vec[i]);
                }
              else
                {
                  marker_sizes_vec.push_back(2.0);
                }
            }
          if (!c_vec.empty())
            {
              if (i < c_length)
                {
                  c_index = 1000 + (int)(255.0 * (c_vec[i] - c_min) / (c_max - c_min) + 0.5);
                  if (c_index < 1000 || c_index > 1255)
                    {
                      // colorind -1000 will be skipped
                      marker_color_inds_vec.push_back(-1000);
                      continue;
                    }
                }
              else
                {
                  c_index = 989;
                }
              marker_color_inds_vec.push_back(c_index);
            }
          else if (c_index != -1)
            {
              marker_color_inds_vec.push_back(1000 + c_index);
            }
        }

      auto id = static_cast<int>(global_root->getAttribute("_id"));
      auto str = std::to_string(id);
      global_root->setAttribute("_id", ++id);

      if (is_horizontal)
        {
          if (del != del_values::update_without_default && del != del_values::update_with_default)
            {
              marker = global_render->createPolymarker("x" + str, x_vec, "y" + str, y_vec);
              marker->setAttribute("_child_id", child_id++);
              element->append(marker);
            }
          else
            {
              marker = element->querySelectors("polymarker[_child_id=" + std::to_string(child_id++) + "]");
              if (marker != nullptr)
                global_render->createPolymarker("x" + str, x_vec, "y" + str, y_vec, nullptr, 0, 0.0, 0, marker);
            }
        }
      else
        {
          if (del != del_values::update_without_default && del != del_values::update_with_default)
            {
              marker = global_render->createPolymarker("x" + str, y_vec, "y" + str, x_vec);
              marker->setAttribute("_child_id", child_id++);
              element->append(marker);
            }
          else
            {
              marker = element->querySelectors("polymarker[_child_id=" + std::to_string(child_id++) + "]");
              if (marker != nullptr)
                global_render->createPolymarker("x" + str, y_vec, "y" + str, x_vec, nullptr, 0, 0.0, 0, marker);
            }
        }

      if (!marker_sizes_vec.empty())
        {
          global_render->setMarkerSize(element, "marker_sizes" + str, marker_sizes_vec);
        }
      if (!marker_color_inds_vec.empty())
        {
          global_render->setMarkerColorInd(element, "marker_color_indices" + str, marker_color_inds_vec);
        }
    }
  else
    {
      auto id = static_cast<int>(global_root->getAttribute("_id"));
      auto str = std::to_string(id);

      if (is_horizontal)
        {
          if (del != del_values::update_without_default && del != del_values::update_with_default)
            {
              marker = global_render->createPolymarker("x" + str, x_vec, "y" + str, y_vec);
              marker->setAttribute("_child_id", child_id++);
              element->append(marker);
            }
          else
            {
              marker = element->querySelectors("polymarker[_child_id=" + std::to_string(child_id++) + "]");
              if (marker != nullptr)
                global_render->createPolymarker("x" + str, x_vec, "y" + str, y_vec, nullptr, 0, 0.0, 0, marker);
            }
        }
      else
        {
          if (del != del_values::update_without_default && del != del_values::update_with_default)
            {
              marker = global_render->createPolymarker("x" + str, y_vec, "y" + str, x_vec);
              marker->setAttribute("_child_id", child_id++);
              element->append(marker);
            }
          else
            {
              marker = element->querySelectors("polymarker[_child_id=" + std::to_string(child_id++) + "]");
              if (marker != nullptr)
                global_render->createPolymarker("x" + str, y_vec, "y" + str, x_vec, nullptr, 0, 0.0, 0, marker);
            }
        }
      global_root->setAttribute("_id", ++id);
    }

  // error_bar handling
  for (const auto &child : element->children())
    {
      if (child->localName() == "error_bars") extendErrorBars(child, context, x_vec, y_vec);
    }
}

static void processScatter3(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for scatter3
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double c_min, c_max;
  unsigned int x_length, y_length, z_length, c_length, i, c_index;
  std::vector<double> x_vec, y_vec, z_vec, c_vec;
  del_values del = del_values::update_without_default;
  int child_id = 0;
  std::shared_ptr<GRM::Element> marker;

  if (!element->hasAttribute("x")) throw NotFoundError("Scatter3 series is missing required attribute x-data.\n");
  auto x = static_cast<std::string>(element->getAttribute("x"));
  if (!element->hasAttribute("y")) throw NotFoundError("Scatter3 series is missing required attribute y-data.\n");
  auto y = static_cast<std::string>(element->getAttribute("y"));
  if (!element->hasAttribute("z")) throw NotFoundError("Scatter3 series is missing required attribute z-data.\n");
  auto z = static_cast<std::string>(element->getAttribute("z"));
  x_vec = GRM::get<std::vector<double>>((*context)[x]);
  y_vec = GRM::get<std::vector<double>>((*context)[y]);
  z_vec = GRM::get<std::vector<double>>((*context)[z]);
  x_length = x_vec.size();
  y_length = y_vec.size();
  z_length = z_vec.size();
  if (x_length != y_length || x_length != z_length)
    throw std::length_error("For scatter3 series x-, y- and z-data must have the same size.\n");

  std::vector<int> marker_c_vec;

  global_render->setMarkerType(element, GKS_K_MARKERTYPE_SOLID_CIRCLE);
  processMarkerType(element);
  if (element->hasAttribute("c"))
    {
      auto c = static_cast<std::string>(element->getAttribute("c"));
      c_vec = GRM::get<std::vector<double>>((*context)[c]);
      c_length = c_vec.size();
      auto plot_parent = element->parentElement();
      getPlotParent(plot_parent);

      c_min = static_cast<double>(plot_parent->getAttribute("_c_lim_min"));
      c_max = static_cast<double>(plot_parent->getAttribute("_c_lim_max"));

      for (i = 0; i < x_length; i++)
        {
          if (i < c_length)
            {
              c_index = 1000 + (int)(255.0 * (c_vec[i] - c_min) / (c_max - c_min) + 0.5);
            }
          else
            {
              c_index = 989;
            }
          marker_c_vec.push_back((int)c_index);
        }
    }

  auto id_int = static_cast<int>(global_root->getAttribute("_id"));
  global_root->setAttribute("_id", ++id_int);
  std::string id = std::to_string(id_int);

  if (!marker_c_vec.empty())
    {
      global_render->setMarkerColorInd(element, "marker_color_indices" + id, marker_c_vec);
    }
  else if (element->hasAttribute("color_ind"))
    {
      global_render->setMarkerColorInd(element, (int)c_index);
    }

  /* clear old marker */
  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  if (del != del_values::update_without_default && del != del_values::update_with_default)
    {
      marker = global_render->createPolymarker3d("x" + id, x_vec, "y" + id, y_vec, "z" + id, z_vec);
      marker->setAttribute("_child_id", child_id++);
      element->append(marker);
    }
  else
    {
      marker = element->querySelectors("polymarker_3d[_child_id=" + std::to_string(child_id++) + "]");
      if (marker != nullptr)
        global_render->createPolymarker3d("x" + id, x_vec, "y" + id, y_vec, "z" + id, z_vec, nullptr, marker);
    }
}

static void processStairs(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for stairs
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  std::string kind, orientation = PLOT_DEFAULT_ORIENTATION, line_spec = SERIES_DEFAULT_SPEC;
  bool is_vertical;
  unsigned int x_length, y_length, mask, i;
  std::vector<double> x_vec, y_vec;
  std::shared_ptr<GRM::Element> element_context = element, line;
  del_values del = del_values::update_without_default;
  int child_id = 0;

  if (element->parentElement()->parentElement()->hasAttribute("marginal_heatmap_side_plot"))
    element_context = element->parentElement()->parentElement()->parentElement();

  if (!element_context->hasAttribute("x")) throw NotFoundError("Stairs series is missing required attribute x-data.\n");
  auto x = static_cast<std::string>(element_context->getAttribute("x"));
  if (!element_context->hasAttribute("y")) throw NotFoundError("Stairs series is missing required attribute y-data.\n");
  auto y = static_cast<std::string>(element_context->getAttribute("y"));

  x_vec = GRM::get<std::vector<double>>((*context)[x]);
  y_vec = GRM::get<std::vector<double>>((*context)[y]);
  x_length = x_vec.size();
  y_length = y_vec.size();

  kind = static_cast<std::string>(element->getAttribute("kind"));
  if (element->hasAttribute("orientation"))
    {
      orientation = static_cast<std::string>(element->getAttribute("orientation"));
    }
  else
    {
      element->setAttribute("orientation", orientation);
    }
  if (element->hasAttribute("line_spec"))
    {
      line_spec = static_cast<std::string>(element->getAttribute("line_spec"));
    }
  else
    {
      element->setAttribute("line_spec", line_spec);
    }
  is_vertical = orientation == "vertical";

  auto id = static_cast<int>(global_root->getAttribute("_id"));
  auto str = std::to_string(id);
  global_root->setAttribute("_id", id + 1);

  /* clear old marker and lines */
  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  if (element->parentElement()->parentElement()->hasAttribute("marginal_heatmap_side_plot"))
    {
      double y_max = 0;
      int x_offset = 0, y_offset = 0;
      std::vector<double> z_vec;

      if (element_context->parentElement()->hasAttribute("x_log") &&
          static_cast<int>(element_context->parentElement()->getAttribute("x_log")))
        {
          for (i = 0; i < x_length; i++)
            {
              if (grm_isnan(x_vec[i])) x_offset += 1;
            }
        }
      if (element_context->parentElement()->hasAttribute("y_log") &&
          static_cast<int>(element_context->parentElement()->getAttribute("y_log")))
        {
          for (i = 0; i < y_length; i++)
            {
              if (grm_isnan(y_vec[i])) y_offset += 1;
            }
        }

      auto z = static_cast<std::string>(element_context->getAttribute("z"));
      z_vec = GRM::get<std::vector<double>>((*context)[z]);

      std::vector<double> xi_vec((is_vertical ? y_length - y_offset : x_length - x_offset));
      (*context)["xi" + str] = xi_vec;
      element->setAttribute("xi", "xi" + str);

      processMarginalHeatmapSidePlot(element->parentElement()->parentElement());
      processMarginalHeatmapKind(element_context);
    }
  else
    {
      if (x_length != y_length) throw std::length_error("For stairs series x- and y-data must have the same size.\n");

      const char *spec_char = line_spec.c_str();
      mask = gr_uselinespec((char *)spec_char);

      if (int_equals_any((int)mask, 5, 0, 1, 3, 4, 5))
        {
          std::string where = PLOT_DEFAULT_STEP_WHERE;
          if (element->hasAttribute("step_where"))
            {
              where = static_cast<std::string>(element->getAttribute("step_where"));
            }
          else
            {
              element->setAttribute("step_where", where);
            }
          if (where == "pre")
            {
              std::vector<double> x_step_boundaries(2 * x_length - 1);
              std::vector<double> y_step_values(2 * x_length - 1);

              x_step_boundaries[0] = x_vec[0];
              for (i = 1; i < 2 * x_length - 2; i += 2)
                {
                  x_step_boundaries[i] = x_vec[i / 2];
                  x_step_boundaries[i + 1] = x_vec[i / 2 + 1];
                }
              y_step_values[0] = y_vec[0];
              for (i = 1; i < 2 * x_length - 1; i += 2)
                {
                  y_step_values[i] = y_step_values[i + 1] = y_vec[i / 2 + 1];
                }

              std::vector<double> line_x = x_step_boundaries, line_y = y_step_values;
              if (is_vertical)
                {
                  line_x = y_step_values, line_y = x_step_boundaries;
                }
              if (del != del_values::update_without_default && del != del_values::update_with_default)
                {
                  line = global_render->createPolyline("x" + str, line_x, "y" + str, line_y);
                  line->setAttribute("_child_id", child_id++);
                  element->append(line);
                }
              else
                {
                  line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
                  if (line != nullptr)
                    global_render->createPolyline("x" + str, line_x, "y" + str, line_y, nullptr, 0, 0.0, 0, line);
                }
            }
          else if (where == "post")
            {
              std::vector<double> x_step_boundaries(2 * x_length - 1);
              std::vector<double> y_step_values(2 * x_length - 1);
              for (i = 0; i < 2 * x_length - 2; i += 2)
                {
                  x_step_boundaries[i] = x_vec[i / 2];
                  x_step_boundaries[i + 1] = x_vec[i / 2 + 1];
                }
              x_step_boundaries[2 * x_length - 2] = x_vec[x_length - 1];
              for (i = 0; i < 2 * x_length - 2; i += 2)
                {
                  y_step_values[i] = y_step_values[i + 1] = y_vec[i / 2];
                }
              y_step_values[2 * x_length - 2] = y_vec[x_length - 1];

              std::vector<double> line_x = x_step_boundaries, line_y = y_step_values;
              if (is_vertical)
                {
                  line_x = y_step_values, line_y = x_step_boundaries;
                }
              if (del != del_values::update_without_default && del != del_values::update_with_default)
                {
                  line = global_render->createPolyline("x" + str, line_x, "y" + str, line_y);
                  line->setAttribute("_child_id", child_id++);
                  element->append(line);
                }
              else
                {
                  line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
                  if (line != nullptr)
                    global_render->createPolyline("x" + str, line_x, "y" + str, line_y, nullptr, 0, 0.0, 0, line);
                }
            }
          else if (where == "mid")
            {
              std::vector<double> x_step_boundaries(2 * x_length);
              std::vector<double> y_step_values(2 * x_length);
              x_step_boundaries[0] = x_vec[0];
              for (i = 1; i < 2 * x_length - 2; i += 2)
                {
                  x_step_boundaries[i] = x_step_boundaries[i + 1] = (x_vec[i / 2] + x_vec[i / 2 + 1]) / 2.0;
                }
              x_step_boundaries[2 * x_length - 1] = x_vec[x_length - 1];
              for (i = 0; i < 2 * x_length - 1; i += 2)
                {
                  y_step_values[i] = y_step_values[i + 1] = y_vec[i / 2];
                }

              std::vector<double> line_x = x_step_boundaries, line_y = y_step_values;
              if (is_vertical)
                {
                  line_x = y_step_values, line_y = x_step_boundaries;
                }
              if (del != del_values::update_without_default && del != del_values::update_with_default)
                {
                  line = global_render->createPolyline("x" + str, line_x, "y" + str, line_y);
                  line->setAttribute("_child_id", child_id++);
                  element->append(line);
                }
              else
                {
                  line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
                  if (line != nullptr)
                    global_render->createPolyline("x" + str, line_x, "y" + str, line_y, nullptr, 0, 0.0, 0, line);
                }
            }
        }
      if (mask & 2)
        {
          std::vector<double> line_x = x_vec, line_y = y_vec;
          if (is_vertical)
            {
              line_x = y_vec, line_y = x_vec;
            }
          if (del != del_values::update_without_default && del != del_values::update_with_default)
            {
              line = global_render->createPolyline("x" + str, line_x, "y" + str, line_y);
              line->setAttribute("_child_id", child_id++);
              element->append(line);
            }
          else
            {
              line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
              if (line != nullptr)
                global_render->createPolyline("x" + str, line_x, "y" + str, line_y, nullptr, 0, 0.0, 0, line);
            }
        }
    }
}

static void processStem(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for stem
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double stem_x[2], stem_y[2] = {0.0};
  std::string orientation = PLOT_DEFAULT_ORIENTATION, line_spec = SERIES_DEFAULT_SPEC;
  bool is_vertical;
  double x_min, x_max, y_min, y_max;
  unsigned int x_length, y_length;
  unsigned int i;
  std::vector<double> x_vec, y_vec;
  del_values del = del_values::update_without_default;
  int child_id = 0;
  std::shared_ptr<GRM::Element> line, marker;
  double old_stem_y0;

  if (!element->hasAttribute("x")) throw NotFoundError("Stem series is missing required attribute x-data.\n");
  auto x = static_cast<std::string>(element->getAttribute("x"));
  if (!element->hasAttribute("y")) throw NotFoundError("Stem series is missing required attribute y-data.\n");
  auto y = static_cast<std::string>(element->getAttribute("y"));
  if (element->hasAttribute("orientation"))
    {
      orientation = static_cast<std::string>(element->getAttribute("orientation"));
    }
  else
    {
      element->setAttribute("orientation", orientation);
    }
  if (element->hasAttribute("line_spec"))
    {
      line_spec = static_cast<std::string>(element->getAttribute("line_spec"));
    }
  else
    {
      element->setAttribute("line_spec", line_spec);
    }

  is_vertical = orientation == "vertical";
  x_vec = GRM::get<std::vector<double>>((*context)[x]);
  y_vec = GRM::get<std::vector<double>>((*context)[y]);
  x_length = x_vec.size();
  y_length = y_vec.size();
  if (x_length != y_length) throw std::length_error("For stem series x- and y-data must have the same size.\n");

  if (element->hasAttribute("y_range_min")) stem_y[0] = static_cast<double>(element->getAttribute("y_range_min"));

  /* clear all old polylines and -marker */
  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  old_stem_y0 = stem_y[0];
  for (i = 0; i < x_length; ++i)
    {
      stem_x[0] = stem_x[1] = x_vec[i];
      stem_y[0] = old_stem_y0;
      stem_y[1] = y_vec[i];

      if (is_vertical)
        {
          double tmp1, tmp2;
          tmp1 = stem_x[0], tmp2 = stem_x[1];
          stem_x[0] = stem_y[0], stem_x[1] = stem_y[1];
          stem_y[0] = tmp1, stem_y[1] = tmp2;
        }
      if (del != del_values::update_without_default && del != del_values::update_with_default)
        {
          line = global_render->createPolyline(stem_x[0], stem_x[1], stem_y[0], stem_y[1]);
          line->setAttribute("_child_id", child_id++);
          element->append(line);
        }
      else
        {
          line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
          if (line != nullptr)
            global_render->createPolyline(stem_x[0], stem_x[1], stem_y[0], stem_y[1], 0, 0.0, 0, line);
        }
    }

  auto id = static_cast<int>(global_root->getAttribute("_id"));
  global_root->setAttribute("_id", id + 1);
  auto str = std::to_string(id);

  std::vector<double> marker_x = x_vec, marker_y = y_vec;
  if (is_vertical)
    {
      marker_x = y_vec, marker_y = x_vec;
    }
  if (del != del_values::update_without_default && del != del_values::update_with_default)
    {
      marker = global_render->createPolymarker("x" + str, marker_x, "y" + str, marker_y, nullptr,
                                               GKS_K_MARKERTYPE_SOLID_CIRCLE);
      marker->setAttribute("_child_id", child_id++);
      element->append(marker);
    }
  else
    {
      marker = element->querySelectors("polymarker[_child_id=" + std::to_string(child_id++) + "]");
      if (marker != nullptr)
        global_render->createPolymarker("x" + str, marker_x, "y" + str, marker_y, nullptr,
                                        GKS_K_MARKERTYPE_SOLID_CIRCLE, 0.0, 0, marker);
    }
}

static void processShade(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  int xform = 5, x_bins = 1200, y_bins = 1200, n;
  std::vector<double> x_vec, y_vec;
  double *x_p, *y_p;

  auto x_key = static_cast<std::string>(element->getAttribute("x"));
  auto y_key = static_cast<std::string>(element->getAttribute("y"));

  x_vec = GRM::get<std::vector<double>>((*context)[x_key]);
  y_vec = GRM::get<std::vector<double>>((*context)[y_key]);

  if (element->hasAttribute("transformation")) xform = static_cast<int>(element->getAttribute("transformation"));
  if (element->hasAttribute("x_bins")) x_bins = static_cast<int>(element->getAttribute("x_bins"));
  if (element->hasAttribute("y_bins")) y_bins = static_cast<int>(element->getAttribute("y_bins"));

  x_p = &(x_vec[0]);
  y_p = &(y_vec[0]);
  n = std::min<int>((int)x_vec.size(), (int)y_vec.size());
  applyMoveTransformation(element);

  if (redraw_ws) gr_shadepoints(n, x_p, y_p, xform, x_bins, y_bins);
}

static void processSurface(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for surface
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  int accelerate = PLOT_DEFAULT_ACCELERATE; /* this argument decides if GR3 or GR is used to plot the surface */
  std::vector<double> x_vec, y_vec, z_vec;
  unsigned int x_length, y_length, z_length;
  double x_min, x_max, y_min, y_max;

  if (element->hasAttribute("accelerate"))
    {
      accelerate = static_cast<int>(element->getAttribute("accelerate"));
    }
  else
    {
      element->setAttribute("accelerate", accelerate);
    }

  if (element->hasAttribute("x"))
    {
      auto x = static_cast<std::string>(element->getAttribute("x"));
      x_vec = GRM::get<std::vector<double>>((*context)[x]);
      x_length = x_vec.size();
    }
  if (element->hasAttribute("y"))
    {
      auto y = static_cast<std::string>(element->getAttribute("y"));
      y_vec = GRM::get<std::vector<double>>((*context)[y]);
      y_length = y_vec.size();
    }

  if (!element->hasAttribute("z")) throw NotFoundError("Surface series is missing required attribute z-data.\n");

  auto z = static_cast<std::string>(element->getAttribute("z"));
  z_vec = GRM::get<std::vector<double>>((*context)[z]);
  z_length = z_vec.size();

  if (x_vec.empty() && y_vec.empty())
    {
      /* If neither `x` nor `y` are given, we need more information about the shape of `z` */
      if (!element->hasAttribute("z_dims"))
        throw NotFoundError("Surface series is missing required attribute zdims.\n");
      auto z_dims_key = static_cast<std::string>(element->getAttribute("z_dims"));
      auto z_dims_vec = GRM::get<std::vector<int>>((*context)[z_dims_key]);
      x_length = z_dims_vec[0];
      y_length = z_dims_vec[1];
    }
  else if (x_vec.empty())
    {
      x_length = z_length / y_length;
    }
  else if (y_vec.empty())
    {
      y_length = z_length / x_length;
    }

  if (x_vec.empty())
    {
      x_min = static_cast<double>(element->getAttribute("x_range_min"));
      x_max = static_cast<double>(element->getAttribute("x_range_max"));
    }
  else
    {
      for (int j = 0; j < x_length; j++)
        {
          if (!grm_isnan(x_vec[j]))
            {
              x_min = x_vec[j];
              break;
            }
        }
      x_max = x_vec[x_length - 1];
    }
  if (y_vec.empty())
    {
      y_min = static_cast<double>(element->getAttribute("y_range_min"));
      y_max = static_cast<double>(element->getAttribute("y_range_max"));
    }
  else
    {
      for (int j = 0; j < y_length; j++)
        {
          if (!grm_isnan(y_vec[j]))
            {
              y_min = y_vec[j];
              break;
            }
        }
      y_max = y_vec[y_length - 1];
    }

  if (x_vec.empty())
    {
      std::vector<double> tmp(x_length);
      for (int j = 0; j < x_length; ++j)
        {
          tmp[j] = (int)(x_min + (x_max - x_min) / x_length * j + 0.5);
        }
      x_vec = tmp;
    }
  if (y_vec.empty())
    {
      std::vector<double> tmp(y_length);
      for (int j = 0; j < y_length; ++j)
        {
          tmp[j] = (int)(y_min + (y_max - y_min) / y_length * j + 0.5);
        }
      y_vec = tmp;
    }

  if (x_length == y_length && x_length == z_length)
    {
      logger((stderr, "Create a %d x %d grid for \"surface\" with \"gridit\"\n", PLOT_SURFACE_GRIDIT_N,
              PLOT_SURFACE_GRIDIT_N));

      std::vector<double> gridit_x_vec(PLOT_SURFACE_GRIDIT_N);
      std::vector<double> gridit_y_vec(PLOT_SURFACE_GRIDIT_N);
      std::vector<double> gridit_z_vec(PLOT_SURFACE_GRIDIT_N * PLOT_SURFACE_GRIDIT_N);

      double *gridit_x = &(gridit_x_vec[0]);
      double *gridit_y = &(gridit_y_vec[0]);
      double *gridit_z = &(gridit_z_vec[0]);
      double *x_p = &(x_vec[0]);
      double *y_p = &(y_vec[0]);
      double *z_p = &(z_vec[0]);
      gr_gridit((int)x_length, x_p, y_p, z_p, PLOT_SURFACE_GRIDIT_N, PLOT_SURFACE_GRIDIT_N, gridit_x, gridit_y,
                gridit_z);

      x_vec = std::vector<double>(gridit_x, gridit_x + PLOT_SURFACE_GRIDIT_N);
      y_vec = std::vector<double>(gridit_y, gridit_y + PLOT_SURFACE_GRIDIT_N);
      z_vec = std::vector<double>(gridit_z, gridit_z + PLOT_SURFACE_GRIDIT_N * PLOT_SURFACE_GRIDIT_N);

      x_length = y_length = PLOT_SURFACE_GRIDIT_N;
    }
  else
    {
      logger((stderr, "x_length; %u, y_length: %u, z_length: %u\n", x_length, y_length, z_length));
      if (x_length * y_length != z_length)
        throw std::length_error("For surface series x_length * y_length must be z_length.\n");
    }

  applyMoveTransformation(element);
  if (!accelerate)
    {
      double *px_p = &(x_vec[0]);
      double *py_p = &(y_vec[0]);
      double *pz_p = &(z_vec[0]);

      if (redraw_ws) gr_surface((int)x_length, (int)y_length, px_p, py_p, pz_p, GR_OPTION_COLORED_MESH);
    }
  else
    {
      std::vector<float> px_vec_f(x_vec.begin(), x_vec.end());
      std::vector<float> py_vec_f(y_vec.begin(), y_vec.end());
      std::vector<float> pz_vec_f(z_vec.begin(), z_vec.end());

      float *px_p = &(px_vec_f[0]);
      float *py_p = &(py_vec_f[0]);
      float *pz_p = &(pz_vec_f[0]);

      if (redraw_ws) gr3_surface((int)x_length, (int)y_length, px_p, py_p, pz_p, GR_OPTION_COLORED_MESH);
    }
}

static void processLine(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for line
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  std::string orientation = PLOT_DEFAULT_ORIENTATION, line_spec = SERIES_DEFAULT_SPEC;
  std::vector<double> x_vec, y_vec;
  unsigned int x_length = 0, y_length = 0;
  del_values del = del_values::update_without_default;
  int child_id = 0;
  std::shared_ptr<GRM::Element> line, marker;
  int mask;

  if (element->hasAttribute("orientation"))
    {
      orientation = static_cast<std::string>(element->getAttribute("orientation"));
    }
  else
    {
      element->setAttribute("orientation", orientation);
    }

  if (!element->hasAttribute("y")) throw NotFoundError("Line series is missing required attribute y-data.\n");
  auto y = static_cast<std::string>(element->getAttribute("y"));
  y_vec = GRM::get<std::vector<double>>((*context)[y]);
  y_length = y_vec.size();

  if (!element->hasAttribute("x"))
    {
      x_length = y_length;
      for (int i = 0; i < y_length; ++i) /* julia starts with 1, so GRM starts with 1 to be consistent */
        {
          x_vec.push_back(i + 1);
        }
    }
  else
    {
      auto x = static_cast<std::string>(element->getAttribute("x"));
      x_vec = GRM::get<std::vector<double>>((*context)[x]);
      x_length = x_vec.size();
    }
  if (x_length != y_length) throw std::length_error("For line series x- and y-data must have the same size.\n");

  if (element->hasAttribute("line_spec"))
    {
      line_spec = static_cast<std::string>(element->getAttribute("line_spec"));
    }
  else
    {
      element->setAttribute("line_spec", line_spec);
    }
  const char *spec_char = line_spec.c_str();
  mask = gr_uselinespec((char *)spec_char);

  /* clear old line */
  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  if (int_equals_any(mask, 5, 0, 1, 3, 4, 5))
    {
      int current_line_color_ind;
      gr_inqlinecolorind(&current_line_color_ind);
      auto id = static_cast<int>(global_root->getAttribute("_id"));
      auto str = std::to_string(id);

      std::vector<double> line_x = x_vec, line_y = y_vec;
      if (orientation == "vertical")
        {
          line_x = y_vec, line_y = x_vec;
        }
      if (del != del_values::update_without_default && del != del_values::update_with_default)
        {
          line = global_render->createPolyline("x" + str, line_x, "y" + str, line_y);
          line->setAttribute("_child_id", child_id++);
          element->append(line);
        }
      else
        {
          line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
          if (line != nullptr)
            global_render->createPolyline("x" + str, line_x, "y" + str, line_y, nullptr, 0, 0.0, 0, line);
        }

      global_root->setAttribute("_id", ++id);
      if (line != nullptr) line->setAttribute("line_color_ind", current_line_color_ind);
    }
  if (mask & 2)
    {
      int current_marker_color_ind;
      gr_inqmarkercolorind(&current_marker_color_ind);
      auto id = static_cast<int>(global_root->getAttribute("_id"));
      auto str = std::to_string(id);

      std::vector<double> marker_x = x_vec, marker_y = y_vec;
      if (orientation == "vertical")
        {
          marker_x = y_vec, marker_y = x_vec;
        }
      if (del != del_values::update_without_default && del != del_values::update_with_default)
        {
          marker = global_render->createPolymarker("x" + str, marker_x, "y" + str, marker_y);
          marker->setAttribute("_child_id", child_id++);
          element->append(marker);
        }
      else
        {
          marker = element->querySelectors("polymarker[_child_id=" + std::to_string(child_id++) + "]");
          if (marker != nullptr)
            global_render->createPolymarker("x" + str, marker_x, "y" + str, marker_y, nullptr, 0, 0.0, 0, marker);
        }

      if (marker != nullptr)
        {
          marker->setAttribute("marker_color_ind", current_marker_color_ind);
          marker->setAttribute("z_index", 2);

          if (element->hasAttribute("marker_type"))
            {
              marker->setAttribute("marker_type", static_cast<int>(element->getAttribute("marker_type")));
            }
          else
            {
              marker->setAttribute("marker_type", *previous_line_marker_type++);
              if (*previous_line_marker_type == INT_MAX)
                {
                  previous_line_marker_type = plot_scatter_markertypes;
                }
            }
        }
      global_root->setAttribute("_id", ++id);
    }

  // error_bar handling
  for (const auto &child : element->children())
    {
      if (child->localName() == "error_bars") extendErrorBars(child, context, x_vec, y_vec);
    }
}

static void processMarginalHeatmapPlot(const std::shared_ptr<GRM::Element> &element,
                                       const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for marginal heatmap
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */

  double c_max;
  int flip, options;
  int x_ind = PLOT_DEFAULT_MARGINAL_INDEX, y_ind = PLOT_DEFAULT_MARGINAL_INDEX;
  unsigned int i, j, k;
  std::string algorithm = PLOT_DEFAULT_MARGINAL_ALGORITHM, marginal_heatmap_kind = PLOT_DEFAULT_MARGINAL_KIND;
  std::vector<double> bins;
  unsigned int num_bins_x = 0, num_bins_y = 0;
  std::shared_ptr<GRM::Element> sub_group, central_region, side_region;
  auto plot_parent = element;
  del_values del = del_values::update_without_default;
  int child_id = 0;
  bool z_log = false;
  int x_offset = 0, y_offset = 0;

  getPlotParent(plot_parent);
  for (const auto &children : element->children())
    {
      if (children->localName() == "central_region")
        {
          central_region = children;
          break;
        }
    }

  z_log = static_cast<int>(plot_parent->getAttribute("z_log"));

  if (element->hasAttribute("x_ind"))
    {
      x_ind = static_cast<int>(element->getAttribute("x_ind"));
    }
  else
    {
      element->setAttribute("x_ind", x_ind);
    }
  if (element->hasAttribute("y_ind"))
    {
      y_ind = static_cast<int>(element->getAttribute("y_ind"));
    }
  else
    {
      element->setAttribute("y_ind", y_ind);
    }
  if (element->hasAttribute("algorithm"))
    {
      algorithm = static_cast<std::string>(element->getAttribute("algorithm"));
    }
  else
    {
      element->setAttribute("algorithm", algorithm);
    }
  if (element->hasAttribute("marginal_heatmap_kind"))
    {
      marginal_heatmap_kind = static_cast<std::string>(element->getAttribute("marginal_heatmap_kind"));
    }
  else
    {
      element->setAttribute("marginal_heatmap_kind", marginal_heatmap_kind);
    }

  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto x_vec = GRM::get<std::vector<double>>((*context)[x]);
  num_bins_x = x_vec.size();
  if (plot_parent->hasAttribute("x_log") && static_cast<int>(plot_parent->getAttribute("x_log")))
    {
      for (i = 0; i < num_bins_x; i++)
        {
          if (grm_isnan(x_vec[i])) x_offset += 1;
        }
    }
  num_bins_x -= x_offset;

  auto y = static_cast<std::string>(element->getAttribute("y"));
  auto y_vec = GRM::get<std::vector<double>>((*context)[y]);
  num_bins_y = y_vec.size();
  if (plot_parent->hasAttribute("y_log") && static_cast<int>(plot_parent->getAttribute("y_log")))
    {
      for (i = 0; i < num_bins_y; i++)
        {
          if (grm_isnan(y_vec[i])) y_offset += 1;
        }
    }
  num_bins_y -= y_offset;

  auto plot = static_cast<std::string>(element->getAttribute("z"));
  auto plot_vec = GRM::get<std::vector<double>>((*context)[plot]);

  /* clear old child nodes */
  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  auto id = static_cast<int>(global_root->getAttribute("_id"));
  auto str = std::to_string(id);

  std::shared_ptr<GRM::Element> heatmap;
  if (del != del_values::update_without_default && del != del_values::update_with_default)
    {
      heatmap = global_render->createSeries("heatmap");
      heatmap->setAttribute("_child_id", child_id++);
      central_region->append(heatmap);
    }
  else
    {
      heatmap = element->querySelectors("series_heatmap[_child_id=" + std::to_string(child_id++) + "]");
    }

  // for validation
  if (heatmap != nullptr)
    {
      (*context)["x" + str] = x_vec;
      heatmap->setAttribute("x", "x" + str);
      (*context)["y" + str] = y_vec;
      heatmap->setAttribute("y", "y" + str);
      (*context)["z" + str] = plot_vec;
      heatmap->setAttribute("z", "z" + str);
    }

  global_root->setAttribute("_id", ++id);

  for (k = 0; k < 2; k++)
    {
      double value, bin_max = 0;
      int bar_color_index = 989;
      double bar_color_rgb[3] = {-1};
      int edge_color_index = 1;
      double edge_color_rgb[3] = {-1};

      id = static_cast<int>(global_root->getAttribute("_id"));
      str = std::to_string(id);

      if (!std::isnan(static_cast<double>(plot_parent->getAttribute("_c_lim_max"))))
        {
          c_max = static_cast<double>(plot_parent->getAttribute("_c_lim_max"));
        }
      else
        {
          c_max = static_cast<double>(plot_parent->getAttribute("_z_lim_max"));
        }

      if (marginal_heatmap_kind == "all")
        {
          unsigned int x_len = num_bins_x, y_len = num_bins_y;

          bins = std::vector<double>((k == 0) ? num_bins_y : num_bins_x);

          for (i = 0; i < ((k == 0) ? num_bins_y : num_bins_x); i++)
            {
              bins[i] = 0;
            }
          for (i = 0; i < y_len; i++)
            {
              for (j = 0; j < x_len; j++)
                {
                  if (z_log)
                    {
                      value = (grm_isnan(log10(plot_vec[(i + y_offset) * (num_bins_x + x_offset) + (j + x_offset)])))
                                  ? 0
                                  : log10(plot_vec[(i + y_offset) * (num_bins_x + x_offset) + (j + x_offset)]);
                    }
                  else
                    {
                      value = (grm_isnan(plot_vec[(i + y_offset) * (num_bins_x + x_offset) + (j + x_offset)]))
                                  ? 0
                                  : plot_vec[(i + y_offset) * (num_bins_x + x_offset) + (j + x_offset)];
                    }
                  if (algorithm == "sum")
                    {
                      bins[(k == 0) ? i : j] += value;
                    }
                  else if (algorithm == "max")
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

          side_region = element->querySelectors("side_region[location=\"" +
                                                (k == 0 ? std::string("right") : std::string("top")) + "\"]");
          if (del != del_values::update_without_default && del != del_values::update_with_default)
            {
              sub_group = global_render->createSeries("hist");
              sub_group->setAttribute("_child_id", child_id++);
              auto side_plot_region = global_render->createSidePlotRegion();
              side_region->append(side_plot_region);
              side_plot_region->append(sub_group);
            }
          else
            {
              sub_group = side_region->querySelectors("series_hist[_child_id=\"" + std::to_string(child_id++) + "\"]");
              sub_group->setAttribute("_update_required", true);
            }
          if (side_region != nullptr)
            {
              side_region->setAttribute("marginal_heatmap_side_plot", true);
            }

          if (sub_group != nullptr)
            {
              std::vector<double> bar_color_rgb_vec(bar_color_rgb, bar_color_rgb + 3);
              (*context)["fill_color_rgb" + str] = bar_color_rgb_vec;
              sub_group->setAttribute("fill_color_rgb", "fill_color_rgb" + str);
              sub_group->setAttribute("fill_color_ind", bar_color_index);

              std::vector<double> edge_color_rgb_vec(edge_color_rgb, edge_color_rgb + 3);
              (*context)["line_color_rgb" + str] = edge_color_rgb_vec;
              sub_group->setAttribute("line_color_rgb", "line_color_rgb" + str);
              sub_group->setAttribute("line_color_ind", edge_color_index);

              (*context)["bins" + str] = bins;
              sub_group->setAttribute("bins", "bins" + str);

              (*context)["x" + str] = x_vec;
              sub_group->setAttribute("x", "x" + str);
            }
        }
      else if (marginal_heatmap_kind == "line" && x_ind != -1 && y_ind != -1)
        {
          side_region = element->querySelectors("side_region[location=\"" +
                                                (k == 0 ? std::string("right") : std::string("top")) + "\"]");
          if (side_region != nullptr)
            {
              side_region->setAttribute("marginal_heatmap_side_plot", true);
            }
          // special case for marginal_heatmap_kind line - when new indices != -1 are received the 2 lines should be
          // displayed
          sub_group = side_region->querySelectors("series_stairs[_child_id=0]");
          auto side_plot_region = side_region->querySelectors("side_plot_region");
          if ((del != del_values::update_without_default && del != del_values::update_with_default) ||
              (sub_group == nullptr && static_cast<int>(element->getAttribute("_update_required"))))
            {
              sub_group = global_render->createSeries("stairs");
              sub_group->setAttribute("_child_id", child_id++);
              if (!side_plot_region) side_plot_region = global_render->createSidePlotRegion();
              side_region->append(side_plot_region);
              side_plot_region->append(sub_group);
            }
          else
            {
              child_id++;
            }

          if (sub_group != nullptr)
            {
              sub_group->setAttribute("line_spec", SERIES_DEFAULT_SPEC);

              // for validation
              (*context)["y" + str] = y_vec;
              sub_group->setAttribute("y", "y" + str);

              (*context)["x" + str] = x_vec;
              sub_group->setAttribute("x", "x" + str);
            }
        }

      if (marginal_heatmap_kind == "all" || (marginal_heatmap_kind == "line" && x_ind != -1 && y_ind != -1))
        {
          if (sub_group != nullptr)
            {
              if (k == 0)
                {
                  sub_group->setAttribute("orientation", "vertical");
                }
              else
                {
                  sub_group->setAttribute("orientation", "horizontal");
                }
            }
        }

      auto tmp = element->querySelectorsAll("series_hist");
      for (const auto &child : tmp)
        {
          if (!child->parentElement()->parentElement()->hasAttribute("marginal_heatmap_side_plot")) continue;
          if (static_cast<std::string>(child->getAttribute("kind")) == "hist" ||
              static_cast<std::string>(child->getAttribute("kind")) == "stairs")
            {
              child->parentElement()->setAttribute("y_flip", 0);
              child->parentElement()->setAttribute("x_flip", 0);
              if (static_cast<int>(child->getAttribute("_child_id")) == 1)
                {
                  child->parentElement()->setAttribute("x_flip", 0);
                  if (static_cast<int>(plot_parent->getAttribute("y_flip")) == 1)
                    child->parentElement()->setAttribute("y_flip", 1);
                  processFlip(child->parentElement());
                }
              if (static_cast<int>(child->getAttribute("_child_id")) == 2)
                {
                  if (static_cast<int>(plot_parent->getAttribute("x_flip")) == 1)
                    child->parentElement()->setAttribute("x_flip", 1);
                  child->parentElement()->setAttribute("y_flip", 0);
                  processFlip(child->parentElement());
                }
            }
        }
      global_root->setAttribute("_id", ++id);
      processFlip(plot_parent);
    }
}

/*!
 * \brief Set colors from color index or rgb arrays. The render version
 *
 * Call the function first with an argument container and a key. Afterwards, call the `set_next_color` with `nullptr`
 * pointers to iterate through the color arrays. If `key` does not exist in `args`, the function falls back to default
 * colors.
 *
 * \param key The key of the colors in the argument container. The key may reference integer or double arrays. Integer
 * arrays describe colors of the GKS color table (0 - 1255). Double arrays contain RGB tuples in the range [0.0, 1.0].
 * If key does not exist, the routine falls back to default colors (taken from `gr_uselinespec`). \param color_type
 * The color type to set. Can be one of `GR_COLOR_LINE`, `GR_COLOR_MARKER`, `GR_COLOR_FILL`, `GR_COLOR_TEXT`,
 * `GR_COLOR_BORDER` or any combination of them (combined with OR). The special value `GR_COLOR_RESET` resets all
 * color modifications.
 */
static int set_next_color(const std::string &key, gr_color_type_t color_type,
                          const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  std::vector<int> fallback_color_indices = {989, 982, 980, 981, 996, 983, 995, 988, 986, 990,
                                             991, 984, 992, 993, 994, 987, 985, 997, 998, 999};
  static double saved_color[3];
  static int last_array_index = -1;
  static std::vector<int> color_indices;
  static std::vector<double> color_rgb_values;
  static unsigned int color_array_length = -1;
  int current_array_index = last_array_index + 1;
  int color_index = 0;
  int reset = (color_type == GR_COLOR_RESET);
  int gks_err_ind = GKS_K_NO_ERROR;

  if (reset || !key.empty())
    {
      if (last_array_index >= 0 && !color_rgb_values.empty())
        {
          gr_setcolorrep(PLOT_CUSTOM_COLOR_INDEX, saved_color[0], saved_color[1], saved_color[2]);
        }
      last_array_index = -1;
      if (!reset && !key.empty())
        {
          if (!element->hasAttribute("color_ind_values") && !element->hasAttribute("color_rgb_values"))
            {
              /* use fallback colors if `key` cannot be read from `args` */
              logger((stderr, "Cannot read \"%s\" from args, falling back to default colors\n", key.c_str()));
              color_indices = fallback_color_indices;
              color_array_length = size(fallback_color_indices);
            }
          else
            {
              if (element->hasAttribute("color_ind_values"))
                {
                  auto c = static_cast<std::string>(element->getAttribute("color_ind_values"));
                  color_indices = GRM::get<std::vector<int>>((*context)[c]);
                  color_array_length = color_indices.size();
                }
              else if (element->hasAttribute("color_rgb_values"))
                {
                  auto c = static_cast<std::string>(element->getAttribute("color_rgb_values"));
                  color_rgb_values = GRM::get<std::vector<double>>((*context)[c]);
                  color_array_length = color_rgb_values.size();
                }
            }
        }
      else
        {
          color_array_length = -1;
        }

      if (reset)
        {
          color_indices.clear();
          color_rgb_values.clear();
          return 0;
        }
      return 0;
    }

  if (last_array_index < 0 && !color_rgb_values.empty())
    {
      gks_inq_color_rep(1, PLOT_CUSTOM_COLOR_INDEX, GKS_K_VALUE_SET, &gks_err_ind, &saved_color[0], &saved_color[1],
                        &saved_color[2]);
    }

  current_array_index %= color_array_length;

  if (!color_indices.empty())
    {
      color_index = color_indices[current_array_index];
      last_array_index = current_array_index;
    }
  else if (!color_rgb_values.empty())
    {
      color_index = PLOT_CUSTOM_COLOR_INDEX;
      last_array_index = current_array_index + 2;
      global_render->setColorRep(element, PLOT_CUSTOM_COLOR_INDEX, color_rgb_values[current_array_index],
                                 color_rgb_values[current_array_index + 1], color_rgb_values[current_array_index + 2]);
    }

  if (color_type & GR_COLOR_LINE)
    {
      global_render->setLineColorInd(element, color_index);
    }
  if (color_type & GR_COLOR_MARKER)
    {
      global_render->setMarkerColorInd(element, color_index);
    }
  if (color_type & GR_COLOR_FILL)
    {
      global_render->setFillColorInd(element, color_index);
    }
  if (color_type & GR_COLOR_TEXT)
    {
      global_render->setTextColorInd(element, color_index);
    }
  if (color_type & GR_COLOR_BORDER)
    {
      global_render->setBorderColorInd(element, color_index);
    }
  return color_index;
}

static void processPieSegment(const std::shared_ptr<GRM::Element> &element,
                              const std::shared_ptr<GRM::Context> &context)
{
  double start_angle, end_angle, middle_angle;
  del_values del = del_values::update_without_default;
  int child_id = 0;
  std::shared_ptr<GRM::Element> arc, text_elem;
  double text_pos[2];
  std::string text;

  /* clear old child nodes */
  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  start_angle = static_cast<double>(element->getAttribute("start_angle"));
  end_angle = static_cast<double>(element->getAttribute("end_angle"));
  text = static_cast<std::string>(element->getAttribute("text"));

  if (del != del_values::update_without_default && del != del_values::update_with_default)
    {
      arc = global_render->createFillArc(0.035, 0.965, 0.07, 1.0, start_angle, end_angle);
      arc->setAttribute("_child_id", child_id++);
      element->append(arc);
    }
  else
    {
      arc = element->querySelectors("fill_arc[_child_id=" + std::to_string(child_id++) + "]");
      if (arc != nullptr) global_render->createFillArc(0.035, 0.965, 0.07, 1.0, start_angle, end_angle, 0, 0, -1, arc);
    }

  middle_angle = (start_angle + end_angle) / 2.0;

  text_pos[0] = 0.5 + 0.25 * cos(middle_angle * M_PI / 180.0);
  text_pos[1] = 0.5 + 0.25 * sin(middle_angle * M_PI / 180.0);

  if (del != del_values::update_without_default && del != del_values::update_with_default)
    {
      text_elem = global_render->createText(text_pos[0], text_pos[1], text, CoordinateSpace::WC);
      text_elem->setAttribute("_child_id", child_id++);
      element->append(text_elem);
    }
  else
    {
      text_elem = element->querySelectors("text[_child_id=" + std::to_string(child_id++) + "]");
      if (text_elem != nullptr)
        global_render->createText(text_pos[0], text_pos[1], text, CoordinateSpace::WC, text_elem);
    }
  if (text_elem != nullptr)
    {
      text_elem->setAttribute("z_index", 2);
      text_elem->setAttribute("set_text_color_for_background", true);
      processTextColorForBackground(text_elem);
    }
}

static void processPie(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for pie
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  unsigned int x_length;
  int color_index;
  double start_angle, middle_angle, end_angle;
  double text_pos[2];
  char text[80];
  std::string title;
  unsigned int i;
  del_values del = del_values::update_without_default;
  int child_id = 0;
  std::shared_ptr<GRM::Element> pie_segment;

  global_render->setFillIntStyle(element, GKS_K_INTSTYLE_SOLID);
  global_render->setTextAlign(element, GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_HALF);

  if (!element->hasAttribute("x")) throw NotFoundError("Pie series is missing required attribute x-data.\n");
  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto x_vec = GRM::get<std::vector<double>>((*context)[x]);
  x_length = x_vec.size();

  std::vector<double> normalized_x(x_length);
  GRM::normalize_vec(x_vec, &normalized_x);
  std::vector<unsigned int> normalized_x_int(x_length);
  GRM::normalize_vec_int(x_vec, &normalized_x_int, 1000);

  start_angle = 90;
  color_index = set_next_color("c", GR_COLOR_FILL, element, context); // key doesn't matter as long as it's not empty

  /* clear old pie_segments */
  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  for (i = 0; i < x_length; ++i)
    {
      end_angle = start_angle - normalized_x[i] * 360.0;

      snprintf(text, 80, "%.2lf\n%.1lf %%", x_vec[i], normalized_x_int[i] / 10.0);
      if (del != del_values::update_without_default && del != del_values::update_with_default)
        {
          pie_segment = global_render->createPieSegment(start_angle, end_angle, text, color_index);
          pie_segment->setAttribute("_child_id", child_id++);
          element->append(pie_segment);
        }
      else
        {
          pie_segment = element->querySelectors("pie_segment[_child_id=" + std::to_string(child_id++) + "]");
          if (pie_segment != nullptr)
            global_render->createPieSegment(start_angle, end_angle, text, color_index, pie_segment);
        }
      if (pie_segment != nullptr)
        {
          color_index = set_next_color("", GR_COLOR_FILL, pie_segment, context);
          processFillColorInd(pie_segment);
        }

      start_angle = end_angle;
      if (start_angle < 0)
        {
          start_angle += 360.0;
        }
    }
  set_next_color("", GR_COLOR_RESET, element, context);
  processFillColorInd(element);
  processFillIntStyle(element);
  processTextAlign(element);
}

static void processPlot3(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for plot3
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  unsigned int x_length, y_length, z_length;
  del_values del = del_values::update_without_default;
  int child_id = 0;

  if (!element->hasAttribute("x")) throw NotFoundError("Plot3 series is missing required attribute x-data.\n");
  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto x_vec = GRM::get<std::vector<double>>((*context)[x]);
  x_length = x_vec.size();

  if (!element->hasAttribute("y")) throw NotFoundError("Plot3 series is missing required attribute y-data.\n");
  auto y = static_cast<std::string>(element->getAttribute("y"));
  auto y_vec = GRM::get<std::vector<double>>((*context)[y]);
  y_length = y_vec.size();

  if (!element->hasAttribute("z")) throw NotFoundError("Plot3 series is missing required attribute z-data.\n");
  auto z = static_cast<std::string>(element->getAttribute("z"));
  auto z_vec = GRM::get<std::vector<double>>((*context)[z]);
  z_length = z_vec.size();

  if (x_length != y_length || x_length != z_length)
    throw std::length_error("For plot3 series x-, y- and z-data must have the same size.\n");

  /* clear old line */
  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  auto id_int = static_cast<int>(global_root->getAttribute("_id"));
  global_root->setAttribute("_id", ++id_int);
  std::string id = std::to_string(id_int);

  std::shared_ptr<GRM::Element> line;
  if (del != del_values::update_without_default && del != del_values::update_with_default)
    {
      line = global_render->createPolyline3d("x" + id, x_vec, "y" + id, y_vec, "z" + id, z_vec);
      line->setAttribute("_child_id", child_id++);
      element->append(line);
    }
  else
    {
      line = element->querySelectors("polyline_3d[_child_id=" + std::to_string(child_id++) + "]");
      if (line != nullptr)
        global_render->createPolyline3d("x" + id, x_vec, "y" + id, y_vec, "z" + id, z_vec, nullptr, line);
    }
}

static void processImshow(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for imshow
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double z_min, z_max;
  unsigned int dims, z_data_length, i, j, k;
  bool grplot = false;
  int rows, cols;
  auto plot_parent = element->parentElement();
  del_values del = del_values::update_without_default;
  int child_id = 0;
  std::vector<double> z_data_vec;
  std::vector<int> z_dims_vec;

  getPlotParent(plot_parent);
  if (plot_parent->hasAttribute("grplot"))
    {
      grplot = static_cast<int>(plot_parent->getAttribute("grplot"));
    }
  if (std::isnan(static_cast<double>(element->getAttribute("z_range_min"))))
    throw NotFoundError("Imshow series is missing required attribute z_range.\n");
  z_min = static_cast<double>(element->getAttribute("z_range_min"));
  if (std::isnan(static_cast<double>(element->getAttribute("z_range_max"))))
    throw NotFoundError("Imshow series is missing required attribute z_range.\n");
  z_max = static_cast<double>(element->getAttribute("z_range_max"));
  logger((stderr, "Got min, max %lf %lf\n", z_min, z_max));

  if (!element->hasAttribute("z")) throw NotFoundError("Imshow series is missing required attribute z-data.\n");
  auto z_key = static_cast<std::string>(element->getAttribute("z"));
  if (!element->hasAttribute("z_dims"))
    throw NotFoundError("Imshow series is missing required attribute z_dims-data.\n");
  auto z_dims_key = static_cast<std::string>(element->getAttribute("z_dims"));

  z_data_vec = GRM::get<std::vector<double>>((*context)[z_key]);
  z_dims_vec = GRM::get<std::vector<int>>((*context)[z_dims_key]);
  z_data_length = z_data_vec.size();
  dims = z_dims_vec.size();
  if (dims != 2) throw std::length_error("The size of dims data from imshow has to be 2.\n");
  if (z_dims_vec[0] * z_dims_vec[1] != z_data_length)
    throw std::length_error("For imshow shape[0] * shape[1] must be z-data length.\n");

  cols = z_dims_vec[0];
  rows = z_dims_vec[1];

  std::vector<int> img_data(z_data_length);

  k = 0;
  for (j = 0; j < rows; ++j)
    {
      for (i = 0; i < cols; ++i)
        {
          img_data[k++] = 1000 + (int)grm_round((1.0 * z_data_vec[j * cols + i] - z_min) / (z_max - z_min) * 255);
        }
    }

  auto id = static_cast<int>(global_root->getAttribute("_id"));
  global_root->setAttribute("_id", ++id);
  auto str = std::to_string(id);

  std::vector<double> x_vec, y_vec;
  linspace(0, cols - 1, cols, x_vec);
  linspace(0, rows - 1, rows, y_vec);

  (*context)["x" + str] = x_vec;
  element->setAttribute("x", "x" + str);
  (*context)["y" + str] = y_vec;
  element->setAttribute("y", "y" + str);
  std::string img_data_key = "data" + str;
  (*context)[img_data_key] = img_data;
  element->setAttribute("data", img_data_key);

  global_render->setSelectSpecificXform(element, 0);
  global_render->setScale(element, 0);
  GRM::Render::processScale(element);
  processSelectSpecificXform(element);

  double x_min, x_max, y_min, y_max;
  double vp[4];
  int scale;

  if (plot_parent->hasAttribute("viewport_x_min") && plot_parent->hasAttribute("viewport_x_max") &&
      plot_parent->hasAttribute("viewport_y_min") && plot_parent->hasAttribute("viewport_y_max"))
    {
      vp[0] = static_cast<double>(plot_parent->getAttribute("viewport_x_min"));
      vp[1] = static_cast<double>(plot_parent->getAttribute("viewport_x_max"));
      vp[2] = static_cast<double>(plot_parent->getAttribute("viewport_y_min"));
      vp[3] = static_cast<double>(plot_parent->getAttribute("viewport_y_max"));
    }
  else
    {
      throw NotFoundError("No viewport was found\n");
    }

  gr_inqscale(&scale);

  if (cols * (vp[3] - vp[2]) < rows * (vp[1] - vp[0]))
    {
      double w = (double)cols / (double)rows * (vp[3] - vp[2]);
      x_min = grm_max(0.5 * (vp[0] + vp[1] - w), vp[0]);
      x_max = grm_min(0.5 * (vp[0] + vp[1] + w), vp[1]);
      y_min = vp[2];
      y_max = vp[3];
    }
  else
    {
      double h = (double)rows / (double)cols * (vp[1] - vp[0]);
      x_min = vp[0];
      x_max = vp[1];
      y_min = grm_max(0.5 * (vp[3] + vp[2] - h), vp[2]);
      y_max = grm_min(0.5 * (vp[3] + vp[2] + h), vp[3]);
    }

  if (scale & GR_OPTION_FLIP_X)
    {
      double tmp = x_max;
      x_max = x_min;
      x_min = tmp;
    }
  if (scale & GR_OPTION_FLIP_Y)
    {
      double tmp = y_max;
      y_max = y_min;
      y_min = tmp;
    }
  if (grplot)
    {
      double tmp = y_min;
      y_min = y_max;
      y_max = tmp;
    }

  /* remove old cell arrays if they exist */
  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  std::shared_ptr<GRM::Element> cellarray;
  if (del != del_values::update_without_default && del != del_values::update_with_default)
    {
      cellarray = global_render->createCellArray(x_min, x_max, y_min, y_max, cols, rows, 1, 1, cols, rows, img_data_key,
                                                 std::nullopt);
      cellarray->setAttribute("_child_id", child_id++);
      element->append(cellarray);
    }
  else
    {
      cellarray = element->querySelectors("cellarray[_child_id=" + std::to_string(child_id++) + "]");
      if (cellarray != nullptr)
        global_render->createCellArray(x_min, x_max, y_min, y_max, cols, rows, 1, 1, cols, rows, img_data_key,
                                       std::nullopt, nullptr, cellarray);
    }
  if (cellarray != nullptr) cellarray->setAttribute("name", "imshow");
}

static void processText(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for text
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  gr_savestate();
  double tbx[4], tby[4];
  int text_color_ind = 1, scientific_format = 0;
  bool text_fits = true;
  auto x = static_cast<double>(element->getAttribute("x"));
  auto y = static_cast<double>(element->getAttribute("y"));
  auto str = static_cast<std::string>(element->getAttribute("text"));
  auto available_width = static_cast<double>(element->getAttribute("width"));
  auto available_height = static_cast<double>(element->getAttribute("height"));
  auto space = static_cast<CoordinateSpace>(static_cast<int>(element->getAttribute("space")));
  if (element->hasAttribute("text_color_ind"))
    text_color_ind = static_cast<int>(element->getAttribute("text_color_ind"));
  if (element->hasAttribute("scientific_format"))
    scientific_format = static_cast<int>(element->getAttribute("scientific_format"));

  applyMoveTransformation(element);
  if (space == CoordinateSpace::WC)
    {
      gr_wctondc(&x, &y);
    }
  if (element->hasAttribute("width") && element->hasAttribute("height"))
    {
      gr_wctondc(&available_width, &available_height);
      gr_inqtext(x, y, &str[0], tbx, tby);
      auto minmax_x = std::minmax_element(std::begin(tbx), std::end(tbx));
      auto minmax_y = std::minmax_element(std::begin(tby), std::end(tby));
      auto width = static_cast<double>((minmax_x.second - minmax_x.first));
      auto height = static_cast<double>((minmax_y.second - minmax_y.first));
      if (width > available_width && height > available_height)
        {
          gr_setcharup(0.0, 1.0);
          gr_settextalign(2, 3);
          gr_inqtext(x, y, &str[0], tbx, tby);
          width = tbx[2] - tbx[0];
          height = tby[2] - tby[0];
          if (width < available_width && height < available_height)
            {
              gr_setcharup(0.0, 1.0);
              gr_settextalign(2, 3);
            }
          else if (height < available_width && width < available_height)
            {
              gr_setcharup(-1.0, 0.0);
              gr_settextalign(2, 3);
            }
          else
            {
              text_fits = false;
            }
        }
    }

  if (text_fits && redraw_ws && scientific_format == 2)
    {
      gr_settextcolorind(text_color_ind); // needed to have a visible text after update
      gr_textext(x, y, &str[0]);
    }
  else if (text_fits && redraw_ws && scientific_format == 3)
    {
      gr_settextcolorind(text_color_ind); // needed to have a visible text after update
      gr_mathtex(x, y, &str[0]);
    }
  else if (text_fits && redraw_ws)
    {
      gr_settextcolorind(text_color_ind); // needed to have a visible text after update
      gr_text(x, y, &str[0]);
    }
  gr_restorestate();
}

static void processTextRegion(const std::shared_ptr<GRM::Element> &element,
                              const std::shared_ptr<GRM::Context> &context)
{
  double viewport[4], char_height;
  double x, y;
  std::string kind, location, text;
  bool is_title;
  del_values del = del_values::update_without_default;
  std::shared_ptr<GRM::Element> plot_parent = element->parentElement(), side_region = element->parentElement(),
                                text_elem;
  getPlotParent(plot_parent);

  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  gr_inqcharheight(&char_height);
  calculateViewport(element);
  applyMoveTransformation(element);

  viewport[0] = static_cast<double>(element->getAttribute("viewport_x_min"));
  viewport[1] = static_cast<double>(element->getAttribute("viewport_x_max"));
  viewport[2] = static_cast<double>(element->getAttribute("viewport_y_min"));
  viewport[3] = static_cast<double>(element->getAttribute("viewport_y_max"));
  kind = static_cast<std::string>(plot_parent->getAttribute("kind"));
  location = static_cast<std::string>(side_region->getAttribute("location"));
  is_title = side_region->hasAttribute("text_is_title") && static_cast<int>(side_region->getAttribute("text_is_title"));
  text = static_cast<std::string>(side_region->getAttribute("text_content"));

  if (location == "left")
    {
      x = viewport[0] + 0.5 * char_height;
      y = 0.5 * (viewport[2] + viewport[3]);
    }
  else if (location == "right")
    {
      x = viewport[1] - 0.5 * char_height;
      y = 0.5 * (viewport[2] + viewport[3]);
    }
  else if (location == "bottom")
    {
      x = 0.5 * (viewport[0] + viewport[1]);
      y = viewport[2] + 0.5 * char_height;
    }
  else if (location == "top")
    {
      x = 0.5 * (viewport[0] + viewport[1]);
      y = viewport[3];
      if (!is_title) y -= 0.5 * char_height;
    }

  if ((del != del_values::update_without_default && del != del_values::update_with_default) && !text.empty())
    {
      text_elem = global_render->createText(x, y, text);
      text_elem->setAttribute("_child_id", 0);
      element->appendChild(text_elem);
    }
  else
    {
      if (!text.empty())
        {
          text_elem = element->querySelectors("text[_child_id=\"0\"]");
          if (text_elem) global_render->createText(x, y, text, CoordinateSpace::NDC, text_elem);
        }
    }
  if (text_elem)
    {
      if (location == "left" || location == "top")
        global_render->setTextAlign(text_elem, GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_TOP);
      if (location == "bottom" || location == "bottom")
        global_render->setTextAlign(text_elem, GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_BOTTOM);
      if (location == "top" && is_title) text_elem->setAttribute("z_index", 2);
      if (location == "left" || location == "right") global_render->setCharUp(text_elem, -1, 0);
    }
}

static void tickLabelAdjustment(const std::shared_ptr<GRM::Element> &tick_group, int child_id, del_values del)
{
  double char_height, tmp = 0;
  double available_width, available_height;
  double x, y, y_org, width;
  double window[4];
  int scientific_format = 2;
  std::shared_ptr<GRM::Element> text_elem, plot_parent = tick_group;
  char new_label[256];
  int cur_start = 0, scale = 0, cur_child_count = 0, child_id_org = child_id - 1, i = 0;
  bool x_flip, y_flip, text_is_empty_or_number = true;
  int pixel_width, pixel_height;
  double metric_width, metric_height;
  double aspect_ratio_ws_metric;

  GRM::Render::getFigureSize(&pixel_width, &pixel_height, &metric_width, &metric_height);
  aspect_ratio_ws_metric = metric_width / metric_height;
  getPlotParent(plot_parent);
  gr_inqcharheight(&char_height);

  auto text = static_cast<std::string>(tick_group->getAttribute("tick_label"));
  auto axis_type = static_cast<std::string>(tick_group->parentElement()->getAttribute("axis_type"));
  auto value = static_cast<double>(tick_group->getAttribute("value"));
  auto label_pos = static_cast<double>(tick_group->parentElement()->getAttribute("label_pos"));
  auto pos = static_cast<double>(tick_group->parentElement()->getAttribute("pos"));
  // todo: window should come from axis if axis has window defined on it
  window[0] =
      static_cast<double>(tick_group->parentElement()->parentElement()->parentElement()->getAttribute("window_x_min"));
  window[1] =
      static_cast<double>(tick_group->parentElement()->parentElement()->parentElement()->getAttribute("window_x_max"));
  window[2] =
      static_cast<double>(tick_group->parentElement()->parentElement()->parentElement()->getAttribute("window_y_min"));
  window[3] =
      static_cast<double>(tick_group->parentElement()->parentElement()->parentElement()->getAttribute("window_y_max"));

  if (tick_group->parentElement()->hasAttribute("scale"))
    scale = static_cast<int>(tick_group->parentElement()->getAttribute("scale"));
  if (plot_parent->hasAttribute("x_flip")) x_flip = static_cast<int>(plot_parent->getAttribute("x_flip"));
  if (plot_parent->hasAttribute("y_flip")) y_flip = static_cast<int>(plot_parent->getAttribute("y_flip"));
  if (tick_group->parentElement()->hasAttribute("scientific_format"))
    scientific_format = static_cast<int>(tick_group->parentElement()->getAttribute("scientific_format"));
  if (tick_group->hasAttribute("scientific_format"))
    scientific_format = static_cast<int>(tick_group->getAttribute("scientific_format"));

  if (text.empty() && del != del_values::update_without_default && del != del_values::update_with_default) return;
  if (axis_type == "x")
    {
      available_height = 2.0; // Todo: change this number respecting the other objects
      available_width = 1.0;  // Todo: change this number respecting the other label
      x = value;
      y = label_pos;
    }
  else if (axis_type == "y")
    {
      available_height = 3.0; // Todo: change this number respecting the other label
      available_width = 1.0;  // Todo: change this number respecting the other objects
      x = label_pos;
      y = value;
    }
  if (aspect_ratio_ws_metric > 1)
    available_width *= 1.0 / (aspect_ratio_ws_metric);
  else
    available_width *= aspect_ratio_ws_metric;
  gr_wctondc(&x, &y);
  y_org = y;

  // get number of text children to figure out if children must be added or removed
  for (const auto &child : tick_group->children())
    {
      if (child->localName() == "text" && child->hasAttribute("_child_id")) cur_child_count += 1;
    }

  if (is_number(text))
    {
      if (text.size() > 7)
        {
          char text_c[256];
          format_reference_t reference = {1, 1};
          const char minus[] = {(char)0xe2, (char)0x88, (char)0x92, '\0'}; // gr minus sign
          auto em_dash = std::string(minus);
          size_t start_pos = 0;

          gr_setscientificformat(scientific_format);

          if (starts_with(text, em_dash))
            {
              start_pos = em_dash.size();
            }
          auto without_minus = text.substr(start_pos);

          snprintf(text_c, 256, "%s", without_minus.c_str());
          text = gr_ftoa(text_c, atof(without_minus.c_str()), &reference);
          if (start_pos != 0) text = em_dash + text;
        }
    }
  else
    {
      double tbx[4], tby[4];
      char text_c[256];
      snprintf(text_c, 256, "%s", text.c_str());
      gr_inqtext(x, y, text_c, tbx, tby);
      gr_wctondc(&tbx[0], &tby[0]);
      gr_wctondc(&tbx[1], &tby[1]);
      width = tbx[1] - tbx[0];
      tick_group->setAttribute("width", width);

      if (width / char_height > available_width)
        {
          int breakpoint_positions[128];
          int cur_num_breakpoints = 0;
          const char *label = text.c_str();
          for (i = 0; i == 0 || label[i - 1] != '\0'; ++i)
            {
              if (label[i] == ' ' || label[i] == '\0')
                {
                  /* calculate width of the next part of the label to be drawn */
                  new_label[i] = '\0';
                  gr_inqtext(x, y, new_label + cur_start, tbx, tby);
                  gr_wctondc(&tbx[0], &tby[0]);
                  gr_wctondc(&tbx[1], &tby[1]);
                  width = tbx[1] - tbx[0];
                  new_label[i] = ' ';

                  /* add possible breakpoint */
                  breakpoint_positions[cur_num_breakpoints++] = i;

                  if (width / char_height > available_width)
                    {
                      /* part is too big but doesn't have a breakpoint in it */
                      if (cur_num_breakpoints == 1)
                        {
                          new_label[i] = '\0';
                        }
                      else /* part is too big and has breakpoints in it */
                        {
                          /* break label at last breakpoint that still allowed the text to fit */
                          new_label[breakpoint_positions[cur_num_breakpoints - 2]] = '\0';
                        }

                      if ((del != del_values::update_without_default && del != del_values::update_with_default) ||
                          cur_child_count + child_id_org < child_id)
                        {
                          text_elem = global_render->createText(x, y, new_label + cur_start, CoordinateSpace::NDC);
                          tick_group->append(text_elem);
                          text_elem->setAttribute("_child_id", child_id++);
                        }
                      else
                        {
                          text_elem = tick_group->querySelectors("text[_child_id=" + std::to_string(child_id++) + "]");
                          if (text_elem != nullptr)
                            global_render->createText(x, y, new_label + cur_start, CoordinateSpace::NDC, text_elem);
                        }
                      if (text_elem != nullptr)
                        {
                          if (axis_type == "x")
                            {
                              text_elem->setAttribute("text_align_horizontal", GKS_K_TEXT_HALIGN_CENTER);
                              if (pos <= 0.5 * (window[2] + window[3]) ||
                                  ((scale & GR_OPTION_FLIP_Y || y_flip) && pos > 0.5 * (window[2] + window[3])))
                                {
                                  text_elem->setAttribute("text_align_vertical", GKS_K_TEXT_VALIGN_TOP);
                                }
                              else
                                {
                                  text_elem->setAttribute("text_align_vertical", GKS_K_TEXT_VALIGN_BOTTOM);
                                }
                            }
                          else if (axis_type == "y")
                            {
                              text_elem->setAttribute("text_align_vertical", GKS_K_TEXT_VALIGN_HALF);
                              if ((pos <= 0.5 * (window[0] + window[1]) && !(scale & GR_OPTION_FLIP_X || x_flip)) ||
                                  ((scale & GR_OPTION_FLIP_X || x_flip) && pos > 0.5 * (window[0] + window[1]) &&
                                   tick_group->parentElement()->parentElement()->localName() != "colorbar"))
                                {
                                  text_elem->setAttribute("text_align_horizontal", GKS_K_TEXT_HALIGN_RIGHT);
                                }
                              else
                                {
                                  text_elem->setAttribute("text_align_horizontal", GKS_K_TEXT_HALIGN_LEFT);
                                }
                            }

                          text_elem->setAttribute("scientific_format", scientific_format);
                        }

                      if (cur_num_breakpoints == 1)
                        {
                          cur_start = i + 1;
                          cur_num_breakpoints = 0;
                        }
                      else
                        {
                          cur_start = breakpoint_positions[cur_num_breakpoints - 2] + 1;
                          breakpoint_positions[0] = breakpoint_positions[cur_num_breakpoints - 1];
                          cur_num_breakpoints = 1;
                        }
                      y -= 1.1 * char_height;

                      // if the available height is passed the rest of the label won't get displayed
                      if ((y_org - y) / char_height > available_height) break;
                    }
                }
              else
                {
                  new_label[i] = label[i];
                }
            }
          /* 0-terminate the new label */
          new_label[i] = '\0';

          text = new_label + cur_start;
          if (text == "" && (del != del_values::update_without_default && del != del_values::update_with_default))
            del = del_values::update_without_default;
        }
      if (i >= cur_start && text != " " && text != "\0" && text != "" && cur_child_count + child_id_org < child_id)
        text_is_empty_or_number = false;
      if (i < cur_start && !text_is_empty_or_number) child_id_org += 1;
    }

  if ((del != del_values::update_without_default && del != del_values::update_with_default) || !text_is_empty_or_number)
    {
      text_elem = global_render->createText(x, y, text, CoordinateSpace::NDC);
      text_elem->setAttribute("_child_id", child_id);
      tick_group->append(text_elem);
    }
  else
    {
      text_elem = tick_group->querySelectors("text[_child_id=" + std::to_string(child_id) + "]");
      if (text_elem != nullptr) global_render->createText(x, y, text, CoordinateSpace::NDC, text_elem);
    }
  if ((text_elem != nullptr && del != del_values::update_without_default) || !text_is_empty_or_number)
    {
      text_elem->setAttribute("text_color_ind", 1);
      // set text align if not set by user
      if (!(tick_group->hasAttribute("text_align_vertical") && tick_group->hasAttribute("text_align_horizontal")))
        {
          if (axis_type == "x")
            {
              text_elem->setAttribute("text_align_horizontal", GKS_K_TEXT_HALIGN_CENTER);
              if (pos <= 0.5 * (window[2] + window[3]) ||
                  ((scale & GR_OPTION_FLIP_Y || y_flip) && pos > 0.5 * (window[2] + window[3])))
                {
                  text_elem->setAttribute("text_align_vertical", GKS_K_TEXT_VALIGN_TOP);
                }
              else
                {
                  text_elem->setAttribute("text_align_vertical", GKS_K_TEXT_VALIGN_BOTTOM);
                }
            }
          else if (axis_type == "y")
            {
              text_elem->setAttribute("text_align_vertical", GKS_K_TEXT_VALIGN_HALF);
              if ((pos <= 0.5 * (window[0] + window[1]) && !(scale & GR_OPTION_FLIP_X || x_flip)) ||
                  ((scale & GR_OPTION_FLIP_X || x_flip) && pos > 0.5 * (window[0] + window[1]) &&
                   tick_group->parentElement()->parentElement()->localName() != "colorbar"))
                {
                  text_elem->setAttribute("text_align_horizontal", GKS_K_TEXT_HALIGN_RIGHT);
                }
              else
                {
                  text_elem->setAttribute("text_align_horizontal", GKS_K_TEXT_HALIGN_LEFT);
                }
            }
        }
      text_elem->setAttribute("scientific_format", scientific_format);
    }


  // remove all text children with _child_id > child_id
  while (child_id < cur_child_count + child_id_org)
    {
      tick_group->querySelectors("text[_child_id=" + std::to_string(child_id++) + "]")->remove();
      if (child_id == cur_child_count + child_id_org && i >= cur_start &&
          tick_group->querySelectors("text[_child_id=" + std::to_string(child_id) + "]"))
        tick_group->querySelectors("text[_child_id=" + std::to_string(child_id) + "]")->remove();
    }
}

static void applyTickModificationMap(const std::shared_ptr<GRM::Element> &tick_group,
                                     const std::shared_ptr<GRM::Context> &context, int child_id, del_values del)
{
  std::shared_ptr<GRM::Element> text_elem = nullptr;
  bool tick_group_attr_changed = false, old_automatic_update = automatic_update;

  auto value = static_cast<double>(tick_group->getAttribute("value"));
  auto map_idx = static_cast<int>(tick_group->parentElement()->getAttribute("_axis_id"));
  for (const auto &child : tick_group->children())
    {
      if (child->localName() == "text")
        {
          text_elem = child;
          break;
        }
    }

  automatic_update = false;
  if (tick_modification_map.find(map_idx) != tick_modification_map.end())
    {
      auto tick_value_to_map = tick_modification_map[map_idx];
      if (tick_value_to_map.find(value) != tick_value_to_map.end())
        {
          auto key_value_map = tick_value_to_map[value];
          if (!key_value_map.empty())
            {
              for (auto const &[attr, val] : key_value_map)
                {
                  if (str_equals_any(attr, "is_major", "line_color_ind", "line_spec", "line_width",
                                     "text_align_horizontal", "text_align_vertical", "tick_label", "tick_size",
                                     "value"))
                    {
                      tick_group->setAttribute(attr, val);
                      if (attr == "tick_label")
                        {
                          tick_group->setAttribute("tick_label", val);
                          tickLabelAdjustment(tick_group, child_id,
                                              text_elem != nullptr ? del_values::update_without_default : del);
                        }
                      else if (str_equals_any(attr, "is_major", "value"))
                        {
                          for (const auto &child : tick_group->children())
                            {
                              child->setAttribute(attr, val);
                            }
                        }
                      tick_group_attr_changed = true;
                    }
                  else if (str_equals_any(attr, "font", "font_precision", "scientific_format", "text", "text_color_ind",
                                          "text_align_horizontal", "text_align_vertical", "x", "y"))
                    {
                      if (text_elem != nullptr) text_elem->setAttribute(attr, val);
                    }
                }
            }
          if (tick_group_attr_changed) GRM::Render::processAttributes(tick_group);
        }
    }
  automatic_update = old_automatic_update;
}

static void processTickGroup(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  int child_id = 0, z_index = 0;
  double x, y;
  double window[4];
  std::shared_ptr<GRM::Element> tick_elem, text, grid_line;
  del_values del = del_values::update_without_default;

  auto value = static_cast<double>(element->getAttribute("value"));
  auto is_major = static_cast<int>(element->getAttribute("is_major"));
  auto tick_label = static_cast<std::string>(element->getAttribute("tick_label"));
  auto axis_type = static_cast<std::string>(element->parentElement()->getAttribute("axis_type"));
  auto draw_grid = static_cast<int>(element->parentElement()->getAttribute("draw_grid"));
  bool mirrored_axis = element->parentElement()->hasAttribute("mirrored_axis") &&
                       static_cast<int>(element->parentElement()->getAttribute("mirrored_axis"));
  // todo: window should come from axis if axis has window defined on it
  window[0] =
      static_cast<double>(element->parentElement()->parentElement()->parentElement()->getAttribute("window_x_min"));
  window[1] =
      static_cast<double>(element->parentElement()->parentElement()->parentElement()->getAttribute("window_x_max"));
  window[2] =
      static_cast<double>(element->parentElement()->parentElement()->parentElement()->getAttribute("window_y_min"));
  window[3] =
      static_cast<double>(element->parentElement()->parentElement()->parentElement()->getAttribute("window_y_max"));

  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  // tick
  if (del != del_values::update_without_default && del != del_values::update_with_default)
    {
      tick_elem = global_render->createTick(is_major, value);
      tick_elem->setAttribute("_child_id", child_id++);
      element->append(tick_elem);
    }
  else
    {
      tick_elem = element->querySelectors("tick[_child_id=" + std::to_string(child_id++) + "]");
      if (tick_elem != nullptr) global_render->createTick(is_major, value, tick_elem);
    }
  if (tick_elem != nullptr)
    {
      if (!is_major)
        z_index = -8;
      else
        z_index = -4;
      if (axis_type == "y") z_index -= 2;
      if (element->parentElement()->parentElement()->localName() == "colorbar") z_index = 1;
      tick_elem->setAttribute("z_index", z_index);
    }

  // mirrored tick
  if (mirrored_axis)
    {
      if (del != del_values::update_without_default && del != del_values::update_with_default)
        {
          tick_elem = global_render->createTick(is_major, value);
          tick_elem->setAttribute("_child_id", child_id++);
          element->append(tick_elem);
        }
      else
        {
          tick_elem = element->querySelectors("tick[_child_id=" + std::to_string(child_id++) + "]");
          if (tick_elem != nullptr) global_render->createTick(is_major, value, tick_elem);
        }
      if (tick_elem != nullptr)
        {
          if (!is_major)
            z_index = -9;
          else
            z_index = -5;
          if (axis_type == "y") z_index -= 2;
          tick_elem->setAttribute("z_index", z_index);
          tick_elem->setAttribute("is_mirrored", true);
        }
    }

  // grid
  if (draw_grid)
    {
      if (del != del_values::update_without_default && del != del_values::update_with_default)
        {
          grid_line = global_render->createGridLine(is_major, value);
          grid_line->setAttribute("_child_id", child_id++);
          element->append(grid_line);
        }
      else
        {
          grid_line = element->querySelectors("grid_line[_child_id=" + std::to_string(child_id++) + "]");
          if (grid_line != nullptr) global_render->createGridLine(is_major, value, grid_line);
        }
      if (grid_line != nullptr)
        {
          if (!is_major)
            z_index = -14;
          else
            z_index = -12;
          if (axis_type == "y") z_index -= 1;
          grid_line->setAttribute("z_index", z_index);
        }
    }

  tickLabelAdjustment(element, child_id, del);
  applyTickModificationMap(element, context, child_id, del);
}

static void processTick(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  int mask = 0;
  std::shared_ptr<GRM::Element> axis_elem = element->parentElement()->parentElement();
  std::shared_ptr<GRM::Element> plot_parent = element;
  getPlotParent(plot_parent);

  auto coordinate_system = plot_parent->querySelectors("coordinate_system");
  bool hide =
      (coordinate_system->hasAttribute("hide")) ? static_cast<int>(coordinate_system->getAttribute("hide")) : false;
  auto coordinate_system_type = static_cast<std::string>(coordinate_system->getAttribute("plot_type"));
  auto axis_type = static_cast<std::string>(axis_elem->getAttribute("axis_type"));
  auto min_val = static_cast<double>(axis_elem->getAttribute("min_value"));
  auto max_val = static_cast<double>(axis_elem->getAttribute("max_value"));
  auto org = static_cast<double>(axis_elem->getAttribute("org"));
  auto pos = static_cast<double>(axis_elem->getAttribute("pos"));
  auto tick = static_cast<double>(axis_elem->getAttribute("tick"));
  auto major_count = static_cast<int>(axis_elem->getAttribute("major_count"));
  auto tick_size = static_cast<double>(axis_elem->getAttribute("tick_size"));
  if (element->parentElement()->hasAttribute("tick_size"))
    tick_size = static_cast<double>(element->parentElement()->getAttribute("tick_size"));
  auto tick_orientation = static_cast<int>(axis_elem->getAttribute("tick_orientation"));
  auto value = static_cast<double>(element->getAttribute("value"));
  auto is_major = static_cast<int>(element->getAttribute("is_major"));
  auto label_pos = static_cast<double>(axis_elem->getAttribute("label_pos"));
  bool mirrored_axis = element->hasAttribute("is_mirrored") && static_cast<int>(element->getAttribute("is_mirrored"));

  tick_t t = {value, is_major};
  axis_t drawn_tick = {min_val, max_val, tick,      org,  pos, major_count, 1, &t, tick_size * tick_orientation,
                       0,       nullptr, label_pos, false};
  if (redraw_ws && !hide && (coordinate_system_type == "2d" || axis_elem->parentElement()->localName() == "colorbar"))
    {
      mask = mirrored_axis ? 2 : 1;
      if (axis_type == "x")
        {
          gr_drawaxes(&drawn_tick, nullptr, mask);
        }
      else
        {
          gr_drawaxes(nullptr, &drawn_tick, mask);
        }
    }
}

static void processTitles3d(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for titles 3d
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  std::string xlabel, ylabel, zlabel;
  auto coordinate_system = element->parentElement();
  bool hide =
      (coordinate_system->hasAttribute("hide")) ? static_cast<int>(coordinate_system->getAttribute("hide")) : false;
  auto coordinate_systemType = static_cast<std::string>(coordinate_system->getAttribute("plot_type"));
  xlabel = static_cast<std::string>(element->getAttribute("x_label_3d"));
  ylabel = static_cast<std::string>(element->getAttribute("y_label_3d"));
  zlabel = static_cast<std::string>(element->getAttribute("z_label_3d"));
  applyMoveTransformation(element);
  if (redraw_ws && !hide && coordinate_systemType == "3d") gr_titles3d(xlabel.data(), ylabel.data(), zlabel.data());
}

static void processTriContour(const std::shared_ptr<GRM::Element> &element,
                              const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for tricontour
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double z_min, z_max;
  int num_levels = PLOT_DEFAULT_TRICONT_LEVELS;
  int i;
  unsigned int x_length, y_length, z_length;
  std::vector<double> x_vec, y_vec, z_vec;
  auto plot_parent = element->parentElement();
  getPlotParent(plot_parent);

  z_min = static_cast<double>(plot_parent->getAttribute("_z_lim_min"));
  z_max = static_cast<double>(plot_parent->getAttribute("_z_lim_max"));
  if (element->hasAttribute("levels"))
    {
      num_levels = static_cast<int>(element->getAttribute("levels"));
    }
  else
    {
      element->setAttribute("levels", num_levels);
    }

  std::vector<double> levels(num_levels);

  for (i = 0; i < num_levels; ++i)
    {
      levels[i] = z_min + ((1.0 * i) / (num_levels - 1)) * (z_max - z_min);
    }

  if (!element->hasAttribute("x")) throw NotFoundError("Tricontour series is missing required attribute px-data.\n");
  auto x = static_cast<std::string>(element->getAttribute("x"));
  if (!element->hasAttribute("y")) throw NotFoundError("Tricontour series is missing required attribute py-data.\n");
  auto y = static_cast<std::string>(element->getAttribute("y"));
  if (!element->hasAttribute("z")) throw NotFoundError("Tricontour series is missing required attribute pz-data.\n");
  auto z = static_cast<std::string>(element->getAttribute("z"));

  x_vec = GRM::get<std::vector<double>>((*context)[x]);
  y_vec = GRM::get<std::vector<double>>((*context)[y]);
  z_vec = GRM::get<std::vector<double>>((*context)[z]);
  x_length = x_vec.size();
  y_length = y_vec.size();
  z_length = z_vec.size();

  if (x_length != y_length || x_length != z_length)
    throw std::length_error("For tricontour series x-, y- and z-data must have the same size.\n");

  double *px_p = &(x_vec[0]);
  double *py_p = &(y_vec[0]);
  double *pz_p = &(z_vec[0]);
  double *l_p = &(levels[0]);
  applyMoveTransformation(element);

  if (redraw_ws) gr_tricontour((int)x_length, px_p, py_p, pz_p, num_levels, l_p);
}

static void processTriSurface(const std::shared_ptr<GRM::Element> &element,
                              const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for trisurface
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  if (!element->hasAttribute("x")) throw NotFoundError("Trisurface series is missing required attribute px-data.\n");
  auto px = static_cast<std::string>(element->getAttribute("x"));
  if (!element->hasAttribute("y")) throw NotFoundError("Trisurface series is missing required attribute py-data.\n");
  auto py = static_cast<std::string>(element->getAttribute("y"));
  if (!element->hasAttribute("z")) throw NotFoundError("Trisurface series is missing required attribute pz-data.\n");
  auto pz = static_cast<std::string>(element->getAttribute("z"));

  std::vector<double> px_vec = GRM::get<std::vector<double>>((*context)[px]);
  std::vector<double> py_vec = GRM::get<std::vector<double>>((*context)[py]);
  std::vector<double> pz_vec = GRM::get<std::vector<double>>((*context)[pz]);

  auto nx = (int)px_vec.size();
  auto ny = (int)py_vec.size();
  auto nz = (int)pz_vec.size();
  if (nx != ny || nx != nz)
    throw std::length_error("For trisurface series px-, py- and pz-data must have the same size.\n");

  double *px_p = &(px_vec[0]);
  double *py_p = &(py_vec[0]);
  double *pz_p = &(pz_vec[0]);
  applyMoveTransformation(element);

  if (redraw_ws) gr_trisurface(nx, px_p, py_p, pz_p);
}

static void volume(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  int width, height;
  double device_pixel_ratio;
  double d_min = -1, d_max = -1;

  auto z_key = static_cast<std::string>(element->getAttribute("z"));
  auto z_vec = GRM::get<std::vector<double>>((*context)[z_key]);
  auto z_dims_key = static_cast<std::string>(element->getAttribute("z_dims"));
  auto z_dims_vec = GRM::get<std::vector<int>>((*context)[z_dims_key]);
  int algorithm = getVolumeAlgorithm(element);
  if (element->hasAttribute("d_min")) d_min = static_cast<double>(element->getAttribute("d_min"));
  if (element->hasAttribute("d_max")) d_max = static_cast<double>(element->getAttribute("d_max"));

  applyMoveTransformation(element);
  if (redraw_ws)
    {
      gr_inqvpsize(&width, &height, &device_pixel_ratio);
      gr_setpicturesizeforvolume((int)(width * device_pixel_ratio), (int)(height * device_pixel_ratio));
    }
  if (element->hasAttribute("_volume_context_address"))
    {
      auto address = static_cast<std::string>(element->getAttribute("_volume_context_address"));
      long volume_address = stol(address, nullptr, 16);
      const gr3_volume_2pass_t *volume_context = (gr3_volume_2pass_t *)volume_address;
      if (redraw_ws)
        gr_volume_2pass(z_dims_vec[0], z_dims_vec[1], z_dims_vec[2], &(z_vec[0]), algorithm, &d_min, &d_max,
                        volume_context);
      element->removeAttribute("_volume_context_address");
    }
  else
    {
      if (redraw_ws) gr_volume(z_dims_vec[0], z_dims_vec[1], z_dims_vec[2], &(z_vec[0]), algorithm, &d_min, &d_max);
    }
}

static void processVolume(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  double dlim[2] = {INFINITY, -INFINITY};
  unsigned int z_length, dims;
  int algorithm = PLOT_DEFAULT_VOLUME_ALGORITHM;
  std::string algorithm_str;
  double d_min, d_max;
  int width, height;
  double device_pixel_ratio;

  if (!element->hasAttribute("z")) throw NotFoundError("Volume series is missing required attribute z-data.\n");
  auto z_key = static_cast<std::string>(element->getAttribute("z"));
  auto z_vec = GRM::get<std::vector<double>>((*context)[z_key]);
  z_length = z_vec.size();

  if (!element->hasAttribute("z_dims")) throw NotFoundError("Volume series is missing required attribute z_dims.\n");
  auto z_dims_key = static_cast<std::string>(element->getAttribute("z_dims"));
  auto z_dims_vec = GRM::get<std::vector<int>>((*context)[z_dims_key]);
  dims = z_dims_vec.size();

  if (dims != 3) throw std::length_error("For volume series the size of z_dims has to be 3.\n");
  if (z_dims_vec[0] * z_dims_vec[1] * z_dims_vec[2] != z_length)
    throw std::length_error("For volume series shape[0] * shape[1] * shape[2] must be z length.\n");
  if (z_length <= 0) throw NotFoundError("For volume series the size of z has to be greater than 0.\n");

  if (!element->hasAttribute("algorithm"))
    {
      element->setAttribute("algorithm", algorithm);
    }
  else
    {
      algorithm = getVolumeAlgorithm(element);
    }
  if (algorithm != GR_VOLUME_ABSORPTION && algorithm != GR_VOLUME_EMISSION && algorithm != GR_VOLUME_MIP)
    {
      logger((stderr, "Got unknown volume algorithm \"%d\"\n", algorithm));
      throw std::logic_error("For volume series the given algorithm is unknown.\n");
    }

  d_min = d_max = -1.0;
  if (element->hasAttribute("d_min")) d_min = static_cast<double>(element->getAttribute("d_min"));
  if (element->hasAttribute("d_max")) d_max = static_cast<double>(element->getAttribute("d_max"));

  if (redraw_ws)
    {
      gr_inqvpsize(&width, &height, &device_pixel_ratio);
      gr_setpicturesizeforvolume((int)(width * device_pixel_ratio), (int)(height * device_pixel_ratio));
    }
  const gr3_volume_2pass_t *volumeContext =
      gr_volume_2pass(z_dims_vec[0], z_dims_vec[1], z_dims_vec[2], &(z_vec[0]), algorithm, &d_min, &d_max, nullptr);

  std::ostringstream get_address;
  get_address << volumeContext;
  element->setAttribute("_volume_context_address", get_address.str());

  auto parent_element = element->parentElement();
  getPlotParent(parent_element); // parent is plot in this case
  if (parent_element->hasAttribute("z_lim_min") && parent_element->hasAttribute("z_lim_max"))
    {
      dlim[0] = static_cast<double>(parent_element->getAttribute("z_lim_min"));
      dlim[1] = static_cast<double>(parent_element->getAttribute("z_lim_max"));
      dlim[0] = grm_min(dlim[0], d_min);
      dlim[1] = grm_max(dlim[1], d_max);
    }
  else
    {
      dlim[0] = d_min;
      dlim[1] = d_max;
    }

  parent_element->setAttribute("_c_lim_min", dlim[0]);
  parent_element->setAttribute("_c_lim_max", dlim[1]);

  if (redraw_ws)
    {
      PushDrawableToZQueue pushVolumeToZQueue(volume);
      pushVolumeToZQueue(element, context);
    }
}

static void processWireframe(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for wireframe
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  unsigned int x_length, y_length, z_length;

  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));
  auto z = static_cast<std::string>(element->getAttribute("z"));

  std::vector<double> x_vec = GRM::get<std::vector<double>>((*context)[x]);
  std::vector<double> y_vec = GRM::get<std::vector<double>>((*context)[y]);
  std::vector<double> z_vec = GRM::get<std::vector<double>>((*context)[z]);
  x_length = x_vec.size();
  y_length = y_vec.size();
  z_length = z_vec.size();

  global_render->setFillColorInd(element, 0);
  processFillColorInd(element);

  auto id_int = static_cast<int>(global_root->getAttribute("_id"));
  global_root->setAttribute("_id", ++id_int);
  auto id = std::to_string(id_int);

  if (x_length == y_length && x_length == z_length)
    {
      std::vector<double> gridit_x_vec(PLOT_WIREFRAME_GRIDIT_N);
      std::vector<double> gridit_y_vec(PLOT_WIREFRAME_GRIDIT_N);
      std::vector<double> gridit_z_vec(PLOT_WIREFRAME_GRIDIT_N * PLOT_WIREFRAME_GRIDIT_N);

      double *gridit_x = &(gridit_x_vec[0]);
      double *gridit_y = &(gridit_y_vec[0]);
      double *gridit_z = &(gridit_z_vec[0]);
      double *x_p = &(x_vec[0]);
      double *y_p = &(y_vec[0]);
      double *z_p = &(z_vec[0]);

      gr_gridit((int)x_length, x_p, y_p, z_p, PLOT_WIREFRAME_GRIDIT_N, PLOT_WIREFRAME_GRIDIT_N, gridit_x, gridit_y,
                gridit_z);

      x_vec = std::vector<double>(gridit_x, gridit_x + PLOT_WIREFRAME_GRIDIT_N);
      y_vec = std::vector<double>(gridit_y, gridit_y + PLOT_WIREFRAME_GRIDIT_N);
      z_vec = std::vector<double>(gridit_z, gridit_z + PLOT_WIREFRAME_GRIDIT_N * PLOT_WIREFRAME_GRIDIT_N);
    }
  else
    {
      if (x_length * y_length != z_length)
        throw std::length_error("For wireframe series x_length * y_length must be z_length.\n");
    }

  double *px_p = &(x_vec[0]);
  double *py_p = &(y_vec[0]);
  double *pz_p = &(z_vec[0]);
  applyMoveTransformation(element);
  if (redraw_ws) gr_surface((int)x_length, (int)y_length, px_p, py_p, pz_p, GR_OPTION_FILLED_MESH);
}

void plotProcessWsWindowWsViewport(const std::shared_ptr<GRM::Element> &element,
                                   const std::shared_ptr<GRM::Context> &context)
{
  int pixel_width, pixel_height;
  double metric_width, metric_height;
  double aspect_ratio_ws_metric;
  double ws_viewport[4] = {0.0, 0.0, 0.0, 0.0};
  double ws_window[4] = {0.0, 0.0, 0.0, 0.0};

  // set ws_window/ws_viewport on active figure
  GRM::Render::getFigureSize(&pixel_width, &pixel_height, &metric_width, &metric_height);

  if (!active_figure->hasAttribute("_previous_pixel_width") || !active_figure->hasAttribute("_previous_pixel_height") ||
      (static_cast<int>(active_figure->getAttribute("_previous_pixel_width")) != pixel_width ||
       static_cast<int>(active_figure->getAttribute("_previous_pixel_height")) != pixel_height))
    {
      /* TODO: handle error return value? */
      auto figure_id_str = static_cast<std::string>(active_figure->getAttribute("figure_id"));
      if (starts_with(figure_id_str, "figure"))
        {
          figure_id_str = figure_id_str.substr(6);
        }
      auto figure_id = std::stoi(figure_id_str);
      event_queue_enqueue_size_event(event_queue, figure_id, pixel_width, pixel_height);
    }

  aspect_ratio_ws_metric = metric_width / metric_height;
  if (aspect_ratio_ws_metric > 1)
    {
      ws_window[1] = 1.0;
      ws_window[3] = 1.0 / (aspect_ratio_ws_metric);
    }
  else
    {
      ws_window[1] = aspect_ratio_ws_metric;
      ws_window[3] = 1.0;
    }
  ws_viewport[1] = metric_width;
  ws_viewport[3] = metric_height;
  global_render->setWSViewport(active_figure, ws_viewport[0], ws_viewport[1], ws_viewport[2], ws_viewport[3]);
  global_render->setWSWindow(active_figure, ws_window[0], ws_window[1], ws_window[2], ws_window[3]);

  active_figure->setAttribute("_previous_pixel_width", pixel_width);
  active_figure->setAttribute("_previous_pixel_height", pixel_height);

  logger((stderr, "Stored ws_window (%lf, %lf, %lf, %lf)\n", ws_window[0], ws_window[1], ws_window[2], ws_window[3]));
  logger((stderr, "Stored ws_viewport (%lf, %lf, %lf, %lf)\n", ws_viewport[0], ws_viewport[1], ws_viewport[2],
          ws_viewport[3]));
}

static void plotCoordinateRanges(const std::shared_ptr<GRM::Element> &element,
                                 const std::shared_ptr<GRM::Context> &context)
{
  std::string kind, style;
  const char *fmt;
  unsigned int series_count = 0;
  std::vector<std::string> data_component_names = {"x", "y", "z", "c", ""};
  std::vector<std::string>::iterator current_component_name;
  std::vector<double> current_component;
  std::shared_ptr<GRM::Element> central_region;
  unsigned int current_point_count = 0;
  struct
  {
    const char *subplot;
    const char *series;
  } * current_range_keys,
      range_keys[] = {{"x_lim", "x_range"}, {"y_lim", "y_range"}, {"z_lim", "z_range"}, {"c_lim", "c_range"}};
  std::vector<double> bins;
  unsigned int i;

  logger((stderr, "Storing coordinate ranges\n"));

  for (const auto &child : element->children())
    {
      if (child->localName() == "central_region")
        {
          central_region = child;
          break;
        }
    }

  /* If a pan and/or zoom was performed before, do not overwrite limits
   * -> the user fully controls limits by interaction */
  if (element->hasAttribute("original_x_lim"))
    {
      logger((stderr, "Panzoom active, do not modify limits...\n"));
    }
  else
    {
      element->setAttribute("_x_lim_min", NAN);
      element->setAttribute("_x_lim_max", NAN);
      element->setAttribute("_y_lim_min", NAN);
      element->setAttribute("_y_lim_max", NAN);
      element->setAttribute("_z_lim_min", NAN);
      element->setAttribute("_z_lim_max", NAN);
      element->setAttribute("_c_lim_min", NAN);
      element->setAttribute("_c_lim_max", NAN);
      kind = static_cast<std::string>(element->getAttribute("kind"));
      if (!string_map_at(fmt_map, static_cast<const char *>(kind.c_str()), static_cast<const char **>(&fmt)))
        {
          std::stringstream ss;
          ss << "Invalid kind \"" << kind << "\" was given.";
          throw NotFoundError(ss.str());
        }
      if (!str_equals_any(kind, "pie", "polar_histogram"))
        {
          current_component_name = data_component_names.begin();
          current_range_keys = range_keys;
          while (!(*current_component_name).empty())
            {
              double min_component = DBL_MAX;
              double max_component = -DBL_MAX;
              double step = -DBL_MAX;

              if (static_cast<std::string>(fmt).find(*current_component_name) == std::string::npos)
                {
                  ++current_range_keys;
                  ++current_component_name;
                  continue;
                }
              /* Heatmaps need calculated range keys, so run the calculation even if limits are given */
              if (!element->hasAttribute(static_cast<std::string>(current_range_keys->subplot) + "_min") ||
                  !element->hasAttribute(static_cast<std::string>(current_range_keys->subplot) + "_max") ||
                  str_equals_any(kind, "heatmap", "marginal_heatmap", "polar_heatmap"))
                {
                  std::shared_ptr<GRM::Element> series_parent = central_region;
                  if (kind == "marginal_heatmap") series_parent = element;
                  for (const auto &series : series_parent->children())
                    {
                      if (!starts_with(series->localName(), "series") && central_region == series_parent) continue;
                      if (series->hasAttribute("style"))
                        style = static_cast<std::string>(series->getAttribute("style"));
                      double current_min_component = DBL_MAX, current_max_component = -DBL_MAX;
                      if (!series->hasAttribute(static_cast<std::string>(current_range_keys->series) + "_min") ||
                          !series->hasAttribute((static_cast<std::string>(current_range_keys->series) + "_max")))
                        {
                          if (series->hasAttribute(*current_component_name))
                            {
                              auto key = static_cast<std::string>(series->getAttribute(*current_component_name));
                              current_component = GRM::get<std::vector<double>>((*context)[key]);
                              current_point_count = current_component.size();
                              if (style == "stacked")
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
                                  if (kind == "barplot")
                                    {
                                      current_min_component = 0.0;
                                      current_max_component = 0.0;
                                    }
                                  for (i = 0; i < current_point_count; i++)
                                    {
                                      if (!std::isnan(current_component[i]))
                                        {
                                          current_min_component = grm_min(current_component[i], current_min_component);
                                          current_max_component = grm_max(current_component[i], current_max_component);
                                        }
                                    }
                                }
                            }
                          /* TODO: Add more plot types which can omit `x` */
                          else if (kind == "line" && *current_component_name == "x")
                            {
                              unsigned int y_length;
                              if (!series->hasAttribute("y"))
                                throw NotFoundError("Series is missing required attribute y.\n");
                              auto key = static_cast<std::string>(series->getAttribute("y"));
                              auto y_vec = GRM::get<std::vector<double>>((*context)[key]);
                              y_length = y_vec.size();
                              current_min_component = 0.0;
                              current_max_component = y_length - 1;
                            }
                          else if (ends_with(series->localName(), kind) &&
                                   str_equals_any(kind, "heatmap", "marginal_heatmap", "polar_heatmap", "surface") &&
                                   str_equals_any((*current_component_name), "x", "y"))
                            {
                              /* in this case `x` or `y` (or both) are missing
                               * -> set the current grm_min/max_component to the dimensions of `z`
                               *    (shifted by half a unit to center color blocks) */
                              const char *other_component_name = (*current_component_name == "x") ? "y" : "x";
                              std::vector<double> other_component;
                              unsigned int other_point_count;
                              if (series->hasAttribute(other_component_name))
                                {
                                  /* The other component is given -> the missing dimension can be calculated */
                                  unsigned int z_length;

                                  auto key = static_cast<std::string>(series->getAttribute(other_component_name));
                                  other_component = GRM::get<std::vector<double>>((*context)[key]);
                                  other_point_count = other_component.size();

                                  if (!series->hasAttribute("z"))
                                    throw NotFoundError("Series is missing required attribute z.\n");
                                  auto z_key = static_cast<std::string>(series->getAttribute("z"));
                                  auto z_vec = GRM::get<std::vector<double>>((*context)[z_key]);
                                  z_length = z_vec.size();
                                  current_point_count = z_length / other_point_count;
                                }
                              else
                                {
                                  /* A heatmap/surface without `x` and `y` values
                                   * -> dimensions can only be read from `z_dims` */
                                  int rows, cols;
                                  if (!series->hasAttribute("z_dims"))
                                    throw NotFoundError("Series is missing attribute z_dims.\n");
                                  auto z_dims_key = static_cast<std::string>(series->getAttribute("z_dims"));
                                  auto z_dims_vec = GRM::get<std::vector<int>>((*context)[z_dims_key]);
                                  cols = z_dims_vec[0];
                                  rows = z_dims_vec[1];
                                  current_point_count = (*current_component_name == "x") ? cols : rows;
                                }
                              current_min_component = 0.5;
                              current_max_component = current_point_count + 0.5;
                            }
                          else if (series->hasAttribute("indices"))
                            {
                              auto indices_key = static_cast<std::string>(series->getAttribute("indices"));
                              auto indices = GRM::get<std::vector<int>>((*context)[indices_key]);

                              if (series->hasAttribute(*current_component_name))
                                {
                                  int index_sum = 0;
                                  auto key = static_cast<std::string>(series->getAttribute(*current_component_name));
                                  current_component = GRM::get<std::vector<double>>((*context)[key]);
                                  current_point_count = current_component.size();

                                  current_max_component = 0;
                                  current_min_component = 0;
                                  auto act_index = indices.begin();
                                  index_sum += *act_index;
                                  for (i = 0; i < current_point_count; i++)
                                    {
                                      if (!std::isnan(current_component[i]))
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
                                      if (i + 1 == index_sum)
                                        {
                                          max_component = grm_max(current_max_component, max_component);
                                          min_component = grm_min(current_min_component, min_component);

                                          current_max_component = 0;
                                          current_min_component = 0;
                                          ++act_index;
                                          index_sum += *act_index;
                                        }
                                    }
                                }
                            }
                        }
                      else
                        {
                          current_min_component = static_cast<double>(
                              series->getAttribute(static_cast<std::string>(current_range_keys->series) + "_min"));
                          current_max_component = static_cast<double>(
                              series->getAttribute(static_cast<std::string>(current_range_keys->series) + "_max"));
                        }

                      if (current_min_component != DBL_MAX && current_max_component != -DBL_MAX)
                        {
                          if (static_cast<std::string>(current_range_keys->series) == "x_range")
                            {
                              series->setAttribute("x_range_min", current_min_component);
                              series->setAttribute("x_range_max", current_max_component);
                            }
                          else if (static_cast<std::string>(current_range_keys->series) == "y_range")
                            {
                              series->setAttribute("y_range_min", current_min_component);
                              series->setAttribute("y_range_max", current_max_component);
                            }
                          else if (static_cast<std::string>(current_range_keys->series) == "z_range")
                            {
                              series->setAttribute("z_range_min", current_min_component);
                              series->setAttribute("z_range_max", current_max_component);
                            }
                          else if (static_cast<std::string>(current_range_keys->series) == "c_range")
                            {
                              series->setAttribute("c_range_min", current_min_component);
                              series->setAttribute("c_range_max", current_max_component);
                            }
                        }
                      min_component = grm_min(current_min_component, min_component);
                      max_component = grm_max(current_max_component, max_component);
                      series_count++;
                    }
                }
              if (element->hasAttribute(static_cast<std::string>(current_range_keys->subplot) + "_min") &&
                  element->hasAttribute(static_cast<std::string>(current_range_keys->subplot) + "_max"))
                {
                  min_component = static_cast<double>(
                      element->getAttribute(static_cast<std::string>(current_range_keys->subplot) + "_min"));
                  max_component = static_cast<double>(
                      element->getAttribute(static_cast<std::string>(current_range_keys->subplot) + "_max"));
                  if (static_cast<std::string>(current_range_keys->subplot) == "x_lim")
                    {
                      element->setAttribute("_x_lim_min", min_component);
                      element->setAttribute("_x_lim_max", max_component);
                    }
                  else if (static_cast<std::string>(current_range_keys->subplot) == "y_lim")
                    {
                      element->setAttribute("_y_lim_min", min_component);
                      element->setAttribute("_y_lim_max", max_component);
                    }
                  else if (static_cast<std::string>(current_range_keys->subplot) == "z_lim")
                    {
                      element->setAttribute("_z_lim_min", min_component);
                      element->setAttribute("_z_lim_max", max_component);
                    }
                  else if (static_cast<std::string>(current_range_keys->subplot) == "c_lim")
                    {
                      element->setAttribute("_c_lim_min", min_component);
                      element->setAttribute("_c_lim_max", max_component);
                    }
                }
              else if (min_component != DBL_MAX && max_component != -DBL_MAX)
                {
                  if (kind == "quiver")
                    {
                      bool x_log = false, y_log = false;
                      if (element->hasAttribute("x_log")) x_log = static_cast<int>(element->getAttribute("x_log"));
                      if (element->hasAttribute("y_log")) y_log = static_cast<int>(element->getAttribute("y_log"));

                      step = grm_max(findMaxStep(current_point_count, current_component), step);
                      if (step > 0.0)
                        {
                          min_component -= step;
                          max_component += step;
                        }
                      if (static_cast<std::string>(current_range_keys->subplot) == "x_lim" && x_log)
                        {
                          min_component = (min_component > 0) ? min_component : 1;
                          max_component = (max_component > 0) ? max_component : min_component + 1;
                        }
                      if (static_cast<std::string>(current_range_keys->subplot) == "y_lim" && y_log)
                        {
                          min_component = (min_component > 0) ? min_component : 1;
                          max_component = (max_component > 0) ? max_component : min_component + 1;
                        }
                    }
                  // TODO: Support mixed orientations
                  std::string orientation = PLOT_DEFAULT_ORIENTATION;
                  if (kind != "marginal_heatmap")
                    {
                      for (const auto &series : central_region->children())
                        {
                          if (series->hasAttribute("orientation"))
                            orientation = static_cast<std::string>(series->getAttribute("orientation"));
                          if (!starts_with(series->localName(), "series")) continue;
                        }
                    }
                  if (static_cast<std::string>(current_range_keys->subplot) == "x_lim")
                    {
                      if (orientation == "horizontal")
                        {
                          element->setAttribute("_x_lim_min", min_component);
                          element->setAttribute("_x_lim_max", max_component);
                        }
                      else
                        {
                          element->setAttribute("_y_lim_min", min_component);
                          element->setAttribute("_y_lim_max", max_component);
                        }
                    }
                  else if (static_cast<std::string>(current_range_keys->subplot) == "y_lim")
                    {
                      if (orientation == "horizontal")
                        {
                          element->setAttribute("_y_lim_min", min_component);
                          element->setAttribute("_y_lim_max", max_component);
                        }
                      else
                        {
                          element->setAttribute("_x_lim_min", min_component);
                          element->setAttribute("_x_lim_max", max_component);
                        }
                    }
                  else if (static_cast<std::string>(current_range_keys->subplot) == "z_lim")
                    {
                      element->setAttribute("_z_lim_min", min_component);
                      element->setAttribute("_z_lim_max", max_component);
                    }
                  else if (static_cast<std::string>(current_range_keys->subplot) == "c_lim")
                    {
                      element->setAttribute("_c_lim_min", min_component);
                      element->setAttribute("_c_lim_max", max_component);
                    }
                }
              ++current_range_keys;
              ++current_component_name;
            }
        }
      if (str_equals_any(kind.c_str(), 5, "polar_histogram", "polar", "polar_heatmap", "nonuniform_heatmap",
                         "nonuniform_polar_heatmap"))
        {
          if (kind == "polar_heatmap" || kind == "nonuniform_polar_heatmap" || kind == "polar")
            {
              auto ymax = static_cast<double>(element->getAttribute("_y_lim_max"));
              auto ymin = static_cast<double>(element->getAttribute("_y_lim_min"));

              element->setAttribute("r_min", ymin);
              element->setAttribute("r_max", ymax);
            }
          element->setAttribute("_x_lim_min", -1.0);
          element->setAttribute("_x_lim_max", 1.0);
          element->setAttribute("_y_lim_min", -1.0);
          element->setAttribute("_y_lim_max", 1.0);
        }

      /* For quiver plots use u^2 + v^2 as z value */
      if (kind == "quiver")
        {
          double z_min = DBL_MAX, z_max = -DBL_MAX;

          if (!element->hasAttribute("z_lim_min") || !element->hasAttribute("z_lim_max"))
            {
              for (const auto &series : central_region->children())
                {
                  if (!starts_with(series->localName(), "series")) continue;
                  double current_min_component = DBL_MAX;
                  double current_max_component = -DBL_MAX;
                  if (!series->hasAttribute("z_range_min") || !series->hasAttribute("z_range_max"))
                    {
                      unsigned int u_length, v_length;
                      if (!series->hasAttribute("u"))
                        throw NotFoundError("Quiver series is missing required attribute u-data.\n");
                      auto u_key = static_cast<std::string>(series->getAttribute("u"));
                      if (!series->hasAttribute("v"))
                        throw NotFoundError("Quiver series is missing required attribute v-data.\n");
                      auto v_key = static_cast<std::string>(series->getAttribute("v"));

                      std::vector<double> u = GRM::get<std::vector<double>>((*context)[u_key]);
                      std::vector<double> v = GRM::get<std::vector<double>>((*context)[v_key]);
                      u_length = u.size();
                      v_length = v.size();
                      if (u_length != v_length)
                        throw std::length_error("For quiver series the shape of u and v must be the same.\n");

                      for (i = 0; i < u_length; i++)
                        {
                          double z = u[i] * u[i] + v[i] * v[i];
                          current_min_component = grm_min(z, current_min_component);
                          current_max_component = grm_max(z, current_max_component);
                        }
                      current_min_component = sqrt(current_min_component);
                      current_max_component = sqrt(current_max_component);
                    }
                  else
                    {
                      current_min_component = static_cast<double>(series->getAttribute("z_range_min"));
                      current_max_component = static_cast<double>(series->getAttribute("z_range_max"));
                    }
                  z_min = grm_min(current_min_component, z_min);
                  z_max = grm_max(current_max_component, z_max);
                }
            }
          else
            {
              z_min = static_cast<double>(element->getAttribute("z_lim_min"));
              z_max = static_cast<double>(element->getAttribute("z_lim_max"));
            }
          element->setAttribute("_z_lim_min", z_min);
          element->setAttribute("_z_lim_max", z_max);
        }
      else if (str_equals_any(kind, "imshow", "isosurface", "volume"))
        {
          /* Iterate over `x` and `y` range keys (and `z` depending on `kind`) */
          current_range_keys = range_keys;
          for (i = 0; i < 3; i++)
            {
              double min_component = (kind == "imshow" ? 0.0 : -1.0);
              double max_component = 1.0;
              if (element->hasAttribute(static_cast<std::string>(current_range_keys->subplot) + "_min") &&
                  element->hasAttribute(static_cast<std::string>(current_range_keys->subplot) + "_max"))
                {
                  min_component = static_cast<double>(
                      element->getAttribute(static_cast<std::string>(current_range_keys->subplot) + "_min"));
                  max_component = static_cast<double>(
                      element->getAttribute(static_cast<std::string>(current_range_keys->subplot) + "_max"));
                }
              if (static_cast<std::string>(current_range_keys->subplot) == "x_lim")
                {
                  element->setAttribute("_x_lim_min", min_component);
                  element->setAttribute("_x_lim_max", max_component);
                }
              else if (static_cast<std::string>(current_range_keys->subplot) == "y_lim")
                {
                  element->setAttribute("_y_lim_min", min_component);
                  element->setAttribute("_y_lim_max", max_component);
                }
              else if (static_cast<std::string>(current_range_keys->subplot) == "z_lim")
                {
                  element->setAttribute("_z_lim_min", min_component);
                  element->setAttribute("_z_lim_max", max_component);
                }
              else if (static_cast<std::string>(current_range_keys->subplot) == "c_lim")
                {
                  element->setAttribute("_c_lim_min", min_component);
                  element->setAttribute("_c_lim_max", max_component);
                }
              ++current_range_keys;
            }
        }
      else if (kind == "barplot")
        {
          double x_min = 0.0, x_max = -DBL_MAX, y_min = DBL_MAX, y_max = -DBL_MAX;
          std::string orientation = PLOT_DEFAULT_ORIENTATION;

          if (!element->hasAttribute("x_lim_min") || !element->hasAttribute("x_lim_max"))
            {
              double xmin, xmax;

              for (const auto &series : central_region->children())
                {
                  if (!starts_with(series->localName(), "series")) continue;
                  if (series->hasAttribute("style")) style = static_cast<std::string>(series->getAttribute("style"));
                  if (str_equals_any(style, "lined", "stacked"))
                    {
                      x_max = series_count + 1;
                    }
                  else
                    {
                      auto key = static_cast<std::string>(series->getAttribute("y"));
                      auto y = GRM::get<std::vector<double>>((*context)[key]);
                      current_point_count = y.size();
                      x_max = grm_max(current_point_count + 1, x_max);
                    }
                }

              for (const auto &series : central_region->children())
                {
                  if (!starts_with(series->localName(), "series")) continue;
                  if (series->hasAttribute("orientation"))
                    orientation = static_cast<std::string>(series->getAttribute("orientation"));
                  auto key = static_cast<std::string>(series->getAttribute("y"));
                  auto y = GRM::get<std::vector<double>>((*context)[key]);
                  current_point_count = y.size();

                  if (series->hasAttribute("x_range_min") && series->hasAttribute("x_range_max"))
                    {
                      xmin = static_cast<double>(series->getAttribute("x_range_min"));
                      xmax = static_cast<double>(series->getAttribute("x_range_max"));
                      double step_x = (xmax - xmin) / (current_point_count - 1);
                      if (!str_equals_any(style, "lined", "stacked"))
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
                }
            }
          else
            {
              x_min = static_cast<double>(element->getAttribute("x_lim_min"));
              x_max = static_cast<double>(element->getAttribute("x_lim_max"));
            }

          if (!element->hasAttribute("y_lim_min") || !element->hasAttribute("y_lim_max"))
            {
              double ymin, ymax;
              for (const auto &series : central_region->children())
                {
                  if (!starts_with(series->localName(), "series")) continue;
                  if (series->hasAttribute("style")) style = static_cast<std::string>(series->getAttribute("style"));
                  if (series->hasAttribute("orientation"))
                    orientation = static_cast<std::string>(series->getAttribute("orientation"));
                  auto key = static_cast<std::string>(series->getAttribute("y"));
                  auto y = GRM::get<std::vector<double>>((*context)[key]);
                  current_point_count = y.size();

                  if (series->hasAttribute("y_range_min") && series->hasAttribute("y_range_max"))
                    {
                      ymin = static_cast<double>(series->getAttribute("y_range_min"));
                      ymax = static_cast<double>(series->getAttribute("y_range_max"));
                      y_min = grm_min(y_min, ymin);
                      if (style == "stacked")
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
                }
            }
          else
            {
              y_min = static_cast<double>(element->getAttribute("y_lim_min"));
              y_max = static_cast<double>(element->getAttribute("y_lim_max"));
            }

          if (orientation == "horizontal")
            {
              element->setAttribute("_x_lim_min", x_min);
              element->setAttribute("_x_lim_max", x_max);
              element->setAttribute("_y_lim_min", y_min);
              element->setAttribute("_y_lim_max", y_max);
            }
          else
            {
              element->setAttribute("_x_lim_min", y_min);
              element->setAttribute("_x_lim_max", y_max);
              element->setAttribute("_y_lim_min", x_min);
              element->setAttribute("_y_lim_max", x_max);
            }
        }
      else if (kind == "hist")
        {
          double x_min = 0.0, x_max = 0.0, y_min = 0.0, y_max = 0.0;
          std::string orientation = PLOT_DEFAULT_ORIENTATION;
          bool is_horizontal;

          if (!element->hasAttribute("y_lim_min") || !element->hasAttribute("y_lim_max"))
            {
              for (const auto &series : central_region->children())
                {
                  if (series->hasAttribute("orientation"))
                    orientation = static_cast<std::string>(series->getAttribute("orientation"));
                  is_horizontal = orientation == "horizontal";
                  if (!starts_with(series->localName(), "series")) continue;
                  double current_y_min = DBL_MAX, current_y_max = -DBL_MAX;

                  if (!series->hasAttribute("bins")) histBins(series, context);
                  auto bins_key = static_cast<std::string>(series->getAttribute("bins"));
                  bins = GRM::get<std::vector<double>>((*context)[bins_key]);
                  auto num_bins = (int)bins.size();

                  for (i = 0; i < num_bins; i++)
                    {
                      current_y_min = grm_min(current_y_min, bins[i]);
                      current_y_max = grm_max(current_y_max, bins[i]);
                    }

                  y_min = grm_min(current_y_min, y_min);
                  y_max = grm_max(current_y_max, y_max);
                  x_max = current_point_count - 1;
                  if (series->hasAttribute("y_range_min") && series->hasAttribute("y_range_max"))
                    {
                      y_min = static_cast<double>(series->getAttribute("y_range_min"));
                      y_max = static_cast<double>(series->getAttribute("y_range_max"));
                    }
                  if (series->hasAttribute("x_range_min") && series->hasAttribute("x_range_max"))
                    {
                      x_min = static_cast<double>(series->getAttribute("x_range_min"));
                      x_max = static_cast<double>(series->getAttribute("x_range_max"));
                    }
                }
              if (is_horizontal)
                {
                  element->setAttribute("_x_lim_min", x_min);
                  element->setAttribute("_x_lim_max", x_max);
                  element->setAttribute("_y_lim_min", y_min);
                  element->setAttribute("_y_lim_max", y_max);
                }
              else
                {
                  element->setAttribute("_x_lim_min", y_min);
                  element->setAttribute("_x_lim_max", y_max);
                  element->setAttribute("_y_lim_min", x_min);
                  element->setAttribute("_y_lim_max", x_max);
                }
            }
        }
      else if (str_equals_any(kind, "stem", "stairs"))
        {
          double x_min = 0.0, x_max = 0.0, y_min = 0.0, y_max = 0.0;
          std::string orientation = PLOT_DEFAULT_ORIENTATION;
          bool is_horizontal;

          for (const auto &series : central_region->children())
            {
              if (series->hasAttribute("orientation"))
                orientation = static_cast<std::string>(series->getAttribute("orientation"));
              is_horizontal = orientation == "horizontal";
              if (!starts_with(series->localName(), "series")) continue;
              if (series->hasAttribute("x_range_min") && series->hasAttribute("x_range_max") &&
                  !(element->hasAttribute("x_lim_min") && element->hasAttribute("x_lim_max")))
                {
                  x_min = static_cast<double>(series->getAttribute("x_range_min"));
                  x_max = static_cast<double>(series->getAttribute("x_range_max"));
                  if (is_horizontal)
                    {
                      element->setAttribute("_x_lim_min", x_min);
                      element->setAttribute("_x_lim_max", x_max);
                    }
                  else
                    {
                      element->setAttribute("_y_lim_min", x_min);
                      element->setAttribute("_y_lim_max", x_max);
                    }
                }
              if (series->hasAttribute("y_range_min") && series->hasAttribute("y_range_max") &&
                  !(element->hasAttribute("y_lim_min") && element->hasAttribute("y_lim_max")))
                {
                  y_min = static_cast<double>(series->getAttribute("y_range_min"));
                  y_max = static_cast<double>(series->getAttribute("y_range_max"));
                  if (is_horizontal)
                    {
                      element->setAttribute("_y_lim_min", y_min);
                      element->setAttribute("_y_lim_max", y_max);
                    }
                  else
                    {
                      element->setAttribute("_x_lim_min", y_min);
                      element->setAttribute("_x_lim_max", y_max);
                    }
                }
            }
        }
    }
}

static void processSideRegion(const std::shared_ptr<GRM::Element> &element,
                              const std::shared_ptr<GRM::Context> &context)
{
  int child_id = 0;
  del_values del = del_values::update_without_default;
  std::shared_ptr<GRM::Element> plot_parent = element;
  getPlotParent(plot_parent);

  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  if (element->hasAttribute("text_content"))
    {
      auto kind = static_cast<std::string>(plot_parent->getAttribute("kind"));
      auto text = static_cast<std::string>(element->getAttribute("text_content"));
      auto location = static_cast<std::string>(element->getAttribute("location"));

      if (((del != del_values::update_without_default && del != del_values::update_with_default)) && !text.empty() &&
          kind != "imshow" &&
          (std::find(kinds_3d.begin(), kinds_3d.end(), kind) == kinds_3d.end() || location == "top"))
        {
          auto text_elem = global_render->createTextRegion();
          text_elem->setAttribute("_child_id", child_id++);
          element->appendChild(text_elem);
        }
      else
        {
          auto text_elem = element->querySelectors("text_region[_child_id=\"" + std::to_string(child_id++) + "\"]");
          if (text_elem) global_render->createTextRegion(text_elem);
        }
    }

  calculateViewport(element);
  applyMoveTransformation(element);
  GRM::Render::processViewport(element);
  GRM::Render::processWindow(element);    /* needs to be set before space 3d is processed */
  GRM::Render::processScale(plot_parent); /* needs to be set before flip is processed */
}

static void processSidePlotRegion(const std::shared_ptr<GRM::Element> &element,
                                  const std::shared_ptr<GRM::Context> &context)
{
  calculateViewport(element);
  applyMoveTransformation(element);
}

static void processCoordinateSystem(const std::shared_ptr<GRM::Element> &element,
                                    const std::shared_ptr<GRM::Context> &context)
{
  int child_id = 0;
  del_values del = del_values::update_without_default;
  std::shared_ptr<GRM::Element> polar_axes, axis, grid_3d, axes_3d, titles_3d;
  std::string type;
  auto plot_parent = element->parentElement();
  getPlotParent(plot_parent);
  auto kind = static_cast<std::string>(plot_parent->getAttribute("kind"));

  del = del_values(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  for (const auto &parent_child : element->parentElement()->children())
    {
      if (parent_child->localName() == "series_barplot" || parent_child->localName() == "series_stem")
        {
          auto series_kind = static_cast<std::string>(parent_child->getAttribute("kind"));
          if (series_kind == "barplot" || series_kind == "stem")
            {
              if (!element->hasAttribute("y_line")) element->setAttribute("y_line", true);
            }
          break;
        }
    }
  /* 0-line */
  if (element->hasAttribute("y_line") && static_cast<int>(element->getAttribute("y_line")))
    {
      drawYLine(element, context);
    }
  else if (element->querySelectors("[name=\"y_line\"]"))
    {
      auto line = element->querySelectors("[name=\"y_line\"]");
      line->remove();
    }

  type = static_cast<std::string>(element->getAttribute("plot_type"));
  if (type == "3d")
    {
      std::shared_ptr<GRM::Element> central_region, central_region_parent = plot_parent;
      if (kind == "marginal_heatmap") central_region_parent = element->querySelectors("marginal_heatmap_plot");

      for (const auto &child : central_region_parent->children())
        {
          if (child->localName() == "central_region")
            {
              central_region = child;
              break;
            }
        }

      // 3d plots are always in keep_aspect_ratio mode so the scaling with the aspect_ratio isn't needed here
      double char_height = PLOT_3D_CHAR_HEIGHT;
      auto diag_factor = static_cast<double>(central_region->getAttribute("diag_factor"));
      element->setAttribute("char_height", char_height * diag_factor);
      processCharHeight(element);
    }

  if (element->hasAttribute("hide") && static_cast<int>(element->getAttribute("hide")))
    {
      del = del_values::recreate_own_children;
      clearOldChildren(&del, element);
      return;
    }

  if (type == "polar")
    {
      // create polar coordinate system
      int angle_ticks = 8, phiflip = 0;
      std::string norm = "count";
      double tick = 0.0;

      if (element->hasAttribute("angle_ticks"))
        {
          angle_ticks = static_cast<int>(element->getAttribute("angle_ticks"));
        }

      if (element->hasAttribute("phi_flip"))
        {
          phiflip = static_cast<int>(element->getAttribute("phi_flip"));
        }

      if (kind == "polar_histogram")
        {
          if (element->hasAttribute("normalization"))
            {
              norm = static_cast<std::string>(element->getAttribute("normalization"));
            }
          tick = 1.0;
        }

      if (del != del_values::update_without_default && del != del_values::update_with_default)
        {
          polar_axes = global_render->createDrawPolarAxes(angle_ticks, kind, phiflip, norm, tick, 0.0);
          polar_axes->setAttribute("_child_id", child_id++);
          element->append(polar_axes);
        }
      else
        {
          polar_axes = element->querySelectors("polar_axes[_child_id=" + std::to_string(child_id++) + "]");
          if (polar_axes != nullptr)
            global_render->createDrawPolarAxes(angle_ticks, kind, phiflip, norm, tick, 0.0, polar_axes);
        }
    }
  else
    {
      int tick_orientation = 1;
      bool x_grid =
          (element->hasAttribute("x_grid")) ? static_cast<int>(element->getAttribute("x_grid")) : PLOT_DEFAULT_XGRID;
      bool y_grid =
          (element->hasAttribute("y_grid")) ? static_cast<int>(element->getAttribute("y_grid")) : PLOT_DEFAULT_YGRID;

      global_render->setLineColorInd(element, 1);
      global_render->setLineWidth(element, 1);
      processLineColorInd(element);
      processLineWidth(element);

      if (type == "3d")
        {
          bool z_grid;
          if (element->hasAttribute("z_grid"))
            {
              z_grid = static_cast<int>(element->getAttribute("z_grid"));
            }
          else
            {
              z_grid = PLOT_DEFAULT_ZGRID;
              element->setAttribute("z_grid", z_grid);
            }

          if (del != del_values::update_without_default && del != del_values::update_with_default)
            {
              grid_3d = global_render->createEmptyGrid3d(x_grid, false, z_grid);
              grid_3d->setAttribute("_child_id", child_id++);
              element->append(grid_3d);
            }
          else
            {
              grid_3d = element->querySelectors("grid_3d[_child_id=" + std::to_string(child_id++) + "]");
              if (grid_3d != nullptr) global_render->createEmptyGrid3d(x_grid, false, z_grid, grid_3d);
            }
          if (grid_3d != nullptr)
            {
              global_render->setOriginPosition3d(grid_3d, "low", "high", "low");
              grid_3d->setAttribute("x_major", 2);
              grid_3d->setAttribute("y_major", 0);
              grid_3d->setAttribute("z_major", 2);
            }

          if (del != del_values::update_without_default && del != del_values::update_with_default)
            {
              grid_3d = global_render->createEmptyGrid3d(false, y_grid, false);
              grid_3d->setAttribute("_child_id", child_id++);
              element->append(grid_3d);
            }
          else
            {
              grid_3d = element->querySelectors("grid_3d[_child_id=" + std::to_string(child_id++) + "]");
              if (grid_3d != nullptr) global_render->createEmptyGrid3d(false, y_grid, false, grid_3d);
            }
          if (grid_3d != nullptr)
            {
              global_render->setOriginPosition3d(grid_3d, "low", "high", "low");
              grid_3d->setAttribute("x_major", 0);
              grid_3d->setAttribute("y_major", 2);
              grid_3d->setAttribute("z_major", 0);
            }

          if (del != del_values::update_without_default && del != del_values::update_with_default)
            {
              axes_3d = global_render->createEmptyAxes3d(-tick_orientation);
              axes_3d->setAttribute("_child_id", child_id++);
              element->append(axes_3d);
            }
          else
            {
              axes_3d = element->querySelectors("axes_3d[_child_id=" + std::to_string(child_id++) + "]");
              if (axes_3d != nullptr)
                {
                  tick_orientation = -tick_orientation;
                  if (axes_3d->hasAttribute("tick_orientation") && del != del_values::update_with_default)
                    tick_orientation = static_cast<int>(axes_3d->getAttribute("tick_orientation"));
                  global_render->createEmptyAxes3d(tick_orientation, axes_3d);
                }
            }
          if (axes_3d != nullptr)
            {
              global_render->setOriginPosition3d(axes_3d, "low", "low", "low");
              axes_3d->setAttribute("y_tick", 0);
              axes_3d->setAttribute("y_major", 0);
              axes_3d->setAttribute("z_index", 7);
            }

          if (del != del_values::update_without_default && del != del_values::update_with_default)
            {
              axes_3d = global_render->createEmptyAxes3d(tick_orientation);
              axes_3d->setAttribute("_child_id", child_id++);
              element->append(axes_3d);
            }
          else
            {
              axes_3d = element->querySelectors("axes_3d[_child_id=" + std::to_string(child_id++) + "]");
              if (axes_3d != nullptr)
                {
                  if (axes_3d->hasAttribute("tick_orientation") && del != del_values::update_with_default)
                    tick_orientation = static_cast<int>(axes_3d->getAttribute("tick_orientation"));
                  global_render->createEmptyAxes3d(tick_orientation, axes_3d);
                }
            }
          if (axes_3d != nullptr)
            {
              global_render->setOriginPosition3d(axes_3d, "high", "low", "low");
              axes_3d->setAttribute("x_tick", 0);
              axes_3d->setAttribute("z_tick", 0);
              axes_3d->setAttribute("x_major", 0);
              axes_3d->setAttribute("z_major", 0);
              axes_3d->setAttribute("z_index", 7);
            }

          std::string x_label =
              (element->hasAttribute("x_label")) ? static_cast<std::string>(element->getAttribute("x_label")) : "";
          std::string y_label =
              (element->hasAttribute("y_label")) ? static_cast<std::string>(element->getAttribute("y_label")) : "";
          std::string z_label =
              (element->hasAttribute("z_label")) ? static_cast<std::string>(element->getAttribute("z_label")) : "";
          if (!x_label.empty() || !y_label.empty() || !z_label.empty())
            {
              if (del != del_values::update_without_default && del != del_values::update_with_default)
                {
                  titles_3d = global_render->createTitles3d(x_label, y_label, z_label);
                  titles_3d->setAttribute("_child_id", child_id++);
                  element->append(titles_3d);
                }
              else
                {
                  titles_3d = element->querySelectors("titles_3d[_child_id=" + std::to_string(child_id++) + "]");
                  if (titles_3d != nullptr)
                    titles_3d = global_render->createTitles3d(x_label, y_label, z_label, titles_3d);
                }
              if (titles_3d) titles_3d->setAttribute("z_index", 7);
            }
        }
      else
        {
          // y-axis
          if (del != del_values::update_without_default && del != del_values::update_with_default)
            {
              axis = global_render->createEmptyAxis();
              axis->setAttribute("_child_id", child_id++);
              element->append(axis);
            }
          else
            {
              axis = element->querySelectors("axis[_child_id=" + std::to_string(child_id++) + "]");
              if (axis != nullptr) axis = global_render->createEmptyAxis(axis);
            }
          if (axis != nullptr && del != del_values::update_without_default)
            {
              axis->setAttribute("axis_type", "y");
              axis->setAttribute("name", "y-axis mirrored");
            }

          // x-axis
          if (del != del_values::update_without_default && del != del_values::update_with_default)
            {
              axis = global_render->createEmptyAxis();
              axis->setAttribute("_child_id", child_id++);
              element->append(axis);
            }
          else
            {
              axis = element->querySelectors("axis[_child_id=" + std::to_string(child_id++) + "]");
              if (axis != nullptr) axis = global_render->createEmptyAxis(axis);
            }
          if (axis != nullptr && del != del_values::update_without_default)
            {
              axis->setAttribute("axis_type", "x");
              axis->setAttribute("name", "x-axis mirrored");
            }
        }
    }
  applyMoveTransformation(element);
}

static void processPlot(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  std::shared_ptr<GRM::Element> central_region, central_region_parent = element;
  if (static_cast<std::string>(element->getAttribute("kind")) == "marginal_heatmap")
    central_region_parent = element->querySelectors("marginal_heatmap_plot");

  for (const auto &child : central_region_parent->children())
    {
      if (child->localName() == "central_region")
        {
          central_region = child;
          break;
        }
    }

  // set the x-, y- and z-data to NAN if the value is <= 0
  // if the plot contains the marginal_heatmap_plot the marginal_heatmap should be child in the following for
  for (const auto &child : (central_region_parent == element) ? central_region->children() : element->children())
    {
      if ((!starts_with(child->localName(), "series_") && central_region_parent == element) ||
          (child->localName() != "marginal_heatmap_plot" && central_region_parent != element))
        continue;
      auto id = static_cast<int>(global_root->getAttribute("_id"));
      auto str = std::to_string(id);

      // save the original data so it can be restored
      if (child->hasAttribute("x") && !child->hasAttribute("_x_org"))
        {
          auto x = static_cast<std::string>(child->getAttribute("x"));
          child->setAttribute("_x_org", x);
          (*context)["_x_org"].use_context_key(static_cast<std::string>(x), ""); // increase context cnt
        }
      if (child->hasAttribute("x_range_min") && !child->hasAttribute("_x_range_min_org"))
        child->setAttribute("_x_range_min_org", static_cast<double>(child->getAttribute("x_range_min")));
      if (child->hasAttribute("x_range_max") && !child->hasAttribute("_x_range_max_org"))
        child->setAttribute("_x_range_max_org", static_cast<double>(child->getAttribute("x_range_max")));
      if (child->hasAttribute("y") && !child->hasAttribute("_y_org"))
        {
          auto y = static_cast<std::string>(child->getAttribute("y"));
          child->setAttribute("_y_org", y);
          (*context)["_y_org"].use_context_key(static_cast<std::string>(y), ""); // increase context cnt
        }
      if (child->hasAttribute("y_range_min") && !child->hasAttribute("_y_range_min_org"))
        child->setAttribute("_y_range_min_org", static_cast<double>(child->getAttribute("y_range_min")));
      if (child->hasAttribute("y_range_max") && !child->hasAttribute("_y_range_max_org"))
        child->setAttribute("_y_range_max_org", static_cast<double>(child->getAttribute("y_range_max")));
      if (child->hasAttribute("z") && !child->hasAttribute("_z_org"))
        {
          auto z = static_cast<std::string>(child->getAttribute("z"));
          child->setAttribute("_z_org", z);
          (*context)["_z_org"].use_context_key(static_cast<std::string>(z), ""); // increase context cnt
        }
      if (child->hasAttribute("z_range_min") && !child->hasAttribute("_z_range_min_org"))
        child->setAttribute("_z_range_min_org", static_cast<double>(child->getAttribute("z_range_min")));
      if (child->hasAttribute("z_range_max") && !child->hasAttribute("_z_range_max_org"))
        child->setAttribute("_z_range_max_org", static_cast<double>(child->getAttribute("z_range_max")));

      // the original data must be set on the imshow series so it can be used when the imshow plot should be
      // switched to a new kind. The reason for it is that the imshow plot defines x and y as a lin-space
      // from 0 to length. The cases for log can be ignored cause log gets ignored on imshow plots.
      if (child->localName() == "series_imshow") continue;

      // save the original plot rotation so it can be restored
      if (central_region->hasAttribute("space_3d_phi") && !central_region->hasAttribute("_space_3d_phi_org"))
        central_region->setAttribute("_space_3d_phi_org",
                                     static_cast<double>(central_region->getAttribute("space_3d_phi")));
      if (central_region->hasAttribute("space_3d_theta") && !central_region->hasAttribute("_space_3d_theta_org"))
        central_region->setAttribute("_space_3d_theta_org",
                                     static_cast<double>(central_region->getAttribute("space_3d_theta")));

      if (static_cast<int>(element->getAttribute("x_log")) && child->hasAttribute("_x_org"))
        {
          double x_min = INFINITY, x_max = -INFINITY;
          auto x = static_cast<std::string>(child->getAttribute("_x_org"));
          auto x_vec = GRM::get<std::vector<double>>((*context)[x]);
          auto x_len = x_vec.size();

          if (static_cast<std::string>(element->getAttribute("kind")) == "volume")
            {
              fprintf(stderr, "The option x_log is not supported for volume. It will be set to false.\n");
              element->setAttribute("x_log", 0);
            }
          else
            {
              auto child_kind = static_cast<std::string>(child->getAttribute("kind"));
              for (int i = 0; i < x_len; i++)
                {
                  if (x_vec[i] <= 0)
                    {
                      if (child_kind == "trisurface" || child_kind == "tricontour" || child_kind == "plot3")
                        {
                          fprintf(stderr,
                                  "The option x_log is not supported for x-values <= 0. It will be set to false.\n");
                          element->setAttribute("x_log", 0);
                        }
                      else
                        {
                          x_vec[i] = NAN;
                        }
                    }
                  if (!grm_isnan(x_vec[i])) x_min = (x_min < x_vec[i]) ? x_min : x_vec[i];
                  if (!grm_isnan(x_vec[i])) x_max = (x_max > x_vec[i]) ? x_max : x_vec[i];
                }

              (*context)["x" + str] = x_vec;
              child->setAttribute("x", "x" + str);
              if (child->hasAttribute("x_range_min")) child->setAttribute("x_range_min", x_min);
              if (child->hasAttribute("x_range_max")) child->setAttribute("x_range_max", x_max);
            }
        }
      if (static_cast<int>(element->getAttribute("y_log")) && child->hasAttribute("_y_org"))
        {
          double y_min = INFINITY, y_max = -INFINITY;
          auto y = static_cast<std::string>(child->getAttribute("_y_org"));
          auto y_vec = GRM::get<std::vector<double>>((*context)[y]);
          auto y_len = y_vec.size();

          if (static_cast<std::string>(element->getAttribute("kind")) == "volume")
            {
              fprintf(stderr, "The option y_log is not supported for volume. It will be set to false.\n");
              element->setAttribute("y_log", 0);
            }
          else
            {
              auto child_kind = static_cast<std::string>(child->getAttribute("kind"));
              for (int i = 0; i < y_len; i++)
                {
                  if (y_vec[i] <= 0)
                    {
                      if (child_kind == "trisurface" || child_kind == "tricontour" || child_kind == "plot3")
                        {
                          fprintf(stderr,
                                  "The option y_log is not supported for y-values <= 0. It will be set to false.\n");
                          element->setAttribute("y_log", 0);
                        }
                      else
                        {
                          y_vec[i] = NAN;
                        }
                    }
                  if (!grm_isnan(y_vec[i])) y_min = (y_min < y_vec[i]) ? y_min : y_vec[i];
                  if (!grm_isnan(y_vec[i])) y_max = (y_max > y_vec[i]) ? y_max : y_vec[i];
                }

              (*context)["y" + str] = y_vec;
              child->setAttribute("y", "y" + str);
              if (child->hasAttribute("y_range_min")) child->setAttribute("y_range_min", y_min);
              if (child->hasAttribute("y_range_max")) child->setAttribute("y_range_max", y_max);
            }
        }
      if (static_cast<int>(element->getAttribute("z_log")) && child->hasAttribute("_z_org"))
        {
          double z_min = INFINITY, z_max = -INFINITY;
          auto z = static_cast<std::string>(child->getAttribute("_z_org"));
          auto z_vec = GRM::get<std::vector<double>>((*context)[z]);
          auto z_len = z_vec.size();

          if (static_cast<std::string>(element->getAttribute("kind")) == "volume")
            {
              fprintf(stderr, "The option y_log is not supported for volume. It will be set to false.\n");
              element->setAttribute("z_log", 0);
            }
          else
            {
              auto child_kind = static_cast<std::string>(child->getAttribute("kind"));
              for (int i = 0; i < z_len; i++)
                {
                  if (z_vec[i] <= 0)
                    {
                      if (child_kind == "trisurface" || child_kind == "tricontour" || child_kind == "plot3")
                        {
                          fprintf(stderr,
                                  "The option z_log is not supported for z-values <= 0. It will be set to false.\n");
                          element->setAttribute("z_log", 0);
                        }
                      else
                        {
                          z_vec[i] = NAN;
                        }
                    }
                  if (!grm_isnan(z_vec[i])) z_min = (z_min < z_vec[i]) ? z_min : z_vec[i];
                  if (!grm_isnan(z_vec[i])) z_max = (z_max > z_vec[i]) ? z_max : z_vec[i];
                }

              (*context)["z" + str] = z_vec;
              child->setAttribute("z", "z" + str);
              if (child->hasAttribute("z_range_min")) child->setAttribute("z_range_min", z_min);
              if (child->hasAttribute("z_range_max")) child->setAttribute("z_range_max", z_max);
            }
        }
      global_root->setAttribute("_id", ++id);
    }

  if (!element->hasAttribute("_x_lim_min") || !element->hasAttribute("_x_lim_max") ||
      !element->hasAttribute("_y_lim_min") || !element->hasAttribute("_y_lim_max") ||
      element->hasAttribute("_update_limits") && static_cast<int>(element->getAttribute("_update_limits")))
    {
      plotCoordinateRanges(element, context);
      element->removeAttribute("_update_limits");
    }
  calculateViewport(element);
  // todo: there are cases that element does not have char_height set
  // char_height is always calculated (and set in the gr) in calculateCharHeight
  // it is however not stored on the element as it can be calculated from other attributes
  if (element->hasAttribute("char_height"))
    {
      processCharHeight(element);
    }
  GRM::Render::processLimits(element);
  GRM::Render::processScale(element); /* needs to be set before flip is processed */

  /* Map for calculations on the plot level */
  static std::map<std::string,
                  std::function<void(const std::shared_ptr<GRM::Element> &, const std::shared_ptr<GRM::Context> &)>>
      kindNameToFunc{
          {std::string("barplot"), preBarplot},
          {std::string("polar_histogram"), prePolarHistogram},
      };

  for (const auto &child : central_region->children())
    {
      if (child->localName() == "series_barplot" || child->localName() == "series_polar_histogram")
        {
          auto kind = static_cast<std::string>(child->getAttribute("kind"));
          if (kindNameToFunc.find(kind) != kindNameToFunc.end())
            {
              kindNameToFunc[kind](element, context);
            }
          break;
        }
    }

  std::shared_ptr<GRM::Element> side_region;
  auto kind = static_cast<std::string>(element->getAttribute("kind"));

  if (!element->querySelectors("side_region[location=\"right\"]"))
    {
      side_region = global_render->createSideRegion("right");
      central_region_parent->append(side_region);
    }
  if (!element->querySelectors("side_region[location=\"top\"]"))
    {
      side_region = global_render->createSideRegion("top");
      central_region_parent->append(side_region);
    }
  if (!element->querySelectors("side_region[location=\"left\"]"))
    {
      side_region = global_render->createSideRegion("left");
      central_region_parent->append(side_region);
    }
  if (!element->querySelectors("side_region[location=\"bottom\"]"))
    {
      side_region = global_render->createSideRegion("bottom");
      central_region_parent->append(side_region);
    }
}

static void processSeries(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  static std::map<std::string,
                  std::function<void(const std::shared_ptr<GRM::Element>, const std::shared_ptr<GRM::Context>)>>
      seriesNameToFunc{
          {std::string("barplot"), processBarplot},
          {std::string("contour"), PushDrawableToZQueue(processContour)},
          {std::string("contourf"), PushDrawableToZQueue(processContourf)},
          {std::string("heatmap"), processHeatmap},
          {std::string("hexbin"), processHexbin},
          {std::string("hist"), processHist},
          {std::string("imshow"), processImshow},
          {std::string("isosurface"), PushDrawableToZQueue(processIsosurface)},
          {std::string("line"), processLine},
          {std::string("pie"), processPie},
          {std::string("plot3"), processPlot3},
          {std::string("polar"), processPolar},
          {std::string("polar_heatmap"), processPolarHeatmap},
          {std::string("polar_histogram"), processPolarHistogram},
          {std::string("quiver"), PushDrawableToZQueue(processQuiver)},
          {std::string("scatter"), processScatter},
          {std::string("scatter3"), processScatter3},
          {std::string("shade"), PushDrawableToZQueue(processShade)},
          {std::string("stairs"), processStairs},
          {std::string("stem"), processStem},
          {std::string("surface"), PushDrawableToZQueue(processSurface)},
          {std::string("tricontour"), PushDrawableToZQueue(processTriContour)},
          {std::string("trisurface"), PushDrawableToZQueue(processTriSurface)},
          {std::string("volume"), processVolume},
          {std::string("wireframe"), PushDrawableToZQueue(processWireframe)},
      };

  auto kind = static_cast<std::string>(element->getAttribute("kind"));
  auto plot_elem = getSubplotElement(element);

  if (auto search = seriesNameToFunc.find(kind); search != seriesNameToFunc.end())
    {
      auto f = search->second;
      f(element, context);
    }
  else
    {
      throw NotFoundError("Series is not in render implemented yet\n");
    }

  std::shared_ptr<GRM::Element> central_region, central_region_parent;

  central_region_parent = plot_elem;
  if (kind == "marginal_heatmap") central_region_parent = plot_elem->children()[0];
  for (const auto &child : central_region_parent->children())
    {
      if (child->localName() == "central_region")
        {
          central_region = child;
          break;
        }
    }
  // special case where the data of a series could inflict the window
  // its important that the series gets first processed so the changed data gets used inside plotCoordinateRanges
  if (element->parentElement()->parentElement()->localName() == "plot" &&
      !static_cast<int>(central_region->getAttribute("keep_window")))
    {
      plotCoordinateRanges(element->parentElement()->parentElement(), global_render->getContext());
    }
}

static void processElement(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for all kinds of elements
   *
   * \param[in] element The GRM::Element containing attributes and data keys
   * \param[in] context The GRM::Context containing the actual data
   */

  // Map used for processing all kinds of elements
  bool update_required = static_cast<int>(element->getAttribute("_update_required"));
  static std::map<std::string,
                  std::function<void(const std::shared_ptr<GRM::Element>, const std::shared_ptr<GRM::Context>)>>
      elemStringToFunc{
          {std::string("axes_3d"), PushDrawableToZQueue(processAxes3d)},
          {std::string("axis"), processAxis},
          {std::string("bar"), processBar},
          {std::string("cellarray"), PushDrawableToZQueue(processCellArray)},
          {std::string("colorbar"), processColorbar},
          {std::string("coordinate_system"), processCoordinateSystem},
          {std::string("error_bar"), processErrorBar},
          {std::string("error_bars"), processErrorBars},
          {std::string("legend"), processLegend},
          {std::string("polar_axes"), processPolarAxes},
          {std::string("draw_arc"), PushDrawableToZQueue(processDrawArc)},
          {std::string("draw_graphics"), PushDrawableToZQueue(processDrawGraphics)},
          {std::string("draw_image"), PushDrawableToZQueue(processDrawImage)},
          {std::string("draw_rect"), PushDrawableToZQueue(processDrawRect)},
          {std::string("fill_arc"), PushDrawableToZQueue(processFillArc)},
          {std::string("fill_area"), PushDrawableToZQueue(processFillArea)},
          {std::string("fill_rect"), PushDrawableToZQueue(processFillRect)},
          {std::string("grid_3d"), PushDrawableToZQueue(processGrid3d)},
          {std::string("grid_line"), PushDrawableToZQueue(processGridLine)},
          {std::string("integral"), processIntegral},
          {std::string("integral_group"), processIntegralGroup},
          {std::string("isosurface_render"), PushDrawableToZQueue(processIsosurfaceRender)},
          {std::string("layout_grid"), PushDrawableToZQueue(processLayoutGrid)},
          {std::string("marginal_heatmap_plot"), processMarginalHeatmapPlot},
          {std::string("nonuniform_polar_cellarray"), PushDrawableToZQueue(processNonUniformPolarCellArray)},
          {std::string("nonuniform_cellarray"), PushDrawableToZQueue(processNonuniformcellarray)},
          {std::string("panzoom"), PushDrawableToZQueue(processPanzoom)},
          {std::string("pie_segment"), processPieSegment},
          {std::string("polar_bar"), processPolarBar},
          {std::string("polar_cellarray"), PushDrawableToZQueue(processPolarCellArray)},
          {std::string("polyline"), PushDrawableToZQueue(processPolyline)},
          {std::string("polyline_3d"), PushDrawableToZQueue(processPolyline3d)},
          {std::string("polymarker"), PushDrawableToZQueue(processPolymarker)},
          {std::string("polymarker_3d"), PushDrawableToZQueue(processPolymarker3d)},
          {std::string("series"), processSeries},
          {std::string("side_region"), processSideRegion},
          {std::string("side_plot_region"), processSidePlotRegion},
          {std::string("text"), PushDrawableToZQueue(processText)},
          {std::string("text_region"), processTextRegion},
          {std::string("tick"), PushDrawableToZQueue(processTick)},
          {std::string("tick_group"), processTickGroup},
          {std::string("titles_3d"), PushDrawableToZQueue(processTitles3d)},
      };

  /* Modifier */
  if (str_equals_any(element->localName(), "axes_text_group", "axis", "central_region", "figure", "plot", "label",
                     "labels_group", "root", "layout_grid_element", "side_region", "text_region", "side_plot_region",
                     "tick_group"))
    {
      bool old_state = automatic_update;
      automatic_update = false;
      /* check if figure is active; skip inactive elements */
      if (element->localName() == "figure")
        {
          if (!static_cast<int>(element->getAttribute("active"))) return;
          if (global_root->querySelectorsAll("draw_graphics").empty()) plotProcessWsWindowWsViewport(element, context);
        }
      if (element->localName() == "plot")
        {
          std::shared_ptr<GRM::Element> central_region_parent = element;
          processPlot(element, context);
          if (static_cast<std::string>(element->getAttribute("kind")) == "marginal_heatmap")
            central_region_parent = element->children()[0]; // if the kind is marginal_heatmap plot can only has 1 child
                                                            // and this child is the marginal_heatmap_plot

          if (central_region_parent != element) calculateViewport(central_region_parent);

          for (const auto &child : central_region_parent->children())
            {
              if (child->localName() == "central_region")
                {
                  calculateViewport(child);
                  GRM::Render::calculateCharHeight(child);
                  GRM::Render::processWindow(child);
                  GRM::Render::processScale(element);
                  break;
                }
            }
        }
      else
        {
          calculateViewport(element);
        }
      if (element->localName() == "side_region") processSideRegion(element, context);
      if (element->localName() == "text_region") processTextRegion(element, context);
      if (element->localName() == "side_plot_region") processSidePlotRegion(element, context);
      if (element->localName() == "axis") processAxis(element, context);
      if (element->localName() == "tick_group") processTickGroup(element, context);
      GRM::Render::processAttributes(element);
      automatic_update = old_state;
      if (element->localName() != "root") applyMoveTransformation(element);
    }
  else
    {
      if (element->localName() == "marginal_heatmap_plot")
        {
          bool old_state = automatic_update;
          automatic_update = false;
          calculateViewport(element);
          applyMoveTransformation(element);
          automatic_update = old_state;
        }
      // TODO: something like series_contour shouldn't be in this list
      if (!automatic_update ||
          ((static_cast<int>(global_root->getAttribute("_modified")) &&
            (str_equals_any(element->localName(), "axes_3d", "cellarray", "colorbar", "draw_arc", "draw_image",
                            "draw_rect", "fill_arc", "fill_area", "fill_rect", "grid", "grid_3d", "legend",
                            "nonuniform_polar_cellarray", "nonuniform_cellarray", "polar_cellarray", "polyline",
                            "polyline_3d", "polymarker", "polymarker_3d", "series_contour", "series_contourf", "text",
                            "titles_3d", "coordinate_system", "series_hexbin", "series_isosurface", "series_quiver",
                            "series_shade", "series_surface", "series_tricontour", "series_trisurface",
                            "series_volume") ||
             !element->hasChildNodes())) ||
           update_required))
        {
          // elements without children are the draw-functions which need to be processed everytime, else there could
          // be problems with overlapping elements
          std::string local_name = getLocalName(element);

          bool old_state = automatic_update;
          automatic_update = false;
          /* The attributes of drawables (except for the z_index itself) are being processed when the z_queue is being
           * processed */
          if (element->hasAttribute("viewport_x_min")) calculateViewport(element);
          if (isDrawable(element))
            {
              if (element->hasAttribute("z_index")) processZIndex(element);
            }
          else
            {
              GRM::Render::processAttributes(element);
            }

          if (auto search = elemStringToFunc.find(local_name); search != elemStringToFunc.end())
            {
              auto f = search->second;
              f(element, context);
            }
          else
            {
              throw NotFoundError("No dom render function found for element with local name: " + element->localName() +
                                  "\n");
            }

          // reset _update_required
          element->setAttribute("_update_required", false);
          element->setAttribute("_delete_children", 0);
          if (update_required)
            {
              for (const auto &child : element->children())
                {
                  if (!str_equals_any(element->localName(), "axes_text_group", "figure", "plot", "label",
                                      "labels_group", "root", "layout_grid_element"))
                    {
                      child->setAttribute("_update_required", true);
                      resetOldBoundingBoxes(child);
                    }
                }
            }
          automatic_update = old_state;
        }
      else if (automatic_update && static_cast<int>(global_root->getAttribute("_modified")) ||
               element->parentElement()->parentElement()->hasAttribute("marginal_heatmap_side_plot"))
        {
          bool old_state = automatic_update;
          automatic_update = false;
          GRM::Render::processAttributes(element);
          automatic_update = old_state;
        }
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ render functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

static void renderHelper(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Recursive helper function for render; Not part of render class
   * Only renders / processes children if the parent is in parent_types (group etc.)
   * Used for traversing the tree
   *
   * \param[in] element A GRM::Element
   * \param[in] context A GRM::Context
   */
  gr_savestate();
  z_index_manager.savestate();
  custom_color_index_manager.savestate();

  bool bounding_boxes = (getenv("GRDISPLAY") && strcmp(getenv("GRDISPLAY"), "edit") == 0);

  if (bounding_boxes && !isDrawable(element))
    {
      gr_setbboxcallback(bounding_id, &receiverFunction);
      bounding_map[bounding_id] = element;
      bounding_id++;
    }

  processElement(element, context);
  if (element->hasChildNodes() && parent_types.count(element->localName()))
    {
      for (const auto &child : element->children())
        {
          if (child->localName() == "figure" && !static_cast<int>(child->getAttribute("active"))) continue;
          renderHelper(child, context);
        }
    }
  if (bounding_boxes && !isDrawable(element))
    {
      gr_cancelbboxcallback();
    }

  custom_color_index_manager.restorestate();
  z_index_manager.restorestate();
  gr_restorestate();
}

static void missingBboxCalculator(const std::shared_ptr<GRM::Element> &element,
                                  const std::shared_ptr<GRM::Context> &context, double *bbox_xmin = nullptr,
                                  double *bbox_xmax = nullptr, double *bbox_ymin = nullptr, double *bbox_ymax = nullptr)
{
  double elem_bbox_xmin = DBL_MAX, elem_bbox_xmax = -DBL_MAX, elem_bbox_ymin = DBL_MAX, elem_bbox_ymax = -DBL_MAX;

  if (element->hasAttribute("_bbox_id") && static_cast<int>(element->getAttribute("_bbox_id")) != -1)
    {
      *bbox_xmin = static_cast<double>(element->getAttribute("_bbox_x_min"));
      *bbox_xmax = static_cast<double>(element->getAttribute("_bbox_x_max"));
      *bbox_ymin = static_cast<double>(element->getAttribute("_bbox_y_min"));
      *bbox_ymax = static_cast<double>(element->getAttribute("_bbox_y_max"));
    }
  else
    {
      if (element->hasChildNodes() && parent_types.count(element->localName()))
        {
          for (const auto &child : element->children())
            {
              double tmp_bbox_xmin = DBL_MAX, tmp_bbox_xmax = -DBL_MAX, tmp_bbox_ymin = DBL_MAX,
                     tmp_bbox_ymax = -DBL_MAX;
              missingBboxCalculator(child, context, &tmp_bbox_xmin, &tmp_bbox_xmax, &tmp_bbox_ymin, &tmp_bbox_ymax);
              elem_bbox_xmin = grm_min(elem_bbox_xmin, tmp_bbox_xmin);
              elem_bbox_xmax = grm_max(elem_bbox_xmax, tmp_bbox_xmax);
              elem_bbox_ymin = grm_min(elem_bbox_ymin, tmp_bbox_ymin);
              elem_bbox_ymax = grm_max(elem_bbox_ymax, tmp_bbox_ymax);
            }
        }
    }

  if (element->localName() != "root" &&
      (!element->hasAttribute("_bbox_id") || static_cast<int>(element->getAttribute("_bbox_id")) == -1))
    {
      if (!(elem_bbox_xmin == DBL_MAX || elem_bbox_xmax == -DBL_MAX || elem_bbox_ymin == DBL_MAX ||
            elem_bbox_ymax == -DBL_MAX))
        {
          if (static_cast<int>(element->getAttribute("_bbox_id")) != -1)
            {
              element->setAttribute("_bbox_id", bounding_id++);
            }
          element->setAttribute("_bbox_x_min", elem_bbox_xmin);
          element->setAttribute("_bbox_x_max", elem_bbox_xmax);
          element->setAttribute("_bbox_y_min", elem_bbox_ymin);
          element->setAttribute("_bbox_y_max", elem_bbox_ymax);
        }

      if (bbox_xmin != nullptr) *bbox_xmin = elem_bbox_xmin;
      if (bbox_xmax != nullptr) *bbox_xmax = elem_bbox_xmax;
      if (bbox_ymin != nullptr) *bbox_ymin = elem_bbox_ymin;
      if (bbox_ymax != nullptr) *bbox_ymax = elem_bbox_ymax;
    }
}

static void renderZQueue(const std::shared_ptr<GRM::Context> &context)
{
  z_queue_is_being_rendered = true;
  bool bounding_boxes = (getenv("GRDISPLAY") && strcmp(getenv("GRDISPLAY"), "edit") == 0);

  gr_savestate();
  for (; !z_queue.empty(); z_queue.pop())
    {
      const auto &drawable = z_queue.top();
      auto element = drawable->getElement();
      if (!element->parentElement()) continue;

      if (bounding_boxes)
        {
          gr_setbboxcallback(bounding_id, &receiverFunction);
          bounding_map[bounding_id] = element;
          bounding_id++;
        }

      custom_color_index_manager.selectcontext(drawable->getGrContextId());
      drawable->draw();

      if (bounding_boxes)
        {
          gr_cancelbboxcallback();
        }
    }
  gr_context_id_manager.markAllIdsAsUnused();
  parent_to_context = {};
  gr_unselectcontext();
  gr_restorestate();
  z_queue_is_being_rendered = false;
}

static void initializeGridElements(const std::shared_ptr<GRM::Element> &element, grm::Grid *grid)
{
  if (element->hasChildNodes())
    {
      for (const auto &child : element->children())
        {
          if (child->localName() != "layout_grid_element" && child->localName() != "layout_grid")
            {
              return;
            }

          double abs_height = (child->hasAttribute("absolute_height"))
                                  ? static_cast<double>(child->getAttribute("absolute_height"))
                                  : -1;
          double abs_width =
              (child->hasAttribute("absolute_width")) ? static_cast<double>(child->getAttribute("absolute_width")) : -1;
          int abs_height_pxl = (child->hasAttribute("absolute_height_pxl"))
                                   ? static_cast<int>(child->getAttribute("absolute_height_pxl"))
                                   : -1;
          int abs_width_pxl = (child->hasAttribute("absolute_width_pxl"))
                                  ? static_cast<int>(child->getAttribute("absolute_width_pxl"))
                                  : -1;
          double relative_height = (child->hasAttribute("relative_height"))
                                       ? static_cast<double>(child->getAttribute("relative_height"))
                                       : -1;
          auto relative_width =
              (child->hasAttribute("relative_width")) ? static_cast<double>(child->getAttribute("relative_width")) : -1;
          auto aspect_ratio =
              (child->hasAttribute("aspect_ratio")) ? static_cast<double>(child->getAttribute("aspect_ratio")) : -1;
          auto fit_parents_height = static_cast<int>(child->getAttribute("fit_parents_height"));
          auto fit_parents_width = static_cast<int>(child->getAttribute("fit_parents_width"));
          auto row_start = static_cast<int>(child->getAttribute("start_row"));
          auto row_stop = static_cast<int>(child->getAttribute("stop_row"));
          auto col_start = static_cast<int>(child->getAttribute("start_col"));
          auto col_stop = static_cast<int>(child->getAttribute("stop_col"));
          auto *slice = new grm::Slice(row_start, row_stop, col_start, col_stop);

          if (child->localName() == "layout_grid_element")
            {
              auto *cur_grid_element =
                  new grm::GridElement(abs_height, abs_width, abs_height_pxl, abs_width_pxl, fit_parents_height,
                                       fit_parents_width, relative_height, relative_width, aspect_ratio);
              cur_grid_element->elementInDOM = child;
              grid->setElement(slice, cur_grid_element);
            }

          if (child->localName() == "layout_grid")
            {
              int nrows = static_cast<int>(child->getAttribute("num_row"));
              int ncols = static_cast<int>(child->getAttribute("num_col"));

              auto *cur_grid =
                  new grm::Grid(nrows, ncols, abs_height, abs_width, abs_height_pxl, abs_width_pxl, fit_parents_height,
                                fit_parents_width, relative_height, relative_width, aspect_ratio);
              cur_grid->elementInDOM = child;
              grid->setElement(slice, cur_grid);
              initializeGridElements(child, cur_grid);
            }
        }
    }
}

static void finalizeGrid(const std::shared_ptr<GRM::Element> &figure)
{
  grm::Grid *root_grid = nullptr;
  if (figure->hasChildNodes())
    {
      for (const auto &child : figure->children())
        {
          if (child->localName() == "layout_grid")
            {
              int nrows = static_cast<int>(child->getAttribute("num_row"));
              int ncols = static_cast<int>(child->getAttribute("num_col"));
              root_grid = new grm::Grid(nrows, ncols);
              child->setAttribute("plot_x_min", 0);
              child->setAttribute("plot_x_max", 1);
              child->setAttribute("plot_y_min", 0);
              child->setAttribute("plot_y_max", 1);

              initializeGridElements(child, root_grid);
              root_grid->finalizeSubplot();
              break;
            }
        }
    }
}

static void applyCentralRegionDefaults(const std::shared_ptr<GRM::Element> &central_region)
{
  auto plot = central_region->parentElement();
  auto kind = static_cast<std::string>(plot->getAttribute("kind"));
  bool overwrite = plot->hasAttribute("_overwrite_kind_dependent_defaults")
                       ? static_cast<int>(plot->getAttribute("_overwrite_kind_dependent_defaults"))
                       : false;

  if (!central_region->hasAttribute("resample_method"))
    central_region->setAttribute("resample_method", (int)PLOT_DEFAULT_RESAMPLE_METHOD);
  if (!central_region->hasAttribute("keep_window"))
    central_region->setAttribute("keep_window", PLOT_DEFAULT_KEEP_WINDOW);
  if ((!central_region->hasAttribute("space_3d_fov") || overwrite) && kinds_3d.count(kind) != 0)
    {
      if (str_equals_any(kind, "wireframe", "surface", "plot3", "scatter3", "trisurface", "volume"))
        {
          central_region->setAttribute("space_3d_fov", PLOT_DEFAULT_SPACE_3D_FOV);
        }
      else
        {
          central_region->setAttribute("space_3d_fov", 45.0);
        }
    }
  if ((!central_region->hasAttribute("space_3d_camera_distance") || overwrite) && kinds_3d.count(kind) != 0)
    {
      if (str_equals_any(kind, "wireframe", "surface", "plot3", "scatter3", "trisurface", "volume"))
        {
          central_region->setAttribute("space_3d_camera_distance", PLOT_DEFAULT_SPACE_3D_DISTANCE);
        }
      else
        {
          central_region->setAttribute("space_3d_camera_distance", 2.5);
        }
    }
}

static void applyPlotDefaults(const std::shared_ptr<GRM::Element> &plot)
{
  if (!plot->hasAttribute("kind")) plot->setAttribute("kind", PLOT_DEFAULT_KIND);
  if (!plot->hasAttribute("keep_aspect_ratio")) plot->setAttribute("keep_aspect_ratio", PLOT_DEFAULT_KEEP_ASPECT_RATIO);
  if (!plot->hasAttribute("only_quadratic_aspect_ratio"))
    plot->setAttribute("only_quadratic_aspect_ratio", PLOT_DEFAULT_ONLY_QUADRATIC_ASPECT_RATIO);
  if (!plot->hasAttribute("plot_x_min")) plot->setAttribute("plot_x_min", PLOT_DEFAULT_SUBPLOT_MIN_X);
  if (!plot->hasAttribute("plot_x_max")) plot->setAttribute("plot_x_max", PLOT_DEFAULT_SUBPLOT_MAX_X);
  if (!plot->hasAttribute("plot_y_min")) plot->setAttribute("plot_y_min", PLOT_DEFAULT_SUBPLOT_MIN_Y);
  if (!plot->hasAttribute("plot_y_max")) plot->setAttribute("plot_y_max", PLOT_DEFAULT_SUBPLOT_MAX_Y);
  auto kind = static_cast<std::string>(plot->getAttribute("kind"));
  bool overwrite = plot->hasAttribute("_overwrite_kind_dependent_defaults")
                       ? static_cast<int>(plot->getAttribute("_overwrite_kind_dependent_defaults"))
                       : false;
  if (!plot->hasAttribute("adjust_x_lim") || overwrite)
    {
      if (kind == "heatmap" || kind == "marginal_heatmap" || kind == "barplot")
        {
          plot->setAttribute("adjust_x_lim", 0);
        }
      else
        {
          plot->setAttribute("adjust_x_lim", (plot->hasAttribute("x_lim_min") ? 0 : PLOT_DEFAULT_ADJUST_XLIM));
        }
    }
  if (!plot->hasAttribute("adjust_y_lim") || overwrite)
    {
      if (kind == "heatmap" || kind == "marginal_heatmap")
        {
          plot->setAttribute("adjust_y_lim", 0);
        }
      else
        {
          plot->setAttribute("adjust_y_lim", (plot->hasAttribute("y_lim_min") ? 0 : PLOT_DEFAULT_ADJUST_YLIM));
        }
    }
  if (!plot->hasAttribute("adjust_z_lim") || overwrite)
    {
      if (kind != "heatmap" && kind != "marginal_heatmap")
        {
          plot->setAttribute("adjust_z_lim", (plot->hasAttribute("z_lim_min") ? 0 : PLOT_DEFAULT_ADJUST_ZLIM));
        }
    }
  if (!plot->hasAttribute("line_spec")) plot->setAttribute("line_spec", " ");
  if (!plot->hasAttribute("x_log")) plot->setAttribute("x_log", PLOT_DEFAULT_XLOG);
  if (!plot->hasAttribute("y_log")) plot->setAttribute("y_log", PLOT_DEFAULT_YLOG);
  if (!plot->hasAttribute("z_log")) plot->setAttribute("z_log", PLOT_DEFAULT_ZLOG);
  if (!plot->hasAttribute("x_flip")) plot->setAttribute("x_flip", PLOT_DEFAULT_XFLIP);
  if (!plot->hasAttribute("y_flip")) plot->setAttribute("y_flip", PLOT_DEFAULT_YFLIP);
  if (!plot->hasAttribute("z_flip")) plot->setAttribute("z_flip", PLOT_DEFAULT_ZFLIP);
  if (!plot->hasAttribute("font")) plot->setAttribute("font", PLOT_DEFAULT_FONT);
  if (!plot->hasAttribute("font_precision")) plot->setAttribute("font_precision", PLOT_DEFAULT_FONT_PRECISION);
  if (!plot->hasAttribute("colormap")) plot->setAttribute("colormap", PLOT_DEFAULT_COLORMAP);

  auto central_region_parent = plot;
  if (kind == "marginal_heatmap") central_region_parent = plot->children()[0];
  for (const auto &child : central_region_parent->children())
    {
      if (child->localName() == "central_region")
        {
          applyCentralRegionDefaults(child);
          break;
        }
    }
}

static void applyPlotDefaultsHelper(const std::shared_ptr<GRM::Element> &element)
{
  if (element->localName() == "layout_grid_element")
    {
      for (const auto &child : element->children())
        {
          if (child->localName() == "plot")
            {
              applyPlotDefaults(child);
            }
        }
    }
  if (element->localName() == "layout_grid")
    {
      for (const auto &child : element->children())
        {
          applyPlotDefaultsHelper(child);
        }
    }
}

static void applyRootDefaults(const std::shared_ptr<GRM::Element> &root)
{
  if (!root->hasAttribute("clear_ws")) root->setAttribute("clear_ws", PLOT_DEFAULT_CLEAR);
  if (!root->hasAttribute("update_ws")) root->setAttribute("update_ws", PLOT_DEFAULT_UPDATE);
  if (!root->hasAttribute("_modified")) root->setAttribute("_modified", false);

  for (const auto &figure : root->children())
    {
      if (figure->localName() == "figure")
        {
          if (!figure->hasAttribute("size_x"))
            {
              figure->setAttribute("size_x", PLOT_DEFAULT_WIDTH);
              figure->setAttribute("size_x_type", "double");
              figure->setAttribute("size_x_unit", "px");
            }
          if (!figure->hasAttribute("size_y"))
            {
              figure->setAttribute("size_y", PLOT_DEFAULT_HEIGHT);
              figure->setAttribute("size_y_type", "double");
              figure->setAttribute("size_y_unit", "px");
            }

          for (const auto &child : figure->children())
            {
              if (child->localName() == "plot")
                {
                  applyPlotDefaults(child);
                }
              if (child->localName() == "layout_grid")
                {
                  applyPlotDefaultsHelper(child);
                }
            }
        }
    }
}


std::shared_ptr<GRM::Context> GRM::Render::getRenderContext()
{
  return this->context;
}

void GRM::Render::render(const std::shared_ptr<GRM::Document> &document,
                         const std::shared_ptr<GRM::Context> &ext_context)
{
  /*!
   * static GRM::Render::render receiving external document and context
   *
   * \param[in] document A GRM::Document that will be rendered
   * \param[in] ext_context A GRM::Context
   */
  auto root = document->firstChildElement();
  global_root->setAttribute("_modified", false);
  if (root->hasChildNodes())
    {
      for (const auto &child : root->children())
        {
          gr_savestate();
          ::renderHelper(child, ext_context);
          gr_restorestate();
        }
    }
  global_root->setAttribute("_modified", false); // reset the modified flag, cause all updates are made
}

void GRM::Render::render(std::shared_ptr<GRM::Document> const &document)
{
  /*!
   * GRM::Render::render that receives an external document but uses the GRM::Render instance's context.
   *
   * \param[in] document A GRM::Document that will be rendered
   */
  auto root = document->firstChildElement();
  global_root->setAttribute("_modified", false);
  if (root->hasChildNodes())
    {
      for (const auto &child : root->children())
        {
          gr_savestate();
          ::renderHelper(child, this->context);
          gr_restorestate();
        }
    }
  global_root->setAttribute("_modified", false); // reset the modified flag, cause all updates are made
}

void GRM::Render::render(const std::shared_ptr<GRM::Context> &ext_context)
{
  /*!
   *GRM::Render::render uses GRM::Render instance's document and an external context
   *
   * \param[in] ext_context A GRM::Context
   */
  auto root = this->firstChildElement();
  global_root->setAttribute("_modified", false);
  if (root->hasChildNodes())
    {
      for (const auto &child : root->children())
        {
          gr_savestate();
          ::renderHelper(child, ext_context);
          gr_restorestate();
        }
    }
  global_root->setAttribute("_modified", false); // reset the modified flag, cause all updates are made
}

void GRM::Render::render()
{
  /*!
   * GRM::Render::render uses both instance's document and context
   */
  auto root = this->firstChildElement();
  global_root = root;
  if (root->hasChildNodes())
    {
      active_figure = this->firstChildElement()->querySelectorsAll("[active=1]")[0];
      const unsigned int indent = 2;

      redraw_ws = true;
      bounding_id = 0;
      if (!global_render) GRM::Render::createRender();
      applyRootDefaults(root);
      if (logger_enabled())
        {
          std::cerr << toXML(root, GRM::SerializerOptions{std::string(indent, ' '),
                                                          GRM::SerializerOptions::InternalAttributesFormat::Plain})
                    << "\n";
        }
      if (static_cast<int>(root->getAttribute("clear_ws"))) gr_clearws();
      root->setAttribute("_modified", true);
      finalizeGrid(active_figure);
      renderHelper(root, this->context);
      renderZQueue(this->context);
      root->setAttribute("_modified", false); // reset the modified flag, cause all updates are made
      if (getenv("GRDISPLAY") && strcmp(getenv("GRDISPLAY"), "edit") == 0) missingBboxCalculator(root, this->context);
      if (root->hasAttribute("update_ws") && static_cast<int>(root->getAttribute("update_ws"))) gr_updatews();
      // needed when series_line is changed to series_scatter for example
      if (getenv("GRDISPLAY") && strcmp(getenv("GRDISPLAY"), "edit") == 0)
        {
          for (const auto &child : global_render->querySelectorsAll("[_bbox_id=-1]"))
            {
              child->removeAttribute("_bbox_id");
              missingBboxCalculator(child, this->context);
            }
        }
      if (logger_enabled())
        {
          std::cerr << toXML(root, GRM::SerializerOptions{std::string(indent, ' '),
                                                          GRM::SerializerOptions::InternalAttributesFormat::Plain})
                    << "\n";
        }
      redraw_ws = false;
      // reset marker types
      previous_scatter_marker_type = plot_scatter_markertypes;
      previous_line_marker_type = plot_scatter_markertypes;
    }
}

void GRM::Render::process_tree()
{
  global_root->setAttribute("_modified", true);
  renderHelper(global_root, this->context);
  renderZQueue(this->context);
  global_root->setAttribute("_modified", false); // reset the modified flag, cause all updates are made
}

void GRM::Render::finalize()
{
  gr_context_id_manager.destroyGRContexts();
}

std::shared_ptr<GRM::Render> GRM::Render::createRender()
{
  /*!
   * This function can be used to create a Render object
   */
  global_render = std::shared_ptr<Render>(new Render());
  global_render->ownerDocument()->setUpdateFct(&renderCaller, &updateFilter);
  global_render->ownerDocument()->setContextFct(&deleteContextAttribute, &updateContextAttribute);
  return global_render;
}

GRM::Render::Render()
{
  /*!
   * This is the constructor for GRM::Render
   */
  this->context = std::shared_ptr<GRM::Context>(new Context());
}

std::shared_ptr<GRM::Context> GRM::Render::getContext()
{
  return context;
}

/*
 * Searches in elementToTooltip for attributeName and returns a string vector
 * containing:
 * [0] The default value for this attribute
 * [1] The description for this attribute
 */
std::vector<std::string> GRM::Render::getDefaultAndTooltip(const std::shared_ptr<Element> &element,
                                                           const std::string &attribute_name)
{
  static std::unordered_map<std::string, std::vector<std::string>> attributeToTooltip{
      {std::string("absolute_downwards"),
       std::vector<std::string>{"None", "A context reference for the absolute downward errors"}},
      {std::string("absolute_downwards_flt"),
       std::vector<std::string>{"None", "The flat absolute downward error. It gets applied at all error points"}},
      {std::string("absolute_upwards"),
       std::vector<std::string>{"None", "A context reference for the absolute upward errors"}},
      {std::string("absolute_upwards_flt"),
       std::vector<std::string>{"None", "The flat absolute upward error. It gets applied at all error points"}},
      {std::string("absolute_height"), std::vector<std::string>{"None", "Absolut height in percent of window"}},
      {std::string("absolute_height_pxl"), std::vector<std::string>{"None", "Absolut height in pixel"}},
      {std::string("absolute_width"), std::vector<std::string>{"None", "Absolut width in percent of window"}},
      {std::string("absolute_width_pxl"), std::vector<std::string>{"None", "Absolut width in pixel"}},
      {std::string("accelerate"), std::vector<std::string>{"1", "Sets if the GR3 or the GR surface should be used"}},
      {std::string("active"), std::vector<std::string>{"None", "Sets if the element is shown/active"}},
      {std::string("adjust_x_lim"), std::vector<std::string>{"1", "Sets if the x-limits gets adjusted"}},
      {std::string("adjust_y_lim"), std::vector<std::string>{"1", "Sets if the y-limits gets adjusted"}},
      {std::string("adjust_z_lim"), std::vector<std::string>{"1", "Sets if the z-limits gets adjusted"}},
      {std::string("aspect_ratio"), std::vector<std::string>{"None", "Aspect ratio"}},
      {std::string("algorithm"), std::vector<std::string>{"sum", "The used algorithm for the calculation"}},
      {std::string("ambient"), std::vector<std::string>{"0.2", "The ambient light"}},
      {std::string("angle_ticks"),
       std::vector<std::string>{"None", "The interval between minor tick marks on the angle-axis"}},
      {std::string("axis_type"), std::vector<std::string>{"None", "Defines if the axis is getting used for x or y"}},
      {std::string("bar_width"), std::vector<std::string>{"None", "The width of all bars"}},
      {std::string("bin_counts"), std::vector<std::string>{"None", "References the bin-counts stored in the context"}},
      {std::string("bin_edges"), std::vector<std::string>{"None", "References the bin-edges stored in the context"}},
      {std::string("bin_width"), std::vector<std::string>{"None", "The width all bins have"}},
      {std::string("bin_widths"), std::vector<std::string>{"None", "References the bin-widths stored in the context"}},
      {std::string("bins"), std::vector<std::string>{"None", "References the bin-values stored in the context"}},
      {std::string("c"), std::vector<std::string>{"None", "References the color-values stored in the context"}},
      {std::string("c_lim_max"), std::vector<std::string>{"NAN", "The ending color limit"}},
      {std::string("c_lim_min"), std::vector<std::string>{"NAN", "The beginning color limit"}},
      {std::string("c_range_max"), std::vector<std::string>{"None", "The ending color-value"}},
      {std::string("c_range_min"), std::vector<std::string>{"None", "The beginning color-value"}},
      {std::string("cap_y_max"), std::vector<std::string>{"None", "The y-value for the upwards cap"}},
      {std::string("cap_y_min"), std::vector<std::string>{"None", "The y-value for the downwards cap"}},
      {std::string("char_height"), std::vector<std::string>{"None", "The height of the chars"}},
      {std::string("char_up_x"), std::vector<std::string>{"None", "Upside char angle in x-direction of the text"}},
      {std::string("char_up_y"), std::vector<std::string>{"None", "Upside char angle in y-direction of the text"}},
      {std::string("class_nr"), std::vector<std::string>{"None", "Specify the polar bar by a number"}},
      {std::string("classes"),
       std::vector<std::string>{"None", "References the histogram classes stored in the context"}},
      {std::string("clip_transformation"), std::vector<std::string>{"None", "The transformation for clipping"}},
      {std::string("clip_negative"),
       std::vector<std::string>{"0", "Sets if negative radii get clipped, otherwise mirror"}},
      {std::string("color_ind"), std::vector<std::string>{"None", "The index of the used color"}},
      {std::string("color_ind_values"),
       std::vector<std::string>{"None", "References the color-values stored in the context in index format"}},
      {std::string("color_rgb_values"),
       std::vector<std::string>{"None", "References the color-values stored in the context in rgb format"}},
      {std::string("colormap"), std::vector<std::string>{"viridis", "Sets the current colormap"}},
      {std::string("count"), std::vector<std::string>{"None", "The total number of bars"}},
      {std::string("data"), std::vector<std::string>{"None", "Data which gets displayed in the graphic"}},
      {std::string("diag_factor"),
       std::vector<std::string>{
           "None", "Diagonal factor for the char height. Its calculated from the plot viewport with aspect ratio 4/3"}},
      {std::string("diffuse"), std::vector<std::string>{"0.8", "The diffuse light"}},
      {std::string("d_max"), std::vector<std::string>{"None", "The ending dimension for the volume"}},
      {std::string("d_min"), std::vector<std::string>{"None", "The beginning dimension for the volume"}},
      {std::string("disable_x_trans"),
       std::vector<std::string>{"0", "Sets if the parameters for movable transformation in x-direction gets ignored."}},
      {std::string("disable_y_trans"),
       std::vector<std::string>{"0", "Sets if the parameters for movable transformation in y-direction gets ignored."}},
      {std::string("downwards_cap_color"), std::vector<std::string>{"None", "The color value for the downwards caps"}},
      {std::string("hide"), std::vector<std::string>{"1", "Determines if the element will be visible or not"}},
      {std::string("draw_edges"),
       std::vector<std::string>{"0", "Used in combination with x- and y-colormap to set if edges are drawn"}},
      {std::string("draw_grid"), std::vector<std::string>{"None", "Defines if the axis has grid lines or not"}},
      {std::string("e_downwards"), std::vector<std::string>{"None", "The x-value for the downward error"}},
      {std::string("e_upwards"), std::vector<std::string>{"None", "The x-value for the upward error"}},
      {std::string("edge_width"), std::vector<std::string>{"None", "The width of all edges"}},
      {std::string("end_angle"), std::vector<std::string>{"None", "The end angle of the element"}},
      {std::string("error_bar_color"),
       std::vector<std::string>{"None", "The color value for the middle error-bar caps"}},
      {std::string("error_bar_x"), std::vector<std::string>{"None", "The x-value for the error"}},
      {std::string("error_bar_y_max"), std::vector<std::string>{"None", "The ending y-value for the error"}},
      {std::string("error_bar_y_min"), std::vector<std::string>{"None", "The beginning y-value for the error"}},
      {std::string("face_alpha"), std::vector<std::string>{"None", "The alpha-value of the bins"}},
      {std::string("fig_size_x"), std::vector<std::string>{"None", "The figure x-size"}},
      {std::string("fig_size_y"), std::vector<std::string>{"None", "The figure y-size"}},
      {std::string("figure_id"), std::vector<std::string>{"None", "The unique ID of the figure"}},
      {std::string("fill_color_ind"), std::vector<std::string>{"None", "Sets the index of the current fill color"}},
      {std::string("fill_color_rgb"), std::vector<std::string>{"None", "Color for the bars in rgb format"}},
      {std::string("fill_int_style"), std::vector<std::string>{"None", "Sets the index of the current fill style"}},
      {std::string("fill_style"), std::vector<std::string>{"None", "Sets the index of the current fill style"}},
      {std::string("fit_parents_height"),
       std::vector<std::string>{"None", "Toggle if the parent grid should match the element`s height"}},
      {std::string("fit_parents_width"),
       std::vector<std::string>{"None", "Toggle if the parent grid should match the element`s width"}},
      {std::string("font"), std::vector<std::string>{"computermodern", "The used text font"}},
      {std::string("font_precision"), std::vector<std::string>{"precision_outline", "The precision of the text font"}},
      {std::string("grplot"), std::vector<std::string>{"0", "Sets if GRPlot is used or not"}},
      {std::string("height"), std::vector<std::string>{"None", "The height of the element"}},
      {std::string("indices"),
       std::vector<std::string>{"None",
                                "References the bars which gets calculated as inner bars stored in the context"}},
      {std::string("int_lim_high"), std::vector<std::string>{"None", "Sets the upper limit for the integral"}},
      {std::string("int_lim_low"), std::vector<std::string>{"0", "Sets the lower limit for the integral"}},
      {std::string("int_limits_high"),
       std::vector<std::string>{"None", "References the upper integral limit-values stored in the context"}},
      {std::string("int_limits_low"),
       std::vector<std::string>{"None", "References the lower integral limit-values stored in the context"}},
      {std::string("is_major"), std::vector<std::string>{"None", "Defines if the tick is a major tick"}},
      {std::string("is_mirrored"), std::vector<std::string>{"None", "Defines if the tick is mirrored"}},
      {std::string("isovalue"), std::vector<std::string>{"0.5", "The used isovalue"}},
      {std::string("keep_aspect_ratio"), std::vector<std::string>{"1", "Sets if the aspect ratio is kept"}},
      {std::string("keep_radii_axes"),
       std::vector<std::string>{"0", "Sets if the radii_axes start at 0.0 when using y_lims"}},
      {std::string("keep_window"),
       std::vector<std::string>{"1", "Sets if the window will be inflicted by attribute changes"}},
      {std::string("kind"),
       std::vector<std::string>{
           "None", "Defines which kind the displayed series has. Depending on the set kind the kind can be changed."}},
      {std::string("labels"), std::vector<std::string>{"None", "The labels which are displayed in the legend"}},
      {std::string("label_pos"),
       std::vector<std::string>{"None", "The offset from the axis where the label should be placed"}},
      {std::string("levels"), std::vector<std::string>{"20", "Number of contour levels"}},
      {std::string("line_color_ind"), std::vector<std::string>{"1", "The line-color index"}},
      {std::string("line_color_rgb"), std::vector<std::string>{"None", "Color for the edges in rgb format"}},
      {std::string("line_spec"), std::vector<std::string>{"", "Sets the string specifier for line styles"}},
      {std::string("line_type"), std::vector<std::string>{"None", "The type of the line"}},
      {std::string("line_width"), std::vector<std::string>{"None", "The width of the line"}},
      {std::string("location"), std::vector<std::string>{"None", "The elements location"}},
      {std::string("major_count"), std::vector<std::string>{"None", "Defines the how many tick is a major tick"}},
      {std::string("major_h"),
       std::vector<std::string>{"0 or 1000", "Defines if and which contour lines gets their value as a label. A offset "
                                             "of 1000 to this parameter will color the contour lines"}},
      {std::string("marginal_heatmap_kind"), std::vector<std::string>{"all", "The marginal heatmap kind (all, line)"}},
      {std::string("marginal_heatmap_side_plot"),
       std::vector<std::string>{"None",
                                "Used in marginal heatmap children to specify that the viewport and window from "
                                "the marginal heatmap is used to calculate the ones of the current element"}},
      {std::string("marker_color_ind"),
       std::vector<std::string>{"989", "Sets the color of the marker according to the current colormap"}},
      {std::string("marker_size"), std::vector<std::string>{"None", "Sets the size of the displayed markers"}},
      {std::string("marker_sizes"),
       std::vector<std::string>{"None", "References the marker-sizes stored in the context"}},
      {std::string("marker_type"), std::vector<std::string>{"None", "Sets the marker type"}},
      {std::string("max_char_height"), std::vector<std::string>{"0.012", "The maximum height of the chars"}},
      {std::string("max_value"), std::vector<std::string>{"None", "The maximum value of the axis"}},
      {std::string("max_y_length"), std::vector<std::string>{"None", "The maximum y length inside the barplot"}},
      {std::string("min_value"), std::vector<std::string>{"None", "The minimum value of the axis"}},
      {std::string("mirrored_axis"), std::vector<std::string>{"0", "Defines if the axis should be mirrored"}},
      {std::string("model"), std::vector<std::string>{"None", "The used model for the image"}},
      {std::string("movable"),
       std::vector<std::string>{
           "0",
           "Defines if the element can be moved via interaction. This attribute allows to only move certain parts"}},
      {std::string("name"), std::vector<std::string>{"None", "The name of the element"}},
      {std::string("num_bins"), std::vector<std::string>{"None", "Number of bins"}},
      {std::string("num_col"), std::vector<std::string>{"None", "Number of columns"}},
      {std::string("num_row"), std::vector<std::string>{"None", "Number of rows"}},
      {std::string("num_tick_labels"), std::vector<std::string>{"None", "Number of tick labels"}},
      {std::string("num_ticks"), std::vector<std::string>{"None", "Number of ticks"}},
      {std::string("norm"), std::vector<std::string>{"None", "Specify the used normalisation"}},
      {std::string("offset"), std::vector<std::string>{"None", "The offset for the side region viewport"}},
      {std::string("only_quadratic_aspect_ratio"),
       std::vector<std::string>{"0", "Sets if the aspect ratio is forced to be quadratic and kept this way"}},
      {std::string("org"), std::vector<std::string>{"None", "The org of the axis. Needed if org != min_value"}},
      {std::string("orientation"), std::vector<std::string>{"horizontal", "The orientation of the element"}},
      {std::string("phi"), std::vector<std::string>{"None", "References the phi-angles stored in the context"}},
      {std::string("phi_dim"), std::vector<std::string>{"None", "The dimension of the phi-angles"}},
      {std::string("phi_max"), std::vector<std::string>{"None", "The ending phi-angle of the polar cellarray"}},
      {std::string("phi_min"), std::vector<std::string>{"None", "The beginning phi-angle of the polar cellarray"}},
      {std::string("phi_flip"), std::vector<std::string>{"0", "Sets if the phi-angles gets flipped"}},
      {std::string("phi_lim_max"), std::vector<std::string>{"None", "The ending phi-limit"}},
      {std::string("phi_lim_min"), std::vector<std::string>{"None", "The beginning phi-limit"}},
      {std::string("plot_group"),
       std::vector<std::string>{"None", "The plot group. Its used when more than one plot exists in the tree"}},
      {std::string("plot_id"), std::vector<std::string>{"None", "The unique ID of the plot"}},
      {std::string("plot_type"), std::vector<std::string>{"None", "The type of the plot. It can be 2d, 3d or polar"}},
      {std::string("plot_x_max"), std::vector<std::string>{"None", "The ending x-coordinate of the plot"}},
      {std::string("plot_x_min"), std::vector<std::string>{"None", "The beginning x-coordinate of the plot"}},
      {std::string("plot_y_max"), std::vector<std::string>{"None", "The ending y-coordinate of the plot"}},
      {std::string("plot_y_min"), std::vector<std::string>{"None", "The beginning y-coordinate of the plot"}},
      {std::string("pos"),
       std::vector<std::string>{
           "None", "F.e. where the x-axis should be placed in relation to the y-axis (position on the y-axis)"}},
      {std::string("projection_type"), std::vector<std::string>{"None", "The used projection type"}},
      {std::string("px"), std::vector<std::string>{"None", "References the px-values stored in the context. The "
                                                           "px-values are the modified version of the x-values"}},
      {std::string("py"), std::vector<std::string>{"None", "References the py-values stored in the context. The "
                                                           "py-values are the modified version of the y-values"}},
      {std::string("pz"), std::vector<std::string>{"None", "References the pz-values stored in the context. The "
                                                           "pz-values are the modified version of the z-values"}},
      {std::string("r"), std::vector<std::string>{"None", "References the radius-values stored in the context"}},
      {std::string("r_dim"), std::vector<std::string>{"None", "The dimension of the radius-values"}},
      {std::string("r_lim_max"), std::vector<std::string>{"None", "The ending radius limit"}},
      {std::string("r_lim_min"), std::vector<std::string>{"None", "The beginning radius limit"}},
      {std::string("r_max"), std::vector<std::string>{"None", "The ending value for the radius"}},
      {std::string("r_min"), std::vector<std::string>{"None", "The beginning value for the radius"}},
      {std::string("relative_downwards"),
       std::vector<std::string>{"None", "A context reference for the relative downward errors"}},
      {std::string("relative_downwards_flt"),
       std::vector<std::string>{"None", "The flat relative downward error. It gets applied at all error points"}},
      {std::string("relative_height"),
       std::vector<std::string>{"None", "Height in percent relative to the parent`s height"}},
      {std::string("relative_width"),
       std::vector<std::string>{"None", "Width in percent relative to the parent`s width"}},
      {std::string("relative_upwards"),
       std::vector<std::string>{"None", "A context reference for the relative upward errors"}},
      {std::string("relative_upwards_flt"),
       std::vector<std::string>{"None", "The flat relative upward error. It gets applied at all error points"}},
      {std::string("resample_method"), std::vector<std::string>{"None", "The used resample method"}},
      {std::string("rings"), std::vector<std::string>{"None", "The number of rings for polar coordinate systems"}},
      {std::string("scale"), std::vector<std::string>{"None", "The set scale"}},
      {std::string("scientific_format"),
       std::vector<std::string>{"None", "Set the used format which will determine how a specific text will be drawn. "
                                        "The text can be plain or for example interpreted with LaTeX."}},
      {std::string("select_specific_xform"),
       std::vector<std::string>{
           "None", "Selects a predefined transformation from world coordinates to normalized device coordinates"}},
      {std::string("series_index"), std::vector<std::string>{"None", "The index of the inner series"}},
      {std::string("size_x"), std::vector<std::string>{"None", "The figure width"}},
      {std::string("size_x_type"), std::vector<std::string>{"double", "The figure width type (integer, double, ...)"}},
      {std::string("size_x_unit"), std::vector<std::string>{"px", "The figure width unit (px, ...)"}},
      {std::string("size_y"), std::vector<std::string>{"None", "The figure height"}},
      {std::string("size_y_type"), std::vector<std::string>{"double", "The figure height type (integer, double, ...)"}},
      {std::string("size_y_unit"), std::vector<std::string>{"px", "The figure height unit (px, ...)"}},
      {std::string("space"), std::vector<std::string>{"None", "Set if space is used"}},
      {std::string("space_rotation"), std::vector<std::string>{"0", "The rotation for space"}},
      {std::string("space_tilt"), std::vector<std::string>{"90", "The tilt for space"}},
      {std::string("space_z_max"), std::vector<std::string>{"None", "The ending z-coordinate for space"}},
      {std::string("space_z_min"), std::vector<std::string>{"None", "The beginning z-coordinate for space"}},
      {std::string("space_3d_camera_distance"), std::vector<std::string>{"None", "The camera distance for 3d-space"}},
      {std::string("space_3d_fov"), std::vector<std::string>{"None", "The field of view for 3d-space"}},
      {std::string("space_3d_phi"), std::vector<std::string>{"40.0", "The phi-angle for 3d-space"}},
      {std::string("space_3d_theta"), std::vector<std::string>{"60.0", "The theta-angle for 3d-space"}},
      {std::string("specs"), std::vector<std::string>{"None", "The string specifiers for styles"}},
      {std::string("specular"), std::vector<std::string>{"0.7", "The specular light"}},
      {std::string("specular_power"), std::vector<std::string>{"128", "The specular light power"}},
      {std::string("set_text_color_for_background"),
       std::vector<std::string>{"None", "The background color for the text"}},
      {std::string("stairs"),
       std::vector<std::string>{"0", "This is a format of the polar histogram where only outline edges are drawn"}},
      {std::string("start_angle"), std::vector<std::string>{"None", "The start angle of the element"}},
      {std::string("start_col"), std::vector<std::string>{"None", "Start column"}},
      {std::string("start_row"), std::vector<std::string>{"None", "Start row"}},
      {std::string("step_where"), std::vector<std::string>{"None", "Sets where the next stair step should start"}},
      {std::string("stop_col"), std::vector<std::string>{"None", "Stop column"}},
      {std::string("stop_row"), std::vector<std::string>{"None", "Stop row"}},
      {std::string("style"), std::vector<std::string>{"default", "The barplot style (default, lined, stacked)"}},
      {std::string("text"), std::vector<std::string>{"None", "The text displayed by this element"}},
      {std::string("text_align_horizontal"),
       std::vector<std::string>{
           "None", "The horizontal text alignment. Defines where the horizontal anker point of the test is placed."}},
      {std::string("text_align_vertical"),
       std::vector<std::string>{
           "None", "The vertical text alignment. Defines where the vertical anker point of the test is placed."}},
      {std::string("text_color_ind"), std::vector<std::string>{"None", "The index of the text-color"}},
      {std::string("text_encoding"), std::vector<std::string>{"utf8", "The internal text encoding"}},
      {std::string("theta"), std::vector<std::string>{"None", "References the theta-values stored in the context"}},
      {std::string("tick"), std::vector<std::string>{"None", "The polar ticks or the interval between minor ticks"}},
      {std::string("tick_label"), std::vector<std::string>{"", "The label which will be placed next to the tick"}},
      {std::string("tick_orientation"), std::vector<std::string>{"None", "The orientation of the axes ticks"}},
      {std::string("tick_size"), std::vector<std::string>{"0.005", "The size of the ticks"}},
      {std::string("title"), std::vector<std::string>{"None", "The plot title"}},
      {std::string("total"), std::vector<std::string>{"None", "The total-value of the bins"}},
      {std::string("transformation"), std::vector<std::string>{"5", "The used transformation"}},
      {std::string("transparency"), std::vector<std::string>{"None", "Sets the transparency value"}},
      {std::string("u"), std::vector<std::string>{"None", "References the u-values stored in the context"}},
      {std::string("upwards_cap_color"), std::vector<std::string>{"None", "The color value for the upwards caps"}},
      {std::string("v"), std::vector<std::string>{"None", "References the v-values stored in the context"}},
      {std::string("value"), std::vector<std::string>{"None", "The value/number of the tick"}},
      {std::string("viewport_x_max"), std::vector<std::string>{"None", "The ending viewport x-coordinate"}},
      {std::string("viewport_x_min"), std::vector<std::string>{"None", "The beginning viewport x-coordinate"}},
      {std::string("viewport_y_max"), std::vector<std::string>{"None", "The ending viewport y-coordinate"}},
      {std::string("viewport_y_min"), std::vector<std::string>{"None", "The beginning viewport y-coordinate"}},
      {std::string("weights"), std::vector<std::string>{"None", "References the weights stored in the context"}},
      {std::string("width"),
       std::vector<std::string>{"None", "The width of the side region element - inflicting the viewport"}},
      {std::string("window_x_max"), std::vector<std::string>{"None", "The ending window x-coordinate"}},
      {std::string("window_x_min"), std::vector<std::string>{"None", "The beginning window x-coordinate"}},
      {std::string("window_y_max"), std::vector<std::string>{"None", "The ending window y-coordinate"}},
      {std::string("window_y_min"), std::vector<std::string>{"None", "The beginning window y-coordinate"}},
      {std::string("window_z_max"), std::vector<std::string>{"None", "The ending window z-coordinate"}},
      {std::string("window_z_min"), std::vector<std::string>{"None", "The beginning window z-coordinate"}},
      {std::string("ws_viewport_x_max"),
       std::vector<std::string>{"None", "The ending workstation viewport x-coordinate"}},
      {std::string("ws_viewport_x_min"),
       std::vector<std::string>{"None", "The beginning workstation viewport x-coordinate"}},
      {std::string("ws_viewport_y_max"),
       std::vector<std::string>{"None", "The ending workstation viewport y-coordinate"}},
      {std::string("ws_viewport_y_min"),
       std::vector<std::string>{"None", "The beginning workstation viewport y-coordinate"}},
      {std::string("ws_window_x_max"),
       std::vector<std::string>{"None", "The beginning workstation window x-coordinate"}},
      {std::string("ws_window_x_min"), std::vector<std::string>{"None", "The ending workstation window x-coordinate"}},
      {std::string("ws_window_y_max"),
       std::vector<std::string>{"None", "The beginning workstation window y-coordinate"}},
      {std::string("ws_window_y_min"), std::vector<std::string>{"None", "The ending workstation window y-coordinate"}},
      {std::string("x"), std::vector<std::string>{"None", "References the x-values stored in the context"}},
      {std::string("x_bins"), std::vector<std::string>{"1200", "Bins in x-direction"}},
      {std::string("x_colormap"), std::vector<std::string>{"None", "The used colormap in x-direction"}},
      {std::string("x_dim"), std::vector<std::string>{"None", "The dimension of the x-values"}},
      {std::string("x_flip"), std::vector<std::string>{"0", "Set if the x-values gets flipped"}},
      {std::string("x_grid"), std::vector<std::string>{"1", "When set a x-grid is created"}},
      {std::string("x_ind"),
       std::vector<std::string>{"-1", "An index which is used to highlight a specific x-position"}},
      {std::string("x_label"), std::vector<std::string>{"None", "The label of the x-axis"}},
      {std::string("x_label_3d"), std::vector<std::string>{"None", "The label of the x-axis"}},
      {std::string("x_lim_max"), std::vector<std::string>{"None", "The ending x-limit"}},
      {std::string("x_lim_min"), std::vector<std::string>{"None", "The beginning x-limit"}},
      {std::string("x_log"), std::vector<std::string>{"0", "Set if the x-values are logarithmic"}},
      {std::string("x_major"),
       std::vector<std::string>{"5", "Unitless integer values specifying the number of minor tick intervals "
                                     "between major tick marks. Values of 0 or 1 imply no minor ticks. Negative "
                                     "values specify no labels will be drawn for the x-axis"}},
      {std::string("x_max"), std::vector<std::string>{"None", "The ending x-coordinate of the element"}},
      {std::string("x_min"), std::vector<std::string>{"None", "The beginning x-coordinate of the element"}},
      {std::string("x_org"),
       std::vector<std::string>{"0", "The world coordinates of the origin (point of intersection) of the x-axis"}},
      {std::string("x_org_pos"),
       std::vector<std::string>{"low",
                                "The world coordinates position of the origin (point of intersection) of the x-axis"}},
      {std::string("x_range_max"), std::vector<std::string>{"None", "The ending x-value"}},
      {std::string("x_range_min"), std::vector<std::string>{"None", "The beginning x-value"}},
      {std::string("x_scale_ndc"),
       std::vector<std::string>{"1", "The x-direction scale for movable transformation in ndc-space"}},
      {std::string("x_scale_wc"),
       std::vector<std::string>{"1", "The x-direction scale for movable transformation in wc-space"}},
      {std::string("x_shift_ndc"),
       std::vector<std::string>{"0", "The x-direction shift for movable transformation in ndc-space"}},
      {std::string("x_shift_wc"),
       std::vector<std::string>{"0", "The x-direction shift for movable transformation in wc-space"}},
      {std::string("x_tick"), std::vector<std::string>{"1", "The interval between minor tick marks on the x-axis"}},
      {std::string("xi"), std::vector<std::string>{"None", "References the xi-values stored in the context"}},
      {std::string("x1"), std::vector<std::string>{"None", "The beginning x-coordinate"}},
      {std::string("x2"), std::vector<std::string>{"None", "The ending x-coordinate"}},
      {std::string("y"), std::vector<std::string>{"None", "References the y-values stored in the context"}},
      {std::string("y_bins"), std::vector<std::string>{"1200", "Bins in y-direction"}},
      {std::string("y_colormap"), std::vector<std::string>{"None", "The used colormap in y-direction"}},
      {std::string("y_dim"), std::vector<std::string>{"None", "The dimension of the y-values"}},
      {std::string("y_flip"), std::vector<std::string>{"0", "Set if the y-values gets flipped"}},
      {std::string("y_grid"), std::vector<std::string>{"1", "When set a y-grid is created"}},
      {std::string("y_ind"),
       std::vector<std::string>{"-1", "An index which is used to highlight a specific y-position"}},
      {std::string("y_label"), std::vector<std::string>{"None", "The label of the y-axis"}},
      {std::string("y_label_3d"), std::vector<std::string>{"None", "The label of the y-axis"}},
      {std::string("y_labels"), std::vector<std::string>{"None", "References the y-labels stored in the context"}},
      {std::string("y_lim_max"), std::vector<std::string>{"None", "The ending y-limit"}},
      {std::string("y_lim_min"), std::vector<std::string>{"None", "The beginning y-limit"}},
      {std::string("y_line"), std::vector<std::string>{"None", "Sets if there is a y-line"}},
      {std::string("y_log"), std::vector<std::string>{"0", "Set if the y-values are logarithmic"}},
      {std::string("y_major"),
       std::vector<std::string>{"5", "Unitless integer values specifying the number of minor tick intervals "
                                     "between major tick marks. Values of 0 or 1 imply no minor ticks. Negative "
                                     "values specify no labels will be drawn for the y-axis"}},
      {std::string("y_max"), std::vector<std::string>{"None", "The ending y-coordinate of the element"}},
      {std::string("y_min"), std::vector<std::string>{"None", "The beginning y-coordinate of the element"}},
      {std::string("y_org"),
       std::vector<std::string>{"0", "The world coordinates of the origin (point of intersection) of the y-axis"}},
      {std::string("y_org_pos"),
       std::vector<std::string>{"low",
                                "The world coordinates position of the origin (point of intersection) of the y-axis"}},
      {std::string("y_range_max"), std::vector<std::string>{"None", "The ending y-value"}},
      {std::string("y_range_min"), std::vector<std::string>{"None", "The beginning y-value"}},
      {std::string("y_scale_ndc"),
       std::vector<std::string>{"1", "The y-direction scale for movable transformation in ndc-space"}},
      {std::string("y_scale_wc"),
       std::vector<std::string>{"1", "The y-direction scale for movable transformation in wc-space"}},
      {std::string("y_shift_ndc"),
       std::vector<std::string>{"0", "The y-direction shift for movable transformation in ndc-space"}},
      {std::string("y_shift_wc"),
       std::vector<std::string>{"0", "The y-direction shift for movable transformation in wc-space"}},
      {std::string("y_tick"), std::vector<std::string>{"1", "The interval between minor tick marks on the y-axis"}},
      {std::string("y1"), std::vector<std::string>{"None", "The beginning y-coordinate"}},
      {std::string("y2"), std::vector<std::string>{"None", "The ending y-coordinate"}},
      {std::string("z"), std::vector<std::string>{"None", "References the z-values stored in the context"}},
      {std::string("z_dims"), std::vector<std::string>{"None", "References the z-dimensions stored in the context"}},
      {std::string("z_flip"), std::vector<std::string>{"0", "Set if the z-values gets flipped"}},
      {std::string("z_grid"), std::vector<std::string>{"1", "When set a z-grid is created"}},
      {std::string("z_label_3d"), std::vector<std::string>{"None", "The label of the z-axis"}},
      {std::string("z_lim_max"), std::vector<std::string>{"NAN", "The ending z-limit"}},
      {std::string("z_lim_min"), std::vector<std::string>{"NAN", "The beginning z-limit"}},
      {std::string("z_log"), std::vector<std::string>{"0", "Set if the z-values are logarithmic"}},
      {std::string("z_index"), std::vector<std::string>{"0", "Sets the render order compared to the other elements"}},
      {std::string("z_major"),
       std::vector<std::string>{"5", "Unitless integer values specifying the number of minor tick intervals "
                                     "between major tick marks. Values of 0 or 1 imply no minor ticks. Negative "
                                     "values specify no labels will be drawn for the z-axis"}},
      {std::string("z_max"),
       std::vector<std::string>{
           "None",
           "The maximum z-coordinate of a contour(f) plot (after transforming the input data to a rectangular grid)"}},
      {std::string("z_min"),
       std::vector<std::string>{
           "None",
           "The minimum z-coordinate of a contour(f) plot (after transforming the input data to a rectangular grid)"}},
      {std::string("z_org"),
       std::vector<std::string>{"0", "The world coordinates of the origin (point of intersection) of the z-axis"}},
      {std::string("z_org_pos"),
       std::vector<std::string>{"low",
                                "The world coordinates position of the origin (point of intersection) of the z-axis"}},
      {std::string("z_range_max"), std::vector<std::string>{"None", "The ending z-value"}},
      {std::string("z_range_min"), std::vector<std::string>{"None", "The beginning z-value"}},
      {std::string("z_tick"), std::vector<std::string>{"1", "The interval between minor tick marks on the z-axis"}},
  };
  if (attributeToTooltip.count(attribute_name))
    {
      if (element->localName() == "text")
        {
          if (attribute_name == "width") return std::vector<std::string>{"None", "The width of the text"};
          if (attribute_name == "x") return std::vector<std::string>{"None", "x-position of the text"};
          if (attribute_name == "y") return std::vector<std::string>{"None", "y-position of the text"};
        }
      return attributeToTooltip[attribute_name];
    }
  else
    {
      return std::vector<std::string>{"", "No description found"};
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ create functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

std::shared_ptr<GRM::Element> GRM::Render::createPlot(int plotId, const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> plot = (ext_element == nullptr) ? createElement("plot") : ext_element;
  plot->setAttribute("plot_id", "plot" + std::to_string(plotId));
  plot->setAttribute("plot_group", true);
  return plot;
}

std::shared_ptr<GRM::Element> GRM::Render::createCentralRegion(const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> central_region =
      (ext_element == nullptr) ? createElement("central_region") : ext_element;
  return central_region;
}

std::shared_ptr<GRM::Element>
GRM::Render::createPolymarker(const std::string &x_key, std::optional<std::vector<double>> x, const std::string &y_key,
                              std::optional<std::vector<double>> y, const std::shared_ptr<GRM::Context> &ext_context,
                              int marker_type, double marker_size, int marker_colorind,
                              const std::shared_ptr<GRM::Element> &ext_element)
{
  /*!
   * This function can be used to create a Polymarker GRM::Element
   *
   * \param[in] x_key A string used for storing the x coordinates in GRM::Context
   * \param[in] x A vector containing double values representing x coordinates
   * \param[in] y_key A string used for storing the y coordinates in GRM::Context
   * \param[in] y A vector containing double values representing y coordinates
   * \param[in] ext_context A GRM::Context that is used for storing the vectors. By default it uses GRM::Render's
   * GRM::Context object but an external GRM::Context can be used \param[in] marker_type An Integer setting the
   * gr_markertype. By default it is 0 \param[in] marker_size A Double value setting the gr_markersize. By default it
   * is 0.0 \param[in] marker_colorind An Integer setting the gr_markercolorind. By default it is 0
   */

  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("polymarker") : ext_element;
  if (x != std::nullopt)
    {
      (*use_context)[x_key] = x.value();
    }
  element->setAttribute("x", x_key);

  if (y != std::nullopt)
    {
      (*use_context)[y_key] = y.value();
    }
  element->setAttribute("y", y_key);

  if (marker_type != 0)
    {
      element->setAttribute("marker_type", marker_type);
    }
  if (marker_size != 0.0)
    {
      element->setAttribute("marker_size", marker_size);
    }
  if (marker_colorind != 0)
    {
      element->setAttribute("marker_color_ind", marker_colorind);
    }
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createPolymarker(double x, double y, int marker_type, double marker_size,
                                                            int marker_colorind,
                                                            const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("polymarker") : ext_element;
  element->setAttribute("x", x);
  element->setAttribute("y", y);
  if (marker_type != 0)
    {
      element->setAttribute("marker_type", marker_type);
    }
  if (marker_size != 0.0)
    {
      element->setAttribute("marker_size", marker_size);
    }
  if (marker_colorind != 0)
    {
      element->setAttribute("marker_color_ind", marker_colorind);
    }
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createPolyline(double x1, double x2, double y1, double y2, int line_type,
                                                          double line_width, int line_colorind,
                                                          const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("polyline") : ext_element;
  element->setAttribute("x1", x1);
  element->setAttribute("x2", x2);
  element->setAttribute("y1", y1);
  element->setAttribute("y2", y2);
  if (line_type != 0)
    {
      element->setAttribute("line_type", line_type);
    }
  if (line_width != 0.0)
    {
      element->setAttribute("line_width", line_width);
    }
  if (line_colorind != 0)
    {
      element->setAttribute("line_color_ind", line_colorind);
    }
  return element;
}

std::shared_ptr<GRM::Element>
GRM::Render::createPolyline(const std::string &x_key, std::optional<std::vector<double>> x, const std::string &y_key,
                            std::optional<std::vector<double>> y, const std::shared_ptr<GRM::Context> &ext_context,
                            int line_type, double line_width, int line_colorind,
                            const std::shared_ptr<GRM::Element> &ext_element)
{
  /*!
   * This function can be used to create a Polyline GRM::Element
   *
   * \param[in] x_key A string used for storing the x coordinates in GRM::Context
   * \param[in] x A vector containing double values representing x coordinates
   * \param[in] y_key A string used for storing the y coordinates in GRM::Context
   * \param[in] y A vector containing double values representing y coordinates
   * \param[in] ext_context A GRM::Context that is used for storing the vectors. By default it uses GRM::Render's
   * GRM::Context object but an external GRM::Context can be used \param[in] line_type An Integer setting the
   * gr_linetype. By default it is 0 \param[in] line_width A Double value setting the gr_linewidth. By default it is
   * 0.0 \param[in] marker_colorind An Integer setting the gr_linecolorind. By default it is 0
   */

  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("polyline") : ext_element;
  if (x != std::nullopt)
    {
      (*use_context)[x_key] = *x;
    }
  element->setAttribute("x", x_key);
  if (y != std::nullopt)
    {
      (*use_context)[y_key] = *y;
    }
  element->setAttribute("y", y_key);
  if (line_type != 0)
    {
      element->setAttribute("line_type", line_type);
    }
  if (line_width != 0.0)
    {
      element->setAttribute("line_width", line_width);
    }
  if (line_colorind != 0)
    {
      element->setAttribute("line_color_ind", line_colorind);
    }
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createText(double x, double y, const std::string &text,
                                                      CoordinateSpace space,
                                                      const std::shared_ptr<GRM::Element> &ext_element)
{
  /*!
   * This function can be used to create a Text GRM::Element
   *
   * \param[in] x A double value representing the x coordinate
   * \param[in] y A double value representing the y coordinate
   * \param[in] text A string
   * \param[in] space the coordinate space (WC or NDC) for x and y, default NDC
   */

  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("text") : ext_element;
  element->setAttribute("x", x);
  element->setAttribute("y", y);
  element->setAttribute("text", text);
  element->setAttribute("space", static_cast<int>(space));
  return element;
}

std::shared_ptr<GRM::Element>
GRM::Render::createFillArea(const std::string &x_key, std::optional<std::vector<double>> x, const std::string &y_key,
                            std::optional<std::vector<double>> y, const std::shared_ptr<GRM::Context> &ext_context,
                            int fillintstyle, int fillstyle, int fillcolorind,
                            const std::shared_ptr<GRM::Element> &ext_element)
{
  /*!
   * This function can be used to create a FillArea GRM::Element
   *
   * \param[in] n The number of data points
   * \param[in] x_key A string used for storing the x coordinates in GRM::Context
   * \param[in] x A vector containing double values representing x coordinates
   * \param[in] y_key A string used for storing the y coordinates in GRM::Context
   * \param[in] y A vector containing double values representing y coordinates
   * \param[in] ext_context A GRM::Context that is used for storing the vectors. By default it uses GRM::Render's
   * GRM::Context object but an external GRM::Context can be used \param[in] fillintstyle An Integer setting the
   * gr_fillintstyle. By default it is 0 \param[in] fillstyle An Integer setting the gr_fillstyle. By default it is 0
   */

  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("fill_area") : ext_element;
  if (x != std::nullopt)
    {
      (*use_context)[x_key] = *x;
    }
  element->setAttribute("x", x_key);
  if (y != std::nullopt)
    {
      (*use_context)[y_key] = *y;
    }
  element->setAttribute("y", y_key);

  if (fillintstyle != 0)
    {
      element->setAttribute("fill_int_style", fillintstyle);
    }
  if (fillstyle != 0)
    {
      element->setAttribute("fill_style", fillstyle);
    }
  if (fillcolorind != -1)
    {
      element->setAttribute("fill_color_ind", fillcolorind);
    }
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createCellArray(double xmin, double xmax, double ymin, double ymax, int dimx,
                                                           int dimy, int scol, int srow, int ncol, int nrow,
                                                           const std::string &color_key,
                                                           std::optional<std::vector<int>> color,
                                                           const std::shared_ptr<GRM::Context> &ext_context,
                                                           const std::shared_ptr<GRM::Element> &ext_element)
{
  /*!
   * This function can be used to create a CellArray GRM::Element
   *
   * \param[in] xmin A double value
   * \param[in] xmax A double value
   * \param[in] ymin A double value
   * \param[in] ymax A double value
   * \param[in] dimx An Integer value
   * \param[in] dimy An Integer value
   * \param[in] scol An Integer value
   * \param[in] srow An Integer value
   * \param[in] ncol An Integer value
   * \param[in] nrow An Integer value
   * \param[in] color_key A string used as a key for storing color
   * \param[in] color A vector with Integers
   * \param[in] ext_context A GRM::Context used for storing color. By default it uses GRM::Render's GRM::Context object
   * but an external GRM::Context can be used
   */

  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("cellarray") : ext_element;
  element->setAttribute("x_min", xmin);
  element->setAttribute("x_max", xmax);
  element->setAttribute("y_min", ymin);
  element->setAttribute("y_max", ymax);
  element->setAttribute("x_dim", dimx);
  element->setAttribute("y_dim", dimy);
  element->setAttribute("start_col", scol);
  element->setAttribute("start_row", srow);
  element->setAttribute("num_col", ncol);
  element->setAttribute("num_row", nrow);
  element->setAttribute("color_ind_values", color_key);
  if (color != std::nullopt)
    {
      (*use_context)[color_key] = *color;
    }
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createEmptyAxis(const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("axis") : ext_element;
  if (!element->hasAttribute("_axis_id")) element->setAttribute("_axis_id", axis_id++);
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createAxis(double min_val, double max_val, double tick, double org,
                                                      double pos, int major_count, int num_ticks, int num_tick_labels,
                                                      double tick_size, int tick_orientation, double label_pos,
                                                      const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("axis") : ext_element;
  element->setAttribute("min_value", min_val);
  element->setAttribute("max_value", max_val);
  element->setAttribute("tick", tick);
  element->setAttribute("org", org);
  element->setAttribute("pos", pos);
  element->setAttribute("major_count", major_count);
  element->setAttribute("num_ticks", num_ticks);
  element->setAttribute("num_tick_labels", num_tick_labels);
  element->setAttribute("tick_size", tick_size);
  element->setAttribute("tick_orientation", tick_orientation);
  element->setAttribute("label_pos", label_pos);
  if (!element->hasAttribute("_axis_id")) element->setAttribute("_axis_id", axis_id++);
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createTickGroup(int is_major, std::string tick_label, double value,
                                                           double width,
                                                           const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("tick_group") : ext_element;
  element->setAttribute("is_major", is_major);
  element->setAttribute("tick_label", tick_label);
  element->setAttribute("value", value);
  element->setAttribute("width", width);
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createTick(int is_major, double value,
                                                      const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("tick") : ext_element;
  element->setAttribute("is_major", is_major);
  element->setAttribute("value", value);
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createGridLine(int is_major, double value,
                                                          const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("grid_line") : ext_element;
  element->setAttribute("is_major", is_major);
  element->setAttribute("value", value);
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createLegend(const std::string &labels_key,
                                                        std::optional<std::vector<std::string>> labels,
                                                        const std::string &specs_key,
                                                        std::optional<std::vector<std::string>> specs,
                                                        const std::shared_ptr<GRM::Context> &ext_context,
                                                        const std::shared_ptr<GRM::Element> &ext_element)
{
  /*!
   * This function can be used for creating a legend GRM::Element
   * This element is different compared to most of Render's GRM::Element, the legend GRM::Element will incorporate
   * plot_draw_legend code from plot.cxx and will create new GRM::Elements as child nodes in the render document
   *
   * \param[in] labels_key A std::string for the labels vector
   * \param[in] labels May be an std::vector<std::string>> containing the labels
   * \param[in] spec A std::string
   */

  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("legend") : ext_element;
  element->setAttribute("z_index", 4);
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  element->setAttribute("specs", specs_key);
  element->setAttribute("labels", labels_key);

  if (labels != std::nullopt)
    {
      (*use_context)[labels_key] = *labels;
    }
  if (specs != std::nullopt)
    {
      (*use_context)[specs_key] = *specs;
    }

  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createDrawPolarAxes(int angle_ticks, const std::string &kind, int phiflip,
                                                               const std::string &norm, double tick, double line_width,
                                                               const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("polar_axes") : ext_element;
  if (!norm.empty())
    {
      element->setAttribute("norm", norm);
    }
  if (tick != 0.0)
    {
      element->setAttribute("tick", tick);
    }
  if (line_width != 0.0)
    {
      element->setAttribute("line_width", line_width);
    }
  element->setAttribute("angle_ticks", angle_ticks);
  // todo should phiflip be passed when creating a polarAxesElement
  //  element->setAttribute("phi_flip", phiflip);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createPieLegend(const std::string &labels_key,
                                                           std::optional<std::vector<std::string>> labels,
                                                           const std::shared_ptr<GRM::Context> &ext_context,
                                                           const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("legend") : ext_element;
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  element->setAttribute("labels", labels_key);

  if (labels != std::nullopt)
    {
      (*use_context)[labels_key] = *labels;
    }
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createPieSegment(const double start_angle, const double end_angle,
                                                            const std::string &text, const int color_index,
                                                            const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("pie_segment") : ext_element;
  element->setAttribute("start_angle", start_angle);
  element->setAttribute("end_angle", end_angle);
  element->setAttribute("text", text);
  element->setAttribute("color_ind", color_index);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createBar(const double x1, const double x2, const double y1, const double y2,
                                                     const int bar_color_index, const int edge_color_index,
                                                     const std::string &bar_color_rgb,
                                                     const std::string &edge_color_rgb, const double linewidth,
                                                     const std::string &text,
                                                     const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("bar") : ext_element;
  element->setAttribute("x1", x1);
  element->setAttribute("x2", x2);
  element->setAttribute("y1", y1);
  element->setAttribute("y2", y2);
  element->setAttribute("line_color_ind", edge_color_index);
  element->setAttribute("fill_color_ind", bar_color_index);
  if (!bar_color_rgb.empty()) element->setAttribute("fill_color_rgb", bar_color_rgb);
  if (!edge_color_rgb.empty()) element->setAttribute("line_color_rgb", edge_color_rgb);
  if (linewidth != -1) element->setAttribute("line_width", linewidth);
  if (!text.empty()) element->setAttribute("text", text);

  return element;
}


std::shared_ptr<GRM::Element> GRM::Render::createSeries(const std::string &name)
{
  auto element = createElement("series_" + name);
  element->setAttribute("kind", name);
  element->setAttribute("_update_required", false);
  element->setAttribute("_delete_children", 0);
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createDrawImage(double xmin, double ymin, double xmax, double ymax,
                                                           int width, int height, const std::string &data_key,
                                                           std::optional<std::vector<int>> data, int model,
                                                           const std::shared_ptr<GRM::Context> &ext_context,
                                                           const std::shared_ptr<GRM::Element> &ext_element)
{
  /*!
   * This function can be used to create a DrawImage GRM::Element
   *
   * \param[in] xmin A Double value
   * \param[in] xmax A Double value
   * \param[in] ymin A Double value
   * \param[in] ymax A Double value
   * \param[in] width An Integer value
   * \param[in] height An Integer value
   * \param[in] data_key A String used as a key for storing data
   * \param[in] data A vector containing Integers
   * \param[in] model An Integer setting the model
   * \param[in] ext_context A GRM::Context used for storing data. By default it uses GRM::Render's GRM::Context object
   * but an external GRM::Context can be used
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("draw_image") : ext_element;
  element->setAttribute("x_min", xmin);
  element->setAttribute("x_max", xmax);
  element->setAttribute("y_min", ymin);
  element->setAttribute("y_max", ymax);
  element->setAttribute("width", width);
  element->setAttribute("height", height);
  element->setAttribute("model", model);
  element->setAttribute("data", data_key);
  if (data != std::nullopt)
    {
      (*use_context)[data_key] = *data;
    }
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createDrawArc(double xmin, double xmax, double ymin, double ymax,
                                                         double start_angle, double end_angle,
                                                         const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("draw_arc") : ext_element;
  element->setAttribute("x_min", xmin);
  element->setAttribute("x_max", xmax);
  element->setAttribute("y_min", ymin);
  element->setAttribute("y_max", ymax);
  element->setAttribute("start_angle", start_angle);
  element->setAttribute("end_angle", end_angle);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createFillArc(double xmin, double xmax, double ymin, double ymax, double a1,
                                                         double a2, int fillintstyle, int fillstyle, int fillcolorind,
                                                         const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("fill_arc") : ext_element;
  element->setAttribute("x_min", xmin);
  element->setAttribute("x_max", xmax);
  element->setAttribute("y_min", ymin);
  element->setAttribute("y_max", ymax);
  element->setAttribute("start_angle", a1);
  element->setAttribute("end_angle", a2);

  if (fillintstyle != 0)
    {
      element->setAttribute("fill_int_style", fillintstyle);
    }
  if (fillstyle != 0)
    {
      element->setAttribute("fill_style", fillstyle);
    }
  if (fillcolorind != -1)
    {
      element->setAttribute("fill_color_ind", fillcolorind);
    }
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createDrawRect(double xmin, double xmax, double ymin, double ymax,
                                                          const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("draw_rect") : ext_element;
  element->setAttribute("x_min", xmin);
  element->setAttribute("x_max", xmax);
  element->setAttribute("y_min", ymin);
  element->setAttribute("y_max", ymax);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createFillRect(double xmin, double xmax, double ymin, double ymax,
                                                          int fillintstyle, int fillstyle, int fillcolorind,
                                                          const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("fill_rect") : ext_element;
  element->setAttribute("x_min", xmin);
  element->setAttribute("x_max", xmax);
  element->setAttribute("y_min", ymin);
  element->setAttribute("y_max", ymax);

  if (fillintstyle != 0)
    {
      element->setAttribute("fill_int_style", fillintstyle);
    }
  if (fillstyle != 0)
    {
      element->setAttribute("fill_style", fillstyle);
    }
  if (fillcolorind != -1)
    {
      element->setAttribute("fill_color_ind", fillcolorind);
    }

  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createQuiver(const std::string &x_key, std::optional<std::vector<double>> x,
                                                        const std::string &y_key, std::optional<std::vector<double>> y,
                                                        const std::string &u_key, std::optional<std::vector<double>> u,
                                                        const std::string &v_key, std::optional<std::vector<double>> v,
                                                        int color, const std::shared_ptr<GRM::Context> &ext_context)
{
  /*
   * This function can be used to create a Quiver GRM::Element
   *
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  auto element = createSeries("quiver");
  element->setAttribute("x", x_key);
  element->setAttribute("y", y_key);
  element->setAttribute("u", u_key);
  element->setAttribute("v", v_key);
  element->setAttribute("color_ind", color);

  if (x != std::nullopt)
    {
      (*use_context)[x_key] = *x;
    }
  if (y != std::nullopt)
    {
      (*use_context)[y_key] = *y;
    }
  if (u != std::nullopt)
    {
      (*use_context)[u_key] = *u;
    }
  if (v != std::nullopt)
    {
      (*use_context)[v_key] = *v;
    }

  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createHexbin(const std::string &x_key, std::optional<std::vector<double>> x,
                                                        const std::string &y_key, std::optional<std::vector<double>> y,
                                                        const std::shared_ptr<GRM::Context> &ext_context)
{
  /*!
   * This function can be used to create a hexbin GRM::Element
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  auto element = createSeries("hexbin");
  element->setAttribute("x", x_key);
  element->setAttribute("y", y_key);

  if (x != std::nullopt)
    {
      (*use_context)[x_key] = *x;
    }
  if (y != std::nullopt)
    {
      (*use_context)[y_key] = *y;
    }

  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createColorbar(unsigned int colors,
                                                          const std::shared_ptr<GRM::Context> &ext_context,
                                                          const std::shared_ptr<GRM::Element> &ext_element)
{
  /*!
   * This function can be used to create a colorbar GRM::Element
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("colorbar") : ext_element;
  element->setAttribute("color_ind", static_cast<int>(colors));
  element->setAttribute("_update_required", false);
  element->setAttribute("_delete_children", 0);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createPolarCellArray(
    double x_org, double y_org, double phimin, double phimax, double rmin, double rmax, int dimphi, int dimr, int scol,
    int srow, int ncol, int nrow, const std::string &color_key, std::optional<std::vector<int>> color,
    const std::shared_ptr<Context> &ext_context, const std::shared_ptr<GRM::Element> &ext_element)
{
  /*!
   * Display a two dimensional color index array mapped to a disk using polar
   * coordinates.
   *
   * \param[in] x_org X coordinate of the disk center in world coordinates
   * \param[in] y_org Y coordinate of the disk center in world coordinates
   * \param[in] phimin start angle of the disk sector in degrees
   * \param[in] phimax end angle of the disk sector in degrees
   * \param[in] rmin inner radius of the punctured disk in world coordinates
   * \param[in] rmax outer radius of the disk in world coordinates
   * \param[in] dimphi Phi (X) dimension of the color index array
   * \param[in] dimr R (Y) dimension of the color index array
   * \param[in] scol number of leading columns in the color index array
   * \param[in] srow number of leading rows in the color index array
   * \param[in] ncol total number of columns in the color index array
   * \param[in] nrow total number of rows in the color index array
   * \param[in] color color index array
   *
   * The two dimensional color index array is mapped to the resulting image by
   * interpreting the X-axis of the array as the angle and the Y-axis as the radius.
   * The center point of the resulting disk is located at `x_org`, `y_org` and the
   * radius of the disk is `rmax`.
   *
   * To draw a contiguous array as a complete disk use:
   *
   *     gr_polarcellarray(x_org, y_org, 0, 360, 0, rmax, dimphi, dimr, 1, 1, dimphi, dimr, color)
   *
   * The additional parameters to the function can be used to further control the
   * mapping from polar to cartesian coordinates.
   *
   * If `rmin` is greater than 0 the input data is mapped to a punctured disk (or
   * annulus) with an inner radius of `rmin` and an outer radius `rmax`. If `rmin`
   * is greater than `rmax` the Y-axis of the array is reversed.
   *
   * The parameter `phimin` and `phimax` can be used to map the data to a sector
   * of the (punctured) disk starting at `phimin` and ending at `phimax`. If
   * `phimin` is greater than `phimax` the X-axis is reversed. The visible sector
   * is the one starting in mathematically positive direction (counterclockwise)
   * at the smaller angle and ending at the larger angle. An example of the four
   * possible options can be found below:
   *
   * \verbatim embed:rst:leading-asterisk
   *
   * +-----------+-----------+---------------------------------------------------+
   * |**phimin** |**phimax** |**Result**                                         |
   * +-----------+-----------+---------------------------------------------------+
   * |90         |270        |Left half visible, mapped counterclockwise         |
   * +-----------+-----------+---------------------------------------------------+
   * |270        |90         |Left half visible, mapped clockwise                |
   * +-----------+-----------+---------------------------------------------------+
   * |-90        |90         |Right half visible, mapped counterclockwise        |
   * +-----------+-----------+---------------------------------------------------+
   * |90         |-90        |Right half visible, mapped clockwise               |
   * +-----------+-----------+---------------------------------------------------+
   *
   * \endverbatim
   *
   * `scol` and `srow` can be used to specify a (1-based) starting column and row
   * in the `color` array. `ncol` and `nrow` specify the actual dimension of the
   * array in the memory whereof `dimphi` and `dimr` values are mapped to the disk.
   *
   */

  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("polar_cellarray") : ext_element;
  element->setAttribute("x_org", x_org);
  element->setAttribute("y_org", y_org);
  element->setAttribute("phi_min", phimin);
  element->setAttribute("phi_max", phimax);
  element->setAttribute("r_min", rmin);
  element->setAttribute("r_max", rmax);
  element->setAttribute("phi_dim", dimphi);
  element->setAttribute("r_dim", dimr);
  element->setAttribute("start_col", scol);
  element->setAttribute("start_row", srow);
  element->setAttribute("num_col", ncol);
  element->setAttribute("num_row", nrow);
  element->setAttribute("color_ind_values", color_key);
  if (color != std::nullopt)
    {
      (*use_context)[color_key] = *color;
    }
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createNonUniformPolarCellArray(
    double x_org, double y_org, const std::string &phi_key, std::optional<std::vector<double>> phi,
    const std::string &r_key, std::optional<std::vector<double>> r, int dimphi, int dimr, int scol, int srow, int ncol,
    int nrow, const std::string &color_key, std::optional<std::vector<int>> color,
    const std::shared_ptr<GRM::Context> &ext_context, const std::shared_ptr<GRM::Element> &ext_element)
{
  /*!
   * Display a two dimensional color index array mapped to a disk using polar
   * coordinates.
   *
   * \param[in] x_org X coordinate of the disk center in world coordinates
   * \param[in] y_org Y coordinate of the disk center in world coordinates
   * \param[in] phimin start angle of the disk sector in degrees
   * \param[in] phimax end angle of the disk sector in degrees
   * \param[in] rmin inner radius of the punctured disk in world coordinates
   * \param[in] rmax outer radius of the disk in world coordinates
   * \param[in] dimphi Phi (X) dimension of the color index array
   * \param[in] dimr R (Y) dimension of the color index array
   * \param[in] scol number of leading columns in the color index array
   * \param[in] srow number of leading rows in the color index array
   * \param[in] ncol total number of columns in the color index array
   * \param[in] nrow total number of rows in the color index array
   * \param[in] color color index array
   *
   * The two dimensional color index array is mapped to the resulting image by
   * interpreting the X-axis of the array as the angle and the Y-axis as the radius.
   * The center point of the resulting disk is located at `x_org`, `y_org` and the
   * radius of the disk is `rmax`.
   *
   * To draw a contiguous array as a complete disk use:
   *
   *     gr_polarcellarray(x_org, y_org, 0, 360, 0, rmax, dimphi, dimr, 1, 1, dimphi, dimr, color)
   *
   * The additional parameters to the function can be used to further control the
   * mapping from polar to cartesian coordinates.
   *
   * If `rmin` is greater than 0 the input data is mapped to a punctured disk (or
   * annulus) with an inner radius of `rmin` and an outer radius `rmax`. If `rmin`
   * is greater than `rmax` the Y-axis of the array is reversed.
   *
   * The parameter `phimin` and `phimax` can be used to map the data to a sector
   * of the (punctured) disk starting at `phimin` and ending at `phimax`. If
   * `phimin` is greater than `phimax` the X-axis is reversed. The visible sector
   * is the one starting in mathematically positive direction (counterclockwise)
   * at the smaller angle and ending at the larger angle. An example of the four
   * possible options can be found below:
   *
   * \verbatim embed:rst:leading-asterisk
   *
   * +-----------+-----------+---------------------------------------------------+
   * |**phimin** |**phimax** |**Result**                                         |
   * +-----------+-----------+---------------------------------------------------+
   * |90         |270        |Left half visible, mapped counterclockwise         |
   * +-----------+-----------+---------------------------------------------------+
   * |270        |90         |Left half visible, mapped clockwise                |
   * +-----------+-----------+---------------------------------------------------+
   * |-90        |90         |Right half visible, mapped counterclockwise        |
   * +-----------+-----------+---------------------------------------------------+
   * |90         |-90        |Right half visible, mapped clockwise               |
   * +-----------+-----------+---------------------------------------------------+
   *
   * \endverbatim
   *
   * `scol` and `srow` can be used to specify a (1-based) starting column and row
   * in the `color` array. `ncol` and `nrow` specify the actual dimension of the
   * array in the memory whereof `dimphi` and `dimr` values are mapped to the disk.
   *
   */

  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  std::shared_ptr<GRM::Element> element =
      (ext_element == nullptr) ? createElement("nonuniform_polar_cellarray") : ext_element;
  element->setAttribute("x_org", x_org);
  element->setAttribute("y_org", y_org);
  element->setAttribute("r", r_key);
  element->setAttribute("phi", phi_key);
  element->setAttribute("phi_dim", dimphi);
  element->setAttribute("r_dim", dimr);
  element->setAttribute("start_col", scol);
  element->setAttribute("start_row", srow);
  element->setAttribute("num_col", ncol);
  element->setAttribute("num_row", nrow);
  element->setAttribute("color_ind_values", color_key);
  if (color != std::nullopt)
    {
      (*use_context)[color_key] = *color;
    }
  if (phi != std::nullopt)
    {
      (*use_context)[phi_key] = *phi;
    }
  if (r != std::nullopt)
    {
      (*use_context)[r_key] = *r;
    }
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createNonUniformCellArray(
    const std::string &x_key, std::optional<std::vector<double>> x, const std::string &y_key,
    std::optional<std::vector<double>> y, int dimx, int dimy, int scol, int srow, int ncol, int nrow,
    const std::string &color_key, std::optional<std::vector<int>> color,
    const std::shared_ptr<GRM::Context> &ext_context, const std::shared_ptr<GRM::Element> &ext_element)
{
  /*!
   * This function can be used to create a non uniform cell array GRM::Element
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  std::shared_ptr<GRM::Element> element =
      (ext_element == nullptr) ? createElement("nonuniform_cellarray") : ext_element;
  element->setAttribute("x", x_key);
  element->setAttribute("y", y_key);
  element->setAttribute("color_ind_values", color_key);
  element->setAttribute("x_dim", dimx);
  element->setAttribute("y_dim", dimy);
  element->setAttribute("start_col", scol);
  element->setAttribute("start_row", srow);
  element->setAttribute("num_col", ncol);
  element->setAttribute("num_row", nrow);

  if (x != std::nullopt)
    {
      (*use_context)[x_key] = *x;
    }
  if (y != std::nullopt)
    {
      (*use_context)[y_key] = *y;
    }
  if (color != std::nullopt)
    {
      (*use_context)[color_key] = *color;
    }

  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createGrid3d(double x_tick, double y_tick, double z_tick, double x_org,
                                                        double y_org, double z_org, int x_major, int y_major,
                                                        int z_major, const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("grid_3d") : ext_element;
  element->setAttribute("x_tick", x_tick);
  element->setAttribute("y_tick", y_tick);
  element->setAttribute("z_tick", z_tick);
  element->setAttribute("x_org", x_org);
  element->setAttribute("y_org", y_org);
  element->setAttribute("z_org", z_org);
  element->setAttribute("x_major", x_major);
  element->setAttribute("y_major", y_major);
  element->setAttribute("z_major", z_major);
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createEmptyGrid3d(bool x_grid, bool y_grid, bool z_grid,
                                                             const std::shared_ptr<GRM::Element> &ext_element)
{
  auto element = (ext_element == nullptr) ? createElement("grid_3d") : ext_element;
  if (!x_grid)
    {
      element->setAttribute("x_tick", 0);
    }
  if (!y_grid)
    {
      element->setAttribute("y_tick", 0);
    }
  if (!z_grid)
    {
      element->setAttribute("z_tick", 0);
    }
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createAxes3d(double x_tick, double y_tick, double z_tick, double x_org,
                                                        double y_org, double z_org, int major_x, int major_y,
                                                        int major_z, int tick_orientation,
                                                        const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("axes_3d") : ext_element;
  element->setAttribute("x_tick", x_tick);
  element->setAttribute("y_tick", y_tick);
  element->setAttribute("z_tick", z_tick);
  element->setAttribute("x_org", x_org);
  element->setAttribute("y_org", y_org);
  element->setAttribute("z_org", z_org);
  element->setAttribute("major_x", major_x);
  element->setAttribute("major_y", major_y);
  element->setAttribute("major_z", major_z);
  element->setAttribute("tick_orientation", tick_orientation);
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createEmptyAxes3d(int tick_orientation,
                                                             const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("axes_3d") : ext_element;
  element->setAttribute("tick_orientation", tick_orientation);
  return element;
}

std::shared_ptr<GRM::Element>
GRM::Render::createPolyline3d(const std::string &x_key, std::optional<std::vector<double>> x, const std::string &y_key,
                              std::optional<std::vector<double>> y, const std::string &z_key,
                              std::optional<std::vector<double>> z, const std::shared_ptr<GRM::Context> &ext_context,
                              const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("polyline_3d") : ext_element;
  element->setAttribute("x", x_key);
  element->setAttribute("y", y_key);
  element->setAttribute("z", z_key);

  if (x != std::nullopt)
    {
      (*use_context)[x_key] = *x;
    }
  if (y != std::nullopt)
    {
      (*use_context)[y_key] = *y;
    }
  if (z != std::nullopt)
    {
      (*use_context)[z_key] = *z;
    }

  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createPolymarker3d(
    const std::string &x_key, std::optional<std::vector<double>> x, const std::string &y_key,
    std::optional<std::vector<double>> y, const std::string &z_key, std::optional<std::vector<double>> z,
    const std::shared_ptr<GRM::Context> &ext_context, const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("polymarker_3d") : ext_element;
  element->setAttribute("x", x_key);
  element->setAttribute("y", y_key);
  element->setAttribute("z", z_key);

  if (x != std::nullopt)
    {
      (*use_context)[x_key] = *x;
    }
  if (y != std::nullopt)
    {
      (*use_context)[y_key] = *y;
    }
  if (z != std::nullopt)
    {
      (*use_context)[z_key] = *z;
    }

  return element;
}

std::shared_ptr<GRM::Element>
GRM::Render::createTriSurface(const std::string &px_key, std::optional<std::vector<double>> px,
                              const std::string &py_key, std::optional<std::vector<double>> py,
                              const std::string &pz_key, std::optional<std::vector<double>> pz,
                              const std::shared_ptr<GRM::Context> &ext_context)
{
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  auto element = createSeries("trisurface");
  element->setAttribute("x", px_key);
  element->setAttribute("y", py_key);
  element->setAttribute("z", pz_key);

  if (px != std::nullopt)
    {
      (*use_context)[px_key] = *px;
    }
  if (py != std::nullopt)
    {
      (*use_context)[py_key] = *py;
    }
  if (pz != std::nullopt)
    {
      (*use_context)[pz_key] = *pz;
    }

  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createTitles3d(const std::string &xlabel, const std::string &ylabel,
                                                          const std::string &zlabel,
                                                          const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("titles_3d") : ext_element;
  element->setAttribute("x_label_3d", xlabel);
  element->setAttribute("y_label_3d", ylabel);
  element->setAttribute("z_label_3d", zlabel);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createDrawGraphics(const std::string &data_key,
                                                              std::optional<std::vector<int>> data,
                                                              const std::shared_ptr<GRM::Context> &ext_context,
                                                              const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("draw_graphics") : ext_element;

  if (data != std::nullopt)
    {
      (*use_context)[data_key] = *data;
    }
  element->setAttribute("data", data_key);
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createIsoSurfaceRenderElement(int drawable_type)
{
  auto element = createElement("isosurface_render");
  element->setAttribute("drawable_type", drawable_type);
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createLayoutGrid(const grm::Grid &grid)
{
  auto element = createElement("layout_grid");

  if (grid.absHeight != -1) element->setAttribute("absolute_height", grid.absHeight);
  if (grid.absWidth != -1) element->setAttribute("absolute_width", grid.absWidth);
  if (grid.absHeightPxl != -1) element->setAttribute("absolute_height_pxl", grid.absHeightPxl);
  if (grid.absWidthPxl != -1) element->setAttribute("absolute_width_pxl", grid.absWidthPxl);
  if (grid.relativeHeight != -1) element->setAttribute("relative_height", grid.relativeHeight);
  if (grid.relativeWidth != -1) element->setAttribute("relative_width", grid.relativeWidth);
  if (grid.aspectRatio != -1) element->setAttribute("aspect_ratio", grid.aspectRatio);
  element->setAttribute("fit_parents_height", grid.fitParentsHeight);
  element->setAttribute("fit_parents_width", grid.fitParentsWidth);
  element->setAttribute("num_row", grid.getNRows());
  element->setAttribute("num_col", grid.getNCols());

  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createLayoutGridElement(const grm::GridElement &gridElement,
                                                                   const grm::Slice &slice)
{
  auto element = createElement("layout_grid_element");

  if (gridElement.absHeight != -1) element->setAttribute("absolute_height", gridElement.absHeight);
  if (gridElement.absWidth != -1) element->setAttribute("absolute_width", gridElement.absWidth);
  if (gridElement.absHeightPxl != -1) element->setAttribute("absolute_height_pxl", gridElement.absHeightPxl);
  if (gridElement.absWidthPxl != -1) element->setAttribute("absolute_width_pxl", gridElement.absWidthPxl);
  element->setAttribute("fit_parents_height", gridElement.fitParentsHeight);
  element->setAttribute("fit_parents_width", gridElement.fitParentsWidth);
  if (gridElement.relativeHeight != -1) element->setAttribute("relative_height", gridElement.relativeHeight);
  if (gridElement.relativeWidth != -1) element->setAttribute("relative_width", gridElement.relativeWidth);
  if (gridElement.aspectRatio != -1) element->setAttribute("aspect_ratio", gridElement.aspectRatio);
  element->setAttribute("start_row", slice.rowStart);
  element->setAttribute("stop_row", slice.rowStop);
  element->setAttribute("start_col", slice.colStart);
  element->setAttribute("stop_col", slice.colStop);

  double *subplot = gridElement.subplot;
  GRM::Render::setSubplot(element, subplot[0], subplot[1], subplot[2], subplot[3]);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createPanzoom(double x, double y, double xzoom, double yzoom)
{
  auto element = createElement("panzoom");
  element->setAttribute("x", x);
  element->setAttribute("y", y);
  element->setAttribute("x_zoom", xzoom);
  element->setAttribute("y_zoom", yzoom);
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createPolarBar(double count, int class_nr,
                                                          const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("polar_bar") : ext_element;
  element->setAttribute("count", count);
  element->setAttribute("class_nr", class_nr);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createErrorBar(double error_bar_x, double error_bar_y_min,
                                                          double error_bar_y_max, int color_error_bar,
                                                          const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("error_bar") : ext_element;
  element->setAttribute("error_bar_x", error_bar_x);
  element->setAttribute("error_bar_y_min", error_bar_y_min);
  element->setAttribute("error_bar_y_max", error_bar_y_max);
  element->setAttribute("error_bar_color", color_error_bar);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createIntegral(double int_lim_low, double int_lim_high,
                                                          const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("integral") : ext_element;
  element->setAttribute("int_lim_low", int_lim_low);
  element->setAttribute("int_lim_high", int_lim_high);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createSideRegion(std::string location,
                                                            const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("side_region") : ext_element;
  element->setAttribute("location", location);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createTextRegion(const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("text_region") : ext_element;

  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createSidePlotRegion(const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("side_plot_region") : ext_element;

  return element;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ modifier functions~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

void GRM::Render::setViewport(const std::shared_ptr<GRM::Element> &element, double xmin, double xmax, double ymin,
                              double ymax)
{
  /*!
   * This function can be used to set the viewport of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] xmin The left horizontal coordinate of the viewport (0 <= xmin < xmax)
   * \param[in] xmax The right horizontal coordinate of the viewport (xmin < xmax <= 1)
   * \param[in] ymin TThe bottom vertical coordinate of the viewport (0 <= ymin < ymax)
   * \param[in] ymax The top vertical coordinate of the viewport (ymin < ymax <= 1)
   */

  element->setAttribute("viewport_x_min", xmin);
  element->setAttribute("viewport_x_max", xmax);
  element->setAttribute("viewport_y_min", ymin);
  element->setAttribute("viewport_y_max", ymax);
}


void GRM::Render::setWSViewport(const std::shared_ptr<GRM::Element> &element, double xmin, double xmax, double ymin,
                                double ymax)
{
  /*!
   * This function can be used to set the ws_viewport of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] xmin The left horizontal coordinate of the viewport (0 <= xmin < xmax)
   * \param[in] xmax The right horizontal coordinate of the viewport (xmin < xmax <= 1)
   * \param[in] ymin TThe bottom vertical coordinate of the viewport (0 <= ymin < ymax)
   * \param[in] ymax The top vertical coordinate of the viewport (ymin < ymax <= 1)
   */

  element->setAttribute("ws_viewport_x_min", xmin);
  element->setAttribute("ws_viewport_x_max", xmax);
  element->setAttribute("ws_viewport_y_min", ymin);
  element->setAttribute("ws_viewport_y_max", ymax);
}

void GRM::Render::setWindow(const std::shared_ptr<Element> &element, double xmin, double xmax, double ymin, double ymax)
{
  /*!
   * This function can be used to set the window of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] xmin The left horizontal coordinate of the window (xmin < xmax)
   * \param[in] xmax The right horizontal coordinate of the window (xmin < xmax)
   * \param[in] ymin The bottom vertical coordinate of the window (ymin < ymax)
   * \param[in] ymax The top vertical coordinate of the window (ymin < ymax)
   */

  element->setAttribute("window_x_min", xmin);
  element->setAttribute("window_x_max", xmax);
  element->setAttribute("window_y_min", ymin);
  element->setAttribute("window_y_max", ymax);
}

void GRM::Render::setWSWindow(const std::shared_ptr<Element> &element, double xmin, double xmax, double ymin,
                              double ymax)
{
  /*!
   * This function can be used to set the ws_window of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] xmin The left horizontal coordinate of the window (xmin < xmax)
   * \param[in] xmax The right horizontal coordinate of the window (xmin < xmax)
   * \param[in] ymin The bottom vertical coordinate of the window (ymin < ymax)
   * \param[in] ymax The top vertical coordinate of the window (ymin < ymax)
   */

  element->setAttribute("ws_window_x_min", xmin);
  element->setAttribute("ws_window_x_max", xmax);
  element->setAttribute("ws_window_y_min", ymin);
  element->setAttribute("ws_window_y_max", ymax);
}

void GRM::Render::setMarkerType(const std::shared_ptr<Element> &element, int type)
{
  /*!
   * This function can be used to set a MarkerType of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] type An Integer setting the MarkerType
   */
  element->setAttribute("marker_type", type);
}

void GRM::Render::setMarkerType(const std::shared_ptr<Element> &element, const std::string &types_key,
                                std::optional<std::vector<int>> types, const std::shared_ptr<GRM::Context> &ext_context)
{
  /*!
   * This function can be used to set a vector of MarkerTypes of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] types_key A string used as a key for storing the types
   * \param[in] types A vector containing the MarkerTypes
   * \param[in] ext_context A GRM::Context used for storing types. By default it uses GRM::Render's GRM::Context object
   * but an external GRM::Context can be used
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  if (types != std::nullopt)
    {
      (*use_context)[types_key] = *types;
    }
  element->setAttribute("marker_types", types_key);
}

void GRM::Render::setMarkerSize(const std::shared_ptr<Element> &element, double size)
{
  /*!
   * This function can be used to set a MarkerSize of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] type A Double setting the MarkerSize
   */
  element->setAttribute("marker_size", size);
}

void GRM::Render::setMarkerSize(const std::shared_ptr<Element> &element, const std::string &sizes_key,
                                std::optional<std::vector<double>> sizes,
                                const std::shared_ptr<GRM::Context> &ext_context)
{
  /*!
   * This function can be used to set a vector of MarkerTypes of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] sizes_key A string used as a key for storing the sizes
   * \param[in] sizes A vector containing the MarkerSizes
   * \param[in] ext_context A GRM::Context used for storing sizes. By default it uses GRM::Render's GRM::Context object
   * but an external GRM::Context can be used
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  if (sizes != std::nullopt)
    {
      (*use_context)[sizes_key] = *sizes;
    }
  element->setAttribute("marker_sizes", sizes_key);
}

void GRM::Render::setMarkerColorInd(const std::shared_ptr<Element> &element, int color)
{
  /*!
   * This function can be used to set a MarkerColorInd of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] color An Integer setting the MarkerColorInd
   */
  element->setAttribute("marker_color_ind", color);
}

void GRM::Render::setMarkerColorInd(const std::shared_ptr<Element> &element, const std::string &colorinds_key,
                                    std::optional<std::vector<int>> colorinds,
                                    const std::shared_ptr<GRM::Context> &ext_context)
{
  /*!
   * This function can be used to set a vector of MarkerColorInds of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] colorinds_key A string used as a key for storing the colorinds
   * \param[in] colorinds A vector containing the MarkerColorInds
   * \param[in] ext_context A GRM::Context used for storing colorinds. By default it uses GRM::Render's GRM::Context
   * object but an external GRM::Context can be used
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  if (colorinds != std::nullopt)
    {
      (*use_context)[colorinds_key] = *colorinds;
    }
  element->setAttribute("marker_color_indices", colorinds_key);
}

void GRM::Render::setLineType(const std::shared_ptr<Element> &element, const std::string &types_key,
                              std::optional<std::vector<int>> types, const std::shared_ptr<GRM::Context> &ext_context)
{
  /*!
   * This function can be used to set a vector of LineTypes of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] types_key A string used as a key for storing the types
   * \param[in] types A vector containing the LineTypes
   * \param[in] ext_context A GRM::Context used for storing types. By default it uses GRM::Render's GRM::Context object
   * but an external GRM::Context can be used
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  if (types != std::nullopt)
    {
      (*use_context)[types_key] = *types;
    }
  element->setAttribute("line_types", types_key);
}

void GRM::Render::setLineType(const std::shared_ptr<Element> &element, int type)
{
  /*!
   * This function can be used to set a LineType of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] type An Integer setting the LineType
   */
  element->setAttribute("line_type", type);
}

void GRM::Render::setLineWidth(const std::shared_ptr<Element> &element, const std::string &widths_key,
                               std::optional<std::vector<double>> widths,
                               const std::shared_ptr<GRM::Context> &ext_context)
{
  /*!
   * This function can be used to set a vector of LineWidths of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] widths_key A string used as a key for storing the widths
   * \param[in] widths A vector containing the LineWidths
   * \param[in] ext_context A GRM::Context used for storing widths. By default it uses GRM::Render's GRM::Context
   * object but an external GRM::Context can be used
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  if (widths != std::nullopt)
    {
      (*use_context)[widths_key] = *widths;
    }
  element->setAttribute("line_widths", widths_key);
}

void GRM::Render::setLineWidth(const std::shared_ptr<Element> &element, double width)
{
  /*!
   * This function can be used to set a LineWidth of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] type A Double setting the LineWidth
   */
  element->setAttribute("line_width", width);
}

void GRM::Render::setLineColorInd(const std::shared_ptr<Element> &element, const std::string &colorinds_key,
                                  std::optional<std::vector<int>> colorinds,
                                  const std::shared_ptr<GRM::Context> &ext_context)
{
  /*!
   * This funciton can be used to set a vector of LineColorInds of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] colorinds_key A string used as a key for storing the colorinds
   * \param[in] colorinds A vector containing the Colorinds
   * \param[in] ext_context A GRM::Context used for storing colorinds. By default it uses GRM::Render's GRM::Context
   * object but an external GRM::Context can be used
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  if (colorinds != std::nullopt)
    {
      (*use_context)[colorinds_key] = *colorinds;
    }
  element->setAttribute("line_color_indices", colorinds_key);
}

void GRM::Render::setLineColorInd(const std::shared_ptr<Element> &element, int color)
{
  /*!
   * This function can be used to set LineColorInd of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] color An Integer value setting the LineColorInd
   */
  element->setAttribute("line_color_ind", color);
}

void GRM::Render::setCharUp(const std::shared_ptr<Element> &element, double ux, double uy)
{
  /*!
   * This function can be used to set CharUp of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] ux  X coordinate of the text up vector
   * \param[in] uy  y coordinate of the text up vector
   */

  element->setAttribute("char_up_x", ux);
  element->setAttribute("char_up_y", uy);
}

void GRM::Render::setTextAlign(const std::shared_ptr<Element> &element, int horizontal, int vertical)
{
  /*!
   * This function can be used to set TextAlign of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] horizontal  Horizontal text alignment
   * \param[in] vertical Vertical text alignment
   */
  element->setAttribute("text_align_horizontal", horizontal);
  element->setAttribute("text_align_vertical", vertical);
}

void GRM::Render::setTextWidthAndHeight(const std::shared_ptr<Element> &element, double width, double height)
{
  /*!
   * This function can be used to set the width and height of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] width Width of the Element
   * \param[in] height Height of the Element
   */
  element->setAttribute("width", width);
  element->setAttribute("height", height);
}

void GRM::Render::setLineSpec(const std::shared_ptr<Element> &element, const std::string &spec)
{
  /*!
   * This function can be used to set the linespec of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] spec An std::string
   *
   */
  element->setAttribute("line_spec", spec);
}

void GRM::Render::setColorRep(const std::shared_ptr<Element> &element, int index, double red, double green, double blue)
{
  /*!
   * This function can be used to set the colorrep of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] index Color index in the range 0 to 1256
   * \param[in] red Red intensity in the range 0.0 to 1.0
   * \param[in] green Green intensity in the range 0.0 to 1.0
   * \param[in] blue Blue intensity in the range 0.0 to 1.0
   */

  int precision = 255;
  int red_int = int(red * precision + 0.5), green_int = int(green * precision + 0.5),
      blue_int = int(blue * precision + 0.5);

  // Convert RGB to hex
  std::stringstream stream;
  std::string hex;
  stream << std::hex << (red_int << 16 | green_int << 8 | blue_int);

  std::string name = "colorrep." + std::to_string(index);

  element->setAttribute(name, stream.str());
}

void GRM::Render::setFillIntStyle(const std::shared_ptr<GRM::Element> &element, int index)
{
  /*!
   * This function can be used to set the fillintstyle of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] index The style of fill to be used
   */
  element->setAttribute("fill_int_style", index);
}

void GRM::Render::setFillColorInd(const std::shared_ptr<GRM::Element> &element, int color)
{
  /*!
   * This function can be used to set the fillcolorind of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] color The fill area color index (COLOR < 1256)
   */
  element->setAttribute("fill_color_ind", color);
}

void GRM::Render::setFillStyle(const std::shared_ptr<GRM::Element> &element, int index)
{
  /*!
   * This function can be used to set the fillintstyle of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] index The fill style index to be used
   */

  element->setAttribute("fill_style", index);
}

void GRM::Render::setScale(const std::shared_ptr<GRM::Element> &element, int scale)
{
  /*!
   * This function can be used to set the scale of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] index The scale index to be used
   */
  element->setAttribute("scale", scale);
}

void GRM::Render::setWindow3d(const std::shared_ptr<GRM::Element> &element, double xmin, double xmax, double ymin,
                              double ymax, double zmin, double zmax)
{
  /*!
   * This function can be used to set the window3d of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] xmin The left horizontal coordinate of the window (xmin < xmax)
   * \param[in] xmax The right horizontal coordinate of the window (xmin < xmax)
   * \param[in] ymin The bottom vertical coordinate of the window (ymin < ymax)
   * \param[in] ymax The top vertical coordinate of the window (ymin < ymax)
   * \param[in] zmin min z-value
   * \param[in] zmax max z-value
   */

  element->setAttribute("window_x_min", xmin);
  element->setAttribute("window_x_max", xmax);
  element->setAttribute("window_y_min", ymin);
  element->setAttribute("window_y_max", ymax);
  element->setAttribute("window_z_min", zmin);
  element->setAttribute("window_z_max", zmax);
}

void GRM::Render::setSpace3d(const std::shared_ptr<GRM::Element> &element, double fov, double camera_distance)
{
  /*! This function can be used to set the space3d of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] phi: azimuthal angle of the spherical coordinates
   * \param[in] theta: polar angle of the spherical coordinates
   * \param[in] fov: vertical field of view(0 or NaN for orthographic projection)
   * \param[in] camera_distance: distance between the camera and the focus point (in arbitrary units, 0 or NaN for the
   * radius of the object's smallest bounding sphere)
   */

  element->setAttribute("space_3d_fov", fov);
  element->setAttribute("space_3d_camera_distance", camera_distance);
}

void GRM::Render::setSpace(const std::shared_ptr<Element> &element, double zmin, double zmax, int rotation, int tilt)
{
  /*!
   * This function can be used to set the space of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] zmin
   * \param[in] zmax
   * \param[in] rotation
   * \param[in] tilt
   */

  element->setAttribute("space", true);
  element->setAttribute("space_z_min", zmin);
  element->setAttribute("space_z_max", zmax);
  element->setAttribute("space_rotation", rotation);
  element->setAttribute("space_tilt", tilt);
}

void GRM::Render::setSelectSpecificXform(const std::shared_ptr<Element> &element, int transform)
{
  /*! This function can be used to set the window 3d of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] transform Select a predefined transformation from world coordinates to normalized device coordinates.
   */

  element->setAttribute("select_specific_xform", transform);
}

void GRM::Render::setTextColorInd(const std::shared_ptr<GRM::Element> &element, int index)
{
  /*!
   * This function can be used to set the textcolorind of a GRM::Element
   * \param[in] element A GRM::Element
   * \param[in] index The color index
   */

  element->setAttribute("text_color_ind", index);
}

void GRM::Render::setBorderColorInd(const std::shared_ptr<GRM::Element> &element, int index)
{
  /*!
   * This function can be used to set the bordercolorind of a GRM::Element
   * \param[in] element A GRM::Element
   * \param[in] index The color index
   */
  element->setAttribute("border_color_ind", index);
}

void GRM::Render::selectClipXForm(const std::shared_ptr<GRM::Element> &element, int form)
{
  /*!
   * This function can be used to set the clip_transformation of a GRM::Element
   * \param[in] element A GRM::Element
   * \param[in] form the clip_transformation
   */
  element->setAttribute("clip_transformation", form);
}

void GRM::Render::setCharHeight(const std::shared_ptr<GRM::Element> &element, double height)
{
  /*!
   * This function can be used to set the char height of a GRM::Element
   * \param[in] element A GRM::Element
   * \param[in] height the char height
   */
  element->setAttribute("char_height", height);
}

void GRM::Render::setProjectionType(const std::shared_ptr<GRM::Element> &element, int type)
{
  /*!
   * This function can be used to set the projection type of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] type The projection type
   */
  element->setAttribute("projection_type", type);
}

void GRM::Render::setTransparency(const std::shared_ptr<GRM::Element> &element, double alpha)
{
  /*!
   * This function can be used to set the transparency of a GRM::Element
   * \param[in] element A GRM::Element
   * \param[in] alpha The alpha
   */
  element->setAttribute("transparency", alpha);
}

void GRM::Render::setResampleMethod(const std::shared_ptr<GRM::Element> &element, int resample)
{
  /*!
   * This function can be used to set the resamplemethod of a GRM::Element
   * \param[in] element A GRM::Element
   * \param[in] resample The resample method
   */

  element->setAttribute("resample_method", resample);
}

void GRM::Render::setTextEncoding(const std::shared_ptr<Element> &element, int encoding)
{
  /*!
   * This function can be used to set the textencoding of a GRM::Element
   * \param[in] element A GRM::Element
   * \param[in] encoding The textencoding
   */
  element->setAttribute("text_encoding", encoding);
}

void GRM::Render::setSubplot(const std::shared_ptr<GRM::Element> &element, double xmin, double xmax, double ymin,
                             double ymax)
{
  element->setAttribute("plot_x_min", xmin);
  element->setAttribute("plot_x_max", xmax);
  element->setAttribute("plot_y_min", ymin);
  element->setAttribute("plot_y_max", ymax);
}

void GRM::Render::setNextColor(const std::shared_ptr<GRM::Element> &element, const std::string &color_indices_key,
                               const std::vector<int> &color_indices, const std::shared_ptr<GRM::Context> &ext_context)
{
  auto use_context = (ext_context == nullptr) ? context : ext_context;
  element->setAttribute("set_next_color", 1);
  if (!color_indices.empty())
    {
      (*use_context)[color_indices_key] = color_indices;
      element->setAttribute("color_ind_values", color_indices_key);
    }
  else
    {
      throw NotFoundError("Color indices are missing in vector\n");
    }
}

void GRM::Render::setNextColor(const std::shared_ptr<GRM::Element> &element, const std::string &color_rgb_values_key,
                               const std::vector<double> &color_rgb_values,
                               const std::shared_ptr<GRM::Context> &ext_context)
{
  auto use_context = (ext_context == nullptr) ? context : ext_context;
  element->setAttribute("set_next_color", true);
  if (!color_rgb_values.empty())
    {
      (*use_context)[color_rgb_values_key] = color_rgb_values;
      element->setAttribute("color_rgb_values", color_rgb_values_key);
    }
}

void GRM::Render::setNextColor(const std::shared_ptr<GRM::Element> &element,
                               std::optional<std::string> color_indices_key,
                               std::optional<std::string> color_rgb_values_key)
{
  if (color_indices_key != std::nullopt)
    {
      element->setAttribute("color_ind_values", (*color_indices_key));
      element->setAttribute("set_next_color", true);
    }
  else if (color_rgb_values_key != std::nullopt)
    {
      element->setAttribute("set_next_color", true);
      element->setAttribute("color_rgb_values", (*color_rgb_values_key));
    }
}

void GRM::Render::setNextColor(const std::shared_ptr<GRM::Element> &element)
{
  element->setAttribute("set_next_color", true);
  element->setAttribute("snc_fallback", true);
}

void GRM::Render::setOriginPosition(const std::shared_ptr<GRM::Element> &element, const std::string &x_org_pos,
                                    const std::string &y_org_pos)
{
  element->setAttribute("x_org_pos", x_org_pos);
  element->setAttribute("y_org_pos", y_org_pos);
}

void GRM::Render::setOriginPosition3d(const std::shared_ptr<GRM::Element> &element, const std::string &x_org_pos,
                                      const std::string &y_org_pos, const std::string &z_org_pos)
{
  setOriginPosition(element, x_org_pos, y_org_pos);
  element->setAttribute("z_org_pos", z_org_pos);
}

void GRM::Render::setGR3LightParameters(const std::shared_ptr<GRM::Element> &element, double ambient, double diffuse,
                                        double specular, double specular_power)
{
  element->setAttribute("ambient", ambient);
  element->setAttribute("diffuse", diffuse);
  element->setAttribute("specular", specular);
  element->setAttribute("specular_power", specular_power);
}

void GRM::Render::setAutoUpdate(bool update)
{
  automatic_update = update;
}

void GRM::Render::getAutoUpdate(bool *update)
{
  *update = automatic_update;
}

std::map<int, std::map<double, std::map<std::string, GRM::Value>>> *GRM::Render::getTickModificationMap()
{
  return &tick_modification_map;
}

int GRM::Render::getAxisId()
{
  return axis_id;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ filter functions~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
static void setRanges(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Element> &new_series)
{
  if (element->hasAttribute("x_range_min"))
    new_series->setAttribute("x_range_min", static_cast<double>(element->getAttribute("x_range_min")));
  if (element->hasAttribute("x_range_max"))
    new_series->setAttribute("x_range_max", static_cast<double>(element->getAttribute("x_range_max")));
  if (element->hasAttribute("y_range_min"))
    new_series->setAttribute("y_range_min", static_cast<double>(element->getAttribute("y_range_min")));
  if (element->hasAttribute("y_range_max"))
    new_series->setAttribute("y_range_max", static_cast<double>(element->getAttribute("y_range_max")));
  if (element->hasAttribute("z_range_min"))
    new_series->setAttribute("z_range_min", static_cast<double>(element->getAttribute("z_range_min")));
  if (element->hasAttribute("z_range_max"))
    new_series->setAttribute("z_range_max", static_cast<double>(element->getAttribute("z_range_max")));
}

void updateFilter(const std::shared_ptr<GRM::Element> &element, const std::string &attr, const std::string &value = "")
{
  std::vector<std::string> bar{
      "fill_color_rgb", "fill_color_ind", "line_color_rgb", "line_color_ind", "text", "x1", "x2", "y1", "y2",
  };
  std::vector<std::string> error_bar{
      "cap_x_max", "cap_x_min", "e_downwards", "e_upwards", "error_bar_x", "error_bar_y_max", "error_bar_y_min",
  };
  std::vector<std::string> marginal_heatmap_plot{
      "algorithm", "marginal_heatmap_kind", "x", "x_flip", "y", "y_flip", "z",
  };
  std::vector<std::string> polar_bar{
      "bin_width",  "bin_widths", "bin_edges", "class_nr",   "count",
      "draw_edges", "norm",       "phi_flip",  "x_colormap", "y_colormap",
  };
  std::vector<std::string> series_barplot{
      "bar_width",
      "clip_transformation",
      "color_ind_values",
      "color_rgb_values",
      "ind_bar_color",
      "ind_edge_color",
      "ind_edge_width",
      "indices",
      "inner_series",
      "orientation",
      "rgb",
      "style",
      "width",
      "y",
      "y_labels",
  };
  std::vector<std::string> series_contour{
      "levels", "px", "py", "pz", "x", "y", "z", "z_max", "z_min",
  };
  std::vector<std::string> series_contourf = series_contour;
  std::vector<std::string> series_heatmap{
      "x", "y", "z", "z_range_max", "z_range_min",
  };
  std::vector<std::string> series_hexbin{
      "num_bins",
      "x",
      "y",
  };
  std::vector<std::string> series_hist{
      "bins", "fill_color_ind", "fill_color_rgb", "line_color_ind", "line_color_rgb", "orientation",
  };
  std::vector<std::string> series_imshow{
      "data", "x", "y", "z", "z_dims",
  };
  std::vector<std::string> series_isosurface{
      "ambient", "color_rgb_values", "diffuse", "isovalue", "specular", "specular_power", "z", "z_dims",
  };
  std::vector<std::string> series_line{
      "line_spec",
      "orientation",
      "x",
      "y",
  };
  std::vector<std::string> series_nonuniformheatmap = series_heatmap;
  std::vector<std::string> series_nonuniformpolar_heatmap{
      "x", "y", "z", "z_range_max", "z_range_min",
  };
  std::vector<std::string> series_pie{
      "color_ind_values",
      "x",
  };
  std::vector<std::string> series_plot3{
      "x",
      "y",
      "z",
  };
  std::vector<std::string> series_polar{
      "clip_negative", "line_spec", "r_max", "r_min", "x", "y",
  };
  std::vector<std::string> series_polar_heatmap = series_nonuniformpolar_heatmap;
  std::vector<std::string> series_polar_histogram{
      "bin_counts", "bin_edges",    "bin_width",       "bin_widths", "classes",
      "draw_edges", "face_alpha",   "keep_radii_axes", "num_bins",   "norm",
      "r_max",      "r_min",        "stairs",          "theta",      "tick",
      "total",      "transparency", "x_colormap",      "y_colormap",
  };
  std::vector<std::string> series_quiver{
      "color_ind", "u", "v", "x", "y",
  };
  std::vector<std::string> series_scatter{
      "c", "color_ind", "orientation", "x", "y", "z",
  };
  std::vector<std::string> series_scatter3{
      "c",
      "x",
      "y",
      "z",
  };
  std::vector<std::string> series_shade{
      "transformation", "x", "x_bins", "y", "y_bins",
  };
  std::vector<std::string> series_stairs{
      "orientation", "step_where", "x", "y", "z",
  };
  std::vector<std::string> series_stem{
      "orientation",
      "x",
      "y",
      "y_range_min",
  };
  std::vector<std::string> series_surface{
      "accelerate",
      "x",
      "y",
      "z",
  };
  std::vector<std::string> series_tricontour{
      "levels",
      "x",
      "y",
      "z",
  };
  std::vector<std::string> series_trisurface{
      "x",
      "y",
      "z",
  };
  std::vector<std::string> series_volume{
      "d_max", "d_min", "x", "y", "z", "z_dims",
  };
  std::vector<std::string> series_wireframe{
      "x",
      "y",
      "z",
  };
  std::vector<std::string> coordinate_system_element{"angle_ticks", "normalization", "phi_flip", "x_grid",
                                                     "y_grid",      "z_grid",        "plot_type"};
  static std::map<std::string, std::vector<std::string>> element_names{
      {std::string("bar"), bar},
      {std::string("error_bar"), error_bar},
      {std::string("polar_bar"), polar_bar},
      {std::string("coordinate_system"), coordinate_system_element},
      {std::string("marginal_heatmap_plot"), marginal_heatmap_plot},
      {std::string("series_barplot"), series_barplot},
      {std::string("series_contour"), series_contour},
      {std::string("series_contourf"), series_contourf},
      {std::string("series_heatmap"), series_heatmap},
      {std::string("series_hexbin"), series_hexbin},
      {std::string("series_hist"), series_hist},
      {std::string("series_imshow"), series_imshow},
      {std::string("series_isosurface"), series_isosurface},
      {std::string("series_line"), series_line},
      {std::string("series_nonuniform_heatmap"), series_nonuniformheatmap},
      {std::string("series_nonuniform_polar_heatmap"), series_nonuniformpolar_heatmap},
      {std::string("series_pie"), series_pie},
      {std::string("series_plot3"), series_plot3},
      {std::string("series_polar"), series_polar},
      {std::string("series_polar_heatmap"), series_polar_heatmap},
      {std::string("series_polar_histogram"), series_polar_histogram},
      {std::string("series_quiver"), series_quiver},
      {std::string("series_scatter"), series_scatter},
      {std::string("series_scatter3"), series_scatter3},
      {std::string("series_shade"), series_shade},
      {std::string("series_stairs"), series_stairs},
      {std::string("series_stem"), series_stem},
      {std::string("series_surface"), series_surface},
      {std::string("series_tricontour"), series_tricontour},
      {std::string("series_trisurface"), series_trisurface},
      {std::string("series_volume"), series_volume},
      {std::string("series_wireframe"), series_wireframe},
  };
  // plot attributes which needs a bounding box redraw
  // TODO: critical update in plot means critical update inside childs, extend the following lists
  std::vector<std::string> plot_bbox_attributes{
      "keep_aspect_ratio",
      "only_quadratic_aspect_ratio",
      "reset_ranges",
  };
  std::vector<std::string> plot_critical_attributes{
      "colormap", "phi_flip", "x_flip", "x_log", "y_flip", "y_log", "z_flip", "z_log",
  };
  std::vector<std::string> integral_critical_attributes{
      "int_lim_high",
      "int_lim_low",
      "x_shift_wc",
  };

  // only do updates when there is a change made
  if (automatic_update && !starts_with(attr, "_"))
    {
      automatic_update = false;
      if (attr == "kind")
        {
          // special case for kind attributes to support switching the kind of a plot
          std::vector<std::string> line_group = {"line", "scatter"};
          std::vector<std::string> heatmap_group = {"contour",          "contourf", "heatmap",  "imshow",
                                                    "marginal_heatmap", "surface",  "wireframe"};
          std::vector<std::string> isosurface_group = {"isosurface", "volume"};
          std::vector<std::string> plot3_group = {"plot3", "scatter", "scatter3", "tricontour", "trisurface"};
          std::vector<std::string> barplot_group = {"barplot", "hist", "stem", "stairs"};
          std::vector<std::string> hexbin_group = {"hexbin", "shade"};
          std::shared_ptr<GRM::Element> new_element = nullptr;
          std::shared_ptr<GRM::Element> central_region, central_region_parent;

          auto plot_parent = element->parentElement();
          getPlotParent(plot_parent);
          central_region_parent = plot_parent;
          if (plot_parent->children()[0]->localName() == "marginal_heatmap_plot")
            central_region_parent = plot_parent->children()[0];
          for (const auto &child : central_region_parent->children())
            {
              if (child->localName() == "central_region")
                {
                  central_region = child;
                  break;
                }
            }

          if (std::find(line_group.begin(), line_group.end(), value) != line_group.end() &&
              std::find(line_group.begin(), line_group.end(),
                        static_cast<std::string>(element->getAttribute("kind"))) != line_group.end())
            {
              auto new_series = global_render->createSeries(static_cast<std::string>(element->getAttribute("kind")));
              new_element = new_series;
              element->parentElement()->insertBefore(new_series, element);
              plot_parent->setAttribute("kind", static_cast<std::string>(element->getAttribute("kind")));
              new_series->setAttribute("x", element->getAttribute("x"));
              new_series->setAttribute("y", element->getAttribute("y"));
              new_series->setAttribute("_bbox_id", -1);
              if (element->hasAttribute("orientation"))
                new_series->setAttribute("orientation", static_cast<std::string>(element->getAttribute("orientation")));
              if (static_cast<int>(central_region->getAttribute("keep_window"))) setRanges(element, new_series);
              for (const auto &child : element->children())
                {
                  if (child->localName() == "error_bars") new_series->append(child);
                  child->remove();
                }
              element->remove();
              new_series->setAttribute("_update_required", true);
              new_series->setAttribute("_delete_children", 2);
              auto legend = global_render->querySelectors("legend");
              if (legend != nullptr)
                {
                  legend->setAttribute("_delete_children", 2);
                  legend->removeAttribute("_legend_elems");
                }
            }
          else if (std::find(heatmap_group.begin(), heatmap_group.end(), value) != heatmap_group.end() &&
                   std::find(heatmap_group.begin(), heatmap_group.end(),
                             static_cast<std::string>(element->getAttribute("kind"))) != heatmap_group.end())
            {
              std::shared_ptr<GRM::Element> new_series;
              if (static_cast<std::string>(element->getAttribute("kind")) == "marginal_heatmap")
                {
                  new_series = global_render->createElement("marginal_heatmap_plot");
                  new_series->setAttribute("kind", "marginal_heatmap");
                }
              else
                {
                  new_series = global_render->createSeries(static_cast<std::string>(element->getAttribute("kind")));
                }
              new_element = new_series;
              if (value == "marginal_heatmap")
                {
                  std::shared_ptr<GRM::Element> first_side_region = nullptr;
                  // kind was 'marginal_heatmap' so the marginal_heatmap_plot must be removed and a new series created
                  for (const auto &child : element->children())
                    {
                      if (child->localName() == "side_region")
                        {
                          element->parentElement()->append(child);
                          if (first_side_region == nullptr) first_side_region = child;
                          for (const auto &side_region_child : child->children())
                            {
                              if (side_region_child->localName() == "side_plot_region" &&
                                  child->hasAttribute("marginal_heatmap_side_plot"))
                                {
                                  side_region_child->remove();
                                  break;
                                }
                            }
                          if (child->hasAttribute("marginal_heatmap_side_plot"))
                            child->removeAttribute("marginal_heatmap_side_plot");
                          continue;
                        }
                      if (child->localName() == "central_region")
                        {
                          for (const auto &central_region_child : child->children())
                            {
                              if (central_region_child->hasAttribute("_child_id") &&
                                  static_cast<int>(central_region_child->getAttribute("_child_id")) == 0 &&
                                  central_region_child->localName() == "series_heatmap")
                                central_region_child->remove();
                            }
                        }
                    }
                  element->parentElement()->insertBefore(central_region, first_side_region);
                  central_region->append(new_series);
                }
              else if (static_cast<std::string>(element->getAttribute("kind")) == "marginal_heatmap")
                {
                  std::shared_ptr<GRM::Element> first_side_region = nullptr;
                  // move the side_regions into the marginal_heatmap_plot
                  for (const auto &side_region_child : central_region->parentElement()->children())
                    {
                      if (side_region_child->localName() == "side_region")
                        {
                          if (first_side_region == nullptr) first_side_region = side_region_child;
                          new_series->append(side_region_child);
                          if (side_region_child->querySelectors("colorbar"))
                            {
                              side_region_child->querySelectors("colorbar")->parentElement()->remove();
                              side_region_child->setAttribute("offset", PLOT_DEFAULT_SIDEREGION_OFFSET);
                              side_region_child->setAttribute("width", PLOT_DEFAULT_SIDEREGION_WIDTH);
                            }
                        }
                    }
                  // create marginal_heatmap_plot as central_region father
                  central_region->parentElement()->insertBefore(new_series, central_region);
                  new_series->insertBefore(central_region, first_side_region);
                  // declare which side_region contains the marginal_heatmap side_plot
                  new_series->querySelectors("side_region[location=\"top\"]")
                      ->setAttribute("marginal_heatmap_side_plot", 1);
                  new_series->querySelectors("side_region[location=\"right\"]")
                      ->setAttribute("marginal_heatmap_side_plot", 1);
                }
              else
                {
                  element->parentElement()->insertBefore(new_series, element);
                }
              plot_parent->setAttribute("kind", static_cast<std::string>(element->getAttribute("kind")));
              new_series->setAttribute("x", element->getAttribute("x"));
              new_series->setAttribute("y", element->getAttribute("y"));
              new_series->setAttribute("z", element->getAttribute("z"));
              new_series->setAttribute("_bbox_id", -1);
              if (static_cast<int>(central_region->getAttribute("keep_window"))) setRanges(element, new_series);
              if (static_cast<std::string>(element->getAttribute("kind")) == "imshow")
                {
                  auto context = global_render->getContext();
                  auto id = static_cast<int>(global_root->getAttribute("_id"));
                  auto str = std::to_string(id);
                  auto x = static_cast<std::string>(element->getAttribute("x"));
                  auto x_vec = GRM::get<std::vector<double>>((*context)[x]);
                  int x_length = (int)x_vec.size();
                  auto y = static_cast<std::string>(element->getAttribute("y"));
                  auto y_vec = GRM::get<std::vector<double>>((*context)[y]);
                  int y_length = (int)y_vec.size();
                  std::vector<int> z_dims_vec = {(int)x_length, (int)y_length};
                  (*context)["z_dims" + str] = z_dims_vec;
                  new_series->setAttribute("z_dims", "z_dims" + str);
                  global_root->setAttribute("_id", id++);
                }
              if (value == "imshow")
                {
                  if (element->hasAttribute("_x_org"))
                    {
                      auto x_key = static_cast<std::string>(element->getAttribute("_x_org"));
                      new_series->setAttribute("x", x_key);
                    }
                  if (element->hasAttribute("_y_org"))
                    {
                      auto y_key = static_cast<std::string>(element->getAttribute("_y_org"));
                      new_series->setAttribute("y", y_key);
                    }
                }

              for (const auto &child : element->children())
                {
                  child->remove();
                }
              element->remove();
              new_series->setAttribute("_update_required", true);
              new_series->setAttribute("_delete_children", 2);
            }
          else if (std::find(isosurface_group.begin(), isosurface_group.end(), value) != isosurface_group.end() &&
                   std::find(isosurface_group.begin(), isosurface_group.end(),
                             static_cast<std::string>(element->getAttribute("kind"))) != isosurface_group.end())
            {
              auto new_series = global_render->createSeries(static_cast<std::string>(element->getAttribute("kind")));
              new_element = new_series;
              element->parentElement()->insertBefore(new_series, element);
              plot_parent->setAttribute("kind", static_cast<std::string>(element->getAttribute("kind")));
              new_series->setAttribute("z", element->getAttribute("z"));
              new_series->setAttribute("z_dims", element->getAttribute("z_dims"));
              if (element->hasAttribute("d_min")) new_series->setAttribute("d_min", element->getAttribute("d_min"));
              if (element->hasAttribute("d_max")) new_series->setAttribute("d_max", element->getAttribute("d_max"));
              new_series->setAttribute("_bbox_id", -1);
              if (static_cast<int>(central_region->getAttribute("keep_window"))) setRanges(element, new_series);
              for (const auto &child : element->children())
                {
                  child->remove();
                }
              element->remove();
              new_series->setAttribute("_update_required", true);
              new_series->setAttribute("_delete_children", 2);
            }
          else if (std::find(plot3_group.begin(), plot3_group.end(), value) != plot3_group.end() &&
                   std::find(plot3_group.begin(), plot3_group.end(),
                             static_cast<std::string>(element->getAttribute("kind"))) != plot3_group.end())
            {
              auto new_series = global_render->createSeries(static_cast<std::string>(element->getAttribute("kind")));
              new_element = new_series;
              element->parentElement()->insertBefore(new_series, element);
              plot_parent->setAttribute("kind", static_cast<std::string>(element->getAttribute("kind")));
              new_series->setAttribute("x", element->getAttribute("x"));
              new_series->setAttribute("y", element->getAttribute("y"));
              new_series->setAttribute("z", element->getAttribute("z"));
              new_series->setAttribute("_bbox_id", -1);
              if (static_cast<int>(central_region->getAttribute("keep_window"))) setRanges(element, new_series);
              for (const auto &child : element->children())
                {
                  child->remove();
                }
              element->remove();
              new_series->setAttribute("_update_required", true);
              new_series->setAttribute("_delete_children", 2);
            }
          else if (std::find(barplot_group.begin(), barplot_group.end(), value) != barplot_group.end() &&
                   std::find(barplot_group.begin(), barplot_group.end(),
                             static_cast<std::string>(element->getAttribute("kind"))) != barplot_group.end())
            {
              auto new_series = global_render->createSeries(static_cast<std::string>(element->getAttribute("kind")));
              new_element = new_series;
              element->parentElement()->insertBefore(new_series, element);
              plot_parent->setAttribute("kind", static_cast<std::string>(element->getAttribute("kind")));
              new_series->setAttribute("x", element->getAttribute("x"));
              new_series->setAttribute("_bbox_id", -1);
              if (element->hasAttribute("orientation"))
                new_series->setAttribute("orientation", static_cast<std::string>(element->getAttribute("orientation")));
              if (static_cast<int>(central_region->getAttribute("keep_window"))) setRanges(element, new_series);
              if (static_cast<std::string>(element->getAttribute("kind")) == "hist")
                {
                  new_series->setAttribute("weights", element->getAttribute("y"));
                }
              else
                {
                  if (value == "hist")
                    {
                      new_series->setAttribute("y", element->getAttribute("weights"));
                    }
                  else
                    {
                      new_series->setAttribute("y", element->getAttribute("y"));
                    }
                }

              for (const auto &child : element->children())
                {
                  if (child->localName() == "error_bars") new_series->append(child);
                  child->remove();
                }
              element->remove();
              new_series->setAttribute("_update_required", true);
              new_series->setAttribute("_delete_children", 2);
            }
          else if (std::find(hexbin_group.begin(), hexbin_group.end(), value) != hexbin_group.end() &&
                   std::find(hexbin_group.begin(), hexbin_group.end(),
                             static_cast<std::string>(element->getAttribute("kind"))) != hexbin_group.end())
            {
              auto new_series = global_render->createSeries(static_cast<std::string>(element->getAttribute("kind")));
              new_element = new_series;
              element->parentElement()->insertBefore(new_series, element);
              plot_parent->setAttribute("kind", static_cast<std::string>(element->getAttribute("kind")));
              new_series->setAttribute("x", element->getAttribute("x"));
              new_series->setAttribute("y", element->getAttribute("y"));
              new_series->setAttribute("_bbox_id", -1);
              if (static_cast<int>(central_region->getAttribute("keep_window"))) setRanges(element, new_series);
              for (const auto &child : element->children())
                {
                  child->remove();
                }
              element->remove();
              new_series->setAttribute("_update_required", true);
              new_series->setAttribute("_delete_children", 2);
            }
          else
            {
              element->setAttribute("kind", value);
              fprintf(stderr, "Update kind %s to %s is not possible\n",
                      static_cast<std::string>(element->getAttribute("kind")).c_str(), value.c_str());
              std::cerr << toXML(element->getRootNode(), GRM::SerializerOptions{std::string(2, ' ')}) << "\n";
            }

          if (new_element)
            {
              auto plot = new_element->parentElement();
              getPlotParent(plot);

              /* update the limits since they depend on the kind */
              plot->setAttribute("_update_limits", 1);

              /* overwrite attributes for which a kind dependent default exists */
              bool grplot = plot->hasAttribute("grplot") ? static_cast<int>(plot->getAttribute("grplot")) : false;
              if (grplot)
                {
                  plot->setAttribute("_overwrite_kind_dependent_defaults", 1);
                  applyPlotDefaults(plot);
                  plot->removeAttribute("_overwrite_kind_dependent_defaults");
                }

              /* update coordinate system if needed */
              std::vector<std::string> colorbar_group = {"quiver",        "contour",   "contourf", "hexbin",
                                                         "polar_heatmap", "heatmap",   "surface",  "volume",
                                                         "trisurface",    "tricontour"};

              std::shared_ptr<GRM::Element> coordinate_system = plot->querySelectors("coordinate_system");
              std::string new_kind = static_cast<std::string>(new_element->getAttribute("kind"));
              std::string new_type = "2d";
              const std::string &old_kind = value;
              if (polar_kinds.count(new_kind) != 0) new_type = "polar";
              if (kinds_3d.count(new_kind) != 0) new_type = "3d";

              // the default diag_factor must be recalculated cause the default plot size can diverge
              // f.e. surface plots are smaller than heatmap plots so the diag_factor isn't the same
              if (old_kind != new_kind)
                {
                  for (const auto &elem : global_root->querySelectorsAll("[_default_diag_factor]"))
                    {
                      elem->removeAttribute("_default_diag_factor");
                    }
                  if (new_type == "3d" && !central_region->hasAttribute("_diag_factor_set_by_user"))
                    central_region->removeAttribute("diag_factor");
                }

              if (coordinate_system)
                {
                  auto left_side_region = plot->querySelectors("side_region[location=\"left\"]");
                  auto bottom_side_region = plot->querySelectors("side_region[location=\"bottom\"]");
                  auto old_type = static_cast<std::string>(coordinate_system->getAttribute("plot_type"));
                  if (grplot && old_type == "2d" &&
                      new_type == "2d") // special case which will reset the tick_orientation when kind is changed
                    {
                      coordinate_system->setAttribute("_update_required", true);
                      coordinate_system->setAttribute("_delete_children",
                                                      static_cast<int>(del_values::update_with_default));
                    }
                  if (new_type != old_type)
                    {
                      coordinate_system->setAttribute("plot_type", new_type);
                      coordinate_system->setAttribute("_update_required", true);
                      coordinate_system->setAttribute("_delete_children",
                                                      static_cast<int>(del_values::recreate_all_children));

                      if (old_type == "2d")
                        {
                          if (bottom_side_region->hasAttribute("text_content"))
                            {
                              coordinate_system->setAttribute(
                                  "x_label",
                                  static_cast<std::string>(bottom_side_region->getAttribute("text_content")));
                              bottom_side_region->removeAttribute("text_content");
                              bottom_side_region->setAttribute("_update_required", true);

                              auto text_child = bottom_side_region->querySelectors("text_region");
                              if (text_child != nullptr) bottom_side_region->removeChild(text_child);
                            }
                          if (left_side_region->hasAttribute("text_content"))
                            {
                              coordinate_system->setAttribute(
                                  "y_label", static_cast<std::string>(left_side_region->getAttribute("text_content")));
                              left_side_region->removeAttribute("text_content");
                              left_side_region->setAttribute("_update_required", true);

                              auto text_child = left_side_region->querySelectors("text_region");
                              if (text_child != nullptr) left_side_region->removeChild(text_child);
                            }
                        }
                      else if (old_type == "3d" && new_type == "2d")
                        {
                          if (coordinate_system->hasAttribute("x_label"))
                            {
                              bottom_side_region->setAttribute(
                                  "text_content", static_cast<std::string>(coordinate_system->getAttribute("x_label")));
                              bottom_side_region->setAttribute("_update_required", true);
                              coordinate_system->removeAttribute("x_label");
                            }
                          if (coordinate_system->hasAttribute("y_label"))
                            {
                              left_side_region->setAttribute(
                                  "text_content", static_cast<std::string>(coordinate_system->getAttribute("y_label")));
                              left_side_region->setAttribute("_update_required", true);
                              coordinate_system->removeAttribute("y_label");
                            }
                        }
                    }
                  if (new_kind == "imshow" || new_kind == "isosurface")
                    {
                      auto top_side_region = plot->querySelectors("side_region[location=\"top\"]");

                      left_side_region->setAttribute("_update_required", true);
                      left_side_region->setAttribute("_delete_children", 2);
                      bottom_side_region->setAttribute("_update_required", true);
                      bottom_side_region->setAttribute("_delete_children", 2);
                      top_side_region->setAttribute("_update_required", true);
                      top_side_region->setAttribute("_delete_children", 2);
                      coordinate_system->setAttribute("hide", true);
                    }
                  else if (old_kind == "imshow" || old_kind == "isosurface")
                    {
                      coordinate_system->removeAttribute("hide");
                    }
                  else if ((new_kind == "shade" && old_kind == "hexbin") ||
                           (new_kind == "hexbin" && old_kind == "shade"))
                    {
                      /* special case cause shade doesn't have a grid element while hexbin has */
                      coordinate_system->setAttribute("_update_required", true);
                      coordinate_system->setAttribute("_delete_children",
                                                      static_cast<int>(del_values::recreate_all_children));

                      for (const auto &child : coordinate_system->children())
                        {
                          if (child->localName() == "axis")
                            {
                              if (new_kind == "shade") child->setAttribute("draw_grid", false);
                              if (new_kind == "hexbin") child->setAttribute("draw_grid", true);
                            }
                        }
                    }
                  if (grplot && (new_kind == "barplot" || old_kind == "barplot"))
                    {
                      // barplot has different x_major value
                      int major_count, x_major = 1;
                      for (const auto &child : coordinate_system->children())
                        {
                          if (child->localName() == "axis")
                            {
                              auto axis_type = static_cast<std::string>(child->getAttribute("axis_type"));
                              std::string orientation = PLOT_DEFAULT_ORIENTATION;
                              if (new_element != nullptr)
                                orientation = static_cast<std::string>(new_element->getAttribute("orientation"));

                              getMajorCount(child, new_kind, major_count);
                              if (new_kind == "barplot")
                                {
                                  bool problematic_bar_num = false, only_barplot = true;
                                  auto context = global_render->getContext();
                                  auto barplots = central_region->querySelectorsAll("series_barplot");
                                  for (const auto &barplot : barplots)
                                    {
                                      if (!barplot->hasAttribute("style") ||
                                          static_cast<std::string>(barplot->getAttribute("style")) == "default")
                                        {
                                          auto y_key = static_cast<std::string>(barplot->getAttribute("y"));
                                          std::vector<double> y_vec = GRM::get<std::vector<double>>((*context)[y_key]);
                                          if (size(y_vec) > 20)
                                            {
                                              problematic_bar_num = true;
                                              break;
                                            }
                                        }
                                    }
                                  x_major = problematic_bar_num ? major_count : 1;

                                  // barplot has some special default which can only be applied if no other kind is
                                  // included inside the central_region
                                  for (const auto &series : central_region->children())
                                    {
                                      if (starts_with(series->localName(), "series_") &&
                                          series->localName() != "series_barplot")
                                        {
                                          only_barplot = false;
                                          break;
                                        }
                                    }
                                  if (only_barplot)
                                    {
                                      plot_parent->setAttribute("adjust_x_lim", false);
                                      if (axis_type == "x" && orientation == "horizontal")
                                        child->setAttribute("draw_grid", false);
                                      if (axis_type == "y" && orientation == "vertical")
                                        child->setAttribute("draw_grid", false);
                                    }
                                }
                              else
                                {
                                  x_major = major_count;
                                  plot_parent->setAttribute("adjust_x_lim", true);
                                  if (axis_type == "x" && orientation == "horizontal")
                                    child->setAttribute("draw_grid", true);
                                  if (axis_type == "y" && orientation == "vertical")
                                    child->setAttribute("draw_grid", true);
                                }
                              // reset attributes on axis so the axis gets recreated which is needed cause the barplot
                              // has a different window
                              clearAxisAttributes(child);
                              child->setAttribute("x_major", x_major);
                            }
                        }
                    }
                  if (grplot && (new_kind == "barplot" || new_kind == "stem"))
                    {
                      if (coordinate_system->hasAttribute("y_line")) coordinate_system->setAttribute("y_line", true);
                    }
                  else if (grplot && (old_kind == "barplot" || old_kind == "stem"))
                    {
                      if (coordinate_system->hasAttribute("y_line")) coordinate_system->setAttribute("y_line", false);
                    }
                  // heatmap and marginal_heatmap have a different default behaviour when it comes to adjust lims
                  if (grplot && (new_kind == "heatmap" || new_kind == "marginal_heatmap"))
                    {
                      bool no_other_kind = true;
                      for (const auto &series : central_region->children())
                        {
                          if (starts_with(series->localName(), "series_") && series->localName() != "series_heatmap")
                            {
                              no_other_kind = false;
                              break;
                            }
                        }

                      if (no_other_kind)
                        {
                          for (const auto &child : coordinate_system->children())
                            {
                              if (child->localName() == "axis") clearAxisAttributes(child);
                            }
                          plot_parent->setAttribute("adjust_x_lim", false);
                          plot_parent->setAttribute("adjust_y_lim", false);
                          plot_parent->setAttribute("adjust_z_lim", false);
                        }
                    }
                  else if (grplot && (old_kind == "heatmap" || old_kind == "marginal_heatmap"))
                    {
                      for (const auto &child : coordinate_system->children())
                        {
                          if (child->localName() == "axis") clearAxisAttributes(child);
                        }
                      plot_parent->setAttribute("adjust_x_lim", true);
                      plot_parent->setAttribute("adjust_y_lim", true);
                      plot_parent->setAttribute("adjust_z_lim", true);
                    }
                  // need to change tick_orientation if old_kind was heatmap, marginal_heatmap or shade and no other
                  // series of these kinds is left
                  if (grplot && (str_equals_any(old_kind, "contourf", "heatmap", "marginal_heatmap", "shade")))
                    {
                      bool one_of_these_kinds_left = true;
                      for (const auto &series : central_region->children())
                        {
                          if (starts_with(series->localName(), "series_") &&
                              !str_equals_any(series->localName(), "series_contourf", "series_heatmap", "series_shade"))
                            {
                              one_of_these_kinds_left = false;
                              break;
                            }
                        }
                      if (!one_of_these_kinds_left)
                        {
                          for (const auto &child : coordinate_system->children())
                            {
                              if (child->localName() == "axis")
                                {
                                  if (child->hasAttribute("tick_orientation"))
                                    child->setAttribute("tick_orientation", 1);
                                }
                            }
                        }
                    }
                }
              else if (new_kind != "imshow" && new_kind != "isosurface")
                {
                  coordinate_system = global_render->createElement("coordinate_system");
                  coordinate_system->setAttribute("plot_type", new_type);
                  plot->prepend(coordinate_system);
                }
              if (std::find(colorbar_group.begin(), colorbar_group.end(), new_kind) == colorbar_group.end())
                {
                  std::shared_ptr<GRM::Element> side_region, colorbar = plot->querySelectors("colorbar"), side_plot;
                  if (colorbar)
                    {
                      side_plot = colorbar->parentElement();
                      side_region = side_plot->parentElement();
                      for (const auto &child : colorbar->children())
                        {
                          child->remove();
                        }
                      colorbar->remove();
                      side_region->remove();
                      side_plot->remove();
                    }
                }
              else
                {
                  double offset;
                  int colors;
                  std::shared_ptr<GRM::Element> side_region = plot->querySelectors("side_region[location=\"right\"]"),
                                                colorbar = plot->querySelectors("colorbar"), side_plot = nullptr;
                  std::tie(offset, colors) = getColorbarAttributes(new_kind, plot);

                  if (side_region == nullptr)
                    side_region = global_render->createSideRegion(PLOT_DEFAULT_SIDEREGION_LOCATION);
                  else
                    side_plot = side_region->querySelectors("side_plot_region");
                  side_region->setAttribute("offset", offset + PLOT_DEFAULT_COLORBAR_OFFSET);
                  side_region->setAttribute("width", PLOT_DEFAULT_COLORBAR_WIDTH);
                  side_region->setAttribute("_update_required", true);

                  if (side_plot == nullptr) side_plot = global_render->createSidePlotRegion();

                  colorbar = global_render->createColorbar(colors, nullptr, colorbar);
                  colorbar->setAttribute("max_char_height", PLOT_DEFAULT_COLORBAR_MAX_CHAR_HEIGHT);

                  colorbar->setAttribute("_update_required", true);
                  colorbar->setAttribute("_delete_children", static_cast<int>(del_values::recreate_all_children));

                  if (!plot->querySelectors("side_region[location=\"right\"]")) plot->append(side_region);
                  if (!side_region->querySelectors("side_plot_region")) side_region->append(side_plot);
                  if (!plot->querySelectors("colorbar")) side_plot->append(colorbar);
                }
            }
        }
      else
        {
          if (auto search = element_names.find(element->localName()); search != element_names.end())
            {
              // attributes which causes a critical update
              if (std::find(search->second.begin(), search->second.end(), attr) != search->second.end())
                {
                  element->setAttribute("_update_required", true);
                  element->setAttribute("_delete_children", 1);
                  if (str_equals_any(attr, "bin_edges", "bin_widths", "bins", "c", "draw_edges", "inner_series",
                                     "levels", "marginal_heatmap_kind", "num_bins", "px", "py", "pz", "stairs", "u",
                                     "v", "x", "x_colormap", "y", "y_colormap", "z"))
                    element->setAttribute("_delete_children", 2);
                  if (attr == "orientation")
                    {
                      resetOldBoundingBoxes(element->parentElement());
                      resetOldBoundingBoxes(element);
                      element->removeAttribute("_bbox_id");
                    }
                  if (element->localName() == "series_polar_histogram" && (attr == "num_bins" || attr == "bin_widths"))
                    {
                      for (const auto &child : element->children())
                        {
                          if (child->localName() == "polar_bar") child->setAttribute("_update_required", true);
                        }
                    }
                  if (element->localName() == "coordinate_system")
                    {
                      element->setAttribute("_delete_children", 0);
                    }
                }
            }
          else if (starts_with(element->localName(), "series"))
            {
              // conditional critical attributes
              auto kind = static_cast<std::string>(element->getAttribute("kind"));
              if (kind == "heatmap" || kind == "polar_heatmap")
                {
                  if (element->hasAttribute("c_range_max") && element->hasAttribute("c_range_min"))
                    {
                      if (attr == "c_range_max" || attr == "c_range_min")
                        {
                          element->setAttribute("_update_required", true);
                          element->setAttribute("_delete_children", 2);
                        }
                    }
                }
              if (kind == "heatmap" || kind == "surface" || kind == "polar_heatmap")
                {
                  if (!element->hasAttribute("x") && !element->hasAttribute("y"))
                    {
                      if (attr == "z_dims")
                        {
                          element->setAttribute("_update_required", true);
                          element->setAttribute("_delete_children", 2);
                        }
                    }
                  if (!element->hasAttribute("x"))
                    {
                      if (attr == "x_range_max" || attr == "x_range_min")
                        {
                          element->setAttribute("_update_required", true);
                          element->setAttribute("_delete_children", 2);
                        }
                    }
                  if (!element->hasAttribute("y"))
                    {
                      if (attr == "y_range_max" || attr == "y_range_min")
                        {
                          element->setAttribute("_update_required", true);
                          element->setAttribute("_delete_children", 2);
                        }
                    }
                }
              else if (kind == "marginal_heatmap" && (attr == "x_ind" || attr == "y_ind"))
                {
                  auto x_ind = static_cast<int>(element->getAttribute("x_ind"));
                  auto y_ind = static_cast<int>(element->getAttribute("y_ind"));
                  if (attr == "x_ind" && y_ind != -1) element->setAttribute("_update_required", true);
                  if (attr == "y_ind" && x_ind != -1) element->setAttribute("_update_required", true);
                }
            }
          else if ((attr == "size_x" || attr == "size_y") && element->localName() == "figure" &&
                   static_cast<int>(element->getAttribute("active")))
            {
              // imshow needs vp which gets inflicted by the size attributes in figure, this means when the size is
              // changed the imshow_series must be recalculated
              auto imshow = global_render->querySelectorsAll("series_imshow");
              for (const auto &imshow_elem : imshow)
                {
                  imshow_elem->setAttribute("_update_required", true);
                }

              // reset the bounding boxes for all series
              for (const auto &child : element->children()) // plot level
                {
                  for (const auto &childchild : child->children())
                    {
                      if (starts_with(childchild->localName(), "series_")) resetOldBoundingBoxes(childchild);
                      if (childchild->localName() == "series_marginal_heatmap")
                        {
                          for (const auto &childchildchild : childchild->children())
                            {
                              if (starts_with(childchildchild->localName(), "series_"))
                                resetOldBoundingBoxes(childchildchild);
                            }
                        }
                    }
                }

              // reset the bounding boxes for elements in list
              for (const std::string name : {"bar", "pie_segment", "colorbar", "polar_bar", "coordinate_system",
                                             "axes_text_group", "plot", "error_bar", "integral", "integral_group"})
                {
                  for (const auto &elem : element->querySelectorsAll(name))
                    {
                      resetOldBoundingBoxes(elem);
                      // plot gets calculated to quit so this special case is needed to get the right bboxes
                      if (name == "plot") elem->removeAttribute("_bbox_id");
                    }
                }

              // reset the bounding boxes for figure
              resetOldBoundingBoxes(element);
              element->removeAttribute("_bbox_id");
            }
          else if (element->localName() == "plot" && std::find(plot_bbox_attributes.begin(), plot_bbox_attributes.end(),
                                                               attr) != plot_bbox_attributes.end())
            {
              // when the ranges gets reseted the bounding boxes of the series can be wrong, to solve this problem
              // they get calculated again out of their children
              if (attr == "reset_ranges")
                {
                  for (const auto &child : element->children())
                    {
                      if (starts_with(child->localName(), "series_")) resetOldBoundingBoxes(child);
                    }

                  // reset the bounding boxes for elements in list
                  for (const std::string name : {"bar", "pie_segment", "colorbar", "polar_bar", "axes_text_group",
                                                 "error_bar", "integral", "integral_group"})
                    {
                      for (const auto &elem : element->querySelectorsAll(name))
                        {
                          resetOldBoundingBoxes(elem);
                        }
                    }
                }
              else
                {
                  resetOldBoundingBoxes(element);
                }
            }
          else if (element->localName() == "plot" &&
                   std::find(plot_critical_attributes.begin(), plot_critical_attributes.end(), attr) !=
                       plot_critical_attributes.end())
            {
              for (const auto &child : element->children())
                {
                  if (starts_with(child->localName(), "series_"))
                    {
                      child->setAttribute("_update_required", true);

                      // reset data when log is set to false
                      if (attr == "x_log")
                        {
                          if (child->hasAttribute("x") && child->hasAttribute("_x_org"))
                            {
                              auto x_org = static_cast<std::string>(element->getAttribute("_x_org"));
                              element->setAttribute("x", x_org);
                            }
                          if (element->hasAttribute("x_range_min") && !element->hasAttribute("_x_range_min_org"))
                            element->setAttribute("x_range_min",
                                                  static_cast<double>(element->getAttribute("_x_range_min_org")));
                          if (element->hasAttribute("x_range_max") && !element->hasAttribute("_x_range_max_org"))
                            element->setAttribute("x_range_max",
                                                  static_cast<double>(element->getAttribute("_x_range_max_org")));
                        }
                      if (attr == "y_log")
                        {
                          if (child->hasAttribute("y") && child->hasAttribute("_y_org"))
                            {
                              auto y_org = static_cast<std::string>(element->getAttribute("_y_org"));
                              element->setAttribute("y", y_org);
                            }
                          if (element->hasAttribute("y_range_min") && !element->hasAttribute("_y_range_min_org"))
                            element->setAttribute("y_range_min",
                                                  static_cast<double>(element->getAttribute("_y_range_min_org")));
                          if (element->hasAttribute("y_range_max") && !element->hasAttribute("_y_range_max_org"))
                            element->setAttribute("y_range_max",
                                                  static_cast<double>(element->getAttribute("_y_range_max_org")));
                        }
                      if (attr == "z_log")
                        {
                          if (child->hasAttribute("z") && child->hasAttribute("_z_org"))
                            {
                              auto z_org = static_cast<std::string>(element->getAttribute("_z_org"));
                              element->setAttribute("z", z_org);
                            }
                          if (element->hasAttribute("z_range_min") && !element->hasAttribute("_z_range_min_org"))
                            element->setAttribute("z_range_min",
                                                  static_cast<double>(element->getAttribute("_z_range_min_org")));
                          if (element->hasAttribute("z_range_max") && !element->hasAttribute("_z_range_max_org"))
                            element->setAttribute("z_range_max",
                                                  static_cast<double>(element->getAttribute("_z_range_max_org")));
                        }
                    }
                }
            }
          else if (attr == "location")
            {
              if (element->hasAttribute("x_scale_ndc")) element->removeAttribute("x_scale_ndc");
              if (element->hasAttribute("x_shift_ndc")) element->removeAttribute("x_shift_ndc");
              if (element->hasAttribute("y_scale_ndc")) element->removeAttribute("y_scale_ndc");
              if (element->hasAttribute("y_shift_ndc")) element->removeAttribute("y_shift_ndc");
              resetOldBoundingBoxes(element);
            }
          else if (element->localName() == "integral" &&
                   std::find(integral_critical_attributes.begin(), integral_critical_attributes.end(), attr) !=
                       integral_critical_attributes.end())
            {
              element->setAttribute("_update_required", true);
            }
          else if (element->parentElement() != nullptr && element->parentElement()->localName() == "integral" &&
                   element->localName() == "fill_area" && attr == "x_shift_wc")
            {
              element->parentElement()->setAttribute("_update_required", true);
            }
          else if (attr == "viewport_x_min")
            {
              double old_shift = (element->hasAttribute("_x_min_shift"))
                                     ? static_cast<double>(element->getAttribute("_x_min_shift"))
                                     : 0;
              element->setAttribute("_x_min_shift",
                                    old_shift + static_cast<double>(element->getAttribute(attr)) - std::stod(value));
              element->setAttribute("_update_required", true);
            }
          else if (attr == "viewport_x_max")
            {
              double old_shift = (element->hasAttribute("_x_max_shift"))
                                     ? static_cast<double>(element->getAttribute("_x_max_shift"))
                                     : 0;
              element->setAttribute("_x_max_shift",
                                    old_shift + static_cast<double>(element->getAttribute(attr)) - std::stod(value));
              element->setAttribute("_update_required", true);
            }
          else if (attr == "viewport_y_min")
            {
              double old_shift = (element->hasAttribute("_y_min_shift"))
                                     ? static_cast<double>(element->getAttribute("_y_min_shift"))
                                     : 0;
              element->setAttribute("_y_min_shift",
                                    old_shift + static_cast<double>(element->getAttribute(attr)) - std::stod(value));
              element->setAttribute("_update_required", true);
            }
          else if (attr == "viewport_y_max")
            {
              double old_shift = (element->hasAttribute("_y_max_shift"))
                                     ? static_cast<double>(element->getAttribute("_y_max_shift"))
                                     : 0;
              element->setAttribute("_y_max_shift",
                                    old_shift + static_cast<double>(element->getAttribute(attr)) - std::stod(value));
              element->setAttribute("_update_required", true);
            }
          else if (element->localName() == "tick_group")
            {
              auto val = static_cast<double>(element->getAttribute("value"));
              auto map_idx = static_cast<int>(element->parentElement()->getAttribute("_axis_id"));
              if (tick_modification_map[map_idx][val].count(attr) > 0)
                {
                  tick_modification_map[map_idx][val][attr] = element->getAttribute(attr);
                }
              else
                {
                  tick_modification_map[map_idx][val].emplace(attr, element->getAttribute(attr));
                }
            }
          else if (element->localName() == "text" && element->parentElement()->localName() == "tick_group")
            {
              auto val = static_cast<double>(element->parentElement()->getAttribute("value"));
              auto map_idx = static_cast<int>(element->parentElement()->parentElement()->getAttribute("_axis_id"));
              if (tick_modification_map[map_idx][val].count(attr) > 0)
                {
                  tick_modification_map[map_idx][val][attr] = element->getAttribute(attr);
                }
              else
                {
                  tick_modification_map[map_idx][val].emplace(attr, element->getAttribute(attr));
                }
            }
          else if (attr == "tick_size")
            {
              element->setAttribute("_tick_size_set_by_user", element->getAttribute(attr));
            }
        }
      global_root->setAttribute("_modified", true);

      automatic_update = true;
    }
}

void renderCaller()
{
  if (global_root && static_cast<int>(global_root->getAttribute("_modified")) && automatic_update)
    {
      global_render->process_tree();
    }
}

void GRM::Render::setActiveFigure(const std::shared_ptr<GRM::Element> &element)
{
  auto result = this->firstChildElement()->querySelectorsAll("[active=1]");
  for (auto &child : result)
    {
      child->setAttribute("active", 0);
    }
  element->setAttribute("active", 1);
}

std::shared_ptr<GRM::Element> GRM::Render::getActiveFigure()
{
  return active_figure;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ modify context ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

void updateContextAttribute(const std::shared_ptr<GRM::Element> &element, const std::string &attr,
                            const GRM::Value &old_value)
{
  // when the attribute is a context the counter for the attribute value inside the context must be modified
  if (valid_context_keys.find(attr) != valid_context_keys.end())
    {
      auto new_value = element->getAttribute(attr);
      if (new_value.isString())
        {
          auto context = global_render->getContext();
          (*context)[attr].use_context_key(static_cast<std::string>(new_value), static_cast<std::string>(old_value));
        }
    }
}

void deleteContextAttribute(const std::shared_ptr<GRM::Element> &element)
{
  auto elem_attribs = element->getAttributeNames();
  std::vector<std::string> attr_inter;
  std::vector<std::string> elem_attribs_vec(elem_attribs.begin(), elem_attribs.end());
  std::vector<std::string> valid_keys_copy(valid_context_keys.begin(), valid_context_keys.end());

  std::sort(elem_attribs_vec.begin(), elem_attribs_vec.end());
  std::sort(valid_keys_copy.begin(), valid_keys_copy.end());
  std::set_intersection(elem_attribs_vec.begin(), elem_attribs_vec.end(), valid_keys_copy.begin(),
                        valid_keys_copy.end(), std::back_inserter(attr_inter));

  auto context = global_render->getContext();
  for (const auto &attr : attr_inter)
    {
      auto value = element->getAttribute(attr);
      if (value.isString())
        {
          (*context)[attr].decrement_key(static_cast<std::string>(value));
        }
    }
}
