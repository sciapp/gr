#ifndef RENDER_HXX
#define RENDER_HXX

#include <iostream>
#include <optional>
#include <queue>
#include <stack>
#include <functional>

#include <grm/dom_render/context.hxx>
#include <grm/dom_render/graphics_tree/Element.hxx>
#include <grm/dom_render/graphics_tree/Document.hxx>
#include "gr.h"
#include "grm/layout.hxx"
#include <grm/util.h>
#include <grm/dom_render/GroupMask.hxx>

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
#define PLOT_DEFAULT_ACCELERATE 1
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


/* ========================= Enums ================================================================================== */

typedef enum
{
  GR_COLOR_RESET = 0,
  GR_COLOR_LINE = GR_SPEC_LINE,
  GR_COLOR_MARKER = GR_SPEC_MARKER,
  GR_COLOR_FILL = GR_SPEC_COLOR,
  GR_COLOR_TEXT = 1 << 3,
  GR_COLOR_BORDER = 1 << 4
} GRColorType;

enum class CoordinateSpace
{
  WC,
  NDC
};

/* ========================= cast functions ========================================================================= */

namespace GRM
{

GRM_EXPORT int algorithmStringToInt(const std::string &algorithm_str);
GRM_EXPORT int colormapStringToInt(const std::string &colormap_str);
GRM_EXPORT int fontStringToInt(const std::string &font_str);
GRM_EXPORT int fontPrecisionStringToInt(const std::string &font_precision_str);
GRM_EXPORT int lineTypeStringToInt(const std::string &line_type_str);
GRM_EXPORT int locationStringToInt(const std::string &location_str);
GRM_EXPORT int xAxisLocationStringToInt(const std::string &location_str);
GRM_EXPORT int yAxisLocationStringToInt(const std::string &location_str);
GRM_EXPORT int markerTypeStringToInt(const std::string &marker_type_str);
GRM_EXPORT int projectionTypeStringToInt(const std::string &projection_type_str);
GRM_EXPORT int modelStringToInt(const std::string &model_str);
GRM_EXPORT int scientificFormatStringToInt(const std::string &scientific_format_str);
GRM_EXPORT int textAlignHorizontalStringToInt(const std::string &text_align_horizontal_str);
GRM_EXPORT int textAlignVerticalStringToInt(const std::string &text_align_vertical_str);
GRM_EXPORT int textEncodingStringToInt(const std::string &text_encoding_str);
GRM_EXPORT int tickOrientationStringToInt(const std::string &tick_orientation_str);
GRM_EXPORT int errorBarStyleStringToInt(const std::string &error_bar_stylr_str);
GRM_EXPORT int clipRegionStringToInt(const std::string &error_bar_stylr_str);
GRM_EXPORT int resampleMethodStringToInt(const std::string &error_bar_stylr_str);
GRM_EXPORT int fillStyleStringToInt(const std::string &fill_style_str);
GRM_EXPORT int fillIntStyleStringToInt(const std::string &fill_int_style_str);
GRM_EXPORT int transformationStringToInt(const std::string &transformation_str);
GRM_EXPORT int labelOrientationStringToInt(const std::string &label_orientation_str);
GRM_EXPORT int worldCoordinatesStringToInt(const std::string &world_coordinates_str);

GRM_EXPORT std::string algorithmIntToString(int algorithm);
GRM_EXPORT std::string colormapIntToString(int colormap);
GRM_EXPORT std::string fontIntToString(int font);
GRM_EXPORT std::string fontPrecisionIntToString(int font_precision);
GRM_EXPORT std::string lineTypeIntToString(int line_type);
GRM_EXPORT std::string locationIntToString(int location);
GRM_EXPORT std::string xAxisLocationIntToString(int location);
GRM_EXPORT std::string yAxisLocationIntToString(int location);
GRM_EXPORT std::string markerTypeIntToString(int marker_type);
GRM_EXPORT std::string projectionTypeIntToString(int projection_type);
GRM_EXPORT std::string modelIntToString(int model);
GRM_EXPORT std::string scientificFormatIntToString(int scientific_format);
GRM_EXPORT std::string textAlignHorizontalIntToString(int text_align_horizontal);
GRM_EXPORT std::string textAlignVerticalIntToString(int text_align_vertical);
GRM_EXPORT std::string textEncodingIntToString(int text_encoding);
GRM_EXPORT std::string tickOrientationIntToString(int tick_orientation);
GRM_EXPORT std::string errorBarStyleIntToString(int error_bar_style);
GRM_EXPORT std::string clipRegionIntToString(int error_bar_style);
GRM_EXPORT std::string resampleMethodIntToString(int error_bar_style);
GRM_EXPORT std::string fillStyleIntToString(int fill_style);
GRM_EXPORT std::string fillIntStyleIntToString(int fill_int_style);
GRM_EXPORT std::string transformationIntToString(int transformation);
GRM_EXPORT std::string labelOrientationIntToString(int label_orientation);
GRM_EXPORT std::string worldCoordinatesIntToString(int world_coordinates);

GRM_EXPORT std::vector<std::string> getSizeUnits();
GRM_EXPORT std::vector<std::string> getColormaps();
GRM_EXPORT std::vector<std::string> getFonts();
GRM_EXPORT std::vector<std::string> getFontPrecisions();
GRM_EXPORT std::vector<std::string> getLineTypes();
GRM_EXPORT std::vector<std::string> getLocations();
GRM_EXPORT std::vector<std::string> getXAxisLocations();
GRM_EXPORT std::vector<std::string> getYAxisLocations();
GRM_EXPORT std::vector<std::string> getMarkerTypes();
GRM_EXPORT std::vector<std::string> getTextAlignHorizontal();
GRM_EXPORT std::vector<std::string> getTextAlignVertical();
GRM_EXPORT std::vector<std::string> getAlgorithm();
GRM_EXPORT std::vector<std::string> getModel();
GRM_EXPORT std::vector<std::string> getContextAttributes();
GRM_EXPORT std::vector<std::string> getFillStyles();
GRM_EXPORT std::vector<std::string> getFillIntStyles();
GRM_EXPORT std::vector<std::string> getTransformation();

GRM_EXPORT void addValidContextKey(std::string key);

GRM_EXPORT const GRM::GroupMask *getGroupMask();

/* ========================= classes ================================================================================ */


class PushDrawableToZQueue
{
public:
  PushDrawableToZQueue(
      std::function<void(const std::shared_ptr<Element> &, const std::shared_ptr<Context> &)> draw_function);
  void operator()(const std::shared_ptr<Element> &element, const std::shared_ptr<Context> &context);

private:
  std::function<void(const std::shared_ptr<Element> &, const std::shared_ptr<Context> &)> draw_function;
};

GRM_EXPORT void updateFilter(const std::shared_ptr<Element> &element, const std::string &attr,
                             const std::string &value);
GRM_EXPORT void renderCaller();
GRM_EXPORT void updateContextAttribute(const std::shared_ptr<Element> &element, const std::string &attr,
                                       const Value &old_value);
GRM_EXPORT void deleteContextAttribute(const std::shared_ptr<Element> &element);
GRM_EXPORT void cleanupElement(Element &element);

class GRM_EXPORT Render : public Document
{
  /*!
   * The Render class is used for creating or modifying elements that can also be processed by this class
   * to create plots etc.
   *
   * Render has an std::shared_ptr Context as a private member for storing certain datatypes
   */

public:
  const char *initializeHistory();

  /* ------------------------------- create functions ----------------------------------------------------------------*/

  static std::shared_ptr<Render> createRender();

  std::shared_ptr<Element> createPlot(int plot_id, const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createCentralRegion(const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createPolymarker(const std::string &x_key, std::optional<std::vector<double>> x,
                                            const std::string &y_key, std::optional<std::vector<double>> y,
                                            const std::shared_ptr<Context> &ext_context = nullptr, int marker_type = 0,
                                            double marker_size = 0.0, int marker_color_ind = 0,
                                            const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createPolymarker(double x, double y, int marker_type = 0, double marker_size = 0.0,
                                            int marker_colorind = 0,
                                            const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createPolyline(const std::string &x_key, std::optional<std::vector<double>> x,
                                          const std::string &y_key, std::optional<std::vector<double>> y,
                                          const std::shared_ptr<Context> &ext_context = nullptr, int line_type = 0,
                                          double line_width = 0.0, int line_colorind = 0,
                                          const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createPolyline(double x1, double x2, double y1, double y2, int line_type = 0,
                                          double line_width = 0.0, int line_colorind = 0,
                                          const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createText(double x, double y, const std::string &text,
                                      CoordinateSpace space = CoordinateSpace::NDC,
                                      const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createFillArea(const std::string &x_key, std::optional<std::vector<double>> x,
                                          const std::string &y_key, std::optional<std::vector<double>> y,
                                          const std::shared_ptr<Context> &ext_context = nullptr, int fill_int_style = 0,
                                          int fill_style = 0, int fill_color_ind = -1,
                                          const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createCellArray(double xmin, double xmax, double ymin, double ymax, int dimx, int dimy,
                                           int scol, int srow, int ncol, int nrow, const std::string &color_key,
                                           std::optional<std::vector<int>> color,
                                           const std::shared_ptr<Context> &ext_context = nullptr,
                                           const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createNonUniformPolarCellArray(
      double x_org, double y_org, const std::string &theta_key, std::optional<std::vector<double>> theta,
      const std::string &r_key, std::optional<std::vector<double>> r, int dim_theta, int dim_r, int s_col, int s_row,
      int n_col, int n_row, const std::string &color_key, std::optional<std::vector<int>> color,
      const std::shared_ptr<Context> &ext_context = nullptr, const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createPolarCellArray(double x_org, double y_org, double theta_min, double theta_max,
                                                double r_min, double r_max, int dim_theta, int dim_r, int s_col,
                                                int s_row, int n_col, int n_row, const std::string &color_key,
                                                std::optional<std::vector<int>> color,
                                                const std::shared_ptr<Context> &ext_context = nullptr,
                                                const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createEmptyAxis(const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createAxis(double min_val, double max_val, double tick, double org, double pos,
                                      int major_count, int num_ticks, int num_tick_labels, double tick_size,
                                      int tick_orientation, double label_pos,
                                      const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createTickGroup(int is_major, const std::string &tick_label, double value, double width,
                                           const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createTick(int is_major, double value,
                                      const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createGridLine(int is_major, double value,
                                          const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createLegend(const std::shared_ptr<Element> &ext_element = nullptr,
                                        const std::shared_ptr<Context> &ext_context = nullptr);

  std::shared_ptr<Element> createPieSegment(const double start_angle, const double end_angle, const std::string &text,
                                            const int color_index,
                                            const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createBar(const double x1, const double x2, const double y1, const double y2,
                                     const int bar_color_index, const int edge_color_index,
                                     const std::string &bar_color_rgb = "", const std::string &edge_color_rgb = "",
                                     const double linewidth = -1, const std::string &text = "",
                                     const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createSeries(const std::string &name);

  std::shared_ptr<Element> createDrawImage(double xmin, double ymin, double xmax, double ymax, int width, int height,
                                           const std::string &data_key, std::optional<std::vector<int>> data, int model,
                                           const std::shared_ptr<Context> &ext_context = nullptr,
                                           const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createDrawArc(double xmin, double xmax, double ymin, double ymax, double start_angle,
                                         double end_angle, const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createFillArc(double xmin, double xmax, double ymin, double ymax, double a1, double a2,
                                         int fill_int_style = 0, int fill_style = 0, int fill_color_ind = -1,
                                         const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createDrawRect(double xmin, double xmax, double ymin, double ymax,
                                          const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createFillRect(double xmin, double xmax, double ymin, double ymax, int fill_int_style = 0,
                                          int fill_style = 0, int fill_color_ind = -1,
                                          const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createQuiver(const std::string &x_key, std::optional<std::vector<double>> x,
                                        const std::string &y_key, std::optional<std::vector<double>> y,
                                        const std::string &u_key, std::optional<std::vector<double>> u,
                                        const std::string &v_key, std::optional<std::vector<double>> v, int colored,
                                        const std::shared_ptr<Context> &ext_context = nullptr);

  std::shared_ptr<Element> createHexbin(const std::string &x_key, std::optional<std::vector<double>> x,
                                        const std::string &y_key, std::optional<std::vector<double>> y,
                                        const std::shared_ptr<Context> &ext_context = nullptr);

  std::shared_ptr<Element> createColorbar(unsigned int num_color_values,
                                          const std::shared_ptr<Context> &ext_context = nullptr,
                                          const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createNonUniformCellArray(const std::string &x_key, std::optional<std::vector<double>> x,
                                                     const std::string &y_key, std::optional<std::vector<double>> y,
                                                     int dimx, int dimy, int scol, int srow, int ncol, int nrow,
                                                     const std::string &color_key,
                                                     std::optional<std::vector<int>> color,
                                                     const std::shared_ptr<Context> &ext_context = nullptr,
                                                     const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createGrid3d(double x_tick, double y_tick, double z_tick, double x_org, double y_org,
                                        double z_org, int major_x, int major_y, int major_z,
                                        const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createEmptyGrid3d(bool x_grid, bool y_grid, bool z_grid,
                                             const std::shared_ptr<Element> &ext_element = nullptr);


  std::shared_ptr<Element> createAxes3d(double x_tick, double y_tick, double z_tick, double x_org, double y_org,
                                        double z_org, int major_x, int major_y, int major_z, int tick_orientation,
                                        const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createEmptyAxes3d(int tick_orientation,
                                             const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createPolyline3d(const std::string &x_key, std::optional<std::vector<double>> x,
                                            const std::string &y_key, std::optional<std::vector<double>> y,
                                            const std::string &z_key, std::optional<std::vector<double>> z,
                                            const std::shared_ptr<Context> &ext_context = nullptr,
                                            const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createPolymarker3d(const std::string &x_key, std::optional<std::vector<double>> x,
                                              const std::string &y_key, std::optional<std::vector<double>> y,
                                              const std::string &z_key, std::optional<std::vector<double>> z,
                                              const std::shared_ptr<Context> &ext_context = nullptr,
                                              const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createDrawGraphics(const std::string &data_key, std::optional<std::vector<int>> data,
                                              const std::shared_ptr<Context> &ext_context = nullptr,
                                              const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createTriSurface(const std::string &px_key, std::optional<std::vector<double>> px,
                                            const std::string &py_key, std::optional<std::vector<double>> py,
                                            const std::string &pz_key, std::optional<std::vector<double>> pz,
                                            const std::shared_ptr<Context> &ext_context = nullptr);

  std::shared_ptr<Element> createTitles3d(const std::string &x_label, const std::string &y_label,
                                          const std::string &z_label,
                                          const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createLayoutGrid(const Grid &grid);

  std::shared_ptr<Element> createLayoutGridElement(const GridElement &grid_element, const Slice &slice);

  std::shared_ptr<Element> createIsoSurfaceRenderElement(int drawable_type);

  std::shared_ptr<Element> createPanzoom(double x, double y, double xzoom, double yzoom);

  std::shared_ptr<Element> createPolarBar(double count, int class_nr,
                                          const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createErrorBar(double error_bar_x, double error_bar_y_min, double error_bar_y_max,
                                          int color_error_bar, const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createIntegral(double int_lim_low, double int_lim_high,
                                          const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createSideRegion(const std::string &location,
                                            const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createTextRegion(const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createSidePlotRegion(const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createRadialAxes(const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createThetaAxes(const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createAngleLine(double x, double y, const std::string &angle_label,
                                           const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createArcGridLine(double value, const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createOverlay(const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createOverlayElement(double x, double y, std::string type,
                                                const std::shared_ptr<Element> &ext_element = nullptr);

  //! Modifierfunctions

  /* ------------------------------- setter functions ----------------------------------------------------------------*/

  //! next 2 functions -> store color indices vec or color rgb values
  void setNextColor(const std::shared_ptr<Element> &element, const std::string &color_indices_key,
                    const std::vector<int> &color_indices, const std::shared_ptr<Context> &ext_context = nullptr);

  void setNextColor(const std::shared_ptr<Element> &element, const std::string &color_rgb_values_key,
                    const std::vector<double> &color_rgb_values, const std::shared_ptr<Context> &ext_context = nullptr);

  //! only keys -> reusing stored context vectors
  void setNextColor(const std::shared_ptr<Element> &element, std::optional<std::string> color_indices_key,
                    std::optional<std::string> color_rgb_values_key);

  //! Use Fallback
  void setNextColor(const std::shared_ptr<Element> &element);

  void setClipRegion(const std::shared_ptr<Element> &element, int region);

  void setViewport(const std::shared_ptr<Element> &element, double xmin, double xmax, double ymin, double ymax);

  void setWSViewport(const std::shared_ptr<Element> &element, double xmin, double xmax, double ymin, double ymax);

  void setWindow(const std::shared_ptr<Element> &element, double xmin, double xmax, double ymin, double ymax);

  void setWSWindow(const std::shared_ptr<Element> &element, double xmin, double xmax, double ymin, double ymax);

  void setMarkerType(const std::shared_ptr<Element> &element, int type);

  void setMarkerType(const std::shared_ptr<Element> &element, const std::string &types_key,
                     std::optional<std::vector<int>> types, const std::shared_ptr<Context> &ext_context = nullptr);

  void setMarkerSize(const std::shared_ptr<Element> &element, const std::string &sizes_key,
                     std::optional<std::vector<double>> sizes, const std::shared_ptr<Context> &ext_context = nullptr);

  void setMarkerSize(const std::shared_ptr<Element> &element, double size);

  void setMarkerColorInd(const std::shared_ptr<Element> &element, const std::string &colorinds_key,
                         std::optional<std::vector<int>> colorinds,
                         const std::shared_ptr<Context> &ext_context = nullptr);

  void setMarkerColorInd(const std::shared_ptr<Element> &element, int color);

  void setLineType(const std::shared_ptr<Element> &element, const std::string &types_key,
                   std::optional<std::vector<int>> types, const std::shared_ptr<Context> &ext_context = nullptr);

  void setLineType(const std::shared_ptr<Element> &element, int type);

  void setLineWidth(const std::shared_ptr<Element> &element, const std::string &widths_key,
                    std::optional<std::vector<double>> widths, const std::shared_ptr<Context> &ext_context = nullptr);

  void setLineWidth(const std::shared_ptr<Element> &element, double width);

  void setLineColorInd(const std::shared_ptr<Element> &element, const std::string &colorinds_key,
                       std::optional<std::vector<int>> colorinds,
                       const std::shared_ptr<Context> &ext_context = nullptr);

  void setLineColorInd(const std::shared_ptr<Element> &element, int color);

  void setCharUp(const std::shared_ptr<Element> &element, double ux, double uy);

  void setTextAlign(const std::shared_ptr<Element> &element, int horizontal, int vertical);

  void setTextWidthAndHeight(const std::shared_ptr<Element> &element, double width, double height);

  void setColorRep(const std::shared_ptr<Element> &element, int index, double red, double green, double blue);


  void setLineSpec(const std::shared_ptr<Element> &element, const std::string &spec);

  void setFillIntStyle(const std::shared_ptr<Element> &element, int index);

  void setFillColorInd(const std::shared_ptr<Element> &element, int color);

  void setFillStyle(const std::shared_ptr<Element> &element, int index);

  void setScale(const std::shared_ptr<Element> &element, int scale);

  void setWindow3d(const std::shared_ptr<Element> &element, double xmin, double xmax, double ymin, double ymax,
                   double zmin, double zmax);

  void setSpace3d(const std::shared_ptr<Element> &element, double fov, double camera_distance);

  void setSpace(const std::shared_ptr<Element> &element, double zmin, double zmax, int rotation, int tilt);

  void setSelectSpecificXform(const std::shared_ptr<Element> &element, int transform);

  void setTextColorInd(const std::shared_ptr<Element> &element, int index);

  void setBorderColorInd(const std::shared_ptr<Element> &element, int index);

  void setBorderWidth(const std::shared_ptr<Element> &element, double width);

  void selectClipXForm(const std::shared_ptr<Element> &element, int form);

  void setCharHeight(const std::shared_ptr<Element> &element, double height);

  void setTransparency(const std::shared_ptr<Element> &element, double transparency);

  void setResampleMethod(const std::shared_ptr<Element> &element, int resample);

  void setTextEncoding(const std::shared_ptr<Element> &element, int encoding);

  void setProjectionType(const std::shared_ptr<Element> &element, int type);

  static void setViewportNormalized(const std::shared_ptr<Element> &element, double xmin, double xmax, double ymin,
                                    double ymax);

  void setOriginPosition(const std::shared_ptr<Element> &element, const std::string &x_org_pos,
                         const std::string &y_org_pos);

  void setOriginPosition3d(const std::shared_ptr<Element> &element, const std::string &x_org_pos,
                           const std::string &y_org_pos, const std::string &z_org_pos);

  void setGR3LightParameters(const std::shared_ptr<Element> &element, double ambient, double diffuse, double specular,
                             double specular_power);

  static void setAutoUpdate(bool update);

  void setActiveFigure(const std::shared_ptr<Element> &element);

  /* ------------------------------- getter functions ----------------------------------------------------------------*/

  std::shared_ptr<Element> getActiveFigure();

  static void getAutoUpdate(bool *update);

  static void getFigureSize(int *pixel_width, int *pixel_height, double *metric_width, double *metric_height);

  std::map<int, std::map<double, std::map<std::string, Value>>> *getTickModificationMap();

  int getAxisId();

  static bool getViewport(const std::shared_ptr<Element> &element, double *xmin, double *xmax, double *ymin,
                          double *ymax);

  std::shared_ptr<Context> getRenderContext();

  void render();                                            // render doc and render context
  void render(const std::shared_ptr<Context> &ext_context); // render doc and external context
  void render(const std::shared_ptr<Document> &document);   // external doc and render context
  static void render(const std::shared_ptr<Document> &document,
                     const std::shared_ptr<Context> &ext_context); // external doc and external context; could be static
  void processTree();
  static void finalize();
  std::shared_ptr<Context> getContext();

  static void processViewport(const std::shared_ptr<Element> &elem);
  static void calculateCharHeight(const std::shared_ptr<Element> &elem);
  static void processLimits(const std::shared_ptr<Element> &elem);
  static void processWindow(const std::shared_ptr<Element> &elem);
  static void processScale(const std::shared_ptr<Element> &elem);
  static void processAttributes(const std::shared_ptr<Element> &element);
  static std::vector<std::string> getDefaultAndTooltip(const std::shared_ptr<Element> &element,
                                                       const std::string &attribute_name);


private:
  Render();
  std::shared_ptr<Context> context;
};

} // namespace GRM

#endif
