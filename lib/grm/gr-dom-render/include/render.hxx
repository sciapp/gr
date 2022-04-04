#ifndef RENDER_HXX_INCLUDED
#define RENDER_HXX_INCLUDED

#include "iostream"
#include "gr.h"
#include "context.hxx"
#include "GR/Element.hxx"
#include "GR/Document.hxx"


using namespace GR;

class Render : public Document
{

public:
  static std::shared_ptr<Render> createRender();
  std::shared_ptr<Element> createPolymarker(int n, const std::vector<double> &x, const std::vector<double> &y,
                                            const std::shared_ptr<Context> &extContext = nullptr, int marker_type = 0,
                                            double marker_size = 0.0, int marker_colorind = 0);

  std::shared_ptr<Element> createPolyline(int n, const std::vector<double> &x, const std::vector<double> &y,
                                          const std::shared_ptr<Context> &extContext = nullptr, int line_type = 0,
                                          double line_width = 0.0, int line_colorind = 0);
  std::shared_ptr<Element> createText(double x, double y, const std::string &text);
  std::shared_ptr<Element> createFillArea(int n, const std::vector<double> &x, const std::vector<double> &y,
                                          const std::shared_ptr<Context> &extContext = nullptr, int fillintstyle = 0,
                                          int fillstyle = 0, int fillcolorind = 0);
  std::shared_ptr<Element> createCellArray(double xmin, double xmax, double ymin, double ymax, int dimx, int dimy,
                                           int scol, int srow, int ncol, int nrow, const std::vector<int> &color,
                                           const std::shared_ptr<Context> &extContext = nullptr);
  std::shared_ptr<Element> createAxes(double x_tick, double y_tick, double x_org, double y_org, int major_x,
                                      int major_y, double tick_size);
  std::shared_ptr<Element> createSetWindow(double xmin, double xmax, double ymin, double ymax);
  std::shared_ptr<Element> createGroup();
  std::shared_ptr<Element> createDrawImage(double xmin, double ymin, double xmax, double ymax, int width, int height,
                                           const std::vector<int> &data, int model,
                                           const std::shared_ptr<Context> &extContext = nullptr);


  void setMarkerType(const std::shared_ptr<Element> &element, int type);
  void setMarkerType(const std::shared_ptr<Element> &element, const std::vector<int> &types,
                     const std::shared_ptr<Context> &extContext = nullptr);

  void setMarkerSize(const std::shared_ptr<Element> &element, const std::vector<double> &sizes,
                     const std::shared_ptr<Context> &extContext = nullptr);
  void setMarkerSize(const std::shared_ptr<Element> &element, double size);

  void setMarkerColorInd(const std::shared_ptr<Element> &element, const std::vector<int> &colorinds,
                         const std::shared_ptr<Context> &extContext = nullptr);
  void setMarkerColorInd(const std::shared_ptr<Element> &element, int color);

  void setLineType(const std::shared_ptr<Element> &element, const std::vector<int> &types,
                   const std::shared_ptr<Context> &extContext = nullptr);
  void setLineType(const std::shared_ptr<Element> &element, int type);

  void setLineWidth(const std::shared_ptr<Element> &element, const std::vector<double> &widths,
                    const std::shared_ptr<Context> &extContext = nullptr);
  void setLineWidth(const std::shared_ptr<Element> &element, double width);

  void setLineColorInd(const std::shared_ptr<Element> &element, const std::vector<int> &colorinds,
                       const std::shared_ptr<Context> &extContext = nullptr);
  void setLineColorInd(const std::shared_ptr<Element> &element, int color);

  // TODO: static or out of class
  static void setTextFontPrec(const std::shared_ptr<Element> &element, int font, int prec);

  void render();                                           // render doc and render context
  void render(const std::shared_ptr<Context> &extContext); // render doc and external context
  void render(const std::shared_ptr<Document> &document);  // external doc and render context
  static void render(const std::shared_ptr<Document> &document,
                     const std::shared_ptr<Context> &extContext); // external doc and external context; could be static


private:
  Render();
  std::shared_ptr<Context> context;
};

#endif /* ifndef RENDER_HXX_INCLUDED */
