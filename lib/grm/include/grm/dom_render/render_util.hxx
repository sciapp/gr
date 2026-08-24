#ifndef GR_RENDER_UTIL_HXX
#define GR_RENDER_UTIL_HXX

#ifdef _WIN32
/*
 * Headers on Windows can define `min` and `max` as macros which causes
 * problem when using `std::min` and `std::max`
 * -> Define `NOMINMAX` to prevent the definition of these macros
 */
#define NOMINMAX
#endif

#include <set>
#include <climits>

#include <grm/dom_render/graphics_tree/element.hxx>
#include <grm/dom_render/graphics_tree/document.hxx>
#include <grm/dom_render/context.hxx>
#include <grm/utilcpp_int.hxx>
#include <grm/util.h>
#include "gks.h"
#include "gr.h"
extern "C" {
#include "grm/datatype/string_map_int.h"
}


/* ========================= macros ================================================================================= */

/* ------------------------- plot ----------------------------------------------------------------------------------- */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plot arguments ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#define PLOT_DEFAULT_CLEAR 1
#define PLOT_DEFAULT_UPDATE 1
#define PLOT_DEFAULT_LOCATION 1
#define PLOT_DEFAULT_VIEWPORT_NORMALIZED_MIN_X 0.0
#define PLOT_DEFAULT_VIEWPORT_NORMALIZED_MAX_X 1.0
#define PLOT_DEFAULT_VIEWPORT_NORMALIZED_MIN_Y 0.0
#define PLOT_DEFAULT_VIEWPORT_NORMALIZED_MAX_Y 1.0
#define PLOT_DEFAULT_ROTATION 40.0
#define PLOT_DEFAULT_TILT 60.0
#define PLOT_DEFAULT_KEEP_ASPECT_RATIO 1
#define PLOT_DEFAULT_KEEP_WINDOW 0
#define PLOT_DEFAULT_TRICONT_LEVELS 20
#define PLOT_DEFAULT_CONTOUR_LEVELS 20
#define PLOT_DEFAULT_USE_GR3 1
#define SERIES_DEFAULT_SPEC ""
#define PLOT_DEFAULT_STEP_WHERE "mid"
#define PLOT_DEFAULT_HEXBIN_NBINS 40
#define PLOT_DEFAULT_VOLUME_ALGORITHM GR_VOLUME_EMISSION
#define PLOT_DEFAULT_ADJUST_XLIM 1
#define PLOT_DEFAULT_ADJUST_YLIM 1
#define PLOT_DEFAULT_ADJUST_ZLIM 1
#define PLOT_DEFAULT_XLOG 0
#define PLOT_DEFAULT_YLOG 0
#define PLOT_DEFAULT_ZLOG 0
#define PLOT_DEFAULT_RESAMPLE_METHOD GKS_K_RESAMPLE_DEFAULT
#define PLOT_DEFAULT_COLORMAP 44                                 /* VIRIDIS */
#define PLOT_DEFAULT_FONT 232                                    /* CMUSerif-Math */
#define PLOT_DEFAULT_FONT_PRECISION GKS_K_TEXT_PRECISION_OUTLINE /* hardware font rendering */
#define PLOT_DEFAULT_MARGINAL_INDEX (-1)
#define PLOT_DEFAULT_MARGINAL_KIND "all"
#define PLOT_DEFAULT_MARGINAL_ALGORITHM "sum"
#define PLOT_DEFAULT_XFLIP 0
#define PLOT_DEFAULT_YFLIP 0
#define PLOT_DEFAULT_ZFLIP 0
#define PLOT_POLAR_AXES_TEXT_BUFFER 40
#define PLOT_CONTOUR_GRIDIT_N 200
#define PLOT_WIREFRAME_GRIDIT_N 50
#define PLOT_SURFACE_GRIDIT_N 200
#define PLOT_DEFAULT_ORIENTATION "horizontal"
#define PLOT_DEFAULT_CONTOUR_MAJOR_H 1000
#define PLOT_DEFAULT_CONTOURF_MAJOR_H 0
#define PLOT_DEFAULT_ORG_POS "low"
#define PLOT_DEFAULT_XGRID 1
#define PLOT_DEFAULT_YGRID 1
#define PLOT_DEFAULT_ZGRID 1
#define PLOT_DEFAULT_SPACE_3D_FOV 30.0
#define PLOT_DEFAULT_SPACE_3D_DISTANCE 0.0
#define PLOT_DEFAULT_COLORBAR_WIDTH 0.03
#define PLOT_DEFAULT_COLORBAR_CHAR_HEIGHT 0.016
#define PLOT_DEFAULT_COLORBAR_OFFSET 0.02
#define PLOT_3D_COLORBAR_OFFSET 0.05
#define PLOT_POLAR_COLORBAR_OFFSET 0.025
#define PLOT_DEFAULT_COLORBAR_TICK_SIZE 0.005
#define PLOT_DEFAULT_SIDEREGION_WIDTH 0.1
#define PLOT_DEFAULT_SIDEREGION_OFFSET 0.02
#define PLOT_DEFAULT_SIDEREGION_LOCATION "right"
#define PLOT_3D_CHAR_HEIGHT 0.024
#define PLOT_2D_CHAR_HEIGHT 0.018
#define PLOT_POLAR_CHAR_HEIGHT 0.018
#define PLOT_DEFAULT_AXES_TICK_SIZE 0.0075
#define DEFAULT_ASPECT_RATIO_FOR_SCALING (4.0 / 3.0)
#define PLOT_DEFAULT_ONLY_SQUARE_ASPECT_RATIO 0
#define MIRRORED_AXIS_DEFAULT 1
#define SCIENTIFIC_FORMAT_OPTION 2
#define PLOT_DEFAULT_MODEL 0
#define ERRORBAR_DEFAULT_STYLE 0
#define PLOT_DEFAULT_ADDITIONAL_AXIS_WIDTH 0.03
#define CENTRAL_REGION_X_MIN_VP_AXIS_MARGIN 0.075
#define CENTRAL_REGION_X_MAX_VP_AXIS_MARGIN 0.05
#define CENTRAL_REGION_Y_MIN_VP_AXIS_MARGIN 0.075
#define CENTRAL_REGION_Y_MAX_VP_AXIS_MARGIN 0.025
#define CENTRAL_REGION_X_MIN_VP_AXIS_MARGIN_3D 0.035
#define CENTRAL_REGION_X_MAX_VP_AXIS_MARGIN_3D 0.025
#define CENTRAL_REGION_Y_MIN_VP_AXIS_MARGIN_3D 0.05
#define CENTRAL_REGION_Y_MAX_VP_AXIS_MARGIN_3D 0.025
#define CENTRAL_REGION_VP_AXIS_MARGIN_POLAR_BBOX 0.05
#define CENTRAL_REGION_VP_AXIS_MARGIN_POLAR 0.025
#define CENTRAL_REGION_VP_AXIS_MARGIN_PIE_BBOX 0.1
#define CENTRAL_REGION_VP_AXIS_MARGIN_PIE 0.05
#define TIME_AXIS_DEFAULT_MAJOR_COUNT 2

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ util ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#define PLOT_CUSTOM_COLOR_INDEX 979
#define UNDEF -1

#define VAN_DER_WAAL_RADIUS 0
#define ATOMIC_RADIUS 1
#define IONIC_RADIUS 2
#define COVALENT_RADIUS 3
#define CRYSTAL_RADIUS 4
#define JMOL_COLOR_SCHEME 0
#define CPK_COLOR_SCHEME 1
#define NATURAL_COLOR_SCHEME 2
#define NO_COLOR_SCHEME 3


/* =========================== enums ================================================================================ */

enum class DelValues
{
  UPDATE_WITHOUT_DEFAULT = 0,
  UPDATE_WITH_DEFAULT = 1,
  RECREATE_OWN_CHILDREN = 2,
  RECREATE_ALL_CHILDREN = 3
};

typedef enum
{
  GR_COLOR_RESET = 0,
  GR_COLOR_LINE = GR_SPEC_LINE,
  GR_COLOR_MARKER = GR_SPEC_MARKER,
  GR_COLOR_FILL = GR_SPEC_COLOR,
  GR_COLOR_TEXT = 1 << 3,
  GR_COLOR_BORDER = 1 << 4
} GRColorType;


/* =========================== sets ================================================================================= */

inline std::set<std::string> kinds_3d = {
    "wireframe", "surface", "line3", "scatter3", "trisurface", "volume", "isosurface", "molecule",
};
inline std::set<std::string> polar_kinds = {
    "nonuniform_polar_heatmap", "polar_heatmap", "polar_histogram", "polar_line", "polar_scatter",
};

inline std::set<std::string> kinds_classic_2d = {"barplot", "contour", "contourf", "heatmap", "hexbin", "histogram",
                                                 "line",    "quiver",  "scatter",  "shade",   "stairs", "stem"};

inline std::set<std::string> drawable_types = {
    "angle_line",
    "arc_grid_line",
    "axes_3d",
    "cell_array",
    "draw_arc",
    "draw_graphics",
    "draw_image",
    "draw_rect",
    "fill_arc",
    "fill_area",
    "fill_rect",
    "grid_3d",
    "grid_line",
    "isosurface_render",
    "layout_grid",
    "layout_grid_element",
    "legend",
    "nonuniform_cell_array",
    "nonuniform_polar_cell_array",
    "polar_cell_array",
    "polyline",
    "polyline_3d",
    "polymarker",
    "polymarker_3d",
    "text",
    "tick",
    "titles_3d",
};

inline std::set<std::string> drawable_kinds = {
    "contour", "contourf",   "hexbin",     "isosurface", "quiver",   "shade",
    "surface", "tricontour", "trisurface", "volume",     "molecule",
};

//! This vector is used for storing element types which children get processed. Other types' children will be ignored
inline std::set<std::string> parent_types = {
    "angle_line",
    "arc_grid_line",
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
    "layout_grid",
    "layout_grid_element",
    "legend",
    "overlay",
    "overlay_element",
    "pie_segment",
    "plot",
    "polar_bar",
    "marginal_heatmap_plot",
    "radial_axes",
    "root",
    "series_barplot",
    "series_contour",
    "series_contourf",
    "series_heatmap",
    "series_hexbin",
    "series_histogram",
    "series_imshow",
    "series_isosurface",
    "series_line",
    "series_molecule",
    "series_nonuniform_heatmap",
    "series_nonuniform_polar_heatmap",
    "series_pie",
    "series_line3",
    "series_polar_heatmap",
    "series_polar_histogram",
    "series_polar_line",
    "series_polar_scatter",
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
    "theta_axes",
    "tick_group",
};

extern "C" {
inline StringMapEntry kind_to_fmt[] = {
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
    {"isosurface", "z"},
    {"imshow", "z"},
    {"nonuniform_heatmap", "xyzc"},
    {"polar_histogram", "theta"},
    {"pie", "x"},
    {"volume", "z"},
    {"marginal_heatmap", "xyzc"},
    {"polar_heatmap", "thetarzc"},
    {"nonuniform_polar_heatmap", "thetarzc"},
    {"polar_scatter", "thetars"},
    {"molecule", "xyz"},
};
}

inline int plot_scatter_markertypes[] = {
    GKS_K_MARKERTYPE_SOLID_CIRCLE,   GKS_K_MARKERTYPE_SOLID_TRI_UP, GKS_K_MARKERTYPE_SOLID_TRI_DOWN,
    GKS_K_MARKERTYPE_SOLID_SQUARE,   GKS_K_MARKERTYPE_SOLID_BOWTIE, GKS_K_MARKERTYPE_SOLID_HGLASS,
    GKS_K_MARKERTYPE_SOLID_DIAMOND,  GKS_K_MARKERTYPE_SOLID_STAR,   GKS_K_MARKERTYPE_SOLID_TRI_RIGHT,
    GKS_K_MARKERTYPE_SOLID_TRI_LEFT, GKS_K_MARKERTYPE_SOLID_PLUS,   GKS_K_MARKERTYPE_PENTAGON,
    GKS_K_MARKERTYPE_HEXAGON,        GKS_K_MARKERTYPE_HEPTAGON,     GKS_K_MARKERTYPE_OCTAGON,
    GKS_K_MARKERTYPE_STAR_4,         GKS_K_MARKERTYPE_STAR_5,       GKS_K_MARKERTYPE_STAR_6,
    GKS_K_MARKERTYPE_STAR_7,         GKS_K_MARKERTYPE_STAR_8,       GKS_K_MARKERTYPE_VLINE,
    GKS_K_MARKERTYPE_HLINE,          GKS_K_MARKERTYPE_OMARK,        INT_MAX};

inline std::unordered_map<std::string, double> elem_symbol_to_atomic_radius = {
    {"H", 0.53},   {"He", 0.31},  {"Li", 1.67},  {"Be", 1.12},  {"B", 0.87},   {"C", 0.67},   {"N", 0.56},
    {"O", 0.48},   {"F", 0.42},   {"Ne", 0.38},  {"Na", 1.90},  {"Mg", 1.45},  {"Al", 1.18},  {"Si", 1.11},
    {"P", 0.98},   {"S", 0.88},   {"Cl", 0.79},  {"Ar", 0.71},  {"K", 2.43},   {"Ca", 1.94},  {"Sc", 1.84},
    {"Ti", 1.76},  {"V", 1.71},   {"Cr", 1.66},  {"Mn", 1.61},  {"Fe", 1.56},  {"Co", 1.52},  {"Ni", 1.49},
    {"Cu", 1.45},  {"Zn", 1.42},  {"Ga", 1.36},  {"Ge", 1.25},  {"As", 1.14},  {"Se", 1.03},  {"Br", 0.94},
    {"Kr", 0.88},  {"Rb", 2.65},  {"Sr", 2.19},  {"Y", 2.12},   {"Zr", 2.06},  {"Nb", 1.98},  {"Mo", 1.90},
    {"Tc", 1.83},  {"Ru", 1.78},  {"Rh", 1.73},  {"Pd", 1.69},  {"Ag", 1.65},  {"Cd", 1.61},  {"In", 1.56},
    {"Sn", 1.45},  {"Sb", 1.33},  {"Te", 1.23},  {"I", 1.15},   {"Xe", 1.08},  {"Cs", 2.98},  {"Ba", 2.53},
    {"La", 1.95},  {"Ce", 1.85},  {"Pr", 2.47},  {"Nd", 2.06},  {"Pm", 2.05},  {"Sm", 2.38},  {"Eu", 2.31},
    {"Gd", 2.33},  {"Tb", 2.25},  {"Dy", 2.28},  {"Ho", 2.26},  {"Er", 2.26},  {"Tm", 2.22},  {"Yb", 2.22},
    {"Lu", 2.17},  {"Hf", 2.08},  {"Ta", 2.00},  {"W", 1.93},   {"Re", 1.88},  {"Os", 1.85},  {"Ir", 1.80},
    {"Pt", 1.77},  {"Au", 1.74},  {"Hg", 1.71},  {"Tl", 1.56},  {"Pb", 1.54},  {"Bi", 1.43},  {"Po", 1.35},
    {"At", 1.27},  {"Rn", 1.20},  {"Fr", UNDEF}, {"Ra", UNDEF}, {"Ac", 1.95},  {"Th", 1.80},  {"Pa", 1.80},
    {"U", 1.75},   {"Np", 1.75},  {"Pu", 1.75},  {"Am", 1.75},  {"Cm", 1.76},  {"Bk", UNDEF}, {"Cf", UNDEF},
    {"Es", UNDEF}, {"Fm", UNDEF}, {"Md", UNDEF}, {"No", UNDEF}, {"Lr", UNDEF}, {"Rf", UNDEF}, {"Db", UNDEF},
    {"Sg", UNDEF}, {"Bh", UNDEF}, {"Hs", UNDEF}, {"Mt", UNDEF}, {"Ds", UNDEF}, {"Rg", UNDEF}, {"Cn", UNDEF},
    {"Nh", UNDEF}, {"Fl", UNDEF}, {"Mc", UNDEF}, {"Lv", UNDEF}, {"Ts", UNDEF}, {"Og", UNDEF},
};
inline std::unordered_map<std::string, double> elem_symbol_to_ionic_radius = {
    {"H", 0.25},   {"He", 0.31},  {"Li", 1.45},  {"Be", 1.05},  {"B", 0.85},   {"C", 0.70},   {"N", 0.65},
    {"O", 0.60},   {"F", 0.50},   {"Ne", 0.38},  {"Na", 1.80},  {"Mg", 1.50},  {"Al", 1.25},  {"Si", 1.10},
    {"P", 1.00},   {"S", 1.00},   {"Cl", 1.00},  {"Ar", 0.71},  {"K", 2.20},   {"Ca", 1.80},  {"Sc", 1.60},
    {"Ti", 1.40},  {"V", 1.35},   {"Cr", 1.40},  {"Mn", 1.40},  {"Fe", 1.40},  {"Co", 1.35},  {"Ni", 1.35},
    {"Cu", 1.35},  {"Zn", 1.35},  {"Ga", 1.30},  {"Ge", 1.25},  {"As", 1.15},  {"Se", 1.15},  {"Br", 1.15},
    {"Kr", 0.88},  {"Rb", 2.35},  {"Sr", 2.00},  {"Y", 1.85},   {"Zr", 1.55},  {"Nb", 1.45},  {"Mo", 1.45},
    {"Tc", 1.35},  {"Ru", 1.30},  {"Rh", 1.35},  {"Pd", 1.40},  {"Ag", 1.60},  {"Cd", 1.55},  {"In", 1.55},
    {"Sn", 1.45},  {"Sb", 1.45},  {"Te", 1.40},  {"I", 1.40},   {"Xe", 1.08},  {"Cs", 2.60},  {"Ba", 2.15},
    {"La", 1.95},  {"Ce", 1.85},  {"Pr", 1.85},  {"Nd", 1.85},  {"Pm", 1.85},  {"Sm", 1.85},  {"Eu", 1.85},
    {"Gd", 1.80},  {"Tb", 1.75},  {"Dy", 1.75},  {"Ho", 1.75},  {"Er", 1.75},  {"Tm", 1.75},  {"Yb", 1.75},
    {"Lu", 1.75},  {"Hf", 1.55},  {"Ta", 1.45},  {"W", 1.35},   {"Re", 1.35},  {"Os", 1.30},  {"Ir", 1.35},
    {"Pt", 1.35},  {"Au", 1.35},  {"Hg", 1.50},  {"Tl", 1.90},  {"Pb", 1.80},  {"Bi", 1.60},  {"Po", 1.90},
    {"At", 1.27},  {"Rn", 1.20},  {"Fr", UNDEF}, {"Ra", 2.15},  {"Ac", 1.95},  {"Th", 1.80},  {"Pa", 1.80},
    {"U", 1.75},   {"Np", 1.75},  {"Pu", 1.75},  {"Am", 1.75},  {"Cm", UNDEF}, {"Bk", UNDEF}, {"Cf", UNDEF},
    {"Es", UNDEF}, {"Fm", UNDEF}, {"Md", UNDEF}, {"No", UNDEF}, {"Lr", UNDEF}, {"Rf", UNDEF}, {"Db", UNDEF},
    {"Sg", UNDEF}, {"Bh", UNDEF}, {"Hs", UNDEF}, {"Mt", UNDEF}, {"Ds", UNDEF}, {"Rg", UNDEF}, {"Cn", UNDEF},
    {"Nh", UNDEF}, {"Fl", UNDEF}, {"Mc", UNDEF}, {"Lv", UNDEF}, {"Ts", UNDEF}, {"Og", UNDEF},
};
inline std::unordered_map<std::string, double> elem_symbol_to_covalent_radius = {
    {"H", 0.37},   {"He", 0.32},  {"Li", 1.34},  {"Be", 0.90},  {"B", 0.82},   {"C", 0.77},   {"N", 0.75},
    {"O", 0.73},   {"F", 0.71},   {"Ne", 0.69},  {"Na", 1.54},  {"Mg", 1.30},  {"Al", 1.18},  {"Si", 1.11},
    {"P", 1.06},   {"S", 1.02},   {"Cl", 0.99},  {"Ar", 0.97},  {"K", 1.96},   {"Ca", 1.74},  {"Sc", 1.44},
    {"Ti", 1.36},  {"V", 1.25},   {"Cr", 1.27},  {"Mn", 1.39},  {"Fe", 1.25},  {"Co", 1.26},  {"Ni", 1.21},
    {"Cu", 1.38},  {"Zn", 1.31},  {"Ga", 1.26},  {"Ge", 1.22},  {"As", 1.19},  {"Se", 1.16},  {"Br", 1.14},
    {"Kr", 1.10},  {"Rb", 2.11},  {"Sr", 1.92},  {"Y", 1.62},   {"Zr", 1.48},  {"Nb", 1.37},  {"Mo", 1.45},
    {"Tc", 1.56},  {"Ru", 1.26},  {"Rh", 1.35},  {"Pd", 1.31},  {"Ag", 1.53},  {"Cd", 1.48},  {"In", 1.44},
    {"Sn", 1.41},  {"Sb", 1.38},  {"Te", 1.35},  {"I", 1.33},   {"Xe", 1.30},  {"Cs", 2.25},  {"Ba", 1.98},
    {"La", 1.69},  {"Ce", UNDEF}, {"Pr", UNDEF}, {"Nd", UNDEF}, {"Pm", UNDEF}, {"Sm", UNDEF}, {"Eu", UNDEF},
    {"Gd", UNDEF}, {"Tb", UNDEF}, {"Dy", UNDEF}, {"Ho", UNDEF}, {"Er", UNDEF}, {"Tm", UNDEF}, {"Yb", UNDEF},
    {"Lu", 1.60},  {"Hf", 1.50},  {"Ta", 1.38},  {"W", 1.46},   {"Re", 1.59},  {"Os", 1.28},  {"Ir", 1.37},
    {"Pt", 1.28},  {"Au", 1.44},  {"Hg", 1.49},  {"Tl", 1.48},  {"Pb", 1.47},  {"Bi", 1.46},  {"Po", UNDEF},
    {"At", UNDEF}, {"Rn", 1.45},  {"Fr", UNDEF}, {"Ra", UNDEF}, {"Ac", UNDEF}, {"Th", UNDEF}, {"Pa", UNDEF},
    {"U", UNDEF},  {"Np", UNDEF}, {"Pu", UNDEF}, {"Am", UNDEF}, {"Cm", UNDEF}, {"Bk", UNDEF}, {"Cf", UNDEF},
    {"Es", UNDEF}, {"Fm", UNDEF}, {"Md", UNDEF}, {"No", UNDEF}, {"Lr", UNDEF}, {"Rf", UNDEF}, {"Db", UNDEF},
    {"Sg", UNDEF}, {"Bh", UNDEF}, {"Hs", UNDEF}, {"Mt", UNDEF}, {"Ds", UNDEF}, {"Rg", UNDEF}, {"Cn", UNDEF},
    {"Nh", UNDEF}, {"Fl", UNDEF}, {"Mc", UNDEF}, {"Lv", UNDEF}, {"Ts", UNDEF}, {"Og", UNDEF},
};
inline std::unordered_map<std::string, double> elem_symbol_to_van_der_waals_radius = {
    {"H", 1.20},   {"He", 1.40},  {"Li", 1.82},  {"Be", 1.53},  {"B", 1.92},   {"C", 1.70},   {"N", 1.55},
    {"O", 1.52},   {"F", 1.47},   {"Ne", 1.54},  {"Na", 2.27},  {"Mg", 1.73},  {"Al", 1.84},  {"Si", 2.10},
    {"P", 1.80},   {"S", 1.80},   {"Cl", 1.75},  {"Ar", 1.88},  {"K", 2.75},   {"Ca", 2.31},  {"Sc", 2.11},
    {"Ti", 1.87},  {"V", 1.79},   {"Cr", 1.89},  {"Mn", 1.97},  {"Fe", 1.94},  {"Co", 1.92},  {"Ni", 1.63},
    {"Cu", 1.40},  {"Zn", 1.39},  {"Ga", 1.87},  {"Ge", 2.11},  {"As", 1.85},  {"Se", 1.90},  {"Br", 1.85},
    {"Kr", 2.02},  {"Rb", 3.03},  {"Sr", 2.49},  {"Y", 2.19},   {"Zr", 1.86},  {"Nb", 2.07},  {"Mo", 2.09},
    {"Tc", 2.09},  {"Ru", 2.07},  {"Rh", 1.95},  {"Pd", 2.02},  {"Ag", 1.72},  {"Cd", 1.58},  {"In", 1.93},
    {"Sn", 2.17},  {"Sb", 2.06},  {"Te", 2.06},  {"I", 1.98},   {"Xe", 2.16},  {"Cs", 3.43},  {"Ba", 2.68},
    {"La", 2.40},  {"Ce", 2.35},  {"Pr", 2.39},  {"Nd", 2.39},  {"Pm", 2.36},  {"Sm", 2.29},  {"Eu", 2.33},
    {"Gd", 2.37},  {"Tb", 2.21},  {"Dy", 2.29},  {"Ho", 2.16},  {"Er", 2.35},  {"Tm", 2.27},  {"Yb", 2.42},
    {"Lu", 2.21},  {"Hf", 2.12},  {"Ta", 2.17},  {"W", 2.10},   {"Re", 2.17},  {"Os", 2.16},  {"Ir", 2.02},
    {"Pt", 2.09},  {"Au", 1.66},  {"Hg", 2.09},  {"Tl", 1.96},  {"Pb", 2.02},  {"Bi", 2.07},  {"Po", 1.97},
    {"At", 2.02},  {"Rn", 2.20},  {"Fr", 3.48},  {"Ra", 2.83},  {"Ac", 2.60},  {"Th", 2.37},  {"Pa", 2.43},
    {"U", 2.40},   {"Np", 2.21},  {"Pu", 2.43},  {"Am", 2.44},  {"Cm", 2.45},  {"Bk", 2.44},  {"Cf", 2.45},
    {"Es", 2.45},  {"Fm", UNDEF}, {"Md", UNDEF}, {"No", UNDEF}, {"Lr", UNDEF}, {"Rf", UNDEF}, {"Db", UNDEF},
    {"Sg", UNDEF}, {"Bh", UNDEF}, {"Hs", UNDEF}, {"Mt", UNDEF}, {"Ds", UNDEF}, {"Rg", UNDEF}, {"Cn", UNDEF},
    {"Nh", UNDEF}, {"Fl", UNDEF}, {"Mc", UNDEF}, {"Lv", UNDEF}, {"Ts", UNDEF}, {"Og", UNDEF},
};
inline std::unordered_map<std::string, double> elem_symbol_to_crystal_radius = {
    {"H", 0.10},   {"He", UNDEF}, {"Li", 0.90},  {"Be", 0.41},  {"B", 0.25},   {"C", 0.29},   {"N", 0.30},
    {"O", 1.21},   {"F", 1.19},   {"Ne", UNDEF}, {"Na", 1.16},  {"Mg", 0.86},  {"Al", 0.53},  {"Si", 0.40},
    {"P", 0.31},   {"S", 0.43},   {"Cl", 1.67},  {"Ar", UNDEF}, {"K", 1.52},   {"Ca", 1.14},  {"Sc", 0.89},
    {"Ti", 0.75},  {"V", 0.68},   {"Cr", 0.76},  {"Mn", 0.81},  {"Fe", 0.69},  {"Co", 0.54},  {"Ni", 0.70},
    {"Cu", 0.71},  {"Zn", 0.74},  {"Ga", 0.76},  {"Ge", 0.53},  {"As", 0.72},  {"Se", 0.56},  {"Br", 1.82},
    {"Kr", UNDEF}, {"Rb", 1.66},  {"Sr", 1.32},  {"Y", 1.04},   {"Zr", 0.86},  {"Nb", 0.78},  {"Mo", 0.79},
    {"Tc", 0.79},  {"Ru", 0.82},  {"Rh", 0.81},  {"Pd", 0.78},  {"Ag", 1.29},  {"Cd", 0.92},  {"In", 0.94},
    {"Sn", 0.69},  {"Sb", 0.90},  {"Te", 1.11},  {"I", 2.06},   {"Xe", 0.62},  {"Cs", 1.81},  {"Ba", 1.49},
    {"La", 1.36},  {"Ce", 1.15},  {"Pr", 1.32},  {"Nd", 1.30},  {"Pm", 1.28},  {"Sm", 1.10},  {"Eu", 1.31},
    {"Gd", 1.08},  {"Tb", 1.18},  {"Dy", 1.05},  {"Ho", 1.04},  {"Er", 1.03},  {"Tm", 1.02},  {"Yb", 1.13},
    {"Lu", 1.00},  {"Hf", 0.85},  {"Ta", 0.78},  {"W", 0.74},   {"Re", 0.77},  {"Os", 0.77},  {"Ir", 0.77},
    {"Pt", 0.74},  {"Au", 1.51},  {"Hg", 0.83},  {"Tl", 1.03},  {"Pb", 1.49},  {"Bi", 1.17},  {"Po", 1.08},
    {"At", 0.76},  {"Rn", UNDEF}, {"Fr", 1.94},  {"Ra", 1.62},  {"Ac", 1.26},  {"Th", 1.19},  {"Pa", 1.09},
    {"U", 0.87},   {"Np", UNDEF}, {"Pu", 1.00},  {"Am", 1.12},  {"Cm", 1.11},  {"Bk", UNDEF}, {"Cf", UNDEF},
    {"Es", UNDEF}, {"Fm", UNDEF}, {"Md", UNDEF}, {"No", UNDEF}, {"Lr", UNDEF}, {"Rf", UNDEF}, {"Db", UNDEF},
    {"Sg", UNDEF}, {"Bh", UNDEF}, {"Hs", UNDEF}, {"Mt", UNDEF}, {"Ds", UNDEF}, {"Rg", UNDEF}, {"Cn", UNDEF},
    {"Nh", UNDEF}, {"Fl", UNDEF}, {"Mc", UNDEF}, {"Lv", UNDEF}, {"Ts", UNDEF}, {"Og", UNDEF},
};
inline std::unordered_map<std::string, std::vector<int>> elem_symbol_to_cpk_color = {
    {"H", {255, 255, 255}},  {"He", {0, 255, 255}},   {"Li", {160, 32, 240}},  {"Be", {0, 100, 0}},
    {"B", {245, 245, 220}},  {"C", {0, 0, 0}},        {"N", {0, 0, 255}},      {"O", {255, 0, 0}},
    {"F", {0, 255, 0}},      {"Ne", {0, 255, 255}},   {"Na", {160, 32, 240}},  {"Mg", {0, 100, 0}},
    {"Al", {255, 192, 203}}, {"Si", {255, 192, 203}}, {"P", {255, 165, 0}},    {"S", {255, 255, 0}},
    {"Cl", {0, 255, 0}},     {"Ar", {0, 255, 255}},   {"K", {160, 32, 240}},   {"Ca", {0, 100, 0}},
    {"Sc", {245, 245, 220}}, {"Ti", {128, 128, 128}}, {"V", {245, 245, 220}},  {"Cr", {245, 245, 220}},
    {"Mn", {245, 245, 220}}, {"Fe", {255, 140, 0}},   {"Co", {245, 245, 220}}, {"Ni", {245, 245, 220}},
    {"Cu", {245, 245, 220}}, {"Zn", {245, 245, 220}}, {"Ga", {255, 192, 203}}, {"Ge", {255, 192, 203}},
    {"As", {255, 192, 203}}, {"Se", {255, 192, 203}}, {"Br", {139, 0, 0}},     {"Kr", {0, 255, 255}},
    {"Rb", {160, 32, 240}},  {"Sr", {0, 100, 0}},     {"Y", {245, 245, 220}},  {"Zr", {245, 245, 220}},
    {"Nb", {245, 245, 220}}, {"Mo", {245, 245, 220}}, {"Tc", {245, 245, 220}}, {"Ru", {245, 245, 220}},
    {"Rh", {245, 245, 220}}, {"Pd", {245, 245, 220}}, {"Ag", {245, 245, 220}}, {"Cd", {245, 245, 220}},
    {"In", {255, 192, 203}}, {"Sn", {255, 192, 203}}, {"Sb", {255, 192, 203}}, {"Te", {255, 192, 203}},
    {"I", {48, 25, 52}},     {"Xe", {0, 255, 255}},   {"Cs", {160, 32, 240}},  {"Ba", {0, 100, 0}},
    {"La", {245, 245, 220}}, {"Ce", {255, 192, 203}}, {"Pr", {255, 192, 203}}, {"Nd", {255, 192, 203}},
    {"Pm", {255, 192, 203}}, {"Sm", {255, 192, 203}}, {"Eu", {255, 192, 203}}, {"Gd", {255, 192, 203}},
    {"Tb", {255, 192, 203}}, {"Dy", {255, 192, 203}}, {"Ho", {255, 192, 203}}, {"Er", {255, 192, 203}},
    {"Tm", {255, 192, 203}}, {"Yb", {255, 192, 203}}, {"Lu", {255, 192, 203}}, {"Hf", {245, 245, 220}},
    {"Ta", {245, 245, 220}}, {"W", {245, 245, 220}},  {"Re", {245, 245, 220}}, {"Os", {245, 245, 220}},
    {"Ir", {245, 245, 220}}, {"Pt", {245, 245, 220}}, {"Au", {245, 245, 220}}, {"Hg", {245, 245, 220}},
    {"Tl", {255, 192, 203}}, {"Pb", {255, 192, 203}}, {"Bi", {255, 192, 203}}, {"Po", {255, 192, 203}},
    {"At", {255, 192, 203}}, {"Rn", {0, 255, 255}},   {"Fr", {160, 32, 240}},  {"Ra", {0, 100, 0}},
    {"Ac", {245, 245, 220}}, {"Th", {255, 192, 203}}, {"Pa", {255, 192, 203}}, {"U", {255, 192, 203}},
    {"Np", {255, 192, 203}}, {"Pu", {255, 192, 203}}, {"Am", {255, 192, 203}}, {"Cm", {255, 192, 203}},
    {"Bk", {255, 192, 203}}, {"Cf", {255, 192, 203}}, {"Es", {255, 192, 203}}, {"Fm", {255, 192, 203}},
    {"Md", {255, 192, 203}}, {"No", {255, 192, 203}}, {"Lr", {255, 192, 203}}, {"Rf", {245, 245, 220}},
    {"Db", {245, 245, 220}}, {"Sg", {245, 245, 220}}, {"Bh", {245, 245, 220}}, {"Hs", {245, 245, 220}},
    {"Mt", {245, 245, 220}}, {"Ds", {245, 245, 220}}, {"Rg", {245, 245, 220}}, {"Cn", {245, 245, 220}},
    {"Nh", {255, 192, 203}}, {"Fl", {255, 192, 203}}, {"Mc", {255, 192, 203}}, {"Lv", {255, 192, 203}},
    {"Ts", {255, 192, 203}}, {"Og", {255, 192, 203}},
};
inline std::unordered_map<std::string, std::vector<int>> elem_symbol_to_natural_color = {
    {"H", {255, 255, 255}},  {"He", {255, 255, 255}}, {"Li", {220, 225, 230}}, {"Be", {168, 175, 180}},
    {"B", {50, 45, 45}},     {"C", {30, 30, 30}},     {"N", {255, 255, 255}},  {"O", {255, 255, 255}},
    {"F", {245, 245, 180}},  {"Ne", {255, 255, 255}}, {"Na", {220, 225, 230}}, {"Mg", {220, 225, 230}},
    {"Al", {160, 165, 170}}, {"Si", {112, 128, 144}}, {"P", {230, 225, 190}},  {"S", {245, 230, 60}},
    {"Cl", {210, 240, 160}}, {"Ar", {255, 255, 255}}, {"K", {160, 32, 240}},   {"Ca", {220, 225, 230}},
    {"Sc", {220, 225, 230}}, {"Ti", {160, 165, 170}}, {"V", {160, 165, 170}},  {"Cr", {160, 165, 170}},
    {"Mn", {160, 165, 170}}, {"Fe", {160, 165, 170}}, {"Co", {145, 160, 175}}, {"Ni", {212, 210, 195}},
    {"Cu", {184, 115, 51}},  {"Zn", {245, 245, 220}}, {"Ga", {255, 192, 203}}, {"Ge", {160, 165, 170}},
    {"As", {130, 130, 130}}, {"Se", {100, 105, 110}}, {"Br", {139, 35, 35}},   {"Kr", {255, 255, 255}},
    {"Rb", {160, 32, 240}},  {"Sr", {220, 225, 230}}, {"Y", {220, 225, 230}},  {"Zr", {160, 165, 170}},
    {"Nb", {160, 165, 170}}, {"Mo", {160, 165, 170}}, {"Tc", {160, 165, 170}}, {"Ru", {220, 225, 230}},
    {"Rh", {220, 225, 230}}, {"Pd", {220, 225, 230}}, {"Ag", {220, 225, 230}}, {"Cd", {220, 225, 230}},
    {"In", {220, 225, 230}}, {"Sn", {220, 225, 230}}, {"Sb", {160, 165, 170}}, {"Te", {220, 225, 230}},
    {"I", {75, 0, 130}},     {"Xe", {255, 255, 255}}, {"Cs", {225, 210, 160}}, {"Ba", {220, 225, 230}},
    {"La", {220, 225, 230}}, {"Ce", {160, 165, 170}}, {"Pr", {220, 225, 230}}, {"Nd", {220, 225, 230}},
    {"Pm", {220, 225, 230}}, {"Sm", {220, 225, 230}}, {"Eu", {220, 225, 230}}, {"Gd", {220, 225, 230}},
    {"Tb", {220, 225, 230}}, {"Dy", {220, 225, 230}}, {"Ho", {220, 225, 230}}, {"Er", {220, 225, 230}},
    {"Tm", {160, 165, 170}}, {"Yb", {220, 225, 230}}, {"Lu", {220, 225, 230}}, {"Hf", {160, 165, 170}},
    {"Ta", {140, 155, 170}}, {"W", {135, 140, 145}},  {"Re", {160, 165, 170}}, {"Os", {160, 165, 170}},
    {"Ir", {220, 225, 230}}, {"Pt", {220, 225, 230}}, {"Au", {212, 175, 55}},  {"Hg", {220, 225, 230}},
    {"Tl", {220, 225, 230}}, {"Pb", {119, 136, 153}}, {"Bi", {225, 205, 210}}, {"Po", {160, 165, 170}},
    {"At", {40, 40, 40}},    {"Rn", {255, 255, 255}}, {"Fr", {220, 225, 230}}, {"Ra", {220, 225, 230}},
    {"Ac", {220, 225, 230}}, {"Th", {220, 225, 230}}, {"Pa", {220, 225, 230}}, {"U", {160, 165, 170}},
    {"Np", {160, 165, 170}}, {"Pu", {160, 165, 170}}, {"Am", {220, 225, 230}}, {"Cm", {255, 192, 203}},
    {"Bk", {255, 192, 203}}, {"Cf", {255, 192, 203}}, {"Es", {255, 192, 203}}, {"Fm", {200, 200, 200}},
    {"Md", {200, 200, 200}}, {"No", {200, 200, 200}}, {"Lr", {200, 200, 200}}, {"Rf", {200, 200, 200}},
    {"Db", {200, 200, 200}}, {"Sg", {200, 200, 200}}, {"Bh", {200, 200, 200}}, {"Hs", {200, 200, 200}},
    {"Mt", {200, 200, 200}}, {"Ds", {200, 200, 200}}, {"Rg", {200, 200, 200}}, {"Cn", {190, 190, 190}},
    {"Nh", {200, 200, 200}}, {"Fl", {200, 200, 200}}, {"Mc", {200, 200, 200}}, {"Lv", {200, 200, 200}},
    {"Ts", {50, 50, 50}},    {"Og", {240, 240, 240}},
};
inline std::unordered_map<std::string, std::vector<int>> elem_symbol_to_jmol_color = {
    {"H", {255, 255, 255}},  {"He", {217, 255, 255}}, {"Li", {204, 128, 255}}, {"Be", {194, 255, 0}},
    {"B", {255, 181, 181}},  {"C", {144, 144, 144}},  {"N", {48, 80, 248}},    {"O", {255, 13, 13}},
    {"F", {144, 224, 80}},   {"Ne", {179, 227, 245}}, {"Na", {171, 92, 242}},  {"Mg", {138, 255, 0}},
    {"Al", {191, 166, 166}}, {"Si", {240, 200, 160}}, {"P", {255, 128, 0}},    {"S", {255, 255, 48}},
    {"Cl", {31, 240, 31}},   {"Ar", {128, 209, 227}}, {"K", {143, 64, 212}},   {"Ca", {61, 255, 0}},
    {"Sc", {230, 230, 230}}, {"Ti", {191, 194, 199}}, {"V", {166, 166, 171}},  {"Cr", {138, 153, 199}},
    {"Mn", {156, 122, 199}}, {"Fe", {224, 102, 51}},  {"Co", {240, 144, 160}}, {"Ni", {80, 208, 80}},
    {"Cu", {200, 128, 51}},  {"Zn", {125, 128, 176}}, {"Ga", {194, 143, 143}}, {"Ge", {102, 143, 143}},
    {"As", {189, 128, 227}}, {"Se", {255, 161, 0}},   {"Br", {166, 41, 41}},   {"Kr", {92, 184, 209}},
    {"Rb", {112, 46, 176}},  {"Sr", {0, 255, 0}},     {"Y", {148, 255, 255}},  {"Zr", {148, 224, 224}},
    {"Nb", {115, 194, 201}}, {"Mo", {84, 181, 181}},  {"Tc", {59, 158, 158}},  {"Ru", {36, 143, 143}},
    {"Rh", {10, 125, 140}},  {"Pd", {0, 105, 133}},   {"Ag", {192, 192, 192}}, {"Cd", {255, 217, 143}},
    {"In", {166, 117, 115}}, {"Sn", {102, 128, 128}}, {"Sb", {158, 99, 181}},  {"Te", {212, 122, 0}},
    {"I", {148, 0, 148}},    {"Xe", {66, 158, 176}},  {"Cs", {87, 23, 143}},   {"Ba", {0, 201, 0}},
    {"La", {112, 212, 255}}, {"Ce", {255, 255, 199}}, {"Pr", {217, 255, 199}}, {"Nd", {199, 255, 199}},
    {"Pm", {163, 255, 199}}, {"Sm", {143, 255, 199}}, {"Eu", {97, 255, 199}},  {"Gd", {69, 255, 199}},
    {"Tb", {48, 255, 199}},  {"Dy", {31, 255, 199}},  {"Ho", {0, 255, 156}},   {"Er", {0, 230, 117}},
    {"Tm", {0, 212, 82}},    {"Yb", {0, 191, 56}},    {"Lu", {0, 171, 36}},    {"Hf", {77, 194, 255}},
    {"Ta", {77, 166, 255}},  {"W", {33, 148, 214}},   {"Re", {38, 125, 171}},  {"Os", {38, 102, 150}},
    {"Ir", {23, 84, 135}},   {"Pt", {208, 208, 224}}, {"Au", {255, 209, 35}},  {"Hg", {184, 184, 208}},
    {"Tl", {166, 84, 77}},   {"Pb", {87, 89, 97}},    {"Bi", {158, 79, 181}},  {"Po", {171, 92, 0}},
    {"At", {117, 79, 69}},   {"Rn", {66, 130, 150}},  {"Fr", {66, 0, 102}},    {"Ra", {0, 125, 0}},
    {"Ac", {112, 171, 250}}, {"Th", {0, 186, 255}},   {"Pa", {0, 161, 255}},   {"U", {0, 143, 255}},
    {"Np", {0, 128, 255}},   {"Pu", {0, 107, 255}},   {"Am", {84, 92, 242}},   {"Cm", {120, 92, 227}},
    {"Bk", {138, 79, 227}},  {"Cf", {161, 54, 212}},  {"Es", {179, 31, 212}},  {"Fm", {179, 31, 186}},
    {"Md", {179, 13, 166}},  {"No", {189, 13, 135}},  {"Lr", {199, 0, 102}},   {"Rf", {204, 0, 89}},
    {"Db", {209, 0, 79}},    {"Sg", {217, 0, 69}},    {"Bh", {224, 0, 56}},    {"Hs", {230, 0, 46}},
    {"Mt", {235, 0, 38}},    {"Ds", {235, 0, 38}},    {"Rg", {235, 0, 38}},    {"Cn", {235, 0, 38}},
    {"Nh", {235, 0, 38}},    {"Fl", {235, 0, 38}},    {"Mc", {235, 0, 38}},    {"Lv", {235, 0, 38}},
    {"Ts", {235, 0, 38}},    {"Og", {235, 0, 38}},
};

/* ========================= functions ============================================================================== */

namespace GRM
{
void GRM_EXPORT getFigureSize(int *pixel_width, int *pixel_height, double *metric_width, double *metric_height);
void GRM_EXPORT calculateCharHeight(const std::shared_ptr<GRM::Element> &element);
void GRM_EXPORT getPlotParent(std::shared_ptr<GRM::Element> &element);
} // namespace GRM

bool isUniformData(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
double getMaxViewport(const std::shared_ptr<GRM::Element> &element, bool x);
double getMinViewport(const std::shared_ptr<GRM::Element> &element, bool x);
bool applyBoundingBoxId(GRM::Element &new_element, GRM::Element &old_element, bool only_reserve_id = false);
IdPool<int> &idPool();
void setRanges(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Element> &new_series);
void getMajorCount(const std::shared_ptr<GRM::Element> &element, const std::string &kind, int &major_count);
void clearAxisAttributes(const std::shared_ptr<GRM::Element> &axis);
std::tuple<double, int> getColorbarAttributes(const std::string &kind, const std::shared_ptr<GRM::Element> &plot);
double getLightness(int color);
void resetOldBoundingBoxes(const std::shared_ptr<GRM::Element> &element);
bool removeBoundingBoxId(GRM::Element &element);
double transformCoordinate(double value, double v_min, double v_max, double range_min, double range_max,
                           bool log_scale = false);
void transformCoordinatesVector(std::vector<double> &coords, double v_min, double v_max, double range_min,
                                double range_max, bool log_scale = false);
void clearOldChildren(DelValues *del, const std::shared_ptr<GRM::Element> &element);
void legendSize(const std::shared_ptr<GRM::Element> &element, double *w, double *h);
void sidePlotMargin(const std::shared_ptr<GRM::Element> &side_region, double *margin, double inc);
void capSidePlotMarginInNonKeepAspectRatio(const std::shared_ptr<GRM::Element> &side_region, double *margin,
                                           const std::string &kind);
void bboxViewportAdjustmentsForSideRegions(const std::shared_ptr<GRM::Element> &element, std::string location);
std::string getLocalName(const std::shared_ptr<GRM::Element> &element);
bool isDrawable(const std::shared_ptr<GRM::Element> &element);
double getLightnessFromRGB(double r, double g, double b);
void applyMoveTransformation(const std::shared_ptr<GRM::Element> &element);
bool hasHighlightedParent(const std::shared_ptr<GRM::Element> &element);
double autoTick(double min, double max);
std::shared_ptr<GRM::Element> getPlotElement(const std::shared_ptr<GRM::Element> &element);
int setNextColor(const std::string &key, GRColorType color_type, const std::shared_ptr<GRM::Element> &element,
                 const std::shared_ptr<GRM::Context> &context);
void calculateWindowTransformationParameter(const std::shared_ptr<GRM::Element> &plot_parent, double w1_min,
                                            double w1_max, double w2_min, double w2_max, std::string location,
                                            double *a, double *b);
void newWindowForTwinAxis(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Element> &axis_ref,
                          double *new_w_min, double *new_w_max, double old_w_min, double old_w_max);
void getTickSize(const std::shared_ptr<GRM::Element> &element, double &tick_size);
void adjustValueForNonStandardAxis(const std::shared_ptr<GRM::Element> &plot_parent, double *value,
                                   std::string location);
void getAxesInformation(const std::shared_ptr<GRM::Element> &element, const std::string &x_org_pos,
                        const std::string &y_org_pos, double &x_org, double &y_org, int &x_major, int &y_major,
                        double &x_tick, double &y_tick);
void getAxes3dInformation(const std::shared_ptr<GRM::Element> &element, const std::string &x_org_pos,
                          const std::string &y_org_pos, const std::string &z_org_pos, double &x_org, double &y_org,
                          double &z_org, int &x_major, int &y_major, int &z_major, double &x_tick, double &y_tick,
                          double &z_tick);
void markerHelper(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context,
                  const std::string &str);
void lineHelper(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context,
                const std::string &str);
bool getLimitsForColorbar(const std::shared_ptr<GRM::Element> &element, double &c_min, double &c_max);
double findMaxStep(unsigned int n, std::vector<double> x);
void extendErrorBars(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context,
                     std::vector<double> x, std::vector<double> y);
void axisArgumentsConvertedIntoTickGroups(tick_t *ticks, tick_label_t *tick_labels,
                                          const std::shared_ptr<GRM::Element> &axis, DelValues del);
void calculatePolarLimits(const std::shared_ptr<GRM::Element> &central_region,
                          const std::shared_ptr<GRM::Context> &context);
void adjustPolarGridLineTextPosition(double theta_lim_min, double theta_lim_max, double *theta_r, double *r_r,
                                     double value, std::shared_ptr<GRM::Element> central_region);
void histBins(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void calculatePolarThetaAndR(std::vector<double> &theta, std::vector<double> &r,
                             const std::shared_ptr<GRM::Element> &element,
                             const std::shared_ptr<GRM::Context> &context);
void tickLabelAdjustment(const std::shared_ptr<GRM::Element> &tick_group, int child_id, DelValues del);
void applyTickModificationMap(const std::shared_ptr<GRM::Element> &tick_group,
                              const std::shared_ptr<GRM::Context> &context, int child_id, DelValues del);
void kindDependentCoordinateLimAdjustments(const std::shared_ptr<GRM::Element> &element,
                                           const std::shared_ptr<GRM::Context> &context, double *min_component,
                                           double *max_component, std::string lim, std::string location);
void calculateInitialCoordinateLims(const std::shared_ptr<GRM::Element> &element,
                                    const std::shared_ptr<GRM::Context> &context);
std::map<int, std::map<double, std::map<std::string, GRM::Value>>> *getTickModificationMap();

void applyRootDefaults(const std::shared_ptr<GRM::Element> &root);
void applyPlotDefaultsHelper(const std::shared_ptr<GRM::Element> &element);
void applyPlotDefaults(const std::shared_ptr<GRM::Element> &plot);
void applyCentralRegionDefaults(const std::shared_ptr<GRM::Element> &central_region);

#endif // GR_RENDER_UTIL_HXX
