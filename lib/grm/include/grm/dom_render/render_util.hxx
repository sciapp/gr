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
#define PLOT_DEFAULT_KEEP_WINDOW 1
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

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ util ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#define PLOT_CUSTOM_COLOR_INDEX 979


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
    "wireframe", "surface", "line3", "scatter3", "trisurface", "volume", "isosurface",
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
    "contour", "contourf", "hexbin", "isosurface", "quiver", "shade", "surface", "tricontour", "trisurface", "volume",
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

/* ========================= functions ============================================================================== */

namespace GRM
{
void GRM_EXPORT getFigureSize(int *pixel_width, int *pixel_height, double *metric_width, double *metric_height);
void GRM_EXPORT calculateCharHeight(const std::shared_ptr<GRM::Element> &element);
} // namespace GRM

void getPlotParent(std::shared_ptr<GRM::Element> &element);
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
