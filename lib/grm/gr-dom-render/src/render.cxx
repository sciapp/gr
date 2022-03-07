#include "GR/Element.hxx"
#include "GR/Document.hxx"
#include "GR/Value.hxx"
#include "GR/util.hxx"
#include "gr.h"
#include "render.hxx"
#include "NotFoundError.h"
#include <vector>
#include "context.hxx"
#include <functional>


static void markerHelper(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context,
                         const std::string &str)
{
  /*!
   * Helperfunction for marker functions using vectors for marker parameters
   *
   * \param[in] element The GR::Element that contains all marker attributes and data keys. If element's parent is a
   * group element it may fallback to its marker attributes
   * \param[in] context The GR::Context that contains the actual data
   * \param[in] str The std::string that specifies what GR Routine should be called (polymarker)
   *
   */
  std::vector<int> type, colorind;
  std::vector<double> size;

  auto parent = element->parentElement();
  bool group = parent->localName() == "group";

  auto attr = element->getAttribute("markertypes");
  if (attr.isString())
    {
      type = GR::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
    }
  else if (group)
    {
      attr = parent->getAttribute("markertypes");
      if (attr.isString())
        {
          type = GR::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
        }
    }

  attr = element->getAttribute("markercolorinds");
  if (attr.isString())
    {
      colorind = GR::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
    }
  else if (group)
    {
      attr = parent->getAttribute("markercolorinds");
      if (attr.isString())
        {
          colorind = GR::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
        }
    }

  attr = element->getAttribute("markersizes");
  if (attr.isString())
    {
      size = GR::get<std::vector<double>>((*context)[static_cast<std::string>(attr)]);
    }
  else if (group)
    {
      attr = parent->getAttribute("markersizes");
      if (attr.isString())
        {
          size = GR::get<std::vector<double>>((*context)[static_cast<std::string>(attr)]);
        }
    }

  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));

  for (int i = 0; i < GR::get<std::vector<double>>((*context)[x]).size(); ++i)
    {
      //! fallback to the last element when lists are too short;
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
      if (!colorind.empty())
        {
          if (colorind.size() > i)
            {
              gr_setmarkercolorind(colorind[i]);
            }
          else
            {
              gr_setmarkercolorind(colorind.back());
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

      if (str == "polymarker")
        {
          gr_polymarker(1, (double *)&(GR::get<std::vector<double>>((*context)[x])[i]),
                        (double *)&(GR::get<std::vector<double>>((*context)[y])[i]));
        }
    }
}

static void lineHelper(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context,
                       const std::string &str)
{
  /*!
   * Helperfunction for line functions using vectors for line parameters
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   * \param[in] str The std::string that specifies what GR Routine should be called (polyline)
   *
   *
   */
  std::vector<int> type, colorind;
  std::vector<double> width;

  auto parent = element->parentElement();
  bool group = parent->localName() == "group";

  auto attr = element->getAttribute("linetypes");
  if (attr.isString())
    {
      type = GR::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
    }
  else if (group)
    {
      attr = parent->getAttribute("linetypes");
      if (attr.isString())
        {
          type = GR::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
        }
    }

  attr = element->getAttribute("linecolorinds");
  if (attr.isString())
    {
      colorind = GR::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
    }
  else if (group)
    {
      attr = parent->getAttribute("linecolorinds");
      if (attr.isString())
        {
          colorind = GR::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
        }
    }

  attr = element->getAttribute("linewidths");
  if (attr.isString())
    {
      width = GR::get<std::vector<double>>((*context)[static_cast<std::string>(attr)]);
    }
  else if (group)
    {
      attr = parent->getAttribute("linewidths");
      if (attr.isString())
        {
          width = GR::get<std::vector<double>>((*context)[static_cast<std::string>(attr)]);
        }
    }

  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));
  for (int i = 0; i < GR::get<std::vector<double>>((*context)[x]).size() - 1; ++i)
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
      if (!colorind.empty())
        {
          if (colorind.size() > i)
            {
              gr_setlinecolorind(colorind[i]);
            }
          else
            {
              gr_setlinecolorind(colorind.back());
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
      if (str == "polyline")
        {
          gr_polyline(2, (double *)&(GR::get<std::vector<double>>((*context)[x])[i]),
                      (double *)&(GR::get<std::vector<double>>((*context)[y])[i]));
        }
    }
}


static void polymarker(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processing function for polymarker
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  int n = static_cast<int>(element->getAttribute("n"));
  if (element->getAttribute("x").isString() && element->getAttribute("y").isString())
    {
      auto x = static_cast<std::string>(element->getAttribute("x"));
      auto y = static_cast<std::string>(element->getAttribute("y"));
      auto group = element->parentElement();
      if ((element->hasAttribute("markertypes") || element->hasAttribute("markersizes") ||
           element->hasAttribute("markercolorinds")) ||
          (group->localName() == "group" && (group->hasAttribute("markertypes") || group->hasAttribute("markersizes") ||
                                             group->hasAttribute("markercolorinds"))))
        {
          markerHelper(element, context, "polymarker");
        }
      else
        {
          gr_polymarker(n, (double *)&(GR::get<std::vector<double>>((*context)[x])[0]),
                        (double *)&(GR::get<std::vector<double>>((*context)[y])[0]));
        }
    }
  else if (element->getAttribute("x").isDouble())
    {
      auto x = static_cast<double>(element->getAttribute("x"));
      auto y = static_cast<double>(element->getAttribute("y"));
      gr_polymarker(n, &x, &y);
    }
}

static void polyline(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processing Function for polyline
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  int n = static_cast<int>(element->getAttribute("n"));
  if (element->getAttribute("x").isString() && element->getAttribute("y").isString())
    {
      auto x = static_cast<std::string>(element->getAttribute("x"));
      auto y = static_cast<std::string>(element->getAttribute("y"));
      auto group = element->parentElement();
      if ((element->hasAttribute("linetypes") || element->hasAttribute("linewidths") ||
           element->hasAttribute("linecolorinds")) ||
          ((group->localName() == "group") && (group->hasAttribute("linetypes") || group->hasAttribute("linewidths") ||
                                               group->hasAttribute("linecolorinds"))))
        {
          lineHelper(element, context, "polyline");
        }
      else
        gr_polyline(n, (double *)&(GR::get<std::vector<double>>((*context)[x])[0]),
                    (double *)&(GR::get<std::vector<double>>((*context)[y])[0]));
    }
  else if (element->getAttribute("x").isDouble())
    {
      auto x = static_cast<double>(element->getAttribute("x"));
      auto y = static_cast<double>(element->getAttribute("y"));
      gr_polyline(n, &x, &y);
    }
}

static void text(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processing funcions for text
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  auto x = static_cast<double>(element->getAttribute("x"));
  auto y = static_cast<double>(element->getAttribute("y"));
  auto str = static_cast<std::string>(element->getAttribute("text"));
  gr_text(x, y, &str[0]);
}

static void fillArea(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processing function for fillArea
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  auto n = static_cast<int>(element->getAttribute("n"));
  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));
  gr_fillarea(n, (double *)&(GR::get<std::vector<double>>((*context)[x])[0]),
              (double *)&(GR::get<std::vector<double>>((*context)[y])[0]));
}

static void cellArray(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processing function for cellArray
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  double xmin = static_cast<double>(element->getAttribute("xmin"));
  double xmax = static_cast<double>(element->getAttribute("xmax"));
  double ymin = static_cast<double>(element->getAttribute("ymin"));
  double ymax = static_cast<double>(element->getAttribute("ymax"));
  int dimx = static_cast<int>(element->getAttribute("dimx"));
  int dimy = static_cast<int>(element->getAttribute("dimy"));
  int scol = static_cast<int>(element->getAttribute("scol"));
  int srow = static_cast<int>(element->getAttribute("srow"));
  int ncol = static_cast<int>(element->getAttribute("ncol"));
  int nrow = static_cast<int>(element->getAttribute("nrow"));
  auto color = static_cast<std::string>(element->getAttribute("color"));
  gr_cellarray(xmin, xmax, ymin, ymax, dimx, dimy, scol, srow, ncol, nrow,
               (int *)&(GR::get<std::vector<int>>((*context)[color])[0]));
}

static void axes(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processing function for axes
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  double x_tick = static_cast<double>(element->getAttribute("x_tick"));
  double y_tick = static_cast<double>(element->getAttribute("y_tick"));
  double x_org = static_cast<double>(element->getAttribute("x_org"));
  double y_org = static_cast<double>(element->getAttribute("y_org"));
  int major_x = static_cast<int>(element->getAttribute("major_x"));
  int major_y = static_cast<int>(element->getAttribute("major_y"));
  double tick_size = static_cast<double>(element->getAttribute("tick_size"));
  gr_axes(x_tick, y_tick, x_org, y_org, major_x, major_y, tick_size);
}

static void drawImage(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processing function for drawImage
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  double xmin = static_cast<double>(element->getAttribute("xmin"));
  double xmax = static_cast<double>(element->getAttribute("xmax"));
  double ymin = static_cast<double>(element->getAttribute("ymin"));
  double ymax = static_cast<double>(element->getAttribute("ymax"));
  int width = static_cast<int>(element->getAttribute("width"));
  int height = static_cast<int>(element->getAttribute("height"));
  int model = static_cast<int>(element->getAttribute("model"));
  auto data = static_cast<std::string>(element->getAttribute("data"));
  gr_drawimage(xmin, ymin, xmax, ymax, width, height, (int *)&(GR::get<std::vector<int>>((*context)[data])[0]), model);
}

static void processWindow(const std::shared_ptr<GR::Element> &elem)
{
  /*!
   * Procesing function for gr_window
   *
   * \param[in] element The GR::Element that contains the attributes
   */
  double xmin, xmax, ymin, ymax;
  xmin = (double)elem->getAttribute("window_xmin");
  xmax = (double)elem->getAttribute("window_xmax");
  ymin = (double)elem->getAttribute("window_ymin");
  ymax = (double)elem->getAttribute("window_ymax");
  gr_setwindow(xmin, xmax, ymin, ymax);
}


static void processViewport(const std::shared_ptr<GR::Element> &elem)
{
  /*!
   * Procesing function for gr_viewport
   *
   * \param[in] element The GR::Element that contains the attributes
   */
  double xmin, xmax, ymin, ymax;
  xmin = (double)elem->getAttribute("viewport_xmin");
  xmax = (double)elem->getAttribute("viewport_xmax");
  ymin = (double)elem->getAttribute("viewport_ymin");
  ymax = (double)elem->getAttribute("viewport_ymax");
  gr_setviewport(xmin, xmax, ymin, ymax);
}


static void processAttributes(const std::shared_ptr<GR::Element> &element)
{
  /*!
   * processing function for all kinds of attributes
   *
   * \param[in] element The GR::Element containing attributes
   */

  //! Map used for processing all kinds of attributes
  static std::map<std::string, std::function<void(const std::shared_ptr<GR::Element> &)>> attrStringToFunc{
      {std::string("markertype"),
       [](const std::shared_ptr<GR::Element> &elem) { gr_setmarkertype((int)elem->getAttribute("markertype")); }},
      {std::string("markercolorind"),
       [](const std::shared_ptr<GR::Element> &elem) {
         gr_setmarkercolorind((int)elem->getAttribute("markercolorind"));
       }},
      {std::string("markersize"),
       [](const std::shared_ptr<GR::Element> &elem) { gr_setmarkersize((double)elem->getAttribute("markersize")); }},
      {std::string("linetype"),
       [](const std::shared_ptr<GR::Element> &elem) { gr_setlinetype((int)elem->getAttribute("linetype")); }},
      {std::string("linecolorind"),
       [](const std::shared_ptr<GR::Element> &elem) { gr_setlinecolorind((int)elem->getAttribute("linecolorind")); }},
      {std::string("linewidth"),
       [](const std::shared_ptr<GR::Element> &elem) { gr_setlinewidth((double)elem->getAttribute("linewidth")); }},
      {std::string("fillintstyle"),
       [](const std::shared_ptr<GR::Element> &elem) { gr_setfillintstyle((int)elem->getAttribute("fillintstyle")); }},
      {std::string("fillstyle"),
       [](const std::shared_ptr<GR::Element> &elem) { gr_setfillstyle((int)elem->getAttribute("fillstyle")); }},
      {std::string("fillcolorind"),
       [](const std::shared_ptr<GR::Element> &elem) { gr_setfillcolorind((int)elem->getAttribute("fillcolorind")); }},
      {std::string("textfontprec"),
       [](const std::shared_ptr<GR::Element> &elem) {
         gr_settextfontprec((int)elem->getAttribute("textfontprec_font"), (int)elem->getAttribute("textfontprec_prec"));
       }},
      {std::string("charexpan"),
       [](const std::shared_ptr<GR::Element> &elem) { gr_setcharexpan((double)elem->getAttribute("charexpan")); }},
      {std::string("charspace"),
       [](const std::shared_ptr<GR::Element> &elem) { gr_setcharspace((double)elem->getAttribute("charspace")); }},
      {std::string("textcolorind"),
       [](const std::shared_ptr<GR::Element> &elem) { gr_settextcolorind((int)elem->getAttribute("textcolorind")); }},
      {std::string("charheight"),
       [](const std::shared_ptr<GR::Element> &elem) { gr_setcharheight((double)elem->getAttribute("charheight")); }},
      {std::string("charup"),
       [](const std::shared_ptr<GR::Element> &elem) {
         gr_setcharup((double)elem->getAttribute("charup_ux"), (double)elem->getAttribute("charup_uy"));
       }},
      {std::string("textpath"),
       [](const std::shared_ptr<GR::Element> &elem) { gr_settextpath((int)elem->getAttribute("textpath")); }},
      {std::string("textalign"),
       [](const std::shared_ptr<GR::Element> &elem) {
         gr_settextalign((int)elem->getAttribute("textalign_horizontal"),
                         (int)elem->getAttribute("textalign_vertical"));
       }},
      {std::string("window"), processWindow},
      {std::string("viewport"), processViewport}};
  for (auto &pair : attrStringToFunc)
    {
      if (element->hasAttribute(pair.first))
        {
          pair.second(element);
        }
    }
}


static void processElement(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processing function for all kinds of elements
   *
   * \param[in] element The GR::Element containing attributes and data keys
   * \param[in] context The GR::Context containing the actual data
   */

  //! Map used for processing all kinds of elements
  static std::map<std::string,
                  std::function<void(const std::shared_ptr<GR::Element> &, const std::shared_ptr<GR::Context> &)>>
      elemStringToFunc{
          {std::string("polymarker"), polymarker}, {std::string("polyline"), polyline},   {std::string("text"), text},
          {std::string("fillArea"), fillArea},     {std::string("cellArray"), cellArray}, {std::string("axes"), axes},
          {std::string("drawImage"), drawImage}};

  /*! Modifier */
  if (element->localName() == "group")
    {
      processAttributes(element);
    }
  else
    {
      /*! Drawnodes */
      gr_savestate();
      processAttributes(element);
      std::function<void(const std::shared_ptr<GR::Element> &, const std::shared_ptr<GR::Context> &)> f =
          elemStringToFunc[element->localName()];
      f(element, context);
      gr_restorestate();
    }
}


//! Create Functions

std::shared_ptr<GR::Element> GR::Render::createPolymarker(int n, const std::string &x_key,
                                                          std::optional<std::vector<double>> x,
                                                          const std::string &y_key,
                                                          std::optional<std::vector<double>> y,
                                                          const std::shared_ptr<Context> &extContext, int marker_type,
                                                          double marker_size, int marker_colorind)
{
  /*!
   * This function can be used to create a Polymarker GR::Element
   *
   * \param[in] n The number of data points
   * \param[in] x_key A string used for storing the x coordinates in GR::Context
   * \param[in] x A vector containing double values representing x coordinates
   * \param[in] y_key A string used for storing the y coordinates in GR::Context
   * \param[in] y A vector containing double values representing y coordinates
   * \param[in] extContext A GR::Context that is used for storing the vectors. By default it uses GR::Render's
   * GR::Context object but an external GR::Context can be used \param[in] marker_type An Integer setting the
   * gr_markertype. By default it is 0 \param[in] marker_size A Double value setting the gr_markersize. By default it is
   * 0.0 \param[in] marker_colorind An Integer setting the gr_markercolorind. By default it is 0
   */

  std::shared_ptr<Context> useContext = (extContext == nullptr) ? context : extContext;
  auto polymarker_element = createElement("polymarker");
  polymarker_element->setAttribute("n", n);
  if (x != std::nullopt)
    {
      (*useContext)[x_key] = x.value();
    }
  polymarker_element->setAttribute("x", x_key);

  if (y != std::nullopt)
    {
      (*useContext)[y_key] = y.value();
    }
  polymarker_element->setAttribute("y", y_key);

  if (marker_type != 0)
    {
      polymarker_element->setAttribute("markertype", marker_type);
    }
  if (marker_size != 0.0)
    {
      polymarker_element->setAttribute("markersize", marker_size);
    }
  if (marker_colorind != 0)
    {
      polymarker_element->setAttribute("markercolorind", marker_colorind);
    }
  return polymarker_element;
}


std::shared_ptr<GR::Element> GR::Render::createPolyline(int n, const std::string &x_key,
                                                        std::optional<std::vector<double>> x, const std::string &y_key,
                                                        std::optional<std::vector<double>> y,
                                                        const std::shared_ptr<Context> &extContext, int line_type,
                                                        double line_width, int line_colorind)
{
  /*!
   * This function can be used to create a Polyline GR::Element
   *
   * \param[in] n The number of data points
   * \param[in] x_key A string used for storing the x coordinates in GR::Context
   * \param[in] x A vector containing double values representing x coordinates
   * \param[in] y_key A string used for storing the y coordinates in GR::Context
   * \param[in] y A vector containing double values representing y coordinates
   * \param[in] extContext A GR::Context that is used for storing the vectors. By default it uses GR::Render's
   * GR::Context object but an external GR::Context can be used \param[in] line_type An Integer setting the
   * gr_linertype. By default it is 0 \param[in] line_width A Double value setting the gr_linewidth. By default it is
   * 0.0 \param[in] marker_colorind An Integer setting the gr_linecolorind. By default it is 0
   */

  std::shared_ptr<Context> useContext = (extContext == nullptr) ? context : extContext;
  auto element = createElement("polyline");
  element->setAttribute("n", n);
  if (x != std::nullopt)
    {
      (*useContext)[x_key] = *x;
    }
  element->setAttribute("x", x_key);
  if (y != std::nullopt)
    {
      (*useContext)[y_key] = *y;
    }
  element->setAttribute("y", y_key);
  if (line_type != 0)
    {
      element->setAttribute("linetype", line_type);
    }
  if (line_width != 0.0)
    {
      element->setAttribute("linewidth", line_width);
    }
  if (line_colorind != 0)
    {
      element->setAttribute("linecolorind", line_colorind);
    }
  return element;
}


std::shared_ptr<GR::Element> GR::Render::createText(double x, double y, const std::string &text)
{
  /*!
   * This function can be used to create a Text GR::Element
   *
   * \param[in] x A double value representing the x coordinate
   * \param[in] y A double value representing the y coordinate
   * \param[in] text A string
   */
  auto element = createElement("text");
  element->setAttribute("x", x);
  element->setAttribute("y", y);
  element->setAttribute("text", text);
  return element;
}


std::shared_ptr<GR::Element> GR::Render::createFillArea(int n, const std::string &x_key,
                                                        std::optional<std::vector<double>> x, const std::string &y_key,
                                                        std::optional<std::vector<double>> y,
                                                        const std::shared_ptr<Context> &extContext, int fillintstyle,
                                                        int fillstyle, int fillcolorind)
{
  /*!
   * This function can be used to create a FillArea GR::Element
   *
   * \param[in] n The number of data points
   * \param[in] x_key A string used for storing the x coordinates in GR::Context
   * \param[in] x A vector containing double values representing x coordinates
   * \param[in] y_key A string used for storing the y coordinates in GR::Context
   * \param[in] y A vector containing double values representing y coordinates
   * \param[in] extContext A GR::Context that is used for storing the vectors. By default it uses GR::Render's
   * GR::Context object but an external GR::Context can be used \param[in] fillintstyle An Integer setting the
   * gr_fillintstyle. By default it is 0 \param[in] fillstyle An Integer setting the gr_fillstyle. By default it is 0
   */

  std::shared_ptr<Context> useContext = (extContext == nullptr) ? context : extContext;
  auto element = createElement("fillArea");
  element->setAttribute("n", n);
  if (x != std::nullopt)
    {
      (*useContext)[x_key] = *x;
    }
  element->setAttribute("x", x_key);
  if (y != std::nullopt)
    {
      (*useContext)[y_key] = *y;
    }
  element->setAttribute("y", y_key);

  if (fillintstyle != 0)
    {
      element->setAttribute("fillintstyle", fillintstyle);
    }
  if (fillstyle != 0)
    {
      element->setAttribute("fillstyle", fillstyle);
    }
  if (fillcolorind != 0)
    {
      element->setAttribute("fillcolorind", fillcolorind);
    }
  return element;
}


std::shared_ptr<GR::Element> GR::Render::createCellArray(double xmin, double xmax, double ymin, double ymax, int dimx,
                                                         int dimy, int scol, int srow, int ncol, int nrow,
                                                         const std::string &color_key,
                                                         std::optional<std::vector<int>> color,
                                                         const std::shared_ptr<Context> &extContext)
{
  /*!
   * This function can be used to create a CellArray GR::Element
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
   * \param[in] extContext A GR::Context used for storing color. By default it uses GR::Render's GR::Context object but
   * an external GR::Context can be used
   */

  std::shared_ptr<Context> useContext = (extContext == nullptr) ? context : extContext;
  auto element = createElement("cellArray");
  element->setAttribute("xmin", xmin);
  element->setAttribute("xmax", xmax);
  element->setAttribute("ymin", ymin);
  element->setAttribute("ymin", ymax);
  element->setAttribute("dimx", dimx);
  element->setAttribute("dimy", dimy);
  element->setAttribute("scol", scol);
  element->setAttribute("srow", srow);
  element->setAttribute("ncol", ncol);
  element->setAttribute("nrow", nrow);
  element->setAttribute("color", color_key);
  if (color != std::nullopt)
    {
      (*useContext)[color_key] = *color;
    }
  return element;
}

std::shared_ptr<GR::Element> GR::Render::createAxes(double x_tick, double y_tick, double x_org, double y_org,
                                                    int major_x, int major_y, double tick_size)
{
  /*!
   * This function can be used for creating an Axes GR::Element
   *
   * \param[in] x_tick A double value
   * \param[in] y_tick A double value
   * \param[in] x_org A double value
   * \param[in] y_org A double value
   * \param[in] major_x An Integer value
   * \param[in] major_y An Integer value
   * \param[in] tick_size A Double value
   */
  auto element = createElement("axes");
  element->setAttribute("x_tick", x_tick);
  element->setAttribute("y_tick", y_tick);
  element->setAttribute("x_ord", x_org);
  element->setAttribute("y_org", y_org);
  element->setAttribute("major_x", major_x);
  element->setAttribute("major_y", major_y);
  element->setAttribute("tick_size", tick_size);
  return element;
}


std::shared_ptr<GR::Element> GR::Render::createGroup()
{
  /*!
   * This function can be used to create a group GR::Element
   *
   * A group Element sets all attributes for its children
   */
  return createElement("group");
}


std::shared_ptr<GR::Element> GR::Render::createDrawImage(double xmin, double ymin, double xmax, double ymax, int width,
                                                         int height, std::string &data_key,
                                                         std::optional<std::vector<int>> data, int model,
                                                         const std::shared_ptr<Context> &extContext)
{
  /*!
   * This function can be used to create a DrawImage GR::Element
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
   * \param[in] extContext A GR::Context used for storing data. By default it uses GR::Render's GR::Context object but
   * an external GR::Context can be used
   */
  std::shared_ptr<Context> useContext = (extContext == nullptr) ? context : extContext;
  auto element = createElement("drawImage");
  element->setAttribute("xmin", xmin);
  element->setAttribute("xmax", xmax);
  element->setAttribute("ymin", ymin);
  element->setAttribute("ymin", ymax);
  element->setAttribute("width", width);
  element->setAttribute("height", height);
  element->setAttribute("model", model);
  element->setAttribute("data", data_key);
  if (data != std::nullopt)
    {
      (*useContext)[data_key] = *data;
    }
  return element;
}


//! Modifierfunctions

void GR::Render::setViewport(const std::shared_ptr<Element> &element, double xmin, double xmax, double ymin,
                             double ymax)
{
  /*!
   * This function can be used to set the window of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] xmin The left horizontal coordinate of the viewport (0 <= xmin < xmax)
   * \param[in] xmax The right horizontal coordinate of the viewport (xmin < xmax <= 1)
   * \param[in] ymin TThe bottom vertical coordinate of the viewport (0 <= ymin < ymax)
   * \param[in] ymax The top vertical coordinate of the viewport (ymin < ymax <= 1)
   */

  element->setAttribute("viewport", true);
  element->setAttribute("viewport_xmin", xmin);
  element->setAttribute("viewport_xmax", xmax);
  element->setAttribute("viewport_ymin", ymin);
  element->setAttribute("viewport_ymax", ymax);
}


void GR::Render::setWindow(const std::shared_ptr<Element> &element, double xmin, double xmax, double ymin, double ymax)
{
  /*!
   * This function can be used to set the window of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] xmin The left horizontal coordinate of the window (xmin < xmax)
   * \param[in] xmax The right horizontal coordinate of the window (xmin < xmax)
   * \param[in] ymin The bottom vertical coordinate of the window (ymin < ymax)
   * \param[in] ymax The top vertical coordinate of the window (ymin < ymax)
   */

  element->setAttribute("window", true);
  element->setAttribute("window_xmin", xmin);
  element->setAttribute("window_xmax", xmax);
  element->setAttribute("window_ymin", ymin);
  element->setAttribute("window_ymax", ymax);
}

void GR::Render::setMarkerType(const std::shared_ptr<Element> &element, int type)
{
  /*!
   * This function can be used to set a MarkerType of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] type An Integer setting the MarkerType
   */
  element->setAttribute("markertype", type);
}


void GR::Render::setMarkerType(const std::shared_ptr<Element> &element, const std::string &types_key,
                               const std::vector<int> &types, const std::shared_ptr<Context> &extContext)
{
  /*!
   * This function can be used to set a vector of MarkerTypes of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] types_key A string used as a key for storing the types
   * \param[in] types A vector containing the MarkerTypes
   * \param[in] extContext A GR::Context used for storing types. By default it uses GR::Render's GR::Context object but
   * an external GR::Context can be used
   */
  std::shared_ptr<Context> useContext = (extContext == nullptr) ? context : extContext;
  (*useContext)[types_key] = types;
  element->setAttribute("markertypes", types_key);
}


void GR::Render::setMarkerSize(const std::shared_ptr<Element> &element, double size)
{
  /*!
   * This function can be used to set a MarkerSize of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] type A Double setting the MarkerSize
   */
  element->setAttribute("markersize", size);
}

void GR::Render::setMarkerSize(const std::shared_ptr<Element> &element, const std::string &sizes_key,
                               const std::vector<double> &sizes, const std::shared_ptr<Context> &extContext)
{
  /*!
   * This function can be used to set a vector of MarkerTypes of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] sizes_key A string used as a key for storing the sizes
   * \param[in] sizes A vector containing the MarkerSizes
   * \param[in] extContext A GR::Context used for storing sizes. By default it uses GR::Render's GR::Context object but
   * an external GR::Context can be used
   */
  std::shared_ptr<Context> useContext = (extContext == nullptr) ? context : extContext;
  (*useContext)[sizes_key] = sizes;
  element->setAttribute("markersizes", sizes_key);
}
void GR::Render::setMarkerColorInd(const std::shared_ptr<Element> &element, int color)
{
  /*!
   * This function can be used to set a MarkerCololrInd of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] color An Integer setting the MarkerColorInd
   */
  element->setAttribute("markercolorind", color);
}

void GR::Render::setMarkerColorInd(const std::shared_ptr<Element> &element, const std::string &colorinds_key,
                                   const std::vector<int> &colorinds, const std::shared_ptr<Context> &extContext)
{
  /*!
   * This function can be used to set a vector of MarkerColorInds of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] colorinds_key A string used as a key for storing the colorinds
   * \param[in] colorinds A vector containing the MarkerColorInds
   * \param[in] extContext A GR::Context used for storing colorinds. By default it uses GR::Render's GR::Context object
   * but an external GR::Context can be used
   */
  std::shared_ptr<Context> useContext = (extContext == nullptr) ? context : extContext;
  (*useContext)[colorinds_key] = colorinds;
  element->setAttribute("markercolorinds", colorinds_key);
}


void GR::Render::setLineType(const std::shared_ptr<Element> &element, const std::string &types_key,
                             const std::vector<int> &types, const std::shared_ptr<Context> &extContext)
{
  /*!
   * This function can be used to set a vector of LineTypes of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] types_key A string used as a key for storing the types
   * \param[in] types A vector containing the LineTypes
   * \param[in] extContext A GR::Context used for storing types. By default it uses GR::Render's GR::Context object but
   * an external GR::Context can be used
   */
  std::shared_ptr<Context> useContext = (extContext == nullptr) ? context : extContext;
  (*useContext)[types_key] = types;
  element->setAttribute("linetypes", types_key);
}

void GR::Render::setLineType(const std::shared_ptr<Element> &element, int type)
{
  /*!
   * This function can be used to set a LineType of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] type An Integer setting the LineType
   */
  element->setAttribute("linetype", type);
}


void GR::Render::setLineWidth(const std::shared_ptr<Element> &element, const std::string &widths_key,
                              const std::vector<double> &widths, const std::shared_ptr<Context> &extContext)
{
  /*!
   * This function can be used to set a vector of LineWidths of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] widths_key A string used as a key for storing the widths
   * \param[in] widths A vector containing the LineWidths
   * \param[in] extContext A GR::Context used for storing widths. By default it uses GR::Render's GR::Context object but
   * an external GR::Context can be used
   */
  std::shared_ptr<Context> useContext = (extContext == nullptr) ? context : extContext;
  (*useContext)[widths_key] = widths;
  element->setAttribute("linewidths", widths_key);
}


void GR::Render::setLineWidth(const std::shared_ptr<Element> &element, double width)
{
  /*!
   * This function can be used to set a LineWidth of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] type A Double setting the LineWidth
   */
  element->setAttribute("linewidth", width);
}


void GR::Render::setLineColorInd(const std::shared_ptr<Element> &element, const std::string &colorinds_key,
                                 const std::vector<int> &colorinds, const std::shared_ptr<Context> &extContext)
{
  /*!
   * This funciton can be used to set a vector of LineColorInds of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] colorinds_key A string used as a key for storing the colorinds
   * \param[in] colorinds A vector containing the Colorinds
   * \param[in] extContext A GR::Context used for storing colorinds. By default it uses GR::Render's GR::Context object
   * but an external GR::Context can be used
   */
  std::shared_ptr<Context> useContext = (extContext == nullptr) ? context : extContext;
  (*useContext)[colorinds_key] = colorinds;
  element->setAttribute("linecolorinds", colorinds_key);
}

void GR::Render::setLineColorInd(const std::shared_ptr<Element> &element, int color)
{
  /*!
   * This function can be used to set LineColorInd of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] color An Integer value setting the LineColorInd
   */
  element->setAttribute("linecolorind", color);
}


void GR::Render::setTextFontPrec(const std::shared_ptr<Element> &element, int font, int prec)
{
  /*!
   * This function can be used to TextFontPrec of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] font An Integer value representing a font type
   * \param[in] prec An Integer value representing a font precision
   */
  element->setAttribute("textfontprec", true);
  element->setAttribute("textfontprec_font", font);
  element->setAttribute("textfontprec_prec", prec);
}


static void renderHelper(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Recursive helper function for render; Not part of render class
   * Used for traversing the tree
   *
   * \param[in] element A GR::Element
   * \param[in] context A GR::Context
   */
  processElement(element, context);
  gr_savestate();
  if (element->hasChildNodes())
    {
      for (const auto &child : element->children())
        {
          renderHelper(child, context);
          gr_restorestate();
        }
    }
}


void GR::Render::render(const std::shared_ptr<GR::Document> &document, const std::shared_ptr<GR::Context> &extContext)
{
  /*!
   * static GR::Render::render receiving external document and context
   *
   * \param[in] document A GR::Document that will be rendered
   * \param[in] extContext A GR::Context
   */
  auto root = document->firstChildElement();
  gr_savestate();
  if (root->hasChildNodes())
    {
      for (const auto &child : root->children())
        {
          ::renderHelper(child, extContext);
          gr_restorestate();
        }
    }
}


void GR::Render::render(std::shared_ptr<GR::Document> const &document)
{
  /*!
   * GR::Render::render that receives an external document but uses the GR::Render instance's context.
   *
   * \param[in] document A GR::Document that will be rendered
   */
  auto root = document->firstChildElement();
  gr_savestate();
  if (root->hasChildNodes())
    {
      for (const auto &child : root->children())
        {
          ::renderHelper(child, this->context);
          gr_restorestate();
        }
    }
}


void GR::Render::render(const std::shared_ptr<GR::Context> &extContext)
{
  /*!
   *GR::Render::render uses GR::Render instance's document and an external context
   *
   * \param[in] extContext A GR::Context
   */
  auto root = this->firstChildElement();
  gr_savestate();
  if (root->hasChildNodes())
    {
      for (const auto &child : root->children())
        {
          ::renderHelper(child, extContext);
          gr_restorestate();
        }
    }
}

void GR::Render::render()
{
  /*!
   * GR::Render::render uses both instance's document and context
   */
  auto root = this->firstChildElement();
  gr_savestate();
  if (root->hasChildNodes())
    {
      for (const auto &child : root->children())
        {
          ::renderHelper(child, this->context);
          gr_restorestate();
        }
    }
}


std::shared_ptr<GR::Render> GR::Render::createRender()
{
  /*!
   * This function can be used to create a Render object
   */
  auto render = std::shared_ptr<Render>(new Render());
  return render;
}


GR::Render::Render()
{
  /*!
   * This is the constructor for GR::Render
   */
  this->context = std::shared_ptr<Context>(new Context());
}
