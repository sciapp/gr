#ifdef _WIN32
/*
 * Headers on Windows can define `min` and `max` as macros which causes
 * problem when ustd::sing `std::min` and `std::max`
 * -> Define `NOMINMAX` to prevent the definition of these macros
 */
#define NOMINMAX
#endif

#define _USE_MATH_DEFINES


#include <functional>
#include <vector>
#include <set>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <grm/dom_render/graphics_tree/Element.hxx>
#include <grm/dom_render/graphics_tree/Document.hxx>
#include <grm/dom_render/graphics_tree/Value.hxx>
#include <grm/dom_render/graphics_tree/util.hxx>
#include <grm/dom_render/render.hxx>
#include <grm/dom_render/NotFoundError.hxx>
#include <grm/dom_render/context.hxx>
#include "gks.h"
#include "gr.h"
#include "gr3.h"
#include "grm/layout.hxx"
#include "grm/plot_int.h"

std::shared_ptr<GR::Element> global_root;
std::shared_ptr<GR::Render> global_render;

//! This vector is used for storing element types which children get processed. Other types' children will be ignored
static std::set<std::string> parentTypes = {"group",           "layout-grid",     "layout-gridelement",
                                            "draw-legend",     "draw-polar-axes", "pie-plot-title-render",
                                            "draw-pie-legend", "marginalheatmap"};

static std::map<std::string, double> symbol_to_meters_per_unit{
    {"m", 1.0},     {"dm", 0.1},    {"cm", 0.01},  {"mm", 0.001},        {"in", 0.0254},
    {"\"", 0.0254}, {"ft", 0.3048}, {"'", 0.0254}, {"pc", 0.0254 / 6.0}, {"pt", 0.0254 / 72.0},
};

double auto_tick(double amin, double amax)
{
  double tick_size[] = {5.0, 2.0, 1.0, 0.5, 0.2, 0.1, 0.05, 0.02, 0.01};
  double scale, tick;
  int i, n;

  scale = pow(10.0, (int)(log10(amax - amin)));
  tick = 1.0;
  for (i = 0; i < 9; i++)
    {
      n = (amax - amin) / scale / tick_size[i];
      if (n > 7)
        {
          tick = tick_size[i - 1];
          break;
        }
    }
  tick *= scale;
  return tick;
}

static void markerHelper(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context,
                         const std::string &str)
{
  /*!
   * Helperfunction for marker functions ustd::sing vectors for marker parameters
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
  int skipColorInd = -1000;

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


  auto n = std::min(x_vec.size(), y_vec.size());

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
              if (colorind[i] == skipColorInd)
                {
                  continue;
                }
              gr_setmarkercolorind(colorind[i]);
            }
          else
            {
              if (colorind.back() == skipColorInd)
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
   * Helperfunction for line functions ustd::sing vectors for line parameters
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

static std::shared_ptr<GR::Element> getSubplotElement(const std::shared_ptr<GR::Element> &element)
{
  auto ancestor = element;

  while (ancestor->localName() != "root")
    {
      bool ancestorIsSubplotGroup =
          (ancestor->hasAttribute("subplotGroup") && static_cast<int>(ancestor->getAttribute("subplotGroup")));
      if (ancestor->localName() == "layout-gridelement" || ancestorIsSubplotGroup)
        {
          return ancestor;
        }
      else
        {
          ancestor = ancestor->parentElement();
        }
    }
  return nullptr;
}

static void getTickSize(const std::shared_ptr<GR::Element> &element, double &tick_size)
{
  if (element->hasAttribute("tick_size"))
    {
      tick_size = static_cast<double>(element->getAttribute("tick_size"));
    }
  else
    {
      double viewport[4];
      auto subplot_element = getSubplotElement(element);
      viewport[0] = (double)subplot_element->getAttribute("viewport_xmin");
      viewport[1] = (double)subplot_element->getAttribute("viewport_xmax");
      viewport[2] = (double)subplot_element->getAttribute("viewport_ymin");
      viewport[3] = (double)subplot_element->getAttribute("viewport_ymax");

      double diag = std::sqrt((viewport[1] - viewport[0]) * (viewport[1] - viewport[0]) +
                              (viewport[3] - viewport[2]) * (viewport[3] - viewport[2]));

      tick_size = 0.0075 * diag;
    }
}

static void getMajorCount(const std::shared_ptr<GR::Element> &element, const std::string kind, int &major_count)
{
  if (element->hasAttribute("major"))
    {
      major_count = static_cast<int>(element->getAttribute("major"));
    }
  else
    {
      if (str_equals_any(kind.c_str(), 6, "wireframe", "surface", "plot3", "scatter3", "polar", "trisurf"))
        {
          major_count = 2;
        }
      else
        {
          major_count = 5;
        }
    }
}

static void getAxesInformation(const std::shared_ptr<GR::Element> &element, std::string x_org_pos,
                               std::string y_org_pos, double &x_org, double &y_org, int &x_major, int &y_major,
                               double &x_tick, double &y_tick)
{
  double x_org_low, x_org_high;
  double y_org_low, y_org_high;
  int major_count;

  auto draw_axes_group = element->parentElement();
  auto subplot_element = getSubplotElement(element);
  std::string kind = static_cast<std::string>(subplot_element->getAttribute("kind"));
  int scale = static_cast<int>(subplot_element->getAttribute("scale"));
  double xmin = static_cast<double>(subplot_element->getAttribute("window_xmin"));
  double xmax = static_cast<double>(subplot_element->getAttribute("window_xmax"));
  double ymin = static_cast<double>(subplot_element->getAttribute("window_ymin"));
  double ymax = static_cast<double>(subplot_element->getAttribute("window_ymax"));

  getMajorCount(element, kind, major_count);

  if (element->hasAttribute("x_major"))
    {
      x_major = static_cast<int>(element->getAttribute("x_major"));
    }
  else
    {
      if (!(scale & GR_OPTION_X_LOG))
        {
          if (kind == "barplot")
            {
              if (draw_axes_group->hasAttribute("xticklabels"))
                {
                  x_major = 0;
                }
              else
                {
                  x_major = 1;
                }
            }
          else
            {
              x_major = major_count;
            }
        }
      else
        {
          x_major = 1;
        }
    }

  if (element->hasAttribute("x_tick"))
    {
      x_tick = static_cast<double>(element->getAttribute("x_tick"));
    }
  else
    {
      if (!(scale & GR_OPTION_X_LOG))
        {
          if (kind == "barplot")
            {
              x_tick = 1;
            }
          else
            {
              if (x_major != 0)
                {
                  x_tick = auto_tick(xmin, xmax) / x_major;
                }
              else
                {
                  x_tick = 1;
                }
            }
        }
      else
        {
          x_tick = 1;
        }
    }

  if (element->hasAttribute("x_org"))
    {
      x_org = static_cast<double>(element->getAttribute("x_org"));
    }
  else
    {
      if (!(scale & GR_OPTION_FLIP_X))
        {
          x_org_low = xmin;
          x_org_high = xmax;
        }
      else
        {
          x_org_low = xmax;
          x_org_high = xmin;
        }
      if (x_org_pos == "low")
        {
          x_org = x_org_low;
        }
      else
        {
          x_org = x_org_high;
          x_major = -x_major;
        }
    }


  if (element->hasAttribute("y_major"))
    {
      y_major = static_cast<int>(element->getAttribute("y_major"));
    }
  else
    {
      if (!(scale & GR_OPTION_Y_LOG))
        {
          y_major = major_count;
        }
      else
        {
          y_major = 1;
        }
    }

  if (element->hasAttribute("y_tick"))
    {
      y_tick = static_cast<double>(element->getAttribute("y_tick"));
    }
  else
    {
      if (!(scale & GR_OPTION_Y_LOG))
        {
          if (y_major != 0)
            {
              y_tick = auto_tick(ymin, ymax) / y_major;
            }
          else
            {
              y_tick = 1;
            }
        }
      else
        {
          y_tick = 1;
        }
    }

  if (element->hasAttribute("y_org"))
    {
      y_org = static_cast<double>(element->getAttribute("y_org"));
    }
  else
    {
      if (!(scale & GR_OPTION_FLIP_Y))
        {
          y_org_low = ymin;
          y_org_high = ymax;
        }
      else
        {
          y_org_low = ymax;
          y_org_high = ymin;
        }
      if (y_org_pos == "low")
        {
          y_org = y_org_low;
        }
      else
        {
          y_org = y_org_high;
          y_major = -y_major;
        }
    }
}

static void getAxes3dInformation(const std::shared_ptr<GR::Element> &element, std::string x_org_pos,
                                 std::string y_org_pos, std::string z_org_pos, double &x_org, double &y_org,
                                 double &z_org, int &x_major, int &y_major, int &z_major, double &x_tick,
                                 double &y_tick, double &z_tick)
{
  getAxesInformation(element, x_org_pos, y_org_pos, x_org, y_org, x_major, y_major, x_tick, y_tick);

  double z_org_low, z_org_high;
  int major_count;

  auto draw_axes_group = element->parentElement();
  auto subplot_element = getSubplotElement(element);
  std::string kind = static_cast<std::string>(subplot_element->getAttribute("kind"));
  int scale = static_cast<int>(subplot_element->getAttribute("scale"));
  double zmin = static_cast<double>(subplot_element->getAttribute("window_zmin"));
  double zmax = static_cast<double>(subplot_element->getAttribute("window_zmax"));

  getMajorCount(element, kind, major_count);

  if (element->hasAttribute("z_major"))
    {
      z_major = static_cast<int>(element->getAttribute("z_major"));
    }
  else
    {
      if (!(scale & GR_OPTION_Z_LOG))
        {
          z_major = major_count;
        }
      else
        {
          z_major = 1;
        }
    }

  if (element->hasAttribute("z_tick"))
    {
      z_tick = static_cast<double>(element->getAttribute("z_tick"));
    }
  else
    {
      if (!(scale & GR_OPTION_Z_LOG))
        {
          if (z_major != 0)
            {
              z_tick = auto_tick(zmin, zmax) / z_major;
            }
          else
            {
              z_tick = 1;
            }
        }
      else
        {
          z_tick = 1;
        }
    }

  if (element->hasAttribute("z_org"))
    {
      z_org = static_cast<double>(element->getAttribute("z_org"));
    }
  else
    {
      if (!(scale & GR_OPTION_FLIP_Z))
        {
          z_org_low = zmin;
          z_org_high = zmax;
        }
      else
        {
          z_org_low = zmax;
          z_org_high = zmin;
        }
      if (z_org_pos == "low")
        {
          z_org = z_org_low;
        }
      else
        {
          z_org = z_org_high;
          z_major = -z_major;
        }
    }
}

static void polymarker(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processtd::sing function for polymarker
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
   * Processtd::sing Function for polyline
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
   * Processtd::sing funcions for text
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  gr_savestate();
  auto x = static_cast<double>(element->getAttribute("x"));
  auto y = static_cast<double>(element->getAttribute("y"));
  auto str = static_cast<std::string>(element->getAttribute("text"));
  auto available_width = static_cast<double>(element->getAttribute("width"));
  auto available_height = static_cast<double>(element->getAttribute("height"));
  double tbx[4], tby[4];
  bool text_fits = true;
  CoordinateSpace space = static_cast<CoordinateSpace>(static_cast<int>(element->getAttribute("space")));


  if (space == CoordinateSpace::WC)
    {
      gr_wctondc(&x, &y);
    }
  if (element->hasAttribute("width") && element->hasAttribute("height"))
    {
      gr_wctondc(&available_width, &available_height);
      gr_inqtext(x, y, &str[0], tbx, tby);
      auto minmax_x = std::minmax_element(std::begin(tbx), std::end(tbx));
      auto minmax_y = std::minmax_element(std::begin(tby), std::end(tby));
      double width = minmax_x.second - minmax_x.first;
      double height = minmax_y.second - minmax_y.first;
      if (width > available_width && height > available_height)
        {
          gr_setcharup(0.0, 1.0);
          gr_settextalign(2, 3);
          gr_inqtext(x, y, &str[0], tbx, tby);
          width = tbx[2] - tbx[0];
          height = tby[2] - tby[0];
          if (width < available_width && height < available_height)
            {
              gr_setcharup(0.0, 1.0);
              gr_settextalign(2, 3);
            }
          else if (height < available_width && width < available_height)
            {
              gr_setcharup(-1.0, 0.0);
              gr_settextalign(2, 3);
            }
          else
            {
              text_fits = false;
            }
        }
    }
  if (text_fits) gr_text(x, y, &str[0]);
  gr_restorestate();
}

static void fillArea(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processtd::sing function for fillArea
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
   * Processtd::sing function for cellArray
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
   * Processtd::sing function for axes
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  double x_tick, x_org;
  double y_tick, y_org;
  int x_major;
  int y_major;
  int tick_orientation = 1;
  double tick_size;
  std::string x_org_pos, y_org_pos;

  if (element->hasAttribute("x_org_pos"))
    {
      x_org_pos = static_cast<std::string>(element->getAttribute("x_org_pos"));
    }
  else
    {
      x_org_pos = "low";
    }
  if (element->hasAttribute("y_org_pos"))
    {
      y_org_pos = static_cast<std::string>(element->getAttribute("y_org_pos"));
    }
  else
    {
      y_org_pos = "low";
    }

  getAxesInformation(element, x_org_pos, y_org_pos, x_org, y_org, x_major, y_major, x_tick, y_tick);

  auto draw_axes_group = element->parentElement();
  auto subplot_element = getSubplotElement(element);

  if (element->hasAttribute("tick_orientation"))
    {
      tick_orientation = static_cast<int>(element->getAttribute("tick_orientation"));
    }

  getTickSize(element, tick_size);
  tick_size *= tick_orientation;

  gr_axes(x_tick, y_tick, x_org, y_org, x_major, y_major, tick_size);
}

static void grid(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processtd::sing function for grid
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  double x_tick, y_tick, x_org, y_org;
  int x_major, y_major;
  std::string x_org_pos, y_org_pos;

  if (element->hasAttribute("x_org_pos"))
    {
      x_org_pos = static_cast<std::string>(element->getAttribute("x_org_pos"));
    }
  else
    {
      x_org_pos = "low";
    }
  if (element->hasAttribute("y_org_pos"))
    {
      y_org_pos = static_cast<std::string>(element->getAttribute("y_org_pos"));
    }
  else
    {
      y_org_pos = "low";
    }

  getAxesInformation(element, x_org_pos, y_org_pos, x_org, y_org, x_major, y_major, x_tick, y_tick);

  gr_grid(x_tick, y_tick, x_org, y_org, x_major, y_major);
}

static void drawImage(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processtd::sing function for drawImage
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
   * Processtd::sing function for drawArc
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
   * Processtd::sing function for drawArc
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
   * Processtd::sing function for drawArc
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
   * Processtd::sing function for drawArc
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
   * Processtd::sing function for quiver
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
   * Processtd::sing function for contour
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
   * Processtd::sing function for contourf
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
   * Processtd::sing function for hexbin
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
   * Processtd::sing function for nonuniformcellarray
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
   * Processtd::sing function for surface
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  auto px = static_cast<std::string>(element->getAttribute("px"));
  auto py = static_cast<std::string>(element->getAttribute("py"));
  auto pz = static_cast<std::string>(element->getAttribute("pz"));
  int option = static_cast<int>(element->getAttribute("option"));
  int accelerate = static_cast<int>(element->getAttribute("accelerate"));

  std::vector<double> px_vec = GR::get<std::vector<double>>((*context)[px]);
  std::vector<double> py_vec = GR::get<std::vector<double>>((*context)[py]);
  std::vector<double> pz_vec = GR::get<std::vector<double>>((*context)[pz]);

  int nx = px_vec.size();
  int ny = py_vec.size();

  if (!accelerate)
    {

      double *px_p = &(px_vec[0]);
      double *py_p = &(py_vec[0]);
      double *pz_p = &(pz_vec[0]);

      gr_surface(nx, ny, px_p, py_p, pz_p, option);
    }
  else
    {
      std::vector<float> px_vec_f(px_vec.begin(), px_vec.end());
      std::vector<float> py_vec_f(py_vec.begin(), py_vec.end());
      std::vector<float> pz_vec_f(pz_vec.begin(), pz_vec.end());

      float *px_p = &(px_vec_f[0]);
      float *py_p = &(py_vec_f[0]);
      float *pz_p = &(pz_vec_f[0]);

      gr3_surface(nx, ny, px_p, py_p, pz_p, option);
    }
}

static void grid3d(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processtd::sing function for grid3d
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  double x_tick, x_org;
  double y_tick, y_org;
  double z_tick, z_org;
  int x_major;
  int y_major;
  int z_major;
  std::string x_org_pos, y_org_pos, z_org_pos;
  if (element->hasAttribute("x_org_pos"))
    {
      x_org_pos = static_cast<std::string>(element->getAttribute("x_org_pos"));
    }
  else
    {
      x_org_pos = "low";
    }
  if (element->hasAttribute("y_org_pos"))
    {
      y_org_pos = static_cast<std::string>(element->getAttribute("y_org_pos"));
    }
  else
    {
      y_org_pos = "low";
    }
  if (element->hasAttribute("z_org_pos"))
    {
      z_org_pos = static_cast<std::string>(element->getAttribute("z_org_pos"));
    }
  else
    {
      z_org_pos = "low";
    }
  getAxes3dInformation(element, x_org_pos, y_org_pos, z_org_pos, x_org, y_org, z_org, x_major, y_major, z_major, x_tick,
                       y_tick, z_tick);

  gr_grid3d(x_tick, y_tick, z_tick, x_org, y_org, z_org, x_major, y_major, z_major);
}

static void axes3d(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processtd::sing function for axes3d
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  double x_tick, x_org;
  double y_tick, y_org;
  double z_tick, z_org;
  int x_major;
  int y_major;
  int z_major;
  int tick_orientation = 1;
  double tick_size;
  std::string x_org_pos, y_org_pos, z_org_pos;

  if (element->hasAttribute("x_org_pos"))
    {
      x_org_pos = static_cast<std::string>(element->getAttribute("x_org_pos"));
    }
  else
    {
      x_org_pos = "low";
    }
  if (element->hasAttribute("y_org_pos"))
    {
      y_org_pos = static_cast<std::string>(element->getAttribute("y_org_pos"));
    }
  else
    {
      y_org_pos = "low";
    }
  if (element->hasAttribute("z_org_pos"))
    {
      z_org_pos = static_cast<std::string>(element->getAttribute("z_org_pos"));
    }
  else
    {
      z_org_pos = "low";
    }
  getAxes3dInformation(element, x_org_pos, y_org_pos, z_org_pos, x_org, y_org, z_org, x_major, y_major, z_major, x_tick,
                       y_tick, z_tick);

  auto subplot_element = getSubplotElement(element);

  if (element->hasAttribute("tick_orientation"))
    {
      tick_orientation = static_cast<int>(element->getAttribute("tick_orientation"));
    }

  getTickSize(element, tick_size);
  tick_size *= tick_orientation;

  gr_axes3d(x_tick, y_tick, z_tick, x_org, y_org, z_org, x_major, y_major, z_major, tick_size);
}

static void polyline3d(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processtd::sing function for polyline3d
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
   * Processtd::sing function for polymarker3d
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
   * Processtd::sing function for gr3_drawmesh
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
   * Processtd::sing function for trisurface
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
   * Processtd::sing function for tricontour
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

static void titles3d(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processtd::sing function for titles3d
   *
   * \param[in] element The GR::Element that contains the attributes and data keys
   * \param[in] context The GR::Context that contains the actual data
   */
  std::string x, y, z;
  x = static_cast<std::string>(element->getAttribute("x"));
  y = static_cast<std::string>(element->getAttribute("y"));
  z = static_cast<std::string>(element->getAttribute("z"));
  gr_titles3d(x.data(), y.data(), z.data());
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

static void layoutGrid(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  double xmin, xmax, ymin, ymax;
  xmin = (double)element->getAttribute("subplot_xmin");
  xmax = (double)element->getAttribute("subplot_xmax");
  ymin = (double)element->getAttribute("subplot_ymin");
  ymax = (double)element->getAttribute("subplot_ymax");

  gr_setviewport(xmin, xmax, ymin, ymax);
}

static void layoutGridElement(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  // todo is layoutgridelement actually needed? Can it be replaced by an ordinary group
  double xmin, xmax, ymin, ymax;
  xmin = (double)element->getAttribute("subplot_xmin");
  xmax = (double)element->getAttribute("subplot_xmax");
  ymin = (double)element->getAttribute("subplot_ymin");
  ymax = (double)element->getAttribute("subplot_ymax");

  //  gr_setviewport(xmin, xmax, ymin, ymax);
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

static void processTitle(const std::shared_ptr<GR::Element> &elem)
{
  double viewport[4], vp[4];

  auto subplot_element = getSubplotElement(elem);
  std::string name = (std::string)subplot_element->getAttribute("name");
  if (name != "polarhistogram")
    {
      viewport[0] = (double)subplot_element->getAttribute("viewport_xmin");
      viewport[1] = (double)subplot_element->getAttribute("viewport_xmax");
      viewport[2] = (double)subplot_element->getAttribute("viewport_ymin");
      viewport[3] = (double)subplot_element->getAttribute("viewport_ymax");
      vp[0] = (double)subplot_element->getAttribute("vp_xmin");
      vp[1] = (double)subplot_element->getAttribute("vp_xmax");
      vp[2] = (double)subplot_element->getAttribute("vp_ymin");
      vp[3] = (double)subplot_element->getAttribute("vp_ymax");

      double x = 0.5 * (viewport[0] + viewport[1]);
      double y = vp[3];
      std::string title = (std::string)elem->getAttribute("title");

      if (auto render = std::dynamic_pointer_cast<GR::Render>(elem->ownerDocument()))
        {
          auto title_elem = render->createText(x, y, title);
          render->setTextAlign(title_elem, GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_TOP);
          elem->appendChild(title_elem);
        }
    }
}

static void processXlabel(const std::shared_ptr<GR::Element> &elem)
{
  double viewport[4], vp[4], charheight;

  auto subplot_element = getSubplotElement(elem);

  gr_inqcharheight(&charheight);
  viewport[0] = (double)subplot_element->getAttribute("viewport_xmin");
  viewport[1] = (double)subplot_element->getAttribute("viewport_xmax");
  viewport[2] = (double)subplot_element->getAttribute("viewport_ymin");
  viewport[3] = (double)subplot_element->getAttribute("viewport_ymax");
  vp[0] = (double)subplot_element->getAttribute("vp_xmin");
  vp[1] = (double)subplot_element->getAttribute("vp_xmax");
  vp[2] = (double)subplot_element->getAttribute("vp_ymin");
  vp[3] = (double)subplot_element->getAttribute("vp_ymax");

  double x = 0.5 * (viewport[0] + viewport[1]);
  double y = vp[2] + 0.5 * charheight;
  std::string x_label = (std::string)elem->getAttribute("xlabel");

  if (auto render = std::dynamic_pointer_cast<GR::Render>(elem->ownerDocument()))
    {
      auto text = render->createText(x, y, x_label);
      render->setTextAlign(text, GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_BOTTOM);
      elem->appendChild(text);
    }
}

/*!
 * Draw an xticklabel at the specified position while trying to stay in the available space.
 * Every space character (' ') is seen as a possible position to break the label into the next line.
 * The label will not break into the next line when enough space is available.
 * If a label or a part of it does not fit in the available space but doesnt have a space character to break it up
 * it will get drawn anyway.
 *
 * \param[in] x1 The X coordinate of starting position of the label.
 * \param[in] x2 The Y coordinate of starting position of the label.
 * \param[in] label The label to be drawn.
 * \param[in] available_width The available space in X direction around the starting position.
 */
void draw_xticklabel(double x1, double x2, const char *label, double available_width,
                     const std::shared_ptr<GR::Element> &element, std::shared_ptr<GR::Render> global_render)
{
  char new_label[256];
  int breakpoint_positions[128];
  int cur_num_breakpoints = 0;
  int i = 0;
  int cur_start = 0;
  double tbx[4], tby[4];
  double width;
  double charheight;

  gr_inqcharheight(&charheight);

  for (i = 0; i == 0 || label[i - 1] != '\0'; ++i)
    {
      if (label[i] == ' ' || label[i] == '\0')
        {
          /* calculate width of the next part of the label to be drawn */
          new_label[i] = '\0';
          gr_inqtext(x1, x2, new_label + cur_start, tbx, tby);
          gr_wctondc(&tbx[0], &tby[0]);
          gr_wctondc(&tbx[1], &tby[1]);
          width = tbx[1] - tbx[0];
          new_label[i] = ' ';

          /* add possible breakpoint */
          breakpoint_positions[cur_num_breakpoints++] = i;

          if (width > available_width)
            {
              /* part is too big but doesnt have a breakpoint in it */
              if (cur_num_breakpoints == 1)
                {
                  new_label[i] = '\0';
                  element->append(global_render->createText(x1, x2, new_label + cur_start));

                  cur_start = i + 1;
                  cur_num_breakpoints = 0;
                }
              /* part is too big and has breakpoints in it */
              else
                {
                  /* break label at last breakpoint that still allowed the text to fit */
                  new_label[breakpoint_positions[cur_num_breakpoints - 2]] = '\0';
                  element->append(global_render->createText(x1, x2, new_label + cur_start));

                  cur_start = breakpoint_positions[cur_num_breakpoints - 2] + 1;
                  breakpoint_positions[0] = breakpoint_positions[cur_num_breakpoints - 1];
                  cur_num_breakpoints = 1;
                }
              x2 -= charheight * 1.5;
            }
        }
      else
        {
          new_label[i] = label[i];
        }
    }

  /* 0-terminate the new label */
  new_label[i] = '\0';

  /* draw the rest */
  element->append(global_render->createText(x1, x2, std::string(new_label + cur_start)));
}

static void processXTickLabels(const std::shared_ptr<GR::Element> &elem)
{
  double viewport[4], vp[4], window[4], charheight;
  std::vector<std::string> xticklabels;

  auto subplot_element = getSubplotElement(elem);

  gr_inqcharheight(&charheight);
  viewport[0] = (double)subplot_element->getAttribute("viewport_xmin");
  viewport[1] = (double)subplot_element->getAttribute("viewport_xmax");
  viewport[2] = (double)subplot_element->getAttribute("viewport_ymin");
  viewport[3] = (double)subplot_element->getAttribute("viewport_ymax");
  vp[0] = (double)subplot_element->getAttribute("vp_xmin");
  vp[1] = (double)subplot_element->getAttribute("vp_xmax");
  vp[2] = (double)subplot_element->getAttribute("vp_ymin");
  vp[3] = (double)subplot_element->getAttribute("vp_ymax");
  window[0] = (double)subplot_element->getAttribute("window_xmin");
  window[1] = (double)subplot_element->getAttribute("window_xmax");
  window[2] = (double)subplot_element->getAttribute("window_ymin");
  window[3] = (double)subplot_element->getAttribute("window_ymax");

  if (auto render = std::dynamic_pointer_cast<GR::Render>(elem->ownerDocument()))
    {
      std::shared_ptr<GR::Context> context = render->getContext();
      std::string key = static_cast<std::string>(elem->getAttribute("xticklabels"));
      xticklabels = GR::get<std::vector<std::string>>((*context)[key]);

      double x1, x2;
      double x_left = 0, x_right = 1, null;
      double available_width;
      const double *window;
      auto xtick_group = render->createGroup("barplot_xtick");

      elem->append(xtick_group);

      /* calculate width available for xticknotations */
      gr_wctondc(&x_left, &null);
      gr_wctondc(&x_right, &null);
      available_width = x_right - x_left;
      render->setTextAlign(xtick_group, GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_TOP);
      for (int i = 1; i <= xticklabels.size(); i++)
        {
          x1 = i;
          gr_wctondc(&x1, &x2);
          x2 = viewport[2] - 0.5 * charheight;
          draw_xticklabel(x1, x2, xticklabels[i - 1].c_str(), available_width, xtick_group, render);
        }
    }
}

static void processYlabel(const std::shared_ptr<GR::Element> &elem)
{
  double viewport[4], vp[4], charheight;

  auto subplot_element = getSubplotElement(elem);

  gr_inqcharheight(&charheight);
  viewport[0] = (double)subplot_element->getAttribute("viewport_xmin");
  viewport[1] = (double)subplot_element->getAttribute("viewport_xmax");
  viewport[2] = (double)subplot_element->getAttribute("viewport_ymin");
  viewport[3] = (double)subplot_element->getAttribute("viewport_ymax");
  vp[0] = (double)subplot_element->getAttribute("vp_xmin");
  vp[1] = (double)subplot_element->getAttribute("vp_xmax");
  vp[2] = (double)subplot_element->getAttribute("vp_ymin");
  vp[3] = (double)subplot_element->getAttribute("vp_ymax");

  double x = vp[0] + 0.5 * charheight;
  double y = 0.5 * (viewport[2] + viewport[3]);
  std::string y_label = (std::string)elem->getAttribute("ylabel");

  if (auto render = std::dynamic_pointer_cast<GR::Render>(elem->ownerDocument()))
    {
      auto text = render->createText(x, y, y_label);
      render->setTextAlign(text, GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_TOP);
      render->setCharUp(text, -1, 0);
      elem->appendChild(text);
    }
}

static void processImshowInformation(const std::shared_ptr<GR::Element> &elem)
{
  double x_min, x_max, y_min, y_max;
  double vp[4];
  int scale;
  int cols = static_cast<int>(elem->getAttribute("cols"));
  int rows = static_cast<int>(elem->getAttribute("rows"));
  std::string image_data_key = static_cast<std::string>(elem->getAttribute("image_data_key"));
  std::shared_ptr<GR::Element> ancestor = elem->parentElement();
  bool vp_found = false;
  int grplot;

  // Get vp from ancestor GR::element, usually the q"plot-group"
  while (ancestor->localName() != "root")
    {
      if (ancestor->hasAttribute("vp"))
        {
          vp[0] = static_cast<double>(ancestor->getAttribute("vp_xmin"));
          vp[1] = static_cast<double>(ancestor->getAttribute("vp_xmax"));
          vp[2] = static_cast<double>(ancestor->getAttribute("vp_ymin"));
          vp[3] = static_cast<double>(ancestor->getAttribute("vp_ymax"));
          vp_found = true;
          grplot = static_cast<int>(ancestor->getAttribute("grplot"));
          break;
        }
      else
        {
          ancestor = ancestor->parentElement();
        }
    }
  if (!vp_found)
    {
      throw NotFoundError("No vp was found within ancestors");
    }

  gr_inqscale(&scale);

  if (cols * (vp[3] - vp[2]) < rows * (vp[1] - vp[0]))
    {
      double w = (double)cols / (double)rows * (vp[3] - vp[2]);
      x_min = grm_max(0.5 * (vp[0] + vp[1] - w), vp[0]);
      x_max = grm_min(0.5 * (vp[0] + vp[1] + w), vp[1]);
      y_min = vp[2];
      y_max = vp[3];
    }
  else
    {
      double h = (double)rows / (double)cols * (vp[1] - vp[0]);
      x_min = vp[0];
      x_max = vp[1];
      y_min = grm_max(0.5 * (vp[3] + vp[2] - h), vp[2]);
      y_max = grm_min(0.5 * (vp[3] + vp[2] + h), vp[3]);
    }

  if (scale & GR_OPTION_FLIP_X)
    {
      double tmp = x_max;
      x_max = x_min;
      x_min = tmp;
    }
  if (scale & GR_OPTION_FLIP_Y)
    {
      double tmp = y_max;
      y_max = y_min;
      y_min = tmp;
    }
  if (grplot)
    {
      double tmp = y_min;
      y_min = y_max;
      y_max = tmp;
    }

  auto temp = global_render->createCellArray(x_min, x_max, y_min, y_max, cols, rows, 1, 1, cols, rows, image_data_key,
                                             std::nullopt);

  elem->append(temp);
}

static void processGROptionFlipX(const std::shared_ptr<GR::Element> &elem)
{
  int options;
  int flip_x = static_cast<int>(elem->getAttribute("gr-option-flip-x"));
  gr_inqscale(&options);

  if (flip_x)
    {
      gr_setscale(options | GR_OPTION_FLIP_X);
    }
  else
    {
      gr_setscale(options & ~GR_OPTION_FLIP_X);
    }
}

static void processGROptionFlipY(const std::shared_ptr<GR::Element> &elem)
{
  int options;
  int flip_y = static_cast<int>(elem->getAttribute("gr-option-flip-y"));
  gr_inqscale(&options);

  if (flip_y)
    {
      gr_setscale(options | GR_OPTION_FLIP_Y);
    }
  else
    {
      gr_setscale(options & ~GR_OPTION_FLIP_Y);
    }
}

static void processWindowAndViewportFromParent(const std::shared_ptr<GR::Element> &elem)
{
  std::string kind = static_cast<std::string>(elem->getAttribute("kind"));
  double viewport[4];
  double x_min, x_max, y_min, y_max, c_min, c_max;

  if (str_equals_any(kind.c_str(), 2, "hist", "line"))
    {
      std::string orientation = static_cast<std::string>(elem->getAttribute("orientation"));
      auto marginalheatmap_group = elem->parentElement();
      viewport[0] = static_cast<double>(marginalheatmap_group->getAttribute("viewport_xmin"));
      viewport[1] = static_cast<double>(marginalheatmap_group->getAttribute("viewport_xmax"));
      viewport[2] = static_cast<double>(marginalheatmap_group->getAttribute("viewport_ymin"));
      viewport[3] = static_cast<double>(marginalheatmap_group->getAttribute("viewport_ymax"));
      x_min = static_cast<double>(marginalheatmap_group->getAttribute("lim_xmin"));
      x_max = static_cast<double>(marginalheatmap_group->getAttribute("lim_xmax"));
      y_min = static_cast<double>(marginalheatmap_group->getAttribute("lim_ymin"));
      y_max = static_cast<double>(marginalheatmap_group->getAttribute("lim_ymax"));
      if (marginalheatmap_group->hasAttribute("lim_cmin"))
        {
          c_min = static_cast<double>(marginalheatmap_group->getAttribute("lim_cmin"));
        }
      else
        {
          c_min = static_cast<double>(marginalheatmap_group->getAttribute("lim_zmin"));
        }
      if (marginalheatmap_group->hasAttribute("lim_cmax"))
        {
          c_max = static_cast<double>(marginalheatmap_group->getAttribute("lim_cmax"));
        }
      else
        {
          c_max = static_cast<double>(marginalheatmap_group->getAttribute("lim_zmax"));
        }

      if (orientation == "vertical")
        {
          gr_setwindow(0.0, c_max / 10, y_min, y_max);
          gr_setviewport(viewport[1] + 0.02 + 0.0, viewport[1] + 0.12 + 0.0, viewport[2], viewport[3]);
        }
      else if (orientation == "horizontal")
        {
          gr_setwindow(x_min, x_max, 0.0, c_max / 10);
          gr_setviewport(viewport[0], viewport[1], viewport[3] + 0.02, grm_min(viewport[3] + 0.12, 1));
        }
    }
}

static void processColorbarPosition(const std::shared_ptr<GR::Element> &elem)
{
  double viewport[4];

  auto subplot_element = getSubplotElement(elem);

  double width = static_cast<double>(elem->getAttribute("width"));
  double offset = static_cast<double>(elem->getAttribute("offset"));

  if (!subplot_element->hasAttribute("viewport"))
    {
      throw NotFoundError("Misstd::sing viewport\n");
    }

  viewport[0] = static_cast<double>(subplot_element->getAttribute("viewport_xmin"));
  viewport[1] = static_cast<double>(subplot_element->getAttribute("viewport_xmax"));
  viewport[2] = static_cast<double>(subplot_element->getAttribute("viewport_ymin"));
  viewport[3] = static_cast<double>(subplot_element->getAttribute("viewport_ymax"));

  gr_setviewport(viewport[1] + offset, viewport[1] + offset + width, viewport[2], viewport[3]);
}

static void processRelativeCharHeight(const std::shared_ptr<GR::Element> &elem)
{
  double viewport[4];
  auto subplot_element = getSubplotElement(elem);
  double charheight, diagFactor, maxCharHeight;

  if (!subplot_element->hasAttribute("viewport"))
    {
      throw NotFoundError("Viewport not found\n");
    }
  viewport[0] = static_cast<double>(subplot_element->getAttribute("viewport_xmin"));
  viewport[1] = static_cast<double>(subplot_element->getAttribute("viewport_xmax"));
  viewport[2] = static_cast<double>(subplot_element->getAttribute("viewport_ymin"));
  viewport[3] = static_cast<double>(subplot_element->getAttribute("viewport_ymax"));
  diagFactor = static_cast<double>(elem->getAttribute("diag_factor"));
  maxCharHeight = static_cast<double>(elem->getAttribute("max_charheight"));


  double diag = std::sqrt((viewport[1] - viewport[0]) * (viewport[1] - viewport[0]) +
                          (viewport[3] - viewport[2]) * (viewport[3] - viewport[2]));

  charheight = std::max(diag * diagFactor, maxCharHeight);
  gr_setcharheight(charheight);
}

void GR::Render::processLimits(const std::shared_ptr<GR::Element> &elem)
{
  /*!
   * Procestd::sing function for gr_window
   *
   * \param[in] element The GR::Element that contains the attributes
   */
  int adjust_xlim, adjust_ylim;
  std::string kind = static_cast<std::string>(elem->getAttribute("kind"));

  int scale = static_cast<int>(elem->getAttribute("scale"));

  double xmin = static_cast<double>(elem->getAttribute("lim_xmin"));
  double xmax = static_cast<double>(elem->getAttribute("lim_xmax"));
  double ymin = static_cast<double>(elem->getAttribute("lim_ymin"));
  double ymax = static_cast<double>(elem->getAttribute("lim_ymax"));

  if (elem->hasAttribute("reset_ranges") && static_cast<int>(elem->getAttribute("reset_ranges")))
    {
      if (elem->hasAttribute("_original_xmin") && elem->hasAttribute("_original_xmax") &&
          elem->hasAttribute("_original_ymin") && elem->hasAttribute("_original_ymax") &&
          elem->hasAttribute("_original_adjust_xlim") && elem->hasAttribute("_original_adjust_ylim"))
        {
          xmin = static_cast<double>(elem->getAttribute("_original_xmin"));
          xmax = static_cast<double>(elem->getAttribute("_original_xmax"));
          ymin = static_cast<double>(elem->getAttribute("_original_ymin"));
          ymax = static_cast<double>(elem->getAttribute("_original_ymax"));
          adjust_xlim = static_cast<int>(elem->getAttribute("_original_adjust_xlim"));
          adjust_ylim = static_cast<int>(elem->getAttribute("_original_adjust_ylim"));
          elem->setAttribute("adjust_xlim", adjust_xlim);
          elem->setAttribute("adjust_ylim", adjust_ylim);
          elem->removeAttribute("_original_xmin");
          elem->removeAttribute("_original_xmax");
          elem->removeAttribute("_original_ymin");
          elem->removeAttribute("_original_ymax");
          elem->removeAttribute("_original_adjust_xlim");
          elem->removeAttribute("_original_adjust_ylim");
        }
      elem->removeAttribute("reset_ranges");
    }

  if (elem->hasAttribute("panzoom") && static_cast<int>(elem->getAttribute("panzoom")))
    {
      if (!elem->hasAttribute("_original_xlim"))
        {
          elem->setAttribute("_original_xmin", xmin);
          elem->setAttribute("_original_xmax", xmax);
          elem->setAttribute("_original_xlim", true);
          adjust_xlim = static_cast<int>(elem->getAttribute("adjust_xlim"));
          elem->setAttribute("_original_adjust_xlim", adjust_xlim);
          elem->setAttribute("adjust_xlim", 0);
        }
      if (!elem->hasAttribute("_original_ylim"))
        {
          elem->setAttribute("_original_ymin", ymin);
          elem->setAttribute("_original_ymax", ymax);
          elem->setAttribute("_original_ylim", true);
          adjust_ylim = static_cast<int>(elem->getAttribute("adjust_ylim"));
          elem->setAttribute("_original_adjust_ylim", adjust_ylim);
          elem->setAttribute("adjust_ylim", 0);
        }
      auto panzoom_element = elem->getElementsByTagName("panzoom")[0];
      double x = static_cast<double>(panzoom_element->getAttribute("x"));
      double y = static_cast<double>(panzoom_element->getAttribute("y"));
      double xzoom = static_cast<double>(panzoom_element->getAttribute("xzoom"));
      double yzoom = static_cast<double>(panzoom_element->getAttribute("yzoom"));

      /* Ensure the correct window is set in GR */
      if (elem->hasAttribute("window") && static_cast<int>(elem->getAttribute("window")))
        {
          double stored_window_xmin = static_cast<double>(elem->getAttribute("window_xmin"));
          double stored_window_xmax = static_cast<double>(elem->getAttribute("window_xmax"));
          double stored_window_ymin = static_cast<double>(elem->getAttribute("window_ymin"));
          double stored_window_ymax = static_cast<double>(elem->getAttribute("window_ymax"));

          gr_setwindow(stored_window_xmin, stored_window_xmax, stored_window_ymin, stored_window_ymax);
        }

      gr_panzoom(x, y, xzoom, yzoom, &xmin, &xmax, &ymin, &ymax);

      elem->setAttribute("panzoom", false);
      elem->removeChild(panzoom_element);
    }

  if (!(scale & GR_OPTION_X_LOG))
    {
      adjust_xlim = static_cast<int>(elem->getAttribute("adjust_xlim"));
      if (adjust_xlim)
        {
          logger((stderr, "_xlim before \"gr_adjustlimits\": (%lf, %lf)\n", xmin, xmax));
          gr_adjustlimits(&xmin, &xmax);
          logger((stderr, "_xlim after \"gr_adjustlimits\": (%lf, %lf)\n", xmin, xmax));
        }
    }

  if (!(scale & GR_OPTION_Y_LOG))
    {
      adjust_ylim = static_cast<int>(elem->getAttribute("adjust_ylim"));
      if (adjust_ylim)
        {
          logger((stderr, "_ylim before \"gr_adjustlimits\": (%lf, %lf)\n", ymin, ymax));
          gr_adjustlimits(&ymin, &ymax);
          logger((stderr, "_ylim after \"gr_adjustlimits\": (%lf, %lf)\n", ymin, ymax));
        }
    }

  if (str_equals_any(kind.c_str(), 7, "wireframe", "surface", "plot3", "scatter3", "trisurf", "volume", "isosurface"))
    {
      double zmin = static_cast<double>(elem->getAttribute("lim_zmin"));
      double zmax = static_cast<double>(elem->getAttribute("lim_zmax"));
      if (!(scale & GR_OPTION_Z_LOG))
        {
          int adjust_zlim = static_cast<int>(elem->hasAttribute("adjust_zlim"));
          if (adjust_zlim)
            {
              logger((stderr, "_zlim before \"gr_adjustlimits\": (%lf, %lf)\n", zmin, zmax));
              gr_adjustlimits(&zmin, &zmax);
              logger((stderr, "_zlim after \"gr_adjustlimits\": (%lf, %lf)\n", zmin, zmax));
            }
        }
      logger((stderr, "Storing window3d (%lf, %lf, %lf, %lf, %lf, %lf)\n", xmin, xmax, ymin, ymax, zmin, zmax));
      gr_setwindow3d(xmin, xmax, ymin, ymax, zmin, zmax);
      global_render->setWindow3d(elem, xmin, xmax, ymin, ymax, zmin, zmax);
    }
  else
    {
      logger((stderr, "Storing window (%lf, %lf, %lf, %lf)\n", xmin, xmax, ymin, ymax));
      if (!str_equals_any(kind.c_str(), 2, "polar", "polar_histogram"))
        {
          gr_setwindow(xmin, xmax, ymin, ymax);
          global_render->setWindow(elem, xmin, xmax, ymin, ymax);
        }
      else
        {
          xmin = -1.0;
          xmax = 1.0;
          ymin = -1.0;
          ymax = 1.0;
          gr_setwindow(xmin, xmax, ymin, ymax);
          global_render->setWindow(elem, xmin, xmax, ymin, ymax);
        }
    }
}

static void processWindow(const std::shared_ptr<GR::Element> &elem)
{
  double xmin = static_cast<double>(elem->getAttribute("window_xmin"));
  double xmax = static_cast<double>(elem->getAttribute("window_xmax"));
  double ymin = static_cast<double>(elem->getAttribute("window_ymin"));
  double ymax = static_cast<double>(elem->getAttribute("window_ymax"));

  gr_setwindow(xmin, xmax, ymin, ymax);
}

static void processProjectionType(const std::shared_ptr<GR::Element> &elem)
{
  int type = (int)elem->getAttribute("projectiontype");
  gr_setprojectiontype(type);
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

static void processSpace(const std::shared_ptr<GR::Element> &elem)
{
  double zmin, zmax;
  int rotation, tilt;
  zmin = (double)elem->getAttribute("space_zmin");
  zmax = (double)elem->getAttribute("space_zmax");
  rotation = (int)elem->getAttribute("space_rotation");
  tilt = (int)elem->getAttribute("space_tilt");

  gr_setspace(zmin, zmax, rotation, tilt);
}

void GR::Render::processViewport(const std::shared_ptr<GR::Element> &elem)
{
  /*!
   * Procestd::sing function for gr_viewport
   *
   * \param[in] element The GR::Element that contains the attributes
   */
  double viewport[4];
  double diag;
  double charheight;
  std::string kind;

  viewport[0] = (double)elem->getAttribute("viewport_xmin");
  viewport[1] = (double)elem->getAttribute("viewport_xmax");
  viewport[2] = (double)elem->getAttribute("viewport_ymin");
  viewport[3] = (double)elem->getAttribute("viewport_ymax");
  kind = (std::string)elem->getAttribute("kind");

  diag = std::sqrt((viewport[1] - viewport[0]) * (viewport[1] - viewport[0]) +
                   (viewport[3] - viewport[2]) * (viewport[3] - viewport[2]));

  if (str_equals_any(kind.c_str(), 6, "wireframe", "surface", "plot3", "scatter3", "trisurf", "volume"))
    {
      charheight = grm_max(0.024 * diag, 0.012);
    }
  else
    {
      charheight = grm_max(0.018 * diag, 0.012);
    }
  gr_setcharheight(charheight);
}

static void legend_size(const std::shared_ptr<GR::Element> &elem, double *w, double *h)
{
  double tbx[4], tby[4];
  int labelsExist = 1;
  unsigned int num_labels;
  std::vector<std::string> labels;
  *w = 0;
  *h = 0;

  if (auto render = std::dynamic_pointer_cast<GR::Render>(elem->ownerDocument()))
    {
      auto context = render->getContext();
      std::string key = static_cast<std::string>(elem->getAttribute("labels"));
      labels = GR::get<std::vector<std::string>>((*context)[key]);
    }
  if (labelsExist)
    {
      for (auto current_label : labels)
        {
          gr_inqtext(0, 0, current_label.data(), tbx, tby);
          *w = grm_max(*w, tbx[2] - tbx[0]);
          *h += grm_max(tby[2] - tby[0], 0.03);
        }
    }
}

static void get_figure_size(int *pixel_width, int *pixel_height, double *metric_width, double *metric_height)
{
  double display_metric_width, display_metric_height;
  int display_pixel_width, display_pixel_height;
  double dpm[2], dpi[2];
  int tmp_size_i[2], pixel_size[2];
  double tmp_size_d[2], metric_size[2];
  int i;
  std::string size_unit, size_type;
  std::string vars[2] = {"x", "y"};

  std::shared_ptr<GR::Element> root = global_root;


#ifdef __EMSCRIPTEN__
  display_metric_width = 0.16384;
  display_metric_height = 0.12288;
  display_pixel_width = 640;
  display_pixel_height = 480;
#else
  gr_inqdspsize(&display_metric_width, &display_metric_height, &display_pixel_width, &display_pixel_height);
#endif
  dpm[0] = display_pixel_width / display_metric_width;
  dpm[1] = display_pixel_height / display_metric_height;
  dpi[0] = dpm[0] * 0.0254;
  dpi[1] = dpm[1] * 0.0254;

  /* TODO: Overwork this calculation */
  if (root->hasAttribute("figsize"))
    {
      tmp_size_d[0] = (double)root->getAttribute("figsize_x");
      tmp_size_d[1] = (double)root->getAttribute("figsize_y");
      for (i = 0; i < 2; ++i)
        {
          pixel_size[i] = (int)grm_round(tmp_size_d[i] * dpi[i]);
          metric_size[i] = tmp_size_d[i] / 0.0254;
        }
    }
  else if (root->hasAttribute("size"))
    {
      for (i = 0; i < 2; ++i)
        {
          size_unit = (std::string)root->getAttribute("size_unit_" + vars[i]);
          size_type = (std::string)root->getAttribute("size_type_" + vars[i]);
          if (size_unit.empty()) size_unit = "m";

          auto meters_per_unit_iter = symbol_to_meters_per_unit.find(size_unit);
          if (meters_per_unit_iter != symbol_to_meters_per_unit.end())
            {
              double meters_per_unit = meters_per_unit_iter->second;
              double pixels_per_unit = meters_per_unit * dpm[i];

              if (size_type == "double")
                {
                  tmp_size_d[i] = tmp_size_d[i] * pixels_per_unit;
                }
              else if (size_type == "int")
                {
                  tmp_size_d[i] = tmp_size_i[i] * pixels_per_unit;
                }
              else
                {
                  tmp_size_d[0] = PLOT_DEFAULT_WIDTH;
                  tmp_size_d[1] = PLOT_DEFAULT_HEIGHT;
                }
              pixel_size[i] = (int)grm_round(tmp_size_d[i]);
              metric_size[i] = tmp_size_d[i] / dpm[i];
            }
          else
            {
              pixel_size[0] = (int)grm_round(PLOT_DEFAULT_WIDTH);
              pixel_size[1] = (int)grm_round(PLOT_DEFAULT_HEIGHT);
              metric_size[0] = PLOT_DEFAULT_WIDTH / dpm[0];
              metric_size[1] = PLOT_DEFAULT_HEIGHT / dpm[1];
            }
        }
    }
  else
    {
      pixel_size[0] = (int)grm_round(PLOT_DEFAULT_WIDTH);
      pixel_size[1] = (int)grm_round(PLOT_DEFAULT_HEIGHT);
      metric_size[0] = PLOT_DEFAULT_WIDTH / dpm[0];
      metric_size[1] = PLOT_DEFAULT_HEIGHT / dpm[1];
    }

  if (pixel_width != NULL)
    {
      *pixel_width = pixel_size[0];
    }
  if (pixel_height != NULL)
    {
      *pixel_height = pixel_size[1];
    }
  if (metric_width != NULL)
    {
      *metric_width = metric_size[0];
    }
  if (metric_height != NULL)
    {
      *metric_height = metric_size[1];
    }
}

static void processSubplot(const std::shared_ptr<GR::Element> &elem)
{
  /*!
   * Procestd::sing function for the subplot
   *
   * \param[in] element The GR::Element that contains the attributes
   */
  int y_label_margin, x_label_margin, title_margin;
  std::string kind;
  double subplot[4];
  int keep_aspect_ratio;
  double metric_width, metric_height;
  int pixel_width, pixel_height;
  double aspect_ratio_ws;
  double vp[4];
  double vp0, vp1, vp2, vp3;
  double left_margin, right_margin, bottom_margin, top_margin;
  double viewport[4] = {0.0, 0.0, 0.0, 0.0};
  int background_color_index;

  subplot[0] = (double)elem->getAttribute("subplot_xmin");
  subplot[1] = (double)elem->getAttribute("subplot_xmax");
  subplot[2] = (double)elem->getAttribute("subplot_ymin");
  subplot[3] = (double)elem->getAttribute("subplot_ymax");
  kind = (std::string)elem->getAttribute("kind");
  y_label_margin = (int)elem->getAttribute("ylabel_margin");
  x_label_margin = (int)elem->getAttribute("xlabel_margin");
  title_margin = (int)elem->getAttribute("title_margin");
  keep_aspect_ratio = static_cast<int>(elem->getAttribute("keep_aspect_ratio"));

  get_figure_size(&pixel_width, &pixel_height, nullptr, nullptr);

  aspect_ratio_ws = (double)pixel_width / pixel_height;
  memcpy(vp, subplot, sizeof(vp));
  if (aspect_ratio_ws > 1)
    {
      vp[2] /= aspect_ratio_ws;
      vp[3] /= aspect_ratio_ws;
      if (keep_aspect_ratio)
        {
          double border = 0.5 * (vp[1] - vp[0]) * (1.0 - 1.0 / aspect_ratio_ws);
          vp[0] += border;
          vp[1] -= border;
        }
    }
  else
    {
      vp[0] *= aspect_ratio_ws;
      vp[1] *= aspect_ratio_ws;
      if (keep_aspect_ratio)
        {
          double border = 0.5 * (vp[3] - vp[2]) * (1.0 - aspect_ratio_ws);
          vp[2] += border;
          vp[3] -= border;
        }
    }

  if (str_equals_any(kind.c_str(), 6, "wireframe", "surface", "plot3", "scatter3", "trisurf", "volume"))
    {
      double extent;

      extent = grm_min(vp[1] - vp[0], vp[3] - vp[2]);
      vp0 = 0.5 * (vp[0] + vp[1] - extent);
      vp1 = 0.5 * (vp[0] + vp[1] + extent);
      vp2 = 0.5 * (vp[2] + vp[3] - extent);
      vp3 = 0.5 * (vp[2] + vp[3] + extent);
    }
  else
    {
      vp0 = vp[0];
      vp1 = vp[1];
      vp2 = vp[2];
      vp3 = vp[3];
    }

  left_margin = y_label_margin ? 0.05 : 0;
  if (str_equals_any(kind.c_str(), 13, "contour", "contourf", "hexbin", "heatmap", "nonuniformheatmap", "surface",
                     "tricont", "trisurf", "volume", "marginalheatmap", "quiver", "polar_heatmap",
                     "nonuniformpolar_heatmap"))
    {
      right_margin = (vp1 - vp0) * 0.1;
    }
  else
    {
      right_margin = 0;
    }
  bottom_margin = x_label_margin ? 0.05 : 0;

  if (kind == "marginalheatmap")
    {
      top_margin = title_margin ? 0.075 + 0.5 * (vp[1] - vp[0]) * (1.0 - 1.0 / aspect_ratio_ws)
                                : 0.5 * (vp[1] - vp[0]) * (1.0 - 1.0 / aspect_ratio_ws);
      if (keep_aspect_ratio) right_margin += title_margin ? 0.075 : 0;
    }
  else
    {
      top_margin = title_margin ? 0.075 : 0;
      if (keep_aspect_ratio) right_margin -= 0.5 * (vp[1] - vp[0]) * (1.0 - 1.0 / aspect_ratio_ws) - top_margin;
    }
  if (kind == "imshow")
    {
      unsigned int i;
      unsigned int *shape;
      double w, h, x_min, x_max, y_min, y_max, *x, *y;

      int cols = static_cast<int>(elem->getAttribute("cols"));
      int rows = static_cast<int>(elem->getAttribute("rows"));

      h = (double)rows / (double)cols * (vp[1] - vp[0]);
      w = (double)cols / (double)rows * (vp[3] - vp[2]);

      x_min = grm_max(0.5 * (vp[0] + vp[1] - w), vp[0]);
      x_max = grm_min(0.5 * (vp[0] + vp[1] + w), vp[1]);
      y_min = grm_max(0.5 * (vp[3] + vp[2] - h), vp[2]);
      y_max = grm_min(0.5 * (vp[3] + vp[2] + h), vp[3]);

      left_margin = (x_min == vp[0]) ? -0.075 : (x_min - vp[0]) / (vp[1] - vp[0]) - 0.075;
      right_margin = (x_max == vp[1]) ? -0.05 : 0.95 - (x_max - vp[0]) / (vp[1] - vp[0]);
      bottom_margin = (y_min == vp[2]) ? -0.075 : (y_min - vp[2]) / (vp[3] - vp[2]) - 0.075;
      top_margin = (y_max == vp[3]) ? -0.025 : 0.975 - (y_max - vp[2]) / (vp[3] - vp[2]);
    }

  viewport[0] = vp0 + (0.075 + left_margin) * (vp1 - vp0);
  viewport[1] = vp0 + (0.95 - right_margin) * (vp1 - vp0);
  viewport[2] = vp2 + (0.075 + bottom_margin) * (vp3 - vp2);
  viewport[3] = vp2 + (0.975 - top_margin) * (vp3 - vp2);

  if (str_equals_any(kind.c_str(), 4, "line", "stairs", "scatter", "stem"))
    {
      double w, h;
      int location;
      GR::Value locationValue = elem->getAttribute("location");

      if (!locationValue.isUndefined())
        {
          location = (int)locationValue;
          if (location == 11 || location == 12 || location == 13)
            {
              legend_size(elem, &w, &h);
              viewport[1] -= w + 0.1;
            }
        }
    }

  if (elem->hasAttribute("backgroundcolor"))
    {
      background_color_index = static_cast<int>(elem->getAttribute("backgroundcolor"));
      gr_savestate();
      gr_selntran(0);
      gr_setfillintstyle(GKS_K_INTSTYLE_SOLID);
      gr_setfillcolorind(background_color_index);
      if (aspect_ratio_ws > 1)
        {
          gr_fillrect(subplot[0], subplot[1], subplot[2] / aspect_ratio_ws, subplot[3] / aspect_ratio_ws);
        }
      else
        {
          gr_fillrect(subplot[0] * aspect_ratio_ws, subplot[1] * aspect_ratio_ws, subplot[2], subplot[3]);
        }
      gr_selntran(1);
      gr_restorestate();
    }

  if (str_equals_any(kind.c_str(), 5, "pie", "polar", "polar_histogram", "polar_heatmap", "nonuniformpolar_heatmap"))
    {
      double x_center, y_center, r;

      x_center = 0.5 * (viewport[0] + viewport[1]);
      y_center = 0.5 * (viewport[2] + viewport[3]);
      r = 0.45 * grm_min(viewport[1] - viewport[0], viewport[3] - viewport[2]);
      if (title_margin)
        {
          r *= 0.975;
          y_center -= 0.025 * r;
        }
      viewport[0] = x_center - r;
      viewport[1] = x_center + r;
      viewport[2] = y_center - r;
      viewport[3] = y_center + r;
    }

  gr_setviewport(viewport[0], viewport[1], viewport[2], viewport[3]);
  elem->setAttribute("viewport", true);
  elem->setAttribute("viewport_xmin", viewport[0]);
  elem->setAttribute("viewport_xmax", viewport[1]);
  elem->setAttribute("viewport_ymin", viewport[2]);
  elem->setAttribute("viewport_ymax", viewport[3]);
  elem->setAttribute("vp", true);
  elem->setAttribute("vp_xmin", vp[0]);
  elem->setAttribute("vp_xmax", vp[1]);
  elem->setAttribute("vp_ymin", vp[2]);
  elem->setAttribute("vp_ymax", vp[3]);
}

static void legend_size(std::vector<std::string> labels, double *w, double *h)
{
  double tbx[4], tby[4];
  unsigned int num_labels;

  *w = 0;
  *h = 0;

  if (!labels.empty())
    {
      for (std::string current_label : labels)
        {
          gr_inqtext(0, 0, current_label.data(), tbx, tby);
          *w = grm_max(*w, tbx[2] - tbx[0]);
          *h += grm_max(tby[2] - tby[0], 0.03);
        }
    }
}

static void drawLegend(const std::shared_ptr<GR::Element> &elem, const std::shared_ptr<GR::Context> &context)
{
  double viewport[4];
  int location;
  double px, py, w, h;
  double tbx[4], tby[4];
  double legend_symbol_x[2], legend_symbol_y[2];
  int i;
  std::shared_ptr<GR::Render> render;
  gr_savestate();
  gr_inqviewport(&viewport[0], &viewport[1], &viewport[2], &viewport[3]);
  auto labels_key = static_cast<std::string>(elem->getAttribute("labels"));
  auto specs_key = static_cast<std::string>(elem->getAttribute("specs"));

  render = std::dynamic_pointer_cast<GR::Render>(elem->ownerDocument());
  if (!render)
    {
      throw NotFoundError("No render-document found for element\n");
    }
  std::vector<std::string> labels = GR::get<std::vector<std::string>>((*context)[labels_key]);
  std::vector<std::string> specs = GR::get<std::vector<std::string>>((*context)[specs_key]);

  location = static_cast<int>(elem->getAttribute("location"));

  legend_size(labels, &w, &h);

  if (int_equals_any(location, 3, 11, 12, 13))
    {
      px = viewport[1] + 0.11;
    }
  else if (int_equals_any(location, 3, 8, 9, 10))
    {
      px = 0.5 * (viewport[0] + viewport[1] - w + 0.05);
    }
  else if (int_equals_any(location, 3, 2, 3, 6))
    {
      px = viewport[0] + 0.11;
    }
  else
    {
      px = viewport[1] - 0.05 - w;
    }
  if (int_equals_any(location, 5, 5, 6, 7, 10, 12))
    {
      py = 0.5 * (viewport[2] + viewport[3] + h) - 0.03;
    }
  else if (location == 13)
    {
      py = viewport[2] + h;
    }
  else if (int_equals_any(location, 3, 3, 4, 8))
    {
      py = viewport[2] + h + 0.03;
    }
  else if (location == 11)
    {
      py = viewport[3] - 0.03;
    }
  else
    {
      py = viewport[3] - 0.06;
    }

  if (elem->hasChildNodes())
    {
      for (const auto &child : elem->children())
        {
          elem->removeChild(child);
          child->remove();
        }
    }
  auto newGroup = render->createGroup("groupCreatedDuringRender");
  elem->append(newGroup);

  render->setSelntran(newGroup, 0);
  render->setScale(newGroup, 0);

  auto fr = render->createFillRect(px - 0.08, px + w + 0.02, py + 0.03, py - h);
  newGroup->append(fr);

  render->setFillIntStyle(newGroup, GKS_K_INTSTYLE_SOLID);
  render->setFillColorInd(newGroup, 0);

  auto dr = render->createDrawRect(px - 0.08, px + w + 0.02, py + 0.03, py - h);
  newGroup->append(dr);

  render->setLineType(dr, GKS_K_INTSTYLE_SOLID);
  render->setLineColorInd(dr, 1);
  render->setLineWidth(dr, 1);

  i = 0;
  render->setLineSpec(newGroup, const_cast<char *>(" "));

  for (std::string spec : specs)
    {
      int mask;
      double dy;

      if (i <= labels.size())
        {
          gr_inqtext(0, 0, labels[i].data(), tbx, tby);
          dy = grm_max((tby[2] - tby[0]) - 0.03, 0);
          py -= 0.5 * dy;
        }

      gr_savestate();
      mask = gr_uselinespec(spec.data());
      gr_restorestate();

      if (int_equals_any(mask, 5, 0, 1, 3, 4, 5))
        {
          legend_symbol_x[0] = px - 0.07;
          legend_symbol_x[1] = px - 0.01;
          legend_symbol_y[0] = py;
          legend_symbol_y[1] = py;
          auto pl =
              render->createPolyline(legend_symbol_x[0], legend_symbol_x[1], legend_symbol_y[0], legend_symbol_y[1]);
          newGroup->append(pl);
          render->setLineSpec(pl, spec);
        }
      if (mask & 2)
        {
          legend_symbol_x[0] = px - 0.06;
          legend_symbol_x[1] = px - 0.02;
          legend_symbol_y[0] = py;
          legend_symbol_y[1] = py;
          auto pl =
              render->createPolyline(legend_symbol_x[0], legend_symbol_x[1], legend_symbol_y[0], legend_symbol_y[1]);
          newGroup->append(pl);
          render->setLineSpec(pl, spec);
        }
      if (i < labels.size())
        {
          auto tx = render->createText(px, py, labels[i].data());
          newGroup->append(tx);
          render->setTextAlign(tx, GKS_K_TEXT_HALIGN_LEFT, GKS_K_TEXT_VALIGN_HALF);
          py -= 0.5 * dy;
          i += 1;
        }
      py -= 0.03;
    }
  auto resetGroup = render->createGroup("reset_selntran");
  render->setSelntran(resetGroup, 1);
  elem->append(resetGroup);
  gr_restorestate();
}

static void drawPolarAxes(const std::shared_ptr<GR::Element> &elem, const std::shared_ptr<GR::Context> &context)
{
  double window[4], viewport[4], vp[4];
  double diag;
  double charheight;
  double r_min, r_max;
  double tick;
  double x[2], y[2];
  int i, n;
  double alpha;
  char text_buffer[PLOT_POLAR_AXES_TEXT_BUFFER];
  std::string kind;
  int angle_ticks, rings;
  int phiflip = 0;
  double interval;
  std::string title;
  std::string norm;

  std::shared_ptr<GR::Render> render;

  gr_inqviewport(&viewport[0], &viewport[1], &viewport[2], &viewport[3]);
  gr_inqwindow(&window[0], &window[1], &window[2], &window[3]);

  render = std::dynamic_pointer_cast<GR::Render>(elem->ownerDocument());
  if (!render)
    {
      throw NotFoundError("No render-document for element found\n");
    }

  auto newGroup = render->createGroup("groupCreatedDuringRender");
  elem->append(newGroup);

  auto subplotElement = getSubplotElement(elem);
  vp[0] = static_cast<double>(subplotElement->getAttribute("vp_xmin"));
  vp[1] = static_cast<double>(subplotElement->getAttribute("vp_xmax"));
  vp[2] = static_cast<double>(subplotElement->getAttribute("vp_ymin"));
  vp[3] = static_cast<double>(subplotElement->getAttribute("vp_ymax"));

  diag = std::sqrt((viewport[1] - viewport[0]) * (viewport[1] - viewport[0]) +
                   (viewport[3] - viewport[2]) * (viewport[3] - viewport[2]));
  charheight = grm_max(0.018 * diag, 0.012);

  r_min = window[2];
  r_max = window[3];

  angle_ticks = static_cast<int>(elem->getAttribute("angle_ticks"));
  rings = static_cast<int>(elem->getAttribute("rings"));

  kind = static_cast<std::string>(elem->getAttribute("kind"));

  render->setCharHeight(newGroup, charheight);
  render->setLineType(newGroup, GKS_K_LINETYPE_SOLID);

  tick = static_cast<double>(elem->getAttribute("tick"));
  if (kind == "polar_histogram")
    {
      r_min = 0.0;
      norm = static_cast<std::string>(elem->getAttribute("norm"));
      r_max = static_cast<double>(elem->getAttribute("r_max"));
    }
  else
    {
      rings = grm_max(4, (int)(r_max - r_min));
      elem->setAttribute("rings", rings);
    }


  n = rings;
  phiflip = static_cast<int>(elem->getAttribute("phiflip"));

  for (i = 0; i <= n; i++)
    {
      double r = 1.0 / n * i;
      if (i % 2 == 0)
        {
          if (i > 0)
            {
              auto temp = render->createDrawArc(-r, r, -r, r, 0, 180);
              newGroup->append(temp);
              render->setLineColorInd(temp, 88);

              temp = render->createDrawArc(-r, r, -r, r, 180, 360);
              newGroup->append(temp);
              render->setLineColorInd(temp, 88);
            }
          render->setTextAlign(newGroup, GKS_K_TEXT_HALIGN_LEFT, GKS_K_TEXT_VALIGN_HALF);

          x[0] = 0.05;
          y[0] = r;
          snprintf(text_buffer, PLOT_POLAR_AXES_TEXT_BUFFER, "%.1lf", r_min + tick * i);
          newGroup->append(render->createText(x[0], y[0], text_buffer, CoordinateSpace::WC));
        }
      else
        {
          auto temp = render->createDrawArc(-r, r, -r, r, 0, 180);
          newGroup->append(temp);
          render->setLineColorInd(temp, 90);

          temp = render->createDrawArc(-r, r, -r, r, 180, 360);
          newGroup->append(temp);
          render->setLineColorInd(temp, 90);
        }
    }
  interval = 360.0 / angle_ticks;
  for (alpha = 0.0; alpha < 360; alpha += interval)
    {
      x[0] = std::cos(alpha * M_PI / 180.0);
      y[0] = std::sin(alpha * M_PI / 180.0);
      x[1] = 0.0;
      y[1] = 0.0;

      auto temp = render->createPolyline(x[0], x[1], y[0], y[1]);
      newGroup->append(temp);
      render->setLineColorInd(temp, 90 - i % 2 * 2);

      x[0] *= 1.1;
      y[0] *= 1.1;
      if (phiflip == 0)
        {
          snprintf(text_buffer, PLOT_POLAR_AXES_TEXT_BUFFER, "%d\xb0", (int)grm_round(alpha));
        }
      else
        {
          if (alpha == 0.0)
            snprintf(text_buffer, PLOT_POLAR_AXES_TEXT_BUFFER, "%d\xb0", 0);
          else
            snprintf(text_buffer, PLOT_POLAR_AXES_TEXT_BUFFER, "%d\xb0", 330 - (int)grm_round(alpha - interval));
        }
      temp = render->createText(x[0], y[0], text_buffer, CoordinateSpace::WC);
      newGroup->append(temp);
      render->setTextAlign(temp, GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_HALF);
    }
  title = static_cast<std::string>(elem->getAttribute("title"));
  if (title != "")
    {
      auto temp = render->createText(0.5 * (viewport[0] + viewport[1]), vp[3] - 0.02, title);
      newGroup->append(temp);
      render->setTextAlign(temp, GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_TOP);
    }
}

static void setNextColor(gr_color_type_t color_type, std::vector<int> &color_indices,
                         std::vector<double> &color_rgb_values, const std::shared_ptr<GR::Element> &elem)
{
  const static std::vector<int> fallback_color_indices{989, 982, 980, 981, 996, 983, 995, 988, 986, 990,
                                                       991, 984, 992, 993, 994, 987, 985, 997, 998, 999};
  static double saved_color[3];
  static int last_array_index = -1;
  static unsigned int color_array_length = -1;
  int current_array_index = last_array_index + 1;
  int color_index = 0;
  int reset = (color_type == GR_COLOR_RESET);
  int gks_errind = GKS_K_NO_ERROR;

  if (reset)
    {
      last_array_index = -1;
      color_array_length = -1;
      return;
    }

  if (color_indices.empty() && color_rgb_values.empty())
    {
      color_indices = fallback_color_indices;
    }

  if (last_array_index < 0 && !color_rgb_values.empty())
    {
      gks_inq_color_rep(1, PLOT_CUSTOM_COLOR_INDEX, GKS_K_VALUE_SET, &gks_errind, &saved_color[0], &saved_color[1],
                        &saved_color[2]);
    }

  current_array_index %= color_array_length;

  if (!color_indices.empty())
    {
      color_index = color_indices[current_array_index];
      last_array_index = current_array_index;
    }
  else if (!color_rgb_values.empty())
    {
      color_index = PLOT_CUSTOM_COLOR_INDEX;
      global_render->setColorRep(elem, PLOT_CUSTOM_COLOR_INDEX, color_rgb_values[current_array_index],
                                 color_rgb_values[current_array_index + 1], color_rgb_values[current_array_index + 2]);
      last_array_index = current_array_index + 2;
    }

  if (color_type & GR_COLOR_LINE)
    {
      global_render->setLineColorInd(elem, color_index);
    }
  if (color_type & GR_COLOR_MARKER)
    {
      global_render->setMarkerColorInd(elem, color_index);
    }
  if (color_type & GR_COLOR_FILL)
    {
      global_render->setFillColorInd(elem, color_index);
    }
  if (color_type & GR_COLOR_TEXT)
    {
      global_render->setTextColorInd(elem, color_index);
    }
  if (color_type & GR_COLOR_BORDER)
    {
      global_render->setBorderColorInd(elem, color_index);
    }
}

static void drawPieLegend(const std::shared_ptr<GR::Element> &elem, const std::shared_ptr<GR::Context> &context)
{
  double viewport[4];
  double px, py, w, h;
  double tbx[4], tby[4];
  std::string labels_key = static_cast<std::string>(elem->getAttribute("labels"));
  auto labels = GR::get<std::vector<std::string>>((*context)[labels_key]);
  unsigned int num_labels = labels.size();
  std::shared_ptr<GR::Render> render;
  std::string color_indices_key;
  std::string color_rgb_values_key;

  std::vector<int> color_indices_vec;
  std::vector<double> color_rgb_values_vec;

  gr_inqviewport(&viewport[0], &viewport[1], &viewport[2], &viewport[3]);

  render = std::dynamic_pointer_cast<GR::Render>(elem->ownerDocument());
  if (!render)
    {
      throw NotFoundError("No render-document found for element\n");
    }

  if (elem->hasChildNodes())
    {
      for (const auto &child : elem->children())
        {
          elem->removeChild(child);
          child->remove();
        }
    }

  if (elem->parentElement()->hasAttribute("color_indices"))
    {
      color_indices_key = static_cast<std::string>(elem->parentElement()->getAttribute("color_indices"));
      color_indices_vec = GR::get<std::vector<int>>((*context)[color_indices_key]);
    }
  else if (elem->parentElement()->hasAttribute("color_rgb_values"))
    {
      color_rgb_values_key = static_cast<std::string>(elem->parentElement()->getAttribute("color_rgb_values"));
      color_rgb_values_vec = GR::get<std::vector<double>>((*context)[color_rgb_values_key]);
    }

  auto subGroup = render->createGroup("drawPieLegendSubGroup");
  elem->append(subGroup);

  render->setSelntran(subGroup, 0);
  render->setScale(subGroup, 0);
  w = 0;
  h = 0;
  for (auto current_label : labels)
    {
      gr_inqtext(0, 0, current_label.data(), tbx, tby);
      w += tbx[2] - tbx[0];
      h = grm_max(h, tby[2] - tby[0]);
    }
  w += num_labels * 0.03 + (num_labels - 1) * 0.02;

  px = 0.5 * (viewport[0] + viewport[1] - w);
  py = viewport[2] - 0.75 * h;

  auto fr = render->createFillRect(px - 0.02, px + w + 0.02, py - 0.5 * h - 0.02, py + 0.5 * h + 0.02);
  subGroup->append(fr);
  render->setFillIntStyle(subGroup, GKS_K_INTSTYLE_SOLID);
  render->setFillColorInd(subGroup, 0);
  auto dr = render->createDrawRect(px - 0.02, px + w + 0.02, py - 0.5 * h - 0.02, py + 0.5 * h + 0.02);
  subGroup->append(dr);
  render->setLineType(subGroup, GKS_K_INTSTYLE_SOLID);
  render->setLineColorInd(subGroup, 1);
  render->setLineWidth(subGroup, 1);

  auto subsubGroup = render->createGroup("labels_group");
  subGroup->append(subsubGroup);
  render->setLineSpec(subsubGroup, const_cast<char *>(" "));
  render->setTextAlign(subsubGroup, GKS_K_TEXT_HALIGN_LEFT, GKS_K_TEXT_VALIGN_HALF);
  // reset setNextColor
  setNextColor(GR_COLOR_RESET, color_indices_vec, color_rgb_values_vec, subGroup);


  for (auto &current_label : labels)
    {
      auto labelGroup = render->createGroup("label");
      subsubGroup->append(labelGroup);
      render->setLineColorInd(labelGroup, 1);
      setNextColor(GR_COLOR_FILL, color_indices_vec, color_rgb_values_vec, labelGroup);

      labelGroup->append(render->createFillRect(px, px + 0.02, py - 0.01, py + 0.01));
      labelGroup->append(render->createDrawRect(px, px + 0.02, py - 0.01, py + 0.01));
      labelGroup->append(render->createText(px + 0.03, py, current_label));

      gr_inqtext(0, 0, current_label.data(), tbx, tby);
      px += tbx[2] - tbx[0] + 0.05;
    }
  // reset setNextColor
  setNextColor(GR_COLOR_RESET, color_indices_vec, color_rgb_values_vec, subGroup);

  auto reset_group = render->createGroup("reset_group");
  render->setSelntran(reset_group, 1);
  elem->append(reset_group);
}

static void piePlotTitleRender(const std::shared_ptr<GR::Element> &elem, const std::shared_ptr<GR::Context> &context)
{
  double viewport[4], vp[4];
  bool vp_found = false;
  std::shared_ptr<GR::Render> render;
  std::shared_ptr<GR::Element> ancestor = elem->parentElement();

  gr_inqviewport(&viewport[0], &viewport[1], &viewport[2], &viewport[3]);

  // Get vp from ancestor GR::element, usually the group with the name "pie"
  while (ancestor->localName() != "root")
    {
      if (ancestor->hasAttribute("vp"))
        {
          vp[0] = static_cast<double>(ancestor->getAttribute("vp_xmin"));
          vp[1] = static_cast<double>(ancestor->getAttribute("vp_xmax"));
          vp[2] = static_cast<double>(ancestor->getAttribute("vp_ymin"));
          vp[3] = static_cast<double>(ancestor->getAttribute("vp_ymax"));
          vp_found = true;
          break;
        }
      else
        {
          ancestor = ancestor->parentElement();
        }
    }
  if (!vp_found)
    {
      throw NotFoundError("No vp was found within ancestors");
    }

  render = std::dynamic_pointer_cast<GR::Render>(elem->ownerDocument());
  if (!render)
    {
      throw NotFoundError("No render-document found for element\n");
    }
  std::string title = static_cast<std::string>(elem->getAttribute("pie-plot-title"));
  if (elem->hasChildNodes())
    {
      for (const auto &child : elem->children())
        {
          elem->removeChild(child);
          child->remove();
        }
    }

  auto tx = render->createText(0.5 * (viewport[0] + viewport[1]), vp[3] - 0.02, title);
  render->setTextColorInd(tx, 1);
  render->setTextAlign(tx, GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_TOP);
  elem->append(tx);
}

static void isosurfaceRender(const std::shared_ptr<GR::Element> &elem, const std::shared_ptr<GR::Context> &context)
{
  double viewport[4];
  double x_min, x_max, y_min, y_max;
  int fig_width, fig_height;
  int subplot_width, subplot_height;
  int drawable_type;

  drawable_type = static_cast<int>(elem->getAttribute("drawable-type"));

  gr_inqviewport(&viewport[0], &viewport[1], &viewport[2], &viewport[3]);

  x_min = viewport[0];
  x_max = viewport[1];
  y_min = viewport[2];
  y_max = viewport[3];

  get_figure_size(NULL, &fig_width, &fig_height, NULL, NULL);
  subplot_width = (int)(grm_max(fig_width, fig_height) * (x_max - x_min));
  subplot_height = (int)(grm_max(fig_width, fig_height) * (y_max - y_min));

  logger((stderr, "viewport: (%lf, %lf, %lf, %lf)\n", x_min, x_max, y_min, y_max));
  logger((stderr, "viewport ratio: %lf\n", (x_min - x_max) / (y_min - y_max)));
  logger((stderr, "subplot size: (%d, %d)\n", subplot_width, subplot_height));
  logger((stderr, "subplot ratio: %lf\n", ((double)subplot_width / (double)subplot_height)));

  gr3_drawimage(x_min, x_max, y_min, y_max, subplot_width, subplot_height, GR3_DRAWABLE_GKS);
}

static void panzoom(const std::shared_ptr<GR::Element> &elem, const std::shared_ptr<GR::Context> &context)
{
  ; /* panzoom is being processed in the processLimits routine */
}

static void drawYLine(const std::shared_ptr<GR::Element> &elem, const std::shared_ptr<GR::Context> &context)
{
  auto draw_axes_element = elem->parentElement();
  double x_tick, x_org_low, x_org_high;
  double y_tick, y_org_low, y_org_high;
  int x_major;
  int y_major;

  getAxesInformation(elem, "low", "low", x_org_low, y_org_low, x_major, y_major, x_tick, y_tick);
  getAxesInformation(elem, "high", "high", x_org_high, y_org_high, x_major, y_major, x_tick, y_tick);
  auto orientation = static_cast<std::string>(elem->getAttribute("orientation"));
  int is_vertical = orientation == "vertical";

  double x[2] = {x_org_low, x_org_high};
  double y[2] = {y_org_low, y_org_low};

  gr_setlinecolorind(1);
  if (is_vertical)
    {
      x[1] = y_org_low;
      y[1] = y_org_high;
    }
  gr_polyline(2, x, y);
}

static void drawIsosurface3(const std::shared_ptr<GR::Element> &elem, const std::shared_ptr<GR::Context> &context)
{
  auto nx = static_cast<int>(elem->getAttribute("nx"));
  auto ny = static_cast<int>(elem->getAttribute("ny"));
  auto nz = static_cast<int>(elem->getAttribute("nz"));
  auto data_key = static_cast<std::string>(elem->getAttribute("data"));
  float isovalue = static_cast<double>(elem->getAttribute("isovalue"));
  auto color_key = static_cast<std::string>(elem->getAttribute("color"));
  auto strides_key = static_cast<std::string>(elem->getAttribute("strides"));

  float ambient = static_cast<double>(elem->getAttribute("ambient"));
  float diffuse = static_cast<double>(elem->getAttribute("diffuse"));
  float specular = static_cast<double>(elem->getAttribute("specular"));
  float specular_power = static_cast<double>(elem->getAttribute("specular_power"));

  auto data_vec = GR::get<std::vector<double>>((*context)[data_key]);
  auto color_vec = GR::get<std::vector<double>>((*context)[color_key]);
  auto strides_vec = GR::get<std::vector<int>>((*context)[strides_key]);

  std::vector<float> data_vec_f(data_vec.begin(), data_vec.end());
  std::vector<float> color_vec_f(color_vec.begin(), color_vec.end());

  float *data = &data_vec_f[0];
  float *color = &color_vec_f[0];
  int *strides = &strides_vec[0];

  float light_parameters[4];

  /* Save and restore original light parameters */
  gr3_getlightparameters(&light_parameters[0], &light_parameters[1], &light_parameters[2], &light_parameters[3]);
  gr3_setlightparameters(ambient, diffuse, specular, specular_power);

  gr3_isosurface(nx, ny, nz, data, isovalue, color, strides);

  gr3_setlightparameters(light_parameters[0], light_parameters[1], light_parameters[2], light_parameters[3]);
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

static void setTextColorForBackground(const std::shared_ptr<GR::Element> &elem)
/*  The set_text_volor_for_background function used in plot.cxx now as an attribute function
    It is now possible to inquire colors during runtime -> No colors are given as parameters
    The new color is set on `elem`
    There are no params apart from elem
    \param[in] elem The GR::Element the color should be set in. Also contains other attributes which may function as
 parameters

    Attributes as Parameters (with prefix "stcfb-"):
    plot: for which plot it is used: right now only pie plot
 */
{
  int color_ind;
  int inq_color;
  unsigned char color_rgb[4];
  std::string plot = "pie";

  if (elem->hasAttribute("stcfb-plot"))
    {
      plot = static_cast<std::string>(elem->getAttribute("stcfb-plot"));
    }

  if (plot == "pie")
    {
      double r, g, b;
      double color_lightness;
      std::shared_ptr<GR::Render> render;

      render = std::dynamic_pointer_cast<GR::Render>(elem->ownerDocument());
      if (!render)
        {
          throw NotFoundError("Render-document not found for element\n");
        }
      if (elem->hasAttribute("color_index"))
        {
          color_ind = static_cast<int>(elem->getAttribute("color_index"));
        }
      else
        {
          gr_inqfillcolorind(&color_ind);
        }
      gr_inqcolor(color_ind, (int *)color_rgb);

      r = color_rgb[0] / 255.0;
      g = color_rgb[1] / 255.0;
      b = color_rgb[2] / 255.0;

      color_lightness = get_lightness_from_rbg(r, g, b);
      if (color_lightness < 0.4)
        {
          gr_settextcolorind(0);
        }
      else
        {
          gr_settextcolorind(1);
        }
    }
}

static void processKind(const std::shared_ptr<GR::Element> &elem)
{
  std::string kind = static_cast<std::string>(elem->getAttribute("kind"));
  if (kind == "line")
    {
      if (elem->hasAttribute("marginalheatmap"))
        {
          std::shared_ptr<GR::Context> context;
          std::shared_ptr<GR::Render> render;

          render = std::dynamic_pointer_cast<GR::Render>(elem->ownerDocument());
          if (!render)
            {
              throw NotFoundError("Render-document not found for element\n");
            }
          context = render->getContext();
          auto orientation = static_cast<std::string>(elem->getAttribute("orientation"));
          int is_vertical = orientation == "vertical";
          int xind = static_cast<int>(elem->getAttribute("xind"));
          int yind = static_cast<int>(elem->getAttribute("yind"));

          auto marginalheatmap_group = elem->parentElement();

          double c_min = static_cast<double>(marginalheatmap_group->getAttribute("lim_zmin"));
          double c_max = static_cast<double>(marginalheatmap_group->getAttribute("lim_zmax"));

          for (auto &child : elem->children())
            {
              if (child->hasChildNodes())
                {
                  for (auto &grandchild : child->children())
                    {
                      child->removeChild(grandchild);
                      grandchild->remove();
                    }
                }
              int i;
              double y_max = 0;
              std::vector<double> plot =
                  GR::get<std::vector<double>>((*context)[static_cast<std::string>(child->getAttribute("plot"))]);
              std::vector<double> y =
                  GR::get<std::vector<double>>((*context)[static_cast<std::string>(child->getAttribute("y"))]);
              std::vector<double> xi =
                  GR::get<std::vector<double>>((*context)[static_cast<std::string>(child->getAttribute("xi"))]);
              std::vector<double> x =
                  GR::get<std::vector<double>>((*context)[static_cast<std::string>(child->getAttribute("x"))]);

              int y_length = y.size();
              int x_length = xi.size();

              double xmin = static_cast<double>(marginalheatmap_group->getAttribute("lim_xmin"));
              double xmax = static_cast<double>(marginalheatmap_group->getAttribute("lim_xmax"));
              double ymin = static_cast<double>(marginalheatmap_group->getAttribute("lim_ymin"));
              double ymax = static_cast<double>(marginalheatmap_group->getAttribute("lim_ymax"));
              // plot step in marginal
              for (i = 0; i < (is_vertical ? y_length : x_length); i++)
                {
                  if (is_vertical)
                    {
                      y[(is_vertical ? y_length : x_length) - i - 1] =
                          std::isnan(plot[xind + i * x_length]) ? 0 : plot[xind + i * x_length];
                      y_max = grm_max(y_max, y[(is_vertical ? y_length : x_length) - i - 1]);
                    }
                  else
                    {
                      y[i] = std::isnan(plot[x_length * (y_length - 1 - yind) + i])
                                 ? 0
                                 : plot[x_length * (y_length - 1 - yind) + i];
                      y_max = grm_max(y_max, y[i]);
                    }
                }
              for (i = 0; i < (is_vertical ? y_length : x_length); i++)
                {
                  y[i] = y[i] / y_max * (c_max / 15);
                  xi[i] = x[i] + (is_vertical ? ymin : xmin);
                }

              double x_pos, y_pos;
              unsigned int len = is_vertical ? y_length : x_length;
              std::vector<double> x_step_boundaries(2 * len);
              std::vector<double> y_step_values(2 * len);

              x_step_boundaries[0] = is_vertical ? ymin : xmin;
              for (i = 2; i < 2 * len; i += 2)
                {
                  x_step_boundaries[i - 1] = x_step_boundaries[i] =
                      x_step_boundaries[0] + (i / 2) * (is_vertical ? (ymax - ymin) : (xmax - xmin)) / len;
                }
              x_step_boundaries[2 * len - 1] = is_vertical ? ymax : xmax;
              y_step_values[0] = y[0];
              for (i = 2; i < 2 * len; i += 2)
                {
                  y_step_values[i - 1] = y[i / 2 - 1];
                  y_step_values[i] = y[i / 2];
                }
              y_step_values[2 * len - 1] = y[len - 1];

              int id = static_cast<int>(global_root->getAttribute("id"));
              global_root->setAttribute("id", id + 1);
              auto id_str = std::to_string(id);

              std::shared_ptr<GR::Element> line_elem, marker_elem;

              if (is_vertical)
                {
                  line_elem =
                      global_render->createPolyline("x" + id_str, y_step_values, "y" + id_str, x_step_boundaries);
                  x_pos = (x_step_boundaries[yind * 2] + x_step_boundaries[yind * 2 + 1]) / 2;
                  y_pos = y[yind];
                  marker_elem = global_render->createPolymarker(y_pos, x_pos);
                }
              else
                {
                  line_elem =
                      global_render->createPolyline("x" + id_str, x_step_boundaries, "y" + id_str, y_step_values);
                  x_pos = (x_step_boundaries[xind * 2] + x_step_boundaries[xind * 2 + 1]) / 2;
                  y_pos = y[xind];
                  marker_elem = global_render->createPolymarker(x_pos, y_pos);
                }

              global_render->setLineColorInd(line_elem, 989);
              global_render->setMarkerColorInd(marker_elem, 2);
              global_render->setMarkerType(marker_elem, -1);
              global_render->setMarkerSize(marker_elem, 1.5 * (len / (is_vertical ? (ymax - ymin) : (xmax - xmin))));
              child->append(marker_elem);
              child->append(line_elem);
            } // end for child_series
        }
    }
  else if (kind == "hist")
    {
      if (elem->hasAttribute("xind") || elem->hasAttribute("yind"))
        {
          int xind, yind;
          xind = elem->hasAttribute("xind") ? static_cast<int>(elem->getAttribute("xind")) : -1;
          yind = elem->hasAttribute("yind") ? static_cast<int>(elem->getAttribute("yind")) : -1;

          if (xind == -1 && yind == -1)
            {
              return;
            }

          bool is_horizontal = static_cast<std::string>(elem->getAttribute("orientation")) == "horizontal";
          for (auto &childSeries : elem->children()) // iterate over all hist series
            {
              std::vector<std::shared_ptr<GR::Element>> groups = childSeries->children();
              std::vector<std::shared_ptr<GR::Element>> bars;

              if (groups.size() == 2)
                {
                  bars = groups[0]->children();
                }
              else
                {
                  // error no fillgroups?
                  return;
                }

              if ((is_horizontal && xind == -1) || (!is_horizontal && yind == -1))
                {
                  continue;
                }
              if ((is_horizontal ? xind : yind) >= bars.size())
                {
                  continue;
                }
              bars[(is_horizontal ? xind : yind)]->setAttribute("fillcolorind", 2);
            }
        }
    }
}

static void processAttributes(const std::shared_ptr<GR::Element> &element)
{
  /*!
   * processtd::sing function for all kinds of attributes
   *
   * \param[in] element The GR::Element containing attributes
   */

  //! Map used for processtd::sing all kinds of attributes
  static std::map<std::string, std::function<void(const std::shared_ptr<GR::Element> &)>> attrStringToFunc{
      {std::string("kind"), processKind},
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
      {std::string("relative-charheight"), processRelativeCharHeight},
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
      {std::string("title"), processTitle},
      {std::string("transparency"),
       [](const std::shared_ptr<GR::Element> &elem) {
         gr_settransparency((double)elem->getAttribute("transparency"));
       }},
      {std::string("linespec"),
       [](const std::shared_ptr<GR::Element> &elem) {
         gr_uselinespec(((std::string)elem->getAttribute("linespec")).data());
       }},
      {std::string("limits"), GR::Render::processLimits},
      {std::string("window"), processWindow},
      {std::string("resamplemethod"),
       [](const std::shared_ptr<GR::Element> &elem) {
         gr_setresamplemethod((int)elem->getAttribute("resamplemethod"));
       }},
      {std::string("projectiontype"), processProjectionType},
      {std::string("space3d"), processSpace3d},
      {std::string("space"), processSpace},
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
       [](const std::shared_ptr<GR::Element> &elem) { gr_selectclipxform((int)elem->getAttribute("clipxform")); }},
      {std::string("xlabel"), processXlabel},
      {std::string("xticklabels"), processXTickLabels},
      {std::string("ylabel"), processYlabel},
      {std::string("imshow-information"), processImshowInformation},
      {std::string("gr-option-flip-x"), processGROptionFlipX},
      {std::string("gr-option-flip-y"), processGROptionFlipY},
      {std::string("calc-window-and-viewport-from-parent"), processWindowAndViewportFromParent},
      {std::string("colorbar-position"), processColorbarPosition}};


  static std::map<std::string, std::function<void(const std::shared_ptr<GR::Element> &)>> attrStringToFuncPost{
      /* This map contains functions for attributes that should be called after some attributes have been processed
       * already. These functions can contain e.g. inquire function calls for colors.
       * */
      {std::string("set-text-color-for-background"), setTextColorForBackground}};

  static std::map<std::string, std::function<void(const std::shared_ptr<GR::Element> &)>> attrStringToFuncPre{
      /* This map contains functions for attributes that should be called before other attributes are being processed.
       * */
      {std::string("subplot"), processSubplot},
      {std::string("viewport"), GR::Render::processViewport},
      {std::string("charheight"),
       [](const std::shared_ptr<GR::Element> &elem) { gr_setcharheight((double)elem->getAttribute("charheight")); }}};

  for (auto attributeToFunctionPair : attrStringToFuncPre)
    {
      if (element->getAttributeNames().find(attributeToFunctionPair.first) != element->getAttributeNames().end())
        {
          attrStringToFuncPre[attributeToFunctionPair.first](element);
        }
    }

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

  for (auto &attribute : element->getAttributeNames())
    /*
     * Post process attribute run
     */
    {
      if (attrStringToFuncPost.find(attribute) != attrStringToFuncPost.end())
        {
          attrStringToFuncPost[attribute](element);
        }
    }
}

static void processElement(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Processtd::sing function for all kinds of elements
   *
   * \param[in] element The GR::Element containing attributes and data keys
   * \param[in] context The GR::Context containing the actual data
   */

  //! Map used for processtd::sing all kinds of elements
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
                       {std::string("titles3d"), titles3d},
                       {std::string("gr3clear"), gr3Clear},
                       {std::string("gr3deletemesh"), gr3DeleteMesh},
                       {std::string("gr3drawimage"), gr3DrawImage},
                       {std::string("shadepoints"), shadePoints},
                       {std::string("clearws"), clearWS},
                       {std::string("updatews"), updateWS},
                       {std::string("drawgraphics"), drawGraphics},
                       {std::string("layout-grid"), layoutGrid},
                       {std::string("layout-gridelement"), layoutGridElement},
                       {std::string("draw-legend"), drawLegend},
                       {std::string("draw-polar-axes"), drawPolarAxes},
                       {std::string("draw-pie-legend"), drawPieLegend},
                       {std::string("pie-plot-title-render"), piePlotTitleRender},
                       {std::string("panzoom"), panzoom},
                       {std::string("y-line"), drawYLine},
                       {std::string("isosurface3"), drawIsosurface3},
                       {std::string("isosurface-render"), isosurfaceRender}};
  /*! Modifier */
  if (element->localName() == "group")
    {
      processAttributes(element);
    }
  else
    {
      /*! Drawnodes */
      processAttributes(element);
      try
        {
          std::function<void(const std::shared_ptr<GR::Element> &, const std::shared_ptr<GR::Context> &)> f =
              elemStringToFunc[element->localName()];
          f(element, context);
        }
      catch (std::bad_function_call &e)
        {
          throw NotFoundError("No dom render function found for element with local name: " + element->localName());
        }
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
  element->setAttribute("space", static_cast<int>(space));
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
                                                    int x_major, int y_major, int tick_orientation)
{
  /*!
   * This function can be used for creating an Axes GR::Element
   *
   * \param[in] x_tick A double value
   * \param[in] y_tick A double value
   * \param[in] x_org A double value
   * \param[in] y_org A double value
   * \param[in] x_major An Integer value
   * \param[in] y_major An Integer value
   * \param[in] tick_orientation A Double value
   */
  auto element = createElement("axes");
  element->setAttribute("x_tick", x_tick);
  element->setAttribute("y_tick", y_tick);
  element->setAttribute("x_org", x_org);
  element->setAttribute("y_org", y_org);
  element->setAttribute("x_major", x_major);
  element->setAttribute("y_major", y_major);
  element->setAttribute("tick_orientation", tick_orientation);
  return element;
}

std::shared_ptr<GR::Element> GR::Render::createEmptyAxes(int tick_orientation)
{
  /*!
   * This function can be used for creating an Axes GR::Element with missing information
   *
   * \param[in] tick_orientation A Int value specifing the direction of the ticks
   */
  auto element = createElement("axes");
  element->setAttribute("tick_orientation", tick_orientation);
  return element;
}

std::shared_ptr<GR::Element> GR::Render::createDrawLegend(const std::string &labels_key,
                                                          std::optional<std::vector<std::string>> labels, int location,
                                                          const std::string &specs_key,
                                                          std::optional<std::vector<std::string>> specs,
                                                          const std::shared_ptr<GR::Context> &extContext)
{
  /*!
   * This function can be used for creating a DrawLegend GR::Element
   * This element is different compared to most of Render's GR::Element, the DrawLegend GR::Element will incorporate
   * plot_draw_legend code from plot.cxx and will create new GR::Elements as child nodes in the render document
   *
   * \param[in] labels_key A std::string for the labels vector
   * \param[in] labels May be an std::vector<std::string>> containing the labels
   * \param[in] location An int value
   * \param[in] spec An std::string
   */

  auto element = createElement("draw-legend");
  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;
  element->setAttribute("location", location);
  element->setAttribute("specs", specs_key);
  element->setAttribute("labels", labels_key);

  if (labels != std::nullopt)
    {
      (*useContext)[labels_key] = *labels;
    }
  if (specs != std::nullopt)
    {
      (*useContext)[specs_key] = *specs;
    }

  return element;
}


std::shared_ptr<GR::Element> GR::Render::createDrawPolarAxes(int angle_ticks, int rings, const std::string &kind,
                                                             int phiflip, const std::string &title, double r_max,
                                                             const std::string &norm, double tick, double line_width)
{
  auto element = createElement("draw-polar-axes");
  if (!title.empty())
    {
      element->setAttribute("title", title);
    }
  if (!norm.empty())
    {
      element->setAttribute("norm", norm);
    }
  if (tick != 0.0)
    {
      element->setAttribute("tick", tick);
    }
  if (r_max != -1.0)
    {
      element->setAttribute("r_max", r_max);
    }
  if (line_width != 0.0)
    {
      element->setAttribute("linewidth", line_width);
    }
  element->setAttribute("angle_ticks", angle_ticks);
  element->setAttribute("rings", rings);
  element->setAttribute("kind", kind);
  element->setAttribute("phiflip", phiflip);

  return element;
}

std::shared_ptr<GR::Element> GR::Render::createDrawPieLegend(const std::string &labels_key,
                                                             std::optional<std::vector<std::string>> labels,
                                                             const std::shared_ptr<GR::Context> &extContext)
{
  auto element = createElement("draw-pie-legend");
  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;
  element->setAttribute("labels", labels_key);

  if (labels != std::nullopt)
    {
      (*useContext)[labels_key] = *labels;
    }
  return element;
}

std::shared_ptr<GR::Element> GR::Render::createPiePlotTitleRenderElement(std::string title)
{
  auto element = createElement("pie-plot-title-render");
  element->setAttribute("pie-plot-title", title);
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

std::shared_ptr<GR::Element> GR::Render::createEmptyGrid(bool x_grid, bool y_grid)
{
  auto element = createElement("grid");
  if (!x_grid)
    {
      element->setAttribute("x_tick", 0);
    }
  if (!y_grid)
    {
      element->setAttribute("y_tick", 0);
    }
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
                                                       const std::string &py_key, std::optional<std::vector<double>> py,
                                                       const std::string &pz_key, std::optional<std::vector<double>> pz,
                                                       int option, int accelerate,
                                                       const std::shared_ptr<GR::Context> &extContext)
{
  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;
  auto element = createElement("surface");
  element->setAttribute("px", px_key);
  element->setAttribute("py", py_key);
  element->setAttribute("pz", pz_key);
  element->setAttribute("option", option);
  element->setAttribute("accelerate", accelerate);

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
                                                      double y_org, double z_org, int x_major, int y_major, int z_major)
{
  auto element = createElement("grid3d");
  element->setAttribute("x_tick", x_tick);
  element->setAttribute("y_tick", y_tick);
  element->setAttribute("z_tick", z_tick);
  element->setAttribute("x_org", x_org);
  element->setAttribute("y_org", y_org);
  element->setAttribute("z_org", z_org);
  element->setAttribute("x_major", x_major);
  element->setAttribute("y_major", y_major);
  element->setAttribute("z_major", z_major);
  return element;
}

std::shared_ptr<GR::Element> GR::Render::createEmptyGrid3d(bool x_grid, bool y_grid, bool z_grid)
{
  auto element = createElement("grid3d");
  if (!x_grid)
    {
      element->setAttribute("x_tick", 0);
    }
  if (!y_grid)
    {
      element->setAttribute("y_tick", 0);
    }
  if (!z_grid)
    {
      element->setAttribute("z_tick", 0);
    }
  return element;
}

std::shared_ptr<GR::Element> GR::Render::createAxes3d(double x_tick, double y_tick, double z_tick, double x_org,
                                                      double y_org, double z_org, int major_x, int major_y, int major_z,
                                                      int tick_orientation)
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
  element->setAttribute("tick_orientation", tick_orientation);
  return element;
}

std::shared_ptr<GR::Element> GR::Render::createEmptyAxes3d(int tick_orientation)
{
  auto element = createElement("axes3d");
  element->setAttribute("tick_orientation", tick_orientation);
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

std::shared_ptr<GR::Element>
GR::Render::createGR3Isosurface(int nx, int ny, int nz, const std::string &data_key,
                                std::optional<std::vector<double>> data, double isovalue, const std::string &color_key,
                                std::optional<std::vector<double>> color, const std::string &strides_key,
                                std::optional<std::vector<int>> strides, const std::shared_ptr<GR::Context> &extContext)
{
  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;

  auto element = createElement("isosurface3");
  element->setAttribute("nx", nx);
  element->setAttribute("ny", ny);
  element->setAttribute("nz", nz);
  element->setAttribute("data", data_key);
  element->setAttribute("isovalue", isovalue);
  element->setAttribute("color", color_key);
  element->setAttribute("strides", strides_key);

  if (data != std::nullopt)
    {
      (*useContext)[data_key] = *data;
    }
  if (color != std::nullopt)
    {
      (*useContext)[color_key] = *color;
    }
  if (strides != std::nullopt)
    {
      (*useContext)[strides_key] = *strides;
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

std::shared_ptr<GR::Element> GR::Render::createTitles3d(const std::string &x, const std::string &y,
                                                        const std::string &z)
{
  auto element = createElement("titles3d");
  element->setAttribute("x", x);
  element->setAttribute("y", y);
  element->setAttribute("z", z);

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


std::shared_ptr<GR::Element> GR::Render::createIsoSurfaceRenderElement(int drawable_type)
{
  auto element = createElement("isosurface-render");
  element->setAttribute("drawable-type", drawable_type);
  return element;
}


std::shared_ptr<GR::Element> GR::Render::createLayoutGrid(const grm::Grid &grid)
{
  auto element = createElement("layout-grid");

  element->setAttribute("absHeight", grid.absHeight);
  element->setAttribute("absWidth", grid.absWidth);
  element->setAttribute("absHeightPxl", grid.absHeightPxl);
  element->setAttribute("absWidthPxl", grid.absWidthPxl);
  element->setAttribute("fitParentsHeight", grid.fitParentsHeight);
  element->setAttribute("fitParentsWidth", grid.fitParentsWidth);
  element->setAttribute("relativeHeight", grid.relativeHeight);
  element->setAttribute("relativeWidth", grid.relativeWidth);
  element->setAttribute("aspectRatio", grid.aspectRatio);
  element->setAttribute("widthSet", grid.widthSet);
  element->setAttribute("heightSet", grid.heightSet);
  element->setAttribute("arSet", grid.arSet);
  element->setAttribute("nrows", grid.getNRows());
  element->setAttribute("ncols", grid.getNCols());

  return element;
}


std::shared_ptr<GR::Element> GR::Render::createLayoutGridElement(const grm::GridElement &gridElement,
                                                                 const grm::Slice &slice)
{
  auto element = createElement("layout-gridelement");

  element->setAttribute("absHeight", gridElement.absHeight);
  element->setAttribute("absWidth", gridElement.absWidth);
  element->setAttribute("absHeightPxl", gridElement.absHeightPxl);
  element->setAttribute("absWidthPxl", gridElement.absWidthPxl);
  element->setAttribute("fitParentsHeight", gridElement.fitParentsHeight);
  element->setAttribute("fitParentsWidth", gridElement.fitParentsWidth);
  element->setAttribute("relativeHeight", gridElement.relativeHeight);
  element->setAttribute("relativeWidth", gridElement.relativeWidth);
  element->setAttribute("aspectRatio", gridElement.aspectRatio);
  element->setAttribute("rowStart", slice.rowStart);
  element->setAttribute("rowStop", slice.rowStop);
  element->setAttribute("colStart", slice.colStart);
  element->setAttribute("colStop", slice.colStop);

  double *subplot = gridElement.subplot;
  GR::Render::setSubplot(element, subplot[0], subplot[1], subplot[2], subplot[3]);

  return element;
}

std::shared_ptr<GR::Element> GR::Render::createPanzoom(double x, double y, double xzoom, double yzoom)
{

  auto element = createElement("panzoom");
  element->setAttribute("x", x);
  element->setAttribute("y", y);
  element->setAttribute("xzoom", xzoom);
  element->setAttribute("yzoom", yzoom);
  return element;
}

std::shared_ptr<GR::Element> GR::Render::createYLine()
{

  auto element = createElement("y-line");
  return element;
}

//! Modifierfunctions

void GR::Render::setViewport(const std::shared_ptr<GR::Element> &element, double xmin, double xmax, double ymin,
                             double ymax)
{
  /*!
   * This function can be used to set the viewport of a GR::Element
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
                               std::optional<std::vector<int>> types, const std::shared_ptr<GR::Context> &extContext)
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
  if (types != std::nullopt)
    {
      (*useContext)[types_key] = *types;
    }
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
                               std::optional<std::vector<double>> sizes, const std::shared_ptr<GR::Context> &extContext)
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
  if (sizes != std::nullopt)
    {
      (*useContext)[sizes_key] = *sizes;
    }
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
                                   std::optional<std::vector<int>> colorinds,
                                   const std::shared_ptr<GR::Context> &extContext)
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
  if (colorinds != std::nullopt)
    {
      (*useContext)[colorinds_key] = *colorinds;
    }
  element->setAttribute("markercolorinds", colorinds_key);
}


void GR::Render::setLineType(const std::shared_ptr<Element> &element, const std::string &types_key,
                             std::optional<std::vector<int>> types, const std::shared_ptr<GR::Context> &extContext)
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
  if (types != std::nullopt)
    {
      (*useContext)[types_key] = *types;
    }
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
                              std::optional<std::vector<double>> widths, const std::shared_ptr<GR::Context> &extContext)
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
  if (widths != std::nullopt)
    {
      (*useContext)[widths_key] = *widths;
    }
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
                                 std::optional<std::vector<int>> colorinds,
                                 const std::shared_ptr<GR::Context> &extContext)
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
  if (colorinds != std::nullopt)
    {
      (*useContext)[colorinds_key] = *colorinds;
    }
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

void GR::Render::setTextWidthAndHeight(const std::shared_ptr<Element> &element, double width, double height)
{
  /*!
   * This function can be used to set the width and height of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] width Width of the Element
   * \param[in] height Height of the Element
   */
  element->setAttribute("width", width);
  element->setAttribute("height", height);
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
  element->setAttribute("window_xmin", xmin);
  element->setAttribute("window_xmax", xmax);
  element->setAttribute("window_ymin", ymin);
  element->setAttribute("window_ymax", ymax);
  element->setAttribute("window_zmin", zmin);
  element->setAttribute("window_zmax", zmax);
}

void GR::Render::setSpace3d(const std::shared_ptr<GR::Element> &element, double phi, double theta, double fov,
                            double camera_distance)
{
  /*! This function can be used to set the space3d of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] phi: azimuthal angle of the spherical coordinates
   * \param[in] theta: polar angle of the spherical coordinates
   * \param[in] fov: vertical field of view(0 or NaN for orthographic projection)
   * \param[in] camera_distance: distance between the camera and the focus point (in arbitrary units, 0 or NaN for the
   * radius of the object's smallest bounding sphere)
   */

  element->setAttribute("space3d", true);
  element->setAttribute("space3d_phi", phi);
  element->setAttribute("space3d_theta", theta);
  element->setAttribute("space3d_fov", fov);
  element->setAttribute("space3d_camera-distance", camera_distance);
}

void GR::Render::setSpace(const std::shared_ptr<Element> &element, double zmin, double zmax, int rotation, int tilt)
{
  /*!
   * This function can be used to set the space of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] zmin
   * \param[in] zmax
   * \param[in] rotation
   * \param[in] tilt
   */

  element->setAttribute("space", true);
  element->setAttribute("space_zmin", zmin);
  element->setAttribute("space_zmax", zmax);
  element->setAttribute("space_rotation", rotation);
  element->setAttribute("space_tilt", tilt);
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

void GR::Render::setProjectionType(const std::shared_ptr<GR::Element> &element, int type)
{
  /*!
   * This function can be used to set the projectiontype of a GR::Element
   *
   * \param[in] element A GR::Element
   * \param[in] type The projectiontype
   */
  element->setAttribute("projectiontype", type);
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


void GR::Render::setSubplot(const std::shared_ptr<GR::Element> &element, double xmin, double xmax, double ymin,
                            double ymax)
{
  element->setAttribute("subplot", true);
  element->setAttribute("subplot_xmin", xmin);
  element->setAttribute("subplot_xmax", xmax);
  element->setAttribute("subplot_ymin", ymin);
  element->setAttribute("subplot_ymax", ymax);
}

void GR::Render::setXTickLabels(std::shared_ptr<GR::Element> group, const std::string &key,
                                std::optional<std::vector<std::string>> xticklabels,
                                const std::shared_ptr<GR::Context> &extContext)
{
  /*!
   * This function can be used to create a XTickLabel GR::Element
   *
   * \param[in] key A string used for storing the xticklabels in GR::Context
   * \param[in] xticklabels A vector containing string values representing xticklabels
   * \param[in] extContext A GR::Context that is used for storing the vectors. By default it uses GR::Render's
   * GR::Context object but an external GR::Context can be used
   */

  std::shared_ptr<GR::Context> useContext = (extContext == nullptr) ? context : extContext;
  if (xticklabels != std::nullopt)
    {
      (*useContext)[key] = *xticklabels;
    }
  group->setAttribute("xticklabels", key);
}


void GR::Render::setNextColor(const std::shared_ptr<GR::Element> &element, const std::string &color_indices_key,
                              const std::vector<int> &color_indices, const std::shared_ptr<GR::Context> &extContext)
{
  auto useContext = (extContext == nullptr) ? context : extContext;
  element->setAttribute("setNextColor", 1);
  if (!color_indices.empty())
    {
      (*useContext)[color_indices_key] = color_indices;
      element->setAttribute("color_indices", color_indices_key);
    }
  else
    {
      throw NotFoundError("Color indices are misstd::sing in vector\n");
    }
}

void GR::Render::setNextColor(const std::shared_ptr<GR::Element> &element, const std::string &color_rgb_values_key,
                              const std::vector<double> &color_rgb_values,
                              const std::shared_ptr<GR::Context> &extContext)
{
  auto useContext = (extContext == nullptr) ? context : extContext;
  element->setAttribute("setNextColor", true);
  if (!color_rgb_values.empty())
    {
      (*useContext)[color_rgb_values_key] = color_rgb_values;
      element->setAttribute("color_rgb_values", color_rgb_values_key);
    }
}

void GR::Render::setNextColor(const std::shared_ptr<GR::Element> &element, std::optional<std::string> color_indices_key,
                              std::optional<std::string> color_rgb_values_key)
{
  if (color_indices_key != std::nullopt)
    {
      element->setAttribute("color_indices", (*color_indices_key));
      element->setAttribute("setNextColor", true);
    }
  else if (color_rgb_values_key != std::nullopt)
    {
      element->setAttribute("setNextColor", true);
      element->setAttribute("color_rgb_values", (*color_rgb_values_key));
    }
}

void GR::Render::setNextColor(const std::shared_ptr<GR::Element> &element)
{
  element->setAttribute("setNextColor", true);
  element->setAttribute("snc-fallback", true);
}

void GR::Render::setOriginPosition(const std::shared_ptr<GR::Element> &element, std::string x_org_pos,
                                   std::string y_org_pos)
{
  element->setAttribute("x_org_pos", x_org_pos);
  element->setAttribute("y_org_pos", y_org_pos);
}

void GR::Render::setOriginPosition3d(const std::shared_ptr<GR::Element> &element, std::string x_org_pos,
                                     std::string y_org_pos, std::string z_org_pos)
{
  setOriginPosition(element, x_org_pos, y_org_pos);
  element->setAttribute("z_org_pos", z_org_pos);
}

void GR::Render::setImshowInformation(const std::shared_ptr<GR::Element> &element, unsigned int cols, unsigned int rows,
                                      std::string img_data_key, std::optional<std::vector<int>> img_data,
                                      const std::shared_ptr<GR::Context> &extContext)
{
  auto useContext = (extContext == nullptr) ? context : extContext;
  if (img_data != std::nullopt)
    {
      (*useContext)[img_data_key] = *img_data;
    }
  element->setAttribute("image_data_key", img_data_key);
  element->setAttribute("cols", (int)cols);
  element->setAttribute("rows", (int)rows);
  element->setAttribute("imshow-information", true);
}

void GR::Render::setGR3LightParameters(const std::shared_ptr<GR::Element> &element, double ambient, double diffuse,
                                       double specular, double specular_power)
{
  element->setAttribute("ambient", ambient);
  element->setAttribute("diffuse", diffuse);
  element->setAttribute("specular", specular);
  element->setAttribute("specular_power", specular_power);
}

//! Render functions
static void renderHelper(const std::shared_ptr<GR::Element> &element, const std::shared_ptr<GR::Context> &context)
{
  /*!
   * Recursive helper function for render; Not part of render class
   * Only renders / processes children if the parent is in parentTypes (group etc.)
   * Used for traverstd::sing the tree
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

static void initializeGridElements(const std::shared_ptr<GR::Element> &element, grm::Grid *grid)
{
  if (element->hasChildNodes())
    {
      for (const auto &child : element->children())
        {
          if (child->localName() != "layout-gridelement" && child->localName() != "layout-grid")
            {
              return;
            }

          double absHeight = static_cast<double>(child->getAttribute("absHeight"));
          double absWidth = static_cast<double>(child->getAttribute("absWidth"));
          int absHeightPxl = static_cast<int>(child->getAttribute("absHeightPxl"));
          int absWidthPxl = static_cast<int>(child->getAttribute("absWidthPxl"));
          int fitParentsHeight = static_cast<int>(child->getAttribute("fitParentsHeight"));
          int fitParentsWidth = static_cast<int>(child->getAttribute("fitParentsWidth"));
          double relativeHeight = static_cast<double>(child->getAttribute("relativeHeight"));
          double relativeWidth = static_cast<double>(child->getAttribute("relativeWidth"));
          double aspectRatio = static_cast<double>(child->getAttribute("aspectRatio"));
          int rowStart = static_cast<int>(child->getAttribute("rowStart"));
          int rowStop = static_cast<int>(child->getAttribute("rowStop"));
          int colStart = static_cast<int>(child->getAttribute("colStart"));
          int colStop = static_cast<int>(child->getAttribute("colStop"));
          grm::Slice *slice = new grm::Slice(rowStart, rowStop, colStart, colStop);

          if (child->localName() == "layout-gridelement")
            {
              grm::GridElement *curGridElement =
                  new grm::GridElement(absHeight, absWidth, absHeightPxl, absWidthPxl, fitParentsHeight,
                                       fitParentsWidth, relativeHeight, relativeWidth, aspectRatio);
              curGridElement->elementInDOM = child;
              grid->setElement(slice, curGridElement);
            }

          if (child->localName() == "layout-grid")
            {
              int nrows = static_cast<int>(child->getAttribute("nrows"));
              int ncols = static_cast<int>(child->getAttribute("ncols"));

              grm::Grid *curGrid =
                  new grm::Grid(nrows, ncols, absHeight, absWidth, absHeightPxl, absWidthPxl, fitParentsHeight,
                                fitParentsWidth, relativeHeight, relativeWidth, aspectRatio);
              curGrid->elementInDOM = child;
              grid->setElement(slice, curGrid);
              initializeGridElements(child, curGrid);
            }
        }
    }
}

static void finalizeGrid(const std::shared_ptr<GR::Element> &root)
{
  grm::Grid *rootGrid = nullptr;
  if (root->hasChildNodes())
    {
      for (const auto &child : root->children())
        {
          if (child->localName() == "layout-grid")
            {
              int nrows = static_cast<int>(child->getAttribute("nrows"));
              int ncols = static_cast<int>(child->getAttribute("ncols"));
              rootGrid = new grm::Grid(nrows, ncols);
              child->setAttribute("subplot", true);
              child->setAttribute("subplot_xmin", 0);
              child->setAttribute("subplot_xmax", 1);
              child->setAttribute("subplot_ymin", 0);
              child->setAttribute("subplot_ymax", 1);

              initializeGridElements(child, rootGrid);
              rootGrid->finalizeSubplot();
              break;
            }
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
  global_root = root;

  global_render = (std::dynamic_pointer_cast<GR::Render>(root->ownerDocument()))
                      ? std::dynamic_pointer_cast<GR::Render>(root->ownerDocument())
                      : GR::Render::createRender();
  std::cout << toXML(root) << "\n";
  if (root->hasChildNodes())
    {
      finalizeGrid(root);
      int i = 0;
      for (const auto &child : root->children())
        {
          gr_savestate();
          ::renderHelper(child, this->context);
          gr_restorestate();
        }
    }
  std::cout << toXML(root) << "\n";
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

std::shared_ptr<GR::Context> GR::Render::getContext()
{
  return context;
}
