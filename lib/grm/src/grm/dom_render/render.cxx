#ifdef _WIN32
/*
 * Headers on Windows can define `min` and `max` as macros which causes
 * problem when using `std::min` and `std::max`
 * -> Define `NOMINMAX` to prevent the definition of these macros
 */
#define NOMINMAX
#endif

#include <functional>
#include <vector>
#include <set>
#include <sstream>
#include <grm/dom_render/graphics_tree/Element.hxx>
#include <grm/dom_render/graphics_tree/Document.hxx>
#include <grm/dom_render/graphics_tree/Value.hxx>
#include <grm/dom_render/graphics_tree/util.hxx>
#include <grm/dom_render/render.hxx>
#include <grm/dom_render/NotFoundError.hxx>
#include <grm/dom_render/context.hxx>
#include "gr.h"
#include "gr3.h"


//! This vector is used for storing element types which children get processed. Other types' children will be ignored
static std::set<std::string> parentTypes = {
    "group",
};

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
  bool group = parentTypes.count(parent->localName());

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
  std::string z;
  if (element->hasAttribute("z"))
    {
      z = static_cast<std::string>(element->getAttribute("z"));
    }

  std::vector<double> x_vec = GR::get<std::vector<double>>((*context)[x]);
  std::vector<double> y_vec = GR::get<std::vector<double>>((*context)[y]);
  std::vector<double> z_vec;
  if (auto z_ptr = GR::get_if<std::vector<double>>((*context)[z]))
    {
      z_vec = *z_ptr;
    }


  int n = std::min(x_vec.size(), y_vec.size());

  for (int i = 0; i < n; ++i)
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
              if (colorind[i] == 0)
                {
                  continue;
                }
              gr_setmarkercolorind(colorind[i]);
            }
          else
            {
              if (colorind.back() == 0)
                {
                  continue;
                }
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
          gr_polymarker(1, (double *)&(x_vec[i]), (double *)&(y_vec[i]));
        }
      else if (str == "polymarker3d")
        {
          gr_polymarker3d(1, (double *)&(x_vec[i]), (double *)&(y_vec[i]), (double *)&(z_vec[i]));
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
  bool group = parentTypes.count(parent->localName());

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
  std::string z;
  if (element->hasAttribute("z"))
    {
      z = static_cast<std::string>(element->getAttribute("z"));
    }

  std::vector<double> x_vec = GR::get<std::vector<double>>((*context)[x]);
  std::vector<double> y_vec = GR::get<std::vector<double>>((*context)[y]);
  std::vector<double> z_vec;

  if (auto z_ptr = GR::get_if<std::vector<double>>((*context)[z]))
    {
      z_vec = *z_ptr;
    }

  int n = std::min(x_vec.size(), y_vec.size());
  for (int i = 0; i < n; ++i)
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
          gr_polyline(2, (double *)&(x_vec[i]), (double *)&(y_vec[i]));
        }
      else if (str == "polyline3d")
        {
          gr_polyline3d(2, (double *)&(x_vec[i]), (double *)&(y_vec[i]), (double *)&(z_vec[i]));
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
  if (element->getAttribute("x").isString() && element->getAttribute("y").isString())
    {
      auto x = static_cast<std::string>(element->getAttribute("x"));
      auto y = static_cast<std::string>(element->getAttribute("y"));

      std::vector<double> x_vec = GR::get<std::vector<double>>((*context)[x]);
      std::vector<double> y_vec = GR::get<std::vector<double>>((*context)[y]);

      int n = std::min(x_vec.size(), y_vec.size());
      auto group = element->parentElement();
      if ((element->hasAttribute("markertypes") || element->hasAttribute("markersizes") ||
           element->hasAttribute("markercolorinds")) ||
          (parentTypes.count(group->localName()) &&
           (group->hasAttribute("markertypes") || group->hasAttribute("markersizes") ||
            group->hasAttribute("markercolorinds"))))
        {
          markerHelper(element, context, "polymarker");
        }
      else
        {
          gr_polymarker(n, (double *)&(x_vec[0]), (double *)&(y_vec[0]));
        }
    }
  else if (element->getAttribute("x").isDouble() && element->getAttribute("y").isDouble())
    {
      double x = static_cast<double>(element->getAttribute("x"));
      double y = static_cast<double>(element->getAttribute("y"));
      gr_polymarker(1, &x, &y);
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

      std::vector<double> x_vec = GR::get<std::vector<double>>((*context)[x]);
      std::vector<double> y_vec = GR::get<std::vector<double>>((*context)[y]);

      int n = std::min(x_vec.size(), y_vec.size());
      auto group = element->parentElement();
      if ((element->hasAttribute("linetypes") || element->hasAttribute("linewidths") ||
           element->hasAttribute("linecolorinds")) ||
          ((parentTypes.count(group->localName())) &&
           (group->hasAttribute("linetypes") || group->hasAttribute("linewidths") ||
            group->hasAttribute("linecolorinds"))))
        {
          lineHelper(element, context, "polyline");
        }
      else
        gr_polyline(n, (double *)&(x_vec[0]), (double *)&(y_vec[0]));
    }
  else if (element->getAttribute("x1").isDouble() && element->getAttribute("x2").isDouble() &&
           element->getAttribute("y1").isDouble() && element->getAttribute("y2").isDouble())
    {
      auto x1 = static_cast<double>(element->getAttribute("x1"));
      auto x2 = static_cast<double>(element->getAttribute("x2"));
      auto y1 = static_cast<double>(element->getAttribute("y1"));
      auto y2 = static_cast<double>(element->getAttribute("y2"));
      double x[2] = {x1, x2};
      double y[2] = {y1, y2};

      gr_polyline(2, x, y);
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
  CoordinateSpace space = static_cast<CoordinateSpace>(static_cast<int>(element->getAttribute("space")));
  if (space == WC)
    {
      gr_wctondc(&x, &y);
    }
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
  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));

  std::vector<double> x_vec = GR::get<std::vector<double>>((*context)[x]);
  std::vector<double> y_vec = GR::get<std::vector<double>>((*context)[y]);

  int n = std::min(x_vec.size(), y_vec.size());

  gr_fillarea(n, (double *)&(x_vec[0]), (double *)&(y_vec[0]));
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

static void grid(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processing function for grid
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
  gr_grid(x_tick, y_tick, x_org, y_org, major_x, major_y);
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

static void drawArc(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processing function for drawArc
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  double xmin = static_cast<double>(element->getAttribute("xmin"));
  double xmax = static_cast<double>(element->getAttribute("xmax"));
  double ymin = static_cast<double>(element->getAttribute("ymin"));
  double ymax = static_cast<double>(element->getAttribute("ymax"));
  double a1 = static_cast<double>(element->getAttribute("a1"));
  double a2 = static_cast<double>(element->getAttribute("a2"));
  gr_drawarc(xmin, xmax, ymin, ymax, a1, a2);
}

static void fillArc(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processing function for drawArc
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  double xmin = static_cast<double>(element->getAttribute("xmin"));
  double xmax = static_cast<double>(element->getAttribute("xmax"));
  double ymin = static_cast<double>(element->getAttribute("ymin"));
  double ymax = static_cast<double>(element->getAttribute("ymax"));
  double a1 = static_cast<double>(element->getAttribute("a1"));
  double a2 = static_cast<double>(element->getAttribute("a2"));
  gr_fillarc(xmin, xmax, ymin, ymax, a1, a2);
}

static void drawRect(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processing function for drawArc
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  double xmin = static_cast<double>(element->getAttribute("xmin"));
  double xmax = static_cast<double>(element->getAttribute("xmax"));
  double ymin = static_cast<double>(element->getAttribute("ymin"));
  double ymax = static_cast<double>(element->getAttribute("ymax"));
  gr_drawrect(xmin, xmax, ymin, ymax);
}

static void fillRect(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processing function for drawArc
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  double xmin = static_cast<double>(element->getAttribute("xmin"));
  double xmax = static_cast<double>(element->getAttribute("xmax"));
  double ymin = static_cast<double>(element->getAttribute("ymin"));
  double ymax = static_cast<double>(element->getAttribute("ymax"));
  gr_fillrect(xmin, xmax, ymin, ymax);
}

static void quiver(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processing function for quiver
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));
  auto u = static_cast<std::string>(element->getAttribute("u"));
  auto v = static_cast<std::string>(element->getAttribute("v"));
  int color = static_cast<int>(element->getAttribute("color"));

  std::vector<double> x_vec = GR::get<std::vector<double>>((*context)[x]);
  std::vector<double> y_vec = GR::get<std::vector<double>>((*context)[y]);


  double *x_p = &(x_vec[0]);
  double *y_p = &(y_vec[0]);
  double *u_p = &(GR::get<std::vector<double>>((*context)[u])[0]);
  double *v_p = &(GR::get<std::vector<double>>((*context)[v])[0]);

  gr_quiver(x_vec.size(), y_vec.size(), x_p, y_p, u_p, v_p, color);
}

static void contour(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processing function for contour
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  auto px = static_cast<std::string>(element->getAttribute("px"));
  auto py = static_cast<std::string>(element->getAttribute("py"));
  auto h = static_cast<std::string>(element->getAttribute("h"));
  auto pz = static_cast<std::string>(element->getAttribute("pz"));
  int major_h = static_cast<int>(element->getAttribute("major_h"));

  std::vector<double> px_vec = GR::get<std::vector<double>>((*context)[px]);
  std::vector<double> py_vec = GR::get<std::vector<double>>((*context)[py]);
  std::vector<double> h_vec = GR::get<std::vector<double>>((*context)[h]);
  std::vector<double> pz_vec = GR::get<std::vector<double>>((*context)[pz]);

  int nx = px_vec.size();
  int ny = py_vec.size();
  int nh = h_vec.size();

  double *px_p = &(px_vec[0]);
  double *py_p = &(py_vec[0]);
  double *h_p = &(h_vec[0]);
  double *pz_p = &(pz_vec[0]);

  gr_contour(nx, ny, nh, px_p, py_p, h_p, pz_p, major_h);
}


static void contourf(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processing function for contourf
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  auto px = static_cast<std::string>(element->getAttribute("px"));
  auto py = static_cast<std::string>(element->getAttribute("py"));
  auto h = static_cast<std::string>(element->getAttribute("h"));
  auto pz = static_cast<std::string>(element->getAttribute("pz"));
  int major_h = static_cast<int>(element->getAttribute("major_h"));

  std::vector<double> px_vec = GR::get<std::vector<double>>((*context)[px]);
  std::vector<double> py_vec = GR::get<std::vector<double>>((*context)[py]);
  std::vector<double> h_vec = GR::get<std::vector<double>>((*context)[h]);
  std::vector<double> pz_vec = GR::get<std::vector<double>>((*context)[pz]);

  int nx = px_vec.size();
  int ny = py_vec.size();
  int nh = h_vec.size();

  double *px_p = &(px_vec[0]);
  double *py_p = &(py_vec[0]);
  double *h_p = &(h_vec[0]);
  double *pz_p = &(pz_vec[0]);

  gr_contourf(nx, ny, nh, px_p, py_p, h_p, pz_p, major_h);
}

static void hexbin(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processing function for hexbin
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */

  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));
  int nbins = static_cast<int>(element->getAttribute("nbins"));

  int x_length = static_cast<int>(element->getAttribute("x_length"));

  double *x_p = &(GR::get<std::vector<double>>((*context)[x])[0]);
  double *y_p = &(GR::get<std::vector<double>>((*context)[y])[0]);

  gr_hexbin(x_length, x_p, y_p, nbins);
}

static void nonuniformcellarray(const std::shared_ptr<GR::Element> &element,
                                const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processing function for nonuniformcellarray
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));

  int dimx = static_cast<int>(element->getAttribute("dimx"));
  int dimy = static_cast<int>(element->getAttribute("dimy"));
  int scol = static_cast<int>(element->getAttribute("scol"));
  int srow = static_cast<int>(element->getAttribute("srow"));
  int ncol = static_cast<int>(element->getAttribute("ncol"));
  int nrow = static_cast<int>(element->getAttribute("nrow"));
  auto color = static_cast<std::string>(element->getAttribute("color"));

  auto x_p = (double *)&(GR::get<std::vector<double>>((*context)[x])[0]);
  auto y_p = (double *)&(GR::get<std::vector<double>>((*context)[y])[0]);

  auto color_p = (int *)&(GR::get<std::vector<int>>((*context)[color])[0]);
  gr_nonuniformcellarray(x_p, y_p, dimx, dimy, scol, srow, ncol, nrow, color_p);
}

static void surface(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processing function for surface
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  auto px = static_cast<std::string>(element->getAttribute("px"));
  auto py = static_cast<std::string>(element->getAttribute("py"));
  auto pz = static_cast<std::string>(element->getAttribute("pz"));
  int option = static_cast<int>(element->getAttribute("option"));

  std::vector<double> px_vec = GR::get<std::vector<double>>((*context)[px]);
  std::vector<double> py_vec = GR::get<std::vector<double>>((*context)[py]);
  std::vector<double> pz_vec = GR::get<std::vector<double>>((*context)[pz]);

  int nx = px_vec.size();
  int ny = py_vec.size();

  double *px_p = &(px_vec[0]);
  double *py_p = &(py_vec[0]);
  double *pz_p = &(pz_vec[0]);

  gr_surface(nx, ny, px_p, py_p, pz_p, option);
}

static void grid3d(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processing function for grid3d
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  double x_tick = static_cast<double>(element->getAttribute("x_tick"));
  double y_tick = static_cast<double>(element->getAttribute("y_tick"));
  double z_tick = static_cast<double>(element->getAttribute("z_tick"));
  double x_org = static_cast<double>(element->getAttribute("x_org"));
  double y_org = static_cast<double>(element->getAttribute("y_org"));
  double z_org = static_cast<double>(element->getAttribute("z_org"));
  int major_x = static_cast<int>(element->getAttribute("major_x"));
  int major_y = static_cast<int>(element->getAttribute("major_y"));
  int major_z = static_cast<int>(element->getAttribute("major_z"));
  gr_grid3d(x_tick, y_tick, z_tick, x_org, y_org, z_org, major_x, major_y, major_z);
}


static void axes3d(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processing function for axes3d
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  double x_tick = static_cast<double>(element->getAttribute("x_tick"));
  double y_tick = static_cast<double>(element->getAttribute("y_tick"));
  double z_tick = static_cast<double>(element->getAttribute("z_tick"));
  double x_org = static_cast<double>(element->getAttribute("x_org"));
  double y_org = static_cast<double>(element->getAttribute("y_org"));
  double z_org = static_cast<double>(element->getAttribute("z_org"));
  int major_x = static_cast<int>(element->getAttribute("major_x"));
  int major_y = static_cast<int>(element->getAttribute("major_y"));
  int major_z = static_cast<int>(element->getAttribute("major_z"));
  double tick_size = static_cast<double>(element->getAttribute("tick_size"));

  gr_axes3d(x_tick, y_tick, z_tick, x_org, y_org, z_org, major_x, major_y, major_z, tick_size);
}


static void polyline3d(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processing function for polyline3d
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));
  auto z = static_cast<std::string>(element->getAttribute("z"));

  std::vector<double> x_vec = GR::get<std::vector<double>>((*context)[x]);
  std::vector<double> y_vec = GR::get<std::vector<double>>((*context)[y]);
  std::vector<double> z_vec = GR::get<std::vector<double>>((*context)[z]);


  double *x_p = &(x_vec[0]);
  double *y_p = &(y_vec[0]);
  double *z_p = &(z_vec[0]);
  auto group = element->parentElement();

  if ((element->hasAttribute("linetypes") || element->hasAttribute("linewidths") ||
       element->hasAttribute("linecolorinds")) ||
      ((parentTypes.count(group->localName())) &&
       (group->hasAttribute("linetypes") || group->hasAttribute("linewidths") || group->hasAttribute("linecolorinds"))))
    {
      lineHelper(element, context, "polyline3d");
    }
  else
    {
      gr_polyline3d(x_vec.size(), x_p, y_p, z_p);
    }
}


static void polymarker3d(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processing function for polymarker3d
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));
  auto z = static_cast<std::string>(element->getAttribute("z"));

  std::vector<double> x_vec = GR::get<std::vector<double>>((*context)[x]);
  std::vector<double> y_vec = GR::get<std::vector<double>>((*context)[y]);
  std::vector<double> z_vec = GR::get<std::vector<double>>((*context)[z]);


  double *x_p = &(x_vec[0]);
  double *y_p = &(y_vec[0]);
  double *z_p = &(z_vec[0]);

  auto group = element->parentElement();
  if ((element->hasAttribute("markertypes") || element->hasAttribute("markersizes") ||
       element->hasAttribute("markercolorinds")) ||
      (parentTypes.count(group->localName()) &&
       (group->hasAttribute("markertypes") || group->hasAttribute("markersizes") ||
        group->hasAttribute("markercolorinds"))))
    {
      markerHelper(element, context, "polymarker3d");
    }
  else
    {
      gr_polymarker3d(x_vec.size(), x_p, y_p, z_p);
    }
}

static void gr3DrawMesh(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processing function for gr3_drawmesh
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */

  int mesh = (int)element->getAttribute("mesh");
  int n = (int)element->getAttribute("n");

  auto positions = static_cast<std::string>(element->getAttribute("positions"));
  auto directions = static_cast<std::string>(element->getAttribute("directions"));
  auto ups = static_cast<std::string>(element->getAttribute("ups"));
  auto colors = static_cast<std::string>(element->getAttribute("colors"));
  auto scales = static_cast<std::string>(element->getAttribute("scales"));

  std::vector<double> positions_vec = GR::get<std::vector<double>>((*context)[positions]);
  std::vector<double> directions_vec = GR::get<std::vector<double>>((*context)[directions]);
  std::vector<double> ups_vec = GR::get<std::vector<double>>((*context)[ups]);
  std::vector<double> colors_vec = GR::get<std::vector<double>>((*context)[colors]);
  std::vector<double> scales_vec = GR::get<std::vector<double>>((*context)[scales]);

  // TODO? : float workaround here -> support float in gr tree interface and GRender?
  std::vector<float> pf_vec(positions_vec.begin(), positions_vec.end());
  std::vector<float> df_vec(directions_vec.begin(), directions_vec.end());
  std::vector<float> uf_vec(ups_vec.begin(), ups_vec.end());
  std::vector<float> cf_vec(colors_vec.begin(), colors_vec.end());
  std::vector<float> sf_vec(scales_vec.begin(), scales_vec.end());

  float *positions_p = &(pf_vec[0]);
  float *directions_p = &(df_vec[0]);
  float *ups_p = &(uf_vec[0]);
  float *colors_p = &(cf_vec[0]);
  float *scales_p = &(sf_vec[0]);

  gr3_drawmesh(mesh, n, positions_p, directions_p, ups_p, colors_p, scales_p);
}


static void volume(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  int nx, ny, nz, algorithm;
  std::string dmin_key, dmax_key;
  double dmin, dmax;

  auto data = static_cast<std::string>(element->getAttribute("data"));
  nx = (int)element->getAttribute("nx");
  ny = (int)element->getAttribute("ny");
  nz = (int)element->getAttribute("nz");
  algorithm = (int)element->getAttribute("algorithm");
  dmin_key = (std::string)element->getAttribute("dmin");
  dmax_key = (std::string)element->getAttribute("dmax");

  auto dmin_vec = GR::get<std::vector<double>>((*context)[dmin_key]);


  std::vector<double> data_vec = GR::get<std::vector<double>>((*context)[data]);

  gr_volume(nx, ny, nz, &(data_vec[0]), algorithm, &dmin, &dmax);
}

static void triSurface(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processing function for trisurface
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  auto px = static_cast<std::string>(element->getAttribute("px"));
  auto py = static_cast<std::string>(element->getAttribute("py"));
  auto pz = static_cast<std::string>(element->getAttribute("pz"));

  std::vector<double> px_vec = GR::get<std::vector<double>>((*context)[px]);
  std::vector<double> py_vec = GR::get<std::vector<double>>((*context)[py]);
  std::vector<double> pz_vec = GR::get<std::vector<double>>((*context)[pz]);

  int nx = px_vec.size();
  int ny = py_vec.size();

  double *px_p = &(px_vec[0]);
  double *py_p = &(py_vec[0]);
  double *pz_p = &(pz_vec[0]);

  gr_trisurface(nx, px_p, py_p, pz_p);
}


static void triContour(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processing function for tricontour
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  auto px = static_cast<std::string>(element->getAttribute("px"));
  auto py = static_cast<std::string>(element->getAttribute("py"));
  auto pz = static_cast<std::string>(element->getAttribute("pz"));
  auto levels = static_cast<std::string>(element->getAttribute("levels"));

  std::vector<double> px_vec = GR::get<std::vector<double>>((*context)[px]);
  std::vector<double> py_vec = GR::get<std::vector<double>>((*context)[py]);
  std::vector<double> pz_vec = GR::get<std::vector<double>>((*context)[pz]);
  std::vector<double> l_vec = GR::get<std::vector<double>>((*context)[levels]);

  int nx = px_vec.size();
  int nl = l_vec.size();

  double *px_p = &(px_vec[0]);
  double *py_p = &(py_vec[0]);
  double *pz_p = &(pz_vec[0]);
  double *l_p = &(l_vec[0]);

  gr_tricontour(nx, px_p, py_p, pz_p, nl, l_p);
}

static void gr3Clear(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  gr3_clear();
}

static void gr3DeleteMesh(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  int mesh = static_cast<int>(element->getAttribute("mesh"));
  gr3_deletemesh(mesh);
}


static void gr3DrawImage(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  double xmin, xmax, ymin, ymax;
  int width, height, drawable_type;

  xmin = (double)element->getAttribute("xmin");
  xmax = (double)element->getAttribute("xmax");
  ymin = (double)element->getAttribute("ymin");
  ymax = (double)element->getAttribute("ymax");
  width = (int)element->getAttribute("width");
  height = (int)element->getAttribute("height");
  drawable_type = (int)element->getAttribute("drawable_type");


  logger((stderr, "gr3_drawimage returned %i\n", gr3_drawimage(xmin, xmax, ymin, ymax, width, height, drawable_type)));
}


static void shadePoints(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  int xform, w, h, n;
  std::string xx, yy;
  double *x, *y;

  xform = (int)element->getAttribute("xform");
  w = (int)element->getAttribute("w");
  h = (int)element->getAttribute("h");
  xx = (std::string)element->getAttribute("x");
  yy = (std::string)element->getAttribute("y");

  auto x_vec = GR::get<std::vector<double>>((*context)[xx]);
  auto y_vec = GR::get<std::vector<double>>((*context)[yy]);

  x = &(x_vec[0]);
  y = &(y_vec[0]);
  n = std::min(x_vec.size(), y_vec.size());

  gr_shadepoints(n, x, y, xform, w, h);
}

static void clearWS(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  gr_clearws();
}

static void updateWS(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  gr_updatews();
}


static void drawGraphics(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  auto key = (std::string)element->getAttribute("data");
  auto data_vec = GR::get<std::vector<int>>((*context)[key]);

  std::vector<char> char_vec;
  char_vec.reserve(data_vec.size());
  for (int i : data_vec)
    {
      char_vec.push_back((char)i);
    }
  char *data_p = &(char_vec[0]);

  gr_drawgraphics(data_p);
}

static void processColorRep(const std::shared_ptr<GR::Element> &elem)
{
  int index, hex_int;
  double red, green, blue;
  std::stringstream stringstream;
  std::string name, hex_string;

  for (auto &attr : elem->getAttributeNames())
    {
      auto start = 0U;
      auto end = attr.find('_');
      if (attr.substr(start, end) == "colorrep")
        {
          name = attr;
          index = std::stoi(attr.substr(end + 1, attr.size()));
        }
    }

  hex_string = static_cast<std::string>(elem->getAttribute(name));
  stringstream << std::hex << hex_string;
  stringstream >> hex_int;

  red = ((hex_int >> 16) & 0xFF) / 255.0;
  green = ((hex_int >> 8) & 0xFF) / 255.0;
  blue = ((hex_int)&0xFF) / 255.0;

  gr_setcolorrep(index, red, green, blue);
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

static void processWindow3d(const std::shared_ptr<GR::Element> &elem)
{
  /*!
   * Procesing function for gr_window3d
   *
   * \param[in] element The GR::Element that contains the attributes
   */
  double xmin, xmax, ymin, ymax, zmin, zmax;
  xmin = (double)elem->getAttribute("window3d_xmin");
  xmax = (double)elem->getAttribute("window3d_xmax");
  ymin = (double)elem->getAttribute("window3d_ymin");
  ymax = (double)elem->getAttribute("window3d_ymax");
  zmin = (double)elem->getAttribute("window3d_zmin");
  zmax = (double)elem->getAttribute("window3d_zmax");
  gr_setwindow3d(xmin, xmax, ymin, ymax, zmin, zmax);
}

static void processSpace3d(const std::shared_ptr<GR::Element> &elem)
{
  double phi, theta, fov, camera_distance;
  phi = (double)elem->getAttribute("space3d_phi");
  theta = (double)elem->getAttribute("space3d_theta");
  fov = (double)elem->getAttribute("space3d_fov");
  camera_distance = (double)elem->getAttribute("space3d_camera-distance");

  gr_setspace3d(phi, theta, fov, camera_distance);
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


static void processGR3CameraLookAt(const std::shared_ptr<GR::Element> &elem)
{
  double camera_x, camera_y, camera_z, center_x, center_y, center_z, up_x, up_y, up_z;

  camera_x = (double)elem->getAttribute("gr3cameralookat_camera-x");
  camera_y = (double)elem->getAttribute("gr3cameralookat_camera-y");
  camera_z = (double)elem->getAttribute("gr3cameralookat_camera-z");
  center_x = (double)elem->getAttribute("gr3cameralookat_center-x");
  center_y = (double)elem->getAttribute("gr3cameralookat_center-y");
  center_z = (double)elem->getAttribute("gr3cameralookat_center-z");
  up_x = (double)elem->getAttribute("gr3cameralookat_up-x");
  up_y = (double)elem->getAttribute("gr3cameralookat_up-y");
  up_z = (double)elem->getAttribute("gr3cameralookat_up-z");

  gr3_cameralookat(camera_x, camera_y, camera_z, center_x, center_y, center_z, up_x, up_y, up_z);
}

static void processGR3BackgroundColor(const std::shared_ptr<GR::Element> &elem)
{
  double r, g, b, a;
  r = (double)elem->getAttribute("gr3backgroundcolor_red");
  g = (double)elem->getAttribute("gr3backgroundcolor_green");
  b = (double)elem->getAttribute("gr3backgroundcolor_blue");
  a = (double)elem->getAttribute("gr3backgroundcolor_alpha");

  gr3_setbackgroundcolor(r, g, b, a);
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
      {std::string("colorrep"), processColorRep},
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
      {std::string("textencoding"),
       [](const std::shared_ptr<GR::Element> &elem) { gr_settextencoding((int)elem->getAttribute("textencoding")); }},
      {std::string("transparency"),
       [](const std::shared_ptr<GR::Element> &elem) {
         gr_settransparency((double)elem->getAttribute("transparency"));
       }},
      {std::string("linespec"),
       [](const std::shared_ptr<GR::Element> &elem) {
         gr_uselinespec(((std::string)elem->getAttribute("linespec")).data());
       }},
      {std::string("window"), processWindow},
      {std::string("window3d"), processWindow3d},
      {std::string("resamplemethod"),
       [](const std::shared_ptr<GR::Element>
              &elem) { gr_setresamplemethod((int)elem->getAttribute("resamplemethod")); }},
      {std::string("space3d"), processSpace3d},
      {std::string("viewport"), processViewport},
      {std::string("scale"),
       [](const std::shared_ptr<GR::Element> &elem) { gr_setscale((int)elem->getAttribute("scale")); }},
      {std::string("selntran"),
       [](const std::shared_ptr<GR::Element> &elem) { gr_selntran((int)elem->getAttribute("selntran")); }},
      {std::string("gr3cameralookat"), processGR3CameraLookAt},
      {std::string("gr3backgroundcolor"), processGR3BackgroundColor},
      {std::string("bordercolorind"),
       [](const std::shared_ptr<GR::Element>
              &elem) { gr_setbordercolorind((int)elem->getAttribute("bordercolorind")); }},
      {std::string("clipxform"),
       [](const std::shared_ptr<GR::Element> &elem) { gr_selectclipxform((int)elem->getAttribute("clipxform")); }}

  };

  for (auto &attribute : element->getAttributeNames())
    {
      auto start = 0U;
      auto end = attribute.find('_');
      if (end == std::string::npos)
        {
          if (attrStringToFunc.find(attribute) != attrStringToFunc.end())
            {
              attrStringToFunc[attribute](element);
            }
        }
      else
        {
          auto substr = attribute.substr(start, end);
          if (attrStringToFunc.find(substr) != attrStringToFunc.end())
            {
              attrStringToFunc[substr](element);
            }
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
      elemStringToFunc{{std::string("polymarker"), polymarker},
                       {std::string("polyline"), polyline},
                       {std::string("text"), text},
                       {std::string("fillarea"), fillArea},
                       {std::string("cellarray"), cellArray},
                       {std::string("axes"), axes},
                       {std::string("grid"), grid},
                       {std::string("drawimage"), drawImage},
                       {std::string("drawarc"), drawArc},
                       {std::string("fillarc"), fillArc},
                       {std::string("drawrect"), drawRect},
                       {std::string("fillrect"), fillRect},
                       {std::string("quiver"), quiver},
                       {std::string("contour"), contour},
                       {std::string("contourf"), contourf},
                       {std::string("hexbin"), hexbin},
                       {std::string("nonuniformcellarray"), nonuniformcellarray},
                       {std::string("grid3d"), grid3d},
                       {std::string("surface"), surface},
                       {std::string("axes3d"), axes3d},
                       {std::string("polyline3d"), polyline3d},
                       {std::string("polymarker3d"), polymarker3d},
                       {std::string("gr3drawmesh"), gr3DrawMesh},
                       {std::string("volume"), volume},
                       {std::string("trisurface"), triSurface},
                       {std::string("tricontour"), triContour},
                       {std::string("gr3clear"), gr3Clear},
                       {std::string("gr3deletemesh"), gr3DeleteMesh},
                       {std::string("gr3drawimage"), gr3DrawImage},
                       {std::string("shadepoints"), shadePoints},
                       {std::string("clearws"), clearWS},
                       {std::string("updatews"), updateWS},
                       {std::string("drawgraphics"), drawGraphics}

      };

  /*! Modifier */
  if (parentTypes.count(element->localName()))
    {
      processAttributes(element);
    }
  else
    {
      /*! Drawnodes */
      gr_savestate();
      processAttributes(element);
      try
        {
          std::function<void(const std::shared_ptr<GR::Element> &, const std::shared_ptr<GR::Context> &)> f =
              elemStringToFunc[element->localName()];
          f(element, context);
        }
      catch (std::__1::bad_function_call &e)
        {
          throw NotFoundError("No dom render function found for element with local name: " + element->localName());
        }
      gr_restorestate();
    }
}


//! Create Functions

std::shared_ptr<GR::Element>
GR::Render::createPolymarker(const std::string &x_key, std::optional<std::vector<double>> x, const std::string &y_key,
                             std::optional<std::vector<double>> y, const std::shared_ptr<GR::Context> &extContext,
                             int marker_type, double marker_size, int marker_colorind)
{
  /*!
   * This function can be used to create a Polymarker GR::Element
   *
   * \param[in] x_key A string used for storing the x coordinates in GR::Context
   * \param[in] x A vector containing double values representing x coordinates
   * \param[in] y_key A string used for storing the y coordinates in GR::Context
   * \param[in] y A vector containing double values representing y coordinates
   * \param[in] extContext A GR::Context that is used for storing the vectors. By default it uses GR::Render's
   * GR::Context object but an external GR::Context can be used \param[in] marker_type An Integer setting the
   * gr_markertype. By default it is 0 \param[in] marker_size A Double value setting the gr_markersize. By default it is
   * 0.0 \param[in] marker_colorind An Integer setting the gr_markercolorind. By default it is 0
   */

  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;
  auto element = createElement("polymarker");
  if (x != std::nullopt)
    {
      (*useContext)[x_key] = x.value();
    }
  element->setAttribute("x", x_key);

  if (y != std::nullopt)
    {
      (*useContext)[y_key] = y.value();
    }
  element->setAttribute("y", y_key);

  if (marker_type != 0)
    {
      element->setAttribute("markertype", marker_type);
    }
  if (marker_size != 0.0)
    {
      element->setAttribute("markersize", marker_size);
    }
  if (marker_colorind != 0)
    {
      element->setAttribute("markercolorind", marker_colorind);
    }
  return element;
}

std::shared_ptr<GR::Element> GR::Render::createPolymarker(double x, double y, int marker_type, double marker_size,
                                                          int marker_colorind)
{
  auto element = createElement("polymarker");
  element->setAttribute("x", x);
  element->setAttribute("y", y);
  if (marker_type != 0)
    {
      element->setAttribute("markertype", marker_type);
    }
  if (marker_size != 0.0)
    {
      element->setAttribute("markersize", marker_size);
    }
  if (marker_colorind != 0)
    {
      element->setAttribute("markercolorind", marker_colorind);
    }
  return element;
}

std::shared_ptr<GR::Element> GR::Render::createPolyline(double x1, double x2, double y1, double y2, int line_type,
                                                        double line_width, int line_colorind)
{
  auto element = createElement("polyline");
  element->setAttribute("x1", x1);
  element->setAttribute("x2", x2);
  element->setAttribute("y1", y1);
  element->setAttribute("y2", y2);
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


std::shared_ptr<GR::Element> GR::Render::createPolyline(const std::string &x_key, std::optional<std::vector<double>> x,
                                                        const std::string &y_key, std::optional<std::vector<double>> y,
                                                        const std::shared_ptr<GR::Context> &extContext, int line_type,
                                                        double line_width, int line_colorind)
{
  /*!
   * This function can be used to create a Polyline GR::Element
   *
   * \param[in] x_key A string used for storing the x coordinates in GR::Context
   * \param[in] x A vector containing double values representing x coordinates
   * \param[in] y_key A string used for storing the y coordinates in GR::Context
   * \param[in] y A vector containing double values representing y coordinates
   * \param[in] extContext A GR::Context that is used for storing the vectors. By default it uses GR::Render's
   * GR::Context object but an external GR::Context can be used \param[in] line_type An Integer setting the
   * gr_linertype. By default it is 0 \param[in] line_width A Double value setting the gr_linewidth. By default it is
   * 0.0 \param[in] marker_colorind An Integer setting the gr_linecolorind. By default it is 0
   */

  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;
  auto element = createElement("polyline");
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


std::shared_ptr<GR::Element> GR::Render::createText(double x, double y, const std::string &text, CoordinateSpace space)
{
  /*!
   * This function can be used to create a Text GR::Element
   *
   * \param[in] x A double value representing the x coordinate
   * \param[in] y A double value representing the y coordinate
   * \param[in] text A string
   * \param[in] space the coordinate space (WC or NDC) for x and y, default NDC
   */

  auto element = createElement("text");
  element->setAttribute("x", x);
  element->setAttribute("y", y);
  element->setAttribute("text", text);
  element->setAttribute("space", space);
  return element;
}


std::shared_ptr<GR::Element> GR::Render::createFillArea(const std::string &x_key, std::optional<std::vector<double>> x,
                                                        const std::string &y_key, std::optional<std::vector<double>> y,
                                                        const std::shared_ptr<GR::Context> &extContext,
                                                        int fillintstyle, int fillstyle, int fillcolorind)
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

  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;
  auto element = createElement("fillarea");
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
  if (fillcolorind != -1)
    {
      element->setAttribute("fillcolorind", fillcolorind);
    }
  return element;
}


std::shared_ptr<GR::Element> GR::Render::createCellArray(double xmin, double xmax, double ymin, double ymax, int dimx,
                                                         int dimy, int scol, int srow, int ncol, int nrow,
                                                         const std::string &color_key,
                                                         std::optional<std::vector<int>> color,
                                                         const std::shared_ptr<GR::Context> &extContext)
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

  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;
  auto element = createElement("cellarray");
  element->setAttribute("xmin", xmin);
  element->setAttribute("xmax", xmax);
  element->setAttribute("ymin", ymin);
  element->setAttribute("ymax", ymax);
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
  element->setAttribute("x_org", x_org);
  element->setAttribute("y_org", y_org);
  element->setAttribute("major_x", major_x);
  element->setAttribute("major_y", major_y);
  element->setAttribute("tick_size", tick_size);
  return element;
}


std::shared_ptr<GR::Element> GR::Render::createGrid(double x_tick, double y_tick, double x_org, double y_org,
                                                    int major_x, int major_y)
{
  auto element = createElement("grid");
  element->setAttribute("x_tick", x_tick);
  element->setAttribute("y_tick", y_tick);
  element->setAttribute("x_org", x_org);
  element->setAttribute("y_org", y_org);
  element->setAttribute("major_x", major_x);
  element->setAttribute("major_y", major_y);
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

std::shared_ptr<GR::Element> GR::Render::createGroup(const std::string &name)
{
  auto element = createElement("group");
  element->setAttribute("name", name);
  return element;
}


std::shared_ptr<GR::Element> GR::Render::createDrawImage(double xmin, double ymin, double xmax, double ymax, int width,
                                                         int height, const std::string &data_key,
                                                         std::optional<std::vector<int>> data, int model,
                                                         const std::shared_ptr<GR::Context> &extContext)
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
  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;
  auto element = createElement("drawimage");
  element->setAttribute("xmin", xmin);
  element->setAttribute("xmax", xmax);
  element->setAttribute("ymin", ymin);
  element->setAttribute("ymax", ymax);
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


std::shared_ptr<GR::Element> GR::Render::createDrawArc(double xmin, double xmax, double ymin, double ymax, double a1,
                                                       double a2)
{
  auto element = createElement("drawarc");
  element->setAttribute("xmin", xmin);
  element->setAttribute("xmax", xmax);
  element->setAttribute("ymin", ymin);
  element->setAttribute("ymax", ymax);
  element->setAttribute("a1", a1);
  element->setAttribute("a2", a2);

  return element;
}

std::shared_ptr<GR::Element> GR::Render::createFillArc(double xmin, double xmax, double ymin, double ymax, double a1,
                                                       double a2, int fillintstyle, int fillstyle, int fillcolorind)
{
  auto element = createElement("fillarc");
  element->setAttribute("xmin", xmin);
  element->setAttribute("xmax", xmax);
  element->setAttribute("ymin", ymin);
  element->setAttribute("ymax", ymax);
  element->setAttribute("a1", a1);
  element->setAttribute("a2", a2);


  if (fillintstyle != 0)
    {
      element->setAttribute("fillintstyle", fillintstyle);
    }
  if (fillstyle != 0)
    {
      element->setAttribute("fillstyle", fillstyle);
    }
  if (fillcolorind != -1)
    {
      element->setAttribute("fillcolorind", fillcolorind);
    }
  return element;
}


std::shared_ptr<GR::Element> GR::Render::createDrawRect(double xmin, double xmax, double ymin, double ymax)
{
  auto element = createElement("drawrect");
  element->setAttribute("xmin", xmin);
  element->setAttribute("xmax", xmax);
  element->setAttribute("ymin", ymin);
  element->setAttribute("ymax", ymax);

  return element;
}

std::shared_ptr<GR::Element> GR::Render::createFillRect(double xmin, double xmax, double ymin, double ymax,
                                                        int fillintstyle, int fillstyle, int fillcolorind)
{
  auto element = createElement("fillrect");
  element->setAttribute("xmin", xmin);
  element->setAttribute("xmax", xmax);
  element->setAttribute("ymin", ymin);
  element->setAttribute("ymax", ymax);

  if (fillintstyle != 0)
    {
      element->setAttribute("fillintstyle", fillintstyle);
    }
  if (fillstyle != 0)
    {
      element->setAttribute("fillstyle", fillstyle);
    }
  if (fillcolorind != -1)
    {
      element->setAttribute("fillcolorind", fillcolorind);
    }

  return element;
}


std::shared_ptr<GR::Element> GR::Render::createQuiver(const std::string &x_key, std::optional<std::vector<double>> x,
                                                      const std::string &y_key, std::optional<std::vector<double>> y,
                                                      const std::string &u_key, std::optional<std::vector<double>> u,
                                                      const std::string &v_key, std::optional<std::vector<double>> v,
                                                      int color, const std::shared_ptr<GR::Context> &extContext)
{
  /*
   * This function can be used to create a Quiver GR::Element
   *
   */
  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;
  auto element = createElement("quiver");
  element->setAttribute("x", x_key);
  element->setAttribute("y", y_key);
  element->setAttribute("u", u_key);
  element->setAttribute("v", v_key);
  element->setAttribute("color", color);

  if (x != std::nullopt)
    {
      (*useContext)[x_key] = *x;
    }
  if (y != std::nullopt)
    {
      (*useContext)[y_key] = *y;
    }
  if (u != std::nullopt)
    {
      (*useContext)[u_key] = *u;
    }
  if (v != std::nullopt)
    {
      (*useContext)[v_key] = *v;
    }

  return element;
}


std::shared_ptr<GR::Element> GR::Render::createContour(const std::string &px_key, std::optional<std::vector<double>> px,
                                                       const std::string &py_key, std::optional<std::vector<double>> py,
                                                       const std::string &h_key, std::optional<std::vector<double>> h,
                                                       const std::string &pz_key, std::optional<std::vector<double>> pz,
                                                       int major_h, const std::shared_ptr<GR::Context> &extContext)
{
  /*
   * This function can be used to create a contour GR::Element
   *
   */
  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;
  auto element = createElement("contour");
  element->setAttribute("px", px_key);
  element->setAttribute("py", py_key);
  element->setAttribute("h", h_key);
  element->setAttribute("pz", pz_key);
  element->setAttribute("major_h", major_h);

  if (px != std::nullopt)
    {
      (*useContext)[px_key] = *px;
    }
  if (py != std::nullopt)
    {
      (*useContext)[py_key] = *py;
    }
  if (h != std::nullopt)
    {
      (*useContext)[h_key] = *h;
    }
  if (pz != std::nullopt)
    {
      (*useContext)[pz_key] = *pz;
    }

  return element;
}


std::shared_ptr<GR::Element> GR::Render::createContourf(const std::string &px_key,
                                                        std::optional<std::vector<double>> px,
                                                        const std::string &py_key,
                                                        std::optional<std::vector<double>> py, const std::string &h_key,
                                                        std::optional<std::vector<double>> h, const std::string &pz_key,
                                                        std::optional<std::vector<double>> pz, int major_h,
                                                        const std::shared_ptr<GR::Context> &extContext)
{
  /*!
   * This function can be used to create a contour GR::Element
   *
   */
  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;
  auto element = createElement("contourf");

  element->setAttribute("px", px_key);
  element->setAttribute("py", py_key);
  element->setAttribute("h", h_key);
  element->setAttribute("pz", pz_key);
  element->setAttribute("major_h", major_h);

  if (px != std::nullopt)
    {
      (*useContext)[px_key] = *px;
    }
  if (py != std::nullopt)
    {
      (*useContext)[py_key] = *py;
    }
  if (h != std::nullopt)
    {
      (*useContext)[h_key] = *h;
    }
  if (pz != std::nullopt)
    {
      (*useContext)[pz_key] = *pz;
    }

  return element;
}


std::shared_ptr<GR::Element> GR::Render::createHexbin(int x_length, const std::string &x_key,
                                                      std::optional<std::vector<double>> x, const std::string &y_key,
                                                      std::optional<std::vector<double>> y, int nbins,
                                                      const std::shared_ptr<GR::Context> &extContext)
{
  /*!
   * This function can be used to create a hexbin GR::Element
   */
  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;

  auto element = createElement("hexbin");
  element->setAttribute("x_length", x_length);
  element->setAttribute("x", x_key);
  element->setAttribute("y", y_key);
  element->setAttribute("nbins", nbins);

  if (x != std::nullopt)
    {
      (*useContext)[x_key] = *x;
    }
  if (y != std::nullopt)
    {
      (*useContext)[y_key] = *y;
    }

  return element;
}

std::shared_ptr<GR::Element> GR::Render::createNonUniformCellArray(
    const std::string &x_key, std::optional<std::vector<double>> x, const std::string &y_key,
    std::optional<std::vector<double>> y, int dimx, int dimy, int scol, int srow, int ncol, int nrow,
    const std::string &color_key, std::optional<std::vector<int>> color, const std::shared_ptr<GR::Context> &extContext)
{
  /*!
   * This function can be used to create a non uniform cell array GR::Element
   */
  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;

  auto element = createElement("nonuniformcellarray");
  element->setAttribute("x", x_key);
  element->setAttribute("y", y_key);
  element->setAttribute("color", color_key);
  element->setAttribute("dimx", dimx);
  element->setAttribute("dimy", dimy);
  element->setAttribute("scol", scol);
  element->setAttribute("srow", srow);
  element->setAttribute("ncol", ncol);
  element->setAttribute("nrow", nrow);

  if (x != std::nullopt)
    {
      (*useContext)[x_key] = *x;
    }
  if (y != std::nullopt)
    {
      (*useContext)[y_key] = *y;
    }
  if (color != std::nullopt)
    {
      (*useContext)[color_key] = *color;
    }

  return element;
}

std::shared_ptr<GR::Element> GR::Render::createSurface(const std::string &px_key, std::optional<std::vector<double>> px,
                                                       const std::string py_key, std::optional<std::vector<double>> py,
                                                       const std::string &pz_key, std::optional<std::vector<double>> pz,
                                                       int option, const std::shared_ptr<GR::Context> &extContext)
{
  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;
  auto element = createElement("surface");
  element->setAttribute("px", px_key);
  element->setAttribute("py", py_key);
  element->setAttribute("pz", pz_key);
  element->setAttribute("option", option);

  if (px != std::nullopt)
    {
      (*useContext)[px_key] = *px;
    }
  if (py != std::nullopt)
    {
      (*useContext)[py_key] = *py;
    }
  if (pz != std::nullopt)
    {
      (*useContext)[pz_key] = *pz;
    }

  return element;
}

std::shared_ptr<GR::Element> GR::Render::createGrid3d(double x_tick, double y_tick, double z_tick, double x_org,
                                                      double y_org, double z_org, int major_x, int major_y, int major_z)
{
  auto element = createElement("grid3d");
  element->setAttribute("x_tick", x_tick);
  element->setAttribute("y_tick", y_tick);
  element->setAttribute("z_tick", z_tick);
  element->setAttribute("x_org", x_org);
  element->setAttribute("y_org", y_org);
  element->setAttribute("z_org", z_org);
  element->setAttribute("major_x", major_x);
  element->setAttribute("major_y", major_y);
  element->setAttribute("major_z", major_z);
  return element;
}

std::shared_ptr<GR::Element> GR::Render::createAxes3d(double x_tick, double y_tick, double z_tick, double x_org,
                                                      double y_org, double z_org, int major_x, int major_y, int major_z,
                                                      double tick_size)
{
  auto element = createElement("axes3d");
  element->setAttribute("x_tick", x_tick);
  element->setAttribute("y_tick", y_tick);
  element->setAttribute("z_tick", z_tick);
  element->setAttribute("x_org", x_org);
  element->setAttribute("y_org", y_org);
  element->setAttribute("z_org", z_org);
  element->setAttribute("major_x", major_x);
  element->setAttribute("major_y", major_y);
  element->setAttribute("major_z", major_z);
  element->setAttribute("tick_size", tick_size);
  return element;
}


std::shared_ptr<GR::Element>
GR::Render::createPolyline3d(const std::string &x_key, std::optional<std::vector<double>> x, const std::string &y_key,
                             std::optional<std::vector<double>> y, const std::string &z_key,
                             std::optional<std::vector<double>> z, const std::shared_ptr<GR::Context> &extContext)
{
  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;

  auto element = createElement("polyline3d");
  element->setAttribute("x", x_key);
  element->setAttribute("y", y_key);
  element->setAttribute("z", z_key);

  if (x != std::nullopt)
    {
      (*useContext)[x_key] = *x;
    }
  if (y != std::nullopt)
    {
      (*useContext)[y_key] = *y;
    }
  if (z != std::nullopt)
    {
      (*useContext)[z_key] = *z;
    }

  return element;
}


std::shared_ptr<GR::Element>
GR::Render::createPolymarker3d(const std::string &x_key, std::optional<std::vector<double>> x, const std::string &y_key,
                               std::optional<std::vector<double>> y, const std::string &z_key,
                               std::optional<std::vector<double>> z, const std::shared_ptr<GR::Context> &extContext)
{
  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;

  auto element = createElement("polymarker3d");
  element->setAttribute("x", x_key);
  element->setAttribute("y", y_key);
  element->setAttribute("z", z_key);

  if (x != std::nullopt)
    {
      (*useContext)[x_key] = *x;
    }
  if (y != std::nullopt)
    {
      (*useContext)[y_key] = *y;
    }
  if (z != std::nullopt)
    {
      (*useContext)[z_key] = *z;
    }

  return element;
}


std::shared_ptr<GR::Element>
GR::Render::createGR3DrawMesh(int mesh, int n, const std::string &positions_key,
                              std::optional<std::vector<double>> positions, const std::string &directions_key,
                              std::optional<std::vector<double>> directions, const std::string &ups_key,
                              std::optional<std::vector<double>> ups, const std::string &colors_key,
                              std::optional<std::vector<double>> colors, const std::string &scales_key,
                              std::optional<std::vector<double>> scales, const std::shared_ptr<GR::Context> &extContext)
{

  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;

  auto element = createElement("gr3drawmesh");
  element->setAttribute("mesh", mesh);
  element->setAttribute("n", n);
  element->setAttribute("positions", positions_key);
  element->setAttribute("directions", directions_key);
  element->setAttribute("ups", ups_key);
  element->setAttribute("colors", colors_key);
  element->setAttribute("scales", scales_key);


  if (positions != std::nullopt)
    {
      (*useContext)[positions_key] = *positions;
    }
  if (directions != std::nullopt)
    {
      (*useContext)[directions_key] = *directions;
    }
  if (ups != std::nullopt)
    {
      (*useContext)[ups_key] = *ups;
    }
  if (colors != std::nullopt)
    {
      (*useContext)[colors_key] = *colors;
    }
  if (scales != std::nullopt)
    {
      (*useContext)[scales_key] = *scales;
    }
  return element;
}

std::shared_ptr<GR::Element> GR::Render::createVolume(int nx, int ny, int nz, const std::string &data_key,
                                                      std::optional<std::vector<double>> data, int algorithm,
                                                      double dmin, double dmax,
                                                      const std::shared_ptr<GR::Context> &extContext)
{
  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;

  auto element = createElement("volume");
  element->setAttribute("data", data_key);
  element->setAttribute("nx", nx);
  element->setAttribute("ny", ny);
  element->setAttribute("nz", nz);
  element->setAttribute("algorithm", algorithm);
  element->setAttribute("dmin", dmin);
  element->setAttribute("dmax", dmax);


  if (data != std::nullopt)
    {
      (*useContext)[data_key] = *data;
    }


  return element;
}

std::shared_ptr<GR::Element> GR::Render::createVolume(int nx, int ny, int nz, const std::string &data_key,
                                                      std::optional<std::vector<double>> data, int algorithm,
                                                      const std::string &dmin_key, double dmin,
                                                      const std::string &dmax_key, double dmax,
                                                      const std::shared_ptr<GR::Context> &extContext)
{
  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;

  auto element = createElement("volume");
  element->setAttribute("data", data_key);
  element->setAttribute("nx", nx);
  element->setAttribute("ny", ny);
  element->setAttribute("nz", nz);
  element->setAttribute("algorithm", algorithm);
  element->setAttribute("dmin", dmin_key);
  element->setAttribute("dmax", dmax_key);


  if (auto dmin_ptr = GR::get_if<std::vector<double>>((*useContext)[dmin_key]))
    {
      dmin_ptr->push_back(dmin);
    }
  else
    {
      std::vector<double> dmin_vec{dmin};
      (*useContext)[dmin_key] = dmin_vec;
    }
  if (auto dmax_ptr = GR::get_if<std::vector<double>>((*useContext)[dmax_key]))
    {
      dmax_ptr->push_back(dmax);
    }
  else
    {
      std::vector<double> dmax_vec{dmax};
      (*useContext)[dmax_key] = dmax_vec;
    }

  if (data != std::nullopt)
    {
      (*useContext)[data_key] = *data;
    }


  return element;
}

std::shared_ptr<GR::Element>
GR::Render::createTriSurface(const std::string &px_key, std::optional<std::vector<double>> px,
                             const std::string &py_key, std::optional<std::vector<double>> py,
                             const std::string &pz_key, std::optional<std::vector<double>> pz,
                             const std::shared_ptr<GR::Context> &extContext)
{
  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;
  auto element = createElement("trisurface");
  element->setAttribute("px", px_key);
  element->setAttribute("py", py_key);
  element->setAttribute("pz", pz_key);

  if (px != std::nullopt)
    {
      (*useContext)[px_key] = *px;
    }
  if (py != std::nullopt)
    {
      (*useContext)[py_key] = *py;
    }
  if (pz != std::nullopt)
    {
      (*useContext)[pz_key] = *pz;
    }

  return element;
}

std::shared_ptr<GR::Element>
GR::Render::createTriContour(const std::string &px_key, std::optional<std::vector<double>> px,
                             const std::string &py_key, std::optional<std::vector<double>> py,
                             const std::string &pz_key, std::optional<std::vector<double>> pz,
                             const std::string &levels_key, std::optional<std::vector<double>> levels,
                             const std::shared_ptr<GR::Context> &extContext)
{
  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;
  auto element = createElement("tricontour");
  element->setAttribute("px", px_key);
  element->setAttribute("py", py_key);
  element->setAttribute("pz", pz_key);
  element->setAttribute("levels", levels_key);

  if (px != std::nullopt)
    {
      (*useContext)[px_key] = *px;
    }
  if (py != std::nullopt)
    {
      (*useContext)[py_key] = *py;
    }
  if (pz != std::nullopt)
    {
      (*useContext)[pz_key] = *pz;
    }
  if (levels != std::nullopt)
    {
      (*useContext)[levels_key] = *levels;
    }

  return element;
}


std::shared_ptr<GR::Element> GR::Render::createGR3Clear()
{
  auto element = createElement("gr3clear");
  return element;
}

std::shared_ptr<GR::Element> GR::Render::createGR3DeleteMesh(int mesh)
{
  auto element = createElement("gr3deletemesh");
  element->setAttribute("mesh", mesh);
  return element;
}

std::shared_ptr<GR::Element> GR::Render::createGR3DrawImage(double xmin, double xmax, double ymin, double ymax,
                                                            int width, int height, int drawable_type)
{
  auto element = createElement("gr3drawimage");
  element->setAttribute("xmin", xmin);
  element->setAttribute("xmax", xmax);
  element->setAttribute("ymin", ymin);
  element->setAttribute("ymax", ymax);
  element->setAttribute("width", width);
  element->setAttribute("height", height);
  element->setAttribute("drawable_type", drawable_type);
  return element;
}

std::shared_ptr<GR::Element> GR::Render::createShadePoints(const std::string &x_key,
                                                           std::optional<std::vector<double>> x,
                                                           const std::string &y_key,
                                                           std::optional<std::vector<double>> y, int xform, int w,
                                                           int h, const std::shared_ptr<GR::Context> &extContext)
{
  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;
  auto element = createElement("shadepoints");
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
  element->setAttribute("xform", xform);
  element->setAttribute("w", w);
  element->setAttribute("h", h);
  return element;
}


std::shared_ptr<GR::Element> GR::Render::createClearWS()
{
  auto element = createElement("clearws");
  return element;
}

std::shared_ptr<GR::Element> GR::Render::createUpdateWS()
{
  return createElement("updatews");
}

std::shared_ptr<GR::Element> GR::Render::createDrawGraphics(const std::string &data_key,
                                                            std::optional<std::vector<int>> data,
                                                            const std::shared_ptr<GR::Context> &extContext)
{

  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;

  auto element = createElement("drawgraphics");

  if (data != std::nullopt)
    {
      (*useContext)[data_key] = *data;
    }
  element->setAttribute("data", data_key);
  return element;
}


// std::shared_ptr<GR::Element> GR::Render::createGR3IsoSurfaceMesh(int* mesh, GR3_MC_DTYPE *data, GR3_MC_DTYPE
// isolevel, unsigned int dim_x, unsigned int dim_y, unsigned int dim_z, unsigned int stride_x, unsigned int stride_y,
// unsigned int stride_z, double step_x, double step_y, double step_z, double offset_x, double offset_y, double
// offset_z);


//! Modifierfunctions

void GR::Render::setViewport(const std::shared_ptr<GR::Element> &element, double xmin, double xmax, double ymin,
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
                               const std::vector<int> &types, const std::shared_ptr<GR::Context> &extContext)
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
  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;
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
                               const std::vector<double> &sizes, const std::shared_ptr<GR::Context> &extContext)
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
  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;
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
                                   const std::vector<int> &colorinds, const std::shared_ptr<GR::Context> &extContext)
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
  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;
  (*useContext)[colorinds_key] = colorinds;
  element->setAttribute("markercolorinds", colorinds_key);
}


void GR::Render::setLineType(const std::shared_ptr<Element> &element, const std::string &types_key,
                             const std::vector<int> &types, const std::shared_ptr<GR::Context> &extContext)
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
  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;
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
                              const std::vector<double> &widths, const std::shared_ptr<GR::Context> &extContext)
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
  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;
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
                                 const std::vector<int> &colorinds, const std::shared_ptr<GR::Context> &extContext)
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
  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;
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
   * This function can be used to set TextFontPrec of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] font An Integer value representing a font type
   * \param[in] prec An Integer value representing a font precision
   */
  element->setAttribute("textfontprec", true);
  element->setAttribute("textfontprec_font", font);
  element->setAttribute("textfontprec_prec", prec);
}


void GR::Render::setCharUp(const std::shared_ptr<Element> &element, double ux, double uy)
{
  /*!
   * This function can be used to set CharUp of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] ux  X coordinate of the text up vector
   * \param[in] uy  y coordinate of the text up vector
   */
  element->setAttribute("charup", true);
  element->setAttribute("charup_ux", ux);
  element->setAttribute("charup_uy", uy);
}

void GR::Render::setTextAlign(const std::shared_ptr<Element> &element, int horizontal, int vertical)
{
  /*!
   * This function can be used to set TextAlign of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] horizontal  Horizontal text alignment
   * \param[in] vertical Vertical text alignment
   */
  element->setAttribute("textalign", true);
  element->setAttribute("textalign_horizontal", horizontal);
  element->setAttribute("textalign_vertical", vertical);
}


void GR::Render::setLineSpec(const std::shared_ptr<Element> &element, const std::string &spec)
{
  /*!
   * This function can be used to set the linespec of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] spec An std::string
   *
   */
  element->setAttribute("linespec", spec);
}

void GR::Render::setColorRep(const std::shared_ptr<Element> &element, int index, double red, double green, double blue)
{
  /*!
   * This function can be used to set the colorrep of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] index Color index in the range 0 to 1256
   * \param[in] red Red intensity in the range 0.0 to 1.0
   * \param[in] green Green intensity in the range 0.0 to 1.0
   * \param[in] blue Blue intensity in the range 0.0 to 1.0
   */

  int precision = 255;
  int red_int = red * precision, green_int = green * precision, blue_int = blue * precision;


  // Convert RGB to hex
  std::stringstream stream;
  std::string hex;
  stream << std::hex << (red_int << 16 | green_int << 8 | blue_int);

  std::string name = "colorrep_" + std::to_string(index);

  element->setAttribute(name, stream.str());
}

void GR::Render::setFillIntStyle(const std::shared_ptr<GR::Element> &element, int index)
{
  /*!
   * This function can be used to set the fillintstyle of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] index The style of fill to be used
   */
  element->setAttribute("fillintstyle", index);
}


void GR::Render::setFillColorInd(const std::shared_ptr<GR::Element> &element, int color)
{
  /*!
   * This function can be used to set the fillcolorind of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] color The fill area color index (COLOR < 1256)
   */
  element->setAttribute("fillcolorind", color);
}

void GR::Render::setFillStyle(const std::shared_ptr<GR::Element> &element, int index)
{
  /*!
   * This function can be used to set the fillintstyle of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] index The fill style index to be used
   */

  element->setAttribute("fillstyle", index);
}


void GR::Render::setScale(const std::shared_ptr<GR::Element> &element, int scale)
{
  /*!
   * This function can be used to set the scale of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] index The scale index to be used
   */
  element->setAttribute("scale", scale);
}

void GR::Render::setWindow3d(const std::shared_ptr<GR::Element> &element, double xmin, double xmax, double ymin,
                             double ymax, double zmin, double zmax)
{
  /*!
   * This function can be used to set the window3d of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] xmin The left horizontal coordinate of the window (xmin < xmax)
   * \param[in] xmax The right horizontal coordinate of the window (xmin < xmax)
   * \param[in] ymin The bottom vertical coordinate of the window (ymin < ymax)
   * \param[in] ymax The top vertical coordinate of the window (ymin < ymax)
   * \param[in] zmin min z-value
   * \param[in] zmax max z-value
   */

  element->setAttribute("window3d", true);
  element->setAttribute("window3d_xmin", xmin);
  element->setAttribute("window3d_xmax", xmax);
  element->setAttribute("window3d_ymin", ymin);
  element->setAttribute("window3d_ymax", ymax);
  element->setAttribute("window3d_zmin", zmin);
  element->setAttribute("window3d_zmax", zmax);
}

void GR::Render::setSpace3d(const std::shared_ptr<GR::Element> &element, double phi, double theta, double fov,
                            double camera_distance)
{
  /*! This function can be used to set the window3d of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] phi: azimuthal angle of the spherical coordinates
   * \param[in] theta: polar angle of the spherical coordinates
   * \param[in] fov: vertical field of view(0 or NaN for orthographic projection)
   * \param[in] camera_distance: distance between the camera and the focus point (in arbitrary units, 0 or NaN for the
   * radius of the object’s smallest bounding sphere)
   */

  element->setAttribute("space3d", true);
  element->setAttribute("space3d_phi", phi);
  element->setAttribute("space3d_theta", theta);
  element->setAttribute("space3d_fov", fov);
  element->setAttribute("space3d_camera-distance", camera_distance);
}

void GR::Render::setSelntran(const std::shared_ptr<Element> &element, int transform)
{
  /*! This function can be used to set the window3d of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] transform Select a predefined transformation from world coordinates to normalized device coordinates.
   */

  element->setAttribute("selntran", transform);
}


void GR::Render::setGR3BackgroundColor(const std::shared_ptr<GR::Element> &element, double red, double green,
                                       double blue, double alpha)
{
  /*! This function can be used to set the gr3 backgroundcolor of a GR::Element
   *
   * \param[in] element A GR::Element
   */
  element->setAttribute("gr3backgroundcolor", true);
  element->setAttribute("gr3backgroundcolor_red", red);
  element->setAttribute("gr3backgroundcolor_green", green);
  element->setAttribute("gr3backgroundcolor_blue", blue);
  element->setAttribute("gr3backgroundcolor_alpha", alpha);
}

void GR::Render::setGR3CameraLookAt(const std::shared_ptr<GR::Element> &element, double camera_x, double camera_y,
                                    double camera_z, double center_x, double center_y, double center_z, double up_x,
                                    double up_y, double up_z)
{
  /*! This function can be used to set the gr3 camerlookat of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] camera_x: The x-coordinate of the camera
   * \param[in] camera_y: The y-coordinate of the camera
   * \param[in] camera_z: The z-coordinate of the camera
   * \param[in] center_x: The x-coordinate of the center of focus
   * \param[in] center_y: The y-coordinate of the center of focus
   * \param[in] center_z: The z-coordinate of the center of focus
   * \param[in] up_x: The x-component of the up direction
   * \param[in] up_y: The y-component of the up direction
   * \param[in] up_z: The z-component of the up direction
   */

  element->setAttribute("gr3cameralookat_camera-x", camera_x);
  element->setAttribute("gr3cameralookat_camera-y", camera_y);
  element->setAttribute("gr3cameralookat_camera-z", camera_z);
  element->setAttribute("gr3cameralookat_center-x", center_x);
  element->setAttribute("gr3cameralookat_center-y", center_y);
  element->setAttribute("gr3cameralookat_center-z", center_z);
  element->setAttribute("gr3cameralookat_up-x", up_x);
  element->setAttribute("gr3cameralookat_up-y", up_y);
  element->setAttribute("gr3cameralookat_up-z", up_z);
}


void GR::Render::setTextColorInd(const std::shared_ptr<GR::Element> &element, int index)
{
  /*!
   * This function can be used to set the textcolorind of a GR::Element
   * \param[in] element A GR::Element
   * \param[in] index The color index
   */

  element->setAttribute("textcolorind", index);
}


void GR::Render::setBorderColorInd(const std::shared_ptr<GR::Element> &element, int index)
{
  /*!
   * This function can be used to set the bordercolorind of a GR::Element
   * \param[in] element A GR::Element
   * \param[in] index The color index
   */
  element->setAttribute("bordercolorind", index);
}

void GR::Render::selectClipXForm(const std::shared_ptr<GR::Element> &element, int form)
{
  /*!
   * This function can be used to set the clipxform of a GR::Element
   * \param[in] element A GR::Element
   * \param[in] form the clipxform
   */
  element->setAttribute("clipxform", form);
}

void GR::Render::setCharHeight(const std::shared_ptr<GR::Element> &element, double height)
{
  /*!
   * This function can be used to set the charheight of a GR::Element
   * \param[in] element A GR::Element
   * \param[in] height the charheight
   */
  element->setAttribute("charheight", height);
}


void GR::Render::setTransparency(const std::shared_ptr<GR::Element> &element, double alpha)
{
  /*!
   * This function can be used to set the transparency of a GR::Element
   * \param[in] element A GR::Element
   * \param[in] alpha The alpha
   */
  element->setAttribute("transparency", alpha);
}

void GR::Render::setResampleMethod(const std::shared_ptr<GR::Element> &element, int resample)
{
  /*!
   * This function can be used to set the resamplemethod of a GR::Element
   * \param[in] element A GR::Element
   * \param[in] resample The resample method
   */

  element->setAttribute("resamplemethod", resample);
}


void GR::Render::setTextEncoding(const std::shared_ptr<Element> &element, int encoding)
{
  /*!
   * This function can be used to set the textencoding of a GR::Element
   * \param[in] element A GR::Element
   * \param[in] encoding The textencoding
   */
  element->setAttribute("textencoding", encoding);
}


//! Render functions
static void renderHelper(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Recursive helper function for render; Not part of render class
   * Only renders / processes children if the parent is in parentTypes (group etc.)
   * Used for traversing the tree
   *
   * \param[in] element A GR::Element
   * \param[in] context A GR::Context
   */
  processElement(element, context);
  if (element->hasChildNodes() && parentTypes.count(element->localName()))
    {
      for (const auto &child : element->children())
        {
          gr_savestate();
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
  if (root->hasChildNodes())
    {
      for (const auto &child : root->children())
        {
          gr_savestate();
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
  if (root->hasChildNodes())
    {
      for (const auto &child : root->children())
        {
          gr_savestate();
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
  if (root->hasChildNodes())
    {
      for (const auto &child : root->children())
        {
          gr_savestate();
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
  if (root->hasChildNodes())
    {
      for (const auto &child : root->children())
        {
          gr_savestate();
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
  this->context = std::shared_ptr<GR::Context>(new Context());
}
