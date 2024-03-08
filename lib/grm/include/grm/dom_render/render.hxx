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

/* ========================= macros ================================================================================= */

/* ------------------------- plot ----------------------------------------------------------------------------------- */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plot arguments ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#define PLOT_DEFAULT_CLEAR 1
#define PLOT_DEFAULT_UPDATE 1
#define PLOT_DEFAULT_LOCATION 1
#define PLOT_DEFAULT_SUBPLOT_MIN_X 0.0
#define PLOT_DEFAULT_SUBPLOT_MAX_X 1.0
#define PLOT_DEFAULT_SUBPLOT_MIN_Y 0.0
#define PLOT_DEFAULT_SUBPLOT_MAX_Y 1.0
#define PLOT_DEFAULT_ROTATION 40.0
#define PLOT_DEFAULT_TILT 60.0
#define PLOT_DEFAULT_KEEP_ASPECT_RATIO 0
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
#define PLOT_DEFAULT_COLORBAR_DIAG_FACTOR 0.016
#define PLOT_DEFAULT_COLORBAR_WIDTH 0.03
#define PLOT_DEFAULT_COLORBAR_MAX_CHAR_HEIGHT 0.012
#define PLOT_DEFAULT_COLORBAR_OFFSET 0.02

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ util ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#define PLOT_CUSTOM_COLOR_INDEX 979

/* ========================= Enums ================================================================================== */

typedef enum
{
  GR_COLOR_RESET = 0,
  GR_COLOR_LINE = 1 << 0,
  GR_COLOR_MARKER = 1 << 1,
  GR_COLOR_FILL = 1 << 2,
  GR_COLOR_TEXT = 1 << 3,
  GR_COLOR_BORDER = 1 << 4
} gr_color_type_t;

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

enum class EXPORT CoordinateSpace
{
  WC,
  NDC
};

/* ========================= cast functions ========================================================================= */

EXPORT int algorithmStringToInt(const std::string &algorithm_str);
EXPORT int colormapStringToInt(const std::string &colormap_str);
EXPORT int fillIntStyleStringToInt(const std::string &fillintstyle_str);
EXPORT int fillStyleStringToInt(const std::string &fillstyle_str);
EXPORT int fontStringToInt(const std::string &font_str);
EXPORT int fontPrecisionStringToInt(const std::string &font_precision_str);
EXPORT int lineTypeStringToInt(const std::string &line_type_str);
EXPORT int locationStringToInt(const std::string &location_str);
EXPORT int markerTypeStringToInt(const std::string &marker_type_str);
EXPORT int projectionTypeStringToInt(const std::string &projection_type_str);
EXPORT int modelStringToInt(const std::string &model_str);
EXPORT int textAlignHorizontalStringToInt(const std::string &text_align_horizontal_str);
EXPORT int textAlignVerticalStringToInt(const std::string &text_align_vertical_str);
EXPORT int textEncodingStringToInt(const std::string &text_encoding_str);
EXPORT int tickOrientationStringToInt(const std::string &tick_orientation_str);

EXPORT std::string algorithmIntToString(int algorithm);
EXPORT std::string colormapIntToString(int colormap);
EXPORT std::string fillIntStyleIntToString(int fillintstyle);
EXPORT std::string fillStyleIntToString(int fillstyle);
EXPORT std::string fontIntToString(int font);
EXPORT std::string fontPrecisionIntToString(int font_precision);
EXPORT std::string lineTypeIntToString(int line_type);
EXPORT std::string locationIntToString(int location);
EXPORT std::string markerTypeIntToString(int marker_type);
EXPORT std::string projectionTypeIntToString(int projection_type);
EXPORT std::string modelIntToString(int model);
EXPORT std::string textAlignHorizontalIntToString(int text_align_horizontal);
EXPORT std::string textAlignVerticalIntToString(int text_align_vertical);
EXPORT std::string textEncodingIntToString(int text_encoding);
EXPORT std::string tickOrientationIntToString(int tick_orientation);

EXPORT std::vector<std::string> getSizeUnits();
EXPORT std::vector<std::string> getColormaps();
EXPORT std::vector<std::string> getFonts();
EXPORT std::vector<std::string> getFontPrecisions();
EXPORT std::vector<std::string> getLineTypes();
EXPORT std::vector<std::string> getLocations();
EXPORT std::vector<std::string> getMarkerTypes();
EXPORT std::vector<std::string> getTextAlignHorizontal();
EXPORT std::vector<std::string> getTextAlignVertical();
EXPORT std::vector<std::string> getAlgorithm();
EXPORT std::vector<std::string> getModel();

/* ========================= classes ================================================================================ */


class PushDrawableToZQueue
{
public:
  PushDrawableToZQueue(
      std::function<void(const std::shared_ptr<GRM::Element> &, const std::shared_ptr<GRM::Context> &)> drawFunction);
  void operator()(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);

private:
  std::function<void(const std::shared_ptr<GRM::Element> &, const std::shared_ptr<GRM::Context> &)> drawFunction;
};

EXPORT void updateFilter(const std::shared_ptr<GRM::Element> &element, const std::string &attr,
                         const std::string &value);
EXPORT void renderCaller();
EXPORT void updateContextAttribute(const std::shared_ptr<GRM::Element> &element, const std::string &attr,
                                   const GRM::Value &old_value);
EXPORT void deleteContextAttribute(const std::shared_ptr<GRM::Element> &element);

namespace GRM
{

class EXPORT Render : public Document
{
  /*!
   * The GRM::Render class is used for creating or modifying elements that can also be processed by this class
   * to create plots etc.
   *
   * GRM::Render has an std::shared_ptr GRM::Context as a private member for storing certain datatypes
   */

public:
  /* ------------------------------- create functions ----------------------------------------------------------------*/

  static std::shared_ptr<Render> createRender();

  std::shared_ptr<GRM::Element> createPlot(int plotId, const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<GRM::Element> createCentralRegion(const std::shared_ptr<GRM::Element> &ext_element = nullptr);

  std::shared_ptr<Element> createPolymarker(const std::string &x_key, std::optional<std::vector<double>> x,
                                            const std::string &y_key, std::optional<std::vector<double>> y,
                                            const std::shared_ptr<Context> &extContext = nullptr, int marker_type = 0,
                                            double marker_size = 0.0, int marker_colorind = 0,
                                            const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<Element> createPolymarker(double x, double y, int marker_type = 0, double marker_size = 0.0,
                                            int marker_colorind = 0,
                                            const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<Element> createPolyline(const std::string &x_key, std::optional<std::vector<double>> x,
                                          const std::string &y_key, std::optional<std::vector<double>> y,
                                          const std::shared_ptr<Context> &extContext = nullptr, int line_type = 0,
                                          double line_width = 0.0, int line_colorind = 0,
                                          const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<Element> createPolyline(double x1, double x2, double y1, double y2, int line_type = 0,
                                          double line_width = 0.0, int line_colorind = 0,
                                          const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<Element> createText(double x, double y, const std::string &text,
                                      CoordinateSpace space = CoordinateSpace::NDC,
                                      const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<Element> createFillArea(const std::string &x_key, std::optional<std::vector<double>> x,
                                          const std::string &y_key, std::optional<std::vector<double>> y,
                                          const std::shared_ptr<Context> &extContext = nullptr, int fillintstyle = 0,
                                          int fillstyle = 0, int fillcolorind = -1,
                                          const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<Element> createCellArray(double xmin, double xmax, double ymin, double ymax, int dimx, int dimy,
                                           int scol, int srow, int ncol, int nrow, const std::string &color_key,
                                           std::optional<std::vector<int>> color,
                                           const std::shared_ptr<Context> &extContext = nullptr,
                                           const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<Element> createNonUniformPolarCellArray(
      double x_org, double y_org, const std::string &phi_key, std::optional<std::vector<double>> phi,
      const std::string &r_key, std::optional<std::vector<double>> r, int dimphi, int dimr, int scol, int srow,
      int ncol, int nrow, const std::string &color_key, std::optional<std::vector<int>> color,
      const std::shared_ptr<Context> &extContext = nullptr, const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<Element> createPolarCellArray(double x_org, double y_org, double phimin, double phimax, double rmin,
                                                double rmax, int dimphi, int dimr, int scol, int srow, int ncol,
                                                int nrow, const std::string &color_key,
                                                std::optional<std::vector<int>> color,
                                                const std::shared_ptr<Context> &extContext = nullptr,
                                                const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<Element> createAxes(double x_tick, double y_tick, double x_org, double y_org, int x_major,
                                      int y_major, int tick_orientation,
                                      const std::shared_ptr<GRM::Element> &ext_element = nullptr);

  std::shared_ptr<GRM::Element> createEmptyAxes(int tick_orientation,
                                                const std::shared_ptr<GRM::Element> &ext_element = nullptr);

  std::shared_ptr<GRM::Element> createEmptyDoubleAxes();

  std::shared_ptr<Element> createLegend(const std::string &labels_key, std::optional<std::vector<std::string>> labels,
                                        const std::string &specs_key, std::optional<std::vector<std::string>> specs,
                                        const std::shared_ptr<GRM::Context> &extContext = nullptr,
                                        const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<Element> createDrawPolarAxes(int angle_ticks, const std::string &kind, int phiflip,
                                               const std::string &norm = "", double tick = 0.0, double line_width = 0.0,
                                               const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<Element> createPieLegend(const std::string &labels_key,
                                           std::optional<std::vector<std::string>> labels,
                                           const std::shared_ptr<GRM::Context> &extContext = nullptr,
                                           const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<GRM::Element> createPieSegment(const double start_angle, const double end_angle,
                                                 const std::string &text, const int color_index,
                                                 const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<GRM::Element> createBar(const double x1, const double x2, const double y1, const double y2,
                                          const int bar_color_index, const int edge_color_index,
                                          const std::string &bar_color_rgb = "", const std::string &edge_color_rgb = "",
                                          const double linewidth = -1, const std::string &text = "",
                                          const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<Element> createGrid(double x_tick, double y_tick, double x_org, double y_org, int major_x,
                                      int major_y, const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<GRM::Element> createEmptyGrid(bool x_grid, bool y_grid,
                                                const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<Element> createSeries(const std::string &name);

  std::shared_ptr<Element> createDrawImage(double xmin, double ymin, double xmax, double ymax, int width, int height,
                                           const std::string &data_key, std::optional<std::vector<int>> data, int model,
                                           const std::shared_ptr<Context> &extContext = nullptr,
                                           const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<Element> createDrawArc(double xmin, double xmax, double ymin, double ymax, double start_angle,
                                         double end_angle, const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<Element> createFillArc(double xmin, double xmax, double ymin, double ymax, double a1, double a2,
                                         int fillintstyle = 0, int fillstyle = 0, int fillcolorind = -1,
                                         const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<Element> createDrawRect(double xmin, double xmax, double ymin, double ymax,
                                          const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<Element> createFillRect(double xmin, double xmax, double ymin, double ymax, int fillintstyle = 0,
                                          int fillstyle = 0, int fillcolorind = -1,
                                          const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<Element> createQuiver(const std::string &x_key, std::optional<std::vector<double>> x,
                                        const std::string &y_key, std::optional<std::vector<double>> y,
                                        const std::string &u_key, std::optional<std::vector<double>> u,
                                        const std::string &v_key, std::optional<std::vector<double>> v, int color,
                                        const std::shared_ptr<Context> &extContext = nullptr);

  std::shared_ptr<Element> createHexbin(const std::string &x_key, std::optional<std::vector<double>> x,
                                        const std::string &y_key, std::optional<std::vector<double>> y,
                                        const std::shared_ptr<Context> &extContext = nullptr);

  std::shared_ptr<Element> createColorbar(unsigned int colors, const std::shared_ptr<Context> &extContext = nullptr,
                                          const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<Element> createNonUniformCellArray(const std::string &x_key, std::optional<std::vector<double>> x,
                                                     const std::string &y_key, std::optional<std::vector<double>> y,
                                                     int dimx, int dimy, int scol, int srow, int ncol, int nrow,
                                                     const std::string &color_key,
                                                     std::optional<std::vector<int>> color,
                                                     const std::shared_ptr<Context> &extContext = nullptr,
                                                     const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<Element> createGrid3d(double x_tick, double y_tick, double z_tick, double x_org, double y_org,
                                        double z_org, int major_x, int major_y, int major_z,
                                        const std::shared_ptr<GRM::Element> &ext_element = nullptr);

  std::shared_ptr<GRM::Element> createEmptyGrid3d(bool x_grid, bool y_grid, bool z_grid,
                                                  const std::shared_ptr<GRM::Element> &ext_element = nullptr);


  std::shared_ptr<Element> createAxes3d(double x_tick, double y_tick, double z_tick, double x_org, double y_org,
                                        double z_org, int major_x, int major_y, int major_z, int tick_orientation,
                                        const std::shared_ptr<GRM::Element> &ext_element = nullptr);

  std::shared_ptr<GRM::Element> createEmptyAxes3d(int tick_orientation,
                                                  const std::shared_ptr<GRM::Element> &ext_element = nullptr);

  std::shared_ptr<Element> createPolyline3d(const std::string &x_key, std::optional<std::vector<double>> x,
                                            const std::string &y_key, std::optional<std::vector<double>> y,
                                            const std::string &z_key, std::optional<std::vector<double>> z,
                                            const std::shared_ptr<Context> &extContext = nullptr,
                                            const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<Element> createPolymarker3d(const std::string &x_key, std::optional<std::vector<double>> x,
                                              const std::string &y_key, std::optional<std::vector<double>> y,
                                              const std::string &z_key, std::optional<std::vector<double>> z,
                                              const std::shared_ptr<Context> &extContext = nullptr,
                                              const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<Element> createDrawGraphics(const std::string &data_key, std::optional<std::vector<int>> data,
                                              const std::shared_ptr<Context> &extContext = nullptr,
                                              const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<Element> createTriSurface(const std::string &px_key, std::optional<std::vector<double>> px,
                                            const std::string &py_key, std::optional<std::vector<double>> py,
                                            const std::string &pz_key, std::optional<std::vector<double>> pz,
                                            const std::shared_ptr<Context> &extContext = nullptr);

  std::shared_ptr<Element> createTitles3d(const std::string &xlabel, const std::string &ylabel,
                                          const std::string &zlabel,
                                          const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<Element> createLayoutGrid(const grm::Grid &grid);

  std::shared_ptr<Element> createLayoutGridElement(const grm::GridElement &gridElement, const grm::Slice &slice);

  std::shared_ptr<Element> createIsoSurfaceRenderElement(int drawable_type);

  std::shared_ptr<GRM::Element> createPanzoom(double x, double y, double xzoom, double yzoom);

  std::shared_ptr<Element> createPolarBar(double count, int class_nr,
                                          const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<Element> createErrorBar(double error_bar_x, double error_bar_y_min, double error_bar_y_max,
                                          int color_error_bar,
                                          const std::shared_ptr<GRM::Element> &extElement = nullptr);

  std::shared_ptr<Element> createIntegral(double int_lim_low, double int_lim_high,
                                          const std::shared_ptr<GRM::Element> &extElement = nullptr);
  //! Modifierfunctions

  /* ------------------------------- setter functions ----------------------------------------------------------------*/

  //! next 2 functions -> store color indices vec or color rgb values
  void setNextColor(const std::shared_ptr<Element> &element, const std::string &color_indices_key,
                    const std::vector<int> &color_indices, const std::shared_ptr<Context> &extContext = nullptr);

  void setNextColor(const std::shared_ptr<Element> &element, const std::string &color_rgb_values_key,
                    const std::vector<double> &color_rgb_values, const std::shared_ptr<Context> &extContext = nullptr);

  //! only keys -> reusing stored context vectors
  void setNextColor(const std::shared_ptr<Element> &element, std::optional<std::string> color_indices_key,
                    std::optional<std::string> color_rgb_values_key);

  //! Use Fallback
  void setNextColor(const std::shared_ptr<Element> &element);

  void setViewport(const std::shared_ptr<Element> &element, double xmin, double xmax, double ymin, double ymax);

  void setWSViewport(const std::shared_ptr<Element> &element, double xmin, double xmax, double ymin, double ymax);

  void setWindow(const std::shared_ptr<Element> &element, double xmin, double xmax, double ymin, double ymax);

  void setWSWindow(const std::shared_ptr<Element> &element, double xmin, double xmax, double ymin, double ymax);

  void setMarkerType(const std::shared_ptr<Element> &element, int type);

  void setMarkerType(const std::shared_ptr<Element> &element, const std::string &types_key,
                     std::optional<std::vector<int>> types, const std::shared_ptr<Context> &extContext = nullptr);

  void setMarkerSize(const std::shared_ptr<Element> &element, const std::string &sizes_key,
                     std::optional<std::vector<double>> sizes, const std::shared_ptr<Context> &extContext = nullptr);

  void setMarkerSize(const std::shared_ptr<Element> &element, double size);

  void setMarkerColorInd(const std::shared_ptr<Element> &element, const std::string &colorinds_key,
                         std::optional<std::vector<int>> colorinds,
                         const std::shared_ptr<Context> &extContext = nullptr);

  void setMarkerColorInd(const std::shared_ptr<Element> &element, int color);

  void setLineType(const std::shared_ptr<Element> &element, const std::string &types_key,
                   std::optional<std::vector<int>> types, const std::shared_ptr<Context> &extContext = nullptr);

  void setLineType(const std::shared_ptr<Element> &element, int type);

  void setLineWidth(const std::shared_ptr<Element> &element, const std::string &widths_key,
                    std::optional<std::vector<double>> widths, const std::shared_ptr<Context> &extContext = nullptr);

  void setLineWidth(const std::shared_ptr<Element> &element, double width);

  void setLineColorInd(const std::shared_ptr<Element> &element, const std::string &colorinds_key,
                       std::optional<std::vector<int>> colorinds, const std::shared_ptr<Context> &extContext = nullptr);

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

  void selectClipXForm(const std::shared_ptr<Element> &element, int form);

  void setCharHeight(const std::shared_ptr<Element> &element, double height);

  void setTransparency(const std::shared_ptr<Element> &element, double alpha);

  void setResampleMethod(const std::shared_ptr<Element> &element, int resample);

  void setTextEncoding(const std::shared_ptr<Element> &element, int encoding);

  void setProjectionType(const std::shared_ptr<Element> &element, int type);

  void setSubplot(const std::shared_ptr<Element> &element, double xmin, double xmax, double ymin, double ymax);

  void setXTickLabels(const std::shared_ptr<GRM::Element> &element, const std::string &key,
                      std::optional<std::vector<std::string>> x_tick_labels,
                      const std::shared_ptr<GRM::Context> &extContext = nullptr);

  void setYTickLabels(const std::shared_ptr<GRM::Element> &element, const std::string &key,
                      std::optional<std::vector<std::string>> y_tick_labels,
                      const std::shared_ptr<GRM::Context> &extContext = nullptr);

  void setOriginPosition(const std::shared_ptr<GRM::Element> &element, const std::string &x_org_pos,
                         const std::string &y_org_pos);

  void setOriginPosition3d(const std::shared_ptr<GRM::Element> &element, const std::string &x_org_pos,
                           const std::string &y_org_pos, const std::string &z_org_pos);

  void setGR3LightParameters(const std::shared_ptr<GRM::Element> &element, double ambient, double diffuse,
                             double specular, double specular_power);

  static void setAutoUpdate(bool update);

  void setActiveFigure(const std::shared_ptr<GRM::Element> &element);

  /* ------------------------------- getter functions ----------------------------------------------------------------*/

  std::shared_ptr<GRM::Element> getActiveFigure();

  static void getAutoUpdate(bool *update);

  static void getFigureSize(int *pixel_width, int *pixel_height, double *metric_width, double *metric_height);

  void render();                                           // render doc and render context
  void render(const std::shared_ptr<Context> &extContext); // render doc and external context
  void render(const std::shared_ptr<Document> &document);  // external doc and render context
  static void render(const std::shared_ptr<Document> &document,
                     const std::shared_ptr<Context> &extContext); // external doc and external context; could be static
  void process_tree();
  static void finalize();
  std::shared_ptr<Context> getContext();

  static void processViewport(const std::shared_ptr<GRM::Element> &elem);
  static void processLimits(const std::shared_ptr<GRM::Element> &elem);
  static void processWindow(const std::shared_ptr<GRM::Element> &elem);
  static void processScale(const std::shared_ptr<GRM::Element> &elem);
  static void processAttributes(const std::shared_ptr<GRM::Element> &element);
  static std::vector<std::string> getDefaultAndTooltip(const std::shared_ptr<Element> &element,
                                                       const std::string &attributeName);


private:
  Render();
  std::shared_ptr<Context> context;
};

} // namespace GRM

#endif
