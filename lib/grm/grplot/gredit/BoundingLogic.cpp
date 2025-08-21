#ifdef _WIN32
/*
 * Headers on Windows can define `min` and `max` as macros which causes
 * problem when using `std::min` and `std::max`
 * -> Define `NOMINMAX` to prevent the definition of these macros
 */
#define NOMINMAX
#endif

#include "BoundingLogic.hxx"

#include "../util.hxx"

#include <algorithm>
#include <vector>
#include <cmath>
#include <cfloat>
#include <grm.h>

bool boundingObjectCompareFunction(const BoundingObject &i, const BoundingObject &j)
{
  int i_index = 1, j_index = 1;
  if (i.getRef()->hasAttribute("z_index") && static_cast<int>(i.getRef()->getAttribute("z_index")) < 0)
    i_index = abs(static_cast<int>(i.getRef()->getAttribute("z_index")));
  if (j.getRef()->hasAttribute("z_index") && static_cast<int>(j.getRef()->getAttribute("z_index")) < 0)
    j_index = abs(static_cast<int>(j.getRef()->getAttribute("z_index")));

  if (abs(i.boundingRect().width() * i.boundingRect().height() - j.boundingRect().width() * j.boundingRect().height()) <
      1e-8)
    {
      double camx, camy;
      double i_xmin, i_xmax, i_ymin, i_ymax, i_dist;
      double j_xmin, j_xmax, j_ymin, j_ymax, j_dist;

      i.getCam(&camx, &camy);
      i.getCorner(&i_xmin, &i_xmax, &i_ymin, &i_ymax);
      j.getCorner(&j_xmin, &j_xmax, &j_ymin, &j_ymax);

      i_dist = sqrt(pow(((i_xmax + i_xmin) / 2 - camx), 2) + pow(((i_ymax + i_ymin) / 2 - camy), 2));
      j_dist = sqrt(pow(((j_xmax + j_xmin) / 2 - camx), 2) + pow(((j_ymax + j_ymin) / 2 - camy), 2));
      return i_dist * i_index < j_dist * j_index;
    }
  return i.boundingRect().width() * i.boundingRect().height() * i_index <
         j.boundingRect().width() * j.boundingRect().height() * j_index;
}

std::vector<BoundingObject> BoundingLogic::getBoundingObjectsAtPoint(int x, int y, bool grid_hidden,
                                                                     bool advanced_editor)
{
  std::vector<BoundingObject> ret;
  double x_px, y_px, x_range_min, x_range_max, y_range_min, y_range_max, dx, dy;
  double x_min, x_max, y_min, y_max, mindiff = DBL_MAX, diff;
  int width = 0, height = 0;
  std::shared_ptr<GRM::Context> context = grm_get_render()->getContext();

  GRM::Render::getFigureSize(&width, &height, nullptr, nullptr);
  auto max_width_height = std::max(width, height);
  dx = (double)x / max_width_height;
  dy = (double)(height - y) / max_width_height;

  auto subplot_element = grm_get_subplot_from_ndc_points_using_dom(1, &dx, &dy);

  if (subplot_element)
    {
      GRM::Render::processLimits(subplot_element);
      auto central_region = subplot_element->querySelectors("central_region");
      GRM::Render::processWindow(central_region);
      if (central_region->hasAttribute("viewport_x_min"))
        {
          double viewport[4];
          if (!GRM::Render::getViewport(central_region, &viewport[0], &viewport[1], &viewport[2], &viewport[3]))
            throw NotFoundError("Central region doesn't have a viewport but it should.\n");
          gr_setviewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        }
      if (central_region->hasAttribute("window_x_min"))
        {
          gr_setwindow(static_cast<double>(central_region->getAttribute("window_x_min")),
                       static_cast<double>(central_region->getAttribute("window_x_max")),
                       static_cast<double>(central_region->getAttribute("window_y_min")),
                       static_cast<double>(central_region->getAttribute("window_y_max")));
        }
      GRM::Render::calculateCharHeight(central_region);
      gr_setscale(static_cast<int>(subplot_element->getAttribute("scale")));
      gr_ndctowc(&dx, &dy);

      x_range_min = (double)(x - 50) / max_width_height;
      x_range_max = (double)(x + 50) / max_width_height;
      y_range_min = (double)(height - (y + 50)) / max_width_height;
      y_range_max = (double)(height - (y - 50)) / max_width_height;

      gr_ndctowc(&x_range_min, &y_range_min);
      gr_ndctowc(&x_range_max, &y_range_max);

      x_min = static_cast<double>(subplot_element->getAttribute("_x_lim_min"));
      x_max = static_cast<double>(subplot_element->getAttribute("_x_lim_max"));
      y_min = static_cast<double>(subplot_element->getAttribute("_y_lim_min"));
      y_max = static_cast<double>(subplot_element->getAttribute("_y_lim_max"));

      x_range_min = (x_min > x_range_min) ? x_min : x_range_min;
      y_range_min = (y_min > y_range_min) ? y_min : y_range_min;
      x_range_max = (x_max < x_range_max) ? x_max : x_range_max;
      y_range_max = (y_max < y_range_max) ? y_max : y_range_max;
    }

  for (auto &bounding_object : bounding_objects)
    {
      auto elem_name = bounding_object.getRef()->localName();
      if (grid_hidden && (elem_name == "grid_line" || elem_name == "tick" || elem_name == "tick_group")) continue;
      if (!advanced_editor &&
          ((elem_name == "polyline" || elem_name == "polymarker" || elem_name == "draw_rect" ||
            elem_name == "polyline_3d" || elem_name == "polymarker_3d" || elem_name == "fill_rect" ||
            elem_name == "cell_array" || elem_name == "nonuniform_cell_array" || elem_name == "polar_cell_array" ||
            elem_name == "nonuniform_polar_cell_array" || elem_name == "draw_image" || elem_name == "draw_arc" ||
            elem_name == "fill_arc" || elem_name == "fill_area" || elem_name == "overlay_element")))
        continue;

      if (subplot_element && (bounding_object.getRef()->localName() == "series_line" ||
                              bounding_object.getRef()->localName() == "series_scatter"))
        {
          if (util::startsWith(elem_name, "series_") &&
              !subplot_element->querySelectors(
                  elem_name + "[_bbox_id=\"" +
                  std::to_string(static_cast<int>(bounding_object.getRef()->getAttribute("_bbox_id"))) +
                  ""
                  "\"]"))
            continue;
          auto x_key = static_cast<std::string>(bounding_object.getRef()->getAttribute("x"));
          auto y_key = static_cast<std::string>(bounding_object.getRef()->getAttribute("y"));
          auto x_series_vec = GRM::get<std::vector<double>>((*context)[x_key]);
          auto y_series_vec = GRM::get<std::vector<double>>((*context)[y_key]);

          for (int i = 0; i < x_series_vec.size(); i++)
            {
              x_px = x_series_vec[i];
              if (bounding_object.getRef()->hasAttribute("ref_x_axis_location") &&
                  static_cast<std::string>(bounding_object.getRef()->getAttribute("ref_x_axis_location")) != "x")
                {
                  auto location =
                      static_cast<std::string>(bounding_object.getRef()->getAttribute("ref_x_axis_location"));
                  auto a = static_cast<double>(subplot_element->getAttribute("_" + location + "_window_xform_a"));
                  auto b = static_cast<double>(subplot_element->getAttribute("_" + location + "_window_xform_b"));

                  x_px = (x_px - b) / a;
                }
              if (x_px < x_range_min || x_px > x_range_max) continue;

              y_px = y_series_vec[i];
              if (bounding_object.getRef()->hasAttribute("ref_y_axis_location") &&
                  static_cast<std::string>(bounding_object.getRef()->getAttribute("ref_y_axis_location")) != "y")
                {
                  auto location =
                      static_cast<std::string>(bounding_object.getRef()->getAttribute("ref_y_axis_location"));
                  auto a = static_cast<double>(subplot_element->getAttribute("_" + location + "_window_xform_a"));
                  auto b = static_cast<double>(subplot_element->getAttribute("_" + location + "_window_xform_b"));

                  y_px = (y_px - b) / a;
                }

              if (y_px < y_range_min || y_px > y_range_max) continue;

              gr_wctondc(&x_px, &y_px);
              x_px = x_px * max_width_height;
              y_px = y_px * max_width_height;
              diff = sqrt(pow(x_px - x, 2) + pow((height - y_px) - y, 2));
              if (diff < mindiff && diff <= 50)
                {
                  mindiff = diff;
                  ret.clear();
                  ret.push_back(bounding_object);
                }
            }
        }
    }
  if (!ret.empty()) return ret;

  for (auto &bounding_object : bounding_objects)
    {
      auto elem_name = bounding_object.getRef()->localName();
      if (grid_hidden && (elem_name == "grid_line" || elem_name == "tick" || elem_name == "tick_group")) continue;
      if (!advanced_editor &&
          ((elem_name == "polyline" || elem_name == "polymarker" || elem_name == "draw_rect" ||
            elem_name == "polyline_3d" || elem_name == "polymarker_3d" || elem_name == "fill_rect" ||
            elem_name == "cell_array" || elem_name == "nonuniform_cell_array" || elem_name == "polar_cell_array" ||
            elem_name == "nonuniform_polar_cell_array" || elem_name == "draw_image" || elem_name == "draw_arc" ||
            elem_name == "fill_arc" || elem_name == "fill_area")))
        continue;

      if (bounding_object.containsPoint(x, y))
        {
          bounding_object.setCam(x, y);
          ret.push_back(bounding_object);
        }
    }
  std::sort(ret.begin(), ret.end(), boundingObjectCompareFunction);
  return ret;
}


void BoundingLogic::addBoundingObject(int id, double xmin, double xmax, double ymin, double ymax,
                                      std::shared_ptr<GRM::Element> ref)
{
  bounding_objects.emplace_back(BoundingObject(id, xmin, xmax, ymin, ymax, std::move(ref)));
}

void BoundingLogic::addBoundingObject(const BoundingObject &obj)
{
  bounding_objects.emplace_back(obj);
}

BoundingLogic::BoundingLogic()
{
  bounding_objects = std::vector<BoundingObject>();
}

void BoundingLogic::clear()
{
  bounding_objects.clear();
}
