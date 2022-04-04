#include <functional>

#include "GR/Element.hxx"
#include "GR/Document.hxx"
#include "GR/Value.hxx"
#include "GR/util.hxx"
#include "gr.h"
#include "render.hxx"


static void markerHelper(const std::shared_ptr<Element> &element, const std::shared_ptr<Context> &context,
                         const std::string &str)
{
  /*!
   * Helperfunction for marker functions using vectors for marker parameters
   *
   */

  std::vector<int> type, colorind;
  std::vector<double> size;
  auto attr = element->getAttribute("markertypes");
  if (attr.isString()) type = context->tableInt[static_cast<std::string>(attr)];

  attr = element->getAttribute("markercolorinds");
  if (attr.isString()) colorind = context->tableInt[static_cast<std::string>(attr)];

  attr = element->getAttribute("markersizes");
  if (attr.isString()) size = context->tableDouble[static_cast<std::string>(attr)];

  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));

  for (int i = 0; i < x.size(); ++i)
    {
      // fallback to the last element when lists are too short; TODO make it cleaner
      if (!type.empty())
        {
          if (type.size() > i)
            gr_setmarkertype(type[i]);
          else
            gr_setmarkertype(type.back());
        }
      if (!colorind.empty())
        {
          if (colorind.size() > i)
            gr_setmarkercolorind(colorind[i]);
          else
            gr_setmarkercolorind(colorind.back());
        }
      if (!size.empty())
        {
          if (size.size() > i)
            gr_setmarkersize(size[i]);
          else
            gr_setmarkersize(size.back());
        }

      if (str == "polymarker")
        gr_polymarker(1, (double *)&(context->getFromDoubleTable(x)[i]),
                      (double *)&(context->getFromDoubleTable(y)[i]));
    }
}

static void lineHelper(const std::shared_ptr<Element> &element, const std::shared_ptr<Context> &context,
                       const std::string &str)
{
  /*!
   * Helperfunction for line functions using vectors for line parameters
   */
  std::vector<int> type, colorind;
  std::vector<double> width;
  auto attr = element->getAttribute("linetypes");
  if (attr.isString()) type = context->tableInt[static_cast<std::string>(attr)];

  attr = element->getAttribute("linecolorinds");
  if (attr.isString()) colorind = context->tableInt[static_cast<std::string>(attr)];

  attr = element->getAttribute("linewidths");
  if (attr.isString()) width = context->tableDouble[static_cast<std::string>(attr)];

  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));

  for (int i = 0; i < x.size() - 1; ++i)
    {
      if (!type.empty())
        {
          if (type.size() > i)
            gr_setlinetype(type[i]);
          else
            gr_setlinetype(type.back());
        }
      if (!colorind.empty())
        {
          if (colorind.size() > i)
            gr_setlinecolorind(colorind[i]);
          else
            gr_setlinecolorind(colorind.back());
        }
      if (!width.empty())
        {
          if (width.size() > i)
            gr_setlinewidth(width[i]);
          else
            gr_setlinewidth(width.back());
        }

      if (str == "polyline")
        gr_polyline(2, (double *)&(context->getFromDoubleTable(x)[i]), (double *)&(context->getFromDoubleTable(y)[i]));
    }
}


static void polymarker(const std::shared_ptr<Element> &element, const std::shared_ptr<Context> &context)
{
  int n = static_cast<int>(element->getAttribute("n"));
  if (element->getAttribute("x").isString() && element->getAttribute("y").isString())
    {
      auto x = static_cast<std::string>(element->getAttribute("x"));
      auto y = static_cast<std::string>(element->getAttribute("y"));
      if (element->hasAttribute("markertypes") || element->hasAttribute("markersizes") ||
          element->hasAttribute("markercolorinds"))
        {
          markerHelper(element, context, "polymarker");
        }
      else
        gr_polymarker(n, (double *)&(context->getFromDoubleTable(x)[0]),
                      (double *)&(context->getFromDoubleTable(y)[0]));
    }
  else if (element->getAttribute("x").isDouble())
    {
      auto x = static_cast<double>(element->getAttribute("x"));
      auto y = static_cast<double>(element->getAttribute("y"));
      gr_polymarker(n, &x, &y);
    }
}

static void polyline(const std::shared_ptr<Element> &element, const std::shared_ptr<Context> &context)
{
  int n = static_cast<int>(element->getAttribute("n"));
  if (element->getAttribute("x").isString() && element->getAttribute("y").isString())
    {

      auto x = static_cast<std::string>(element->getAttribute("x"));
      auto y = static_cast<std::string>(element->getAttribute("y"));
      if (element->hasAttribute("linetypes") || element->hasAttribute("linewidths") ||
          element->hasAttribute("linecolorinds"))
        {
          lineHelper(element, context, "polyline");
        }
      else
        gr_polyline(n, (double *)&(context->getFromDoubleTable(x)[0]), (double *)&(context->getFromDoubleTable(y)[0]));
    }
  else if (element->getAttribute("x").isDouble())
    {
      auto x = static_cast<double>(element->getAttribute("x"));
      auto y = static_cast<double>(element->getAttribute("y"));
      gr_polyline(n, &x, &y);
    }
}

static void text(const std::shared_ptr<Element> &element, const std::shared_ptr<Context> &context)
{
  auto x = static_cast<double>(element->getAttribute("x"));
  auto y = static_cast<double>(element->getAttribute("y"));
  auto str = static_cast<std::string>(element->getAttribute("text"));
  gr_text(x, y, &str[0]);
}

static void fillarea(const std::shared_ptr<Element> &element, const std::shared_ptr<Context> &context)
{
  auto n = static_cast<int>(element->getAttribute("n"));
  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));
  gr_fillarea(n, (double *)&(context->getFromDoubleTable(x)[0]), (double *)&(context->getFromDoubleTable(y)[0]));
}

static void cellarray(const std::shared_ptr<Element> &element, const std::shared_ptr<Context> &context)
{
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
  gr_cellarray(xmin, xmax, ymin, ymax, dimx, dimy, scol, srow, ncol, nrow, &(context->getFromIntTable(color)[0]));
}

static void axes(const std::shared_ptr<Element> &element, const std::shared_ptr<Context> &context)
{
  double x_tick = static_cast<double>(element->getAttribute("x_tick"));
  double y_tick = static_cast<double>(element->getAttribute("y_tick"));
  double x_org = static_cast<double>(element->getAttribute("x_org"));
  double y_org = static_cast<double>(element->getAttribute("y_org"));
  int major_x = static_cast<int>(element->getAttribute("major_x"));
  int major_y = static_cast<int>(element->getAttribute("major_y"));
  double tick_size = static_cast<double>(element->getAttribute("tick_size"));
  gr_axes(x_tick, y_tick, x_org, y_org, major_x, major_y, tick_size);
}

static void drawimage(const std::shared_ptr<Element> &element, const std::shared_ptr<Context> &context)
{
  double xmin = static_cast<double>(element->getAttribute("xmin"));
  double xmax = static_cast<double>(element->getAttribute("xmax"));
  double ymin = static_cast<double>(element->getAttribute("ymin"));
  double ymax = static_cast<double>(element->getAttribute("ymax"));
  int width = static_cast<int>(element->getAttribute("width"));
  int height = static_cast<int>(element->getAttribute("height"));
  int model = static_cast<int>(element->getAttribute("model"));
  auto data = static_cast<std::string>(element->getAttribute("data"));
  gr_drawimage(xmin, ymin, xmax, ymax, width, height, &(context->getFromIntTable(data)[0]), model);
}

static void processWindow(const std::shared_ptr<Element> &elem)
{
  double xmin, xmax, ymin, ymax;
  xmin = (double)elem->getAttribute("window_xmin");
  xmax = (double)elem->getAttribute("window_xmax");
  ymin = (double)elem->getAttribute("window_ymin");
  ymax = (double)elem->getAttribute("window_ymax");
  gr_setwindow(xmin, xmax, ymin, ymax);
}

static void processAttributes(const std::shared_ptr<Element> &element)
{
  static std::map<std::string, std::function<void(const std::shared_ptr<Element> &)>> string_to_func{
      {std::string("markertype"),
       [](const std::shared_ptr<Element> &elem) { gr_setmarkertype((int)elem->getAttribute("markertype")); }},
      {std::string("markercolorind"),
       [](const std::shared_ptr<Element> &elem) { gr_setmarkercolorind((int)elem->getAttribute("markercolorind")); }},
      {std::string("markersize"),
       [](const std::shared_ptr<Element> &elem) { gr_setmarkersize((double)elem->getAttribute("markersize")); }},
      {std::string("linetype"),
       [](const std::shared_ptr<Element> &elem) { gr_setlinetype((int)elem->getAttribute("linetype")); }},
      {std::string("linecolorind"),
       [](const std::shared_ptr<Element> &elem) { gr_setlinecolorind((int)elem->getAttribute("linecolorind")); }},
      {std::string("linewidth"),
       [](const std::shared_ptr<Element> &elem) { gr_setlinewidth((double)elem->getAttribute("linewidth")); }},
      {std::string("fillintstyle"),
       [](const std::shared_ptr<Element> &elem) { gr_setfillintstyle((int)elem->getAttribute("fillintstyle")); }},
      {std::string("fillstyle"),
       [](const std::shared_ptr<Element> &elem) { gr_setfillstyle((int)elem->getAttribute("fillstyle")); }},
      {std::string("fillcolorind"),
       [](const std::shared_ptr<Element> &elem) { gr_setfillcolorind((int)elem->getAttribute("fillcolorind")); }},
      // TODO: textfontprec etc.? two attributes or vectors or one Integer containing two numbers via bitshifts?
      {std::string("textfontprec"),
       [](const std::shared_ptr<Element> &elem) {
         gr_settextfontprec((int)elem->getAttribute("textfontprec_font"), (int)elem->getAttribute("textfontprec_prec"));
       }},
      {std::string("charexpan"),
       [](const std::shared_ptr<Element> &elem) { gr_setcharexpan((double)elem->getAttribute("charexpan")); }},
      {std::string("charspace"),
       [](const std::shared_ptr<Element> &elem) { gr_setcharspace((double)elem->getAttribute("charspace")); }},
      {std::string("textcolorind"),
       [](const std::shared_ptr<Element> &elem) { gr_settextcolorind((int)elem->getAttribute("textcolorind")); }},
      {std::string("charheight"),
       [](const std::shared_ptr<Element> &elem) { gr_setcharheight((double)elem->getAttribute("charheight")); }},
      {std::string("charup"),
       [](const std::shared_ptr<Element> &elem) {
         gr_setcharup((double)elem->getAttribute("charup_ux"), (double)elem->getAttribute("charup_uy"));
       }},
      {std::string("textpath"),
       [](const std::shared_ptr<Element> &elem) { gr_settextpath((int)elem->getAttribute("textpath")); }},
      {std::string("textalign"),
       [](const std::shared_ptr<Element> &elem) {
         gr_settextalign((int)elem->getAttribute("textalign_horizontal"),
                         (int)elem->getAttribute("textalign_vertical"));
       }},
      {std::string("window"), processWindow},
  };
  for (auto &pair : string_to_func)
    {
      if (element->hasAttribute(pair.first)) pair.second(element);
    }
}


static void processElement(const std::shared_ptr<Element> &element, const std::shared_ptr<Context> &context)
{
  static std::map<std::string, std::function<void(const std::shared_ptr<Element> &, const std::shared_ptr<Context> &)>>
      string_to_func{
          {std::string("polymarker"), polymarker}, {std::string("polyline"), polyline},   {std::string("text"), text},
          {std::string("fillarea"), fillarea},     {std::string("cellarray"), cellarray}, {std::string("axes"), axes},
          {std::string("drawimage"), drawimage}};

  /* Modifier */
  if (element->localName() == "group")
    {
      processAttributes(element);
    }
  else
    {
      /* Drawnodes */
      gr_savestate();
      processAttributes(element);
      std::function<void(const std::shared_ptr<Element> &, const std::shared_ptr<Context> &)> f =
          string_to_func[element->localName()];
      f(element, context);
      gr_restorestate();
    }
}


std::shared_ptr<Element> Render::createPolymarker(int n, const std::vector<double> &x, const std::vector<double> &y,
                                                  const std::shared_ptr<Context> &extContext, int marker_type,
                                                  double marker_size, int marker_colorind)
{
  std::shared_ptr<Context> useContext = (extContext == nullptr) ? context : extContext;

  auto polymarker_element = createElement("polymarker");


  polymarker_element->setAttribute("n", n);

  auto str = useContext->insertIntoDoubleTable("$x", x);
  polymarker_element->setAttribute("x", str);

  str = useContext->insertIntoDoubleTable("$y", y);
  polymarker_element->setAttribute("y", str);

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


std::shared_ptr<Element> Render::createPolyline(int n, const std::vector<double> &x, const std::vector<double> &y,
                                                const std::shared_ptr<Context> &extContext, int line_type,
                                                double line_width, int line_colorind)
{
  std::shared_ptr<Context> useContext = (extContext == nullptr) ? context : extContext;

  auto element = createElement("polyline");

  element->setAttribute("n", n);


  auto str = useContext->insertIntoDoubleTable("$x", x);
  element->setAttribute("x", str);

  str = useContext->insertIntoDoubleTable("$y", y);
  element->setAttribute("y", str);

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


std::shared_ptr<Element> Render::createText(double x, double y, const std::string &text)
{
  auto element = createElement("text");
  element->setAttribute("x", x);
  element->setAttribute("y", y);
  element->setAttribute("text", text);
  return element;
}


std::shared_ptr<Element> Render::createFillArea(int n, const std::vector<double> &x, const std::vector<double> &y,
                                                const std::shared_ptr<Context> &extContext, int fillintstyle,
                                                int fillstyle, int fillcolorind)
{

  std::shared_ptr<Context> useContext = (extContext == nullptr) ? context : extContext;

  auto element = createElement("fillarea");


  element->setAttribute("n", n);

  auto str = useContext->insertIntoDoubleTable("$x", x);
  element->setAttribute("x", str);

  str = useContext->insertIntoDoubleTable("$y", y);
  element->setAttribute("y", str);

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


std::shared_ptr<Element> Render::createCellArray(double xmin, double xmax, double ymin, double ymax, int dimx, int dimy,
                                                 int scol, int srow, int ncol, int nrow, const std::vector<int> &color,
                                                 const std::shared_ptr<Context> &extContext)
{
  std::shared_ptr<Context> useContext = (extContext == nullptr) ? context : extContext;
  auto element = createElement("cellarray");


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

  auto str = useContext->insertIntoIntTable("$color", color);
  element->setAttribute("color", str);

  return element;
}

std::shared_ptr<Element> Render::createAxes(double x_tick, double y_tick, double x_org, double y_org, int major_x,
                                            int major_y, double tick_size)
{
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

std::shared_ptr<Element> Render::createSetWindow(double xmin, double xmax, double ymin, double ymax)
{
  auto element = createElement("window");
  element->setAttribute("xmin", xmin);
  element->setAttribute("xmax", xmax);
  element->setAttribute("ymin", ymin);
  element->setAttribute("ymax", ymax);
  return element;
}


std::shared_ptr<Element> Render::createGroup()
{
  return createElement("group");
}


std::shared_ptr<Element> Render::createDrawImage(double xmin, double ymin, double xmax, double ymax, int width,
                                                 int height, const std::vector<int> &data, int model,
                                                 const std::shared_ptr<Context> &extContext)
{
  std::shared_ptr<Context> useContext = (extContext == nullptr) ? context : extContext;
  auto element = createElement("drawimage");


  element->setAttribute("xmin", xmin);
  element->setAttribute("xmax", xmax);
  element->setAttribute("ymin", ymin);
  element->setAttribute("ymin", ymax);
  element->setAttribute("width", width);
  element->setAttribute("height", height);
  element->setAttribute("model", model);

  auto str = useContext->insertIntoIntTable("$data", data);
  element->setAttribute("data", str);

  return element;
}


void Render::setMarkerType(const std::shared_ptr<Element> &element, int type)
{
  element->setAttribute("markertype", type);
}


void Render::setMarkerType(const std::shared_ptr<Element> &element, const std::vector<int> &types,
                           const std::shared_ptr<Context> &extContext)
{
  // TODO: Just overload setmarker etc. functions -> no more standalone vec functions!
  std::shared_ptr<Context> useContext = (extContext == nullptr) ? context : extContext;
  auto str = useContext->insertIntoIntTable("$markertypse", types);
  element->setAttribute("markertypes", str);
}


void Render::setMarkerSize(const std::shared_ptr<Element> &element, double size)
{
  element->setAttribute("markersize", size);
}

void Render::setMarkerSize(const std::shared_ptr<Element> &element, const std::vector<double> &sizes,
                           const std::shared_ptr<Context> &extContext)
{
  std::shared_ptr<Context> useContext = (extContext == nullptr) ? context : extContext;
  auto str = useContext->insertIntoDoubleTable("$markersizes", sizes);
  element->setAttribute("markersizes", str);
}
void Render::setMarkerColorInd(const std::shared_ptr<Element> &element, int color)
{
  element->setAttribute("markercolorind", color);
}

void Render::setMarkerColorInd(const std::shared_ptr<Element> &element, const std::vector<int> &colorinds,
                               const std::shared_ptr<Context> &extContext)
{
  std::shared_ptr<Context> useContext = (extContext == nullptr) ? context : extContext;
  auto str = useContext->insertIntoIntTable("$markercolorinds", colorinds);
  element->setAttribute("markercolorinds", str);
}


void Render::setLineType(const std::shared_ptr<Element> &element, const std::vector<int> &types,
                         const std::shared_ptr<Context> &extContext)
{
  std::shared_ptr<Context> useContext = (extContext == nullptr) ? context : extContext;
  auto str = useContext->insertIntoIntTable("$linetypes", types);
  element->setAttribute("linetypes", str);
}

void Render::setLineType(const std::shared_ptr<Element> &element, int type)
{
  element->setAttribute("linetype", type);
}


void Render::setLineWidth(const std::shared_ptr<Element> &element, const std::vector<double> &widths,
                          const std::shared_ptr<Context> &extContext)
{
  std::shared_ptr<Context> useContext = (extContext == nullptr) ? context : extContext;
  auto str = useContext->insertIntoDoubleTable("$linewidths", widths);
  element->setAttribute("linewidths", str);
}

void Render::setLineWidth(const std::shared_ptr<Element> &element, double width)
{
  element->setAttribute("linewidth", width);
}


void Render::setLineColorInd(const std::shared_ptr<Element> &element, const std::vector<int> &colorinds,
                             const std::shared_ptr<Context> &extContext)
{
  std::shared_ptr<Context> useContext = (extContext == nullptr) ? context : extContext;
  auto str = useContext->insertIntoIntTable("$linecolorinds", colorinds);
  element->setAttribute("linecolorinds", str);
}

void Render::setLineColorInd(const std::shared_ptr<Element> &element, int color)
{
  element->setAttribute("linecolorind", color);
}


void Render::setTextFontPrec(const std::shared_ptr<Element> &element, int font, int prec)
{
  element->setAttribute("textfontprec", true);
  element->setAttribute("textfontprec_font", font);
  element->setAttribute("textfontprec_prec", prec);
}


static void renderHelper(const std::shared_ptr<Element> &element, const std::shared_ptr<Context> &context)
{
  /*!
   * Helper function for render; Not part of render class
   * Can be private static member of render?
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

void Render::render(const std::shared_ptr<Document> &document, const std::shared_ptr<Context> &extContext)
{
  /*!
   * static Render::render receiving external document and context
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

void Render::render(std::shared_ptr<Document> const &document)
{
  /*!
   * Render::render receiving external document but uses instances context.
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

void Render::render(const std::shared_ptr<Context> &extContext)
{
  /*!
   * Render::render uses instances document and external context
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

void Render::render()
{
  /*!
   * Render::render uses instances document and context
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


std::shared_ptr<Render> Render::createRender()
{
  auto render = std::shared_ptr<Render>(new Render());
  return render;
}


Render::Render()
{
  this->context = std::shared_ptr<Context>(new Context());
}
