#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <cfloat>

#include <grm/dom_render/invalid_value_error.hxx>
#include <grm/dom_render/render_util.hxx>
#include <grm/dom_render/render.hxx>
#include <grm/dom_render/graphics_tree/util.hxx>
#include <grm/plot_int.h>
#include <grm/dom_render/process_attributes.hxx>
#include <grm/dom_render/casts.hxx>


extern "C" {
StringMap *grm_fmt_map = stringMapNewWithData(std::size(kind_to_fmt), kind_to_fmt);
}

static bool bounding_boxes = !getenv("GRDISPLAY") || (getenv("GRDISPLAY") && strcmp(getenv("GRDISPLAY"), "view") != 0);
static std::map<int, std::map<double, std::map<std::string, GRM::Value>>> tick_modification_map;

void getPlotParent(std::shared_ptr<GRM::Element> &element)
{
  auto plot_parent = element;
  if (strEqualsAny(plot_parent->localName(), "root", "figure", "layout_grid", "layout_grid_element", "draw_graphics",
                   "overlay", "overlay_element"))
    {
      element = nullptr;
      return;
    }
  while (plot_parent->localName() != "plot")
    {
      if (plot_parent->parentElement() == nullptr) break;
      plot_parent = plot_parent->parentElement();
    }
  element = plot_parent;
}

bool isUniformData(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  std::string x_key, y_key, kind;
  kind = static_cast<std::string>(element->getAttribute("kind"));
  if (strEqualsAny(kind, "line", "scatter", "pie", "polar_line", "polar_histogram", "polar_heatmap", "polar_scatter",
                   "imshow", "histogram", "barplot", "stem", "stairs") ||
      kinds_3d.find(kind) != kinds_3d.end())
    return false;

  if (kind == "heatmap" && (!element->hasAttribute("x") || !element->hasAttribute("y")))
    {
      auto z_key = static_cast<std::string>(element->getAttribute("z"));
      auto z_vec = GRM::get<std::vector<double>>((*context)[z_key]);
      if (!element->hasAttribute("x") && !element->hasAttribute("y"))
        {
          auto z_dims_key = static_cast<std::string>(element->getAttribute("z_dims"));
          auto z_dims_vec = GRM::get<std::vector<int>>((*context)[z_dims_key]);
          return z_dims_vec[0] == z_dims_vec[1];
        }
      else if (!element->hasAttribute("x"))
        {
          y_key = static_cast<std::string>(element->getAttribute("y"));
          auto y_vec = GRM::get<std::vector<double>>((*context)[y_key]);
          return z_vec.size() / y_vec.size() == y_vec.size();
        }
      else if (!element->hasAttribute("y"))
        {
          x_key = static_cast<std::string>(element->getAttribute("x"));
          auto x_vec = GRM::get<std::vector<double>>((*context)[x_key]);
          return z_vec.size() / x_vec.size() == x_vec.size();
        }
    }
  x_key = static_cast<std::string>(element->getAttribute("x"));
  y_key = static_cast<std::string>(element->getAttribute("y"));

  auto x_vec = GRM::get<std::vector<double>>((*context)[x_key]);
  auto y_vec = GRM::get<std::vector<double>>((*context)[y_key]);
  return x_vec.size() == y_vec.size();
}

double getMaxViewport(const std::shared_ptr<GRM::Element> &element, bool x)
{
  double max_vp;
  int pixel_width, pixel_height;
  double metric_width, metric_height;
  auto plot_element = element;
  if (strEqualsAny(element->localName(), "figure", "layout_grid"))
    plot_element = element;
  else if (element->localName() == "layout_grid_element")
    element->querySelectors("plot");
  else
    getPlotParent(plot_element);

  GRM::getFigureSize(&pixel_width, &pixel_height, &metric_width, &metric_height);
  auto aspect_ratio_ws = metric_width / metric_height;
  if (plot_element != nullptr && plot_element->parentElement() != nullptr &&
      plot_element->parentElement()->localName() == "layout_grid_element" &&
      !strEqualsAny(element->localName(), "figure", "layout_grid") && !element->hasAttribute("y_max_shift_ndc") &&
      !element->hasAttribute("y_min_shift_ndc"))
    {
      double figure_viewport[4];
      auto figure_vp_element = plot_element->parentElement();
      figure_viewport[0] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_x_min_org"));
      figure_viewport[1] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_x_max_org"));
      figure_viewport[2] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_y_min_org"));
      figure_viewport[3] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_y_max_org"));

      metric_width *= (figure_viewport[1] - figure_viewport[0]);
      metric_height *= (figure_viewport[3] - figure_viewport[2]);
      aspect_ratio_ws = metric_width / metric_height;
    }

  if (plot_element == nullptr && !strEqualsAny(element->localName(), "figure", "layout_grid")) return 1;
  if (x)
    max_vp = (aspect_ratio_ws < 1) ? aspect_ratio_ws : 1.0;
  else
    max_vp = (aspect_ratio_ws > 1) ? 1.0 / aspect_ratio_ws : 1.0;
  return max_vp;
}

double getMinViewport(const std::shared_ptr<GRM::Element> &element, bool x)
{
  return 0.0;
}

void getMajorCount(const std::shared_ptr<GRM::Element> &element, const std::string &kind, int &major_count)
{
  if (element->hasAttribute("major"))
    {
      major_count = static_cast<int>(element->getAttribute("major"));
    }
  else
    {
      if (strEqualsAny(kind, "wireframe", "surface", "line3", "scatter3", "polar_line", "trisurface", "polar_heatmap",
                       "nonuniform_polar_heatmap", "polar_scatter", "volume"))
        {
          major_count = 2;
        }
      else
        {
          major_count = 5;
        }
    }
}

bool applyBoundingBoxId(GRM::Element &new_element, GRM::Element &old_element, bool only_reserve_id)
{
  if (old_element.hasAttribute("_bbox_id"))
    {
      new_element.setAttribute("_bbox_id", std::abs(static_cast<int>(old_element.getAttribute("_bbox_id"))) *
                                               (only_reserve_id ? -1 : 1));
      old_element.removeAttribute("_bbox_id");
      return true;
    }
  else if (bounding_boxes)
    {
      new_element.setAttribute("_bbox_id", idPool().next() * (only_reserve_id ? -1 : 1));
    }

  return false;
}

IdPool<int> &idPool()
{
  /*
   * Use a static pointer to heap memory instead of a static object (`static IdPool<int> id_pool`) to guarantee that
   * - the object is constructed on first use
   * - the object will remain alive as long as the program runs
   * The second point is most important since various other global `Element` objects will call `IdPool::release` in
   * their destructors, so it must be ensured that id_pool is alive until the last `Element` object is destructed.
   */
  static auto id_pool = new IdPool<int>;
  return *id_pool;
}

void setRanges(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Element> &new_series)
{
  if (element->hasAttribute("x_range_min"))
    new_series->setAttribute("x_range_min", static_cast<double>(element->getAttribute("x_range_min")));
  if (element->hasAttribute("x_range_max"))
    new_series->setAttribute("x_range_max", static_cast<double>(element->getAttribute("x_range_max")));
  if (element->hasAttribute("y_range_min"))
    new_series->setAttribute("y_range_min", static_cast<double>(element->getAttribute("y_range_min")));
  if (element->hasAttribute("y_range_max"))
    new_series->setAttribute("y_range_max", static_cast<double>(element->getAttribute("y_range_max")));
  if (element->hasAttribute("z_range_min"))
    new_series->setAttribute("z_range_min", static_cast<double>(element->getAttribute("z_range_min")));
  if (element->hasAttribute("z_range_max"))
    new_series->setAttribute("z_range_max", static_cast<double>(element->getAttribute("z_range_max")));
}

void clearAxisAttributes(const std::shared_ptr<GRM::Element> &axis)
{
  if (axis->hasAttribute("min_value")) axis->removeAttribute("min_value");
  if (axis->hasAttribute("max_value")) axis->removeAttribute("max_value");
  if (axis->hasAttribute("origin")) axis->removeAttribute("origin");
  if (axis->hasAttribute("pos")) axis->removeAttribute("pos");
  if (axis->hasAttribute("tick")) axis->removeAttribute("tick");
  if (axis->hasAttribute("major_count")) axis->removeAttribute("major_count");
  if (axis->hasAttribute("tick_size")) axis->removeAttribute("tick_size");
  if (axis->hasAttribute("_tick_size_org")) axis->removeAttribute("_tick_size_org");
  if (axis->hasAttribute("tick_orientation")) axis->removeAttribute("tick_orientation");
}

std::tuple<double, int> getColorbarAttributes(const std::string &kind, const std::shared_ptr<GRM::Element> &plot)
{
  double offset = 0.0;
  int colors = 256;

  if (kind == "contour")
    {
      if (auto series = plot->querySelectors("series_contour"); series && series->hasAttribute("levels"))
        colors = static_cast<int>(series->getAttribute("levels"));
      else
        colors = PLOT_DEFAULT_CONTOUR_LEVELS;
    }
  if (kind == "contourf")
    {
      if (auto series = plot->querySelectors("series_contourf"); series && series->hasAttribute("levels"))
        colors = static_cast<int>(series->getAttribute("levels"));
      else
        colors = PLOT_DEFAULT_CONTOUR_LEVELS;
    }
  if (kind == "polar_heatmap" || kind == "nonuniform_polar_heatmap") offset = PLOT_POLAR_COLORBAR_OFFSET;
  if (kind == "surface" || kind == "volume" || kind == "trisurface") offset = PLOT_3D_COLORBAR_OFFSET;
  return {offset, colors};
}

double getLightness(int color)
{
  unsigned char rgb[sizeof(int)];

  gr_inqcolor(color, reinterpret_cast<int *>(rgb));
  double y = (0.2126729 * rgb[0] / 255 + 0.7151522 * rgb[1] / 255 + 0.0721750 * rgb[2] / 255);
  return 116 * pow(y / 100, 1.0 / 3) - 16;
}

void resetOldBoundingBoxes(const std::shared_ptr<GRM::Element> &element)
{
  if (!bounding_boxes) return;

  if (element->hasAttribute("_bbox_id"))
    {
      element->setAttribute("_bbox_id", -std::abs(static_cast<int>(element->getAttribute("_bbox_id"))));
    }
  else
    {
      element->setAttribute("_bbox_id", -idPool().next());
    }
  element->removeAttribute("_bbox_x_min");
  element->removeAttribute("_bbox_x_max");
  element->removeAttribute("_bbox_y_min");
  element->removeAttribute("_bbox_y_max");
}

bool removeBoundingBoxId(GRM::Element &element)
{
  if (element.hasAttribute("_bbox_id"))
    {
      auto bbox_id = std::abs(static_cast<int>(element.getAttribute("_bbox_id")));
      element.removeAttribute("_bbox_id");
      idPool().release(bbox_id);
      return true;
    }
  return false;
}

// transform single coordinate (like x or y) into range (and or log scale)
// first transform into range and then log scale
double transformCoordinate(double value, double v_min, double v_max, double range_min, double range_max, bool log_scale)
{
  if (log_scale)
    {
      if (range_min != 0.0 || range_max != 0.0)
        {
          value = (range_max - range_min) * (value - v_min) / (v_max - v_min) + range_min;
        }
      else
        {
          range_min = v_min;
          range_max = v_max;
        }
      return (log10(value) - range_min) * range_max / (range_max - range_min);
    }
  return (range_max - range_min) * (value - v_min) / (v_max - v_min) + range_min;
}

void clearOldChildren(DelValues *del, const std::shared_ptr<GRM::Element> &element)
{
  /* clear all old children of an element when del is 2 or 3, in the special case where no children exist del gets
   * manipulated so that new children will be created in caller functions*/
  if (*del != DelValues::UPDATE_WITHOUT_DEFAULT && *del != DelValues::UPDATE_WITH_DEFAULT)
    {
      for (const auto &child : element->children())
        {
          if (*del == DelValues::RECREATE_OWN_CHILDREN)
            {
              if (child->hasAttribute("_child_id"))
                {
                  child->remove();
                }
              else if (element->localName() == "marginal_heatmap_plot")
                {
                  for (const auto &real_child : child->children())
                    {
                      if (real_child->hasAttribute("_child_id")) real_child->remove();
                      if (real_child->localName() == "side_plot_region")
                        {
                          for (const auto &side_plot_child : real_child->children())
                            {
                              if (side_plot_child->hasAttribute("_child_id")) side_plot_child->remove();
                            }
                        }
                    }
                }
            }
          else if (*del == DelValues::RECREATE_ALL_CHILDREN)
            {
              if (!(element->localName() == "marginal_heatmap_plot" &&
                    (child->localName() != "central_region" || child->localName() != "side_region")))
                child->remove();
            }
        }
      element->setAttribute("_delete_children", 0);
    }
  else if (!element->hasChildNodes())
    *del = DelValues::RECREATE_OWN_CHILDREN;
  else
    {
      bool only_children_created_from_attributes = true;
      bool only_error_child = true;
      bool only_non_marginal_heatmap_children = true;
      bool only_side_plot_region_child = true;
      /* types of children the coordinate system can have that are created from attributes */
      std::vector<std::string> coordinate_system_children = {"titles_3d"};
      for (const auto &child : element->children())
        {
          if (element->localName() == "coordinate_system" &&
              std::find(coordinate_system_children.begin(), coordinate_system_children.end(), child->localName()) ==
                  coordinate_system_children.end())
            {
              only_children_created_from_attributes = false;
              break;
            }
          if (element->localName() == "marginal_heatmap_plot")
            {
              if (child->localName() == "side_region" && child->hasAttribute("marginal_heatmap_side_plot"))
                {
                  for (const auto &side_region_child : child->children())
                    {
                      if (side_region_child->localName() != "text_region")
                        {
                          only_non_marginal_heatmap_children = false;
                          break;
                        }
                    }
                }
              else if (child->localName() == "central_region")
                {
                  for (const auto &central_region_child : child->children())
                    {
                      if (central_region_child->localName() == "series_heatmap")
                        only_non_marginal_heatmap_children = false;
                    }
                }
            }
          if (element->localName() == "side_region")
            {
              for (const auto &side_region_child : element->children())
                {
                  if (side_region_child->localName() == "text_region") only_side_plot_region_child = false;
                }
            }
          if (child->localName() != "error_bars" && child->localName() != "integral_group" &&
              element->localName() != "coordinate_system" && element->localName() != "marginal_heatmap_plot")
            {
              only_error_child = false;
              break;
            }
        }
      if (element->localName() == "coordinate_system" && only_children_created_from_attributes)
        *del = DelValues::RECREATE_OWN_CHILDREN;
      if (startsWith(element->localName(), "series_") && only_error_child) *del = DelValues::RECREATE_OWN_CHILDREN;
      if (element->localName() == "marginal_heatmap_plot" && only_non_marginal_heatmap_children)
        *del = DelValues::RECREATE_OWN_CHILDREN;
      if (element->localName() == "side_region" && only_side_plot_region_child) *del = DelValues::RECREATE_OWN_CHILDREN;
    }
}

void transformCoordinatesVector(std::vector<double> &coords, double v_min, double v_max, double range_min,
                                double range_max, bool log_scale)
{
  // TODO: does this method actually work correctly?
  for (auto &coord : coords)
    {
      coord = transformCoordinate(coord, v_min, v_max, range_min, range_max, log_scale);
    }
}

void legendSize(const std::shared_ptr<GRM::Element> &element, double *w, double *h)
{
  double tbx[4], tby[4];
  auto plot_parent = element;
  getPlotParent(plot_parent);
  auto kind = static_cast<std::string>(plot_parent->getAttribute("_kind"));

  *w = 0;
  *h = 0;

  if (kind != "pie")
    {
      int legend_elems = 0;
      auto central_region = plot_parent->querySelectors("central_region");
      for (auto series : central_region->children())
        {
          bool has_valid_child = false;
          if (!strEqualsAny(series->localName(), "series_line", "series_polar_line", "series_polar_scatter",
                            "series_scatter", "series_stairs", "series_stem", "series_line3", "series_scatter3"))
            continue;
          if (!series->hasAttribute("label")) continue;

          for (const auto &child : series->children())
            {
              if (child->localName() != "polyline" && child->localName() != "polymarker" &&
                  child->localName() != "polyline_3d" && child->localName() != "polymarker_3d")
                continue;
              has_valid_child = true;
              break;
            }

          if (has_valid_child)
            {
              auto current_label = static_cast<std::string>(series->getAttribute("label"));
              gr_inqtext(0, 0, current_label.data(), tbx, tby);
              *w = grm_max(*w, tbx[2] - tbx[0]);
              *h += grm_max(tby[2] - tby[0], 0.03);
            }
          legend_elems += 1;
        }

      // if a series gets removed _start_w and _start_h must be reseted so the legend box adjust to its new shrinked
      // size
      if (element->hasAttribute("_legend_elems"))
        {
          if (static_cast<int>(element->getAttribute("_legend_elems")) != legend_elems)
            {
              if (element->hasAttribute("_start_w")) element->removeAttribute("_start_w");
              if (element->hasAttribute("_start_h")) element->removeAttribute("_start_h");
            }
        }
    }
  else
    {
      std::vector<std::string> labels;
      int num_labels;

      if (auto render = std::dynamic_pointer_cast<GRM::Render>(element->ownerDocument()))
        {
          auto context = render->getContext();
          auto key = static_cast<std::string>(plot_parent->querySelectors("series_pie")->getAttribute("labels"));
          labels = GRM::get<std::vector<std::string>>((*context)[key]);
          num_labels = labels.size();
        }

      if (!labels.empty())
        {
          for (auto current_label : labels)
            {
              gr_inqtext(0, 0, current_label.data(), tbx, tby);
              *w += tbx[2] - tbx[0];
              *h = grm_max(*h, tby[2] - tby[0]);
            }
          *w += num_labels * 0.03 + (num_labels - 1) * 0.02;
        }

      // if a series gets removed _start_w and _start_h must be reseted so the legend box adjust to its new shrinked
      // size
      if (element->hasAttribute("_legend_elems"))
        {
          if (static_cast<int>(element->getAttribute("_legend_elems")) != num_labels)
            {
              if (element->hasAttribute("_start_w")) element->removeAttribute("_start_w");
              if (element->hasAttribute("_start_h")) element->removeAttribute("_start_h");
            }
        }
    }
}

void sidePlotMargin(const std::shared_ptr<GRM::Element> &side_region, double *margin, double inc)
{
  if (side_region->querySelectors("side_plot_region") ||
      (side_region->hasAttribute("marginal_heatmap_side_plot") &&
       static_cast<int>(side_region->getAttribute("marginal_heatmap_side_plot"))))
    {
      *margin += inc;
    }
}

void capSidePlotMarginInNonKeepAspectRatio(const std::shared_ptr<GRM::Element> &side_region, double *margin,
                                           const std::string &kind)
{
  // TODO: Overwork max value workaround
  if (side_region->querySelectors("side_plot_region"))
    {
      if (strEqualsAny(kind, "surface", "volume", "trisurface"))
        {
          *margin = grm_max(0.125, *margin);
        }
      else
        {
          *margin = grm_max(0.075, *margin);
        }
    }
}

void bboxViewportAdjustmentsForSideRegions(const std::shared_ptr<GRM::Element> &element, std::string location)
{
  double plot_vp[4];
  auto plot_parent = element;
  getPlotParent(plot_parent);
  if (!GRM::Render::getViewport(plot_parent, &plot_vp[0], &plot_vp[1], &plot_vp[2], &plot_vp[3]))
    throw NotFoundError(plot_parent->localName() + " doesn't have a viewport but it should.\n");

  if (location == "right")
    {
      auto vp_x_max = static_cast<double>(element->getAttribute("viewport_x_max"));

      vp_x_max += 0.075 * (plot_vp[1] - plot_vp[0]);
      element->setAttribute("_viewport_offset",
                            0.075 * (plot_vp[1] - plot_vp[0]) - (plot_vp[1] < vp_x_max ? (vp_x_max - plot_vp[1]) : 0));
      vp_x_max = grm_min(plot_vp[1], vp_x_max);

      element->setAttribute("viewport_x_max", vp_x_max);
      element->setAttribute("_viewport_x_max_org", vp_x_max);
    }
  else if (location == "left")
    {
      auto vp_x_min = static_cast<double>(element->getAttribute("viewport_x_min"));

      vp_x_min -= 0.075 * (plot_vp[1] - plot_vp[0]);
      element->setAttribute("_viewport_offset",
                            0.075 * (plot_vp[1] - plot_vp[0]) - (plot_vp[0] > vp_x_min ? (plot_vp[0] - vp_x_min) : 0));
      vp_x_min = grm_max(plot_vp[0], vp_x_min);

      element->setAttribute("viewport_x_min", vp_x_min);
      element->setAttribute("_viewport_x_min_org", vp_x_min);
    }
  else if (location == "top")
    {
      bool has_title = false;

      if (element->localName() == "side_region" && element->hasAttribute("text_content") &&
          element->hasAttribute("text_is_title"))
        {
          has_title = static_cast<int>(element->getAttribute("text_is_title"));
        }

      if (!has_title)
        {
          auto vp_y_max = static_cast<double>(element->getAttribute("viewport_y_max"));

          vp_y_max += 0.075 * (plot_vp[3] - plot_vp[2]);

          element->setAttribute("_viewport_offset", 0.075 * (plot_vp[3] - plot_vp[2]) -
                                                        (plot_vp[3] < vp_y_max ? (vp_y_max - plot_vp[3]) : 0));
          vp_y_max = grm_min(plot_vp[3], vp_y_max);

          element->setAttribute("viewport_y_max", vp_y_max);
          element->setAttribute("_viewport_y_max_org", vp_y_max);
        }
    }
  else if (location == "bottom")
    {
      auto vp_y_min = static_cast<double>(element->getAttribute("viewport_y_min"));

      vp_y_min -= 0.075 * (plot_vp[3] - plot_vp[2]);
      element->setAttribute("_viewport_offset",
                            0.075 * (plot_vp[3] - plot_vp[2]) - (plot_vp[2] > vp_y_min ? (plot_vp[2] - vp_y_min) : 0));
      vp_y_min = grm_max(plot_vp[2], vp_y_min);

      element->setAttribute("viewport_y_min", vp_y_min);
      element->setAttribute("_viewport_y_min_org", vp_y_min);
    }
}

std::string getLocalName(const std::shared_ptr<GRM::Element> &element)
{
  std::string local_name = element->localName();
  if (startsWith(element->localName(), "series")) local_name = "series";
  return local_name;
}

bool isDrawable(const std::shared_ptr<GRM::Element> &element)
{
  auto local_name = getLocalName(element);
  if (drawable_types.find(local_name) != drawable_types.end()) return true;
  if (local_name == "series")
    {
      auto kind = static_cast<std::string>(element->getAttribute("kind"));
      if (drawable_kinds.find(kind) != drawable_kinds.end()) return true;
    }
  return false;
}

/*!
 * Convert an RGB triple to a luminance value following the CCIR 601 format.
 *
 * \param[in] r The red component of the RGB triple in the range [0.0, 1.0].
 * \param[in] g The green component of the RGB triple in the range [0.0, 1.0].
 * \param[in] b The blue component of the RGB triple in the range [0.0, 1.0].
 * \return The luminance of the given RGB triple in the range [0.0, 1.0].
 */
double getLightnessFromRGB(double r, double g, double b)
{
  return 0.299 * r + 0.587 * g + 0.114 * b;
}

void applyMoveTransformation(const std::shared_ptr<GRM::Element> &element)
{
  double w[4], vp[4], vp_org[4];
  double x_shift = 0, y_shift = 0;
  double x_max_shift = 0, x_min_shift = 0, y_max_shift = 0, y_min_shift = 0;
  bool disable_x_trans = false, disable_y_trans = false, any_xform = false; // only for wc
  std::shared_ptr<GRM::Element> parent_element = element->parentElement();
  std::string post_fix = "_wc";
  std::vector<std::string> ndc_transformation_elems = {
      "figure",
      "plot",
      "colorbar",
      "label",
      "titles_3d",
      "text",
      "layout_grid_element",
      "layout_grid",
      "central_region",
      "side_region",
      "marginal_heatmap_plot",
      "legend",
      "axis",
      "side_plot_region",
      "text_region",
      "coordinate_system",
      "overlay_element",
  };

  if (std::find(ndc_transformation_elems.begin(), ndc_transformation_elems.end(), element->localName()) !=
      ndc_transformation_elems.end())
    post_fix = "_ndc";

  if (element->hasAttribute("x_max_shift" + post_fix))
    {
      x_max_shift = static_cast<double>(element->getAttribute("x_max_shift" + post_fix));
      any_xform = true;
    }
  if (element->hasAttribute("x_min_shift" + post_fix))
    {
      x_min_shift = static_cast<double>(element->getAttribute("x_min_shift" + post_fix));
      any_xform = true;
    }
  if (element->hasAttribute("y_max_shift" + post_fix))
    {
      y_max_shift = static_cast<double>(element->getAttribute("y_max_shift" + post_fix));
      any_xform = true;
    }
  if (element->hasAttribute("y_min_shift" + post_fix))
    {
      y_min_shift = static_cast<double>(element->getAttribute("y_min_shift" + post_fix));
      any_xform = true;
    }
  if (element->hasAttribute("x_shift" + post_fix))
    {
      x_shift = static_cast<double>(element->getAttribute("x_shift" + post_fix));
      any_xform = true;
    }
  if (element->hasAttribute("y_shift" + post_fix))
    {
      y_shift = static_cast<double>(element->getAttribute("y_shift" + post_fix));
      any_xform = true;
    }
  if (element->hasAttribute("disable_x_trans"))
    disable_x_trans = static_cast<int>(element->getAttribute("disable_x_trans"));
  if (element->hasAttribute("disable_y_trans"))
    disable_y_trans = static_cast<int>(element->getAttribute("disable_y_trans"));

  // get parent transformation when an element doesn't have an own in wc case
  if (!any_xform && post_fix == "_wc")
    {
      while (parent_element->localName() != "root" && !any_xform)
        {
          if (parent_element->hasAttribute("x_max_shift" + post_fix))
            {
              x_max_shift = static_cast<double>(parent_element->getAttribute("x_max_shift" + post_fix));
              any_xform = true;
            }
          if (parent_element->hasAttribute("x_min_shift" + post_fix))
            {
              x_min_shift = static_cast<double>(parent_element->getAttribute("x_min_shift" + post_fix));
              any_xform = true;
            }
          if (parent_element->hasAttribute("y_max_shift" + post_fix))
            {
              y_max_shift = static_cast<double>(parent_element->getAttribute("y_max_shift" + post_fix));
              any_xform = true;
            }
          if (parent_element->hasAttribute("y_min_shift" + post_fix))
            {
              y_min_shift = static_cast<double>(parent_element->getAttribute("y_min_shift" + post_fix));
              any_xform = true;
            }
          if (parent_element->hasAttribute("x_shift" + post_fix))
            {
              x_shift = static_cast<double>(parent_element->getAttribute("x_shift" + post_fix));
              any_xform = true;
            }
          if (parent_element->hasAttribute("y_shift" + post_fix))
            {
              y_shift = static_cast<double>(parent_element->getAttribute("y_shift" + post_fix));
              any_xform = true;
            }
          if (parent_element->hasAttribute("disable_x_trans"))
            disable_x_trans = static_cast<int>(parent_element->getAttribute("disable_x_trans"));
          if (parent_element->hasAttribute("disable_y_trans"))
            disable_y_trans = static_cast<int>(parent_element->getAttribute("disable_y_trans"));

          parent_element = parent_element->parentElement();
        }
    }

  if (post_fix == "_ndc")
    {
      // elements in ndc space gets transformed in ndc space which is equal to changing their viewport
      double diff;
      double vp_border_x_min = 0.0, vp_border_x_max, vp_border_y_min = 0.0, vp_border_y_max;
      bool private_shift = false;
      double plot[4] = {NAN, NAN, NAN, NAN};

      if (element->hasAttribute("viewport_x_min"))
        {
          vp_org[0] = static_cast<double>(element->getAttribute("_viewport_x_min_org"));
          vp_org[1] = static_cast<double>(element->getAttribute("_viewport_x_max_org"));
          vp_org[2] = static_cast<double>(element->getAttribute("_viewport_y_min_org"));
          vp_org[3] = static_cast<double>(element->getAttribute("_viewport_y_max_org"));
        }
      else
        {
          gr_inqviewport(&vp_org[0], &vp_org[1], &vp_org[2], &vp_org[3]);
        }

      if (element->hasAttribute("viewport_normalized_x_min") && element->hasAttribute("viewport_normalized_x_max") &&
          element->hasAttribute("viewport_normalized_y_min") && element->hasAttribute("viewport_normalized_y_max"))
        {
          // the org values are always 0 and 1
          plot[0] = static_cast<double>(element->getAttribute("_viewport_normalized_x_min_org"));
          plot[1] = static_cast<double>(element->getAttribute("_viewport_normalized_x_max_org"));
          plot[2] = static_cast<double>(element->getAttribute("_viewport_normalized_y_min_org"));
          plot[3] = static_cast<double>(element->getAttribute("_viewport_normalized_y_max_org"));
        }

      // apply viewport changes defined by the user via setAttribute first
      if (element->hasAttribute("_x_min_shift"))
        {
          auto shift = static_cast<double>(element->getAttribute("_x_min_shift"));
          vp_org[0] += shift;
          if (!grm_isnan(plot[0])) plot[0] += shift;
          private_shift = true;
        }
      if (element->hasAttribute("_x_max_shift"))
        {
          auto shift = static_cast<double>(element->getAttribute("_x_max_shift"));
          vp_org[1] += shift;
          if (!grm_isnan(plot[1])) plot[1] += shift;
          private_shift = true;
        }
      if (element->hasAttribute("_y_min_shift"))
        {
          auto shift = static_cast<double>(element->getAttribute("_y_min_shift"));
          vp_org[2] += shift;
          if (!grm_isnan(plot[2])) plot[2] += shift;
          private_shift = true;
        }
      if (element->hasAttribute("_y_max_shift"))
        {
          auto shift = static_cast<double>(element->getAttribute("_y_max_shift"));
          vp_org[3] += shift;
          if (!grm_isnan(plot[3])) plot[3] += shift;
          private_shift = true;
        }

      if (element->localName() == "text")
        {
          if (x_shift != 0 || y_shift != 0)
            {
              double metric_width, metric_height;
              GRM::getFigureSize(nullptr, nullptr, &metric_width, &metric_height);
              auto aspect_ratio_ws = metric_width / metric_height;
              auto text_x = static_cast<double>(element->getAttribute("x"));
              auto text_y = static_cast<double>(element->getAttribute("y"));

              // clipping for the text offset
              if (x_shift > 0)
                if (aspect_ratio_ws > 1)
                  x_shift = grm_min(0.99 - text_x, x_shift);
                else
                  x_shift = grm_min(aspect_ratio_ws - text_x - 0.01, x_shift);
              else
                x_shift = grm_max(0.01 - text_x, x_shift);

              if (y_shift < 0)
                if (aspect_ratio_ws > 1)
                  y_shift = grm_max(-text_y, y_shift);
                else
                  y_shift = grm_max(text_y - 1.0, y_shift);
              else
                y_shift = grm_min(text_y - 0.01, y_shift);
              gr_settextoffset(x_shift, -y_shift);
            }
          else
            gr_settextoffset(0, 0);
        }

      // when the element contains an axes the max viewport must be smaller than normal to respect the axes
      vp_border_x_min = getMinViewport(element, true);
      vp_border_x_max = getMaxViewport(element, true);
      vp_border_y_min = getMinViewport(element, false);
      vp_border_y_max = getMaxViewport(element, false);

      if (private_shift || (x_shift != 0 || x_max_shift != 0 || x_min_shift != 0 || y_shift != 0 || y_max_shift != 0 ||
                            y_min_shift != 0))
        {
          if (element->hasAttribute("viewport_normalized_x_min") &&
              element->hasAttribute("viewport_normalized_x_max") &&
              element->hasAttribute("viewport_normalized_y_min") && element->hasAttribute("viewport_normalized_y_max"))
            {
              plot[0] = plot[0] + x_min_shift + x_shift;
              plot[1] = plot[1] + x_max_shift + x_shift;
              plot[0] = grm_max(0, plot[0]);
              plot[1] = grm_min(1, plot[1]);

              plot[2] = plot[2] + y_min_shift + y_shift;
              plot[3] = plot[3] + y_max_shift + y_shift;
              plot[2] = grm_max(0, plot[2]);
              plot[3] = grm_min(1, plot[3]);

              bool old_state;
              GRM::Render::getAutoUpdate(&old_state);
              GRM::Render::setAutoUpdate(false);
              element->setAttribute("viewport_normalized_x_min", plot[0]);
              element->setAttribute("viewport_normalized_x_max", plot[1]);
              element->setAttribute("viewport_normalized_y_min", plot[2]);
              element->setAttribute("viewport_normalized_y_max", plot[3]);
              GRM::Render::setAutoUpdate(old_state);
            }

          // calculate viewport changes in x-direction
          vp[0] = vp_org[0] + x_min_shift + x_shift;
          vp[1] = vp_org[1] + x_max_shift + x_shift;
          diff = grm_min(vp[1] - vp[0], vp_border_x_max - vp_border_x_min);

          // the viewport cant leave the [vp_border_x_min, vp_border_x_max] space
          if (vp[0] < vp_border_x_min)
            {
              vp[0] = vp_border_x_min;
              vp[1] = vp_border_x_min + diff;
            }
          if (vp[1] > vp_border_x_max)
            {
              vp[0] = vp_border_x_max - diff;
              vp[1] = vp_border_x_max;
            }

          // calculate viewport changes in y-direction
          vp[2] = vp_org[2] - y_min_shift - y_shift;
          vp[3] = vp_org[3] - y_max_shift - y_shift;
          diff = grm_min(vp_border_y_max - vp_border_y_min, vp[3] - vp[2]);

          // the viewport cant leave the [vp_border_y_min, vp_border_y_max] space
          if (vp[2] < vp_border_y_min)
            {
              vp[3] = vp_border_y_min + diff;
              vp[2] = vp_border_y_min;
            }
          if (vp[3] > vp_border_y_max)
            {
              vp[2] = vp_border_y_max - diff;
              vp[3] = vp_border_y_max;
            }

          bool old_state;
          GRM::Render::getAutoUpdate(&old_state);
          GRM::Render::setAutoUpdate(false);
          grm_get_render()->setViewport(element, vp[0], vp[1], vp[2], vp[3]);
          processViewport(element);
          GRM::Render::setAutoUpdate(old_state);
        }
    }
  else if (x_shift != 0 || x_max_shift != 0 || x_min_shift != 0 || y_shift != 0 || y_max_shift != 0 || y_min_shift != 0)
    {
      // elements in world space gets transformed in world space which is equal to changing their window
      gr_inqwindow(&w[0], &w[1], &w[2], &w[3]);
      if (!disable_x_trans)
        {
          w[0] = w[0] + x_min_shift - x_shift;
          w[1] = w[1] + x_max_shift - x_shift;
        }
      if (!disable_y_trans)
        {
          w[2] = (w[2] - y_shift) - y_min_shift;
          w[3] = (w[3] - y_shift) - y_max_shift;
        }
      if (w[1] - w[0] > 0.0 && w[3] - w[2] > 0.0) gr_setwindow(w[0], w[1], w[2], w[3]);
    }
}

bool hasHighlightedParent(const std::shared_ptr<GRM::Element> &element)
{
  if (element->localName() == "root") return false;
  if (auto parent = element->parentElement(); parent->localName() != "root")
    {
      if (parent->hasAttribute("_highlighted") && static_cast<int>(parent->getAttribute("_highlighted"))) return true;
      return hasHighlightedParent(parent);
    }
  return false;
}

double autoTick(double min, double max)
{
  std::vector tick_sizes = {5.0, 2.0, 1.0, 0.5, 0.2, 0.1, 0.05, 0.02, 0.01};
  double tick = 1.0;
  int i;
  auto scale = pow(10.0, static_cast<int>(log10(max - min)));

  for (i = 0; i < tick_sizes.size(); i++)
    {
      if (static_cast<int>((max - min) / scale / tick_sizes[i]) > 7)
        {
          tick = tick_sizes[i - 1];
          break;
        }
    }
  tick *= scale;
  return tick;
}

std::shared_ptr<GRM::Element> getPlotElement(const std::shared_ptr<GRM::Element> &element)
{
  auto ancestor = element;

  while (ancestor->localName() != "figure")
    {
      if (bool ancestor_has_plot_group = (ancestor->hasAttribute("plot_group"));
          ancestor->parentElement()->localName() == "layout_grid_element" || ancestor_has_plot_group)
        return ancestor;
      ancestor = ancestor->parentElement();
    }
  return nullptr;
}

/*!
 * \brief Set colors from color index or rgb arrays. The render version
 *
 * Call the function first with an argument container and a key. Afterwards, call the `set_next_color` with `nullptr`
 * pointers to iterate through the color arrays. If `key` does not exist in `args`, the function falls back to default
 * colors.
 *
 * \param key The key of the colors in the argument container. The key may reference integer or double arrays. Integer
 * arrays describe colors of the GKS color table (0 - 1255). Double arrays contain RGB tuples in the range [0.0, 1.0].
 * If key does not exist, the routine falls back to default colors (taken from `gr_uselinespec`). \param color_type
 * The color type to set. Can be one of `GR_COLOR_LINE`, `GR_COLOR_MARKER`, `GR_COLOR_FILL`, `GR_COLOR_TEXT`,
 * `GR_COLOR_BORDER` or any combination of them (combined with OR). The special value `GR_COLOR_RESET` resets all
 * color modifications.
 */
int setNextColor(const std::string &key, GRColorType color_type, const std::shared_ptr<GRM::Element> &element,
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
  int gks_err_ind = GKS_K_NO_ERROR;
  auto global_render = grm_get_render();

  if (reset || !key.empty())
    {
      if (last_array_index >= 0 && !color_rgb_values.empty())
        {
          gr_setcolorrep(PLOT_CUSTOM_COLOR_INDEX, saved_color[0], saved_color[1], saved_color[2]);
        }
      last_array_index = -1;
      if (!reset && !key.empty())
        {
          if (!element->hasAttribute("color_ind_values") && !element->hasAttribute("color_rgb_values"))
            {
              /* use fallback colors if `key` cannot be read from `args` */
              logger((stderr, "Cannot read \"%s\" from args, falling back to default colors\n", key.c_str()));
              color_indices = fallback_color_indices;
              color_array_length = size(fallback_color_indices);
            }
          else
            {
              if (element->hasAttribute("color_ind_values"))
                {
                  auto c = static_cast<std::string>(element->getAttribute("color_ind_values"));
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
      gks_inq_color_rep(1, PLOT_CUSTOM_COLOR_INDEX, GKS_K_VALUE_SET, &gks_err_ind, &saved_color[0], &saved_color[1],
                        &saved_color[2]);
    }

  current_array_index %= static_cast<int>(color_array_length);

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

  if (color_type & GR_COLOR_LINE) global_render->setLineColorInd(element, color_index);
  if (color_type & GR_COLOR_MARKER) global_render->setMarkerColorInd(element, color_index);
  if (color_type & GR_COLOR_FILL) global_render->setFillColorInd(element, color_index);
  if (color_type & GR_COLOR_TEXT) global_render->setTextColorInd(element, color_index);
  if (color_type & GR_COLOR_BORDER) global_render->setBorderColorInd(element, color_index);

  return color_index;
}

void calculateWindowTransformationParameter(const std::shared_ptr<GRM::Element> &plot_parent, double w1_min,
                                            double w1_max, double w2_min, double w2_max, std::string location,
                                            double *a, double *b)
{
  bool x_log = false, y_log = false;
  if (plot_parent->hasAttribute("x_log")) x_log = static_cast<int>(plot_parent->getAttribute("x_log"));
  if (plot_parent->hasAttribute("y_log")) y_log = static_cast<int>(plot_parent->getAttribute("y_log"));

  if ((x_log && strEqualsAny(location, "bottom", "top", "twin_x")) ||
      (y_log && strEqualsAny(location, "left", "right", "twin_y")))
    {
      *a = (log10(w2_max) - log10(w2_min)) / (log10(w1_max) - log10(w1_min));
      *b = log10(w2_min) - *a * log10(w1_min);
    }
  else
    {
      *a = (w2_max - w2_min) / (w1_max - w1_min);
      *b = w2_min - *a * w1_min;
    }
}

void newWindowForTwinAxis(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Element> &axis_ref,
                          double *new_w_min, double *new_w_max, double old_w_min, double old_w_max)
{
  double a, b;
  double rounded_tick, tick, diff;
  int diff_log, num_ticks_log, decimal_places, num_ticks;
  std::shared_ptr<GRM::Element> plot_parent = element;
  getPlotParent(plot_parent);

  auto location = static_cast<std::string>(element->getAttribute("location"));

  diff = *new_w_max - *new_w_min;
  diff_log = ceil(log10(diff));
  num_ticks = static_cast<int>(axis_ref->getAttribute("num_ticks")) - 1;
  num_ticks_log = ceil(log10(num_ticks));
  decimal_places = diff_log - num_ticks_log - 1;

  // modify start window to control the first labels decimal places
  *new_w_min = floor(*new_w_min, decimal_places);
  *new_w_max = ceil(*new_w_max, decimal_places);

  diff = *new_w_max - *new_w_min;
  tick = diff / num_ticks;
  rounded_tick = round(tick, decimal_places);

  if (fabs(tick - rounded_tick) > 1e-12) //  more digits than wanted
    {
      auto new_tick = ceil(tick, decimal_places);
      auto modification = fabs(new_tick * num_ticks - diff) / 2.0;
      *new_w_min -= modification;
      *new_w_max += modification;
    }
  calculateWindowTransformationParameter(element, old_w_min, old_w_max, *new_w_min, *new_w_max, location, &a, &b);

  element->setAttribute("_" + location + "_window_xform_a", a);
  element->setAttribute("_" + location + "_window_xform_b", b);
  plot_parent->setAttribute("_" + location + "_window_xform_a", a);
  plot_parent->setAttribute("_" + location + "_window_xform_b", b);
}

void GRM::calculateCharHeight(const std::shared_ptr<GRM::Element> &element)
{
  /*!
   * processing function for gr_viewport
   *
   * \param[in] element The GRM::Element that contains the attributes
   */
  double plot_viewport[4];
  double figure_viewport[4]; // figure vp unless there are more plots inside a figure; then it's the vp for each plot
  auto plot_parent = getPlotElement(element);
  double char_height;
  std::shared_ptr<GRM::Element> figure_vp_element;
  auto kind = static_cast<std::string>(plot_parent->getAttribute("_kind"));
  double diag_factor;
  double metric_width, metric_height;
  bool uniform_data = true, keep_aspect_ratio = false, only_square_aspect_ratio = false;
  auto active_figure = grm_get_render()->getActiveFigure();

  // special case where the figure vp is not stored inside the plot element
  figure_vp_element = (plot_parent->parentElement()->localName() == "layout_grid_element")
                          ? figure_vp_element = plot_parent->parentElement()
                          : plot_parent;
  figure_viewport[0] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_x_min_org"));
  figure_viewport[1] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_x_max_org"));
  figure_viewport[2] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_y_min_org")) /
                       (DEFAULT_ASPECT_RATIO_FOR_SCALING);
  figure_viewport[3] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_y_max_org")) /
                       (DEFAULT_ASPECT_RATIO_FOR_SCALING);

  GRM::getFigureSize(nullptr, nullptr, &metric_width, &metric_height);
  auto aspect_ratio_ws = metric_width / metric_height;
  if (plot_parent->parentElement()->localName() == "layout_grid_element")
    {
      metric_width *= (figure_viewport[1] - figure_viewport[0]);
      metric_height *= (figure_viewport[3] - figure_viewport[2]) * DEFAULT_ASPECT_RATIO_FOR_SCALING;
      aspect_ratio_ws = metric_width / metric_height;
    }
  else if (kinds_3d.count(kind) > 0 && plot_parent->parentElement()->localName() != "layout_grid_element")
    {
      aspect_ratio_ws =
          static_cast<double>(plot_parent->querySelectors("central_region")->getAttribute("_vp_with_extent"));
    }
  auto start_aspect_ratio_ws = static_cast<double>(plot_parent->getAttribute("_start_aspect_ratio"));

  plot_viewport[0] = static_cast<double>(plot_parent->getAttribute("_viewport_x_min_org"));
  plot_viewport[1] = static_cast<double>(plot_parent->getAttribute("_viewport_x_max_org"));
  plot_viewport[2] = static_cast<double>(plot_parent->getAttribute("_viewport_y_min_org"));
  plot_viewport[3] = static_cast<double>(plot_parent->getAttribute("_viewport_y_max_org"));

  if (aspect_ratio_ws > start_aspect_ratio_ws)
    {
      plot_viewport[0] *= (start_aspect_ratio_ws / aspect_ratio_ws);
      plot_viewport[1] *= (start_aspect_ratio_ws / aspect_ratio_ws);
    }
  else
    {
      plot_viewport[2] *= (aspect_ratio_ws / start_aspect_ratio_ws);
      plot_viewport[3] *= (aspect_ratio_ws / start_aspect_ratio_ws);
    }
  keep_aspect_ratio = static_cast<int>(plot_parent->getAttribute("keep_aspect_ratio"));
  only_square_aspect_ratio = static_cast<int>(plot_parent->getAttribute("only_square_aspect_ratio"));

  // special case for keep_aspect_ratio with uniform data which can lead to smaller plots
  if (keep_aspect_ratio && only_square_aspect_ratio)
    {
      auto render = grm_get_render();
      for (const auto &series : plot_parent->querySelectors("central_region")->children())
        {
          if (!startsWith(series->localName(), "series_")) continue;
          uniform_data = isUniformData(series, render->getContext());
          if (!uniform_data) break;
        }
      if (kind == "marginal_heatmap" && uniform_data)
        uniform_data = isUniformData(plot_parent->children()[0], render->getContext());
      if (uniform_data)
        {
          double border =
              0.5 * (figure_viewport[1] - figure_viewport[0]) * (1.0 - 1.0 / (DEFAULT_ASPECT_RATIO_FOR_SCALING));
          figure_viewport[0] += border;
          figure_viewport[1] -= border;
        }
    }

  if ((keep_aspect_ratio && uniform_data && only_square_aspect_ratio) || kinds_3d.count(kind) > 0 || !keep_aspect_ratio)
    {
      auto initial_size_x = static_cast<double>(active_figure->getAttribute("_initial_width"));
      auto initial_size_y = static_cast<double>(active_figure->getAttribute("_initial_height"));
      auto size_x = static_cast<double>(active_figure->getAttribute("size_x"));
      auto size_y = static_cast<double>(active_figure->getAttribute("size_y"));
      double size_scale_factor = 1.0, multi_plot_factor;
      bool treat_like_no_layout = false;
      if ((initial_size_x != size_x || initial_size_y != size_y) && (active_figure->hasAttribute("_kind_changed")))
        size_scale_factor = (size_x < size_y) ? (size_y / size_x) : (size_x / size_y);

      if (figure_vp_element != plot_parent && kinds_3d.count(kind) > 0)
        {
          auto num_col = static_cast<int>(figure_vp_element->parentElement()->getAttribute("num_col"));
          auto num_row = static_cast<int>(figure_vp_element->parentElement()->getAttribute("num_row"));
          auto start_col = static_cast<int>(figure_vp_element->getAttribute("_start_col"));
          auto stop_col = static_cast<int>(figure_vp_element->getAttribute("_stop_col"));
          auto start_row = static_cast<int>(figure_vp_element->getAttribute("_start_row"));
          auto stop_row = static_cast<int>(figure_vp_element->getAttribute("_stop_row"));
          if (figure_vp_element->parentElement()->parentElement() != nullptr &&
              figure_vp_element->parentElement()->parentElement()->localName() == "layout_grid")
            {
              num_col = static_cast<int>(figure_vp_element->parentElement()->parentElement()->getAttribute("num_col"));
              num_row = static_cast<int>(figure_vp_element->parentElement()->parentElement()->getAttribute("num_row"));
            }
          num_col -= stop_col - start_col - 1;
          num_row -= stop_row - start_row - 1;
          if (num_col < num_row && num_col == 1) treat_like_no_layout = true;
          multi_plot_factor =
              grm_max(std::sqrt((figure_viewport[1] - figure_viewport[0]) * (figure_viewport[1] - figure_viewport[0]) +
                                (figure_viewport[3] - figure_viewport[2]) * (figure_viewport[3] - figure_viewport[2]) *
                                    DEFAULT_ASPECT_RATIO_FOR_SCALING * DEFAULT_ASPECT_RATIO_FOR_SCALING) /
                          sqrt(5),
                      figure_viewport[1] - figure_viewport[0]);
          figure_viewport[0] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_x_min_org"));
          figure_viewport[1] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_x_max_org"));
          figure_viewport[2] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_y_min_org"));
          figure_viewport[3] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_y_max_org"));
        }

      // calculate the diagonal viewport size of the default viewport with the fix aspect_ratio 4/3
      calculateCentralRegionMarginOrDiagFactor(element, &figure_viewport[0], &figure_viewport[1], &figure_viewport[2],
                                               &figure_viewport[3], true);
      diag_factor = std::sqrt((figure_viewport[1] - figure_viewport[0]) * (figure_viewport[1] - figure_viewport[0]) +
                              (figure_viewport[3] - figure_viewport[2]) * (figure_viewport[3] - figure_viewport[2]));

      if (figure_vp_element != plot_parent && kinds_3d.count(kind) > 0 && !treat_like_no_layout)
        {
          diag_factor *= size_scale_factor *
                         (start_aspect_ratio_ws <= 1 ? start_aspect_ratio_ws : (1.0 / start_aspect_ratio_ws)) *
                         DEFAULT_ASPECT_RATIO_FOR_SCALING * multi_plot_factor;
          element->setAttribute("_diag_factor", diag_factor);
        }
    }
  else
    {
      if (element->localName() != "colorbar")
        {
          diag_factor = std::sqrt((plot_viewport[1] - plot_viewport[0]) * (plot_viewport[1] - plot_viewport[0]) +
                                  (plot_viewport[3] - plot_viewport[2]) * (plot_viewport[3] - plot_viewport[2]));
          if (!element->hasAttribute("_default_diag_factor"))
            {
              double default_diag_factor;
              auto initial_size_x = static_cast<double>(active_figure->getAttribute("_initial_width"));
              auto initial_size_y = static_cast<double>(active_figure->getAttribute("_initial_height"));
              auto size_x = static_cast<double>(active_figure->getAttribute("size_x"));
              auto size_y = static_cast<double>(active_figure->getAttribute("size_y"));
              auto size_scale_factor = 1.0;
              if ((initial_size_x != size_x || initial_size_y != size_y) &&
                  active_figure->hasAttribute("_kind_changed"))
                size_scale_factor = (size_x < size_y) ? (size_y / size_x) : (size_x / size_y);
              double multi_plot_factor = grm_max(
                  std::sqrt((figure_viewport[1] - figure_viewport[0]) * (figure_viewport[1] - figure_viewport[0]) +
                            (figure_viewport[3] - figure_viewport[2]) * (figure_viewport[3] - figure_viewport[2]) *
                                DEFAULT_ASPECT_RATIO_FOR_SCALING * DEFAULT_ASPECT_RATIO_FOR_SCALING) /
                      sqrt(5),
                  figure_viewport[1] - figure_viewport[0]);

              if (figure_vp_element == plot_parent)
                {
                  calculateCentralRegionMarginOrDiagFactor(element, &figure_viewport[0], &figure_viewport[1],
                                                           &figure_viewport[2], &figure_viewport[3], true);
                  double plot_diag_factor =
                      std::sqrt((figure_viewport[1] - figure_viewport[0]) * (figure_viewport[1] - figure_viewport[0]) +
                                (figure_viewport[3] - figure_viewport[2]) * (figure_viewport[3] - figure_viewport[2]));
                  default_diag_factor =
                      ((DEFAULT_ASPECT_RATIO_FOR_SCALING) *
                       (start_aspect_ratio_ws <= 1 ? start_aspect_ratio_ws : (1.0 / start_aspect_ratio_ws))) *
                      (plot_diag_factor / (diag_factor * size_scale_factor));
                }
              else
                {
                  figure_viewport[0] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_x_min_org"));
                  figure_viewport[1] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_x_max_org"));
                  figure_viewport[2] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_y_min_org"));
                  figure_viewport[3] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_y_max_org"));
                  calculateCentralRegionMarginOrDiagFactor(element, &figure_viewport[0], &figure_viewport[1],
                                                           &figure_viewport[2], &figure_viewport[3], true);

                  auto num_col = static_cast<int>(figure_vp_element->parentElement()->getAttribute("num_col"));
                  auto num_row = static_cast<int>(figure_vp_element->parentElement()->getAttribute("num_row"));
                  auto start_col = static_cast<int>(figure_vp_element->getAttribute("_start_col"));
                  auto stop_col = static_cast<int>(figure_vp_element->getAttribute("_stop_col"));
                  auto start_row = static_cast<int>(figure_vp_element->getAttribute("_start_row"));
                  auto stop_row = static_cast<int>(figure_vp_element->getAttribute("_stop_row"));
                  if (figure_vp_element->parentElement()->parentElement() != nullptr &&
                      figure_vp_element->parentElement()->parentElement()->localName() == "layout_grid")
                    {
                      num_col = static_cast<int>(
                          figure_vp_element->parentElement()->parentElement()->getAttribute("num_col"));
                      num_row = static_cast<int>(
                          figure_vp_element->parentElement()->parentElement()->getAttribute("num_row"));
                    }
                  num_col -= stop_col - start_col - 1;
                  num_row -= stop_row - start_row - 1;

                  double plot_diag_factor =
                      std::sqrt((figure_viewport[1] - figure_viewport[0]) * (figure_viewport[1] - figure_viewport[0]) +
                                (figure_viewport[3] - figure_viewport[2]) * (figure_viewport[3] - figure_viewport[2]));
                  default_diag_factor =
                      ((DEFAULT_ASPECT_RATIO_FOR_SCALING) *
                       (start_aspect_ratio_ws <= 1 ? start_aspect_ratio_ws : (1.0 / start_aspect_ratio_ws))) *
                      (plot_diag_factor / (diag_factor * size_scale_factor));

                  if (num_col < num_row && num_col == 1) multi_plot_factor = 1. / (DEFAULT_ASPECT_RATIO_FOR_SCALING);
                  default_diag_factor *= multi_plot_factor;
                }
              element->setAttribute("_default_diag_factor", default_diag_factor);
            }
          diag_factor *= static_cast<double>(element->getAttribute("_default_diag_factor"));
        }
      else
        {
          double viewport[4];
          double default_diag_factor;
          std::string location;

          if (!element->parentElement()->hasAttribute("viewport_x_min") ||
              !element->parentElement()->hasAttribute("viewport_x_max") ||
              !element->parentElement()->hasAttribute("viewport_y_min") ||
              !element->parentElement()->hasAttribute("viewport_y_max"))
            {
              throw NotFoundError("Viewport not found\n");
            }
          if (!GRM::Render::getViewport(element->parentElement(), &viewport[0], &viewport[1], &viewport[2],
                                        &viewport[3]))
            throw NotFoundError(element->parentElement()->localName() + " doesn't have a viewport but it should.\n");

          location = static_cast<std::string>(element->parentElement()->parentElement()->getAttribute("location"));
          diag_factor = std::sqrt((plot_viewport[1] - plot_viewport[0]) * (plot_viewport[1] - plot_viewport[0]) +
                                  (plot_viewport[3] - plot_viewport[2]) * (plot_viewport[3] - plot_viewport[2]));
          // adjustment especially for horizontal colorbars where the char_height otherwise would be too big
          if (location == "bottom" || location == "top")
            diag_factor *= (figure_viewport[3] - figure_viewport[2]);
          else if (location == "left" || location == "right")
            diag_factor *= (figure_viewport[1] - figure_viewport[0]);
          if (element->hasAttribute("_default_diag_factor"))
            {
              default_diag_factor = static_cast<double>(element->getAttribute("_default_diag_factor"));
            }
          else
            {
              auto initial_size_x = static_cast<double>(active_figure->getAttribute("_initial_width"));
              auto initial_size_y = static_cast<double>(active_figure->getAttribute("_initial_height"));
              auto size_x = static_cast<double>(active_figure->getAttribute("size_x"));
              auto size_y = static_cast<double>(active_figure->getAttribute("size_y"));
              auto size_scale_factor = 1.0;
              if ((initial_size_x != size_x || initial_size_y != size_y) &&
                  active_figure->hasAttribute("_kind_changed"))
                size_scale_factor = (size_x < size_y) ? (size_y / size_x) : (size_x / size_y);
              double multi_plot_factor = grm_max(
                  std::sqrt((figure_viewport[1] - figure_viewport[0]) * (figure_viewport[1] - figure_viewport[0]) +
                            (figure_viewport[3] - figure_viewport[2]) * (figure_viewport[3] - figure_viewport[2]) *
                                DEFAULT_ASPECT_RATIO_FOR_SCALING * DEFAULT_ASPECT_RATIO_FOR_SCALING) /
                      sqrt(5),
                  figure_viewport[1] - figure_viewport[0]);

              if (figure_vp_element == plot_parent)
                {
                  calculateCentralRegionMarginOrDiagFactor(element, &figure_viewport[0], &figure_viewport[1],
                                                           &figure_viewport[2], &figure_viewport[3], true);
                  double plot_diag_factor =
                      std::sqrt((figure_viewport[1] - figure_viewport[0]) * (figure_viewport[1] - figure_viewport[0]) +
                                (figure_viewport[3] - figure_viewport[2]) * (figure_viewport[3] - figure_viewport[2]));
                  default_diag_factor =
                      ((DEFAULT_ASPECT_RATIO_FOR_SCALING) *
                       (start_aspect_ratio_ws <= 1 ? start_aspect_ratio_ws : (1.0 / start_aspect_ratio_ws))) *
                      (plot_diag_factor / (diag_factor * size_scale_factor));
                }
              else
                {
                  GRM::getFigureSize(nullptr, nullptr, &metric_width, &metric_height);
                  auto default_aspect_ratio_ws =
                      (metric_width * (figure_viewport[1] - figure_viewport[0])) /
                      (metric_height * (figure_viewport[3] - figure_viewport[2]) * DEFAULT_ASPECT_RATIO_FOR_SCALING);
                  figure_viewport[0] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_x_min_org"));
                  figure_viewport[1] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_x_max_org"));
                  figure_viewport[2] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_y_min_org"));
                  figure_viewport[3] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_y_max_org"));
                  if (default_aspect_ratio_ws > 1)
                    {
                      viewport[2] /= default_aspect_ratio_ws;
                      viewport[3] /= default_aspect_ratio_ws;
                    }
                  else
                    {
                      viewport[0] *= default_aspect_ratio_ws;
                      viewport[1] *= default_aspect_ratio_ws;
                    }
                  calculateCentralRegionMarginOrDiagFactor(element, &figure_viewport[0], &figure_viewport[1],
                                                           &figure_viewport[2], &figure_viewport[3], true);
                  calculateCentralRegionMarginOrDiagFactor(element, &figure_viewport[0], &figure_viewport[1],
                                                           &figure_viewport[2], &figure_viewport[3], true);

                  auto num_col = static_cast<int>(figure_vp_element->parentElement()->getAttribute("num_col"));
                  auto num_row = static_cast<int>(figure_vp_element->parentElement()->getAttribute("num_row"));
                  auto start_col = static_cast<int>(figure_vp_element->getAttribute("_start_col"));
                  auto stop_col = static_cast<int>(figure_vp_element->getAttribute("_stop_col"));
                  auto start_row = static_cast<int>(figure_vp_element->getAttribute("_start_row"));
                  auto stop_row = static_cast<int>(figure_vp_element->getAttribute("_stop_row"));
                  if (figure_vp_element->parentElement()->parentElement() != nullptr &&
                      figure_vp_element->parentElement()->parentElement()->localName() == "layout_grid")
                    {
                      num_col = static_cast<int>(
                          figure_vp_element->parentElement()->parentElement()->getAttribute("num_col"));
                      num_row = static_cast<int>(
                          figure_vp_element->parentElement()->parentElement()->getAttribute("num_row"));
                    }
                  num_col -= stop_col - start_col - 1;
                  num_row -= stop_row - start_row - 1;

                  double plot_diag_factor =
                      std::sqrt((figure_viewport[1] - figure_viewport[0]) * (figure_viewport[1] - figure_viewport[0]) +
                                (figure_viewport[3] - figure_viewport[2]) * (figure_viewport[3] - figure_viewport[2]));
                  default_diag_factor =
                      ((DEFAULT_ASPECT_RATIO_FOR_SCALING) *
                       (start_aspect_ratio_ws <= 1 ? start_aspect_ratio_ws : (1.0 / start_aspect_ratio_ws))) *
                      (plot_diag_factor / (diag_factor * size_scale_factor));

                  if (num_col < num_row && num_col == 1) multi_plot_factor = 1. / (DEFAULT_ASPECT_RATIO_FOR_SCALING);
                  default_diag_factor *= multi_plot_factor;
                }
              element->setAttribute("_default_diag_factor", default_diag_factor);
            }
          diag_factor *= default_diag_factor;
        }
    }

  if (!element->hasAttribute("_diag_factor")) element->setAttribute("_diag_factor", diag_factor);

  if (element->localName() != "colorbar")
    {
      if (!element->parentElement()->hasAttribute("_char_height_set_by_user"))
        {
          if (strEqualsAny(kind, "wireframe", "surface", "line3", "scatter3", "trisurface", "volume"))
            {
              char_height = PLOT_3D_CHAR_HEIGHT;
            }
          else if (polar_kinds.count(kind) > 0)
            {
              char_height = PLOT_POLAR_CHAR_HEIGHT;
            }
          else
            {
              char_height = PLOT_2D_CHAR_HEIGHT;
            }
          char_height *= diag_factor;

          if ((keep_aspect_ratio && uniform_data && only_square_aspect_ratio) || kinds_3d.count(kind) > 0 ||
              !keep_aspect_ratio)
            {
              if (aspect_ratio_ws > 1)
                {
                  char_height /= aspect_ratio_ws;
                }
              else
                {
                  char_height *= aspect_ratio_ws;
                }
              if (plot_parent->parentElement()->localName() != "layout_grid_element")
                char_height *= DEFAULT_ASPECT_RATIO_FOR_SCALING;
            }
        }
      else
        {
          char_height = static_cast<double>(plot_parent->getAttribute("char_height"));
        }

      plot_parent->setAttribute("char_height", char_height);
      processCharHeight(plot_parent);
    }
  else
    {
      // is always set otherwise the method wouldn't be called
      char_height = PLOT_DEFAULT_COLORBAR_CHAR_HEIGHT;

      if (!element->hasAttribute("_char_height_set_by_user"))
        {
          double char_height_rel;
          if ((keep_aspect_ratio && uniform_data && only_square_aspect_ratio) || kinds_3d.count(kind) > 0 ||
              !keep_aspect_ratio)
            {
              if (plot_parent->parentElement()->localName() != "layout_grid_element")
                char_height *= DEFAULT_ASPECT_RATIO_FOR_SCALING;
              if (aspect_ratio_ws <= 1)
                {
                  char_height_rel = char_height * aspect_ratio_ws;
                }
              else
                {
                  char_height_rel = char_height / aspect_ratio_ws;
                }
            }
          else
            {
              char_height_rel = char_height;
            }
          element->setAttribute("char_height", char_height_rel * diag_factor);
        }
      processCharHeight(element);
    }
}

void getTickSize(const std::shared_ptr<GRM::Element> &element, double &tick_size)
{
  auto active_figure = grm_get_render()->getActiveFigure();
  if (element->hasAttribute("tick_size") && element->parentElement()->localName() == "colorbar")
    {
      double tick_size_rel;
      double metric_width, metric_height;
      bool keep_aspect_ratio = false, uniform_data = true, only_square_aspect_ratio = false;
      std::shared_ptr<GRM::Element> figure_vp_element;
      auto plot_parent = element->parentElement();
      getPlotParent(plot_parent);

      GRM::getFigureSize(nullptr, nullptr, &metric_width, &metric_height);
      auto aspect_ratio_ws = metric_width / metric_height;

      // special case where the figure vp is not stored inside the plot element
      figure_vp_element = (plot_parent->parentElement()->localName() == "layout_grid_element")
                              ? figure_vp_element = plot_parent->parentElement()
                              : plot_parent;

      auto kind = static_cast<std::string>(plot_parent->getAttribute("_kind"));

      if (plot_parent->parentElement()->localName() == "layout_grid_element")
        {
          double figure_viewport[4];
          figure_viewport[0] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_x_min_org"));
          figure_viewport[1] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_x_max_org"));
          figure_viewport[2] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_y_min_org"));
          figure_viewport[3] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_y_max_org"));

          metric_width *= (figure_viewport[1] - figure_viewport[0]);
          metric_height *= (figure_viewport[3] - figure_viewport[2]);
          aspect_ratio_ws = metric_width / metric_height;
        }
      else if (kinds_3d.count(kind) > 0 && plot_parent->parentElement()->localName() != "layout_grid_element")
        {
          aspect_ratio_ws =
              static_cast<double>(plot_parent->querySelectors("central_region")->getAttribute("_vp_with_extent"));
        }
      auto start_aspect_ratio_ws = static_cast<double>(plot_parent->getAttribute("_start_aspect_ratio"));
      auto location = static_cast<std::string>(
          element->parentElement()->parentElement()->parentElement()->getAttribute("location"));

      if (element->hasAttribute("_tick_size_set_by_user"))
        {
          tick_size = static_cast<double>(element->getAttribute("_tick_size_set_by_user"));
        }
      else
        {
          if (element->hasAttribute("_tick_size_org"))
            {
              tick_size = static_cast<double>(element->getAttribute("_tick_size_org"));
            }
          else
            {
              tick_size = static_cast<double>(element->getAttribute("tick_size"));
              element->setAttribute("_tick_size_org", tick_size);
            }
        }

      keep_aspect_ratio = static_cast<int>(plot_parent->getAttribute("keep_aspect_ratio"));
      only_square_aspect_ratio = static_cast<int>(plot_parent->getAttribute("only_square_aspect_ratio"));
      // special case for keep_aspect_ratio with uniform data which can lead to smaller plots
      if (keep_aspect_ratio && only_square_aspect_ratio)
        {
          auto render = grm_get_render();
          for (const auto &series : plot_parent->querySelectors("central_region")->children())
            {
              if (!startsWith(series->localName(), "series_")) continue;
              uniform_data = isUniformData(series, render->getContext());
              if (!uniform_data) break;
            }
          if (kind == "marginal_heatmap" && uniform_data)
            uniform_data = isUniformData(plot_parent->children()[0], render->getContext());
        }

      if ((keep_aspect_ratio && uniform_data && only_square_aspect_ratio) || kinds_3d.count(kind) > 0 ||
          !keep_aspect_ratio)
        {
          if (aspect_ratio_ws <= 1)
            {
              tick_size_rel = tick_size * aspect_ratio_ws;
            }
          else
            {
              tick_size_rel = tick_size / aspect_ratio_ws;
            }

          if (figure_vp_element != plot_parent && kinds_3d.count(kind) > 0)
            {
              double multi_plot_factor, size_scale_factor = 1.0;
              double figure_viewport[4];
              bool treat_like_no_layout = false;
              auto initial_size_x = static_cast<double>(active_figure->getAttribute("_initial_width"));
              auto initial_size_y = static_cast<double>(active_figure->getAttribute("_initial_height"));
              auto size_x = static_cast<double>(active_figure->getAttribute("size_x"));
              auto size_y = static_cast<double>(active_figure->getAttribute("size_y"));

              figure_viewport[0] =
                  static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_x_min_org"));
              figure_viewport[1] =
                  static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_x_max_org"));
              figure_viewport[2] =
                  static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_y_min_org")) /
                  (DEFAULT_ASPECT_RATIO_FOR_SCALING);
              figure_viewport[3] =
                  static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_y_max_org")) /
                  (DEFAULT_ASPECT_RATIO_FOR_SCALING);

              if ((initial_size_x != size_x || initial_size_y != size_y) &&
                  active_figure->hasAttribute("_kind_changed"))
                size_scale_factor = (size_x < size_y) ? (size_y / size_x) : (size_x / size_y);

              auto num_col = static_cast<int>(figure_vp_element->parentElement()->getAttribute("num_col"));
              auto num_row = static_cast<int>(figure_vp_element->parentElement()->getAttribute("num_row"));
              auto start_col = static_cast<int>(figure_vp_element->getAttribute("_start_col"));
              auto stop_col = static_cast<int>(figure_vp_element->getAttribute("_stop_col"));
              auto start_row = static_cast<int>(figure_vp_element->getAttribute("_start_row"));
              auto stop_row = static_cast<int>(figure_vp_element->getAttribute("_stop_row"));
              if (figure_vp_element->parentElement()->parentElement() != nullptr &&
                  figure_vp_element->parentElement()->parentElement()->localName() == "layout_grid")
                {
                  num_col =
                      static_cast<int>(figure_vp_element->parentElement()->parentElement()->getAttribute("num_col"));
                  num_row =
                      static_cast<int>(figure_vp_element->parentElement()->parentElement()->getAttribute("num_row"));
                }
              num_col -= stop_col - start_col - 1;
              num_row -= stop_row - start_row - 1;
              if (num_col < num_row && num_col == 1) treat_like_no_layout = true;
              multi_plot_factor = grm_max(
                  std::sqrt((figure_viewport[1] - figure_viewport[0]) * (figure_viewport[1] - figure_viewport[0]) +
                            (figure_viewport[3] - figure_viewport[2]) * (figure_viewport[3] - figure_viewport[2]) *
                                DEFAULT_ASPECT_RATIO_FOR_SCALING * DEFAULT_ASPECT_RATIO_FOR_SCALING) /
                      sqrt(5),
                  figure_viewport[1] - figure_viewport[0]);
              figure_viewport[0] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_x_min_org"));
              figure_viewport[1] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_x_max_org"));
              figure_viewport[2] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_y_min_org"));
              figure_viewport[3] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_y_max_org"));

              // calculate the diagonal viewport size of the default viewport with the fix aspect_ratio 4/3
              calculateCentralRegionMarginOrDiagFactor(element, &figure_viewport[0], &figure_viewport[1],
                                                       &figure_viewport[2], &figure_viewport[3], true);
              auto diag_factor =
                  std::sqrt((figure_viewport[1] - figure_viewport[0]) * (figure_viewport[1] - figure_viewport[0]) +
                            (figure_viewport[3] - figure_viewport[2]) * (figure_viewport[3] - figure_viewport[2]));

              if (!treat_like_no_layout)
                {
                  diag_factor *= size_scale_factor *
                                 (start_aspect_ratio_ws <= 1 ? start_aspect_ratio_ws : (1.0 / start_aspect_ratio_ws)) *
                                 DEFAULT_ASPECT_RATIO_FOR_SCALING * multi_plot_factor;
                }
              tick_size_rel *= diag_factor;
            }

          if (!element->hasAttribute("_tick_size_set_by_user") &&
              plot_parent->parentElement()->localName() != "layout_grid_element")
            tick_size_rel *= DEFAULT_ASPECT_RATIO_FOR_SCALING;
        }
      else
        {
          double plot_viewport[4];
          double default_diag_factor;
          std::shared_ptr<GRM::Element> central_region, central_region_parent;

          central_region_parent = plot_parent;
          if (kind == "marginal_heatmap") central_region_parent = plot_parent->children()[0];
          for (const auto &child : central_region_parent->children())
            {
              if (child->localName() == "central_region")
                {
                  central_region = child;
                  break;
                }
            }

          plot_viewport[0] = static_cast<double>(plot_parent->getAttribute("_viewport_x_min_org"));
          plot_viewport[1] = static_cast<double>(plot_parent->getAttribute("_viewport_x_max_org"));
          plot_viewport[2] = static_cast<double>(plot_parent->getAttribute("_viewport_y_min_org"));
          plot_viewport[3] = static_cast<double>(plot_parent->getAttribute("_viewport_y_max_org"));

          if (aspect_ratio_ws > start_aspect_ratio_ws)
            {
              plot_viewport[0] *= (start_aspect_ratio_ws / aspect_ratio_ws);
              plot_viewport[1] *= (start_aspect_ratio_ws / aspect_ratio_ws);
            }
          else
            {
              plot_viewport[2] *= (aspect_ratio_ws / start_aspect_ratio_ws);
              plot_viewport[3] *= (aspect_ratio_ws / start_aspect_ratio_ws);
            }

          auto diag_factor = std::sqrt((plot_viewport[1] - plot_viewport[0]) * (plot_viewport[1] - plot_viewport[0]) +
                                       (plot_viewport[3] - plot_viewport[2]) * (plot_viewport[3] - plot_viewport[2]));
          if (!element->hasAttribute("_default_diag_factor"))
            {
              auto initial_size_x = static_cast<double>(active_figure->getAttribute("_initial_width"));
              auto initial_size_y = static_cast<double>(active_figure->getAttribute("_initial_height"));
              auto size_x = static_cast<double>(active_figure->getAttribute("size_x"));
              auto size_y = static_cast<double>(active_figure->getAttribute("size_y"));
              auto size_scale_factor = 1.0;
              if ((initial_size_x != size_x || initial_size_y != size_y) &&
                  (active_figure->hasAttribute("_kind_changed")))
                size_scale_factor = (size_x < size_y) ? (size_y / size_x) : (size_x / size_y);

              default_diag_factor =
                  ((DEFAULT_ASPECT_RATIO_FOR_SCALING) *
                   (start_aspect_ratio_ws <= 1 ? start_aspect_ratio_ws : (1.0 / start_aspect_ratio_ws))) /
                  (diag_factor * size_scale_factor);
              if (figure_vp_element != plot_parent)
                {
                  auto num_col = static_cast<int>(figure_vp_element->parentElement()->getAttribute("num_col"));
                  auto num_row = static_cast<int>(figure_vp_element->parentElement()->getAttribute("num_row"));
                  auto start_col = static_cast<int>(figure_vp_element->getAttribute("_start_col"));
                  auto stop_col = static_cast<int>(figure_vp_element->getAttribute("_stop_col"));
                  auto start_row = static_cast<int>(figure_vp_element->getAttribute("_start_row"));
                  auto stop_row = static_cast<int>(figure_vp_element->getAttribute("_stop_row"));
                  if (figure_vp_element->parentElement()->parentElement() != nullptr &&
                      figure_vp_element->parentElement()->parentElement()->localName() == "layout_grid")
                    {
                      num_col = static_cast<int>(
                          figure_vp_element->parentElement()->parentElement()->getAttribute("num_col"));
                      num_row = static_cast<int>(
                          figure_vp_element->parentElement()->parentElement()->getAttribute("num_row"));
                    }
                  num_col -= stop_col - start_col - 1;
                  num_row -= stop_row - start_row - 1;

                  if (num_row > 1 && num_row < num_col && num_row % 2 != num_col % 2)
                    {
                      default_diag_factor *= (1. / (num_col + 1));
                      if (num_row % 2 == 0) default_diag_factor /= start_aspect_ratio_ws;
                      if (polar_kinds.count(kind) > 0) default_diag_factor *= DEFAULT_ASPECT_RATIO_FOR_SCALING;
                    }
                  else if (num_row > 1)
                    {
                      default_diag_factor *= (1. / num_col);
                      if (polar_kinds.count(kind) > 0 && num_row != num_col)
                        default_diag_factor *= DEFAULT_ASPECT_RATIO_FOR_SCALING;
                    }
                  else if (num_row == 1 && num_row % 2 == num_col % 2)
                    default_diag_factor *= 0.6;
                  else if (num_row == 1 && num_row % 2 != num_col % 2)
                    default_diag_factor *= 0.5;
                }
              element->setAttribute("_default_diag_factor", default_diag_factor);
            }
          default_diag_factor = static_cast<double>(element->getAttribute("_default_diag_factor"));
          tick_size_rel = tick_size * diag_factor * default_diag_factor;
        }
      tick_size = tick_size_rel;
    }
  else
    {
      double vp_x_min, vp_x_max, vp_y_min, vp_y_max;
      auto plot_element = getPlotElement(element);
      std::shared_ptr<GRM::Element> central_region, central_region_parent;
      auto kind = static_cast<std::string>(plot_element->getAttribute("_kind"));
      double metric_width, metric_height;

      central_region_parent = plot_element;
      if (kind == "marginal_heatmap") central_region_parent = plot_element->children()[0];
      for (const auto &child : central_region_parent->children())
        {
          if (child->localName() == "central_region")
            {
              central_region = child;
              break;
            }
        }

      if (!GRM::Render::getViewport(central_region, &vp_x_min, &vp_x_max, &vp_y_min, &vp_y_max))
        throw NotFoundError("Central region doesn't have a viewport but it should.\n");

      GRM::getFigureSize(nullptr, nullptr, &metric_width, &metric_height);
      auto aspect_ratio_ws = metric_width / metric_height;

      double diag =
          std::sqrt((vp_x_max - vp_x_min) * (vp_x_max - vp_x_min) + (vp_y_max - vp_y_min) * (vp_y_max - vp_y_min));
      if (aspect_ratio_ws < 1 && kinds_3d.count(kind) > 0) diag /= aspect_ratio_ws;

      tick_size = PLOT_DEFAULT_AXES_TICK_SIZE * diag;
    }
}

void adjustValueForNonStandardAxis(const std::shared_ptr<GRM::Element> &plot_parent, double *value,
                                   std::string location)
{
  if (strEqualsAny(location, "bottom", "left", "right", "top", "twin_x", "twin_y"))
    {
      bool x_log = false, y_log = false;
      auto a = static_cast<double>(plot_parent->getAttribute("_" + location + "_window_xform_a"));
      auto b = static_cast<double>(plot_parent->getAttribute("_" + location + "_window_xform_b"));
      if (plot_parent->hasAttribute("x_log")) x_log = static_cast<int>(plot_parent->getAttribute("x_log"));
      if (plot_parent->hasAttribute("y_log")) y_log = static_cast<int>(plot_parent->getAttribute("y_log"));
      if ((x_log && strEqualsAny(location, "bottom", "top", "twin_x")) ||
          (y_log && strEqualsAny(location, "left", "right", "twin_y")))
        {
          *value = pow(10, (log10(*value) - b) / a);
        }
      else
        {
          *value = (*value - b) / a;
        }
    }
}

void getAxesInformation(const std::shared_ptr<GRM::Element> &element, const std::string &x_org_pos,
                        const std::string &y_org_pos, double &x_org, double &y_org, int &x_major, int &y_major,
                        double &x_tick, double &y_tick)
{
  double x_org_low, x_org_high;
  double y_org_low, y_org_high;
  int major_count;
  double xmin, xmax, ymin, ymax;
  std::shared_ptr<GRM::Element> central_region, central_region_parent, window_parent;
  auto global_render = grm_get_render();

  auto plot_element = getPlotElement(element);
  auto kind = static_cast<std::string>(plot_element->getAttribute("_kind"));

  central_region_parent = plot_element;
  if (kind == "marginal_heatmap") central_region_parent = plot_element->children()[0];
  for (const auto &child : central_region_parent->children())
    {
      if (child->localName() == "central_region")
        {
          central_region = child;
          break;
        }
    }

  auto scale = static_cast<int>(plot_element->getAttribute("scale"));

  window_parent = central_region;
  if (element->hasAttribute("window_x_min") && element->hasAttribute("window_x_max") &&
      element->hasAttribute("window_y_min") && element->hasAttribute("window_y_max"))
    window_parent = element;

  xmin = static_cast<double>(window_parent->getAttribute("window_x_min"));
  xmax = static_cast<double>(window_parent->getAttribute("window_x_max"));
  ymin = static_cast<double>(window_parent->getAttribute("window_y_min"));
  ymax = static_cast<double>(window_parent->getAttribute("window_y_max"));

  getMajorCount(element, kind, major_count);

  if (scale & GR_OPTION_X_LOG)
    {
      x_major = 1;
    }
  else
    {
      if (element->hasAttribute("x_major") && kind != "barplot")
        {
          x_major = static_cast<int>(element->getAttribute("x_major"));
        }
      else
        {
          if (kind == "barplot")
            {
              bool problematic_bar_num = false;
              auto context = global_render->getContext();
              auto barplots = central_region->querySelectorsAll("series_barplot");
              for (const auto &barplot : barplots)
                {
                  auto y_key = static_cast<std::string>(barplot->getAttribute("y"));
                  auto y_vec = GRM::get<std::vector<double>>((*context)[y_key]);
                  if (size(y_vec) > 20 || xmax - xmin > 20) // 20 based on the looking of the resulting plots
                    {
                      problematic_bar_num = true;
                      break;
                    }
                }
              x_major = problematic_bar_num ? major_count : 1;
            }
          else
            {
              x_major = major_count;
            }
        }
      element->setAttribute("x_major", x_major);
    }

  if (scale & GR_OPTION_X_LOG &&
      !(element->hasAttribute("x_tick") && static_cast<std::string>(element->getAttribute("name")) == "colorbar"))
    {
      x_tick = 1;
    }
  else
    {
      if (element->hasAttribute("x_tick") && kind != "barplot")
        {
          x_tick = static_cast<double>(element->getAttribute("x_tick"));
        }
      else
        {
          if (kind == "barplot")
            {
              x_tick = 1;
              // 60 based on the looking of the resulting plots
              if (x_major != 1 && xmax - xmin > 60) x_tick = autoTick(xmin, xmax) / x_major;
            }
          else
            {
              if (x_major != 0)
                {
                  x_tick = autoTick(xmin, xmax) / x_major;
                }
              else
                {
                  x_tick = 1;
                }
            }
        }
    }

  if (scale & GR_OPTION_FLIP_X &&
      !(element->hasAttribute("x_origin") && (static_cast<std::string>(element->getAttribute("name")) == "colorbar" ||
                                              element->localName() == "grid" || element->localName() == "grid3d")))
    {
      x_org_low = xmax;
      x_org_high = xmin;
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
  else
    {
      if (element->hasAttribute("x_origin"))
        {
          x_org = static_cast<double>(element->getAttribute("x_origin"));
        }
      else
        {
          x_org_low = xmin;
          x_org_high = xmax;
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
    }

  if (scale & GR_OPTION_Y_LOG)
    {
      y_major = 1;
    }
  else
    {
      if (element->hasAttribute("y_major"))
        {
          y_major = static_cast<int>(element->getAttribute("y_major"));
        }
      else
        {
          y_major = major_count;
          element->setAttribute("y_major", y_major);
        }
    }

  if (scale & GR_OPTION_Y_LOG &&
      !((element->localName() == "axes_3d" || static_cast<std::string>(element->getAttribute("name")) == "colorbar") &&
        element->hasAttribute("y_tick")))
    {
      y_tick = 1;
    }
  else
    {
      if (element->hasAttribute("y_tick"))
        {
          y_tick = static_cast<double>(element->getAttribute("y_tick"));
        }
      else
        {
          if (y_major != 0)
            {
              y_tick = autoTick(ymin, ymax) / y_major;
            }
          else
            {
              y_tick = 1;
            }
        }
    }

  if (scale & GR_OPTION_FLIP_Y &&
      !(element->hasAttribute("y_origin") && (static_cast<std::string>(element->getAttribute("name")) == "colorbar" ||
                                              element->localName() == "grid" || element->localName() == "grid3d")))
    {
      y_org_low = ymax;
      y_org_high = ymin;
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
  else
    {
      if (element->hasAttribute("y_origin"))
        {
          y_org = static_cast<double>(element->getAttribute("y_origin"));
        }
      else
        {
          y_org_low = ymin;
          y_org_high = ymax;
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
}

void getAxes3dInformation(const std::shared_ptr<GRM::Element> &element, const std::string &x_org_pos,
                          const std::string &y_org_pos, const std::string &z_org_pos, double &x_org, double &y_org,
                          double &z_org, int &x_major, int &y_major, int &z_major, double &x_tick, double &y_tick,
                          double &z_tick)
{
  getAxesInformation(element, x_org_pos, y_org_pos, x_org, y_org, x_major, y_major, x_tick, y_tick);

  double z_org_low, z_org_high;
  int major_count;
  std::shared_ptr<GRM::Element> central_region;

  auto draw_axes_group = element->parentElement();
  auto plot_element = getPlotElement(element);
  for (const auto &child : plot_element->children()) // don't need special case for marginal_heatmap cause it's 3d
    {
      if (child->localName() == "central_region")
        {
          central_region = child;
          break;
        }
    }

  auto kind = static_cast<std::string>(plot_element->getAttribute("_kind"));
  auto scale = static_cast<int>(plot_element->getAttribute("scale"));
  auto zmin = static_cast<double>(central_region->getAttribute("window_z_min"));
  auto zmax = static_cast<double>(central_region->getAttribute("window_z_max"));

  getMajorCount(element, kind, major_count);

  if (scale & GR_OPTION_Z_LOG)
    {
      z_major = 1;
    }
  else
    {
      if (element->hasAttribute("z_major"))
        {
          z_major = static_cast<int>(element->getAttribute("z_major"));
        }
      else
        {
          z_major = major_count;
        }
    }

  if (scale & GR_OPTION_Z_LOG && !(element->localName() == "axes_3d" && element->hasAttribute("z_tick")))
    {
      z_tick = 1;
    }
  else
    {
      if (element->hasAttribute("z_tick"))
        {
          z_tick = static_cast<double>(element->getAttribute("z_tick"));
        }
      else
        {
          if (z_major != 0)
            {
              z_tick = autoTick(zmin, zmax) / z_major;
            }
          else
            {
              z_tick = 1;
            }
        }
    }

  if (scale & GR_OPTION_FLIP_Z)
    {
      z_org_low = zmax;
      z_org_high = zmin;
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
  else
    {
      if (element->hasAttribute("z_origin"))
        {
          z_org = static_cast<double>(element->getAttribute("z_origin"));
        }
      else
        {
          z_org_low = zmin;
          z_org_high = zmax;
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
}

void markerHelper(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context,
                  const std::string &str)
{
  /*!
   * Helper function for marker functions using vectors for marker parameters
   *
   * \param[in] element The GRM::Element that contains all marker attributes and data keys. If element's parent is a
   * group element it may fallback to its marker attributes
   * \param[in] context The GRM::Context that contains the actual data
   * \param[in] str The std::string that specifies what GRM Routine should be called (polymarker)
   *
   */
  std::vector<int> type, color_ind;
  std::vector<double> size;
  std::string x_key, y_key, z_key;
  int skip_color_ind = -1000;
  auto parent = element->parentElement();
  bool group = parent_types.count(parent->localName());
  auto attr = element->getAttribute("marker_types");
  bool hidden = element->hasAttribute("_hidden") && static_cast<int>(element->getAttribute("_hidden"));
  auto render = grm_get_render();
  auto redraw_ws = render->getRedrawWs();

  if (attr.isString())
    {
      type = GRM::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
    }
  else if (group)
    {
      attr = parent->getAttribute("marker_types");
      if (attr.isString()) type = GRM::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
    }

  attr = element->getAttribute("marker_color_indices");
  if (attr.isString())
    {
      color_ind = GRM::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
    }
  else if (group)
    {
      attr = parent->getAttribute("marker_color_indices");
      if (attr.isString()) color_ind = GRM::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
    }

  attr = element->getAttribute("marker_sizes");
  if (attr.isString())
    {
      size = GRM::get<std::vector<double>>((*context)[static_cast<std::string>(attr)]);
    }
  else if (group)
    {
      attr = parent->getAttribute("marker_sizes");
      if (attr.isString()) size = GRM::get<std::vector<double>>((*context)[static_cast<std::string>(attr)]);
    }

  x_key = static_cast<std::string>(element->getAttribute("x"));
  y_key = static_cast<std::string>(element->getAttribute("y"));
  if (element->hasAttribute("z")) z_key = static_cast<std::string>(element->getAttribute("z"));

  auto x_vec = GRM::get<std::vector<double>>((*context)[x_key]);
  auto y_vec = GRM::get<std::vector<double>>((*context)[y_key]);
  std::vector<double> z_vec;
  if (auto z_ptr = GRM::getIf<std::vector<double>>((*context)[z_key])) z_vec = *z_ptr;

  auto n = std::min<int>(static_cast<int>(x_vec.size()), static_cast<int>(y_vec.size()));

  for (int i = 0; i < n; ++i)
    {
      // fallback to the last element when lists are too short
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
      if (!color_ind.empty())
        {
          if (color_ind.size() > i)
            {
              if (color_ind[i] == skip_color_ind) continue;
              gr_setmarkercolorind(color_ind[i]);
            }
          else
            {
              if (color_ind.back() == skip_color_ind) continue;
              gr_setmarkercolorind(color_ind.back());
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

      applyMoveTransformation(element);
      if (str == "polymarker")
        {
          if (redraw_ws && !hidden) gr_polymarker(1, (double *)&(x_vec[i]), (double *)&(y_vec[i]));
        }
      else if (str == "polymarker_3d")
        {
          processSpace3d(element->parentElement()->parentElement());
          if (redraw_ws && !hidden)
            gr_polymarker3d(1, (double *)&(x_vec[i]), (double *)&(y_vec[i]), (double *)&(z_vec[i]));
        }
    }
}

void lineHelper(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context,
                const std::string &str)
{
  /*!
   * Helper function for line functions using vectors for line parameters
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   * \param[in] str The std::string that specifies what GRM Routine should be called (polyline)
   *
   *
   */
  std::vector<int> type, color_ind;
  std::vector<double> width;
  std::string x_key, y_key, z_key;
  auto render = grm_get_render();
  auto redraw_ws = render->getRedrawWs();

  auto parent = element->parentElement();
  bool group = parent_types.count(parent->localName());
  bool hidden = element->hasAttribute("_hidden") && static_cast<int>(element->getAttribute("_hidden"));

  auto attr = element->getAttribute("line_types");
  if (attr.isString())
    {
      type = GRM::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
    }
  else if (group)
    {
      attr = parent->getAttribute("line_types");
      if (attr.isString()) type = GRM::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
    }

  attr = element->getAttribute("line_color_indices");
  if (attr.isString())
    {
      color_ind = GRM::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
    }
  else if (group)
    {
      attr = parent->getAttribute("line_color_indices");
      if (attr.isString()) color_ind = GRM::get<std::vector<int>>((*context)[static_cast<std::string>(attr)]);
    }

  attr = element->getAttribute("line_widths");
  if (attr.isString())
    {
      width = GRM::get<std::vector<double>>((*context)[static_cast<std::string>(attr)]);
    }
  else if (group)
    {
      attr = parent->getAttribute("line_widths");
      if (attr.isString()) width = GRM::get<std::vector<double>>((*context)[static_cast<std::string>(attr)]);
    }

  x_key = static_cast<std::string>(element->getAttribute("x"));
  y_key = static_cast<std::string>(element->getAttribute("y"));
  if (element->hasAttribute("z")) z_key = static_cast<std::string>(element->getAttribute("z"));

  auto x_vec = GRM::get<std::vector<double>>((*context)[x_key]);
  auto y_vec = GRM::get<std::vector<double>>((*context)[y_key]);
  std::vector<double> z_vec;

  if (auto z_ptr = GRM::getIf<std::vector<double>>((*context)[z_key])) z_vec = *z_ptr;

  auto n = std::min<int>(static_cast<int>(x_vec.size()), static_cast<int>(y_vec.size()));
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
      if (!color_ind.empty())
        {
          if (color_ind.size() > i)
            {
              gr_setlinecolorind(color_ind[i]);
            }
          else
            {
              gr_setlinecolorind(color_ind.back());
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

      applyMoveTransformation(element);
      if (str == "polyline")
        {
          if (redraw_ws && !hidden) gr_polyline(2, (double *)&(x_vec[i]), (double *)&(y_vec[i]));
        }
      else if (str == "polyline_3d")
        {
          processSpace3d(element->parentElement()->parentElement());
          if (redraw_ws && !hidden)
            gr_polyline3d(2, (double *)&(x_vec[i]), (double *)&(y_vec[i]), (double *)&(z_vec[i]));
        }
    }
}

void GRM::getFigureSize(int *pixel_width, int *pixel_height, double *metric_width, double *metric_height)
{
  double display_metric_width, display_metric_height;
  int display_pixel_width, display_pixel_height;
  double dpm[2], dpi[2];
  int pixel_size[2];
  double tmp_size_d[2], metric_size[2];
  int i;
  std::string size_unit, size_type;
  std::array<std::string, 2> vars = {"x", "y"};
  std::array<double, 2> default_size = {PLOT_DEFAULT_WIDTH, PLOT_DEFAULT_HEIGHT};
  std::shared_ptr<GRM::Element> figure = grm_get_render()->getActiveFigure();

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
  if (figure->hasAttribute("size_x") && figure->hasAttribute("size_y"))
    {
      for (i = 0; i < 2; ++i)
        {
          size_unit = static_cast<std::string>(figure->getAttribute("size_" + vars[i] + "_unit"));
          size_type = static_cast<std::string>(figure->getAttribute("size_" + vars[i] + "_type"));
          if (size_unit.empty()) size_unit = "px";
          tmp_size_d[i] = default_size[i];

          if (size_type == "double" || size_type == "int")
            {
              tmp_size_d[i] = static_cast<double>(figure->getAttribute("size_" + vars[i]));
              auto meters_per_unit_iter = symbol_to_meters_per_unit.find(size_unit);
              if (meters_per_unit_iter != symbol_to_meters_per_unit.end())
                {
                  double meters_per_unit = meters_per_unit_iter->second;
                  double pixels_per_unit = meters_per_unit * dpm[i];

                  tmp_size_d[i] *= pixels_per_unit;
                }
            }
          pixel_size[i] = static_cast<int>(grm_round(tmp_size_d[i]));
          metric_size[i] = tmp_size_d[i] / dpm[i];
        }
    }
  else
    {
      pixel_size[0] = static_cast<int>(grm_round(PLOT_DEFAULT_WIDTH));
      pixel_size[1] = static_cast<int>(grm_round(PLOT_DEFAULT_HEIGHT));
      metric_size[0] = PLOT_DEFAULT_WIDTH / dpm[0];
      metric_size[1] = PLOT_DEFAULT_HEIGHT / dpm[1];
    }

  if (pixel_width != nullptr) *pixel_width = pixel_size[0];
  if (pixel_height != nullptr) *pixel_height = pixel_size[1];
  if (metric_width != nullptr) *metric_width = metric_size[0];
  if (metric_height != nullptr) *metric_height = metric_size[1];
}

bool getLimitsForColorbar(const std::shared_ptr<GRM::Element> &element, double &c_min, double &c_max)
{
  bool limits_found = true;
  auto plot_parent = element->parentElement();
  getPlotParent(plot_parent);

  if (!std::isnan(static_cast<double>(plot_parent->getAttribute("_c_lim_min"))) &&
      !std::isnan(static_cast<double>(plot_parent->getAttribute("_c_lim_max"))))
    {
      c_min = static_cast<double>(plot_parent->getAttribute("_c_lim_min"));
      c_max = static_cast<double>(plot_parent->getAttribute("_c_lim_max"));
    }
  else if (!std::isnan(static_cast<double>(plot_parent->getAttribute("_z_lim_min"))) &&
           !std::isnan(static_cast<double>(plot_parent->getAttribute("_z_lim_max"))))
    {
      c_min = static_cast<double>(plot_parent->getAttribute("_z_lim_min"));
      c_max = static_cast<double>(plot_parent->getAttribute("_z_lim_max"));
    }
  else
    {
      limits_found = false;
    }

  return limits_found;
}

double findMaxStep(unsigned int n, std::vector<double> x)
{
  double max_step = 0.0;
  unsigned int i;

  if (n >= 2)
    {
      for (i = 1; i < n; ++i) max_step = grm_max(x[i] - x[i - 1], max_step);
    }

  return max_step;
}

void extendErrorBars(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context,
                     std::vector<double> x, std::vector<double> y)
{
  auto global_root = grm_get_document_root();
  auto id = static_cast<int>(global_root->getAttribute("_id"));
  auto str = std::to_string(id);
  global_root->setAttribute("_id", ++id);

  (*context)["x" + str] = std::move(x);
  element->setAttribute("x", "x" + str);
  (*context)["y" + str] = std::move(y);
  element->setAttribute("y", "y" + str);
}

void axisArgumentsConvertedIntoTickGroups(tick_t *ticks, tick_label_t *tick_labels,
                                          const std::shared_ptr<GRM::Element> &axis, DelValues del)
{
  int child_id = 1, label_ind = 0;
  std::string filter;
  std::shared_ptr<GRM::Element> tick_group, axis_ref;
  auto num_ticks = static_cast<int>(axis->getAttribute("num_ticks"));
  auto num_labels = static_cast<int>(axis->getAttribute("num_tick_labels"));
  auto axis_type = static_cast<std::string>(axis->getAttribute("axis_type"));
  auto location = static_cast<std::string>(axis->getAttribute("location"));
  auto global_creator = grm_get_creator();

  if (location == "twin_x")
    filter = "x";
  else if (location == "twin_y")
    filter = "y";
  if (!filter.empty()) axis_ref = axis->parentElement()->querySelectors("axis[location=\"" + filter + "\"]");

  if (static_cast<int>(axis->getAttribute("mirrored_axis"))) child_id += 1;
  if (!strEqualsAny(location, "twin_x", "twin_y") || !axis->hasAttribute("_" + location + "_window_xform_a") ||
      !axis_ref->hasAttribute("draw_grid") || !static_cast<int>(axis_ref->getAttribute("draw_grid")))
    {
      for (int i = 0; i < num_ticks; i++)
        {
          std::string label;
          double width = 0.0;
          if (label_ind < num_labels && tick_labels[label_ind].tick == ticks[i].value)
            {
              if (tick_labels[label_ind].label) label = tick_labels[label_ind].label;
              if (tick_labels[label_ind].width) width = tick_labels[label_ind].width;
              label_ind += 1;
            }

          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              tick_group = global_creator->createTickGroup(ticks[i].is_major, label, ticks[i].value, width);
              tick_group->setAttribute("_child_id", child_id++);
              axis->append(tick_group);
            }
          else
            {
              tick_group = axis->querySelectors("tick_group[_child_id=" + std::to_string(child_id++) + "]");
              if (tick_group != nullptr)
                tick_group =
                    global_creator->createTickGroup(ticks[i].is_major, label, ticks[i].value, width, tick_group);
            }
        }
    }
  else
    {
      std::shared_ptr<GRM::Element> plot_parent = axis;
      getPlotParent(plot_parent);

      axis->setAttribute("num_ticks", static_cast<int>(axis_ref->getAttribute("num_ticks")));
      axis->setAttribute("num_tick_labels", static_cast<int>(axis_ref->getAttribute("num_tick_labels")));
      auto org = static_cast<double>(axis->getAttribute("origin"));
      auto min = static_cast<double>(axis->getAttribute("min"));
      auto max = static_cast<double>(axis->getAttribute("max"));
      auto tick = static_cast<double>(axis->getAttribute("tick"));
      auto major_count = static_cast<double>(axis->getAttribute("major_count"));

      for (int i = 0; i < static_cast<int>(axis->getAttribute("num_ticks")); i++)
        {
          std::string label;
          double width = 0.0;

          auto a = static_cast<double>(plot_parent->getAttribute("_" + location + "_window_xform_a"));
          auto b = static_cast<double>(plot_parent->getAttribute("_" + location + "_window_xform_b"));
          auto ref_tick_group = axis_ref->querySelectors("tick_group[_child_id=" + std::to_string(child_id) + "]");
          auto is_major = static_cast<int>(ref_tick_group->getAttribute("is_major"));
          auto tick_value = static_cast<double>(ref_tick_group->getAttribute("value"));
          tick_value = a * tick_value + b;

          if (label_ind < static_cast<int>(axis->getAttribute("num_tick_labels")) &&
              !static_cast<std::string>(ref_tick_group->getAttribute("tick_label")).empty())
            {
              char text_buffer[PLOT_POLAR_AXES_TEXT_BUFFER] = "";
              format_reference_t reference;
              gr_getformat(&reference, org, min, max, tick, major_count);
              snprintf(text_buffer, PLOT_POLAR_AXES_TEXT_BUFFER, "%s", std::to_string(tick_value).c_str());

              auto value_string = gr_ftoa(text_buffer, tick_value, &reference);
              label = value_string;
              width = static_cast<double>(ref_tick_group->getAttribute("width"));
              label_ind += 1;
            }

          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              tick_group = global_creator->createTickGroup(is_major, label, tick_value, width);
              tick_group->setAttribute("_child_id", child_id++);
              axis->append(tick_group);
            }
          else
            {
              tick_group = axis->querySelectors("tick_group[_child_id=" + std::to_string(child_id++) + "]");
              if (tick_group != nullptr)
                tick_group = global_creator->createTickGroup(is_major, label, tick_value, width, tick_group);
            }
        }
    }
}

void calculatePolarLimits(const std::shared_ptr<GRM::Element> &central_region,
                          const std::shared_ptr<GRM::Context> &context)
{
  double r_min = 0.0, r_max = 0.0, tick;
  double min_scale = 0, max_scale; // used for y_log (with negative exponents)
  int rings = -1;
  double r_lim_min = 0.0, r_lim_max = 0.0;
  bool r_lims = false, r_log = false, keep_radii_axes = false;
  std::shared_ptr<GRM::Element> plot_parent = central_region;
  getPlotParent(plot_parent);

  if (plot_parent->hasAttribute("r_log")) r_log = static_cast<int>(plot_parent->getAttribute("r_log"));
  auto kind = static_cast<std::string>(plot_parent->getAttribute("_kind"));

  if (central_region->hasAttribute("r_min"))
    r_min = (kind == "uniform_polar_heatmap") ? 0.0 : static_cast<double>(central_region->getAttribute("r_min"));
  if (central_region->hasAttribute("r_max")) r_max = static_cast<double>(central_region->getAttribute("r_max"));

  if (plot_parent->hasAttribute("r_lim_min") && plot_parent->hasAttribute("r_lim_max"))
    {
      r_lims = true;
      r_lim_min = static_cast<double>(plot_parent->getAttribute("r_lim_min"));
      r_lim_max = static_cast<double>(plot_parent->getAttribute("r_lim_max"));
    }

  if (r_log && r_lim_min == 0.0)
    {
      r_lim_min = pow(10, -1);
      plot_parent->setAttribute("r_lim_min", r_lim_min);
    }

  if (plot_parent->hasAttribute("keep_radii_axes"))
    keep_radii_axes = static_cast<int>(plot_parent->getAttribute("keep_radii_axes"));

  if ((kind == "polar_histogram" && (!r_lims || keep_radii_axes)) ||
      ((kind == "polar_line" || kind == "polar_scatter") && !r_lims))
    {
      if (kind == "polar_histogram" || ((kind == "polar_line" || kind == "polar_scatter") && !r_log))
        {
          if (kind == "polar_line" || kind == "polar_scatter")
            {
              // find the maximum yrange of all series, so that all plots are visible
              for (const auto &series_elem : central_region->querySelectorsAll("series_" + kind))
                {
                  r_max = grm_max(r_max, static_cast<double>(series_elem->getAttribute("r_range_max")));
                }
            }
          r_min = 0.0;
          if (kind == "polar_histogram" && r_log) r_min = 1.0;
          tick = autoTick(r_min, r_max);
          rings = static_cast<int>(round(r_max / tick));
          if (rings * tick < r_max) rings += 1;
        }
      else if ((kind == "polar_line" || kind == "polar_scatter") && r_log)
        {
          if (r_max <= 0.0) throw InvalidValueError("The max radius has to be bigger than 0.0 when using r_log");
          max_scale = ceil(log10(r_max));

          if (r_min > 0.0)
            {
              min_scale = ceil(abs(log10(r_min)));
              if (min_scale != 0.0) min_scale *= (log10(r_min) / abs(log10(r_min)));
            }
          else
            {
              min_scale = 0.0;
              if (max_scale <= 0) min_scale = max_scale - 5;
            }

          if (max_scale == min_scale)
            {
              throw InvalidValueError(
                  "The minimum and maximum radius are of the same magnitude, incompatible with the y_log option");
            }

          // Todo: smart ring calculation for r_log especially with large differences in magnitudes (other cases
          //  too)
          r_min = pow(10, min_scale);
          rings = static_cast<int>(abs(abs(max_scale) - abs(min_scale)));
          // overwrite r_max and r_min because of rounded scales?
          central_region->setAttribute("r_max", pow(10, max_scale));
        }
      if ((kind != "polar_line" && kind != "polar_scatter") || !r_log)
        {
          central_region->setAttribute("tick", tick);
          central_region->setAttribute("r_max", tick * rings);
        }
      central_region->setAttribute("r_min", r_min);
    }
  else
    {
      // currently only plot_polar r_log is supported
      if ((kind == "polar_line" || kind == "polar_scatter") && r_log) // also with r_lims
        {
          max_scale = ceil(abs(log10(r_lim_max)));
          if (max_scale != 0.0) max_scale *= log10(r_lim_max) / abs(log10(r_lim_max));

          if (r_lim_min <= 0.0)
            {
              min_scale = (max_scale <= 0) ? max_scale - 5 : 0;
              if (r_min > 0.0) min_scale = ceil(abs(log10(r_min))) * (log10(r_min) / abs(log10(r_min)));
              r_lim_min = pow(10, min_scale);
              plot_parent->setAttribute("r_lim_min", r_lim_min);
            }
          else
            {
              min_scale = ceil(abs(log10(r_lim_min)));
              if (min_scale != 0.0) min_scale *= (log10(r_lim_min) / abs(log10(r_lim_min)));
            }

          central_region->setAttribute("r_min", pow(10, min_scale));
          central_region->setAttribute("r_max", pow(10, max_scale));
        }
      else // this currently includes polar(r_lims), p_hist(r_lims, !keep_radii_axes), polar_heatmap(all cases)
        {
          if (r_lims)
            {
              r_max = r_lim_max;
              central_region->setAttribute("r_min", r_lim_min);
            }
          else if (central_region->hasAttribute("tick"))
            {
              rings = grm_max(4, (int)(r_max - r_min));
              tick = static_cast<double>(central_region->getAttribute("tick"));
              r_max = tick * rings;
              central_region->setAttribute("r_min", r_min);
            }
          central_region->setAttribute("r_max", r_max);
        }
    }
}

void adjustPolarGridLineTextPosition(double theta_lim_min, double theta_lim_max, double *theta_r, double *r_r,
                                     double value, std::shared_ptr<GRM::Element> central_region)
{
  double window[4];
  double theta0, r0;

  window[0] = static_cast<double>(central_region->getAttribute("window_x_min"));
  window[1] = static_cast<double>(central_region->getAttribute("window_x_max"));
  window[2] = static_cast<double>(central_region->getAttribute("window_y_min"));
  window[3] = static_cast<double>(central_region->getAttribute("window_y_max"));
  if (theta_lim_min > 0 || theta_lim_max < 360)
    {
      // calculate unscaled position for label
      theta0 = std::cos(theta_lim_min * M_PI / 180.0), r0 = std::sin(theta_lim_min * M_PI / 180.0);
      // scale label with the real value of the arc
      theta0 *= value * window[3];
      r0 *= value * window[3];
      // add small offset to the resulting position so that the label have some space to the polyline
      // tested by going through the angles by steps of 15 degree
      if (theta_lim_min <= 135 && theta_lim_min >= 45)
        theta0 += 0.03 * (window[1] - window[0]) / 2.0;
      else if (theta_lim_min >= 225 && theta_lim_min <= 315)
        theta0 -= 0.03 * (window[1] - window[0]) / 2.0;
      if (theta_lim_min < 23 && theta_lim_min >= 0)
        r0 -= 0.03 * (window[3] - window[2]) / 2.0;
      else if (theta_lim_min < 45 && theta_lim_min >= 23)
        r0 -= 0.015 * (window[3] - window[2]) / 2.0;
      else if (theta_lim_min > 45 && theta_lim_min <= 68)
        r0 += 0.015 * (window[3] - window[2]) / 2.0;
      else if (theta_lim_min > 68 && theta_lim_min < 90)
        r0 += 0.03 * (window[3] - window[2]) / 2.0;
      else if (theta_lim_min < 112 && theta_lim_min > 90)
        r0 -= 0.03 * (window[3] - window[2]) / 2.0;
      else if (theta_lim_min >= 112 && theta_lim_min < 135)
        r0 -= 0.015 * (window[3] - window[2]) / 2.0;
      else if (theta_lim_min > 135 && theta_lim_min <= 158)
        r0 += 0.015 * (window[3] - window[2]) / 2.0;
      else if (theta_lim_min < 180 && theta_lim_min > 158)
        r0 += 0.03 * (window[3] - window[2]) / 2.0;
      else if (theta_lim_min < 180 && theta_lim_min > 135)
        r0 += 0.03 * (window[3] - window[2]) / 2.0;
      else if (theta_lim_min < 180 && theta_lim_min > 135)
        r0 += 0.03 * (window[3] - window[2]) / 2.0;
      else if (theta_lim_min >= 202 && theta_lim_min < 225)
        r0 += 0.015 * (window[3] - window[2]) / 2.0;
      else if (theta_lim_min <= 248 && theta_lim_min > 225)
        r0 -= 0.015 * (window[3] - window[2]) / 2.0;
      else if (theta_lim_min < 270 && theta_lim_min > 248)
        r0 -= 0.03 * (window[3] - window[2]) / 2.0;
      else if (theta_lim_min > 270 && theta_lim_min < 292)
        r0 += 0.03 * (window[3] - window[2]) / 2.0;
      else if (theta_lim_min >= 292 && theta_lim_min < 315)
        r0 += 0.015 * (window[3] - window[2]) / 2.0;
      else if (theta_lim_min > 315 && theta_lim_min <= 338)
        r0 -= 0.015 * (window[3] - window[2]) / 2.0;
      else if (theta_lim_min > 338)
        r0 -= 0.03 * (window[3] - window[2]) / 2.0;
      *theta_r = theta0;
      *r_r = r0;
    }
}

void histBins(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  double *tmp_bins;
  std::vector<double> x, weights;
  unsigned int num_bins = 0, num_weights = 0;
  double ymin = 0;
  auto global_root = grm_get_document_root();

  if (!element->hasAttribute("x")) throw NotFoundError("Histogram series is missing required attribute x-data.\n");
  auto key = static_cast<std::string>(element->getAttribute("x"));
  x = GRM::get<std::vector<double>>((*context)[key]);
  auto current_point_count = (int)x.size();

  if (element->hasAttribute("num_bins")) num_bins = static_cast<int>(element->getAttribute("num_bins"));
  if (element->hasAttribute("weights"))
    {
      auto weights_key = static_cast<std::string>(element->getAttribute("weights"));
      weights = GRM::get<std::vector<double>>((*context)[weights_key]);
      num_weights = weights.size();
    }
  if (!weights.empty() && current_point_count != num_weights)
    throw std::length_error("For histogram series the size of data and weights must be the same.\n");
  if (element->hasAttribute("y_range_min")) ymin = static_cast<double>(element->getAttribute("y_range_min"));

  if (num_bins <= 1) num_bins = (int)(3.3 * log10(current_point_count) + 0.5) + 1;
  auto bins = std::vector<double>(num_bins);
  double *x_p = &(x[0]);
  double *weights_p = (weights.empty()) ? nullptr : &(weights[0]);
  tmp_bins = &(bins[0]);
  binData(current_point_count, x_p, num_bins, tmp_bins, weights_p, ymin);
  std::vector<double> tmp(tmp_bins, tmp_bins + num_bins);

  auto id = static_cast<int>(global_root->getAttribute("_id"));
  auto str = std::to_string(id);
  (*context)["bins" + str] = tmp;
  element->setAttribute("bins", "bins" + str);
  global_root->setAttribute("_id", ++id);
}

void calculatePolarThetaAndR(std::vector<double> &theta, std::vector<double> &r,
                             const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  double r_min, r_max;
  double r_lim_min, r_lim_max, r_range_min, r_range_max, theta_range_min, theta_range_max;
  double theta_min, theta_max;
  bool transform_radii = false, transform_angles = false, clip_negative = false, r_log = false;
  unsigned int radial_length, theta_length;
  unsigned int i, index = 0;
  std::string kind;
  std::vector<double> theta_vec, radial_vec;
  std::shared_ptr<GRM::Element> plot_parent = element, central_region;
  getPlotParent(plot_parent);

  central_region = plot_parent->querySelectors("central_region");
  kind = static_cast<std::string>(plot_parent->getAttribute("_kind"));

  if (!element->hasAttribute("theta"))
    throw NotFoundError(kind + " series is missing required attribute theta-data (x).\n");
  auto theta_key = static_cast<std::string>(element->getAttribute("theta"));
  if (!element->hasAttribute("r")) throw NotFoundError(kind + " series is missing required attribute r-data (y).\n");
  auto r_key = static_cast<std::string>(element->getAttribute("r"));
  theta_vec = GRM::get<std::vector<double>>((*context)[theta_key]);
  radial_vec = GRM::get<std::vector<double>>((*context)[r_key]);
  theta_length = theta_vec.size();
  radial_length = radial_vec.size();

  if (plot_parent->hasAttribute("r_lim_max") && plot_parent->hasAttribute("r_lim_min"))
    {
      r_lim_min = static_cast<double>(plot_parent->getAttribute("r_lim_min"));
      r_lim_max = static_cast<double>(plot_parent->getAttribute("r_lim_max"));
    }
  else
    {
      r_lim_min = static_cast<double>(central_region->getAttribute("r_min"));
      r_lim_max = static_cast<double>(central_region->getAttribute("r_max"));
    }

  if (element->hasAttribute("r_range_min") && element->hasAttribute("r_range_max"))
    {
      transform_radii = true;
      r_range_min = static_cast<double>(element->getAttribute("r_range_min"));
      r_range_max = static_cast<double>(element->getAttribute("r_range_max"));
    }

  if (plot_parent->hasAttribute("r_log")) r_log = static_cast<int>(plot_parent->getAttribute("r_log"));
  if (r_log)
    {
      // apply r_log to lims so that the data can get the log10 applied
      r_lim_min = log10(r_lim_min);
      r_lim_max = log10(r_lim_max);
    }

  if (element->hasAttribute("theta_range_min") && element->hasAttribute("theta_range_max"))
    {
      theta_range_min = static_cast<double>(element->getAttribute("theta_range_min"));
      theta_range_max = static_cast<double>(element->getAttribute("theta_range_max"));
      transform_angles = true;

      if (theta_range_max > 2 * M_PI)
        {
          // convert from degrees to radians
          theta_range_max *= (M_PI / 180.0);
          theta_range_min *= (M_PI / 180.0);
        }
    }

  if (element->hasAttribute("clip_negative")) clip_negative = static_cast<int>(element->getAttribute("clip_negative"));

  // negative radii or NAN are clipped before the transformation into specified y_range (also when r_log is given)
  if (clip_negative || r_log)
    {
      std::vector<unsigned int> indices_vec;
      for (i = 0; i < radial_length; i++)
        {
          if (std::signbit(radial_vec[i]) || std::isnan(radial_vec[i])) indices_vec.insert(indices_vec.begin(), i);
          if (clip_negative && r_log && radial_vec[i] <= 0) indices_vec.insert(indices_vec.begin(), i);
        }

      for (auto ind : indices_vec)
        {
          radial_vec.erase(radial_vec.begin() + ind);
          theta_vec.erase(theta_vec.begin() + ind);
        }
      indices_vec.clear();
      radial_length = radial_vec.size();
      theta_length = theta_vec.size();
    }

  // get the minima and maxima from the data for possible transformations
  r_min = *std::min_element(radial_vec.begin(), radial_vec.end());
  r_max = *std::max_element(radial_vec.begin(), radial_vec.end());
  theta_min = *std::min_element(theta_vec.begin(), theta_vec.end());
  theta_max = *std::max_element(theta_vec.begin(), theta_vec.end());

  // clip_negative is not compatible with user given ranges, it overwrites
  if (clip_negative)
    {
      if (std::signbit(r_range_min))
        {
          theta_range_min = theta_min;
          r_range_min = r_min;
        }
      if (std::signbit(theta_range_max))
        {
          theta_range_max = theta_max;
          r_range_max = r_max;
        }
      transform_radii = false;
      transform_angles = false;
    }
  if (r_min == r_range_min && r_max == r_range_max) transform_radii = false;
  if (theta_min == theta_range_min && theta_max == theta_range_max) transform_angles = false;

  if (radial_length != theta_length)
    throw std::length_error("For " + kind + "series r(y)- and theta(x)-data must have the same size.\n");
  theta.resize(radial_length);
  r.resize(radial_length);

  // transform angles into specified x_ranges if given
  if (transform_angles) transformCoordinatesVector(theta_vec, theta_min, theta_max, theta_range_min, theta_range_max);

  // transform radii into y_range if given or log scale
  for (i = 0; i < radial_length; i++)
    {
      double current_radial;
      if (transform_radii || r_log)
        {
          double temp_radial = radial_vec[i];
          if (std::isnan(radial_vec[i])) continue; // skip NAN data

          if (r_log && !transform_radii)
            {
              current_radial = transformCoordinate(temp_radial, r_lim_min, r_lim_max, 0.0, 0.0, r_log);
            }
          else
            {
              current_radial = transformCoordinate(temp_radial, r_min, r_max, r_range_min, r_range_max, r_log);
            }
        }
      else
        {
          if (radial_vec[i] < 0)
            {
              // iterate over radial_vec and for each negative value add 180 degrees in radian to the corresponding
              // value in theta_vec and make the radial_vec value positive
              theta_vec[i] += M_PI;
              // if theta_vec[i] is bigger than 2 * PI, subtract 2 * PI
              if (theta_vec[i] > 2 * M_PI) theta_vec[i] -= 2 * M_PI;
              radial_vec[i] = -radial_vec[i];
            }

          current_radial = radial_vec[i];
        }
      if (r_lim_max != 0.0) current_radial /= r_lim_max;
      theta[index] = current_radial * cos(theta_vec[index]);
      r[index] = current_radial * sin(theta_vec[index]);
      ++index;
    }
  theta.resize(index);
  r.resize(index);
}

void tickLabelAdjustment(const std::shared_ptr<GRM::Element> &tick_group, int child_id, DelValues del)
{
  double char_height;
  double available_width, available_height;
  double x, y, y_org, width;
  double window[4];
  int scientific_format = 2;
  std::shared_ptr<GRM::Element> text_elem, plot_parent = tick_group, window_parent;
  char new_label[256];
  int cur_start = 0, scale = 0, cur_child_count = 0, child_id_org = child_id - 1, i = 0;
  bool x_flip, y_flip, text_is_empty_or_number = true;
  double metric_width, metric_height;
  double aspect_ratio_ws_metric;
  auto global_creator = grm_get_creator();

  GRM::getFigureSize(nullptr, nullptr, &metric_width, &metric_height);
  aspect_ratio_ws_metric = metric_width / metric_height;
  getPlotParent(plot_parent);
  gr_inqcharheight(&char_height);

  auto text = static_cast<std::string>(tick_group->getAttribute("tick_label"));
  auto axis_type = static_cast<std::string>(tick_group->parentElement()->getAttribute("axis_type"));
  auto value = static_cast<double>(tick_group->getAttribute("value"));
  auto label_pos = static_cast<double>(tick_group->parentElement()->getAttribute("label_pos"));
  auto pos = static_cast<double>(tick_group->parentElement()->getAttribute("pos"));
  auto location = static_cast<std::string>(tick_group->parentElement()->getAttribute("location"));

  // todo: window should come from axis if axis has window defined on it
  window_parent = tick_group->parentElement()->parentElement()->parentElement();
  if (strEqualsAny(location, "bottom", "left", "right", "top"))
    window_parent = tick_group->parentElement()->parentElement();
  window[0] = static_cast<double>(window_parent->getAttribute("window_x_min"));
  window[1] = static_cast<double>(window_parent->getAttribute("window_x_max"));
  window[2] = static_cast<double>(window_parent->getAttribute("window_y_min"));
  window[3] = static_cast<double>(window_parent->getAttribute("window_y_max"));

  adjustValueForNonStandardAxis(plot_parent, &value, location);

  if (tick_group->parentElement()->hasAttribute("scale"))
    scale = static_cast<int>(tick_group->parentElement()->getAttribute("scale"));
  if (plot_parent->hasAttribute("x_flip")) x_flip = static_cast<int>(plot_parent->getAttribute("x_flip"));
  if (plot_parent->hasAttribute("y_flip")) y_flip = static_cast<int>(plot_parent->getAttribute("y_flip"));
  if (tick_group->parentElement()->parentElement()->localName() == "colorbar")
    {
      if (tick_group->parentElement()->parentElement()->hasAttribute("x_flip"))
        x_flip = static_cast<int>(tick_group->parentElement()->parentElement()->getAttribute("x_flip"));
      if (tick_group->parentElement()->parentElement()->hasAttribute("y_flip"))
        y_flip = static_cast<int>(tick_group->parentElement()->parentElement()->getAttribute("y_flip"));
    }
  if (tick_group->parentElement()->hasAttribute("scientific_format"))
    scientific_format = static_cast<int>(tick_group->parentElement()->getAttribute("scientific_format"));
  if (tick_group->hasAttribute("scientific_format"))
    scientific_format = static_cast<int>(tick_group->getAttribute("scientific_format"));

  if (text.empty() && del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT) return;
  if (axis_type == "x")
    {
      available_height = 2.0; // Todo: change this number respecting the other objects
      available_width = 1.0;  // Todo: change this number respecting the other label
      x = value;
      y = label_pos;
    }
  else if (axis_type == "y")
    {
      available_height = 3.0; // Todo: change this number respecting the other label
      available_width = 1.0;  // Todo: change this number respecting the other objects
      x = label_pos;
      y = value;
    }
  if (aspect_ratio_ws_metric > 1)
    available_width *= 1.0 / (aspect_ratio_ws_metric);
  else
    available_width *= aspect_ratio_ws_metric;
  gr_wctondc(&x, &y);
  y_org = y;

  // get number of text children to figure out if children must be added or removed
  for (const auto &child : tick_group->children())
    {
      if (child->localName() == "text" && child->hasAttribute("_child_id")) cur_child_count += 1;
    }

  if (isNumber(text))
    {
      if (text.size() > 7)
        {
          char text_c[256];
          format_reference_t reference = {1, 1};
          const char minus[] = {(char)0xe2, (char)0x88, (char)0x92, '\0'}; // gr minus sign
          auto em_dash = std::string(minus);
          size_t start_pos = 0;

          gr_setscientificformat(scientific_format);

          if (startsWith(text, em_dash)) start_pos = em_dash.size();
          auto without_minus = text.substr(start_pos);

          snprintf(text_c, 256, "%s", without_minus.c_str());
          text = gr_ftoa(text_c, atof(without_minus.c_str()), &reference);
          if (start_pos != 0) text = em_dash + text;
        }
    }
  else
    {
      double tbx[4], tby[4];
      char text_c[256];
      snprintf(text_c, 256, "%s", text.c_str());
      gr_inqtext(x, y, text_c, tbx, tby);
      gr_wctondc(&tbx[0], &tby[0]);
      gr_wctondc(&tbx[1], &tby[1]);
      width = tbx[1] - tbx[0];
      tick_group->setAttribute("width", width);

      if (width / char_height > available_width)
        {
          int breakpoint_positions[128];
          int cur_num_breakpoints = 0;
          const char *label = text.c_str();
          for (i = 0; i == 0 || label[i - 1] != '\0'; ++i)
            {
              if (label[i] == ' ' || label[i] == '\0')
                {
                  /* calculate width of the next part of the label to be drawn */
                  new_label[i] = '\0';
                  gr_inqtext(x, y, new_label + cur_start, tbx, tby);
                  gr_wctondc(&tbx[0], &tby[0]);
                  gr_wctondc(&tbx[1], &tby[1]);
                  width = tbx[1] - tbx[0];
                  new_label[i] = ' ';

                  /* add possible breakpoint */
                  breakpoint_positions[cur_num_breakpoints++] = i;

                  if (width / char_height > available_width)
                    {
                      /* part is too big but doesn't have a breakpoint in it */
                      if (cur_num_breakpoints == 1)
                        {
                          new_label[i] = '\0';
                        }
                      else /* part is too big and has breakpoints in it */
                        {
                          /* break label at last breakpoint that still allowed the text to fit */
                          new_label[breakpoint_positions[cur_num_breakpoints - 2]] = '\0';
                        }

                      if ((del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT) ||
                          cur_child_count + child_id_org < child_id)
                        {
                          text_elem = global_creator->createText(x, y, new_label + cur_start, CoordinateSpace::NDC);
                          tick_group->append(text_elem);
                          text_elem->setAttribute("_child_id", child_id++);
                        }
                      else
                        {
                          text_elem = tick_group->querySelectors("text[_child_id=" + std::to_string(child_id++) + "]");
                          if (text_elem != nullptr)
                            text_elem = global_creator->createText(x, y, new_label + cur_start, CoordinateSpace::NDC,
                                                                   text_elem);
                        }
                      if (text_elem != nullptr)
                        {
                          if (!text_elem->hasAttribute("text_color_ind")) text_elem->setAttribute("text_color_ind", 1);
                          if (axis_type == "x")
                            {
                              int label_orientation = 0;
                              if (tick_group->parentElement()->hasAttribute("label_orientation"))
                                label_orientation =
                                    static_cast<int>(tick_group->parentElement()->getAttribute("label_orientation"));
                              text_elem->setAttribute("text_align_horizontal", GKS_K_TEXT_HALIGN_CENTER);
                              if (((pos <= 0.5 * (window[2] + window[3]) ||
                                    ((scale & GR_OPTION_FLIP_Y || y_flip) && pos > 0.5 * (window[2] + window[3]))) &&
                                   label_orientation == 0) ||
                                  label_orientation < 0)
                                {
                                  text_elem->setAttribute("text_align_vertical", GKS_K_TEXT_VALIGN_TOP);
                                }
                              else
                                {
                                  text_elem->setAttribute("text_align_vertical", GKS_K_TEXT_VALIGN_BOTTOM);
                                }
                            }
                          else if (axis_type == "y")
                            {
                              int label_orientation = 0;
                              if (tick_group->parentElement()->hasAttribute("label_orientation"))
                                label_orientation =
                                    static_cast<int>(tick_group->parentElement()->getAttribute("label_orientation"));
                              text_elem->setAttribute("text_align_vertical", GKS_K_TEXT_VALIGN_HALF);
                              if ((((pos <= 0.5 * (window[0] + window[1]) && !(scale & GR_OPTION_FLIP_X || x_flip)) ||
                                    ((scale & GR_OPTION_FLIP_X || x_flip) && pos > 0.5 * (window[0] + window[1]) &&
                                     tick_group->parentElement()->parentElement()->localName() != "colorbar")) &&
                                   label_orientation == 0) ||
                                  label_orientation < 0)
                                {
                                  text_elem->setAttribute("text_align_horizontal", GKS_K_TEXT_HALIGN_RIGHT);
                                }
                              else
                                {
                                  text_elem->setAttribute("text_align_horizontal", GKS_K_TEXT_HALIGN_LEFT);
                                }
                            }

                          text_elem->setAttribute("scientific_format", scientific_format);
                        }

                      if (cur_num_breakpoints == 1)
                        {
                          cur_start = i + 1;
                          cur_num_breakpoints = 0;
                        }
                      else
                        {
                          cur_start = breakpoint_positions[cur_num_breakpoints - 2] + 1;
                          breakpoint_positions[0] = breakpoint_positions[cur_num_breakpoints - 1];
                          cur_num_breakpoints = 1;
                        }
                      y -= 1.1 * char_height;

                      // if the available height is passed the rest of the label won't get displayed
                      if ((y_org - y) / char_height > available_height) break;
                    }
                }
              else
                {
                  new_label[i] = label[i];
                }
            }
          /* 0-terminate the new label */
          new_label[i] = '\0';

          text = new_label + cur_start;
          if (text.empty() && (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT))
            del = DelValues::UPDATE_WITHOUT_DEFAULT;
        }
      if (i >= cur_start && text != " " && text != "\0" && !text.empty() && cur_child_count + child_id_org < child_id)
        text_is_empty_or_number = false;
      if (i < cur_start && !text_is_empty_or_number) child_id_org += 1;
    }

  if ((del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT) || !text_is_empty_or_number)
    {
      text_elem = global_creator->createText(x, y, text, CoordinateSpace::NDC);
      text_elem->setAttribute("_child_id", child_id);
      tick_group->append(text_elem);
    }
  else
    {
      text_elem = tick_group->querySelectors("text[_child_id=" + std::to_string(child_id) + "]");
      if (text_elem != nullptr) text_elem = global_creator->createText(x, y, text, CoordinateSpace::NDC, text_elem);
    }
  // Todo: Maybe special case for colorbar so that the adjustment is depending on the location and not on the pos
  if ((text_elem != nullptr && del != DelValues::UPDATE_WITHOUT_DEFAULT) || !text_is_empty_or_number ||
      (text_elem != nullptr && tick_group->parentElement()->parentElement()->localName() == "colorbar"))
    {
      if (!text_elem->hasAttribute("text_color_ind")) text_elem->setAttribute("text_color_ind", 1);
      // set text align if not set by user
      if (!tick_group->hasAttribute("_text_align_vertical_set_by_user") &&
          !tick_group->hasAttribute("_text_align_horizontal_set_by_user"))
        {
          if (axis_type == "x")
            {
              int label_orientation = 0;
              if (tick_group->parentElement()->hasAttribute("label_orientation"))
                label_orientation = static_cast<int>(tick_group->parentElement()->getAttribute("label_orientation"));
              text_elem->setAttribute("text_align_horizontal", GKS_K_TEXT_HALIGN_CENTER);
              if ((((pos <= 0.5 * (window[2] + window[3]) && !(scale & GR_OPTION_FLIP_Y || y_flip)) ||
                    ((scale & GR_OPTION_FLIP_Y || y_flip) && pos > 0.5 * (window[2] + window[3]))) &&
                   label_orientation == 0) ||
                  label_orientation < 0)
                {
                  text_elem->setAttribute("text_align_vertical", GKS_K_TEXT_VALIGN_TOP);
                }
              else
                {
                  text_elem->setAttribute("text_align_vertical", GKS_K_TEXT_VALIGN_BOTTOM);
                }
            }
          else if (axis_type == "y")
            {
              int label_orientation = 0;
              if (tick_group->parentElement()->hasAttribute("label_orientation"))
                label_orientation = static_cast<int>(tick_group->parentElement()->getAttribute("label_orientation"));
              text_elem->setAttribute("text_align_vertical", GKS_K_TEXT_VALIGN_HALF);
              if ((((pos <= 0.5 * (window[0] + window[1]) && !(scale & GR_OPTION_FLIP_X || x_flip)) ||
                    ((scale & GR_OPTION_FLIP_X || x_flip) && pos > 0.5 * (window[0] + window[1]))) &&
                   label_orientation == 0) ||
                  label_orientation < 0)
                {
                  text_elem->setAttribute("text_align_horizontal", GKS_K_TEXT_HALIGN_RIGHT);
                }
              else
                {
                  text_elem->setAttribute("text_align_horizontal", GKS_K_TEXT_HALIGN_LEFT);
                }
            }
          if (tick_group->parentElement()->hasAttribute("char_up_x") &&
              tick_group->parentElement()->hasAttribute("char_up_y"))
            {
              auto char_up_x = static_cast<double>(tick_group->parentElement()->getAttribute("char_up_x"));
              auto char_up_y = static_cast<double>(tick_group->parentElement()->getAttribute("char_up_y"));
              if (!text_elem->hasAttribute("_char_up_x_set_by_user")) text_elem->setAttribute("char_up_x", char_up_x);
              if (!text_elem->hasAttribute("_char_up_y_set_by_user")) text_elem->setAttribute("char_up_y", char_up_y);
            }
        }
      text_elem->setAttribute("scientific_format", scientific_format);
    }

  // remove all text children with _child_id > child_id
  while (child_id < cur_child_count + child_id_org)
    {
      tick_group->querySelectors("text[_child_id=" + std::to_string(child_id++) + "]")->remove();
      if (child_id == cur_child_count + child_id_org && i >= cur_start &&
          tick_group->querySelectors("text[_child_id=" + std::to_string(child_id) + "]"))
        tick_group->querySelectors("text[_child_id=" + std::to_string(child_id) + "]")->remove();
    }
}

void applyTickModificationMap(const std::shared_ptr<GRM::Element> &tick_group,
                              const std::shared_ptr<GRM::Context> &context, int child_id, DelValues del)
{
  std::shared_ptr<GRM::Element> text_elem = nullptr;
  bool tick_group_attr_changed = false, old_automatic_update;

  auto render = grm_get_render();
  render->getAutoUpdate(&old_automatic_update);

  auto value = static_cast<double>(tick_group->getAttribute("value"));
  auto map_idx = static_cast<int>(tick_group->parentElement()->getAttribute("_axis_id"));
  for (const auto &child : tick_group->children())
    {
      if (child->localName() == "text")
        {
          text_elem = child;
          break;
        }
    }

  render->setAutoUpdate(false);
  if (tick_modification_map.find(map_idx) != tick_modification_map.end())
    {
      if (auto tick_value_to_map = tick_modification_map[map_idx];
          tick_value_to_map.find(value) != tick_value_to_map.end())
        {
          if (auto key_value_map = tick_value_to_map[value]; !key_value_map.empty())
            {
              for (auto const &[attr, val] : key_value_map)
                {
                  if (strEqualsAny(attr, "is_major", "line_color_ind", "line_spec", "line_width",
                                   "text_align_horizontal", "text_align_vertical", "tick_label", "tick_size", "value"))
                    {
                      tick_group->setAttribute(attr, val);
                      if (attr == "tick_label")
                        {
                          tick_group->setAttribute("tick_label", val);
                          tickLabelAdjustment(tick_group, child_id,
                                              text_elem != nullptr ? DelValues::UPDATE_WITHOUT_DEFAULT : del);
                        }
                      else if (strEqualsAny(attr, "is_major", "value"))
                        {
                          for (const auto &child : tick_group->children())
                            {
                              child->setAttribute(attr, val);
                            }
                        }
                      tick_group_attr_changed = true;
                    }
                  else if (strEqualsAny(attr, "font", "font_precision", "scientific_format", "text", "text_color_ind",
                                        "text_align_horizontal", "text_align_vertical", "x", "y"))
                    {
                      if (text_elem != nullptr) text_elem->setAttribute(attr, val);
                    }
                }
            }
          if (tick_group_attr_changed) processAttributes(tick_group);
        }
    }
  render->setAutoUpdate(old_automatic_update);
}

void kindDependentCoordinateLimAdjustments(const std::shared_ptr<GRM::Element> &element,
                                           const std::shared_ptr<GRM::Context> &context, double *min_component,
                                           double *max_component, std::string lim, std::string location)
{
  std::shared_ptr<GRM::Element> central_region;
  unsigned int series_count = 0;
  double x_min = DBL_MAX, x_max = -DBL_MAX, y_min = DBL_MAX, y_max = -DBL_MAX, z_min = DBL_MAX, z_max = -DBL_MAX;
  bool x_log = false, y_log = false;

  auto kind = static_cast<std::string>(element->getAttribute("_kind"));
  central_region = element->querySelectors("central_region");

  if (kind == "barplot")
    {
      for (const auto &series : central_region->children())
        {
          if (!startsWith(series->localName(), "series")) continue;
          series_count += 1;
        }
    }
  x_log = element->hasAttribute("x_log") && static_cast<int>(element->getAttribute("x_log"));
  y_log = element->hasAttribute("y_log") && static_cast<int>(element->getAttribute("y_log"));

  std::string orientation = PLOT_DEFAULT_ORIENTATION;
  if (kinds_classic_2d.count(kind) > 0)
    {
      if (central_region->hasAttribute("orientation"))
        orientation = static_cast<std::string>(central_region->getAttribute("orientation"));
      for (const auto &series : central_region->children())
        {
          if (!startsWith(series->localName(), "series")) continue;
          auto series_kind = static_cast<std::string>(series->getAttribute("kind"));
          if (kinds_classic_2d.count(series_kind) == 0) orientation = PLOT_DEFAULT_ORIENTATION;
        }
    }
  if (orientation == "vertical")
    {
      if (lim == "x_lim")
        lim = "y_lim";
      else if (lim == "y_lim")
        lim = "x_lim";
    }

  for (const auto &series : central_region->children())
    {
      if (!startsWith(series->localName(), "series_")) continue;
      std::string ref_x_axis_location = "x", ref_y_axis_location = "y";
      auto series_kind = static_cast<std::string>(series->getAttribute("kind"));
      if (series->hasAttribute("ref_x_axis_location"))
        ref_x_axis_location = static_cast<std::string>(series->getAttribute("ref_x_axis_location"));
      if (series->hasAttribute("ref_y_axis_location"))
        ref_y_axis_location = static_cast<std::string>(series->getAttribute("ref_y_axis_location"));
      if (lim == "x_lim" && ref_x_axis_location != location) continue;
      if (lim == "y_lim" && ref_y_axis_location != location) continue;

      kind = static_cast<std::string>(series->getAttribute("kind"));
      if (kind == "quiver")
        {
          /* For quiver plots use u^2 + v^2 as z value */
          double current_min_component = DBL_MAX, current_max_component = -DBL_MAX;
          if (!series->hasAttribute("z_range_min") || !series->hasAttribute("z_range_max"))
            {
              if (!series->hasAttribute("u"))
                throw NotFoundError("Quiver series is missing required attribute u-data.\n");
              auto u_key = static_cast<std::string>(series->getAttribute("u"));
              if (!series->hasAttribute("v"))
                throw NotFoundError("Quiver series is missing required attribute v-data.\n");
              auto v_key = static_cast<std::string>(series->getAttribute("v"));

              auto u = GRM::get<std::vector<double>>((*context)[u_key]);
              auto v = GRM::get<std::vector<double>>((*context)[v_key]);
              auto u_length = u.size();
              auto v_length = v.size();
              if (u_length != v_length)
                throw std::length_error("For quiver series the shape of u and v must be the same.\n");

              for (int i = 0; i < u_length; i++)
                {
                  double z = u[i] * u[i] + v[i] * v[i];
                  current_min_component = grm_min(z, current_min_component);
                  current_max_component = grm_max(z, current_max_component);
                }
              current_min_component = sqrt(current_min_component);
              current_max_component = sqrt(current_max_component);
            }
          else
            {
              current_min_component = static_cast<double>(series->getAttribute("z_range_min"));
              current_max_component = static_cast<double>(series->getAttribute("z_range_max"));
            }
          z_min = grm_min(current_min_component, z_min);
          z_max = grm_max(current_max_component, z_max);
        }
      else if (kind == "barplot")
        {
          std::string style;
          double xmin, xmax, ymin, ymax;

          if (series->hasAttribute("style")) style = static_cast<std::string>(series->getAttribute("style"));

          auto key = static_cast<std::string>(series->getAttribute("y"));
          auto y = GRM::get<std::vector<double>>((*context)[key]);
          auto len = static_cast<int>(y.size());
          if (series->hasAttribute("x_range_min") && series->hasAttribute("x_range_max"))
            {
              xmin = static_cast<double>(series->getAttribute("x_range_min"));
              xmax = static_cast<double>(series->getAttribute("x_range_max"));
              double step_x = (xmax - xmin) / (len - 1);
              if (!strEqualsAny(style, "lined", "stacked"))
                {
                  if (!x_log || (x_log && xmin - step_x > 0))
                    xmin -= step_x;
                  else if (x_log && xmin - step_x <= 0)
                    xmin = 1;
                  x_min = grm_min(x_min, xmin);
                  x_max = grm_max(x_max, xmax + step_x);
                }
              else
                {
                  if (x_min == DBL_MAX) x_min = xmin;
                  x_min = style == "stacked" ? xmin - series_count : grm_min(x_min, xmin - (x_max - 1));
                  if (x_max == -DBL_MAX) x_max = xmax;
                  x_max = style == "stacked" ? xmin + series_count : grm_max(x_max, xmin + (x_max - 1));
                }
            }
          else
            {
              x_min = x_log ? 1 : 0;
              x_max = strEqualsAny(style, "lined", "stacked") ? series_count + 1 : grm_max(len + 1, x_max);
            }

          if (series->hasAttribute("y_range_min") && series->hasAttribute("y_range_max"))
            {
              ymin = static_cast<double>(series->getAttribute("y_range_min"));
              ymax = static_cast<double>(series->getAttribute("y_range_max"));
              y_min = grm_min(y_min, ymin);
              if (style == "stacked")
                {
                  ymin = y_log ? 1 : 0;
                  for (int i = 0; i < len; i++)
                    {
                      if (y[i] < 0) ymin += y[i];
                    }
                  y_min = grm_min(y_min, ymin);
                  ymax = ymin;
                  for (int i = 0; i < len; i++)
                    {
                      ymax += (y_min < 0) ? fabs(y[i]) : y[i] - y_min;
                    }
                }
              y_max = grm_max(y_max, ymax);
            }
        }
      else if (kind == "histogram")
        {
          double current_y_min = 0.0, current_y_max = 0.0;

          if (!series->hasAttribute("bins")) histBins(series, context);
          auto bins_key = static_cast<std::string>(series->getAttribute("bins"));
          auto bins = GRM::get<std::vector<double>>((*context)[bins_key]);
          auto num_bins = static_cast<int>(bins.size());

          for (int i = 0; i < num_bins; i++)
            {
              current_y_min = grm_min(current_y_min, bins[i]);
              current_y_max = grm_max(current_y_max, bins[i]);
            }
          // y_max is a bit strange because it doesn't get affected by y_range, the histogram displays the sum of 1
          // or more y-values in each bar -> use the data-values along with the ranges to find the real max
          y_max = grm_max(current_y_max, y_max);
          if (series->hasAttribute("y_range_min") && series->hasAttribute("y_range_max"))
            {
              y_min = grm_min(y_min, static_cast<double>(series->getAttribute("y_range_min")));
              y_max = grm_max(y_max, static_cast<double>(series->getAttribute("y_range_max")));
            }
          else
            {
              y_min = grm_min(current_y_min, y_min);
            }
          if (series->hasAttribute("x_range_min") && series->hasAttribute("x_range_max"))
            {
              x_min = grm_min(x_min, static_cast<double>(series->getAttribute("x_range_min")));
              x_max = grm_max(x_max, static_cast<double>(series->getAttribute("x_range_max")));
            }
          else
            {
              x_min = 0.0;
              x_max = num_bins - 1;
            }
        }
      else if (strEqualsAny(kind, "stem", "stairs"))
        {
          if (series->hasAttribute("x_range_min") && series->hasAttribute("x_range_max"))
            {
              x_min = grm_min(x_min, static_cast<double>(series->getAttribute("x_range_min")));
              x_max = grm_max(x_max, static_cast<double>(series->getAttribute("x_range_max")));
            }
          if (series->hasAttribute("y_range_min") && series->hasAttribute("y_range_max"))
            {
              y_min = grm_min(y_min, static_cast<double>(series->getAttribute("y_range_min")));
              y_max = grm_max(y_max, static_cast<double>(series->getAttribute("y_range_max")));
            }
        }
    }

  if (x_min != DBL_MAX && x_max != -DBL_MAX && lim == "x_lim")
    {
      *min_component = x_min;
      *max_component = x_max;
    }
  if (y_min != DBL_MAX && y_max != -DBL_MAX && lim == "y_lim")
    {
      *min_component = y_min;
      *max_component = y_max;
    }
  if (z_min != DBL_MAX && z_max != -DBL_MAX && lim == "z_lim")
    {
      *min_component = z_min;
      *max_component = z_max;
    }
}

void calculateInitialCoordinateLims(const std::shared_ptr<GRM::Element> &element,
                                    const std::shared_ptr<GRM::Context> &context)
{
  std::string kind, style;
  const char *fmt;
  std::vector<std::string> data_component_names = {"x", "y", "z", "c", "r", "theta", ""};
  std::vector<std::string>::iterator current_component_name;
  std::vector<double> current_component;
  std::shared_ptr<GRM::Element> central_region;
  unsigned int current_point_count = 0;
  struct
  {
    const char *plot;
    const char *series;
  } * current_range_keys, range_keys[] = {{"x_lim", "x_range"}, {"y_lim", "y_range"}, {"z_lim", "z_range"},
                                          {"c_lim", "c_range"}, {"r_lim", "r_range"}, {"theta_lim", "theta_range"}};

  logger((stderr, "Storing coordinate ranges\n"));

  central_region = element->querySelectors("central_region");

  /* If a pan and/or zoom was performed before, do not overwrite limits
   * -> the user fully controls limits by interaction */
  if (element->hasAttribute("original_x_lim"))
    {
      logger((stderr, "Panzoom active, do not modify limits...\n"));
    }
  else
    {
      element->setAttribute("_x_lim_min", -1);
      element->setAttribute("_x_lim_max", 1);
      element->setAttribute("_y_lim_min", -1);
      element->setAttribute("_y_lim_max", 1);
      element->setAttribute("_z_lim_min", NAN);
      element->setAttribute("_z_lim_max", NAN);
      element->setAttribute("_c_lim_min", NAN);
      element->setAttribute("_c_lim_max", NAN);
      element->setAttribute("_r_lim_min", NAN);
      element->setAttribute("_r_lim_max", NAN);
      element->setAttribute("_theta_lim_min", NAN);
      element->setAttribute("_theta_lim_max", NAN);
      kind = static_cast<std::string>(element->getAttribute("_kind"));
      if (!stringMapAt(grm_fmt_map, kind.c_str(), &fmt))
        {
          std::stringstream ss;
          ss << "Invalid kind \"" << kind << "\" was given.";
          throw NotFoundError(ss.str());
        }
      if (!strEqualsAny(kind, "pie", "polar_histogram"))
        {
          current_component_name = data_component_names.begin();
          current_range_keys = range_keys;

          // TODO: Support mixed orientations
          std::string orientation = PLOT_DEFAULT_ORIENTATION;
          if (kinds_classic_2d.count(kind) > 0)
            {
              if (central_region->hasAttribute("orientation"))
                orientation = static_cast<std::string>(central_region->getAttribute("orientation"));
              for (const auto &series : central_region->children())
                {
                  if (!startsWith(series->localName(), "series")) continue;
                  auto series_kind = static_cast<std::string>(series->getAttribute("kind"));
                  if (kinds_classic_2d.count(series_kind) == 0) orientation = PLOT_DEFAULT_ORIENTATION;
                }
            }
          auto plot_type =
              static_cast<std::string>(central_region->querySelectors("coordinate_system")->getAttribute("plot_type"));

          while (!(*current_component_name).empty())
            {
              if (orientation == "vertical" && (*current_component_name) == "x")
                (*current_component_name) = "y";
              else if (orientation == "vertical" && (*current_component_name) == "y")
                (*current_component_name) = "x";
              std::list<std::string> location_names = {"tmp"};
              if (*current_component_name == "x" && plot_type == "2d")
                location_names = {"x", "twin_x", "top", "bottom"};
              else if (*current_component_name == "y" && plot_type == "2d")
                location_names = {"y", "twin_y", "right", "left"};

              for (const auto location : location_names)
                {
                  double min_component = DBL_MAX, max_component = -DBL_MAX, step = -DBL_MAX;
                  if (static_cast<std::string>(fmt).find(*current_component_name) != std::string::npos)
                    {
                      std::shared_ptr<GRM::Element> series_parent =
                          (kind == "marginal_heatmap") ? element : central_region;
                      for (const auto &series : series_parent->children())
                        {
                          std::string ref_x_axis_location = "x", ref_y_axis_location = "y";
                          double current_min_component = DBL_MAX, current_max_component = -DBL_MAX;
                          auto series_kind = static_cast<std::string>(series->getAttribute("kind"));
                          if (series->hasAttribute("ref_x_axis_location"))
                            ref_x_axis_location = static_cast<std::string>(series->getAttribute("ref_x_axis_location"));
                          if (series->hasAttribute("ref_y_axis_location"))
                            ref_y_axis_location = static_cast<std::string>(series->getAttribute("ref_y_axis_location"));
                          if (*current_component_name == "x" && (ref_x_axis_location != location && plot_type == "2d"))
                            continue;
                          if (*current_component_name == "y" && (ref_y_axis_location != location && plot_type == "2d"))
                            continue;

                          /* Heatmaps need calculated range keys, so run the calculation even if limits are given */
                          if (!element->hasAttribute(static_cast<std::string>(current_range_keys->plot) + "_min") ||
                              !element->hasAttribute(static_cast<std::string>(current_range_keys->plot) + "_max") ||
                              strEqualsAny(series_kind, "heatmap", "marginal_heatmap") ||
                              polar_kinds.count(series_kind) > 0)
                            {
                              if (!startsWith(series->localName(), "series") && central_region == series_parent)
                                continue;
                              if (series->hasAttribute("style"))
                                style = static_cast<std::string>(series->getAttribute("style"));

                              auto key = static_cast<std::string>(current_range_keys->series);
                              if (orientation == "vertical") key = (key == "x_range") ? "y_range" : "x_range";
                              if (!series->hasAttribute(key + "_min") || !series->hasAttribute(key + "_max"))
                                {
                                  if (series->hasAttribute(*current_component_name))
                                    {
                                      auto cntx_key =
                                          static_cast<std::string>(series->getAttribute(*current_component_name));
                                      current_component = GRM::get<std::vector<double>>((*context)[cntx_key]);
                                      current_point_count = (int)current_component.size();
                                      if (series_kind == "barplot")
                                        {
                                          current_min_component = 0.0;
                                          current_max_component = 0.0;
                                        }
                                      for (int i = 0; i < current_point_count; i++)
                                        {
                                          if (series_kind == "barplot" && style == "stacked")
                                            {
                                              if (current_component[i] > 0)
                                                current_max_component += current_component[i];
                                              else
                                                current_min_component += current_component[i];
                                            }
                                          else
                                            {
                                              if (!std::isnan(current_component[i]))
                                                {
                                                  current_min_component =
                                                      grm_min(current_component[i], current_min_component);
                                                  current_max_component =
                                                      grm_max(current_component[i], current_max_component);
                                                }
                                            }
                                        }
                                    }
                                  /* TODO: Add more plot types which can omit `x` */
                                  else if (series_kind == "line" && *current_component_name == "x")
                                    {
                                      if (!series->hasAttribute("y"))
                                        throw NotFoundError("Series is missing required attribute y.\n");
                                      auto cntx_key = static_cast<std::string>(series->getAttribute("y"));
                                      auto y_vec = GRM::get<std::vector<double>>((*context)[cntx_key]);
                                      auto y_length = y_vec.size();
                                      current_min_component = 0.0;
                                      current_max_component = y_length - 1;
                                    }
                                  else if (endsWith(series->localName(), series_kind) &&
                                           ((strEqualsAny(series_kind, "heatmap", "marginal_heatmap", "surface") &&
                                             strEqualsAny((*current_component_name), "x", "y")) ||
                                            (strEqualsAny(series_kind, "polar_heatmap", "nonuniform_polar_heatmap") &&
                                             strEqualsAny((*current_component_name), "theta", "r"))))
                                    {
                                      /* in this case `x`/`theta` or `y`/`r` (or both) are missing
                                       * -> set the current grm_min/max_component to the dimensions of `z`
                                       *    (shifted by half a unit to center color blocks) */
                                      const char *other_component_name;
                                      if (strEqualsAny(series_kind, "polar_heatmap", "nonuniform_polar_heatmap"))
                                        other_component_name = (*current_component_name == "theta") ? "r" : "theta";
                                      else
                                        other_component_name = (*current_component_name == "x") ? "y" : "x";
                                      if (series->hasAttribute(other_component_name))
                                        {
                                          /* The other component is given -> the missing dimension can be calculated */
                                          unsigned int z_length;

                                          auto cntx_key =
                                              static_cast<std::string>(series->getAttribute(other_component_name));
                                          auto other_component = GRM::get<std::vector<double>>((*context)[cntx_key]);
                                          auto other_point_count = other_component.size();

                                          if (!series->hasAttribute("z"))
                                            throw NotFoundError("Series is missing required attribute z.\n");
                                          auto z_key = static_cast<std::string>(series->getAttribute("z"));
                                          auto z_vec = GRM::get<std::vector<double>>((*context)[z_key]);
                                          z_length = z_vec.size();
                                          current_point_count = z_length / other_point_count;
                                        }
                                      else
                                        {
                                          /* A heatmap/surface without `x` and `y` values
                                           * -> dimensions can only be read from `z_dims` */
                                          int rows, cols;
                                          if (!series->hasAttribute("z_dims"))
                                            throw NotFoundError("Series is missing attribute z_dims.\n");
                                          auto z_dims_key = static_cast<std::string>(series->getAttribute("z_dims"));
                                          auto z_dims_vec = GRM::get<std::vector<int>>((*context)[z_dims_key]);
                                          cols = z_dims_vec[0];
                                          rows = z_dims_vec[1];
                                          if (strEqualsAny(series_kind, "polar_heatmap", "nonuniform_polar_heatmap"))
                                            current_point_count = (*current_component_name == "theta") ? cols : rows;
                                          else
                                            current_point_count = (*current_component_name == "x") ? cols : rows;
                                        }
                                      current_min_component = 0.5;
                                      current_max_component = current_point_count + 0.5;
                                    }
                                  else if (series->hasAttribute("indices"))
                                    {
                                      auto indices_key = static_cast<std::string>(series->getAttribute("indices"));
                                      auto indices = GRM::get<std::vector<int>>((*context)[indices_key]);

                                      if (series->hasAttribute(*current_component_name))
                                        {
                                          int index_sum = 0;
                                          auto cntx_key =
                                              static_cast<std::string>(series->getAttribute(*current_component_name));
                                          current_component = GRM::get<std::vector<double>>((*context)[cntx_key]);
                                          current_point_count = (int)current_component.size();

                                          current_max_component = 0;
                                          current_min_component = 0;
                                          auto act_index = indices.begin();
                                          index_sum += *act_index;
                                          for (int i = 0; i < current_point_count; i++)
                                            {
                                              if (!std::isnan(current_component[i]))
                                                {
                                                  if (current_component[i] > 0)
                                                    current_max_component += current_component[i];
                                                  else
                                                    current_min_component += current_component[i];
                                                }
                                              if (i + 1 == index_sum)
                                                {
                                                  max_component = grm_max(current_max_component, max_component);
                                                  min_component = grm_min(current_min_component, min_component);

                                                  current_max_component = 0;
                                                  current_min_component = 0;
                                                  ++act_index;
                                                  index_sum += *act_index;
                                                }
                                            }
                                        }
                                    }
                                }
                              else
                                {
                                  current_min_component = static_cast<double>(series->getAttribute(key + "_min"));
                                  current_max_component = static_cast<double>(series->getAttribute(key + "_max"));
                                }

                              if (current_min_component != DBL_MAX && current_max_component != -DBL_MAX)
                                {
                                  series->setAttribute(key + "_min", current_min_component);
                                  series->setAttribute(key + "_max", current_max_component);
                                }
                              min_component = grm_min(current_min_component, min_component);
                              max_component = grm_max(current_max_component, max_component);
                            }
                          if (series_kind == "quiver")
                            {
                              bool x_log = false, y_log = false;
                              if (element->hasAttribute("x_log"))
                                x_log = static_cast<int>(element->getAttribute("x_log"));
                              if (element->hasAttribute("y_log"))
                                y_log = static_cast<int>(element->getAttribute("y_log"));

                              step = grm_max(findMaxStep(current_point_count, current_component), step);
                              if (step > 0.0)
                                {
                                  current_min_component -= step;
                                  current_max_component += step;
                                }
                              if ((static_cast<std::string>(current_range_keys->plot) == "x_lim" && x_log) ||
                                  (static_cast<std::string>(current_range_keys->plot) == "y_lim" && y_log))
                                {
                                  current_min_component = (current_min_component > 0) ? current_min_component : 1;
                                  current_max_component =
                                      (current_max_component > 0) ? current_max_component : current_min_component + 1;
                                }
                              min_component = grm_min(current_min_component, min_component);
                              max_component = grm_max(current_max_component, max_component);
                            }
                        }
                    }

                  if (strEqualsAny(location, "x", "y") || plot_type != "2d" ||
                      !strEqualsAny(*current_component_name, "x", "y"))
                    {
                      if (strEqualsAny(kind, "imshow", "isosurface", "volume"))
                        {
                          min_component = (kind == "imshow" ? 0.0 : -1.0);
                          max_component = 1.0;
                        }
                    }

                  kindDependentCoordinateLimAdjustments(element, context, &min_component, &max_component,
                                                        static_cast<std::string>(current_range_keys->plot), location);

                  if (strEqualsAny(location, "x", "y") || plot_type != "2d" ||
                      !strEqualsAny(*current_component_name, "x", "y"))
                    {
                      if (element->hasAttribute(static_cast<std::string>(current_range_keys->plot) + "_min") &&
                          element->hasAttribute(static_cast<std::string>(current_range_keys->plot) + "_max"))
                        {
                          min_component = static_cast<double>(
                              element->getAttribute(static_cast<std::string>(current_range_keys->plot) + "_min"));
                          max_component = static_cast<double>(
                              element->getAttribute(static_cast<std::string>(current_range_keys->plot) + "_max"));
                        }
                      if (strEqualsAny(static_cast<std::string>(current_range_keys->plot), "x_lim", "y_lim"))
                        {
                          std::string l = (static_cast<std::string>(current_range_keys->plot) == "x_lim") ? "x" : "y";
                          if (auto axis = element->querySelectors("axis[location=\"" + l + "\"]");
                              axis != nullptr && axis->hasAttribute(l + "_lim_min") &&
                              axis->hasAttribute(l + "_lim_max") && kinds_3d.count(kind) == 0 &&
                              polar_kinds.count(kind) == 0)
                            {
                              min_component = static_cast<double>(axis->getAttribute(l + "_lim_min"));
                              max_component = static_cast<double>(axis->getAttribute(l + "_lim_max"));
                            }
                        }

                      if (polar_kinds.count(kind) > 0)
                        {
                          if (static_cast<std::string>(current_range_keys->plot) == "r_lim")
                            {
                              central_region->setAttribute("r_min", min_component);
                              central_region->setAttribute("r_max", max_component);
                            }
                          // this is needed for interactions replaces gr_setwindow(-1, 1, -1, 1);
                          if (strEqualsAny(static_cast<std::string>(current_range_keys->plot), "theta_lim", "r_lim"))
                            {
                              min_component = -1.0;
                              max_component = 1.0;
                            }
                        }
                      if (kind == "isosurface")
                        {
                          min_component = -1.0;
                          max_component = 1.0;
                        }
                    }

                  if (min_component != DBL_MAX && max_component != -DBL_MAX)
                    {
                      auto lim = static_cast<std::string>(current_range_keys->plot);
                      if (strEqualsAny(*current_component_name, "x", "y") && !strEqualsAny(location, "x", "y") &&
                          plot_type == "2d")
                        {
                          double a, b;
                          std::string real_location = location;
                          // calculate transformation from default window to extra axis window (a * w1 + b = w2)
                          auto lim_min = static_cast<double>(element->getAttribute("_" + lim + "_min"));
                          auto lim_max = static_cast<double>(element->getAttribute("_" + lim + "_max"));

                          calculateWindowTransformationParameter(element, lim_min, lim_max, min_component,
                                                                 max_component, location, &a, &b);

                          if (orientation == "vertical")
                            {
                              if (location == "twin_x") real_location = "twin_y";
                              if (location == "top") real_location = "right";
                              if (location == "bottom") real_location = "left";
                              if (location == "twin_y") real_location = "twin_x";
                              if (location == "right") real_location = "top";
                              if (location == "left") real_location = "bottom";
                            }
                          element->setAttribute("_" + real_location + "_window_xform_a_org", a);
                          element->setAttribute("_" + real_location + "_window_xform_a", a);
                          element->setAttribute("_" + real_location + "_window_xform_b_org", b);
                          element->setAttribute("_" + real_location + "_window_xform_b", b);
                        }
                      else
                        {
                          element->setAttribute("_" + lim + "_min", min_component);
                          element->setAttribute("_" + lim + "_max", max_component);
                        }
                    }
                }
              if (orientation == "vertical" && *current_component_name == "x")
                *current_component_name = "y";
              else if (orientation == "vertical" && *current_component_name == "y")
                *current_component_name = "x";
              ++current_range_keys;
              ++current_component_name;
            }
        }
    }
  // for resetting the side-axis in case of kind switch
  element->setAttribute("reset_ranges", true);
}

std::map<int, std::map<double, std::map<std::string, GRM::Value>>> *getTickModificationMap()
{
  return &tick_modification_map;
}

void applyCentralRegionDefaults(const std::shared_ptr<GRM::Element> &central_region)
{
  auto plot = central_region->parentElement();
  auto kind = static_cast<std::string>(plot->getAttribute("_kind"));
  bool overwrite = plot->hasAttribute("_overwrite_kind_dependent_defaults")
                       ? static_cast<int>(plot->getAttribute("_overwrite_kind_dependent_defaults"))
                       : false;

  if (!central_region->hasAttribute("resample_method"))
    central_region->setAttribute("resample_method", (int)PLOT_DEFAULT_RESAMPLE_METHOD);
  if (!central_region->hasAttribute("keep_window"))
    central_region->setAttribute("keep_window", PLOT_DEFAULT_KEEP_WINDOW);
  if ((!central_region->hasAttribute("space_3d_fov") || overwrite) && kinds_3d.count(kind) != 0)
    {
      if (strEqualsAny(kind, "wireframe", "surface", "line3", "scatter3", "trisurface", "volume"))
        {
          central_region->setAttribute("space_3d_fov", PLOT_DEFAULT_SPACE_3D_FOV);
        }
      else
        {
          central_region->setAttribute("space_3d_fov", 45.0);
        }
    }
  if ((!central_region->hasAttribute("space_3d_camera_distance") || overwrite) && kinds_3d.count(kind) != 0)
    {
      if (strEqualsAny(kind, "wireframe", "surface", "line3", "scatter3", "trisurface", "volume"))
        {
          central_region->setAttribute("space_3d_camera_distance", PLOT_DEFAULT_SPACE_3D_DISTANCE);
        }
      else
        {
          central_region->setAttribute("space_3d_camera_distance", 2.5);
        }
    }
}

void applyPlotDefaults(const std::shared_ptr<GRM::Element> &plot)
{
  if (!plot->hasAttribute("_kind")) plot->setAttribute("_kind", PLOT_DEFAULT_KIND);
  if (!plot->hasAttribute("keep_aspect_ratio")) plot->setAttribute("keep_aspect_ratio", PLOT_DEFAULT_KEEP_ASPECT_RATIO);
  if (!plot->hasAttribute("only_square_aspect_ratio"))
    plot->setAttribute("only_square_aspect_ratio", PLOT_DEFAULT_ONLY_SQUARE_ASPECT_RATIO);
  if (!plot->hasAttribute("viewport_normalized_x_min"))
    plot->setAttribute("viewport_normalized_x_min", PLOT_DEFAULT_VIEWPORT_NORMALIZED_MIN_X);
  if (!plot->hasAttribute("_viewport_normalized_x_min_org"))
    plot->setAttribute("_viewport_normalized_x_min_org", PLOT_DEFAULT_VIEWPORT_NORMALIZED_MIN_X);
  if (!plot->hasAttribute("viewport_normalized_x_max"))
    plot->setAttribute("viewport_normalized_x_max", PLOT_DEFAULT_VIEWPORT_NORMALIZED_MAX_X);
  if (!plot->hasAttribute("_viewport_normalized_x_max_org"))
    plot->setAttribute("_viewport_normalized_x_max_org", PLOT_DEFAULT_VIEWPORT_NORMALIZED_MAX_X);
  if (!plot->hasAttribute("viewport_normalized_y_min"))
    plot->setAttribute("viewport_normalized_y_min", PLOT_DEFAULT_VIEWPORT_NORMALIZED_MIN_Y);
  if (!plot->hasAttribute("_viewport_normalized_y_min_org"))
    plot->setAttribute("_viewport_normalized_y_min_org", PLOT_DEFAULT_VIEWPORT_NORMALIZED_MIN_Y);
  if (!plot->hasAttribute("viewport_normalized_y_max"))
    plot->setAttribute("viewport_normalized_y_max", PLOT_DEFAULT_VIEWPORT_NORMALIZED_MAX_Y);
  if (!plot->hasAttribute("_viewport_normalized_y_max_org"))
    plot->setAttribute("_viewport_normalized_y_max_org", PLOT_DEFAULT_VIEWPORT_NORMALIZED_MAX_Y);
  auto kind = static_cast<std::string>(plot->getAttribute("_kind"));
  bool overwrite = plot->hasAttribute("_overwrite_kind_dependent_defaults")
                       ? static_cast<int>(plot->getAttribute("_overwrite_kind_dependent_defaults"))
                       : false;
  if (!plot->hasAttribute("adjust_x_lim") || overwrite)
    {
      if (kind == "heatmap" || kind == "marginal_heatmap" || kind == "barplot")
        {
          plot->setAttribute("adjust_x_lim", 0);
        }
      else
        {
          plot->setAttribute("adjust_x_lim", (plot->hasAttribute("x_lim_min") ? 0 : PLOT_DEFAULT_ADJUST_XLIM));
        }
    }
  if (!plot->hasAttribute("adjust_y_lim") || overwrite)
    {
      if (kind == "heatmap" || kind == "marginal_heatmap")
        {
          plot->setAttribute("adjust_y_lim", 0);
        }
      else
        {
          if (polar_kinds.count(kind) > 0 || kind == "pie")
            {
              plot->setAttribute("adjust_y_lim", PLOT_DEFAULT_ADJUST_YLIM);
            }
          else
            {
              plot->setAttribute("adjust_y_lim", (plot->hasAttribute("y_lim_min") ? 0 : PLOT_DEFAULT_ADJUST_YLIM));
            }
        }
    }
  if (!plot->hasAttribute("adjust_z_lim") || overwrite)
    {
      if (kind != "heatmap" && kind != "marginal_heatmap")
        {
          plot->setAttribute("adjust_z_lim", (plot->hasAttribute("z_lim_min") ? 0 : PLOT_DEFAULT_ADJUST_ZLIM));
        }
    }
  if (!plot->hasAttribute("line_spec")) plot->setAttribute("line_spec", " ");
  if (!plot->hasAttribute("x_log")) plot->setAttribute("x_log", PLOT_DEFAULT_XLOG);
  if (!plot->hasAttribute("y_log")) plot->setAttribute("y_log", PLOT_DEFAULT_YLOG);
  if (!plot->hasAttribute("z_log")) plot->setAttribute("z_log", PLOT_DEFAULT_ZLOG);
  if (!plot->hasAttribute("x_flip")) plot->setAttribute("x_flip", PLOT_DEFAULT_XFLIP);
  if (!plot->hasAttribute("y_flip")) plot->setAttribute("y_flip", PLOT_DEFAULT_YFLIP);
  if (!plot->hasAttribute("z_flip")) plot->setAttribute("z_flip", PLOT_DEFAULT_ZFLIP);
  if (!plot->hasAttribute("font")) plot->setAttribute("font", PLOT_DEFAULT_FONT);
  if (!plot->hasAttribute("font_precision")) plot->setAttribute("font_precision", PLOT_DEFAULT_FONT_PRECISION);
  if (!plot->hasAttribute("colormap")) plot->setAttribute("colormap", PLOT_DEFAULT_COLORMAP);

  auto central_region_parent = plot;
  if (kind == "marginal_heatmap") central_region_parent = plot->children()[0];
  for (const auto &child : central_region_parent->children())
    {
      if (child->localName() == "central_region")
        {
          applyCentralRegionDefaults(child);
          break;
        }
    }
}

void applyPlotDefaultsHelper(const std::shared_ptr<GRM::Element> &element)
{
  if (element->localName() == "layout_grid_element")
    {
      for (const auto &child : element->children())
        {
          if (child->localName() == "plot") applyPlotDefaults(child);
        }
    }
  if (element->localName() == "layout_grid")
    {
      for (const auto &child : element->children())
        {
          applyPlotDefaultsHelper(child);
        }
    }
}

void applyRootDefaults(const std::shared_ptr<GRM::Element> &root)
{
  if (!root->hasAttribute("_clear_ws")) root->setAttribute("_clear_ws", PLOT_DEFAULT_CLEAR);
  if (!root->hasAttribute("_update_ws")) root->setAttribute("_update_ws", PLOT_DEFAULT_UPDATE);
  if (!root->hasAttribute("_modified")) root->setAttribute("_modified", false);

  for (const auto &figure : root->children())
    {
      if (figure->localName() == "figure")
        {
          if (!figure->hasAttribute("size_x"))
            {
              figure->setAttribute("size_x", PLOT_DEFAULT_WIDTH);
              figure->setAttribute("size_x_type", "double");
              figure->setAttribute("size_x_unit", "px");
            }
          if (!figure->hasAttribute("size_y"))
            {
              figure->setAttribute("size_y", PLOT_DEFAULT_HEIGHT);
              figure->setAttribute("size_y_type", "double");
              figure->setAttribute("size_y_unit", "px");
            }

          for (const auto &child : figure->children())
            {
              if (child->localName() == "plot") applyPlotDefaults(child);
              if (child->localName() == "layout_grid") applyPlotDefaultsHelper(child);
            }
        }
    }
}
