#ifdef _WIN32
/*
 * Headers on Windows can define `min` and `max` as macros which causes
 * problem when using `std::min` and `std::max`
 * -> Define `NOMINMAX` to prevent the definition of these macros
 */
#define NOMINMAX
#endif

#define _USE_MATH_DEFINES


#include <functional>
#include <vector>
#include <array>
#include <set>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <cfloat>
#include <climits>
#include <grm/dom_render/graphics_tree/Element.hxx>
#include <grm/dom_render/graphics_tree/Document.hxx>
#include <grm/dom_render/graphics_tree/Value.hxx>
#include <grm/dom_render/graphics_tree/util.hxx>
#include <grm/dom_render/render.hxx>
#include <grm/dom_render/NotFoundError.hxx>
#include <grm/dom_render/GRMaxContextReachedError.hxx>
#include <grm/dom_render/ParentHasToBeProcessedBeforeChildError.hxx>
#include <grm/dom_render/context.hxx>
#include "gks.h"
#include "gr.h"
#include "gr3.h"
#include "grm/layout.hxx"
#include "grm/plot_int.h"
#include <cm.h>

/* ------------------------- re-implementation of x_lin/x_log ------------------------------------------------------- */

#define X_FLIP_IF(x, scale_options, xmin, xmax) \
  (GR_OPTION_FLIP_X & scale_options ? xmin + xmax : 0) + (GR_OPTION_FLIP_X & scale_options ? -1 : 1) * x

#define X_LIN(x, scale_options, xmin, xmax, a, b) \
  X_FLIP_IF((GR_OPTION_X_LOG & scale_options ? (x > 0 ? a * log10(x) + b : -FLT_MAX) : x), scale_options, xmin, xmax)

#define X_LOG(x, scale_options, xmin, xmax, a, b)                                                             \
  (GR_OPTION_X_LOG & scale_options ? (pow(10.0, (double)((X_FLIP_IF(x, scale_options, xmin, xmax) - b) / a))) \
                                   : X_FLIP_IF(x, scale_options, xmin, xmax))

std::shared_ptr<GRM::Element> global_root;
std::shared_ptr<GRM::Render> global_render;
std::priority_queue<std::shared_ptr<GRM::Element>, std::vector<std::shared_ptr<GRM::Element>>, CompareZIndex> z_queue;
bool z_queue_is_being_rendered = false;
int current_z_index = 0;
ManageGRContextIds grContextIDManager;

//! This vector is used for storing element types which children get processed. Other types' children will be ignored
static std::set<std::string> parentTypes = {
    "group",    "layout_grid", "layout_gridelement", "polar_axes", "legend", "figure", "hexbin",
    "colorbar", "plot",        "coordinate_system",  "series",     "axes",   "label",  "errorbars",
};

static std::map<std::string, double> symbol_to_meters_per_unit{
    {"m", 1.0},     {"dm", 0.1},    {"cm", 0.01},  {"mm", 0.001},        {"in", 0.0254},
    {"\"", 0.0254}, {"ft", 0.3048}, {"'", 0.0254}, {"pc", 0.0254 / 6.0}, {"pt", 0.0254 / 72.0},
};

static int bounding_id = 0;
static std::map<int, std::shared_ptr<GRM::Element>> bounding_map;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ utility functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void ManageGRContextIds::destroyGRContexts()
{
  for (int id = 1; id <= no_currently_allocated_gr_contexts; ++id)
    {
      gr_destroycontext(id);
    }
  available_gr_context_ids = {};
  no_currently_allocated_gr_contexts = 0;
}

int ManageGRContextIds::getUnusedGRContextId()
{
  if (available_gr_context_ids.empty())
    {
      if (no_currently_allocated_gr_contexts + 1 > GR_MAX_CONTEXT)
        {
          throw GRMaxContextReachedError("Internal Error: All gr context ids are being used");
        }
      available_gr_context_ids.push(++no_currently_allocated_gr_contexts);
    }
  int context_id = available_gr_context_ids.front();
  available_gr_context_ids.pop();
  return context_id;
}

void ManageGRContextIds::markIdAsUnused(int id)
{
  if (id <= no_currently_allocated_gr_contexts)
    {
      available_gr_context_ids.push(id);
    }
}

bool CompareZIndex::operator()(std::shared_ptr<GRM::Element> const &lhs, std::shared_ptr<GRM::Element> const &rhs)
{
  int lhs_z = 0, rhs_z = 0;
  if (lhs->hasAttribute("z_index"))
    {
      lhs_z = static_cast<int>(lhs->getAttribute("z_index"));
    }
  if (rhs->hasAttribute("z_index"))
    {
      rhs_z = static_cast<int>(rhs->getAttribute("z_index"));
    }
  return lhs_z > rhs_z;
}

static double auto_tick(double amin, double amax)
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

static double auto_tick_rings_polar(double rmax, int &rings, const std::string &norm)
{
  double scale;
  bool decimal = false;

  std::vector<int> largeRings = {6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
  std::vector<int> normalRings = {3, 4, 5, 6, 7};

  std::vector<int> *whichVector;

  // -1 --> auto rings
  if (rings == -1)
    {
      if (norm == "cdf")
        {
          rings = 4;
          return 1.0 / rings;
        }

      if (rmax > 20)
        {
          whichVector = &largeRings;
        }
      else
        {
          whichVector = &normalRings;
        }
      scale = ceil(abs(log10(rmax)));
      if (rmax < 1.0)
        {
          decimal = true;
          rmax = static_cast<int>(ceil(rmax * pow(10.0, scale)));
        }

      while (true)
        {
          for (int i : *whichVector)
            {
              if (static_cast<int>(rmax) % i == 0)
                {
                  if (decimal)
                    {
                      rmax = rmax / pow(10.0, scale);
                    }
                  rings = i;
                  return rmax / rings;
                }
            }
          // rmax not divisible by whichVector
          ++rmax;
        }
    }

  // given rings
  if (norm == "cdf")
    {
      return 1.0 / rings;
    }

  if (rmax > rings)
    {
      return (static_cast<int>(rmax) + (rings - (static_cast<int>(rmax) % rings))) / rings;
    }
  else if (rmax > (rings * 0.6))
    {
      // returns rings / rings -> 1.0 so that rmax = rings * tick -> rings. Number of rings is rmax then
      return 1.0;
    }
  scale = ceil(abs(log10(rmax)));
  rmax = static_cast<int>(rmax * pow(10.0, scale));
  if (static_cast<int>(rmax) % rings == 0)
    {
      rmax = rmax / pow(10.0, scale);
      return rmax / rings;
    }
  rmax += rings - (static_cast<int>(rmax) % rings);
  rmax = rmax / pow(10.0, scale);

  return rmax / rings;
}

/*!
 * Convert an RGB triple to a luminance value following the CCIR 601 format.
 *
 * \param[in] r The red component of the RGB triple in the range [0.0, 1.0].
 * \param[in] g The green component of the RGB triple in the range [0.0, 1.0].
 * \param[in] b The blue component of the RGB triple in the range [0.0, 1.0].
 * \return The luminance of the given RGB triple in the range [0.0, 1.0].
 */
static double get_lightness_from_rbg(double r, double g, double b)
{
  return 0.299 * r + 0.587 * g + 0.114 * b;
}

/*
 * mixes gr colormaps with size = size * size. If x and or y < 0
 * */
static int *create_colormap(int x, int y, int size)
{
  int r, g, b, a;
  int outer, inner;
  int r1, g1, b1;
  int r2, g2, b2;
  int *colormap = nullptr;
  if (x > 47 || y > 47)
    {
      logger((stderr, "values for the keyword \"colormap\" can not be greater than 47\n"));
      return nullptr;
    }

  colormap = static_cast<int *>(malloc(size * size * sizeof(int)));
  if (colormap == nullptr)
    {
      debug_print_malloc_error();
      return nullptr;
    }
  if (x >= 0 && y < 0)
    {
      for (outer = 0; outer < size; outer++)
        {
          for (inner = 0; inner < size; inner++)
            {
              a = 255;
              r = ((cmap_h[x][(int)(inner * 255.0 / size)] >> 16) & 0xff);
              g = ((cmap_h[x][(int)(inner * 255.0 / size)] >> 8) & 0xff);
              b = (cmap_h[x][(int)(inner * 255.0 / size)] & 0xff);

              colormap[outer * size + inner] = (a << 24) + (b << 16) + (g << 8) + (r);
            }
        }
      return colormap;
    }

  if (x < 0 && y >= 0)
    {
      gr_setcolormap(y);
      for (outer = 0; outer < size; outer++)
        {
          for (inner = 0; inner < size; inner++)
            {
              a = 255;
              r = ((cmap_h[y][(int)(inner * 255.0 / size)] >> 16) & 0xff);
              g = ((cmap_h[y][(int)(inner * 255.0 / size)] >> 8) & 0xff);
              b = (cmap_h[y][(int)(inner * 255.0 / size)] & 0xff);

              colormap[inner * size + outer] = (a << 24) + (b << 16) + (g << 8) + (r);
            }
        }


      return colormap;
    }

  if ((x >= 0 && y >= 0) || (x < 0 && y < 0))
    {
      if (x < 0 && y < 0)
        {
          x = y = 0;
        }
      gr_setcolormap(x);
      for (outer = 0; outer < size; outer++)
        {
          for (inner = 0; inner < size; inner++)
            {
              a = 255;
              r1 = ((cmap_h[x][(int)(inner * 255.0 / size)] >> 16) & 0xff);
              g1 = ((cmap_h[x][(int)(inner * 255.0 / size)] >> 8) & 0xff);
              b1 = (cmap_h[x][(int)(inner * 255.0 / size)] & 0xff);

              r2 = ((cmap_h[y][(int)(outer * 255.0 / size)] >> 16) & 0xff);
              g2 = ((cmap_h[y][(int)(outer * 255.0 / size)] >> 8) & 0xff);
              b2 = (cmap_h[y][(int)(outer * 255.0 / size)] & 0xff);

              colormap[outer * size + inner] =
                  (a << 24) + (((b1 + b2) / 2) << 16) + (((g1 + g2) / 2) << 8) + ((r1 + r2) / 2);
            }
        }
      return colormap;
    }
  return nullptr;
}

/* like python list comprehension [factor * func(element) for element in list] saves values in result starting at start
 * index */
static double *listcomprehension(double factor, double (*pFunction)(double), double *list, int num, int start,
                                 double *result)
{
  int i;
  if (result == nullptr)
    {
      result = static_cast<double *>(malloc(num * sizeof(double)));
      if (result == nullptr)
        {
          return nullptr;
        }
    }

  for (i = 0; i < num; ++i)
    {
      result[start] = factor * (*pFunction)(list[i]);
      start++;
    }

  return result;
}

static double *moivre(double r, int x, int n)
{
  double *result = static_cast<double *>(malloc(2 * sizeof(double)));
  if (result != nullptr)
    {
      if (n != 0)
        {
          *result = pow(r, (1.0 / n)) * (cos(2.0 * x * M_PI / n));
          *(result + 1) = pow(r, (1.0 / n)) * (sin(2.0 * x * M_PI / n));
        }
      else
        {
          *result = 1.0;
          *(result + 1) = 0.0;
        }
    }
  return result;
}

static void markerHelper(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context,
                         const std::string &str)
{
  /*!
   * Helperfunction for marker functions using vectors for marker parameters
   *
   * \param[in] element The GRM::Element that contains all marker attributes and data keys. If element's parent is a
   * group element it may fallback to its marker attributes
   * \param[in] context The GRM::Context that contains the actual data
   * \param[in] str The std::string that specifies what GRM Routine should be called (polymarker)
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
      type = GRM::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
    }
  else if (group)
    {
      attr = parent->getAttribute("markertypes");
      if (attr.isString())
        {
          type = GRM::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
        }
    }

  attr = element->getAttribute("markercolorinds");
  if (attr.isString())
    {
      colorind = GRM::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
    }
  else if (group)
    {
      attr = parent->getAttribute("markercolorinds");
      if (attr.isString())
        {
          colorind = GRM::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
        }
    }

  attr = element->getAttribute("markersizes");
  if (attr.isString())
    {
      size = GRM::get<std::vector<double>>((*context)[static_cast<std::string>(attr)]);
    }
  else if (group)
    {
      attr = parent->getAttribute("markersizes");
      if (attr.isString())
        {
          size = GRM::get<std::vector<double>>((*context)[static_cast<std::string>(attr)]);
        }
    }

  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));
  std::string z;
  if (element->hasAttribute("z"))
    {
      z = static_cast<std::string>(element->getAttribute("z"));
    }

  std::vector<double> x_vec = GRM::get<std::vector<double>>((*context)[x]);
  std::vector<double> y_vec = GRM::get<std::vector<double>>((*context)[y]);
  std::vector<double> z_vec;
  if (auto z_ptr = GRM::get_if<std::vector<double>>((*context)[z]))
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

static void lineHelper(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context,
                       const std::string &str)
{
  /*!
   * Helperfunction for line functions using vectors for line parameters
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   * \param[in] str The std::string that specifies what GRM Routine should be called (polyline)
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
      type = GRM::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
    }
  else if (group)
    {
      attr = parent->getAttribute("linetypes");
      if (attr.isString())
        {
          type = GRM::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
        }
    }

  attr = element->getAttribute("linecolorinds");
  if (attr.isString())
    {
      colorind = GRM::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
    }
  else if (group)
    {
      attr = parent->getAttribute("linecolorinds");
      if (attr.isString())
        {
          colorind = GRM::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
        }
    }

  attr = element->getAttribute("linewidths");
  if (attr.isString())
    {
      width = GRM::get<std::vector<double>>((*context)[static_cast<std::string>(attr)]);
    }
  else if (group)
    {
      attr = parent->getAttribute("linewidths");
      if (attr.isString())
        {
          width = GRM::get<std::vector<double>>((*context)[static_cast<std::string>(attr)]);
        }
    }

  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));
  std::string z;
  if (element->hasAttribute("z"))
    {
      z = static_cast<std::string>(element->getAttribute("z"));
    }

  std::vector<double> x_vec = GRM::get<std::vector<double>>((*context)[x]);
  std::vector<double> y_vec = GRM::get<std::vector<double>>((*context)[y]);
  std::vector<double> z_vec;

  if (auto z_ptr = GRM::get_if<std::vector<double>>((*context)[z]))
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

static std::shared_ptr<GRM::Element> getSubplotElement(const std::shared_ptr<GRM::Element> &element)
{
  auto ancestor = element;

  while (ancestor->localName() != "figure")
    {
      bool ancestorIsSubplotGroup =
          (ancestor->hasAttribute("subplotGroup") && static_cast<int>(ancestor->getAttribute("subplotGroup")));
      if (ancestor->localName() == "layout_gridelement" || ancestorIsSubplotGroup)
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

static void getTickSize(const std::shared_ptr<GRM::Element> &element, double &tick_size)
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

static void getMajorCount(const std::shared_ptr<GRM::Element> &element, const std::string kind, int &major_count)
{
  if (element->hasAttribute("major"))
    {
      major_count = static_cast<int>(element->getAttribute("major"));
    }
  else
    {
      if (str_equals_any(kind.c_str(), 9, "wireframe", "surface", "plot3", "scatter3", "polar", "trisurf",
                         "polar_heatmap", "nonuniformpolar_heatmap", "volume"))
        {
          major_count = 2;
        }
      else
        {
          major_count = 5;
        }
    }
}

static void getAxesInformation(const std::shared_ptr<GRM::Element> &element, std::string x_org_pos,
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

static void getAxes3dInformation(const std::shared_ptr<GRM::Element> &element, std::string x_org_pos,
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
                     const std::shared_ptr<GRM::Element> &element, std::shared_ptr<GRM::Render> global_render)
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
              /* part is too big but doesn't have a breakpoint in it */
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

static void legend_size(const std::shared_ptr<GRM::Element> &elem, double *w, double *h)
{
  double tbx[4], tby[4];
  int labelsExist = 1;
  unsigned int num_labels;
  std::vector<std::string> labels;
  *w = 0;
  *h = 0;

  if (auto render = std::dynamic_pointer_cast<GRM::Render>(elem->ownerDocument()))
    {
      auto context = render->getContext();
      std::string key = static_cast<std::string>(elem->getAttribute("labels"));
      labels = GRM::get<std::vector<std::string>>((*context)[key]);
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
  std::array<std::string, 2> vars = {"x", "y"};
  std::array<double, 2> default_size = {PLOT_DEFAULT_WIDTH, PLOT_DEFAULT_HEIGHT};

  std::shared_ptr<GRM::Element> root = global_root;

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
          size_unit = (std::string)root->getAttribute("size_" + vars[i] + "_unit");
          size_type = (std::string)root->getAttribute("size_" + vars[i] + "_type");
          if (size_unit.empty()) size_unit = "px";
          tmp_size_d[i] = default_size[i];

          if (size_type == "double" || size_type == "int")
            {
              tmp_size_d[i] = (double)root->getAttribute("size_" + vars[i]);
              auto meters_per_unit_iter = symbol_to_meters_per_unit.find(size_unit);
              if (meters_per_unit_iter != symbol_to_meters_per_unit.end())
                {
                  double meters_per_unit = meters_per_unit_iter->second;
                  double pixels_per_unit = meters_per_unit * dpm[i];

                  tmp_size_d[i] *= pixels_per_unit;
                }
            }
          pixel_size[i] = (int)grm_round(tmp_size_d[i]);
          metric_size[i] = tmp_size_d[i] / dpm[i];
        }
    }
  else
    {
      pixel_size[0] = (int)grm_round(PLOT_DEFAULT_WIDTH);
      pixel_size[1] = (int)grm_round(PLOT_DEFAULT_HEIGHT);
      metric_size[0] = PLOT_DEFAULT_WIDTH / dpm[0];
      metric_size[1] = PLOT_DEFAULT_HEIGHT / dpm[1];
    }

  if (pixel_width != nullptr)
    {
      *pixel_width = pixel_size[0];
    }
  if (pixel_height != nullptr)
    {
      *pixel_height = pixel_size[1];
    }
  if (metric_width != nullptr)
    {
      *metric_width = metric_size[0];
    }
  if (metric_height != nullptr)
    {
      *metric_height = metric_size[1];
    }
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

static void setNextColor(gr_color_type_t color_type, std::vector<int> &color_indices,
                         std::vector<double> &color_rgb_values, const std::shared_ptr<GRM::Element> &elem)
{
  // TODO: is this method really needed? Cant it be replaced by set_next_color?
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

void receiverfunction(int id, double x_min, double x_max, double y_min, double y_max)
{
  if (!(x_min == DBL_MAX || x_max == -DBL_MAX || y_min == DBL_MAX || y_max == -DBL_MAX))
    {
      bounding_map[id]->setAttribute("bbox_id", id);
      bounding_map[id]->setAttribute("bbox_xmin", x_min);
      bounding_map[id]->setAttribute("bbox_xmax", x_max);
      bounding_map[id]->setAttribute("bbox_ymin", y_min);
      bounding_map[id]->setAttribute("bbox_ymax", y_max);
    }
}

static bool getLimitsForColorbar(const std::shared_ptr<GRM::Element> &element, double &c_min, double &c_max)
{
  bool limits_found = true;
  if (element->hasAttribute("lim_cmin") && element->hasAttribute("lim_cmax"))
    {
      c_min = static_cast<double>(element->getAttribute("lim_cmin"));
      c_max = static_cast<double>(element->getAttribute("lim_cmax"));
    }
  else if (element->hasAttribute("lim_zmin") && element->hasAttribute("lim_zmax"))
    {
      c_min = static_cast<double>(element->getAttribute("lim_zmin"));
      c_max = static_cast<double>(element->getAttribute("lim_zmax"));
    }
  else
    {
      limits_found = false;
    }

  return limits_found;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ attribute processing functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static void processAlpha(const std::shared_ptr<GRM::Element> &elem)
{
  gr_settransparency(static_cast<double>(elem->getAttribute("alpha")));
}

static void processBorderColorInd(const std::shared_ptr<GRM::Element> &elem)
{
  gr_setbordercolorind(static_cast<int>(elem->getAttribute("bordercolorind")));
}

static void processCalcWindowAndViewportFromParent(const std::shared_ptr<GRM::Element> &elem)
{
  std::string kind = static_cast<std::string>(elem->getAttribute("kind"));
  double viewport[4];
  double x_min, x_max, y_min, y_max, c_min, c_max;

  if (elem->parentElement()->hasAttribute("marginalheatmap_kind"))
    {
      std::string orientation = static_cast<std::string>(elem->getAttribute("orientation"));
      auto plot_group = elem->parentElement()->parentElement();
      viewport[0] = static_cast<double>(plot_group->getAttribute("viewport_xmin"));
      viewport[1] = static_cast<double>(plot_group->getAttribute("viewport_xmax"));
      viewport[2] = static_cast<double>(plot_group->getAttribute("viewport_ymin"));
      viewport[3] = static_cast<double>(plot_group->getAttribute("viewport_ymax"));
      x_min = static_cast<double>(plot_group->getAttribute("lim_xmin"));
      x_max = static_cast<double>(plot_group->getAttribute("lim_xmax"));
      y_min = static_cast<double>(plot_group->getAttribute("lim_ymin"));
      y_max = static_cast<double>(plot_group->getAttribute("lim_ymax"));
      if (plot_group->hasAttribute("lim_cmin"))
        {
          c_min = static_cast<double>(plot_group->getAttribute("lim_cmin"));
        }
      else
        {
          c_min = static_cast<double>(plot_group->getAttribute("lim_zmin"));
        }
      if (plot_group->hasAttribute("lim_cmax"))
        {
          c_max = static_cast<double>(plot_group->getAttribute("lim_cmax"));
        }
      else
        {
          c_max = static_cast<double>(plot_group->getAttribute("lim_zmax"));
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

static void processCharExpan(const std::shared_ptr<GRM::Element> &elem)
{
  gr_setcharexpan(static_cast<double>(elem->getAttribute("charexpan")));
}

static void processCharHeight(const std::shared_ptr<GRM::Element> &elem)
{
  gr_setcharheight(static_cast<double>(elem->getAttribute("charheight")));
}

static void processCharSpace(const std::shared_ptr<GRM::Element> &elem)
{
  gr_setcharspace(static_cast<double>(elem->getAttribute("charspace")));
}

static void processCharUp(const std::shared_ptr<GRM::Element> &elem)
{
  gr_setcharup(static_cast<double>(elem->getAttribute("charup_ux")),
               static_cast<double>(elem->getAttribute("charup_uy")));
}

static void processClipXForm(const std::shared_ptr<GRM::Element> &elem)
{
  gr_selectclipxform(static_cast<double>(elem->getAttribute("clipxform")));
}

static void processColorbarPosition(const std::shared_ptr<GRM::Element> &elem)
{
  double viewport[4];

  auto subplot_element = getSubplotElement(elem);

  double width = static_cast<double>(elem->getAttribute("width"));
  double offset = static_cast<double>(elem->getAttribute("offset"));

  if (!subplot_element->hasAttribute("viewport"))
    {
      throw NotFoundError("Missing viewport\n");
    }

  viewport[0] = static_cast<double>(subplot_element->getAttribute("viewport_xmin"));
  viewport[1] = static_cast<double>(subplot_element->getAttribute("viewport_xmax"));
  viewport[2] = static_cast<double>(subplot_element->getAttribute("viewport_ymin"));
  viewport[3] = static_cast<double>(subplot_element->getAttribute("viewport_ymax"));

  gr_setviewport(viewport[1] + offset, viewport[1] + offset + width, viewport[2], viewport[3]);
}

static void processColormap(const std::shared_ptr<GRM::Element> &elem)
{
  int colormap;

  gr_setcolormap(static_cast<int>(elem->getAttribute("colormap")));
  /* TODO: Implement other datatypes for `colormap` */
}

static void processColorRep(const std::shared_ptr<GRM::Element> &elem)
{
  int index, hex_int;
  double red, green, blue;
  std::stringstream stringstream;
  std::string name, hex_string;

  for (auto &attr : elem->getAttributeNames())
    {
      auto start = 0U;
      auto end = attr.find('$');
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

static void processFillColorInd(const std::shared_ptr<GRM::Element> &elem)
{
  gr_setfillcolorind(static_cast<int>(elem->getAttribute("fillcolorind")));
}

static void processFillIntStyle(const std::shared_ptr<GRM::Element> &elem)
{
  gr_setfillintstyle(static_cast<int>(elem->getAttribute("fillintstyle")));
}

static void processFillStyle(const std::shared_ptr<GRM::Element> &elem)
{
  gr_setfillstyle(static_cast<int>(elem->getAttribute("fillstyle")));
}

static void processFlip(const std::shared_ptr<GRM::Element> &elem)
{
  int options;
  int xflip = static_cast<int>(elem->getAttribute("xflip"));
  int yflip = static_cast<int>(elem->getAttribute("yflip"));
  gr_inqscale(&options);

  if (xflip)
    {
      options = (options | GR_OPTION_FLIP_Y) & ~GR_OPTION_FLIP_X;
    }
  else if (yflip)
    {
      options = options & ~GR_OPTION_FLIP_Y & ~GR_OPTION_FLIP_X;
    }
  else
    {
      options = options & ~GR_OPTION_FLIP_X;
    }
}

static void processFont(const std::shared_ptr<GRM::Element> &elem)
{
  int font, font_precision;

  /* `font` and `font_precision` are always set */
  if (elem->hasAttribute("font_precision"))
    {
      font = static_cast<int>(elem->getAttribute("font"));
      font_precision = static_cast<int>(elem->getAttribute("font_precision"));
      logger((stderr, "Using font: %d with precision %d\n", font, font_precision));
      gr_settextfontprec(font, font_precision);
    }
  /* TODO: Implement other datatypes for `font` and `font_precision` */
}

static void processGROptionFlipX(const std::shared_ptr<GRM::Element> &elem)
{
  int options;
  int flip_x = static_cast<int>(elem->getAttribute("gr_option_flip_x"));
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

static void processGROptionFlipY(const std::shared_ptr<GRM::Element> &elem)
{
  int options;
  int flip_y = static_cast<int>(elem->getAttribute("gr_option_flip_y"));
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

static void processGR3BackgroundColor(const std::shared_ptr<GRM::Element> &elem)
{
  double r, g, b, a;
  r = (double)elem->getAttribute("gr3backgroundcolor_red");
  g = (double)elem->getAttribute("gr3backgroundcolor_green");
  b = (double)elem->getAttribute("gr3backgroundcolor_blue");
  a = (double)elem->getAttribute("gr3backgroundcolor_alpha");

  gr3_setbackgroundcolor(r, g, b, a);
}

static void processGR3CameraLookAt(const std::shared_ptr<GRM::Element> &elem)
{
  double camera_x, camera_y, camera_z, center_x, center_y, center_z, up_x, up_y, up_z;

  camera_x = (double)elem->getAttribute("gr3cameralookat_camera_x");
  camera_y = (double)elem->getAttribute("gr3cameralookat_camera_y");
  camera_z = (double)elem->getAttribute("gr3cameralookat_camera_z");
  center_x = (double)elem->getAttribute("gr3cameralookat_center_x");
  center_y = (double)elem->getAttribute("gr3cameralookat_center_y");
  center_z = (double)elem->getAttribute("gr3cameralookat_center_z");
  up_x = (double)elem->getAttribute("gr3cameralookat_up_x");
  up_y = (double)elem->getAttribute("gr3cameralookat_up_y");
  up_z = (double)elem->getAttribute("gr3cameralookat_up_z");

  gr3_cameralookat(camera_x, camera_y, camera_z, center_x, center_y, center_z, up_x, up_y, up_z);
}

static void polarHistogram(const std::shared_ptr<GRM::Element> &element)
{
  unsigned int num_bins;
  double *classes = nullptr;
  unsigned int length;
  double max;
  double *inner = nullptr, *outer = nullptr;
  double r;
  double rect;
  double *liste = nullptr;
  double liste0;
  double liste1;
  double *liste2 = nullptr;
  double *mlist = nullptr;
  double *rectlist = nullptr;
  const char *norm = nullptr;
  double bin_width = -1.0;
  double *bin_edges = nullptr;
  unsigned int num_bin_edges;
  double *bin_widths = nullptr;
  double *philim = nullptr;
  double *rlim = nullptr;
  unsigned int dummy;
  double *r_min_list = nullptr;
  double *r_min_list2 = nullptr;
  int stairs;
  double r_min = 0.0;
  double r_max = 1.0;
  double *phi_array = nullptr;
  double *arc_2_x = nullptr;
  double *arc_2_y = nullptr;
  int xcolormap;
  int ycolormap;
  int *colormap = nullptr;
  double *angles = nullptr;
  int draw_edges = 0;
  int phiflip = 0;
  int x;
  const double convert = 180 / M_PI;
  int edge_color = 1;
  int face_color = 989;
  double face_alpha = 0.75;
  unsigned int resample = 0;
  int *lineardata = nullptr;
  int *bin_counts = nullptr;
  double *f1 = nullptr;
  double *f2 = nullptr;
  int freeable_bin_widths = 0;
  int freeable_bin_edges = 0;
  int freeable_angles = 0;
  err_t error = ERROR_NONE;

  std::vector<double> bin_edges_vec;
  std::vector<double> bin_widths_vec;
  std::vector<double> r_lim_vec;

  std::shared_ptr<GRM::Element> group = element;
  std::shared_ptr<GRM::Context> context;

  if (auto render = std::dynamic_pointer_cast<GRM::Render>(element->ownerDocument()))
    {
      context = render->getContext();

      std::cout << toXML(render->firstChildElement()) << "\n";
    }
  else
    {
      throw NotFoundError("Render-document not found for element\n");
    }

  std::shared_ptr<GRM::Element> temp_elem;
  std::string str;

  resample = static_cast<int>(group->getAttribute("original_resample"));

  global_render->setResampleMethod(group, 0x2020202);

  auto classes_key = static_cast<std::string>(group->getAttribute("classes"));
  auto classes_vec = GRM::get<std::vector<double>>((*context)[classes_key]);
  classes = &(classes_vec[0]);
  length = classes_vec.size();

  /* edge_color */
  if (group->hasAttribute("edge_color") == 0)
    {
      edge_color = 1;
    }
  else
    {
      edge_color = static_cast<int>(group->getAttribute("edge_color"));
    }

  /* face_color */
  if (group->hasAttribute("face_color") == 0)
    {
      face_color = 989;
    }
  else
    {
      face_color = static_cast<int>(group->getAttribute("face_color"));
    }

  /* face_alpha */
  if (group->hasAttribute("face_alpha") == 0)
    {
      face_alpha = 0.75;
    }
  else
    {
      face_alpha = static_cast<double>(group->getAttribute("face_alpha"));
    }

  global_render->setTransparency(group, face_alpha);

  num_bins = static_cast<int>(group->getAttribute("nbins"));

  if (group->hasAttribute("normalization") == 0)
    {
      norm = "count";
    }
  else
    {
      norm = static_cast<std::string>(group->getAttribute("normalization")).c_str();
    }

  max = static_cast<double>(group->getAttribute("r_max"));

  int rings = -1;

  double tick = auto_tick_rings_polar(max, rings, norm);
  group->setAttribute("tick", tick);
  max = tick * rings;
  group->setAttribute("r_max", max);
  group->setAttribute("rings", rings);

  if (group->hasAttribute("phiflip") == 0)
    {
      phiflip = 0;
    }
  else
    {
      phiflip = static_cast<int>(group->getAttribute("phiflip"));
    }

  if (group->hasAttribute("draw_edges") == 0)
    {
      draw_edges = 0;
    }
  else
    {
      draw_edges = static_cast<int>(group->getAttribute("draw_edges"));
    }

  if (group->hasAttribute("bin_edges") == 0)
    {
      bin_edges = nullptr;
      num_bin_edges = 0;
      if (group->hasAttribute("bin_width"))
        {
          bin_width = static_cast<double>(group->getAttribute("bin_width"));
        }
    }
  else
    {
      auto bin_edges_key = static_cast<std::string>(group->getAttribute("bin_edges"));
      bin_edges_vec = GRM::get<std::vector<double>>((*context)[bin_edges_key]);
      bin_edges = &(bin_edges_vec[0]);
      num_bin_edges = bin_edges_vec.size();

      auto bin_widths_key = static_cast<std::string>(group->getAttribute("bin_widths"));
      bin_widths_vec = GRM::get<std::vector<double>>((*context)[bin_widths_key]);
      bin_widths = &(bin_widths_vec[0]);
      num_bins = bin_widths_vec.size();
    }

  if (group->hasAttribute("stairs") == 0)
    {
      stairs = 0;
    }
  else
    {
      stairs = static_cast<int>(group->getAttribute("stairs"));
      if (stairs)
        {
          if (draw_edges != 0)
            {
              logger((stderr, "stairs is not compatible with draw_edges / colormap\n"));
              cleanup_and_set_error(ERROR_PLOT_INCOMPATIBLE_ARGUMENTS);
            }
          /* no bin_edges */
          else if (num_bin_edges == 0)
            {
              mlist = static_cast<double *>(malloc(num_bins * 4 * sizeof(double)));
              cleanup_and_set_error_if(mlist == nullptr, ERROR_MALLOC);
              if (stairs != 0)
                {
                  stairs = 1;
                }
            }
          else
            {
              rectlist = static_cast<double *>(malloc(num_bins * sizeof(double)));
              cleanup_and_set_error_if(rectlist == nullptr, ERROR_MALLOC);
            }
        }
    }

  if (group->hasAttribute("rlim") == 0)
    {
      rlim = nullptr;
    }
  else
    {
      r_lim_vec.push_back(static_cast<double>(group->getAttribute("rlim0")));
      r_lim_vec.push_back(static_cast<double>(group->getAttribute("rlim1")));
      rlim = &(r_lim_vec[0]);

      /* TODO: Potential memory leak, s. `malloc` in line 3788 */
      mlist = static_cast<double *>(malloc((num_bins + 1) * 4 * sizeof(double)));
      cleanup_and_set_error_if(mlist == nullptr, ERROR_MALLOC);
      if (rlim[0] > rlim[1])
        {
          r_min = rlim[1];
          r_max = rlim[0];
          rlim[0] = r_min;
          rlim[1] = r_max;
        }
      else
        {
          r_min = rlim[0];
          r_max = rlim[1];
        }

      if (r_max > 1.0)
        {
          r_max = 1.0;
          logger((stderr, "the max value of rlim can not exceed 1.0\n"));
          cleanup_and_set_error(ERROR_PLOT_OUT_OF_RANGE);
        }
      if (r_min < 0.0) r_min = 0.0;
    }

  length /= num_bins;
  outer = classes;
  if (phiflip != 0)
    {
      outer += (num_bins - 1) * length;
    }

  if (!(group->hasAttribute("xcolormap") && group->hasAttribute("ycolormap")))
    {
      colormap = nullptr;
      if (draw_edges != 0)
        {
          logger((stderr, "draw_edges can only be used with colormap\n"));
          cleanup_and_set_error(ERROR_PLOT_INCOMPATIBLE_ARGUMENTS);
        }
    }
  else
    {
      xcolormap = static_cast<int>(group->getAttribute("xcolormap"));
      ycolormap = static_cast<int>(group->getAttribute("ycolormap"));

      if (-1 > xcolormap || xcolormap > 47 || ycolormap < -1 || ycolormap > 47)
        {
          logger((stderr, "the value for keyword \"colormap\" must contain two integer between -1 and 47\n"));
          cleanup_and_set_error(ERROR_PLOT_OUT_OF_RANGE);
        }
      else
        {
          int colormap_size = 500;
          int image_size = 2000;
          int y, center, center_x, center_y;
          double radius, angle;
          int temp1, temp2;
          int temp = 0;
          int total = 0;
          double norm_factor = 1;
          int count = 0;
          double max_radius;

          lineardata = static_cast<int *>(calloc(image_size * image_size, sizeof(int)));
          cleanup_and_set_error_if(lineardata == nullptr, ERROR_MALLOC);

          bin_counts = static_cast<int *>(malloc(num_bins * sizeof(int)));
          cleanup_and_set_error_if(bin_counts == nullptr, ERROR_MALLOC);

          colormap = create_colormap(xcolormap, ycolormap, colormap_size);
          cleanup_and_set_error_if(colormap == nullptr, ERROR_PLOT_COLORMAP);

          if (num_bin_edges == 0)
            {
              angles = static_cast<double *>(malloc((num_bins + 1) * sizeof(double)));
              cleanup_and_set_error_if(angles == nullptr, ERROR_MALLOC);
              freeable_angles = 1;
              linspace(0, M_PI * 2, num_bins + 1, angles);
            }
          else
            {
              angles = bin_edges;
            }

          outer = classes;

          center_x = image_size / 2;
          center_y = image_size / 2;
          center = image_size / 2;

          max_radius = center;

          for (temp1 = 0; temp1 < num_bins; temp1++)
            {
              temp = 0;
              for (temp2 = 0; temp2 < length; temp2++)
                {
                  if (classes[temp1 * length + temp2] == -1) break;
                  ++temp;
                }
              bin_counts[temp1] = temp;
            }

          if (str_equals_any(norm, 2, "probability", "pdf"))
            {
              for (temp1 = 0; temp1 < num_bins; ++temp1)
                {
                  total += bin_counts[temp1];
                }
            }
          else if (strcmp(norm, "cdf") == 0)
            total = bin_counts[num_bins - 1];

          if (str_equals_any(norm, 2, "probability", "cdf"))
            norm_factor = total;
          else if (num_bin_edges == 0 && strcmp(norm, "pdf") == 0)
            norm_factor = total * bin_width;
          else if (num_bin_edges == 0 && strcmp(norm, "countdensity") == 0)
            norm_factor = bin_width;

          if (rlim != nullptr)
            {
              r_min *= max_radius;
              r_max *= max_radius;
            }
          else
            {
              r_min = 0.0;
              r_max = max_radius;
            }

          for (y = 0; y < image_size; y++)
            {
              for (x = 0; x < image_size; x++)
                {
                  int q;
                  radius = sqrt(pow(x - center_x, 2) + pow(y - center_y, 2));
                  angle = atan2(y - center_y, x - center_x);

                  if (angle < 0) angle += M_PI * 2;
                  if (phiflip == 0) angle = 2 * M_PI - angle;

                  for (q = 0; q < num_bins; ++q)
                    {
                      if (angle > angles[q] && angle <= angles[q + 1])
                        {
                          count = bin_counts[q];
                          if (strcmp(norm, "pdf") == 0 && num_bin_edges > 0)
                            {
                              norm_factor = total * bin_widths[q];
                            }
                          else if (strcmp(norm, "countdensity") == 0 && num_bin_edges > 0)
                            {
                              norm_factor = bin_widths[q];
                            }

                          if ((grm_round(radius * 100) / 100) <=
                                  (grm_round((count * 1.0 / norm_factor / max * center) * 100) / 100) &&
                              radius <= r_max && radius > r_min)
                            {
                              lineardata[y * image_size + x] = colormap
                                  [(int)(radius / (center * pow(2, 0.5)) * (colormap_size - 1)) * colormap_size +
                                   grm_max(grm_min((int)(angle / (2 * M_PI) * colormap_size), colormap_size - 1), 0)];
                            }

                        } /* end angle check */
                    }     /* end for q loop: bin check*/
                }         /* end x loop*/
            }             /* end y loop */
          if (rlim != nullptr)
            {
              r_min = rlim[0];
              r_max = rlim[1];
            }

          int id = (int)global_root->getAttribute("id");
          global_root->setAttribute("id", id + 1);
          str = std::to_string(id);
          std::vector<int> l_vec(lineardata, lineardata + image_size * image_size);

          temp_elem =
              global_render->createDrawImage(-1.0, 1.0, -1.0, 1.0, image_size, image_size, "data" + str, l_vec, 0);
          group->append(temp_elem);
          free(lineardata);
          free(bin_counts);
          lineardata = nullptr;
          bin_counts = nullptr;
        } /* end colormap calculation*/

    } /* end colormap condition */

  outer = classes;
  if (phiflip != 0) outer += (num_bins - 1) * length;

  if (phiflip != 0 && num_bin_edges > 0)
    {
      double *temp = nullptr;
      double *temp2 = nullptr;
      temp = static_cast<double *>(malloc(num_bin_edges * sizeof(double)));
      cleanup_and_set_error_if(temp == nullptr, ERROR_MALLOC);
      temp2 = static_cast<double *>(malloc(num_bins * sizeof(double)));
      cleanup_and_set_error_if(temp2 == nullptr, ERROR_MALLOC);
      int u;
      for (u = 0; u < num_bin_edges; u++)
        {
          temp[u] = 2 * M_PI - bin_edges[num_bin_edges - 1 - u];
        }
      for (u = num_bins - 1; u >= 0; --u)
        {
          temp2[u] = bin_widths[num_bins - 1 - u];
        }
      bin_widths = temp2;
      freeable_bin_widths = 1;
      bin_edges = temp;
      temp = temp2 = nullptr;
      freeable_bin_edges = 1;
    }

  /* no colormap or colormap combined with draw_edges */
  if (colormap == nullptr || draw_edges == 1)
    {
      for (x = 0; x < num_bins; ++x)
        {
          double count;
          int y;

          /*
           * free memory from the previous iteration
           * (end of loop is not possible because of `continue` statements)
           * last iteration memory is freed in the cleanup block
           */
          free(liste);
          liste = nullptr;
          free(liste2);
          liste2 = nullptr;
          free(r_min_list);
          r_min_list = nullptr;
          free(r_min_list2);
          r_min_list2 = nullptr;

          count = 0.0;
          inner = outer;

          if (*inner == -1)
            {
              /* stairs bin_edges / philim  */
              if (rectlist != nullptr && philim != nullptr)
                rectlist[x] = r_min;
              else if (rectlist != nullptr)
                rectlist[x] = 0.0;
            }


          for (y = 0; y < length; ++y)
            {
              if (*inner != -1)
                {
                  count++;
                  inner++;
                }
            }

          if (str_equals_any(norm, 2, "probability", "cdf"))
            {
              count /= length;
            }
          else if (strcmp(norm, "pdf") == 0)
            {
              if (num_bin_edges == 0)
                {
                  count /= length * bin_width;
                }
              else
                {
                  count /= (length * *(bin_widths + x));
                }
            }
          else if (strcmp(norm, "countdensity") == 0)
            {
              if (num_bin_edges == 0)
                {
                  count /= bin_width;
                }
              else
                {
                  count /= *(bin_widths + x);
                }
            }

          /* no stairs*/
          if (stairs == 0)
            {
              r = pow((count / max), num_bins * 2);
              liste = moivre(r, 2 * x, num_bins * 2);
              cleanup_and_set_error_if(liste == nullptr, ERROR_MALLOC);
              liste0 = liste[0];
              liste1 = liste[1];
              rect = sqrt(liste0 * liste0 + liste1 * liste1);

              if (rlim != nullptr)
                {
                  double temporary;
                  int i;
                  liste2 = moivre(r, (2 * x + 2), (num_bins * 2));
                  cleanup_and_set_error_if(liste2 == nullptr, ERROR_MALLOC);

                  *(mlist + x * 4) = liste0;
                  *(mlist + x * 4 + 1) = liste1;
                  *(mlist + x * 4 + 2) = *(liste2);
                  *(mlist + x * 4 + 3) = *(liste2 + 1);

                  r_min_list = moivre(pow((r_min), (num_bins * 2)), (x * 2), num_bins * 2);
                  cleanup_and_set_error_if(r_min_list == nullptr, ERROR_MALLOC);
                  r_min_list2 = moivre(pow((r_min), (num_bins * 2)), (x * 2 + 2), num_bins * 2);
                  cleanup_and_set_error_if(r_min_list2 == nullptr, ERROR_MALLOC);

                  for (i = 0; i < 2; ++i)
                    {
                      temporary = fabs(sqrt(pow(mlist[x * 4 + 2 - i * 2], 2) + pow(mlist[x * 4 + 3 - i * 2], 2)));
                      if (temporary > r_max)
                        {
                          double factor = fabs(r_max / temporary);
                          mlist[x * 4 + 2 - i * 2] *= factor;
                          mlist[x * 4 + 3 - i * 2] *= factor;
                        }
                    }
                  r = count / max;
                  if (r > r_max)
                    {
                      r = r_max;
                    }
                  free(liste2);
                  liste2 = nullptr;
                }

              /*  no binedges */
              if (num_bin_edges == 0)
                {
                  if (rlim != nullptr)
                    {
                      double start_angle;
                      double end_angle;

                      double diff_angle;
                      int num_angle;

                      int i;

                      if (r <= r_min)
                        {
                          if (phiflip == 1)
                            outer -= length;
                          else
                            outer += length;
                          continue;
                        }

                      start_angle = x * (360.0 / num_bins) / convert;
                      end_angle = (x + 1) * (360.0 / num_bins) / convert;

                      diff_angle = end_angle - start_angle;
                      num_angle = (int)(diff_angle / (0.2 / convert));

                      phi_array = static_cast<double *>(malloc(num_angle * sizeof(double)));
                      cleanup_and_set_error_if(phi_array == nullptr, ERROR_MALLOC);
                      linspace(start_angle, end_angle, num_angle, phi_array);

                      f1 = static_cast<double *>(malloc((4 + 2 * num_angle) * sizeof(double)));
                      cleanup_and_set_error_if(f1 == nullptr, ERROR_MALLOC);
                      /* line_1_x[0] and [1]*/
                      f1[0] = r_min_list[0];
                      f1[1] = mlist[4 * x];
                      /* arc_1_x */
                      listcomprehension(r, cos, phi_array, num_angle, 2, f1);
                      /* reversed line_2_x [0] and [1] */
                      f1[2 + num_angle + 1] = r_min_list2[0];
                      f1[2 + num_angle] = mlist[4 * x + 2];
                      /* reversed arc_2_x */
                      arc_2_x = listcomprehension(r_min, cos, phi_array, num_angle, 0, nullptr);
                      cleanup_and_set_error_if(arc_2_x == nullptr, ERROR_MALLOC);
                      for (i = 0; i < num_angle; ++i)
                        {
                          f1[2 + num_angle + 2 + i] = arc_2_x[num_angle - 1 - i];
                        }
                      free(arc_2_x);
                      arc_2_x = nullptr;

                      f2 = static_cast<double *>(malloc((4 + 2 * num_angle) * sizeof(double)));
                      cleanup_and_set_error_if(f2 == nullptr, ERROR_MALLOC);
                      /* line_1_y[0] and [1] */
                      f2[0] = r_min_list[1];
                      f2[1] = mlist[4 * x + 1];
                      /*arc_1_y */
                      listcomprehension(r, sin, phi_array, num_angle, 2, f2);
                      /* reversed line_2_y [0] and [1] */
                      f2[2 + num_angle + 1] = r_min_list2[1];
                      f2[2 + num_angle] = mlist[4 * x + 3];
                      /* reversed arc_2_y */
                      arc_2_y = listcomprehension(r_min, sin, phi_array, num_angle, 0, nullptr);
                      cleanup_and_set_error_if(arc_2_y == nullptr, ERROR_MALLOC);
                      for (i = 0; i < num_angle; ++i)
                        {
                          f2[2 + num_angle + 2 + i] = arc_2_y[num_angle - 1 - i];
                        }
                      free(arc_2_y);
                      arc_2_y = nullptr;

                      if (draw_edges == 0)
                        {
                          int id = (int)global_root->getAttribute("id");
                          global_root->setAttribute("id", id + 1);
                          std::vector<double> f1_vec(f1, f1 + 4 + 2 * num_angle), f2_vec(f2, f2 + 4 + 2 * num_angle);
                          str = std::to_string(id);

                          temp_elem = global_render->createFillArea("x" + str, f1_vec, "y" + str, f2_vec);
                          group->append(temp_elem);
                          global_render->setFillColorInd(temp_elem, face_color);
                          global_render->setFillIntStyle(temp_elem, 1);

                          temp_elem = global_render->createFillArea("x" + str, std::nullopt, "y" + str, std::nullopt);
                          group->append(temp_elem);
                          global_render->setFillColorInd(temp_elem, edge_color);
                          global_render->setFillIntStyle(temp_elem, 0);
                        }
                      else
                        {
                          int id = (int)global_root->getAttribute("id");
                          global_root->setAttribute("id", id + 1);
                          std::vector<double> f1_vec(f1, f1 + 4 + 2 * num_angle), f2_vec(f2, f2 + 4 + 2 * num_angle);
                          str = std::to_string(id);

                          temp_elem = global_render->createFillArea("x" + str, std::nullopt, "y" + str, std::nullopt);
                          group->append(temp_elem);
                          global_render->setFillColorInd(temp_elem, edge_color);
                          global_render->setFillIntStyle(temp_elem, 0);
                        }

                      free(f1);
                      f1 = nullptr;
                      free(f2);
                      f2 = nullptr;
                      free(phi_array);
                      phi_array = nullptr;
                    } /* end rlim condition */
                  /* no rlim */
                  else
                    {
                      if (draw_edges == 0)
                        {
                          temp_elem = global_render->createFillArc(-rect, rect, -rect, rect, x * (360.0 / num_bins),
                                                                   (x + 1) * (360.0 / num_bins));
                          group->append(temp_elem);

                          global_render->setFillIntStyle(temp_elem, 1);
                          global_render->setFillColorInd(temp_elem, face_color);
                        }

                      temp_elem = global_render->createFillArc(-rect, rect, -rect, rect, x * (360.0 / num_bins),
                                                               (x + 1) * (360.0 / num_bins));
                      group->append(temp_elem);

                      global_render->setFillIntStyle(temp_elem, 0);
                      global_render->setFillColorInd(temp_elem, edge_color);
                    }
                }
              /* bin_egdes */
              else
                {
                  if (rlim != nullptr)
                    {
                      double start_angle;
                      double end_angle;

                      double diff_angle;
                      int num_angle;

                      int i;

                      if (r <= r_min)
                        {
                          if (phiflip != 0)
                            outer -= length;
                          else
                            outer += length;
                          continue;
                        }

                      start_angle = bin_edges[x];
                      end_angle = bin_edges[x + 1];

                      diff_angle = end_angle - start_angle;
                      num_angle = (int)(diff_angle / (0.2 / convert));
                      phi_array = static_cast<double *>(malloc(num_angle * sizeof(double)));
                      cleanup_and_set_error_if(phi_array == nullptr, ERROR_MALLOC);
                      linspace(start_angle, end_angle, num_angle, phi_array);

                      f1 = static_cast<double *>(malloc((4 + 2 * num_angle) * sizeof(double)));
                      cleanup_and_set_error_if(f1 == nullptr, ERROR_MALLOC);
                      /* line_1_x[0] and [1]*/
                      f1[0] = cos(bin_edges[x]) * r_min;
                      f1[1] = grm_min(rect, r_max) * cos(bin_edges[x]);
                      /*arc_1_x */
                      listcomprehension(r, cos, phi_array, num_angle, 2, f1);
                      /* reversed line_2_x [0] and [1] */
                      f1[2 + num_angle + 1] = cos(bin_edges[x + 1]) * r_min;
                      f1[2 + num_angle] = grm_min(rect, r_max) * cos(bin_edges[x + 1]);
                      /* reversed arc_2_x */
                      arc_2_x = listcomprehension(r_min, cos, phi_array, num_angle, 0, nullptr);
                      cleanup_and_set_error_if(arc_2_x == nullptr, ERROR_MALLOC);
                      for (i = 0; i < num_angle; ++i)
                        {
                          f1[2 + num_angle + 2 + i] = arc_2_x[num_angle - 1 - i];
                        }
                      free(arc_2_x);
                      arc_2_x = nullptr;

                      f2 = static_cast<double *>(malloc((4 + 2 * num_angle) * sizeof(double)));
                      cleanup_and_set_error_if(f2 == nullptr, ERROR_MALLOC);
                      /* line_1_y[0] and [1] */
                      f2[0] = r_min * sin(bin_edges[x]);
                      f2[1] = grm_min(rect, r_max) * sin(bin_edges[x]);
                      /*arc_1_y */
                      listcomprehension(r, sin, phi_array, num_angle, 2, f2);
                      /* reversed line_2_y [0] and [1] */
                      f2[2 + num_angle + 1] = r_min * sin(bin_edges[x + 1]);
                      f2[2 + num_angle] = grm_min(rect, r_max) * sin(bin_edges[x + 1]);
                      /* reversed arc_2_y */
                      arc_2_y = listcomprehension(r_min, sin, phi_array, num_angle, 0, nullptr);
                      cleanup_and_set_error_if(arc_2_y == nullptr, ERROR_MALLOC);
                      for (i = 0; i < num_angle; ++i)
                        {
                          f2[2 + num_angle + 2 + i] = arc_2_y[num_angle - 1 - i];
                        }
                      free(arc_2_y);
                      arc_2_y = nullptr;

                      if (draw_edges == 0)
                        {
                          int id = (int)global_root->getAttribute("id");
                          global_root->setAttribute("id", id + 1);
                          std::vector<double> f1_vec(f1, f1 + 4 + 2 * num_angle), f2_vec(f2, f2 + 4 + 2 * num_angle);
                          str = std::to_string(id);

                          temp_elem = global_render->createFillArea("x" + str, f1_vec, "y" + str, f2_vec);
                          group->append(temp_elem);
                          global_render->setFillColorInd(temp_elem, face_color);
                          global_render->setFillIntStyle(temp_elem, 1);

                          temp_elem = global_render->createFillArea("x" + str, std::nullopt, "y" + str, std::nullopt);
                          group->append(temp_elem);
                          global_render->setFillColorInd(temp_elem, edge_color);
                          global_render->setFillIntStyle(temp_elem, 0);
                        }
                      else
                        {
                          int id = (int)global_root->getAttribute("id");
                          global_root->setAttribute("id", id + 1);
                          std::vector<double> f1_vec(f1, f1 + 4 + 2 * num_angle), f2_vec(f2, f2 + 4 + 2 * num_angle);
                          str = std::to_string(id);

                          temp_elem = global_render->createFillArea("x" + str, std::nullopt, "y" + str, std::nullopt);
                          group->append(temp_elem);
                          global_render->setFillColorInd(temp_elem, edge_color);
                          global_render->setFillIntStyle(temp_elem, 0);
                        }

                      free(f1);
                      f1 = nullptr;
                      free(f2);
                      f2 = nullptr;
                      free(phi_array);
                      phi_array = nullptr;
                    }
                  /* no rlim */
                  else
                    {
                      if (draw_edges == 0)
                        {
                          temp_elem = global_render->createFillArc(-rect, rect, -rect, rect, bin_edges[x] * convert,
                                                                   bin_edges[x + 1] * convert);
                          group->append(temp_elem);

                          global_render->setFillIntStyle(temp_elem, 1);
                          global_render->setFillColorInd(temp_elem, face_color);
                        }
                      temp_elem = global_render->createFillArc(-rect, rect, -rect, rect, bin_edges[x] * convert,
                                                               bin_edges[x + 1] * convert);
                      group->append(temp_elem);

                      global_render->setFillIntStyle(temp_elem, 0);
                      global_render->setFillColorInd(temp_elem, edge_color);
                    }
                }
            } /* end no stairs condition */
          /* stairs without draw_edges (not compatible) */
          else if (draw_edges == 0 && colormap == nullptr)
            {
              global_render->setFillColorInd(group, 0);
              global_render->setLineColorInd(group, edge_color);
              global_render->setLineWidth(group, 2.3);

              r = pow((count / max), (num_bins * 2));
              liste = moivre(r, (2 * x), num_bins * 2);
              cleanup_and_set_error_if(liste == nullptr, ERROR_MALLOC);
              liste2 = moivre(r, (2 * x + 2), (num_bins * 2));
              cleanup_and_set_error_if(liste2 == nullptr, ERROR_MALLOC);
              rect = sqrt(*liste * *liste + *(liste + 1) * *(liste + 1));

              /*  no bin_edges */
              if (num_bin_edges == 0)
                {
                  *(mlist + x * 4) = *liste;
                  *(mlist + x * 4 + 1) = *(liste + 1);
                  *(mlist + x * 4 + 2) = *(liste2);
                  *(mlist + x * 4 + 3) = *(liste2 + 1);

                  if (rlim != nullptr)
                    {
                      double temporary;
                      int i;
                      for (i = 0; i < 2; ++i)
                        {
                          temporary = fabs(sqrt(pow(mlist[x * 4 + 2 - i * 2], 2) + pow(mlist[x * 4 + 3 - i * 2], 2)));
                          if (temporary > r_max)
                            {
                              double factor = fabs(r_max / temporary);
                              mlist[x * 4 + 2 - i * 2] *= factor;
                              mlist[x * 4 + 3 - i * 2] *= factor;
                            }
                        }

                      if (rect > r_min)
                        {
                          group->append(global_render->createDrawArc(
                              -grm_min(rect, r_max), grm_min(rect, r_max), -grm_min(rect, r_max), grm_min(rect, r_max),
                              x * (360.0 / num_bins), (x + 1) * 360.0 / num_bins));

                          group->append(global_render->createDrawArc(
                              -r_min, r_min, -r_min, r_min, x * (360.0 / num_bins), (x + 1) * (360.0 / num_bins)));
                        }
                    }
                  /* no rlim */
                  else
                    {
                      group->append(global_render->createDrawArc(-rect, rect, -rect, rect, x * (360.0 / num_bins),
                                                                 (x + 1) * (360.0 / num_bins)));
                    }
                }
              /* with bin_edges */
              else
                {
                  /* rlim and bin_edges*/
                  if (rlim != nullptr)
                    {
                      if (rect < r_min)
                        {
                          rectlist[x] = r_min;
                        }
                      else if (rect > r_max)
                        {
                          rectlist[x] = r_max;
                        }
                      else
                        {
                          rectlist[x] = rect;
                        }

                      if (rect > r_min)
                        {
                          group->append(global_render->createDrawArc(
                              -grm_min(rect, r_max), grm_min(rect, r_max), -grm_min(rect, r_max), grm_min(rect, r_max),
                              bin_edges[x] * convert, bin_edges[x + 1] * convert));

                          group->append(global_render->createDrawArc(
                              -r_min, r_min, -r_min, r_min, bin_edges[x] * convert, bin_edges[x + 1] * convert));
                        }
                    }
                  /* no rlim */
                  else
                    {
                      *(rectlist + x) = rect;
                      if (x == num_bin_edges - 1)
                        {
                          break;
                        }
                      group->append(global_render->createDrawArc(-rect, rect, -rect, rect, *(bin_edges + x) * convert,
                                                                 *(bin_edges + x + 1) * convert));
                    }
                }
            }

          if (phiflip == 0)
            outer += length;
          else
            {
              outer -= length;
            }
        } /* end of classes for loop */

      if (stairs != 0 && draw_edges == 0)
        {
          /* stairs without binedges, rlim */
          if (mlist != nullptr && rlim == nullptr && rectlist == nullptr)
            {
              int s;
              double line_x[2];
              double line_y[2];
              for (s = 0; s < num_bins * 4; s += 2)
                {
                  if (s > 2 && s % 4 == 0)
                    {
                      line_x[0] = *(mlist + s);
                      line_x[1] = *(mlist + s - 2);
                      line_y[0] = *(mlist + s + 1);
                      line_y[1] = *(mlist + s - 1);
                      group->append(global_render->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1]));
                    }
                }
              line_x[0] = *(mlist);
              line_x[1] = *(mlist + (num_bins - 1) * 4 + 2);
              line_y[0] = *(mlist + 1);
              line_y[1] = *(mlist + (num_bins - 1) * 4 + 3);
              group->append(global_render->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1]));
            }

          /* stairs without bin_edges with rlim*/
          else if (mlist != nullptr && rlim != nullptr && rectlist == nullptr)
            {
              double line_x[2], line_y[2];
              double rect1, rect2;
              for (x = 0; x < num_bins; ++x)
                {
                  if (x > 0)
                    {
                      rect1 = sqrt(pow(mlist[x * 4], 2) + pow(mlist[x * 4 + 1], 2));
                      rect2 = sqrt(pow(mlist[(x - 1) * 4 + 2], 2) + pow(mlist[(x - 1) * 4 + 3], 2));

                      if (rect1 < r_min && rect2 < r_min) continue;
                      if (rect1 < r_min)
                        {
                          mlist[4 * x] = r_min * cos(2 * M_PI / num_bins * x);
                          mlist[4 * x + 1] = r_min * sin(2 * M_PI / num_bins * x);
                        }
                      else if (rect2 < r_min)
                        {
                          mlist[(x - 1) * 4 + 2] = r_min * cos(2 * M_PI / num_bins * x);
                          mlist[(x - 1) * 4 + 3] = r_min * sin(2 * M_PI / num_bins * x);
                        }
                      line_x[0] = mlist[x * 4];
                      line_x[1] = mlist[(x - 1) * 4 + 2];
                      line_y[0] = mlist[x * 4 + 1];
                      line_y[1] = mlist[(x - 1) * 4 + 3];
                      group->append(global_render->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1]));
                    }
                }
              line_x[0] = mlist[(num_bins - 1) * 4 + 2] = grm_max(mlist[(num_bins - 1) * 4 + 2], r_min * cos(0));
              line_y[0] = mlist[(num_bins - 1) * 4 + 3] = grm_max(mlist[(num_bins - 1) * 4 + 3], r_min * sin(0));
              line_x[1] = mlist[0] = grm_max(mlist[0], r_min * cos(0));
              line_y[1] = mlist[1] = grm_max(mlist[1], r_min * sin(0));

              group->append(global_render->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1]));
            }

          /* stairs with binedges without rlim */
          else if (rectlist != nullptr && rlim == nullptr)
            {
              double startx = 0.0, starty = 0.0;
              double line_x[2], line_y[2];

              for (x = 0; x < num_bin_edges - 1; ++x)
                {
                  line_x[0] = startx;
                  line_x[1] = *(rectlist + x) * cos(*(bin_edges + x));
                  line_y[0] = starty;
                  line_y[1] = *(rectlist + x) * sin(*(bin_edges + x));

                  startx = *(rectlist + x) * cos(*(bin_edges + x + 1));
                  starty = *(rectlist + x) * sin(*(bin_edges + x + 1));

                  if (!(*bin_edges == 0.0 && *(bin_edges + num_bin_edges - 1) > 1.96 * M_PI) || x > 0)
                    {
                      group->append(global_render->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1]));
                    }
                }

              if (*bin_edges == 0.0 && *(bin_edges + num_bin_edges - 1) > 1.96 * M_PI)
                {
                  line_x[0] = *rectlist * cos(*bin_edges);
                  line_x[1] = startx;
                  line_y[0] = *rectlist * sin(*bin_edges);
                  line_y[1] = starty;
                  group->append(global_render->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1]));
                }
              else
                {
                  line_x[0] = *(rectlist + num_bin_edges - 2) * cos(*(bin_edges + num_bin_edges - 1));
                  line_x[1] = 0.0;
                  line_y[0] = *(rectlist + num_bin_edges - 2) * sin(*(bin_edges + num_bin_edges - 1));
                  line_y[1] = 0.0;
                  group->append(global_render->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1]));
                }
            }

          /* stairs with bin_edges and rlim */
          else if (rectlist != nullptr && rlim != nullptr)
            {
              double startx = grm_max(rectlist[0] * cos(bin_edges[0]), r_min * cos(bin_edges[0]));
              double starty = grm_max(rectlist[0] * sin(bin_edges[0]), r_min * sin(bin_edges[0]));

              double line_x[2];
              double line_y[2];

              for (x = 0; x < num_bin_edges - 1; ++x)
                {
                  *line_x = startx;
                  *(line_x + 1) = *(rectlist + x) * cos(*(bin_edges + x));
                  *line_y = starty;
                  *(line_y + 1) = *(rectlist + x) * sin(*(bin_edges + x));

                  startx = *(rectlist + x) * cos(*(bin_edges + x + 1));
                  starty = *(rectlist + x) * sin(*(bin_edges + x + 1));

                  if (((phiflip == 0) &&
                       (!((*bin_edges > 0.0 && *bin_edges < 0.001) && *(bin_edges + num_bin_edges - 1) > 1.96 * M_PI) ||
                        x > 0)) ||
                      ((*bin_edges > 1.96 * M_PI &&
                        !(*(bin_edges + num_bin_edges - 1) > 0.0 && *(bin_edges + num_bin_edges - 1) < 0.001)) ||
                       x > 0))
                    {
                      group->append(global_render->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1]));
                    }
                }

              if (*bin_edges == 0.0 && *(bin_edges + num_bin_edges - 1) > 1.96 * M_PI)
                {
                  *line_x = *rectlist * cos(*bin_edges);
                  *(line_x + 1) = rectlist[num_bin_edges - 2] * cos(bin_edges[num_bin_edges - 1]);
                  *line_y = *rectlist * sin(*bin_edges);
                  *(line_y + 1) = rectlist[num_bin_edges - 2] * sin(bin_edges[num_bin_edges - 1]);
                  group->append(global_render->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1]));
                }
              else
                {
                  *line_x = *(rectlist + num_bin_edges - 2) * cos(*(bin_edges + num_bin_edges - 1));
                  *(line_x + 1) = r_min * cos(bin_edges[num_bin_edges - 1]);
                  *line_y = *(rectlist + num_bin_edges - 2) * sin(*(bin_edges + num_bin_edges - 1));
                  *(line_y + 1) = r_min * sin(bin_edges[num_bin_edges - 1]);

                  group->append(global_render->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1]));

                  line_x[0] = r_min * cos(bin_edges[0]);
                  line_x[1] = rectlist[0] * cos(bin_edges[0]);
                  line_y[0] = r_min * sin(bin_edges[0]);
                  line_y[1] = rectlist[0] * sin(bin_edges[0]);

                  group->append(global_render->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1]));
                }
            }
        }
    }

  gr_setresamplemethod(resample);

cleanup:
  free(mlist);
  free(rectlist);
  free(lineardata);
  free(bin_counts);
  if (freeable_bin_widths == 1)
    {
      free(bin_widths);
    }
  if (freeable_bin_edges == 1)
    {
      free(bin_edges);
    }
  if (freeable_angles == 1)
    {
      free(angles);
    }
  free(phi_array);
  free(r_min_list);
  free(r_min_list2);
  free(f1);
  free(f2);
  free(arc_2_x);
  free(arc_2_y);
  free(liste);
  free(liste2);
  free(colormap);
}

static void processMarginalheatmapKind(const std::shared_ptr<GRM::Element> &elem)
{
  std::string mkind = static_cast<std::string>(elem->getAttribute("marginalheatmap_kind"));
  for (auto child : elem->children())
    {
      if (!child->hasAttribute("calc_window_and_viewport_from_parent")) continue;
      if (mkind == "line")
        {
          std::shared_ptr<GRM::Context> context;
          std::shared_ptr<GRM::Render> render;

          render = std::dynamic_pointer_cast<GRM::Render>(child->ownerDocument());
          if (!render)
            {
              throw NotFoundError("Render-document not found for element\n");
            }
          context = render->getContext();
          auto orientation = static_cast<std::string>(child->getAttribute("orientation"));
          int is_vertical = orientation == "vertical";
          int xind = static_cast<int>(elem->getAttribute("xind"));
          int yind = static_cast<int>(elem->getAttribute("yind"));

          auto plot_group = elem->parentElement();

          double c_min = static_cast<double>(plot_group->getAttribute("lim_zmin"));
          double c_max = static_cast<double>(plot_group->getAttribute("lim_zmax"));

          int i;
          double y_max = 0;
          std::vector<double> z =
              GRM::get<std::vector<double>>((*context)[static_cast<std::string>(elem->getAttribute("z"))]);
          std::vector<double> y =
              GRM::get<std::vector<double>>((*context)[static_cast<std::string>(elem->getAttribute("y"))]);
          std::vector<double> xi =
              GRM::get<std::vector<double>>((*context)[static_cast<std::string>(child->getAttribute("xi"))]);
          std::vector<double> x =
              GRM::get<std::vector<double>>((*context)[static_cast<std::string>(elem->getAttribute("x"))]);

          int y_length = y.size();
          int x_length = xi.size();

          double xmin = static_cast<double>(plot_group->getAttribute("lim_xmin"));
          double xmax = static_cast<double>(plot_group->getAttribute("lim_xmax"));
          double ymin = static_cast<double>(plot_group->getAttribute("lim_ymin"));
          double ymax = static_cast<double>(plot_group->getAttribute("lim_ymax"));
          // plot step in marginal
          for (i = 0; i < (is_vertical ? y_length : x_length); i++)
            {
              if (is_vertical)
                {
                  y[i] = std::isnan(z[xind + i * x_length]) ? 0 : z[xind + i * x_length];
                  y_max = grm_max(y_max, y[i]);
                }
              else
                {
                  y[i] = std::isnan(z[x_length * yind + i]) ? 0 : z[x_length * yind + i];
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

          std::shared_ptr<GRM::Element> line_elem, marker_elem;

          if (is_vertical)
            {
              line_elem = global_render->createPolyline("x" + id_str, y_step_values, "y" + id_str, x_step_boundaries);
              x_pos = (x_step_boundaries[yind * 2] + x_step_boundaries[yind * 2 + 1]) / 2;
              y_pos = y[yind];
              marker_elem = global_render->createPolymarker(y_pos, x_pos);
            }
          else
            {
              line_elem = global_render->createPolyline("x" + id_str, x_step_boundaries, "y" + id_str, y_step_values);
              x_pos = (x_step_boundaries[xind * 2] + x_step_boundaries[xind * 2 + 1]) / 2;
              y_pos = y[xind];
              marker_elem = global_render->createPolymarker(x_pos, y_pos);
            }

          global_render->setLineColorInd(line_elem, 989);
          global_render->setMarkerColorInd(marker_elem, 2);
          global_render->setMarkerType(marker_elem, -1);
          global_render->setMarkerSize(marker_elem, 1.5 * (len / (is_vertical ? (ymax - ymin) : (xmax - xmin))));

          marker_elem->setAttribute("name", "line");
          line_elem->setAttribute("name", "line");
          child->append(marker_elem);
          child->append(line_elem);
        }
      else if (mkind == "all")
        {
          int xind, yind;
          xind = static_cast<int>(elem->getAttribute("xind"));
          yind = static_cast<int>(elem->getAttribute("yind"));

          if (xind == -1 && yind == -1)
            {
              return;
            }

          bool is_horizontal = static_cast<std::string>(child->getAttribute("orientation")) == "horizontal";
          std::vector<std::shared_ptr<GRM::Element>> groups = child->children();

          if ((is_horizontal && xind == -1) || (!is_horizontal && yind == -1))
            {
              continue;
            }
          if ((is_horizontal ? xind : yind) >= groups.size())
            {
              continue;
            }

          int cnt = 0;
          for (const auto &rect : groups)
            {
              if (rect->hasAttribute("linecolorind")) continue;
              if (cnt == (is_horizontal ? xind : yind))
                {
                  rect->setAttribute("fillcolorind", 2);
                  break;
                }
              cnt += 1;
            }
        }
    }
}

static void processKind(const std::shared_ptr<GRM::Element> &elem)
{
  std::string kind = static_cast<std::string>(elem->getAttribute("kind"));
  if (kind == "polar_histogram")
    {
      polarHistogram(elem);
    }
}

void GRM::Render::processLimits(const std::shared_ptr<GRM::Element> &elem)
{
  /*!
   * processing function for gr_window
   *
   * \param[in] element The GRM::Element that contains the attributes
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
          elem->removeAttribute("_original_xlim");
          elem->removeAttribute("_original_xmin");
          elem->removeAttribute("_original_xmax");
          elem->removeAttribute("_original_ylim");
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

      /* Ensure the correct window is set in GRM */
      bool window_exists = elem->hasAttribute("window") && static_cast<int>(elem->getAttribute("window"));
      bool window3d_exists = elem->hasAttribute("window3d") && static_cast<int>(elem->getAttribute("window3d"));
      if (window_exists || window3d_exists)
        {
          double stored_window_xmin = static_cast<double>(elem->getAttribute("window_xmin"));
          double stored_window_xmax = static_cast<double>(elem->getAttribute("window_xmax"));
          double stored_window_ymin = static_cast<double>(elem->getAttribute("window_ymin"));
          double stored_window_ymax = static_cast<double>(elem->getAttribute("window_ymax"));

          gr_setwindow(stored_window_xmin, stored_window_xmax, stored_window_ymin, stored_window_ymax);
        }
      else
        {
          throw NotFoundError("Window not found\n");
        }

      gr_panzoom(x, y, xzoom, yzoom, &xmin, &xmax, &ymin, &ymax);

      elem->setAttribute("panzoom", false);
      elem->removeChild(panzoom_element);
    }

  elem->setAttribute("lim_xmin", xmin);
  elem->setAttribute("lim_xmax", xmax);
  elem->setAttribute("lim_ymin", ymin);
  elem->setAttribute("lim_ymax", ymax);

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
      if (zmax > 0)
        {
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
          global_render->setWindow3d(elem, xmin, xmax, ymin, ymax, zmin, zmax);
        }
    }
  else
    {
      logger((stderr, "Storing window (%lf, %lf, %lf, %lf)\n", xmin, xmax, ymin, ymax));
      // todo: grplot zoom etc. dont set -1 1 -1 1 window here --> use _xlim _ylim like in plot.cxx
      if (!str_equals_any(kind.c_str(), 4, "polar", "polar_histogram", "polar_heatmap", "nonuniformpolar_heatmap"))
        {
          global_render->setWindow(elem, xmin, xmax, ymin, ymax);
        }
      else
        {
          xmin = -1.0;
          xmax = 1.0;
          ymin = -1.0;
          ymax = 1.0;
          global_render->setWindow(elem, xmin, xmax, ymin, ymax);
        }
    }
}

static void processLineColorInd(const std::shared_ptr<GRM::Element> &elem)
{
  gr_setlinecolorind(static_cast<int>(elem->getAttribute("linecolorind")));
}

static void processLineSpec(const std::shared_ptr<GRM::Element> &elem)
{
  gr_uselinespec((static_cast<std::string>(elem->getAttribute("linespec"))).data());
}

static void processLineType(const std::shared_ptr<GRM::Element> &elem)
{
  gr_setlinetype(static_cast<int>(elem->getAttribute("linetype")));
}

static void processLineWidth(const std::shared_ptr<GRM::Element> &elem)
{
  gr_setlinewidth(static_cast<double>(elem->getAttribute("linewidth")));
}

static void processMarkerColorInd(const std::shared_ptr<GRM::Element> &elem)
{
  gr_setmarkercolorind(static_cast<int>(elem->getAttribute("markercolorind")));
}

static void processMarkerSize(const std::shared_ptr<GRM::Element> &elem)
{
  gr_setmarkersize(static_cast<double>(elem->getAttribute("markersize")));
}

static void processMarkerType(const std::shared_ptr<GRM::Element> &elem)
{
  gr_setmarkertype(static_cast<int>(elem->getAttribute("markertype")));
}

static void processProjectionType(const std::shared_ptr<GRM::Element> &elem)
{
  int type = (int)elem->getAttribute("projectiontype");
  gr_setprojectiontype(type);
}

static void processRelativeCharHeight(const std::shared_ptr<GRM::Element> &elem)
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

static void processResampleMethod(const std::shared_ptr<GRM::Element> &elem)
{
  unsigned int resample_method_flag;
  if (!elem->getAttribute("resample_method").isInt())
    {
      auto resample_method_str = static_cast<std::string>(elem->getAttribute("resample_method"));

      if (resample_method_str == "nearest")
        {
          resample_method_flag = GKS_K_RESAMPLE_NEAREST;
        }
      else if (resample_method_str == "linear")
        {
          resample_method_flag = GKS_K_RESAMPLE_LINEAR;
        }
      else if (resample_method_str == "lanczos")
        {
          resample_method_flag = GKS_K_RESAMPLE_LANCZOS;
        }
      else
        {
          resample_method_flag = GKS_K_RESAMPLE_DEFAULT;
        }
    }
  else
    {
      resample_method_flag = static_cast<int>(elem->getAttribute("resample_method"));
    }
  gr_setresamplemethod(resample_method_flag);
}

static void processScale(const std::shared_ptr<GRM::Element> &elem)
{
  gr_setscale(static_cast<int>(elem->getAttribute("scale")));
}

static void processSelntran(const std::shared_ptr<GRM::Element> &elem)
{
  gr_selntran(static_cast<int>(elem->getAttribute("selntran")));
}

static void processSpace(const std::shared_ptr<GRM::Element> &elem)
{
  double zmin, zmax;
  int rotation, tilt;
  zmin = (double)elem->getAttribute("space_zmin");
  zmax = (double)elem->getAttribute("space_zmax");
  rotation = (int)elem->getAttribute("space_rotation");
  tilt = (int)elem->getAttribute("space_tilt");

  gr_setspace(zmin, zmax, rotation, tilt);
}

static void processSpace3d(const std::shared_ptr<GRM::Element> &elem)
{
  double phi = PLOT_DEFAULT_ROTATION, theta = PLOT_DEFAULT_TILT, fov, camera_distance;

  if (elem->hasAttribute("space3d_phi"))
    {
      phi = (double)elem->getAttribute("space3d_phi");
    }
  else
    {
      elem->setAttribute("space3d_phi", phi);
    }
  if (elem->hasAttribute("space3d_theta"))
    {
      theta = (double)elem->getAttribute("space3d_theta");
    }
  else
    {
      elem->setAttribute("space3d_theta", theta);
    }
  fov = (double)elem->getAttribute("space3d_fov");
  camera_distance = (double)elem->getAttribute("space3d_camera_distance");

  gr_setspace3d(-phi, theta, fov, camera_distance);
}

static void processSubplot(const std::shared_ptr<GRM::Element> &elem)
{
  /*!
   * processing function for the subplot
   *
   * \param[in] element The GRM::Element that contains the attributes
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
      int location = PLOT_DEFAULT_LOCATION;
      if (elem->hasAttribute("location"))
        {
          location = static_cast<int>(elem->getAttribute("location"));
        }
      else
        {
          elem->setAttribute("location", location);
        }

      if (location == 11 || location == 12 || location == 13)
        {
          legend_size(elem, &w, &h);
          viewport[1] -= w + 0.1;
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

static void processTextAlign(const std::shared_ptr<GRM::Element> &elem)
{
  gr_settextalign(static_cast<int>(elem->getAttribute("textalign_horizontal")),
                  static_cast<int>(elem->getAttribute("textalign_vertical")));
}

static void processTextColorForBackground(const std::shared_ptr<GRM::Element> &elem)
/*  The set_text_color_for_background function used in plot.cxx now as an attribute function
    It is now possible to inquire colors during runtime -> No colors are given as parameters
    The new color is set on `elem`
    There are no params apart from elem
    \param[in] elem The GRM::Element the color should be set in. Also contains other attributes which may function as
 parameters

    Attributes as Parameters (with prefix "stcfb-"):
    plot: for which plot it is used: right now only pie plot
 */
{
  int color_ind;
  int inq_color;
  unsigned char color_rgb[4];
  std::string plot = "pie";

  if (elem->hasAttribute("stcfb_plot"))
    {
      plot = static_cast<std::string>(elem->getAttribute("stcfb_plot"));
    }

  if (plot == "pie")
    {
      double r, g, b;
      double color_lightness;
      std::shared_ptr<GRM::Render> render;

      render = std::dynamic_pointer_cast<GRM::Render>(elem->ownerDocument());
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

static void processTextColorInd(const std::shared_ptr<GRM::Element> &elem)
{

  gr_settextcolorind(static_cast<int>(elem->getAttribute("textcolorind")));
}

static void processTextEncoding(const std::shared_ptr<GRM::Element> &elem)
{

  gr_settextencoding(static_cast<int>(elem->getAttribute("textencoding")));
}

static void processTextFontPrec(const std::shared_ptr<GRM::Element> &elem)
{

  gr_settextfontprec(static_cast<int>(elem->getAttribute("textfontprec_font")),
                     static_cast<int>(elem->getAttribute("textfontprec_prec")));
}

static void processTextPath(const std::shared_ptr<GRM::Element> &elem)
{
  gr_settextpath(static_cast<int>(elem->getAttribute("textpath")));
}

static void processTitle(const std::shared_ptr<GRM::Element> &elem)
{
  double viewport[4], vp[4];

  auto subplot_element = getSubplotElement(elem);
  std::string name = (std::string)subplot_element->getAttribute("name");
  if (name != "polarhistogram" && name != "pie")
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

      if (title.empty()) return; // Empty title is pointless, no need to waste the space for nothing
      if (auto render = std::dynamic_pointer_cast<GRM::Render>(elem->ownerDocument()))
        {
          auto new_title_elem = render->createText(x, y, title);
          new_title_elem->setAttribute("name", "title");
          render->setTextAlign(new_title_elem, GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_TOP);
          auto title_elem = elem->querySelectors("[name=\"title\"]");
          bool removed_elem = false;
          if (title_elem && (std::string)title_elem->getAttribute("text") != title)
            {
              title_elem->remove();
              removed_elem = true;
            }
          if (removed_elem || !title_elem)
            {
              elem->appendChild(new_title_elem);
            }
        }
    }
  else if (name == "pie")
    {
      bool vp_found = false;
      std::shared_ptr<GRM::Render> render;
      std::shared_ptr<GRM::Element> ancestor = elem->parentElement();

      gr_inqviewport(&viewport[0], &viewport[1], &viewport[2], &viewport[3]);

      vp[0] = (double)subplot_element->getAttribute("vp_xmin");
      vp[1] = (double)subplot_element->getAttribute("vp_xmax");
      vp[2] = (double)subplot_element->getAttribute("vp_ymin");
      vp[3] = (double)subplot_element->getAttribute("vp_ymax");

      render = std::dynamic_pointer_cast<GRM::Render>(elem->ownerDocument());
      if (!render)
        {
          throw NotFoundError("No render-document found for element.\n");
        }

      std::string title = static_cast<std::string>(elem->getAttribute("title"));
      auto new_title_elem = render->createText(0.5 * (viewport[0] + viewport[1]), vp[3] - 0.02, title);
      render->setTextColorInd(new_title_elem, 1);
      render->setTextAlign(new_title_elem, GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_TOP);

      auto old_title_elem = elem->querySelectors("[name=\"title\"]");
      bool removed_elem = false;
      if (old_title_elem && (std::string)old_title_elem->getAttribute("text") != title)
        {
          old_title_elem->remove();
          removed_elem = true;
        }
      if (removed_elem || !old_title_elem)
        {
          new_title_elem->setAttribute("name", "title");
          elem->appendChild(new_title_elem);
        }
    }
}

static void processTransparency(const std::shared_ptr<GRM::Element> &elem)
{
  gr_settransparency(static_cast<double>(elem->getAttribute("transparency")));
}

static void processWindow(const std::shared_ptr<GRM::Element> &elem)
{
  int scale = 0;
  double xmin = static_cast<double>(elem->getAttribute("window_xmin"));
  double xmax = static_cast<double>(elem->getAttribute("window_xmax"));
  double ymin = static_cast<double>(elem->getAttribute("window_ymin"));
  double ymax = static_cast<double>(elem->getAttribute("window_ymax"));

  auto kind = static_cast<std::string>(elem->getAttribute("kind"));
  if (kind != "pie" && kind != "polar" && kind != "polar_histogram" && kind != "polar_heatmap" &&
      kind != "nonuniformpolar_heatmap")
    {
      scale |= static_cast<int>(elem->getAttribute("xlog")) ? GR_OPTION_X_LOG : 0;
      scale |= static_cast<int>(elem->getAttribute("ylog")) ? GR_OPTION_Y_LOG : 0;
      scale |= static_cast<int>(elem->getAttribute("zlog")) ? GR_OPTION_Z_LOG : 0;
      scale |= static_cast<int>(elem->getAttribute("xflip")) ? GR_OPTION_FLIP_X : 0;
      scale |= static_cast<int>(elem->getAttribute("yflip")) ? GR_OPTION_FLIP_Y : 0;
      scale |= static_cast<int>(elem->getAttribute("zflip")) ? GR_OPTION_FLIP_Z : 0;
    }
  elem->setAttribute("scale", scale);
  processScale(elem);

  gr_setwindow(xmin, xmax, ymin, ymax);
}

static void processWindow3d(const std::shared_ptr<GRM::Element> &elem)
{
  double xmin = static_cast<double>(elem->getAttribute("window_xmin"));
  double xmax = static_cast<double>(elem->getAttribute("window_xmax"));
  double ymin = static_cast<double>(elem->getAttribute("window_ymin"));
  double ymax = static_cast<double>(elem->getAttribute("window_ymax"));
  double zmin = static_cast<double>(elem->getAttribute("window_zmin"));
  double zmax = static_cast<double>(elem->getAttribute("window_zmax"));

  gr_setwindow3d(xmin, xmax, ymin, ymax, zmin, zmax);
}

static void processWSViewport(const std::shared_ptr<GRM::Element> &elem)
{
  /*!
   * processing function for gr_wsviewport
   *
   * \param[in] element The GRM::Element that contains the attributes
   */
  double wsviewport[4];
  wsviewport[0] = (double)elem->getAttribute("wsviewport_xmin");
  wsviewport[1] = (double)elem->getAttribute("wsviewport_xmax");
  wsviewport[2] = (double)elem->getAttribute("wsviewport_ymin");
  wsviewport[3] = (double)elem->getAttribute("wsviewport_ymax");

  gr_setwsviewport(wsviewport[0], wsviewport[1], wsviewport[2], wsviewport[3]);
}

static void processWSWindow(const std::shared_ptr<GRM::Element> &elem)
{
  double wswindow[4];
  double xmin = static_cast<double>(elem->getAttribute("wswindow_xmin"));
  double xmax = static_cast<double>(elem->getAttribute("wswindow_xmax"));
  double ymin = static_cast<double>(elem->getAttribute("wswindow_ymin"));
  double ymax = static_cast<double>(elem->getAttribute("wswindow_ymax"));

  gr_setwswindow(xmin, xmax, ymin, ymax);
}

void GRM::Render::processViewport(const std::shared_ptr<GRM::Element> &elem)
{
  /*!
   * processing function for gr_viewport
   *
   * \param[in] element The GRM::Element that contains the attributes
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

static void processXlabel(const std::shared_ptr<GRM::Element> &elem)
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
  if (x_label.empty()) return; // Empty xlabel is pointless, no need to waste the space for nothing

  if (auto render = std::dynamic_pointer_cast<GRM::Render>(elem->ownerDocument()))
    {
      auto text = render->createText(x, y, x_label);
      text->setAttribute("name", "xlabel");
      render->setTextAlign(text, GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_BOTTOM);
      auto xlabel_elem = elem->querySelectors("[name=\"xlabel\"]");
      bool removed_elem = false;
      if (xlabel_elem && (std::string)xlabel_elem->getAttribute("text") != x_label)
        {
          xlabel_elem->remove();
          removed_elem = true;
        }
      if (removed_elem || !xlabel_elem)
        {
          elem->appendChild(text);
        }
    }
}

static void processXTickLabels(const std::shared_ptr<GRM::Element> &elem)
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

  if (auto render = std::dynamic_pointer_cast<GRM::Render>(elem->ownerDocument()))
    {
      std::shared_ptr<GRM::Context> context = render->getContext();
      std::string key = static_cast<std::string>(elem->getAttribute("xticklabels"));
      xticklabels = GRM::get<std::vector<std::string>>((*context)[key]);

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

static void processYlabel(const std::shared_ptr<GRM::Element> &elem)
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
  if (y_label.empty()) return; // Empty ylabel is pointless, no need to waste the space for nothing

  if (auto render = std::dynamic_pointer_cast<GRM::Render>(elem->ownerDocument()))
    {
      auto text = render->createText(x, y, y_label);
      text->setAttribute("name", "ylabel");
      render->setTextAlign(text, GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_TOP);
      auto ylabel_elem = elem->querySelectors("[name=\"ylabel\"]");
      bool removed_elem = false;
      if (ylabel_elem && (std::string)ylabel_elem->getAttribute("text") != y_label)
        {
          ylabel_elem->remove();
          removed_elem = true;
        }
      if (removed_elem || !ylabel_elem)
        {
          render->setCharUp(text, -1, 0);
          elem->appendChild(text);
        }
    }
}

static void processClassesPolarHistogram(const std::shared_ptr<GRM::Element> &element)
{
  unsigned int num_bins;
  double *theta = nullptr;
  unsigned int length;
  const char *norm;
  double *classes = nullptr;

  double interval;
  double start;
  double *p;
  double max;
  double temp_max;

  double *bin_edges = nullptr;
  double *bin_edges_buf = nullptr;
  unsigned int num_bin_edges;

  double bin_width;

  double *bin_widths = nullptr;

  int is_bin_counts = 0;
  int *bin_counts = nullptr;

  double *philim = nullptr;
  double philim_arr[2];
  unsigned int dummy;

  double *new_theta = nullptr;
  double *new_edges = nullptr;

  err_t error = ERROR_NONE;

  std::shared_ptr<GRM::Element> group = element;
  std::shared_ptr<GRM::Context> context;

  if (auto render = std::dynamic_pointer_cast<GRM::Render>(element->ownerDocument()))
    {
      context = render->getContext();
    }
  else
    {
      throw NotFoundError("no context / render found for given element");
    }

  //! define keys for later usages;
  auto str = static_cast<std::string>(group->getAttribute("id"));
  std::string bin_widths_key = "bin_widths" + str;
  std::string bin_edges_key = "bin_edges" + str;
  std::string bin_counts_key;
  std::string classes_key = "classes" + str;

  //! define vectors for later usages
  std::vector<double> theta_vec;
  std::vector<int> bin_counts_vec;
  std::vector<double> bin_edges_vec;

  if (group->hasAttribute("bin_counts"))
    {
      is_bin_counts = 1;
      bin_counts_key = static_cast<std::string>(group->getAttribute("bin_counts"));
      bin_counts_vec = GRM::get<std::vector<int>>((*context)[bin_counts_key]);

      bin_counts = &(bin_counts_vec[0]);
      length = bin_counts_vec.size();
      num_bins = length;
      group->setAttribute("nbins", static_cast<int>(num_bins));
    }
  else if (group->hasAttribute("theta"))
    {
      auto theta_key = static_cast<std::string>(group->getAttribute("theta"));
      theta_vec = GRM::get<std::vector<double>>((*context)[theta_key]);
      theta = &(theta_vec[0]);
      length = theta_vec.size();
    }

  if (!group->hasAttribute("philim"))
    {
      philim = nullptr;
    }
  else
    {
      philim = philim_arr;
      philim[0] = static_cast<double>(group->getAttribute("phimin"));
      philim[1] = static_cast<double>(group->getAttribute("phimax"));

      if (philim[1] < philim[0])
        {
          int phiflip = 0;
          std::swap(philim[0], philim[1]);

          group->setAttribute("phiflip", 1);
        }
      if (philim[0] < 0.0 || philim[1] > 2 * M_PI)
        {
          logger((stderr, "philim must be between 0 and 2 * pi\n"));
          cleanup_and_set_error(ERROR_PLOT_OUT_OF_RANGE);
        }
      group->setAttribute("philim", true);
      group->setAttribute("phi_min", philim[0]);
      group->setAttribute("phi_max", philim[1]);
    }

  /* bin_edges and nbins */
  if (group->hasAttribute("bin_edges") == 0)
    {
      if (group->hasAttribute("nbins") == 0)
        {
          num_bins = grm_min(12, (int)(length * 1.0 / 2) - 1);
          group->setAttribute("nbins", static_cast<int>(num_bins));
        }
      else
        {
          num_bins = static_cast<int>(group->getAttribute("nbins"));
          if (num_bins <= 0 || num_bins > 200)
            {
              num_bins = grm_min(12, (int)(length * 1.0 / 2) - 1);
              group->setAttribute("nbins", static_cast<int>(num_bins));
            }
        }
      //! check philim again
      if (philim == nullptr)
        num_bin_edges = 0;
      else
        {
          //! if philim is given, it will create equidistant bin_edges from phi_min to phi_max
          bin_edges = bin_edges_buf = static_cast<double *>(malloc((num_bins + 1) * sizeof(double)));
          cleanup_and_set_error_if(bin_edges == nullptr, ERROR_MALLOC);
          linspace(philim[0], philim[1], num_bins + 1, bin_edges);
          num_bin_edges = num_bins + 1;
          bin_edges_vec = std::vector<double>(bin_edges, bin_edges + num_bin_edges);
          (*context)[bin_edges_key] = bin_edges_vec;
          group->setAttribute("bin_edges", bin_edges_key);
        }
    }
  /* with bin_edges */
  else
    {
      bin_edges_key = static_cast<std::string>(group->getAttribute("bin_edges"));
      bin_edges_vec = GRM::get<std::vector<double>>((*context)[bin_edges_key]);
      bin_edges = &(bin_edges_vec[0]);
      num_bin_edges = bin_edges_vec.size();
      /* no philim */
      if (philim == nullptr)
        {

          /* filter bin_edges */
          int temp = 0;
          int i;
          new_edges = static_cast<double *>(malloc(num_bin_edges * sizeof(double)));
          cleanup_and_set_error_if(new_edges == nullptr, ERROR_MALLOC);

          for (i = 0; i < num_bin_edges; ++i)
            {
              if (0.0 <= bin_edges[i] && bin_edges[i] <= 2 * M_PI)
                {
                  new_edges[temp] = bin_edges[i];
                  temp++;
                }
              else
                {
                  logger((stderr, "Only values between 0 and 2 * pi allowed\n"));
                  cleanup_and_set_error(ERROR_PLOT_OUT_OF_RANGE);
                }
            }
          if (num_bin_edges > temp)
            {
              num_bin_edges = temp;
              bin_edges = bin_edges_buf = (double *)realloc(new_edges, temp * sizeof(double));
              cleanup_and_set_error_if(bin_edges == nullptr, ERROR_MALLOC);
            }
          else
            {
              bin_edges = new_edges;
            }
          num_bins = num_bin_edges - 1;
          group->setAttribute("nbins", static_cast<int>(num_bins));
        }
      /* with philim and binedges */
      else
        {
          /* filter bin_edges */
          int temp = 0;
          new_edges = static_cast<double *>(malloc(num_bin_edges * sizeof(double)));
          cleanup_and_set_error_if(new_edges == nullptr, ERROR_MALLOC);

          int i;
          for (i = 0; i < num_bin_edges; ++i)
            {
              if (philim[0] <= bin_edges[i] && bin_edges[i] <= philim[1])
                {
                  new_edges[temp] = bin_edges[i];
                  temp++;
                }
            }
          if (temp > 1)
            {
              if (num_bin_edges > temp)
                {
                  num_bin_edges = temp;
                  bin_edges = bin_edges_buf = (double *)realloc(new_edges, temp * sizeof(double));
                }
              else
                {
                  bin_edges = new_edges;
                }
            }
          if (num_bin_edges == 1)
            {
              logger(
                  (stderr, "given philim and given bin_edges are not compatible --> filtered len(bin_edges) == 1\n"));
              cleanup_and_set_error(ERROR_PLOT_INCOMPATIBLE_ARGUMENTS);
            }
          else
            {
              num_bins = num_bin_edges - 1;
              group->setAttribute("nbins", static_cast<int>(num_bins));
              group->setAttribute("bin_edges", bin_edges_key);
              std::vector<double> bin_edges_vec(bin_edges, bin_edges + num_bin_edges);
              (*context)[bin_edges_key] = bin_edges_vec;
            }
        }
    }

  if (group->hasAttribute("normalization") == 0)
    {
      norm = "count";
    }
  else
    {
      norm = static_cast<std::string>(group->getAttribute("normalization")).c_str();
      if (!str_equals_any(norm, 6, "count", "countdensity", "pdf", "probability", "cumcount", "cdf"))
        {
          logger((stderr, "Got keyword \"norm\"  with invalid value \"%s\"\n", norm));
          cleanup_and_set_error(ERROR_PLOT_NORMALIZATION);
        }
    }

  if (group->hasAttribute("bin_width") == 0)
    {
      if (num_bin_edges > 0)
        {
          int i;
          bin_widths = static_cast<double *>(malloc((num_bins + 1) * sizeof(double)));
          cleanup_and_set_error_if(bin_widths == nullptr, ERROR_MALLOC);

          for (i = 1; i <= num_bin_edges - 1; ++i)
            {
              *(bin_widths + i - 1) = *(bin_edges + i) - *(bin_edges + i - 1);
            }
          std::vector<double> bin_widths_vec(bin_widths, bin_widths + num_bins);
          group->setAttribute("bin_widths", bin_widths_key);
          (*context)[bin_widths_key] = bin_widths_vec;
        }
      else
        {
          bin_width = 2 * M_PI / num_bins;
          group->setAttribute("bin_width", bin_width);
        }
    }
  /* bin_width is given*/
  else
    {
      bin_width = static_cast<double>(group->getAttribute("bin_width"));

      int n = 0;
      int temp;

      if (num_bin_edges > 0 && *philim == -1.0)
        {
          int i;
          logger((stderr, "bin_width is not compatible with bin_edges\n"));
          cleanup_and_set_error(ERROR_PLOT_INCOMPATIBLE_ARGUMENTS);

          bin_widths = static_cast<double *>(malloc(num_bins * sizeof(double)));
          cleanup_and_set_error_if(bin_widths == nullptr, ERROR_MALLOC);

          for (i = 1; i <= num_bin_edges - 1; ++i)
            {
              *(bin_widths + i - 1) = *(bin_edges + i) - *(bin_edges + i - 1);
            }
          std::vector<double> bin_widths_vec(bin_widths, bin_widths + num_bins);
          group->setAttribute("bin_widths", bin_widths_key);
          (*context)[bin_widths_key] = bin_widths_vec;
        }

      /* with philim (with bin_width) */
      if (philim != nullptr)
        {
          if (bin_width < 0 || bin_width > 2 * M_PI)
            {
              logger((stderr, "bin_width must be between 0 and 2 * Pi\n"));
              cleanup_and_set_error(ERROR_PLOT_OUT_OF_RANGE);
            }
          if (philim[1] - philim[0] < bin_width)
            {
              logger((stderr, "the given philim range does not work with the given bin_width\n"));
              cleanup_and_set_error(ERROR_PLOT_INCOMPATIBLE_ARGUMENTS);
            }
          else
            {
              n = (int)((philim[1] - philim[0]) / bin_width);
              if (is_bin_counts == 1)
                {
                  if (num_bins > n)
                    {
                      logger((stderr,
                              "bin_width does not work with this specific bin_count. Nbins do not fit bin_width\n"));
                      cleanup_and_set_error(ERROR_PLOT_INCOMPATIBLE_ARGUMENTS);
                    }
                  n = num_bins;
                }
              bin_edges = bin_edges_buf = static_cast<double *>(malloc((n + 1) * sizeof(double)));
              cleanup_and_set_error_if(bin_edges == nullptr, ERROR_MALLOC);

              linspace(philim[0], n * bin_width, n + 1, bin_edges);
            }
        }
      /* without philim */
      else
        {
          if (bin_width <= 0 || bin_width > 2 * M_PI)
            {
              logger((stderr, "bin_width must be between 0 (exclusive) and 2 * Pi\n"));
              cleanup_and_set_error(ERROR_PLOT_OUT_OF_RANGE);
            }
          else if ((int)(2 * M_PI / bin_width) > 200)
            {
              n = 200;
              bin_width = 2 * M_PI / n;
            }
          n = (int)(2 * M_PI / bin_width);
          if (is_bin_counts == 1)
            {
              if (num_bins > n)
                {
                  logger(
                      (stderr, "bin_width does not work with this specific bin_count. Nbins do not fit bin_width\n"));
                  cleanup_and_set_error(ERROR_PLOT_INCOMPATIBLE_ARGUMENTS);
                }
              n = num_bins;
            }
          bin_edges = bin_edges_buf = static_cast<double *>(malloc((n + 1) * sizeof(double)));
          cleanup_and_set_error_if(bin_edges == nullptr, ERROR_MALLOC);

          linspace(0, n * bin_width, n + 1, bin_edges);
        }
      group->setAttribute("nbins", n);
      num_bin_edges = n + 1;
      num_bins = n;
      std::vector<double> bin_edges_vec(bin_edges, bin_edges + num_bin_edges);
      group->setAttribute("bin_edges", bin_edges_key);
      (*context)[bin_edges_key] = bin_edges_vec;
      group->setAttribute("bin_width", bin_width);
      bin_widths = static_cast<double *>(malloc(num_bins * sizeof(double)));
      cleanup_and_set_error_if(bin_widths == nullptr, ERROR_MALLOC);

      for (temp = 0; temp < num_bins; ++temp)
        {
          bin_widths[temp] = bin_width;
        }
      std::vector<double> bin_widths_vec(bin_widths, bin_widths + num_bins);
      group->setAttribute("bin_widths", bin_widths_key);
      (*context)[bin_widths_key] = bin_widths_vec;
    }

  /* is_bin_counts */
  if (is_bin_counts == 1)
    {
      double temp_max_bc = 0.0;
      int i;
      int total = 0;
      int j;
      int prev = 0;

      if (num_bin_edges > 0 && num_bins != num_bin_edges - 1)
        {
          logger((stderr, "Number of bin_edges must be number of bin_counts + 1\n"));
          cleanup_and_set_error(ERROR_PLOT_INCOMPATIBLE_ARGUMENTS);
        }

      /* total number of observations */
      for (i = 0; i < num_bins; ++i)
        {
          total += bin_counts[i];
        }
      for (i = 0; i < num_bins; ++i)
        {
          if (num_bin_edges > 0) bin_width = bin_widths[i];

          if (strcmp(norm, "pdf") == 0)
            {
              if (bin_counts[i] * 1.0 / (total * bin_width) > temp_max_bc)
                {
                  temp_max_bc = bin_counts[i] * 1.0 / (total * bin_width);
                }
            }
          else if (strcmp(norm, "countdensity") == 0)
            {
              if (bin_counts[i] * 1.0 / (bin_width) > temp_max_bc)
                {
                  temp_max_bc = bin_counts[i] * 1.0 / (bin_width);
                }
            }
          else
            {
              if (bin_counts[i] > temp_max_bc) temp_max_bc = bin_counts[i];
            }
        }

      /* double classes[num_bins][total]; */
      classes = static_cast<double *>(malloc((num_bins * total) * sizeof(double)));
      cleanup_and_set_error_if(classes == nullptr, ERROR_MALLOC);

      length = (int)temp_max_bc;
      p = classes;

      for (i = 0; i < (num_bins * total); ++i)
        {
          *p = -1.0;
          p++;
        }

      /* fill classes with bin counts */
      for (i = 0; i < num_bins; ++i)
        {
          p = classes + i * total;
          if (str_equals_any(norm, 2, "cdf", "cumcount"))
            {
              prev += bin_counts[i];
              for (j = 0; j < prev; ++j)
                {
                  *p = 1.0;
                  ++p;
                }
            }

          else
            {
              for (j = 0; j < bin_counts[i]; ++j)
                {
                  *p = 1.0;
                  ++p;
                }
            }
        }

      group->setAttribute("classes", classes_key);
      std::vector<double> classes_vec(classes, classes + total * num_bins);
      (*context)[classes_key] = classes_vec;

      if (strcmp(norm, "probability") == 0)
        max = temp_max_bc * 1.0 / total;
      else if (strcmp(norm, "cdf") == 0)
        max = 1.0;
      else if (strcmp(norm, "cumcount") == 0)
        max = total * 1.0;
      else
        max = temp_max_bc;
    }

  /* no is_bin_counts */
  else
    {
      int x;

      /* no bin_edges */
      if (num_bin_edges == 0)
        {
          max = 0.0;
          start = 0;
          interval = 2 * M_PI / num_bins;
          if (str_equals_any(norm, 4, "count", "pdf", "countdensity", "probability"))
            {
              /*double classes[num_bins][length];*/
              classes = static_cast<double *>(malloc(num_bins * length * sizeof(double)));
              cleanup_and_set_error_if(classes == nullptr, ERROR_MALLOC);
              p = classes;
              int i;
              for (i = 0; i < (num_bins * length); ++i)
                {
                  *p = -1.0;
                  p++;
                }
              for (x = 0; x < num_bins; ++x)
                {
                  int y;
                  p = classes + x * length;
                  temp_max = 0.0;
                  for (y = 0; y < length; ++y)
                    {
                      if (start <= theta[y] && theta[y] < (start + interval))
                        {
                          temp_max++;
                          *p = theta[y];
                          p++;
                          if (max < temp_max) max = temp_max;
                        }
                    }
                  start += interval;
                }
              std::vector<double> classes_vec(classes, classes + num_bins * length);
              group->setAttribute("classes", classes_key);
              (*context)[classes_key] = classes_vec;
            }
          else if (str_equals_any(norm, 2, "cdf", "cumcount"))
            {
              /*double classes[num_bins][length];*/
              classes = static_cast<double *>(malloc(num_bins * length * sizeof(double)));
              cleanup_and_set_error_if(classes == nullptr, ERROR_MALLOC);
              int i;
              int prev;
              p = classes;

              for (i = 0; i < (num_bins * length); ++i)
                {
                  *p = -1;
                  p++;
                }

              prev = 0;
              for (x = 0; x < num_bins; ++x)
                {
                  p = classes + x * length;

                  if (x > 0)
                    {
                      double *prev_list;
                      int y;
                      /*prev_list = classes[x - 1];*/
                      prev_list = classes + (x - 1) * length;
                      for (y = 0; y < prev; ++y)
                        {
                          *p = *prev_list;
                          p++;
                          prev_list++;
                        }
                    }
                  for (i = 0; i < length; ++i)
                    {
                      if (start <= theta[i] && theta[i] < (start + interval))
                        {
                          *p = theta[i];
                          p++;
                          prev++;
                        }
                    }
                  start += interval;
                }
              max = length;

              std::vector<double> classes_vec(classes, classes + num_bins * length);
              group->setAttribute("classes", classes_key);
              (*context)[classes_key] = classes_vec;
            }

          if (str_equals_any(norm, 2, "probability", "cdf"))
            {
              max = max / length;
            }
          else if (strcmp(norm, "pdf") == 0)
            {
              max = max / (length * bin_width);
            }
        }
      /* bin_edges */
      else
        {
          /* filter theta list */
          int filter;
          int temp = 0;
          double bin_min = *bin_edges;
          double bin_max = bin_edges[num_bin_edges - 1];
          new_theta = static_cast<double *>(malloc(length * sizeof(double)));
          cleanup_and_set_error_if(new_theta == nullptr, ERROR_MALLOC);
          for (filter = 0; filter < length; ++filter)
            {
              if (theta[filter] >= bin_min && theta[filter] < bin_max)
                {
                  new_theta[temp] = theta[filter];
                  temp++;
                }
            }
          theta = new_theta;
          length = temp;

          max = 0.0;
          if (str_equals_any(norm, 4, "count", "pdf", "countdensity", "probability"))
            {
              int a;
              /* double classes[num_bins][length]; */
              classes = static_cast<double *>(malloc(num_bins * length * sizeof(double)));
              cleanup_and_set_error_if(classes == nullptr, ERROR_MALLOC);
              p = classes;
              for (a = 0; a < (num_bins * length); ++a)
                {
                  *p = -1;
                  p++;
                }
              for (x = 0; x < num_bins; ++x)
                {
                  int b;
                  p = classes + x * length;
                  temp_max = 0.0;
                  for (b = 0; b < length; ++b)
                    {
                      if (x == num_bin_edges - 1) break;
                      if (*bin_edges <= theta[b] && theta[b] < *(bin_edges + 1))
                        {
                          temp_max++;
                          *p = theta[b];
                          p++;
                        }
                    }
                  if (strcmp(norm, "pdf") == 0)
                    {
                      temp_max /= length * *(bin_widths + x);
                    }
                  else if (strcmp(norm, "countdensity") == 0)
                    {
                      temp_max /= *(bin_widths + x);
                    }

                  if (max < temp_max) max = temp_max;

                  bin_edges++;
                }
              std::vector<double> classes_vec(classes, classes + num_bins * length);
              group->setAttribute("classes", classes_key);
              (*context)[classes_key] = classes_vec;
            }
          else if (str_equals_any(norm, 2, "cdf", "cumcount"))
            {
              int c;
              int prev;
              int d;
              classes = static_cast<double *>(malloc(num_bins * length * sizeof(double)));
              cleanup_and_set_error_if(classes == nullptr, ERROR_MALLOC);
              p = classes;
              for (c = 0; c < (num_bins * length); ++c)
                {
                  *p = -1;
                  p++;
                }

              prev = 0;
              for (d = 0; d < num_bins; ++d)
                {
                  int i;
                  p = classes + d * num_bins;

                  if (d > 0)
                    {
                      double *prev_list;
                      int y;
                      prev_list = classes + (d - 1) * num_bins;

                      for (y = 0; y < prev; ++y)
                        {
                          *p = *prev_list;
                          p++;
                          prev_list++;
                        }
                    }
                  for (i = 0; i < length; ++i)
                    {
                      if (*bin_edges <= theta[i] && theta[i] < *(bin_edges + 1))
                        {
                          *p = theta[i];
                          p++;
                          prev++;
                        }
                    }
                  bin_edges++;
                }
              max = length;
              std::vector<double> classes_vec(classes, classes + num_bins * length);
              group->setAttribute("classes", classes_key);
              (*context)[classes_key] = classes_vec;
            }

          if (str_equals_any(norm, 2, "probability", "cdf"))
            {
              max = max / length;
            }
        }
    }

  group->setAttribute("r_max", max);

cleanup:
  free(bin_edges_buf);
  free(bin_widths);
  free(classes);
  free(new_theta);
  free(new_edges);
}

static void processAttributes(const std::shared_ptr<GRM::Element> &element)
{
  /*!
   * processing function for all kinds of attributes
   *
   * \param[in] element The GRM::Element containing attributes
   */

  //! Map used for processing all kinds of attributes
  static std::map<std::string, std::function<void(const std::shared_ptr<GRM::Element> &)>> attrStringToFunc{
      {std::string("alpha"), processAlpha},
      {std::string("bordercolorind"), processBorderColorInd},
      {std::string("calc_window_and_viewport_from_parent"), processCalcWindowAndViewportFromParent},
      {std::string("charexpan"), processCharExpan},
      {std::string("charspace"), processCharSpace},
      {std::string("charup"), processCharUp},
      {std::string("clipxform"), processClipXForm},
      {std::string("colorbar_position"), processColorbarPosition},
      {std::string("colormap"), processColormap},
      {std::string("colorrep"), processColorRep},
      {std::string("fillcolorind"), processFillColorInd},
      {std::string("fillintstyle"), processFillIntStyle},
      {std::string("fillstyle"), processFillStyle},
      {std::string("flip"), processFlip},
      {std::string("font"), processFont},
      {std::string("gr_option_flip_x"), processGROptionFlipX},
      {std::string("gr_option_flip_y"), processGROptionFlipY},
      {std::string("gr3backgroundcolor"), processGR3BackgroundColor},
      {std::string("gr3cameralookat"), processGR3CameraLookAt},
      {std::string("kind"), processKind},
      {std::string("linecolorind"), processLineColorInd},
      {std::string("linespec"), processLineSpec},
      {std::string("linetype"), processLineType},
      {std::string("linewidth"), processLineWidth},
      {std::string("marginalheatmap_kind"), processMarginalheatmapKind},
      {std::string("markercolorind"), processMarkerColorInd},
      {std::string("markersize"), processMarkerSize},
      {std::string("markertype"), processMarkerType},
      {std::string("projectiontype"), processProjectionType},
      {std::string("relative_charheight"), processRelativeCharHeight},
      {std::string("resample_method"), processResampleMethod},
      {std::string("selntran"), processSelntran},
      {std::string("space"), processSpace},
      {std::string("space3d"), processSpace3d},
      {std::string("textalign"), processTextAlign},
      {std::string("textcolorind"), processTextColorInd},
      {std::string("textencoding"), processTextEncoding},
      {std::string("textfontprec"), processTextFontPrec},
      {std::string("textpath"), processTextPath},
      {std::string("title"), processTitle},
      {std::string("wsviewport"), processWSViewport},
      {std::string("wswindow"), processWSWindow},
      {std::string("xlabel"), processXlabel},
      {std::string("xticklabels"), processXTickLabels},
      {std::string("ylabel"), processYlabel},
  };

  static std::map<std::string, std::function<void(const std::shared_ptr<GRM::Element> &)>> attrStringToFuncPost{
      /* This map contains functions for attributes that should be called after some attributes have been processed
       * already. These functions can contain e.g. inquire function calls for colors.
       * */
      {std::string("transparency"), processTransparency},
      {std::string("set_text_color_for_background"), processTextColorForBackground},
  };

  static std::map<std::string, std::function<void(const std::shared_ptr<GRM::Element> &)>> attrStringToFuncPre{
      /* This map contains functions for attributes that should be called before other attributes of the element are
       * being processed. The attributes are being processed in the order of their appearance in the map.
       * */
      {std::string("subplot"), processSubplot},
      {std::string("viewport"), GRM::Render::processViewport},
      {std::string("charheight"), processCharHeight},
      {std::string("limits"), GRM::Render::processLimits},
      {std::string("window"), processWindow},
      {std::string("window3d"), processWindow3d}, /* needs to be set before space3d is processed */
      {std::string("scale"), processScale},       /* needs to be set before flip is processed */
      {std::string("polar_histogram_classes"), processClassesPolarHistogram},

  };

  for (auto attributeToFunctionPair : attrStringToFuncPre)
    /*
     * Pre process attribute run
     */
    {
      if (element->getAttributeNames().find(attributeToFunctionPair.first) != element->getAttributeNames().end())
        {
          attrStringToFuncPre[attributeToFunctionPair.first](element);
        }
    }

  for (auto attribute : element->getAttributeNames())
    {
      auto start = 0U;
      auto end = attribute.find('$');
      if (end != std::string::npos)
        {
          attribute = attribute.substr(start, end);
        }
      if (attrStringToFunc.find(attribute) != attrStringToFunc.end())
        {
          attrStringToFunc[attribute](element);
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

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ element processing functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static void axes(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for axes
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
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

static void axes3d(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for axes3d
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
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

static void cellArray(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for cellArray
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
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
               (int *)&(GRM::get<std::vector<int>>((*context)[color])[0]));
}

static void colorbar(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  double c_min, c_max;
  int data, i;
  int options;
  unsigned int colors = static_cast<int>(element->getAttribute("colors"));

  if (!getLimitsForColorbar(element, c_min, c_max))
    {
      auto subplot_element = getSubplotElement(element);
      if (!getLimitsForColorbar(subplot_element, c_min, c_max))
        {
          throw NotFoundError("Missing limits\n");
        }
    }

  gr_setwindow(0.0, 1.0, c_min, c_max);

  /* remove old cell arrays and axes if they exist */
  auto colorbar_elements = element->querySelectorsAll("[name=\"colorbar\"]");
  for (auto &colorbar_element : colorbar_elements)
    {
      colorbar_element->remove();
    }

  /* create cell array */
  std::vector<int> data_vec;
  for (i = 0; i < colors; ++i)
    {
      data = 1000 + (int)((255.0 * i) / (colors - 1) + 0.5);
      data_vec.push_back(data);
    }
  int id = (int)global_root->getAttribute("id");
  std::string str = std::to_string(id);
  global_root->setAttribute("id", id + 1);

  auto cellArray =
      global_render->createCellArray(0, 1, c_max, c_min, 1, colors, 1, 1, 1, colors, "data" + str, data_vec);
  cellArray->setAttribute("name", "colorbar");
  element->append(cellArray);

  /* create axes */
  gr_inqscale(&options);
  std::shared_ptr<GRM::Element> axes;
  if (options & GR_OPTION_Z_LOG)
    {
      axes = global_render->createAxes(0, 2, 1, c_min, 0, 1, 1);
      global_render->setScale(axes, GR_OPTION_Y_LOG);
    }
  else
    {
      double c_tick = auto_tick(c_min, c_max);
      axes = global_render->createAxes(0, c_tick, 1, c_min, 0, 1, 1);
    }
  axes->setAttribute("tick_size", 0.005);
  axes->setAttribute("name", "colorbar");
  global_render->setLineColorInd(axes, 1);
  element->append(axes);
}

static void contour(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for contour
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double z_min, z_max;
  int num_levels = PLOT_DEFAULT_CONTOUR_LEVELS;
  int i;
  unsigned int x_length, y_length, z_length;
  std::vector<double> x_vec, y_vec, z_vec;
  std::vector<double> px_vec, py_vec, pz_vec;
  int major_h = 1000;

  z_min = static_cast<double>(element->parentElement()->getAttribute("lim_zmin"));
  z_max = static_cast<double>(element->parentElement()->getAttribute("lim_zmax"));
  if (element->hasAttribute("levels"))
    {
      num_levels = static_cast<int>(element->getAttribute("levels"));
    }
  else
    {
      element->setAttribute("levels", num_levels);
    }

  gr_setprojectiontype(0);
  gr_setspace(z_min, z_max, 0, 90);

  std::vector<double> h(num_levels);

  if (!element->hasAttribute("px") || !element->hasAttribute("py") || !element->hasAttribute("pz"))
    {
      if (!element->hasAttribute("x")) throw NotFoundError("Contour series is missing required attribute x-data.\n");
      auto x = static_cast<std::string>(element->getAttribute("x"));
      if (!element->hasAttribute("y")) throw NotFoundError("Contour series is missing required attribute y-data.\n");
      auto y = static_cast<std::string>(element->getAttribute("y"));
      if (!element->hasAttribute("z")) throw NotFoundError("Contour series is missing required attribute z-data.\n");
      auto z = static_cast<std::string>(element->getAttribute("z"));

      x_vec = GRM::get<std::vector<double>>((*context)[x]);
      y_vec = GRM::get<std::vector<double>>((*context)[y]);
      z_vec = GRM::get<std::vector<double>>((*context)[z]);
      x_length = x_vec.size();
      y_length = y_vec.size();
      z_length = z_vec.size();

      int id = (int)global_root->getAttribute("id");
      global_root->setAttribute("id", id + 1);
      std::string str = std::to_string(id);

      if (x_length == y_length && x_length == z_length)
        {
          std::vector<double> gridit_x_vec(PLOT_CONTOUR_GRIDIT_N);
          std::vector<double> gridit_y_vec(PLOT_CONTOUR_GRIDIT_N);
          std::vector<double> gridit_z_vec(PLOT_CONTOUR_GRIDIT_N * PLOT_CONTOUR_GRIDIT_N);

          double *gridit_x = &(gridit_x_vec[0]);
          double *gridit_y = &(gridit_y_vec[0]);
          double *gridit_z = &(gridit_z_vec[0]);
          double *x_p = &(x_vec[0]);
          double *y_p = &(y_vec[0]);
          double *z_p = &(z_vec[0]);

          gr_gridit(x_length, x_p, y_p, z_p, PLOT_CONTOUR_GRIDIT_N, PLOT_CONTOUR_GRIDIT_N, gridit_x, gridit_y,
                    gridit_z);
          for (i = 0; i < PLOT_CONTOUR_GRIDIT_N * PLOT_CONTOUR_GRIDIT_N; i++)
            {
              z_min = grm_min(gridit_z[i], z_min);
              z_max = grm_max(gridit_z[i], z_max);
            }

          global_render->setSpace(element->parentElement(), z_min, z_max, 0, 90);
          processSpace(element->parentElement());

          px_vec = std::vector<double>(gridit_x, gridit_x + PLOT_CONTOUR_GRIDIT_N);
          py_vec = std::vector<double>(gridit_y, gridit_y + PLOT_CONTOUR_GRIDIT_N);
          pz_vec = std::vector<double>(gridit_z, gridit_z + PLOT_CONTOUR_GRIDIT_N * PLOT_CONTOUR_GRIDIT_N);
        }
      else
        {
          if (x_length * y_length != z_length)
            throw std::length_error("For contour series x_length * y_length must be z_length.\n");

          px_vec = x_vec;
          py_vec = y_vec;
          pz_vec = z_vec;
        }

      (*context)["px" + str] = px_vec;
      element->setAttribute("px", "px" + str);
      (*context)["py" + str] = py_vec;
      element->setAttribute("py", "py" + str);
      (*context)["pz" + str] = pz_vec;
      element->setAttribute("pz", "pz" + str);
    }
  else
    {
      auto px = static_cast<std::string>(element->getAttribute("px"));
      auto py = static_cast<std::string>(element->getAttribute("py"));
      auto pz = static_cast<std::string>(element->getAttribute("pz"));

      px_vec = GRM::get<std::vector<double>>((*context)[px]);
      py_vec = GRM::get<std::vector<double>>((*context)[py]);
      pz_vec = GRM::get<std::vector<double>>((*context)[pz]);
    }

  for (i = 0; i < num_levels; ++i)
    {
      h[i] = z_min + (1.0 * i) / num_levels * (z_max - z_min);
    }

  int nx = px_vec.size();
  int ny = py_vec.size();

  double *px_p = &(px_vec[0]);
  double *py_p = &(py_vec[0]);
  double *h_p = &(h[0]);
  double *pz_p = &(pz_vec[0]);

  gr_contour(nx, ny, num_levels, px_p, py_p, h_p, pz_p, major_h);
}

static void contourf(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for contourf
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double z_min, z_max;
  int num_levels = PLOT_DEFAULT_CONTOUR_LEVELS;
  int i;
  unsigned int x_length, y_length, z_length;
  std::vector<double> x_vec, y_vec, z_vec;
  std::vector<double> px_vec, py_vec, pz_vec;
  int major_h = 0;

  z_min = static_cast<double>(element->parentElement()->getAttribute("lim_zmin"));
  z_max = static_cast<double>(element->parentElement()->getAttribute("lim_zmax"));
  if (element->hasAttribute("levels"))
    {
      num_levels = static_cast<int>(element->getAttribute("levels"));
    }
  else
    {
      element->setAttribute("levels", num_levels);
    }

  global_render->setProjectionType(element->parentElement(), 0);
  global_render->setSpace(element->parentElement(), z_min, z_max, 0, 90);

  std::vector<double> h(num_levels);

  if (!element->hasAttribute("px") || !element->hasAttribute("py") || !element->hasAttribute("pz"))
    {
      if (!element->hasAttribute("x")) throw NotFoundError("Contourf series is missing required attribute x-data.\n");
      auto x = static_cast<std::string>(element->getAttribute("x"));
      if (!element->hasAttribute("y")) throw NotFoundError("Contourf series is missing required attribute y-data.\n");
      auto y = static_cast<std::string>(element->getAttribute("y"));
      if (!element->hasAttribute("z")) throw NotFoundError("Contourf series is missing required attribute z-data.\n");
      auto z = static_cast<std::string>(element->getAttribute("z"));

      x_vec = GRM::get<std::vector<double>>((*context)[x]);
      y_vec = GRM::get<std::vector<double>>((*context)[y]);
      z_vec = GRM::get<std::vector<double>>((*context)[z]);
      x_length = x_vec.size();
      y_length = y_vec.size();
      z_length = z_vec.size();

      int id = (int)global_root->getAttribute("id");
      global_root->setAttribute("id", id + 1);
      std::string str = std::to_string(id);

      gr_setlinecolorind(1);
      global_render->setLineColorInd(element, 1);

      if (x_length == y_length && x_length == z_length)
        {
          std::vector<double> gridit_x_vec(PLOT_CONTOUR_GRIDIT_N);
          std::vector<double> gridit_y_vec(PLOT_CONTOUR_GRIDIT_N);
          std::vector<double> gridit_z_vec(PLOT_CONTOUR_GRIDIT_N * PLOT_CONTOUR_GRIDIT_N);

          double *gridit_x = &(gridit_x_vec[0]);
          double *gridit_y = &(gridit_y_vec[0]);
          double *gridit_z = &(gridit_z_vec[0]);
          double *x_p = &(x_vec[0]);
          double *y_p = &(y_vec[0]);
          double *z_p = &(z_vec[0]);

          gr_gridit(x_length, x_p, y_p, z_p, PLOT_CONTOUR_GRIDIT_N, PLOT_CONTOUR_GRIDIT_N, gridit_x, gridit_y,
                    gridit_z);
          for (i = 0; i < PLOT_CONTOUR_GRIDIT_N * PLOT_CONTOUR_GRIDIT_N; i++)
            {
              z_min = grm_min(gridit_z[i], z_min);
              z_max = grm_max(gridit_z[i], z_max);
            }

          global_render->setLineColorInd(element, 989);

          px_vec = std::vector<double>(gridit_x, gridit_x + PLOT_CONTOUR_GRIDIT_N);
          py_vec = std::vector<double>(gridit_y, gridit_y + PLOT_CONTOUR_GRIDIT_N);
          pz_vec = std::vector<double>(gridit_z, gridit_z + PLOT_CONTOUR_GRIDIT_N * PLOT_CONTOUR_GRIDIT_N);
        }
      else
        {
          if (x_length * y_length != z_length)
            throw std::length_error("For contourf series x_length * y_length must be z_length.\n");

          global_render->setLineColorInd(element, 989);

          px_vec = x_vec;
          py_vec = y_vec;
          pz_vec = z_vec;
        }

      (*context)["px" + str] = px_vec;
      element->setAttribute("px", "px" + str);
      (*context)["py" + str] = py_vec;
      element->setAttribute("py", "py" + str);
      (*context)["pz" + str] = pz_vec;
      element->setAttribute("pz", "pz" + str);
      processLineColorInd(element);
    }
  else
    {
      auto px = static_cast<std::string>(element->getAttribute("px"));
      auto py = static_cast<std::string>(element->getAttribute("py"));
      auto pz = static_cast<std::string>(element->getAttribute("pz"));

      px_vec = GRM::get<std::vector<double>>((*context)[px]);
      py_vec = GRM::get<std::vector<double>>((*context)[py]);
      pz_vec = GRM::get<std::vector<double>>((*context)[pz]);
    }

  for (i = 0; i < num_levels; ++i)
    {
      h[i] = z_min + (1.0 * i) / num_levels * (z_max - z_min);
    }

  int nx = px_vec.size();
  int ny = py_vec.size();

  double *px_p = &(px_vec[0]);
  double *py_p = &(py_vec[0]);
  double *h_p = &(h[0]);
  double *pz_p = &(pz_vec[0]);

  gr_contourf(nx, ny, num_levels, px_p, py_p, h_p, pz_p, major_h);
}

static void drawArc(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for drawArc
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double xmin = static_cast<double>(element->getAttribute("xmin"));
  double xmax = static_cast<double>(element->getAttribute("xmax"));
  double ymin = static_cast<double>(element->getAttribute("ymin"));
  double ymax = static_cast<double>(element->getAttribute("ymax"));
  double a1 = static_cast<double>(element->getAttribute("a1"));
  double a2 = static_cast<double>(element->getAttribute("a2"));
  gr_drawarc(xmin, xmax, ymin, ymax, a1, a2);
}

static void drawGraphics(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  auto key = (std::string)element->getAttribute("data");
  auto data_vec = GRM::get<std::vector<int>>((*context)[key]);

  std::vector<char> char_vec;
  char_vec.reserve(data_vec.size());
  for (int i : data_vec)
    {
      char_vec.push_back((char)i);
    }
  char *data_p = &(char_vec[0]);

  gr_drawgraphics(data_p);
}

static void drawImage(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for drawImage
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double xmin = static_cast<double>(element->getAttribute("xmin"));
  double xmax = static_cast<double>(element->getAttribute("xmax"));
  double ymin = static_cast<double>(element->getAttribute("ymin"));
  double ymax = static_cast<double>(element->getAttribute("ymax"));
  int width = static_cast<int>(element->getAttribute("width"));
  int height = static_cast<int>(element->getAttribute("height"));
  int model = static_cast<int>(element->getAttribute("model"));
  auto data = static_cast<std::string>(element->getAttribute("data"));
  gr_drawimage(xmin, xmax, ymax, ymin, width, height, (int *)&(GRM::get<std::vector<int>>((*context)[data])[0]), model);
}

static void errorbars(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  std::string orientation = PLOT_DEFAULT_ORIENTATION, kind;
  int is_horizontal;
  std::vector<double> absolute_upwards_vec, absolute_downwards_vec, relative_upwards_vec, relative_downwards_vec;
  std::string absolute_upwards, absolute_downwards, relative_upwards, relative_downwards;
  double absolute_upwards_flt, relative_upwards_flt, absolute_downwards_flt, relative_downwards_flt;
  unsigned int upwards_length, downwards_length, i;
  int scale_options, color_upwardscap, color_downwardscap, color_errorbar;
  double marker_size, xmin, xmax, ymin, ymax, tick, a, b, e_upwards, e_downwards, x_value;
  double line_x[2], line_y[2];
  std::vector<double> x_vec, y_vec;
  unsigned int x_length;
  std::string x, y;
  std::shared_ptr<GRM::Element> series;

  absolute_upwards_flt = absolute_downwards_flt = relative_upwards_flt = relative_downwards_flt = FLT_MAX;
  if (static_cast<std::string>(element->parentElement()->parentElement()->localName()) == "plot")
    {
      series = element->parentElement();
    }
  else
    {
      series = element->parentElement()->parentElement(); // marginalheatmap
    }

  if (!element->hasAttribute("x")) throw NotFoundError("Errorbars are missing required attribute x-data.\n");
  x = static_cast<std::string>(element->getAttribute("x"));
  if (!element->hasAttribute("y")) throw NotFoundError("Errorbars are missing required attribute y-data.\n");
  y = static_cast<std::string>(element->getAttribute("y"));

  x_vec = GRM::get<std::vector<double>>((*context)[x]);
  y_vec = GRM::get<std::vector<double>>((*context)[y]);
  x_length = x_vec.size();
  kind = static_cast<std::string>(series->parentElement()->getAttribute("kind"));
  if (element->hasAttribute("orientation"))
    {
      orientation = static_cast<std::string>(element->getAttribute("orientation"));
    }
  else
    {
      element->setAttribute("orientation", orientation);
    }

  if (!element->hasAttribute("absolute_downwards") && !element->hasAttribute("relative_downwards"))
    throw NotFoundError("Errorbars are missing required attribute downwards.\n");
  if (!element->hasAttribute("absolute_upwards") && !element->hasAttribute("relative_upwards"))
    throw NotFoundError("Errorbars are missing required attribute upwards.\n");
  if (element->hasAttribute("absolute_downwards"))
    {
      absolute_downwards = static_cast<std::string>(element->getAttribute("absolute_downwards"));
      absolute_downwards_vec = GRM::get<std::vector<double>>((*context)[absolute_downwards]);
      downwards_length = absolute_downwards_vec.size();
    }
  if (element->hasAttribute("relative_downwards"))
    {
      relative_downwards = static_cast<std::string>(element->getAttribute("relative_downwards"));
      relative_downwards_vec = GRM::get<std::vector<double>>((*context)[relative_downwards]);
      downwards_length = absolute_downwards_vec.size();
    }
  if (element->hasAttribute("absolute_upwards"))
    {
      absolute_upwards = static_cast<std::string>(element->getAttribute("absolute_upwards"));
      absolute_upwards_vec = GRM::get<std::vector<double>>((*context)[absolute_upwards]);
      upwards_length = absolute_upwards_vec.size();
    }
  if (element->hasAttribute("relative_upwards"))
    {
      relative_upwards = static_cast<std::string>(element->getAttribute("relative_upwards"));
      relative_upwards_vec = GRM::get<std::vector<double>>((*context)[relative_upwards]);
      upwards_length = absolute_upwards_vec.size();
    }
  if (!element->hasAttribute("absolute_downwards_flt"))
    throw NotFoundError("Errorbars are missing required attribute absolute_downwards_flt.\n");
  absolute_downwards_flt = static_cast<double>(element->getAttribute("absolute_downwards_flt"));
  if (!element->hasAttribute("absolute_upwards_flt"))
    throw NotFoundError("Errorbars are missing required attribute absolute_upwards_flt.\n");
  absolute_upwards_flt = static_cast<double>(element->getAttribute("absolute_upwards_flt"));
  if (!element->hasAttribute("relative_downwards_flt"))
    throw NotFoundError("Errorbars are missing required attribute relative_downwards_flt.\n");
  relative_downwards_flt = static_cast<double>(element->getAttribute("relative_downwards_flt"));
  if (!element->hasAttribute("relative_upwards_flt"))
    throw NotFoundError("Errorbars are missing required attribute relative_upwards_flt.\n");
  relative_upwards_flt = static_cast<double>(element->getAttribute("relative_upwards_flt"));

  is_horizontal = orientation == "horizontal";

  /* Getting GRM options and sizes. See gr_verrorbars. */
  gr_savestate();
  gr_inqmarkersize(&marker_size);
  gr_inqwindow(&xmin, &xmax, &ymin, &ymax);
  gr_inqscale(&scale_options);
  tick = marker_size * 0.0075 * (xmax - xmin);
  a = (xmax - xmin) / log10(xmax / xmin);
  b = xmin - a * log10(xmin);

  gr_inqlinecolorind(&color_errorbar);
  color_upwardscap = color_downwardscap = color_errorbar;
  if (element->hasAttribute("upwardscap_color"))
    color_upwardscap = static_cast<int>(element->getAttribute("upwardscap_color"));
  if (element->hasAttribute("downwardscap_color"))
    color_downwardscap = static_cast<int>(element->getAttribute("downwardscap_color"));
  if (element->hasAttribute("errorbar_color"))
    color_errorbar = static_cast<int>(element->getAttribute("errorbar_color"));

  //   clear old lines
  for (auto elem : element->children())
    {
      if (elem->localName() == "polyline") elem->remove();
    }

  /* Actual drawing of bars */
  e_upwards = e_downwards = FLT_MAX;
  for (i = 0; i < x_length; i++)
    {
      if (!absolute_upwards.empty() || !relative_upwards.empty() || absolute_upwards_flt != FLT_MAX ||
          relative_upwards_flt != FLT_MAX)
        {
          e_upwards = y_vec[i] * (1. + (!relative_upwards.empty()
                                            ? relative_upwards_vec[i]
                                            : (relative_upwards_flt != FLT_MAX ? relative_upwards_flt : 0))) +
                      (!absolute_upwards.empty() ? absolute_upwards_vec[i]
                                                 : (absolute_upwards_flt != FLT_MAX ? absolute_upwards_flt : 0.));
        }

      if (!absolute_downwards.empty() || !relative_downwards.empty() || absolute_downwards_flt != FLT_MAX ||
          relative_downwards_flt != FLT_MAX)
        {
          e_downwards =
              y_vec[i] * (1. - (!relative_downwards.empty()
                                    ? relative_downwards_vec[i]
                                    : (relative_downwards_flt != FLT_MAX ? relative_downwards_flt : 0))) -
              (!absolute_downwards.empty() ? absolute_downwards_vec[i]
                                           : (absolute_downwards_flt != FLT_MAX ? absolute_downwards_flt : 0.));
        }

      /* See gr_verrorbars for reference */
      x_value = x_vec[i];
      line_x[0] = X_LOG(X_LIN(x_value - tick, scale_options, xmin, xmax, a, b), scale_options, xmin, xmax, a, b);
      line_x[1] = X_LOG(X_LIN(x_value + tick, scale_options, xmin, xmax, a, b), scale_options, xmin, xmax, a, b);
      if (e_upwards != FLT_MAX && color_upwardscap >= 0)
        {
          line_y[0] = e_upwards;
          line_y[1] = e_upwards;
          if (is_horizontal)
            {
              element->append(
                  global_render->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1], 0, 0.0, color_upwardscap));
            }
          else
            {
              element->append(
                  global_render->createPolyline(line_y[0], line_y[1], line_x[0], line_x[1], 0, 0.0, color_upwardscap));
            }
        }

      if (e_downwards != FLT_MAX && color_downwardscap >= 0)
        {
          line_y[0] = e_downwards;
          line_y[1] = e_downwards;
          if (is_horizontal)
            {
              element->append(global_render->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1], 0, 0.0,
                                                            color_downwardscap));
            }
          else
            {
              element->append(global_render->createPolyline(line_y[0], line_y[1], line_x[0], line_x[1], 0, 0.0,
                                                            color_downwardscap));
            }
        }

      if (color_errorbar >= 0)
        {
          line_x[0] = x_value;
          line_x[1] = x_value;
          line_y[0] = e_upwards != FLT_MAX ? e_upwards : y_vec[i];
          line_y[1] = e_downwards != FLT_MAX ? e_downwards : y_vec[i];
          if (is_horizontal)
            {
              element->append(
                  global_render->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1], 0, 0.0, color_errorbar));
            }
          else
            {
              element->append(
                  global_render->createPolyline(line_y[0], line_y[1], line_x[0], line_x[1], 0, 0.0, color_errorbar));
            }
        }
    }
  gr_restorestate();
}

static void isosurface(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  std::vector<double> c_vec, temp_colors;
  unsigned int i, c_length, dims;
  int strides[3];
  double c_min, c_max, isovalue;
  float foreground_colors[3];

  if (!element->hasAttribute("c")) throw NotFoundError("Isosurface series is missing required attribute c-data.\n");
  auto c = static_cast<std::string>(element->getAttribute("c"));
  c_vec = GRM::get<std::vector<double>>((*context)[c]);
  c_length = c_vec.size();

  if (!element->hasAttribute("c_dims"))
    throw NotFoundError("Isosurface series is missing required attribute c_dims.\n");
  auto c_dims = static_cast<std::string>(element->getAttribute("c_dims"));
  auto shape_vec = GRM::get<std::vector<int>>((*context)[c_dims]);
  dims = shape_vec.size();

  if (dims != 3) throw std::length_error("For isosurface series the size of c_dims has to be 3.\n");
  if (shape_vec[0] * shape_vec[1] * shape_vec[2] != c_length)
    throw std::length_error("For isosurface series shape[0] * shape[1] * shape[2] must be c_length.\n");
  if (c_length <= 0) throw NotFoundError("For isosurface series the size of c has to be greater than 0.\n");

  isovalue = 0.5;
  foreground_colors[0] = 0.0;
  foreground_colors[1] = 0.5;
  foreground_colors[2] = 0.8;
  if (element->hasAttribute("isovalue")) isovalue = static_cast<double>(element->getAttribute("isovalue"));
  element->setAttribute("isovalue", isovalue);
  /*
   * We need to convert the double values to floats, as GR3 expects floats, but an argument can only contain
   * doubles.
   */
  if (element->hasAttribute("foreground_color"))
    {
      auto temp_c = static_cast<std::string>(element->getAttribute("foreground_color"));
      temp_colors = GRM::get<std::vector<double>>((*context)[temp_c]);
      i = temp_colors.size();
      if (i != 3) throw std::length_error("For isosurface series the foreground colors must have size 3.\n");
      while (i-- > 0)
        {
          foreground_colors[i] = (float)temp_colors[i];
        }
    }
  logger((stderr, "Colors; %f %f %f\n", foreground_colors[0], foreground_colors[1], foreground_colors[2]));

  /* Check if any value is finite in array, also calculation of real min and max */
  c_min = c_max = c_vec[0];
  for (i = 0; i < c_length; ++i)
    {
      if (std::isfinite(c_vec[i]))
        {
          if (grm_isnan(c_min) || c_min > c_vec[i])
            {
              c_min = c_vec[i];
            }
          if (grm_isnan(c_max) || c_max < c_vec[i])
            {
              c_max = c_vec[i];
            }
        }
    }
  if (c_min == c_max || !std::isfinite(c_min) || !std::isfinite(c_max))
    throw NotFoundError("For isosurface series the given c-data isn't enough.\n");

  logger((stderr, "c_min %lf c_max %lf isovalue %lf\n ", c_min, c_max, isovalue));
  std::vector<float> conv_data(c_vec.begin(), c_vec.end());

  strides[0] = shape_vec[1] * shape_vec[2];
  strides[1] = shape_vec[2];
  strides[2] = 1;

  global_render->setGR3LightParameters(element, 0.2, 0.8, 0.7, 128);

  float ambient = static_cast<double>(element->getAttribute("ambient"));
  float diffuse = static_cast<double>(element->getAttribute("diffuse"));
  float specular = static_cast<double>(element->getAttribute("specular"));
  float specular_power = static_cast<double>(element->getAttribute("specular_power"));

  float *data = &(conv_data[0]);
  float light_parameters[4];

  gr3_clear();

  /* Save and restore original light parameters */
  gr3_getlightparameters(&light_parameters[0], &light_parameters[1], &light_parameters[2], &light_parameters[3]);
  gr3_setlightparameters(ambient, diffuse, specular, specular_power);

  gr3_isosurface(shape_vec[0], shape_vec[1], shape_vec[2], data, (float)isovalue, foreground_colors, strides);

  gr3_setlightparameters(light_parameters[0], light_parameters[1], light_parameters[2], light_parameters[3]);
}

static void legend(const std::shared_ptr<GRM::Element> &elem, const std::shared_ptr<GRM::Context> &context)
{
  double viewport[4];
  double px, py, w, h;
  double tbx[4], tby[4];
  std::shared_ptr<GRM::Render> render;
  std::string labels_key = static_cast<std::string>(elem->getAttribute("labels"));
  auto labels = GRM::get<std::vector<std::string>>((*context)[labels_key]);

  render = std::dynamic_pointer_cast<GRM::Render>(elem->ownerDocument());
  if (!render)
    {
      throw NotFoundError("No render-document found for element\n");
    }

  gr_inqviewport(&viewport[0], &viewport[1], &viewport[2], &viewport[3]);

  if (static_cast<std::string>(elem->parentElement()->getAttribute("kind")) != "pie")
    {
      int location = PLOT_DEFAULT_LOCATION;
      double legend_symbol_x[2], legend_symbol_y[2];
      int i;

      gr_savestate();

      auto specs_key = static_cast<std::string>(elem->getAttribute("specs"));
      std::vector<std::string> specs = GRM::get<std::vector<std::string>>((*context)[specs_key]);
      if (elem->hasAttribute("location"))
        {
          location = static_cast<int>(elem->getAttribute("location"));
        }
      else
        {
          elem->setAttribute("location", location);
        }

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
              child->remove();
            }
        }

      gr_selntran(1);

      render->setSelntran(elem, 0);
      render->setScale(elem, 0);

      auto fr = render->createFillRect(px - 0.08, px + w + 0.02, py + 0.03, py - h);
      elem->append(fr);

      render->setFillIntStyle(elem, GKS_K_INTSTYLE_SOLID);
      render->setFillColorInd(elem, 0);

      auto dr = render->createDrawRect(px - 0.08, px + w + 0.02, py + 0.03, py - h);
      elem->append(dr);

      render->setLineType(dr, GKS_K_INTSTYLE_SOLID);
      render->setLineColorInd(dr, 1);
      render->setLineWidth(dr, 1);

      i = 0;
      render->setLineSpec(elem, const_cast<char *>(" "));

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
              auto pl = render->createPolyline(legend_symbol_x[0], legend_symbol_x[1], legend_symbol_y[0],
                                               legend_symbol_y[1]);
              elem->append(pl);
              render->setLineSpec(pl, spec);
            }
          if (mask & 2)
            {
              legend_symbol_x[0] = px - 0.06;
              legend_symbol_x[1] = px - 0.02;
              legend_symbol_y[0] = py;
              legend_symbol_y[1] = py;
              auto pl = render->createPolyline(legend_symbol_x[0], legend_symbol_x[1], legend_symbol_y[0],
                                               legend_symbol_y[1]);
              elem->append(pl);
              render->setLineSpec(pl, spec);
            }
          if (i < labels.size())
            {
              auto tx = render->createText(px, py, labels[i].data());
              elem->append(tx);
              render->setTextAlign(tx, GKS_K_TEXT_HALIGN_LEFT, GKS_K_TEXT_VALIGN_HALF);
              py -= 0.5 * dy;
              i += 1;
            }
          py -= 0.03;
        }
      gr_restorestate();

      processLineSpec(elem);
    }
  else
    {
      unsigned int num_labels = labels.size();
      std::string color_indices_key;
      std::string color_rgb_values_key;

      std::vector<int> color_indices_vec;
      std::vector<double> color_rgb_values_vec;

      if (elem->hasChildNodes())
        {
          for (const auto &child : elem->children())
            {
              child->remove();
            }
        }

      for (const auto &child : elem->parentElement()->children())
        {
          if (child->localName() == "series")
            {
              if (child->hasAttribute("color_indices"))
                {
                  color_indices_key = static_cast<std::string>(child->getAttribute("color_indices"));
                  color_indices_vec = GRM::get<std::vector<int>>((*context)[color_indices_key]);
                }
              else if (child->hasAttribute("color_rgb_values"))
                {
                  color_rgb_values_key = static_cast<std::string>(child->getAttribute("color_rgb_values"));
                  color_rgb_values_vec = GRM::get<std::vector<double>>((*context)[color_rgb_values_key]);
                }
              break;
            }
        }

      gr_selntran(1);

      render->setSelntran(elem, 0);
      render->setScale(elem, 0);
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
      elem->append(fr);
      render->setFillIntStyle(elem, GKS_K_INTSTYLE_SOLID);
      render->setFillColorInd(elem, 0);
      auto dr = render->createDrawRect(px - 0.02, px + w + 0.02, py - 0.5 * h - 0.02, py + 0.5 * h + 0.02);
      elem->append(dr);
      render->setLineType(elem, GKS_K_INTSTYLE_SOLID);
      render->setLineColorInd(elem, 1);
      render->setLineWidth(elem, 1);

      auto subsubGroup = render->createGroup("labels_group");
      elem->append(subsubGroup);
      render->setLineSpec(subsubGroup, const_cast<char *>(" "));
      render->setTextAlign(subsubGroup, GKS_K_TEXT_HALIGN_LEFT, GKS_K_TEXT_VALIGN_HALF);
      // reset setNextColor
      setNextColor(GR_COLOR_RESET, color_indices_vec, color_rgb_values_vec, elem);

      for (auto &current_label : labels)
        {
          auto labelGroup = render->createElement("label");
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
      setNextColor(GR_COLOR_RESET, color_indices_vec, color_rgb_values_vec, elem);

      processLineColorInd(elem);
      processLineWidth(elem);
      processLineType(elem);
    }
  processSelntran(elem);
  processScale(elem);
  processFillIntStyle(elem);
  processFillColorInd(elem);
}

static void drawPolarAxes(const std::shared_ptr<GRM::Element> &elem, const std::shared_ptr<GRM::Context> &context)
{
  double viewport[4], vp[4];
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

  std::shared_ptr<GRM::Render> render;

  gr_inqviewport(&viewport[0], &viewport[1], &viewport[2], &viewport[3]);
  auto subplot = getSubplotElement(elem);

  render = std::dynamic_pointer_cast<GRM::Render>(elem->ownerDocument());
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

  r_min = static_cast<double>(subplot->getAttribute("lim_ymin"));
  r_max = static_cast<double>(subplot->getAttribute("lim_ymax"));

  angle_ticks = static_cast<int>(elem->getAttribute("angle_ticks"));

  kind = static_cast<std::string>(subplotElement->getAttribute("kind"));

  render->setCharHeight(newGroup, charheight);
  render->setLineType(newGroup, GKS_K_LINETYPE_SOLID);

  if (kind == "polar_histogram")
    {
      // polar_histogram: needed attributes are calculated prior to drawPolarAxes in parent
      rings = static_cast<int>(subplotElement->getAttribute("rings"));
      r_min = 0.0;

      r_max = static_cast<double>(subplotElement->getAttribute("r_max"));
      norm = static_cast<std::string>(elem->getAttribute("norm"));
      tick = static_cast<double>(subplotElement->getAttribute("tick"));
    }
  else
    {
      rings = grm_max(4, (int)(r_max - r_min));
      elem->setAttribute("rings", rings);

      if (elem->hasAttribute("tick"))
        {
          tick = static_cast<double>(elem->getAttribute("tick"));
        }
      else
        {
          tick = auto_tick(r_min, r_max);
        }
    }

  /* delete old polar_axes_elements */
  auto polar_axes_elements = elem->querySelectorsAll("[name=\"polar_axes\"]");
  for (auto &polar_axes_element : polar_axes_elements)
    {
      polar_axes_element->remove();
    }

  n = rings;
  phiflip = static_cast<int>(elem->getAttribute("phiflip"));

  // Draw rings
  for (i = 0; i <= n; i++)
    {
      double r = 1.0 / n * i;
      if (i % 2 == 0)
        {
          if (i > 0)
            {
              auto temp = render->createDrawArc(-r, r, -r, r, 0, 360);
              temp->setAttribute("name", "polar_axes");
              newGroup->append(temp);
              render->setLineColorInd(temp, 88);
            }
          //          render->setTextAlign(newGroup, GKS_K_TEXT_HALIGN_LEFT, GKS_K_TEXT_VALIGN_HALF);
          //          x[0] = 0.05;
          //          y[0] = r;
          //          snprintf(text_buffer, PLOT_POLAR_AXES_TEXT_BUFFER, "%.1lf", r_min + tick * i);
          //          auto temp = render->createText(x[0], y[0], text_buffer, CoordinateSpace::WC);
          //          temp->setAttribute("name", "polar_axes");
          //          newGroup->append(temp);
        }
      else
        {
          auto temp = render->createDrawArc(-r, r, -r, r, 0, 360);
          temp->setAttribute("name", "polar_axes");
          newGroup->append(temp);
          render->setLineColorInd(temp, 90);
        }
    }
  // Draw sectorlines
  interval = 360.0 / angle_ticks;
  for (alpha = 0.0; alpha < 360; alpha += interval)
    {
      x[0] = std::cos(alpha * M_PI / 180.0);
      y[0] = std::sin(alpha * M_PI / 180.0);
      x[1] = 0.0;
      y[1] = 0.0;

      auto temp = render->createPolyline(x[0], x[1], y[0], y[1]);
      temp->setAttribute("name", "polar_axes");
      newGroup->append(temp);
      render->setLineColorInd(temp, 90 - i % 2 * 2);

      x[0] *= 1.1;
      y[0] *= 1.1;
      if (phiflip == 0)
        {
          snprintf(text_buffer, PLOT_POLAR_AXES_TEXT_BUFFER, "%d\xc2\xb0", (int)grm_round(alpha));
        }
      else
        {
          if (alpha == 0.0)
            snprintf(text_buffer, PLOT_POLAR_AXES_TEXT_BUFFER, "%d\xc2\xb0", 0);
          else
            snprintf(text_buffer, PLOT_POLAR_AXES_TEXT_BUFFER, "%d\xc2\xb0", 330 - (int)grm_round(alpha - interval));
        }
      temp = render->createText(x[0], y[0], text_buffer, CoordinateSpace::WC);
      temp->setAttribute("name", "polar_axes");
      newGroup->append(temp);
      render->setTextAlign(temp, GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_HALF);
    }
  // Draw Text
  render->setTextAlign(newGroup, GKS_K_TEXT_HALIGN_LEFT, GKS_K_TEXT_VALIGN_HALF);
  for (i = 0; i <= n; i++)
    {
      double r = 1.0 / n * i;
      if (i % 2 == 0)
        {
          x[0] = 0.05;
          y[0] = r;
          snprintf(text_buffer, PLOT_POLAR_AXES_TEXT_BUFFER, "%.1lf", r_min + tick * i);
          newGroup->append(render->createText(x[0], y[0], text_buffer, CoordinateSpace::WC));
        }
    }
  title = static_cast<std::string>(elem->getAttribute("title"));
  if (title != "")
    {
      auto new_title_elem = render->createText(0.5 * (viewport[0] + viewport[1]), vp[3] - 0.02, title);
      new_title_elem->setAttribute("name", "title");
      render->setTextAlign(new_title_elem, GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_TOP);

      auto old_title_elem = elem->querySelectors("[name=\"title\"]");
      bool removed_elem = false;
      if (old_title_elem && (std::string)old_title_elem->getAttribute("text") != title)
        {
          old_title_elem->remove();
          removed_elem = true;
        }
      if (removed_elem || !old_title_elem)
        {
          elem->appendChild(new_title_elem);
        }
    }
}

static void drawRect(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for drawArc
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double xmin = static_cast<double>(element->getAttribute("xmin"));
  double xmax = static_cast<double>(element->getAttribute("xmax"));
  double ymin = static_cast<double>(element->getAttribute("ymin"));
  double ymax = static_cast<double>(element->getAttribute("ymax"));
  gr_drawrect(xmin, xmax, ymin, ymax);
}

static void drawYLine(const std::shared_ptr<GRM::Element> &elem, const std::shared_ptr<GRM::Context> &context)
{
  auto draw_axes_element = elem->parentElement();
  double x_tick, x_org_low, x_org_high;
  double y_tick, y_org_low, y_org_high;
  int x_major, y_major;
  std::string orientation = PLOT_DEFAULT_ORIENTATION;

  getAxesInformation(elem, "low", "low", x_org_low, y_org_low, x_major, y_major, x_tick, y_tick);
  getAxesInformation(elem, "high", "high", x_org_high, y_org_high, x_major, y_major, x_tick, y_tick);
  if (elem->hasAttribute("orientation"))
    {
      orientation = static_cast<std::string>(elem->getAttribute("orientation"));
    }
  else
    {
      elem->setAttribute("orientation", orientation);
    }
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

static void fillArc(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for drawArc
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double xmin = static_cast<double>(element->getAttribute("xmin"));
  double xmax = static_cast<double>(element->getAttribute("xmax"));
  double ymin = static_cast<double>(element->getAttribute("ymin"));
  double ymax = static_cast<double>(element->getAttribute("ymax"));
  double a1 = static_cast<double>(element->getAttribute("a1"));
  double a2 = static_cast<double>(element->getAttribute("a2"));
  gr_fillarc(xmin, xmax, ymin, ymax, a1, a2);
}

static void fillRect(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for drawArc
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double xmin = static_cast<double>(element->getAttribute("xmin"));
  double xmax = static_cast<double>(element->getAttribute("xmax"));
  double ymin = static_cast<double>(element->getAttribute("ymin"));
  double ymax = static_cast<double>(element->getAttribute("ymax"));
  gr_fillrect(xmin, xmax, ymin, ymax);
}

static void fillArea(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for fillArea
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));

  std::vector<double> x_vec = GRM::get<std::vector<double>>((*context)[x]);
  std::vector<double> y_vec = GRM::get<std::vector<double>>((*context)[y]);

  int n = std::min(x_vec.size(), y_vec.size());

  gr_fillarea(n, (double *)&(x_vec[0]), (double *)&(y_vec[0]));
}

static void gr3Clear(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  gr3_clear();
}

static void gr3DeleteMesh(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  int mesh = static_cast<int>(element->getAttribute("mesh"));
  gr3_deletemesh(mesh);
}

static void gr3DrawImage(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
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

static void gr3DrawMesh(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for gr3_drawmesh
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */

  int mesh = (int)element->getAttribute("mesh");
  int n = (int)element->getAttribute("n");

  auto positions = static_cast<std::string>(element->getAttribute("positions"));
  auto directions = static_cast<std::string>(element->getAttribute("directions"));
  auto ups = static_cast<std::string>(element->getAttribute("ups"));
  auto colors = static_cast<std::string>(element->getAttribute("colors"));
  auto scales = static_cast<std::string>(element->getAttribute("scales"));

  std::vector<double> positions_vec = GRM::get<std::vector<double>>((*context)[positions]);
  std::vector<double> directions_vec = GRM::get<std::vector<double>>((*context)[directions]);
  std::vector<double> ups_vec = GRM::get<std::vector<double>>((*context)[ups]);
  std::vector<double> colors_vec = GRM::get<std::vector<double>>((*context)[colors]);
  std::vector<double> scales_vec = GRM::get<std::vector<double>>((*context)[scales]);

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

static void grid(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for grid
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
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

  gr_grid(x_tick, y_tick, x_org, y_org, abs(x_major), abs(y_major));
}

static void grid3d(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for grid3d
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
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

  gr_grid3d(x_tick, y_tick, z_tick, x_org, y_org, z_org, abs(x_major), abs(y_major), abs(z_major));
}

static void heatmap(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for heatmap
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */

  int icmap[256], zlog = 0;
  unsigned int i, cols, rows, z_length;
  double x_min, x_max, y_min, y_max, z_min, z_max, c_min, c_max, zv;
  int is_uniform_heatmap;
  std::shared_ptr<GRM::Element> plot_parent;
  std::shared_ptr<GRM::Element> element_context = element;
  std::vector<int> data, rgba;
  std::vector<double> x_vec, y_vec, z_vec;

  if (element->parentElement()->localName() == "plot")
    {
      plot_parent = element->parentElement();
    }
  else
    {
      element_context = element->parentElement();
      plot_parent = element->parentElement()->parentElement();
    }
  zlog = static_cast<int>(plot_parent->getAttribute("zlog"));


  if (element_context->hasAttribute("x"))
    {
      auto x = static_cast<std::string>(element_context->getAttribute("x"));
      x_vec = GRM::get<std::vector<double>>((*context)[x]);
      cols = x_vec.size();
    }
  if (element_context->hasAttribute("y"))
    {
      auto y = static_cast<std::string>(element_context->getAttribute("y"));
      y_vec = GRM::get<std::vector<double>>((*context)[y]);
      rows = y_vec.size();
    }

  if (!element_context->hasAttribute("z"))
    throw NotFoundError("Heatmap series is missing required attribute z-data.\n");
  auto z = static_cast<std::string>(element_context->getAttribute("z"));
  z_vec = GRM::get<std::vector<double>>((*context)[z]);
  z_length = z_vec.size();

  if (x_vec.empty() && y_vec.empty())
    {
      /* If neither `x` nor `y` are given, we need more information about the shape of `z` */
      if (!element->hasAttribute("zdims_min") || !element->hasAttribute("zdims_max"))
        throw NotFoundError("Heatmap series is missing required attribute zdims.\n");
      rows = static_cast<int>(element->getAttribute("zdims_min"));
      cols = static_cast<int>(element->getAttribute("zdims_max"));
    }
  else if (x_vec.empty())
    {
      cols = z_length / rows;
    }
  else if (y_vec.empty())
    {
      rows = z_length / cols;
    }

  is_uniform_heatmap = (x_vec.empty() || is_equidistant_array(cols, &(x_vec[0]))) &&
                       (y_vec.empty() || is_equidistant_array(rows, &(y_vec[0])));
  if (!is_uniform_heatmap && (x_vec.empty() || y_vec.empty()))
    throw NotFoundError("Heatmap series is missing x- or y-data or the data has to be uniform.\n");

  if (x_vec.empty())
    {
      x_min = static_cast<double>(plot_parent->getAttribute("xrange_min"));
      x_max = static_cast<double>(plot_parent->getAttribute("xrange_max"));
    }
  else
    {
      x_min = x_vec[0];
      x_max = x_vec[cols - 1];
    }
  if (y_vec.empty())
    {
      y_min = static_cast<double>(plot_parent->getAttribute("yrange_min"));
      y_max = static_cast<double>(plot_parent->getAttribute("yrange_max"));
    }
  else
    {
      y_min = y_vec[0];
      y_max = y_vec[rows - 1];
    }
  z_min = static_cast<double>(plot_parent->getAttribute("zrange_min"));
  z_max = static_cast<double>(plot_parent->getAttribute("zrange_max"));
  if (!element->hasAttribute("crange_min") || !element->hasAttribute("crange_max"))
    {
      c_min = z_min;
      c_max = z_max;
    }
  else
    {
      c_min = static_cast<double>(element->getAttribute("crange_min"));
      c_max = static_cast<double>(element->getAttribute("crange_max"));
    }

  if (zlog)
    {
      z_min = log(z_min);
      z_max = log(z_max);
      c_min = log(c_min);
      c_max = log(c_max);
    }

  if (!is_uniform_heatmap)
    {
      --cols;
      --rows;
    }
  for (i = 0; i < 256; i++)
    {
      gr_inqcolor(1000 + i, icmap + i);
    }

  data = std::vector<int>(rows * cols);
  if (z_max > z_min)
    {
      for (i = 0; i < cols * rows; i++)
        {
          if (zlog)
            {
              zv = log(z_vec[i]);
            }
          else
            {
              zv = z_vec[i];
            }

          if (zv > z_max || zv < z_min || grm_isnan(zv))
            {
              data[i] = -1;
            }
          else
            {
              data[i] = (int)((zv - c_min) / (c_max - c_min) * 255 + 0.5);
              if (data[i] >= 255)
                {
                  data[i] = 255;
                }
              else if (data[i] < 0)
                {
                  data[i] = 0;
                }
            }
        }
    }
  else
    {
      for (i = 0; i < cols * rows; i++)
        {
          data[i] = 0;
        }
    }
  rgba = std::vector<int>(rows * cols);

  //   clear old heatmaps
  for (auto elem : element->children())
    {
      if (elem->localName() == "drawimage") elem->remove();
      if (elem->localName() == "nonuniformcellarray") elem->remove();
    }

  int id = (int)global_root->getAttribute("id");
  global_root->setAttribute("id", id + 1);
  std::string str = std::to_string(id);
  if (is_uniform_heatmap)
    {
      for (i = 0; i < rows * cols; i++)
        {
          if (data[i] == -1)
            {
              rgba[i] = 0;
            }
          else
            {
              rgba[i] = (255 << 24) + icmap[data[i]];
            }
        }

      auto x_tmp = static_cast<double *>(malloc((cols) * sizeof(double)));
      auto y_tmp = static_cast<double *>(malloc((rows) * sizeof(double)));
      linspace(x_min, x_max, cols, x_tmp);
      linspace(y_min, y_max, rows, y_tmp);

      std::vector<double> x_vec_tmp(x_tmp, x_tmp + cols);
      (*context)["x" + str] = x_vec_tmp;
      element->setAttribute("x", "x" + str);
      std::vector<double> y_vec_tmp(y_tmp, y_tmp + rows);
      (*context)["y" + str] = y_vec_tmp;
      element->setAttribute("y", "y" + str);

      element->append(global_render->createDrawImage(x_min, y_min, x_max, y_max, cols, rows, "rgba" + str, rgba, 0));
    }
  else
    {
      for (i = 0; i < rows * cols; i++)
        {
          if (data[i] == -1)
            {
              rgba[i] = 1256 + 1; /* Invalid color index -> gr_nonuniformcellarray draws a transparent rectangle */
            }
          else
            {
              rgba[i] = data[i] + 1000;
            }
        }

      element->append(global_render->createNonUniformCellArray("x" + str, x_vec, "y" + str, y_vec, cols, rows, 1, 1,
                                                               cols, rows, "color" + str, rgba));
    }
}

static void hexbin(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for hexbin
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  int nbins = PLOT_DEFAULT_HEXBIN_NBINS;

  if (!element->hasAttribute("x")) throw NotFoundError("Hexbin series is missing required attribute x-data.\n");
  auto x = static_cast<std::string>(element->getAttribute("x"));
  if (!element->hasAttribute("y")) throw NotFoundError("Hexbin series is missing required attribute y-data.\n");
  auto y = static_cast<std::string>(element->getAttribute("y"));
  if (element->hasAttribute("nbins"))
    {
      nbins = static_cast<int>(element->getAttribute("nbins"));
    }
  else
    {
      element->setAttribute("nbins", nbins);
    }

  double *x_p = &(GRM::get<std::vector<double>>((*context)[x])[0]);
  double *y_p = &(GRM::get<std::vector<double>>((*context)[y])[0]);

  std::vector<double> x_vec = GRM::get<std::vector<double>>((*context)[x]);
  std::vector<double> y_vec = GRM::get<std::vector<double>>((*context)[y]);
  int x_length = x_vec.size();
  int y_length = y_vec.size();
  if (x_length != y_length) throw std::length_error("For Hexbin x- and y-data must have the same size\n.");

  int cntmax = gr_hexbin(x_length, x_p, y_p, nbins);
  auto colorbar = element->querySelectors("colorbar");
  double c_min = 0.0;
  double c_max = cntmax;
  colorbar->setAttribute("lim_cmin", c_min);
  colorbar->setAttribute("lim_cmax", c_max);
}

static void hist(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for hist
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */

  double *bar_centers = nullptr;
  int bar_color_index = 989, i;
  std::vector<double> bar_color_rgb_vec = {-1, -1, -1};
  std::shared_ptr<GRM::Element> plot_parent;

  auto bar_color_rgb = static_cast<std::string>(element->getAttribute("bar_color_rgb"));
  bar_color_rgb_vec = GRM::get<std::vector<double>>((*context)[bar_color_rgb]);

  bar_color_index = static_cast<int>(element->getAttribute("bar_color_index"));

  if (element->parentElement()->localName() != "plot")
    {
      plot_parent = element->parentElement()->parentElement();
    }
  else
    {
      plot_parent = element->parentElement();
    }

  if (bar_color_rgb_vec[0] != -1)
    {
      for (i = 0; i < 3; i++)
        {
          if (bar_color_rgb_vec[i] > 1 || bar_color_rgb_vec[i] < 0)
            throw std::out_of_range("For hist series bar_color_rgb must be inside [0, 1].\n");
        }
      bar_color_index = 1000;
      global_render->setColorRep(plot_parent, bar_color_index, bar_color_rgb[0], bar_color_rgb[1], bar_color_rgb[2]);
      processColorRep(plot_parent);
    }

  int edge_color_index = 1;
  std::vector<double> edge_color_rgb_vec = {-1, -1, -1};
  double x_min, x_max, bar_width, y_min;
  std::vector<double> bins_vec;
  unsigned int num_bins;
  std::string orientation = PLOT_DEFAULT_ORIENTATION;
  int is_horizontal;

  auto edge_color_rgb = static_cast<std::string>(element->getAttribute("edge_color_rgb"));
  edge_color_rgb_vec = GRM::get<std::vector<double>>((*context)[edge_color_rgb]);

  edge_color_index = static_cast<int>(element->getAttribute("edge_color_index"));
  if (edge_color_rgb_vec[0] != -1)
    {
      for (i = 0; i < 3; i++)
        {
          if (edge_color_rgb_vec[i] > 1 || edge_color_rgb_vec[i] < 0)
            throw std::out_of_range("For hist series edge_color_rgb must be inside [0, 1].\n");
        }
      edge_color_index = 1001;
      global_render->setColorRep(plot_parent, edge_color_index, edge_color_rgb[0], edge_color_rgb[1],
                                 edge_color_rgb[2]);
      processColorRep(plot_parent);
    }

  auto bins = static_cast<std::string>(element->getAttribute("bins"));
  bins_vec = GRM::get<std::vector<double>>((*context)[bins]);
  num_bins = bins_vec.size();
  if (element->hasAttribute("orientation"))
    {
      orientation = static_cast<std::string>(element->getAttribute("orientation"));
    }
  else
    {
      element->setAttribute("orientation", orientation);
    }
  is_horizontal = orientation == "horizontal";
  x_min = static_cast<double>(plot_parent->getAttribute("xrange_min"));
  x_max = static_cast<double>(plot_parent->getAttribute("xrange_max"));
  y_min = static_cast<double>(plot_parent->getAttribute("yrange_min"));

  if (element->parentElement()->hasAttribute("marginalheatmap_kind"))
    {
      element->setAttribute("calc_window_and_viewport_from_parent", 1);
      y_min = 0.0;
      processCalcWindowAndViewportFromParent(element);
      processMarginalheatmapKind(element->parentElement());
    }

  bar_width = (x_max - x_min) / num_bins;

  // Use of two groups does not produce the same results as julia...
  // std::shared_ptr<GRM::Element> innerFillGroup = global_render->createGroup("innerFillGroup");
  // std::shared_ptr<GRM::Element> outerFillGroup = global_render->createGroup("outerFillGroup");

  // clear old rects
  for (auto elem : element->children())
    {
      if (elem->localName() == "fillrect") elem->remove();
      if (elem->localName() == "drawrect") elem->remove();
    }

  for (i = 1; i < num_bins + 1; ++i)
    {
      double x = x_min + (i - 1) * bar_width;
      std::shared_ptr<GRM::Element> fillRect, drawRect;

      if (is_horizontal)
        {
          fillRect = global_render->createFillRect(x, x + bar_width, y_min, bins_vec[i - 1]);
          global_render->setFillColorInd(fillRect, bar_color_index);
        }
      else
        {
          fillRect = global_render->createFillRect(y_min, bins_vec[i - 1], x, x + bar_width);
          global_render->setFillColorInd(fillRect, bar_color_index);
        }
      global_render->setFillIntStyle(fillRect, GKS_K_INTSTYLE_SOLID);
      element->append(fillRect);

      if (is_horizontal)
        {
          drawRect = global_render->createDrawRect(x, x + bar_width, y_min, bins_vec[i - 1]);
        }
      else
        {
          drawRect = global_render->createDrawRect(y_min, bins_vec[i - 1], x, x + bar_width);
        }
      global_render->setLineColorInd(drawRect, edge_color_index);
      element->append(drawRect);
    }
}

static void isosurfaceRender(const std::shared_ptr<GRM::Element> &elem, const std::shared_ptr<GRM::Context> &context)
{
  double viewport[4];
  double x_min, x_max, y_min, y_max;
  int fig_width, fig_height;
  int subplot_width, subplot_height;
  int drawable_type;

  drawable_type = static_cast<int>(elem->getAttribute("drawable_type"));

  gr_inqviewport(&viewport[0], &viewport[1], &viewport[2], &viewport[3]);

  x_min = viewport[0];
  x_max = viewport[1];
  y_min = viewport[2];
  y_max = viewport[3];

  get_figure_size(nullptr, &fig_width, &fig_height, nullptr, nullptr);
  subplot_width = (int)(grm_max(fig_width, fig_height) * (x_max - x_min));
  subplot_height = (int)(grm_max(fig_width, fig_height) * (y_max - y_min));

  logger((stderr, "viewport: (%lf, %lf, %lf, %lf)\n", x_min, x_max, y_min, y_max));
  logger((stderr, "viewport ratio: %lf\n", (x_min - x_max) / (y_min - y_max)));
  logger((stderr, "subplot size: (%d, %d)\n", subplot_width, subplot_height));
  logger((stderr, "subplot ratio: %lf\n", ((double)subplot_width / (double)subplot_height)));

  gr3_drawimage(x_min, x_max, y_min, y_max, subplot_width, subplot_height, GR3_DRAWABLE_GKS);
}

static void layoutGrid(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  double xmin, xmax, ymin, ymax;
  xmin = (double)element->getAttribute("subplot_xmin");
  xmax = (double)element->getAttribute("subplot_xmax");
  ymin = (double)element->getAttribute("subplot_ymin");
  ymax = (double)element->getAttribute("subplot_ymax");

  gr_setviewport(xmin, xmax, ymin, ymax);
}

static void layoutGridElement(const std::shared_ptr<GRM::Element> &element,
                              const std::shared_ptr<GRM::Context> &context)
{
  // todo is layoutgridelement actually needed? Can it be replaced by an ordinary group
  double xmin, xmax, ymin, ymax;
  xmin = (double)element->getAttribute("subplot_xmin");
  xmax = (double)element->getAttribute("subplot_xmax");
  ymin = (double)element->getAttribute("subplot_ymin");
  ymax = (double)element->getAttribute("subplot_ymax");

  //  gr_setviewport(xmin, xmax, ymin, ymax);
}

static void nonUniformPolarCellArray(const std::shared_ptr<GRM::Element> &element,
                                     const std::shared_ptr<GRM::Context> &context)
{
  double x_org = static_cast<double>(element->getAttribute("x_org"));
  double y_org = static_cast<double>(element->getAttribute("y_org"));
  std::string phi_key = static_cast<std::string>(element->getAttribute("phi"));
  std::string r_key = static_cast<std::string>(element->getAttribute("r"));
  int dimr = static_cast<int>(element->getAttribute("dimr"));
  int dimphi = static_cast<int>(element->getAttribute("dimphi"));
  int scol = static_cast<int>(element->getAttribute("scol"));
  int srow = static_cast<int>(element->getAttribute("srow"));
  int ncol = static_cast<int>(element->getAttribute("ncol"));
  int nrow = static_cast<int>(element->getAttribute("nrow"));
  std::string color_key = static_cast<std::string>(element->getAttribute("color"));

  auto r_vec = GRM::get<std::vector<double>>((*context)[r_key]);
  auto phi_vec = GRM::get<std::vector<double>>((*context)[phi_key]);
  auto color_vec = GRM::get<std::vector<int>>((*context)[color_key]);

  double *phi = &(phi_vec[0]);
  double *r = &(r_vec[0]);
  int *color = &(color_vec[0]);

  gr_nonuniformpolarcellarray(x_org, y_org, phi, r, dimphi, dimr, scol, srow, ncol, nrow, color);
}

static void nonuniformcellarray(const std::shared_ptr<GRM::Element> &element,
                                const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for nonuniformcellarray
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
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

  auto x_p = (double *)&(GRM::get<std::vector<double>>((*context)[x])[0]);
  auto y_p = (double *)&(GRM::get<std::vector<double>>((*context)[y])[0]);

  auto color_p = (int *)&(GRM::get<std::vector<int>>((*context)[color])[0]);
  gr_nonuniformcellarray(x_p, y_p, dimx, dimy, scol, srow, ncol, nrow, color_p);
}

static void panzoom(const std::shared_ptr<GRM::Element> &elem, const std::shared_ptr<GRM::Context> &context)
{
  ; /* panzoom is being processed in the processLimits routine */
}

static void polarCellArray(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  double x_org = static_cast<double>(element->getAttribute("x_org"));
  double y_org = static_cast<double>(element->getAttribute("y_org"));
  double phimin = static_cast<double>(element->getAttribute("phimin"));
  double phimax = static_cast<double>(element->getAttribute("phimax"));
  double rmin = static_cast<double>(element->getAttribute("rmin"));
  double rmax = static_cast<double>(element->getAttribute("rmax"));
  int dimr = static_cast<int>(element->getAttribute("dimr"));
  int dimphi = static_cast<int>(element->getAttribute("dimphi"));
  int scol = static_cast<int>(element->getAttribute("scol"));
  int srow = static_cast<int>(element->getAttribute("srow"));
  int ncol = static_cast<int>(element->getAttribute("ncol"));
  int nrow = static_cast<int>(element->getAttribute("nrow"));
  std::string color_key = static_cast<std::string>(element->getAttribute("color"));

  auto color_vec = GRM::get<std::vector<int>>((*context)[color_key]);
  int *color = &(color_vec[0]);

  gr_polarcellarray(x_org, y_org, phimin, phimax, rmin, rmax, dimphi, dimr, scol, srow, ncol, nrow, color);
}

static void polyline(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing Function for polyline
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  int n = static_cast<int>(element->getAttribute("n"));
  if (element->getAttribute("x").isString() && element->getAttribute("y").isString())
    {
      auto x = static_cast<std::string>(element->getAttribute("x"));
      auto y = static_cast<std::string>(element->getAttribute("y"));

      std::vector<double> x_vec = GRM::get<std::vector<double>>((*context)[x]);
      std::vector<double> y_vec = GRM::get<std::vector<double>>((*context)[y]);

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

static void polyline3d(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for polyline3d
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));
  auto z = static_cast<std::string>(element->getAttribute("z"));

  std::vector<double> x_vec = GRM::get<std::vector<double>>((*context)[x]);
  std::vector<double> y_vec = GRM::get<std::vector<double>>((*context)[y]);
  std::vector<double> z_vec = GRM::get<std::vector<double>>((*context)[z]);

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

static void polymarker(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for polymarker
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  if (element->getAttribute("x").isString() && element->getAttribute("y").isString())
    {
      auto x = static_cast<std::string>(element->getAttribute("x"));
      auto y = static_cast<std::string>(element->getAttribute("y"));

      std::vector<double> x_vec = GRM::get<std::vector<double>>((*context)[x]);
      std::vector<double> y_vec = GRM::get<std::vector<double>>((*context)[y]);

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

static void polymarker3d(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for polymarker3d
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));
  auto z = static_cast<std::string>(element->getAttribute("z"));

  std::vector<double> x_vec = GRM::get<std::vector<double>>((*context)[x]);
  std::vector<double> y_vec = GRM::get<std::vector<double>>((*context)[y]);
  std::vector<double> z_vec = GRM::get<std::vector<double>>((*context)[z]);

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

static void quiver(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for quiver
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  if (!element->hasAttribute("x")) throw NotFoundError("Quiver series is missing required attribute x-data.\n");
  auto x = static_cast<std::string>(element->getAttribute("x"));
  if (!element->hasAttribute("y")) throw NotFoundError("Quiver series is missing required attribute y-data.\n");
  auto y = static_cast<std::string>(element->getAttribute("y"));
  if (!element->hasAttribute("u")) throw NotFoundError("Quiver series is missing required attribute u-data.\n");
  auto u = static_cast<std::string>(element->getAttribute("u"));
  if (!element->hasAttribute("v")) throw NotFoundError("Quiver series is missing required attribute v-data.\n");
  auto v = static_cast<std::string>(element->getAttribute("v"));
  int color = static_cast<int>(element->getAttribute("color"));

  std::vector<double> x_vec = GRM::get<std::vector<double>>((*context)[x]);
  std::vector<double> y_vec = GRM::get<std::vector<double>>((*context)[y]);
  std::vector<double> u_vec = GRM::get<std::vector<double>>((*context)[u]);
  std::vector<double> v_vec = GRM::get<std::vector<double>>((*context)[v]);
  int x_length = x_vec.size();
  int y_length = y_vec.size();
  int u_length = u_vec.size();
  int v_length = v_vec.size();

  if (x_length * y_length != u_length)
    throw std::length_error("For quiver series x_length * y_length must be u_length.\n");
  if (x_length * y_length != v_length)
    throw std::length_error("For quiver series x_length * y_length must be v_length.\n");

  double *x_p = &(x_vec[0]);
  double *y_p = &(y_vec[0]);
  double *u_p = &(GRM::get<std::vector<double>>((*context)[u])[0]);
  double *v_p = &(GRM::get<std::vector<double>>((*context)[v])[0]);

  gr_quiver(x_length, y_length, x_p, y_p, u_p, v_p, color);
}

static void polar(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for polar
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double r_min, r_max, tick;
  int n;
  unsigned int rho_length, theta_length;
  std::string spec = SERIES_DEFAULT_SPEC;
  unsigned int i;
  std::vector<double> theta_vec, rho_vec;

  if (element->hasAttribute("spec"))
    {
      spec = static_cast<std::string>(element->getAttribute("spec"));
    }
  else
    {
      element->setAttribute("spec", spec);
    }
  r_min = static_cast<double>(element->getAttribute("r_min"));
  r_max = static_cast<double>(element->getAttribute("r_max"));
  tick = 0.5 * auto_tick(r_min, r_max);
  n = (int)ceil((r_max - r_min) / tick);
  r_max = r_min + n * tick;

  if (!element->hasAttribute("x")) throw NotFoundError("Polar series is missing required attribute x-data (theta).\n");
  auto x_key = static_cast<std::string>(element->getAttribute("x"));
  if (!element->hasAttribute("y")) throw NotFoundError("Polar series is missing required attribute y-data (rho).\n");
  auto y_key = static_cast<std::string>(element->getAttribute("y"));
  theta_vec = GRM::get<std::vector<double>>((*context)[x_key]);
  rho_vec = GRM::get<std::vector<double>>((*context)[y_key]);
  theta_length = theta_vec.size();
  rho_length = rho_vec.size();
  if (rho_length != theta_length)
    throw std::length_error("For polar series y(rho)- and x(theta)-data must have the same size.\n");

  std::vector<double> x(rho_length);
  std::vector<double> y(rho_length);

  for (i = 0; i < rho_length; ++i)
    {
      double current_rho = rho_vec[i] / r_max;
      x[i] = current_rho * cos(theta_vec[i]);
      y[i] = current_rho * sin(theta_vec[i]);
    }

  global_render->setLineSpec(element, spec);

  int id = (int)global_root->getAttribute("id");
  global_root->setAttribute("id", id + 1);

  auto temp = global_render->createPolyline("x" + std::to_string(id), x, "y" + std::to_string(id), y);
  element->append(temp);
}

static void polar_heatmap(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for polar_heatmap
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */

  std::string kind;
  int icmap[256], zlog = 0;
  unsigned int i, cols, rows, z_length;
  double x_min, x_max, y_min, y_max, z_min, z_max, c_min, c_max, zv;
  int is_uniform_heatmap;
  std::vector<int> data;
  std::vector<double> x_vec, y_vec, z_vec;

  kind = static_cast<std::string>(element->parentElement()->getAttribute("kind"));
  zlog = static_cast<int>(element->parentElement()->getAttribute("zlog"));

  if (element->hasAttribute("x"))
    {
      auto x = static_cast<std::string>(element->getAttribute("x"));
      x_vec = GRM::get<std::vector<double>>((*context)[x]);
      cols = x_vec.size();
    }
  if (element->hasAttribute("y"))
    {
      auto y = static_cast<std::string>(element->getAttribute("y"));
      y_vec = GRM::get<std::vector<double>>((*context)[y]);
      rows = y_vec.size();
    }

  if (!element->hasAttribute("z")) throw NotFoundError("Polar-heatmap series is missing required attribute z-data.\n");
  auto z = static_cast<std::string>(element->getAttribute("z"));
  z_vec = GRM::get<std::vector<double>>((*context)[z]);
  z_length = z_vec.size();

  if (x_vec.empty() && y_vec.empty())
    {
      /* If neither `x` nor `y` are given, we need more information about the shape of `z` */
      if (!element->hasAttribute("zdims_min") || !element->hasAttribute("zdims_max"))
        throw NotFoundError("Polar-heatmap series is missing required attribute zdims.\n");
      rows = static_cast<int>(element->getAttribute("zdims_min"));
      cols = static_cast<int>(element->getAttribute("zdims_max"));
    }
  else if (x_vec.empty())
    {
      cols = z_length / rows;
    }
  else if (y_vec.empty())
    {
      rows = z_length / cols;
    }

  is_uniform_heatmap = is_equidistant_array(cols, &(x_vec[0])) && is_equidistant_array(rows, &(y_vec[0]));
  if (kind == "nonuniformpolar_heatmap") is_uniform_heatmap = 0;

  if (!is_uniform_heatmap && (x_vec.empty() || y_vec.empty()))
    throw NotFoundError("Polar-heatmap series is missing x- or y-data or the data has to be uniform.\n");

  if (x_vec.empty())
    {
      x_min = static_cast<double>(element->parentElement()->getAttribute("xrange_min"));
      x_max = static_cast<double>(element->parentElement()->getAttribute("xrange_max"));
    }
  else
    {
      x_min = x_vec[0];
      x_max = x_vec[cols - 1];
    }
  if (y_vec.empty())
    {
      y_min = static_cast<double>(element->parentElement()->getAttribute("yrange_min"));
      y_max = static_cast<double>(element->parentElement()->getAttribute("yrange_max"));
    }
  else
    {
      y_min = y_vec[0];
      y_max = y_vec[rows - 1];
    }

  z_min = static_cast<double>(element->parentElement()->getAttribute("zrange_min"));
  z_max = static_cast<double>(element->parentElement()->getAttribute("zrange_max"));
  if (!element->hasAttribute("crange_min") || !element->hasAttribute("crange_max"))
    {
      c_min = z_min;
      c_max = z_max;
    }
  else
    {
      c_min = static_cast<double>(element->getAttribute("crange_min"));
      c_max = static_cast<double>(element->getAttribute("crange_max"));
    }

  if (zlog)
    {
      z_min = log(z_min);
      z_max = log(z_max);
      c_min = log(c_min);
      c_max = log(c_max);
    }

  if (!is_uniform_heatmap)
    {
      --cols;
      --rows;
    }
  for (i = 0; i < 256; i++)
    {
      gr_inqcolor(1000 + i, icmap + i);
    }

  data = std::vector<int>(rows * cols);
  if (z_max > z_min)
    {
      for (i = 0; i < cols * rows; i++)
        {
          if (zlog)
            {
              zv = log(z_vec[i]);
            }
          else
            {
              zv = z_vec[i];
            }

          if (zv > z_max || zv < z_min || grm_isnan(zv))
            {
              data[i] = -1;
            }
          else
            {
              data[i] = 1000 + (int)(255.0 * (zv - c_min) / (c_max - c_min) + 0.5);
              if (data[i] >= 1255)
                {
                  data[i] = 1255;
                }
              else if (data[i] < 1000)
                {
                  data[i] = 1000;
                }
            }
        }
    }
  else
    {
      for (i = 0; i < cols * rows; i++)
        {
          data[i] = 0;
        }
    }

  int id = (int)global_root->getAttribute("id");
  global_root->setAttribute("id", id + 1);
  std::string str = std::to_string(id);

  //   clear old polar_heatmaps
  for (const auto &elem : element->children())
    {
      if (elem->localName() == "polarcellarray") elem->remove();
      if (elem->localName() == "nonuniform_polarcellarray") elem->remove();
    }

  if (is_uniform_heatmap)
    {
      element->append(
          global_render->createPolarCellArray(0, 0, 0, 360, 0, 1, cols, rows, 1, 1, cols, rows, "color" + str, data));
    }
  else
    {
      y_min = static_cast<double>(element->parentElement()->getAttribute("window_ymin"));
      y_max = static_cast<double>(element->parentElement()->getAttribute("window_ymax"));

      std::vector<double> rho, phi;
      for (i = 0; i <= ((cols > rows) ? cols : rows); ++i)
        {
          if (i <= cols)
            {
              phi.push_back(x_vec[i] * 180 / M_PI);
            }
          if (i <= rows)
            {
              rho.push_back(y_min + y_vec[i] / (y_max - y_min));
            }
        }

      element->append(global_render->createNonUniformPolarCellArray(0, 0, "phi" + str, phi, "rho" + str, rho, -cols,
                                                                    -rows, 1, 1, cols, rows, "color" + str, data));
    }
}

static void scatter(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for scatter
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  std::string orientation = PLOT_DEFAULT_ORIENTATION;
  double c_min, c_max;
  unsigned int x_length, y_length, z_length, c_length;
  int i, c_index = -1, markertype;
  std::vector<int> markerColorIndsVec;
  std::vector<double> markerSizesVec;
  std::vector<double> x_vec, y_vec, z_vec, c_vec;

  if (!element->hasAttribute("x")) throw NotFoundError("Scatter series is missing required attribute x-data.\n");
  auto x = static_cast<std::string>(element->getAttribute("x"));
  if (!element->hasAttribute("y")) throw NotFoundError("Scatter series is missing required attribute y-data.\n");
  auto y = static_cast<std::string>(element->getAttribute("y"));
  x_vec = GRM::get<std::vector<double>>((*context)[x]);
  y_vec = GRM::get<std::vector<double>>((*context)[y]);
  x_length = x_vec.size();
  y_length = y_vec.size();
  if (x_length != y_length) throw std::length_error("For scatter series x- and y-data must have the same size.\n");

  if (element->hasAttribute("z"))
    {
      auto z = static_cast<std::string>(element->getAttribute("z"));
      z_vec = GRM::get<std::vector<double>>((*context)[z]);
      z_length = z_vec.size();
      if (x_length != z_length) throw std::length_error("For scatter series x- and z-data must have the same size.\n");
    }
  if (element->hasAttribute("c"))
    {
      auto c = static_cast<std::string>(element->getAttribute("c"));
      c_vec = GRM::get<std::vector<double>>((*context)[c]);
      c_length = c_vec.size();
    }
  if (element->hasAttribute("orientation"))
    {
      orientation = static_cast<std::string>(element->getAttribute("orientation"));
    }
  else
    {
      element->setAttribute("orientation", orientation);
    }

  markertype = static_cast<int>(element->getAttribute("markertype"));
  global_render->setMarkerType(element, markertype);

  if (c_vec.empty() && element->hasAttribute("c_index"))
    {
      c_index = static_cast<int>(element->getAttribute("c_index"));
      if (c_index < 0)
        {
          logger((stderr, "Invalid scatter color %d, using 0 instead\n", c_index));
          c_index = 0;
        }
      else if (c_index > 255)
        {
          logger((stderr, "Invalid scatter color %d, using 255 instead\n", c_index));
          c_index = 255;
        }
    }

  // clear old marker
  for (auto elem : element->children())
    {
      if (elem->localName() == "polymarker") elem->remove();
    }

  if (!z_vec.empty() || !c_vec.empty())
    {
      c_min = static_cast<double>(element->getAttribute("c_min"));
      c_max = static_cast<double>(element->getAttribute("c_max"));

      for (i = 0; i < x_length; i++)
        {
          if (!z_vec.empty())
            {
              if (i < z_length)
                {
                  markerSizesVec.push_back(z_vec[i]);
                }
              else
                {
                  markerSizesVec.push_back(2.0);
                }
            }
          if (!c_vec.empty())
            {
              if (i < c_length)
                {
                  c_index = 1000 + (int)(255.0 * (c_vec[i] - c_min) / (c_max - c_min) + 0.5);
                  if (c_index < 1000 || c_index > 1255)
                    {
                      // colorind -1000 will be skipped
                      markerColorIndsVec.push_back(-1000);
                      continue;
                    }
                }
              else
                {
                  c_index = 989;
                }
              markerColorIndsVec.push_back(c_index);
            }
          else if (c_index != -1)
            {
              markerColorIndsVec.push_back(1000 + c_index);
            }
        }

      int id = static_cast<int>(global_root->getAttribute("id"));
      std::string str = std::to_string(id);
      global_root->setAttribute("id", ++id);

      auto marker = global_render->createPolymarker(str + "x", x_vec, str + "y", y_vec);
      element->append(marker);
      if (!markerSizesVec.empty())
        {
          global_render->setMarkerSize(element, "markersizes" + str, markerSizesVec);
        }
      if (!markerColorIndsVec.empty())
        {
          global_render->setMarkerColorInd(element, "markercolorinds" + str, markerColorIndsVec);
        }
    }
  else
    {
      int id = static_cast<int>(global_root->getAttribute("id"));
      std::string str = std::to_string(id);
      auto marker = global_render->createPolymarker(str + "x", x_vec, str + "y", y_vec);
      element->append(marker);
      global_root->setAttribute("id", ++id);
    }
}

static void scatter3(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for scatter3
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double c_min, c_max;
  unsigned int x_length, y_length, z_length, c_length, i, c_index;
  std::vector<double> x_vec, y_vec, z_vec, c_vec;

  if (!element->hasAttribute("x")) throw NotFoundError("Scatter3 series is missing required attribute x-data.\n");
  auto x = static_cast<std::string>(element->getAttribute("x"));
  if (!element->hasAttribute("y")) throw NotFoundError("Scatter3 series is missing required attribute y-data.\n");
  auto y = static_cast<std::string>(element->getAttribute("y"));
  if (!element->hasAttribute("z")) throw NotFoundError("Scatter3 series is missing required attribute z-data.\n");
  auto z = static_cast<std::string>(element->getAttribute("z"));
  x_vec = GRM::get<std::vector<double>>((*context)[x]);
  y_vec = GRM::get<std::vector<double>>((*context)[y]);
  z_vec = GRM::get<std::vector<double>>((*context)[z]);
  x_length = x_vec.size();
  y_length = y_vec.size();
  z_length = z_vec.size();
  if (x_length != y_length || x_length != z_length)
    throw std::length_error("For scatter3 series x-, y- and z-data must have the same size.\n");

  std::vector<int> markerCVec;

  global_render->setMarkerType(element, GKS_K_MARKERTYPE_SOLID_CIRCLE);
  processMarkerType(element);
  if (element->hasAttribute("c"))
    {
      auto c = static_cast<std::string>(element->getAttribute("c"));
      c_vec = GRM::get<std::vector<double>>((*context)[c]);
      c_length = c_vec.size();
      c_min = static_cast<double>(element->getAttribute("c_min"));
      c_max = static_cast<double>(element->getAttribute("c_max"));

      for (i = 0; i < x_length; i++)
        {
          if (i < c_length)
            {
              c_index = 1000 + (int)(255.0 * (c_vec[i] - c_min) / (c_max - c_min) + 0.5);
            }
          else
            {
              c_index = 989;
            }
          markerCVec.push_back(c_index);
        }
    }

  int id_int = static_cast<int>(global_root->getAttribute("id"));
  global_root->setAttribute("id", ++id_int);
  std::string id = std::to_string(id_int);

  if (!markerCVec.empty())
    {
      global_render->setMarkerColorInd(element, "markercolorinds" + id, markerCVec);
    }
  else if (element->hasAttribute("c_index"))
    {
      global_render->setMarkerColorInd(element, c_index);
    }

  // clear old marker
  for (auto elem : element->children())
    {
      if (elem->localName() == "polymarker3d") elem->remove();
    }

  auto temp = global_render->createPolymarker3d("x" + id, x_vec, "y" + id, y_vec, "z" + id, z_vec);
  element->append(temp);
}

static void stairs(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for stairs
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  std::string kind, orientation = PLOT_DEFAULT_ORIENTATION, spec = SERIES_DEFAULT_SPEC;
  double xmin, xmax, ymin, ymax;
  int is_vertical;
  unsigned int x_length, y_length, mask, i;
  std::vector<double> x_vec, y_vec;
  std::shared_ptr<GRM::Element> element_context = element;

  if (element->parentElement()->hasAttribute("marginalheatmap_kind")) element_context = element->parentElement();

  if (!element_context->hasAttribute("x")) throw NotFoundError("Stairs series is missing required attribute x-data.\n");
  auto x = static_cast<std::string>(element_context->getAttribute("x"));
  if (!element_context->hasAttribute("y")) throw NotFoundError("Stairs series is missing required attribute y-data.\n");
  auto y = static_cast<std::string>(element_context->getAttribute("y"));

  x_vec = GRM::get<std::vector<double>>((*context)[x]);
  y_vec = GRM::get<std::vector<double>>((*context)[y]);
  x_length = x_vec.size();
  y_length = y_vec.size();

  kind = static_cast<std::string>(element->getAttribute("kind"));
  if (element->hasAttribute("orientation"))
    {
      orientation = static_cast<std::string>(element->getAttribute("orientation"));
    }
  else
    {
      element->setAttribute("orientation", orientation);
    }
  if (element->hasAttribute("spec"))
    {
      spec = static_cast<std::string>(element->getAttribute("spec"));
    }
  else
    {
      element->setAttribute("spec", spec);
    }
  is_vertical = orientation == "vertical";

  int id = static_cast<int>(global_root->getAttribute("id"));
  std::string str = std::to_string(id);
  global_root->setAttribute("id", id + 1);

  // clear old marker and lines
  for (auto elem : element->children())
    {
      if (elem->localName() == "polymarker") elem->remove();
      if (elem->localName() == "polyline") elem->remove();
    }

  if (element->parentElement()->hasAttribute("marginalheatmap_kind"))
    {
      double y_max = 0;
      unsigned int z_length = 0;
      std::vector<double> z_vec;

      element->setAttribute("calc_window_and_viewport_from_parent", 1);

      auto z = static_cast<std::string>(element_context->getAttribute("z"));
      z_vec = GRM::get<std::vector<double>>((*context)[z]);
      z_length = z_vec.size();

      std::vector<double> xi_vec((is_vertical ? y_length : x_length));
      (*context)["xi" + str] = xi_vec;
      element->setAttribute("xi", "xi" + str);

      processCalcWindowAndViewportFromParent(element);
      processMarginalheatmapKind(element->parentElement());
    }
  else
    {
      if (x_length != y_length) throw std::length_error("For stairs series x- and y-data must have the same size.\n");

      const char *spec_char = spec.c_str();
      mask = gr_uselinespec((char *)spec_char);

      if (int_equals_any(mask, 5, 0, 1, 3, 4, 5))
        {
          std::string where = PLOT_DEFAULT_STEP_WHERE;
          if (element->hasAttribute("step_where"))
            {
              where = static_cast<std::string>(element->getAttribute("step_where"));
            }
          else
            {
              element->setAttribute("step_where", where);
            }
          if (where == "pre")
            {
              std::vector<double> x_step_boundaries(2 * x_length - 1);
              std::vector<double> y_step_values(2 * x_length - 1);

              x_step_boundaries[0] = x_vec[0];
              for (i = 1; i < 2 * x_length - 2; i += 2)
                {
                  x_step_boundaries[i] = x_vec[i / 2];
                  x_step_boundaries[i + 1] = x_vec[i / 2 + 1];
                }
              y_step_values[0] = y_vec[0];
              for (i = 1; i < 2 * x_length - 1; i += 2)
                {
                  y_step_values[i] = y_step_values[i + 1] = y_vec[i / 2 + 1];
                }

              if (is_vertical)
                {
                  element->append(
                      global_render->createPolyline(str + "x", y_step_values, str + "y", x_step_boundaries));
                }
              else
                {
                  element->append(
                      global_render->createPolyline(str + "x", x_step_boundaries, str + "y", y_step_values));
                }
            }
          else if (where == "post")
            {
              std::vector<double> x_step_boundaries(2 * x_length - 1);
              std::vector<double> y_step_values(2 * x_length - 1);
              for (i = 0; i < 2 * x_length - 2; i += 2)
                {
                  x_step_boundaries[i] = x_vec[i / 2];
                  x_step_boundaries[i + 1] = x_vec[i / 2 + 1];
                }
              x_step_boundaries[2 * x_length - 2] = x_vec[x_length - 1];
              for (i = 0; i < 2 * x_length - 2; i += 2)
                {
                  y_step_values[i] = y_step_values[i + 1] = y_vec[i / 2];
                }
              y_step_values[2 * x_length - 2] = y_vec[x_length - 1];

              if (is_vertical)
                {
                  element->append(
                      global_render->createPolyline(str + "x", y_step_values, str + "y", x_step_boundaries));
                }
              else
                {
                  element->append(
                      global_render->createPolyline(str + "x", x_step_boundaries, str + "y", y_step_values));
                }
            }
          else if (where == "mid")
            {
              std::vector<double> x_step_boundaries(2 * x_length);
              std::vector<double> y_step_values(2 * x_length);
              x_step_boundaries[0] = x_vec[0];
              for (i = 1; i < 2 * x_length - 2; i += 2)
                {
                  x_step_boundaries[i] = x_step_boundaries[i + 1] = (x_vec[i / 2] + x_vec[i / 2 + 1]) / 2.0;
                }
              x_step_boundaries[2 * x_length - 1] = x_vec[x_length - 1];
              for (i = 0; i < 2 * x_length - 1; i += 2)
                {
                  y_step_values[i] = y_step_values[i + 1] = y_vec[i / 2];
                }

              if (is_vertical)
                {
                  element->append(
                      global_render->createPolyline(str + "x", y_step_values, str + "y", x_step_boundaries));
                }
              else
                {
                  element->append(
                      global_render->createPolyline(str + "x", x_step_boundaries, str + "y", y_step_values));
                }
            }
        }
      if (mask & 2)
        {
          if (is_vertical)
            {
              element->append(global_render->createPolyline(str + "x", y_vec, str + "y", x_vec));
            }
          else
            {
              element->append(global_render->createPolyline(str + "x", x_vec, str + "y", y_vec));
            }
        }
    }
}

static void stem(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for stem
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double stem_x[2], stem_y[2] = {0.0};
  std::string orientation = PLOT_DEFAULT_ORIENTATION, spec = SERIES_DEFAULT_SPEC;
  int is_vertical;
  double x_min, x_max, y_min, y_max;
  unsigned int x_length, y_length;
  unsigned int i;
  std::vector<double> x_vec, y_vec;

  if (!element->hasAttribute("x")) throw NotFoundError("Stem series is missing required attribute x-data.\n");
  auto x = static_cast<std::string>(element->getAttribute("x"));
  if (!element->hasAttribute("y")) throw NotFoundError("Stem series is missing required attribute y-data.\n");
  auto y = static_cast<std::string>(element->getAttribute("y"));
  if (element->hasAttribute("orientation"))
    {
      orientation = static_cast<std::string>(element->getAttribute("orientation"));
    }
  else
    {
      element->setAttribute("orientation", orientation);
    }
  if (element->hasAttribute("spec"))
    {
      spec = static_cast<std::string>(element->getAttribute("spec"));
    }
  else
    {
      element->setAttribute("spec", spec);
    }

  is_vertical = orientation == "vertical";
  x_vec = GRM::get<std::vector<double>>((*context)[x]);
  y_vec = GRM::get<std::vector<double>>((*context)[y]);
  x_length = x_vec.size();
  y_length = y_vec.size();
  if (x_length != y_length) throw std::length_error("For stem series x- and y-data must have the same size.\n");

  if (element->parentElement()->hasAttribute("yrange_min"))
    stem_y[0] = static_cast<double>(element->parentElement()->getAttribute("yrange_min"));

  global_render->setLineSpec(element, spec);
  element->append(global_render->createYLine());

  for (i = 0; i < x_length; ++i)
    {
      stem_x[0] = stem_x[1] = x_vec[i];
      stem_y[1] = y_vec[i];
      if (is_vertical)
        {
          element->append(global_render->createPolyline(stem_y[0], stem_y[1], stem_x[0], stem_x[1]));
        }
      else
        {
          element->append(global_render->createPolyline(stem_x[0], stem_x[1], stem_y[0], stem_y[1]));
        }
    }

  int id = static_cast<int>(global_root->getAttribute("id"));
  std::string str = std::to_string(id);
  if (is_vertical)
    {
      element->append(
          global_render->createPolymarker("y" + str, y_vec, "x" + str, x_vec, nullptr, GKS_K_MARKERTYPE_SOLID_CIRCLE));
    }
  else
    {
      element->append(
          global_render->createPolymarker("x" + str, x_vec, "y" + str, y_vec, nullptr, GKS_K_MARKERTYPE_SOLID_CIRCLE));
    }
}

static void shade(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /* char *spec = ""; TODO: read spec from data! */
  int xform = 5, xbins = 1200, ybins = 1200, n;
  unsigned int point_count;
  unsigned int x_length, y_length;
  std::vector<double> x_vec, y_vec;
  double *x_p, *y_p;

  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));

  x_vec = GRM::get<std::vector<double>>((*context)[x]);
  y_vec = GRM::get<std::vector<double>>((*context)[y]);
  x_length = x_vec.size();
  y_length = y_vec.size();

  if (element->hasAttribute("xform")) xform = static_cast<int>(element->getAttribute("xform"));
  if (element->hasAttribute("xbins")) xbins = static_cast<int>(element->getAttribute("xbins"));
  if (element->hasAttribute("ybins")) ybins = static_cast<int>(element->getAttribute("ybins"));

  x_p = &(x_vec[0]);
  y_p = &(y_vec[0]);
  n = std::min(x_vec.size(), y_vec.size());

  gr_shadepoints(n, x_p, y_p, xform, xbins, ybins);
}

static void surface(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for surface
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  int accelerate = PLOT_DEFAULT_ACCELERATE; /* this argument decides if GR3 or GRM is used to plot the surface */
  std::vector<double> x_vec, y_vec, z_vec;
  unsigned int x_length, y_length, z_length;
  double x_min, x_max, y_min, y_max;

  if (element->hasAttribute("accelerate"))
    {
      accelerate = static_cast<int>(element->getAttribute("accelerate"));
    }
  else
    {
      element->setAttribute("accelerate", accelerate);
    }

  if (element->hasAttribute("x"))
    {
      auto x = static_cast<std::string>(element->getAttribute("x"));
      x_vec = GRM::get<std::vector<double>>((*context)[x]);
      x_length = x_vec.size();
    }
  if (element->hasAttribute("y"))
    {
      auto y = static_cast<std::string>(element->getAttribute("y"));
      y_vec = GRM::get<std::vector<double>>((*context)[y]);
      y_length = y_vec.size();
    }

  if (!element->hasAttribute("z")) throw NotFoundError("Surface series is missing required attribute z-data.\n");

  auto z = static_cast<std::string>(element->getAttribute("z"));
  z_vec = GRM::get<std::vector<double>>((*context)[z]);
  z_length = z_vec.size();

  if (x_vec.empty() && y_vec.empty())
    {
      /* If neither `x` nor `y` are given, we need more information about the shape of `z` */
      if (!element->hasAttribute("zdims_min") || !element->hasAttribute("zdims_max"))
        throw NotFoundError("Surface series is missing required attribute zdims.\n");
      x_length = static_cast<int>(element->getAttribute("zdims_min"));
      y_length = static_cast<int>(element->getAttribute("zdims_max"));
    }
  else if (x_vec.empty())
    {
      x_length = z_length / y_length;
    }
  else if (y_vec.empty())
    {
      y_length = z_length / x_length;
    }

  if (x_vec.empty())
    {
      x_min = static_cast<double>(element->parentElement()->getAttribute("xrange_min"));
      x_max = static_cast<double>(element->parentElement()->getAttribute("xrange_max"));
    }
  else
    {
      x_min = x_vec[0];
      x_max = x_vec[x_length - 1];
    }
  if (y_vec.empty())
    {
      y_min = static_cast<double>(element->parentElement()->getAttribute("yrange_min"));
      y_max = static_cast<double>(element->parentElement()->getAttribute("yrange_max"));
    }
  else
    {
      y_min = y_vec[0];
      y_max = y_vec[y_length - 1];
    }

  if (x_vec.empty())
    {
      std::vector<double> tmp(x_length);
      for (int j = 0; j < x_length; ++j)
        {
          tmp[j] = (int)(x_min + (x_max - x_min) / x_length * j + 0.5);
        }
      x_vec = tmp;
    }
  if (y_vec.empty())
    {
      std::vector<double> tmp(y_length);
      for (int j = 0; j < y_length; ++j)
        {
          tmp[j] = (int)(y_min + (y_max - y_min) / y_length * j + 0.5);
        }
      y_vec = tmp;
    }

  if (x_length == y_length && x_length == z_length)
    {
      logger((stderr, "Create a %d x %d grid for \"surface\" with \"gridit\"\n", PLOT_SURFACE_GRIDIT_N,
              PLOT_SURFACE_GRIDIT_N));

      std::vector<double> gridit_x_vec(PLOT_SURFACE_GRIDIT_N);
      std::vector<double> gridit_y_vec(PLOT_SURFACE_GRIDIT_N);
      std::vector<double> gridit_z_vec(PLOT_SURFACE_GRIDIT_N * PLOT_SURFACE_GRIDIT_N);

      double *gridit_x = &(gridit_x_vec[0]);
      double *gridit_y = &(gridit_y_vec[0]);
      double *gridit_z = &(gridit_z_vec[0]);
      double *x_p = &(x_vec[0]);
      double *y_p = &(y_vec[0]);
      double *z_p = &(z_vec[0]);
      gr_gridit(x_length, x_p, y_p, z_p, PLOT_SURFACE_GRIDIT_N, PLOT_SURFACE_GRIDIT_N, gridit_x, gridit_y, gridit_z);

      x_vec = std::vector<double>(gridit_x, gridit_x + PLOT_SURFACE_GRIDIT_N);
      y_vec = std::vector<double>(gridit_y, gridit_y + PLOT_SURFACE_GRIDIT_N);
      z_vec = std::vector<double>(gridit_z, gridit_z + PLOT_SURFACE_GRIDIT_N * PLOT_SURFACE_GRIDIT_N);

      x_length = y_length = PLOT_SURFACE_GRIDIT_N;
    }
  else
    {
      logger((stderr, "x_length; %u, y_length: %u, z_length: %u\n", x_length, y_length, z_length));
      if (x_length * y_length != z_length)
        throw std::length_error("For surface series x_length * y_length must be z_length.\n");
    }

  if (!accelerate)
    {
      double *px_p = &(x_vec[0]);
      double *py_p = &(y_vec[0]);
      double *pz_p = &(z_vec[0]);

      gr_surface(x_length, y_length, px_p, py_p, pz_p, GR_OPTION_COLORED_MESH);
    }
  else
    {
      std::vector<float> px_vec_f(x_vec.begin(), x_vec.end());
      std::vector<float> py_vec_f(y_vec.begin(), y_vec.end());
      std::vector<float> pz_vec_f(z_vec.begin(), z_vec.end());

      float *px_p = &(px_vec_f[0]);
      float *py_p = &(py_vec_f[0]);
      float *pz_p = &(pz_vec_f[0]);

      gr3_surface(x_length, y_length, px_p, py_p, pz_p, GR_OPTION_COLORED_MESH);
    }
}

static void line(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for line
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  std::string orientation = PLOT_DEFAULT_ORIENTATION, spec = SERIES_DEFAULT_SPEC;
  std::vector<double> x_vec, y_vec;
  unsigned int x_length = 0, y_length = 0;

  if (element->hasAttribute("orientation"))
    {
      orientation = static_cast<std::string>(element->getAttribute("orientation"));
    }
  else
    {
      element->setAttribute("orientation", orientation);
    }

  if (!element->hasAttribute("y")) throw NotFoundError("Line series is missing required attribute y-data.\n");
  auto y = static_cast<std::string>(element->getAttribute("y"));
  y_vec = GRM::get<std::vector<double>>((*context)[y]);
  y_length = y_vec.size();

  int mask;
  if (!element->hasAttribute("x"))
    {
      int i;
      x_length = y_length;
      for (i = 0; i < y_length; ++i) /* julia starts with 1, so GRM starts with 1 to be consistent */
        {
          x_vec.push_back(i + 1);
        }
    }
  else
    {
      auto x = static_cast<std::string>(element->getAttribute("x"));
      x_vec = GRM::get<std::vector<double>>((*context)[x]);
      x_length = x_vec.size();
    }
  if (x_length != y_length) throw std::length_error("For line series x- and y-data must have the same size.\n");

  if (element->hasAttribute("spec"))
    {
      spec = static_cast<std::string>(element->getAttribute("spec"));
    }
  else
    {
      element->setAttribute("spec", spec);
    }
  const char *spec_char = spec.c_str();
  mask = gr_uselinespec((char *)spec_char);

  // clear old line
  for (auto elem : element->children())
    {
      if (elem->localName() == "polyline") elem->remove();
    }

  if (int_equals_any(mask, 5, 0, 1, 3, 4, 5))
    {
      int current_line_colorind;
      gr_inqlinecolorind(&current_line_colorind);
      int id = static_cast<int>(global_root->getAttribute("id"));
      std::string str = std::to_string(id);
      std::shared_ptr<GRM::Element> line;
      if (orientation == "horizontal")
        {
          line = global_render->createPolyline(str + "x", x_vec, str + "y", y_vec);
        }
      else
        {
          line = global_render->createPolyline(str + "x", y_vec, str + "y", x_vec);
        }
      global_root->setAttribute("id", ++id);
      line->setAttribute("linecolorind", current_line_colorind);
      element->append(line);
    }
  if (mask & 2)
    {
      int current_marker_colorind;
      gr_inqmarkercolorind(&current_marker_colorind);
      int id = static_cast<int>(global_root->getAttribute("id"));
      std::string str = std::to_string(id);
      std::shared_ptr<GRM::Element> line;
      if (orientation == "horizontal")
        {
          line = global_render->createPolyline(str + "x", x_vec, str + "y", y_vec);
        }
      else
        {
          line = global_render->createPolyline(str + "x", y_vec, str + "y", x_vec);
        }
      global_root->setAttribute("id", ++id);
      line->setAttribute("markercolorind", current_marker_colorind);
      element->append(line);
    }
}

static void marginalheatmap(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for marginalheatmap
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */

  double c_min, c_max;
  int flip, options;
  int xind = PLOT_DEFAULT_MARGINAL_INDEX, yind = PLOT_DEFAULT_MARGINAL_INDEX;
  unsigned int i, j, k;
  std::string algorithm = PLOT_DEFAULT_MARGINAL_ALGORITHM, marginalheatmap_kind = PLOT_DEFAULT_MARGINAL_KIND;
  std::vector<double> bins;
  unsigned int num_bins_x = 0, num_bins_y = 0, n = 0;
  std::shared_ptr<GRM::Element> subGroup;

  if (element->hasAttribute("xind"))
    {
      xind = static_cast<int>(element->getAttribute("xind"));
    }
  else
    {
      element->setAttribute("xind", xind);
    }
  if (element->hasAttribute("yind"))
    {
      yind = static_cast<int>(element->getAttribute("yind"));
    }
  else
    {
      element->setAttribute("yind", yind);
    }
  if (element->hasAttribute("algorithm"))
    {
      algorithm = static_cast<std::string>(element->getAttribute("algorithm"));
    }
  else
    {
      element->setAttribute("algorithm", algorithm);
    }
  if (element->hasAttribute("marginalheatmap_kind"))
    {
      marginalheatmap_kind = static_cast<std::string>(element->getAttribute("marginalheatmap_kind"));
    }
  else
    {
      element->setAttribute("marginalheatmap_kind", marginalheatmap_kind);
    }

  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto x_vec = GRM::get<std::vector<double>>((*context)[x]);
  num_bins_x = x_vec.size();

  auto y = static_cast<std::string>(element->getAttribute("y"));
  auto y_vec = GRM::get<std::vector<double>>((*context)[y]);
  num_bins_y = y_vec.size();

  auto plot = static_cast<std::string>(element->getAttribute("z"));
  auto plot_vec = GRM::get<std::vector<double>>((*context)[plot]);
  n = plot_vec.size();

  // remove all old entry's
  for (const auto &child : element->children())
    {
      child->remove();
    }

  int id = static_cast<int>(global_root->getAttribute("id"));
  std::string str = std::to_string(id);

  auto heatmap = global_render->createSeries("heatmap");
  element->append(heatmap);

  for (k = 0; k < 2; k++)
    {
      double x_min, x_max, y_min, y_max, value, bin_max = 0;
      int bar_color_index = 989;
      double bar_color_rgb[3] = {-1};
      int edge_color_index = 1;
      double edge_color_rgb[3] = {-1};

      x_min = static_cast<double>(element->parentElement()->getAttribute("xrange_min"));
      x_max = static_cast<double>(element->parentElement()->getAttribute("xrange_max"));
      y_min = static_cast<double>(element->parentElement()->getAttribute("yrange_min"));
      y_max = static_cast<double>(element->parentElement()->getAttribute("yrange_max"));
      if (element->parentElement()->hasAttribute("lim_cmin"))
        {
          c_min = static_cast<double>(element->parentElement()->getAttribute("lim_cmin"));
        }
      else
        {
          c_min = static_cast<double>(element->parentElement()->getAttribute("lim_zmin"));
        }
      if (element->parentElement()->hasAttribute("lim_cmax"))
        {
          c_max = static_cast<double>(element->parentElement()->getAttribute("lim_cmax"));
        }
      else
        {
          c_max = static_cast<double>(element->parentElement()->getAttribute("lim_zmax"));
        }

      if (marginalheatmap_kind == "all")
        {
          unsigned int x_len = num_bins_x, y_len = num_bins_y;

          bins = std::vector<double>((k == 0) ? num_bins_y : num_bins_x);

          for (i = 0; i < ((k == 0) ? num_bins_y : num_bins_x); i++)
            {
              bins[i] = 0;
            }
          for (i = 0; i < y_len; i++)
            {
              for (j = 0; j < x_len; j++)
                {
                  value = (grm_isnan(plot[i * num_bins_x + j])) ? 0 : plot_vec[i * num_bins_x + j];
                  if (algorithm == "sum")
                    {
                      bins[(k == 0) ? i : j] += value;
                    }
                  else if (algorithm == "max")
                    {
                      bins[(k == 0) ? i : j] = grm_max(bins[(k == 0) ? i : j], value);
                    }
                }
              if (k == 0)
                {
                  bin_max = grm_max(bin_max, bins[i]);
                }
            }
          if (k == 1)
            {
              for (i = 0; i < x_len; i++)
                {
                  bin_max = grm_max(bin_max, bins[i]);
                }
            }
          for (i = 0; i < ((k == 0) ? y_len : x_len); i++)
            {
              bins[i] = (bin_max == 0) ? 0 : bins[i] / bin_max * (c_max / 15);
            }

          subGroup = global_render->createSeries("hist");
          element->append(subGroup);

          std::vector<double> bar_color_rgb_vec(bar_color_rgb, bar_color_rgb + 3);
          (*context)["bar_color_rgb" + str] = bar_color_rgb_vec;
          subGroup->setAttribute("bar_color_rgb", "bar_color_rgb" + str);
          subGroup->setAttribute("bar_color_index", bar_color_index);

          std::vector<double> edge_color_rgb_vec(edge_color_rgb, edge_color_rgb + 3);
          (*context)["edge_color_rgb" + str] = edge_color_rgb_vec;
          subGroup->setAttribute("edge_color_rgb", "edge_color_rgb" + str);
          subGroup->setAttribute("edge_color_index", edge_color_index);

          (*context)["bins" + str] = bins;
          subGroup->setAttribute("bins", "bins" + str);
        }
      else if (marginalheatmap_kind == "line" && xind != -1 && yind != -1)
        {
          subGroup = global_render->createSeries("stairs");
          element->append(subGroup);

          subGroup->setAttribute("kind", "marginalheatmap");
          subGroup->setAttribute("spec", "");
        }

      if (k == 0)
        {
          subGroup->setAttribute("orientation", "vertical");
        }
      else
        {
          subGroup->setAttribute("orientation", "horizontal");
        }

      for (const auto &child : element->children())
        {
          if (static_cast<std::string>(child->getAttribute("name")) == "hist" ||
              static_cast<std::string>(child->getAttribute("name")) == "stairs")
            {
              if (element->parentElement()->hasAttribute("xflip"))
                {
                  if (static_cast<int>(element->getAttribute("xflip")))
                    {
                      child->setAttribute("gr_option_flip_y", 1);
                      child->setAttribute("gr_option_flip_x", 0);
                    }
                }
              else if (element->parentElement()->hasAttribute("yflip"))
                {
                  if (static_cast<int>(element->getAttribute("yflip")))
                    {
                      child->setAttribute("gr_option_flip_y", 0);
                      child->setAttribute("gr_option_flip_x", 0);
                    }
                }
              else
                {
                  child->setAttribute("gr_option_flip_x", 0);
                }
            }
        }
      global_root->setAttribute("id", ++id);
    }
}

/*!
 * \brief Set colors from color index or rgb arrays. The render version
 *
 * Call the function first with an argument container and a key. Afterwards, call the `set_next_color` with `nullptr`
 * pointers to iterate through the color arrays. If `key` does not exist in `args`, the function falls back to default
 * colors.
 *
 * \param key The key of the colors in the argument container. The key may reference integer or double arrays.
 *            Integer arrays describe colors of the GKS color table (0 - 1255). Double arrays contain RGB tuples in the
 *            range [0.0, 1.0]. If key does not exist, the routine falls back to default colors (taken from
 *            `gr_uselinespec`).
 * \param color_type The color type to set. Can be one of `GR_COLOR_LINE`, `GR_COLOR_MARKER`, `GR_COLOR_FILL`,
 *                   `GR_COLOR_TEXT`, `GR_COLOR_BORDER` or any combination of them (combined with OR). The special value
 *                   `GR_COLOR_RESET` resets all color modifications.
 */
static int set_next_color(std::string key, gr_color_type_t color_type, const std::shared_ptr<GRM::Element> &element,
                          const std::shared_ptr<GRM::Context> &context)
{
  std::vector<int> fallback_color_indices = {989, 982, 980, 981, 996, 983, 995, 988, 986, 990,
                                             991, 984, 992, 993, 994, 987, 985, 997, 998, 999};
  static double saved_color[3];
  static int last_array_index = -1;
  static std::vector<int> color_indices;
  static std::vector<double> color_rgb_values;
  static unsigned int color_array_length = -1;
  int current_array_index = last_array_index + 1;
  int color_index = 0;
  int reset = (color_type == GR_COLOR_RESET);
  int gks_errind = GKS_K_NO_ERROR;

  if (reset || !key.empty())
    {
      if (last_array_index >= 0 && !color_rgb_values.empty())
        {
          gr_setcolorrep(PLOT_CUSTOM_COLOR_INDEX, saved_color[0], saved_color[1], saved_color[2]);
        }
      last_array_index = -1;
      if (!reset && !key.empty())
        {
          if (!element->hasAttribute("color_indices") && !element->hasAttribute("color_rgb_values"))
            {
              /* use fallback colors if `key` cannot be read from `args` */
              logger((stderr, "Cannot read \"%s\" from args, falling back to default colors\n", key.c_str()));
              color_indices = fallback_color_indices;
              color_array_length = size(fallback_color_indices);
            }
          else
            {
              if (element->hasAttribute("color_indices"))
                {
                  auto c = static_cast<std::string>(element->getAttribute("color_indices"));
                  color_indices = GRM::get<std::vector<int>>((*context)[c]);
                  color_array_length = color_indices.size();
                }
              else if (element->hasAttribute("color_rgb_values"))
                {
                  auto c = static_cast<std::string>(element->getAttribute("color_rgb_values"));
                  color_rgb_values = GRM::get<std::vector<double>>((*context)[c]);
                  color_array_length = color_rgb_values.size();
                }
            }
        }
      else
        {
          color_array_length = -1;
        }

      if (reset)
        {
          color_indices.clear();
          color_rgb_values.clear();
          return 0;
        }
      return 0;
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
      last_array_index = current_array_index + 2;
      global_render->setColorRep(element, PLOT_CUSTOM_COLOR_INDEX, color_rgb_values[current_array_index],
                                 color_rgb_values[current_array_index + 1], color_rgb_values[current_array_index + 2]);
    }

  if (color_type & GR_COLOR_LINE)
    {
      global_render->setLineColorInd(element, color_index);
    }
  if (color_type & GR_COLOR_MARKER)
    {
      global_render->setMarkerColorInd(element, color_index);
    }
  if (color_type & GR_COLOR_FILL)
    {
      global_render->setFillColorInd(element, color_index);
    }
  if (color_type & GR_COLOR_TEXT)
    {
      global_render->setTextColorInd(element, color_index);
    }
  if (color_type & GR_COLOR_BORDER)
    {
      global_render->setBorderColorInd(element, color_index);
    }
  return color_index;
}

static void pie(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for pie
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  unsigned int x_length;
  int color_index;
  double start_angle, middle_angle, end_angle;
  double text_pos[2];
  char text[80];
  std::string title;
  unsigned int i;

  global_render->setFillIntStyle(element, GKS_K_INTSTYLE_SOLID);
  global_render->setTextAlign(element, GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_HALF);

  if (!element->hasAttribute("x")) throw NotFoundError("Pie series is missing required attribute x-data.\n");
  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto x_vec = GRM::get<std::vector<double>>((*context)[x]);
  x_length = x_vec.size();

  std::vector<double> normalized_x(x_length);
  GRM::normalize_vec(x_vec, &normalized_x);
  std::vector<unsigned int> normalized_x_int(x_length);
  GRM::normalize_vec_int(x_vec, &normalized_x_int, 1000);

  start_angle = 90;
  color_index = set_next_color("c", GR_COLOR_FILL, element, context);

  // clear old pie
  for (auto elem : element->children())
    {
      if (elem->localName() == "text") elem->remove();
      if (elem->localName() == "fillarc") elem->remove();
    }
  for (i = 0; i < x_length; ++i)
    {
      end_angle = start_angle - normalized_x[i] * 360.0;
      auto temp = global_render->createFillArc(0.05, 0.95, 0.05, 0.95, start_angle, end_angle);
      element->append(temp);

      color_index = set_next_color("", GR_COLOR_FILL, temp, context);
      processFillColorInd(temp);

      middle_angle = (start_angle + end_angle) / 2.0;

      text_pos[0] = 0.5 + 0.25 * cos(middle_angle * M_PI / 180.0);
      text_pos[1] = 0.5 + 0.25 * sin(middle_angle * M_PI / 180.0);

      snprintf(text, 80, "%.2lf\n%.1lf %%", x_vec[i], normalized_x_int[i] / 10.0);

      auto text_elem = global_render->createText(text_pos[0], text_pos[1], text, CoordinateSpace::WC);
      text_elem->setAttribute("color_index", color_index);
      element->append(text_elem);

      text_elem->setAttribute("set_text_color_for_background", true);
      processFillColorInd(text_elem);
      processTextColorForBackground(text_elem);

      start_angle = end_angle;
      if (start_angle < 0)
        {
          start_angle += 360.0;
        }
    }
  set_next_color("", GR_COLOR_RESET, element, context);
  processFillColorInd(element);
  processFillIntStyle(element);
  processTextAlign(element);
}

static void plot3(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for plot3
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  unsigned int x_length, y_length, z_length;

  if (!element->hasAttribute("x")) throw NotFoundError("Plot3 series is missing required attribute x-data.\n");
  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto x_vec = GRM::get<std::vector<double>>((*context)[x]);
  x_length = x_vec.size();

  if (!element->hasAttribute("y")) throw NotFoundError("Plot3 series is missing required attribute y-data.\n");
  auto y = static_cast<std::string>(element->getAttribute("y"));
  auto y_vec = GRM::get<std::vector<double>>((*context)[y]);
  y_length = y_vec.size();

  if (!element->hasAttribute("z")) throw NotFoundError("Plot3 series is missing required attribute z-data.\n");
  auto z = static_cast<std::string>(element->getAttribute("z"));
  auto z_vec = GRM::get<std::vector<double>>((*context)[z]);
  z_length = z_vec.size();

  if (x_length != y_length || x_length != z_length)
    throw std::length_error("For plot3 series x-, y- and z-data must have the same size.\n");

  // clear old line
  for (auto elem : element->children())
    {
      if (elem->localName() == "polyline3d") elem->remove();
    }

  int id_int = static_cast<int>(global_root->getAttribute("id"));
  global_root->setAttribute("id", ++id_int);
  std::string id = std::to_string(id_int);

  element->append(global_render->createPolyline3d("x" + id, x_vec, "y" + id, y_vec, "z" + id, z_vec));
}

static void imshow(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for imshow
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double c_min, c_max;
  unsigned int c_data_length, i, j, k;
  int grplot = 0;
  int rows, cols;

  if (element->parentElement()->hasAttribute("grplot"))
    {
      grplot = static_cast<int>(element->parentElement()->getAttribute("grplot"));
    }
  if (!element->hasAttribute("c_min")) throw NotFoundError("Imshow series is missing required attribute c_min.\n");
  c_min = static_cast<double>(element->getAttribute("c_min"));
  if (!element->hasAttribute("c_max")) throw NotFoundError("Imshow series is missing required attribute c_max.\n");
  c_max = static_cast<double>(element->getAttribute("c_max"));
  logger((stderr, "Got min, max %lf %lf\n", c_min, c_max));

  std::vector<double> c_data_vec, shape_vec;

  if (!element->hasAttribute("c")) throw NotFoundError("Imshow series is missing required attribute c-data.\n");
  auto x = static_cast<std::string>(element->getAttribute("c"));
  if (!element->hasAttribute("cdims")) throw NotFoundError("Imshow series is missing required attribute cdims-data.\n");
  auto y = static_cast<std::string>(element->getAttribute("cdims"));

  c_data_vec = GRM::get<std::vector<double>>((*context)[x]);
  shape_vec = GRM::get<std::vector<double>>((*context)[y]);
  c_data_length = c_data_vec.size();
  i = shape_vec.size();
  if (i != 2) throw std::length_error("The size of shape data from imshow has to be 2.\n");
  if (shape_vec[0] * shape_vec[1] != c_data_length)
    throw std::length_error("For imshow shape[0] * shape[1] must be c_data_length.\n");

  cols = (int)shape_vec[0];
  rows = (int)shape_vec[1];

  std::vector<int> img_data(c_data_length);

  k = 0;
  for (j = 0; j < rows; ++j)
    {
      for (i = 0; i < cols; ++i)
        {
          img_data[k++] = 1000 + (int)grm_round((1.0 * c_data_vec[j * cols + i] - c_min) / (c_max - c_min) * 255);
        }
    }

  int id_int = static_cast<int>(global_root->getAttribute("id"));
  global_root->setAttribute("id", ++id_int);
  std::string id = std::to_string(id_int);

  auto x_tmp = static_cast<double *>(malloc((cols) * sizeof(double)));
  auto y_tmp = static_cast<double *>(malloc((rows) * sizeof(double)));
  linspace(0, cols - 1, cols, x_tmp);
  linspace(0, rows - 1, rows, y_tmp);

  std::vector<double> x_vec(x_tmp, x_tmp + cols);
  (*context)["x" + id] = x_vec;
  element->setAttribute("x", "x" + id);
  std::vector<double> y_vec(y_tmp, y_tmp + rows);
  (*context)["y" + id] = y_vec;
  element->setAttribute("y", "y" + id);
  (*context)["z" + id] = c_data_vec;
  element->setAttribute("z", "z" + id);
  (*context)["img_data_key" + id] = img_data;
  element->setAttribute("img_data_key", "img_data_key" + id);

  global_render->setSelntran(element, 0);
  global_render->setScale(element, 0);
  processScale(element);
  processSelntran(element);

  double x_min, x_max, y_min, y_max;
  double vp[4];
  int scale;
  std::string image_data_key = static_cast<std::string>(element->getAttribute("img_data_key"));
  std::shared_ptr<GRM::Element> ancestor = element->parentElement();
  bool vp_found = false;

  // Get vp from ancestor GRM::element, usually the q"plot-group"
  while (ancestor->localName() != "figure")
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
      throw NotFoundError("No vp was found within ancestors\n");
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

  /* remove old cell arrays if they exist */
  auto imshow_elements = element->children();
  for (auto &imshow_element : imshow_elements)
    {
      if (static_cast<std::string>(imshow_element->getAttribute("name")) == "imshow") imshow_element->remove();
    }

  auto temp = global_render->createCellArray(x_min, x_max, y_min, y_max, cols, rows, 1, 1, cols, rows, image_data_key,
                                             std::nullopt);
  temp->setAttribute("name", "imshow");

  element->append(temp);
}

static void text(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for text
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
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

static void titles3d(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for titles3d
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  std::string x, y, z;
  x = static_cast<std::string>(element->getAttribute("x"));
  y = static_cast<std::string>(element->getAttribute("y"));
  z = static_cast<std::string>(element->getAttribute("z"));
  gr_titles3d(x.data(), y.data(), z.data());
}

static void triContour(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for tricontour
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double z_min, z_max;
  int num_levels = PLOT_DEFAULT_TRICONT_LEVELS;
  int i;
  unsigned int x_length, y_length, z_length;
  std::vector<double> x_vec, y_vec, z_vec;

  z_min = static_cast<double>(element->parentElement()->getAttribute("lim_zmin"));
  z_max = static_cast<double>(element->parentElement()->getAttribute("lim_zmax"));
  if (element->hasAttribute("levels"))
    {
      num_levels = static_cast<int>(element->getAttribute("levels"));
    }
  else
    {
      element->setAttribute("levels", num_levels);
    }

  std::vector<double> levels(num_levels);

  for (i = 0; i < num_levels; ++i)
    {
      levels[i] = z_min + ((1.0 * i) / (num_levels - 1)) * (z_max - z_min);
    }

  if (!element->hasAttribute("px")) throw NotFoundError("Tricontour series is missing required attribute px-data.\n");
  auto x = static_cast<std::string>(element->getAttribute("px"));
  if (!element->hasAttribute("py")) throw NotFoundError("Tricontour series is missing required attribute py-data.\n");
  auto y = static_cast<std::string>(element->getAttribute("py"));
  if (!element->hasAttribute("pz")) throw NotFoundError("Tricontour series is missing required attribute pz-data.\n");
  auto z = static_cast<std::string>(element->getAttribute("pz"));

  x_vec = GRM::get<std::vector<double>>((*context)[x]);
  y_vec = GRM::get<std::vector<double>>((*context)[y]);
  z_vec = GRM::get<std::vector<double>>((*context)[z]);
  x_length = x_vec.size();
  y_length = y_vec.size();
  z_length = z_vec.size();

  if (x_length != y_length || x_length != z_length)
    throw std::length_error("For tricontour series x-, y- and z-data must have the same size.\n");

  double *px_p = &(x_vec[0]);
  double *py_p = &(y_vec[0]);
  double *pz_p = &(z_vec[0]);
  double *l_p = &(levels[0]);

  gr_tricontour(x_length, px_p, py_p, pz_p, num_levels, l_p);
}

static void triSurface(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for trisurface
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  if (!element->hasAttribute("px")) throw NotFoundError("Trisurface series is missing required attribute px-data.\n");
  auto px = static_cast<std::string>(element->getAttribute("px"));
  if (!element->hasAttribute("py")) throw NotFoundError("Trisurface series is missing required attribute py-data.\n");
  auto py = static_cast<std::string>(element->getAttribute("py"));
  if (!element->hasAttribute("pz")) throw NotFoundError("Trisurface series is missing required attribute pz-data.\n");
  auto pz = static_cast<std::string>(element->getAttribute("pz"));

  std::vector<double> px_vec = GRM::get<std::vector<double>>((*context)[px]);
  std::vector<double> py_vec = GRM::get<std::vector<double>>((*context)[py]);
  std::vector<double> pz_vec = GRM::get<std::vector<double>>((*context)[pz]);

  int nx = px_vec.size();
  int ny = py_vec.size();
  int nz = pz_vec.size();
  if (nx != ny || nx != nz)
    throw std::length_error("For trisurface series px-, py- and pz-data must have the same size.\n");

  double *px_p = &(px_vec[0]);
  double *py_p = &(py_vec[0]);
  double *pz_p = &(pz_vec[0]);

  gr_trisurface(nx, px_p, py_p, pz_p);
}

static void volume(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  double dlim[2] = {INFINITY, -INFINITY};
  unsigned int c_length, dims;
  int algorithm = PLOT_DEFAULT_VOLUME_ALGORITHM;
  std::string algorithm_str;
  double dmin, dmax;
  int width, height;
  double device_pixel_ratio;

  if (!element->hasAttribute("c")) throw NotFoundError("Volume series is missing required attribute c-data.\n");
  auto c = static_cast<std::string>(element->getAttribute("c"));
  auto c_vec = GRM::get<std::vector<double>>((*context)[c]);
  c_length = c_vec.size();

  if (!element->hasAttribute("c_dims")) throw NotFoundError("Volume series is missing required attribute c_dims.\n");
  auto c_dims = static_cast<std::string>(element->getAttribute("c_dims"));
  auto shape_vec = GRM::get<std::vector<int>>((*context)[c_dims]);
  dims = shape_vec.size();

  if (dims != 3) throw std::length_error("For volume series the size of c_dims has to be 3.\n");
  if (shape_vec[0] * shape_vec[1] * shape_vec[2] != c_length)
    throw std::length_error("For volume series shape[0] * shape[1] * shape[2] must be c_length.\n");
  if (c_length <= 0) throw NotFoundError("For volume series the size of c has to be greater than 0.\n");

  if (!element->hasAttribute("algorithm"))
    {
      element->setAttribute("algorithm", algorithm);
    }
  else
    {
      if (element->getAttribute("algorithm").isInt())
        {
          algorithm = static_cast<int>(element->getAttribute("algorithm"));
        }
      else if (element->getAttribute("algorithm").isString())
        {
          algorithm_str = static_cast<std::string>(element->getAttribute("algorithm"));
          if (algorithm_str == "emission")
            {
              algorithm = GR_VOLUME_EMISSION;
            }
          else if (algorithm_str == "absorption")
            {
              algorithm = GR_VOLUME_ABSORPTION;
            }
          else if (algorithm_str == "mip" || algorithm_str == "maximum")
            {
              algorithm = GR_VOLUME_MIP;
            }
          else
            {
              logger((stderr, "Got unknown volume algorithm \"%s\"\n", algorithm_str.c_str()));
              throw std::logic_error("For volume series the given algorithm is unknown.\n");
            }
        }
      else
        {
          throw NotFoundError("Volume series is missing attribute algorithm.\n");
        }
    }
  if (algorithm != GR_VOLUME_ABSORPTION && algorithm != GR_VOLUME_EMISSION && algorithm != GR_VOLUME_MIP)
    {
      logger((stderr, "Got unknown volume algorithm \"%d\"\n", algorithm));
      throw std::logic_error("For volume series the given algorithm is unknown.\n");
    }

  dmin = dmax = -1.0;
  if (element->hasAttribute("dmin")) dmin = static_cast<double>(element->getAttribute("dmin"));
  if (element->hasAttribute("dmax")) dmax = static_cast<double>(element->getAttribute("dmax"));

  gr_inqvpsize(&width, &height, &device_pixel_ratio);
  gr_setpicturesizeforvolume((int)(width * device_pixel_ratio), (int)(height * device_pixel_ratio));
  gr_volume(shape_vec[0], shape_vec[1], shape_vec[2], &(c_vec[0]), algorithm, &dmin, &dmax);

  auto parent_element = element->parentElement();
  if (parent_element->hasAttribute("dlim_min") && parent_element->hasAttribute("dlim_max"))
    {
      dlim[0] = static_cast<double>(parent_element->getAttribute("dlim_min"));
      dlim[1] = static_cast<double>(parent_element->getAttribute("dlim_max"));
      dlim[0] = grm_min(dlim[0], dmin);
      dlim[1] = grm_max(dlim[1], dmax);
    }
  else
    {
      dlim[0] = dmin;
      dlim[1] = dmax;
    }
  parent_element->setAttribute("dlim_min", dlim[0]);
  parent_element->setAttribute("dlim_max", dlim[1]);

  auto colorbar = parent_element->querySelectors("colorbar");
  colorbar->setAttribute("lim_cmin", dlim[0]);
  colorbar->setAttribute("lim_cmax", dlim[1]);
}

static void wireframe(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for wireframe
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  unsigned int x_length, y_length, z_length;

  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));
  auto z = static_cast<std::string>(element->getAttribute("z"));

  std::vector<double> x_vec = GRM::get<std::vector<double>>((*context)[x]);
  std::vector<double> y_vec = GRM::get<std::vector<double>>((*context)[y]);
  std::vector<double> z_vec = GRM::get<std::vector<double>>((*context)[z]);
  x_length = x_vec.size();
  y_length = y_vec.size();
  z_length = z_vec.size();

  global_render->setFillColorInd(element, 0);
  processFillColorInd(element);

  int id_int = static_cast<int>(global_root->getAttribute("id"));
  global_root->setAttribute("id", ++id_int);
  std::string id = std::to_string(id_int);

  if (x_length == y_length && x_length == z_length)
    {
      std::vector<double> gridit_x_vec(PLOT_WIREFRAME_GRIDIT_N);
      std::vector<double> gridit_y_vec(PLOT_WIREFRAME_GRIDIT_N);
      std::vector<double> gridit_z_vec(PLOT_WIREFRAME_GRIDIT_N * PLOT_WIREFRAME_GRIDIT_N);

      double *gridit_x = &(gridit_x_vec[0]);
      double *gridit_y = &(gridit_y_vec[0]);
      double *gridit_z = &(gridit_z_vec[0]);
      double *x_p = &(x_vec[0]);
      double *y_p = &(y_vec[0]);
      double *z_p = &(z_vec[0]);

      gr_gridit(x_length, x_p, y_p, z_p, PLOT_WIREFRAME_GRIDIT_N, PLOT_WIREFRAME_GRIDIT_N, gridit_x, gridit_y,
                gridit_z);

      x_vec = std::vector<double>(gridit_x, gridit_x + PLOT_WIREFRAME_GRIDIT_N);
      y_vec = std::vector<double>(gridit_y, gridit_y + PLOT_WIREFRAME_GRIDIT_N);
      z_vec = std::vector<double>(gridit_z, gridit_z + PLOT_WIREFRAME_GRIDIT_N * PLOT_WIREFRAME_GRIDIT_N);
    }
  else
    {
      if (x_length * y_length != z_length)
        throw std::length_error("For wireframe series x_length * y_length must be z_length.\n");
    }

  double *px_p = &(x_vec[0]);
  double *py_p = &(y_vec[0]);
  double *pz_p = &(z_vec[0]);
  gr_surface(x_length, y_length, px_p, py_p, pz_p, GR_OPTION_FILLED_MESH);
}

static void ProcessSeries(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  static std::map<std::string,
                  std::function<void(const std::shared_ptr<GRM::Element> &, const std::shared_ptr<GRM::Context> &)>>
      seriesNameToFunc{
          {std::string("contour"), contour},
          {std::string("contourf"), contourf},
          {std::string("heatmap"), heatmap},
          {std::string("hexbin"), hexbin},
          {std::string("hist"), hist},
          {std::string("imshow"), imshow},
          {std::string("isosurface"), isosurface},
          {std::string("line"), line},
          {std::string("marginalheatmap"), marginalheatmap},
          {std::string("pie"), pie},
          {std::string("plot3"), plot3},
          {std::string("polar"), polar},
          {std::string("polar_heatmap"), polar_heatmap},
          {std::string("quiver"), quiver},
          {std::string("scatter"), scatter},
          {std::string("scatter3"), scatter3},
          {std::string("shade"), shade},
          {std::string("stairs"), stairs},
          {std::string("stem"), stem},
          {std::string("surface"), surface},
          {std::string("tricontour"), triContour},
          {std::string("trisurface"), triSurface},
          {std::string("volume"), volume},
          {std::string("wireframe"), wireframe},
      };

  try
    {
      std::function<void(const std::shared_ptr<GRM::Element> &, const std::shared_ptr<GRM::Context> &)> f =
          seriesNameToFunc[static_cast<std::string>(element->getAttribute("name"))];
      f(element, context);
    }
  catch (std::bad_function_call &e)
    {
      fprintf(stderr,
              "Series is not in render implemented yet\n"); // todo: when all kinds are implemented here throw an error
    }
}

static void processElement(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for all kinds of elements
   *
   * \param[in] element The GRM::Element containing attributes and data keys
   * \param[in] context The GRM::Context containing the actual data
   */

  //! Map used for processing all kinds of elements
  static std::map<std::string,
                  std::function<void(const std::shared_ptr<GRM::Element> &, const std::shared_ptr<GRM::Context> &)>>
      elemStringToFunc{
          {std::string("axes"), axes},
          {std::string("axes3d"), axes3d},
          {std::string("cellarray"), cellArray},
          {std::string("colorbar"), colorbar},
          {std::string("errorbars"), errorbars},
          {std::string("legend"), legend},
          {std::string("polar_axes"), drawPolarAxes},
          {std::string("drawarc"), drawArc},
          {std::string("drawgraphics"), drawGraphics},
          {std::string("drawimage"), drawImage},
          {std::string("drawrect"), drawRect},
          {std::string("fillarc"), fillArc},
          {std::string("fillarea"), fillArea},
          {std::string("fillrect"), fillRect},
          {std::string("gr3clear"), gr3Clear},
          {std::string("gr3deletemesh"), gr3DeleteMesh},
          {std::string("gr3drawimage"), gr3DrawImage},
          {std::string("gr3drawmesh"), gr3DrawMesh},
          {std::string("grid"), grid},
          {std::string("grid3d"), grid3d},
          {std::string("isosurface_render"), isosurfaceRender},
          {std::string("layout_grid"), layoutGrid},
          {std::string("layout_gridelement"), layoutGridElement},
          {std::string("nonuniform_polarcellarray"), nonUniformPolarCellArray},
          {std::string("nonuniformcellarray"), nonuniformcellarray},
          {std::string("panzoom"), panzoom},
          {std::string("polarcellarray"), polarCellArray},
          {std::string("polyline"), polyline},
          {std::string("polyline3d"), polyline3d},
          {std::string("polymarker"), polymarker},
          {std::string("polymarker3d"), polymarker3d},
          {std::string("series"), ProcessSeries},
          {std::string("text"), text},
          {std::string("titles3d"), titles3d},
          {std::string("y_line"), drawYLine},
      };
  /*! Modifier */
  if (str_equals_any(element->localName().c_str(), 5, "group", "figure", "plot", "coordinate_system", "label"))
    {
      processAttributes(element);
    }
  else
    {
      /*! Drawnodes */
      processAttributes(element);
      try
        {
          std::function<void(const std::shared_ptr<GRM::Element> &, const std::shared_ptr<GRM::Context> &)> f =
              elemStringToFunc[element->localName()];
          f(element, context);
        }
      catch (std::bad_function_call &e)
        {
          throw NotFoundError("No dom render function found for element with local name: " + element->localName() +
                              "\n");
        }
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ render functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static bool pushToZQueue(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  if (!element->hasAttribute("z_index"))
    {
      return false;
    }
  int z_index = static_cast<int>(element->getAttribute("z_index"));
  if (z_queue_is_being_rendered)
    {
      if (z_index < current_z_index)
        {
          throw ParentHasToBeProcessedBeforeChildError(
              "The z-index of a child can`t be smaller than it`s parents z-index");
        }
      /* check if element is already in z_queue */
      if (element == z_queue.top())
        {
          return false;
        }
    }
  int gr_context_id = grContextIDManager.getUnusedGRContextId();
  gr_savestateincontext(gr_context_id);
  element->setAttribute("_grcontext", gr_context_id);
  z_queue.push(element);
  return true;
}

static void renderHelper(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Recursive helper function for render; Not part of render class
   * Only renders / processes children if the parent is in parentTypes (group etc.)
   * Used for traversing the tree
   *
   * \param[in] element A GRM::Element
   * \param[in] context A GRM::Context
   */
  gr_savestate();

  if (!pushToZQueue(element, context))
    {
      bool bounding_boxes = getenv("GRPLOT_ENABLE_EDITOR");

      if (bounding_boxes && element->hasAttributes())
        {
          gr_begin_grm_selection(bounding_id, &receiverfunction);
          bounding_map[bounding_id] = element;
          bounding_id++;
        }

      processElement(element, context);
      if (element->hasChildNodes() && parentTypes.count(element->localName()))
        {
          for (const auto &child : element->children())
            {
              renderHelper(child, context);
            }
        }
      if (bounding_boxes && element->hasAttributes())
        {
          gr_end_grm_selection();
        }
    }

  gr_restorestate();
}

static void renderZQueue(const std::shared_ptr<GRM::Context> &context)
{
  z_queue_is_being_rendered = true;
  for (; !z_queue.empty(); z_queue.pop())
    {
      auto element = z_queue.top();
      int gr_context_id = static_cast<int>(element->getAttribute("_grcontext"));
      gr_selectcontext(gr_context_id);
      current_z_index = static_cast<int>(element->getAttribute("z_index"));
      renderHelper(element, context);
      element->removeAttribute("_grcontext");
      grContextIDManager.markIdAsUnused(gr_context_id);
    }
  z_queue_is_being_rendered = false;
  grContextIDManager.destroyGRContexts();
}

static void initializeGridElements(const std::shared_ptr<GRM::Element> &element, grm::Grid *grid)
{
  if (element->hasChildNodes())
    {
      for (const auto &child : element->children())
        {
          if (child->localName() != "layout_gridelement" && child->localName() != "layout_grid")
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

          if (child->localName() == "layout_gridelement")
            {
              grm::GridElement *curGridElement =
                  new grm::GridElement(absHeight, absWidth, absHeightPxl, absWidthPxl, fitParentsHeight,
                                       fitParentsWidth, relativeHeight, relativeWidth, aspectRatio);
              curGridElement->elementInDOM = child;
              grid->setElement(slice, curGridElement);
            }

          if (child->localName() == "layout_grid")
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

static void finalizeGrid(const std::shared_ptr<GRM::Element> &root)
{
  grm::Grid *rootGrid = nullptr;
  if (root->hasChildNodes())
    {
      for (const auto &child : root->children())
        {
          if (child->localName() == "layout_grid")
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

static void applyRootDefaults(std::shared_ptr<GRM::Element> root)
{
  if (!root->hasAttribute("clearws")) root->setAttribute("clearws", PLOT_DEFAULT_CLEAR);
  if (!root->hasAttribute("updatews")) root->setAttribute("updatews", PLOT_DEFAULT_UPDATE);
  if (!root->hasAttribute("size"))
    {
      global_root->setAttribute("size_x", PLOT_DEFAULT_WIDTH);
      global_root->setAttribute("size_y", PLOT_DEFAULT_HEIGHT);
      global_root->setAttribute("size_x_type", "double");
      global_root->setAttribute("size_y_type", "double");
      global_root->setAttribute("size_x_unit", "px");
      global_root->setAttribute("size_y_unit", "px");
    }

  for (auto child : root->children())
    {
      if (child->localName() == "plot")
        {
          if (!child->hasAttribute("kind")) child->setAttribute("kind", PLOT_DEFAULT_KIND);
          if (!child->hasAttribute("keep_aspect_ratio"))
            child->setAttribute("keep_aspect_ratio", PLOT_DEFAULT_KEEP_ASPECT_RATIO);
          if (!child->hasAttribute("subplot"))
            {
              child->setAttribute("subplot", true);
              child->setAttribute("subplot_xmin", PLOT_DEFAULT_SUBPLOT_MIN_X);
              child->setAttribute("subplot_xmax", PLOT_DEFAULT_SUBPLOT_MAX_X);
              child->setAttribute("subplot_ymin", PLOT_DEFAULT_SUBPLOT_MIN_Y);
              child->setAttribute("subplot_ymax", PLOT_DEFAULT_SUBPLOT_MAX_Y);
            }
          auto kind = static_cast<std::string>(child->getAttribute("kind"));
          if (!child->hasAttribute("adjust_xlim"))
            {
              if (kind == "heatmap" || kind == "marginalheatmap")
                {
                  child->setAttribute("adjust_xlim", 0);
                }
              else
                {
                  child->setAttribute("adjust_xlim", (child->hasAttribute("xlim_min") ? 0 : PLOT_DEFAULT_ADJUST_XLIM));
                }
            }
          if (!child->hasAttribute("adjust_ylim"))
            {
              if (kind == "heatmap" || kind == "marginalheatmap")
                {
                  child->setAttribute("adjust_ylim", 0);
                }
              else
                {
                  child->setAttribute("adjust_ylim", (child->hasAttribute("ylim_min") ? 0 : PLOT_DEFAULT_ADJUST_YLIM));
                }
            }
          if (!child->hasAttribute("adjust_zlim"))
            {
              if (kind != "heatmap" && kind != "marginalheatmap")
                {
                  child->setAttribute("adjust_zlim", (child->hasAttribute("zlim_min") ? 0 : PLOT_DEFAULT_ADJUST_ZLIM));
                }
            }
          if (!child->hasAttribute("linespec")) child->setAttribute("linespec", " ");
          if (!child->hasAttribute("xlog")) child->setAttribute("xlog", PLOT_DEFAULT_XLOG);
          if (!child->hasAttribute("ylog")) child->setAttribute("ylog", PLOT_DEFAULT_YLOG);
          if (!child->hasAttribute("zlog")) child->setAttribute("zlog", PLOT_DEFAULT_ZLOG);
          if (!child->hasAttribute("xflip")) child->setAttribute("xflip", PLOT_DEFAULT_XFLIP);
          if (!child->hasAttribute("yflip")) child->setAttribute("yflip", PLOT_DEFAULT_YFLIP);
          if (!child->hasAttribute("zflip")) child->setAttribute("zflip", PLOT_DEFAULT_ZFLIP);
          if (!child->hasAttribute("resample_method"))
            child->setAttribute("resample_method", (int)PLOT_DEFAULT_RESAMPLE_METHOD);
          if (!child->hasAttribute("font")) child->setAttribute("font", PLOT_DEFAULT_FONT);
          if (!child->hasAttribute("font_precision"))
            child->setAttribute("font_precision", PLOT_DEFAULT_FONT_PRECISION);
          if (!child->hasAttribute("colormap")) child->setAttribute("colormap", PLOT_DEFAULT_COLORMAP);
        }
    }
}

void GRM::Render::render(const std::shared_ptr<GRM::Document> &document,
                         const std::shared_ptr<GRM::Context> &extContext)
{
  /*!
   * static GRM::Render::render receiving external document and context
   *
   * \param[in] document A GRM::Document that will be rendered
   * \param[in] extContext A GRM::Context
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

void GRM::Render::render(std::shared_ptr<GRM::Document> const &document)
{
  /*!
   * GRM::Render::render that receives an external document but uses the GRM::Render instance's context.
   *
   * \param[in] document A GRM::Document that will be rendered
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

void GRM::Render::render(const std::shared_ptr<GRM::Context> &extContext)
{
  /*!
   *GRM::Render::render uses GRM::Render instance's document and an external context
   *
   * \param[in] extContext A GRM::Context
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

void GRM::Render::render()
{
  /*!
   * GRM::Render::render uses both instance's document and context
   */
  auto root = this->firstChildElement();
  global_root = root;
  const unsigned int indent = 2;

  bounding_id = 0;
  global_render = (std::dynamic_pointer_cast<GRM::Render>(root->ownerDocument()))
                      ? std::dynamic_pointer_cast<GRM::Render>(root->ownerDocument())
                      : GRM::Render::createRender();
  applyRootDefaults(root);
  std::cerr << toXML(root, GRM::SerializerOptions{std::string(indent, ' ')}) << "\n";
  if (static_cast<int>(root->getAttribute("clearws"))) gr_clearws();
  renderHelper(root, this->context);
  renderZQueue(this->context);
  if (static_cast<int>(root->getAttribute("updatews"))) gr_updatews();
  std::cerr << toXML(root, GRM::SerializerOptions{std::string(indent, ' ')}) << "\n";
}

std::shared_ptr<GRM::Render> GRM::Render::createRender()
{
  /*!
   * This function can be used to create a Render object
   */
  auto render = std::shared_ptr<Render>(new Render());
  return render;
}

GRM::Render::Render()
{
  /*!
   * This is the constructor for GRM::Render
   */
  this->context = std::shared_ptr<GRM::Context>(new Context());
}

std::shared_ptr<GRM::Context> GRM::Render::getContext()
{
  return context;
}

/*
 * Searches in elementToTooltip for attributeName and returns a string vector
 * containing:
 * [0] The default value for this attribute
 * [1] The description for this attribute
 */
std::vector<std::string> GRM::Render::getDefaultAndTooltip(const std::shared_ptr<Element> &element,
                                                           std::string attributeName)
{
  static std::unordered_map<std::string, std::map<std::string, std::vector<std::string>>> elementToTooltip{
      {std::string("polyline"),
       std::map<std::string, std::vector<std::string>>{
           {std::string("linecolorind"),
            std::vector<std::string>{"989", "Sets the linecolor according to the current colormap"}},
           {std::string("x"), std::vector<std::string>{"None", "References the x-values stored in the context"}},
           {std::string("y"), std::vector<std::string>{"None", "References the y-values stored in the context"}},
           {std::string("x1"), std::vector<std::string>{"None", "The beginning x-coordinate"}},
           {std::string("y1"), std::vector<std::string>{"None", "The beginning y-coordinate"}},
           {std::string("x2"), std::vector<std::string>{"None", "The ending x-coordinate"}},
           {std::string("y2"), std::vector<std::string>{"None", "The ending y-coordinate"}},
       }},
      {std::string("text"),
       std::map<std::string, std::vector<std::string>>{
           {std::string("x"), std::vector<std::string>{"None", "X-position of the text"}},
           {std::string("y"), std::vector<std::string>{"None", "Y-position of the text"}},
           {std::string("text"), std::vector<std::string>{"Title", "The text diplayed by this node"}},
           {std::string("render_method"),
            std::vector<std::string>{"gr_text", "Render method used to display the text"}},
           {std::string("textalign"), std::vector<std::string>{"1", "Use alignment"}},
           {std::string("textalign_horizontal"), std::vector<std::string>{"2", "Horizontal alignment method"}},
           {std::string("textalign_vertical"), std::vector<std::string>{"1", "Horizontal alignment method"}},
       }},
      {std::string("grid"),
       std::map<std::string, std::vector<std::string>>{
           {std::string("x_tick"),
            std::vector<std::string>{"1", "The interval between minor tick marks on the X-axis"}},
           {std::string("y_tick"),
            std::vector<std::string>{"1", "The interval between minor tick marks on the Y-axis"}},
           {std::string("x_org"),
            std::vector<std::string>{"0", "The world coordinates of the origin (point of intersection) of the X-axis"}},
           {std::string("y_org"),
            std::vector<std::string>{"0", "The world coordinates of the origin (point of intersection) of the Y-axis"}},
           {std::string("major_x"),
            std::vector<std::string>{"5", "Unitless integer values specifying the number of minor tick intervals "
                                          "between major tick marks. Values of 0 or 1 imply no minor ticks. Negative "
                                          "values specify no labels will be drawn for the X-axis"}},
           {std::string("major_y"),
            std::vector<std::string>{"5", "Unitless integer values specifying the number of minor tick intervals "
                                          "between major tick marks. Values of 0 or 1 imply no minor ticks. Negative "
                                          "values specify no labels will be drawn for the Y-axis"}},
       }}};
  if (elementToTooltip.count(element->localName()) &&
      elementToTooltip[element->localName().c_str()].count(attributeName))
    {
      return elementToTooltip[element->localName().c_str()][attributeName];
    }
  else
    {
      return std::vector<std::string>{"", "No description found"};
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ create functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

std::shared_ptr<GRM::Element>
GRM::Render::createPolymarker(const std::string &x_key, std::optional<std::vector<double>> x, const std::string &y_key,
                              std::optional<std::vector<double>> y, const std::shared_ptr<GRM::Context> &extContext,
                              int marker_type, double marker_size, int marker_colorind)
{
  /*!
   * This function can be used to create a Polymarker GRM::Element
   *
   * \param[in] x_key A string used for storing the x coordinates in GRM::Context
   * \param[in] x A vector containing double values representing x coordinates
   * \param[in] y_key A string used for storing the y coordinates in GRM::Context
   * \param[in] y A vector containing double values representing y coordinates
   * \param[in] extContext A GRM::Context that is used for storing the vectors. By default it uses GRM::Render's
   * GRM::Context object but an external GRM::Context can be used \param[in] marker_type An Integer setting the
   * gr_markertype. By default it is 0 \param[in] marker_size A Double value setting the gr_markersize. By default it is
   * 0.0 \param[in] marker_colorind An Integer setting the gr_markercolorind. By default it is 0
   */

  std::shared_ptr<GRM::Context> useContext = (extContext == nullptr) ? context : extContext;
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

std::shared_ptr<GRM::Element> GRM::Render::createPolymarker(double x, double y, int marker_type, double marker_size,
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

std::shared_ptr<GRM::Element> GRM::Render::createPolyline(double x1, double x2, double y1, double y2, int line_type,
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

std::shared_ptr<GRM::Element>
GRM::Render::createPolyline(const std::string &x_key, std::optional<std::vector<double>> x, const std::string &y_key,
                            std::optional<std::vector<double>> y, const std::shared_ptr<GRM::Context> &extContext,
                            int line_type, double line_width, int line_colorind)
{
  /*!
   * This function can be used to create a Polyline GRM::Element
   *
   * \param[in] x_key A string used for storing the x coordinates in GRM::Context
   * \param[in] x A vector containing double values representing x coordinates
   * \param[in] y_key A string used for storing the y coordinates in GRM::Context
   * \param[in] y A vector containing double values representing y coordinates
   * \param[in] extContext A GRM::Context that is used for storing the vectors. By default it uses GRM::Render's
   * GRM::Context object but an external GRM::Context can be used \param[in] line_type An Integer setting the
   * gr_linertype. By default it is 0 \param[in] line_width A Double value setting the gr_linewidth. By default it is
   * 0.0 \param[in] marker_colorind An Integer setting the gr_linecolorind. By default it is 0
   */

  std::shared_ptr<GRM::Context> useContext = (extContext == nullptr) ? context : extContext;
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

std::shared_ptr<GRM::Element> GRM::Render::createText(double x, double y, const std::string &text,
                                                      CoordinateSpace space)
{
  /*!
   * This function can be used to create a Text GRM::Element
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

std::shared_ptr<GRM::Element>
GRM::Render::createFillArea(const std::string &x_key, std::optional<std::vector<double>> x, const std::string &y_key,
                            std::optional<std::vector<double>> y, const std::shared_ptr<GRM::Context> &extContext,
                            int fillintstyle, int fillstyle, int fillcolorind)
{
  /*!
   * This function can be used to create a FillArea GRM::Element
   *
   * \param[in] n The number of data points
   * \param[in] x_key A string used for storing the x coordinates in GRM::Context
   * \param[in] x A vector containing double values representing x coordinates
   * \param[in] y_key A string used for storing the y coordinates in GRM::Context
   * \param[in] y A vector containing double values representing y coordinates
   * \param[in] extContext A GRM::Context that is used for storing the vectors. By default it uses GRM::Render's
   * GRM::Context object but an external GRM::Context can be used \param[in] fillintstyle An Integer setting the
   * gr_fillintstyle. By default it is 0 \param[in] fillstyle An Integer setting the gr_fillstyle. By default it is 0
   */

  std::shared_ptr<GRM::Context> useContext = (extContext == nullptr) ? context : extContext;
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

std::shared_ptr<GRM::Element> GRM::Render::createCellArray(double xmin, double xmax, double ymin, double ymax, int dimx,
                                                           int dimy, int scol, int srow, int ncol, int nrow,
                                                           const std::string &color_key,
                                                           std::optional<std::vector<int>> color,
                                                           const std::shared_ptr<GRM::Context> &extContext)
{
  /*!
   * This function can be used to create a CellArray GRM::Element
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
   * \param[in] extContext A GRM::Context used for storing color. By default it uses GRM::Render's GRM::Context object
   * but an external GRM::Context can be used
   */

  std::shared_ptr<GRM::Context> useContext = (extContext == nullptr) ? context : extContext;
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

std::shared_ptr<GRM::Element> GRM::Render::createAxes(double x_tick, double y_tick, double x_org, double y_org,
                                                      int x_major, int y_major, int tick_orientation)
{
  /*!
   * This function can be used for creating an Axes GRM::Element
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

std::shared_ptr<GRM::Element> GRM::Render::createEmptyAxes(int tick_orientation)
{
  /*!
   * This function can be used for creating an Axes GRM::Element with missing information
   *
   * \param[in] tick_orientation A Int value specifing the direction of the ticks
   */
  auto element = createElement("axes");
  element->setAttribute("tick_orientation", tick_orientation);
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createLegend(const std::string &labels_key,
                                                        std::optional<std::vector<std::string>> labels,
                                                        const std::string &specs_key,
                                                        std::optional<std::vector<std::string>> specs,
                                                        const std::shared_ptr<GRM::Context> &extContext)
{
  /*!
   * This function can be used for creating a legend GRM::Element
   * This element is different compared to most of Render's GRM::Element, the legend GRM::Element will incorporate
   * plot_draw_legend code from plot.cxx and will create new GRM::Elements as child nodes in the render document
   *
   * \param[in] labels_key A std::string for the labels vector
   * \param[in] labels May be an std::vector<std::string>> containing the labels
   * \param[in] spec An std::string
   */

  auto element = createElement("legend");
  std::shared_ptr<GRM::Context> useContext = (extContext == nullptr) ? context : extContext;
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

std::shared_ptr<GRM::Element> GRM::Render::createDrawPolarAxes(int angle_ticks, const std::string &kind, int phiflip,
                                                               const std::string &norm, double tick, double line_width)
{
  auto element = createElement("polar_axes");
  if (!norm.empty())
    {
      element->setAttribute("norm", norm);
    }
  if (tick != 0.0)
    {
      element->setAttribute("tick", tick);
    }
  if (line_width != 0.0)
    {
      element->setAttribute("linewidth", line_width);
    }
  element->setAttribute("angle_ticks", angle_ticks);
  element->setAttribute("phiflip", phiflip);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createPieLegend(const std::string &labels_key,
                                                           std::optional<std::vector<std::string>> labels,
                                                           const std::shared_ptr<GRM::Context> &extContext)
{
  auto element = createElement("legend");
  std::shared_ptr<GRM::Context> useContext = (extContext == nullptr) ? context : extContext;
  element->setAttribute("labels", labels_key);

  if (labels != std::nullopt)
    {
      (*useContext)[labels_key] = *labels;
    }
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createGrid(double x_tick, double y_tick, double x_org, double y_org,
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

std::shared_ptr<GRM::Element> GRM::Render::createEmptyGrid(bool x_grid, bool y_grid)
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

std::shared_ptr<GRM::Element> GRM::Render::createGroup()
{
  /*!
   * This function can be used to create a group GRM::Element
   *
   * A group Element sets all attributes for its children
   */
  return createElement("group");
}

std::shared_ptr<GRM::Element> GRM::Render::createGroup(const std::string &name)
{
  auto element = createElement("group");
  element->setAttribute("name", name);
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createSeries(const std::string &name)
{
  auto element = createElement("series");
  element->setAttribute("name", name);
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createDrawImage(double xmin, double ymin, double xmax, double ymax,
                                                           int width, int height, const std::string &data_key,
                                                           std::optional<std::vector<int>> data, int model,
                                                           const std::shared_ptr<GRM::Context> &extContext)
{
  /*!
   * This function can be used to create a DrawImage GRM::Element
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
   * \param[in] extContext A GRM::Context used for storing data. By default it uses GRM::Render's GRM::Context object
   * but an external GRM::Context can be used
   */
  std::shared_ptr<GRM::Context> useContext = (extContext == nullptr) ? context : extContext;
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

std::shared_ptr<GRM::Element> GRM::Render::createDrawArc(double xmin, double xmax, double ymin, double ymax, double a1,
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

std::shared_ptr<GRM::Element> GRM::Render::createFillArc(double xmin, double xmax, double ymin, double ymax, double a1,
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

std::shared_ptr<GRM::Element> GRM::Render::createDrawRect(double xmin, double xmax, double ymin, double ymax)
{
  auto element = createElement("drawrect");
  element->setAttribute("xmin", xmin);
  element->setAttribute("xmax", xmax);
  element->setAttribute("ymin", ymin);
  element->setAttribute("ymax", ymax);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createFillRect(double xmin, double xmax, double ymin, double ymax,
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

std::shared_ptr<GRM::Element> GRM::Render::createQuiver(const std::string &x_key, std::optional<std::vector<double>> x,
                                                        const std::string &y_key, std::optional<std::vector<double>> y,
                                                        const std::string &u_key, std::optional<std::vector<double>> u,
                                                        const std::string &v_key, std::optional<std::vector<double>> v,
                                                        int color, const std::shared_ptr<GRM::Context> &extContext)
{
  /*
   * This function can be used to create a Quiver GRM::Element
   *
   */
  std::shared_ptr<GRM::Context> useContext = (extContext == nullptr) ? context : extContext;
  auto element = createSeries("quiver");
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

std::shared_ptr<GRM::Element> GRM::Render::createHexbin(const std::string &x_key, std::optional<std::vector<double>> x,
                                                        const std::string &y_key, std::optional<std::vector<double>> y,
                                                        const std::shared_ptr<GRM::Context> &extContext)
{
  /*!
   * This function can be used to create a hexbin GRM::Element
   */
  std::shared_ptr<GRM::Context> useContext = (extContext == nullptr) ? context : extContext;

  auto element = createSeries("hexbin");
  element->setAttribute("x", x_key);
  element->setAttribute("y", y_key);

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

std::shared_ptr<GRM::Element> GRM::Render::createColorbar(unsigned int colors,
                                                          const std::shared_ptr<GRM::Context> &extContext)
{
  /*!
   * This function can be used to create a colorbar GRM::Element
   */
  std::shared_ptr<GRM::Context> useContext = (extContext == nullptr) ? context : extContext;

  auto element = createElement("colorbar");
  element->setAttribute("colors", static_cast<int>(colors));

  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createPolarCellArray(double x_org, double y_org, double phimin,
                                                                double phimax, double rmin, double rmax, int dimphi,
                                                                int dimr, int scol, int srow, int ncol, int nrow,
                                                                const std::string &color_key,
                                                                std::optional<std::vector<int>> color,
                                                                const std::shared_ptr<Context> &extContext)
{
  /*!
   * Display a two dimensional color index array mapped to a disk using polar
   * coordinates.
   *
   * \param[in] x_org X coordinate of the disk center in world coordinates
   * \param[in] y_org Y coordinate of the disk center in world coordinates
   * \param[in] phimin start angle of the disk sector in degrees
   * \param[in] phimax end angle of the disk sector in degrees
   * \param[in] rmin inner radius of the punctured disk in world coordinates
   * \param[in] rmax outer radius of the disk in world coordinates
   * \param[in] dimphi Phi (X) dimension of the color index array
   * \param[in] dimr R (Y) dimension of the color index array
   * \param[in] scol number of leading columns in the color index array
   * \param[in] srow number of leading rows in the color index array
   * \param[in] ncol total number of columns in the color index array
   * \param[in] nrow total number of rows in the color index array
   * \param[in] color color index array
   *
   * The two dimensional color index array is mapped to the resulting image by
   * interpreting the X-axis of the array as the angle and the Y-axis as the radius.
   * The center point of the resulting disk is located at `x_org`, `y_org` and the
   * radius of the disk is `rmax`.
   *
   * To draw a contiguous array as a complete disk use:
   *
   *     gr_polarcellarray(x_org, y_org, 0, 360, 0, rmax, dimphi, dimr, 1, 1, dimphi, dimr, color)
   *
   * The additional parameters to the function can be used to further control the
   * mapping from polar to cartesian coordinates.
   *
   * If `rmin` is greater than 0 the input data is mapped to a punctured disk (or
   * annulus) with an inner radius of `rmin` and an outer radius `rmax`. If `rmin`
   * is greater than `rmax` the Y-axis of the array is reversed.
   *
   * The parameter `phimin` and `phimax` can be used to map the data to a sector
   * of the (punctured) disk starting at `phimin` and ending at `phimax`. If
   * `phimin` is greater than `phimax` the X-axis is reversed. The visible sector
   * is the one starting in mathematically positive direction (counterclockwise)
   * at the smaller angle and ending at the larger angle. An example of the four
   * possible options can be found below:
   *
   * \verbatim embed:rst:leading-asterisk
   *
   * +-----------+-----------+---------------------------------------------------+
   * |**phimin** |**phimax** |**Result**                                         |
   * +-----------+-----------+---------------------------------------------------+
   * |90         |270        |Left half visible, mapped counterclockwise         |
   * +-----------+-----------+---------------------------------------------------+
   * |270        |90         |Left half visible, mapped clockwise                |
   * +-----------+-----------+---------------------------------------------------+
   * |-90        |90         |Right half visible, mapped counterclockwise        |
   * +-----------+-----------+---------------------------------------------------+
   * |90         |-90        |Right half visible, mapped clockwise               |
   * +-----------+-----------+---------------------------------------------------+
   *
   * \endverbatim
   *
   * `scol` and `srow` can be used to specify a (1-based) starting column and row
   * in the `color` array. `ncol` and `nrow` specify the actual dimension of the
   * array in the memory whereof `dimphi` and `dimr` values are mapped to the disk.
   *
   */

  std::shared_ptr<GRM::Context> useContext = (extContext == nullptr) ? context : extContext;
  auto element = createElement("polarcellarray");
  element->setAttribute("x_org", x_org);
  element->setAttribute("y_org", y_org);
  element->setAttribute("phimin", phimin);
  element->setAttribute("phimax", phimax);
  element->setAttribute("rmin", rmin);
  element->setAttribute("rmax", rmax);
  element->setAttribute("dimphi", dimphi);
  element->setAttribute("dimr", dimr);
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

std::shared_ptr<GRM::Element> GRM::Render::createNonUniformPolarCellArray(
    double x_org, double y_org, const std::string &phi_key, std::optional<std::vector<double>> phi,
    const std::string &r_key, std::optional<std::vector<double>> r, int dimphi, int dimr, int scol, int srow, int ncol,
    int nrow, const std::string &color_key, std::optional<std::vector<int>> color,
    const std::shared_ptr<GRM::Context> &extContext)
{
  /*!
   * Display a two dimensional color index array mapped to a disk using polar
   * coordinates.
   *
   * \param[in] x_org X coordinate of the disk center in world coordinates
   * \param[in] y_org Y coordinate of the disk center in world coordinates
   * \param[in] phimin start angle of the disk sector in degrees
   * \param[in] phimax end angle of the disk sector in degrees
   * \param[in] rmin inner radius of the punctured disk in world coordinates
   * \param[in] rmax outer radius of the disk in world coordinates
   * \param[in] dimphi Phi (X) dimension of the color index array
   * \param[in] dimr R (Y) dimension of the color index array
   * \param[in] scol number of leading columns in the color index array
   * \param[in] srow number of leading rows in the color index array
   * \param[in] ncol total number of columns in the color index array
   * \param[in] nrow total number of rows in the color index array
   * \param[in] color color index array
   *
   * The two dimensional color index array is mapped to the resulting image by
   * interpreting the X-axis of the array as the angle and the Y-axis as the radius.
   * The center point of the resulting disk is located at `x_org`, `y_org` and the
   * radius of the disk is `rmax`.
   *
   * To draw a contiguous array as a complete disk use:
   *
   *     gr_polarcellarray(x_org, y_org, 0, 360, 0, rmax, dimphi, dimr, 1, 1, dimphi, dimr, color)
   *
   * The additional parameters to the function can be used to further control the
   * mapping from polar to cartesian coordinates.
   *
   * If `rmin` is greater than 0 the input data is mapped to a punctured disk (or
   * annulus) with an inner radius of `rmin` and an outer radius `rmax`. If `rmin`
   * is greater than `rmax` the Y-axis of the array is reversed.
   *
   * The parameter `phimin` and `phimax` can be used to map the data to a sector
   * of the (punctured) disk starting at `phimin` and ending at `phimax`. If
   * `phimin` is greater than `phimax` the X-axis is reversed. The visible sector
   * is the one starting in mathematically positive direction (counterclockwise)
   * at the smaller angle and ending at the larger angle. An example of the four
   * possible options can be found below:
   *
   * \verbatim embed:rst:leading-asterisk
   *
   * +-----------+-----------+---------------------------------------------------+
   * |**phimin** |**phimax** |**Result**                                         |
   * +-----------+-----------+---------------------------------------------------+
   * |90         |270        |Left half visible, mapped counterclockwise         |
   * +-----------+-----------+---------------------------------------------------+
   * |270        |90         |Left half visible, mapped clockwise                |
   * +-----------+-----------+---------------------------------------------------+
   * |-90        |90         |Right half visible, mapped counterclockwise        |
   * +-----------+-----------+---------------------------------------------------+
   * |90         |-90        |Right half visible, mapped clockwise               |
   * +-----------+-----------+---------------------------------------------------+
   *
   * \endverbatim
   *
   * `scol` and `srow` can be used to specify a (1-based) starting column and row
   * in the `color` array. `ncol` and `nrow` specify the actual dimension of the
   * array in the memory whereof `dimphi` and `dimr` values are mapped to the disk.
   *
   */

  std::shared_ptr<GRM::Context> useContext = (extContext == nullptr) ? context : extContext;
  auto element = createElement("nonuniform_polarcellarray");
  element->setAttribute("x_org", x_org);
  element->setAttribute("y_org", y_org);
  element->setAttribute("r", r_key);
  element->setAttribute("phi", phi_key);
  element->setAttribute("dimphi", dimphi);
  element->setAttribute("dimr", dimr);
  element->setAttribute("scol", scol);
  element->setAttribute("srow", srow);
  element->setAttribute("ncol", ncol);
  element->setAttribute("nrow", nrow);
  element->setAttribute("color", color_key);
  if (color != std::nullopt)
    {
      (*useContext)[color_key] = *color;
    }
  if (phi != std::nullopt)
    {
      (*useContext)[phi_key] = *phi;
    }
  if (r != std::nullopt)
    {
      (*useContext)[r_key] = *r;
    }
  return element;
}

std::shared_ptr<GRM::Element>
GRM::Render::createNonUniformCellArray(const std::string &x_key, std::optional<std::vector<double>> x,
                                       const std::string &y_key, std::optional<std::vector<double>> y, int dimx,
                                       int dimy, int scol, int srow, int ncol, int nrow, const std::string &color_key,
                                       std::optional<std::vector<int>> color,
                                       const std::shared_ptr<GRM::Context> &extContext)
{
  /*!
   * This function can be used to create a non uniform cell array GRM::Element
   */
  std::shared_ptr<GRM::Context> useContext = (extContext == nullptr) ? context : extContext;

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

std::shared_ptr<GRM::Element> GRM::Render::createGrid3d(double x_tick, double y_tick, double z_tick, double x_org,
                                                        double y_org, double z_org, int x_major, int y_major,
                                                        int z_major)
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

std::shared_ptr<GRM::Element> GRM::Render::createEmptyGrid3d(bool x_grid, bool y_grid, bool z_grid)
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

std::shared_ptr<GRM::Element> GRM::Render::createAxes3d(double x_tick, double y_tick, double z_tick, double x_org,
                                                        double y_org, double z_org, int major_x, int major_y,
                                                        int major_z, int tick_orientation)
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

std::shared_ptr<GRM::Element> GRM::Render::createEmptyAxes3d(int tick_orientation)
{
  auto element = createElement("axes3d");
  element->setAttribute("tick_orientation", tick_orientation);
  return element;
}

std::shared_ptr<GRM::Element>
GRM::Render::createPolyline3d(const std::string &x_key, std::optional<std::vector<double>> x, const std::string &y_key,
                              std::optional<std::vector<double>> y, const std::string &z_key,
                              std::optional<std::vector<double>> z, const std::shared_ptr<GRM::Context> &extContext)
{
  std::shared_ptr<GRM::Context> useContext = (extContext == nullptr) ? context : extContext;

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

std::shared_ptr<GRM::Element>
GRM::Render::createPolymarker3d(const std::string &x_key, std::optional<std::vector<double>> x,
                                const std::string &y_key, std::optional<std::vector<double>> y,
                                const std::string &z_key, std::optional<std::vector<double>> z,
                                const std::shared_ptr<GRM::Context> &extContext)
{
  std::shared_ptr<GRM::Context> useContext = (extContext == nullptr) ? context : extContext;

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

std::shared_ptr<GRM::Element> GRM::Render::createGR3DrawMesh(
    int mesh, int n, const std::string &positions_key, std::optional<std::vector<double>> positions,
    const std::string &directions_key, std::optional<std::vector<double>> directions, const std::string &ups_key,
    std::optional<std::vector<double>> ups, const std::string &colors_key, std::optional<std::vector<double>> colors,
    const std::string &scales_key, std::optional<std::vector<double>> scales,
    const std::shared_ptr<GRM::Context> &extContext)
{

  std::shared_ptr<GRM::Context> useContext = (extContext == nullptr) ? context : extContext;

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

std::shared_ptr<GRM::Element>
GRM::Render::createTriSurface(const std::string &px_key, std::optional<std::vector<double>> px,
                              const std::string &py_key, std::optional<std::vector<double>> py,
                              const std::string &pz_key, std::optional<std::vector<double>> pz,
                              const std::shared_ptr<GRM::Context> &extContext)
{
  std::shared_ptr<GRM::Context> useContext = (extContext == nullptr) ? context : extContext;
  auto element = createSeries("trisurface");
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

std::shared_ptr<GRM::Element> GRM::Render::createTitles3d(const std::string &x, const std::string &y,
                                                          const std::string &z)
{
  auto element = createElement("titles3d");
  element->setAttribute("x", x);
  element->setAttribute("y", y);
  element->setAttribute("z", z);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createGR3Clear()
{
  auto element = createElement("gr3clear");
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createGR3DeleteMesh(int mesh)
{
  auto element = createElement("gr3deletemesh");
  element->setAttribute("mesh", mesh);
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createGR3DrawImage(double xmin, double xmax, double ymin, double ymax,
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

std::shared_ptr<GRM::Element> GRM::Render::createDrawGraphics(const std::string &data_key,
                                                              std::optional<std::vector<int>> data,
                                                              const std::shared_ptr<GRM::Context> &extContext)
{
  std::shared_ptr<GRM::Context> useContext = (extContext == nullptr) ? context : extContext;

  auto element = createElement("drawgraphics");

  if (data != std::nullopt)
    {
      (*useContext)[data_key] = *data;
    }
  element->setAttribute("data", data_key);
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createIsoSurfaceRenderElement(int drawable_type)
{
  auto element = createElement("isosurface_render");
  element->setAttribute("drawable_type", drawable_type);
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createLayoutGrid(const grm::Grid &grid)
{
  auto element = createElement("layout_grid");

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

std::shared_ptr<GRM::Element> GRM::Render::createLayoutGridElement(const grm::GridElement &gridElement,
                                                                   const grm::Slice &slice)
{
  auto element = createElement("layout_gridelement");

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
  GRM::Render::setSubplot(element, subplot[0], subplot[1], subplot[2], subplot[3]);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createPanzoom(double x, double y, double xzoom, double yzoom)
{
  auto element = createElement("panzoom");
  element->setAttribute("x", x);
  element->setAttribute("y", y);
  element->setAttribute("xzoom", xzoom);
  element->setAttribute("yzoom", yzoom);
  return element;
}

std::shared_ptr<GRM::Element> GRM::Render::createYLine()
{
  auto element = createElement("y_line");
  return element;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ modifier functions~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void GRM::Render::setViewport(const std::shared_ptr<GRM::Element> &element, double xmin, double xmax, double ymin,
                              double ymax)
{
  /*!
   * This function can be used to set the viewport of a GRM::Element
   *
   * \param[in] element A GRM::Element
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


void GRM::Render::setWSViewport(const std::shared_ptr<GRM::Element> &element, double xmin, double xmax, double ymin,
                                double ymax)
{
  /*!
   * This function can be used to set the wsviewport of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] xmin The left horizontal coordinate of the viewport (0 <= xmin < xmax)
   * \param[in] xmax The right horizontal coordinate of the viewport (xmin < xmax <= 1)
   * \param[in] ymin TThe bottom vertical coordinate of the viewport (0 <= ymin < ymax)
   * \param[in] ymax The top vertical coordinate of the viewport (ymin < ymax <= 1)
   */

  element->setAttribute("wsviewport", true);
  element->setAttribute("wsviewport_xmin", xmin);
  element->setAttribute("wsviewport_xmax", xmax);
  element->setAttribute("wsviewport_ymin", ymin);
  element->setAttribute("wsviewport_ymax", ymax);
}

void GRM::Render::setWindow(const std::shared_ptr<Element> &element, double xmin, double xmax, double ymin, double ymax)
{
  /*!
   * This function can be used to set the window of a GRM::Element
   *
   * \param[in] element A GRM::Element
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

void GRM::Render::setWSWindow(const std::shared_ptr<Element> &element, double xmin, double xmax, double ymin,
                              double ymax)
{
  /*!
   * This function can be used to set the ws_window of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] xmin The left horizontal coordinate of the window (xmin < xmax)
   * \param[in] xmax The right horizontal coordinate of the window (xmin < xmax)
   * \param[in] ymin The bottom vertical coordinate of the window (ymin < ymax)
   * \param[in] ymax The top vertical coordinate of the window (ymin < ymax)
   */

  element->setAttribute("wswindow", true);
  element->setAttribute("wswindow_xmin", xmin);
  element->setAttribute("wswindow_xmax", xmax);
  element->setAttribute("wswindow_ymin", ymin);
  element->setAttribute("wswindow_ymax", ymax);
}

void GRM::Render::setMarkerType(const std::shared_ptr<Element> &element, int type)
{
  /*!
   * This function can be used to set a MarkerType of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] type An Integer setting the MarkerType
   */
  element->setAttribute("markertype", type);
}

void GRM::Render::setMarkerType(const std::shared_ptr<Element> &element, const std::string &types_key,
                                std::optional<std::vector<int>> types, const std::shared_ptr<GRM::Context> &extContext)
{
  /*!
   * This function can be used to set a vector of MarkerTypes of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] types_key A string used as a key for storing the types
   * \param[in] types A vector containing the MarkerTypes
   * \param[in] extContext A GRM::Context used for storing types. By default it uses GRM::Render's GRM::Context object
   * but an external GRM::Context can be used
   */
  std::shared_ptr<GRM::Context> useContext = (extContext == nullptr) ? context : extContext;
  if (types != std::nullopt)
    {
      (*useContext)[types_key] = *types;
    }
  element->setAttribute("markertypes", types_key);
}

void GRM::Render::setMarkerSize(const std::shared_ptr<Element> &element, double size)
{
  /*!
   * This function can be used to set a MarkerSize of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] type A Double setting the MarkerSize
   */
  element->setAttribute("markersize", size);
}

void GRM::Render::setMarkerSize(const std::shared_ptr<Element> &element, const std::string &sizes_key,
                                std::optional<std::vector<double>> sizes,
                                const std::shared_ptr<GRM::Context> &extContext)
{
  /*!
   * This function can be used to set a vector of MarkerTypes of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] sizes_key A string used as a key for storing the sizes
   * \param[in] sizes A vector containing the MarkerSizes
   * \param[in] extContext A GRM::Context used for storing sizes. By default it uses GRM::Render's GRM::Context object
   * but an external GRM::Context can be used
   */
  std::shared_ptr<GRM::Context> useContext = (extContext == nullptr) ? context : extContext;
  if (sizes != std::nullopt)
    {
      (*useContext)[sizes_key] = *sizes;
    }
  element->setAttribute("markersizes", sizes_key);
}

void GRM::Render::setMarkerColorInd(const std::shared_ptr<Element> &element, int color)
{
  /*!
   * This function can be used to set a MarkerColorInd of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] color An Integer setting the MarkerColorInd
   */
  element->setAttribute("markercolorind", color);
}

void GRM::Render::setMarkerColorInd(const std::shared_ptr<Element> &element, const std::string &colorinds_key,
                                    std::optional<std::vector<int>> colorinds,
                                    const std::shared_ptr<GRM::Context> &extContext)
{
  /*!
   * This function can be used to set a vector of MarkerColorInds of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] colorinds_key A string used as a key for storing the colorinds
   * \param[in] colorinds A vector containing the MarkerColorInds
   * \param[in] extContext A GRM::Context used for storing colorinds. By default it uses GRM::Render's GRM::Context
   * object but an external GRM::Context can be used
   */
  std::shared_ptr<GRM::Context> useContext = (extContext == nullptr) ? context : extContext;
  if (colorinds != std::nullopt)
    {
      (*useContext)[colorinds_key] = *colorinds;
    }
  element->setAttribute("markercolorinds", colorinds_key);
}

void GRM::Render::setLineType(const std::shared_ptr<Element> &element, const std::string &types_key,
                              std::optional<std::vector<int>> types, const std::shared_ptr<GRM::Context> &extContext)
{
  /*!
   * This function can be used to set a vector of LineTypes of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] types_key A string used as a key for storing the types
   * \param[in] types A vector containing the LineTypes
   * \param[in] extContext A GRM::Context used for storing types. By default it uses GRM::Render's GRM::Context object
   * but an external GRM::Context can be used
   */
  std::shared_ptr<GRM::Context> useContext = (extContext == nullptr) ? context : extContext;
  if (types != std::nullopt)
    {
      (*useContext)[types_key] = *types;
    }
  element->setAttribute("linetypes", types_key);
}

void GRM::Render::setLineType(const std::shared_ptr<Element> &element, int type)
{
  /*!
   * This function can be used to set a LineType of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] type An Integer setting the LineType
   */
  element->setAttribute("linetype", type);
}

void GRM::Render::setLineWidth(const std::shared_ptr<Element> &element, const std::string &widths_key,
                               std::optional<std::vector<double>> widths,
                               const std::shared_ptr<GRM::Context> &extContext)
{
  /*!
   * This function can be used to set a vector of LineWidths of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] widths_key A string used as a key for storing the widths
   * \param[in] widths A vector containing the LineWidths
   * \param[in] extContext A GRM::Context used for storing widths. By default it uses GRM::Render's GRM::Context object
   * but an external GRM::Context can be used
   */
  std::shared_ptr<GRM::Context> useContext = (extContext == nullptr) ? context : extContext;
  if (widths != std::nullopt)
    {
      (*useContext)[widths_key] = *widths;
    }
  element->setAttribute("linewidths", widths_key);
}

void GRM::Render::setLineWidth(const std::shared_ptr<Element> &element, double width)
{
  /*!
   * This function can be used to set a LineWidth of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] type A Double setting the LineWidth
   */
  element->setAttribute("linewidth", width);
}

void GRM::Render::setLineColorInd(const std::shared_ptr<Element> &element, const std::string &colorinds_key,
                                  std::optional<std::vector<int>> colorinds,
                                  const std::shared_ptr<GRM::Context> &extContext)
{
  /*!
   * This funciton can be used to set a vector of LineColorInds of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] colorinds_key A string used as a key for storing the colorinds
   * \param[in] colorinds A vector containing the Colorinds
   * \param[in] extContext A GRM::Context used for storing colorinds. By default it uses GRM::Render's GRM::Context
   * object but an external GRM::Context can be used
   */
  std::shared_ptr<GRM::Context> useContext = (extContext == nullptr) ? context : extContext;
  if (colorinds != std::nullopt)
    {
      (*useContext)[colorinds_key] = *colorinds;
    }
  element->setAttribute("linecolorinds", colorinds_key);
}

void GRM::Render::setLineColorInd(const std::shared_ptr<Element> &element, int color)
{
  /*!
   * This function can be used to set LineColorInd of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] color An Integer value setting the LineColorInd
   */
  element->setAttribute("linecolorind", color);
}

void GRM::Render::setTextFontPrec(const std::shared_ptr<Element> &element, int font, int prec)
{
  /*!
   * This function can be used to set TextFontPrec of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] font An Integer value representing a font type
   * \param[in] prec An Integer value representing a font precision
   */
  element->setAttribute("textfontprec", true);
  element->setAttribute("textfontprec_font", font);
  element->setAttribute("textfontprec_prec", prec);
}

void GRM::Render::setCharUp(const std::shared_ptr<Element> &element, double ux, double uy)
{
  /*!
   * This function can be used to set CharUp of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] ux  X coordinate of the text up vector
   * \param[in] uy  y coordinate of the text up vector
   */
  element->setAttribute("charup", true);
  element->setAttribute("charup_ux", ux);
  element->setAttribute("charup_uy", uy);
}

void GRM::Render::setTextAlign(const std::shared_ptr<Element> &element, int horizontal, int vertical)
{
  /*!
   * This function can be used to set TextAlign of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] horizontal  Horizontal text alignment
   * \param[in] vertical Vertical text alignment
   */
  element->setAttribute("textalign", true);
  element->setAttribute("textalign_horizontal", horizontal);
  element->setAttribute("textalign_vertical", vertical);
}

void GRM::Render::setTextWidthAndHeight(const std::shared_ptr<Element> &element, double width, double height)
{
  /*!
   * This function can be used to set the width and height of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] width Width of the Element
   * \param[in] height Height of the Element
   */
  element->setAttribute("width", width);
  element->setAttribute("height", height);
}

void GRM::Render::setLineSpec(const std::shared_ptr<Element> &element, const std::string &spec)
{
  /*!
   * This function can be used to set the linespec of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] spec An std::string
   *
   */
  element->setAttribute("linespec", spec);
}

void GRM::Render::setColorRep(const std::shared_ptr<Element> &element, int index, double red, double green, double blue)
{
  /*!
   * This function can be used to set the colorrep of a GRM::Element
   *
   * \param[in] element A GRM::Element
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

  std::string name = "colorrep$" + std::to_string(index);

  element->setAttribute(name, stream.str());
}

void GRM::Render::setFillIntStyle(const std::shared_ptr<GRM::Element> &element, int index)
{
  /*!
   * This function can be used to set the fillintstyle of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] index The style of fill to be used
   */
  element->setAttribute("fillintstyle", index);
}

void GRM::Render::setFillColorInd(const std::shared_ptr<GRM::Element> &element, int color)
{
  /*!
   * This function can be used to set the fillcolorind of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] color The fill area color index (COLOR < 1256)
   */
  element->setAttribute("fillcolorind", color);
}

void GRM::Render::setFillStyle(const std::shared_ptr<GRM::Element> &element, int index)
{
  /*!
   * This function can be used to set the fillintstyle of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] index The fill style index to be used
   */

  element->setAttribute("fillstyle", index);
}

void GRM::Render::setScale(const std::shared_ptr<GRM::Element> &element, int scale)
{
  /*!
   * This function can be used to set the scale of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] index The scale index to be used
   */
  element->setAttribute("scale", scale);
}

void GRM::Render::setWindow3d(const std::shared_ptr<GRM::Element> &element, double xmin, double xmax, double ymin,
                              double ymax, double zmin, double zmax)
{
  /*!
   * This function can be used to set the window3d of a GRM::Element
   *
   * \param[in] element A GRM::Element
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

void GRM::Render::setSpace3d(const std::shared_ptr<GRM::Element> &element, double fov, double camera_distance)
{
  /*! This function can be used to set the space3d of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] phi: azimuthal angle of the spherical coordinates
   * \param[in] theta: polar angle of the spherical coordinates
   * \param[in] fov: vertical field of view(0 or NaN for orthographic projection)
   * \param[in] camera_distance: distance between the camera and the focus point (in arbitrary units, 0 or NaN for the
   * radius of the object's smallest bounding sphere)
   */

  element->setAttribute("space3d", true);
  element->setAttribute("space3d_fov", fov);
  element->setAttribute("space3d_camera_distance", camera_distance);
}

void GRM::Render::setSpace(const std::shared_ptr<Element> &element, double zmin, double zmax, int rotation, int tilt)
{
  /*!
   * This function can be used to set the space of a GRM::Element
   *
   * \param[in] element A GRM::Element
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

void GRM::Render::setSelntran(const std::shared_ptr<Element> &element, int transform)
{
  /*! This function can be used to set the window3d of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] transform Select a predefined transformation from world coordinates to normalized device coordinates.
   */

  element->setAttribute("selntran", transform);
}

void GRM::Render::setGR3BackgroundColor(const std::shared_ptr<GRM::Element> &element, double red, double green,
                                        double blue, double alpha)
{
  /*! This function can be used to set the gr3 backgroundcolor of a GRM::Element
   *
   * \param[in] element A GRM::Element
   */
  element->setAttribute("gr3backgroundcolor", true);
  element->setAttribute("gr3backgroundcolor_red", red);
  element->setAttribute("gr3backgroundcolor_green", green);
  element->setAttribute("gr3backgroundcolor_blue", blue);
  element->setAttribute("gr3backgroundcolor_alpha", alpha);
}

void GRM::Render::setGR3CameraLookAt(const std::shared_ptr<GRM::Element> &element, double camera_x, double camera_y,
                                     double camera_z, double center_x, double center_y, double center_z, double up_x,
                                     double up_y, double up_z)
{
  /*! This function can be used to set the gr3 camerlookat of a GRM::Element
   *
   * \param[in] element A GRM::Element
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

  element->setAttribute("gr3cameralookat_camera_x", camera_x);
  element->setAttribute("gr3cameralookat_camera_y", camera_y);
  element->setAttribute("gr3cameralookat_camera_z", camera_z);
  element->setAttribute("gr3cameralookat_center_x", center_x);
  element->setAttribute("gr3cameralookat_center_y", center_y);
  element->setAttribute("gr3cameralookat_center_z", center_z);
  element->setAttribute("gr3cameralookat_up_x", up_x);
  element->setAttribute("gr3cameralookat_up_y", up_y);
  element->setAttribute("gr3cameralookat_up_z", up_z);
}

void GRM::Render::setTextColorInd(const std::shared_ptr<GRM::Element> &element, int index)
{
  /*!
   * This function can be used to set the textcolorind of a GRM::Element
   * \param[in] element A GRM::Element
   * \param[in] index The color index
   */

  element->setAttribute("textcolorind", index);
}

void GRM::Render::setBorderColorInd(const std::shared_ptr<GRM::Element> &element, int index)
{
  /*!
   * This function can be used to set the bordercolorind of a GRM::Element
   * \param[in] element A GRM::Element
   * \param[in] index The color index
   */
  element->setAttribute("bordercolorind", index);
}

void GRM::Render::selectClipXForm(const std::shared_ptr<GRM::Element> &element, int form)
{
  /*!
   * This function can be used to set the clipxform of a GRM::Element
   * \param[in] element A GRM::Element
   * \param[in] form the clipxform
   */
  element->setAttribute("clipxform", form);
}

void GRM::Render::setCharHeight(const std::shared_ptr<GRM::Element> &element, double height)
{
  /*!
   * This function can be used to set the charheight of a GRM::Element
   * \param[in] element A GRM::Element
   * \param[in] height the charheight
   */
  element->setAttribute("charheight", height);
}

void GRM::Render::setProjectionType(const std::shared_ptr<GRM::Element> &element, int type)
{
  /*!
   * This function can be used to set the projectiontype of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] type The projectiontype
   */
  element->setAttribute("projectiontype", type);
}

void GRM::Render::setTransparency(const std::shared_ptr<GRM::Element> &element, double alpha)
{
  /*!
   * This function can be used to set the transparency of a GRM::Element
   * \param[in] element A GRM::Element
   * \param[in] alpha The alpha
   */
  element->setAttribute("transparency", alpha);
}

void GRM::Render::setResampleMethod(const std::shared_ptr<GRM::Element> &element, int resample)
{
  /*!
   * This function can be used to set the resamplemethod of a GRM::Element
   * \param[in] element A GRM::Element
   * \param[in] resample The resample method
   */

  element->setAttribute("resample_method", resample);
}

void GRM::Render::setTextEncoding(const std::shared_ptr<Element> &element, int encoding)
{
  /*!
   * This function can be used to set the textencoding of a GRM::Element
   * \param[in] element A GRM::Element
   * \param[in] encoding The textencoding
   */
  element->setAttribute("textencoding", encoding);
}

void GRM::Render::setSubplot(const std::shared_ptr<GRM::Element> &element, double xmin, double xmax, double ymin,
                             double ymax)
{
  element->setAttribute("subplot", true);
  element->setAttribute("subplot_xmin", xmin);
  element->setAttribute("subplot_xmax", xmax);
  element->setAttribute("subplot_ymin", ymin);
  element->setAttribute("subplot_ymax", ymax);
}

void GRM::Render::setXTickLabels(std::shared_ptr<GRM::Element> group, const std::string &key,
                                 std::optional<std::vector<std::string>> xticklabels,
                                 const std::shared_ptr<GRM::Context> &extContext)
{
  /*!
   * This function can be used to create a XTickLabel GRM::Element
   *
   * \param[in] key A string used for storing the xticklabels in GRM::Context
   * \param[in] xticklabels A vector containing string values representing xticklabels
   * \param[in] extContext A GRM::Context that is used for storing the vectors. By default it uses GRM::Render's
   * GRM::Context object but an external GRM::Context can be used
   */

  std::shared_ptr<GRM::Context> useContext = (extContext == nullptr) ? context : extContext;
  if (xticklabels != std::nullopt)
    {
      (*useContext)[key] = *xticklabels;
    }
  group->setAttribute("xticklabels", key);
}

void GRM::Render::setNextColor(const std::shared_ptr<GRM::Element> &element, const std::string &color_indices_key,
                               const std::vector<int> &color_indices, const std::shared_ptr<GRM::Context> &extContext)
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
      throw NotFoundError("Color indices are missing in vector\n");
    }
}

void GRM::Render::setNextColor(const std::shared_ptr<GRM::Element> &element, const std::string &color_rgb_values_key,
                               const std::vector<double> &color_rgb_values,
                               const std::shared_ptr<GRM::Context> &extContext)
{
  auto useContext = (extContext == nullptr) ? context : extContext;
  element->setAttribute("setNextColor", true);
  if (!color_rgb_values.empty())
    {
      (*useContext)[color_rgb_values_key] = color_rgb_values;
      element->setAttribute("color_rgb_values", color_rgb_values_key);
    }
}

void GRM::Render::setNextColor(const std::shared_ptr<GRM::Element> &element,
                               std::optional<std::string> color_indices_key,
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

void GRM::Render::setNextColor(const std::shared_ptr<GRM::Element> &element)
{
  element->setAttribute("setNextColor", true);
  element->setAttribute("snc_fallback", true);
}

void GRM::Render::setOriginPosition(const std::shared_ptr<GRM::Element> &element, std::string x_org_pos,
                                    std::string y_org_pos)
{
  element->setAttribute("x_org_pos", x_org_pos);
  element->setAttribute("y_org_pos", y_org_pos);
}

void GRM::Render::setOriginPosition3d(const std::shared_ptr<GRM::Element> &element, std::string x_org_pos,
                                      std::string y_org_pos, std::string z_org_pos)
{
  setOriginPosition(element, x_org_pos, y_org_pos);
  element->setAttribute("z_org_pos", z_org_pos);
}

void GRM::Render::setGR3LightParameters(const std::shared_ptr<GRM::Element> &element, double ambient, double diffuse,
                                        double specular, double specular_power)
{
  element->setAttribute("ambient", ambient);
  element->setAttribute("diffuse", diffuse);
  element->setAttribute("specular", specular);
  element->setAttribute("specular_power", specular_power);
}
