#include "grm/plot.h"
#include <grm/dom_render/process_attributes.hxx>
#include <grm/dom_render/process_elements.hxx>
#include <grm/dom_render/render.hxx>
#include <grm/dom_render/render_util.hxx>
#include <grm/util_int.h>
#include <grm/dom_render/casts.hxx>
#include <grm/event_int.h>
#include <grm/plot_int.h>

void processAttributes(const std::shared_ptr<GRM::Element> &element)
{
  /*!
   * processing function for all kinds of attributes
   *
   * \param[in] element The GRM::Element containing attributes
   */
  // Map used for processing all kinds of attributes
  static std::map<std::string, std::function<void(const std::shared_ptr<GRM::Element> &)>> attr_string_to_func{
      {std::string("background_color"), processBackgroundColor},
      {std::string("border_color_ind"), processBorderColorInd},
      {std::string("border_width"), processBorderWidth},
      {std::string("marginal_heatmap_side_plot"), processMarginalHeatmapSidePlot},
      {std::string("char_expan"), processCharExpan},
      {std::string("char_space"), processCharSpace},
      {std::string("char_up_x"), processCharUp}, // the x element can be used cause both must be set
      {std::string("clip_region"), processClipRegion},
      {std::string("colormap"), processColormap},
      {std::string("fill_color_ind"), processFillColorInd},
      {std::string("fill_int_style"), processFillIntStyle},
      {std::string("fill_style"), processFillStyle},
      {std::string("font"), processFont},
      {std::string("line_color_ind"), processLineColorInd},
      {std::string("line_spec"), processLineSpec},
      {std::string("line_type"), processLineType},
      {std::string("line_width"), processLineWidth},
      {std::string("marginal_heatmap_kind"), processMarginalHeatmapKind},
      {std::string("marker_color_ind"), processMarkerColorInd},
      {std::string("marker_size"), processMarkerSize},
      {std::string("marker_type"), processMarkerType},
      {std::string("ref_x_axis_location"), processRefAxisLocation},
      {std::string("ref_y_axis_location"), processRefAxisLocation},
      {std::string("resample_method"), processResampleMethod},
      {std::string("reset_rotation"), processResetRotation},
      {std::string("select_specific_xform"), processSelectSpecificXform},
      {std::string("space_tilt"), processSpace},
      {std::string("space_3d_fov"), processSpace3d},          // the fov element can be used cause both must be set
      {std::string("text_align_vertical"), processTextAlign}, // the alignment in both directions is set
      {std::string("text_color_ind"), processTextColorInd},
      {std::string("text_encoding"), processTextEncoding},
      {std::string("viewport"), processViewport},
      {std::string("ws_viewport_x_min"),
       processWSViewport},                               // the xmin element can be used here cause all 4 are required
      {std::string("ws_window_x_min"), processWSWindow}, // the xmin element can be used here cause all 4 are required
      {std::string("x_flip"), processFlip},              // y_flip is also set
      {std::string("z_index"), processZIndex},
  };

  static std::map<std::string, std::function<void(const std::shared_ptr<GRM::Element> &)>> attr_string_to_func_post{
      /* This map contains functions for attributes that should be called after some attributes have been processed
       * already. These functions can contain e.g. inquire function calls for colors.
       * */
      {std::string("transparency"), processTransparency},
      {std::string("set_text_color_for_background"), processTextColorForBackground},
  };

  static std::map<std::string, std::function<void(const std::shared_ptr<GRM::Element> &, const std::string attribute)>>
      multi_attr_string_to_func{
          /* This map contains functions for attributes of which an element can hold more than one e.g. colorrep */
          {std::string("colorrep"), processColorRep},
      };

  for (const auto &attribute : element->getAttributeNames())
    {
      if (auto end = attribute.find('.');
          end != std::string::npos) /* element can hold more than one attribute of this kind */
        {
          if (auto attribute_kind = attribute.substr(0U, end);
              multi_attr_string_to_func.find(attribute_kind) != multi_attr_string_to_func.end())
            multi_attr_string_to_func[attribute_kind](element, attribute);
        }
      else if (attr_string_to_func.find(attribute) != attr_string_to_func.end())
        {
          attr_string_to_func[attribute](element);
        }
    }

  for (auto &attribute : element->getAttributeNames()) // Post process attribute run
    {
      if (attr_string_to_func_post.find(attribute) != attr_string_to_func_post.end())
        attr_string_to_func_post[attribute](element);
    }
}

void calculateCentralRegionMarginOrDiagFactor(const std::shared_ptr<GRM::Element> &element, double *vp_x_min,
                                              double *vp_x_max, double *vp_y_min, double *vp_y_max, bool diag_factor)
{
  bool left_text_margin = false, right_text_margin = false, bottom_text_margin = false, top_text_margin = false;
  bool top_text_is_title = false;
  std::string kind;
  bool keep_aspect_ratio, uniform_data = true, only_square_aspect_ratio = false;
  double metric_width, metric_height;
  double aspect_ratio_ws, start_aspect_ratio_ws;
  double vp0, vp1, vp2, vp3;
  double left_margin = 0.0, right_margin = 0.0, bottom_margin = 0.0, top_margin = 0.0;
  double viewport[4] = {0.0, 0.0, 0.0, 0.0};
  double side_region_size_x = 0.1, side_region_size_y = 0.1;
  std::shared_ptr<GRM::Element> plot_parent = element, left_side_region, right_side_region, bottom_side_region,
                                top_side_region;

  auto render = grm_get_render();
  getPlotParent(plot_parent);

  kind = static_cast<std::string>(plot_parent->getAttribute("_kind"));
  keep_aspect_ratio = static_cast<int>(plot_parent->getAttribute("keep_aspect_ratio"));
  only_square_aspect_ratio = static_cast<int>(plot_parent->getAttribute("only_square_aspect_ratio"));

  left_side_region = plot_parent->querySelectors("side_region[location=\"left\"]");
  right_side_region = plot_parent->querySelectors("side_region[location=\"right\"]");
  bottom_side_region = plot_parent->querySelectors("side_region[location=\"bottom\"]");
  top_side_region = plot_parent->querySelectors("side_region[location=\"top\"]");
  if (left_side_region && left_side_region->hasAttribute("text_content")) left_text_margin = true;
  if (right_side_region && right_side_region->hasAttribute("text_content")) right_text_margin = true;
  if (bottom_side_region && bottom_side_region->hasAttribute("text_content")) bottom_text_margin = true;
  if (top_side_region && top_side_region->hasAttribute("text_content")) top_text_margin = true;
  if (top_side_region && top_side_region->hasAttribute("text_is_title"))
    top_text_is_title = top_text_margin && static_cast<int>(top_side_region->getAttribute("text_is_title"));

  for (const auto &series : element->children())
    {
      if (!startsWith(series->localName(), "series_")) continue;
      uniform_data = isUniformData(series, render->getContext());
      if (!uniform_data) break;
    }
  if (kind == "marginal_heatmap" && uniform_data)
    uniform_data = isUniformData(element->parentElement(), render->getContext());

  GRM::getFigureSize(nullptr, nullptr, &metric_width, &metric_height);
  aspect_ratio_ws = metric_width / metric_height;
  start_aspect_ratio_ws = static_cast<double>(plot_parent->getAttribute("_start_aspect_ratio"));
  if (plot_parent->parentElement()->localName() == "layout_grid_element")
    {
      double figure_viewport[4];
      auto figure_vp_element = plot_parent->parentElement();
      figure_viewport[0] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_x_min_org"));
      figure_viewport[1] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_x_max_org"));
      figure_viewport[2] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_y_min_org"));
      figure_viewport[3] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_y_max_org"));

      metric_width *= (figure_viewport[1] - figure_viewport[0]);
      metric_height *= (figure_viewport[3] - figure_viewport[2]);
      aspect_ratio_ws = metric_width / metric_height;
    }
  if (keep_aspect_ratio && (!only_square_aspect_ratio || (only_square_aspect_ratio && !uniform_data)) && !diag_factor &&
      kind != "imshow" && kinds_3d.count(kind) == 0)
    {
      if (aspect_ratio_ws > start_aspect_ratio_ws)
        {
          auto x_min = *vp_x_min * (start_aspect_ratio_ws / aspect_ratio_ws);
          auto x_max = *vp_x_max * (start_aspect_ratio_ws / aspect_ratio_ws);
          auto diff = 0.5 * ((*vp_x_max - *vp_x_min) - (x_max - x_min));
          *vp_x_min += diff;
          *vp_x_max -= diff;
        }
      else
        {
          auto y_min = *vp_y_min / (start_aspect_ratio_ws / aspect_ratio_ws);
          auto y_max = *vp_y_max / (start_aspect_ratio_ws / aspect_ratio_ws);
          auto diff = 0.5 * ((*vp_y_max - *vp_y_min) - (y_max - y_min));
          *vp_y_min += diff;
          *vp_y_max -= diff;
        }
    }
  else if (keep_aspect_ratio && uniform_data && only_square_aspect_ratio && !diag_factor && kind != "imshow" &&
           kinds_3d.count(kind) == 0)
    {
      if (aspect_ratio_ws > 1)
        {
          double border = 0.5 * (*vp_x_max - *vp_x_min) * (1.0 - 1.0 / aspect_ratio_ws);
          *vp_x_min += border;
          *vp_x_max -= border;
        }
      else if (aspect_ratio_ws <= 1)
        {
          double border = 0.5 * (*vp_y_max - *vp_y_min) * (1.0 - aspect_ratio_ws);
          *vp_y_min += border;
          *vp_y_max -= border;
        }
    }

  // margin respects colorbar and side plot in the specific side_region
  // TODO: respect individual size defined by user
  if (kind == "marginal_heatmap")
    {
      side_region_size_x = 0.075;
      side_region_size_y = 0.075;
    }
  else if (kinds_3d.count(kind) > 0)
    {
      side_region_size_x = 0.2 * (*vp_x_max - *vp_x_min);
      side_region_size_y = 0.2 * (*vp_y_max - *vp_y_min);
    }
  sidePlotMargin(left_side_region, &left_margin, side_region_size_x);
  sidePlotMargin(right_side_region, &right_margin, side_region_size_x);
  sidePlotMargin(bottom_side_region, &bottom_margin, side_region_size_y);
  sidePlotMargin(top_side_region, &top_margin, side_region_size_y);

  if (kinds_3d.count(kind) > 0)
    {
      *vp_x_max -= right_margin;
      *vp_x_min += left_margin;
      *vp_y_max -= top_margin;
      *vp_y_min += bottom_margin;
      left_margin = right_margin = bottom_margin = top_margin = 0.0;

      auto extent = grm_min(*vp_x_max - *vp_x_min, *vp_y_max - *vp_y_min);
      vp0 = 0.5 * (*vp_x_min + *vp_x_max - extent);
      vp1 = 0.5 * (*vp_x_min + *vp_x_max + extent);
      vp2 = 0.5 * (*vp_y_min + *vp_y_max - extent);
      vp3 = 0.5 * (*vp_y_min + *vp_y_max + extent);
      if (!diag_factor) element->setAttribute("_vp_with_extent", vp1 - vp0);
    }
  else
    {
      vp0 = *vp_x_min;
      vp1 = *vp_x_max;
      vp2 = *vp_y_min;
      vp3 = *vp_y_max;
    }

  if (!top_text_is_title && top_margin != 0) top_margin += 0.05;
  if (bottom_margin != 0) bottom_margin += 0.05;

  // in the non keep_aspect_ratio case the viewport vp0 - vp3 can be too small for the resulting side_plot; use a
  // predefined maximum in these cases
  if (kind != "marginal_heatmap" && !keep_aspect_ratio && kinds_3d.count(kind) == 0)
    {
      capSidePlotMarginInNonKeepAspectRatio(left_side_region, &left_margin, kind);
      capSidePlotMarginInNonKeepAspectRatio(right_side_region, &right_margin, kind);
      capSidePlotMarginInNonKeepAspectRatio(bottom_side_region, &bottom_margin, kind);
      capSidePlotMarginInNonKeepAspectRatio(top_side_region, &top_margin, kind);
    }

  // margin respects text in the specific side_region
  if (left_text_margin) left_margin += 0.05;
  if (right_text_margin) right_margin += 0.05;
  if (bottom_text_margin) bottom_margin += 0.05;

  if (plot_parent->hasAttribute("_twin_y_window_xform_a_org")) right_margin += 0.025;
  if (plot_parent->hasAttribute("_twin_x_window_xform_a_org")) top_margin += 0.025;

  // calculate text impact for top_margin and adjust all margins if defined by attributes
  if (kind == "marginal_heatmap")
    {
      top_margin += (right_margin - top_margin) + (top_text_margin ? top_text_is_title ? 0.1 : 0.075 : 0.025);

      if (keep_aspect_ratio && uniform_data && only_square_aspect_ratio)
        {
          if (bottom_margin != left_margin)
            {
              bottom_margin = grm_max(left_margin, bottom_margin);
              left_margin = bottom_margin;
            }
          if (right_margin > top_margin)
            {
              top_margin += (0.975 - top_margin) - (0.95 - right_margin);
            }
          else
            {
              right_margin += (0.95 - right_margin) - (0.975 - top_margin);
            }
        }
    }
  else
    {
      top_margin += (top_text_margin ? top_text_is_title ? 0.075 : 0.05 : 0.0);
      if (keep_aspect_ratio && uniform_data && only_square_aspect_ratio)
        {
          if (bottom_margin != left_margin)
            {
              bottom_margin = grm_max(left_margin, bottom_margin);
              left_margin = bottom_margin;
            }
          right_margin += top_margin;
          if (right_margin > top_margin)
            {
              auto diff = (0.975 - top_margin) - (0.95 - right_margin);
              top_margin += 0.5 * diff;
              bottom_margin += 0.5 * diff;
            }
          else
            {
              auto diff = (0.95 - right_margin) - (0.975 - top_margin);
              right_margin += 0.5 * diff;
              left_margin += 0.5 * diff;
            }
        }
    }

  if (kind == "imshow")
    {
      double w, h, x_min, x_max, y_min, y_max;
      auto context = render->getContext();
      auto imshow_series = element->querySelectors("series_imshow");
      if (!imshow_series->hasAttribute("z_dims"))
        throw NotFoundError("Imshow series is missing required attribute z_dims-data.\n");
      auto z_dims_key = static_cast<std::string>(imshow_series->getAttribute("z_dims"));
      auto z_dims_vec = GRM::get<std::vector<int>>((*context)[z_dims_key]);

      h = static_cast<double>(z_dims_vec[1]) / static_cast<double>(z_dims_vec[0]) * (vp1 - vp0);
      w = static_cast<double>(z_dims_vec[0]) / static_cast<double>(z_dims_vec[1]) * (vp3 - vp2);

      x_min = grm_max(0.5 * (vp0 + vp1 - w), vp0);
      x_max = grm_min(0.5 * (vp0 + vp1 + w), vp1);
      y_min = grm_max(0.5 * (vp3 + vp2 - h), vp2);
      y_max = grm_min(0.5 * (vp3 + vp2 + h), vp3);

      left_margin = (x_min == vp0) ? 0.0 : (x_min - vp0) / (vp1 - vp0);
      right_margin = (x_max == vp1) ? -0.0 : 1.0 - (x_max - vp0) / (vp1 - vp0);
      bottom_margin = (y_min == vp2) ? -0.0 : (y_min - vp2) / (vp3 - vp2);
      top_margin = (y_max == vp3) ? -0.0 : 1.0 - (y_max - vp2) / (vp3 - vp2);
    }

  viewport[0] = vp0 + left_margin * (vp1 - vp0);
  viewport[1] = vp0 + (1.0 - right_margin) * (vp1 - vp0);
  viewport[2] = vp2 + bottom_margin * (vp3 - vp2);
  viewport[3] = vp2 + (1.0 - top_margin) * (vp3 - vp2);

  if ((polar_kinds.count(kind) > 0 || kind == "pie") && !diag_factor)
    {
      element->setAttribute("_before_centering_polar_vp_x_min", viewport[0]);
      element->setAttribute("_before_centering_polar_vp_x_max", viewport[1]);
      element->setAttribute("_before_centering_polar_vp_y_min", viewport[2]);
      element->setAttribute("_before_centering_polar_vp_y_max", viewport[3]);
    }

  if (kind == "pie" && diag_factor)
    {
      viewport[2] += CENTRAL_REGION_VP_AXIS_MARGIN_PIE * (vp3 - vp2); // for legend; legend is a side_region somehow
    }
  else if (kind == "pie" && !diag_factor) // just for visual bbox
    {
      viewport[2] += CENTRAL_REGION_VP_AXIS_MARGIN_PIE_BBOX * (vp3 - vp2);
    }
  else if (polar_kinds.count(kind) > 0 && diag_factor)
    {
      viewport[0] += CENTRAL_REGION_VP_AXIS_MARGIN_POLAR * (vp1 - vp0);
      viewport[1] -= CENTRAL_REGION_VP_AXIS_MARGIN_POLAR * (vp1 - vp0);
      viewport[2] += CENTRAL_REGION_VP_AXIS_MARGIN_POLAR * (vp3 - vp2);
      viewport[3] -= CENTRAL_REGION_VP_AXIS_MARGIN_POLAR * (vp3 - vp2);
    }
  else if (polar_kinds.count(kind) > 0 && !diag_factor) // just for visual bbox
    {
      viewport[0] -= CENTRAL_REGION_VP_AXIS_MARGIN_POLAR_BBOX * (vp1 - vp0);
      viewport[1] += CENTRAL_REGION_VP_AXIS_MARGIN_POLAR_BBOX * (vp1 - vp0);
      viewport[2] -= CENTRAL_REGION_VP_AXIS_MARGIN_POLAR_BBOX * (vp3 - vp2);
      viewport[3] += CENTRAL_REGION_VP_AXIS_MARGIN_POLAR_BBOX * (vp3 - vp2);
    }
  else if (kinds_3d.count(kind) > 0)
    {
      viewport[0] += CENTRAL_REGION_X_MIN_VP_AXIS_MARGIN_3D * (vp1 - vp0);
      viewport[1] -= CENTRAL_REGION_X_MAX_VP_AXIS_MARGIN_3D * (vp1 - vp0);
      viewport[2] += CENTRAL_REGION_Y_MIN_VP_AXIS_MARGIN_3D * (vp3 - vp2);
      viewport[3] -= CENTRAL_REGION_Y_MAX_VP_AXIS_MARGIN_3D * (vp3 - vp2);
    }
  else if (kind != "imshow" && diag_factor)
    {
      viewport[0] += CENTRAL_REGION_X_MIN_VP_AXIS_MARGIN * (vp1 - vp0);
      viewport[1] -= CENTRAL_REGION_X_MAX_VP_AXIS_MARGIN * (vp1 - vp0);
      viewport[2] += CENTRAL_REGION_Y_MIN_VP_AXIS_MARGIN * (vp3 - vp2);
      viewport[3] -= CENTRAL_REGION_Y_MAX_VP_AXIS_MARGIN * (vp3 - vp2);
    }
  if (polar_kinds.count(kind) > 0 && !diag_factor)
    {
      element->setAttribute("_left_axis_border", CENTRAL_REGION_VP_AXIS_MARGIN_POLAR * (vp1 - vp0));
      element->setAttribute("_right_axis_border", CENTRAL_REGION_VP_AXIS_MARGIN_POLAR * (vp1 - vp0));
      element->setAttribute("_bottom_axis_border", CENTRAL_REGION_VP_AXIS_MARGIN_POLAR * (vp3 - vp2));
      element->setAttribute("_top_axis_border", CENTRAL_REGION_VP_AXIS_MARGIN_POLAR * (vp3 - vp2));
    }
  else if (kind == "pie" && !diag_factor)
    {
      element->setAttribute("_left_axis_border", 0);
      element->setAttribute("_right_axis_border", 0);
      element->setAttribute("_bottom_axis_border", CENTRAL_REGION_VP_AXIS_MARGIN_PIE * (vp3 - vp2));
      element->setAttribute("_top_axis_border", 0);
    }
  else if (kind != "imshow" && !diag_factor)
    {
      element->setAttribute("_left_axis_border", CENTRAL_REGION_X_MIN_VP_AXIS_MARGIN * (vp1 - vp0));
      element->setAttribute("_right_axis_border", CENTRAL_REGION_X_MAX_VP_AXIS_MARGIN * (vp1 - vp0));
      element->setAttribute("_bottom_axis_border", CENTRAL_REGION_Y_MIN_VP_AXIS_MARGIN * (vp3 - vp2));
      element->setAttribute("_top_axis_border", CENTRAL_REGION_Y_MAX_VP_AXIS_MARGIN * (vp3 - vp2));
    }

  if (strEqualsAny(kind, "line", "stairs", "scatter", "stem", "line3", "scatter3"))
    {
      double w, h;
      int location = PLOT_DEFAULT_LOCATION;
      if (element->hasAttribute("location"))
        {
          if (element->getAttribute("location").isInt())
            {
              location = static_cast<int>(element->getAttribute("location"));
            }
          else if (element->getAttribute("location").isString())
            {
              location = GRM::locationStringToInt(static_cast<std::string>(element->getAttribute("location")));
            }
        }
      else
        {
          element->setAttribute("location", location);
        }

      if (location == 11 || location == 12 || location == 13)
        {
          legendSize(element, &w, &h);
          viewport[1] -= w + 0.1;
        }
    }

  if (kind == "pie" || polar_kinds.count(kind) > 0)
    {
      double x_center, y_center, r;

      x_center = 0.5 * (viewport[0] + viewport[1]);
      y_center = 0.5 * (viewport[2] + viewport[3]);
      r = 0.45 * grm_min(viewport[1] - viewport[0], viewport[3] - viewport[2]);
      if (top_text_margin)
        {
          r *= 0.975;
          y_center -= 0.025 * r;
        }

      viewport[0] = x_center - r;
      viewport[1] = x_center + r;
      viewport[2] = y_center - r;
      viewport[3] = y_center + r;
    }
  *vp_x_min = viewport[0];
  *vp_x_max = viewport[1];
  *vp_y_min = viewport[2];
  *vp_y_max = viewport[3];
}

void calculateViewport(const std::shared_ptr<GRM::Element> &element)
{
  auto render = grm_get_render();
  auto active_figure = render->getActiveFigure();

  if (element->localName() == "central_region")
    {
      double vp[4];
      std::shared_ptr<GRM::Element> plot_parent = element;
      getPlotParent(plot_parent);

      if (!GRM::Render::getViewport(element->parentElement(), &vp[0], &vp[1], &vp[2], &vp[3]))
        throw NotFoundError(element->parentElement()->localName() + " doesn't have a viewport but it should.\n");

      calculateCentralRegionMarginOrDiagFactor(element, &vp[0], &vp[1], &vp[2], &vp[3], false);

      render->setViewport(element, vp[0], vp[1], vp[2], vp[3]);
      element->setAttribute("_viewport_x_min_org", vp[0]);
      element->setAttribute("_viewport_x_max_org", vp[1]);
      element->setAttribute("_viewport_y_min_org", vp[2]);
      element->setAttribute("_viewport_y_max_org", vp[3]);
    }
  else if (element->localName() == "plot")
    {
      double vp[4];
      double metric_width, metric_height;
      double aspect_ratio_ws;

      /* when grids are being used for layouting the plot information is stored in the parent of the plot */
      if (element->parentElement()->localName() == "layout_grid_element")
        {
          vp[0] = static_cast<double>(element->parentElement()->getAttribute("_viewport_normalized_x_min_org"));
          vp[1] = static_cast<double>(element->parentElement()->getAttribute("_viewport_normalized_x_max_org"));
          vp[2] = static_cast<double>(element->parentElement()->getAttribute("_viewport_normalized_y_min_org"));
          vp[3] = static_cast<double>(element->parentElement()->getAttribute("_viewport_normalized_y_max_org"));
        }
      else
        {
          vp[0] = static_cast<double>(element->getAttribute("_viewport_normalized_x_min_org"));
          vp[1] = static_cast<double>(element->getAttribute("_viewport_normalized_x_max_org"));
          vp[2] = static_cast<double>(element->getAttribute("_viewport_normalized_y_min_org"));
          vp[3] = static_cast<double>(element->getAttribute("_viewport_normalized_y_max_org"));
        }

      GRM::getFigureSize(nullptr, nullptr, &metric_width, &metric_height);
      aspect_ratio_ws = metric_width / metric_height;
      // plot viewport fits into the window, for this the complete window must be considered in aspect_ratio_ws
      if (aspect_ratio_ws > 1)
        {
          vp[2] /= aspect_ratio_ws;
          vp[3] /= aspect_ratio_ws;
        }
      else
        {
          vp[0] *= aspect_ratio_ws;
          vp[1] *= aspect_ratio_ws;
        }

      if (element->parentElement()->localName() == "layout_grid_element")
        {
          double figure_viewport[4];
          auto figure_vp_element = element->parentElement();
          figure_viewport[0] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_x_min_org"));
          figure_viewport[1] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_x_max_org"));
          figure_viewport[2] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_y_min_org"));
          figure_viewport[3] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_y_max_org"));

          metric_width *= (figure_viewport[1] - figure_viewport[0]);
          metric_height *= (figure_viewport[3] - figure_viewport[2]);
          aspect_ratio_ws = metric_width / metric_height;
        }
      if (!element->hasAttribute("_start_aspect_ratio")) element->setAttribute("_start_aspect_ratio", aspect_ratio_ws);

      render->setViewport(element, vp[0], vp[1], vp[2], vp[3]);
      element->setAttribute("_viewport_x_min_org", vp[0]);
      element->setAttribute("_viewport_x_max_org", vp[1]);
      element->setAttribute("_viewport_y_min_org", vp[2]);
      element->setAttribute("_viewport_y_max_org", vp[3]);
    }
  else if (element->localName() == "side_region")
    {
      double viewport_normalized[4];
      double offset = PLOT_DEFAULT_SIDEREGION_OFFSET, width = PLOT_DEFAULT_SIDEREGION_WIDTH;
      std::string location = PLOT_DEFAULT_SIDEREGION_LOCATION, kind;
      bool keep_aspect_ratio = false, uniform_data = true, only_square_aspect_ratio = false, x_flip = false;

      auto plot_parent = element;
      getPlotParent(plot_parent);

      if (element->hasAttribute("location")) location = static_cast<std::string>(element->getAttribute("location"));

      // is set cause processElement is run for every Element even when only attributes gets processed; so the
      // calculateViewport call from the plot element which causes the calculation of the central_region viewport is
      // also processed
      viewport_normalized[0] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_x_min_org"));
      viewport_normalized[1] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_x_max_org"));
      viewport_normalized[2] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_y_min_org")) /
                               (DEFAULT_ASPECT_RATIO_FOR_SCALING);
      viewport_normalized[3] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_y_max_org")) /
                               (DEFAULT_ASPECT_RATIO_FOR_SCALING);

      keep_aspect_ratio = static_cast<int>(plot_parent->getAttribute("keep_aspect_ratio"));
      only_square_aspect_ratio = static_cast<int>(plot_parent->getAttribute("only_square_aspect_ratio"));
      kind = static_cast<std::string>(plot_parent->getAttribute("_kind"));

      if (kind == "marginal_heatmap") width /= 1.5;
      if (element->querySelectors("colorbar")) width = PLOT_DEFAULT_COLORBAR_WIDTH;

      if (keep_aspect_ratio && only_square_aspect_ratio)
        {
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
              double border = 0.5 * (viewport_normalized[1] - viewport_normalized[0]) *
                              (1.0 - 1.0 / (DEFAULT_ASPECT_RATIO_FOR_SCALING));
              viewport_normalized[0] += border;
              viewport_normalized[1] -= border;
            }
        }

      if (element->hasAttribute("viewport_offset") &&
          !(element->hasAttribute("marginal_heatmap_side_plot") &&
            static_cast<int>(element->getAttribute("marginal_heatmap_side_plot"))))
        offset = static_cast<double>(element->getAttribute("viewport_offset"));
      if (element->hasAttribute("width") && !(element->hasAttribute("marginal_heatmap_side_plot") &&
                                              static_cast<int>(element->getAttribute("marginal_heatmap_side_plot"))))
        width = static_cast<double>(element->getAttribute("width"));

      // side_region only has a text_region child - no offset or width is needed
      if (element->querySelectors("side_plot_region") == nullptr &&
          !(element->hasAttribute("marginal_heatmap_side_plot") &&
            static_cast<int>(element->getAttribute("marginal_heatmap_side_plot"))))
        {
          offset = 0.0;
          width = 0.0;
        }

      if (!element->hasAttribute("_viewport_offset_set_by_user")) element->setAttribute("viewport_offset", offset);
      if (!element->hasAttribute("_width_set_by_user")) element->setAttribute("width", width);

      // apply text width to the side_region
      if (kind != "imshow")
        {
          auto additional_axis = element->querySelectors("axis");
          if (location == "top")
            {
              auto is_title =
                  element->hasAttribute("text_is_title") && static_cast<int>(element->getAttribute("text_is_title"));
              if ((element->querySelectors("side_plot_region") == nullptr ||
                   element->querySelectors("text_region") != nullptr) &&
                  !(element->hasAttribute("marginal_heatmap_side_plot") &&
                    static_cast<int>(element->getAttribute("marginal_heatmap_side_plot"))))
                {
                  if ((element->querySelectors("text_region") != nullptr && is_title) &&
                      element->querySelectors("side_plot_region") == nullptr && kind != "pie")
                    {
                      offset += 0.025 * (viewport_normalized[3] - viewport_normalized[2]);
                    }
                  else if ((element->querySelectors("text_region") != nullptr && is_title))
                    {
                      width += 0.025 * (viewport_normalized[3] - viewport_normalized[2]);
                    }
                }
              if (plot_parent->querySelectors("axis[location=\"twin_x\"]"))
                offset += 0.05 * (viewport_normalized[3] - viewport_normalized[2]);
              if (kinds_3d.count(kind) > 0) offset = PLOT_DEFAULT_COLORBAR_OFFSET;
              if (element->hasAttribute("text_content"))
                {
                  width += (is_title ? 0.075 : 0.05) * (viewport_normalized[3] - viewport_normalized[2]);
                  if (additional_axis && is_title) width += 0.025 * (viewport_normalized[3] - viewport_normalized[2]);
                  if (!additional_axis && !is_title && strEqualsAny(kind, "heatmap", "shade", "contourf"))
                    offset += 0.02;
                }
            }
          else if (location == "left")
            {
              if (kinds_3d.count(kind) == 0 && polar_kinds.count(kind) == 0)
                offset += 0.05 * (viewport_normalized[1] - viewport_normalized[0]);
              if (element->querySelectors("side_plot_region") == nullptr ||
                  element->querySelectors("text_region") != nullptr)
                offset += 0.025 * (viewport_normalized[1] - viewport_normalized[0]);
              if (element->hasAttribute("text_content"))
                width += 0.05 * (viewport_normalized[1] - viewport_normalized[0]);
            }
          else if (location == "bottom")
            {
              if (kinds_3d.count(kind) == 0 && polar_kinds.count(kind) == 0)
                offset += 0.05 * (viewport_normalized[3] - viewport_normalized[2]);
              if (element->querySelectors("side_plot_region") == nullptr ||
                  element->querySelectors("text_region") != nullptr)
                offset += 0.025 * (viewport_normalized[3] - viewport_normalized[2]);
              if (element->hasAttribute("text_content"))
                width += 0.05 * (viewport_normalized[3] - viewport_normalized[2]);
            }
          else if (location == "right")
            {
              if (auto axis = plot_parent->querySelectors("axis[name=\"twin-y-axis\"]"))
                {
                  if (!axis->hasAttribute("location") ||
                      (axis->hasAttribute("location") &&
                       static_cast<std::string>(axis->getAttribute("location")) == "twin_y"))
                    offset += 0.05 * (viewport_normalized[1] - viewport_normalized[0]);
                }
              if (element->hasAttribute("text_content"))
                width += 0.05 * (viewport_normalized[1] - viewport_normalized[0]);
            }
        }
      if (plot_parent->hasAttribute("x_flip")) x_flip = static_cast<int>(plot_parent->getAttribute("x_flip"));
      // 180 is bigger than 0 so a adjustment is needed
      if (polar_kinds.count(kind) > 0 && (location == "left" || (location == "right" && x_flip))) offset += 0.01;

      setViewportForSideRegionElements(element, offset, width, uniform_data);

      // special adjustments for side_region bbox so that it includes custom axes
      bboxViewportAdjustmentsForSideRegions(element, location);
    }
  else if (element->localName() == "text_region")
    {
      double width = 0.0, offset = 0.0;
      double viewport_normalized[4];
      std::string kind, location;
      auto plot_parent = element;
      getPlotParent(plot_parent);

      viewport_normalized[0] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_x_min_org"));
      viewport_normalized[1] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_x_max_org"));
      viewport_normalized[2] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_y_min_org")) /
                               (DEFAULT_ASPECT_RATIO_FOR_SCALING);
      viewport_normalized[3] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_y_max_org")) /
                               (DEFAULT_ASPECT_RATIO_FOR_SCALING);

      location = static_cast<std::string>(element->parentElement()->getAttribute("location"));
      kind = static_cast<std::string>(plot_parent->getAttribute("_kind"));

      if (element->parentElement()->hasAttribute("viewport_offset"))
        offset = static_cast<double>(element->parentElement()->getAttribute("viewport_offset"));

      if (element->parentElement()->querySelectors("colorbar"))
        {
          offset = PLOT_DEFAULT_COLORBAR_WIDTH;
        }
      else if ((element->parentElement()->hasAttribute("marginal_heatmap_side_plot") &&
                static_cast<int>(element->parentElement()->getAttribute("marginal_heatmap_side_plot"))))
        {
          offset = PLOT_DEFAULT_SIDEREGION_WIDTH;
        }
      else if (element->parentElement()->querySelectors("axis") &&
               !element->parentElement()->querySelectors("colorbar"))
        {
          offset = PLOT_DEFAULT_ADDITIONAL_AXIS_WIDTH;
        }
      if (!(element->parentElement()->hasAttribute("marginal_heatmap_side_plot") &&
            static_cast<int>(element->parentElement()->getAttribute("marginal_heatmap_side_plot"))))
        {
          // apply text width to the side_region
          if (kind != "imshow")
            {
              auto additional_axis = element->parentElement()->querySelectors("axis");
              if (location == "top")
                {
                  if (!additional_axis) offset += 0.05 * (viewport_normalized[3] - viewport_normalized[2]);
                }
              if (location == "left")
                {
                  if (!additional_axis) offset += 0.05 * (viewport_normalized[1] - viewport_normalized[0]);
                }
              if (location == "bottom")
                {
                  if (!additional_axis) offset += 0.05 * (viewport_normalized[3] - viewport_normalized[2]);
                }
              if (location == "right" && !additional_axis)
                {
                  offset += 0.05 * (viewport_normalized[1] - viewport_normalized[0]);
                }
            }
        }
      else
        {
          if (location == "top") offset = 0.05 * (viewport_normalized[3] - viewport_normalized[2]);
        }

      setViewportForSideRegionElements(element, offset, width, false);
    }
  else if (element->localName() == "side_plot_region")
    {
      double viewport_normalized[4];
      double offset = 0.0, width = PLOT_DEFAULT_SIDEREGION_WIDTH;
      std::string kind, location;
      bool keep_aspect_ratio = false, uniform_data = true, only_square_aspect_ratio = false;
      auto plot_parent = element;
      getPlotParent(plot_parent);

      viewport_normalized[0] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_x_min_org"));
      viewport_normalized[1] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_x_max_org"));
      viewport_normalized[2] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_y_min_org")) /
                               (DEFAULT_ASPECT_RATIO_FOR_SCALING);
      viewport_normalized[3] = static_cast<double>(plot_parent->getAttribute("_viewport_normalized_y_max_org")) /
                               (DEFAULT_ASPECT_RATIO_FOR_SCALING);

      keep_aspect_ratio = static_cast<int>(plot_parent->getAttribute("keep_aspect_ratio"));
      only_square_aspect_ratio = static_cast<int>(plot_parent->getAttribute("only_square_aspect_ratio"));
      location = static_cast<std::string>(element->parentElement()->getAttribute("location"));
      kind = static_cast<std::string>(plot_parent->getAttribute("_kind"));

      if (keep_aspect_ratio && only_square_aspect_ratio)
        {
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
              double border = 0.5 * (viewport_normalized[1] - viewport_normalized[0]) *
                              (1.0 - 1.0 / (DEFAULT_ASPECT_RATIO_FOR_SCALING));
              viewport_normalized[0] += border;
              viewport_normalized[1] -= border;
            }
        }

      if (kind == "marginal_heatmap") width /= 1.5;
      if (element->querySelectors("colorbar")) width = PLOT_DEFAULT_COLORBAR_WIDTH;
      if (!(element->parentElement()->hasAttribute("marginal_heatmap_side_plot") &&
            static_cast<int>(element->parentElement()->getAttribute("marginal_heatmap_side_plot"))))
        {
          if (element->parentElement()->hasAttribute("width"))
            width = static_cast<double>(element->parentElement()->getAttribute("width"));
        }

      if (location == "top" && element->parentElement()->querySelectors("text_region"))
        {
          if (!(element->parentElement()->hasAttribute("text_is_title") &&
                static_cast<int>(element->parentElement()->getAttribute("text_is_title"))))
            {
              offset += 0.05 * (viewport_normalized[3] - viewport_normalized[2]);
            }
        }
      if (location == "right")
        {
          if (element->parentElement()->querySelectors("text_region"))
            offset += 0.05 * (viewport_normalized[1] - viewport_normalized[0]);
        }

      setViewportForSideRegionElements(element, offset, width, false);

      // special adjustments for side_plot_region bbox so that it includes custom axes
      bboxViewportAdjustmentsForSideRegions(element, location);
    }
  else if (element->localName() == "colorbar")
    {
      auto vp_x_min = static_cast<double>(element->parentElement()->getAttribute("viewport_x_min"));
      auto vp_x_max = static_cast<double>(element->parentElement()->getAttribute("viewport_x_max"));
      auto vp_y_min = static_cast<double>(element->parentElement()->getAttribute("viewport_y_min"));
      auto vp_y_max = static_cast<double>(element->parentElement()->getAttribute("viewport_y_max"));

      render->setViewport(element, vp_x_min, vp_x_max, vp_y_min, vp_y_max);
      element->setAttribute("_viewport_x_min_org", vp_x_min);
      element->setAttribute("_viewport_x_max_org", vp_x_max);
      element->setAttribute("_viewport_y_min_org", vp_y_min);
      element->setAttribute("_viewport_y_max_org", vp_y_max);
    }
  else if (element->localName() == "marginal_heatmap_plot")
    {
      double vp_x_min, vp_x_max, vp_y_min, vp_y_max;
      if (!GRM::Render::getViewport(element->parentElement(), &vp_x_min, &vp_x_max, &vp_y_min, &vp_y_max))
        throw NotFoundError(element->parentElement()->localName() + " doesn't have a viewport but it should.\n");
      render->setViewport(element, vp_x_min, vp_x_max, vp_y_min, vp_y_max);
      element->setAttribute("_viewport_x_min_org", vp_x_min);
      element->setAttribute("_viewport_x_max_org", vp_x_max);
      element->setAttribute("_viewport_y_min_org", vp_y_min);
      element->setAttribute("_viewport_y_max_org", vp_y_max);
    }
  else if (element->localName() == "legend")
    {
      int location = PLOT_DEFAULT_LOCATION;
      double px, py, w, h;
      double viewport[4], plot_viewport[4];
      double vp_x_min, vp_x_max, vp_y_min, vp_y_max;
      double scale_factor = 1.0, start_aspect_ratio_ws;
      const std::shared_ptr<GRM::Context> &context = render->getContext();
      std::string kind;
      std::shared_ptr<GRM::Element> central_region, plot_parent = element;
      bool keep_aspect_ratio = false;
      double metric_width, metric_height;

      getPlotParent(plot_parent);
      for (const auto &child : element->parentElement()->children())
        {
          if (child->localName() == "central_region")
            {
              central_region = child;
              break;
            }
        }

      GRM::getFigureSize(nullptr, nullptr, &metric_width, &metric_height);
      auto aspect_ratio_ws = metric_width / metric_height;
      if (plot_parent->parentElement()->localName() == "layout_grid_element")
        {
          double figure_viewport[4];
          auto figure_vp_element = plot_parent->parentElement();
          figure_viewport[0] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_x_min_org"));
          figure_viewport[1] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_x_max_org"));
          figure_viewport[2] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_y_min_org"));
          figure_viewport[3] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_y_max_org"));

          metric_width *= (figure_viewport[1] - figure_viewport[0]);
          metric_height *= (figure_viewport[3] - figure_viewport[2]);
          aspect_ratio_ws = metric_width / metric_height;
        }
      start_aspect_ratio_ws = static_cast<double>(plot_parent->getAttribute("_start_aspect_ratio"));

      if (!GRM::Render::getViewport(central_region, &viewport[0], &viewport[1], &viewport[2], &viewport[3]))
        throw NotFoundError("Central region doesn't have a viewport but it should.\n");
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

      kind = static_cast<std::string>(element->parentElement()->getAttribute("_kind"));
      if (polar_kinds.count(kind) > 0) location = 11;
      if (element->hasAttribute("location"))
        {
          if (element->getAttribute("location").isInt())
            {
              location = static_cast<int>(element->getAttribute("location"));
            }
          else if (element->getAttribute("location").isString())
            {
              location = GRM::locationStringToInt(static_cast<std::string>(element->getAttribute("location")));
            }
        }
      else
        {
          element->setAttribute("location", location);
        }
      keep_aspect_ratio = static_cast<int>(element->parentElement()->getAttribute("keep_aspect_ratio"));

      if (!keep_aspect_ratio)
        {
          if (plot_parent->parentElement()->localName() != "layout_grid_element")
            scale_factor *= DEFAULT_ASPECT_RATIO_FOR_SCALING;
          scale_factor *= (aspect_ratio_ws <= 1) ? aspect_ratio_ws : 1.0 / aspect_ratio_ws;

          if (plot_parent->parentElement()->localName() == "layout_grid_element")
            {
              auto figure_vp_element = plot_parent->parentElement();
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

              if (num_col > 1 && num_row > 1)
                scale_factor /= grm_min(num_col, num_row);
              else
                scale_factor *= grm_max(num_col, num_row);
            }
        }
      else
        {
          double diag_factor = std::sqrt((plot_viewport[1] - plot_viewport[0]) * (plot_viewport[1] - plot_viewport[0]) +
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
              double figure_viewport[4];
              auto figure_vp_element = plot_parent->parentElement()->localName() == "layout_grid_element"
                                           ? plot_parent->parentElement()
                                           : plot_parent;
              figure_viewport[0] =
                  static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_x_min_org"));
              figure_viewport[1] =
                  static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_x_max_org"));
              figure_viewport[2] =
                  static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_y_min_org"));
              figure_viewport[3] =
                  static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_y_max_org"));

              auto default_diag_factor =
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

                  auto plot_diag_factor =
                      std::sqrt((figure_viewport[1] - figure_viewport[0]) * (figure_viewport[1] - figure_viewport[0]) +
                                (figure_viewport[3] - figure_viewport[2]) * (figure_viewport[3] - figure_viewport[2]));
                  default_diag_factor =
                      ((DEFAULT_ASPECT_RATIO_FOR_SCALING) *
                       (start_aspect_ratio_ws <= 1 ? start_aspect_ratio_ws : (1.0 / start_aspect_ratio_ws))) /
                      ((diag_factor * size_scale_factor) / plot_diag_factor);
                  if (num_col > 1 && num_row > num_col) default_diag_factor *= DEFAULT_ASPECT_RATIO_FOR_SCALING;
                }
              element->setAttribute("_default_diag_factor", default_diag_factor);
            }
          auto default_diag_factor = static_cast<double>(element->getAttribute("_default_diag_factor"));

          scale_factor = diag_factor * default_diag_factor;
        }
      element->setAttribute("_scale_factor", scale_factor);
      if (!element->hasAttribute("_initial_scale_factor")) element->setAttribute("_initial_scale_factor", scale_factor);

      if (kind != "pie")
        {
          legendSize(element, &w, &h);
          if (element->hasAttribute("_start_w"))
            {
              w = static_cast<double>(element->getAttribute("_start_w"));
            }
          else
            {
              element->setAttribute("_start_w", w);
            }
          if (element->hasAttribute("_start_h"))
            {
              h = static_cast<double>(element->getAttribute("_start_h"));
            }
          else
            {
              element->setAttribute("_start_h", h);
            }

          if (intEqualsAny(location, 3, 11, 12, 13))
            {
              px = viewport[1] + 0.11 * scale_factor;
            }
          else if (intEqualsAny(location, 3, 8, 9, 10))
            {
              px = 0.5 * (viewport[0] + viewport[1] - (w - 0.05) * scale_factor);
            }
          else if (intEqualsAny(location, 3, 2, 3, 6))
            {
              px = viewport[0] + 0.11 * scale_factor;
            }
          else
            {
              px = viewport[1] - (0.05 + w) * scale_factor;
            }
          if (intEqualsAny(location, 5, 5, 6, 7, 10, 12))
            {
              py = 0.5 * (viewport[2] + viewport[3] + h * scale_factor) - 0.03 * scale_factor;
            }
          else if (location == 13)
            {
              py = viewport[2] + h * scale_factor;
            }
          else if (intEqualsAny(location, 3, 3, 4, 8))
            {
              py = viewport[2] + (h + 0.03) * scale_factor;
            }
          else if (location == 11)
            {
              py = viewport[3] - 0.03 * scale_factor;
            }
          else
            {
              py = viewport[3] - 0.06 * scale_factor;
            }
          vp_x_min = px - 0.08 * scale_factor;
          vp_x_max = px + (w + 0.02) * scale_factor;
          vp_y_min = py - h * scale_factor;
          vp_y_max = py + 0.03 * scale_factor;
        }
      else
        {
          legendSize(element, &w, &h);

          if (element->hasAttribute("_start_w"))
            {
              w = static_cast<double>(element->getAttribute("_start_w"));
            }
          else
            {
              element->setAttribute("_start_w", w);
            }
          if (element->hasAttribute("_start_h"))
            {
              h = static_cast<double>(element->getAttribute("_start_h"));
            }
          else
            {
              element->setAttribute("_start_h", h);
            }

          px = 0.5 * (viewport[0] + viewport[1] - w * scale_factor);
          py = viewport[2] - 0.75 * h * scale_factor;

          vp_x_min = px - 0.02 * scale_factor;
          vp_x_max = px + (w + 0.02) * scale_factor;
          vp_y_min = py - (0.5 * h + 0.02) * scale_factor;
          vp_y_max = py + (0.5 * h + 0.02) * scale_factor;
        }

      render->setViewport(element, vp_x_min, vp_x_max, vp_y_min, vp_y_max);
      element->setAttribute("_viewport_x_min_org", vp_x_min);
      element->setAttribute("_viewport_x_max_org", vp_x_max);
      element->setAttribute("_viewport_y_min_org", vp_y_min);
      element->setAttribute("_viewport_y_max_org", vp_y_max);
    }
  else if (element->localName() == "layout_grid")
    {
      double vp[4];
      double metric_width, metric_height;
      double aspect_ratio_ws;

      /* when grids are being used for layouting the plot information is stored in the parent of the plot */
      vp[0] = static_cast<double>(element->getAttribute("_viewport_normalized_x_min_org"));
      vp[1] = static_cast<double>(element->getAttribute("_viewport_normalized_x_max_org"));
      vp[2] = static_cast<double>(element->getAttribute("_viewport_normalized_y_min_org"));
      vp[3] = static_cast<double>(element->getAttribute("_viewport_normalized_y_max_org"));

      GRM::getFigureSize(nullptr, nullptr, &metric_width, &metric_height);
      aspect_ratio_ws = metric_width / metric_height;
      // plot viewport fits into the window, for this the complete window must be considered in aspect_ratio_ws
      if (aspect_ratio_ws > 1)
        {
          vp[2] /= aspect_ratio_ws;
          vp[3] /= aspect_ratio_ws;
        }
      else
        {
          vp[0] *= aspect_ratio_ws;
          vp[1] *= aspect_ratio_ws;
        }

      render->setViewport(element, vp[0], vp[1], vp[2], vp[3]);
      element->setAttribute("_viewport_x_min_org", vp[0]);
      element->setAttribute("_viewport_x_max_org", vp[1]);
      element->setAttribute("_viewport_y_min_org", vp[2]);
      element->setAttribute("_viewport_y_max_org", vp[3]);
    }
  else if (element->localName() == "axis")
    {
      double plot_vp[4];
      double vp_x_min, vp_x_max, vp_y_min, vp_y_max;
      auto plot_parent = element;
      getPlotParent(plot_parent);

      if (!GRM::Render::getViewport(plot_parent, &plot_vp[0], &plot_vp[1], &plot_vp[2], &plot_vp[3]))
        throw NotFoundError(plot_parent->localName() + " doesn't have a viewport but it should.\n");

      if (strEqualsAny(element->parentElement()->localName(), "side_plot_region", "colorbar"))
        {
          std::string location;
          double tmp;
          auto ref_vp_element = element->parentElement();

          if (element->hasAttribute("location"))
            location = static_cast<std::string>(element->getAttribute("location"));
          else if (ref_vp_element->hasAttribute("location"))
            location = static_cast<std::string>(ref_vp_element->getAttribute("location"));
          else if (ref_vp_element->parentElement()->hasAttribute("location"))
            location = static_cast<std::string>(ref_vp_element->parentElement()->getAttribute("location"));
          else if (ref_vp_element->parentElement()->parentElement()->hasAttribute("location"))
            location =
                static_cast<std::string>(ref_vp_element->parentElement()->parentElement()->getAttribute("location"));

          if (strEqualsAny(location, "left", "right"))
            {
              vp_y_min = static_cast<double>(ref_vp_element->getAttribute("viewport_y_min"));
              vp_y_max = static_cast<double>(ref_vp_element->getAttribute("viewport_y_max"));

              if (location == "right")
                {
                  vp_x_max = static_cast<double>(ref_vp_element->getAttribute("viewport_x_max"));
                  if (!GRM::Render::getViewport(ref_vp_element, &tmp, &vp_x_min, &tmp, &tmp))
                    throw NotFoundError(ref_vp_element->localName() + " doesn't have a viewport but it should.\n");
                  if (ref_vp_element->localName() == "side_plot_region")
                    {
                      bool down_ticks = element->hasAttribute("tick_orientation") &&
                                        static_cast<int>(element->getAttribute("tick_orientation")) < 0;
                      if (down_ticks) vp_x_min -= 0.02 * (plot_vp[1] - plot_vp[0]);
                    }
                }
              else
                {
                  vp_x_min = static_cast<double>(ref_vp_element->getAttribute("viewport_x_min"));
                  if (!GRM::Render::getViewport(ref_vp_element, &vp_x_max, &tmp, &tmp, &tmp))
                    throw NotFoundError(ref_vp_element->localName() + " doesn't have a viewport but it should.\n");
                  if (ref_vp_element->localName() == "colorbar" &&
                      ref_vp_element->parentElement()->parentElement()->hasAttribute("text_content"))
                    {
                      vp_x_max = static_cast<double>(ref_vp_element->getAttribute("viewport_x_max"));
                      vp_x_max -= PLOT_DEFAULT_COLORBAR_WIDTH;
                    }

                  if (ref_vp_element->localName() == "side_plot_region")
                    {
                      bool down_ticks = element->hasAttribute("tick_orientation") &&
                                        static_cast<int>(element->getAttribute("tick_orientation")) < 0;
                      if (!down_ticks) vp_x_max += 0.02 * (plot_vp[1] - plot_vp[0]);
                    }
                }
              vp_x_min = grm_max(plot_vp[0], vp_x_min);
              vp_x_max = grm_min(plot_vp[1], vp_x_max);
            }
          else if (strEqualsAny(location, "bottom", "top"))
            {
              vp_x_min = static_cast<double>(ref_vp_element->getAttribute("viewport_x_min"));
              vp_x_max = static_cast<double>(ref_vp_element->getAttribute("viewport_x_max"));

              if (location == "top")
                {
                  vp_y_max = static_cast<double>(ref_vp_element->getAttribute("viewport_y_max"));
                  if (!GRM::Render::getViewport(ref_vp_element, &tmp, &tmp, &tmp, &vp_y_min))
                    throw NotFoundError(ref_vp_element->localName() + " doesn't have a viewport but it should.\n");
                  if (ref_vp_element->localName() == "side_plot_region")
                    {
                      bool down_ticks = element->hasAttribute("tick_orientation") &&
                                        static_cast<int>(element->getAttribute("tick_orientation")) < 0;
                      if (down_ticks) vp_y_min -= 0.02 * (plot_vp[3] - plot_vp[2]);
                    }
                }
              else
                {
                  vp_y_min = static_cast<double>(ref_vp_element->getAttribute("viewport_y_min"));
                  if (!GRM::Render::getViewport(ref_vp_element, &tmp, &tmp, &vp_y_max, &tmp))
                    throw NotFoundError(ref_vp_element->localName() + " doesn't have a viewport but it should.\n");
                  if (ref_vp_element->localName() == "side_plot_region")
                    {
                      bool down_ticks = element->hasAttribute("tick_orientation") &&
                                        static_cast<int>(element->getAttribute("tick_orientation")) < 0;
                      if (!down_ticks) vp_y_max += 0.02 * (plot_vp[3] - plot_vp[2]);
                    }
                }
              vp_y_min = grm_max(plot_vp[2], vp_y_min);
              vp_y_max = grm_min(plot_vp[3], vp_y_max);
            }
        }
      else
        {
          if (element->parentElement()->localName() == "coordinate_system")
            {
              double central_region_x_min, central_region_x_max, central_region_y_min, central_region_y_max;
              auto central_region = element->parentElement()->parentElement();
              bool mirrored_axis =
                  element->hasAttribute("mirrored_axis") && static_cast<int>(element->getAttribute("mirrored_axis"));
              bool down_ticks = element->hasAttribute("tick_orientation") &&
                                static_cast<int>(element->getAttribute("tick_orientation")) < 0;
              bool draw_grid =
                  element->hasAttribute("draw_grid") && static_cast<int>(element->getAttribute("draw_grid"));
              auto location = static_cast<std::string>(element->getAttribute("location"));

              if (!GRM::Render::getViewport(central_region, &central_region_x_min, &central_region_x_max,
                                            &central_region_y_min, &central_region_y_max))
                throw NotFoundError("Central region doesn't have a viewport but it should.\n");

              if (strEqualsAny(location, "y", "twin_y"))
                {
                  vp_y_min = central_region_y_min;
                  vp_y_max = central_region_y_max;

                  if (location == "y")
                    {
                      vp_x_min = central_region_x_min;

                      vp_x_min -= 0.075 * (plot_vp[1] - plot_vp[0]);
                      if (mirrored_axis || draw_grid)
                        {
                          vp_x_max = central_region_x_max;
                          if (mirrored_axis && down_ticks) vp_x_max += 0.02 * (plot_vp[1] - plot_vp[0]);
                        }
                      else
                        {
                          vp_x_max = central_region_x_min;
                          if (!down_ticks) vp_x_max += 0.02 * (plot_vp[1] - plot_vp[0]);
                        }
                    }
                  else
                    {
                      vp_x_max = central_region_x_max;

                      vp_x_max += 0.075 * (plot_vp[1] - plot_vp[0]);
                      if (mirrored_axis || draw_grid)
                        {
                          vp_x_min = central_region_x_min;
                          if (mirrored_axis && down_ticks) vp_x_min -= 0.02 * (plot_vp[1] - plot_vp[0]);
                        }
                      else
                        {
                          vp_x_min = central_region_x_max;
                          if (down_ticks) vp_x_min -= 0.02 * (plot_vp[1] - plot_vp[0]);
                        }
                    }

                  // 0.025 for tick-label at min/max viewport which will be longer than the normal min/max viewport
                  vp_y_min -= 0.025 * (plot_vp[3] - plot_vp[2]);
                  vp_y_max += 0.025 * (plot_vp[3] - plot_vp[2]);
                }
              else if (strEqualsAny(location, "x", "twin_x"))
                {
                  vp_x_min = central_region_x_min;
                  vp_x_max = central_region_x_max;

                  if (location == "x")
                    {
                      vp_y_min = central_region_y_min;

                      vp_y_min -= 0.075 * (plot_vp[3] - plot_vp[2]);
                      if (mirrored_axis || draw_grid)
                        {
                          vp_y_max = central_region_y_max;
                          if (mirrored_axis && down_ticks) vp_y_max += 0.02 * (plot_vp[3] - plot_vp[2]);
                        }
                      else
                        {
                          vp_y_max = central_region_y_min;
                          if (!down_ticks) vp_y_max += 0.02 * (plot_vp[3] - plot_vp[2]);
                        }
                    }
                  else
                    {
                      vp_y_max = central_region_y_max;

                      vp_y_max += 0.075 * (plot_vp[3] - plot_vp[2]);
                      if (mirrored_axis || draw_grid)
                        {
                          vp_y_min = central_region_y_min;
                          if (mirrored_axis && down_ticks) vp_y_min += 0.02 * (plot_vp[3] - plot_vp[2]);
                        }
                      else
                        {
                          vp_y_min = central_region_y_max;
                          if (down_ticks) vp_y_min -= 0.02 * (plot_vp[3] - plot_vp[2]);
                        }
                    }

                  // 0.025 for tick-label at min/max viewport which will be longer than the normal min/max viewport
                  vp_x_min -= 0.025 * (plot_vp[1] - plot_vp[0]);
                  vp_x_max += 0.025 * (plot_vp[1] - plot_vp[0]);
                }
            }
        }
      render->setViewport(element, vp_x_min, vp_x_max, vp_y_min, vp_y_max);
      element->setAttribute("_viewport_x_min_org", vp_x_min);
      element->setAttribute("_viewport_x_max_org", vp_x_max);
      element->setAttribute("_viewport_y_min_org", vp_y_min);
      element->setAttribute("_viewport_y_max_org", vp_y_max);
    }
  else if (element->localName() == "coordinate_system")
    {
      double vp_x_min, vp_x_max, vp_y_min, vp_y_max;
      auto central_region = element->parentElement();

      vp_x_min = static_cast<double>(central_region->getAttribute("viewport_x_min"));
      vp_x_max = static_cast<double>(central_region->getAttribute("viewport_x_max"));
      vp_y_min = static_cast<double>(central_region->getAttribute("viewport_y_min"));
      vp_y_max = static_cast<double>(central_region->getAttribute("viewport_y_max"));

      render->setViewport(element, vp_x_min, vp_x_max, vp_y_min, vp_y_max);
      element->setAttribute("_viewport_x_min_org", vp_x_min);
      element->setAttribute("_viewport_x_max_org", vp_x_max);
      element->setAttribute("_viewport_y_min_org", vp_y_min);
      element->setAttribute("_viewport_y_max_org", vp_y_max);
    }
  else if (element->localName() == "overlay")
    {
      double vp[4];
      double metric_width, metric_height;
      double aspect_ratio_ws;

      vp[0] = static_cast<double>(element->getAttribute("_viewport_normalized_x_min_org"));
      vp[1] = static_cast<double>(element->getAttribute("_viewport_normalized_x_max_org"));
      vp[2] = static_cast<double>(element->getAttribute("_viewport_normalized_y_min_org"));
      vp[3] = static_cast<double>(element->getAttribute("_viewport_normalized_y_max_org"));

      GRM::getFigureSize(nullptr, nullptr, &metric_width, &metric_height);
      aspect_ratio_ws = metric_width / metric_height;
      // plot viewport fits into the window, for this the complete window must be considered in aspect_ratio_ws
      if (aspect_ratio_ws > 1)
        {
          vp[2] /= aspect_ratio_ws;
          vp[3] /= aspect_ratio_ws;
        }
      else
        {
          vp[0] *= aspect_ratio_ws;
          vp[1] *= aspect_ratio_ws;
        }

      render->setViewport(element, vp[0], vp[1], vp[2], vp[3]);
      element->setAttribute("_viewport_x_min_org", vp[0]);
      element->setAttribute("_viewport_x_max_org", vp[1]);
      element->setAttribute("_viewport_y_min_org", vp[2]);
      element->setAttribute("_viewport_y_max_org", vp[3]);
    }
  processViewport(element);
}

void setViewportForSideRegionElements(const std::shared_ptr<GRM::Element> &element, double offset, double width,
                                      bool uniform_data)
{
  double viewport[4];
  std::string location = PLOT_DEFAULT_SIDEREGION_LOCATION, kind;
  double max_vp, min_vp;
  double offset_rel, width_rel;
  double metric_width, metric_height, start_aspect_ratio_ws;
  bool keep_aspect_ratio = false, only_square_aspect_ratio = false;
  std::shared_ptr<GRM::Element> plot_parent = element, central_region, side_region = element;
  getPlotParent(plot_parent);
  auto render = grm_get_render();
  auto active_figure = render->getActiveFigure();

  central_region = plot_parent->querySelectors("central_region");
  if (element->localName() != "side_region") side_region = element->parentElement();

  GRM::getFigureSize(nullptr, nullptr, &metric_width, &metric_height);
  auto aspect_ratio_ws = metric_width / metric_height;
  if (plot_parent->parentElement()->localName() == "layout_grid_element")
    {
      double figure_viewport[4];
      auto figure_vp_element = plot_parent->parentElement();
      figure_viewport[0] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_x_min_org"));
      figure_viewport[1] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_x_max_org"));
      figure_viewport[2] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_y_min_org"));
      figure_viewport[3] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_y_max_org"));
      metric_width *= (figure_viewport[1] - figure_viewport[0]);
      metric_height *= (figure_viewport[3] - figure_viewport[2]);
      aspect_ratio_ws = metric_width / metric_height;
    }
  start_aspect_ratio_ws = static_cast<double>(plot_parent->getAttribute("_start_aspect_ratio"));

  if (!GRM::Render::getViewport(central_region, &viewport[0], &viewport[1], &viewport[2], &viewport[3]))
    throw NotFoundError("Central region doesn't have a viewport but it should.\n");

  double diag_factor = std::sqrt((viewport[1] - viewport[0]) * (viewport[1] - viewport[0]) +
                                 (viewport[3] - viewport[2]) * (viewport[3] - viewport[2]));
  if (element->parentElement()->localName() == "side_region")
    {
      if (!GRM::Render::getViewport(element->parentElement(), &viewport[0], &viewport[1], &viewport[2], &viewport[3]))
        throw NotFoundError(element->parentElement()->localName() + " doesn't have a viewport but it should.\n");
    }
  keep_aspect_ratio = static_cast<int>(plot_parent->getAttribute("keep_aspect_ratio"));
  only_square_aspect_ratio = static_cast<int>(plot_parent->getAttribute("only_square_aspect_ratio"));
  location = static_cast<std::string>(side_region->getAttribute("location"));
  kind = static_cast<std::string>(plot_parent->getAttribute("_kind"));

  if (!element->hasAttribute("_default_diag_factor"))
    {
      auto initial_size_x = static_cast<double>(active_figure->getAttribute("_initial_width"));
      auto initial_size_y = static_cast<double>(active_figure->getAttribute("_initial_height"));
      auto size_x = static_cast<double>(active_figure->getAttribute("size_x"));
      auto size_y = static_cast<double>(active_figure->getAttribute("size_y"));
      auto size_scale_factor = 1.0;
      if ((initial_size_x != size_x || initial_size_y != size_y) && (active_figure->hasAttribute("_kind_changed")))
        size_scale_factor = (size_x < size_y) ? (size_y / size_x) : (size_x / size_y);
      auto figure_vp_element = plot_parent->parentElement()->localName() == "layout_grid_element"
                                   ? plot_parent->parentElement()
                                   : plot_parent;

      auto default_diag_factor =
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
              num_col = static_cast<int>(figure_vp_element->parentElement()->parentElement()->getAttribute("num_col"));
              num_row = static_cast<int>(figure_vp_element->parentElement()->parentElement()->getAttribute("num_row"));
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

  // special case for keep_aspect_ratio with uniform data which can lead to smaller plots
  if ((keep_aspect_ratio && uniform_data && only_square_aspect_ratio) || !keep_aspect_ratio)
    {
      auto figure_vp_element = plot_parent->parentElement()->localName() == "layout_grid_element"
                                   ? plot_parent->parentElement()
                                   : plot_parent;

      if (figure_vp_element == plot_parent)
        {
          if (!element->hasAttribute("_viewport_offset_set_by_user")) offset *= DEFAULT_ASPECT_RATIO_FOR_SCALING;
          if (!element->hasAttribute("_width_set_by_user")) width *= DEFAULT_ASPECT_RATIO_FOR_SCALING;
        }
      if (aspect_ratio_ws <= 1)
        {
          offset_rel = offset * aspect_ratio_ws;
          width_rel = width * aspect_ratio_ws;
        }
      else
        {
          offset_rel = offset / aspect_ratio_ws;
          width_rel = width / aspect_ratio_ws;
        }
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
              num_col = static_cast<int>(figure_vp_element->parentElement()->parentElement()->getAttribute("num_col"));
              num_row = static_cast<int>(figure_vp_element->parentElement()->parentElement()->getAttribute("num_row"));
            }
          num_col -= stop_col - start_col - 1;
          num_row -= stop_row - start_row - 1;

          if (location == "left" || location == "right")
            {
              if (num_row != 1)
                {
                  offset_rel /= num_col;
                  width_rel /= num_col;
                }
            }
          else
            {
              if (num_col != 1)
                {
                  offset_rel /= num_row;
                  width_rel /= num_row;
                }
            }
        }
    }
  else
    {
      auto default_diag_factor = static_cast<double>(element->getAttribute("_default_diag_factor"));
      offset_rel = offset * diag_factor * default_diag_factor;
      width_rel = width * diag_factor * default_diag_factor;
    }

  // for visual bbox the viewport must be increased for the tick-labels
  double plot_vp[4];
  if (!GRM::Render::getViewport(plot_parent, &plot_vp[0], &plot_vp[1], &plot_vp[2], &plot_vp[3]))
    throw NotFoundError(plot_parent->localName() + " doesn't have a viewport but it should.\n");

  if (location == "right")
    {
      double vp_x_min = viewport[1] + offset_rel, vp_x_max = viewport[1] + offset_rel + width_rel;
      max_vp = getMaxViewport(element, true);
      if (max_vp != 1) max_vp = static_cast<double>(plot_parent->getAttribute("_viewport_x_max_org"));
      if (element->localName() == "side_plot_region")
        {
          vp_x_min = viewport[0] + offset_rel;
          vp_x_max = viewport[0] + offset_rel + width_rel;
        }
      else if (element->localName() == "text_region")
        {
          vp_x_min = viewport[0];
          vp_x_max = viewport[0] + offset_rel;
        }
      render->setViewport(element, vp_x_min, grm_min(vp_x_max, max_vp), viewport[2], viewport[3]);
      element->setAttribute("_viewport_x_min_org", vp_x_min);
      element->setAttribute("_viewport_x_max_org", grm_min(vp_x_max, max_vp));
      element->setAttribute("_viewport_y_min_org", viewport[2]);
      element->setAttribute("_viewport_y_max_org", viewport[3]);

      if (strEqualsAny(element->localName(), "side_region", "side_plot_region"))
        {
          // 0.025 for tick-label at min/max viewport which will be longer than the normal min/max viewport
          auto vp_y_min = viewport[2] - 0.025 * (plot_vp[3] - plot_vp[2]);
          auto vp_y_max = viewport[3] + 0.025 * (plot_vp[3] - plot_vp[2]);

          render->setViewport(element, vp_x_min, grm_min(vp_x_max, max_vp), vp_y_min, vp_y_max);
          element->setAttribute("_viewport_y_min_org", vp_y_min);
          element->setAttribute("_viewport_y_max_org", vp_y_max);
        }
    }
  else if (location == "left")
    {
      double vp_x_min = viewport[0] - offset_rel - width_rel, vp_x_max = viewport[0] - offset_rel;
      min_vp = getMinViewport(element, true);
      if (element->localName() == "side_plot_region")
        {
          vp_x_min = viewport[0];
          vp_x_max = viewport[0] + width_rel;
        }
      else if (element->localName() == "text_region")
        {
          vp_x_min = viewport[1] - offset_rel;
          vp_x_max = viewport[1];
        }
      render->setViewport(element, grm_max(vp_x_min, min_vp), vp_x_max, viewport[2], viewport[3]);
      element->setAttribute("_viewport_x_min_org", grm_max(vp_x_min, min_vp));
      element->setAttribute("_viewport_x_max_org", vp_x_max);
      element->setAttribute("_viewport_y_min_org", viewport[2]);
      element->setAttribute("_viewport_y_max_org", viewport[3]);

      if (strEqualsAny(element->localName(), "side_region", "side_plot_region"))
        {
          // 0.025 for tick-label at min/max viewport which will be longer than the normal min/max viewport
          auto vp_y_min = viewport[2] - 0.025 * (plot_vp[3] - plot_vp[2]);
          auto vp_y_max = viewport[3] + 0.025 * (plot_vp[3] - plot_vp[2]);

          render->setViewport(element, grm_max(vp_x_min, min_vp), vp_x_max, vp_y_min, vp_y_max);
          element->setAttribute("_viewport_y_min_org", vp_y_min);
          element->setAttribute("_viewport_y_max_org", vp_y_max);
        }
    }
  else if (location == "top")
    {
      double vp_y_min = viewport[3] + offset_rel, vp_y_max = viewport[3] + offset_rel + width_rel;
      max_vp = getMaxViewport(element, false);
      if (max_vp != 1) max_vp = static_cast<double>(plot_parent->getAttribute("_viewport_y_max_org"));
      if (element->localName() == "side_plot_region")
        {
          vp_y_min = viewport[2] + offset_rel;
          vp_y_max = viewport[2] + offset_rel + width_rel;
        }
      else if (element->localName() == "text_region" &&
               !(element->parentElement()->hasAttribute("text_is_title") &&
                 static_cast<int>(element->parentElement()->getAttribute("text_is_title"))))
        {
          vp_y_min = viewport[2];
          vp_y_max = viewport[2] + offset_rel;
        }
      else if (element->localName() == "text_region" &&
               (element->parentElement()->hasAttribute("text_is_title") &&
                static_cast<int>(element->parentElement()->getAttribute("text_is_title"))))
        {
          vp_y_min = viewport[3] - offset_rel;
          vp_y_max = viewport[3];
        }
      render->setViewport(element, viewport[0], viewport[1], vp_y_min, grm_min(vp_y_max, max_vp));
      element->setAttribute("_viewport_x_min_org", viewport[0]);
      element->setAttribute("_viewport_x_max_org", viewport[1]);
      element->setAttribute("_viewport_y_min_org", vp_y_min);
      element->setAttribute("_viewport_y_max_org", grm_min(vp_y_max, max_vp));

      if (strEqualsAny(element->localName(), "side_region", "side_plot_region"))
        {
          // 0.025 for tick-label at min/max viewport which will be longer than the normal min/max viewport
          auto vp_x_min = viewport[0] - 0.025 * (plot_vp[1] - plot_vp[0]);
          auto vp_x_max = viewport[1] + 0.025 * (plot_vp[1] - plot_vp[0]);

          render->setViewport(element, vp_x_min, vp_x_max, vp_y_min, grm_min(vp_y_max, max_vp));
          element->setAttribute("_viewport_x_min_org", vp_x_min);
          element->setAttribute("_viewport_x_max_org", vp_x_max);
        }
    }
  else if (location == "bottom")
    {
      double vp_y_min = viewport[2] - offset_rel - width_rel, vp_y_max = viewport[2] - offset_rel;
      min_vp = getMinViewport(element, false);
      if (element->localName() == "side_plot_region")
        {
          vp_y_min = viewport[2];
          vp_y_max = viewport[2] + width_rel;
        }
      else if (element->localName() == "text_region")
        {
          vp_y_min = viewport[3] - offset_rel;
          vp_y_max = viewport[3];
        }
      render->setViewport(element, viewport[0], viewport[1], grm_max(vp_y_min, min_vp), vp_y_max);
      element->setAttribute("_viewport_x_min_org", viewport[0]);
      element->setAttribute("_viewport_x_max_org", viewport[1]);
      element->setAttribute("_viewport_y_min_org", grm_max(vp_y_min, min_vp));
      element->setAttribute("_viewport_y_max_org", vp_y_max);

      if (strEqualsAny(element->localName(), "side_region", "side_plot_region"))
        {
          // 0.025 for tick-label at min/max viewport which will be longer than the normal min/max viewport
          auto vp_x_min = viewport[0] - 0.025 * (plot_vp[1] - plot_vp[0]);
          auto vp_x_max = viewport[1] + 0.025 * (plot_vp[1] - plot_vp[0]);

          render->setViewport(element, vp_x_min, vp_x_max, grm_max(vp_y_min, min_vp), vp_y_max);
          element->setAttribute("_viewport_x_min_org", vp_x_min);
          element->setAttribute("_viewport_x_max_org", vp_x_max);
        }
    }
}

void processClipRegion(const std::shared_ptr<GRM::Element> &element)
{
  gr_setclipregion(static_cast<int>(element->getAttribute("clip_region")));
}

void processCharExpan(const std::shared_ptr<GRM::Element> &element)
{
  gr_setcharexpan(static_cast<double>(element->getAttribute("char_expan")));
}

void processCharHeight(const std::shared_ptr<GRM::Element> &element)
{
  gr_setcharheight(static_cast<double>(element->getAttribute("char_height")));
}

void processCharSpace(const std::shared_ptr<GRM::Element> &element)
{
  gr_setcharspace(static_cast<double>(element->getAttribute("char_space")));
}

void processCharUp(const std::shared_ptr<GRM::Element> &element)
{
  gr_setcharup(static_cast<double>(element->getAttribute("char_up_x")),
               static_cast<double>(element->getAttribute("char_up_y")));
}

void processMarginalHeatmapKind(const std::shared_ptr<GRM::Element> &element)
{
  auto mkind = static_cast<std::string>(element->getAttribute("marginal_heatmap_kind"));
  auto global_root = grm_get_document_root();
  auto global_creator = grm_get_creator();
  auto global_render = grm_get_render();

  for (const auto &side_region : element->children())
    {
      if (!side_region->hasAttribute("marginal_heatmap_side_plot") ||
          !side_region->querySelectors("side_plot_region") ||
          static_cast<int>(element->getAttribute("_delete_children")) >= 2)
        continue;
      if (mkind == "line")
        {
          for (const auto &series : side_region->querySelectors("side_plot_region")->children())
            {
              if (!series->hasAttribute("x")) continue;
              int i, x_offset = 0, y_offset = 0;
              auto x_ind = static_cast<int>(element->getAttribute("x_ind"));
              auto y_ind = static_cast<int>(element->getAttribute("y_ind"));
              if (x_ind == -1 || y_ind == -1)
                {
                  series->remove();
                  continue;
                }
              double y_max = 0;
              std::shared_ptr<GRM::Context> context;
              std::shared_ptr<GRM::Render> render;
              std::shared_ptr<GRM::Element> line_elem, marker_elem;

              render = std::dynamic_pointer_cast<GRM::Render>(side_region->ownerDocument());
              if (!render) throw NotFoundError("Render-document not found for element\n");
              context = render->getContext();
              auto plot_group = element->parentElement();
              getPlotParent(plot_group);

              auto location = static_cast<std::string>(side_region->getAttribute("location"));
              auto c_max = static_cast<double>(plot_group->getAttribute("_z_lim_max"));
              auto xmin = static_cast<double>(element->getAttribute("x_range_min"));
              auto xmax = static_cast<double>(element->getAttribute("x_range_max"));
              auto ymin = static_cast<double>(element->getAttribute("y_range_min"));
              auto ymax = static_cast<double>(element->getAttribute("y_range_max"));

              auto marker_x_min = xmin;
              if (plot_group->hasAttribute("x_lim_min"))
                marker_x_min = static_cast<double>(plot_group->getAttribute("x_lim_min"));
              auto marker_x_max = xmax;
              if (plot_group->hasAttribute("x_lim_max"))
                marker_x_max = static_cast<double>(plot_group->getAttribute("x_lim_max"));
              auto marker_y_min = ymin;
              if (plot_group->hasAttribute("y_lim_min"))
                marker_y_min = static_cast<double>(plot_group->getAttribute("y_lim_min"));
              auto marker_y_max = ymax;
              if (plot_group->hasAttribute("y_lim_max"))
                marker_y_max = static_cast<double>(plot_group->getAttribute("y_lim_max"));

              auto z = GRM::get<std::vector<double>>((*context)[static_cast<std::string>(element->getAttribute("z"))]);
              auto y = GRM::get<std::vector<double>>((*context)[static_cast<std::string>(element->getAttribute("y"))]);
              auto x = GRM::get<std::vector<double>>((*context)[static_cast<std::string>(element->getAttribute("x"))]);
              auto y_length = static_cast<int>(y.size());
              auto x_length = static_cast<int>(x.size());
              auto y_dummy = std::vector<double>(((location == "right") ? (y_length - y_offset) : x_length), 0);

              if (plot_group->hasAttribute("x_log") && static_cast<int>(plot_group->getAttribute("x_log")))
                x_offset = static_cast<int>(x.size()) - x_length;
              if (plot_group->hasAttribute("y_log") && static_cast<int>(plot_group->getAttribute("y_log")))
                {
                  for (i = 0; i < y_length; i++)
                    {
                      if (grm_isnan(y[i])) y_offset += 1;
                    }
                }

              // plot step in marginal
              for (i = 0; i < ((location == "right") ? (y_length - y_offset) : x_length); i++)
                {
                  if (location == "right")
                    {
                      y_dummy[i] = std::isnan(z[(x_ind + x_offset) + (i + y_offset) * (x_length + x_offset)])
                                       ? 0
                                       : z[(x_ind + x_offset) + (i + y_offset) * (x_length + x_offset)];
                      y_max = grm_max(y_max, y_dummy[i]);
                    }
                  else
                    {
                      y_dummy[i] = std::isnan(z[(x_length + x_offset) * (y_ind + y_offset) + (i + x_offset)])
                                       ? 0
                                       : z[(x_length + x_offset) * (y_ind + y_offset) + (i + x_offset)];
                      y_max = grm_max(y_max, y_dummy[i]);
                    }
                }
              for (i = 0; i < ((location == "right") ? (y_length - y_offset) : x_length); i++)
                {
                  // + 0.01 and + 0.5 to prevent line clipping, gathered through testing
                  y_dummy[i] = (y_dummy[i] / y_max + 0.01) * (c_max / (15 + 0.5));
                }

              double x_pos, y_pos;
              unsigned int len = (location == "right") ? (y_length - y_offset) : x_length;
              std::vector<double> x_step_boundaries(2 * len);
              std::vector<double> y_step_values(2 * len);

              x_step_boundaries[0] = (location == "right") ? ymin : xmin;
              for (i = 2; i < 2 * len; i += 2)
                {
                  x_step_boundaries[i - 1] = x_step_boundaries[i] =
                      x_step_boundaries[0] + int(i / 2) * ((location == "right") ? (ymax - ymin) : (xmax - xmin)) / len;
                }
              x_step_boundaries[2 * len - 1] = (location == "right") ? ymax : xmax;
              y_step_values[0] = y_dummy[0];
              for (i = 2; i < 2 * len; i += 2)
                {
                  y_step_values[i - 1] = y_dummy[i / 2 - 1];
                  y_step_values[i] = y_dummy[i / 2];
                }
              y_step_values[2 * len - 1] = y_dummy[len - 1];

              auto id = static_cast<int>(global_root->getAttribute("_id"));
              global_root->setAttribute("_id", id + 1);
              auto id_str = std::to_string(id);

              // special case where it shouldn't be necessary to use the delete attribute from the children
              if (!series->hasChildNodes())
                {
                  if (location == "right")
                    {
                      line_elem =
                          global_creator->createPolyline("x" + id_str, y_step_values, "y" + id_str, x_step_boundaries);
                      x_pos = (x_step_boundaries[y_ind * 2] + x_step_boundaries[y_ind * 2 + 1]) / 2;
                      y_pos = y_dummy[y_ind];
                      marker_elem = global_creator->createPolymarker(y_pos, x_pos);
                    }
                  else
                    {
                      line_elem =
                          global_creator->createPolyline("x" + id_str, x_step_boundaries, "y" + id_str, y_step_values);
                      x_pos = (x_step_boundaries[x_ind * 2] + x_step_boundaries[x_ind * 2 + 1]) / 2;
                      y_pos = y_dummy[x_ind];
                      marker_elem = global_creator->createPolymarker(x_pos, y_pos);
                    }

                  if (!marker_elem->hasAttribute("_line_color_ind_set_by_user"))
                    global_render->setLineColorInd(line_elem, 989);
                  if (!marker_elem->hasAttribute("_marker_color_ind_set_by_user"))
                    global_render->setMarkerColorInd(marker_elem, 2);
                  if (!marker_elem->hasAttribute("_marker_type_set_by_user"))
                    global_render->setMarkerType(marker_elem, -1);
                  auto marker_size = 1.5 * (len / ((location == "right") ? (marker_y_max - marker_y_min)
                                                                         : (marker_x_max - marker_x_min)));
                  global_render->setMarkerSize(marker_elem, grm_min(marker_size, 2));

                  marker_elem->setAttribute("name", "marginal line");
                  line_elem->setAttribute("name", "marginal line");
                  series->append(marker_elem);
                  series->append(line_elem);
                  marker_elem->setAttribute("z_index", 2);
                }
              else
                {
                  for (const auto &child : series->children())
                    {
                      if (child->localName() == "polyline")
                        {
                          std::string x_key = "x" + id_str;
                          std::string y_key = "y" + id_str;
                          if (location == "right")
                            {
                              (*context)[x_key] = y_step_values;
                              (*context)[y_key] = x_step_boundaries;
                            }
                          else
                            {
                              (*context)[y_key] = y_step_values;
                              (*context)[x_key] = x_step_boundaries;
                            }
                          child->setAttribute("x", x_key);
                          child->setAttribute("y", y_key);
                        }
                      else if (child->localName() == "polymarker")
                        {
                          if (location == "right")
                            {
                              x_pos = (x_step_boundaries[y_ind * 2] + x_step_boundaries[y_ind * 2 + 1]) / 2;
                              y_pos = y_dummy[y_ind];
                              child->setAttribute("x", y_pos);
                              child->setAttribute("y", x_pos);
                            }
                          else
                            {
                              x_pos = (x_step_boundaries[x_ind * 2] + x_step_boundaries[x_ind * 2 + 1]) / 2;
                              y_pos = y_dummy[x_ind];
                              child->setAttribute("x", x_pos);
                              child->setAttribute("y", y_pos);
                            }
                          auto marker_size = 1.5 * (len / ((location == "right") ? (marker_y_max - marker_y_min)
                                                                                 : (marker_x_max - marker_x_min)));
                          global_render->setMarkerSize(child, grm_min(marker_size, 2));
                        }
                    }
                }
            }
        }
      else if (mkind == "all")
        {
          for (const auto &series : side_region->querySelectors("side_plot_region")->children())
            {
              int cnt = 0;
              auto x_ind = static_cast<int>(element->getAttribute("x_ind"));
              auto y_ind = static_cast<int>(element->getAttribute("y_ind"));

              if (x_ind == -1 && y_ind == -1)
                {
                  for (const auto &bar : series->children())
                    {
                      for (const auto &rect : bar->children())
                        {
                          if (rect->hasAttribute("line_color_ind")) continue;
                          rect->setAttribute("fill_color_ind", 989);
                        }
                    }
                  continue;
                }

              bool is_horizontal =
                  static_cast<std::string>(series->parentElement()->getAttribute("orientation")) == "horizontal";
              std::vector<std::shared_ptr<GRM::Element>> bar_groups = series->children();

              if ((is_horizontal && x_ind == -1) || (!is_horizontal && y_ind == -1)) continue;
              if ((is_horizontal ? x_ind : y_ind) >= bar_groups.size()) continue;

              for (const auto &group : bar_groups)
                {
                  for (const auto &rect : group->children())
                    {
                      if (rect->hasAttribute("line_color_ind")) continue;
                      rect->setAttribute("fill_color_ind", (cnt == (is_horizontal ? x_ind : y_ind)) ? 2 : 989);
                      cnt += 1;
                    }
                }
            }
        }
    }
}

void processResetRotation(const std::shared_ptr<GRM::Element> &element)
{
  if (element->hasAttribute("_space_3d_phi_org") && element->hasAttribute("_space_3d_theta_org"))
    {
      auto phi = static_cast<double>(element->getAttribute("_space_3d_phi_org"));
      auto theta = static_cast<double>(element->getAttribute("_space_3d_theta_org"));
      element->setAttribute("space_3d_phi", phi);
      element->setAttribute("space_3d_theta", theta);
    }
  element->removeAttribute("reset_rotation");
}

void processLineColorInd(const std::shared_ptr<GRM::Element> &element)
{
  gr_setlinecolorind(static_cast<int>(element->getAttribute("line_color_ind")));
}

void processLineSpec(const std::shared_ptr<GRM::Element> &element)
{
  if (element->localName() != "series_line" && element->localName() != "series_stairs")
    gr_uselinespec((static_cast<std::string>(element->getAttribute("line_spec"))).data());
}

void processLineType(const std::shared_ptr<GRM::Element> &element)
{
  int line_type = 1;
  if (element->getAttribute("line_type").isInt())
    {
      line_type = static_cast<int>(element->getAttribute("line_type"));
    }
  else if (element->getAttribute("line_type").isString())
    {
      line_type = GRM::lineTypeStringToInt(static_cast<std::string>(element->getAttribute("line_type")));
    }
  gr_setlinetype(line_type);
}

void processLineWidth(const std::shared_ptr<GRM::Element> &element)
{
  gr_setlinewidth(static_cast<double>(element->getAttribute("line_width")));
}

void processMarkerColorInd(const std::shared_ptr<GRM::Element> &element)
{
  gr_setmarkercolorind(static_cast<int>(element->getAttribute("marker_color_ind")));
}

void processMarkerSize(const std::shared_ptr<GRM::Element> &element)
{
  gr_setmarkersize(static_cast<double>(element->getAttribute("marker_size")));
}

void processMarkerType(const std::shared_ptr<GRM::Element> &element)
{
  int marker_type = 1;
  if (element->getAttribute("marker_type").isInt())
    {
      marker_type = static_cast<int>(element->getAttribute("marker_type"));
    }
  else if (element->getAttribute("marker_type").isString())
    {
      marker_type = GRM::markerTypeStringToInt(static_cast<std::string>(element->getAttribute("marker_type")));
    }
  gr_setmarkertype(marker_type);
}

void processResampleMethod(const std::shared_ptr<GRM::Element> &element)
{
  unsigned int resample_method_flag;
  if (!element->getAttribute("resample_method").isInt())
    {
      if (auto resample_method_str = static_cast<std::string>(element->getAttribute("resample_method"));
          resample_method_str == "nearest")
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
      resample_method_flag = static_cast<int>(element->getAttribute("resample_method"));
    }
  gr_setresamplemethod(resample_method_flag);
}

void processScale(const std::shared_ptr<GRM::Element> &element)
{
  gr_setscale(static_cast<int>(element->getAttribute("scale")));
}

void processSelectSpecificXform(const std::shared_ptr<GRM::Element> &element)
{
  gr_selntran(static_cast<int>(element->getAttribute("select_specific_xform")));
}

void processSpace(const std::shared_ptr<GRM::Element> &element)
{
  auto z_min = static_cast<double>(element->getAttribute("space_z_min"));
  auto z_max = static_cast<double>(element->getAttribute("space_z_max"));
  auto rotation = static_cast<int>(element->getAttribute("space_rotation"));
  auto tilt = static_cast<int>(element->getAttribute("space_tilt"));

  gr_setspace(z_min, z_max, rotation, tilt);
}

void processSpace3d(const std::shared_ptr<GRM::Element> &element)
{
  double phi = PLOT_DEFAULT_ROTATION, theta = PLOT_DEFAULT_TILT;

  if (element->hasAttribute("space_3d_phi"))
    {
      phi = static_cast<double>(element->getAttribute("space_3d_phi"));
    }
  else
    {
      element->setAttribute("space_3d_phi", phi);
    }
  if (element->hasAttribute("space_3d_theta"))
    {
      theta = static_cast<double>(element->getAttribute("space_3d_theta"));
    }
  else
    {
      element->setAttribute("space_3d_theta", theta);
    }
  // save the original plot rotation so it can be restored
  if (element->hasAttribute("space_3d_phi") && !element->hasAttribute("_space_3d_phi_org"))
    element->setAttribute("_space_3d_phi_org", static_cast<double>(element->getAttribute("space_3d_phi")));
  if (element->hasAttribute("space_3d_theta") && !element->hasAttribute("_space_3d_theta_org"))
    element->setAttribute("_space_3d_theta_org", static_cast<double>(element->getAttribute("space_3d_theta")));
  auto fov = static_cast<double>(element->getAttribute("space_3d_fov"));
  auto camera_distance = static_cast<double>(element->getAttribute("space_3d_camera_distance"));

  gr_setspace3d(-phi, theta, fov, camera_distance);
}

void processTextAlign(const std::shared_ptr<GRM::Element> &element)
{
  int text_align_vertical = 0, text_align_horizontal = 0;
  if (element->getAttribute("text_align_vertical").isInt())
    {
      text_align_vertical = static_cast<int>(element->getAttribute("text_align_vertical"));
    }
  else if (element->getAttribute("text_align_vertical").isString())
    {
      text_align_vertical =
          GRM::textAlignVerticalStringToInt(static_cast<std::string>(element->getAttribute("text_align_vertical")));
    }
  if (element->getAttribute("text_align_horizontal").isInt())
    {
      text_align_horizontal = static_cast<int>(element->getAttribute("text_align_horizontal"));
    }
  else if (element->getAttribute("text_align_horizontal").isString())
    {
      text_align_horizontal =
          GRM::textAlignHorizontalStringToInt(static_cast<std::string>(element->getAttribute("text_align_horizontal")));
    }
  gr_settextalign(text_align_horizontal, text_align_vertical);
}

void processTextColorInd(const std::shared_ptr<GRM::Element> &element)
{
  gr_settextcolorind(static_cast<int>(element->getAttribute("text_color_ind")));
}

void processTextColorForBackground(const std::shared_ptr<GRM::Element> &element)
/*  The set_text_color_for_background function used in plot.cxx now as an attribute function
    It is now possible to inquire colors during runtime -> No colors are given as parameters
    The new color is set on `element`
    There are no params apart from element
    \param[in] element The GRM::Element the color should be set in. Also contains other attributes which may function as
 parameters

    Attributes as Parameters (with prefix "stcfb-"):
    plot: for which plot it is used: right now only pie plot
 */
{
  int color_ind;
  unsigned char color_rgb[4];
  std::shared_ptr<GRM::Element> plot_parent = element;
  getPlotParent(plot_parent);

  if (!static_cast<int>(element->getAttribute("set_text_color_for_background"))) return;
  if (element->hasAttribute("_text_color_ind_set_by_user")) return;

  double r, g, b;
  int text_color_ind = 1;

  if (auto render = std::dynamic_pointer_cast<GRM::Render>(element->ownerDocument()); !render)
    throw NotFoundError("Render-document not found for element\n");

  gr_inqfillcolorind(&color_ind);
  gr_inqcolor(color_ind, (int *)color_rgb);

  r = color_rgb[0] / 255.0;
  g = color_rgb[1] / 255.0;
  b = color_rgb[2] / 255.0;

  if (auto color_lightness = getLightnessFromRGB(r, g, b); color_lightness < 0.4) text_color_ind = 0;
  element->setAttribute("text_color_ind", text_color_ind);
  processTextColorInd(element);
}

void processTextEncoding(const std::shared_ptr<GRM::Element> &element)
{
  int text_encoding = 301;
  if (element->getAttribute("text_encoding").isInt())
    {
      text_encoding = static_cast<int>(element->getAttribute("text_encoding"));
    }
  else if (element->getAttribute("text_encoding").isString())
    {
      text_encoding = GRM::textEncodingStringToInt(static_cast<std::string>(element->getAttribute("text_encoding")));
    }
  gr_settextencoding(text_encoding);
}

void processTransparency(const std::shared_ptr<GRM::Element> &element)
{
  auto global_root = grm_get_document_root();
  double transparency = 1.0;
  if (global_root->querySelectors("[_highlighted=\"1\"]")) gr_inqtransparency(&transparency);
  gr_settransparency(transparency * static_cast<double>(element->getAttribute("transparency")));
}

void processWSViewport(const std::shared_ptr<GRM::Element> &element)
{
  /*!
   * processing function for gr_wsviewport
   *
   * \param[in] element The GRM::Element that contains the attributes
   */
  double ws_viewport[4];
  ws_viewport[0] = static_cast<double>(element->getAttribute("ws_viewport_x_min"));
  ws_viewport[1] = static_cast<double>(element->getAttribute("ws_viewport_x_max"));
  ws_viewport[2] = static_cast<double>(element->getAttribute("ws_viewport_y_min"));
  ws_viewport[3] = static_cast<double>(element->getAttribute("ws_viewport_y_max"));

  gr_setwsviewport(ws_viewport[0], ws_viewport[1], ws_viewport[2], ws_viewport[3]);
}

void processWSWindow(const std::shared_ptr<GRM::Element> &element)
{
  double ws_window[4];
  ws_window[0] = static_cast<double>(element->getAttribute("ws_window_x_min"));
  ws_window[1] = static_cast<double>(element->getAttribute("ws_window_x_max"));
  ws_window[2] = static_cast<double>(element->getAttribute("ws_window_y_min"));
  ws_window[3] = static_cast<double>(element->getAttribute("ws_window_y_max"));

  gr_setwswindow(ws_window[0], ws_window[1], ws_window[2], ws_window[3]);
}

void processViewport(const std::shared_ptr<GRM::Element> &element)
{
  /*!
   * processing function for viewport
   *
   * \param[in] element The GRM::Element that contains the attributes
   */
  double viewport[4];

  // Todo: the getViewport method should be used here if root, figure are excluded and tick_group isn't a problem
  viewport[0] = static_cast<double>(element->getAttribute("viewport_x_min"));
  viewport[1] = static_cast<double>(element->getAttribute("viewport_x_max"));
  viewport[2] = static_cast<double>(element->getAttribute("viewport_y_min"));
  viewport[3] = static_cast<double>(element->getAttribute("viewport_y_max"));

  // TODO: Change this workaround when all elements with viewports really have a valid viewport
  if (viewport[1] - viewport[0] > 0.0 && viewport[3] - viewport[2] > 0.0)
    {
      if (element->localName() == "axis")
        {
          double vp[4], new_vp[4];
          std::string location;
          auto plot_parent = element;
          getPlotParent(plot_parent);

          auto ref_vp_element = plot_parent->querySelectors("central_region");
          if (strEqualsAny(element->parentElement()->localName(), "colorbar", "side_plot_region"))
            ref_vp_element = element->parentElement();

          if (element->hasAttribute("location"))
            location = static_cast<std::string>(element->getAttribute("location"));
          else if (ref_vp_element->hasAttribute("location"))
            location = static_cast<std::string>(ref_vp_element->getAttribute("location"));
          else if (ref_vp_element->parentElement()->hasAttribute("location"))
            location = static_cast<std::string>(ref_vp_element->parentElement()->getAttribute("location"));
          else if (ref_vp_element->parentElement()->parentElement()->hasAttribute("location"))
            location =
                static_cast<std::string>(ref_vp_element->parentElement()->parentElement()->getAttribute("location"));

          if (ref_vp_element->localName() == "central_region")
            {
              if (!GRM::Render::getViewport(element->parentElement(), &vp[0], &vp[1], &vp[2], &vp[3]))
                throw NotFoundError("Central region doesn't have a viewport but it should.\n");
            }
          else
            {
              if (!GRM::Render::getViewport(ref_vp_element, &vp[0], &vp[1], &vp[2], &vp[3]))
                throw NotFoundError(ref_vp_element->localName() + " doesn't have a viewport but it should.\n");
            }

          bool mirrored_axis =
              element->hasAttribute("mirrored_axis") && static_cast<int>(element->getAttribute("mirrored_axis"));
          bool draw_grid = element->hasAttribute("draw_grid") && static_cast<int>(element->getAttribute("draw_grid"));

          if (mirrored_axis || draw_grid)
            {
              // x- and y-axis with gridline and mirrored ticks
              new_vp[0] = viewport[0] + (vp[0] - viewport[0]);
              new_vp[1] = viewport[1] + (vp[1] - viewport[1]);
              new_vp[2] = viewport[2] + (vp[2] - viewport[2]);
              new_vp[3] = viewport[3] + (vp[3] - viewport[3]);
            }
          else
            {
              bool down_ticks = element->hasAttribute("tick_orientation") &&
                                static_cast<int>(element->getAttribute("tick_orientation")) < 0;
              if (strEqualsAny(location, "left", "right"))
                {
                  if (ref_vp_element->localName() == "side_plot_region")
                    {
                      if (location == "left" && !down_ticks)
                        new_vp[0] = vp[0];
                      else
                        new_vp[0] = viewport[0] + (vp[0] - viewport[0]);
                    }
                  else
                    new_vp[0] = viewport[0];
                  if (ref_vp_element->localName() == "side_plot_region")
                    {
                      if (location == "right" && down_ticks)
                        new_vp[1] = vp[1];
                      else
                        new_vp[1] = viewport[1] + (vp[1] - viewport[1]);
                    }
                  else
                    new_vp[1] = viewport[1];
                  new_vp[2] = viewport[2] + (vp[2] - viewport[2]);
                  new_vp[3] = viewport[3] + (vp[3] - viewport[3]);
                }
              else if (strEqualsAny(location, "bottom", "top"))
                {
                  new_vp[0] = viewport[0] + (vp[0] - viewport[0]);
                  new_vp[1] = viewport[1] + (vp[1] - viewport[1]);
                  if (ref_vp_element->localName() == "side_plot_region")
                    {
                      if (location == "bottom" && !down_ticks)
                        new_vp[2] = vp[2];
                      else
                        new_vp[2] = viewport[2] + (vp[2] - viewport[2]);
                    }
                  else
                    new_vp[2] = viewport[2];
                  if (ref_vp_element->localName() == "side_plot_region")
                    {
                      if (location == "top" && down_ticks)
                        new_vp[3] = vp[3];
                      else
                        new_vp[3] = viewport[3] + (vp[3] - viewport[3]);
                    }
                  else
                    new_vp[3] = viewport[3];
                }
              else if (strEqualsAny(location, "x", "twin_x"))
                {
                  new_vp[0] = viewport[0] + (vp[0] - viewport[0]);
                  new_vp[1] = viewport[1] + (vp[1] - viewport[1]);
                  if (location == "x")
                    {
                      new_vp[2] = viewport[2] + (vp[2] - viewport[2]);
                      new_vp[3] = vp[3];
                    }
                  else
                    {
                      new_vp[2] = vp[2];
                      new_vp[3] = viewport[3] + (vp[3] - viewport[3]);
                    }
                }
              else
                {
                  if (location == "y")
                    {
                      new_vp[0] = viewport[0] + (vp[0] - viewport[0]);
                      new_vp[1] = vp[1];
                    }
                  else
                    {
                      new_vp[0] = vp[0];
                      new_vp[1] = viewport[1] + (vp[1] - viewport[1]);
                    }
                  new_vp[2] = viewport[2] + (vp[2] - viewport[2]);
                  new_vp[3] = viewport[3] + (vp[3] - viewport[3]);
                }
            }
          gr_setviewport(new_vp[0], new_vp[1], new_vp[2], new_vp[3]);
        }
      else if (strEqualsAny(element->localName(), "central_region", "side_region", "side_plot_region", "colorbar"))
        {
          double vp_x_min, vp_x_max, vp_y_min, vp_y_max;

          if (!GRM::Render::getViewport(element, &vp_x_min, &vp_x_max, &vp_y_min, &vp_y_max))
            throw NotFoundError("'" + element->localName() + "' doesn't have a viewport but it should.\n");

          if (element->localName() != "side_region" || element->hasChildNodes())
            gr_setviewport(vp_x_min, vp_x_max, vp_y_min, vp_y_max);
        }
      else if (element->localName() == "coordinate_system")
        {
          double vp_x_min, vp_x_max, vp_y_min, vp_y_max;

          if (!GRM::Render::getViewport(element->parentElement(), &vp_x_min, &vp_x_max, &vp_y_min, &vp_y_max))
            throw NotFoundError("Central region doesn't have a viewport but it should.\n");
          gr_setviewport(vp_x_min, vp_x_max, vp_y_min, vp_y_max);
        }
      else
        {
          gr_setviewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        }
    }
}

void processBackgroundColor(const std::shared_ptr<GRM::Element> &element)
{
  if (element->hasAttribute("background_color"))
    {
      double vp[4];
      std::shared_ptr<GRM::Element> plot_elem = element;
      getPlotParent(plot_elem);
      if (plot_elem->parentElement()->localName() == "layout_grid_element") plot_elem = plot_elem->parentElement();

      if (!GRM::Render::getViewport(element, &vp[0], &vp[1], &vp[2], &vp[3])) return;

      auto background_color_index = static_cast<int>(element->getAttribute("background_color"));
      gr_savestate();
      gr_selntran(0);
      gr_setfillintstyle(GKS_K_INTSTYLE_SOLID);
      gr_setfillcolorind(background_color_index);
      if (grm_get_render()->getRedrawWs()) gr_fillrect(vp[0], vp[1], vp[2], vp[3]);
      gr_selntran(1);
      gr_restorestate();
    }
}

void processBorderColorInd(const std::shared_ptr<GRM::Element> &element)
{
  gr_setbordercolorind(static_cast<int>(element->getAttribute("border_color_ind")));
}

void processBorderWidth(const std::shared_ptr<GRM::Element> &element)
{
  gr_setborderwidth(static_cast<double>(element->getAttribute("border_width")));
}

void processMarginalHeatmapSidePlot(const std::shared_ptr<GRM::Element> &element)
{
  double window[4];
  double x_min, x_max, y_min, y_max, c_max;
  auto kind = static_cast<std::string>(element->getAttribute("kind"));

  if (element->parentElement()->localName() == "marginal_heatmap_plot" &&
      element->parentElement()->hasAttribute("marginal_heatmap_kind"))
    {
      auto location = static_cast<std::string>(element->getAttribute("location"));
      auto plot_parent = element->parentElement();
      getPlotParent(plot_parent);
      applyMoveTransformation(element);
      x_min = static_cast<double>(plot_parent->getAttribute("_x_lim_min"));
      x_max = static_cast<double>(plot_parent->getAttribute("_x_lim_max"));
      y_min = static_cast<double>(plot_parent->getAttribute("_y_lim_min"));
      y_max = static_cast<double>(plot_parent->getAttribute("_y_lim_max"));
      if (!std::isnan(static_cast<double>(plot_parent->getAttribute("_c_lim_max"))))
        {
          c_max = static_cast<double>(plot_parent->getAttribute("_c_lim_max"));
        }
      else
        {
          c_max = static_cast<double>(plot_parent->getAttribute("_z_lim_max"));
        }

      if (element->hasAttribute("window_x_min")) window[0] = static_cast<double>(element->getAttribute("window_x_min"));
      if (element->hasAttribute("window_x_max")) window[1] = static_cast<double>(element->getAttribute("window_x_max"));
      if (element->hasAttribute("window_y_min")) window[2] = static_cast<double>(element->getAttribute("window_y_min"));
      if (element->hasAttribute("window_y_max")) window[3] = static_cast<double>(element->getAttribute("window_y_max"));

      if (!element->hasAttribute("_window_set_by_user"))
        {
          if (location == "right")
            {
              window[0] = 0.0;
              window[1] = c_max / 15;
              window[2] = y_min;
              window[3] = y_max;
            }
          else if (location == "top")
            {
              window[0] = x_min;
              window[1] = x_max;
              window[2] = 0.0;
              window[3] = c_max / 15;
            }
        }
      grm_get_render()->setWindow(element, window[0], window[1], window[2], window[3]);
      GRM::processWindow(element);
      calculateViewport(element);
      applyMoveTransformation(element);
      if (element->querySelectors("side_plot_region"))
        {
          auto side_plot_region = element->querySelectors("side_plot_region");
          calculateViewport(side_plot_region);
          applyMoveTransformation(side_plot_region);
        }
    }
}

void processColormap(const std::shared_ptr<GRM::Element> &element)
{
  int colormap = PLOT_DEFAULT_COLORMAP;
  if (element->getAttribute("colormap").isInt())
    {
      colormap = static_cast<int>(element->getAttribute("colormap"));
    }
  else if (element->getAttribute("colormap").isString())
    {
      colormap = GRM::colormapStringToInt(static_cast<std::string>(element->getAttribute("colormap")));
    }

  if (element->hasAttribute("colormap_inverted") && static_cast<int>(element->getAttribute("colormap_inverted")))
    colormap *= -1;

  gr_setcolormap(colormap);
}

void processColorRep(const std::shared_ptr<GRM::Element> &element, const std::string &attribute)
{
  int index, hex_int;
  double red, green, blue;
  std::string name, hex_string;
  std::stringstream string_stream;

  auto end = attribute.find('.');
  index = std::stoi(attribute.substr(end + 1, attribute.size()));

  hex_int = 0;
  hex_string = static_cast<std::string>(element->getAttribute(attribute));
  string_stream << std::hex << hex_string;
  string_stream >> hex_int;

  red = ((hex_int >> 16) & 0xFF) / 255.0;
  green = ((hex_int >> 8) & 0xFF) / 255.0;
  blue = ((hex_int)&0xFF) / 255.0;

  gr_setcolorrep(index, red, green, blue);
}

void processColorReps(const std::shared_ptr<GRM::Element> &element)
{
  for (auto &attr : element->getAttributeNames())
    {
      if (attr.substr(0U, attr.find('.')) == "colorrep") processColorRep(element, attr);
    }
}

void processFillColorInd(const std::shared_ptr<GRM::Element> &element)
{
  gr_setfillcolorind(static_cast<int>(element->getAttribute("fill_color_ind")));
}

void processFillIntStyle(const std::shared_ptr<GRM::Element> &element)
{
  int fill_int_style = 1;
  if (element->getAttribute("fill_int_style").isInt())
    {
      fill_int_style = static_cast<int>(element->getAttribute("fill_int_style"));
    }
  else if (element->getAttribute("fill_int_style").isString())
    {
      fill_int_style = GRM::fillIntStyleStringToInt(static_cast<std::string>(element->getAttribute("fill_int_style")));
    }
  gr_setfillintstyle(fill_int_style);
}

void processFillStyle(const std::shared_ptr<GRM::Element> &element)
{
  int fill_style = 1;
  if (element->getAttribute("fill_style").isInt())
    {
      fill_style = static_cast<int>(element->getAttribute("fill_style"));
    }
  else if (element->getAttribute("fill_style").isString())
    {
      fill_style = GRM::fillStyleStringToInt(static_cast<std::string>(element->getAttribute("fill_style")));
    }
  gr_setfillstyle(fill_style);
}

void processFlip(const std::shared_ptr<GRM::Element> &element)
{
  int options;
  bool x_flip = static_cast<int>(element->getAttribute("x_flip"));
  bool y_flip = static_cast<int>(element->getAttribute("y_flip"));

  gr_inqscale(&options);
  if (x_flip)
    options = options | GR_OPTION_FLIP_X;
  else
    options = options & ~GR_OPTION_FLIP_X;
  if (y_flip)
    options = options | GR_OPTION_FLIP_Y;
  else
    options = options & ~GR_OPTION_FLIP_Y;
  gr_setscale(options);
}

void processFont(const std::shared_ptr<GRM::Element> &element)
{
  int font = PLOT_DEFAULT_FONT, font_precision = PLOT_DEFAULT_FONT_PRECISION;

  /* `font` and `font_precision` are always set */
  if (element->hasAttribute("font_precision"))
    {
      if (element->getAttribute("font_precision").isInt())
        {
          font_precision = static_cast<int>(element->getAttribute("font_precision"));
        }
      else if (element->getAttribute("font_precision").isString())
        {
          font_precision =
              GRM::fontPrecisionStringToInt(static_cast<std::string>(element->getAttribute("font_precision")));
        }
    }
  else
    {
      logger((stderr, "Use default font precision\n"));
    }
  if (element->hasAttribute("font"))
    {
      if (element->getAttribute("font").isInt())
        {
          font = static_cast<int>(element->getAttribute("font"));
        }
      else if (element->getAttribute("font").isString())
        {
          font = GRM::fontStringToInt(static_cast<std::string>(element->getAttribute("font")));
        }
    }
  else
    {
      logger((stderr, "Use default font\n"));
    }
  logger((stderr, "Using font: %d with precision %d\n", font, font_precision));
  gr_settextfontprec(font, font_precision);
  /* TODO: Implement other datatypes for `font` and `font_precision` */
}

void processZIndex(const std::shared_ptr<GRM::Element> &element)
{
  auto render = grm_get_render();
  if (!render->getZQueueIsBeingRendered())
    {
      auto z_index = static_cast<int>(element->getAttribute("z_index"));
      auto z_index_manager = render->getZIndexManager();
      z_index_manager->setZIndex(z_index);
    }
}

void processRefAxisLocation(const std::shared_ptr<GRM::Element> &element)
{
  std::shared_ptr<GRM::Element> plot_parent = element, coordinate_system;
  double window[4];
  std::string orientation = PLOT_DEFAULT_ORIENTATION;
  auto global_render = grm_get_render();

  getPlotParent(plot_parent);
  coordinate_system = plot_parent->querySelectors("coordinate_system");
  if (coordinate_system != nullptr && static_cast<std::string>(coordinate_system->getAttribute("plot_type")) == "2d" &&
      element->localName() != "series_pie")
    {
      if (element->parentElement()->hasAttribute("orientation"))
        orientation = static_cast<std::string>(element->parentElement()->getAttribute("orientation"));

      auto x_location = static_cast<std::string>(element->getAttribute("ref_x_axis_location"));
      if (x_location.empty()) x_location = "x";
      auto y_location = static_cast<std::string>(element->getAttribute("ref_y_axis_location"));
      if (y_location.empty()) y_location = "y";

      if (orientation == "vertical")
        {
          auto tmp = x_location;
          x_location = y_location;
          y_location = tmp;

          if (y_location == "twin_x") y_location = "twin_y";
          if (y_location == "top") y_location = "right";
          if (y_location == "bottom") y_location = "left";
          if (x_location == "twin_y") x_location = "twin_x";
          if (x_location == "right") x_location = "top";
          if (x_location == "left") x_location = "bottom";
        }

      auto ref_x_axis = plot_parent->querySelectors("axis[location=\"" + x_location + "\"]");
      if (!ref_x_axis->hasAttribute("window_x_min") || !ref_x_axis->hasAttribute("window_x_max"))
        processAxis(ref_x_axis, global_render->getContext());
      window[0] = static_cast<double>(ref_x_axis->getAttribute("window_x_min"));
      window[1] = static_cast<double>(ref_x_axis->getAttribute("window_x_max"));

      auto ref_y_axis = plot_parent->querySelectors("axis[location=\"" + y_location + "\"]");
      if (!ref_y_axis->hasAttribute("window_y_min") || !ref_y_axis->hasAttribute("window_y_max"))
        processAxis(ref_y_axis, global_render->getContext());
      window[2] = static_cast<double>(ref_y_axis->getAttribute("window_y_min"));
      window[3] = static_cast<double>(ref_y_axis->getAttribute("window_y_max"));

      gr_setwindow(window[0], window[1], window[2], window[3]);
    }
}

void processPrivateTransparency(const std::shared_ptr<GRM::Element> &element)
{
  auto render = grm_get_render();
  if (render->getHighlightedAttrExist())
    {
      if (!(element->hasAttribute("_highlighted") && static_cast<int>(element->getAttribute("_highlighted"))) &&
          !hasHighlightedParent(element))
        {
          gr_settransparency(0.4);
        }
      else
        {
          gr_settransparency(1.0);
        }
    }
}

void plotProcessWsWindowWsViewport(const std::shared_ptr<GRM::Element> &element,
                                   const std::shared_ptr<GRM::Context> &context)
{
  int pixel_width, pixel_height;
  double metric_width, metric_height;
  double ws_viewport[4] = {0.0, 0.0, 0.0, 0.0};
  double ws_window[4] = {0.0, 0.0, 0.0, 0.0};
  auto global_render = grm_get_render();
  auto active_figure = global_render->getActiveFigure();

  // set ws_window/ws_viewport on active figure
  GRM::getFigureSize(&pixel_width, &pixel_height, &metric_width, &metric_height);

  if (!active_figure->hasAttribute("_previous_pixel_width") || !active_figure->hasAttribute("_previous_pixel_height") ||
      static_cast<int>(active_figure->getAttribute("_previous_pixel_width")) != pixel_width ||
      static_cast<int>(active_figure->getAttribute("_previous_pixel_height")) != pixel_height)
    {
      /* TODO: handle error return value? */
      auto figure_id_str = static_cast<std::string>(active_figure->getAttribute("_figure_id"));
      if (startsWith(figure_id_str, "figure")) figure_id_str = figure_id_str.substr(6);
      auto figure_id = std::stoi(figure_id_str);
      eventQueueEnqueueSizeEvent(event_queue, figure_id, pixel_width, pixel_height);
    }

  auto aspect_ratio_ws_metric = metric_width / metric_height;
  if (!active_figure->hasAttribute("_ws_window_set_by_user") ||
      !static_cast<int>(active_figure->getAttribute("_ws_window_set_by_user")))
    {
      if (aspect_ratio_ws_metric > 1)
        {
          ws_window[1] = 1.0;
          ws_window[3] = 1.0 / (aspect_ratio_ws_metric);
        }
      else
        {
          ws_window[1] = aspect_ratio_ws_metric;
          ws_window[3] = 1.0;
        }
    }
  else
    {
      ws_window[0] = static_cast<double>(active_figure->getAttribute("ws_window_x_min"));
      ws_window[1] = static_cast<double>(active_figure->getAttribute("ws_window_x_max"));
      ws_window[2] = static_cast<double>(active_figure->getAttribute("ws_window_y_min"));
      ws_window[3] = static_cast<double>(active_figure->getAttribute("ws_window_y_max"));
    }
  if (!active_figure->hasAttribute("_ws_viewport_set_by_user") ||
      !static_cast<int>(active_figure->getAttribute("_ws_viewport_set_by_user")))
    {
      ws_viewport[1] = metric_width;
      ws_viewport[3] = metric_height;
    }
  else
    {
      ws_viewport[0] = static_cast<double>(active_figure->getAttribute("ws_viewport_x_min"));
      ws_viewport[1] = static_cast<double>(active_figure->getAttribute("ws_viewport_x_max"));
      ws_viewport[2] = static_cast<double>(active_figure->getAttribute("ws_viewport_y_min"));
      ws_viewport[3] = static_cast<double>(active_figure->getAttribute("ws_viewport_y_max"));
    }
  global_render->setWSViewport(active_figure, ws_viewport[0], ws_viewport[1], ws_viewport[2], ws_viewport[3]);
  global_render->setWSWindow(active_figure, ws_window[0], ws_window[1], ws_window[2], ws_window[3]);

  active_figure->setAttribute("_previous_pixel_width", pixel_width);
  active_figure->setAttribute("_previous_pixel_height", pixel_height);

  logger((stderr, "Stored ws_window (%lf, %lf, %lf, %lf)\n", ws_window[0], ws_window[1], ws_window[2], ws_window[3]));
  logger((stderr, "Stored ws_viewport (%lf, %lf, %lf, %lf)\n", ws_viewport[0], ws_viewport[1], ws_viewport[2],
          ws_viewport[3]));
}

void GRM::processLimits(const std::shared_ptr<GRM::Element> &element)
{
  /*!
   * processing function for gr_window
   *
   * \param[in] element The GRM::Element that contains the attributes
   */
  int adjust_x_lim, adjust_y_lim;
  int scale = 0;
  bool plot_reset_ranges = false;
  std::shared_ptr<GRM::Element> central_region;
  const auto kind = static_cast<std::string>(element->getAttribute("_kind"));
  auto global_render = grm_get_render();
  gr_inqscale(&scale);

  if (kind != "pie" && polar_kinds.count(kind) <= 0)
    {
      // doesn't work for polar-plots cause the set window isn't the 'real' window
      scale |= static_cast<int>(element->getAttribute("x_log")) ? GR_OPTION_X_LOG : 0;
      scale |= static_cast<int>(element->getAttribute("y_log")) ? GR_OPTION_Y_LOG : 0;
      scale |= static_cast<int>(element->getAttribute("z_log")) ? GR_OPTION_Z_LOG : 0;
      scale |= static_cast<int>(element->getAttribute("x_flip")) ? GR_OPTION_FLIP_X : 0;
      scale |= static_cast<int>(element->getAttribute("y_flip")) ? GR_OPTION_FLIP_Y : 0;
      scale |= static_cast<int>(element->getAttribute("z_flip")) ? GR_OPTION_FLIP_Z : 0;
    }
  element->setAttribute("scale", scale);

  auto xmin = static_cast<double>(element->getAttribute("_x_lim_min"));
  auto xmax = static_cast<double>(element->getAttribute("_x_lim_max"));
  auto ymin = static_cast<double>(element->getAttribute("_y_lim_min"));
  auto ymax = static_cast<double>(element->getAttribute("_y_lim_max"));

  if (element->hasAttribute("reset_ranges") && static_cast<int>(element->getAttribute("reset_ranges")))
    {
      if (element->hasAttribute("_original_x_min") && element->hasAttribute("_original_x_max") &&
          element->hasAttribute("_original_y_min") && element->hasAttribute("_original_y_max") &&
          element->hasAttribute("_original_adjust_x_lim") && element->hasAttribute("_original_adjust_y_lim"))
        {
          if (!(scale & GR_OPTION_X_LOG) || !element->hasAttribute("_no_x_reset_ranges"))
            {
              xmin = static_cast<double>(element->getAttribute("_original_x_min"));
              xmax = static_cast<double>(element->getAttribute("_original_x_max"));
              adjust_x_lim = static_cast<int>(element->getAttribute("_original_adjust_x_lim"));
              element->setAttribute("adjust_x_lim", adjust_x_lim);
            }
          if (!(scale & GR_OPTION_Y_LOG) || !element->hasAttribute("_no_y_reset_ranges"))
            {
              ymin = static_cast<double>(element->getAttribute("_original_y_min"));
              ymax = static_cast<double>(element->getAttribute("_original_y_max"));
              adjust_y_lim = static_cast<int>(element->getAttribute("_original_adjust_y_lim"));
              element->setAttribute("adjust_y_lim", adjust_y_lim);
            }
          element->removeAttribute("_original_x_lim");
          element->removeAttribute("_original_x_min");
          element->removeAttribute("_original_x_max");
          element->removeAttribute("_original_y_lim");
          element->removeAttribute("_original_y_min");
          element->removeAttribute("_original_y_max");
          element->removeAttribute("_original_adjust_x_lim");
          element->removeAttribute("_original_adjust_y_lim");
        }
      plot_reset_ranges = true;
      element->removeAttribute("reset_ranges");
    }
  // reset rotation
  for (const auto &plot_child : element->children())
    {
      if (plot_child->localName() == "marginal_heatmap_plot")
        {
          for (const auto &child : plot_child->children())
            {
              if (child->localName() == "central_region")
                {
                  central_region = child;
                  if (central_region->hasAttribute("reset_rotation")) processResetRotation(central_region);
                  break;
                }
            }
        }
      if (plot_child->localName() == "central_region")
        {
          central_region = plot_child;
          if (central_region->hasAttribute("reset_rotation")) processResetRotation(central_region);
          GRM::processWindow(central_region);
          processScale(element);
          break;
        }
    }

  if (element->hasAttribute("panzoom") && static_cast<int>(element->getAttribute("panzoom")))
    {
      gr_savestate();

      // when possible set the viewport of central_region, cause that's the one which is used to determinate the panzoom
      // before it gets set on the tree
      processViewport(central_region);

      if (!element->hasAttribute("_original_x_lim"))
        {
          element->setAttribute("_original_x_min", xmin);
          element->setAttribute("_original_x_max", xmax);
          element->setAttribute("_original_x_lim", true);
          adjust_x_lim = static_cast<int>(element->getAttribute("adjust_x_lim"));
          element->setAttribute("_original_adjust_x_lim", adjust_x_lim);
          element->setAttribute("adjust_x_lim", 0);
        }
      if (!element->hasAttribute("_original_y_lim"))
        {
          element->setAttribute("_original_y_min", ymin);
          element->setAttribute("_original_y_max", ymax);
          element->setAttribute("_original_y_lim", true);
          adjust_y_lim = static_cast<int>(element->getAttribute("adjust_y_lim"));
          element->setAttribute("_original_adjust_y_lim", adjust_y_lim);
          element->setAttribute("adjust_y_lim", 0);
        }
      auto panzoom_element = element->getElementsByTagName("panzoom")[0];
      auto x = static_cast<double>(panzoom_element->getAttribute("x"));
      auto y = static_cast<double>(panzoom_element->getAttribute("y"));
      auto x_zoom = static_cast<double>(panzoom_element->getAttribute("x_zoom"));
      auto y_zoom = static_cast<double>(panzoom_element->getAttribute("y_zoom"));

      /* Ensure the correct window is set in GRM */
      bool window_exists =
          (central_region->hasAttribute("window_x_min") && central_region->hasAttribute("window_x_max") &&
           central_region->hasAttribute("window_y_min") && central_region->hasAttribute("window_y_max"));
      if (window_exists)
        {
          auto stored_window_xmin = static_cast<double>(central_region->getAttribute("window_x_min"));
          auto stored_window_xmax = static_cast<double>(central_region->getAttribute("window_x_max"));
          auto stored_window_ymin = static_cast<double>(central_region->getAttribute("window_y_min"));
          auto stored_window_ymax = static_cast<double>(central_region->getAttribute("window_y_max"));

          if (stored_window_xmax - stored_window_xmin > 0.0 && stored_window_ymax - stored_window_ymin > 0.0)
            gr_setwindow(stored_window_xmin, stored_window_xmax, stored_window_ymin, stored_window_ymax);
        }
      else
        {
          throw NotFoundError("Window not found\n");
        }

      gr_panzoom(x, y, x_zoom, y_zoom, &xmin, &xmax, &ymin, &ymax);

      element->removeAttribute("panzoom");
      element->removeChild(panzoom_element);
      central_region->setAttribute("_zoomed", true);

      for (const auto &child : central_region->children())
        {
          if (startsWith(child->localName(), "series_")) resetOldBoundingBoxes(child);
        }
      gr_restorestate();
    }

  element->setAttribute("_x_lim_min", xmin);
  element->setAttribute("_x_lim_max", xmax);
  element->setAttribute("_y_lim_min", ymin);
  element->setAttribute("_y_lim_max", ymax);

  if (!(scale & GR_OPTION_X_LOG))
    {
      adjust_x_lim = static_cast<int>(element->getAttribute("adjust_x_lim"));
      if (auto x_axis = element->querySelectors("axis[location=\"x\"]");
          x_axis != nullptr && x_axis->hasAttribute("adjust_x_lim") && kinds_3d.count(kind) == 0 &&
          polar_kinds.count(kind) == 0)
        adjust_x_lim = static_cast<int>(x_axis->getAttribute("adjust_x_lim"));
      if (adjust_x_lim)
        {
          logger((stderr, "_x_lim before \"gr_adjustlimits\": (%lf, %lf)\n", xmin, xmax));
          gr_adjustlimits(&xmin, &xmax);
          logger((stderr, "_x_lim after \"gr_adjustlimits\": (%lf, %lf)\n", xmin, xmax));
        }
    }
  else
    {
      element->removeAttribute("_no_x_reset_ranges");
    }

  if (!(scale & GR_OPTION_Y_LOG))
    {
      adjust_y_lim = static_cast<int>(element->getAttribute("adjust_y_lim"));
      if (auto y_axis = element->querySelectors("axis[location=\"y\"]");
          y_axis != nullptr && y_axis->hasAttribute("adjust_y_lim") && kinds_3d.count(kind) == 0 &&
          polar_kinds.count(kind) == 0)
        adjust_y_lim = static_cast<int>(y_axis->getAttribute("adjust_y_lim"));
      if (adjust_y_lim)
        {
          logger((stderr, "_y_lim before \"gr_adjustlimits\": (%lf, %lf)\n", ymin, ymax));
          gr_adjustlimits(&ymin, &ymax);
          logger((stderr, "_y_lim after \"gr_adjustlimits\": (%lf, %lf)\n", ymin, ymax));
        }
    }
  else
    {
      element->removeAttribute("_no_y_reset_ranges");
    }

  if (strEqualsAny(kind, "wireframe", "surface", "line3", "scatter3", "trisurface", "volume", "isosurface"))
    {
      auto zmin = static_cast<double>(element->getAttribute("_z_lim_min"));
      auto zmax = static_cast<double>(element->getAttribute("_z_lim_max"));
      if (zmax > 0)
        {
          if (!(scale & GR_OPTION_Z_LOG))
            {
              auto adjust_z_lim = static_cast<int>(element->hasAttribute("adjust_z_lim"));
              if (adjust_z_lim)
                {
                  logger((stderr, "_z_lim before \"gr_adjustlimits\": (%lf, %lf)\n", zmin, zmax));
                  gr_adjustlimits(&zmin, &zmax);
                  logger((stderr, "_z_lim after \"gr_adjustlimits\": (%lf, %lf)\n", zmin, zmax));
                }
            }
          logger((stderr, "Storing window 3d (%lf, %lf, %lf, %lf, %lf, %lf)\n", xmin, xmax, ymin, ymax, zmin, zmax));
          if (!central_region->hasAttribute("_window_set_by_user"))
            global_render->setWindow3d(central_region, xmin, xmax, ymin, ymax, zmin, zmax);
        }
    }
  else
    {
      logger((stderr, "Storing window (%lf, %lf, %lf, %lf)\n", xmin, xmax, ymin, ymax));
      if (kind == "pie")
        {
          xmin = 0.0;
          xmax = 1.0;
          ymin = 0.0;
          ymax = 1.0;
        }
      if (!central_region->hasAttribute("_window_set_by_user"))
        global_render->setWindow(central_region, xmin, xmax, ymin, ymax);
    }
  if (plot_reset_ranges) central_region->setAttribute("_zoomed", true);
  GRM::processWindow(central_region);
  processScale(element);
}

void GRM::processWindow(const std::shared_ptr<GRM::Element> &element)
{
  auto xmin = static_cast<double>(element->getAttribute("window_x_min"));
  auto xmax = static_cast<double>(element->getAttribute("window_x_max"));
  auto ymin = static_cast<double>(element->getAttribute("window_y_min"));
  auto ymax = static_cast<double>(element->getAttribute("window_y_max"));
  auto global_render = grm_get_render();

  if (element->localName() == "central_region")
    {
      std::shared_ptr<GRM::Element> plot_element = element;
      getPlotParent(plot_element);
      auto kind = static_cast<std::string>(plot_element->getAttribute("_kind"));

      if (kind != "pie" && xmax - xmin > 0.0 && ymax - ymin > 0.0) gr_setwindow(xmin, xmax, ymin, ymax);
      if (kind == "pie") gr_setwindow(0, 1, 0, 1);
      if (strEqualsAny(kind, "wireframe", "surface", "line3", "scatter3", "trisurface", "volume", "isosurface"))
        {
          auto zmin = static_cast<double>(element->getAttribute("window_z_min"));
          auto zmax = static_cast<double>(element->getAttribute("window_z_max"));

          gr_setwindow3d(xmin, xmax, ymin, ymax, zmin, zmax);
        }
      if (element->hasAttribute("_zoomed") && static_cast<int>(element->getAttribute("_zoomed")))
        {
          for (const auto &axis : plot_element->querySelectorsAll("axis"))
            {
              if (axis->parentElement()->localName() == "colorbar") continue;
              clearAxisAttributes(axis);
              processAxis(axis, global_render->getContext());
            }
          for (const auto &axis : element->querySelectorsAll("radial_axes"))
            {
              processRadialAxes(axis, global_render->getContext());
            }
          for (const auto &axis : element->querySelectorsAll("theta_axes"))
            {
              processThetaAxes(axis, global_render->getContext());
            }
          element->setAttribute("_zoomed", false);
        }
    }
  else
    {
      if (xmax - xmin > 0.0 && ymax - ymin > 0.0) gr_setwindow(xmin, xmax, ymin, ymax);
    }
}
