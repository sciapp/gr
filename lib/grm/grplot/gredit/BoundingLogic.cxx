#ifdef _WIN32
/*
 * Headers on Windows can define `min` and `max` as macros which causes
 * problem when using `std::min` and `std::max`
 * -> Define `NOMINMAX` to prevent the definition of these macros
 */
#define NOMINMAX
#endif

#include "BoundingLogic.hxx"

#include "../Util.hxx"
#include "grm/dom_render/render_util.hxx"
#include "grm/dom_render/process_attributes.hxx"

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

  GRM::getFigureSize(&width, &height, nullptr, nullptr);
  auto max_width_height = std::max(width, height);
  dx = static_cast<double>(x) / max_width_height;
  dy = static_cast<double>(height - y) / max_width_height;

  auto subplot_element = grm_get_subplot_from_ndc_points_using_dom(1, &dx, &dy);

  if (subplot_element)
    {
      GRM::processLimits(subplot_element);
      auto central_region = subplot_element->querySelectors("central_region");
      GRM::processWindow(central_region);
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
      GRM::calculateCharHeight(central_region);
      gr_setscale(static_cast<int>(subplot_element->getAttribute("scale")));
      gr_ndctowc(&dx, &dy);

      x_range_min = static_cast<double>(x - 50) / max_width_height;
      x_range_max = static_cast<double>(x + 50) / max_width_height;
      y_range_min = static_cast<double>(height - (y + 50)) / max_width_height;
      y_range_max = static_cast<double>(height - (y - 50)) / max_width_height;

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

static bool isPartOfElement(const std::shared_ptr<GRM::Element> &element, int x, int y)
{
  if (element && element->hasAttribute("_bbox_x_min") && element->hasAttribute("_bbox_x_max") &&
      element->hasAttribute("_bbox_y_min") && element->hasAttribute("_bbox_y_max"))
    {
      auto bbox_x_min = static_cast<double>(element->getAttribute("_bbox_x_min"));
      auto bbox_x_max = static_cast<double>(element->getAttribute("_bbox_x_max"));
      auto bbox_y_min = static_cast<double>(element->getAttribute("_bbox_y_min"));
      auto bbox_y_max = static_cast<double>(element->getAttribute("_bbox_y_max"));

      if (bbox_x_min <= x && x <= bbox_x_max && bbox_y_min <= y && y <= bbox_y_max) return true;
    }
  return false;
}

std::unordered_set<unsigned int> BoundingLogic::getElementsAtPoint(int x, int y, bool grid_hidden, bool advanced_editor)
{
  GRM::getGroupMask()->toPPM("group_mask.ppm", std::nullopt, true);
  auto group_ids = GRM::getGroupMask()->getObjectsInBox(x, y);
  if (!group_ids.empty())
    {
      std::unordered_set<unsigned int> new_group_ids = group_ids;
      int coordinate_system_id = NAN;
      for (auto group_id : group_ids)
        {
          auto root = grm_get_document_root();
          if (auto elem = root->querySelectors("[_bbox_id=\"" + std::to_string((int)(group_id)) + "\"]"))
            {
              auto elem_name = elem->localName();
              if (!advanced_editor)
                {
                  if (elem_name == "polyline" || elem_name == "polymarker" || elem_name == "draw_rect" ||
                      elem_name == "polyline_3d" || elem_name == "polymarker_3d" || elem_name == "fill_rect" ||
                      elem_name == "cell_array" || elem_name == "nonuniform_cell_array" ||
                      elem_name == "polar_cell_array" || elem_name == "nonuniform_polar_cell_array" ||
                      elem_name == "draw_image" || elem_name == "draw_arc" || elem_name == "fill_arc" ||
                      elem_name == "fill_area")
                    {
                      new_group_ids.erase(group_id);
                      auto elem_parent = elem->parentElement();
                      auto elem_parent_name = elem_parent->localName();
                      new_group_ids.emplace(static_cast<int>(elem_parent->getAttribute("_bbox_id")));

                      // special case so that the series is also somehow selectable
                      if (elem_parent_name == "bar" || elem_parent_name == "polar_bar" ||
                          elem_parent_name == "pie_segment" || elem_parent_name == "integral_group" ||
                          elem_parent_name == "error_bar")
                        {
                          new_group_ids.emplace(
                              static_cast<int>(elem_parent->parentElement()->getAttribute("_bbox_id")));
                        }
                    }
                }
              else
                {
                  auto parent = elem->parentElement();
                  elem_name = parent->localName();
                  while (elem_name != "figure" && elem_name != "plot" && elem_name != "layout_grid" &&
                         elem_name != "layout_grid_element" && elem_name != "colorbar" && elem_name != "label" &&
                         elem_name != "titles_3d" && elem_name != "text" && elem_name != "central_region" &&
                         elem_name != "side_region" && elem_name != "marginal_heatmap_plot" && elem_name != "legend" &&
                         elem_name != "text_region" && elem_name != "overlay_element")
                    {
                      new_group_ids.emplace(static_cast<int>(parent->getAttribute("_bbox_id")));
                      parent = parent->parentElement();
                      elem_name = parent->localName();
                    }
                }
              if (grid_hidden)
                {
                  if (elem_name == "grid_line") // returns the coordinate_system
                    {
                      new_group_ids.erase(group_id);
                      coordinate_system_id = static_cast<int>(
                          elem->parentElement()->parentElement()->parentElement()->getAttribute("_bbox_id"));
                      new_group_ids.emplace(coordinate_system_id);
                    }
                  else if (elem_name == "tick") // returns the axis
                    {
                      new_group_ids.erase(group_id);
                      new_group_ids.emplace(
                          static_cast<int>(elem->parentElement()->parentElement()->getAttribute("_bbox_id")));
                    }
                }
            }
          else
            {
              fprintf(stderr, "Got invalid id for bbox mask -> no element found with that id\n");
            }
        }
      if (new_group_ids.size() > 1 && !std::isnan(static_cast<double>(coordinate_system_id)))
        new_group_ids.erase(coordinate_system_id);
      group_ids.clear();
      group_ids = new_group_ids;
    }
  else
    {
      // no drawn object near/at the clicked point -> empty space which could be either:
      // legend, label, overlay, colorbar, side_region, text_region, side_plot_region, plot, figure,
      // marginal_heatmap_plot, central_region, coordinate_system, layout_grid, layout_grid_element
      // use the viewport (= bbox in case of these elements) to figure out which of the elements is most likely

      auto root = grm_get_document_root();
      auto figure = root->querySelectors("figure[active=1]");

      const auto layout_grid = figure->querySelectors("layout_grid");
      auto plot = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                           : figure->querySelectors("plot");
      if (plot == nullptr)
        {
          int width, height;
          double viewport[4];
          GRM::getFigureSize(&width, &height, nullptr, nullptr);
          auto max_width_height = std::max(width, height);
          auto ndc_x = (double)x / max_width_height;
          auto ndc_y = (double)(height - y) / max_width_height;

          for (const auto &plot_elem : layout_grid->querySelectorsAll("plot"))
            {
              if (!GRM::Render::getViewport(plot_elem, &viewport[0], &viewport[1], &viewport[2], &viewport[3]))
                throw NotFoundError("Plot doesn't have a viewport but it should.\n");
              if (viewport[0] <= ndc_x && ndc_x <= viewport[1] && viewport[2] <= ndc_y && ndc_y <= viewport[3])
                {
                  plot = plot_elem;
                  break;
                }
            }
        }
      const auto central_region = plot->querySelectors("central_region");
      const auto legend = plot->querySelectors("legend");
      const auto overlay = figure->querySelectors("overlay");

      // overlay_element isn't a special case since it can only be a text or draw_image but both works better with the
      // mask
      if (isPartOfElement(legend, x, y))
        {
          bool inserted_label = false;
          for (const auto &label : legend->querySelectorsAll("label"))
            {
              if (isPartOfElement(label, x, y))
                {
                  group_ids.emplace(static_cast<int>(label->getAttribute("_bbox_id")));
                  inserted_label = true;
                }
            }
          if (!inserted_label) group_ids.emplace(static_cast<int>(legend->getAttribute("_bbox_id")));
        }
      else
        {
          if (isPartOfElement(central_region, x, y))
            {
              if (const auto coordinate_system = central_region->querySelectors("coordinate_system");
                  isPartOfElement(coordinate_system, x, y))
                {
                  int inserted_axis = 0, size = 0;
                  if (auto plot_type = static_cast<std::string>(coordinate_system->getAttribute("plot_type"));
                      plot_type == "2d")
                    {
                      for (const auto axis : coordinate_system->querySelectorsAll("axis"))
                        {
                          size++;
                          if (isPartOfElement(axis, x, y)) inserted_axis++;
                        }
                    }
                  else if (plot_type == "3d")
                    {
                      for (const auto axis : coordinate_system->querySelectorsAll("grid_3d"))
                        {
                          size++;
                          if (isPartOfElement(axis, x, y)) inserted_axis++;
                        }
                    }
                  else if (plot_type == "polar")
                    {
                      size++;
                      if (isPartOfElement(coordinate_system->querySelectors("radial_axis"), x, y)) inserted_axis++;
                    }
                  if (inserted_axis == size)
                    group_ids.emplace(static_cast<int>(coordinate_system->getAttribute("_bbox_id")));
                  else
                    group_ids.emplace(static_cast<int>(central_region->getAttribute("_bbox_id")));
                }
              else
                {
                  group_ids.emplace(static_cast<int>(central_region->getAttribute("_bbox_id")));
                }
            }
          else
            {
              for (const auto &side_region : plot->querySelectorsAll("side_region"))
                {
                  if (isPartOfElement(side_region, x, y))
                    {
                      bool inserted_colorbar = false, inserted_text_region = false, inserted_side_plot_region = false;
                      for (const auto &text_region : side_region->querySelectorsAll("text_region"))
                        {
                          if (isPartOfElement(text_region, x, y))
                            {
                              group_ids.emplace(static_cast<int>(text_region->getAttribute("_bbox_id")));
                              inserted_text_region = true;
                            }
                        }
                      for (const auto &side_plot_region : plot->querySelectorsAll("side_plot_region"))
                        {
                          if (isPartOfElement(side_plot_region, x, y))
                            {
                              if (const auto colorbar = side_plot_region->querySelectors("colorbar");
                                  isPartOfElement(colorbar, x, y))
                                {
                                  group_ids.emplace(static_cast<int>(colorbar->getAttribute("_bbox_id")));
                                  inserted_colorbar = true;
                                }
                              else
                                {
                                  group_ids.emplace(static_cast<int>(side_plot_region->getAttribute("_bbox_id")));
                                  inserted_side_plot_region = true;
                                }
                            }
                        }
                      if (!inserted_text_region && !inserted_side_plot_region)
                        group_ids.emplace(static_cast<int>(side_region->getAttribute("_bbox_id")));
                      if (!inserted_colorbar)
                        {
                          group_ids.emplace(static_cast<int>(plot->getAttribute("_bbox_id")));
                          group_ids.emplace(static_cast<int>(figure->getAttribute("_bbox_id")));
                          if (const auto marginal_heatmap_plot = plot->querySelectors("marginal_heatmap_plot"))
                            group_ids.emplace(static_cast<int>(marginal_heatmap_plot->getAttribute("_bbox_id")));
                          if (overlay != nullptr)
                            group_ids.emplace(static_cast<int>(overlay->getAttribute("_bbox_id")));

                          if (layout_grid != nullptr)
                            {
                              bool inserted_grid_layout_element = false;
                              for (const auto &layout_grid_elem : layout_grid->querySelectorsAll("layout_grid_element"))
                                {
                                  if (isPartOfElement(layout_grid_elem, x, y))
                                    {
                                      group_ids.emplace(static_cast<int>(layout_grid_elem->getAttribute("_bbox_id")));
                                      inserted_grid_layout_element = true;
                                    }
                                }
                              if (!inserted_grid_layout_element)
                                group_ids.emplace(static_cast<int>(layout_grid->getAttribute("_bbox_id")));
                            }
                        }
                    }
                  else
                    {
                      group_ids.emplace(static_cast<int>(plot->getAttribute("_bbox_id")));
                      group_ids.emplace(static_cast<int>(figure->getAttribute("_bbox_id")));
                      if (const auto marginal_heatmap_plot = plot->querySelectors("marginal_heatmap_plot"))
                        group_ids.emplace(static_cast<int>(marginal_heatmap_plot->getAttribute("_bbox_id")));
                      if (overlay != nullptr) group_ids.emplace(static_cast<int>(overlay->getAttribute("_bbox_id")));

                      if (layout_grid != nullptr)
                        {
                          bool inserted_grid_layout_element = false;
                          for (const auto &layout_grid_elem : layout_grid->querySelectorsAll("layout_grid_element"))
                            {
                              if (isPartOfElement(layout_grid_elem, x, y))
                                {
                                  group_ids.emplace(static_cast<int>(layout_grid_elem->getAttribute("_bbox_id")));
                                  inserted_grid_layout_element = true;
                                }
                            }
                          if (!inserted_grid_layout_element)
                            group_ids.emplace(static_cast<int>(layout_grid->getAttribute("_bbox_id")));
                        }
                    }
                }
            }
        }
    }

  return group_ids;
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
