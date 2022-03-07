#ifndef RENDER_HXX
#define RENDER_HXX


#include <iostream>
#include <optional>

#include "gr.h"
#include "context.hxx"
#include "GR/Element.hxx"
#include "GR/Document.hxx"


namespace GR
{

class Render : public Document
{
  /*!
   * The GR::Render class is used for creating or modifying elements that can also be processed by this class
   * to create plots etc.
   *
   * GR::Render has an std::shared_ptr GR::Context as a private member for storing certain datatypes
   */

public:
  static std::shared_ptr<Render> createRender();
  std::shared_ptr<Element> createPolymarker(int n, const std::string &x_key, std::optional<std::vector<double>> x,
                                            const std::string &y_key, std::optional<std::vector<double>> y,
                                            const std::shared_ptr<Context> &extContext = nullptr, int marker_type = 0,
                                            double marker_size = 0.0, int marker_colorind = 0);

  std::shared_ptr<Element> createPolyline(int n, const std::string &x_key, std::optional<std::vector<double>> x,
                                          const std::string &y_key, std::optional<std::vector<double>> y,
                                          const std::shared_ptr<Context> &extContext = nullptr, int line_type = 0,
                                          double line_width = 0.0, int line_colorind = 0);

  std::shared_ptr<Element> createText(double x, double y, const std::string &text);

  std::shared_ptr<Element> createFillArea(int n, const std::string &x_key, std::optional<std::vector<double>> x,
                                          const std::string &y_key, std::optional<std::vector<double>> y,
                                          const std::shared_ptr<Context> &extContext = nullptr, int fillintstyle = 0,
                                          int fillstyle = 0, int fillcolorind = 0);

  std::shared_ptr<Element> createCellArray(double xmin, double xmax, double ymin, double ymax, int dimx, int dimy,
                                           int scol, int srow, int ncol, int nrow, const std::string &color_key,
                                           std::optional<std::vector<int>> color,
                                           const std::shared_ptr<Context> &extContext = nullptr);

  std::shared_ptr<Element> createAxes(double x_tick, double y_tick, double x_org, double y_org, int major_x,
                                      int major_y, double tick_size);

  std::shared_ptr<Element> createGroup();

  std::shared_ptr<Element> createDrawImage(double xmin, double ymin, double xmax, double ymax, int width, int height,
                                           std::string &data_key, std::optional<std::vector<int>> data, int model,
                                           const std::shared_ptr<Context> &extContext = nullptr);

  void setViewport(const std::shared_ptr<Element> &element, double xmin, double xmax, double ymin, double ymax);

  void setWindow(const std::shared_ptr<Element> &element, double xmin, double xmax, double ymin, double ymax);

  void setMarkerType(const std::shared_ptr<Element> &element, int type);

  void setMarkerType(const std::shared_ptr<Element> &element, const std::string &types_key,
                     const std::vector<int> &types, const std::shared_ptr<Context> &extContext = nullptr);

  void setMarkerSize(const std::shared_ptr<Element> &element, const std::string &sizes_key,
                     const std::vector<double> &sizes, const std::shared_ptr<Context> &extContext = nullptr);

  void setMarkerSize(const std::shared_ptr<Element> &element, double size);

  void setMarkerColorInd(const std::shared_ptr<Element> &element, const std::string &colorinds_key,
                         const std::vector<int> &colorinds, const std::shared_ptr<Context> &extContext = nullptr);

  void setMarkerColorInd(const std::shared_ptr<Element> &element, int color);

  void setLineType(const std::shared_ptr<Element> &element, const std::string &types_key, const std::vector<int> &types,
                   const std::shared_ptr<Context> &extContext = nullptr);

  void setLineType(const std::shared_ptr<Element> &element, int type);

  void setLineWidth(const std::shared_ptr<Element> &element, const std::string &widths_key,
                    const std::vector<double> &widths, const std::shared_ptr<Context> &extContext = nullptr);

  void setLineWidth(const std::shared_ptr<Element> &element, double width);

  void setLineColorInd(const std::shared_ptr<Element> &element, const std::string &colorinds_key,
                       const std::vector<int> &colorinds, const std::shared_ptr<Context> &extContext = nullptr);

  void setLineColorInd(const std::shared_ptr<Element> &element, int color);

  void setTextFontPrec(const std::shared_ptr<Element> &element, int font, int prec);

  void render();                                           // render doc and render context
  void render(const std::shared_ptr<Context> &extContext); // render doc and external context
  void render(const std::shared_ptr<Document> &document);  // external doc and render context
  static void render(const std::shared_ptr<Document> &document,
                     const std::shared_ptr<Context> &extContext); // external doc and external context; could be static


private:
  Render();
  std::shared_ptr<Context> context;
};
} // namespace GR

#endif