#ifndef RENDER_HXX
#define RENDER_HXX

#include <iostream>
#include <optional>

#include <grm/dom_render/context.hxx>
#include <grm/dom_render/graphics_tree/Element.hxx>
#include <grm/dom_render/graphics_tree/Document.hxx>
#include "gr.h"
#include "grm/layout.hxx"
#include <grm/util.h>

enum class EXPORT CoordinateSpace
{
  WC,
  NDC
};


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
  static std::shared_ptr<Render> createRender();
  std::shared_ptr<Element> createPolymarker(const std::string &x_key, std::optional<std::vector<double>> x,
                                            const std::string &y_key, std::optional<std::vector<double>> y,
                                            const std::shared_ptr<Context> &extContext = nullptr, int marker_type = 0,
                                            double marker_size = 0.0, int marker_colorind = 0);

  std::shared_ptr<Element> createPolymarker(double x, double y, int marker_type = 0, double marker_size = 0.0,
                                            int marker_colorind = 0);

  std::shared_ptr<Element> createPolyline(const std::string &x_key, std::optional<std::vector<double>> x,
                                          const std::string &y_key, std::optional<std::vector<double>> y,
                                          const std::shared_ptr<Context> &extContext = nullptr, int line_type = 0,
                                          double line_width = 0.0, int line_colorind = 0);

  std::shared_ptr<Element> createPolyline(double x1, double x2, double y1, double y2, int line_type = 0,
                                          double line_width = 0.0, int line_colorind = 0);

  std::shared_ptr<Element> createText(double x, double y, const std::string &text,
                                      CoordinateSpace space = CoordinateSpace::NDC);

  std::shared_ptr<Element> createFillArea(const std::string &x_key, std::optional<std::vector<double>> x,
                                          const std::string &y_key, std::optional<std::vector<double>> y,
                                          const std::shared_ptr<Context> &extContext = nullptr, int fillintstyle = 0,
                                          int fillstyle = 0, int fillcolorind = -1);

  std::shared_ptr<Element> createCellArray(double xmin, double xmax, double ymin, double ymax, int dimx, int dimy,
                                           int scol, int srow, int ncol, int nrow, const std::string &color_key,
                                           std::optional<std::vector<int>> color,
                                           const std::shared_ptr<Context> &extContext = nullptr);

  std::shared_ptr<Element> createAxes(double x_tick, double y_tick, double x_org, double y_org, int x_major,
                                      int y_major, int tick_orientation);

  std::shared_ptr<GRM::Element> createEmptyAxes(int tick_orientation);

  std::shared_ptr<GRM::Element> createEmptyDoubleAxes();

  std::shared_ptr<Element> createDrawLegend(const std::string &labels_key,
                                            std::optional<std::vector<std::string>> labels, int location,
                                            const std::string &specs_key, std::optional<std::vector<std::string>> specs,
                                            const std::shared_ptr<GRM::Context> &extContext = nullptr);

  std::shared_ptr<Element> createDrawPolarAxes(int angle_ticks, const std::string &kind, int phiflip,
                                               const std::string &title = "", const std::string &norm = "",
                                               double tick = 0.0, double line_width = 0.0);

  std::shared_ptr<Element> createDrawPieLegend(const std::string &labels_key,
                                               std::optional<std::vector<std::string>> labels,
                                               const std::shared_ptr<GRM::Context> &extContext = nullptr);


  std::shared_ptr<Element> createGrid(double x_tick, double y_tick, double x_org, double y_org, int major_x,
                                      int major_y);

  std::shared_ptr<GRM::Element> createEmptyGrid(bool x_grid, bool y_grid);

  std::shared_ptr<Element> createGroup();

  std::shared_ptr<Element> createGroup(const std::string &name);

  std::shared_ptr<Element> createSeries(const std::string &name);

  std::shared_ptr<Element> createDrawImage(double xmin, double ymin, double xmax, double ymax, int width, int height,
                                           const std::string &data_key, std::optional<std::vector<int>> data, int model,
                                           const std::shared_ptr<Context> &extContext = nullptr);

  std::shared_ptr<Element> createDrawArc(double xmin, double xmax, double ymin, double ymax, double a1, double a2);

  std::shared_ptr<Element> createFillArc(double xmin, double xmax, double ymin, double ymax, double a1, double a2,
                                         int fillintstyle = 0, int fillstyle = 0, int fillcolorind = -1);

  std::shared_ptr<Element> createDrawRect(double xmin, double xmax, double ymin, double ymax);

  std::shared_ptr<Element> createFillRect(double xmin, double xmax, double ymin, double ymax, int fillintstyle = 0,
                                          int fillstyle = 0, int fillcolorind = -1);

  std::shared_ptr<Element> createQuiver(const std::string &x_key, std::optional<std::vector<double>> x,
                                        const std::string &y_key, std::optional<std::vector<double>> y,
                                        const std::string &u_key, std::optional<std::vector<double>> u,
                                        const std::string &v_key, std::optional<std::vector<double>> v, int color,
                                        const std::shared_ptr<Context> &extContext = nullptr);

  std::shared_ptr<Element> createContour(const std::string &px_key, std::optional<std::vector<double>> px,
                                         const std::string &py_key, std::optional<std::vector<double>> py,
                                         const std::string &h_key, std::optional<std::vector<double>> h,
                                         const std::string &pz_key, std::optional<std::vector<double>> pz, int major_h,
                                         const std::shared_ptr<Context> &extContext = nullptr);


  std::shared_ptr<Element> createContourf(const std::string &px_key, std::optional<std::vector<double>> px,
                                          const std::string &py_key, std::optional<std::vector<double>> py,
                                          const std::string &h_key, std::optional<std::vector<double>> h,
                                          const std::string &pz_key, std::optional<std::vector<double>> pz, int major_h,
                                          const std::shared_ptr<Context> &extContext = nullptr);

  std::shared_ptr<Element> createHexbin(int x_length, const std::string &x_key, std::optional<std::vector<double>> x,
                                        const std::string &y_key, std::optional<std::vector<double>> y, int nbins,
                                        const std::shared_ptr<Context> &extContext = nullptr);

  std::shared_ptr<Element> createVolume(int nx, int ny, int nz, const std::string &data_key,
                                        std::optional<std::vector<double>> data, int algorithm, int dmin, int dmax,
                                        const std::shared_ptr<Context> &extContext = nullptr);

  std::shared_ptr<Element> createColorbar(unsigned int colors, const std::shared_ptr<Context> &extContext = nullptr);

  std::shared_ptr<Element> createNonUniformCellArray(const std::string &x_key, std::optional<std::vector<double>> x,
                                                     const std::string &y_key, std::optional<std::vector<double>> y,
                                                     int dimx, int dimy, int scol, int srow, int ncol, int nrow,
                                                     const std::string &color_key,
                                                     std::optional<std::vector<int>> color,
                                                     const std::shared_ptr<Context> &extContext = nullptr);

  std::shared_ptr<Element> createSurface(const std::string &px_key, std::optional<std::vector<double>> px,
                                         const std::string &py_key, std::optional<std::vector<double>> py,
                                         const std::string &pz_key, std::optional<std::vector<double>> pz, int option,
                                         int accelerate, const std::shared_ptr<Context> &extContext = nullptr);

  std::shared_ptr<Element> createGrid3d(double x_tick, double y_tick, double z_tick, double x_org, double y_org,
                                        double z_org, int major_x, int major_y, int major_z);

  std::shared_ptr<GRM::Element> createEmptyGrid3d(bool x_grid, bool y_grid, bool z_grid);


  std::shared_ptr<Element> createAxes3d(double x_tick, double y_tick, double z_tick, double x_org, double y_org,
                                        double z_org, int major_x, int major_y, int major_z, int tick_orientation);

  std::shared_ptr<GRM::Element> createEmptyAxes3d(int tick_orientation);

  std::shared_ptr<Element> createPolyline3d(const std::string &x_key, std::optional<std::vector<double>> x,
                                            const std::string &y_key, std::optional<std::vector<double>> y,
                                            const std::string &z_key, std::optional<std::vector<double>> z,
                                            const std::shared_ptr<Context> &extContext = nullptr);

  std::shared_ptr<Element> createPolymarker3d(const std::string &x_key, std::optional<std::vector<double>> x,
                                              const std::string &y_key, std::optional<std::vector<double>> y,
                                              const std::string &z_key, std::optional<std::vector<double>> z,
                                              const std::shared_ptr<Context> &extContext = nullptr);

  std::shared_ptr<Element>
  createGR3DrawMesh(int mesh, int n, const std::string &positions_key, std::optional<std::vector<double>> positions,
                    const std::string &directions_key, std::optional<std::vector<double>> directions,
                    const std::string &ups_key, std::optional<std::vector<double>> ups, const std::string &colors_key,
                    std::optional<std::vector<double>> colors, const std::string &scales_key,
                    std::optional<std::vector<double>> scales, const std::shared_ptr<Context> &extContext = nullptr);

  std::shared_ptr<GRM::Element>
  createGR3Isosurface(int nx, int ny, int nz, const std::string &data_key, std::optional<std::vector<double>> data,
                      double isovalue, const std::string &color_key, std::optional<std::vector<double>> color,
                      const std::string &strides_key, std::optional<std::vector<int>> strides,
                      const std::shared_ptr<GRM::Context> &extContext = nullptr);

  std::shared_ptr<Element> createClearWS();

  std::shared_ptr<Element> createUpdateWS();

  std::shared_ptr<Element> createDrawGraphics(const std::string &data_key, std::optional<std::vector<int>> data,
                                              const std::shared_ptr<Context> &extContext = nullptr);

  std::shared_ptr<Element> createTriSurface(const std::string &px_key, std::optional<std::vector<double>> px,
                                            const std::string &py_key, std::optional<std::vector<double>> py,
                                            const std::string &pz_key, std::optional<std::vector<double>> pz,
                                            const std::shared_ptr<Context> &extContext = nullptr);

  std::shared_ptr<Element> createTriContour(const std::string &px_key, std::optional<std::vector<double>> px,
                                            const std::string &py_key, std::optional<std::vector<double>> py,
                                            const std::string &pz_key, std::optional<std::vector<double>> pz,
                                            const std::string &levels_key, std::optional<std::vector<double>> levels,
                                            const std::shared_ptr<Context> &extContext = nullptr);

  std::shared_ptr<Element> createTitles3d(const std::string &x, const std::string &y, const std::string &z);

  std::shared_ptr<Element> createGR3Clear();

  std::shared_ptr<Element> createGR3DeleteMesh(int mesh);

  std::shared_ptr<Element> createGR3DrawImage(double xmin, double xmax, double ymin, double ymax, int width, int height,
                                              int drawable_type);

  std::shared_ptr<Element> createShadePoints(const std::string &x_key, std::optional<std::vector<double>> x,
                                             const std::string &y_key, std::optional<std::vector<double>> y, int xform,
                                             int w, int h, const std::shared_ptr<Context> &extContext = nullptr);

  std::shared_ptr<Element> createLayoutGrid(const grm::Grid &grid);

  std::shared_ptr<Element> createLayoutGridElement(const grm::GridElement &gridElement, const grm::Slice &slice);

  std::shared_ptr<Element> createPiePlotTitleRenderElement(std::string title);

  std::shared_ptr<Element> createIsoSurfaceRenderElement(int drawable_type);

  std::shared_ptr<GRM::Element> createPanzoom(double x, double y, double xzoom, double yzoom);

  std::shared_ptr<GRM::Element> createYLine();

  //! Modifierfunctions

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

  void setTextFontPrec(const std::shared_ptr<Element> &element, int font, int prec);

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

  void setSpace3d(const std::shared_ptr<Element> &element, double phi, double theta, double fov,
                  double camera_distance);

  void setSpace(const std::shared_ptr<Element> &element, double zmin, double zmax, int rotation, int tilt);

  void setSelntran(const std::shared_ptr<Element> &element, int transform);

  void setGR3BackgroundColor(const std::shared_ptr<Element> &element, double red, double green, double blue,
                             double alpha);

  void setGR3CameraLookAt(const std::shared_ptr<Element> &element, double camera_x, double camera_y, double camera_z,
                          double center_x, double center_y, double center_z, double up_x, double up_y, double up_z);

  void setTextColorInd(const std::shared_ptr<Element> &element, int index);

  void setBorderColorInd(const std::shared_ptr<Element> &element, int index);

  void selectClipXForm(const std::shared_ptr<Element> &element, int form);

  void setCharHeight(const std::shared_ptr<Element> &element, double height);

  void setTransparency(const std::shared_ptr<Element> &element, double alpha);

  void setResampleMethod(const std::shared_ptr<Element> &element, int resample);

  void setTextEncoding(const std::shared_ptr<Element> &element, int encoding);

  void setProjectionType(const std::shared_ptr<Element> &element, int type);

  void setSubplot(const std::shared_ptr<Element> &element, double xmin, double xmax, double ymin, double ymax);

  void setXTickLabels(std::shared_ptr<GRM::Element> group, const std::string &key,
                      std::optional<std::vector<std::string>> xticklabels,
                      const std::shared_ptr<GRM::Context> &extContext = nullptr);

  void setOriginPosition(const std::shared_ptr<GRM::Element> &element, std::string x_org_pos, std::string y_org_pos);

  void setOriginPosition3d(const std::shared_ptr<GRM::Element> &element, std::string x_org_pos, std::string y_org_pos,
                           std::string z_org_pos);

  void setImshowInformation(const std::shared_ptr<GRM::Element> &element, unsigned int cols, unsigned int rows,
                            std::string img_data_key, std::optional<std::vector<int>> img_data,
                            const std::shared_ptr<GRM::Context> &extContext);

  void setGR3LightParameters(const std::shared_ptr<GRM::Element> &element, double ambient, double diffuse,
                             double specular, double specular_power);

  void render();                                           // render doc and render context
  void render(const std::shared_ptr<Context> &extContext); // render doc and external context
  void render(const std::shared_ptr<Document> &document);  // external doc and render context
  static void render(const std::shared_ptr<Document> &document,
                     const std::shared_ptr<Context> &extContext); // external doc and external context; could be static

  std::shared_ptr<Context> getContext();

  static void processViewport(const std::shared_ptr<GRM::Element> &elem);
  static void processLimits(const std::shared_ptr<GRM::Element> &elem);
  static std::vector<std::string> getDefaultAndTooltip(const std::shared_ptr<Element> &element,
                                                       std::string attributeName);


private:
  Render();
  std::shared_ptr<Context> context;
};

} // namespace GRM

#endif
