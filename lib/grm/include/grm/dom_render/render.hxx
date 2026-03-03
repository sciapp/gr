#ifndef RENDER_HXX
#define RENDER_HXX

#include <iostream>
#include <optional>
#include <queue>
#include <stack>
#include <functional>

#include <grm/dom_render/context.hxx>
#include <grm/dom_render/graphics_tree/element.hxx>
#include <grm/dom_render/graphics_tree/document.hxx>
#include "gr.h"
#include "manage_z_index.hxx"
#include "grm/layout.hxx"
#include <grm/util.h>
#include <grm/dom_render/group_mask.hxx>


/* ========================= Sets =================================================================================== */

inline std::set<std::string> valid_context_attributes = {"abs_downwards_e",
                                                         "abs_upwards_e",
                                                         "bin_counts",
                                                         "bin_edges",
                                                         "bin_widths",
                                                         "bins",
                                                         "c",
                                                         "c_rgb",
                                                         "c_ind",
                                                         "color_ind_values",
                                                         "color_rgb",
                                                         "color_rgb_values",
                                                         "counts",
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
                                                         "positions",
                                                         "px",
                                                         "py",
                                                         "pz",
                                                         "r",
                                                         "rel_downwards_e",
                                                         "rel_upwards_e",
                                                         "scales",
                                                         "theta",
                                                         "u",
                                                         "v",
                                                         "weights",
                                                         "x",
                                                         "y",
                                                         "y_labels",
                                                         "z",
                                                         "z_dims",
                                                         "_r_org",
                                                         "_x_org",
                                                         "_y_org",
                                                         "_z_org"};


/* ========================= functions ============================================================================== */

namespace GRM
{

GRM_EXPORT void addValidContextKey(std::string key);
GRM_EXPORT const GRM::GroupMask *getGroupMask();
GRM_EXPORT void renderCaller();
GRM_EXPORT void updateContextAttribute(const std::shared_ptr<Element> &element, const std::string &attr,
                                       const Value &old_value);
GRM_EXPORT void deleteContextAttribute(const std::shared_ptr<Element> &element);
GRM_EXPORT void cleanupElement(Element &element);

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

class GRM_EXPORT Render : public Document
{
  /*!
   * The Render class is used for creating or modifying elements that can also be processed by this class
   * to create plots etc.
   *
   * Render has a std::shared_ptr Context as a private member for storing certain datatypes
   */

public:
  static std::shared_ptr<Render> createRender();

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
  void setCharHeight(const std::shared_ptr<Element> &element, double height);
  void setTransparency(const std::shared_ptr<Element> &element, double transparency);
  void setResampleMethod(const std::shared_ptr<Element> &element, int resample);
  void setTextEncoding(const std::shared_ptr<Element> &element, int encoding);
  static void setViewportNormalized(const std::shared_ptr<Element> &element, double xmin, double xmax, double ymin,
                                    double ymax);
  void setOriginPosition(const std::shared_ptr<Element> &element, const std::string &x_org_pos,
                         const std::string &y_org_pos);
  void setOriginPosition3d(const std::shared_ptr<Element> &element, const std::string &x_org_pos,
                           const std::string &y_org_pos, const std::string &z_org_pos);
  void setGR3LightParameters(const std::shared_ptr<Element> &element, double ambient, double diffuse, double specular,
                             double specular_power);
  static void setAutoUpdate(bool update);
  static void setEnableEditor(bool update);
  void setActiveFigure(const std::shared_ptr<Element> &element);
  void setFirstCall(bool call);
  void setPreviousScatterMarkerType(int *previous_scatter_marker_type);
  void setPreviousLineMarkerType(int *previous_line_marker_type);

  /* ------------------------------- getter functions ----------------------------------------------------------------*/

  std::shared_ptr<Element> getActiveFigure();
  static void getAutoUpdate(bool *update);
  static void getEnableEditor(bool *update);
  bool getFirstCall();
  bool getZQueueIsBeingRendered();
  ManageZIndex *getZIndexManager();
  bool getRedrawWs();
  bool getHighlightedAttrExist();
  int *getPreviousScatterMarkerType();
  int *getPreviousLineMarkerType();
  static bool getViewport(const std::shared_ptr<Element> &element, double *xmin, double *xmax, double *ymin,
                          double *ymax);
  std::shared_ptr<Context> getRenderContext();

  void render();                                            // render doc and render context
  void render(const std::shared_ptr<Context> &ext_context); // render doc and external context
  void render(const std::shared_ptr<Document> &document);   // external doc and render context
  static void render(const std::shared_ptr<Document> &document,
                     const std::shared_ptr<Context> &ext_context); // external doc and external context; could be static
  void processTree();
  void renderMaskHighlight(std::shared_ptr<Element> highlighted_element,
                           void (*image_callback)(int, unsigned int, unsigned int, unsigned int, unsigned int,
                                                  unsigned int *));
  static void finalize();
  std::shared_ptr<Context> getContext();
  static std::vector<std::string> getDefaultAndTooltip(const std::shared_ptr<Element> &element,
                                                       const std::string &attribute_name);

  /* ----------------------- history -------------------------------------------------------------------------------- */

  const char *initializeHistory();

private:
  Render();
  std::shared_ptr<Context> context;
};

} // namespace GRM

#endif
