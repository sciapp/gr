#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <cfloat>

#include <grm/dom_render/process_elements.hxx>
#include <grm/dom_render/render_util.hxx>
#include <grm/dom_render/process_attributes.hxx>
#include <grm/dom_render/render.hxx>
#include <grm/util_int.h>
#include <grm/plot.h>
#include <grm/event_int.h>
#include <grm/plot_int.h>
#include <grm/dom_render/casts.hxx>
#include <grm/dom_render/graphics_tree/util.hxx>

#include "gr.h"
#include "gr3.h"

/* ------------------------- re-implementation of x_lin/x_log ------------------------------------------------------- */

#define xFlipIf(x, scale_options, xmin, xmax) \
  ((GR_OPTION_FLIP_X & (scale_options) ? (xmin) + (xmax) : 0) + (GR_OPTION_FLIP_X & (scale_options) ? -1 : 1) * (x))

#define xLin(x, scale_options, xmin, xmax, a, b)                                                                      \
  xFlipIf((GR_OPTION_X_LOG & (scale_options) ? ((x) > 0 ? (a)*log10(x) + (b) : -FLT_MAX) : (x)), scale_options, xmin, \
          xmax)

#define xLog(x, scale_options, xmin, xmax, a, b)                                                                  \
  (GR_OPTION_X_LOG & (scale_options) ? (pow(10.0, (double)((xFlipIf(x, scale_options, xmin, xmax) - (b)) / (a)))) \
                                     : xFlipIf(x, scale_options, xmin, xmax))

/* ------------------------------- process high lvl elements ---------------------------------------------------------*/

void processElement(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for all kinds of elements
   *
   * \param[in] element The GRM::Element containing attributes and data keys
   * \param[in] context The GRM::Context containing the actual data
   */
  auto global_root = grm_get_document_root();
  auto global_render = grm_get_render();
  auto active_figure = global_render->getActiveFigure();
  bool automatic_update;
  global_render->getAutoUpdate(&automatic_update);

  processPrivateTransparency(element);

  // Map used for processing all kinds of elements
  bool update_required = static_cast<int>(element->getAttribute("_update_required"));
  static std::map<std::string,
                  std::function<void(const std::shared_ptr<GRM::Element>, const std::shared_ptr<GRM::Context>)>>
      elem_string_to_func{
          {std::string("angle_line"), processAngleLine},
          {std::string("arc_grid_line"), processArcGridLine},
          {std::string("axes_3d"), GRM::PushDrawableToZQueue(processAxes3d)},
          {std::string("axis"), processAxis},
          {std::string("bar"), processBar},
          {std::string("cell_array"), GRM::PushDrawableToZQueue(processCellArray)},
          {std::string("colorbar"), processColorbar},
          {std::string("coordinate_system"), processCoordinateSystem},
          {std::string("error_bar"), processErrorBar},
          {std::string("error_bars"), processErrorBars},
          {std::string("legend"), processLegend},
          {std::string("draw_arc"), GRM::PushDrawableToZQueue(processDrawArc)},
          {std::string("draw_graphics"), GRM::PushDrawableToZQueue(processDrawGraphics)},
          {std::string("draw_image"), GRM::PushDrawableToZQueue(processDrawImage)},
          {std::string("draw_rect"), GRM::PushDrawableToZQueue(processDrawRect)},
          {std::string("fill_arc"), GRM::PushDrawableToZQueue(processFillArc)},
          {std::string("fill_area"), GRM::PushDrawableToZQueue(processFillArea)},
          {std::string("fill_rect"), GRM::PushDrawableToZQueue(processFillRect)},
          {std::string("grid_3d"), GRM::PushDrawableToZQueue(processGrid3d)},
          {std::string("grid_line"), GRM::PushDrawableToZQueue(processGridLine)},
          {std::string("integral"), processIntegral},
          {std::string("integral_group"), processIntegralGroup},
          {std::string("layout_grid"), GRM::PushDrawableToZQueue(processLayoutGrid)},
          {std::string("marginal_heatmap_plot"), processMarginalHeatmapPlot},
          {std::string("nonuniform_polar_cell_array"), GRM::PushDrawableToZQueue(processNonUniformPolarCellArray)},
          {std::string("nonuniform_cell_array"), GRM::PushDrawableToZQueue(processNonuniformCellArray)},
          {std::string("overlay_element"), processOverlayElement},
          {std::string("panzoom"), GRM::PushDrawableToZQueue(processPanzoom)},
          {std::string("pie_segment"), processPieSegment},
          {std::string("polar_bar"), processPolarBar},
          {std::string("polar_cell_array"), GRM::PushDrawableToZQueue(processPolarCellArray)},
          {std::string("polyline"), GRM::PushDrawableToZQueue(processPolyline)},
          {std::string("polyline_3d"), GRM::PushDrawableToZQueue(processPolyline3d)},
          {std::string("polymarker"), GRM::PushDrawableToZQueue(processPolymarker)},
          {std::string("polymarker_3d"), GRM::PushDrawableToZQueue(processPolymarker3d)},
          {std::string("radial_axes"), processRadialAxes},
          {std::string("series"), processSeries},
          {std::string("side_region"), processSideRegion},
          {std::string("side_plot_region"), processSidePlotRegion},
          {std::string("text"), GRM::PushDrawableToZQueue(processText)},
          {std::string("text_region"), processTextRegion},
          {std::string("theta_axes"), processThetaAxes},
          {std::string("tick"), GRM::PushDrawableToZQueue(processTick)},
          {std::string("tick_group"), processTickGroup},
          {std::string("titles_3d"), GRM::PushDrawableToZQueue(processTitles3d)},
      };

  /* Modifier */
  if (strEqualsAny(element->localName(), "axis", "central_region", "figure", "plot", "label", "root",
                   "layout_grid_element", "radial_axes", "side_region", "text_region", "side_plot_region", "tick_group",
                   "theta_axes", "arc_grid_line", "angle_line", "layout_grid", "overlay", "overlay_element"))
    {
      bool old_state;
      GRM::Render::getAutoUpdate(&old_state);
      GRM::Render::setAutoUpdate(false);
      /* check if figure is active; skip inactive elements */
      if (element->localName() == "figure")
        {
          gr_selntran(1); // for some reason selntran isn't always resetted correctly after a legend got processed
          if (!static_cast<int>(element->getAttribute("active"))) return;
          if (element->hasAttribute("size_x") && !element->hasAttribute("_initial_width"))
            {
              int m_width, m_height;
              GRM::getFigureSize(&m_width, &m_height, nullptr, nullptr);

              element->setAttribute("_initial_width", static_cast<double>(m_width));
              element->setAttribute("_initial_height", static_cast<double>(m_height));
            }
          if (element->querySelectorsAll("draw_graphics").empty()) plotProcessWsWindowWsViewport(element, context);
        }
      if (element->localName() == "plot")
        {
          std::shared_ptr<GRM::Element> central_region_parent = element;
          processPlot(element, context);
          // for some reason selntran isn't always resseted in multiplotcase even after the additional set on the figure
          gr_selntran(1);
          // if the kind is marginal_heatmap plot can only have 1 child and this child is the marginal_heatmap_plot
          if (static_cast<std::string>(element->getAttribute("_kind")) == "marginal_heatmap")
            central_region_parent = element->children()[0];

          if (central_region_parent != element) calculateViewport(central_region_parent);

          for (const auto &child : central_region_parent->children())
            {
              if (child->localName() == "central_region")
                {
                  calculateViewport(child);
                  calculateCharHeight(child);
                  processWindow(child);
                  processScale(element);
                  break;
                }
            }
        }
      else if (element->localName() != "root")
        {
          calculateViewport(element);
        }
      if (element->localName() == "angle_line") processAngleLine(element, context);
      if (element->localName() == "arc_grid_line") processArcGridLine(element, context);
      if (element->localName() == "axis") processAxis(element, context);
      if (element->localName() == "radial_axes") processRadialAxes(element, context);
      if (element->localName() == "side_plot_region") processSidePlotRegion(element, context);
      if (element->localName() == "side_region") processSideRegion(element, context);
      if (element->localName() == "text_region") processTextRegion(element, context);
      if (element->localName() == "theta_axes") processThetaAxes(element, context);
      if (element->localName() == "tick_group") processTickGroup(element, context);
      if (element->localName() == "overlay_element") processOverlayElement(element, context);
      processAttributes(element);
      GRM::Render::setAutoUpdate(old_state);
      if (element->localName() != "root") applyMoveTransformation(element);
    }
  else
    {
      if (strEqualsAny(element->localName(), "marginal_heatmap_plot", "coordinate_system"))
        {
          bool old_state;
          GRM::Render::getAutoUpdate(&old_state);
          GRM::Render::setAutoUpdate(false);
          calculateViewport(element);
          applyMoveTransformation(element);
          GRM::Render::setAutoUpdate(old_state);
        }
      // TODO: something like series_contour shouldn't be in this list
      if (!automatic_update ||
          ((static_cast<int>(global_root->getAttribute("_modified")) &&
            (strEqualsAny(element->localName(), "axes_3d", "cell_array", "colorbar", "draw_arc", "draw_image",
                          "draw_rect", "fill_arc", "fill_area", "fill_rect", "grid", "grid_3d", "legend",
                          "nonuniform_polar_cell_array", "nonuniform_cell_array", "polar_cell_array", "polyline",
                          "polyline_3d", "polymarker", "polymarker_3d", "series_contour", "series_contourf", "text",
                          "titles_3d", "coordinate_system", "series_hexbin", "series_isosurface", "series_quiver",
                          "series_shade", "series_surface", "series_tricontour", "series_trisurface",
                          "series_volume") ||
             !element->hasChildNodes())) ||
           update_required))
        {
          // elements without children are the draw-functions which need to be processed everytime, else there could
          // be problems with overlapping elements
          std::string local_name = getLocalName(element);

          bool old_state = automatic_update;
          global_render->setAutoUpdate(false);
          /* The attributes of drawables (except for the z_index itself) are being processed when the z_queue is being
           * processed */
          if (element->hasAttribute("viewport_x_min")) calculateViewport(element);
          if (isDrawable(element))
            {
              if (element->hasAttribute("z_index")) processZIndex(element);
            }
          else
            {
              processAttributes(element);
            }

          if (auto search = elem_string_to_func.find(local_name); search != elem_string_to_func.end())
            {
              auto f = search->second;
              f(element, context);
            }
          else
            {
              throw NotFoundError("No dom render function found for element with local name: " + element->localName() +
                                  "\n");
            }

          // reset _update_required
          element->setAttribute("_update_required", false);
          element->setAttribute("_delete_children", 0);
          if (update_required)
            {
              for (const auto &child : element->children())
                {
                  if (!strEqualsAny(element->localName(), "figure", "plot", "label", "root", "layout_grid_element"))
                    {
                      child->setAttribute("_update_required", true);
                      resetOldBoundingBoxes(child);
                    }
                }
            }
          global_render->setAutoUpdate(old_state);
        }
      else if (automatic_update && static_cast<int>(global_root->getAttribute("_modified")) ||
               element->parentElement()->parentElement()->hasAttribute("marginal_heatmap_side_plot"))
        {
          bool old_state;
          global_render->getAutoUpdate(&old_state);
          global_render->setAutoUpdate(false);
          processAttributes(element);
          global_render->setAutoUpdate(old_state);
        }
    }

  // use the correct nominal factor for each plot respecting the actual size of the central_region
  if (element->localName() == "plot")
    {
      double vp[4], viewport[4];
      double metric_width, metric_height;
      int px_width, px_height;
      double initial_factor;
      std::shared_ptr<GRM::Element> plot_parent = element;

      auto central_region_elem = plot_parent->querySelectors("central_region");
      if (central_region_elem == nullptr) return;
      auto figure_vp_element = plot_parent->parentElement()->localName() == "layout_grid_element"
                                   ? plot_parent->parentElement()
                                   : plot_parent;

      vp[0] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_x_min_org"));
      vp[1] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_x_max_org"));
      vp[2] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_y_min_org"));
      vp[3] = static_cast<double>(figure_vp_element->getAttribute("_viewport_normalized_y_max_org"));
      if (!GRM::Render::getViewport(central_region_elem, &viewport[0], &viewport[1], &viewport[2], &viewport[3]))
        throw NotFoundError("Central region doesn't have a viewport but it should.\n");

      GRM::getFigureSize(&px_width, &px_height, &metric_width, &metric_height);
      auto aspect_ratio_ws = metric_width / metric_height;
      auto initial_width = static_cast<int>(active_figure->getAttribute("_initial_width")) * (vp[1] - vp[0]);
      auto initial_height = static_cast<int>(active_figure->getAttribute("_initial_height")) * (vp[3] - vp[2]);
      double central_region_width = (viewport[1] - viewport[0]) * px_width;
      double central_region_height = (viewport[3] - viewport[2]) * px_height;
      if (aspect_ratio_ws > 1)
        central_region_height *= aspect_ratio_ws;
      else
        central_region_width /= aspect_ratio_ws;

      if (!plot_parent->hasAttribute("_initial_factor"))
        {
          initial_factor = (central_region_width * central_region_height) / (initial_width * initial_height);
          plot_parent->setAttribute("_initial_factor", initial_factor);
        }
      else
        {
          initial_factor = static_cast<double>(plot_parent->getAttribute("_initial_factor"));
        }
      double factor = (central_region_width * central_region_height) / (initial_width * initial_height);

      gr_setnominalsize(sqrt(factor / initial_factor) *
                        (grm_min(initial_width, initial_height) / grm_min(px_width, px_height)));
    }
}

void processSeries(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  auto global_render = grm_get_render();
  std::shared_ptr<GRM::Element> central_region;
  static std::map<std::string,
                  std::function<void(const std::shared_ptr<GRM::Element>, const std::shared_ptr<GRM::Context>)>>
      series_name_to_func{
          {std::string("barplot"), processBarplot},
          {std::string("contour"), GRM::PushDrawableToZQueue(processContour)},
          {std::string("contourf"), GRM::PushDrawableToZQueue(processContourf)},
          {std::string("heatmap"), processHeatmap},
          {std::string("hexbin"), processHexbin},
          {std::string("histogram"), processHistogram},
          {std::string("imshow"), processImshow},
          {std::string("isosurface"), GRM::PushDrawableToZQueue(processIsosurface)},
          {std::string("line"), processLine},
          {std::string("pie"), processPie},
          {std::string("line3"), processLine3},
          {std::string("polar_heatmap"), processPolarHeatmap},
          {std::string("polar_histogram"), processPolarHistogram},
          {std::string("polar_line"), processPolarLine},
          {std::string("polar_scatter"), processPolarScatter},
          {std::string("quiver"), GRM::PushDrawableToZQueue(processQuiver)},
          {std::string("scatter"), processScatter},
          {std::string("scatter3"), processScatter3},
          {std::string("shade"), GRM::PushDrawableToZQueue(processShade)},
          {std::string("stairs"), processStairs},
          {std::string("stem"), processStem},
          {std::string("surface"), GRM::PushDrawableToZQueue(processSurface)},
          {std::string("tricontour"), GRM::PushDrawableToZQueue(processTriContour)},
          {std::string("trisurface"), GRM::PushDrawableToZQueue(processTriSurface)},
          {std::string("volume"), processVolume},
          {std::string("wireframe"), GRM::PushDrawableToZQueue(processWireframe)},
      };

  auto kind = static_cast<std::string>(element->getAttribute("kind"));
  auto plot_elem = getPlotElement(element);

  if (auto search = series_name_to_func.find(kind); search != series_name_to_func.end())
    {
      auto f = search->second;
      f(element, context);
    }
  else
    {
      throw NotFoundError("Series is not in render implemented yet\n");
    }

  auto central_region_parent = plot_elem;
  if (kind == "marginal_heatmap") central_region_parent = plot_elem->children()[0];
  for (const auto &child : central_region_parent->children())
    {
      if (child->localName() == "central_region")
        {
          central_region = child;
          break;
        }
    }
  // special case where the data of a series could inflict the window
  // its important that the series gets first processed so the changed data gets used inside
  // calculateInitialCoordinateLims
  if (element->parentElement()->parentElement()->localName() == "plot" &&
      !static_cast<int>(central_region->getAttribute("keep_window")))
    {
      calculateInitialCoordinateLims(element->parentElement()->parentElement(), global_render->getContext());
    }
}

void processPlot(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  std::shared_ptr<GRM::Element> central_region, central_region_parent = element, side_region;
  auto global_render = grm_get_render();
  auto global_root = grm_get_document_root();
  auto kind = static_cast<std::string>(element->getAttribute("_kind"));
  if (kind == "marginal_heatmap") central_region_parent = element->querySelectors("marginal_heatmap_plot");
  auto global_creator = grm_get_creator();

  for (const auto &child : central_region_parent->children())
    {
      if (child->localName() == "central_region")
        {
          central_region = child;
          break;
        }
    }

  if (polar_kinds.count(kind) > 0)
    {
      if (polar_kinds.count(kind) == 0 && element->hasAttribute("x_lim_min") && element->hasAttribute("x_lim_max"))
        {
          auto x_lim_min = static_cast<double>(element->getAttribute("x_lim_min"));
          auto x_lim_max = static_cast<double>(element->getAttribute("x_lim_max"));
          if (!grm_isnan(x_lim_min) && !grm_isnan(x_lim_max)) gr_setclipsector(x_lim_min, x_lim_max);
        }
      else if (polar_kinds.count(kind) > 0 && element->hasAttribute("theta_lim_min") &&
               element->hasAttribute("theta_lim_max"))
        {
          auto theta_lim_min = static_cast<double>(element->getAttribute("theta_lim_min"));
          auto theta_lim_max = static_cast<double>(element->getAttribute("theta_lim_max"));
          if (!grm_isnan(theta_lim_min) && !grm_isnan(theta_lim_max)) gr_setclipsector(theta_lim_min, theta_lim_max);
        }
      if (!element->hasAttribute("polar_with_pan") || !static_cast<int>(element->getAttribute("polar_with_pan")))
        {
          global_render->setClipRegion(central_region, 1);
          global_render->setSelectSpecificXform(central_region, 1);
        }
    }
  else if (kinds_3d.count(kind) == 0)
    {
      global_render->setClipRegion(central_region, 0);
    }

  // set the x-, y- and z-data to NAN if the value is <= 0
  // if the plot contains the marginal_heatmap_plot the marginal_heatmap should be child in the following for
  for (const auto &child : (central_region_parent == element) ? central_region->children() : element->children())
    {
      if ((!startsWith(child->localName(), "series_") && central_region_parent == element) ||
          (child->localName() != "marginal_heatmap_plot" && central_region_parent != element))
        continue;
      auto id = static_cast<int>(global_root->getAttribute("_id"));
      auto str = std::to_string(id);
      std::string prefix = "";

      if (child->hasAttribute("label")) prefix = static_cast<std::string>(child->getAttribute("label")) + "_";

      // save the original data so it can be restored
      if (child->hasAttribute("x") && !child->hasAttribute("_x_org"))
        {
          auto x = static_cast<std::string>(child->getAttribute("x"));
          child->setAttribute("_x_org", x);
          (*context)["_x_org"].useContextKey(static_cast<std::string>(x), ""); // increase context cnt
        }
      if (child->hasAttribute("x_range_min") && !child->hasAttribute("_x_range_min_org"))
        child->setAttribute("_x_range_min_org", static_cast<double>(child->getAttribute("x_range_min")));
      if (child->hasAttribute("x_range_max") && !child->hasAttribute("_x_range_max_org"))
        child->setAttribute("_x_range_max_org", static_cast<double>(child->getAttribute("x_range_max")));
      if (child->hasAttribute("y") && !child->hasAttribute("_y_org"))
        {
          auto y = static_cast<std::string>(child->getAttribute("y"));
          child->setAttribute("_y_org", y);
          (*context)["_y_org"].useContextKey(static_cast<std::string>(y), ""); // increase context cnt
        }
      if (child->hasAttribute("y_range_min") && !child->hasAttribute("_y_range_min_org"))
        child->setAttribute("_y_range_min_org", static_cast<double>(child->getAttribute("y_range_min")));
      if (child->hasAttribute("y_range_max") && !child->hasAttribute("_y_range_max_org"))
        child->setAttribute("_y_range_max_org", static_cast<double>(child->getAttribute("y_range_max")));
      if (child->hasAttribute("z") && !child->hasAttribute("_z_org"))
        {
          auto z = static_cast<std::string>(child->getAttribute("z"));
          child->setAttribute("_z_org", z);
          (*context)["_z_org"].useContextKey(static_cast<std::string>(z), ""); // increase context cnt
        }
      if (child->hasAttribute("z_range_min") && !child->hasAttribute("_z_range_min_org"))
        child->setAttribute("_z_range_min_org", static_cast<double>(child->getAttribute("z_range_min")));
      if (child->hasAttribute("z_range_max") && !child->hasAttribute("_z_range_max_org"))
        child->setAttribute("_z_range_max_org", static_cast<double>(child->getAttribute("z_range_max")));
      if (child->hasAttribute("r") && !child->hasAttribute("_r_org"))
        {
          auto r = static_cast<std::string>(child->getAttribute("r"));
          child->setAttribute("_r_org", r);
          (*context)["_r_org"].useContextKey(static_cast<std::string>(r), ""); // increase context cnt
        }
      if (child->hasAttribute("r_range_min") && !child->hasAttribute("_r_range_min_org"))
        child->setAttribute("_r_range_min_org", static_cast<double>(child->getAttribute("r_range_min")));
      if (child->hasAttribute("r_range_max") && !child->hasAttribute("_r_range_max_org"))
        child->setAttribute("_r_range_max_org", static_cast<double>(child->getAttribute("r_range_max")));

      // the original data must be set on the imshow series so it can be used when the imshow plot should be
      // switched to a new kind. The reason for it is that the imshow plot defines x and y as a lin-space
      // from 0 to length. The cases for log can be ignored cause log gets ignored on imshow plots.
      if (child->localName() == "series_imshow") continue;

      bool x_log = static_cast<int>(element->getAttribute("x_log")) && child->hasAttribute("_x_org");
      bool y_log = static_cast<int>(element->getAttribute("y_log")) && child->hasAttribute("_y_org");
      bool z_log = static_cast<int>(element->getAttribute("z_log")) && child->hasAttribute("_z_org");
      bool r_log = static_cast<int>(element->getAttribute("r_log")) && child->hasAttribute("_r_org");
      if (x_log)
        {
          double x_min = INFINITY, x_max = -INFINITY;
          auto x = static_cast<std::string>(child->getAttribute("_x_org"));
          auto x_vec = GRM::get<std::vector<double>>((*context)[x]);
          auto x_len = x_vec.size();

          if (kind == "volume")
            {
              fprintf(stderr, "The option x_log is not supported for volume. It will be set to false.\n");
              element->setAttribute("x_log", 0);
            }
          else
            {
              auto child_kind = static_cast<std::string>(child->getAttribute("kind"));
              for (int i = 0; i < x_len; i++)
                {
                  if (x_vec[i] <= 0)
                    {
                      if (child_kind == "trisurface" || child_kind == "tricontour" || child_kind == "line3")
                        {
                          fprintf(stderr,
                                  "The option x_log is not supported for x-values <= 0. It will be set to false.\n");
                          element->setAttribute("x_log", 0);
                        }
                      else
                        {
                          x_vec[i] = NAN;
                        }
                    }
                  if (!grm_isnan(x_vec[i])) x_min = (x_min < x_vec[i]) ? x_min : x_vec[i];
                  if (!grm_isnan(x_vec[i])) x_max = (x_max > x_vec[i]) ? x_max : x_vec[i];
                }

              (*context)[prefix + "x" + str] = x_vec;
              child->setAttribute("x", prefix + "x" + str);
              if (child->hasAttribute("x_range_min")) child->setAttribute("x_range_min", x_min);
              if (child->hasAttribute("x_range_max")) child->setAttribute("x_range_max", x_max);
            }
        }
      if (y_log)
        {
          double y_min = INFINITY, y_max = -INFINITY;
          auto y = static_cast<std::string>(child->getAttribute("_y_org"));
          auto y_vec = GRM::get<std::vector<double>>((*context)[y]);
          auto y_len = y_vec.size();

          if (kind == "volume")
            {
              fprintf(stderr, "The option y_log is not supported for volume. It will be set to false.\n");
              element->setAttribute("y_log", 0);
            }
          else
            {
              auto child_kind = static_cast<std::string>(child->getAttribute("kind"));
              for (int i = 0; i < y_len; i++)
                {
                  if (y_vec[i] <= 0)
                    {
                      if (child_kind == "trisurface" || child_kind == "tricontour" || child_kind == "line3")
                        {
                          fprintf(stderr,
                                  "The option y_log is not supported for y-values <= 0. It will be set to false.\n");
                          element->setAttribute("y_log", 0);
                        }
                      else
                        {
                          y_vec[i] = NAN;
                        }
                    }
                  if (!grm_isnan(y_vec[i])) y_min = (y_min < y_vec[i]) ? y_min : y_vec[i];
                  if (!grm_isnan(y_vec[i])) y_max = (y_max > y_vec[i]) ? y_max : y_vec[i];
                }

              (*context)[prefix + "y" + str] = y_vec;
              child->setAttribute("y", prefix + "y" + str);
              if (kind == "barplot" && y_min <= 0) y_min = 1;
              if (child->hasAttribute("y_range_min")) child->setAttribute("y_range_min", y_min);
              if (child->hasAttribute("y_range_max")) child->setAttribute("y_range_max", y_max);
            }
        }
      if (z_log)
        {
          double z_min = INFINITY, z_max = (double)-INFINITY;
          auto z = static_cast<std::string>(child->getAttribute("_z_org"));
          auto z_vec = GRM::get<std::vector<double>>((*context)[z]);
          auto z_len = z_vec.size();

          if (kind == "volume")
            {
              fprintf(stderr, "The option z_log is not supported for volume. It will be set to false.\n");
              element->setAttribute("z_log", 0);
            }
          else
            {
              auto child_kind = static_cast<std::string>(child->getAttribute("kind"));
              for (int i = 0; i < z_len; i++)
                {
                  if (z_vec[i] <= 0)
                    {
                      if (child_kind == "trisurface" || child_kind == "tricontour" || child_kind == "line3")
                        {
                          fprintf(stderr,
                                  "The option z_log is not supported for z-values <= 0. It will be set to false.\n");
                          element->setAttribute("z_log", 0);
                        }
                      else
                        {
                          z_vec[i] = NAN;
                        }
                    }
                  if (!grm_isnan(z_vec[i])) z_min = (z_min < z_vec[i]) ? z_min : z_vec[i];
                  if (!grm_isnan(z_vec[i])) z_max = (z_max > z_vec[i]) ? z_max : z_vec[i];
                }

              (*context)[prefix + "z" + str] = z_vec;
              child->setAttribute("z", prefix + "z" + str);
              if (child->hasAttribute("z_range_min")) child->setAttribute("z_range_min", z_min);
              if (child->hasAttribute("z_range_max")) child->setAttribute("z_range_max", z_max);
            }
        }
      if (r_log)
        {
          double r_min = INFINITY, r_max = -INFINITY;
          auto r = static_cast<std::string>(child->getAttribute("_r_org"));
          auto r_vec = GRM::get<std::vector<double>>((*context)[r]);
          auto r_len = r_vec.size();

          auto child_kind = static_cast<std::string>(child->getAttribute("kind"));
          for (int i = 0; i < r_len; i++)
            {
              if (r_vec[i] <= 0) r_vec[i] = NAN;
              if (!grm_isnan(r_vec[i])) r_min = (r_min < r_vec[i]) ? r_min : r_vec[i];
              if (!grm_isnan(r_vec[i])) r_max = (r_max > r_vec[i]) ? r_max : r_vec[i];
            }

          (*context)[prefix + "r" + str] = r_vec;
          child->setAttribute("r", prefix + "r" + str);
          if (child->hasAttribute("r_range_min")) child->setAttribute("r_range_min", r_min);
          if (child->hasAttribute("r_range_max")) child->setAttribute("r_range_max", r_max);
        }
      if ((x_log || y_log) && kind == "hexbin")
        fprintf(stderr, "Hexbin plots with logarithmic x- or y-values are currently not supported. If you need this "
                        "feature, please send a feature request to the GR developers at "
                        "<https://github.com/sciapp/gr/issues>.\n");
      global_root->setAttribute("_id", ++id);
    }

  if (!element->hasAttribute("_x_lim_min") || !element->hasAttribute("_x_lim_max") ||
      !element->hasAttribute("_y_lim_min") || !element->hasAttribute("_y_lim_max") ||
      element->hasAttribute("_update_limits") && static_cast<int>(element->getAttribute("_update_limits")))
    {
      calculateInitialCoordinateLims(element, context);
      element->removeAttribute("_update_limits");
    }
  calculateViewport(element);
  // todo: there are cases that element does not have char_height set
  // char_height is always calculated (and set in the gr) in calculateCharHeight
  // it is however not stored on the element as it can be calculated from other attributes
  if (element->hasAttribute("char_height")) processCharHeight(element);
  processLimits(element);
  processScale(element); /* needs to be set before flip is processed */

  if (!central_region->hasAttribute("orientation"))
    central_region->setAttribute("orientation", PLOT_DEFAULT_ORIENTATION);

  /* Map for calculations on the plot level */
  static std::map<std::string,
                  std::function<void(const std::shared_ptr<GRM::Element> &, const std::shared_ptr<GRM::Context> &)>>
      kind_name_to_func{
          {std::string("barplot"), preBarplot},
          {std::string("polar_histogram"), prePolarHistogram},
      };

  for (const auto &child : central_region->children())
    {
      if (child->localName() == "series_barplot" || child->localName() == "series_polar_histogram")
        {
          kind = static_cast<std::string>(child->getAttribute("kind"));
          if (kind_name_to_func.find(kind) != kind_name_to_func.end()) kind_name_to_func[kind](element, context);
          break;
        }
    }

  for (const std::string &location : {"right", "left", "bottom", "top"})
    {
      if (!element->querySelectors("side_region[location=\"" + location + "\"]"))
        {
          side_region = global_creator->createSideRegion(location);
          central_region_parent->append(side_region);
        }
      if (element->hasAttribute("_" + location + "_window_xform_a_org") &&
          element->hasAttribute("_" + location + "_window_xform_b_org"))
        {
          side_region = element->querySelectors("side_region[location=\"" + location + "\"]");
          if (!side_region->querySelectors("side_plot_region"))
            {
              std::shared_ptr<GRM::Element> axis;
              auto side_plot_region = global_creator->createSidePlotRegion();
              side_region->append(side_plot_region);
              side_region->setAttribute("width", PLOT_DEFAULT_ADDITIONAL_AXIS_WIDTH);

              if (!side_plot_region->querySelectors("axis"))
                {
                  axis = global_creator->createEmptyAxis();
                  axis->setAttribute("_child_id", side_plot_region->querySelectors("axis") ? 1 : 0);
                  side_plot_region->append(axis);
                }
              else
                {
                  axis = side_plot_region->querySelectors(
                      "axis[_child_id=" + std::to_string(side_plot_region->querySelectors("axis") ? 1 : 0) + "]");
                  if (axis != nullptr) axis = global_creator->createEmptyAxis(axis);
                }
              if (axis != nullptr)
                {
                  axis->setAttribute("axis_type", strEqualsAny(location, "left", "right") ? "y" : "x");
                  axis->setAttribute("name", location + "-axis");
                  axis->setAttribute("location", location);
                  axis->setAttribute("mirrored_axis", false);
                  axis->setAttribute("line_color_ind", 1);
                }
            }
        }
    }
}

/* ------------------------------- pre process elements --------------------------------------------------------------*/

void preBarplot(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  std::vector<int> indices_vec;
  int max_y_length = 0;
  auto global_root = grm_get_document_root();

  for (const auto &series : element->querySelectorsAll("series_barplot"))
    {
      if (!series->hasAttribute("indices"))
        {
          if (!series->hasAttribute("y")) throw NotFoundError("Barplot series is missing indices\n");
          auto y_key = static_cast<std::string>(series->getAttribute("y"));
          auto y_vec = GRM::get<std::vector<double>>((*context)[y_key]);
          indices_vec = std::vector<int>(y_vec.size(), 1);
          auto id = static_cast<int>(global_root->getAttribute("_id"));
          auto id_str = std::to_string(id);

          (*context)["indices" + id_str] = indices_vec;
          series->setAttribute("indices", "indices" + id_str);
          global_root->setAttribute("_id", ++id);
        }
      else
        {
          auto indices_key = static_cast<std::string>(series->getAttribute("indices"));
          indices_vec = GRM::get<std::vector<int>>((*context)[indices_key]);
        }
      auto cur_y_length = static_cast<int>(indices_vec.size());
      max_y_length = grm_max(cur_y_length, max_y_length);
    }
  element->setAttribute("max_y_length", max_y_length);
}

void prePolarHistogram(const std::shared_ptr<GRM::Element> &plot_elem, const std::shared_ptr<GRM::Context> &context)
{
  unsigned int num_bins, length, num_bin_edges = 0, i;
  std::vector<double> theta;
  std::string norm = "count";
  std::vector<int> counts, bin_counts;
  double r_max, temp_max, bin_width, theta_range_min, theta_range_max;
  double *theta_lim = nullptr;
  int max_observations = 0, total_observations = 0;
  std::vector<double> bin_edges, bin_widths, new_edges;
  bool is_bin_counts = false;
  std::shared_ptr<GRM::Element> series = plot_elem->querySelectorsAll("series_polar_histogram")[0];
  auto global_root = grm_get_document_root();

  // define keys for later usages
  auto id = static_cast<int>(global_root->getAttribute("_id"));
  auto str = std::to_string(id);
  global_root->setAttribute("_id", id + 1);
  std::string bin_widths_key = "bin_widths" + str, bin_edges_key = "bin_edges" + str, counts_key = "counts" + str;

  if (series->hasAttribute("bin_counts"))
    {
      is_bin_counts = true;
      auto bin_counts_key = static_cast<std::string>(series->getAttribute("bin_counts"));
      bin_counts = GRM::get<std::vector<int>>((*context)[bin_counts_key]);

      length = bin_counts.size();
      num_bins = length;
      series->setAttribute("num_bins", static_cast<int>(num_bins));
    }
  else if (series->hasAttribute("theta"))
    {
      auto theta_key = static_cast<std::string>(series->getAttribute("theta"));
      theta = GRM::get<std::vector<double>>((*context)[theta_key]);
      length = theta.size();

      if (series->hasAttribute("theta_range_min") && series->hasAttribute("theta_range_max"))
        {
          theta_range_min = static_cast<double>(series->getAttribute("theta_range_min"));
          theta_range_max = static_cast<double>(series->getAttribute("theta_range_max"));
          // convert theta_range_min and max to radian if theta_range_max > 2 * M_PI
          if (theta_range_max > 2 * M_PI)
            {
              theta_range_min *= (M_PI / 180);
              theta_range_max *= (M_PI / 180);
            }
          if (theta_range_min > theta_range_max) std::swap(theta_range_min, theta_range_max);

          double theta_min = *std::min_element(theta.begin(), theta.end());
          double theta_max = *std::max_element(theta.begin(), theta.end());
          transformCoordinatesVector(theta, theta_min, theta_max, theta_range_min, theta_range_max);
        }
    }
  else
    {
      throw NotFoundError("Polar histogram series is missing x-data or bin_counts\n");
    }

  if (series->hasAttribute("theta_data_lim_min") || series->hasAttribute("theta_data_lim_max"))
    {
      double theta_lim_arr[2];
      theta_lim = theta_lim_arr;
      theta_lim[0] = static_cast<double>(series->getAttribute("theta_data_lim_min"));
      theta_lim[1] = static_cast<double>(series->getAttribute("theta_data_lim_max"));

      if (theta_lim[1] < theta_lim[0])
        {
          std::swap(theta_lim[0], theta_lim[1]);
          series->setAttribute("theta_flip", true);
        }
      if (theta_lim[0] < 0.0 || theta_lim[1] > 2 * M_PI)
        logger((stderr, "\"theta_lim\" must be between 0 and 2 * pi\n"));
    }

  /* bin_edges and num_bins */
  if (!series->hasAttribute("bin_edges"))
    {
      if (series->hasAttribute("num_bins")) num_bins = static_cast<int>(series->getAttribute("num_bins"));
      if (!series->hasAttribute("num_bins") || num_bins <= 0 || num_bins > 200)
        {
          num_bins = grm_min(12, (int)(length / 2.0) - 1);
          series->setAttribute("num_bins", static_cast<int>(num_bins));
        }

      if (theta_lim != nullptr)
        {
          // if theta_lim is given, it will create equidistant bin_edges from theta_min to theta_max
          bin_edges.resize(num_bins + 1);
          linSpace(theta_lim[0], theta_lim[1], static_cast<int>(num_bins) + 1, bin_edges);
          num_bin_edges = num_bins + 1;
          (*context)[bin_edges_key] = bin_edges;
          series->setAttribute("bin_edges", bin_edges_key);
        }
    }
  else
    {
      int cnt = 0;

      bin_edges_key = static_cast<std::string>(series->getAttribute("bin_edges"));
      bin_edges = GRM::get<std::vector<double>>((*context)[bin_edges_key]);
      num_bin_edges = bin_edges.size();

      /* filter bin_edges */
      new_edges.resize(num_bin_edges);
      for (i = 0; i < num_bin_edges; i++)
        {
          // only use values for new_edges which are included inside the definition area
          if ((theta_lim == nullptr && 0.0 <= bin_edges[i] && bin_edges[i] <= 2 * M_PI) ||
              (theta_lim != nullptr && theta_lim[0] <= bin_edges[i] && bin_edges[i] <= theta_lim[1]))
            {
              new_edges[cnt++] = bin_edges[i];
            }
          else
            {
              logger((stderr,
                      "Only values between the defined theta_lims or 0 and 2 * pi, if there are no theta_lims, are "
                      "allowed\n"));
            }
        }
      if (num_bin_edges > cnt)
        {
          num_bin_edges = cnt;
          bin_edges.resize(cnt);
        }
      else
        {
          bin_edges = new_edges;
        }
      if (theta_lim == nullptr)
        {
          num_bins = num_bin_edges - 1;
          series->setAttribute("num_bins", static_cast<int>(num_bins));
        }
      else
        {
          if (num_bin_edges != 1)
            {
              num_bins = num_bin_edges - 1;
              series->setAttribute("num_bins", static_cast<int>(num_bins));
              series->setAttribute("bin_edges", bin_edges_key);
              (*context)[bin_edges_key] = bin_edges;
            }
          else
            {
              logger((stderr, "Given \"theta_lim\" and given \"bin_edges\" are not compatible --> filtered "
                              "\"len(bin_edges) == 1\"\n"));
            }
        }
    }

  if (series->hasAttribute("norm"))
    {
      norm = static_cast<std::string>(series->getAttribute("norm"));
      if (!strEqualsAny(norm, "count", "countdensity", "pdf", "probability", "cumcount", "cdf"))
        {
          logger((stderr, "Got keyword \"norm\"  with invalid value \"%s\"\n", norm.c_str()));
        }
    }

  if (!series->hasAttribute("bin_width"))
    {
      if (num_bin_edges > 0)
        {
          bin_widths.resize(num_bins + 1);
          for (i = 1; i <= num_bin_edges - 1; i++)
            {
              bin_widths[i - 1] = bin_edges[i] - bin_edges[i - 1];
            }
          series->setAttribute("bin_widths", bin_widths_key);
          (*context)[bin_widths_key] = bin_widths;
        }
      else
        {
          bin_width = 2.0 * M_PI / num_bins;
          series->setAttribute("bin_width", bin_width);
        }
    }
  else
    {
      int n = 0;

      bin_width = static_cast<double>(series->getAttribute("bin_width"));

      if (num_bin_edges > 0 && theta_lim == nullptr)
        {
          logger((stderr, "\"bin_width\" is not compatible with \"bin_edges\"\n"));

          bin_widths.resize(num_bins);

          for (i = 1; i <= num_bin_edges - 1; i++)
            {
              bin_widths[i - 1] = bin_edges[i] - bin_edges[i - 1];
            }
          series->setAttribute("bin_widths", bin_widths_key);
          (*context)[bin_widths_key] = bin_widths;
        }

      if (bin_width <= 0 || bin_width > 2 * M_PI)
        logger((stderr, "\"bin_width\" must be between 0 (exclusive) and 2 * pi\n"));

      if (theta_lim != nullptr)
        {
          if (theta_lim[1] - theta_lim[0] < bin_width)
            {
              logger((stderr, "The given \"theta_lim\" range does not work with the given \"bin_width\"\n"));
            }
          else
            {
              n = static_cast<int>((theta_lim[1] - theta_lim[0]) / bin_width);
              if (is_bin_counts)
                {
                  if (num_bins > n)
                    logger((stderr, "\"bin_width\" does not work with this specific \"bin_count\". \"nbins\" do not "
                                    "fit \"bin_width\"\n"));
                  n = static_cast<int>(num_bins);
                }
              bin_edges.resize(n + 1);
              linSpace(theta_lim[0], n * bin_width, n + 1, bin_edges);
            }
        }
      else
        {
          if (static_cast<int>(2 * M_PI / bin_width) > 200)
            {
              n = 200;
              bin_width = 2 * M_PI / n;
            }
          n = static_cast<int>(2 * M_PI / bin_width);
          if (is_bin_counts)
            {
              if (num_bins > n)
                logger((stderr, "\"bin_width\" does not work with this specific \"bin_count\". \"nbins\" do not fit "
                                "\"bin_width\"\n"));
              n = static_cast<int>(num_bins);
            }
          bin_edges.resize(n + 1);
          linSpace(0.0, n * bin_width, n + 1, bin_edges);
        }
      num_bins = n;
      series->setAttribute("num_bins", static_cast<int>(num_bins));
      num_bin_edges = n + 1;
      series->setAttribute("bin_edges", bin_edges_key);
      (*context)[bin_edges_key] = bin_edges;
      series->setAttribute("bin_width", bin_width);
      bin_widths.resize(num_bins);

      for (i = 0; i < num_bins; i++) bin_widths[i] = bin_width;
      series->setAttribute("bin_widths", bin_widths_key);
      (*context)[bin_widths_key] = bin_widths;
    }

  if (is_bin_counts)
    {
      double temp_max_bc = 0.0;

      if (num_bin_edges > 0 && num_bins != num_bin_edges - 1)
        {
          logger((stderr, "Number of bin_edges must be number of bin_counts + 1\n"));
        }

      auto total = std::accumulate(bin_counts.begin(), bin_counts.end(), 0);
      for (i = 0; i < num_bins; i++)
        {
          // temp_max_bc is a potential maximum for all bins respecting the given norm
          if (num_bin_edges > 0) bin_width = bin_widths[i];

          if (norm == "pdf" && bin_counts[i] * 1.0 / (total * bin_width) > temp_max_bc)
            temp_max_bc = bin_counts[i] * 1.0 / (total * bin_width);
          else if (norm == "countdensity" && bin_counts[i] * 1.0 / (bin_width) > temp_max_bc)
            temp_max_bc = bin_counts[i] * 1.0 / (bin_width);
          else if (bin_counts[i] > temp_max_bc)
            temp_max_bc = bin_counts[i];
        }

      counts.resize(num_bins);

      // bin_counts is affected by cumulative norms --> bin_counts are summed in later bins
      if (strEqualsAny(norm, "cdf", "cumcount"))
        {
          for (i = 0; i < num_bins; ++i)
            {
              counts[i] = bin_counts[i];
              if (i != 0) counts[i] += counts[i - 1];
            }
        }
      else
        {
          counts = bin_counts;
        }

      series->setAttribute("counts", counts_key);
      (*context)[counts_key] = counts;
      series->setAttribute("_total", total);

      if (norm == "probability")
        r_max = temp_max_bc * 1.0 / total;
      else if (norm == "cdf")
        r_max = 1.0;
      else if (norm == "cumcount")
        r_max = total * 1.0;
      else
        r_max = temp_max_bc;
    }
  else /* !is_bin_counts */
    {
      r_max = 0.0;
      counts.resize(num_bins);

      // prepare bin_edges
      if (num_bin_edges == 0) // no bin_edges --> create bin_edges for uniform code later
        {
          // lin_space the bin_edges
          bin_edges.resize(num_bins + 1);
          linSpace(0.0, 2 * M_PI, (int)num_bins + 1, bin_edges);
        }
      else
        {
          // filter x
          double edge_min = bin_edges[0], edge_max = bin_edges[num_bin_edges - 1];

          auto it = std::remove_if(theta.begin(), theta.end(), [edge_min, edge_max](double angle) {
            return (angle < edge_min || angle > edge_max);
          });
          theta.erase(it, theta.end());
          length = theta.size();
        }

      // calc counts
      for (i = 0; i < num_bins; ++i)
        {
          int observations = 0;

          // iterate x --> filter angles for current bin
          for (int j = 0; j < length; ++j)
            {
              if (bin_edges[i] <= theta[j] && theta[j] < bin_edges[i + 1]) ++observations;
            }

          // differentiate between cumulative and non-cumulative norms
          counts[i] = observations;
          if (i != 0 && strEqualsAny(norm, "cdf", "cumcount")) counts[i] += counts[i - 1];
          // update the total number of observations; used for some norms
          total_observations += observations;
        }

      // get maximum number of observation from all bins
      max_observations = *std::max_element(counts.begin(), counts.end());

      series->setAttribute("counts", counts_key);
      (*context)[counts_key] = counts;
      series->setAttribute("_total", total_observations);

      // calculate the maximum from max_observations respecting the norms
      if (num_bin_edges == 0 && norm == "pdf") // no given bin_edges
        {
          r_max = max_observations * 1.0 / (total_observations * bin_width);
        }
      else if (num_bin_edges != 0 && strEqualsAny(norm, "pdf", "countdensity")) // calc maximum with given bin_edges
        {
          for (i = 0; i < num_bins; i++)
            {
              // temporary maximum respecting norm
              temp_max = counts[i];
              if (norm == "pdf")
                temp_max /= total_observations * bin_widths[i];
              else if (norm == "countdensity")
                temp_max /= bin_widths[i];

              if (temp_max > r_max) r_max = temp_max;
            }
        }
      else if (strEqualsAny(norm, "probability", "cdf"))
        {
          r_max = max_observations * 1.0 / total_observations;
        }
      else
        {
          r_max = static_cast<double>(max_observations);
        }
    }
  // set r_max (radius_max) in parent for later usages for polar axes and polar_histogram
  series->parentElement()->setAttribute("r_max", r_max);
}

/* ------------------------------- process low lvl elements ----------------------------------------------------------*/

void processAxis(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for axes
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double x_tick, x_org;
  double y_tick, y_org;
  int x_major, y_major, major_count = 1;
  int tick_orientation = 1;
  double tick_size = NAN, tick = NAN;
  double min_val = NAN, max_val = NAN;
  double org = NAN, pos = NAN;
  double line_x_min = 0.0, line_x_max = 0.0, line_y_min = 0.0, line_y_max = 0.0;
  std::string x_org_pos = PLOT_DEFAULT_ORG_POS, y_org_pos = PLOT_DEFAULT_ORG_POS;
  std::shared_ptr<GRM::Element> plot_parent = element, line, axis_elem = element, central_region, window_parent;
  double window[4], old_window[4] = {NAN, NAN, NAN, NAN};
  bool mirrored_axis = MIRRORED_AXIS_DEFAULT, x_flip = false, y_flip = false, x_log = false, y_log = false;
  std::string kind, axis_type;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  int scientific_format = SCIENTIFIC_FORMAT_OPTION, scale = 0;
  std::string location;
  int label_orientation = 0;
  auto global_render = grm_get_render();
  auto active_figure = global_render->getActiveFigure();
  auto global_creator = grm_get_creator();

  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);
  getPlotParent(plot_parent);

  /* `processAxis` can be triggered indirectly by `grm_input` but within the interaction processing the default Latin-1
   * encoding is used instead of the configured text encoding. Setting the correct text encoding is important since
   * functions like `gr_axis` modify the axis text based on the chosen encoding. */
  processTextEncoding(active_figure);
  processScale(plot_parent);

  if (axis_elem->hasAttribute("location")) location = static_cast<std::string>(axis_elem->getAttribute("location"));
  axis_type = static_cast<std::string>(element->getAttribute("axis_type"));
  if (plot_parent->hasAttribute("x_log")) x_log = static_cast<int>(plot_parent->getAttribute("x_log"));
  if (plot_parent->hasAttribute("y_log")) y_log = static_cast<int>(plot_parent->getAttribute("y_log"));

  window_parent = element->parentElement()->parentElement();
  if (strEqualsAny(location, "bottom", "left", "right", "top"))
    window_parent = plot_parent->querySelectors("central_region");
  window[0] = static_cast<double>(window_parent->getAttribute("window_x_min"));
  window[1] = static_cast<double>(window_parent->getAttribute("window_x_max"));
  window[2] = static_cast<double>(window_parent->getAttribute("window_y_min"));
  window[3] = static_cast<double>(window_parent->getAttribute("window_y_max"));

  // adjust the window for non-standard axis
  if (axis_elem->hasAttribute("location") && !strEqualsAny(location, "x", "y"))
    {
      double a, b;

      // reset transformation
      plot_parent->setAttribute("_" + location + "_window_xform_a",
                                static_cast<double>(plot_parent->getAttribute("_" + location + "_window_xform_a_org")));
      plot_parent->setAttribute("_" + location + "_window_xform_b",
                                static_cast<double>(plot_parent->getAttribute("_" + location + "_window_xform_b_org")));
      // apply lims if set to window and recalculate transformation
      if (axis_type == "x" && element->hasAttribute("x_lim_min") && element->hasAttribute("x_lim_max"))
        {
          auto x_lim_min = static_cast<double>(element->getAttribute("x_lim_min"));
          auto x_lim_max = static_cast<double>(element->getAttribute("x_lim_max"));
          calculateWindowTransformationParameter(element, window[0], window[1], x_lim_min, x_lim_max, location, &a, &b);

          plot_parent->setAttribute("_" + location + "_window_xform_a", a);
          plot_parent->setAttribute("_" + location + "_window_xform_b", b);
        }
      if (axis_type == "y" && element->hasAttribute("y_lim_min") && element->hasAttribute("y_lim_max"))
        {
          auto y_lim_min = static_cast<double>(element->getAttribute("y_lim_min"));
          auto y_lim_max = static_cast<double>(element->getAttribute("y_lim_max"));
          calculateWindowTransformationParameter(element, window[2], window[3], y_lim_min, y_lim_max, location, &a, &b);

          plot_parent->setAttribute("_" + location + "_window_xform_a", a);
          plot_parent->setAttribute("_" + location + "_window_xform_b", b);
        }

      // calculate the window for non default axis
      a = static_cast<double>(plot_parent->getAttribute("_" + location + "_window_xform_a"));
      b = static_cast<double>(plot_parent->getAttribute("_" + location + "_window_xform_b"));
      if (strEqualsAny(location, "twin_x", "twin_y") && element->hasAttribute("_" + location + "_window_xform_a"))
        {
          std::string filter = (location == "twin_x") ? "x" : "y";
          if (auto axis_ref = element->parentElement()->querySelectors("axis[location=\"" + filter + "\"]");
              axis_ref->hasAttribute("draw_grid") && static_cast<int>(axis_ref->getAttribute("draw_grid")))
            {
              // get special transformation for twin axes with grid
              a = static_cast<double>(element->getAttribute("_" + location + "_window_xform_a"));
              b = static_cast<double>(element->getAttribute("_" + location + "_window_xform_b"));
              plot_parent->setAttribute("_" + location + "_window_xform_a", a);
              plot_parent->setAttribute("_" + location + "_window_xform_b", b);
            }
        }
      if (axis_type == "x")
        {
          double tmp_window_0, tmp_window_1;
          if (x_log)
            {
              tmp_window_0 = pow(10, a * log10(window[0]) + b);
              tmp_window_1 = pow(10, a * log10(window[1]) + b);
            }
          else
            {
              tmp_window_0 = a * window[0] + b;
              tmp_window_1 = a * window[1] + b;
            }
          // apply adjust limits to window if set and recalculate transformation
          if (element->hasAttribute("adjust_x_lim") && static_cast<int>(element->getAttribute("adjust_x_lim")))
            {
              gr_adjustlimits(&tmp_window_0, &tmp_window_1);
              calculateWindowTransformationParameter(element, window[0], window[1], tmp_window_0, tmp_window_1,
                                                     location, &a, &b);

              plot_parent->setAttribute("_" + location + "_window_xform_a", a);
              plot_parent->setAttribute("_" + location + "_window_xform_b", b);
            }

          if (location == "twin_x" && !element->hasAttribute("_" + location + "_window_xform_a"))
            {
              if (auto x_axis = element->parentElement()->querySelectors("axis[location=\"x\"]");
                  x_axis->hasAttribute("draw_grid") && static_cast<int>(x_axis->getAttribute("draw_grid")))
                {
                  newWindowForTwinAxis(element, x_axis, &tmp_window_0, &tmp_window_1, window[0], window[1]);
                }
            }
          window[0] = tmp_window_0;
          window[1] = tmp_window_1;
        }
      else if (axis_type == "y")
        {
          double tmp_window_2, tmp_window_3;
          if (y_log)
            {
              tmp_window_2 = pow(10, a * log10(window[2]) + b);
              tmp_window_3 = pow(10, a * log10(window[3]) + b);
            }
          else
            {
              tmp_window_2 = a * window[2] + b;
              tmp_window_3 = a * window[3] + b;
            }
          // apply adjust limits to window if set and recalculate transformation
          if (element->hasAttribute("adjust_y_lim") && static_cast<int>(element->getAttribute("adjust_y_lim")))
            {
              gr_adjustlimits(&tmp_window_2, &tmp_window_3);
              calculateWindowTransformationParameter(element, window[2], window[3], tmp_window_2, tmp_window_3,
                                                     location, &a, &b);

              plot_parent->setAttribute("_" + location + "_window_xform_a", a);
              plot_parent->setAttribute("_" + location + "_window_xform_b", b);
            }

          if (location == "twin_y" && !element->hasAttribute("_" + location + "_window_xform_a"))
            {
              if (auto y_axis = element->parentElement()->querySelectors("axis[location=\"y\"]");
                  y_axis->hasAttribute("draw_grid") && static_cast<int>(y_axis->getAttribute("draw_grid")))
                {
                  newWindowForTwinAxis(element, y_axis, &tmp_window_2, &tmp_window_3, window[2], window[3]);
                }
            }
          window[2] = tmp_window_2;
          window[3] = tmp_window_3;
        }
    }

  if (element->hasAttribute("_window_set_by_user"))
    {
      window[0] = static_cast<double>(element->getAttribute("window_x_min"));
      window[1] = static_cast<double>(element->getAttribute("window_x_max"));
      window[2] = static_cast<double>(element->getAttribute("window_y_min"));
      window[3] = static_cast<double>(element->getAttribute("window_y_max"));
    }
  if (strEqualsAny(location, "bottom", "left", "right", "top"))
    global_render->setWindow(element->parentElement(), window[0], window[1], window[2], window[3]);
  global_render->setWindow(element, window[0], window[1], window[2], window[3]);

  if (element->hasAttribute("_window_old_x_min"))
    old_window[0] = static_cast<double>(element->getAttribute("_window_old_x_min"));
  if (element->hasAttribute("_window_old_x_max"))
    old_window[1] = static_cast<double>(element->getAttribute("_window_old_x_max"));
  if (element->hasAttribute("_window_old_y_min"))
    old_window[2] = static_cast<double>(element->getAttribute("_window_old_y_min"));
  if (element->hasAttribute("_window_old_y_max"))
    old_window[3] = static_cast<double>(element->getAttribute("_window_old_y_max"));
  element->setAttribute("_window_old_x_min", window[0]);
  element->setAttribute("_window_old_x_max", window[1]);
  element->setAttribute("_window_old_y_min", window[2]);
  element->setAttribute("_window_old_y_max", window[3]);

  getAxesInformation(element, x_org_pos, y_org_pos, x_org, y_org, x_major, y_major, x_tick, y_tick);

  kind = static_cast<std::string>(plot_parent->getAttribute("_kind"));
  if (element->hasAttribute("mirrored_axis")) mirrored_axis = static_cast<int>(element->getAttribute("mirrored_axis"));
  if (element->hasAttribute("scientific_format"))
    scientific_format = static_cast<int>(element->getAttribute("scientific_format"));
  if (plot_parent->hasAttribute("font")) processFont(plot_parent);
  if (plot_parent->hasAttribute("x_flip")) x_flip = static_cast<int>(plot_parent->getAttribute("x_flip"));
  if (plot_parent->hasAttribute("y_flip")) y_flip = static_cast<int>(plot_parent->getAttribute("y_flip"));

  // if twin_x or twin_y is set x or y shouldn't be mirrored
  if (plot_parent->hasAttribute("_twin_" + axis_type + "_window_xform_a")) mirrored_axis = false;

  if (element->hasAttribute("scale"))
    {
      processScale(element);
      scale = static_cast<int>(element->getAttribute("scale"));
    }

  if (axis_type == "x")
    {
      min_val = window[0];
      max_val = window[1];
      tick = x_tick;
      pos = window[2];
      if (strEqualsAny(location, "top", "twin_x")) pos = window[3];
      major_count = x_major;
    }
  else if (axis_type == "y")
    {
      min_val = window[2];
      max_val = window[3];
      tick = y_tick;
      pos = window[0];
      if (strEqualsAny(location, "right", "twin_y")) pos = window[1];
      major_count = y_major;
    }
  getTickSize(element, tick_size); // GRM calculated tick_size

  if (element->parentElement()->localName() == "colorbar" ||
      (axis_type == "x" && ((std::isnan(old_window[0]) || old_window[0] == window[0]) &&
                            (std::isnan(old_window[1]) || old_window[1] == window[1]))) ||
      (axis_type == "y" && ((std::isnan(old_window[2]) || old_window[2] == window[2]) &&
                            (std::isnan(old_window[3]) || old_window[3] == window[3]))))
    {
      if (element->hasAttribute("origin")) org = static_cast<double>(element->getAttribute("origin"));
      if (element->hasAttribute("pos")) pos = static_cast<double>(element->getAttribute("pos"));
      if (element->hasAttribute("tick")) tick = static_cast<double>(element->getAttribute("tick"));
      if (element->hasAttribute("major_count")) major_count = static_cast<int>(element->getAttribute("major_count"));
      if (element->hasAttribute("tick_orientation"))
        tick_orientation = static_cast<int>(element->getAttribute("tick_orientation"));
      if (element->hasAttribute("label_orientation"))
        label_orientation = static_cast<int>(element->getAttribute("label_orientation"));
    }

  if (strEqualsAny(location, "right", "top", "twin_x", "twin_y"))
    {
      tick_orientation = -1;
      if (element->parentElement()->localName() != "colorbar" &&
          !element->hasAttribute("_label_orientation_set_by_user"))
        label_orientation = 1;
    }
  else
    {
      if (element->parentElement()->localName() != "colorbar" &&
          !element->hasAttribute("_label_orientation_set_by_user"))
        label_orientation = -1;
    }

  // axis
  if (element->parentElement()->localName() != "colorbar")
    {
      // special cases for x- and y-flip
      if ((scale & GR_OPTION_FLIP_X || x_flip) && axis_type == "y") pos = window[1];
      if ((scale & GR_OPTION_FLIP_Y || y_flip) && axis_type == "x") pos = window[3];

      central_region = element->parentElement()->parentElement();
      // ticks need to flipped cause a heatmap or shade series is part of the central_region
      for (const auto &series : central_region->children())
        {
          if (startsWith(series->localName(), "series_") &&
              strEqualsAny(series->localName(), "series_contourf", "series_heatmap", "series_shade"))
            {
              tick_orientation = -1;
              if (strEqualsAny(location, "twin_x", "twin_y")) tick_orientation = 1;
              break;
            }
        }
      processViewport(element->parentElement());
    }
  else
    {
      if (element->parentElement()->hasAttribute("x_flip"))
        x_flip = static_cast<int>(element->parentElement()->getAttribute("x_flip"));
      if (element->parentElement()->hasAttribute("y_flip"))
        y_flip = static_cast<int>(element->parentElement()->getAttribute("y_flip"));
      auto parent_location = static_cast<std::string>(
          element->parentElement()->parentElement()->parentElement()->getAttribute("location"));
      if ((scale & GR_OPTION_FLIP_X || x_flip) && axis_type == "y")
        {
          if (parent_location == "right")
            pos = window[1];
          else
            pos = window[0];
        }
      if ((scale & GR_OPTION_FLIP_Y || y_flip) && axis_type == "x")
        {
          if (parent_location == "top")
            pos = window[3];
          else
            pos = window[2];
        }
      processFlip(element->parentElement());
    }
  tick_size *= tick_orientation;
  axis_t axis = {min_val, max_val,   tick, org,     pos, major_count, 0,
                 nullptr, tick_size, 0,    nullptr, NAN, 1,           label_orientation};
  if (axis_type == "x")
    gr_axis('X', &axis);
  else if (axis_type == "y")
    gr_axis('Y', &axis);
  tick_orientation = axis.tick_size < 0 ? -1 : 1;

  if (element->hasAttribute("_min_value_set_by_user"))
    axis.min = static_cast<double>(element->getAttribute("_min_val_set_by_user"));
  if (element->hasAttribute("_max_value_set_by_user"))
    axis.max = static_cast<double>(element->getAttribute("_max_value_set_by_user"));
  if (element->hasAttribute("_tick_set_by_user"))
    axis.tick = static_cast<double>(element->getAttribute("_tick_set_by_user"));
  if (element->hasAttribute("_origin_set_by_user"))
    axis.org = static_cast<double>(element->getAttribute("_origin_set_by_user"));
  if (element->hasAttribute("_pos_set_by_user"))
    axis.position = static_cast<double>(element->getAttribute("_pos_set_by_user"));
  if (element->hasAttribute("_major_count_set_by_user"))
    axis.major_count = static_cast<int>(element->getAttribute("_major_count_set_by_user"));
  if (element->hasAttribute("_num_ticks_set_by_user"))
    axis.num_ticks = static_cast<int>(element->getAttribute("_num_ticks_set_by_user"));
  if (element->hasAttribute("_num_tick_labels_set_by_user"))
    axis.num_tick_labels = static_cast<int>(element->getAttribute("_num_tick_labels_set_by_user"));
  if (element->hasAttribute("_tick_size_set_by_user"))
    tick_size = static_cast<double>(element->getAttribute("_tick_size_set_by_user"));
  if (element->hasAttribute("_tick_orientation_set_by_user"))
    tick_orientation = static_cast<int>(element->getAttribute("_tick_orientation_set_by_user"));
  axis_elem = global_creator->createAxis(axis.min, axis.max, axis.tick, axis.org, axis.position, axis.major_count,
                                         axis.num_ticks, axis.num_tick_labels, abs(tick_size), tick_orientation,
                                         axis.label_position, axis_elem);
  if (del != DelValues::UPDATE_WITHOUT_DEFAULT)
    {
      if (!axis_elem->hasAttribute("draw_grid") && !plot_parent->hasAttribute("_twin_" + axis_type + "_window_xform_a"))
        axis_elem->setAttribute("draw_grid", true);
      if (kind == "barplot")
        {
          bool only_barplot = true;
          std::string orientation = PLOT_DEFAULT_ORIENTATION;
          auto barplot = plot_parent->querySelectors("series_barplot");
          for (const auto &series : central_region->children())
            {
              if (startsWith(series->localName(), "series_") && series->localName() != "series_barplot")
                {
                  only_barplot = false;
                  break;
                }
            }

          if (only_barplot)
            {
              auto parent = axis_elem->parentElement();
              if (axis_elem->parentElement()->localName() == "coordinate_system") parent = parent->parentElement();
              orientation = static_cast<std::string>(parent->getAttribute("orientation"));
              if (axis_type == "x" && orientation == "horizontal") axis_elem->setAttribute("draw_grid", false);
              if (axis_type == "y" && orientation == "vertical") axis_elem->setAttribute("draw_grid", false);
            }
        }
      if (kind == "shade") axis_elem->setAttribute("draw_grid", false);
      // twin axis doesn't have a grid
      if (axis_elem->hasAttribute("location") && !strEqualsAny(location, "x", "y"))
        axis_elem->setAttribute("draw_grid", false);
      axis_elem->setAttribute("mirrored_axis", mirrored_axis);
      axis_elem->setAttribute("label_orientation", label_orientation);
    }
  // create tick_group elements
  if (!element->querySelectors("tick_group") ||
      (axis_type == "x" && ((!std::isnan(old_window[0]) && old_window[0] != window[0]) ||
                            (!std::isnan(old_window[1]) && old_window[1] != window[1]))) ||
      (axis_type == "y" && ((!std::isnan(old_window[2]) && old_window[2] != window[2]) ||
                            (!std::isnan(old_window[3]) && old_window[3] != window[3]))))
    {
      for (const auto &child : element->children())
        {
          if (child->localName() != "polyline" && child->hasAttribute("_child_id")) child->remove();
        }
      element->setAttribute("scientific_format", scientific_format);
      axisArgumentsConvertedIntoTickGroups(axis.ticks, axis.tick_labels, axis_elem, DelValues::RECREATE_OWN_CHILDREN);
    }
  // polyline for axis-line
  if (axis_type == "x")
    {
      line_x_min = axis.min;
      line_x_max = axis.max;
      line_y_min = line_y_max = axis.position;
    }
  else if (axis_type == "y")
    {
      line_x_min = line_x_max = axis.position;
      line_y_min = axis.min;
      line_y_max = axis.max;
    }
  if (strEqualsAny(location, "bottom", "top", "twin_x") && axis_type == "x")
    {
      adjustValueForNonStandardAxis(plot_parent, &line_x_min, location);
      adjustValueForNonStandardAxis(plot_parent, &line_x_max, location);
      if (location == "twin_x") line_y_min = line_y_max = window[3];
      if (scale & GR_OPTION_FLIP_Y || y_flip) line_y_min = line_y_max = window[2];
    }
  else if (strEqualsAny(location, "left", "right", "twin_y") && axis_type == "y")
    {
      adjustValueForNonStandardAxis(plot_parent, &line_y_min, location);
      adjustValueForNonStandardAxis(plot_parent, &line_y_max, location);
      if (location == "twin_y") line_x_min = line_x_max = window[1];
      if (scale & GR_OPTION_FLIP_X || x_flip) line_x_min = line_x_max = window[0];
    }
  if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
    {
      line = global_creator->createPolyline(line_x_min, line_x_max, line_y_min, line_y_max);
      line->setAttribute("_child_id", 0);
      axis_elem->append(line);
    }
  else
    {
      line = element->querySelectors("polyline[_child_id=0]");
      if (line != nullptr)
        global_creator->createPolyline(line_x_min, line_x_max, line_y_min, line_y_max, 0, 0.0, 0, line);
    }
  if (line != nullptr && del != DelValues::UPDATE_WITHOUT_DEFAULT)
    {
      int z_index = axis_type == "x" ? 0 : -2;
      line->setAttribute("name", axis_type + "-axis-line");
      line->setAttribute("z_index", element->parentElement()->localName() == "colorbar" ? 2 : z_index);
    }
  if (axis_type == "x")
    {
      line_y_min = line_y_max = window[3];
      if (scale & GR_OPTION_FLIP_Y || y_flip) line_y_min = line_y_max = window[2];
    }
  else if (axis_type == "y")
    {
      line_x_min = line_x_max = window[1];
      if (scale & GR_OPTION_FLIP_X || x_flip) line_x_min = line_x_max = window[0];
    }
  if (mirrored_axis)
    {
      if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
        {
          line = global_creator->createPolyline(line_x_min, line_x_max, line_y_min, line_y_max);
          line->setAttribute("_child_id", 1);
          axis_elem->append(line);
        }
      else
        {
          line = element->querySelectors("polyline[_child_id=1]");
          if (line != nullptr)
            global_creator->createPolyline(line_x_min, line_x_max, line_y_min, line_y_max, 0, 0.0, 0, line);
        }
      if (line != nullptr && del != DelValues::UPDATE_WITHOUT_DEFAULT)
        {
          line->setAttribute("name", axis_type + "-axis-line mirrored");
          line->setAttribute("z_index", axis_type == "x" ? -1 : -3);
        }
    }

  applyMoveTransformation(element);
  gr_freeaxis(&axis);
}

void processRadialAxes(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  double window[4], old_window[4];
  double first_tick, last_tick, new_tick, r_max;
  double min_scale = 0, max_scale; // used for r_log (with negative exponents)
  double factor = 1;
  int i, start_n = 1, labeled_arc_line_skip;
  std::string kind;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  int child_id = 0;
  double r_lim_min = NAN, r_lim_max = NAN;
  bool r_log = false, skip_calculations = false, keep_radii_axes = false, with_pan = false;
  std::shared_ptr<GRM::Element> central_region, plot_parent = element;
  int scientific_format = 0;
  auto global_render = grm_get_render();
  auto active_figure = global_render->getActiveFigure();
  auto global_creator = grm_get_creator();

  getPlotParent(plot_parent);
  for (const auto &child : plot_parent->children())
    {
      if (child->localName() == "central_region")
        {
          central_region = child;
          break;
        }
    }

  /* `processAxis` can be triggered indirectly by `grm_input` but within the interaction processing the default Latin-1
   * encoding is used instead of the configured text encoding. Setting the correct text encoding is important since
   * functions like `gr_axis` modify the axis text based on the chosen encoding. */
  processTextEncoding(active_figure);

  window[0] = static_cast<double>(central_region->getAttribute("window_x_min"));
  window[1] = static_cast<double>(central_region->getAttribute("window_x_max"));
  window[2] = static_cast<double>(central_region->getAttribute("window_y_min"));
  window[3] = static_cast<double>(central_region->getAttribute("window_y_max"));

  with_pan =
      plot_parent->hasAttribute("polar_with_pan") && static_cast<int>(plot_parent->getAttribute("polar_with_pan"));
  if (with_pan)
    {
      factor = grm_max(abs(window[0]), grm_max(abs(window[1]), grm_max(abs(window[2]), abs(window[3])))) * sqrt(2);
      if (element->hasAttribute("_window_old_x_min"))
        old_window[0] = static_cast<double>(element->getAttribute("_window_old_x_min"));
      if (element->hasAttribute("_window_old_x_max"))
        old_window[1] = static_cast<double>(element->getAttribute("_window_old_x_max"));
      if (element->hasAttribute("_window_old_y_min"))
        old_window[2] = static_cast<double>(element->getAttribute("_window_old_y_min"));
      if (element->hasAttribute("_window_old_y_max"))
        old_window[3] = static_cast<double>(element->getAttribute("_window_old_y_max"));
      element->setAttribute("_window_old_x_min", window[0]);
      element->setAttribute("_window_old_x_max", window[1]);
      element->setAttribute("_window_old_y_min", window[2]);
      element->setAttribute("_window_old_y_max", window[3]);
      if (old_window[0] == window[0] && old_window[1] == window[1] && old_window[2] == window[2] &&
          old_window[3] == window[3])
        return;
    }

  if (element->hasAttribute("scientific_format"))
    scientific_format = static_cast<int>(element->getAttribute("scientific_format"));

  if (plot_parent->hasAttribute("r_lim_min") && plot_parent->hasAttribute("r_lim_max"))
    {
      r_lim_min = static_cast<double>(plot_parent->getAttribute("r_lim_min"));
      r_lim_max = static_cast<double>(plot_parent->getAttribute("r_lim_max"));
      if (plot_parent->hasAttribute("keep_radii_axes"))
        keep_radii_axes = static_cast<int>(plot_parent->getAttribute("keep_radii_axes"));
    }

  if (central_region->hasAttribute("_skip_calculations"))
    {
      skip_calculations = static_cast<int>(central_region->getAttribute("_skip_calculations"));
      central_region->removeAttribute("_skip_calculations");
    }

  if (!skip_calculations) calculatePolarLimits(central_region, context);

  kind = static_cast<std::string>(plot_parent->getAttribute("_kind"));
  if (plot_parent->hasAttribute("r_log")) r_log = static_cast<int>(plot_parent->getAttribute("r_log"));

  if (!element->hasAttribute("_line_type_set_by_user")) global_render->setLineType(element, GKS_K_LINETYPE_SOLID);

  /* clear old radial_axes_elements */
  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  // Draw Text
  if (!element->hasAttribute("_text_align_vertical_set_by_user"))
    element->setAttribute("text_align_vertical", GKS_K_TEXT_VALIGN_HALF);
  if (!element->hasAttribute("_text_align_horizontal_set_by_user"))
    element->setAttribute("text_align_horizontal", GKS_K_TEXT_HALIGN_LEFT);

  if (!grm_isnan(r_lim_min) && !grm_isnan(r_lim_max) && !keep_radii_axes)
    {
      first_tick = r_lim_min;
      last_tick = r_lim_max;
    }
  else
    {
      // without y_lim_min and y_lim_max the origin/first_tick is always 0.0 except for r_log
      last_tick = static_cast<double>(central_region->getAttribute("r_max"));
      first_tick = (r_log) ? static_cast<double>(central_region->getAttribute("r_min")) : 0.0;
    }

  if (r_log) // min_scale only needed with r_log
    {
      min_scale = ceil(abs(log10(first_tick)));
      if (min_scale != 0.0) min_scale *= (log10(first_tick) / abs(log10(first_tick)));

      max_scale = ceil(abs(log10(last_tick)));
      if (max_scale != 0.0) max_scale *= log10(last_tick) / abs(log10(last_tick)); // add signum of max_scale if not 0.0
    }

  // for currently unsupported r_log polar plots (hist, heatmap)
  if (std::isnan(min_scale)) r_log = false;

  r_max = last_tick;
  if (element->hasAttribute("_r_max")) r_max = static_cast<double>(element->getAttribute("_r_max"));
  if ((central_region->hasAttribute("_zoomed") && static_cast<int>(central_region->getAttribute("_zoomed"))) ||
      r_max != last_tick)
    {
      for (const auto &child : element->children())
        {
          child->remove(); // Todo: Change this
        }
      del = DelValues::RECREATE_OWN_CHILDREN;
    }
  new_tick = autoTick(first_tick, r_max);

  auto n = static_cast<int>(round(((r_max * factor) - first_tick) / new_tick));
  if (n <= 4 && kind != "polar_histogram") // special case to get some more useful axes
    {
      new_tick /= 2;
      n *= 2;
    }

  if (r_log)
    {
      start_n = 2 * floor(log10(last_tick) - log10(first_tick)) * factor;
      n = 2 * floor(log10(r_max) - log10(first_tick)) * factor;
    }
  // mechanism to reduce the labels for polar_with_pan and r_log, where n could be pretty high
  labeled_arc_line_skip = 2 + 2 * floor(n / 15);

  int cnt = labeled_arc_line_skip;
  for (i = 0; i <= n + 1; i++) // Create arc_grid_lines and radial_axes line
    {
      std::shared_ptr<GRM::Element> arc_grid_line;
      std::string value_string;
      char text_buffer[PLOT_POLAR_AXES_TEXT_BUFFER] = "";
      double r;
      if (!r_log)
        {
          r = i * new_tick / (r_max - first_tick);
        }
      else
        {
          r = ((i / 2) + (i % 2 == 1 ? log10(5) : 0)) / (log10(r_max) - log10(first_tick));
          if (!with_pan && r > 1) r = 1;
        }

      if (i == n + 1) r = 1;
      if (!with_pan && r > 1) continue;
      if (i % labeled_arc_line_skip == 0 || r == 1)
        {
          double value = first_tick + i * new_tick;
          if (r_log) value = pow(10, (i / 2) + floor(log10(first_tick)));
          if (r == 1)
            {
              double tmp_tick = (last_tick - first_tick) / n;
              if (!with_pan) tmp_tick *= (abs(window[3] - window[2]) / 2.0);

              if (r_log && !with_pan)
                value =
                    first_tick + pow(10, floor(log10(first_tick)) + start_n / 2 * (abs(window[3] - window[2]) / 2.0));
              else
                value = (i == n + 1) ? first_tick + tmp_tick * (i - ((i - 1) * tmp_tick / (r_max - first_tick)))
                                     : first_tick + tmp_tick * i;
              element->setAttribute("_r_max", value);
            }
          if (!with_pan || (window[0] < 0 && window[1] > 0 && r > window[2] && window[3] > r))
            {
              if (r_log) // r_log uses the exponential notation
                {
                  snprintf(text_buffer, PLOT_POLAR_AXES_TEXT_BUFFER, "%d^{%.0f}", 10, log10(value));
                  value_string = text_buffer;
                }
              else
                {
                  format_reference_t reference;
                  gr_getformat(&reference, first_tick, first_tick, last_tick, new_tick, 2);
                  snprintf(text_buffer, PLOT_POLAR_AXES_TEXT_BUFFER, "%s", std::to_string(value).c_str());
                  value_string = gr_ftoa(text_buffer, value, &reference);

                  if (value_string.size() > 7)
                    {
                      reference = {1, 1};
                      const char minus[] = {(char)0xe2, (char)0x88, (char)0x92, '\0'}; // gr minus sign
                      auto em_dash = std::string(minus);
                      size_t start_pos = 0;

                      scientific_format = 2;
                      gr_setscientificformat(scientific_format);

                      if (startsWith(value_string, em_dash)) start_pos = em_dash.size();
                      auto without_minus = value_string.substr(start_pos);

                      snprintf(text_buffer, PLOT_POLAR_AXES_TEXT_BUFFER, "%s", without_minus.c_str());
                      value_string = gr_ftoa(text_buffer, atof(without_minus.c_str()), &reference);
                      if (start_pos != 0) value_string = em_dash.append(value_string);
                      element->setAttribute("scientific_format", scientific_format);
                    }
                }
            }
        }

      // skip the lines without label in r_log case
      if (i > cnt) cnt += labeled_arc_line_skip;
      if (r_log && i != cnt && (labeled_arc_line_skip != 2 || i != cnt - 1) && (r != 1 && !with_pan)) continue;

      if (r != 1)
        {
          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              arc_grid_line = global_creator->createArcGridLine(r);
              arc_grid_line->setAttribute("_child_id", child_id++);
              element->append(arc_grid_line);
            }
          else
            {
              arc_grid_line = element->querySelectors("arc_grid_line[_child_id=" + std::to_string(child_id++) + "]");
              if (arc_grid_line != nullptr) global_creator->createArcGridLine(r, arc_grid_line);
            }
          if (arc_grid_line != nullptr)
            {
              int line_color_ind = 90; // Todo: make the line_color editable like 2d axis
              if (i % 2 == 0 && i > 0) line_color_ind = 88;
              if (!arc_grid_line->hasAttribute("_line_color_ind_set_by_user"))
                {
                  if (element->hasAttribute("line_color_ind"))
                    line_color_ind = static_cast<int>(element->getAttribute("line_color_ind"));
                  arc_grid_line->setAttribute("line_color_ind", line_color_ind);
                }
              if (!value_string.empty()) arc_grid_line->setAttribute("arc_label", value_string);
            }
        }
      else
        {
          std::shared_ptr<GRM::Element> arc, text;
          double theta_lim_min = 0, theta_lim_max = 360;
          if (plot_parent->hasAttribute("theta_lim_min") && plot_parent->hasAttribute("theta_lim_max"))
            {
              theta_lim_min = static_cast<double>(plot_parent->getAttribute("theta_lim_min"));
              theta_lim_max = static_cast<double>(plot_parent->getAttribute("theta_lim_max"));
            }
          double theta0 = 0.05 * (window[1] - window[0]) / 2.0;

          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              if (!with_pan)
                arc = global_creator->createDrawArc(window[0], window[1], window[2], window[3], theta_lim_min,
                                                    theta_lim_max);
              else
                arc = global_creator->createDrawArc(-1, 1, -1, 1, theta_lim_min, theta_lim_max);
              arc->setAttribute("_child_id", child_id++);
              element->append(arc);
            }
          else
            {
              arc = element->querySelectors("draw_arc[_child_id=" + std::to_string(child_id++) + "]");
              if (arc != nullptr && !with_pan)
                global_creator->createDrawArc(window[0], window[1], window[2], window[3], theta_lim_min, theta_lim_max,
                                              arc);
              else if (arc != nullptr)
                global_creator->createDrawArc(-1, 1, -1, 1, theta_lim_min, theta_lim_max, arc);
            }
          if (arc != nullptr)
            {
              if (!with_pan) arc->setAttribute("name", "radial-axes line");
              arc->setAttribute("line_color_ind", 88);
            }

          if (i % labeled_arc_line_skip == 0 && i == n)
            {
              double r0 = window[3];

              adjustPolarGridLineTextPosition(theta_lim_min, theta_lim_max, &theta0, &r0, 1, central_region);
              if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
                {
                  text = global_creator->createText(theta0, r0, value_string, CoordinateSpace::WC);
                  text->setAttribute("_child_id", child_id++);
                  element->append(text);
                }
              else
                {
                  text = element->querySelectors("text[_child_id=" + std::to_string(child_id++) + "]");
                  if (text != nullptr) global_creator->createText(theta0, r0, value_string, CoordinateSpace::WC, text);
                }
              if (text != nullptr)
                {
                  text->setAttribute("name", "radial-axes line");
                  if (r_log) text->setAttribute("scientific_format", 2);
                  if (element->hasAttribute("scientific_format"))
                    text->setAttribute("scientific_format", scientific_format);
                  if (theta_lim_min > 0 || theta_lim_max < 360)
                    {
                      // use correct aligment for non standard axes
                      if (theta_lim_min == 180 || theta_lim_min == 0)
                        text->setAttribute("text_align_horizontal", "center");
                      else if (theta_lim_min < 180)
                        text->setAttribute("text_align_horizontal", "left");
                      else if (theta_lim_min > 180)
                        text->setAttribute("text_align_horizontal", "right");
                      if (theta_lim_min < 90 || theta_lim_min > 270)
                        text->setAttribute("text_align_vertical", "top");
                      else if (theta_lim_min == 90 || theta_lim_min == 270)
                        text->setAttribute("text_align_vertical", "half");
                      else if (theta_lim_min > 90 && theta_lim_min < 270)
                        text->setAttribute("text_align_vertical", "bottom");
                    }
                }
            }
          if (!with_pan) break;
        }
    }

  if (element->parentElement()->hasAttribute("char_height")) processCharHeight(element->parentElement());
  processLineType(element->parentElement());
  processTextAlign(element->parentElement());
}

void processAngleLine(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  double text_x0 = 0.0, text_y0 = 0.0, factor = 1.1;
  std::string angle_label;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  int child_id = 0;
  bool with_pan = false;
  std::shared_ptr<GRM::Element> plot_parent = element, line, text;
  getPlotParent(plot_parent);
  auto global_render = grm_get_render();
  auto global_creator = grm_get_creator();

  with_pan =
      plot_parent->hasAttribute("polar_with_pan") && static_cast<int>(plot_parent->getAttribute("polar_with_pan"));
  if (with_pan) factor = 0.9;

  /* clear old angle_line elements */
  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  if (!element->hasAttribute("_line_type_set_by_user")) global_render->setLineType(element, GKS_K_LINETYPE_SOLID);

  auto line_theta0 = static_cast<double>(element->getAttribute("theta"));
  auto line_r0 = static_cast<double>(element->getAttribute("r"));
  if (element->hasAttribute("angle_label"))
    {
      angle_label = static_cast<std::string>(element->getAttribute("angle_label"));
      if (!angle_label.empty())
        {
          if (!element->hasAttribute("text_x0")) throw NotFoundError("Missing text_x0 data for given angle_label!\n");
          text_x0 = static_cast<double>(element->getAttribute("text_x0"));
          if (!element->hasAttribute("text_y0")) throw NotFoundError("Missing text_y0 data for given angle_label!\n");
          text_y0 = static_cast<double>(element->getAttribute("text_y0"));
        }
    }

  if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
    {
      line = global_creator->createPolyline(line_theta0, 0.0, line_r0, 0.0);
      line->setAttribute("_child_id", child_id++);
      element->append(line);
    }
  else
    {
      line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
      if (line != nullptr) global_creator->createPolyline(line_theta0, 0.0, line_r0, 0.0, 0, 0.0, 0, line);
    }
  if (line != nullptr)
    {
      if (del != DelValues::UPDATE_WITHOUT_DEFAULT)
        {
          if (!line->hasAttribute("_line_color_ind_set_by_user"))
            {
              auto line_color_ind = 88;
              if (element->hasAttribute("line_color_ind"))
                line_color_ind = static_cast<int>(element->getAttribute("line_color_ind"));
              global_render->setLineColorInd(line, line_color_ind);
            }
        }
    }

  if (!angle_label.empty())
    {
      if (with_pan) text = element->querySelectors("text[_child_id=" + std::to_string(child_id) + "]");
      if ((del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT) ||
          (with_pan && text == nullptr))
        {
          text = global_creator->createText(text_x0 * factor, text_y0 * factor, angle_label, CoordinateSpace::WC);
          text->setAttribute("_child_id", child_id++);
          element->append(text);
        }
      else
        {
          text = element->querySelectors("text[_child_id=" + std::to_string(child_id++) + "]");
          if (text != nullptr)
            global_creator->createText(text_x0 * factor, text_y0 * factor, angle_label, CoordinateSpace::WC, text);
        }
      if (text != nullptr && del != DelValues::UPDATE_WITHOUT_DEFAULT)
        {
          if (!text->hasAttribute("_text_align_vertical_set_by_user"))
            {
              auto text_align_vertical = GKS_K_TEXT_VALIGN_HALF;
              if (element->hasAttribute("text_align_vertical"))
                text_align_vertical = static_cast<int>(element->getAttribute("text_align_vertical"));
              text->setAttribute("text_align_vertical", text_align_vertical);
            }
          if (!text->hasAttribute("_text_align_horizontal_set_by_user"))
            {
              auto text_align_horizontal = GKS_K_TEXT_HALIGN_CENTER;
              if (element->hasAttribute("text_align_horizontal"))
                text_align_horizontal = static_cast<int>(element->getAttribute("text_align_horizontal"));
              text->setAttribute("text_align_horizontal", text_align_horizontal);
            }
        }
    }
  else if (with_pan)
    {
      text = element->querySelectors("text[_child_id=" + std::to_string(child_id++) + "]");
      if (text != nullptr) element->removeChild(text);
    }
}

void processThetaAxes(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  double window[4];
  char text_buffer[PLOT_POLAR_AXES_TEXT_BUFFER];
  std::string text;
  double factor = 1;
  int angle_line_num = 8;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  int child_id = 0;
  bool skip_calculations = false, theta_flip = false, pass = false, with_pan = false;
  double theta_lim_min = 0, theta_lim_max = 360;
  std::shared_ptr<GRM::Element> central_region, plot_parent = element, axes_text_group;
  auto global_render = grm_get_render();
  auto global_creator = grm_get_creator();

  getPlotParent(plot_parent);
  for (const auto &child : plot_parent->children())
    {
      if (child->localName() == "central_region")
        {
          central_region = child;
          break;
        }
    }

  window[0] = static_cast<double>(central_region->getAttribute("window_x_min"));
  window[1] = static_cast<double>(central_region->getAttribute("window_x_max"));
  window[2] = static_cast<double>(central_region->getAttribute("window_y_min"));
  window[3] = static_cast<double>(central_region->getAttribute("window_y_max"));

  with_pan =
      plot_parent->hasAttribute("polar_with_pan") && static_cast<int>(plot_parent->getAttribute("polar_with_pan"));
  if (with_pan)
    factor = grm_max(abs(window[0]), grm_max(abs(window[1]), grm_max(abs(window[2]), abs(window[3])))) * sqrt(2);

  if (central_region->hasAttribute("_skip_calculations"))
    {
      skip_calculations = static_cast<int>(central_region->getAttribute("_skip_calculations"));
      central_region->removeAttribute("_skip_calculations");
    }

  if (!skip_calculations) calculatePolarLimits(central_region, context);
  if (plot_parent->hasAttribute("theta_flip")) theta_flip = static_cast<int>(plot_parent->getAttribute("theta_flip"));
  if (plot_parent->hasAttribute("theta_lim_min") && plot_parent->hasAttribute("theta_lim_max"))
    {
      theta_lim_min = static_cast<double>(plot_parent->getAttribute("theta_lim_min"));
      theta_lim_max = static_cast<double>(plot_parent->getAttribute("theta_lim_max"));
    }
  if (with_pan)
    {
      theta_lim_min = 0;
      theta_lim_max = 360;
    }
  if (element->hasAttribute("angle_line_num"))
    angle_line_num = static_cast<int>(element->getAttribute("angle_line_num"));

  if (!element->hasAttribute("_line_type_set_by_user")) global_render->setLineType(element, GKS_K_LINETYPE_SOLID);

  /* clear old theta_axes_elements */
  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  if (angle_line_num != 8) del = DelValues::RECREATE_OWN_CHILDREN; // could be improved
  clearOldChildren(&del, element);

  // Draw sector lines
  auto interval = 360.0 / angle_line_num;
  for (int i = 0; i <= angle_line_num; i++)
    {
      std::shared_ptr<GRM::Element> angle_line;
      int text_number = 0;
      double alpha = i * interval;
      double theta0 = std::cos(alpha * M_PI / 180.0), r0 = std::sin(alpha * M_PI / 180.0);

      if (alpha == 360 && theta_lim_max == 360) continue;
      if (alpha < theta_lim_min && alpha + interval > theta_lim_min)
        {
          theta0 = std::cos(theta_lim_min * M_PI / 180.0), r0 = std::sin(theta_lim_min * M_PI / 180.0);
          pass = true;
        }
      if (alpha > theta_lim_max && alpha - interval < theta_lim_max)
        {
          theta0 = std::cos(theta_lim_max * M_PI / 180.0), r0 = std::sin(theta_lim_max * M_PI / 180.0);
          pass = true;
        }
      if (!with_pan)
        {
          theta0 *= window[1];
          r0 *= window[3];
        }

      // define the number which will be stored inside the text
      text_number = theta_flip ? 360 - static_cast<int>(grm_round(alpha)) : static_cast<int>(grm_round(alpha));
      snprintf(text_buffer, PLOT_POLAR_AXES_TEXT_BUFFER, "%d\xc2\xb0", text_number);
      text = text_buffer;

      if ((alpha >= theta_lim_min && alpha <= theta_lim_max) || pass)
        {
          if (pass)
            {
              text = "";
              gr_setclip(0);
            }
          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              angle_line = global_creator->createAngleLine(theta0 * factor, r0 * factor, text);
              angle_line->setAttribute("_child_id", child_id++);
              element->append(angle_line);
            }
          else
            {
              angle_line = element->querySelectors("angle_line[_child_id=" + std::to_string(child_id++) + "]");
              if (angle_line != nullptr)
                global_creator->createAngleLine(theta0 * factor, r0 * factor, text, angle_line);
            }
          if (angle_line != nullptr)
            {
              if (with_pan && (window[0] >= theta0 * 0.9 || theta0 * 0.9 >= window[1] || window[2] >= r0 * 0.9 ||
                               r0 * 0.9 >= window[3]))
                {
                  text = "";
                  if (angle_line->hasAttribute("text_x0")) angle_line->removeAttribute("text_x0");
                  if (angle_line->hasAttribute("text_y0")) angle_line->removeAttribute("text_y0");
                  angle_line->setAttribute("angle_label", text);
                }
              else if (!text.empty())
                {
                  angle_line->setAttribute("text_x0", theta0);
                  angle_line->setAttribute("text_y0", r0);
                }
            }
          if (pass)
            {
              pass = false;
              gr_setclip(1);
            }
        }
    }

  if (element->parentElement()->hasAttribute("char_height")) processCharHeight(element->parentElement());
  processLineType(element->parentElement());
  processTextAlign(element->parentElement());
}

void processDrawRect(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for draw_rect
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  auto x_min = static_cast<double>(element->getAttribute("x_min"));
  auto x_max = static_cast<double>(element->getAttribute("x_max"));
  auto y_min = static_cast<double>(element->getAttribute("y_min"));
  auto y_max = static_cast<double>(element->getAttribute("y_max"));
  applyMoveTransformation(element);
  if (grm_get_render()->getRedrawWs()) gr_drawrect(x_min, x_max, y_min, y_max);
}

void processFillArc(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for fill_arc
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  auto x_min = static_cast<double>(element->getAttribute("x_min"));
  auto x_max = static_cast<double>(element->getAttribute("x_max"));
  auto y_min = static_cast<double>(element->getAttribute("y_min"));
  auto y_max = static_cast<double>(element->getAttribute("y_max"));
  auto start_angle = static_cast<double>(element->getAttribute("start_angle"));
  auto end_angle = static_cast<double>(element->getAttribute("end_angle"));
  applyMoveTransformation(element);

  if (element->parentElement()->localName() == "polar_bar")
    processTransparency(element->parentElement()->parentElement());

  if (grm_get_render()->getRedrawWs()) gr_fillarc(x_min, x_max, y_min, y_max, start_angle, end_angle);
}

void processFillRect(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for fill_rect
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  auto x_min = static_cast<double>(element->getAttribute("x_min"));
  auto x_max = static_cast<double>(element->getAttribute("x_max"));
  auto y_min = static_cast<double>(element->getAttribute("y_min"));
  auto y_max = static_cast<double>(element->getAttribute("y_max"));
  applyMoveTransformation(element);

  if (element->parentElement()->localName() == "bar" &&
      element->parentElement()->parentElement()->hasAttribute("transparency"))
    processTransparency(element->parentElement()->parentElement());

  if (grm_get_render()->getRedrawWs()) gr_fillrect(x_min, x_max, y_min, y_max);
}

void processFillArea(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for fillArea
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));

  auto x_vec = GRM::get<std::vector<double>>((*context)[x]);
  auto y_vec = GRM::get<std::vector<double>>((*context)[y]);

  int n = std::min<int>(static_cast<int>(x_vec.size()), static_cast<int>(y_vec.size()));
  applyMoveTransformation(element);

  if (grm_get_render()->getRedrawWs()) gr_fillarea(n, (double *)&(x_vec[0]), (double *)&(y_vec[0]));
}

void processGrid3d(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for grid 3d
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double x_tick, y_tick, z_tick;
  double x_org, y_org, z_org;
  int x_major, y_major, z_major;
  std::string x_org_pos = PLOT_DEFAULT_ORG_POS, y_org_pos = PLOT_DEFAULT_ORG_POS, z_org_pos = PLOT_DEFAULT_ORG_POS;
  if (element->hasAttribute("x_origin_pos"))
    x_org_pos = static_cast<std::string>(element->getAttribute("x_origin_pos"));
  if (element->hasAttribute("y_origin_pos"))
    y_org_pos = static_cast<std::string>(element->getAttribute("y_origin_pos"));
  if (element->hasAttribute("z_origin_pos"))
    z_org_pos = static_cast<std::string>(element->getAttribute("z_origin_pos"));

  getAxes3dInformation(element, x_org_pos, y_org_pos, z_org_pos, x_org, y_org, z_org, x_major, y_major, z_major, x_tick,
                       y_tick, z_tick);
  applyMoveTransformation(element);
  processWindow(element->parentElement()->parentElement());
  processSpace3d(element->parentElement()->parentElement());

  if (grm_get_render()->getRedrawWs())
    gr_grid3d(x_tick, y_tick, z_tick, x_org, y_org, z_org, abs(x_major), abs(y_major), abs(z_major));
}

void processGridLine(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  std::shared_ptr<GRM::Element> axis_elem = element->parentElement()->parentElement();
  std::shared_ptr<GRM::Element> plot_parent = element;
  getPlotParent(plot_parent);

  auto coordinate_system = plot_parent->querySelectors("coordinate_system");
  bool hide =
      (coordinate_system->hasAttribute("hide")) ? static_cast<int>(coordinate_system->getAttribute("hide")) : false;
  auto coordinate_system_type = static_cast<std::string>(coordinate_system->getAttribute("plot_type"));
  auto axis_type = static_cast<std::string>(axis_elem->getAttribute("axis_type"));
  auto min_val = static_cast<double>(axis_elem->getAttribute("min_value"));
  auto max_val = static_cast<double>(axis_elem->getAttribute("max_value"));
  auto org = static_cast<double>(axis_elem->getAttribute("origin"));
  auto pos = static_cast<double>(axis_elem->getAttribute("pos"));
  auto tick = static_cast<double>(axis_elem->getAttribute("tick"));
  auto major_count = static_cast<int>(axis_elem->getAttribute("major_count"));
  auto value = static_cast<double>(element->getAttribute("value"));
  if (element->hasAttribute("_value_set_by_user"))
    {
      value = static_cast<double>(element->getAttribute("_value_set_by_user"));
      element->setAttribute("value", value);
    }
  auto is_major = static_cast<int>(element->getAttribute("is_major"));
  if (element->hasAttribute("_is_major_set_by_user"))
    {
      is_major = static_cast<int>(element->getAttribute("_is_major_set_by_user"));
      element->setAttribute("is_major", is_major);
    }

  processPrivateTransparency(element);
  if (element->hasAttribute("transparency")) processTransparency(element);

  tick_t g = {value, is_major};
  axis_t grid = {min_val, max_val, tick, org, pos, major_count, 1, &g, 0.0, 0, nullptr, NAN, false, 0};
  if (grm_get_render()->getRedrawWs() && !hide &&
      (coordinate_system_type == "2d" || axis_elem->parentElement()->localName() == "colorbar"))
    {
      if (axis_type == "x")
        {
          gr_drawaxes(&grid, nullptr, 4);
        }
      else
        {
          gr_drawaxes(nullptr, &grid, 4);
        }
    }
}

void processIntegral(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  double int_lim_low = 0, int_lim_high;
  std::vector<double> x_vec, y_vec, f1, f2;
  int x_length;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  int child_id = 0, id, i;
  std::shared_ptr<GRM::Element> fill_area, left_border, right_border;
  std::string str;
  auto plot_element = getPlotElement(element);
  auto series_element = element->parentElement()->parentElement();
  double x_shift = 0;
  double x1, x2;
  auto global_root = grm_get_document_root();
  auto global_creator = grm_get_creator();

  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  if (element->hasAttribute("int_lim_low")) int_lim_low = static_cast<double>(element->getAttribute("int_lim_low"));
  /* is always given from definition */
  int_lim_high = static_cast<double>(element->getAttribute("int_lim_high"));

  /* if there is a shift defined for x, get the value to shift the x-values in later calculation */
  fill_area = element->querySelectors("fill_area[_child_id=" + std::to_string(child_id) + "]");
  if (fill_area != nullptr && fill_area->hasAttribute("x_shift_wc") &&
      !(fill_area->hasAttribute("disable_x_trans") && static_cast<int>(fill_area->getAttribute("disable_x_trans"))))
    {
      x_shift = static_cast<double>(fill_area->getAttribute("x_shift_wc"));
      int_lim_low += x_shift;
      int_lim_high += x_shift;

      /* trigger an event with the new integral size so that the user can use it for calculations */
      eventQueueEnqueueIntegralUpdateEvent(event_queue, int_lim_low, int_lim_high);
    }
  else if ((fill_area == nullptr || (fill_area != nullptr && !fill_area->hasAttribute("x_shift_wc"))) &&
           element->hasAttribute("x_shift_wc") &&
           !(element->hasAttribute("disable_x_trans") && static_cast<int>(element->getAttribute("disable_x_trans"))))
    {
      x_shift = static_cast<double>(element->getAttribute("x_shift_wc"));
      int_lim_low += x_shift;
      int_lim_high += x_shift;

      /* trigger an event with the new integral size so that the user can use it for calculations */
      eventQueueEnqueueIntegralUpdateEvent(event_queue, int_lim_low, int_lim_high);
    }

  if (int_lim_high < int_lim_low)
    {
      fprintf(stderr, "Integral low limit is greater than the high limit. The limits gets swapped.\n");
      auto tmp = int_lim_high;
      int_lim_high = int_lim_low;
      int_lim_low = tmp;
    }

  auto y = static_cast<std::string>(series_element->getAttribute("y"));
  y_vec = GRM::get<std::vector<double>>((*context)[y]);

  auto x = static_cast<std::string>(series_element->getAttribute("x"));
  x_vec = GRM::get<std::vector<double>>((*context)[x]);
  x_length = static_cast<int>(x_vec.size());

  /* get all points for the fill area from the current line */
  x1 = (int_lim_low < x_vec[0]) ? x_vec[0] - x_shift : int_lim_low - x_shift;
  f1.push_back(x1);
  f2.push_back(static_cast<int>(plot_element->getAttribute("y_log")) ? 1 : 0);
  for (i = 0; i < x_length; i++)
    {
      if (grm_isnan(x_vec[i]) || grm_isnan(y_vec[i])) continue;
      if (x_vec[i] < int_lim_low && x_vec[i + 1] > int_lim_low)
        {
          f1.push_back(int_lim_low - x_shift);
          f2.push_back(y_vec[i] + (1 - abs(x_vec[i + 1] - int_lim_low)) * (y_vec[i + 1] - y_vec[i]));
        }
      if (x_vec[i] >= int_lim_low && x_vec[i] <= int_lim_high)
        {
          f1.push_back(x_vec[i] - x_shift);
          f2.push_back(y_vec[i]);
        }
      if (x_vec[i] < int_lim_high && x_vec[i + 1] > int_lim_high)
        {
          f1.push_back(int_lim_high - x_shift);
          f2.push_back(y_vec[i] + (1 - abs(x_vec[i + 1] - int_lim_high)) * (y_vec[i + 1] - y_vec[i]));
        }
    }
  x2 = (x_vec[x_length - 1] < int_lim_high) ? x_vec[x_length - 1] - x_shift : int_lim_high - x_shift;
  f1.push_back(x2);
  f2.push_back((static_cast<int>(plot_element->getAttribute("y_log"))) ? 1 : 0);

  id = static_cast<int>(global_root->getAttribute("_id"));
  global_root->setAttribute("_id", id + 1);
  str = std::to_string(id);

  if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
    {
      fill_area = global_creator->createFillArea("x" + str, f1, "y" + str, f2);
      fill_area->setAttribute("_child_id", child_id++);
      element->append(fill_area);
    }
  else
    {
      fill_area = element->querySelectors("fill_area[_child_id=" + std::to_string(child_id++) + "]");
      if (fill_area != nullptr)
        global_creator->createFillArea("x" + str, f1, "y" + str, f2, nullptr, 0, 0, -1, fill_area);
    }
  if (fill_area != nullptr)
    {
      int fill_color_ind = 989, fill_int_style = 2;
      /* when there is a color defined on the series element use it */
      if (series_element->hasAttribute("line_color_ind"))
        fill_color_ind = static_cast<int>(series_element->getAttribute("line_color_ind"));
      /* when there is a color defined on the integral_group element use it */
      if (element->parentElement()->hasAttribute("fill_color_ind"))
        fill_color_ind = static_cast<int>(element->parentElement()->getAttribute("fill_color_ind"));
      /* when there is a color defined on the polyline use it no matter if there was a color defined on the series */
      for (const auto &elem : series_element->querySelectorsAll("polyline[_child_id=0]"))
        {
          if (elem->hasAttribute("line_color_ind"))
            fill_color_ind = static_cast<int>(elem->getAttribute("line_color_ind"));
        }
      /* color on the integral element has the highest priority */
      if (element->hasAttribute("fill_color_ind"))
        fill_color_ind = static_cast<int>(element->getAttribute("fill_color_ind"));
      fill_area->setAttribute("fill_color_ind", fill_color_ind);
      /* when there is a fill int style defined on the integral_group element use it */
      if (element->parentElement()->hasAttribute("fill_int_style"))
        fill_int_style = static_cast<int>(element->parentElement()->getAttribute("fill_int_style"));
      if (element->hasAttribute("fill_int_style"))
        fill_int_style = static_cast<int>(element->getAttribute("fill_int_style"));
      fill_area->setAttribute("fill_int_style", fill_int_style);
      fill_area->setAttribute("name", "integral");
    }

  x1 += x_shift;
  if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
    {
      left_border = global_creator->createPolyline(x1, x1, 0, f2[1]);
      left_border->setAttribute("_child_id", child_id++);
      element->append(left_border);
    }
  else
    {
      left_border = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
      if (left_border != nullptr) global_creator->createPolyline(x1, x1, 0, f2[1], 0, 0.0, 0, left_border);
    }
  if (left_border != nullptr)
    {
      int line_color_ind = 1;
      double transparency = 0;
      if (element->hasAttribute("line_color_ind"))
        line_color_ind = static_cast<int>(element->getAttribute("line_color_ind"));
      left_border->setAttribute("line_color_ind", line_color_ind);
      left_border->setAttribute("name", "integral_left");
      if (left_border->hasAttribute("transparency"))
        transparency = static_cast<double>(left_border->getAttribute("transparency"));
      left_border->setAttribute("transparency", transparency);
    }

  x2 += x_shift;
  if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
    {
      right_border = global_creator->createPolyline(x2, x2, 0, f2[f2.size() - 2]);
      right_border->setAttribute("_child_id", child_id++);
      element->append(right_border);
    }
  else
    {
      right_border = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
      if (right_border != nullptr)
        global_creator->createPolyline(x2, x2, 0, f2[f2.size() - 2], 0, 0.0, 0, right_border);
    }
  if (right_border != nullptr)
    {
      int line_color_ind = 1;
      double transparency = 0;
      if (element->hasAttribute("line_color_ind"))
        line_color_ind = static_cast<int>(element->getAttribute("line_color_ind"));
      right_border->setAttribute("line_color_ind", line_color_ind);
      right_border->setAttribute("name", "integral_right");
      if (right_border->hasAttribute("transparency"))
        transparency = static_cast<double>(right_border->getAttribute("transparency"));
      right_border->setAttribute("transparency", transparency);
    }
}

void processIntegralGroup(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  std::vector<double> int_limits_high_vec, int_limits_low_vec;
  int limits_high_num = 0, limits_low_num = 0;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  int child_id = 0;
  std::shared_ptr<GRM::Element> integral;
  std::string str;
  auto global_creator = grm_get_creator();

  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  /* the following 2 attributes are required, so they must be set */
  if (!element->hasAttribute("int_limits_high")) throw NotFoundError("Missing required attribute int_limits_high");
  auto limits_high_key = static_cast<std::string>(element->getAttribute("int_limits_high"));
  int_limits_high_vec = GRM::get<std::vector<double>>((*context)[limits_high_key]);
  limits_high_num = (int)int_limits_high_vec.size();

  if (!element->hasAttribute("int_limits_low")) throw NotFoundError("Missing required attribute int_limits_low");
  auto limits_low_key = static_cast<std::string>(element->getAttribute("int_limits_low"));
  int_limits_low_vec = GRM::get<std::vector<double>>((*context)[limits_low_key]);
  limits_low_num = (int)int_limits_low_vec.size();

  if (limits_low_num != limits_high_num) throw std::length_error("Both limits must have the same number of arguments");

  /* create or update all the children */
  for (int i = 0; i < limits_low_num; i++)
    {
      if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
        {
          integral = global_creator->createIntegral(int_limits_low_vec[i], int_limits_high_vec[i]);
          integral->setAttribute("_child_id", child_id++);
          element->append(integral);
        }
      else
        {
          integral = element->querySelectors("integral[_child_id=" + std::to_string(child_id++) + "]");
          if (integral != nullptr)
            global_creator->createIntegral(int_limits_low_vec[i], int_limits_high_vec[i], integral);
        }
    }
}

void processArcGridLine(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  double window[4];
  std::string kind, arc_label;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  int child_id = 0;
  bool r_log = false, with_pan = false;
  double theta_lim_min = 0, theta_lim_max = 360;
  std::shared_ptr<GRM::Element> text, arc, plot_parent = element, central_region;
  auto global_render = grm_get_render();
  auto global_creator = grm_get_creator();

  getPlotParent(plot_parent);
  for (const auto &child : plot_parent->children())
    {
      if (child->localName() == "central_region")
        {
          central_region = child;
          break;
        }
    }

  with_pan =
      plot_parent->hasAttribute("polar_with_pan") && static_cast<int>(plot_parent->getAttribute("polar_with_pan"));

  /* clear old arc_grid_lines */
  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  window[0] = static_cast<double>(central_region->getAttribute("window_x_min"));
  window[1] = static_cast<double>(central_region->getAttribute("window_x_max"));
  window[2] = static_cast<double>(central_region->getAttribute("window_y_min"));
  window[3] = static_cast<double>(central_region->getAttribute("window_y_max"));

  if (!element->hasAttribute("_line_type_set_by_user")) global_render->setLineType(element, GKS_K_LINETYPE_SOLID);
  if (!element->hasAttribute("_text_align_vertical_set_by_user"))
    element->setAttribute("text_align_vertical", GKS_K_TEXT_VALIGN_HALF);
  if (!element->hasAttribute("_text_align_horizontal_set_by_user"))
    element->setAttribute("text_align_horizontal", GKS_K_TEXT_HALIGN_LEFT);

  kind = static_cast<std::string>(plot_parent->getAttribute("_kind"));
  if (plot_parent->hasAttribute("r_log")) r_log = static_cast<int>(plot_parent->getAttribute("r_log"));
  if (plot_parent->hasAttribute("theta_lim_min") && plot_parent->hasAttribute("theta_lim_max"))
    {
      theta_lim_min = static_cast<double>(plot_parent->getAttribute("theta_lim_min"));
      theta_lim_max = static_cast<double>(plot_parent->getAttribute("theta_lim_max"));
    }
  if (with_pan)
    {
      theta_lim_min = 0;
      theta_lim_max = 360;
    }

  auto value = static_cast<double>(element->getAttribute("value"));
  if (element->hasAttribute("arc_label")) arc_label = static_cast<std::string>(element->getAttribute("arc_label"));

  if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
    {
      if (!with_pan)
        arc = global_creator->createDrawArc(value * window[0], value * window[1], value * window[2], value * window[3],
                                            theta_lim_min, theta_lim_max);
      else
        arc = global_creator->createDrawArc(-value, value, -value, value, theta_lim_min, theta_lim_max);
      arc->setAttribute("_child_id", child_id++);
      element->append(arc);
    }
  else
    {
      arc = element->querySelectors("draw_arc[_child_id=" + std::to_string(child_id++) + "]");
      if (!with_pan && arc != nullptr)
        global_creator->createDrawArc(value * window[0], value * window[1], value * window[2], value * window[3],
                                      theta_lim_min, theta_lim_max, arc);
      else if (arc != nullptr)
        global_creator->createDrawArc(-value, value, -value, value, theta_lim_min, theta_lim_max, arc);
    }
  if (arc != nullptr)
    {
      if (kind != "polar_heatmap" && kind != "nonuniform_polar_heatmap") arc->setAttribute("z_index", -1);
      arc->setAttribute("name", "polar grid line");
      if (element->hasAttribute("line_color_ind"))
        arc->setAttribute("line_color_ind", static_cast<int>(element->getAttribute("line_color_ind")));
    }

  if (!arc_label.empty())
    {
      double theta0 = 0.05, r0 = value;
      if (!with_pan)
        {
          theta0 *= ((window[1] - window[0]) / 2.0);
          r0 *= window[3];
        }

      adjustPolarGridLineTextPosition(theta_lim_min, theta_lim_max, &theta0, &r0, value, central_region);

      if (with_pan) text = element->querySelectors("text[_child_id=" + std::to_string(child_id) + "]");
      if ((del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT) ||
          (with_pan && text == nullptr))
        {
          text = global_creator->createText(theta0, r0, arc_label, CoordinateSpace::WC);
          text->setAttribute("_child_id", child_id++);
          element->append(text);
        }
      else
        {
          text = element->querySelectors("text[_child_id=" + std::to_string(child_id++) + "]");
          if (text != nullptr) global_creator->createText(theta0, r0, arc_label, CoordinateSpace::WC, text);
        }
      if (text != nullptr)
        {
          if (r_log) text->setAttribute("scientific_format", 2);
          if (element->parentElement()->hasAttribute("scientific_format"))
            {
              auto scientific_format = static_cast<int>(element->parentElement()->getAttribute("scientific_format"));
              text->setAttribute("scientific_format", scientific_format);
            }
          text->setAttribute("z_index", 1);
          if (theta_lim_min > 0 || theta_lim_max < 360)
            {
              // use correct aligment for non standard axes
              if (theta_lim_min == 180 || theta_lim_min == 0)
                text->setAttribute("text_align_horizontal", "center");
              else if (theta_lim_min < 180)
                text->setAttribute("text_align_horizontal", "left");
              else if (theta_lim_min > 180)
                text->setAttribute("text_align_horizontal", "right");
              if (theta_lim_min < 90 || theta_lim_min > 270)
                text->setAttribute("text_align_vertical", "top");
              else if (theta_lim_min == 90 || theta_lim_min == 270)
                text->setAttribute("text_align_vertical", "half");
              else if (theta_lim_min > 90 && theta_lim_min < 270)
                text->setAttribute("text_align_vertical", "bottom");
            }
        }
    }
  else if (with_pan)
    {
      text = element->querySelectors("text[_child_id=" + std::to_string(child_id++) + "]");
      if (text != nullptr) element->removeChild(text);
    }
}

void processAxes3d(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for axes 3d
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double x_tick, y_tick, z_tick;
  double x_org, y_org, z_org;
  int x_major, y_major, z_major;
  int tick_orientation = 1;
  double tick_size;
  std::string x_org_pos = PLOT_DEFAULT_ORG_POS, y_org_pos = PLOT_DEFAULT_ORG_POS, z_org_pos = PLOT_DEFAULT_ORG_POS;
  auto global_render = grm_get_render();
  auto active_figure = global_render->getActiveFigure();

  /* `processAxis` can be triggered indirectly by `grm_input` but within the interaction processing the default Latin-1
   * encoding is used instead of the configured text encoding. Setting the correct text encoding is important since
   * functions like `gr_axis` modify the axis text based on the chosen encoding. */
  processTextEncoding(active_figure);

  if (element->hasAttribute("x_origin_pos"))
    x_org_pos = static_cast<std::string>(element->getAttribute("x_origin_pos"));
  if (element->hasAttribute("y_origin_pos"))
    y_org_pos = static_cast<std::string>(element->getAttribute("y_origin_pos"));
  if (element->hasAttribute("z_origin_pos"))
    z_org_pos = static_cast<std::string>(element->getAttribute("z_origin_pos"));

  getAxes3dInformation(element, x_org_pos, y_org_pos, z_org_pos, x_org, y_org, z_org, x_major, y_major, z_major, x_tick,
                       y_tick, z_tick);

  if (element->hasAttribute("tick_orientation"))
    tick_orientation = static_cast<int>(element->getAttribute("tick_orientation"));

  getTickSize(element, tick_size);
  tick_size *= tick_orientation;
  if (element->hasAttribute("_tick_size_set_by_user"))
    tick_size = static_cast<double>(element->getAttribute("_tick_size_set_by_user"));
  applyMoveTransformation(element);
  processWindow(element->parentElement()->parentElement());
  processSpace3d(element->parentElement()->parentElement());

  if (global_render->getRedrawWs())
    gr_axes3d(x_tick, y_tick, z_tick, x_org, y_org, z_org, x_major, y_major, z_major, tick_size);
}

void processCellArray(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for cell_array
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  auto xmin = static_cast<double>(element->getAttribute("x_min"));
  if (element->hasAttribute("_x_min_set_by_user"))
    {
      xmin = static_cast<double>(element->getAttribute("_x_min_set_by_user"));
      element->setAttribute("x_min", xmin);
    }
  auto xmax = static_cast<double>(element->getAttribute("x_max"));
  if (element->hasAttribute("_x_max_set_by_user"))
    {
      xmax = static_cast<double>(element->getAttribute("_x_max_set_by_user"));
      element->setAttribute("x_max", xmax);
    }
  auto ymin = static_cast<double>(element->getAttribute("y_min"));
  if (element->hasAttribute("_y_min_set_by_user"))
    {
      ymin = static_cast<double>(element->getAttribute("_y_min_set_by_user"));
      element->setAttribute("y_min", ymin);
    }
  auto ymax = static_cast<double>(element->getAttribute("y_max"));
  if (element->hasAttribute("_y_max_set_by_user"))
    {
      ymax = static_cast<double>(element->getAttribute("_y_max_set_by_user"));
      element->setAttribute("y_max", ymax);
    }
  auto dimx = static_cast<int>(element->getAttribute("x_dim"));
  if (element->hasAttribute("_x_dim_set_by_user"))
    {
      dimx = static_cast<int>(element->getAttribute("_x_dim_set_by_user"));
      element->setAttribute("x_dim", dimx);
    }
  auto dimy = static_cast<int>(element->getAttribute("y_dim"));
  if (element->hasAttribute("_y_dim_set_by_user"))
    {
      dimy = static_cast<int>(element->getAttribute("_y_dim_set_by_user"));
      element->setAttribute("y_dim", dimy);
    }
  auto scol = static_cast<int>(element->getAttribute("start_col"));
  if (element->hasAttribute("_start_col_set_by_user"))
    {
      scol = static_cast<int>(element->getAttribute("_start_col_set_by_user"));
      element->setAttribute("start_col", scol);
    }
  auto srow = static_cast<int>(element->getAttribute("start_row"));
  if (element->hasAttribute("_start_row_set_by_user"))
    {
      srow = static_cast<int>(element->getAttribute("_start_row_set_by_user"));
      element->setAttribute("start_row", srow);
    }
  auto ncol = static_cast<int>(element->getAttribute("num_col"));
  if (element->hasAttribute("_num_col_set_by_user"))
    {
      ncol = static_cast<int>(element->getAttribute("_num_col_set_by_user"));
      element->setAttribute("num_col", ncol);
    }
  auto nrow = static_cast<int>(element->getAttribute("num_row"));
  if (element->hasAttribute("_num_row_set_by_user"))
    {
      nrow = static_cast<int>(element->getAttribute("_num_row_set_by_user"));
      element->setAttribute("num_row", nrow);
    }
  auto color = static_cast<std::string>(element->getAttribute("color_ind_values"));

  if (element->parentElement()->localName() == "colorbar")
    processColormap(element->parentElement()->parentElement()->parentElement()->parentElement());
  applyMoveTransformation(element);
  if (grm_get_render()->getRedrawWs())
    gr_cellarray(xmin, xmax, ymin, ymax, dimx, dimy, scol, srow, ncol, nrow,
                 (int *)&(GRM::get<std::vector<int>>((*context)[color])[0]));
}

void processColorbar(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  double c_min, c_max, x_min = 0.0, x_max = 1.0, pos;
  int data, i, options, dim_x = 1;
  int z_log = 0;
  int label_orientation = 1;
  std::string location, axis_type = "y";
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  std::shared_ptr<GRM::Element> cell_array = nullptr, axis_elem = nullptr;
  auto num_color_values = static_cast<int>(element->getAttribute("num_color_values"));
  auto plot_parent = element->parentElement();
  getPlotParent(plot_parent);
  auto global_render = grm_get_render();
  auto global_creator = grm_get_creator();
  auto active_figure = global_render->getActiveFigure();
  auto global_root = grm_get_document_root();

  processViewport(element);
  /* `processAxis` can be triggered indirectly by `grm_input` but within the interaction processing the default Latin-1
   * encoding is used instead of the configured text encoding. Setting the correct text encoding is important since
   * functions like `gr_axis` modify the axis text based on the chosen encoding. */
  processTextEncoding(active_figure);
  processColormap(plot_parent);

  if (!getLimitsForColorbar(element, c_min, c_max) && !getLimitsForColorbar(plot_parent, c_min, c_max))
    throw NotFoundError("Missing limits\n");

  z_log = static_cast<int>(plot_parent->getAttribute("z_log"));
  location = static_cast<std::string>(element->parentElement()->parentElement()->getAttribute("location"));

  calculateCharHeight(element);

  if (location == "top" || location == "bottom")
    {
      // swap x and y if the colorbar is in the bottom or top side_region
      x_min = c_min;
      x_max = c_max;
      c_min = 0.0;
      c_max = 1.0;
      axis_type = "x";
    }
  if (element->parentElement()->hasAttribute("_window_set_by_user"))
    {
      x_min = static_cast<double>(element->parentElement()->getAttribute("window_x_min"));
      x_max = static_cast<double>(element->parentElement()->getAttribute("window_x_max"));
      c_min = static_cast<double>(element->parentElement()->getAttribute("window_y_min"));
      c_max = static_cast<double>(element->parentElement()->getAttribute("window_y_max"));
    }
  else
    {
      global_render->setWindow(element->parentElement(), x_min, x_max, c_min, c_max);
    }
  processWindow(element->parentElement());

  calculateViewport(element);
  applyMoveTransformation(element);

  /* create cell array */
  std::vector<int> data_vec;
  for (i = 0; i < num_color_values; ++i)
    {
      data = 1000 + static_cast<int>((255.0 * i) / (num_color_values - 1) + 0.5);
      data_vec.push_back(data);
    }
  auto id = static_cast<int>(global_root->getAttribute("_id"));
  auto str = std::to_string(id);
  global_root->setAttribute("_id", id + 1);

  /* clear old child nodes */
  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  if (location == "top" || location == "bottom")
    {
      // now swap also the shape of the data for x and y
      dim_x = num_color_values;
      num_color_values = 1;
    }

  if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
    {
      cell_array = global_creator->createCellArray(x_min, x_max, c_max, c_min, dim_x, num_color_values, 1, 1, dim_x,
                                                   num_color_values, "data" + str, data_vec);
      cell_array->setAttribute("_child_id", 0);
      element->append(cell_array);
    }
  else
    {
      cell_array = element->querySelectors("cell_array[_child_id=0]");
      if (cell_array != nullptr)
        global_creator->createCellArray(x_min, x_max, c_max, c_min, dim_x, num_color_values, 1, 1, dim_x,
                                        num_color_values, "data" + str, data_vec, context, cell_array);
    }
  if (cell_array != nullptr)
    {
      cell_array->setAttribute("name", "colorbar");
      if (!cell_array->hasAttribute("select_specific_xform")) global_render->setSelectSpecificXform(cell_array, 1);
      if (!cell_array->hasAttribute("clip_region")) global_render->setClipRegion(cell_array, 0);
      processClipRegion(cell_array);
    }

  // depending on the location the position of the axis and the min and max value of the data are different
  if (location == "top" || location == "bottom")
    {
      c_min = x_min;
      c_max = x_max;
      pos = location == "bottom" ? 1 : 0;
    }
  else if (location == "left")
    {
      pos = x_max;
    }
  else
    {
      pos = x_min;
    }

  /* create axes */
  gr_inqscale(&options);
  if (location == "left" || location == "bottom") label_orientation = -1;
  if (options & GR_OPTION_Z_LOG || z_log)
    {
      axis_t axis = {c_min, c_max, 2, c_min, pos, 1, 0, nullptr, NAN, 0, nullptr, NAN, 1, label_orientation};
      if (location == "top" || location == "bottom")
        gr_axis('X', &axis);
      else
        gr_axis('Y', &axis);

      if (location == "left" || location == "bottom") axis.tick_size *= -1.0;
      auto tick_orientation = axis.tick_size > 0 ? 1 : -1;

      if (element->hasAttribute("_min_value_set_by_user"))
        axis.min = static_cast<double>(element->getAttribute("_min_val_set_by_user"));
      if (element->hasAttribute("_max_value_set_by_user"))
        axis.max = static_cast<double>(element->getAttribute("_max_value_set_by_user"));
      if (element->hasAttribute("_tick_set_by_user"))
        axis.tick = static_cast<double>(element->getAttribute("_tick_set_by_user"));
      if (element->hasAttribute("_origin_set_by_user"))
        axis.org = static_cast<double>(element->getAttribute("_origin_set_by_user"));
      if (element->hasAttribute("_pos_set_by_user"))
        axis.position = static_cast<double>(element->getAttribute("_pos_set_by_user"));
      if (element->hasAttribute("_major_count_set_by_user"))
        axis.major_count = static_cast<int>(element->getAttribute("_major_count_set_by_user"));
      if (element->hasAttribute("_num_ticks_set_by_user"))
        axis.num_ticks = static_cast<int>(element->getAttribute("_num_ticks_set_by_user"));
      if (element->hasAttribute("_num_tick_labels_set_by_user"))
        axis.num_tick_labels = static_cast<int>(element->getAttribute("_num_tick_labels_set_by_user"));
      if (element->hasAttribute("_tick_size_set_by_user"))
        axis.tick_size = static_cast<double>(element->getAttribute("_tick_size_set_by_user"));
      if (element->hasAttribute("_tick_orientation_set_by_user"))
        tick_orientation = static_cast<int>(element->getAttribute("_tick_orientation_set_by_user"));

      if ((del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT && axis_elem == nullptr) ||
          !element->hasChildNodes())
        {
          axis_elem = global_creator->createAxis(axis.min, axis.max, axis.tick, axis.org, axis.position,
                                                 axis.major_count, axis.num_ticks, axis.num_tick_labels,
                                                 abs(axis.tick_size), tick_orientation, axis.label_position);
          axis_elem->setAttribute("_child_id", 1);
          if (!axis_elem->hasAttribute("_line_color_ind_set_by_user")) global_render->setLineColorInd(axis_elem, 1);
          element->append(axis_elem);
        }
      else if (axis_elem != nullptr)
        {
          axis_elem = element->querySelectors("axis[_child_id=1]");
          if (axis_elem != nullptr)
            {
              auto tick_size = axis.tick_size;
              // change sign of tick_size depending on the location of the colorbar
              if (axis_elem->hasAttribute("tick_size"))
                {
                  tick_size = static_cast<double>(axis_elem->getAttribute("tick_size"));
                  if (location == "left" || location == "bottom")
                    {
                      tick_size *= -1.0;
                      axis_elem->setAttribute("tick_size", tick_size);
                    }
                }
              if (axis_elem->hasAttribute("_tick_size_set_by_user"))
                {
                  tick_size = static_cast<double>(axis_elem->getAttribute("_tick_size_set_by_user"));
                  if (static_cast<std::string>(axis_elem->getAttribute("axis_type")) != axis_type)
                    {
                      tick_size *= -1.0;
                      axis_elem->setAttribute("_tick_size_set_by_user", tick_size);
                    }
                }

              global_creator->createAxis(axis.min, axis.max, axis.tick, axis.org, axis.position, axis.major_count,
                                         axis.num_ticks, axis.num_tick_labels, abs(tick_size), tick_orientation,
                                         axis.label_position, axis_elem);
            }
        }
      if (axis_elem != nullptr)
        {
          if (!axis_elem->hasAttribute("scale")) global_render->setScale(axis_elem, GR_OPTION_Y_LOG);
          processScale(axis_elem);
          axis_elem->setAttribute("name", "colorbar " + axis_type + "-axis");
          axis_elem->setAttribute("axis_type", axis_type);
          if (!axis_elem->hasAttribute("draw_grid")) axis_elem->setAttribute("draw_grid", false);
          if (!axis_elem->hasAttribute("mirrored_axis")) axis_elem->setAttribute("mirrored_axis", false);
          if (del == DelValues::UPDATE_WITHOUT_DEFAULT) axis_elem->setAttribute("min_value", c_min);
          axis_elem->setAttribute("label_orientation", label_orientation);
        }
      gr_freeaxis(&axis);
    }
  else
    {
      double c_tick = autoTick(c_min, c_max);
      axis_t axis = {c_min, c_max, c_tick, c_min, pos, 1, 0, nullptr, NAN, 0, nullptr, NAN, 1, label_orientation};
      if (location == "top" || location == "bottom")
        gr_axis('X', &axis);
      else
        gr_axis('Y', &axis);

      if (location == "left" || location == "bottom") axis.tick_size *= -1;
      auto tick_orientation = axis.tick_size > 0 ? 1 : -1;

      if (element->hasAttribute("_min_value_set_by_user"))
        axis.min = static_cast<double>(element->getAttribute("_min_val_set_by_user"));
      if (element->hasAttribute("_max_value_set_by_user"))
        axis.max = static_cast<double>(element->getAttribute("_max_value_set_by_user"));
      if (element->hasAttribute("_tick_set_by_user"))
        axis.tick = static_cast<double>(element->getAttribute("_tick_set_by_user"));
      if (element->hasAttribute("_origin_set_by_user"))
        axis.org = static_cast<double>(element->getAttribute("_origin_set_by_user"));
      if (element->hasAttribute("_pos_set_by_user"))
        axis.position = static_cast<double>(element->getAttribute("_pos_set_by_user"));
      if (element->hasAttribute("_major_count_set_by_user"))
        axis.major_count = static_cast<int>(element->getAttribute("_major_count_set_by_user"));
      if (element->hasAttribute("_num_ticks_set_by_user"))
        axis.num_ticks = static_cast<int>(element->getAttribute("_num_ticks_set_by_user"));
      if (element->hasAttribute("_num_tick_labels_set_by_user"))
        axis.num_tick_labels = static_cast<int>(element->getAttribute("_num_tick_labels_set_by_user"));
      if (element->hasAttribute("_tick_size_set_by_user"))
        axis.tick_size = static_cast<double>(element->getAttribute("_tick_size_set_by_user"));
      if (element->hasAttribute("_tick_orientation_set_by_user"))
        tick_orientation = static_cast<int>(element->getAttribute("_tick_orientation_set_by_user"));

      if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
        {
          axis_elem = global_creator->createAxis(axis.min, axis.max, axis.tick, axis.org, axis.position,
                                                 axis.major_count, axis.num_ticks, axis.num_tick_labels,
                                                 abs(axis.tick_size), tick_orientation, axis.label_position);
          axis_elem->setAttribute("_child_id", 1);
          if (!axis_elem->hasAttribute("_line_color_ind_set_by_user")) global_render->setLineColorInd(axis_elem, 1);
          element->append(axis_elem);
        }
      else
        {
          axis_elem = element->querySelectors("axis[_child_id=1]");
          if (axis_elem != nullptr)
            {
              auto tick_size = axis.tick_size;
              // change sign of tick_size depending on the location of the colorbar
              if (axis_elem->hasAttribute("tick_size"))
                {
                  tick_size = static_cast<double>(axis_elem->getAttribute("tick_size"));
                  if (location == "left" || location == "bottom")
                    {
                      tick_size *= -1.0;
                      axis_elem->setAttribute("tick_size", tick_size);
                    }
                }
              if (axis_elem->hasAttribute("_tick_size_set_by_user"))
                {
                  tick_size = static_cast<double>(axis_elem->getAttribute("_tick_size_set_by_user"));
                  if (static_cast<std::string>(axis_elem->getAttribute("axis_type")) != axis_type)
                    {
                      tick_size *= -1.0;
                      axis_elem->setAttribute("_tick_size_set_by_user", tick_size);
                    }
                }

              global_creator->createAxis(axis.min, axis.max, axis.tick, axis.org, axis.position, axis.major_count,
                                         axis.num_ticks, axis.num_tick_labels, abs(tick_size), tick_orientation,
                                         axis.label_position, axis_elem);
            }
        }
      if (axis_elem != nullptr)
        {
          axis_elem->setAttribute("scale", 0);
          if (del == DelValues::UPDATE_WITHOUT_DEFAULT)
            {
              axis_elem->setAttribute("tick", c_tick);
              axis_elem->setAttribute("min_value", c_min);
            }
          axis_elem->setAttribute("label_orientation", label_orientation);
        }
      processFlip(element);
      gr_freeaxis(&axis);
    }
  if (axis_elem != nullptr)
    {
      if (!axis_elem->hasAttribute("_tick_size_set_by_user"))
        axis_elem->setAttribute("tick_size", (location == "left" || location == "bottom")
                                                 ? -PLOT_DEFAULT_COLORBAR_TICK_SIZE
                                                 : PLOT_DEFAULT_COLORBAR_TICK_SIZE);
      else
        axis_elem->setAttribute("tick_size", static_cast<double>(axis_elem->getAttribute("_tick_size_set_by_user")));
      axis_elem->setAttribute("name", "colorbar " + axis_type + "-axis");
      axis_elem->setAttribute("axis_type", axis_type);
      if (del != DelValues::UPDATE_WITHOUT_DEFAULT)
        {
          axis_elem->setAttribute("draw_grid", false);
          axis_elem->setAttribute("mirrored_axis", false);
        }
    }
  applyMoveTransformation(element);
}

void processDrawArc(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for draw_arc
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  auto x_min = static_cast<double>(element->getAttribute("x_min"));
  auto x_max = static_cast<double>(element->getAttribute("x_max"));
  auto y_min = static_cast<double>(element->getAttribute("y_min"));
  auto y_max = static_cast<double>(element->getAttribute("y_max"));
  auto start_angle = static_cast<double>(element->getAttribute("start_angle"));
  auto end_angle = static_cast<double>(element->getAttribute("end_angle"));
  applyMoveTransformation(element);

  if (static_cast<std::string>(element->getAttribute("name")) == "radial-axes line") gr_setclip(0);
  if (grm_get_render()->getRedrawWs()) gr_drawarc(x_min, x_max, y_min, y_max, start_angle, end_angle);
  if (static_cast<std::string>(element->getAttribute("name")) == "radial-axes line") gr_setclip(1);
}

void processDrawGraphics(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  std::vector<char> char_vec;
  auto key = static_cast<std::string>(element->getAttribute("data"));
  auto data_vec = GRM::get<std::vector<int>>((*context)[key]);

  char_vec.reserve(data_vec.size());
  for (int i : data_vec) char_vec.push_back((char)i);
  char *data_p = &(char_vec[0]);
  applyMoveTransformation(element);

  if (grm_get_render()->getRedrawWs()) gr_drawgraphics(data_p);
}

void processDrawImage(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for draw_image
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  int model = PLOT_DEFAULT_MODEL;
  auto x_min = static_cast<double>(element->getAttribute("x_min"));
  auto x_max = static_cast<double>(element->getAttribute("x_max"));
  auto y_min = static_cast<double>(element->getAttribute("y_min"));
  auto y_max = static_cast<double>(element->getAttribute("y_max"));
  auto width = static_cast<int>(element->getAttribute("width"));
  auto height = static_cast<int>(element->getAttribute("height"));
  auto data = static_cast<std::string>(element->getAttribute("data"));
  if (element->getAttribute("color_model").isInt())
    {
      model = static_cast<int>(element->getAttribute("color_model"));
    }
  else if (element->getAttribute("color_model").isString())
    {
      model = GRM::colorModelStringToInt(static_cast<std::string>(element->getAttribute("color_model")));
    }
  applyMoveTransformation(element);
  if (element->parentElement()->localName() != "polar_bar")
    {
      processClipRegion(element->parentElement()->parentElement());
      gr_selntran(1);
    }
  if (grm_get_render()->getRedrawWs())
    gr_drawimage(x_min, x_max, y_max, y_min, width, height, (int *)&(GRM::get<std::vector<int>>((*context)[data])[0]),
                 model);
}

void processErrorBars(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  std::string kind;
  std::vector<double> absolute_upwards_vec, absolute_downwards_vec, relative_upwards_vec, relative_downwards_vec;
  std::string absolute_upwards, absolute_downwards, relative_upwards, relative_downwards;
  double absolute_upwards_flt, relative_upwards_flt, absolute_downwards_flt, relative_downwards_flt;
  int scale_options, color_upwards_cap, color_downwards_cap, color_error_bar;
  double marker_size, x_min, x_max, y_min, y_max, tick, a, b, e_upwards, e_downwards, x_value;
  double line_x[2], line_y[2], last_line_y[2];
  std::vector<double> x_vec, y_vec;
  unsigned int x_length;
  std::string x_key, y_key;
  std::shared_ptr<GRM::Element> series;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  int child_id = 0;
  int error_bar_style = ERRORBAR_DEFAULT_STYLE; // line
  auto global_creator = grm_get_creator();
  auto global_root = grm_get_document_root();

  absolute_upwards_flt = absolute_downwards_flt = relative_upwards_flt = relative_downwards_flt = FLT_MAX;
  series = element->parentElement();
  if (element->parentElement()->parentElement()->localName() != "central_region")
    series = series->parentElement(); // marginal heatmap

  if (!element->hasAttribute("x")) throw NotFoundError("Error-bars are missing required attribute x-data.\n");
  x_key = static_cast<std::string>(element->getAttribute("x"));
  if (!element->hasAttribute("y")) throw NotFoundError("Error-bars are missing required attribute y-data.\n");
  y_key = static_cast<std::string>(element->getAttribute("y"));

  x_vec = GRM::get<std::vector<double>>((*context)[x_key]);
  y_vec = GRM::get<std::vector<double>>((*context)[y_key]);
  x_length = x_vec.size();
  kind = static_cast<std::string>(series->getAttribute("kind"));

  if (!element->hasAttribute("abs_downwards_e") && !element->hasAttribute("rel_downwards_e"))
    throw NotFoundError("Error-bars are missing required attribute downwards.\n");
  if (!element->hasAttribute("abs_upwards_e") && !element->hasAttribute("rel_upwards_e"))
    throw NotFoundError("Error-bars are missing required attribute upwards.\n");
  if (element->hasAttribute("abs_downwards_e"))
    {
      absolute_downwards = static_cast<std::string>(element->getAttribute("abs_downwards_e"));
      absolute_downwards_vec = GRM::get<std::vector<double>>((*context)[absolute_downwards]);
    }
  if (element->hasAttribute("rel_downwards_e"))
    {
      relative_downwards = static_cast<std::string>(element->getAttribute("rel_downwards_e"));
      relative_downwards_vec = GRM::get<std::vector<double>>((*context)[relative_downwards]);
    }
  if (element->hasAttribute("abs_upwards_e"))
    {
      absolute_upwards = static_cast<std::string>(element->getAttribute("abs_upwards_e"));
      absolute_upwards_vec = GRM::get<std::vector<double>>((*context)[absolute_upwards]);
    }
  if (element->hasAttribute("rel_upwards_e"))
    {
      relative_upwards = static_cast<std::string>(element->getAttribute("rel_upwards_e"));
      relative_upwards_vec = GRM::get<std::vector<double>>((*context)[relative_upwards]);
    }
  if (element->hasAttribute("uniform_abs_downwards_e"))
    absolute_downwards_flt = static_cast<double>(element->getAttribute("uniform_abs_downwards_e"));
  if (element->hasAttribute("uniform_abs_upwards_e"))
    absolute_upwards_flt = static_cast<double>(element->getAttribute("uniform_abs_upwards_e"));
  if (element->hasAttribute("uniform_rel_downwards_e"))
    relative_downwards_flt = static_cast<double>(element->getAttribute("uniform_rel_downwards_e"));
  if (element->hasAttribute("uniform_rel_upwards_e"))
    relative_upwards_flt = static_cast<double>(element->getAttribute("uniform_rel_upwards_e"));
  if (element->hasAttribute("error_bar_style"))
    error_bar_style = static_cast<int>(element->getAttribute("error_bar_style"));

  if (absolute_upwards_vec.empty() && relative_upwards_vec.empty() && absolute_upwards_flt == FLT_MAX &&
      relative_upwards_flt == FLT_MAX && absolute_downwards_vec.empty() && relative_downwards_vec.empty() &&
      absolute_downwards_flt == FLT_MAX && relative_downwards_flt == FLT_MAX)
    {
      throw NotFoundError("Error-bar is missing required error-data.");
    }

  /* Getting GRM options and sizes. See gr_verrorbars */
  gr_savestate();
  gr_inqmarkersize(&marker_size);
  gr_inqwindow(&x_min, &x_max, &y_min, &y_max);
  gr_inqscale(&scale_options);
  tick = marker_size * 0.0075 * (x_max - x_min);
  a = (x_max - x_min) / log10(x_max / x_min);
  b = x_min - a * log10(x_min);

  gr_inqlinecolorind(&color_error_bar);
  // special case for barplot
  if (kind == "barplot") color_error_bar = static_cast<int>(element->parentElement()->getAttribute("line_color_ind"));
  color_upwards_cap = color_downwards_cap = color_error_bar;
  if (element->hasAttribute("upwards_cap_color"))
    color_upwards_cap = static_cast<int>(element->getAttribute("upwards_cap_color"));
  if (element->hasAttribute("downwards_cap_color"))
    color_downwards_cap = static_cast<int>(element->getAttribute("downwards_cap_color"));
  if (element->hasAttribute("error_bar_color"))
    color_error_bar = static_cast<int>(element->getAttribute("error_bar_color"));

  /* clear old lines */
  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  /* Actual drawing of bars */
  e_upwards = e_downwards = FLT_MAX;
  for (int i = 0; i < x_length; i++)
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

      if (i > 0)
        {
          last_line_y[0] = line_y[0];
          last_line_y[1] = line_y[1];
        }
      line_y[0] = e_upwards != FLT_MAX ? e_upwards : y_vec[i];
      line_y[1] = e_downwards != FLT_MAX ? e_downwards : y_vec[i];

      if (error_bar_style == 0)
        {
          std::shared_ptr<GRM::Element> error_bar;

          /* See gr_verrorbars for reference */
          x_value = x_vec[i];
          line_x[0] = xLog(xLin(x_value - tick, scale_options, x_min, x_max, a, b), scale_options, x_min, x_max, a, b);
          line_x[1] = xLog(xLin(x_value + tick, scale_options, x_min, x_max, a, b), scale_options, x_min, x_max, a, b);

          if (color_error_bar >= 0)
            {
              if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
                {
                  error_bar = global_creator->createErrorBar(x_value, line_y[0], line_y[1], color_error_bar);
                  error_bar->setAttribute("_child_id", child_id++);
                  element->append(error_bar);
                }
              else
                {
                  error_bar = element->querySelectors("error_bar[_child_id=" + std::to_string(child_id++) + "]");
                  if (error_bar != nullptr)
                    global_creator->createErrorBar(x_value, line_y[0], line_y[1], color_error_bar, error_bar);
                }

              if (error_bar != nullptr)
                {
                  if (e_upwards != FLT_MAX)
                    {
                      error_bar->setAttribute("upwards_e", e_upwards);
                      error_bar->setAttribute("upwards_cap_color", color_upwards_cap);
                    }
                  if (e_downwards != FLT_MAX)
                    {
                      error_bar->setAttribute("downwards_e", e_downwards);
                      error_bar->setAttribute("downwards_cap_color", color_downwards_cap);
                    }
                  if (e_downwards != FLT_MAX || e_upwards != FLT_MAX)
                    {
                      error_bar->setAttribute("cap_x_min", line_x[0]);
                      error_bar->setAttribute("cap_x_max", line_x[1]);
                    }
                }
            }
        }
      else if (error_bar_style == 1 && color_error_bar >= 0)
        {
          std::vector<double> f1, f2;
          std::shared_ptr<GRM::Element> fill_area, bar;
          std::string orientation = PLOT_DEFAULT_ORIENTATION;

          if (element->parentElement()->parentElement()->hasAttribute("orientation"))
            orientation =
                static_cast<std::string>(element->parentElement()->parentElement()->getAttribute("orientation"));
          auto is_horizontal = orientation == "horizontal";

          if (kind == "barplot" || kind == "histogram")
            {
              double x[2], y[2];
              double bar_width = 0.8, transparency = 0.4;
              if (element->parentElement()->hasAttribute("bar_width"))
                bar_width = static_cast<double>(element->parentElement()->getAttribute("bar_width"));

              if (kind == "barplot")
                {
                  if (element->parentElement()->hasAttribute("x_range_min") &&
                      element->parentElement()->hasAttribute("x_range_max"))
                    {
                      x_min = static_cast<double>(element->parentElement()->getAttribute("x_range_min"));
                      x_max = static_cast<double>(element->parentElement()->getAttribute("x_range_max"));
                      if (!element->hasAttribute("bar_width")) bar_width = (x_max - x_min) / (y_vec.size() - 1.0);
                    }
                }
              else
                {
                  bar_width = 1.0;
                }

              auto id = static_cast<int>(global_root->getAttribute("_id"));
              global_root->setAttribute("_id", id + 1);
              auto str = std::to_string(id);

              if (element->hasAttribute("transparency"))
                transparency = static_cast<double>(element->getAttribute("transparency"));
              element->setAttribute("transparency", transparency);

              if (is_horizontal)
                {
                  x[0] = x_vec[i] - bar_width / 2.0;
                  x[1] = x[0] + bar_width;
                  y[0] = e_upwards != FLT_MAX ? e_upwards : y_vec[i];
                  y[1] = e_downwards != FLT_MAX ? e_downwards : y_vec[i];
                }
              else
                {
                  x[0] = e_upwards != FLT_MAX ? e_upwards : y_vec[i];
                  x[1] = e_downwards != FLT_MAX ? e_downwards : y_vec[i];
                  y[0] = x_vec[i] - bar_width / 2.0;
                  y[1] = y[0] + bar_width;
                }
              if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
                {
                  bar = global_creator->createBar(x[0], x[1], y[0], y[1], 1, 1, "", "", 1, "");
                  bar->setAttribute("_child_id", child_id++);
                  element->append(bar);
                }
              else
                {
                  bar = element->querySelectors("bar[_child_id=" + std::to_string(child_id++) + "]");
                  if (bar != nullptr) global_creator->createBar(x[0], x[1], y[0], y[1], 1, 1, "", "", 1, "", bar);
                }
              if (bar != nullptr)
                {
                  int fill_int_style = 1;

                  if (element->hasAttribute("fill_int_style"))
                    fill_int_style = static_cast<int>(element->getAttribute("fill_int_style"));
                  bar->setAttribute("fill_int_style", fill_int_style);
                }
            }
          else
            {
              if (i == 0) continue;

              // fill vector
              if (is_horizontal)
                {
                  f1.push_back(x_vec[i - 1]);
                  f2.push_back(last_line_y[0]);
                  f1.push_back(x_vec[i - 1]);
                  f2.push_back(last_line_y[1]);
                  f1.push_back(x_vec[i]);
                  f2.push_back(line_y[1]);
                  f1.push_back(x_vec[i]);
                  f2.push_back(line_y[0]);
                }
              else
                {
                  f2.push_back(x_vec[i - 1]);
                  f1.push_back(last_line_y[0]);
                  f2.push_back(x_vec[i - 1]);
                  f1.push_back(last_line_y[1]);
                  f2.push_back(x_vec[i]);
                  f1.push_back(line_y[1]);
                  f2.push_back(x_vec[i]);
                  f1.push_back(line_y[0]);
                }

              auto id = static_cast<int>(global_root->getAttribute("_id"));
              global_root->setAttribute("_id", id + 1);
              auto str = std::to_string(id);

              if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
                {
                  fill_area =
                      global_creator->createFillArea("x" + str, f1, "y" + str, f2, nullptr, 0, 0, color_error_bar);
                  fill_area->setAttribute("_child_id", child_id++);
                  element->append(fill_area);
                }
              else
                {
                  fill_area = element->querySelectors("fill_area[_child_id=" + std::to_string(child_id++) + "]");
                  if (fill_area != nullptr)
                    global_creator->createFillArea("x" + str, f1, "y" + str, f2, nullptr, 0, 0, color_error_bar,
                                                   fill_area);
                }
              if (fill_area != nullptr)
                {
                  double transparency = 0.4;
                  int fill_int_style = 1;

                  if (element->hasAttribute("transparency"))
                    transparency = static_cast<double>(element->getAttribute("transparency"));
                  fill_area->setAttribute("transparency", transparency);
                  if (element->hasAttribute("fill_int_style"))
                    fill_int_style = static_cast<int>(element->getAttribute("fill_int_style"));
                  fill_area->setAttribute("fill_int_style", fill_int_style);
                }
            }
        }
    }
  gr_restorestate();
}

void processErrorBar(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  double cap_x_min = 0.0, cap_x_max = 0.0, e_upwards = FLT_MAX, e_downwards = FLT_MAX;
  int color_upwards_cap = 0, color_downwards_cap = 0;
  std::shared_ptr<GRM::Element> line;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  int child_id = 0;
  std::string orientation = PLOT_DEFAULT_ORIENTATION;
  double line_x_min, line_x_max, line_y_min, line_y_max;
  auto global_creator = grm_get_creator();

  /* clear old lines */
  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  if (element->parentElement()->parentElement()->parentElement()->hasAttribute("orientation"))
    orientation = static_cast<std::string>(
        element->parentElement()->parentElement()->parentElement()->getAttribute("orientation"));
  auto is_horizontal = orientation == "horizontal";

  auto error_bar_x = static_cast<double>(element->getAttribute("error_bar_x"));
  auto error_bar_y_min = static_cast<double>(element->getAttribute("error_bar_y_min"));
  auto error_bar_y_max = static_cast<double>(element->getAttribute("error_bar_y_max"));
  auto color_error_bar = static_cast<int>(element->getAttribute("error_bar_color"));

  if (element->hasAttribute("cap_x_min")) cap_x_min = static_cast<double>(element->getAttribute("cap_x_min"));
  if (element->hasAttribute("cap_x_max")) cap_x_max = static_cast<double>(element->getAttribute("cap_x_max"));
  if (element->hasAttribute("upwards_e")) e_upwards = static_cast<double>(element->getAttribute("upwards_e"));
  if (element->hasAttribute("downwards_e")) e_downwards = static_cast<double>(element->getAttribute("downwards_e"));
  if (element->hasAttribute("upwards_cap_color"))
    color_upwards_cap = static_cast<int>(element->getAttribute("upwards_cap_color"));
  if (element->hasAttribute("downwards_cap_color"))
    color_downwards_cap = static_cast<int>(element->getAttribute("downwards_cap_color"));

  if (is_horizontal)
    {
      line_x_min = cap_x_min;
      line_x_max = cap_x_max;
    }
  else
    {
      line_y_min = cap_x_min;
      line_y_max = cap_x_max;
    }

  if (e_upwards != FLT_MAX && color_upwards_cap >= 0)
    {
      if (is_horizontal)
        line_y_min = line_y_max = e_upwards;
      else
        line_x_min = line_x_max = e_upwards;

      if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
        {
          line =
              global_creator->createPolyline(line_x_min, line_x_max, line_y_min, line_y_max, 0, 0.0, color_upwards_cap);
          line->setAttribute("_child_id", child_id++);
          element->append(line);
        }
      else
        {
          line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
          if (line != nullptr)
            global_creator->createPolyline(line_x_min, line_x_max, line_y_min, line_y_max, 0, 0.0, color_upwards_cap,
                                           line);
        }
    }

  if (e_downwards != FLT_MAX && color_downwards_cap >= 0)
    {
      if (is_horizontal)
        line_y_min = line_y_max = e_downwards;
      else
        line_x_min = line_x_max = e_downwards;

      if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
        {
          line = global_creator->createPolyline(line_x_min, line_x_max, line_y_min, line_y_max, 0, 0.0,
                                                color_downwards_cap);
          line->setAttribute("_child_id", child_id++);
          element->append(line);
        }
      else
        {
          line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
          if (line != nullptr)
            global_creator->createPolyline(line_x_min, line_x_max, line_y_min, line_y_max, 0, 0.0, color_downwards_cap,
                                           line);
        }
    }

  if (color_error_bar >= 0)
    {
      if (is_horizontal)
        {
          line_x_min = line_x_max = error_bar_x;
          line_y_min = error_bar_y_min;
          line_y_max = error_bar_y_max;
        }
      else
        {
          line_x_min = error_bar_y_min;
          line_x_max = error_bar_y_max;
          line_y_min = line_y_max = error_bar_x;
        }
      if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
        {
          line =
              global_creator->createPolyline(line_x_min, line_x_max, line_y_min, line_y_max, 0, 0.0, color_error_bar);
          line->setAttribute("_child_id", child_id++);
          element->append(line);
        }
      else
        {
          line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
          if (line != nullptr)
            global_creator->createPolyline(line_x_min, line_x_max, line_y_min, line_y_max, 0, 0.0, color_error_bar,
                                           line);
        }
    }
}

void processLegend(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  double tbx[4], tby[4];
  std::shared_ptr<GRM::Render> render;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  int child_id = 0;
  auto plot_parent = element->parentElement();
  getPlotParent(plot_parent);
  auto global_creator = grm_get_creator();

  auto kind = static_cast<std::string>(plot_parent->getAttribute("_kind"));

  render = std::dynamic_pointer_cast<GRM::Render>(element->ownerDocument());
  if (!render) throw NotFoundError("No render-document found for element\n");

  calculateViewport(element);
  applyMoveTransformation(element);

  if (kind != "pie")
    {
      double legend_symbol_x[2], legend_symbol_y[2];
      int legend_elems = 0;
      double viewport[4];
      std::shared_ptr<GRM::Element> fr, dr;

      gr_savestate();

      auto scale_factor = static_cast<double>(element->getAttribute("_scale_factor"));
      auto initial_scale_factor = static_cast<double>(element->getAttribute("_initial_scale_factor"));

      if (!GRM::Render::getViewport(element, &viewport[0], &viewport[1], &viewport[2], &viewport[3]))
        throw NotFoundError(element->localName() + " doesn't have a viewport but it should.\n");

      del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));

      /* get the amount of series which should be displayed inside the legend */
      for (const auto &plot_child : element->parentElement()->children()) // central_region children
        {
          if (plot_child->localName() != "central_region") continue;
          for (const auto &series : plot_child->children())
            {
              if (!strEqualsAny(series->localName(), "series_line", "series_polar_line", "series_polar_scatter",
                                "series_scatter", "series_stairs", "series_stem", "series_line3", "series_scatter3"))
                continue;
              if (!series->hasAttribute("label")) continue;
              for (const auto &child : series->children())
                {
                  if (child->localName() != "polyline" && child->localName() != "polymarker" &&
                      child->localName() != "polyline_3d" && child->localName() != "polymarker_3d")
                    continue;
                  legend_elems += 1;
                }
            }
        }
      if (element->hasAttribute("_legend_elems"))
        {
          /* the amount has changed - all legend children have to recreated cause its unknown which is new or gone */
          if (static_cast<int>(element->getAttribute("_legend_elems")) != legend_elems)
            {
              del = (del == DelValues::RECREATE_ALL_CHILDREN) ? DelValues::RECREATE_ALL_CHILDREN
                                                              : DelValues::RECREATE_OWN_CHILDREN;
              element->setAttribute("_legend_elems", legend_elems);
            }
        }
      else
        {
          element->setAttribute("_legend_elems", legend_elems);
        }

      /* clear old child nodes */
      clearOldChildren(&del, element);

      gr_selntran(1);

      if (!element->hasAttribute("_select_specific_xform_set_by_user")) render->setSelectSpecificXform(element, 0);
      if (!element->hasAttribute("_scale_set_by_user")) render->setScale(element, 0);

      if (legend_elems > 0)
        {
          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              fr = global_creator->createFillRect(viewport[0], viewport[1], viewport[3], viewport[2]);
              fr->setAttribute("_child_id", child_id++);
              element->append(fr);
            }
          else
            {
              fr = element->querySelectors("fill_rect[_child_id=" + std::to_string(child_id++) + "]");
              if (fr != nullptr)
                global_creator->createFillRect(viewport[0], viewport[1], viewport[3], viewport[2], 0, 0, -1, fr);
            }

          if (!element->hasAttribute("_fill_int_style_set_by_user"))
            render->setFillIntStyle(element, GKS_K_INTSTYLE_SOLID);
          if (!element->hasAttribute("_fill_color_ind_set_by_user")) render->setFillColorInd(element, 0);

          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              dr = global_creator->createDrawRect(viewport[0], viewport[1], viewport[3], viewport[2]);
              dr->setAttribute("_child_id", child_id++);
              element->append(dr);
            }
          else
            {
              dr = element->querySelectors("draw_rect[_child_id=" + std::to_string(child_id++) + "]");
              if (dr != nullptr) global_creator->createDrawRect(viewport[0], viewport[1], viewport[3], viewport[2], dr);
            }

          if (dr != nullptr && del != DelValues::UPDATE_WITHOUT_DEFAULT)
            {
              if (!dr->hasAttribute("_line_type_set_by_user"))
                {
                  auto line_type = GKS_K_INTSTYLE_SOLID;
                  if (element->hasAttribute("line_type"))
                    line_type = static_cast<int>(element->getAttribute("line_type"));
                  render->setLineType(dr, line_type);
                }
              if (!dr->hasAttribute("_line_color_ind_set_by_user"))
                {
                  auto line_color_ind = 1;
                  if (element->hasAttribute("line_color_ind"))
                    line_color_ind = static_cast<int>(element->getAttribute("line_color_ind"));
                  render->setLineColorInd(dr, line_color_ind);
                }
              if (!dr->hasAttribute("_line_width_set_by_user"))
                {
                  auto line_width = 1;
                  if (element->hasAttribute("line_width"))
                    line_width = static_cast<double>(element->getAttribute("line_width"));
                  render->setLineWidth(dr, line_width);
                }
            }
        }

      if (!element->hasAttribute("_line_spec_set_by_user")) render->setLineSpec(element, const_cast<char *>(" "));

      for (const auto &series : plot_parent->querySelectors("central_region")->children())
        {
          int mask;
          double dy;
          bool got_polyline = false, got_polymarker = false;
          std::string spec = "";

          if (!strEqualsAny(series->localName(), "series_line", "series_polar_line", "series_polar_scatter",
                            "series_scatter", "series_stairs", "series_stem", "series_scatter3", "series_line3"))
            continue;
          if (!series->hasAttribute("label")) continue;

          auto label = static_cast<std::string>(series->getAttribute("label"));
          gr_inqtext(0, 0, (char *)label.c_str(), tbx, tby);
          dy = grm_max((tby[2] - tby[0]) - 0.03 * scale_factor, 0);
          viewport[3] -= 0.5 * dy;

          std::shared_ptr<GRM::Element> label_elem;
          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              label_elem = render->createElement("label");
              label_elem->setAttribute("_child_id", child_id++);
              element->append(label_elem);
            }
          else
            {
              label_elem = element->querySelectors("label[_child_id=" + std::to_string(child_id++) + "]");
            }
          if (label_elem != nullptr)
            {
              if (!label_elem->hasAttribute("_char_height_set_by_user"))
                {
                  label_elem->setAttribute("char_height",
                                           static_cast<double>(plot_parent->getAttribute("char_height")) *
                                               initial_scale_factor);
                }
              else
                {
                  label_elem->setAttribute("char_height", static_cast<double>(label_elem->getAttribute("char_height")));
                }
              if (label_elem->hasAttribute("hidden"))
                {
                  label_elem->setAttribute("transparency", 0.5);
                  series->setAttribute("_hidden", true);
                }
              else if (series->hasAttribute("_hidden"))
                {
                  series->removeAttribute("_hidden");
                }
              gr_savestate();
              if (series->hasAttribute("line_spec")) spec = static_cast<std::string>(series->getAttribute("line_spec"));
              const char *spec_char = spec.c_str();
              mask = gr_uselinespec((char *)spec_char);
              gr_restorestate();

              if (intEqualsAny(mask, 5, 0, 1, 3, 4, 5))
                {
                  legend_symbol_x[0] = viewport[0] + 0.01 * scale_factor;
                  legend_symbol_x[1] = viewport[0] + 0.07 * scale_factor;
                  legend_symbol_y[0] = viewport[3] - 0.03 * scale_factor;
                  legend_symbol_y[1] = viewport[3] - 0.03 * scale_factor;
                  for (const auto &child : series->children())
                    {
                      std::shared_ptr<GRM::Element> pl;
                      if (series->localName() == "series_stem")
                        {
                          if (got_polymarker && got_polyline) break;
                          if (child->localName() == "polyline" && got_polyline) continue;
                          if (child->localName() == "polymarker" && got_polymarker) continue;
                        }

                      if (label_elem->hasAttribute("hidden"))
                        {
                          child->setAttribute("_hidden", true);
                        }
                      else if (child->hasAttribute("_hidden"))
                        {
                          child->removeAttribute("_hidden");
                        }

                      if (child->localName() == "polyline")
                        {
                          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
                            {
                              pl = global_creator->createPolyline(legend_symbol_x[0], legend_symbol_x[1],
                                                                  legend_symbol_y[0], legend_symbol_y[1]);
                              pl->setAttribute("_child_id", 0);
                              label_elem->append(pl);
                            }
                          else
                            {
                              pl = label_elem->querySelectors("polyline[_child_id=\"0\"]");
                              if (pl != nullptr)
                                global_creator->createPolyline(legend_symbol_x[0], legend_symbol_x[1],
                                                               legend_symbol_y[0], legend_symbol_y[1], 0, 0.0, 0, pl);
                            }
                          if (pl != nullptr)
                            {
                              render->setLineSpec(pl, spec);
                              if (child->hasAttribute("line_color_ind"))
                                {
                                  pl->setAttribute("line_color_ind",
                                                   static_cast<int>(child->getAttribute("line_color_ind")));
                                }
                              else
                                {
                                  pl->setAttribute("line_color_ind",
                                                   static_cast<int>(series->getAttribute("line_color_ind")));
                                }
                              if (child->hasAttribute("line_type"))
                                {
                                  pl->setAttribute("line_type", static_cast<int>(child->getAttribute("line_type")));
                                }
                              else if (series->hasAttribute("line_type"))
                                {
                                  pl->setAttribute("line_type", static_cast<int>(series->getAttribute("line_type")));
                                }
                              got_polyline = true;
                            }
                        }
                      else if (child->localName() == "polymarker")
                        {
                          int markertype;
                          if (child->hasAttribute("marker_type"))
                            {
                              markertype = static_cast<int>(child->getAttribute("marker_type"));
                            }
                          else
                            {
                              markertype = static_cast<int>(series->getAttribute("marker_type"));
                            }
                          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
                            {
                              pl = global_creator->createPolymarker(legend_symbol_x[0] + 0.02 * scale_factor,
                                                                    legend_symbol_y[0], markertype);
                              pl->setAttribute("_child_id", 0);
                              label_elem->append(pl);
                            }
                          else
                            {
                              pl = label_elem->querySelectors("polymarker[_child_id=\"0\"]");
                              if (pl != nullptr)
                                global_creator->createPolymarker(legend_symbol_x[0] + 0.02 * scale_factor,
                                                                 legend_symbol_y[0], markertype, 0.0, 0, pl);
                            }
                          if (pl != nullptr)
                            {
                              int marker_color_ind = 989;
                              if (child->hasAttribute("marker_color_ind"))
                                {
                                  marker_color_ind = static_cast<int>(child->getAttribute("marker_color_ind"));
                                }
                              else if (series->hasAttribute("marker_color_ind"))
                                {
                                  marker_color_ind = static_cast<int>(series->getAttribute("marker_color_ind"));
                                }
                              render->setMarkerColorInd(pl, marker_color_ind);
                              if (child->hasAttribute("marker_type"))
                                {
                                  pl->setAttribute("marker_type", static_cast<int>(child->getAttribute("marker_type")));
                                }
                              else if (series->hasAttribute("marker_type"))
                                {
                                  pl->setAttribute("marker_type",
                                                   static_cast<int>(series->getAttribute("marker_type")));
                                }
                              if (child->hasAttribute("border_color_ind"))
                                {
                                  pl->setAttribute("border_color_ind",
                                                   static_cast<int>(child->getAttribute("border_color_ind")));
                                }
                              else if (series->hasAttribute("border_color_ind"))
                                {
                                  pl->setAttribute("border_color_ind",
                                                   static_cast<int>(series->getAttribute("border_color_ind")));
                                }
                              if (series->localName() == "series_stem") pl->setAttribute("x", legend_symbol_x[1]);
                              processMarkerColorInd(pl);
                              got_polymarker = true;
                            }
                        }
                      else if (child->localName() == "polyline_3d")
                        {
                          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
                            {
                              pl = global_creator->createPolyline(legend_symbol_x[0], legend_symbol_x[1],
                                                                  legend_symbol_y[0], legend_symbol_y[1]);
                              pl->setAttribute("_child_id", 0);
                              label_elem->append(pl);
                            }
                          else
                            {
                              pl = label_elem->querySelectors("polyline[_child_id=\"0\"]");
                              if (pl != nullptr)
                                global_creator->createPolyline(legend_symbol_x[0], legend_symbol_x[1],
                                                               legend_symbol_y[0], legend_symbol_y[1], 0, 0.0, 0, pl);
                            }
                          if (pl != nullptr)
                            {
                              render->setLineSpec(pl, spec);
                              if (child->hasAttribute("line_color_ind"))
                                {
                                  pl->setAttribute("line_color_ind",
                                                   static_cast<int>(child->getAttribute("line_color_ind")));
                                }
                              else
                                {
                                  pl->setAttribute("line_color_ind",
                                                   static_cast<int>(series->getAttribute("line_color_ind")));
                                }
                              if (child->hasAttribute("line_type"))
                                {
                                  pl->setAttribute("line_type", static_cast<int>(child->getAttribute("line_type")));
                                }
                              else if (series->hasAttribute("line_type"))
                                {
                                  pl->setAttribute("line_type", static_cast<int>(series->getAttribute("line_type")));
                                }
                              got_polyline = true;
                            }
                        }
                      else if (child->localName() == "polymarker_3d")
                        {
                          int markertype;
                          if (child->hasAttribute("marker_type"))
                            {
                              markertype = static_cast<int>(child->getAttribute("marker_type"));
                            }
                          else
                            {
                              markertype = static_cast<int>(series->getAttribute("marker_type"));
                            }
                          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
                            {
                              pl = global_creator->createPolymarker(legend_symbol_x[0] + 0.02 * scale_factor,
                                                                    legend_symbol_y[0], markertype);
                              pl->setAttribute("_child_id", 0);
                              label_elem->append(pl);
                            }
                          else
                            {
                              pl = label_elem->querySelectors("polymarker[_child_id=\"0\"]");
                              if (pl != nullptr)
                                global_creator->createPolymarker(legend_symbol_x[0] + 0.02 * scale_factor,
                                                                 legend_symbol_y[0], markertype, 0.0, 0, pl);
                            }
                          if (pl != nullptr)
                            {
                              int marker_color_ind = 989;
                              if (child->hasAttribute("marker_color_ind"))
                                {
                                  marker_color_ind = static_cast<int>(child->getAttribute("marker_color_ind"));
                                }
                              else if (series->hasAttribute("marker_color_ind"))
                                {
                                  marker_color_ind = static_cast<int>(series->getAttribute("marker_color_ind"));
                                }
                              render->setMarkerColorInd(pl, marker_color_ind);
                              if (child->hasAttribute("marker_type"))
                                {
                                  pl->setAttribute("marker_type", static_cast<int>(child->getAttribute("marker_type")));
                                }
                              else if (series->hasAttribute("marker_type"))
                                {
                                  pl->setAttribute("marker_type",
                                                   static_cast<int>(series->getAttribute("marker_type")));
                                }
                              if (child->hasAttribute("border_color_ind"))
                                {
                                  pl->setAttribute("border_color_ind",
                                                   static_cast<int>(child->getAttribute("border_color_ind")));
                                }
                              else if (series->hasAttribute("border_color_ind"))
                                {
                                  pl->setAttribute("border_color_ind",
                                                   static_cast<int>(series->getAttribute("border_color_ind")));
                                }
                              if (series->localName() == "series_stem") pl->setAttribute("x", legend_symbol_x[1]);
                              processMarkerColorInd(pl);
                              got_polymarker = true;
                            }
                        }
                    }
                }
              else if (mask & 2)
                {
                  legend_symbol_x[0] = viewport[0] + 0.02 * scale_factor;
                  legend_symbol_x[1] = viewport[0] + 0.06 * scale_factor;
                  legend_symbol_y[0] = viewport[3] - 0.03 * scale_factor;
                  legend_symbol_y[1] = viewport[3] - 0.03 * scale_factor;
                  for (const auto &child : series->children())
                    {
                      std::shared_ptr<GRM::Element> pl;
                      if (child->localName() == "polyline")
                        {
                          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
                            {
                              pl = global_creator->createPolyline(legend_symbol_x[0], legend_symbol_x[1],
                                                                  legend_symbol_y[0], legend_symbol_y[1]);
                              pl->setAttribute("_child_id", 1);
                              label_elem->append(pl);
                            }
                          else
                            {
                              pl = label_elem->querySelectors("polyline[_child_id=\"1\"]");
                              if (pl != nullptr)
                                global_creator->createPolyline(legend_symbol_x[0], legend_symbol_x[1],
                                                               legend_symbol_y[0], legend_symbol_y[1], 0, 0.0, 0, pl);
                            }
                          if (pl != nullptr)
                            {
                              render->setLineSpec(pl, spec);
                              if (child->hasAttribute("line_color_ind"))
                                {
                                  render->setLineColorInd(pl, static_cast<int>(child->getAttribute("line_color_ind")));
                                }
                              else
                                {
                                  render->setLineColorInd(pl, static_cast<int>(series->getAttribute("line_color_ind")));
                                }
                            }
                        }
                      else if (child->localName() == "polymarker")
                        {
                          int markertype;
                          if (child->hasAttribute("marker_type"))
                            {
                              markertype = static_cast<int>(child->getAttribute("marker_type"));
                            }
                          else
                            {
                              markertype = static_cast<int>(series->getAttribute("marker_type"));
                            }
                          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
                            {
                              pl = global_creator->createPolymarker(legend_symbol_x[0] + 0.02 * scale_factor,
                                                                    legend_symbol_y[0], markertype);
                              pl->setAttribute("_child_id", 1);
                              label_elem->append(pl);
                            }
                          else
                            {
                              pl = label_elem->querySelectors("polymarker[_child_id=\"1\"]");
                              if (pl != nullptr)
                                global_creator->createPolymarker(legend_symbol_x[0] + 0.02 * scale_factor,
                                                                 legend_symbol_y[0], markertype, 0.0, 0, pl);
                            }
                          if (pl != nullptr)
                            {
                              render->setMarkerColorInd(
                                  pl, (series->hasAttribute("marker_color_ind")
                                           ? static_cast<int>(series->getAttribute("marker_color_ind"))
                                           : 989));
                              if (child->hasAttribute("border_color_ind"))
                                {
                                  pl->setAttribute("border_color_ind",
                                                   static_cast<int>(child->getAttribute("border_color_ind")));
                                }
                              else if (series->hasAttribute("border_color_ind"))
                                {
                                  pl->setAttribute("border_color_ind",
                                                   static_cast<int>(series->getAttribute("border_color_ind")));
                                }
                              processMarkerColorInd(pl);
                            }
                        }
                    }
                }

              std::shared_ptr<GRM::Element> tx;
              if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
                {
                  tx = global_creator->createText(viewport[0] + 0.08 * scale_factor, viewport[3] - 0.03 * scale_factor,
                                                  label);
                  tx->setAttribute("_child_id", 2);
                  label_elem->append(tx);
                }
              else
                {
                  tx = label_elem->querySelectors("text[_child_id=\"2\"]");
                  if (tx != nullptr)
                    global_creator->createText(viewport[0] + 0.08 * scale_factor, viewport[3] - 0.03 * scale_factor,
                                               label, CoordinateSpace::NDC, tx);
                }
              if (tx != nullptr && del != DelValues::UPDATE_WITHOUT_DEFAULT)
                {
                  if (!tx->hasAttribute("_text_align_vertical_set_by_user"))
                    {
                      auto text_align_vertical = GKS_K_TEXT_VALIGN_HALF;
                      if (element->hasAttribute("text_align_vertical"))
                        text_align_vertical = static_cast<int>(element->getAttribute("text_align_vertical"));
                      tx->setAttribute("text_align_vertical", text_align_vertical);
                    }
                  if (!tx->hasAttribute("_text_align_horizontal_set_by_user"))
                    {
                      auto text_align_horizontal = GKS_K_TEXT_HALIGN_LEFT;
                      if (element->hasAttribute("text_align_horizontal"))
                        text_align_horizontal = static_cast<int>(element->getAttribute("text_align_horizontal"));
                      tx->setAttribute("text_align_horizontal", text_align_horizontal);
                    }
                }
              viewport[3] -= 0.5 * dy;
              viewport[3] -= 0.03 * scale_factor;
            }
        }
      gr_restorestate();

      processLineSpec(element);
    }
  else
    {
      std::shared_ptr<GRM::Element> fr, dr, text;
      double viewport[4];

      auto labels_key = static_cast<std::string>(plot_parent->querySelectors("series_pie")->getAttribute("labels"));
      auto labels = GRM::get<std::vector<std::string>>((*context)[labels_key]);

      /* clear child nodes */
      del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
      clearOldChildren(&del, element);

      auto scale_factor = static_cast<double>(element->getAttribute("_scale_factor"));
      auto initial_scale_factor = static_cast<double>(element->getAttribute("_initial_scale_factor"));
      auto h = static_cast<double>(element->getAttribute("_start_h"));

      if (!GRM::Render::getViewport(element, &viewport[0], &viewport[1], &viewport[2], &viewport[3]))
        throw NotFoundError(element->localName() + " doesn't have a viewport but it should.\n");

      gr_selntran(1);

      if (!element->hasAttribute("_select_specific_xform_set_by_user")) render->setSelectSpecificXform(element, 0);
      if (!element->hasAttribute("_scale_set_by_user")) render->setScale(element, 0);

      if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
        {
          fr = global_creator->createFillRect(viewport[0], viewport[1], viewport[2], viewport[3]);
          fr->setAttribute("_child_id", child_id++);
          element->append(fr);
        }
      else
        {
          fr = element->querySelectors("fill_rect[_child_id=" + std::to_string(child_id++) + "]");
          if (fr != nullptr)
            global_creator->createFillRect(viewport[0], viewport[1], viewport[2], viewport[3], 0, 0, -1, fr);
        }

      if (!element->hasAttribute("_fill_int_style_set_by_user")) render->setFillIntStyle(element, GKS_K_INTSTYLE_SOLID);
      if (!element->hasAttribute("_fill_color_ind_set_by_user")) render->setFillColorInd(element, 0);

      if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
        {
          dr = global_creator->createDrawRect(viewport[0], viewport[1], viewport[2], viewport[3]);
          dr->setAttribute("_child_id", child_id++);
          element->append(dr);
        }
      else
        {
          dr = element->querySelectors("draw_rect[_child_id=" + std::to_string(child_id++) + "]");
          if (dr != nullptr) global_creator->createDrawRect(viewport[0], viewport[1], viewport[2], viewport[3], dr);
        }

      if (!element->hasAttribute("_line_type_set_by_user")) render->setLineType(element, GKS_K_INTSTYLE_SOLID);
      if (!element->hasAttribute("_line_color_ind_set_by_user")) render->setLineColorInd(element, 1);
      if (!element->hasAttribute("_line_width_set_by_user")) render->setLineWidth(element, 1);

      for (auto &current_label : labels)
        {
          std::shared_ptr<GRM::Element> label_elem;
          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              label_elem = render->createElement("label");
              label_elem->setAttribute("_child_id", child_id++);
              element->append(label_elem);
            }
          else
            {
              label_elem = element->querySelectors("label[_child_id=" + std::to_string(child_id++) + "]");
            }
          if (label_elem != nullptr)
            {
              if (!label_elem->hasAttribute("_char_height_set_by_user"))
                {
                  label_elem->setAttribute("char_height",
                                           static_cast<double>(plot_parent->getAttribute("char_height")) *
                                               initial_scale_factor);
                }
              else
                {
                  label_elem->setAttribute("char_height", static_cast<double>(label_elem->getAttribute("char_height")));
                }
              processCharHeight(label_elem);
              if (!label_elem->hasAttribute("_text_align_vertical_set_by_user"))
                {
                  auto text_align_vertical = GKS_K_TEXT_VALIGN_HALF;
                  if (element->hasAttribute("text_align_vertical"))
                    text_align_vertical = static_cast<int>(element->getAttribute("text_align_vertical"));
                  label_elem->setAttribute("text_align_vertical", text_align_vertical);
                }
              if (!label_elem->hasAttribute("_text_align_horizontal_set_by_user"))
                {
                  auto text_align_horizontal = GKS_K_TEXT_HALIGN_LEFT;
                  if (element->hasAttribute("text_align_horizontal"))
                    text_align_horizontal = static_cast<int>(element->getAttribute("text_align_horizontal"));
                  label_elem->setAttribute("text_align_horizontal", text_align_horizontal);
                }
              if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
                {
                  fr = global_creator->createFillRect(
                      viewport[0] + 0.02 * scale_factor, viewport[0] + 0.04 * scale_factor,
                      viewport[2] + (0.5 * h + 0.01) * scale_factor, viewport[2] + (0.5 * h + 0.03) * scale_factor);
                  fr->setAttribute("_child_id", 0);
                  label_elem->append(fr);
                }
              else
                {
                  fr = label_elem->querySelectors("fill_rect[_child_id=0]");
                  if (fr != nullptr)
                    global_creator->createFillRect(viewport[0] + 0.02 * scale_factor, viewport[0] + 0.04 * scale_factor,
                                                   viewport[2] + (0.5 * h + 0.01) * scale_factor,
                                                   viewport[2] + (0.5 * h + 0.03) * scale_factor, 0, 0, -1, fr);
                }
              if (fr != nullptr)
                {
                  auto central_region = plot_parent->querySelectors("central_region");
                  for (const auto &child : central_region->children())
                    {
                      if (child->localName() == "series_pie")
                        {
                          std::shared_ptr<GRM::Element> pie_segment;
                          pie_segment =
                              child->querySelectors("pie_segment[_child_id=" + std::to_string(child_id - 3) + "]");
                          if (pie_segment != nullptr)
                            {
                              int color_ind = static_cast<int>(pie_segment->getAttribute("fill_color_ind"));
                              auto color_rep = static_cast<std::string>(
                                  pie_segment->getAttribute("colorrep." + std::to_string(color_ind)));
                              fr->setAttribute("fill_color_ind", color_ind);
                              if (!color_rep.empty())
                                fr->setAttribute("colorrep." + std::to_string(color_ind), color_rep);
                              if (!fr->hasAttribute("_fill_int_style_set_by_user"))
                                {
                                  if (pie_segment->hasAttribute("fill_int_style"))
                                    {
                                      fr->setAttribute("fill_int_style",
                                                       static_cast<int>(pie_segment->getAttribute("fill_int_style")));
                                    }
                                  else if (pie_segment->parentElement()->hasAttribute("fill_int_style"))
                                    {
                                      fr->setAttribute("fill_int_style",
                                                       static_cast<int>(pie_segment->parentElement()->getAttribute(
                                                           "fill_int_style")));
                                    }
                                }
                              if (!fr->hasAttribute("_fill_style_set_by_user"))
                                {
                                  if (pie_segment->hasAttribute("fill_style"))
                                    {
                                      fr->setAttribute("fill_style",
                                                       static_cast<int>(pie_segment->getAttribute("fill_style")));
                                    }
                                  else if (pie_segment->parentElement()->hasAttribute("fill_style"))
                                    {
                                      fr->setAttribute(
                                          "fill_style",
                                          static_cast<int>(pie_segment->parentElement()->getAttribute("fill_style")));
                                    }
                                }
                            }
                          break;
                        }
                    }
                }

              if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
                {
                  dr = global_creator->createDrawRect(
                      viewport[0] + 0.02 * scale_factor, viewport[0] + 0.04 * scale_factor,
                      viewport[2] + (0.5 * h + 0.01) * scale_factor, viewport[2] + (0.5 * h + 0.03) * scale_factor);
                  dr->setAttribute("_child_id", 1);
                  label_elem->append(dr);
                }
              else
                {
                  dr = label_elem->querySelectors("draw_rect[_child_id=1]");
                  if (dr != nullptr)
                    global_creator->createDrawRect(viewport[0] + 0.02 * scale_factor, viewport[0] + 0.04 * scale_factor,
                                                   viewport[2] + (0.5 * h + 0.01) * scale_factor,
                                                   viewport[2] + (0.5 * h + 0.03) * scale_factor, dr);
                }
              if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
                {
                  text = global_creator->createText(viewport[0] + 0.05 * scale_factor,
                                                    viewport[2] + (0.5 * h + 0.02) * scale_factor, current_label);
                  text->setAttribute("_child_id", 2);
                  label_elem->append(text);
                }
              else
                {
                  text = label_elem->querySelectors("text[_child_id=2]");
                  if (text != nullptr)
                    global_creator->createText(viewport[0] + 0.05 * scale_factor,
                                               viewport[2] + (0.5 * h + 0.02) * scale_factor, current_label,
                                               CoordinateSpace::NDC, text);
                }

              gr_inqtext(0, 0, current_label.data(), tbx, tby);
              viewport[0] += tbx[2] - tbx[0] + 0.05 * scale_factor;
            }
        }

      processLineColorInd(element);
      processLineWidth(element);
      processLineType(element);
    }
  processSelectSpecificXform(element);
  processScale(element);
  processFillStyle(element);
  processFillIntStyle(element);
  processFillColorInd(element);
}

void processBar(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  double x1, x2, y1, y2;
  int bar_color_index, edge_color_index, color_save_spot = PLOT_CUSTOM_COLOR_INDEX;
  std::shared_ptr<GRM::Element> fill_rect, draw_rect, text_elem;
  std::string orientation = PLOT_DEFAULT_ORIENTATION, text;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  double line_width = NAN, y_lightness = NAN;
  std::vector<double> bar_color_rgb, edge_color_rgb;
  int child_id = 0;
  auto global_render = grm_get_render();
  auto global_creator = grm_get_creator();

  x1 = static_cast<double>(element->getAttribute("x1"));
  x2 = static_cast<double>(element->getAttribute("x2"));
  y1 = static_cast<double>(element->getAttribute("y1"));
  y2 = static_cast<double>(element->getAttribute("y2"));
  bar_color_index = static_cast<int>(element->getAttribute("fill_color_ind"));
  if (element->hasAttribute("_fill_color_ind_set_by_user"))
    bar_color_index = static_cast<int>(element->getAttribute("_fill_color_ind_set_by_user"));
  edge_color_index = static_cast<int>(element->getAttribute("line_color_ind"));
  if (element->hasAttribute("_line_color_ind_set_by_user"))
    edge_color_index = static_cast<int>(element->getAttribute("_line_color_ind_set_by_user"));
  if (element->hasAttribute("text")) text = static_cast<std::string>(element->getAttribute("text"));
  if (element->hasAttribute("line_width")) line_width = static_cast<double>(element->getAttribute("line_width"));
  if (element->parentElement()->hasAttribute("transparency")) processTransparency(element->parentElement());

  if (element->hasAttribute("fill_color_rgb"))
    {
      auto bar_color_rgb_key = static_cast<std::string>(element->getAttribute("fill_color_rgb"));
      bar_color_rgb = GRM::get<std::vector<double>>((*context)[bar_color_rgb_key]);
    }
  if (element->hasAttribute("line_color_rgb"))
    {
      auto edge_color_rgb_key = static_cast<std::string>(element->getAttribute("line_color_rgb"));
      edge_color_rgb = GRM::get<std::vector<double>>((*context)[edge_color_rgb_key]);
    }

  /* clear old rects */
  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
    {
      fill_rect = global_creator->createFillRect(x1, x2, y1, y2);
      fill_rect->setAttribute("_child_id", child_id++);
      element->append(fill_rect);
    }
  else
    {
      fill_rect = element->querySelectors("fill_rect[_child_id=" + std::to_string(child_id++) + "]");
      if (fill_rect != nullptr) global_creator->createFillRect(x1, x2, y1, y2, 0, 0, -1, fill_rect);
    }
  if (fill_rect != nullptr)
    {
      if (!fill_rect->hasAttribute("_fill_int_style_set_by_user"))
        {
          auto fill_int_style = GKS_K_INTSTYLE_SOLID;
          if (element->hasAttribute("fill_int_style"))
            fill_int_style = static_cast<int>(element->getAttribute("fill_int_style"));
          if (element->hasAttribute("_fill_int_style_set_by_user"))
            fill_int_style = static_cast<int>(element->getAttribute("_fill_int_style_set_by_user"));
          global_render->setFillIntStyle(fill_rect, fill_int_style);
        }
      if (!fill_rect->hasAttribute("_fill_style_set_by_user"))
        {
          auto fill_style = 0;
          if (element->hasAttribute("fill_style")) fill_style = static_cast<int>(element->getAttribute("fill_style"));
          if (element->hasAttribute("_fill_style_set_by_user"))
            fill_style = static_cast<int>(element->getAttribute("_fill_style_set_by_user"));
          if (fill_style != 0) global_render->setFillStyle(fill_rect, fill_style);
        }

      if (!bar_color_rgb.empty() && bar_color_rgb[0] != -1)
        {
          global_render->setColorRep(fill_rect, color_save_spot, bar_color_rgb[0], bar_color_rgb[1], bar_color_rgb[2]);
          bar_color_index = color_save_spot;
          processColorReps(fill_rect);
        }
      global_render->setFillColorInd(fill_rect, bar_color_index);

      if (!text.empty())
        {
          int color;
          if (bar_color_index != -1)
            {
              color = bar_color_index;
            }
          else if (element->hasAttribute("fill_color_ind"))
            {
              color = static_cast<int>(element->getAttribute("fill_color_ind"));
            }
          y_lightness = getLightness(color);
        }
    }

  if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
    {
      draw_rect = global_creator->createDrawRect(x1, x2, y1, y2);
      draw_rect->setAttribute("_child_id", child_id++);
      element->append(draw_rect);
    }
  else
    {
      draw_rect = element->querySelectors("draw_rect[_child_id=" + std::to_string(child_id++) + "]");
      if (draw_rect != nullptr) global_creator->createDrawRect(x1, x2, y1, y2, draw_rect);
    }
  if (draw_rect != nullptr)
    {
      draw_rect->setAttribute("z_index", 2);

      if (!edge_color_rgb.empty() && edge_color_rgb[0] != -1)
        {
          global_render->setColorRep(draw_rect, color_save_spot - 1, edge_color_rgb[0], edge_color_rgb[1],
                                     edge_color_rgb[2]);
          edge_color_index = color_save_spot - 1;
        }
      if (element->parentElement()->localName() == "series_barplot")
        element->parentElement()->setAttribute("line_color_ind", edge_color_index);
      global_render->setLineColorInd(draw_rect, edge_color_index);
      processLineColorInd(draw_rect);
      if (!std::isnan(line_width)) global_render->setLineWidth(draw_rect, line_width);
    }

  if (!text.empty())
    {
      if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
        {
          text_elem = global_creator->createText((x1 + x2) / 2, (y1 + y2) / 2, text, CoordinateSpace::WC);
          text_elem->setAttribute("_child_id", child_id++);
          element->append(text_elem);
        }
      else
        {
          text_elem = element->querySelectors("text[_child_id=" + std::to_string(child_id++) + "]");
          if (text_elem != nullptr)
            global_creator->createText((x1 + x2) / 2, (y1 + y2) / 2, text, CoordinateSpace::WC, text_elem);
        }
      if (text_elem != nullptr)
        {
          text_elem->setAttribute("z_index", 2);
          if (!text_elem->hasAttribute("_text_align_vertical_set_by_user"))
            {
              auto text_align_vertical = 2;
              if (element->hasAttribute("text_align_vertical"))
                text_align_vertical = static_cast<int>(element->getAttribute("text_align_vertical"));
              text_elem->setAttribute("text_align_vertical", text_align_vertical);
            }
          if (!text_elem->hasAttribute("_text_align_horizontal_set_by_user"))
            {
              auto text_align_horizontal = 3;
              if (element->hasAttribute("text_align_horizontal"))
                text_align_horizontal = static_cast<int>(element->getAttribute("text_align_horizontal"));
              text_elem->setAttribute("text_align_horizontal", text_align_horizontal);
            }
          global_render->setTextWidthAndHeight(text_elem, x2 - x1, y2 - y1);
          if (!std::isnan(y_lightness) && !text_elem->hasAttribute("_text_color_ind_set_by_user"))
            global_render->setTextColorInd(text_elem, (y_lightness < 0.4) ? 0 : 1);
        }
    }
}

void processLayoutGrid(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  auto viewport_normalized_x_min = static_cast<double>(element->getAttribute("_viewport_normalized_x_min_org"));
  auto viewport_normalized_x_max = static_cast<double>(element->getAttribute("_viewport_normalized_x_max_org"));
  auto viewport_normalized_y_min = static_cast<double>(element->getAttribute("_viewport_normalized_y_min_org"));
  auto viewport_normalized_y_max = static_cast<double>(element->getAttribute("_viewport_normalized_y_max_org"));

  applyMoveTransformation(element);
  gr_setviewport(viewport_normalized_x_min, viewport_normalized_x_max, viewport_normalized_y_min,
                 viewport_normalized_y_max);
}

void processNonUniformPolarCellArray(const std::shared_ptr<GRM::Element> &element,
                                     const std::shared_ptr<GRM::Context> &context)
{
  auto theta_key = static_cast<std::string>(element->getAttribute("theta"));
  auto r_key = static_cast<std::string>(element->getAttribute("r"));
  auto theta_org = static_cast<double>(element->getAttribute("theta_org"));
  if (element->hasAttribute("_theta_org_set_by_user"))
    {
      theta_org = static_cast<double>(element->getAttribute("_theta_org_set_by_user"));
      element->setAttribute("theta_org", theta_org);
    }
  auto r_org = static_cast<double>(element->getAttribute("r_org"));
  if (element->hasAttribute("_r_org_set_by_user"))
    {
      r_org = static_cast<double>(element->getAttribute("_r_org_set_by_user"));
      element->setAttribute("r_org", r_org);
    }
  auto dim_r = static_cast<int>(element->getAttribute("r_dim"));
  if (element->hasAttribute("_r_dim_set_by_user"))
    {
      dim_r = static_cast<int>(element->getAttribute("_r_dim_set_by_user"));
      element->setAttribute("r_dim", dim_r);
    }
  auto dim_theta = static_cast<int>(element->getAttribute("theta_dim"));
  if (element->hasAttribute("_theta_dim_set_by_user"))
    {
      dim_theta = static_cast<int>(element->getAttribute("_theta_dim_set_by_user"));
      element->setAttribute("theta_dim", dim_theta);
    }
  auto s_col = static_cast<int>(element->getAttribute("start_col"));
  if (element->hasAttribute("_start_col_set_by_user"))
    {
      s_col = static_cast<int>(element->getAttribute("_start_col_set_by_user"));
      element->setAttribute("start_col", s_col);
    }
  auto s_row = static_cast<int>(element->getAttribute("start_row"));
  if (element->hasAttribute("_start_row_set_by_user"))
    {
      s_row = static_cast<int>(element->getAttribute("_start_row_set_by_user"));
      element->setAttribute("start_row", s_row);
    }
  auto n_col = static_cast<int>(element->getAttribute("num_col"));
  if (element->hasAttribute("_num_col_set_by_user"))
    {
      n_col = static_cast<int>(element->getAttribute("_num_col_set_by_user"));
      element->setAttribute("num_col", n_col);
    }
  auto n_row = static_cast<int>(element->getAttribute("num_row"));
  if (element->hasAttribute("_num_row_set_by_user"))
    {
      n_row = static_cast<int>(element->getAttribute("_num_row_set_by_user"));
      element->setAttribute("num_row", n_row);
    }
  auto color_key = static_cast<std::string>(element->getAttribute("color_ind_values"));

  auto r_vec = GRM::get<std::vector<double>>((*context)[r_key]);
  auto theta_vec = GRM::get<std::vector<double>>((*context)[theta_key]);
  auto color_vec = GRM::get<std::vector<int>>((*context)[color_key]);

  double *theta = &(theta_vec[0]);
  double *r = &(r_vec[0]);
  int *color = &(color_vec[0]);
  applyMoveTransformation(element);

  if (grm_get_render()->getRedrawWs())
    gr_nonuniformpolarcellarray(theta_org, r_org, theta, r, dim_theta, dim_r, s_col, s_row, n_col, n_row, color);
}

void processNonuniformCellArray(const std::shared_ptr<GRM::Element> &element,
                                const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for nonuniform_cell_array
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));

  auto dimx = static_cast<int>(element->getAttribute("x_dim"));
  if (element->hasAttribute("_x_dim_set_by_user"))
    {
      dimx = static_cast<int>(element->getAttribute("_x_dim_set_by_user"));
      element->setAttribute("x_dim", dimx);
    }
  auto dimy = static_cast<int>(element->getAttribute("y_dim"));
  if (element->hasAttribute("_y_dim_set_by_user"))
    {
      dimy = static_cast<int>(element->getAttribute("_y_dim_set_by_user"));
      element->setAttribute("y_dim", dimy);
    }
  auto scol = static_cast<int>(element->getAttribute("start_col"));
  if (element->hasAttribute("_start_col_set_by_user"))
    {
      scol = static_cast<int>(element->getAttribute("_start_col_set_by_user"));
      element->setAttribute("start_col", scol);
    }
  auto srow = static_cast<int>(element->getAttribute("start_row"));
  if (element->hasAttribute("_start_row_set_by_user"))
    {
      srow = static_cast<int>(element->getAttribute("_start_row_set_by_user"));
      element->setAttribute("start_row", srow);
    }
  auto ncol = static_cast<int>(element->getAttribute("num_col"));
  if (element->hasAttribute("_num_col_set_by_user"))
    {
      ncol = static_cast<int>(element->getAttribute("_num_col_set_by_user"));
      element->setAttribute("num_col", ncol);
    }
  auto nrow = static_cast<int>(element->getAttribute("num_row"));
  if (element->hasAttribute("_num_row_set_by_user"))
    {
      nrow = static_cast<int>(element->getAttribute("_num_row_set_by_user"));
      element->setAttribute("num_row", nrow);
    }
  auto color = static_cast<std::string>(element->getAttribute("color_ind_values"));

  auto x_p = (double *)&(GRM::get<std::vector<double>>((*context)[x])[0]);
  auto y_p = (double *)&(GRM::get<std::vector<double>>((*context)[y])[0]);

  auto color_p = (int *)&(GRM::get<std::vector<int>>((*context)[color])[0]);
  applyMoveTransformation(element);
  if (grm_get_render()->getRedrawWs()) gr_nonuniformcellarray(x_p, y_p, dimx, dimy, scol, srow, ncol, nrow, color_p);
}

void processOverlayElement(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  int child_id = 0;
  double metric_width, metric_height, aspect_ratio_ws;
  auto global_creator = grm_get_creator();

  auto type = static_cast<std::string>(element->getAttribute("element_type"));
  auto x = static_cast<double>(element->getAttribute("x"));
  auto y = static_cast<double>(element->getAttribute("y"));

  /* clear old childs */
  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  GRM::getFigureSize(nullptr, nullptr, &metric_width, &metric_height);
  aspect_ratio_ws = metric_width / metric_height;

  auto start_aspect_ratio_ws = static_cast<double>(
      element->parentElement()->parentElement()->querySelectors("plot")->getAttribute("_start_aspect_ratio"));
  y /= ((start_aspect_ratio_ws > 1) ? 1.0 / start_aspect_ratio_ws : start_aspect_ratio_ws);

  if (aspect_ratio_ws > 1)
    {
      y /= aspect_ratio_ws;
    }
  else
    {
      x *= aspect_ratio_ws;
    }

  if (type == "text")
    {
      std::shared_ptr<GRM::Element> text;
      if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
        {
          text = global_creator->createText(x, y, "");
          text->setAttribute("_child_id", child_id++);
          element->append(text);
        }
      else
        {
          std::string text_content = "";
          text = element->querySelectors("text[_child_id=" + std::to_string(child_id++) + "]");
          if (text != nullptr)
            {
              if (text->hasAttribute("text")) text_content = static_cast<std::string>(text->getAttribute("text"));
              global_creator->createText(x, y, text_content, CoordinateSpace::NDC, text);
            }
        }
      if (text != nullptr)
        {
          if (!text->hasAttribute("font")) text->setAttribute("font", PLOT_DEFAULT_FONT);
          if (!text->hasAttribute("font_precision")) text->setAttribute("font_precision", PLOT_DEFAULT_FONT_PRECISION);
          if (!text->hasAttribute("_char_height_set_by_user"))
            text->setAttribute("char_height", (aspect_ratio_ws > 1) ? PLOT_2D_CHAR_HEIGHT / aspect_ratio_ws
                                                                    : PLOT_2D_CHAR_HEIGHT * aspect_ratio_ws);
        }
    }
  else if (type == "image")
    {
      std::shared_ptr<GRM::Element> image;
      double w[4] = {0.0, 1.0, 0.0, 1.0};
      auto image_width = static_cast<int>(element->getAttribute("_width"));
      auto image_height = static_cast<int>(element->getAttribute("_height"));
      auto width = static_cast<double>(element->getAttribute("width_abs"));
      auto height = static_cast<double>(element->getAttribute("height_abs"));
      auto data_key = static_cast<std::string>(element->getAttribute("data"));
      auto data = GRM::get<std::vector<int>>((*context)[data_key]);

      if (aspect_ratio_ws > 1)
        {
          w[2] /= aspect_ratio_ws;
          w[3] /= aspect_ratio_ws;
          height /= aspect_ratio_ws;
          width /= aspect_ratio_ws;
        }
      else
        {
          w[0] *= aspect_ratio_ws;
          w[1] *= aspect_ratio_ws;
          width *= aspect_ratio_ws;
          height *= aspect_ratio_ws;
        }

      element->setAttribute("window_x_min", w[0]);
      element->setAttribute("window_x_max", w[1]);
      element->setAttribute("window_y_min", w[2]);
      element->setAttribute("window_y_max", w[3]);
      processWindow(element);

      if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
        {
          image = global_creator->createDrawImage(x, y, x + width, y + height, image_width, image_height, data_key,
                                                  data, 0);
          image->setAttribute("_child_id", child_id++);
          element->append(image);
        }
      else
        {
          image = element->querySelectors("draw_image[_child_id=" + std::to_string(child_id++) + "]");
          if (image != nullptr)
            global_creator->createDrawImage(x, y, x + width, y + height, image_width, image_height, data_key, data, 0,
                                            nullptr, image);
        }
    }
}

void processPanzoom(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  ; /* panzoom is being processed in the processLimits routine */
}

void processPolarCellArray(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  auto theta_org = static_cast<double>(element->getAttribute("theta_org"));
  if (element->hasAttribute("_theta_org_set_by_user"))
    {
      theta_org = static_cast<double>(element->getAttribute("_theta_org_set_by_user"));
      element->setAttribute("theta_org", theta_org);
    }
  auto r_org = static_cast<double>(element->getAttribute("r_org"));
  if (element->hasAttribute("_r_org_set_by_user"))
    {
      r_org = static_cast<double>(element->getAttribute("_r_org_set_by_user"));
      element->setAttribute("r_org", r_org);
    }
  auto theta_min = static_cast<double>(element->getAttribute("theta_min"));
  if (element->hasAttribute("_theta_min_set_by_user"))
    {
      theta_min = static_cast<double>(element->getAttribute("_theta_min_set_by_user"));
      element->setAttribute("theta_min", theta_min);
    }
  auto theta_max = static_cast<double>(element->getAttribute("theta_max"));
  if (element->hasAttribute("_theta_max_set_by_user"))
    {
      theta_max = static_cast<double>(element->getAttribute("_theta_max_set_by_user"));
      element->setAttribute("theta_max", theta_max);
    }
  auto r_min = static_cast<double>(element->getAttribute("r_min"));
  if (element->hasAttribute("_r_min_set_by_user"))
    {
      r_min = static_cast<double>(element->getAttribute("_r_min_set_by_user"));
      element->setAttribute("r_min", r_min);
    }
  auto r_max = static_cast<double>(element->getAttribute("r_max"));
  if (element->hasAttribute("_r_max_set_by_user"))
    {
      r_max = static_cast<double>(element->getAttribute("_r_max_set_by_user"));
      element->setAttribute("r_max", r_max);
    }
  auto dim_r = static_cast<int>(element->getAttribute("r_dim"));
  if (element->hasAttribute("_r_dim_set_by_user"))
    {
      dim_r = static_cast<int>(element->getAttribute("_r_dim_set_by_user"));
      element->setAttribute("r_dim", dim_r);
    }
  auto dim_theta = static_cast<int>(element->getAttribute("theta_dim"));
  if (element->hasAttribute("_theta_dim_set_by_user"))
    {
      dim_theta = static_cast<int>(element->getAttribute("_theta_dim_set_by_user"));
      element->setAttribute("theta_dim", dim_theta);
    }
  auto s_col = static_cast<int>(element->getAttribute("start_col"));
  if (element->hasAttribute("_start_col_set_by_user"))
    {
      s_col = static_cast<int>(element->getAttribute("_start_col_set_by_user"));
      element->setAttribute("start_col", s_col);
    }
  auto s_row = static_cast<int>(element->getAttribute("start_row"));
  if (element->hasAttribute("_start_row_set_by_user"))
    {
      s_row = static_cast<int>(element->getAttribute("_start_row_set_by_user"));
      element->setAttribute("start_row", s_row);
    }
  auto n_col = static_cast<int>(element->getAttribute("num_col"));
  if (element->hasAttribute("_num_col_set_by_user"))
    {
      n_col = static_cast<int>(element->getAttribute("_num_col_set_by_user"));
      element->setAttribute("num_col", n_col);
    }
  auto n_row = static_cast<int>(element->getAttribute("num_row"));
  if (element->hasAttribute("_num_row_set_by_user"))
    {
      n_row = static_cast<int>(element->getAttribute("_num_row_set_by_user"));
      element->setAttribute("num_row", n_row);
    }
  auto color_key = static_cast<std::string>(element->getAttribute("color_ind_values"));

  auto color_vec = GRM::get<std::vector<int>>((*context)[color_key]);
  int *color = &(color_vec[0]);
  applyMoveTransformation(element);

  if (grm_get_render()->getRedrawWs())
    gr_polarcellarray(theta_org, r_org, theta_min, theta_max, r_min, r_max, dim_theta, dim_r, s_col, s_row, n_col,
                      n_row, color);
}

void processPolyline(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing Function for polyline
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  applyMoveTransformation(element);

  auto name = static_cast<std::string>(element->getAttribute("name"));
  bool hidden = element->hasAttribute("_hidden") && static_cast<int>(element->getAttribute("_hidden"));

  if (startsWith(name, "x-axis-line") || startsWith(name, "y-axis-line")) gr_setclip(0);
  if (element->getAttribute("x").isString() && element->getAttribute("y").isString())
    {
      auto x = static_cast<std::string>(element->getAttribute("x"));
      auto y = static_cast<std::string>(element->getAttribute("y"));

      auto x_vec = GRM::get<std::vector<double>>((*context)[x]);
      auto y_vec = GRM::get<std::vector<double>>((*context)[y]);

      auto n = std::min<int>((int)x_vec.size(), (int)y_vec.size());
      auto group = element->parentElement();
      if (element->hasAttribute("line_color_ind")) processLineColorInd(element);
      if ((element->hasAttribute("line_types") || element->hasAttribute("line_widths") ||
           element->hasAttribute("line_color_indices")) ||
          ((parent_types.count(group->localName())) &&
           (group->hasAttribute("line_types") || group->hasAttribute("line_widths") ||
            group->hasAttribute("line_color_indices"))))
        {
          lineHelper(element, context, "polyline");
        }
      else if (grm_get_render()->getRedrawWs() && !hidden)
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

      if (element->hasAttribute("line_color_ind")) processLineColorInd(element);
      if (grm_get_render()->getRedrawWs() && !hidden) gr_polyline(2, x, y);
    }
  if (startsWith(name, "x-axis-line") || startsWith(name, "y-axis-line")) gr_setclip(1);
}

void processPolyline3d(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for polyline 3d
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  bool hidden = element->hasAttribute("_hidden") && static_cast<int>(element->getAttribute("_hidden"));

  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));
  auto z = static_cast<std::string>(element->getAttribute("z"));

  auto x_vec = GRM::get<std::vector<double>>((*context)[x]);
  auto y_vec = GRM::get<std::vector<double>>((*context)[y]);
  auto z_vec = GRM::get<std::vector<double>>((*context)[z]);

  double *x_p = &(x_vec[0]);
  double *y_p = &(y_vec[0]);
  double *z_p = &(z_vec[0]);
  auto group = element->parentElement();

  applyMoveTransformation(element);
  if ((element->hasAttribute("line_types") || element->hasAttribute("line_widths") ||
       element->hasAttribute("line_color_indices")) ||
      ((parent_types.count(group->localName())) &&
       (group->hasAttribute("line_types") || group->hasAttribute("line_widths") ||
        group->hasAttribute("line_color_indices"))))
    {
      lineHelper(element, context, "polyline_3d");
    }
  else
    {
      processSpace3d(element->parentElement()->parentElement());
      if (grm_get_render()->getRedrawWs() && !hidden) gr_polyline3d(static_cast<int>(x_vec.size()), x_p, y_p, z_p);
    }
}

void processPolymarker(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for polymarker
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */

  auto name = static_cast<std::string>(element->getAttribute("name"));
  bool hidden = element->hasAttribute("_hidden") && static_cast<int>(element->getAttribute("_hidden"));
  auto redraw_ws = grm_get_render()->getRedrawWs();

  applyMoveTransformation(element);

  if (startsWith(name, "marginal line")) gr_setclip(0);
  if (element->getAttribute("x").isString() && element->getAttribute("y").isString())
    {
      auto x = static_cast<std::string>(element->getAttribute("x"));
      auto y = static_cast<std::string>(element->getAttribute("y"));

      auto x_vec = GRM::get<std::vector<double>>((*context)[x]);
      auto y_vec = GRM::get<std::vector<double>>((*context)[y]);

      auto n = std::min<int>(static_cast<int>(x_vec.size()), static_cast<int>(y_vec.size()));
      auto group = element->parentElement();
      if ((element->hasAttribute("marker_types") || element->hasAttribute("marker_sizes") ||
           element->hasAttribute("marker_color_indices")) ||
          (parent_types.count(group->localName()) &&
           (group->hasAttribute("marker_types") || group->hasAttribute("marker_sizes") ||
            group->hasAttribute("marker_color_indices"))))
        {
          markerHelper(element, context, "polymarker");
        }
      else
        {
          if (redraw_ws && !hidden) gr_polymarker(n, (double *)&(x_vec[0]), (double *)&(y_vec[0]));
        }
    }
  else if (element->getAttribute("x").isDouble() && element->getAttribute("y").isDouble())
    {
      auto x = static_cast<double>(element->getAttribute("x"));
      auto y = static_cast<double>(element->getAttribute("y"));
      if (redraw_ws) gr_polymarker(1, &x, &y);
    }
  if (startsWith(name, "marginal line")) gr_setclip(0);
}

void processPolymarker3d(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for polymarker_3d
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  bool hidden = element->hasAttribute("_hidden") && static_cast<int>(element->getAttribute("_hidden"));

  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));
  auto z = static_cast<std::string>(element->getAttribute("z"));

  auto x_vec = GRM::get<std::vector<double>>((*context)[x]);
  auto y_vec = GRM::get<std::vector<double>>((*context)[y]);
  auto z_vec = GRM::get<std::vector<double>>((*context)[z]);

  double *x_p = &(x_vec[0]);
  double *y_p = &(y_vec[0]);
  double *z_p = &(z_vec[0]);

  auto group = element->parentElement();
  applyMoveTransformation(element);
  if ((element->hasAttribute("marker_types") || element->hasAttribute("marker_sizes") ||
       element->hasAttribute("marker_color_indices")) ||
      (parent_types.count(group->localName()) &&
       (group->hasAttribute("marker_types") || group->hasAttribute("marker_sizes") ||
        group->hasAttribute("marker_color_indices"))))
    {
      markerHelper(element, context, "polymarker_3d");
    }
  else
    {
      processSpace3d(element->parentElement()->parentElement());
      if (grm_get_render()->getRedrawWs() && !hidden) gr_polymarker3d(static_cast<int>(x_vec.size()), x_p, y_p, z_p);
    }
}

void processSidePlotRegion(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  calculateViewport(element);
  applyMoveTransformation(element);
  processViewport(element);
}

static void drawYLine(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  double window[4];
  double ymin = 0, series_y_min = DBL_MAX;
  double line_x1, line_x2, line_y1, line_y2;
  std::shared_ptr<GRM::Element> series = nullptr, line, central_region, central_region_parent;
  std::string orientation = PLOT_DEFAULT_ORIENTATION;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;

  auto global_creator = grm_get_creator();
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

  window[0] = static_cast<double>(central_region->getAttribute("window_x_min"));
  window[1] = static_cast<double>(central_region->getAttribute("window_x_max"));
  window[2] = static_cast<double>(central_region->getAttribute("window_y_min"));
  window[3] = static_cast<double>(central_region->getAttribute("window_y_max"));

  auto y_log = plot_element->hasAttribute("y_log") && static_cast<int>(plot_element->getAttribute("y_log"));
  for (const auto &child : central_region->children())
    {
      if (child->localName() != "series_barplot" && child->localName() != "series_stem") continue;
      if (series == nullptr) series = child;
      if (child->hasAttribute("y_range_min"))
        {
          if (y_log)
            {
              series_y_min = grm_min(series_y_min, static_cast<double>(child->getAttribute("y_range_min")));
            }
          else
            {
              auto y_min = static_cast<double>(child->getAttribute("y_range_min"));
              if (series_y_min == DBL_MAX) series_y_min = y_min;
              series_y_min = grm_min(series_y_min, y_min);
            }
        }
    }
  if (series_y_min != DBL_MAX) ymin = series_y_min;
  if (plot_element->hasAttribute("_y_line_pos")) ymin = static_cast<double>(plot_element->getAttribute("_y_line_pos"));
  if (ymin <= 0 && y_log)
    {
      ymin = 1;
      if (series_y_min > 0) ymin = grm_min(ymin, series_y_min);
    }
  bool use_grplot_changes = plot_element->hasAttribute("use_grplot_changes")
                                ? static_cast<int>(plot_element->getAttribute("use_grplot_changes"))
                                : false;
  if (series != nullptr && series->localName() == "series_barplot" && ymin < 0 && !use_grplot_changes &&
      static_cast<std::string>(series->getAttribute("style")) == "stacked")
    ymin = 0;

  if (central_region->hasAttribute("orientation"))
    orientation = static_cast<std::string>(central_region->getAttribute("orientation"));

  bool is_vertical = orientation == "vertical";

  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  line = element->querySelectors("[name=\"y_line\"]");

  if (is_vertical)
    {
      line_x1 = ymin;
      line_x2 = ymin;
      line_y1 = window[2];
      line_y2 = window[3];
    }
  else
    {
      line_x1 = window[0];
      line_x2 = window[1];
      line_y1 = ymin;
      line_y2 = ymin;
    }

  if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT || line == nullptr)
    {
      line = global_creator->createPolyline(line_x1, line_x2, line_y1, line_y2, 0, 0.0, 1);
      element->append(line);
    }
  else if (line != nullptr)
    {
      global_creator->createPolyline(line_x1, line_x2, line_y1, line_y2, 0, 0.0, 1, line);
    }
  if (line != nullptr)
    {
      line->setAttribute("name", "y_line");
      line->setAttribute("z_index", 4);
    }
}

void processCoordinateSystem(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  int child_id = 0;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  std::shared_ptr<GRM::Element> axis, grid_3d, axes_3d, titles_3d;
  std::string type;
  auto plot_parent = element->parentElement();
  getPlotParent(plot_parent);
  auto kind = static_cast<std::string>(plot_parent->getAttribute("_kind"));
  auto global_creator = grm_get_creator();
  auto global_render = grm_get_render();

  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  for (const auto &parent_child : element->parentElement()->children())
    {
      if (strEqualsAny(parent_child->localName(), "series_barplot", "series_stem"))
        {
          auto series_kind = static_cast<std::string>(parent_child->getAttribute("kind"));
          if (strEqualsAny(series_kind, "barplot", "stem") && !element->hasAttribute("y_line"))
            element->setAttribute("y_line", true);
          break;
        }
    }
  /* 0-line */
  if (element->hasAttribute("y_line") && static_cast<int>(element->getAttribute("y_line")))
    {
      drawYLine(element, context);
    }
  else if (element->querySelectors("[name=\"y_line\"]"))
    {
      auto line = element->querySelectors("[name=\"y_line\"]");
      line->remove();
    }

  type = static_cast<std::string>(element->getAttribute("plot_type"));
  if (type == "3d")
    {
      // 3d plots are always in keep_aspect_ratio mode so the scaling with the aspect_ratio isn't needed here
      double char_height = PLOT_3D_CHAR_HEIGHT;
      if (!element->hasAttribute("_char_height_set_by_user")) element->setAttribute("char_height", char_height);
      processCharHeight(element);
    }

  if (element->hasAttribute("hide") && static_cast<int>(element->getAttribute("hide")))
    {
      del = DelValues::RECREATE_OWN_CHILDREN;
      clearOldChildren(&del, element);
      return;
    }

  if (type == "polar")
    {
      // create polar coordinate system
      std::shared_ptr<GRM::Element> radial_axes, theta_axes;

      if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
        {
          radial_axes = global_creator->createRadialAxes();
          radial_axes->setAttribute("_child_id", child_id++);
          element->append(radial_axes);
        }
      else
        {
          radial_axes = element->querySelectors("radial_axes[_child_id=" + std::to_string(child_id++) + "]");
          if (radial_axes != nullptr) global_creator->createRadialAxes(radial_axes);
        }
      if (radial_axes != nullptr)
        {
          if (!radial_axes->hasAttribute("text_color_ind")) radial_axes->setAttribute("text_color_ind", 1);
        }

      if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
        {
          theta_axes = global_creator->createThetaAxes();
          theta_axes->setAttribute("_child_id", child_id++);
          element->append(theta_axes);
        }
      else
        {
          theta_axes = element->querySelectors("theta_axes[_child_id=" + std::to_string(child_id++) + "]");
          if (theta_axes != nullptr) global_creator->createThetaAxes(theta_axes);
        }
      if (theta_axes != nullptr)
        {
          if (!theta_axes->hasAttribute("text_color_ind")) theta_axes->setAttribute("text_color_ind", 1);
        }
    }
  else
    {
      int tick_orientation = 1;
      bool x_grid =
          (element->hasAttribute("x_grid")) ? static_cast<int>(element->getAttribute("x_grid")) : PLOT_DEFAULT_XGRID;
      bool y_grid =
          (element->hasAttribute("y_grid")) ? static_cast<int>(element->getAttribute("y_grid")) : PLOT_DEFAULT_YGRID;

      if (!element->hasAttribute("_line_color_ind_set_by_user") ||
          !static_cast<int>(element->getAttribute("_line_color_ind_set_by_user")))
        global_render->setLineColorInd(element, 1);
      if (!element->hasAttribute("_line_width_set_by_user") ||
          !static_cast<int>(element->getAttribute("_line_width_set_by_user")))
        global_render->setLineWidth(element, 1);
      processLineColorInd(element);
      processLineWidth(element);

      if (type == "3d")
        {
          std::string x_label, y_label, z_label;
          bool z_grid;
          if (element->hasAttribute("z_grid"))
            {
              z_grid = static_cast<int>(element->getAttribute("z_grid"));
            }
          else
            {
              z_grid = PLOT_DEFAULT_ZGRID;
              element->setAttribute("z_grid", z_grid);
            }

          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              grid_3d = global_creator->createEmptyGrid3d(x_grid, false, z_grid);
              grid_3d->setAttribute("_child_id", child_id++);
              element->append(grid_3d);
            }
          else
            {
              grid_3d = element->querySelectors("grid_3d[_child_id=" + std::to_string(child_id++) + "]");
              if (grid_3d != nullptr) global_creator->createEmptyGrid3d(x_grid, false, z_grid, grid_3d);
            }
          if (grid_3d != nullptr)
            {
              if (!grid_3d->hasAttribute("x_origin_pos")) grid_3d->setAttribute("x_origin_pos", "low");
              if (!grid_3d->hasAttribute("y_origin_pos")) grid_3d->setAttribute("y_origin_pos", "high");
              if (!grid_3d->hasAttribute("z_origin_pos")) grid_3d->setAttribute("z_origin_pos", "low");
              if (!grid_3d->hasAttribute("x_major")) grid_3d->setAttribute("x_major", 2);
              if (!grid_3d->hasAttribute("y_major")) grid_3d->setAttribute("y_major", 0);
              if (!grid_3d->hasAttribute("z_major")) grid_3d->setAttribute("z_major", 2);
            }

          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              grid_3d = global_creator->createEmptyGrid3d(false, y_grid, false);
              grid_3d->setAttribute("_child_id", child_id++);
              element->append(grid_3d);
            }
          else
            {
              grid_3d = element->querySelectors("grid_3d[_child_id=" + std::to_string(child_id++) + "]");
              if (grid_3d != nullptr) global_creator->createEmptyGrid3d(false, y_grid, false, grid_3d);
            }
          if (grid_3d != nullptr)
            {
              if (!grid_3d->hasAttribute("x_origin_pos")) grid_3d->setAttribute("x_origin_pos", "low");
              if (!grid_3d->hasAttribute("y_origin_pos")) grid_3d->setAttribute("y_origin_pos", "high");
              if (!grid_3d->hasAttribute("z_origin_pos")) grid_3d->setAttribute("z_origin_pos", "low");
              if (!grid_3d->hasAttribute("x_major")) grid_3d->setAttribute("x_major", 0);
              if (!grid_3d->hasAttribute("y_major")) grid_3d->setAttribute("y_major", 2);
              if (!grid_3d->hasAttribute("z_major")) grid_3d->setAttribute("z_major", 0);
            }

          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              axes_3d = global_creator->createEmptyAxes3d(-tick_orientation);
              axes_3d->setAttribute("_child_id", child_id++);
              element->append(axes_3d);
            }
          else
            {
              axes_3d = element->querySelectors("axes_3d[_child_id=" + std::to_string(child_id++) + "]");
              if (axes_3d != nullptr)
                {
                  tick_orientation = -tick_orientation;
                  if (axes_3d->hasAttribute("tick_orientation") && del != DelValues::UPDATE_WITH_DEFAULT)
                    tick_orientation = static_cast<int>(axes_3d->getAttribute("tick_orientation"));
                  global_creator->createEmptyAxes3d(tick_orientation, axes_3d);
                }
            }
          if (axes_3d != nullptr)
            {
              if (!axes_3d->hasAttribute("x_origin_pos")) axes_3d->setAttribute("x_origin_pos", "low");
              if (!axes_3d->hasAttribute("y_origin_pos")) axes_3d->setAttribute("y_origin_pos", "low");
              if (!axes_3d->hasAttribute("z_origin_pos")) axes_3d->setAttribute("z_origin_pos", "low");
              if (!axes_3d->hasAttribute("y_tick")) axes_3d->setAttribute("y_tick", 0);
              if (!axes_3d->hasAttribute("y_major")) axes_3d->setAttribute("y_major", 0);
              if (!axes_3d->hasAttribute("z_index")) axes_3d->setAttribute("z_index", 7);
            }

          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              axes_3d = global_creator->createEmptyAxes3d(tick_orientation);
              axes_3d->setAttribute("_child_id", child_id++);
              element->append(axes_3d);
            }
          else
            {
              axes_3d = element->querySelectors("axes_3d[_child_id=" + std::to_string(child_id++) + "]");
              if (axes_3d != nullptr)
                {
                  if (axes_3d->hasAttribute("tick_orientation") && del != DelValues::UPDATE_WITH_DEFAULT)
                    tick_orientation = static_cast<int>(axes_3d->getAttribute("tick_orientation"));
                  global_creator->createEmptyAxes3d(tick_orientation, axes_3d);
                }
            }
          if (axes_3d != nullptr)
            {
              if (!axes_3d->hasAttribute("x_origin_pos")) axes_3d->setAttribute("x_origin_pos", "high");
              if (!axes_3d->hasAttribute("y_origin_pos")) axes_3d->setAttribute("y_origin_pos", "low");
              if (!axes_3d->hasAttribute("z_origin_pos")) axes_3d->setAttribute("z_origin_pos", "low");
              if (!axes_3d->hasAttribute("x_tick")) axes_3d->setAttribute("x_tick", 0);
              if (!axes_3d->hasAttribute("z_tick")) axes_3d->setAttribute("z_tick", 0);
              if (!axes_3d->hasAttribute("x_major")) axes_3d->setAttribute("x_major", 0);
              if (!axes_3d->hasAttribute("z_major")) axes_3d->setAttribute("z_major", 0);
              if (!axes_3d->hasAttribute("z_index")) axes_3d->setAttribute("z_index", 7);
            }

          x_label = element->hasAttribute("x_label") ? static_cast<std::string>(element->getAttribute("x_label")) : "";
          y_label = element->hasAttribute("y_label") ? static_cast<std::string>(element->getAttribute("y_label")) : "";
          z_label = element->hasAttribute("z_label") ? static_cast<std::string>(element->getAttribute("z_label")) : "";
          if (!x_label.empty() || !y_label.empty() || !z_label.empty())
            {
              if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
                {
                  titles_3d = global_creator->createTitles3d(x_label, y_label, z_label);
                  titles_3d->setAttribute("_child_id", child_id++);
                  element->append(titles_3d);
                }
              else
                {
                  titles_3d = element->querySelectors("titles_3d[_child_id=" + std::to_string(child_id++) + "]");
                  if (titles_3d != nullptr)
                    {
                      if (titles_3d->hasAttribute("x_label_3d"))
                        x_label = static_cast<std::string>(titles_3d->getAttribute("x_label_3d"));
                      if (titles_3d->hasAttribute("y_label_3d"))
                        y_label = static_cast<std::string>(titles_3d->getAttribute("y_label_3d"));
                      if (titles_3d->hasAttribute("z_label_3d"))
                        z_label = static_cast<std::string>(titles_3d->getAttribute("z_label_3d"));
                      titles_3d = global_creator->createTitles3d(x_label, y_label, z_label, titles_3d);
                    }
                }
              if (titles_3d)
                {
                  if (!titles_3d->hasAttribute("z_index")) titles_3d->setAttribute("z_index", 7);
                  if (!titles_3d->hasAttribute("scientific_format")) titles_3d->setAttribute("scientific_format", 3);
                }
            }
        }
      else
        {
          // y-axis
          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              axis = global_creator->createEmptyAxis();
              axis->setAttribute("_child_id", child_id++);
              element->append(axis);
            }
          else
            {
              axis = element->querySelectors("axis[_child_id=" + std::to_string(child_id++) + "]");
              if (axis != nullptr) axis = global_creator->createEmptyAxis(axis);
            }
          if (axis != nullptr && del != DelValues::UPDATE_WITHOUT_DEFAULT)
            {
              axis->setAttribute("axis_type", "y");
              if (plot_parent->hasAttribute("_twin_y_window_xform_a_org"))
                {
                  axis->setAttribute("name", "y-axis");
                  axis->setAttribute("mirrored_axis", false);
                }
              else
                {
                  axis->setAttribute("name", "y-axis mirrored");
                  axis->setAttribute("mirrored_axis", true);
                }
              axis->setAttribute("location", "y");
            }

          // x-axis
          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              axis = global_creator->createEmptyAxis();
              axis->setAttribute("_child_id", child_id++);
              element->append(axis);
            }
          else
            {
              axis = element->querySelectors("axis[_child_id=" + std::to_string(child_id++) + "]");
              if (axis != nullptr) axis = global_creator->createEmptyAxis(axis);
            }
          if (axis != nullptr && del != DelValues::UPDATE_WITHOUT_DEFAULT)
            {
              axis->setAttribute("axis_type", "x");
              if (plot_parent->hasAttribute("_twin_x_window_xform_a_org"))
                {
                  axis->setAttribute("name", "x-axis");
                  axis->setAttribute("mirrored_axis", false);
                }
              else
                {
                  axis->setAttribute("name", "x-axis mirrored");
                  axis->setAttribute("mirrored_axis", true);
                }
              axis->setAttribute("location", "x");
            }

          if (plot_parent->hasAttribute("_twin_y_window_xform_a_org"))
            {
              if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
                {
                  axis = global_creator->createEmptyAxis();
                  axis->setAttribute("_child_id", child_id++);
                  element->append(axis);
                }
              else
                {
                  axis = element->querySelectors("axis[_child_id=" + std::to_string(child_id++) + "]");
                  if (axis != nullptr) axis = global_creator->createEmptyAxis(axis);
                }
              if (axis != nullptr && del != DelValues::UPDATE_WITHOUT_DEFAULT)
                {
                  axis->setAttribute("axis_type", "y");
                  axis->setAttribute("name", "twin-y-axis");
                  axis->setAttribute("location", "twin_y");
                }
            }

          if (plot_parent->hasAttribute("_twin_x_window_xform_a_org"))
            {
              if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
                {
                  axis = global_creator->createEmptyAxis();
                  axis->setAttribute("_child_id", child_id++);
                  element->append(axis);
                }
              else
                {
                  axis = element->querySelectors("axis[_child_id=" + std::to_string(child_id++) + "]");
                  if (axis != nullptr) axis = global_creator->createEmptyAxis(axis);
                }
              if (axis != nullptr && del != DelValues::UPDATE_WITHOUT_DEFAULT)
                {
                  axis->setAttribute("axis_type", "x");
                  axis->setAttribute("name", "twin-x-axis");
                  axis->setAttribute("location", "twin_x");
                }
            }
        }
    }
  calculateViewport(element);
  applyMoveTransformation(element);
  processViewport(element);
}

void processSideRegion(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  int child_id = 0;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  std::shared_ptr<GRM::Element> plot_parent = element;
  getPlotParent(plot_parent);
  auto global_creator = grm_get_creator();

  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  if (element->hasAttribute("text_content"))
    {
      auto kind = static_cast<std::string>(plot_parent->getAttribute("_kind"));
      auto text = static_cast<std::string>(element->getAttribute("text_content"));
      auto location = static_cast<std::string>(element->getAttribute("location"));

      if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT && !text.empty() &&
          kind != "imshow")
        {
          auto text_elem = global_creator->createTextRegion();
          text_elem->setAttribute("_child_id", child_id++);
          element->appendChild(text_elem);
        }
      else
        {
          if (auto text_elem = element->querySelectors("text_region[_child_id=\"" + std::to_string(child_id++) + "\"]"))
            global_creator->createTextRegion(text_elem);
        }
    }

  calculateViewport(element);
  applyMoveTransformation(element);
  processViewport(element);
  processWindow(element);    /* needs to be set before space 3d is processed */
  processScale(plot_parent); /* needs to be set before flip is processed */
}

void processPolarBar(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  double r, rect;
  double r_lim_min, r_lim_max;
  std::complex<double> complex1;
  const double convert = 180.0 / M_PI;
  std::vector<double> f1, f2, arc_2_x, arc_2_y, theta_vec, bin_edges;
  int child_id = 0;
  int edge_color = 1, face_color = 989;
  double count, bin_width = -1.0, r_max;
  int num_bins, num_bin_edges = 0, bin_nr;
  std::string norm = "count", str;
  bool theta_flip = false, draw_edges = false, keep_radii_axes = false, r_lim = false, r_log = false;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  std::shared_ptr<GRM::Element> plot_parent = element;
  getPlotParent(plot_parent);
  auto global_root = grm_get_document_root();
  auto global_creator = grm_get_creator();
  auto global_render = grm_get_render();

  /* clear old polar-histogram children */
  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  // bin_nr is used for the position of the bar in the histogram
  bin_nr = static_cast<int>(element->getAttribute("bin_nr"));
  // count is already converted by normalization
  count = static_cast<double>(element->getAttribute("count"));

  if (plot_parent->hasAttribute("r_log")) r_log = static_cast<int>(plot_parent->getAttribute("r_log"));

  if (element->parentElement()->hasAttribute("transparency")) processTransparency(element->parentElement());
  if (element->hasAttribute("bin_width")) bin_width = static_cast<double>(element->getAttribute("bin_width"));
  if (element->hasAttribute("norm")) norm = static_cast<std::string>(element->getAttribute("norm"));
  if (element->hasAttribute("theta_flip")) theta_flip = static_cast<int>(element->getAttribute("theta_flip"));
  if (element->hasAttribute("draw_edges")) draw_edges = static_cast<int>(element->getAttribute("draw_edges"));
  if (element->hasAttribute("line_color_ind")) edge_color = static_cast<int>(element->getAttribute("line_color_ind"));
  if (element->hasAttribute("fill_color_ind")) face_color = static_cast<int>(element->getAttribute("fill_color_ind"));

  if (element->hasAttribute("bin_edges"))
    {
      auto bin_edges_key = static_cast<std::string>(element->getAttribute("bin_edges"));
      bin_edges = GRM::get<std::vector<double>>((*context)[bin_edges_key]);
      num_bin_edges = static_cast<int>(bin_edges.size());
    }

  num_bins = static_cast<int>(element->parentElement()->getAttribute("num_bins"));
  if (element->parentElement()->hasAttribute("bin_widths"))
    {
      auto bin_widths_key = static_cast<std::string>(element->parentElement()->getAttribute("bin_widths"));
      auto bin_widths_vec = GRM::get<std::vector<double>>((*context)[bin_widths_key]);
      num_bins = static_cast<int>(bin_widths_vec.size());
      bin_width = bin_widths_vec[bin_nr];
    }
  r_max = static_cast<double>(plot_parent->querySelectors("central_region")->getAttribute("r_max"));
  if (r_log) r_max = log10(r_max);

  // no ylims -> max = y_lim_max; with ylims -> max = max_count of series
  if (plot_parent->hasAttribute("r_lim_min") && plot_parent->hasAttribute("r_lim_max"))
    {
      r_lim = true;
      r_lim_min = static_cast<double>(plot_parent->getAttribute("r_lim_min"));
      r_lim_max = static_cast<double>(plot_parent->getAttribute("r_lim_max"));

      if (r_log)
        {
          r_lim_min = log10(r_lim_min);
          r_lim_max = log10(r_lim_max);
        }

      if (plot_parent->hasAttribute("keep_radii_axes"))
        keep_radii_axes = static_cast<int>(plot_parent->getAttribute("keep_radii_axes"));
    }
  else
    {
      r_lim_min = 0.0;
      r_lim_max = r_max;
    }

  if (!keep_radii_axes) count = grm_max(grm_min(count, r_lim_max) - r_lim_min, 0.0); // trim count to [0.0, y_lim_max]

  /* perform calculations for later usages, this r is used for complex calculations */
  if (keep_radii_axes)
    r = grm_min(pow((count / r_max), num_bins * 2), pow(r_lim_max / r_max, num_bins * 2));
  else
    r = pow((count / (r_lim_max - r_lim_min)), num_bins * 2);

  complex1 = moivre(r, 2 * bin_nr, (int)num_bins * 2);

  // draw_arc rectangle
  rect = sqrt(pow(real(complex1), 2) + pow(imag(complex1), 2));

  if (r_lim)
    {
      // this r is used directly for the radii of each draw_arc
      if (keep_radii_axes)
        {
          r = grm_min(count / r_max, r_lim_max / r_max);
        }
      else
        {
          r = count / (r_lim_max - r_lim_min);
          if (r > r_lim_max) r = 1.0;
        }
    }

  // if keep_radii_axes is given, then arcs can not be easily drawn (because of the lower arc [donut shaped]), so
  // additional calculations are needed for arcs and lines
  if (!keep_radii_axes)
    {
      std::shared_ptr<GRM::Element> arc, draw_arc;
      double start_angle, end_angle;

      if (num_bin_edges != 0.0)
        {
          start_angle = bin_edges[0] * convert;
          end_angle = bin_edges[1] * convert;
        }
      else
        {
          start_angle = bin_nr * (360.0 / num_bins);
          end_angle = (bin_nr + 1) * (360.0 / num_bins);
        }
      if (!draw_edges)
        {
          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              arc = global_creator->createFillArc(-rect, rect, -rect, rect, start_angle, end_angle);
              arc->setAttribute("_child_id", child_id++);
              element->append(arc);
            }
          else
            {
              arc = element->querySelectors("fill_arc[_child_id=" + std::to_string(child_id++) + "]");
              if (arc != nullptr)
                global_creator->createFillArc(-rect, rect, -rect, rect, start_angle, end_angle, 0, 0, -1, arc);
            }

          if (arc != nullptr)
            {
              if (!arc->hasAttribute("_fill_int_style_set_by_user"))
                {
                  auto fill_int_style = 1;
                  if (element->parentElement()->hasAttribute("fill_int_style"))
                    fill_int_style = static_cast<int>(element->parentElement()->getAttribute("fill_int_style"));
                  if (element->hasAttribute("fill_int_style"))
                    fill_int_style = static_cast<int>(element->getAttribute("fill_int_style"));
                  global_render->setFillIntStyle(arc, fill_int_style);
                }
              if (!arc->hasAttribute("_fill_style_set_by_user"))
                {
                  auto fill_style = 0;
                  if (element->parentElement()->hasAttribute("fill_style"))
                    fill_style = static_cast<int>(element->parentElement()->getAttribute("fill_style"));
                  if (element->hasAttribute("fill_style"))
                    fill_style = static_cast<int>(element->getAttribute("fill_style"));
                  if (fill_style != 0) global_render->setFillStyle(arc, fill_style);
                }
              if (!arc->hasAttribute("_fill_color_ind_set_by_user")) global_render->setFillColorInd(arc, face_color);
            }
        }

      if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
        {
          draw_arc = global_creator->createFillArc(-rect, rect, -rect, rect, start_angle, end_angle);
          draw_arc->setAttribute("_child_id", child_id++);
          element->append(draw_arc);
        }
      else
        {
          draw_arc = element->querySelectors("fill_arc[_child_id=" + std::to_string(child_id++) + "]");
          if (draw_arc != nullptr)
            global_creator->createFillArc(-rect, rect, -rect, rect, start_angle, end_angle, 0, 0, -1, draw_arc);
        }
      if (draw_arc != nullptr)
        {
          if (!draw_arc->hasAttribute("_fill_int_style_set_by_user"))
            global_render->setFillIntStyle(draw_arc, 0); // because it's a draw_arc
          if (!draw_arc->hasAttribute("_fill_color_ind_set_by_user"))
            global_render->setFillColorInd(draw_arc, edge_color);
          if (!draw_arc->hasAttribute("z_index")) draw_arc->setAttribute("z_index", 2);
        }
    }
  else /* keep_radii_axes is given so extra calculations are needed */
    {
      int num_angle;
      double start_angle, end_angle;
      std::shared_ptr<GRM::Element> area;
      auto id = static_cast<int>(global_root->getAttribute("_id"));

      if (count > r_lim_min && keep_radii_axes) // check if original count (count + r_lim_min) is larger than y_lim_min
        {
          global_root->setAttribute("_id", id + 1);
          str = std::to_string(id);

          if (num_bin_edges != 0.0)
            {
              start_angle = bin_edges[0];
              end_angle = bin_edges[1];
            }
          else
            {
              start_angle = bin_nr * (360.0 / num_bins) / convert;
              end_angle = (bin_nr + 1) * (360.0 / num_bins) / convert;
            }

          // determine number of angles for arc approximations
          num_angle = static_cast<int>((end_angle - start_angle) / (0.2 / convert));
          theta_vec.resize(num_angle);
          linSpace(start_angle, end_angle, num_angle, theta_vec);

          /* 4 because of the 4 corner coordinates and 2 * num_angle for the arc approximations, top and bottom */
          f1.resize(4 + 2 * num_angle);
          f2.resize(4 + 2 * num_angle);

          /* line_1_x/y[0] and [1] */
          f1[0] = cos(start_angle) * r_lim_min / r_max;
          f1[1] = rect * cos(start_angle);
          f2[0] = r_lim_min / r_max * sin(start_angle);
          f2[1] = rect * sin(start_angle);

          /* arc_1_x and arc_1_y */
          listComprehension(r, cos, theta_vec, num_angle, 2, f1);
          listComprehension(r, sin, theta_vec, num_angle, 2, f2);

          /* reversed line_2_x/y[0] and [1] */
          f1[2 + num_angle + 1] = cos(end_angle) * r_lim_min / r_max;
          f1[2 + num_angle] = rect * cos(end_angle);
          f2[2 + num_angle + 1] = r_lim_min / r_max * sin(end_angle);
          f2[2 + num_angle] = rect * sin(end_angle);

          /* reversed arc_2_x and arc_2_y */
          listComprehension(keep_radii_axes ? (r_lim_min / r_max) : 0.0, cos, theta_vec, num_angle, 0, arc_2_x);
          listComprehension(keep_radii_axes ? (r_lim_min / r_max) : 0.0, sin, theta_vec, num_angle, 0, arc_2_y);

          for (int i = 0; i < num_angle; i++)
            {
              f1[2 + num_angle + 2 + i] = arc_2_x[num_angle - 1 - i];
              f2[2 + num_angle + 2 + i] = arc_2_y[num_angle - 1 - i];
            }

          if (!draw_edges)
            {
              if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
                {
                  area = global_creator->createFillArea("x" + str, f1, "y" + str, f2);
                  area->setAttribute("_child_id", child_id++);
                  element->append(area);
                }
              else
                {
                  area = element->querySelectors("fill_area[_child_id=" + std::to_string(child_id++) + "]");
                  if (area != nullptr)
                    global_creator->createFillArea("x" + str, f1, "y" + str, f2, nullptr, 0, 0, -1, area);
                }

              if (area != nullptr)
                {
                  if (!area->hasAttribute("_fill_color_ind_set_by_user"))
                    global_render->setFillColorInd(area, face_color);
                  if (!area->hasAttribute("_fill_int_style_set_by_user"))
                    {
                      auto fill_int_style = 1;
                      if (element->hasAttribute("fill_int_style"))
                        fill_int_style = static_cast<int>(element->getAttribute("fill_int_style"));
                      global_render->setFillIntStyle(area, fill_int_style);
                    }
                }
            }

          // draw_area more likely
          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              area = global_creator->createFillArea("x" + str, f1, "y" + str, f2);
              area->setAttribute("_child_id", child_id++);
              element->append(area);
            }
          else
            {
              area = element->querySelectors("fill_area[_child_id=" + std::to_string(child_id++) + "]");
              if (area != nullptr)
                global_creator->createFillArea("x" + str, f1, "y" + str, f2, nullptr, 0, 0, -1, area);
            }
          if (area != nullptr)
            {
              if (!area->hasAttribute("_fill_color_ind_set_by_user")) global_render->setFillColorInd(area, edge_color);
              if (!area->hasAttribute("_fill_int_style_set_by_user"))
                {
                  auto fill_int_style = 0;
                  if (element->hasAttribute("fill_int_style"))
                    fill_int_style = static_cast<int>(element->getAttribute("fill_int_style"));
                  global_render->setFillIntStyle(area, fill_int_style);
                }
              if (!area->hasAttribute("z_index")) area->setAttribute("z_index", 2);
            }
        }
    }
}

void processPieSegment(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  int child_id = 0;
  std::shared_ptr<GRM::Element> arc, text_elem;
  double text_pos[2];
  auto global_creator = grm_get_creator();

  /* clear old child nodes */
  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  auto start_angle = static_cast<double>(element->getAttribute("start_angle"));
  auto end_angle = static_cast<double>(element->getAttribute("end_angle"));
  auto text = static_cast<std::string>(element->getAttribute("text"));

  if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
    {
      arc = global_creator->createFillArc(0.035, 0.965, 0.07, 1.0, start_angle, end_angle);
      arc->setAttribute("_child_id", child_id++);
      element->append(arc);
    }
  else
    {
      arc = element->querySelectors("fill_arc[_child_id=" + std::to_string(child_id++) + "]");
      if (arc != nullptr) global_creator->createFillArc(0.035, 0.965, 0.07, 1.0, start_angle, end_angle, 0, 0, -1, arc);
    }

  auto middle_angle = (start_angle + end_angle) / 2.0;
  text_pos[0] = 0.5 + 0.25 * cos(middle_angle * M_PI / 180.0);
  text_pos[1] = 0.5 + 0.25 * sin(middle_angle * M_PI / 180.0);

  if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
    {
      text_elem = global_creator->createText(text_pos[0], text_pos[1], text, CoordinateSpace::WC);
      text_elem->setAttribute("_child_id", child_id++);
      element->append(text_elem);
    }
  else
    {
      text_elem = element->querySelectors("text[_child_id=" + std::to_string(child_id++) + "]");
      if (text_elem != nullptr)
        global_creator->createText(text_pos[0], text_pos[1], text, CoordinateSpace::WC, text_elem);
    }
  if (text_elem != nullptr)
    {
      text_elem->setAttribute("z_index", 2);
      text_elem->setAttribute("set_text_color_for_background", true);
      processTextColorForBackground(text_elem);
    }
}

void processText(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for text
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  gr_savestate();
  double tbx[4], tby[4];
  int text_color_ind = 1, scientific_format = 0;
  bool text_fits = true;
  auto x = static_cast<double>(element->getAttribute("x"));
  auto y = static_cast<double>(element->getAttribute("y"));
  auto str = static_cast<std::string>(element->getAttribute("text"));
  auto available_width = static_cast<double>(element->getAttribute("width"));
  auto available_height = static_cast<double>(element->getAttribute("height"));
  auto global_render = grm_get_render();
  auto active_figure = global_render->getActiveFigure();
  auto redraw_ws = global_render->getRedrawWs();

  auto world_coordinates = static_cast<CoordinateSpace>(static_cast<int>(element->getAttribute("world_coordinates")));
  if (element->hasAttribute("_world_coordinates_set_by_user"))
    {
      world_coordinates =
          static_cast<CoordinateSpace>(static_cast<int>(element->getAttribute("_world_coordinates_set_by_user")));
      element->setAttribute("world_coordinates",
                            static_cast<int>(element->getAttribute("_world_coordinates_set_by_user")));
    }
  gr_inqtextcolorind(&text_color_ind);
  if (element->parentElement()->parentElement()->hasAttribute("text_color_ind"))
    text_color_ind = static_cast<int>(element->parentElement()->parentElement()->getAttribute("text_color_ind"));
  if (element->parentElement()->hasAttribute("text_color_ind"))
    text_color_ind = static_cast<int>(element->parentElement()->getAttribute("text_color_ind"));
  if (element->hasAttribute("text_color_ind"))
    text_color_ind = static_cast<int>(element->getAttribute("text_color_ind"));
  if (element->hasAttribute("scientific_format"))
    scientific_format = static_cast<int>(element->getAttribute("scientific_format"));

  applyMoveTransformation(element);
  processTextEncoding(active_figure);
  processPrivateTransparency(element);
  if (element->hasAttribute("transparency")) processTransparency(element);

  if (world_coordinates == CoordinateSpace::WC) gr_wctondc(&x, &y);
  if (element->hasAttribute("width") && element->hasAttribute("height"))
    {
      gr_wctondc(&available_width, &available_height);
      gr_inqtext(x, y, &str[0], tbx, tby);
      auto minmax_x = std::minmax_element(std::begin(tbx), std::end(tbx));
      auto minmax_y = std::minmax_element(std::begin(tby), std::end(tby));
      auto width = static_cast<double>((minmax_x.second - minmax_x.first));
      auto height = static_cast<double>((minmax_y.second - minmax_y.first));
      if (width > available_width && height > available_height)
        {
          if (!element->hasAttribute("_char_up_set_by_user")) gr_setcharup(0.0, 1.0);
          gr_settextalign(2, 3);
          gr_inqtext(x, y, &str[0], tbx, tby);
          width = tbx[2] - tbx[0];
          height = tby[2] - tby[0];
          if (width < available_width && height < available_height)
            {
              if (!element->hasAttribute("_char_up_set_by_user")) gr_setcharup(0.0, 1.0);
              gr_settextalign(2, 3);
            }
          else if (height < available_width && width < available_height)
            {
              if (!element->hasAttribute("_char_up_set_by_user")) gr_setcharup(-1.0, 0.0);
              gr_settextalign(2, 3);
            }
          else
            {
              text_fits = false;
            }
        }
    }

  if (element->parentElement()->localName() == "label") processCharHeight(element->parentElement());
  if (element->parentElement()->localName() == "overlay_element") processCharHeight(element);
  if (text_fits && redraw_ws && scientific_format == 2)
    {
      gr_settextcolorind(text_color_ind); // needed to have a visible text after update
      gr_textext(x, y, &str[0]);
    }
  else if (text_fits && redraw_ws && scientific_format == 3)
    {
      gr_settextcolorind(text_color_ind); // needed to have a visible text after update
      gr_mathtex(x, y, &str[0]);
    }
  else if (text_fits && redraw_ws)
    {
      gr_settextcolorind(text_color_ind); // needed to have a visible text after update
      gr_text(x, y, &str[0]);
    }
  gr_restorestate();
}

void processTextRegion(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  double viewport[4], char_height;
  double x = 0.0, y = 0.0;
  std::string location, text;
  bool is_title;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  std::shared_ptr<GRM::Element> plot_parent = element->parentElement(), side_region = element->parentElement(),
                                text_elem;
  getPlotParent(plot_parent);
  auto global_creator = grm_get_creator();
  auto global_render = grm_get_render();

  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  gr_inqcharheight(&char_height);
  calculateViewport(element);
  applyMoveTransformation(element);

  if (!GRM::Render::getViewport(element, &viewport[0], &viewport[1], &viewport[2], &viewport[3]))
    throw NotFoundError(element->localName() + " doesn't have a viewport but it should.\n");
  location = static_cast<std::string>(side_region->getAttribute("location"));
  is_title = side_region->hasAttribute("text_is_title") && static_cast<int>(side_region->getAttribute("text_is_title"));
  text = static_cast<std::string>(side_region->getAttribute("text_content"));

  if (location == "left")
    {
      x = viewport[0] + 0.5 * char_height;
      y = 0.5 * (viewport[2] + viewport[3]);
    }
  else if (location == "right")
    {
      x = viewport[1] - 0.5 * char_height;
      y = 0.5 * (viewport[2] + viewport[3]);
    }
  else if (location == "bottom")
    {
      x = 0.5 * (viewport[0] + viewport[1]);
      y = viewport[2] + 0.5 * char_height;
    }
  else if (location == "top")
    {
      x = 0.5 * (viewport[0] + viewport[1]);
      y = viewport[3];
      if (!is_title) y -= 0.5 * char_height;
    }

  if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT && !text.empty())
    {
      text_elem = global_creator->createText(x, y, text);
      text_elem->setAttribute("_child_id", 0);
      element->appendChild(text_elem);
    }
  else
    {
      if (!text.empty())
        {
          text_elem = element->querySelectors("text[_child_id=\"0\"]");
          if (text_elem) global_creator->createText(x, y, text, CoordinateSpace::NDC, text_elem);
        }
    }
  if (text_elem)
    {
      if (!element->hasAttribute("_text_align_vertical_set_by_user") &&
          !element->hasAttribute("_text_align_horizontal_set_by_user"))
        {
          if (location == "left" || location == "top")
            global_render->setTextAlign(text_elem, GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_TOP);
          else if (location == "right" || location == "bottom")
            global_render->setTextAlign(text_elem, GKS_K_TEXT_HALIGN_CENTER, GKS_K_TEXT_VALIGN_BOTTOM);
        }
      if (location == "top" && is_title)
        text_elem->setAttribute("z_index", 2);
      else
        text_elem->setAttribute("z_index", 0);
      if (!text_elem->hasAttribute("_char_up_set_by_user"))
        {
          if (location == "left" || location == "right")
            global_render->setCharUp(text_elem, -1, 0);
          else
            global_render->setCharUp(text_elem, 0, 1);
        }
    }
}

void processTickGroup(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  int child_id = 0, z_index = 0;
  std::shared_ptr<GRM::Element> tick_elem, text, grid_line;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  auto global_creator = grm_get_creator();

  auto value = static_cast<double>(element->getAttribute("value"));
  auto is_major = static_cast<int>(element->getAttribute("is_major"));
  auto tick_label = static_cast<std::string>(element->getAttribute("tick_label"));
  auto axis_type = static_cast<std::string>(element->parentElement()->getAttribute("axis_type"));
  auto draw_grid = static_cast<int>(element->parentElement()->getAttribute("draw_grid"));
  bool mirrored_axis = element->parentElement()->hasAttribute("mirrored_axis") &&
                       static_cast<int>(element->parentElement()->getAttribute("mirrored_axis"));

  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  // tick
  if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
    {
      tick_elem = global_creator->createTick(is_major, value);
      tick_elem->setAttribute("_child_id", child_id++);
      element->append(tick_elem);
    }
  else
    {
      tick_elem = element->querySelectors("tick[_child_id=" + std::to_string(child_id++) + "]");
      if (tick_elem != nullptr) global_creator->createTick(is_major, value, tick_elem);
    }
  if (tick_elem != nullptr)
    {
      if (!is_major)
        z_index = -8;
      else
        z_index = -4;
      if (axis_type == "y") z_index -= 2;
      if (element->parentElement()->parentElement()->localName() == "colorbar") z_index = 1;
      tick_elem->setAttribute("z_index", z_index);
    }

  // mirrored tick
  if (mirrored_axis)
    {
      if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
        {
          tick_elem = global_creator->createTick(is_major, value);
          tick_elem->setAttribute("_child_id", child_id++);
          element->append(tick_elem);
        }
      else
        {
          tick_elem = element->querySelectors("tick[_child_id=" + std::to_string(child_id++) + "]");
          if (tick_elem != nullptr) global_creator->createTick(is_major, value, tick_elem);
        }
      if (tick_elem != nullptr)
        {
          if (!is_major)
            z_index = -9;
          else
            z_index = -5;
          if (axis_type == "y") z_index -= 2;
          tick_elem->setAttribute("z_index", z_index);
          tick_elem->setAttribute("is_mirrored", true);
        }
    }

  // grid
  if (draw_grid)
    {
      if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
        {
          grid_line = global_creator->createGridLine(is_major, value);
          grid_line->setAttribute("_child_id", child_id++);
          element->append(grid_line);
        }
      else
        {
          grid_line = element->querySelectors("grid_line[_child_id=" + std::to_string(child_id++) + "]");
          if (grid_line != nullptr) global_creator->createGridLine(is_major, value, grid_line);
        }
      if (grid_line != nullptr)
        {
          if (!is_major)
            z_index = -14;
          else
            z_index = -12;
          if (axis_type == "y") z_index -= 1;
          grid_line->setAttribute("z_index", z_index);
        }
    }

  tickLabelAdjustment(element, child_id, del);
  applyTickModificationMap(element, context, child_id, del);
}

void processTick(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  int mask = 0;
  std::shared_ptr<GRM::Element> axis_elem = element->parentElement()->parentElement();
  std::shared_ptr<GRM::Element> plot_parent = element;
  getPlotParent(plot_parent);

  auto coordinate_system = plot_parent->querySelectors("coordinate_system");
  bool hide =
      (coordinate_system->hasAttribute("hide")) ? static_cast<int>(coordinate_system->getAttribute("hide")) : false;
  auto coordinate_system_type = static_cast<std::string>(coordinate_system->getAttribute("plot_type"));
  auto axis_type = static_cast<std::string>(axis_elem->getAttribute("axis_type"));
  auto min_val = static_cast<double>(axis_elem->getAttribute("min_value"));
  auto max_val = static_cast<double>(axis_elem->getAttribute("max_value"));
  auto org = static_cast<double>(axis_elem->getAttribute("origin"));
  auto pos = static_cast<double>(axis_elem->getAttribute("pos"));
  auto tick = static_cast<double>(axis_elem->getAttribute("tick"));
  auto major_count = static_cast<int>(axis_elem->getAttribute("major_count"));
  auto tick_size = static_cast<double>(axis_elem->getAttribute("tick_size"));
  if (element->parentElement()->hasAttribute("tick_size"))
    tick_size = static_cast<double>(element->parentElement()->getAttribute("tick_size"));
  auto tick_orientation = static_cast<int>(axis_elem->getAttribute("tick_orientation"));
  auto value = static_cast<double>(element->getAttribute("value"));
  if (element->hasAttribute("_value_set_by_user"))
    {
      value = static_cast<double>(element->getAttribute("_value_set_by_user"));
      element->setAttribute("value", value);
    }
  auto is_major = static_cast<int>(element->getAttribute("is_major"));
  auto label_pos = static_cast<double>(axis_elem->getAttribute("label_pos"));
  bool mirrored_axis = element->hasAttribute("is_mirrored") && static_cast<int>(element->getAttribute("is_mirrored"));
  auto location = static_cast<std::string>(element->parentElement()->parentElement()->getAttribute("location"));

  adjustValueForNonStandardAxis(plot_parent, &value, location);

  tick_t t = {value, is_major};
  axis_t drawn_tick = {min_val, max_val, tick,      org,   pos, major_count, 1, &t, tick_size * tick_orientation,
                       0,       nullptr, label_pos, false, 0};
  if (grm_get_render()->getRedrawWs() && !hide &&
      (coordinate_system_type == "2d" || axis_elem->parentElement()->localName() == "colorbar"))
    {
      mask = mirrored_axis ? 2 : 1;
      if (axis_type == "x")
        {
          gr_drawaxes(&drawn_tick, nullptr, mask);
        }
      else
        {
          gr_drawaxes(nullptr, &drawn_tick, mask);
        }
    }
}

void processTitles3d(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for titles 3d
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  std::string xlabel, ylabel, zlabel;
  auto coordinate_system = element->parentElement();
  bool hide =
      (coordinate_system->hasAttribute("hide")) ? static_cast<int>(coordinate_system->getAttribute("hide")) : false;
  auto coordinate_system_type = static_cast<std::string>(coordinate_system->getAttribute("plot_type"));
  xlabel = static_cast<std::string>(element->getAttribute("x_label_3d"));
  ylabel = static_cast<std::string>(element->getAttribute("y_label_3d"));
  zlabel = static_cast<std::string>(element->getAttribute("z_label_3d"));
  applyMoveTransformation(element);
  if (grm_get_render()->getRedrawWs() && !hide && coordinate_system_type == "3d")
    {
      auto scientific_format = static_cast<int>(element->getAttribute("scientific_format"));
      gr_setscientificformat(scientific_format);

      gr_setclip(0); // disable clipping for 3d labels cause they are just texts
      gr_titles3d(xlabel.data(), ylabel.data(), zlabel.data());
      gr_setclip(1); // enable clipping again
    }
}

/* ------------------------------- process series elements -----------------------------------------------------------*/

void processHeatmap(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for heatmap
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  int icmap[256], i, j;
  unsigned int cols, rows, z_length;
  double x_min, x_max, y_min, y_max, z_min, z_max, c_min, c_max, zv;
  bool is_uniform_heatmap;
  std::shared_ptr<GRM::Element> plot_parent;
  std::shared_ptr<GRM::Element> element_context = element;
  std::vector<int> data, rgba;
  std::vector<double> x_vec, y_vec, z_vec;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  int child_id = 0;
  int x_offset = 0, y_offset = 0;
  std::string orientation = PLOT_DEFAULT_ORIENTATION;
  bool is_vertical = false;
  auto global_creator = grm_get_creator();
  auto global_root = grm_get_document_root();

  if (element->parentElement()->parentElement()->localName() == "plot")
    {
      plot_parent = element->parentElement()->parentElement();
    }
  else
    {
      plot_parent = element->parentElement();
      getPlotParent(plot_parent);
      for (const auto &children : plot_parent->children())
        {
          if (children->localName() == "marginal_heatmap_plot")
            {
              element_context = children;
              break;
            }
        }
    }

  if (element->parentElement()->hasAttribute("orientation"))
    orientation = static_cast<std::string>(element->parentElement()->getAttribute("orientation"));
  is_vertical = orientation == "vertical";

  if (element_context->hasAttribute("x"))
    {
      auto x = static_cast<std::string>(element_context->getAttribute("x"));
      x_vec = GRM::get<std::vector<double>>((*context)[x]);
      cols = x_vec.size();

      if (static_cast<int>(plot_parent->getAttribute("x_log")))
        {
          for (i = 0; i < cols; i++)
            {
              if (!grm_isnan(x_vec[i])) break;
              x_vec.erase(x_vec.begin(), x_vec.begin() + 1);
              x_offset += 1;
            }
          cols = x_vec.size();
        }
    }
  if (element_context->hasAttribute("y"))
    {
      auto y = static_cast<std::string>(element_context->getAttribute("y"));
      y_vec = GRM::get<std::vector<double>>((*context)[y]);
      rows = y_vec.size();

      if (static_cast<int>(plot_parent->getAttribute("y_log")))
        {
          for (i = 0; i < rows; i++)
            {
              if (!grm_isnan(y_vec[i])) break;
              y_vec.erase(y_vec.begin(), y_vec.begin() + 1);
              y_offset += 1;
            }
          rows = y_vec.size();
        }
    }

  if (!element_context->hasAttribute("z"))
    throw NotFoundError("Heatmap series is missing required attribute z-data.\n");
  auto z = static_cast<std::string>(element_context->getAttribute("z"));
  z_vec = GRM::get<std::vector<double>>((*context)[z]);
  z_length = z_vec.size();

  if (x_offset > 0 || y_offset > 0)
    {
      std::vector<double> new_z_vec(rows * cols);
      z_length = rows * cols;

      for (i = 0; i < rows; i++)
        {
          for (j = 0; j < cols; j++)
            {
              new_z_vec[j + i * cols] = z_vec[(x_offset + j) + (y_offset + i) * (cols + x_offset)];
            }
        }
      z_vec = new_z_vec;
    }

  if (x_vec.empty() && y_vec.empty())
    {
      /* If neither `x` nor `y` are given, we need more information about the shape of `z` */
      if (!element->hasAttribute("z_dims"))
        throw NotFoundError("Heatmap series is missing required attribute z_dims.\n");
      auto z_dims_key = static_cast<std::string>(element->getAttribute("z_dims"));
      auto z_dims_vec = GRM::get<std::vector<int>>((*context)[z_dims_key]);
      cols = z_dims_vec[0];
      rows = z_dims_vec[1];
    }
  else if (x_vec.empty())
    {
      cols = z_length / rows;
    }
  else if (y_vec.empty())
    {
      rows = z_length / cols;
    }

  is_uniform_heatmap = (x_vec.empty() || isEquidistantArray(cols, &(x_vec[0]))) &&
                       (y_vec.empty() || isEquidistantArray(rows, &(y_vec[0])));
  if (!is_uniform_heatmap && (x_vec.empty() || y_vec.empty()))
    throw NotFoundError("Heatmap series is missing x- or y-data or the data has to be uniform.\n");

  auto id = static_cast<int>(global_root->getAttribute("_id"));
  global_root->setAttribute("_id", id + 1);
  auto str = std::to_string(id);
  if (x_vec.empty())
    {
      std::vector<double> x_vec_tmp;
      x_min = static_cast<double>(element->getAttribute("x_range_min"));
      x_max = static_cast<double>(element->getAttribute("x_range_max"));
      linSpace(x_min, x_max, (int)cols, x_vec_tmp);
      (*context)["x" + str] = x_vec_tmp;
      element->setAttribute("x", "x" + str);
    }
  else
    {
      for (j = 0; j < cols; j++)
        {
          if (!grm_isnan(x_vec[j]))
            {
              x_min = x_vec[j];
              break;
            }
        }
      x_max = x_vec[cols - 1];
    }
  if (y_vec.empty())
    {
      std::vector<double> y_vec_tmp;
      y_min = static_cast<double>(element->getAttribute("y_range_min"));
      y_max = static_cast<double>(element->getAttribute("y_range_max"));
      linSpace(y_min, y_max, (int)rows, y_vec_tmp);
      (*context)["y" + str] = y_vec_tmp;
      element->setAttribute("y", "y" + str);
    }
  else
    {
      for (j = 0; j < rows; j++)
        {
          if (!grm_isnan(y_vec[j]))
            {
              y_min = y_vec[j];
              break;
            }
        }
      y_max = y_vec[rows - 1];
    }

  // swap x and y in case of orientation
  if (is_vertical)
    {
      double tmp = x_min;
      x_min = y_min;
      y_min = tmp;
      tmp = x_max;
      x_max = y_max;
      y_max = tmp;
      tmp = rows;
      rows = cols;
      cols = tmp;
      auto tmp2 = x_vec;
      x_vec = y_vec;
      y_vec = tmp2;
    }

  if (element_context->hasAttribute("marginal_heatmap_kind"))
    {
      z_min = static_cast<double>(element_context->getAttribute("z_range_min"));
      z_max = static_cast<double>(element_context->getAttribute("z_range_max"));
    }
  else
    {
      z_min = static_cast<double>(element->getAttribute("z_range_min"));
      z_max = static_cast<double>(element->getAttribute("z_range_max"));
    }
  if (!element->hasAttribute("c_range_min") || !element->hasAttribute("c_range_max"))
    {
      c_min = z_min;
      c_max = z_max;
    }
  else
    {
      c_min = static_cast<double>(element->getAttribute("c_range_min"));
      c_max = static_cast<double>(element->getAttribute("c_range_max"));
    }

  if (!is_uniform_heatmap)
    {
      --cols;
      --rows;
    }
  for (i = 0; i < 256; i++) gr_inqcolor(1000 + i, icmap + i);

  data = std::vector<int>(rows * cols);
  if (z_max > z_min)
    {
      for (i = 0; i < rows; i++)
        {
          for (j = 0; j < cols; j++)
            {
              if (is_vertical)
                zv = z_vec[i + j * rows];
              else
                zv = z_vec[j + i * cols];

              if (zv > z_max || zv < z_min || grm_isnan(zv))
                {
                  data[j + i * cols] = -1;
                }
              else
                {
                  data[j + i * cols] = (int)((zv - c_min) / (c_max - c_min) * 255 + 0.5);
                  data[j + i * cols] =
                      grm_max(grm_min(data[j + i * cols], 255), 0); // data values must be inside [0, 255]
                }
            }
        }
    }
  else
    {
      for (i = 0; i < cols * rows; i++) data[i] = 0;
    }
  rgba = std::vector<int>(rows * cols);

  /* clear old heatmaps */
  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  if (is_uniform_heatmap)
    {
      for (i = 0; i < rows * cols; i++)
        {
          rgba[i] = data[i] != -1 ? (255 << 24) + icmap[data[i]] : 0;
        }

      std::shared_ptr<GRM::Element> cell_array;
      if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
        {
          cell_array =
              global_creator->createDrawImage(x_min, y_min, x_max, y_max, (int)cols, (int)rows, "rgba" + str, rgba, 0);
          cell_array->setAttribute("_child_id", child_id++);
          element->append(cell_array);
        }
      else
        {
          cell_array = element->querySelectors("draw_image[_child_id=" + std::to_string(child_id++) + "]");
          if (cell_array != nullptr)
            global_creator->createDrawImage(x_min, y_min, x_max, y_max, (int)cols, (int)rows, "rgba" + str, rgba, 0,
                                            nullptr, cell_array);
        }
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

      std::shared_ptr<GRM::Element> cell_array;
      if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
        {
          cell_array =
              global_creator->createNonUniformCellArray("x" + str, x_vec, "y" + str, y_vec, (int)cols, (int)rows, 1, 1,
                                                        (int)cols, (int)rows, "color_ind_values" + str, rgba);
          cell_array->setAttribute("_child_id", child_id++);
          element->append(cell_array);
        }
      else
        {
          cell_array = element->querySelectors("nonuniform_cell_array[_child_id=" + std::to_string(child_id++) + "]");
          if (cell_array != nullptr)
            global_creator->createNonUniformCellArray("x" + str, x_vec, "y" + str, y_vec, (int)cols, (int)rows, 1, 1,
                                                      (int)cols, (int)rows, "color_ind_values" + str, rgba, nullptr,
                                                      cell_array);
        }
    }
  processClipRegion(element->parentElement());
  processColormap(element->parentElement()->parentElement());
}

static void hexbin(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for hexbin
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));
  auto nbins = static_cast<int>(element->getAttribute("num_bins"));

  double *x_p = &(GRM::get<std::vector<double>>((*context)[x])[0]);
  double *y_p = &(GRM::get<std::vector<double>>((*context)[y])[0]);

  auto x_vec = GRM::get<std::vector<double>>((*context)[x]);
  auto y_vec = GRM::get<std::vector<double>>((*context)[y]);
  auto x_length = (int)x_vec.size();

  auto redraw_ws = grm_get_render()->getRedrawWs();

  if (element->hasAttribute("_hexbin_context_address"))
    {
      auto address = static_cast<std::string>(element->getAttribute("_hexbin_context_address"));
      long hex_address = stol(address, nullptr, 16);
      const hexbin_2pass_t *hexbin_context = (hexbin_2pass_t *)hex_address;
      bool cleanup = hexbin_context->action & GR_2PASS_CLEANUP;
      if (redraw_ws) gr_hexbin_2pass(x_length, x_p, y_p, nbins, hexbin_context);
      if (cleanup) element->removeAttribute("_hexbin_context_address");
    }
  else
    {
      applyMoveTransformation(element);
      if (redraw_ws) gr_hexbin(x_length, x_p, y_p, nbins);
    }
}

void processHexbin(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  int nbins = PLOT_DEFAULT_HEXBIN_NBINS;
  std::string orientation = PLOT_DEFAULT_ORIENTATION;

  if (!element->hasAttribute("x")) throw NotFoundError("Hexbin series is missing required attribute x-data.\n");
  auto x = static_cast<std::string>(element->getAttribute("x"));
  if (!element->hasAttribute("y")) throw NotFoundError("Hexbin series is missing required attribute y-data.\n");
  auto y = static_cast<std::string>(element->getAttribute("y"));
  if (element->hasAttribute("num_bins"))
    {
      nbins = static_cast<int>(element->getAttribute("num_bins"));
    }
  else
    {
      element->setAttribute("num_bins", nbins);
    }
  if (element->parentElement()->hasAttribute("orientation"))
    orientation = static_cast<std::string>(element->parentElement()->getAttribute("orientation"));

  double *x_p = &(GRM::get<std::vector<double>>((*context)[x])[0]);
  double *y_p = &(GRM::get<std::vector<double>>((*context)[y])[0]);

  auto x_vec = GRM::get<std::vector<double>>((*context)[x]);
  auto y_vec = GRM::get<std::vector<double>>((*context)[y]);
  auto x_length = static_cast<int>(x_vec.size());
  auto y_length = static_cast<int>(y_vec.size());
  if (x_length != y_length) throw std::length_error("For Hexbin x- and y-data must have the same size.\n");

  if (orientation == "vertical")
    {
      auto tmp = x_p;
      x_p = y_p;
      y_p = tmp;
    }
  const hexbin_2pass_t *hexbin_context = gr_hexbin_2pass(x_length, x_p, y_p, nbins, nullptr);
  double c_min = 0.0, c_max = hexbin_context->cntmax;
  std::ostringstream get_address;
  get_address << hexbin_context;
  element->setAttribute("_hexbin_context_address", get_address.str());

  auto plot_parent = element->parentElement();
  getPlotParent(plot_parent);
  plot_parent->setAttribute("_c_lim_min", c_min);
  plot_parent->setAttribute("_c_lim_max", c_max);
  if (grm_get_render()->getRedrawWs())
    {
      GRM::PushDrawableToZQueue push_hexbin_to_z_queue(hexbin);
      push_hexbin_to_z_queue(element, context);
    }
}

void processBarplot(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for barplot
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  /* plot level */
  int bar_color = 989, edge_color = 1;
  std::vector<double> bar_color_rgb = {-1, -1, -1};
  std::vector<double> edge_color_rgb = {-1, -1, -1};
  double bar_width = 0.8, edge_width = 1.0, bar_shift = 1;
  std::string style = "default", orientation = PLOT_DEFAULT_ORIENTATION, line_spec = SERIES_DEFAULT_SPEC;
  double wfac;
  int len_std_colors = 20;
  int std_colors[20] = {989, 982, 980, 981, 996, 983, 995, 988, 986, 990,
                        991, 984, 992, 993, 994, 987, 985, 997, 998, 999};
  int color_save_spot = PLOT_CUSTOM_COLOR_INDEX;
  unsigned int i;

  /* series level */
  unsigned int y_length, c_length, c_rgb_length;
  std::vector<int> c;
  std::vector<double> c_rgb;
  std::vector<std::string> ylabels;
  unsigned int ylabels_left = 0, ylabels_length = 0;
  /* style variance */
  double pos_vertical_change = 0, neg_vertical_change = 0;
  double x1, x2, y1, y2;
  double x_min = 0, x_max, y_min = 0;
  bool is_vertical, x_log = false;
  bool inner_series, inner_c = false, inner_c_rgb = false;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  int child_id = 0;
  double eps = 1e-12;

  auto global_render = grm_get_render();
  auto global_root = grm_get_document_root();
  auto global_creator = grm_get_creator();

  /* clear old bars */
  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  /* retrieve attributes from the plot level */
  auto plot_parent = element->parentElement();
  getPlotParent(plot_parent);
  auto series_index = static_cast<int>(element->getAttribute("series_index"));
  auto fixed_y_length = static_cast<int>(plot_parent->getAttribute("max_y_length"));

  // Todo: using line_spec here isn't really clean, cause no lines are drawn, but it's the only option atm to get the
  // same different colors like multiple line series have
  const char *spec_char = line_spec.c_str();
  gr_uselinespec((char *)spec_char);
  gr_inqmarkercolorind(&bar_color);

  if (element->hasAttribute("fill_color_rgb"))
    {
      auto bar_color_rgb_key = static_cast<std::string>(element->getAttribute("fill_color_rgb"));
      bar_color_rgb = GRM::get<std::vector<double>>((*context)[bar_color_rgb_key]);
    }
  if (element->hasAttribute("bar_width")) bar_width = static_cast<double>(element->getAttribute("bar_width"));
  if (element->hasAttribute("style"))
    {
      style = static_cast<std::string>(element->getAttribute("style"));
    }
  else
    {
      element->setAttribute("style", style);
    }
  if (element->parentElement()->hasAttribute("orientation"))
    orientation = static_cast<std::string>(element->parentElement()->getAttribute("orientation"));
  is_vertical = orientation == "vertical";
  x_log = plot_parent->hasAttribute("x_log") && static_cast<int>(plot_parent->getAttribute("x_log"));

  if (bar_color_rgb[0] != -1)
    {
      for (i = 0; i < 3; i++)
        {
          if (bar_color_rgb[i] > 1 || bar_color_rgb[i] < 0)
            throw std::out_of_range("For barplot series bar_color_rgb must be inside [0, 1].\n");
        }
    }

  /* retrieve attributes form the series level */
  if (!element->hasAttribute("y")) throw NotFoundError("Barplot series is missing y.\n");

  auto y_key = static_cast<std::string>(element->getAttribute("y"));
  auto y_vec = GRM::get<std::vector<double>>((*context)[y_key]);
  y_length = size(y_vec);

  if (!element->hasAttribute("indices")) throw NotFoundError("Barplot series is missing indices\n");
  auto indices = static_cast<std::string>(element->getAttribute("indices"));
  auto indices_vec = GRM::get<std::vector<int>>((*context)[indices]);

  inner_series = size(indices_vec) != y_length;

  wfac = 0.9 * bar_width;

  if (element->hasAttribute("line_color_rgb"))
    {
      auto edge_color_rgb_key = static_cast<std::string>(element->getAttribute("line_color_rgb"));
      edge_color_rgb = GRM::get<std::vector<double>>((*context)[edge_color_rgb_key]);
    }
  if (element->hasAttribute("line_color_ind")) edge_color = static_cast<int>(element->getAttribute("line_color_ind"));
  if (element->hasAttribute("edge_width")) edge_width = static_cast<double>(element->getAttribute("edge_width"));
  if (!element->hasAttribute("_text_align_vertical_set_by_user") &&
      !element->hasAttribute("_text_align_horizontal_set_by_user"))
    global_render->setTextAlign(element, 2, 3);

  if (edge_color_rgb[0] != -1)
    {
      for (i = 0; i < 3; i++)
        {
          if (edge_color_rgb[i] > 1 || edge_color_rgb[i] < 0)
            throw std::out_of_range("For barplot series edge_color_rgb must be inside [0, 1].\n");
        }
    }

  if (element->hasAttribute("color_ind_values"))
    {
      auto c_key = static_cast<std::string>(element->getAttribute("color_ind_values"));
      c = GRM::get<std::vector<int>>((*context)[c_key]);
      c_length = size(c);
    }
  if (element->hasAttribute("color_rgb_values"))
    {
      auto c_rgb_key = static_cast<std::string>(element->getAttribute("color_rgb_values"));
      c_rgb = GRM::get<std::vector<double>>((*context)[c_rgb_key]);
      c_rgb_length = size(c_rgb);
    }
  if (element->hasAttribute("y_labels"))
    {
      auto ylabels_key = static_cast<std::string>(element->getAttribute("y_labels"));
      ylabels = GRM::get<std::vector<std::string>>((*context)[ylabels_key]);
      ylabels_length = size(ylabels);

      ylabels_left = ylabels_length;
    }

  if (element->hasAttribute("x_range_min") && element->hasAttribute("x_range_max"))
    {
      x_min = static_cast<double>(element->getAttribute("x_range_min"));
      x_max = static_cast<double>(element->getAttribute("x_range_max"));
      if (!element->hasAttribute("bar_width")) bar_width = (x_max - x_min) / (y_length - 1.0);
      bar_shift = (x_max - x_min) / (y_length - 1.0);
      x_min -= 1; // in the later calculation there is always a +1 in combination with x
      wfac = 0.9 * (x_max - x_min) / (y_length - 1.0);
    }
  if (style != "stacked" && element->hasAttribute("y_range_min"))
    y_min = static_cast<double>(element->getAttribute("y_range_min"));
  if (auto coordinate_system = element->parentElement()->querySelectors("coordinate_system");
      coordinate_system != nullptr && coordinate_system->hasAttribute("y_line"))
    {
      if (auto y_line = coordinate_system->querySelectors("polyline[name=\"y_line\"]"); y_line != nullptr)
        {
          y_min = static_cast<double>(y_line->getAttribute(orientation == "horizontal" ? "y1" : "x1"));
        }
    }

  if (style != "lined" && inner_series) throw TypeError("Unsupported operation for barplot series.\n");
  if (!c.empty())
    {
      if (!inner_series && (c_length < y_length))
        throw std::length_error("For a barplot series c_length must be >= y_length.\n");
      if (inner_series)
        {
          if (c_length == y_length)
            {
              inner_c = true;
            }
          else if (c_length != size(indices_vec))
            {
              throw std::length_error("For a barplot series c_length must be >= y_length.\n");
            }
        }
    }
  if (!c_rgb.empty())
    {
      if (!inner_series && (c_rgb_length < y_length * 3))
        throw std::length_error("For a barplot series c_rgb_length must be >= y_length * 3.\n");
      if (inner_series)
        {
          if (c_rgb_length == y_length * 3)
            {
              inner_c_rgb = true;
            }
          else if (c_rgb_length != size(indices_vec) * 3)
            {
              throw std::length_error("For a barplot series c_rgb_length must be >= y_length * 3\n");
            }
        }
      for (i = 0; i < y_length * 3; i++)
        {
          if ((c_rgb[i] > 1 || c_rgb[i] < 0) && c_rgb[i] != -1)
            throw std::out_of_range("For barplot series c_rgb must be inside [0, 1] or -1.\n");
        }
    }

  if (!element->hasAttribute("_fill_int_style_set_by_user")) global_render->setFillIntStyle(element, 1);
  processFillIntStyle(element);
  if (!element->hasAttribute("fill_color_ind"))
    {
      global_render->setFillColorInd(element, bar_color);
    }
  else
    {
      bar_color = static_cast<int>(element->getAttribute("fill_color_ind"));
    }
  processFillColorInd(element);

  /* overrides bar_color */
  if (bar_color_rgb[0] != -1)
    {
      global_render->setColorRep(element, color_save_spot, bar_color_rgb[0], bar_color_rgb[1], bar_color_rgb[2]);
      processColorReps(element);
      bar_color = color_save_spot;
      global_render->setFillColorInd(element, bar_color);
      processFillColorInd(element);
    }
  if (!inner_series)
    {
      /* draw bar */
      for (i = 0; i < y_length; i++)
        {
          y1 = y_min;
          y2 = y_vec[i];

          if (style == "default")
            {
              x1 = i * bar_shift + 1 - 0.5 * bar_width;
              x2 = i * bar_shift + 1 + 0.5 * bar_width;
            }
          else if (style == "stacked")
            {
              x1 = series_index + 1 - 0.5 * bar_width;
              x2 = series_index + 1 + 0.5 * bar_width;
              if (y_vec[i] > 0)
                {
                  y1 = ((i == 0) ? y_min : 0) + pos_vertical_change;
                  pos_vertical_change += y_vec[i] - ((i > 0) ? y_min : 0);
                  y2 = pos_vertical_change;
                }
              else
                {
                  y1 = ((i == 0) ? y_min : 0) + neg_vertical_change;
                  neg_vertical_change += y_vec[i] - ((i > 0) ? y_min : 0);
                  y2 = neg_vertical_change;
                }
            }
          else if (style == "lined")
            {
              bar_width = wfac / y_length;
              x1 = series_index + 1 - 0.5 * wfac + bar_width * i;
              x2 = series_index + 1 - 0.5 * wfac + bar_width + bar_width * i;
            }
          x1 += x_min;
          x2 += x_min;
          if (x_log && x1 <= 0) x1 = 0 + eps;
          if (x_log && x2 <= x1) continue;

          if (is_vertical)
            {
              double tmp1 = x1, tmp2 = x2;
              x1 = y1, x2 = y2;
              y1 = tmp1, y2 = tmp2;
            }

          int fillcolorind = -1;
          std::vector<double> bar_fillcolor_rgb, edge_fillcolor_rgb;
          std::string bar_color_rgb_key, edge_color_rgb_key;
          std::shared_ptr<GRM::Element> bar;
          auto id = static_cast<int>(global_root->getAttribute("_id"));
          auto str = std::to_string(id);

          /* attributes for fill_rect */
          if (style != "default") fillcolorind = std_colors[i % len_std_colors];
          if (!c.empty() && c[i] != -1)
            {
              fillcolorind = c[i];
            }
          else if (!c_rgb.empty() && c_rgb[i * 3] != -1)
            {
              bar_fillcolor_rgb = std::vector<double>{c_rgb[i * 3], c_rgb[i * 3 + 1], c_rgb[i * 3 + 2]};
              bar_color_rgb_key = "fill_color_rgb" + str;
              (*context)[bar_color_rgb_key] = bar_fillcolor_rgb;
            }

          if (fillcolorind == -1)
            fillcolorind = element->hasAttribute("fill_color_ind")
                               ? static_cast<int>(element->getAttribute("fill_color_ind"))
                               : 989;
          if (element->hasAttribute("_fill_color_ind_set_by_user"))
            fillcolorind = static_cast<int>(element->getAttribute("fill_color_ind"));

          /* Colorrep for draw_rect */
          if (edge_color_rgb[0] != -1)
            {
              edge_fillcolor_rgb = std::vector<double>{edge_color_rgb[0], edge_color_rgb[1], edge_color_rgb[2]};
              edge_color_rgb_key = "line_color_rgb" + str;
              (*context)[edge_color_rgb_key] = edge_fillcolor_rgb;
            }

          /* Create bars */
          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              bar = global_creator->createBar(x1, x2, y1, y2, fillcolorind, edge_color, bar_color_rgb_key,
                                              edge_color_rgb_key, edge_width, "");
              bar->setAttribute("_child_id", child_id++);
              element->append(bar);
            }
          else
            {
              bar = element->querySelectors("bar[_child_id=" + std::to_string(child_id++) + "]");
              if (bar != nullptr)
                global_creator->createBar(x1, x2, y1, y2, fillcolorind, edge_color, bar_color_rgb_key,
                                          edge_color_rgb_key, edge_width, "", bar);
            }
          if (bar != nullptr)
            {
              if (element->hasAttribute("_fill_int_style_set_by_user"))
                bar->setAttribute("fill_int_style",
                                  static_cast<int>(element->getAttribute("_fill_int_style_set_by_user")));

              if (element->hasAttribute("_fill_style_set_by_user"))
                bar->setAttribute("fill_style", static_cast<int>(element->getAttribute("_fill_style_set_by_user")));
            }

          /* Draw y-notations */
          if (!ylabels.empty() && ylabels_left > 0)
            {
              if (bar != nullptr) bar->setAttribute("text", ylabels[i]);
              --ylabels_left;
            }
          global_root->setAttribute("_id", ++id);
        }
    }
  else
    {
      /* edge has the same with and color for every inner series */
      global_render->setLineWidth(element, edge_width);
      if (edge_color_rgb[0] != -1)
        {
          global_render->setColorRep(element, color_save_spot, edge_color_rgb[0], edge_color_rgb[1], edge_color_rgb[2]);
          processFillColorInd(element);
          edge_color = color_save_spot;
        }
      global_render->setLineColorInd(element, edge_color);
      element->setAttribute("line_color_ind", edge_color);
      processLineWidth(element);
      processLineColorInd(element);

      int inner_y_start_index = 0;
      /* Draw inner_series */
      for (int inner_series_index = 0; inner_series_index < size(indices_vec); inner_series_index++)
        {
          /* Draw bars from inner_series */
          int inner_y_length = indices_vec[inner_series_index];
          std::vector<double> inner_y_vec(y_vec.begin() + inner_y_start_index,
                                          y_vec.begin() + inner_y_start_index + inner_y_length);
          bar_width = wfac / fixed_y_length;

          for (i = 0; i < inner_y_length; i++)
            {
              x1 = series_index + 1 - 0.5 * wfac + bar_width * inner_series_index;
              x2 = series_index + 1 - 0.5 * wfac + bar_width + bar_width * inner_series_index;
              if (inner_y_vec[i] > 0)
                {
                  y1 = ((i == 0) ? y_min : 0) + pos_vertical_change;
                  pos_vertical_change += inner_y_vec[i] - ((i > 0) ? y_min : 0);
                  y2 = pos_vertical_change;
                }
              else
                {
                  y1 = ((i == 0) ? y_min : 0) + neg_vertical_change;
                  neg_vertical_change += inner_y_vec[i] - ((i > 0) ? y_min : 0);
                  y2 = neg_vertical_change;
                }
              x1 += x_min;
              x2 += x_min;
              if (x_log && x1 <= 0) x1 = 0 + eps;
              if (x_log && x2 <= x1) continue;

              if (is_vertical)
                {
                  double tmp1 = x1, tmp2 = x2;
                  x1 = y1, x2 = y2;
                  y1 = tmp1, y2 = tmp2;
                }

              int fillcolorind = -1;
              std::vector<double> bar_fillcolor_rgb, edge_fillcolor_rgb;
              std::string bar_color_rgb_key, edge_color_rgb_key;
              std::shared_ptr<GRM::Element> bar;
              auto id = static_cast<int>(global_root->getAttribute("_id"));
              auto str = std::to_string(id);

              /* attributes for fill_rect */
              if (!c.empty() && !inner_c && c[inner_series_index] != -1) fillcolorind = c[inner_series_index];
              if (!c_rgb.empty() && !inner_c_rgb && c_rgb[inner_series_index * 3] != -1)
                {
                  bar_fillcolor_rgb =
                      std::vector<double>{c_rgb[inner_series_index * 3], c_rgb[inner_series_index * 3 + 1],
                                          c_rgb[inner_series_index * 3 + 2]};
                  bar_color_rgb_key = "fill_color_rgb" + str;
                  (*context)[bar_color_rgb_key] = bar_fillcolor_rgb;
                }
              if (inner_c && c[inner_y_start_index + i] != -1) fillcolorind = c[inner_y_start_index + i];
              if (inner_c_rgb && c_rgb[(inner_y_start_index + i) * 3] != -1)
                {
                  bar_fillcolor_rgb = std::vector<double>{c_rgb[(inner_y_start_index + i) * 3],
                                                          c_rgb[(inner_y_start_index + i) * 3 + 1],
                                                          c_rgb[(inner_y_start_index + i) * 3 + 2]};
                  bar_color_rgb_key = "fill_color_rgb" + str;
                  (*context)[bar_color_rgb_key] = bar_fillcolor_rgb;
                }
              if (fillcolorind == -1) fillcolorind = std_colors[inner_series_index % len_std_colors];
              if (element->hasAttribute("_fill_color_ind_set_by_user"))
                fillcolorind = static_cast<int>(element->getAttribute("fill_color_ind"));

              /* Create bars */
              if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
                {
                  bar = global_creator->createBar(x1, x2, y1, y2, fillcolorind, edge_color, bar_color_rgb_key,
                                                  edge_color_rgb_key, edge_width, "");
                  bar->setAttribute("_child_id", child_id++);
                  element->append(bar);
                }
              else
                {
                  bar = element->querySelectors("bar[_child_id=" + std::to_string(child_id++) + "]");
                  if (bar != nullptr)
                    global_creator->createBar(x1, x2, y1, y2, fillcolorind, edge_color, bar_color_rgb_key,
                                              edge_color_rgb_key, edge_width, "", bar);
                }
              if (bar != nullptr)
                {
                  if (element->hasAttribute("_fill_int_style_set_by_user"))
                    bar->setAttribute("fill_int_style",
                                      static_cast<int>(element->getAttribute("_fill_int_style_set_by_user")));
                  if (element->hasAttribute("_fill_style_set_by_user"))
                    bar->setAttribute("fill_style", static_cast<int>(element->getAttribute("fill_style_set_by_user")));
                }

              /* Draw y-notations from inner_series */
              if (!ylabels.empty() && ylabels_left > 0)
                {
                  if (bar != nullptr) bar->setAttribute("text", ylabels[ylabels_length - ylabels_left]);
                  --ylabels_left;
                }
              global_root->setAttribute("_id", ++id);
            }
          pos_vertical_change = 0;
          neg_vertical_change = 0;
          y_length = 0;
          inner_y_start_index += inner_y_length;
        }
    }
  element->setAttribute("line_color_ind", edge_color);
  processLineColorInd(element);

  // error_bar handling
  for (const auto &child : element->children())
    {
      if (child->localName() == "error_bars")
        {
          std::vector<double> bar_centers;
          bar_width = wfac / y_length;
          for (i = 0; i < y_length; i++)
            {
              if (style == "default")
                {
                  x1 = x_min + (i * bar_shift) + 1 - 0.5 * bar_width;
                  x2 = x_min + (i * bar_shift) + 1 + 0.5 * bar_width;
                }
              else if (style == "lined")
                {
                  x1 = x_min + series_index + 1 - 0.5 * wfac + bar_width * i;
                  x2 = x_min + series_index + 1 - 0.5 * wfac + bar_width + bar_width * i;
                }
              else
                {
                  x1 = x_min + series_index + 1 - 0.5 * bar_width;
                  x2 = x_min + series_index + 1 + 0.5 * bar_width;
                }
              if (x_log && x1 <= 0) x1 = 0 + eps;
              if (x_log && x2 <= x1) continue;
              bar_centers.push_back((x1 + x2) / 2.0);
            }
          extendErrorBars(child, context, bar_centers, y_vec);
        }
    }
}

void processContour(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for contour
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double z_min, z_max;
  int num_levels = PLOT_DEFAULT_CONTOUR_LEVELS;
  int i, j;
  unsigned int x_length, y_length, z_length;
  std::vector<double> x_vec, y_vec, z_vec;
  std::vector<double> px_vec, py_vec, pz_vec;
  int major_h = PLOT_DEFAULT_CONTOUR_MAJOR_H;
  std::string orientation = PLOT_DEFAULT_ORIENTATION;
  auto plot_parent = element->parentElement();
  getPlotParent(plot_parent);
  auto global_root = grm_get_document_root();
  auto global_render = grm_get_render();

  if (element->parentElement()->hasAttribute("orientation"))
    orientation = static_cast<std::string>(element->parentElement()->getAttribute("orientation"));

  z_min = element->hasAttribute("z_min") ? static_cast<double>(element->getAttribute("z_min"))
                                         : static_cast<double>(plot_parent->getAttribute("_z_lim_min"));
  z_max = element->hasAttribute("z_max") ? static_cast<double>(element->getAttribute("z_max"))
                                         : static_cast<double>(plot_parent->getAttribute("_z_lim_max"));
  if (element->hasAttribute("levels"))
    {
      num_levels = static_cast<int>(element->getAttribute("levels"));
    }
  else
    {
      element->setAttribute("levels", num_levels);
    }
  if (element->hasAttribute("major_h"))
    {
      major_h = static_cast<int>(element->getAttribute("major_h"));
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

      if (orientation == "vertical")
        {
          auto tmp = x_vec;
          x_vec = y_vec;
          y_vec = tmp;
          auto tmp2 = x_length;
          x_length = y_length;
          y_length = tmp2;

          std::vector<double> zv(z_length);
          for (i = 0; i < y_length; i++)
            {
              for (j = 0; j < x_length; j++)
                {
                  zv[j + i * x_length] = z_vec[i + j * y_length];
                }
            }
          z_vec = zv;
        }

      auto id = static_cast<int>(global_root->getAttribute("_id"));
      global_root->setAttribute("_id", id + 1);
      auto str = std::to_string(id);

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

          gr_gridit((int)x_length, x_p, y_p, z_p, PLOT_CONTOUR_GRIDIT_N, PLOT_CONTOUR_GRIDIT_N, gridit_x, gridit_y,
                    gridit_z);
          for (i = 0; i < PLOT_CONTOUR_GRIDIT_N * PLOT_CONTOUR_GRIDIT_N; i++)
            {
              z_min = grm_min(gridit_z[i], z_min);
              z_max = grm_max(gridit_z[i], z_max);
            }
          element->setAttribute("z_min", z_min);
          element->setAttribute("z_max", z_max);

          global_render->setSpace(element->parentElement(), z_min, z_max, 0,
                                  90); // not plot_parent because it should be now on central_region
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

  auto nx = static_cast<int>(px_vec.size());
  auto ny = static_cast<int>(py_vec.size());

  double *px_p = &(px_vec[0]);
  double *py_p = &(py_vec[0]);
  double *h_p = &(h[0]);
  double *pz_p = &(pz_vec[0]);
  applyMoveTransformation(element);

  processColormap(element->parentElement()->parentElement());

  if (global_render->getRedrawWs()) gr_contour(nx, ny, num_levels, px_p, py_p, h_p, pz_p, major_h);
}

void processContourf(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for contourf
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double z_min, z_max;
  int num_levels = PLOT_DEFAULT_CONTOUR_LEVELS;
  int i, j;
  unsigned int x_length, y_length, z_length;
  std::vector<double> x_vec, y_vec, z_vec;
  std::vector<double> px_vec, py_vec, pz_vec;
  int major_h = PLOT_DEFAULT_CONTOURF_MAJOR_H;
  std::string orientation = PLOT_DEFAULT_ORIENTATION;
  auto plot_parent = element->parentElement();
  getPlotParent(plot_parent);
  auto global_root = grm_get_document_root();
  auto global_render = grm_get_render();

  if (element->parentElement()->hasAttribute("orientation"))
    orientation = static_cast<std::string>(element->parentElement()->getAttribute("orientation"));

  z_min = element->hasAttribute("z_min") ? static_cast<double>(element->getAttribute("z_min"))
                                         : static_cast<double>(plot_parent->getAttribute("_z_lim_min"));
  z_max = element->hasAttribute("z_max") ? static_cast<double>(element->getAttribute("z_max"))
                                         : static_cast<double>(plot_parent->getAttribute("_z_lim_max"));
  if (element->hasAttribute("levels"))
    {
      num_levels = static_cast<int>(element->getAttribute("levels"));
    }
  else
    {
      element->setAttribute("levels", num_levels);
    }
  if (element->hasAttribute("major_h"))
    {
      major_h = static_cast<int>(element->getAttribute("major_h"));
    }

  gr_setprojectiontype(0);
  gr_setspace(z_min, z_max, 0, 90);

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

      if (orientation == "vertical")
        {
          auto tmp = x_vec;
          x_vec = y_vec;
          y_vec = tmp;
          auto tmp2 = x_length;
          x_length = y_length;
          y_length = tmp2;

          std::vector<double> zv(z_length);
          for (i = 0; i < y_length; i++)
            {
              for (j = 0; j < x_length; j++)
                {
                  zv[j + i * x_length] = z_vec[i + j * y_length];
                }
            }
          z_vec = zv;
        }

      auto id = static_cast<int>(global_root->getAttribute("_id"));
      global_root->setAttribute("_id", id + 1);
      auto str = std::to_string(id);

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

          gr_gridit(static_cast<int>(x_length), x_p, y_p, z_p, PLOT_CONTOUR_GRIDIT_N, PLOT_CONTOUR_GRIDIT_N, gridit_x,
                    gridit_y, gridit_z);
          for (i = 0; i < PLOT_CONTOUR_GRIDIT_N * PLOT_CONTOUR_GRIDIT_N; i++)
            {
              z_min = grm_min(gridit_z[i], z_min);
              z_max = grm_max(gridit_z[i], z_max);
            }
          element->setAttribute("z_min", z_min);
          element->setAttribute("z_max", z_max);

          if (!element->hasAttribute("_line_color_ind_set_by_user")) global_render->setLineColorInd(element, 989);
          global_render->setSpace(element->parentElement(), z_min, z_max, 0, 90); // central_region
          processSpace(element->parentElement());

          px_vec = std::vector<double>(gridit_x, gridit_x + PLOT_CONTOUR_GRIDIT_N);
          py_vec = std::vector<double>(gridit_y, gridit_y + PLOT_CONTOUR_GRIDIT_N);
          pz_vec = std::vector<double>(gridit_z, gridit_z + PLOT_CONTOUR_GRIDIT_N * PLOT_CONTOUR_GRIDIT_N);
        }
      else
        {
          if (x_length * y_length != z_length)
            throw std::length_error("For contourf series x_length * y_length must be z_length.\n");

          if (!element->hasAttribute("_line_color_ind_set_by_user")) global_render->setLineColorInd(element, 989);

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

  for (i = 0; i < num_levels; ++i) h[i] = z_min + (1.0 * i) / num_levels * (z_max - z_min);

  auto nx = static_cast<int>(px_vec.size());
  auto ny = static_cast<int>(py_vec.size());

  double *px_p = &(px_vec[0]);
  double *py_p = &(py_vec[0]);
  double *h_p = &(h[0]);
  double *pz_p = &(pz_vec[0]);
  applyMoveTransformation(element);

  processColormap(element->parentElement()->parentElement());

  if (global_render->getRedrawWs()) gr_contourf(nx, ny, num_levels, px_p, py_p, h_p, pz_p, major_h);
}

void processIsosurface(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  std::vector<double> z_vec, temp_colors;
  unsigned int i, z_length, dims;
  int strides[3];
  double c_min, c_max, isovalue = 0.5;
  float foreground_colors[3] = {0.0, 0.5, 0.8};
  auto global_root = grm_get_document_root();
  auto global_render = grm_get_render();

  if (!element->hasAttribute("z")) throw NotFoundError("Isosurface series is missing required attribute z-data.\n");
  auto z_key = static_cast<std::string>(element->getAttribute("z"));
  z_vec = GRM::get<std::vector<double>>((*context)[z_key]);
  z_length = z_vec.size();

  if (!element->hasAttribute("z_dims"))
    throw NotFoundError("Isosurface series is missing required attribute z_dims.\n");
  auto z_dims_key = static_cast<std::string>(element->getAttribute("z_dims"));
  auto z_dims_vec = GRM::get<std::vector<int>>((*context)[z_dims_key]);
  dims = z_dims_vec.size();

  if (dims != 3) throw std::length_error("For isosurface series the size of z_dims has to be 3.\n");
  if (z_dims_vec[0] * z_dims_vec[1] * z_dims_vec[2] != z_length)
    throw std::length_error("For isosurface series shape[0] * shape[1] * shape[2] must be c_length.\n");
  if (z_length <= 0) throw NotFoundError("For isosurface series the size of c has to be greater than 0.\n");

  if (element->hasAttribute("isovalue")) isovalue = static_cast<double>(element->getAttribute("isovalue"));
  element->setAttribute("isovalue", isovalue);
  /* We need to convert the double values to floats, as GR3 expects floats, but an argument can only contain doubles. */
  if (element->hasAttribute("color_rgb"))
    {
      auto temp_c = static_cast<std::string>(element->getAttribute("color_rgb"));
      temp_colors = GRM::get<std::vector<double>>((*context)[temp_c]);
      i = temp_colors.size();
      if (i != 3) throw std::length_error("For isosurface series the foreground colors must have size 3.\n");
      while (i-- > 0) foreground_colors[i] = (float)temp_colors[i];
    }
  else
    {
      int id = static_cast<int>(global_root->getAttribute("_id"));
      std::string id_str = std::to_string(id);

      std::vector<double> rgb_vec = {foreground_colors[0], foreground_colors[1], foreground_colors[2]};
      (*context)["color_rgb" + id_str] = rgb_vec;
      element->setAttribute("color_rgb", "color_rgb" + id_str);
    }
  logger((stderr, "Colors; %f %f %f\n", foreground_colors[0], foreground_colors[1], foreground_colors[2]));

  /* Check if any value is finite in array, also calculation of real min and max */
  c_min = c_max = z_vec[0];
  for (i = 0; i < z_length; ++i)
    {
      if (std::isfinite(z_vec[i]))
        {
          if (grm_isnan(c_min) || c_min > z_vec[i]) c_min = z_vec[i];
          if (grm_isnan(c_max) || c_max < z_vec[i]) c_max = z_vec[i];
        }
    }
  if (c_min == c_max || !std::isfinite(c_min) || !std::isfinite(c_max))
    throw NotFoundError("For isosurface series the given c-data isn't enough.\n");

  logger((stderr, "c_min %lf c_max %lf isovalue %lf\n ", c_min, c_max, isovalue));
  std::vector<float> conv_data(z_vec.begin(), z_vec.end());

  strides[0] = z_dims_vec[1] * z_dims_vec[2];
  strides[1] = z_dims_vec[2];
  strides[2] = 1;

  if (!element->hasAttribute("ambient") && !element->hasAttribute("diffuse") && !element->hasAttribute("specular") &&
      !element->hasAttribute("specular_power"))
    global_render->setGR3LightParameters(element, 0.2, 0.8, 0.7, 128);

  auto ambient = (float)static_cast<double>(element->getAttribute("ambient"));
  auto diffuse = (float)static_cast<double>(element->getAttribute("diffuse"));
  auto specular = (float)static_cast<double>(element->getAttribute("specular"));
  auto specular_power = (float)static_cast<double>(element->getAttribute("specular_power"));
  float *data = &(conv_data[0]);

  gr3_clear();
  gr3_setlightparameters(ambient, diffuse, specular, specular_power);

  processWindow(element->parentElement());
  processSpace3d(element->parentElement());

  if (global_render->getRedrawWs())
    gr3_isosurface(z_dims_vec[0], z_dims_vec[1], z_dims_vec[2], data, (float)isovalue, foreground_colors, strides);

  gr3_setdefaultlightparameters();
}

void processHistogram(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for histogram
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  int bar_color_index = 989, i;
  std::vector<double> bar_color_rgb_vec = {-1, -1, -1};
  std::shared_ptr<GRM::Element> plot_parent;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  int child_id = 0;
  int edge_color_index = 1;
  std::vector<double> edge_color_rgb_vec = {-1, -1, -1};
  double x_min, x_max, bar_width, y_min, y_max;
  std::vector<double> bins_vec;
  unsigned int num_bins;
  std::string orientation = PLOT_DEFAULT_ORIENTATION, line_spec = SERIES_DEFAULT_SPEC;
  bool is_horizontal;
  auto global_render = grm_get_render();
  auto global_creator = grm_get_creator();

  if (element->hasAttribute("fill_color_rgb"))
    {
      auto bar_color_rgb = static_cast<std::string>(element->getAttribute("fill_color_rgb"));
      bar_color_rgb_vec = GRM::get<std::vector<double>>((*context)[bar_color_rgb]);
    }

  // Todo: using line_spec here isn't really clean, cause no lines are drawn, but it's the only option atm to get the
  // same different colors like multiple line series have
  const char *spec_char = line_spec.c_str();
  gr_uselinespec((char *)spec_char);
  gr_inqmarkercolorind(&bar_color_index);

  if (element->hasAttribute("fill_color_ind"))
    bar_color_index = static_cast<int>(element->getAttribute("fill_color_ind"));
  else
    element->setAttribute("fill_color_ind", bar_color_index);

  plot_parent = element->parentElement();
  getPlotParent(plot_parent);

  if (bar_color_rgb_vec[0] != -1)
    {
      for (i = 0; i < 3; i++)
        {
          if (bar_color_rgb_vec[i] > 1 || bar_color_rgb_vec[i] < 0)
            throw std::out_of_range("For histogram series bar_color_rgb must be inside [0, 1].\n");
        }
      bar_color_index = 1000;
      global_render->setColorRep(element, bar_color_index, bar_color_rgb_vec[0], bar_color_rgb_vec[1],
                                 bar_color_rgb_vec[2]);
      // processColorRep has to be manually triggered.
      processColorReps(element);
    }

  if (element->hasAttribute("line_color_rgb"))
    {
      auto edge_color_rgb = static_cast<std::string>(element->getAttribute("line_color_rgb"));
      edge_color_rgb_vec = GRM::get<std::vector<double>>((*context)[edge_color_rgb]);
    }

  if (element->hasAttribute("line_color_ind"))
    edge_color_index = static_cast<int>(element->getAttribute("line_color_ind"));
  if (edge_color_rgb_vec[0] != -1)
    {
      for (i = 0; i < 3; i++)
        {
          if (edge_color_rgb_vec[i] > 1 || edge_color_rgb_vec[i] < 0)
            throw std::out_of_range("For histogram series edge_color_rgb must be inside [0, 1].\n");
        }
      edge_color_index = 1001;
      global_render->setColorRep(element, edge_color_index, edge_color_rgb_vec[0], edge_color_rgb_vec[1],
                                 edge_color_rgb_vec[2]);
      processColorReps(element);
    }

  if (!element->hasAttribute("bins")) histBins(element, context);
  auto bins = static_cast<std::string>(element->getAttribute("bins"));
  bins_vec = GRM::get<std::vector<double>>((*context)[bins]);
  num_bins = bins_vec.size();

  if (element->parentElement()->hasAttribute("orientation"))
    orientation = static_cast<std::string>(element->parentElement()->getAttribute("orientation"));
  is_horizontal = orientation == "horizontal";

  x_min = static_cast<double>(element->getAttribute("x_range_min"));
  x_max = static_cast<double>(element->getAttribute("x_range_max"));
  y_min = static_cast<double>(element->getAttribute("y_range_min"));
  y_max = static_cast<double>(element->getAttribute("y_range_max"));
  if (plot_parent->hasAttribute("_y_line_pos")) y_min = static_cast<double>(plot_parent->getAttribute("_y_line_pos"));
  if (std::isnan(y_min)) y_min = 0.0;
  if (plot_parent->hasAttribute("y_log") && static_cast<int>(plot_parent->getAttribute("y_log")) && y_min < 0)
    y_min = 1;

  if (element->parentElement()->parentElement()->hasAttribute("marginal_heatmap_side_plot"))
    {
      std::shared_ptr<GRM::Element> marginal_heatmap;
      for (const auto &children : plot_parent->children())
        {
          if (children->localName() == "marginal_heatmap_plot")
            {
              marginal_heatmap = children;
              break;
            }
        }
      if (marginal_heatmap->hasAttribute("x_range_min"))
        x_min = static_cast<double>(marginal_heatmap->getAttribute("x_range_min"));
      if (marginal_heatmap->hasAttribute("x_range_max"))
        x_max = static_cast<double>(marginal_heatmap->getAttribute("x_range_max"));
      if (marginal_heatmap->hasAttribute("y_range_min"))
        y_min = static_cast<double>(marginal_heatmap->getAttribute("y_range_min"));
      if (marginal_heatmap->hasAttribute("y_range_max"))
        y_max = static_cast<double>(marginal_heatmap->getAttribute("y_range_max"));
      processMarginalHeatmapSidePlot(element->parentElement());
      processMarginalHeatmapKind(marginal_heatmap);

      if (!is_horizontal)
        {
          double tmp_min = x_min, tmp_max = x_max;

          x_min = y_min;
          x_max = y_max;
          y_min = tmp_min;
          y_max = tmp_max;
        }
      y_min = 0.0;
    }

  bar_width = (x_max - x_min) / num_bins;

  /* clear old bars */
  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  for (i = 1; i < num_bins + 1; ++i)
    {
      double x = x_min + (i - 1) * bar_width;
      std::shared_ptr<GRM::Element> bar;

      if (is_horizontal)
        {
          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              bar = global_creator->createBar(x, x + bar_width, y_min, bins_vec[i - 1], bar_color_index,
                                              edge_color_index);
              bar->setAttribute("_child_id", child_id++);
              element->append(bar);
            }
          else
            {
              bar = element->querySelectors("bar[_child_id=" + std::to_string(child_id++) + "]");
              if (bar != nullptr)
                global_creator->createBar(x, x + bar_width, y_min, bins_vec[i - 1], bar_color_index, edge_color_index,
                                          "", "", -1, "", bar);
            }
        }
      else
        {
          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              bar = global_creator->createBar(y_min, bins_vec[i - 1], x, x + bar_width, bar_color_index,
                                              edge_color_index);
              bar->setAttribute("_child_id", child_id++);
              element->append(bar);
            }
          else
            {
              bar = element->querySelectors("bar[_child_id=" + std::to_string(child_id++) + "]");
              if (bar != nullptr)
                global_creator->createBar(y_min, bins_vec[i - 1], x, x + bar_width, bar_color_index, edge_color_index,
                                          "", "", -1, "", bar);
            }
        }

      if (bar != nullptr)
        {
          if (element->hasAttribute("_fill_int_style_set_by_user"))
            bar->setAttribute("_fill_int_style_set_by_user",
                              static_cast<int>(element->getAttribute("_fill_int_style_set_by_user")));
          if (element->hasAttribute("_fill_style_set_by_user"))
            bar->setAttribute("_fill_style_set_by_user",
                              static_cast<int>(element->getAttribute("_fill_style_set_by_user")));
        }
    }

  // error_bar handling
  for (const auto &child : element->children())
    {
      if (child->localName() == "error_bars")
        {
          std::vector<double> bar_centers(num_bins);
          linSpace(x_min + 0.5 * bar_width, x_max - 0.5 * bar_width, static_cast<int>(num_bins), bar_centers);
          extendErrorBars(child, context, bar_centers, bins_vec);
        }
    }
}

void processQuiver(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for quiver
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  std::string orientation = PLOT_DEFAULT_ORIENTATION;
  if (element->parentElement()->hasAttribute("orientation"))
    orientation = static_cast<std::string>(element->parentElement()->getAttribute("orientation"));

  if (!element->hasAttribute("x")) throw NotFoundError("Quiver series is missing required attribute x-data.\n");
  auto x = static_cast<std::string>(element->getAttribute("x"));
  if (!element->hasAttribute("y")) throw NotFoundError("Quiver series is missing required attribute y-data.\n");
  auto y = static_cast<std::string>(element->getAttribute("y"));
  if (!element->hasAttribute("u")) throw NotFoundError("Quiver series is missing required attribute u-data.\n");
  auto u = static_cast<std::string>(element->getAttribute("u"));
  if (!element->hasAttribute("v")) throw NotFoundError("Quiver series is missing required attribute v-data.\n");
  auto v = static_cast<std::string>(element->getAttribute("v"));
  bool colored = static_cast<int>(element->getAttribute("colored"));

  auto x_vec = GRM::get<std::vector<double>>((*context)[x]);
  auto y_vec = GRM::get<std::vector<double>>((*context)[y]);
  auto u_vec = GRM::get<std::vector<double>>((*context)[u]);
  auto v_vec = GRM::get<std::vector<double>>((*context)[v]);
  auto x_length = static_cast<int>(x_vec.size());
  auto y_length = static_cast<int>(y_vec.size());
  auto u_length = static_cast<int>(u_vec.size());
  auto v_length = static_cast<int>(v_vec.size());

  if (x_length * y_length != u_length)
    throw std::length_error("For quiver series x_length * y_length must be u_length.\n");
  if (x_length * y_length != v_length)
    throw std::length_error("For quiver series x_length * y_length must be v_length.\n");

  if (orientation == "vertical")
    {
      auto tmp = x_vec;
      x_vec = y_vec;
      y_vec = tmp;
      auto tmp2 = x_length;
      x_length = y_length;
      y_length = tmp2;

      std::vector<double> uv(u_length), vv(v_length);
      for (int i = 0; i < y_length; i++)
        {
          for (int j = 0; j < x_length; j++)
            {
              uv[j + i * x_length] = u_vec[i + j * y_length];
              vv[j + i * x_length] = v_vec[i + j * y_length];
            }
        }
      u_vec = uv;
      v_vec = vv;
    }

  double *x_p = &(x_vec[0]);
  double *y_p = &(y_vec[0]);
  double *u_p = &(u_vec[0]);
  double *v_p = &(v_vec[0]);
  applyMoveTransformation(element);
  processColormap(element->parentElement()->parentElement());

  if (grm_get_render()->getRedrawWs()) gr_quiver(x_length, y_length, x_p, y_p, u_p, v_p, colored);
}

void processPolarLine(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for polar_line
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  std::vector<double> theta, r;
  std::string line_spec = SERIES_DEFAULT_SPEC;
  std::shared_ptr<GRM::Element> plot_parent = element;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  int child_id = 0;
  getPlotParent(plot_parent);
  auto global_root = grm_get_document_root();
  auto global_creator = grm_get_creator();
  auto global_render = grm_get_render();

  auto previous_line_marker_type = global_render->getPreviousLineMarkerType();

  if (element->hasAttribute("line_spec"))
    line_spec = static_cast<std::string>(element->getAttribute("line_spec"));
  else
    element->setAttribute("line_spec", line_spec);

  calculatePolarThetaAndR(theta, r, element, context);

  auto id = static_cast<int>(global_root->getAttribute("_id"));
  auto str_id = std::to_string(id);

  /* clear old polylines */
  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  const char *spec_char = line_spec.c_str();
  auto mask = gr_uselinespec((char *)spec_char);

  if (intEqualsAny(mask, 5, 0, 1, 3, 4, 5))
    {
      std::shared_ptr<GRM::Element> line;
      int current_line_color_ind;
      gr_inqlinecolorind(&current_line_color_ind);
      if (element->hasAttribute("line_color_ind"))
        current_line_color_ind = static_cast<int>(element->getAttribute("line_color_ind"));
      else
        element->setAttribute("line_color_ind", current_line_color_ind);

      if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
        {
          line = global_creator->createPolyline("x" + str_id, theta, "y" + str_id, r);
          line->setAttribute("_child_id", child_id++);
          element->append(line);
        }
      else
        {
          line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
          if (line != nullptr)
            global_creator->createPolyline("x" + str_id, theta, "y" + str_id, r, nullptr, 0, 0.0, 0, line);
        }
      if (line != nullptr)
        {
          if (!line->hasAttribute("_line_color_ind_set_by_user"))
            line->setAttribute("line_color_ind", current_line_color_ind);
        }
    }
  if (mask & 2)
    {
      std::shared_ptr<GRM::Element> marker;
      int current_marker_color_ind;
      gr_inqmarkercolorind(&current_marker_color_ind);
      if (element->hasAttribute("marker_color_ind"))
        current_marker_color_ind = static_cast<int>(element->getAttribute("marker_color_ind"));
      else
        element->setAttribute("marker_color_ind", current_marker_color_ind);

      if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
        {
          marker = global_creator->createPolymarker("x" + str_id, theta, "y" + str_id, r);
          marker->setAttribute("_child_id", child_id++);
          element->append(marker);
        }
      else
        {
          marker = element->querySelectors("polymarker[_child_id=" + std::to_string(child_id++) + "]");
          if (marker != nullptr)
            global_creator->createPolymarker("x" + str_id, theta, "y" + str_id, r, nullptr, 0, 0.0, 0, marker);
        }
      if (marker != nullptr)
        {
          if (!marker->hasAttribute("_marker_solor_ind_set_by_user"))
            marker->setAttribute("marker_color_ind", current_marker_color_ind);
          marker->setAttribute("z_index", 2);

          if (!marker->hasAttribute("_marker_type_set_by_user"))
            {
              if (element->hasAttribute("marker_type"))
                {
                  marker->setAttribute("marker_type", static_cast<int>(element->getAttribute("marker_type")));
                }
              else
                {
                  marker->setAttribute("marker_type", *previous_line_marker_type++);
                  if (*previous_line_marker_type == INT_MAX) previous_line_marker_type = plot_scatter_markertypes;
                  global_render->setPreviousLineMarkerType(previous_line_marker_type);
                }
            }
        }
    }
  global_root->setAttribute("_id", id + 1);
}

void processPolarScatter(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for polar_scatter
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  std::vector<double> theta, r;
  std::shared_ptr<GRM::Element> plot_parent = element;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  int child_id = 0;
  std::shared_ptr<GRM::Element> marker;
  int current_marker_color_ind;
  getPlotParent(plot_parent);
  auto global_root = grm_get_document_root();
  auto global_render = grm_get_render();
  auto global_creator = grm_get_creator();

  auto previous_scatter_marker_type = global_render->getPreviousScatterMarkerType();

  calculatePolarThetaAndR(theta, r, element, context);

  auto id = static_cast<int>(global_root->getAttribute("_id"));
  auto str_id = std::to_string(id);

  /* clear old polylines */
  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  if (theta.size() == 0 || r.size() == 0) del = DelValues::RECREATE_OWN_CHILDREN;
  clearOldChildren(&del, element);
  if (theta.size() == 0 || r.size() == 0) return;

  if (!element->hasAttribute("marker_type"))
    {
      element->setAttribute("marker_type", *previous_scatter_marker_type++);
      if (*previous_scatter_marker_type == INT_MAX) previous_scatter_marker_type = plot_scatter_markertypes;
      global_render->setPreviousScatterMarkerType(previous_scatter_marker_type);
    }
  processMarkerType(element);

  if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
    {
      marker = global_creator->createPolymarker("x" + str_id, theta, "y" + str_id, r);
      marker->setAttribute("_child_id", child_id++);
      element->append(marker);
    }
  else
    {
      marker = element->querySelectors("polymarker[_child_id=" + std::to_string(child_id++) + "]");
      if (marker != nullptr)
        global_creator->createPolymarker("x" + str_id, theta, "y" + str_id, r, nullptr, 0, 0.0, 0, marker);
    }
  if (marker != nullptr)
    {
      gr_inqmarkercolorind(&current_marker_color_ind);
      if (element->hasAttribute("marker_color_ind"))
        current_marker_color_ind = static_cast<int>(element->getAttribute("marker_color_ind"));
      else
        marker->setAttribute("marker_color_ind", current_marker_color_ind);
      if (element->hasAttribute("marker_size") && !marker->hasAttribute("_marker_size_set_by_user"))
        {
          auto marker_size = static_cast<double>(element->getAttribute("marker_size"));
          marker->setAttribute("marker_size", marker_size);
        }
      if (!marker->hasAttribute("_marker_type_set_by_user") && element->hasAttribute("marker_type"))
        marker->setAttribute("marker_type", static_cast<int>(element->getAttribute("marker_type")));
    }
  global_root->setAttribute("_id", id + 1);
}

void processPolarHeatmap(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for polar_heatmap
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */

  std::string kind;
  int icmap[256];
  unsigned int i, cols, rows, z_length;
  double theta_min, theta_max, r_min, r_max, z_min, z_max, c_min, c_max, zv;
  double theta_range_min, theta_range_max, r_range_min, r_range_max, z_range_min, z_range_max;
  bool is_uniform_heatmap, z_range = false, transform = false, r_log = false;
  std::vector<int> data;
  std::vector<double> theta_vec, r_vec, z_vec;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  int child_id = 0;
  double convert = 1.0;
  std::shared_ptr<GRM::Element> central_region, plot_parent = element;
  getPlotParent(plot_parent);
  auto global_root = grm_get_document_root();
  auto global_creator = grm_get_creator();
  auto global_render = grm_get_render();

  for (const auto &child : plot_parent->children())
    {
      if (child->localName() == "central_region")
        {
          central_region = child;
          break;
        }
    }

  kind = static_cast<std::string>(plot_parent->getAttribute("_kind"));
  if (plot_parent->hasAttribute("r_log")) r_log = static_cast<int>(plot_parent->getAttribute("r_log"));

  // calculate polar limits (r_max)
  calculatePolarLimits(central_region, context);
  central_region->setAttribute("_skip_calculations", true);

  if (element->hasAttribute("theta"))
    {
      auto theta = static_cast<std::string>(element->getAttribute("theta"));
      theta_vec = GRM::get<std::vector<double>>((*context)[theta]);
      cols = theta_vec.size();
    }
  if (element->hasAttribute("r"))
    {
      auto r = static_cast<std::string>(element->getAttribute("r"));
      r_vec = GRM::get<std::vector<double>>((*context)[r]);
      rows = r_vec.size();
    }
  if (!element->hasAttribute("z")) throw NotFoundError("Polar-heatmap series is missing required attribute z-data.\n");
  auto z = static_cast<std::string>(element->getAttribute("z"));
  z_vec = GRM::get<std::vector<double>>((*context)[z]);
  z_length = z_vec.size();

  if (element->hasAttribute("theta_range_min") && element->hasAttribute("theta_range_max"))
    {
      transform = true;
      theta_range_min = static_cast<double>(element->getAttribute("theta_range_min"));
      theta_range_max = static_cast<double>(element->getAttribute("theta_range_max"));
      if (theta_range_max <= 2 * M_PI) convert = 180.0 / M_PI;
    }
  if (element->hasAttribute("r_range_min") && element->hasAttribute("r_range_max"))
    {
      transform = true;
      r_range_min = static_cast<double>(element->getAttribute("r_range_min"));
      r_range_max = static_cast<double>(element->getAttribute("r_range_max"));
    }
  if (element->hasAttribute("z_range_min") && element->hasAttribute("z_range_max"))
    {
      z_range = true;
      z_range_min = static_cast<double>(element->getAttribute("z_range_min"));
      z_range_max = static_cast<double>(element->getAttribute("z_range_max"));
    }

  if (theta_vec.empty() && r_vec.empty())
    {
      /* If neither `theta` nor `r` are given, we need more information about the shape of `z` */
      if (!element->hasAttribute("z_dims"))
        throw NotFoundError("Polar-heatmap series is missing required attribute z_dims.\n");
      auto z_dims_key = static_cast<std::string>(element->getAttribute("z_dims"));
      auto z_dims_vec = GRM::get<std::vector<int>>((*context)[z_dims_key]);
      cols = z_dims_vec[0];
      rows = z_dims_vec[1];
    }
  else if (theta_vec.empty())
    {
      cols = z_length / rows;
    }
  else if (r_vec.empty())
    {
      rows = z_length / cols;
    }

  is_uniform_heatmap = isEquidistantArray(cols, theta_vec.data()) && isEquidistantArray(rows, r_vec.data());
  if (kind == "nonuniform_polar_heatmap") is_uniform_heatmap = false;

  if (!is_uniform_heatmap && (theta_vec.empty() || r_vec.empty()))
    throw NotFoundError("Polar-heatmap series is missing theta- or r-data or the data has to be uniform.\n");

  if (theta_vec.empty())
    {
      theta_min = theta_range_min;
      theta_max = theta_range_max;
    }
  else
    {
      theta_min = theta_vec[0];
      theta_max = theta_vec[cols - 1];
    }
  if (r_vec.empty())
    {
      r_min = r_range_min;
      r_max = r_range_max;
    }
  else
    {
      r_min = r_vec[0];
      r_max = r_vec[rows - 1];
    }

  double central_r_max = static_cast<double>(central_region->getAttribute("r_max"));
  if (r_min > 0.0) is_uniform_heatmap = false;

  // Check if coordinate transformations are needed and then transform if needed
  if (transform &&
      ((!theta_vec.empty() && (theta_vec[0] < theta_range_min || theta_vec[theta_vec.size() - 1] > theta_range_max)) ||
       (!r_vec.empty() && (r_vec[0] < r_range_min || r_vec[r_vec.size() - 1] > r_range_max))))
    {
      auto id = static_cast<int>(global_root->getAttribute("_id"));
      global_root->setAttribute("_id", id + 1);
      auto str = std::to_string(id);
      is_uniform_heatmap = false;

      if (theta_vec.empty())
        {
          theta_vec.resize(cols);
          for (int col = 0; col < cols; col++)
            {
              theta_vec[col] = transformCoordinate(col / (cols - 1.0) * 360.0, 0.0, 360.0, theta_range_min * convert,
                                                   theta_range_max * convert);
            }
          (*context)["theta" + str] = theta_vec;
          element->setAttribute("theta", "theta" + str);
        }
      else
        {
          transformCoordinatesVector(theta_vec, theta_min, theta_max, theta_range_min * convert,
                                     theta_range_max * convert);
        }
      if (r_vec.empty())
        {
          r_vec.resize(rows);
          for (int row = 0; row < rows; row++)
            {
              r_vec[row] = transformCoordinate(row / (rows - 1.0), 0.0, 1.0, r_range_min, r_range_max);
            }
          (*context)["r" + str] = r_vec;
          element->setAttribute("r", "r" + str);
        }
      else
        {
          transformCoordinatesVector(r_vec, r_min, r_max, r_range_min, r_range_max);
        }
    }

  if (z_range)
    {
      double min_val = *std::min_element(z_vec.begin(), z_vec.end());
      double max_val = *std::max_element(z_vec.begin(), z_vec.end());

      for (int elem = 0; elem < rows * cols; ++elem)
        {
          z_vec[elem] = z_range_min + (z_range_max - z_range_min) * (z_vec[elem] - min_val) / (max_val - min_val);
        }
    }

  z_min = static_cast<double>(element->getAttribute("z_range_min"));
  z_max = static_cast<double>(element->getAttribute("z_range_max"));
  if (!element->hasAttribute("c_range_min") || !element->hasAttribute("c_range_max"))
    {
      c_min = z_min;
      c_max = z_max;
    }
  else
    {
      c_min = static_cast<double>(element->getAttribute("c_range_min"));
      c_max = static_cast<double>(element->getAttribute("c_range_max"));
    }

  for (i = 0; i < 256; i++) gr_inqcolor(1000 + static_cast<int>(i), icmap + i);

  data = std::vector<int>(rows * cols);
  if (z_max > z_min)
    {
      for (i = 0; i < cols * rows; i++)
        {
          zv = z_vec[i];

          if (zv > z_max || zv < z_min || grm_isnan(zv))
            {
              data[i] = -1;
            }
          else
            {
              data[i] = 1000 + static_cast<int>(255.0 * (zv - c_min) / (c_max - c_min) + 0.5);
              data[i] = grm_max(grm_min(data[i], 1255), 1000);
            }
        }
    }
  else
    {
      for (i = 0; i < cols * rows; i++) data[i] = 0;
    }

  // for cases like r_max = 3.2342 -> calc new central_r_max = 4.0 and use nonuniform_polar_cell_array
  if (r_max != central_r_max) is_uniform_heatmap = false;

  auto id = static_cast<int>(global_root->getAttribute("_id"));
  global_root->setAttribute("_id", id + 1);
  auto str = std::to_string(id);

  /* clear old polar_heatmaps */
  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  if (r_log) is_uniform_heatmap = false;

  std::shared_ptr<GRM::Element> polar_cell_array;
  if (is_uniform_heatmap)
    {
      if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
        {
          polar_cell_array = global_creator->createPolarCellArray(0, 0, 0, 360, 0, 1, (int)cols, (int)rows, 1, 1,
                                                                  (int)cols, (int)rows, "color_ind_values" + str, data);
          polar_cell_array->setAttribute("_child_id", child_id++);
          element->append(polar_cell_array);
        }
      else
        {
          polar_cell_array = element->querySelectors("polar_cell_array[_child_id=" + std::to_string(child_id++) + "]");
          if (polar_cell_array != nullptr)
            global_creator->createPolarCellArray(0, 0, 0, 360, 0, 1, (int)cols, (int)rows, 1, 1, (int)cols, (int)rows,
                                                 "color_ind_values" + str, data, nullptr, polar_cell_array);
        }
    }
  else
    {
      if (central_region->hasAttribute("r_max")) r_max = static_cast<double>(central_region->getAttribute("r_max"));
      if (theta_vec[cols - 1] <= 2 * M_PI) convert = 180.0 / M_PI;

      std::vector<double> radial(rows), theta(cols);
      for (i = 0; i < ((cols > rows) ? cols : rows); i++)
        {
          if (i < cols) theta[i] = theta_vec[i] * convert;
          if (i < rows)
            {
              if (r_log)
                {
                  radial[i] = (r_vec[i] <= 0) ? NAN : log10(r_vec[i]) / log10(r_max);
                }
              else
                {
                  radial[i] = r_vec[i] / r_max;
                }
            }
        }

      if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
        {
          polar_cell_array = global_creator->createNonUniformPolarCellArray(
              0, 0, "theta" + str, theta, "radial" + str, radial, (int)-cols, (int)-rows, 1, 1, (int)cols, (int)rows,
              "color_ind_values" + str, data);
          polar_cell_array->setAttribute("_child_id", child_id++);
          element->append(polar_cell_array);
        }
      else
        {
          polar_cell_array =
              element->querySelectors("nonuniform_polar_cell_array[_child_id=" + std::to_string(child_id++) + "]");
          if (polar_cell_array != nullptr)
            global_creator->createNonUniformPolarCellArray(0, 0, "theta" + str, theta, "radial" + str, radial,
                                                           (int)-cols, (int)-rows, 1, 1, (int)cols, (int)rows,
                                                           "color_ind_values" + str, data, nullptr, polar_cell_array);
        }
    }
  if (!plot_parent->hasAttribute("polar_with_pan") || !static_cast<int>(plot_parent->getAttribute("polar_with_pan")))
    {
      if (!polar_cell_array->hasAttribute("clip_region")) global_render->setClipRegion(polar_cell_array, 1);
      if (!polar_cell_array->hasAttribute("select_specific_xform"))
        global_render->setSelectSpecificXform(polar_cell_array, 1);
    }
  processColormap(element->parentElement()->parentElement());
}

void processWireframe(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for wireframe
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  unsigned int x_length, y_length, z_length;

  auto global_render = grm_get_render();
  auto global_root = grm_get_document_root();

  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto y = static_cast<std::string>(element->getAttribute("y"));
  auto z = static_cast<std::string>(element->getAttribute("z"));

  auto x_vec = GRM::get<std::vector<double>>((*context)[x]);
  auto y_vec = GRM::get<std::vector<double>>((*context)[y]);
  auto z_vec = GRM::get<std::vector<double>>((*context)[z]);
  x_length = x_vec.size();
  y_length = y_vec.size();
  z_length = z_vec.size();

  if (!element->hasAttribute("fill_color_ind")) global_render->setFillColorInd(element, 0);
  processFillColorInd(element);

  auto id_int = static_cast<int>(global_root->getAttribute("_id"));
  global_root->setAttribute("_id", ++id_int);
  auto id = std::to_string(id_int);

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

      gr_gridit(static_cast<int>(x_length), x_p, y_p, z_p, PLOT_WIREFRAME_GRIDIT_N, PLOT_WIREFRAME_GRIDIT_N, gridit_x,
                gridit_y, gridit_z);

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
  applyMoveTransformation(element);
  processSpace3d(element->parentElement());

  if (global_render->getRedrawWs())
    gr_surface(static_cast<int>(x_length), static_cast<int>(y_length), px_p, py_p, pz_p, GR_OPTION_FILLED_MESH);
}

void processPolarHistogram(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  unsigned int num_bins, num_bin_edges = 0;
  int edge_color = 1, face_color = 989, total_observations = 0;
  int child_id = 0, i;
  double transparency = 0.75, bin_width = -1.0;
  double r_min = 0.0, r_max = 1.0;
  double *theta_lim = nullptr;
  double r_lim_min, r_lim_max;
  bool draw_edges = false, stairs = false, theta_flip = false, keep_radii_axes = false, rlims = false, r_log = false;
  std::string norm = "count";
  std::vector<double> r_lim_vec, bin_edges, bin_widths, rect_list;
  std::vector<int> counts;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  std::shared_ptr<GRM::Element> plot_group = element->parentElement();
  getPlotParent(plot_group);
  auto global_render = grm_get_render();
  auto global_creator = grm_get_creator();
  auto global_root = grm_get_document_root();

  auto counts_key = static_cast<std::string>(element->getAttribute("counts"));
  counts = GRM::get<std::vector<int>>((*context)[counts_key]);

  /* clear old polar-histogram children */
  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  if (plot_group->hasAttribute("r_log")) r_log = static_cast<int>(plot_group->getAttribute("r_log"));

  if (element->hasAttribute("line_color_ind")) edge_color = static_cast<int>(element->getAttribute("line_color_ind"));
  if (element->hasAttribute("fill_color_ind")) face_color = static_cast<int>(element->getAttribute("fill_color_ind"));
  if (element->hasAttribute("transparency")) transparency = static_cast<double>(element->getAttribute("transparency"));
  if (element->hasAttribute("norm")) norm = static_cast<std::string>(element->getAttribute("norm"));
  if (plot_group->hasAttribute("theta_flip")) theta_flip = static_cast<int>(plot_group->getAttribute("theta_flip"));
  if (element->hasAttribute("draw_edges")) draw_edges = static_cast<int>(element->getAttribute("draw_edges"));
  num_bins = static_cast<int>(element->getAttribute("num_bins"));
  r_max = static_cast<double>(element->parentElement()->getAttribute("r_max"));
  if (r_log) r_max = log10(r_max);
  total_observations = static_cast<int>(element->getAttribute("_total"));
  global_render->setTransparency(element, transparency);
  processTransparency(element);

  if (plot_group->hasAttribute("r_lim_min") && (plot_group->hasAttribute("r_lim_max")))
    {
      rlims = true;
      r_lim_min = static_cast<double>(plot_group->getAttribute("r_lim_min"));
      r_lim_max = static_cast<double>(plot_group->getAttribute("r_lim_max"));

      if (r_log)
        {
          r_lim_min = log10(r_lim_min);
          r_lim_max = log10(r_lim_max);
        }

      if (plot_group->hasAttribute("keep_radii_axes"))
        keep_radii_axes = static_cast<int>(plot_group->getAttribute("keep_radii_axes"));
    }
  if (plot_group->hasAttribute("theta_lim_min") || plot_group->hasAttribute("theta_lim_max"))
    {
      double theta_lim_arr[2];
      theta_lim = theta_lim_arr;
      theta_lim[0] = static_cast<double>(plot_group->getAttribute("theta_lim_min"));
      theta_lim[1] = static_cast<double>(plot_group->getAttribute("theta_lim_max"));
    }

  if (!element->hasAttribute("bin_edges"))
    {
      if (element->hasAttribute("bin_width")) bin_width = static_cast<double>(element->getAttribute("bin_width"));
    }
  else
    {
      auto bin_edges_key = static_cast<std::string>(element->getAttribute("bin_edges"));
      bin_edges = GRM::get<std::vector<double>>((*context)[bin_edges_key]);
      num_bin_edges = bin_edges.size();

      auto bin_widths_key = static_cast<std::string>(element->getAttribute("bin_widths"));
      bin_widths = GRM::get<std::vector<double>>((*context)[bin_widths_key]);
      num_bins = bin_widths.size();
    }

  if (element->hasAttribute("stairs"))
    {
      /* Set default stairs transparency values */
      if (!element->hasAttribute("transparency")) element->setAttribute("transparency", 1.0);

      stairs = static_cast<int>(element->getAttribute("stairs"));
      if (stairs) rect_list.resize(num_bins);
    }

  if (theta_flip) std::reverse(counts.begin(), counts.end());

  /* if theta_flip and bin_edges are given --> invert the angles */
  if (theta_flip && num_bin_edges > 0)
    {
      std::vector<double> temp1(num_bin_edges), temp2(num_bins);

      for (i = 0; i < num_bin_edges; i++)
        {
          temp1[i] = 2 * M_PI - bin_edges[num_bin_edges - 1 - i];
        }
      bin_edges = temp1;
      for (i = static_cast<int>(num_bins) - 1; i >= 0; --i)
        {
          temp2[i] = bin_widths[num_bins - 1 - i];
        }
      bin_widths = temp2;
    }

  // Iterate through the counts and create for every bar a polar_bar element
  // main loop used for each bar (and arc in stairs, but not the lines in stairs)
  for (int class_nr = 0; class_nr < counts.size(); class_nr++)
    {
      double count = counts[class_nr];

      if (r_log)
        {
          if (count > 0)
            count = log10(count);
          else
            continue;
        }

      // adjust count according to the given normalization
      if (strEqualsAny(norm, "probability", "cdf"))
        {
          count /= total_observations;
        }
      else if (norm == "pdf")
        {
          count /= num_bin_edges == 0 ? (total_observations * bin_width) : (total_observations * bin_widths[class_nr]);
        }
      else if (norm == "countdensity")
        {
          count /= num_bin_edges == 0 ? bin_width : bin_widths[class_nr];
        }

      if (!stairs) // no stairs uses `polar_bar` logic which is implemented in `processPolarBar`
        {
          std::shared_ptr<GRM::Element> polar_bar;

          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              polar_bar = global_creator->createPolarBar(count, class_nr);
              polar_bar->setAttribute("_child_id", child_id++);
              element->append(polar_bar);
            }
          else
            {
              polar_bar = element->querySelectors("polar_bar[_child_id=" + std::to_string(child_id++) + "]");
              if (polar_bar != nullptr) global_creator->createPolarBar(count, class_nr, polar_bar);
            }

          if (polar_bar != nullptr)
            {
              if (bin_width != -1) polar_bar->setAttribute("bin_width", bin_width);
              if (norm != "count") polar_bar->setAttribute("norm", norm);
              if (theta_flip) polar_bar->setAttribute("theta_flip", theta_flip);
              if (draw_edges) polar_bar->setAttribute("draw_edges", draw_edges);
              if (edge_color != 1) polar_bar->setAttribute("line_color_ind", edge_color);
              if (face_color != 989) polar_bar->setAttribute("fill_color_ind", face_color);
              if (!bin_widths.empty()) polar_bar->setAttribute("bin_widths", bin_widths[class_nr]);
              if (!bin_edges.empty())
                {
                  auto id = static_cast<int>(global_root->getAttribute("_id"));
                  auto str = std::to_string(id);
                  global_root->setAttribute("_id", id + 1);

                  auto bin_edges_vec = std::vector<double>{bin_edges[class_nr], bin_edges[class_nr + 1]};
                  auto bin_edges_key = "bin_edges" + str;
                  (*context)[bin_edges_key] = bin_edges_vec;
                  polar_bar->setAttribute("bin_edges", bin_edges_key);
                }
            }
        }
      else if (!draw_edges) /* stairs without draw_edges (not compatible) */
        {
          // this is for drawing the arcs in stairs.
          double r, rect;
          std::complex<double> complex1;
          const double convert = 180.0 / M_PI;
          double edge_width = 2.3; /* only for stairs */
          bool draw_inner = true;
          double start_angle, end_angle;
          std::shared_ptr<GRM::Element> arc;
          double arc_pos = 0.0;

          if (!element->hasAttribute("_fill_color_ind_set_by_user")) global_render->setFillColorInd(element, 1);
          if (!element->hasAttribute("_line_color_ind_set_by_user"))
            global_render->setLineColorInd(element, edge_color);
          if (!element->hasAttribute("_line_width_set_by_user")) global_render->setLineWidth(element, edge_width);
          processLineColorInd(element);
          processFillColorInd(element);
          processLineWidth(element);

          /* perform calculations for later usages, this r is used for complex calculations */
          if (keep_radii_axes && rlims)
            {
              r = pow(count / r_max, num_bins * 2);
              if (r > pow(r_lim_max / r_max, num_bins * 2)) r = pow(r_lim_max / r_max, num_bins * 2);
            }
          else if (rlims)
            {
              // trim count to [0.0, y_lim_max]
              count = grm_max(0.0, grm_min(count, r_lim_max) - r_lim_min);
              r = pow((count / (r_lim_max - r_lim_min)), num_bins * 2);
            }
          else
            {
              r = pow(count / r_max, num_bins * 2);
            }

          complex1 = moivre(r, (2 * class_nr), (int)num_bins * 2);
          rect = sqrt(pow(real(complex1), 2) + pow(imag(complex1), 2));

          if (num_bin_edges)
            {
              start_angle = bin_edges[class_nr] * convert;
              end_angle = bin_edges[class_nr + 1] * convert;
            }
          else
            {
              start_angle = class_nr * (360.0 / num_bins);
              end_angle = (class_nr + 1) * (360.0 / num_bins);
            }

          rect_list[class_nr] = rect;
          if (rlims)
            {
              if (keep_radii_axes)
                {
                  if (count <= r_lim_min)
                    {
                      rect_list[class_nr] = r_lim_min / r_max;
                      draw_inner = false;
                    }
                  else if (rect > r_max) // Todo: r_max or 1.0 as previous?
                    rect_list[class_nr] = r_lim_max;

                  auto complex_min = moivre(pow(r_lim_min / r_max, num_bins * 2), (2 * class_nr), (int)num_bins * 2);
                  arc_pos = sqrt(pow(real(complex_min), 2) + pow(imag(complex_min), 2));
                  if (count <= r_lim_min) arc_pos = 0.0;
                }
              else
                {
                  draw_inner = false; // without keep_radii_axes draw_inner is not needed

                  // y_lim_min is already subtracted from count
                  if (count < 0)
                    rect_list[class_nr] = 0.0;
                  else if (count >= r_lim_max - r_lim_min)
                    rect_list[class_nr] = 1.0; // 1.0 equals y_lim_max (when no keep_radii_axes is set)
                }

              // this is the outer arc
              if ((count > 0 && !keep_radii_axes) || (count > r_lim_min && keep_radii_axes))
                {
                  double min = grm_min(rect, r_max); // Todo: r_max or 1.0 as previous?
                  if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
                    {
                      arc = global_creator->createDrawArc(-min, min, -min, min, start_angle, end_angle);
                      arc->setAttribute("_child_id", child_id++);
                      element->append(arc);
                    }
                  else
                    {
                      arc = element->querySelectors("draw_arc[_child_id=" + std::to_string(child_id++) + "]");
                      if (arc != nullptr)
                        global_creator->createDrawArc(-min, min, -min, min, start_angle, end_angle, arc);
                    }
                }
            }
          else
            {
              if (class_nr == counts.size()) break;
              arc_pos = rect;
            }

          // these are the inner arcs with ylim (keep_radii_axes only) and the normal_arcs without ylims, only if it's
          // higher than y_lim_min
          if (draw_inner)
            {
              if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
                {
                  arc = global_creator->createDrawArc(-arc_pos, arc_pos, -arc_pos, arc_pos, start_angle, end_angle);
                  arc->setAttribute("_child_id", child_id++);
                  element->append(arc);
                }
              else
                {
                  arc = element->querySelectors("draw_arc[_child_id=" + std::to_string(child_id++) + "]");
                  if (arc != nullptr)
                    global_creator->createDrawArc(-arc_pos, arc_pos, -arc_pos, arc_pos, start_angle, end_angle, arc);
                }
            }
        }
    } /* end of counts for loop */

  // this is for drawing the stair lines
  if (stairs && !draw_edges)
    {
      std::shared_ptr<GRM::Element> line;
      double line_x[2], line_y[2];
      double start_x = 0.0, start_y = 0.0; // start_x/y is the coordinate for minimum radius (y_lim_min)
      std::vector<double> angles_vec;

      if (num_bin_edges != 0)
        {
          start_x = grm_max(rect_list[0] * cos(bin_edges[0]), r_min * cos(bin_edges[0]));
          start_y = grm_max(rect_list[0] * sin(bin_edges[0]), r_min * sin(bin_edges[0]));
          angles_vec = bin_edges;
        }
      else
        {
          start_x = grm_max(rect_list[0], rlims ? (r_lim_min / r_lim_max) : (r_min / r_max));
          start_y = 0.0;
          linSpace(0.0, 2 * M_PI, (int)counts.size() + 1, angles_vec);
        }

      auto start_angle = angles_vec[0];
      auto end_angle = angles_vec[angles_vec.size() - 1];
      for (i = 0; i < angles_vec.size() - 1; i++)
        {
          line_x[0] = start_x;
          line_x[1] = rect_list[i] * cos(angles_vec[i]);
          line_y[0] = start_y;
          line_y[1] = rect_list[i] * sin(angles_vec[i]);

          start_x = rect_list[i] * cos(angles_vec[i + 1]);
          start_y = rect_list[i] * sin(angles_vec[i + 1]);

          if ((!rlims && !(start_angle == 0.0 && end_angle > 1.96 * M_PI) || i > 0) ||
              ((!theta_flip && (!((start_angle > 0.0 && start_angle < 0.001) && end_angle > 1.96 * M_PI) || i > 0)) ||
               ((start_angle > 1.96 * M_PI && !(end_angle > 0.0 && end_angle < 0.001)) || i > 0)))
            {
              if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
                {
                  line = global_creator->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1]);
                  line->setAttribute("_child_id", child_id++);
                  element->append(line);
                }
              else
                {
                  line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
                  if (line != nullptr)
                    global_creator->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1], 0, 0.0, 0, line);
                }
            }
        }

      // draw a line when it is not a full circle
      if (rlims && !(start_angle == 0.0 && end_angle > 1.96 * M_PI))
        {
          line_x[0] = r_lim_min / r_lim_max * cos(start_angle);
          line_x[1] = rect_list[0] * cos(start_angle);
          line_y[0] = r_lim_min / r_lim_max * sin(start_angle);
          line_y[1] = rect_list[0] * sin(start_angle);

          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              line = global_creator->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1]);
              line->setAttribute("_child_id", child_id++);
              element->append(line);
            }
          else
            {
              line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
              if (line != nullptr)
                global_creator->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1], 0, 0.0, 0, line);
            }
        }

      if (start_angle == 0.0 && end_angle > 1.96 * M_PI)
        {
          line_x[0] = rect_list[0];
          line_x[1] = rlims ? rect_list[angles_vec.size() - 2] * cos(end_angle) : start_x;
          line_y[0] = 0.0;
          line_y[1] = rlims ? rect_list[angles_vec.size() - 2] * sin(end_angle) : start_y;
        }
      else
        {
          line_x[0] = rect_list[angles_vec.size() - 2] * cos(end_angle);
          line_x[1] = rlims ? r_lim_min / r_lim_max * cos(end_angle) : 0.0;
          line_y[0] = rect_list[angles_vec.size() - 2] * sin(end_angle);
          line_y[1] = rlims ? r_lim_min / r_lim_max * sin(end_angle) : 0.0;
        }

      if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
        {
          line = global_creator->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1]);
          line->setAttribute("_child_id", child_id++);
          element->append(line);
        }
      else
        {
          line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
          if (line != nullptr)
            global_creator->createPolyline(line_x[0], line_x[1], line_y[0], line_y[1], 0, 0.0, 0, line);
        }
    }
}

void processScatter(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
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
  int i, c_index = -1;
  std::vector<int> marker_color_inds_vec;
  std::vector<double> marker_sizes_vec;
  std::vector<double> x_vec, y_vec, z_vec, c_vec;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  int child_id = 0;
  std::shared_ptr<GRM::Element> marker;
  auto global_render = grm_get_render();
  auto global_creator = grm_get_creator();
  auto global_root = grm_get_document_root();

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
  if (element->parentElement()->hasAttribute("orientation"))
    orientation = static_cast<std::string>(element->parentElement()->getAttribute("orientation"));
  auto is_horizontal = orientation == "horizontal";

  auto previous_scatter_marker_type = global_render->getPreviousScatterMarkerType();
  if (!element->hasAttribute("marker_type"))
    {
      element->setAttribute("marker_type", *previous_scatter_marker_type++);
      if (*previous_scatter_marker_type == INT_MAX) previous_scatter_marker_type = plot_scatter_markertypes;
      global_render->setPreviousScatterMarkerType(previous_scatter_marker_type);
    }
  processMarkerType(element);

  if (c_vec.empty() && element->hasAttribute("marker_color_ind"))
    {
      c_index = static_cast<int>(element->getAttribute("marker_color_ind"));
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

  /* clear old marker */
  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  if (!z_vec.empty() || !c_vec.empty())
    {
      auto plot_parent = element->parentElement();
      getPlotParent(plot_parent);

      c_min = static_cast<double>(plot_parent->getAttribute("_c_lim_min"));
      c_max = static_cast<double>(plot_parent->getAttribute("_c_lim_max"));

      for (i = 0; i < x_length; i++)
        {
          if (!z_vec.empty())
            {
              marker_sizes_vec.push_back((i < z_length) ? z_vec[i] : 2.0);
            }
          if (!c_vec.empty())
            {
              if (i < c_length)
                {
                  c_index = 1000 + static_cast<int>(255.0 * (c_vec[i] - c_min) / (c_max - c_min) + 0.5);
                  if (c_index < 1000 || c_index > 1255)
                    {
                      // color_ind -1000 will be skipped
                      marker_color_inds_vec.push_back(-1000);
                      continue;
                    }
                }
              else
                {
                  c_index = 989;
                }
              marker_color_inds_vec.push_back(c_index);
            }
          else if (c_index != -1)
            {
              marker_color_inds_vec.push_back(1000 + c_index);
            }
        }

      auto id = static_cast<int>(global_root->getAttribute("_id"));
      auto str = std::to_string(id);
      global_root->setAttribute("_id", ++id);

      if (is_horizontal)
        {
          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              marker = global_creator->createPolymarker("x" + str, x_vec, "y" + str, y_vec);
              marker->setAttribute("_child_id", child_id++);
              element->append(marker);
            }
          else
            {
              marker = element->querySelectors("polymarker[_child_id=" + std::to_string(child_id++) + "]");
              if (marker != nullptr)
                global_creator->createPolymarker("x" + str, x_vec, "y" + str, y_vec, nullptr, 0, 0.0, 0, marker);
            }
        }
      else
        {
          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              marker = global_creator->createPolymarker("x" + str, y_vec, "y" + str, x_vec);
              marker->setAttribute("_child_id", child_id++);
              element->append(marker);
            }
          else
            {
              marker = element->querySelectors("polymarker[_child_id=" + std::to_string(child_id++) + "]");
              if (marker != nullptr)
                global_creator->createPolymarker("x" + str, y_vec, "y" + str, x_vec, nullptr, 0, 0.0, 0, marker);
            }
        }
      if (marker != nullptr)
        {
          if (element->hasAttribute("_hidden"))
            marker->setAttribute("_hidden", true);
          else
            marker->removeAttribute("_hidden");
        }

      if (!marker_sizes_vec.empty()) global_render->setMarkerSize(element, "marker_sizes" + str, marker_sizes_vec);
      if (!marker_color_inds_vec.empty())
        global_render->setMarkerColorInd(element, "marker_color_indices" + str, marker_color_inds_vec);
    }
  else
    {
      auto id = static_cast<int>(global_root->getAttribute("_id"));
      auto str = std::to_string(id);

      if (is_horizontal)
        {
          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              marker = global_creator->createPolymarker("x" + str, x_vec, "y" + str, y_vec);
              marker->setAttribute("_child_id", child_id++);
              element->append(marker);
            }
          else
            {
              marker = element->querySelectors("polymarker[_child_id=" + std::to_string(child_id++) + "]");
              if (marker != nullptr)
                global_creator->createPolymarker("x" + str, x_vec, "y" + str, y_vec, nullptr, 0, 0.0, 0, marker);
            }
        }
      else
        {
          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              marker = global_creator->createPolymarker("x" + str, y_vec, "y" + str, x_vec);
              marker->setAttribute("_child_id", child_id++);
              element->append(marker);
            }
          else
            {
              marker = element->querySelectors("polymarker[_child_id=" + std::to_string(child_id++) + "]");
              if (marker != nullptr)
                global_creator->createPolymarker("x" + str, y_vec, "y" + str, x_vec, nullptr, 0, 0.0, 0, marker);
            }
        }
      if (marker != nullptr)
        {
          if (element->hasAttribute("_hidden"))
            marker->setAttribute("_hidden", true);
          else
            marker->removeAttribute("_hidden");
        }
      global_root->setAttribute("_id", ++id);
    }

  // error_bar handling
  for (const auto &child : element->children())
    {
      if (child->localName() == "error_bars") extendErrorBars(child, context, x_vec, y_vec);
    }
}

void processScatter3(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
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
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  int child_id = 0;
  std::shared_ptr<GRM::Element> marker;
  auto global_render = grm_get_render();
  auto global_root = grm_get_document_root();
  auto global_creator = grm_get_creator();

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

  std::vector<int> marker_c_vec;

  if (!element->hasAttribute("_marker_type_set_by_user"))
    global_render->setMarkerType(element, GKS_K_MARKERTYPE_SOLID_CIRCLE);
  processMarkerType(element);
  if (element->hasAttribute("c"))
    {
      auto c = static_cast<std::string>(element->getAttribute("c"));
      c_vec = GRM::get<std::vector<double>>((*context)[c]);
      c_length = c_vec.size();
      auto plot_parent = element->parentElement();
      getPlotParent(plot_parent);

      c_min = static_cast<double>(plot_parent->getAttribute("_c_lim_min"));
      c_max = static_cast<double>(plot_parent->getAttribute("_c_lim_max"));

      for (i = 0; i < x_length; i++)
        {
          c_index = 989;
          if (i < c_length) c_index = 1000 + static_cast<int>(255.0 * (c_vec[i] - c_min) / (c_max - c_min) + 0.5);

          marker_c_vec.push_back(static_cast<int>(c_index));
        }
    }

  auto id_int = static_cast<int>(global_root->getAttribute("_id"));
  global_root->setAttribute("_id", ++id_int);
  auto id = std::to_string(id_int);

  if (!marker_c_vec.empty())
    {
      global_render->setMarkerColorInd(element, "marker_color_indices" + id, marker_c_vec);
    }
  else if (element->hasAttribute("marker_color_ind"))
    {
      global_render->setMarkerColorInd(element, (int)c_index);
    }

  /* clear old marker */
  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
    {
      marker = global_creator->createPolymarker3d("x" + id, x_vec, "y" + id, y_vec, "z" + id, z_vec);
      marker->setAttribute("_child_id", child_id++);
      element->append(marker);
    }
  else
    {
      marker = element->querySelectors("polymarker_3d[_child_id=" + std::to_string(child_id++) + "]");
      if (marker != nullptr)
        global_creator->createPolymarker3d("x" + id, x_vec, "y" + id, y_vec, "z" + id, z_vec, nullptr, marker);
    }
  if (marker != nullptr)
    {
      if (element->hasAttribute("_hidden"))
        marker->setAttribute("_hidden", true);
      else
        marker->removeAttribute("_hidden");
    }
}

void processStairs(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for stairs
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  std::string orientation = PLOT_DEFAULT_ORIENTATION, line_spec = SERIES_DEFAULT_SPEC;
  bool is_vertical;
  unsigned int x_length, y_length, mask, i;
  std::vector<double> x_vec, y_vec;
  std::shared_ptr<GRM::Element> element_context = element, line, marker;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  int child_id = 0;
  auto global_root = grm_get_document_root();
  auto global_creator = grm_get_creator();
  auto global_render = grm_get_render();

  if (element->parentElement()->parentElement()->hasAttribute("marginal_heatmap_side_plot"))
    element_context = element->parentElement()->parentElement()->parentElement();

  if (!element_context->hasAttribute("x")) throw NotFoundError("Stairs series is missing required attribute x-data.\n");
  auto x = static_cast<std::string>(element_context->getAttribute("x"));
  if (!element_context->hasAttribute("y")) throw NotFoundError("Stairs series is missing required attribute y-data.\n");
  auto y = static_cast<std::string>(element_context->getAttribute("y"));

  x_vec = GRM::get<std::vector<double>>((*context)[x]);
  y_vec = GRM::get<std::vector<double>>((*context)[y]);
  x_length = x_vec.size();
  y_length = y_vec.size();

  if (element->parentElement()->hasAttribute("orientation"))
    orientation = static_cast<std::string>(element->parentElement()->getAttribute("orientation"));
  if (element->hasAttribute("line_spec"))
    {
      line_spec = static_cast<std::string>(element->getAttribute("line_spec"));
    }
  else
    {
      element->setAttribute("line_spec", line_spec);
    }
  is_vertical = orientation == "vertical";

  auto id = static_cast<int>(global_root->getAttribute("_id"));
  auto str = std::to_string(id);
  global_root->setAttribute("_id", id + 1);

  /* clear old marker and lines */
  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  if (element->parentElement()->parentElement()->hasAttribute("marginal_heatmap_side_plot"))
    {
      int x_offset = 0, y_offset = 0;

      if (element_context->parentElement()->hasAttribute("x_log") &&
          static_cast<int>(element_context->parentElement()->getAttribute("x_log")))
        {
          for (i = 0; i < x_length; i++)
            {
              if (grm_isnan(x_vec[i])) x_offset += 1;
            }
        }
      if (element_context->parentElement()->hasAttribute("y_log") &&
          static_cast<int>(element_context->parentElement()->getAttribute("y_log")))
        {
          for (i = 0; i < y_length; i++)
            {
              if (grm_isnan(y_vec[i])) y_offset += 1;
            }
        }

      processMarginalHeatmapSidePlot(element->parentElement()->parentElement());
      processMarginalHeatmapKind(element_context);
    }
  else
    {
      if (x_length != y_length) throw std::length_error("For stairs series x- and y-data must have the same size.\n");

      const char *spec_char = line_spec.c_str();
      mask = gr_uselinespec((char *)spec_char);

      if (intEqualsAny(static_cast<int>(mask), 5, 0, 1, 3, 4, 5))
        {
          std::string where = PLOT_DEFAULT_STEP_WHERE;
          int current_line_color_ind;
          gr_inqlinecolorind(&current_line_color_ind);
          if (element->hasAttribute("line_color_ind"))
            current_line_color_ind = static_cast<int>(element->getAttribute("line_color_ind"));
          else
            element->setAttribute("line_color_ind", current_line_color_ind);
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

              std::vector<double> line_x = x_step_boundaries, line_y = y_step_values;
              if (is_vertical) line_x = y_step_values, line_y = x_step_boundaries;
              if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
                {
                  line = global_creator->createPolyline("x" + str, line_x, "y" + str, line_y);
                  line->setAttribute("_child_id", child_id++);
                  element->append(line);
                }
              else
                {
                  line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
                  if (line != nullptr)
                    global_creator->createPolyline("x" + str, line_x, "y" + str, line_y, nullptr, 0, 0.0, 0, line);
                }
              if (line != nullptr)
                {
                  if (element->hasAttribute("_hidden"))
                    line->setAttribute("_hidden", true);
                  else
                    line->removeAttribute("_hidden");
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

              std::vector<double> line_x = x_step_boundaries, line_y = y_step_values;
              if (is_vertical) line_x = y_step_values, line_y = x_step_boundaries;
              if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
                {
                  line = global_creator->createPolyline("x" + str, line_x, "y" + str, line_y);
                  line->setAttribute("_child_id", child_id++);
                  element->append(line);
                }
              else
                {
                  line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
                  if (line != nullptr)
                    global_creator->createPolyline("x" + str, line_x, "y" + str, line_y, nullptr, 0, 0.0, 0, line);
                }
              if (line != nullptr)
                {
                  if (element->hasAttribute("_hidden"))
                    line->setAttribute("_hidden", true);
                  else
                    line->removeAttribute("_hidden");
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

              std::vector<double> line_x = x_step_boundaries, line_y = y_step_values;
              if (is_vertical) line_x = y_step_values, line_y = x_step_boundaries;
              if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
                {
                  line = global_creator->createPolyline("x" + str, line_x, "y" + str, line_y);
                  line->setAttribute("_child_id", child_id++);
                  element->append(line);
                }
              else
                {
                  line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
                  if (line != nullptr)
                    global_creator->createPolyline("x" + str, line_x, "y" + str, line_y, nullptr, 0, 0.0, 0, line);
                }
            }
          if (line != nullptr)
            {
              if (!line->hasAttribute("_line_color_ind_set_by_user"))
                line->setAttribute("line_color_ind", current_line_color_ind);
              if (element->hasAttribute("_hidden"))
                line->setAttribute("_hidden", true);
              else
                line->removeAttribute("_hidden");
            }
        }
      if (mask & 2)
        {
          id = static_cast<int>(global_root->getAttribute("_id"));
          str = std::to_string(id);
          global_root->setAttribute("_id", id + 1);

          std::vector<double> marker_x = x_vec, marker_y = y_vec;
          int current_marker_color_ind;
          gr_inqmarkercolorind(&current_marker_color_ind);
          if (element->hasAttribute("line_color_ind"))
            current_marker_color_ind = static_cast<int>(element->getAttribute("line_color_ind"));
          else
            element->setAttribute("line_color_ind", current_marker_color_ind);
          if (is_vertical) marker_x = y_vec, marker_y = x_vec;
          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              marker = global_creator->createPolymarker("x" + str, marker_x, "y" + str, marker_y);
              marker->setAttribute("_child_id", child_id++);
              element->append(marker);
            }
          else
            {
              marker = element->querySelectors("polymarker[_child_id=" + std::to_string(child_id++) + "]");
              if (marker != nullptr)
                global_creator->createPolymarker("x" + str, marker_x, "y" + str, marker_y, nullptr, 0, 0.0, 0, marker);
            }
          if (marker != nullptr)
            {
              if (!marker->hasAttribute("_marker_color_ind_set_by_user"))
                marker->setAttribute("marker_color_ind", current_marker_color_ind);
              marker->setAttribute("z_index", 2);

              if (!marker->hasAttribute("_marker_type_set_by_user"))
                {
                  if (element->hasAttribute("marker_type"))
                    {
                      marker->setAttribute("marker_type", static_cast<int>(element->getAttribute("marker_type")));
                    }
                  else
                    {
                      auto previous_line_marker_type = global_render->getPreviousLineMarkerType();
                      marker->setAttribute("marker_type", *previous_line_marker_type++);
                      if (*previous_line_marker_type == INT_MAX) previous_line_marker_type = plot_scatter_markertypes;
                      global_render->setPreviousLineMarkerType(previous_line_marker_type);
                    }
                }

              if (element->hasAttribute("_hidden"))
                marker->setAttribute("_hidden", true);
              else
                marker->removeAttribute("_hidden");
            }
        }
    }
}

void processStem(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for stem
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double stem_x[2], stem_y[2] = {0.0};
  std::string orientation = PLOT_DEFAULT_ORIENTATION, line_spec = SERIES_DEFAULT_SPEC;
  bool is_vertical;
  unsigned int x_length, y_length;
  unsigned int i;
  std::vector<double> x_vec, y_vec;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  int child_id = 0;
  std::shared_ptr<GRM::Element> line, marker, coordinate_system;
  double old_stem_y0;
  int mask, current_line_color_ind;
  auto global_creator = grm_get_creator();
  auto global_root = grm_get_document_root();

  if (!element->hasAttribute("x")) throw NotFoundError("Stem series is missing required attribute x-data.\n");
  auto x = static_cast<std::string>(element->getAttribute("x"));
  if (!element->hasAttribute("y")) throw NotFoundError("Stem series is missing required attribute y-data.\n");
  auto y = static_cast<std::string>(element->getAttribute("y"));
  if (element->parentElement()->hasAttribute("orientation"))
    orientation = static_cast<std::string>(element->parentElement()->getAttribute("orientation"));
  if (element->hasAttribute("line_spec"))
    {
      line_spec = static_cast<std::string>(element->getAttribute("line_spec"));
    }
  else
    {
      element->setAttribute("line_spec", line_spec);
    }
  const char *spec_char = line_spec.c_str();
  mask = gr_uselinespec((char *)spec_char);

  is_vertical = orientation == "vertical";
  x_vec = GRM::get<std::vector<double>>((*context)[x]);
  y_vec = GRM::get<std::vector<double>>((*context)[y]);
  x_length = x_vec.size();
  y_length = y_vec.size();
  if (x_length != y_length) throw std::length_error("For stem series x- and y-data must have the same size.\n");

  coordinate_system = element->parentElement()->querySelectors("coordinate_system");
  if (coordinate_system != nullptr && coordinate_system->hasAttribute("y_line"))
    {
      auto y_line = coordinate_system->querySelectors("polyline[name=\"y_line\"]");
      if (y_line != nullptr)
        stem_y[0] = static_cast<double>(y_line->getAttribute(orientation == "horizontal" ? "y1" : "x1"));
    }

  /* clear all old polylines and -marker */
  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  old_stem_y0 = stem_y[0];
  for (i = 0; i < x_length; ++i)
    {
      stem_x[0] = stem_x[1] = x_vec[i];
      stem_y[0] = old_stem_y0;
      stem_y[1] = y_vec[i];

      if (is_vertical)
        {
          double tmp1, tmp2;
          tmp1 = stem_x[0], tmp2 = stem_x[1];
          stem_x[0] = stem_y[0], stem_x[1] = stem_y[1];
          stem_y[0] = tmp1, stem_y[1] = tmp2;
        }
      if (intEqualsAny(mask, 5, 0, 1, 3, 4, 5))
        {
          gr_inqlinecolorind(&current_line_color_ind);
          if (element->hasAttribute("line_color_ind"))
            current_line_color_ind = static_cast<int>(element->getAttribute("line_color_ind"));
          else
            element->setAttribute("line_color_ind", current_line_color_ind);
          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              line = global_creator->createPolyline(stem_x[0], stem_x[1], stem_y[0], stem_y[1]);
              line->setAttribute("_child_id", child_id++);
              element->append(line);
            }
          else
            {
              line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
              if (line != nullptr)
                global_creator->createPolyline(stem_x[0], stem_x[1], stem_y[0], stem_y[1], 0, 0.0, 0, line);
            }
          if (line != nullptr)
            {
              if (!line->hasAttribute("_line_color_ind_set_by_user"))
                line->setAttribute("line_color_ind", current_line_color_ind);
              if (element->hasAttribute("_hidden"))
                line->setAttribute("_hidden", true);
              else
                line->removeAttribute("_hidden");
            }
        }
    }

  auto id = static_cast<int>(global_root->getAttribute("_id"));
  global_root->setAttribute("_id", id + 1);
  auto str = std::to_string(id);

  std::vector<double> marker_x = x_vec, marker_y = y_vec;
  if (is_vertical) marker_x = y_vec, marker_y = x_vec;
  if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
    {
      marker = global_creator->createPolymarker("x" + str, marker_x, "y" + str, marker_y, nullptr,
                                                GKS_K_MARKERTYPE_SOLID_CIRCLE);
      marker->setAttribute("_child_id", child_id++);
      element->append(marker);
    }
  else
    {
      marker = element->querySelectors("polymarker[_child_id=" + std::to_string(child_id++) + "]");
      if (marker != nullptr)
        global_creator->createPolymarker("x" + str, marker_x, "y" + str, marker_y, nullptr,
                                         GKS_K_MARKERTYPE_SOLID_CIRCLE, 0.0, 0, marker);
    }
  if (marker != nullptr)
    {
      marker->setAttribute("z_index", 2);
      if (!marker->hasAttribute("_marker_color_ind_set_by_user"))
        marker->setAttribute("marker_color_ind", current_line_color_ind);
      if (element->hasAttribute("_hidden"))
        marker->setAttribute("_hidden", true);
      else
        marker->removeAttribute("_hidden");
    }
}

void processShade(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  int xform = 5, x_bins = 1200, y_bins = 1200, n;
  double *x_p, *y_p;
  std::string orientation = PLOT_DEFAULT_ORIENTATION;

  if (element->parentElement()->hasAttribute("orientation"))
    orientation = static_cast<std::string>(element->parentElement()->getAttribute("orientation"));

  auto x_key = static_cast<std::string>(element->getAttribute("x"));
  auto y_key = static_cast<std::string>(element->getAttribute("y"));
  auto x_vec = GRM::get<std::vector<double>>((*context)[x_key]);
  auto y_vec = GRM::get<std::vector<double>>((*context)[y_key]);

  if (element->hasAttribute("transformation")) xform = static_cast<int>(element->getAttribute("transformation"));
  if (element->hasAttribute("x_bins")) x_bins = static_cast<int>(element->getAttribute("x_bins"));
  if (element->hasAttribute("y_bins")) y_bins = static_cast<int>(element->getAttribute("y_bins"));

  x_p = &(x_vec[0]);
  y_p = &(y_vec[0]);
  n = std::min<int>(static_cast<int>(x_vec.size()), static_cast<int>(y_vec.size()));
  applyMoveTransformation(element);

  if (orientation == "vertical")
    {
      auto tmp = x_p;
      x_p = y_p;
      y_p = tmp;
      auto tmp2 = x_bins;
      x_bins = y_bins;
      y_bins = tmp2;
    }
  processColormap(element->parentElement()->parentElement());
  if (grm_get_render()->getRedrawWs()) gr_shadepoints(n, x_p, y_p, xform, x_bins, y_bins);
}

void processSurface(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for surface
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  int use_gr3 = PLOT_DEFAULT_USE_GR3; /* this argument decides if GR3 or GR is used to plot the surface */
  std::vector<double> x_vec, y_vec, z_vec;
  unsigned int x_length, y_length, z_length;
  double x_min, x_max, y_min, y_max;
  auto redraw_ws = grm_get_render()->getRedrawWs();

  if (element->hasAttribute("use_gr3"))
    {
      use_gr3 = static_cast<int>(element->getAttribute("use_gr3"));
    }
  else
    {
      element->setAttribute("use_gr3", use_gr3);
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
      if (!element->hasAttribute("z_dims"))
        throw NotFoundError("Surface series is missing required attribute z_dims.\n");
      auto z_dims_key = static_cast<std::string>(element->getAttribute("z_dims"));
      auto z_dims_vec = GRM::get<std::vector<int>>((*context)[z_dims_key]);
      x_length = z_dims_vec[0];
      y_length = z_dims_vec[1];
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
      x_min = static_cast<double>(element->getAttribute("x_range_min"));
      x_max = static_cast<double>(element->getAttribute("x_range_max"));
    }
  else
    {
      for (int j = 0; j < x_length; j++)
        {
          if (!grm_isnan(x_vec[j]))
            {
              x_min = x_vec[j];
              break;
            }
        }
      x_max = x_vec[x_length - 1];
    }
  if (y_vec.empty())
    {
      y_min = static_cast<double>(element->getAttribute("y_range_min"));
      y_max = static_cast<double>(element->getAttribute("y_range_max"));
    }
  else
    {
      for (int j = 0; j < y_length; j++)
        {
          if (!grm_isnan(y_vec[j]))
            {
              y_min = y_vec[j];
              break;
            }
        }
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
      gr_gridit(static_cast<int>(x_length), x_p, y_p, z_p, PLOT_SURFACE_GRIDIT_N, PLOT_SURFACE_GRIDIT_N, gridit_x,
                gridit_y, gridit_z);

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

  applyMoveTransformation(element);
  processSpace3d(element->parentElement());
  processColormap(element->parentElement()->parentElement());
  if (!use_gr3)
    {
      double *px_p = &(x_vec[0]);
      double *py_p = &(y_vec[0]);
      double *pz_p = &(z_vec[0]);

      if (redraw_ws) gr_surface((int)x_length, (int)y_length, px_p, py_p, pz_p, GR_OPTION_COLORED_MESH);
    }
  else
    {
      std::vector<float> px_vec_f(x_vec.begin(), x_vec.end());
      std::vector<float> py_vec_f(y_vec.begin(), y_vec.end());
      std::vector<float> pz_vec_f(z_vec.begin(), z_vec.end());

      float *px_p = &(px_vec_f[0]);
      float *py_p = &(py_vec_f[0]);
      float *pz_p = &(pz_vec_f[0]);

      if (redraw_ws) gr3_surface((int)x_length, (int)y_length, px_p, py_p, pz_p, GR_OPTION_COLORED_MESH);
    }
}

void processLine(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for line
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  std::string orientation = PLOT_DEFAULT_ORIENTATION, line_spec = SERIES_DEFAULT_SPEC;
  std::vector<double> x_vec, y_vec;
  unsigned int x_length = 0, y_length = 0;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  int child_id = 0;
  std::shared_ptr<GRM::Element> line, marker;
  int mask;
  auto global_root = grm_get_document_root();
  auto global_creator = grm_get_creator();
  auto global_render = grm_get_render();

  if (element->parentElement()->hasAttribute("orientation"))
    orientation = static_cast<std::string>(element->parentElement()->getAttribute("orientation"));

  if (!element->hasAttribute("y")) throw NotFoundError("Line series is missing required attribute y-data.\n");
  auto y = static_cast<std::string>(element->getAttribute("y"));
  y_vec = GRM::get<std::vector<double>>((*context)[y]);
  y_length = y_vec.size();

  if (!element->hasAttribute("x"))
    {
      x_length = y_length;
      for (int i = 0; i < y_length; ++i) /* julia starts with 1, so GRM starts with 1 to be consistent */
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

  if (element->hasAttribute("line_spec"))
    {
      line_spec = static_cast<std::string>(element->getAttribute("line_spec"));
    }
  else
    {
      element->setAttribute("line_spec", line_spec);
    }
  const char *spec_char = line_spec.c_str();
  mask = gr_uselinespec((char *)spec_char);

  /* clear old line */
  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  if (intEqualsAny(mask, 5, 0, 1, 3, 4, 5))
    {
      int current_line_color_ind;
      gr_inqlinecolorind(&current_line_color_ind);
      if (element->hasAttribute("_line_color_ind_set_by_user"))
        current_line_color_ind = static_cast<int>(element->getAttribute("_line_color_ind_set_by_user"));
      else if (element->hasAttribute("line_color_ind"))
        current_line_color_ind = static_cast<int>(element->getAttribute("line_color_ind"));
      else
        element->setAttribute("line_color_ind", current_line_color_ind);
      auto id = static_cast<int>(global_root->getAttribute("_id"));
      auto str = std::to_string(id);

      std::vector<double> line_x = x_vec, line_y = y_vec;
      if (orientation == "vertical") line_x = y_vec, line_y = x_vec;
      if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
        {
          line = global_creator->createPolyline("x" + str, line_x, "y" + str, line_y);
          line->setAttribute("_child_id", child_id++);
          element->append(line);
        }
      else
        {
          line = element->querySelectors("polyline[_child_id=" + std::to_string(child_id++) + "]");
          if (line != nullptr)
            global_creator->createPolyline("x" + str, line_x, "y" + str, line_y, nullptr, 0, 0.0, 0, line);
        }
      if (line != nullptr)
        {
          if (!line->hasAttribute("_line_width_set_by_user") && element->hasAttribute("line_width"))
            line->setAttribute("line_width", static_cast<double>(element->getAttribute("line_width")));
          if (element->hasAttribute("_hidden"))
            line->setAttribute("_hidden", true);
          else
            line->removeAttribute("_hidden");
        }

      global_root->setAttribute("_id", ++id);
      if (line != nullptr) line->setAttribute("line_color_ind", current_line_color_ind);
    }
  if (mask & 2)
    {
      int current_marker_color_ind;
      gr_inqmarkercolorind(&current_marker_color_ind);
      if (element->hasAttribute("marker_color_ind"))
        current_marker_color_ind = static_cast<int>(element->getAttribute("marker_color_ind"));
      else
        element->setAttribute("marker_color_ind", current_marker_color_ind);
      auto id = static_cast<int>(global_root->getAttribute("_id"));
      auto str = std::to_string(id);

      std::vector<double> marker_x = x_vec, marker_y = y_vec;
      if (orientation == "vertical") marker_x = y_vec, marker_y = x_vec;
      if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
        {
          marker = global_creator->createPolymarker("x" + str, marker_x, "y" + str, marker_y);
          marker->setAttribute("_child_id", child_id++);
          element->append(marker);
        }
      else
        {
          marker = element->querySelectors("polymarker[_child_id=" + std::to_string(child_id++) + "]");
          if (marker != nullptr)
            global_creator->createPolymarker("x" + str, marker_x, "y" + str, marker_y, nullptr, 0, 0.0, 0, marker);
        }

      if (marker != nullptr)
        {
          marker->setAttribute("marker_color_ind", current_marker_color_ind);
          marker->setAttribute("z_index", 2);

          if (element->hasAttribute("marker_type"))
            {
              marker->setAttribute("marker_type", static_cast<int>(element->getAttribute("marker_type")));
            }
          else
            {
              auto previous_line_marker_type = global_render->getPreviousLineMarkerType();
              marker->setAttribute("marker_type", *previous_line_marker_type++);
              if (*previous_line_marker_type == INT_MAX) previous_line_marker_type = plot_scatter_markertypes;
              global_render->setPreviousLineMarkerType(previous_line_marker_type);
            }
          if (element->hasAttribute("_hidden"))
            marker->setAttribute("_hidden", true);
          else
            marker->removeAttribute("_hidden");
        }
      global_root->setAttribute("_id", ++id);
    }

  // error_bar handling
  for (const auto &child : element->children())
    {
      if (child->localName() == "error_bars") extendErrorBars(child, context, x_vec, y_vec);
    }
}

void processMarginalHeatmapPlot(const std::shared_ptr<GRM::Element> &element,
                                const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for marginal heatmap
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double c_max;
  int x_ind = PLOT_DEFAULT_MARGINAL_INDEX, y_ind = PLOT_DEFAULT_MARGINAL_INDEX;
  unsigned int i, j, k;
  std::string algorithm = PLOT_DEFAULT_MARGINAL_ALGORITHM, marginal_heatmap_kind = PLOT_DEFAULT_MARGINAL_KIND;
  std::vector<double> bins;
  unsigned int num_bins_x = 0, num_bins_y = 0;
  std::shared_ptr<GRM::Element> sub_group, central_region, side_region;
  auto plot_parent = element;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  int child_id = 0;
  bool z_log = false;
  int x_offset = 0, y_offset = 0;
  auto global_root = grm_get_document_root();
  auto global_creator = grm_get_creator();

  getPlotParent(plot_parent);
  for (const auto &children : element->children())
    {
      if (children->localName() == "central_region")
        {
          central_region = children;
          break;
        }
    }

  z_log = static_cast<int>(plot_parent->getAttribute("z_log"));

  if (element->hasAttribute("x_ind"))
    {
      x_ind = static_cast<int>(element->getAttribute("x_ind"));
    }
  else
    {
      element->setAttribute("x_ind", x_ind);
    }
  if (element->hasAttribute("y_ind"))
    {
      y_ind = static_cast<int>(element->getAttribute("y_ind"));
    }
  else
    {
      element->setAttribute("y_ind", y_ind);
    }
  if (element->hasAttribute("algorithm"))
    {
      algorithm = static_cast<std::string>(element->getAttribute("algorithm"));
    }
  else
    {
      element->setAttribute("algorithm", algorithm);
    }
  if (element->hasAttribute("marginal_heatmap_kind"))
    {
      marginal_heatmap_kind = static_cast<std::string>(element->getAttribute("marginal_heatmap_kind"));
    }
  else
    {
      element->setAttribute("marginal_heatmap_kind", marginal_heatmap_kind);
    }

  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto x_vec = GRM::get<std::vector<double>>((*context)[x]);
  num_bins_x = x_vec.size();
  if (plot_parent->hasAttribute("x_log") && static_cast<int>(plot_parent->getAttribute("x_log")))
    {
      for (i = 0; i < num_bins_x; i++)
        {
          if (grm_isnan(x_vec[i])) x_offset += 1;
        }
    }
  num_bins_x -= x_offset;

  auto y = static_cast<std::string>(element->getAttribute("y"));
  auto y_vec = std::vector<double>(GRM::get<std::vector<double>>((*context)[y]));
  num_bins_y = y_vec.size();
  if (plot_parent->hasAttribute("y_log") && static_cast<int>(plot_parent->getAttribute("y_log")))
    {
      for (i = 0; i < num_bins_y; i++)
        {
          if (grm_isnan(y_vec[i])) y_offset += 1;
        }
    }
  num_bins_y -= y_offset;

  auto plot = static_cast<std::string>(element->getAttribute("z"));
  auto plot_vec = GRM::get<std::vector<double>>((*context)[plot]);

  /* clear old child nodes */
  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  auto id = static_cast<int>(global_root->getAttribute("_id"));
  auto str = std::to_string(id);

  std::shared_ptr<GRM::Element> heatmap;
  if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
    {
      heatmap = global_creator->createSeries("heatmap");
      heatmap->setAttribute("_child_id", child_id++);
      central_region->append(heatmap);
    }
  else
    {
      heatmap = element->querySelectors("series_heatmap[_child_id=\"" + std::to_string(child_id++) + "\"]");
    }

  // for validation
  if (heatmap != nullptr)
    {
      (*context)["x" + str] = x_vec;
      heatmap->setAttribute("x", "x" + str);
      (*context)["y" + str] = y_vec;
      heatmap->setAttribute("y", "y" + str);
      (*context)["z" + str] = plot_vec;
      heatmap->setAttribute("z", "z" + str);
    }

  global_root->setAttribute("_id", ++id);

  for (k = 0; k < 2; k++)
    {
      double value, bin_max = 0;
      int bar_color_index = 989;
      double bar_color_rgb[3] = {-1};
      int edge_color_index = 1;
      double edge_color_rgb[3] = {-1};

      id = static_cast<int>(global_root->getAttribute("_id"));
      str = std::to_string(id);

      if (!std::isnan(static_cast<double>(plot_parent->getAttribute("_c_lim_max"))))
        {
          c_max = static_cast<double>(plot_parent->getAttribute("_c_lim_max"));
        }
      else
        {
          c_max = static_cast<double>(plot_parent->getAttribute("_z_lim_max"));
        }

      if (marginal_heatmap_kind == "all")
        {
          unsigned int x_len = num_bins_x, y_len = num_bins_y;

          bins = std::vector<double>((k == 0) ? num_bins_y : num_bins_x);

          for (i = 0; i < ((k == 0) ? num_bins_y : num_bins_x); i++) bins[i] = 0;
          for (i = 0; i < y_len; i++)
            {
              for (j = 0; j < x_len; j++)
                {
                  if (z_log)
                    {
                      value = (grm_isnan(log10(plot_vec[(i + y_offset) * (num_bins_x + x_offset) + (j + x_offset)])))
                                  ? 0
                                  : log10(plot_vec[(i + y_offset) * (num_bins_x + x_offset) + (j + x_offset)]);
                    }
                  else
                    {
                      value = (grm_isnan(plot_vec[(i + y_offset) * (num_bins_x + x_offset) + (j + x_offset)]))
                                  ? 0
                                  : plot_vec[(i + y_offset) * (num_bins_x + x_offset) + (j + x_offset)];
                    }
                  if (algorithm == "sum")
                    {
                      bins[(k == 0) ? i : j] += value;
                    }
                  else if (algorithm == "max")
                    {
                      bins[(k == 0) ? i : j] = grm_max(bins[(k == 0) ? i : j], value);
                    }
                }
              if (k == 0) bin_max = grm_max(bin_max, bins[i]);
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
              // + 0.01 and 0.5 to prevent line clipping, gathered through testing
              bins[i] = ((bin_max == 0 ? 0.0 : bins[i]) / bin_max + 0.01) * (c_max / (15 + 0.5));
            }

          side_region = element->querySelectors("side_region[location=\"" +
                                                (k == 0 ? std::string("right") : std::string("top")) + "\"]");
          if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
            {
              sub_group = global_creator->createSeries("histogram");
              sub_group->setAttribute("_child_id", child_id++);
              auto side_plot_region = global_creator->createSidePlotRegion();
              side_region->append(side_plot_region);
              side_plot_region->append(sub_group);
            }
          else
            {
              sub_group =
                  side_region->querySelectors("series_histogram[_child_id=\"" + std::to_string(child_id++) + "\"]");
              sub_group->setAttribute("_update_required", true);
            }
          if (side_region != nullptr) side_region->setAttribute("marginal_heatmap_side_plot", true);

          if (sub_group != nullptr)
            {
              std::vector<double> bar_color_rgb_vec(bar_color_rgb, bar_color_rgb + 3);
              (*context)["fill_color_rgb" + str] = bar_color_rgb_vec;
              sub_group->setAttribute("fill_color_rgb", "fill_color_rgb" + str);
              sub_group->setAttribute("fill_color_ind", bar_color_index);

              std::vector<double> edge_color_rgb_vec(edge_color_rgb, edge_color_rgb + 3);
              (*context)["line_color_rgb" + str] = edge_color_rgb_vec;
              sub_group->setAttribute("line_color_rgb", "line_color_rgb" + str);
              sub_group->setAttribute("line_color_ind", edge_color_index);

              (*context)["bins" + str] = bins;
              sub_group->setAttribute("bins", "bins" + str);

              (*context)["x" + str] = x_vec;
              sub_group->setAttribute("x", "x" + str);
            }
        }
      else if (marginal_heatmap_kind == "line" && x_ind != -1 && y_ind != -1)
        {
          side_region = element->querySelectors("side_region[location=\"" +
                                                (k == 0 ? std::string("right") : std::string("top")) + "\"]");
          if (side_region != nullptr) side_region->setAttribute("marginal_heatmap_side_plot", true);
          // special case for marginal_heatmap_kind line - when new indices != -1 are received the 2 lines should be
          // displayed
          sub_group = side_region->querySelectors("series_stairs[_child_id=\"" + std::to_string(child_id) + "\"]");
          auto side_plot_region = side_region->querySelectors("side_plot_region");
          if ((del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT) ||
              (sub_group == nullptr && static_cast<int>(element->getAttribute("_update_required"))))
            {
              sub_group = global_creator->createSeries("stairs");
              sub_group->setAttribute("_child_id", child_id++);
              if (!side_plot_region) side_plot_region = global_creator->createSidePlotRegion();
              side_region->append(side_plot_region);
              side_plot_region->append(sub_group);
            }
          else
            {
              child_id++;
            }

          if (sub_group != nullptr)
            {
              sub_group->setAttribute("line_spec", SERIES_DEFAULT_SPEC);

              // for validation
              (*context)["y" + str] = y_vec;
              sub_group->setAttribute("y", "y" + str);

              (*context)["x" + str] = x_vec;
              sub_group->setAttribute("x", "x" + str);
            }
        }

      if (marginal_heatmap_kind == "all" || (marginal_heatmap_kind == "line" && x_ind != -1 && y_ind != -1))
        {
          if (sub_group != nullptr)
            {
              if (k == 0)
                {
                  sub_group->parentElement()->setAttribute("orientation", "vertical");
                }
              else
                {
                  sub_group->parentElement()->setAttribute("orientation", "horizontal");
                }
            }
        }

      auto tmp = element->querySelectorsAll("series_histogram");
      for (const auto &child : tmp)
        {
          if (!child->parentElement()->parentElement()->hasAttribute("marginal_heatmap_side_plot")) continue;
          if (static_cast<std::string>(child->getAttribute("kind")) == "histogram" ||
              static_cast<std::string>(child->getAttribute("kind")) == "stairs")
            {
              child->parentElement()->setAttribute("y_flip", 0);
              child->parentElement()->setAttribute("x_flip", 0);
              if (static_cast<int>(child->getAttribute("_child_id")) == 1)
                {
                  child->parentElement()->setAttribute("x_flip", 0);
                  if (static_cast<int>(plot_parent->getAttribute("y_flip")) == 1)
                    child->parentElement()->setAttribute("y_flip", 1);
                  processFlip(child->parentElement());
                }
              if (static_cast<int>(child->getAttribute("_child_id")) == 2)
                {
                  if (static_cast<int>(plot_parent->getAttribute("x_flip")) == 1)
                    child->parentElement()->setAttribute("x_flip", 1);
                  child->parentElement()->setAttribute("y_flip", 0);
                  processFlip(child->parentElement());
                }
            }
        }
      global_root->setAttribute("_id", ++id);
      processFlip(plot_parent);
    }
}

void processPie(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for pie
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  unsigned int x_length;
  int color_index;
  double start_angle, end_angle;
  char text[80];
  std::string title;
  unsigned int i;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  int child_id = 0;
  std::shared_ptr<GRM::Element> pie_segment;
  auto global_render = grm_get_render();
  auto global_creator = grm_get_creator();

  if (!element->hasAttribute("fill_int_style")) global_render->setFillIntStyle(element, GKS_K_INTSTYLE_SOLID);
  if (!element->hasAttribute("text_align_vertical"))
    element->setAttribute("text_align_vertical", GKS_K_TEXT_VALIGN_HALF);
  if (!element->hasAttribute("text_align_horizontal"))
    element->setAttribute("text_align_horizontal", GKS_K_TEXT_HALIGN_CENTER);

  if (!element->hasAttribute("x")) throw NotFoundError("Pie series is missing required attribute x-data.\n");
  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto x_vec = GRM::get<std::vector<double>>((*context)[x]);
  x_length = x_vec.size();

  std::vector<double> normalized_x(x_length);
  GRM::normalizeVec(x_vec, &normalized_x);
  std::vector<unsigned int> normalized_x_int(x_length);
  GRM::normalizeVecInt(x_vec, &normalized_x_int, 1000);

  start_angle = 90;
  color_index = setNextColor("c", GR_COLOR_FILL, element, context); // key doesn't matter as long as it's not empty

  /* clear old pie_segments */
  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  for (i = 0; i < x_length; ++i)
    {
      end_angle = start_angle - normalized_x[i] * 360.0;

      snprintf(text, 80, "%.2lf\n%.1lf %%", x_vec[i], normalized_x_int[i] / 10.0);
      if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
        {
          pie_segment = global_creator->createPieSegment(start_angle, end_angle, text, color_index);
          pie_segment->setAttribute("_child_id", child_id++);
          element->append(pie_segment);
        }
      else
        {
          pie_segment = element->querySelectors("pie_segment[_child_id=" + std::to_string(child_id++) + "]");
          if (pie_segment != nullptr)
            global_creator->createPieSegment(start_angle, end_angle, text, color_index, pie_segment);
        }
      if (pie_segment != nullptr)
        {
          color_index = setNextColor("", GR_COLOR_FILL, pie_segment, context);
          processFillColorInd(pie_segment);
        }

      start_angle = end_angle;
      if (start_angle < 0) start_angle += 360.0;
    }
  setNextColor("", GR_COLOR_RESET, element, context);
  processFillColorInd(element);
  processFillIntStyle(element);
  processTextAlign(element);
}

void processLine3(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for line3
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  unsigned int x_length, y_length, z_length;
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  int child_id = 0;
  auto global_root = grm_get_document_root();
  auto global_creator = grm_get_creator();

  if (!element->hasAttribute("x")) throw NotFoundError("Line3 series is missing required attribute x-data.\n");
  auto x = static_cast<std::string>(element->getAttribute("x"));
  auto x_vec = GRM::get<std::vector<double>>((*context)[x]);
  x_length = x_vec.size();

  if (!element->hasAttribute("y")) throw NotFoundError("Line3 series is missing required attribute y-data.\n");
  auto y = static_cast<std::string>(element->getAttribute("y"));
  auto y_vec = GRM::get<std::vector<double>>((*context)[y]);
  y_length = y_vec.size();

  if (!element->hasAttribute("z")) throw NotFoundError("Line3 series is missing required attribute z-data.\n");
  auto z = static_cast<std::string>(element->getAttribute("z"));
  auto z_vec = GRM::get<std::vector<double>>((*context)[z]);
  z_length = z_vec.size();

  if (x_length != y_length || x_length != z_length)
    throw std::length_error("For line3 series x-, y- and z-data must have the same size.\n");

  /* clear old line */
  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  auto id_int = static_cast<int>(global_root->getAttribute("_id"));
  global_root->setAttribute("_id", ++id_int);
  auto id = std::to_string(id_int);

  std::shared_ptr<GRM::Element> line;
  if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
    {
      line = global_creator->createPolyline3d("x" + id, x_vec, "y" + id, y_vec, "z" + id, z_vec);
      line->setAttribute("_child_id", child_id++);
      element->append(line);
    }
  else
    {
      line = element->querySelectors("polyline_3d[_child_id=" + std::to_string(child_id++) + "]");
      if (line != nullptr)
        global_creator->createPolyline3d("x" + id, x_vec, "y" + id, y_vec, "z" + id, z_vec, nullptr, line);
    }
  if (line != nullptr)
    {
      if (!line->hasAttribute("_line_color_ind_set_by_user")) line->setAttribute("line_color_ind", 989);
      if (element->hasAttribute("_hidden"))
        line->setAttribute("_hidden", true);
      else
        line->removeAttribute("_hidden");
    }
}

void processImshow(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for imshow
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  double z_min, z_max;
  unsigned int dims, z_data_length, i, j, k;
  bool use_grplot_changes = false;
  int rows, cols;
  auto plot_parent = element->parentElement(), central_region = element->parentElement();
  DelValues del = DelValues::UPDATE_WITHOUT_DEFAULT;
  int child_id = 0;
  std::vector<double> z_data_vec;
  std::vector<int> z_dims_vec;
  auto global_root = grm_get_document_root();
  auto global_render = grm_get_render();
  auto global_creator = grm_get_creator();

  getPlotParent(plot_parent);
  if (plot_parent->hasAttribute("use_grplot_changes"))
    {
      use_grplot_changes = static_cast<int>(plot_parent->getAttribute("use_grplot_changes"));
    }
  if (std::isnan(static_cast<double>(element->getAttribute("z_range_min"))))
    throw NotFoundError("Imshow series is missing required attribute z_range.\n");
  z_min = static_cast<double>(element->getAttribute("z_range_min"));
  if (std::isnan(static_cast<double>(element->getAttribute("z_range_max"))))
    throw NotFoundError("Imshow series is missing required attribute z_range.\n");
  z_max = static_cast<double>(element->getAttribute("z_range_max"));
  logger((stderr, "Got min, max %lf %lf\n", z_min, z_max));

  if (!element->hasAttribute("z")) throw NotFoundError("Imshow series is missing required attribute z-data.\n");
  auto z_key = static_cast<std::string>(element->getAttribute("z"));
  if (!element->hasAttribute("z_dims"))
    throw NotFoundError("Imshow series is missing required attribute z_dims-data.\n");
  auto z_dims_key = static_cast<std::string>(element->getAttribute("z_dims"));

  z_data_vec = GRM::get<std::vector<double>>((*context)[z_key]);
  z_dims_vec = GRM::get<std::vector<int>>((*context)[z_dims_key]);
  z_data_length = z_data_vec.size();
  dims = z_dims_vec.size();
  if (dims != 2) throw std::length_error("The size of dims data from imshow has to be 2.\n");
  if (z_dims_vec[0] * z_dims_vec[1] != z_data_length)
    throw std::length_error("For imshow shape[0] * shape[1] must be z-data length.\n");

  cols = z_dims_vec[0];
  rows = z_dims_vec[1];

  std::vector<int> img_data(z_data_length);

  k = 0;
  for (j = 0; j < rows; ++j)
    {
      for (i = 0; i < cols; ++i)
        {
          img_data[k++] = 1000 + (int)grm_round((1.0 * z_data_vec[j * cols + i] - z_min) / (z_max - z_min) * 255);
        }
    }

  auto id = static_cast<int>(global_root->getAttribute("_id"));
  global_root->setAttribute("_id", ++id);
  auto str = std::to_string(id);

  std::vector<double> x_vec, y_vec;
  linSpace(0, cols - 1, cols, x_vec);
  linSpace(0, rows - 1, rows, y_vec);

  (*context)["x" + str] = x_vec;
  element->setAttribute("x", "x" + str);
  (*context)["y" + str] = y_vec;
  element->setAttribute("y", "y" + str);
  std::string img_data_key = "data" + str;
  (*context)[img_data_key] = img_data;
  element->setAttribute("data", img_data_key);

  if (!element->hasAttribute("select_specific_xform")) global_render->setSelectSpecificXform(element, 0);
  if (!element->hasAttribute("scale")) global_render->setScale(element, 0);
  processScale(element);
  processSelectSpecificXform(element);

  double x_min, x_max, y_min, y_max;
  int scale;

  if (!GRM::Render::getViewport(central_region, &x_min, &x_max, &y_min, &y_max))
    throw NotFoundError("Central_region doesn't have a viewport but it should.\n");

  gr_inqscale(&scale);

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
  if (use_grplot_changes)
    {
      double tmp = y_min;
      y_min = y_max;
      y_max = tmp;
    }

  /* remove old cell arrays if they exist */
  del = DelValues(static_cast<int>(element->getAttribute("_delete_children")));
  clearOldChildren(&del, element);

  std::shared_ptr<GRM::Element> cell_array;
  if (del != DelValues::UPDATE_WITHOUT_DEFAULT && del != DelValues::UPDATE_WITH_DEFAULT)
    {
      cell_array = global_creator->createCellArray(x_min, x_max, y_min, y_max, cols, rows, 1, 1, cols, rows,
                                                   img_data_key, std::nullopt);
      cell_array->setAttribute("_child_id", child_id++);
      element->append(cell_array);
    }
  else
    {
      cell_array = element->querySelectors("cell_array[_child_id=" + std::to_string(child_id++) + "]");
      if (cell_array != nullptr)
        global_creator->createCellArray(x_min, x_max, y_min, y_max, cols, rows, 1, 1, cols, rows, img_data_key,
                                        std::nullopt, nullptr, cell_array);
    }
  if (cell_array != nullptr) cell_array->setAttribute("name", "imshow");
  processColormap(element->parentElement()->parentElement());
}

void processTriContour(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
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
  auto plot_parent = element->parentElement();
  getPlotParent(plot_parent);

  z_min = static_cast<double>(plot_parent->getAttribute("_z_lim_min"));
  z_max = static_cast<double>(plot_parent->getAttribute("_z_lim_max"));
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

  if (!element->hasAttribute("x")) throw NotFoundError("Tricontour series is missing required attribute px-data.\n");
  auto x = static_cast<std::string>(element->getAttribute("x"));
  if (!element->hasAttribute("y")) throw NotFoundError("Tricontour series is missing required attribute py-data.\n");
  auto y = static_cast<std::string>(element->getAttribute("y"));
  if (!element->hasAttribute("z")) throw NotFoundError("Tricontour series is missing required attribute pz-data.\n");
  auto z = static_cast<std::string>(element->getAttribute("z"));

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
  applyMoveTransformation(element);
  processColormap(element->parentElement()->parentElement());

  if (grm_get_render()->getRedrawWs()) gr_tricontour((int)x_length, px_p, py_p, pz_p, num_levels, l_p);
}

void processTriSurface(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Processing function for trisurface
   *
   * \param[in] element The GRM::Element that contains the attributes and data keys
   * \param[in] context The GRM::Context that contains the actual data
   */
  if (!element->hasAttribute("x")) throw NotFoundError("Trisurface series is missing required attribute px-data.\n");
  auto px = static_cast<std::string>(element->getAttribute("x"));
  if (!element->hasAttribute("y")) throw NotFoundError("Trisurface series is missing required attribute py-data.\n");
  auto py = static_cast<std::string>(element->getAttribute("y"));
  if (!element->hasAttribute("z")) throw NotFoundError("Trisurface series is missing required attribute pz-data.\n");
  auto pz = static_cast<std::string>(element->getAttribute("z"));

  auto px_vec = GRM::get<std::vector<double>>((*context)[px]);
  auto py_vec = GRM::get<std::vector<double>>((*context)[py]);
  auto pz_vec = GRM::get<std::vector<double>>((*context)[pz]);

  auto nx = static_cast<int>(px_vec.size());
  auto ny = static_cast<int>(py_vec.size());
  auto nz = static_cast<int>(pz_vec.size());
  if (nx != ny || nx != nz)
    throw std::length_error("For trisurface series px-, py- and pz-data must have the same size.\n");

  double *px_p = &(px_vec[0]);
  double *py_p = &(py_vec[0]);
  double *pz_p = &(pz_vec[0]);
  applyMoveTransformation(element);
  processSpace3d(element->parentElement());
  processColormap(element->parentElement()->parentElement());

  if (grm_get_render()->getRedrawWs()) gr_trisurface(nx, px_p, py_p, pz_p);
}

static int getVolumeAlgorithm(const std::shared_ptr<GRM::Element> &element)
{
  int algorithm;

  if (element->getAttribute("algorithm").isInt())
    {
      algorithm = static_cast<int>(element->getAttribute("algorithm"));
    }
  else if (element->getAttribute("algorithm").isString())
    {
      auto algorithm_str = static_cast<std::string>(element->getAttribute("algorithm"));
      algorithm = GRM::algorithmStringToInt(algorithm_str);
    }
  else
    {
      throw NotFoundError("Volume series is missing attribute algorithm.\n");
    }
  return algorithm;
}

static void volume(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  int width, height;
  double device_pixel_ratio;
  double d_min = -1, d_max = -1;
  auto global_render = grm_get_render();
  auto redraw_ws = global_render->getRedrawWs();

  auto z_key = static_cast<std::string>(element->getAttribute("z"));
  auto z_vec = GRM::get<std::vector<double>>((*context)[z_key]);
  auto z_dims_key = static_cast<std::string>(element->getAttribute("z_dims"));
  auto z_dims_vec = GRM::get<std::vector<int>>((*context)[z_dims_key]);
  auto algorithm = getVolumeAlgorithm(element);
  if (element->hasAttribute("d_min")) d_min = static_cast<double>(element->getAttribute("d_min"));
  if (element->hasAttribute("d_max")) d_max = static_cast<double>(element->getAttribute("d_max"));

  applyMoveTransformation(element);
  if (redraw_ws)
    {
      gr_inqvpsize(&width, &height, &device_pixel_ratio);
      gr_setpicturesizeforvolume((int)(width * device_pixel_ratio), (int)(height * device_pixel_ratio));
    }
  if (element->hasAttribute("_volume_context_address"))
    {
      auto address = static_cast<std::string>(element->getAttribute("_volume_context_address"));
      long volume_address = stol(address, nullptr, 16);
      const gr3_volume_2pass_t *volume_context = reinterpret_cast<gr3_volume_2pass_t *>(volume_address);
      if (redraw_ws)
        gr_volume_2pass(z_dims_vec[0], z_dims_vec[1], z_dims_vec[2], &(z_vec[0]), algorithm, &d_min, &d_max,
                        volume_context);
      element->removeAttribute("_volume_context_address");
    }
  else
    {
      if (redraw_ws) gr_volume(z_dims_vec[0], z_dims_vec[1], z_dims_vec[2], &(z_vec[0]), algorithm, &d_min, &d_max);
    }
}

void processVolume(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  double dlim[2] = {INFINITY, (double)-INFINITY};
  unsigned int z_length, dims;
  int algorithm = PLOT_DEFAULT_VOLUME_ALGORITHM;
  std::string algorithm_str;
  double d_min, d_max;
  int width, height;
  double device_pixel_ratio;
  auto global_render = grm_get_render();
  auto redraw_ws = global_render->getRedrawWs();

  if (!element->hasAttribute("z")) throw NotFoundError("Volume series is missing required attribute z-data.\n");
  auto z_key = static_cast<std::string>(element->getAttribute("z"));
  auto z_vec = GRM::get<std::vector<double>>((*context)[z_key]);
  z_length = z_vec.size();

  if (!element->hasAttribute("z_dims")) throw NotFoundError("Volume series is missing required attribute z_dims.\n");
  auto z_dims_key = static_cast<std::string>(element->getAttribute("z_dims"));
  auto z_dims_vec = GRM::get<std::vector<int>>((*context)[z_dims_key]);
  dims = z_dims_vec.size();

  if (dims != 3) throw std::length_error("For volume series the size of z_dims has to be 3.\n");
  if (z_dims_vec[0] * z_dims_vec[1] * z_dims_vec[2] != z_length)
    throw std::length_error("For volume series shape[0] * shape[1] * shape[2] must be z length.\n");
  if (z_length <= 0) throw NotFoundError("For volume series the size of z has to be greater than 0.\n");

  if (!element->hasAttribute("algorithm"))
    {
      element->setAttribute("algorithm", algorithm);
    }
  else
    {
      algorithm = getVolumeAlgorithm(element);
    }
  if (algorithm != GR_VOLUME_ABSORPTION && algorithm != GR_VOLUME_EMISSION && algorithm != GR_VOLUME_MIP)
    {
      logger((stderr, "Got unknown volume algorithm \"%d\"\n", algorithm));
      throw std::logic_error("For volume series the given algorithm is unknown.\n");
    }

  d_min = d_max = -1.0;
  if (element->hasAttribute("d_min")) d_min = static_cast<double>(element->getAttribute("d_min"));
  if (element->hasAttribute("d_max")) d_max = static_cast<double>(element->getAttribute("d_max"));

  processSpace3d(element->parentElement());
  processColormap(element->parentElement()->parentElement());
  if (redraw_ws)
    {
      gr_inqvpsize(&width, &height, &device_pixel_ratio);
      gr_setpicturesizeforvolume((int)(width * device_pixel_ratio), (int)(height * device_pixel_ratio));
    }
  const gr3_volume_2pass_t *volume_context =
      gr_volume_2pass(z_dims_vec[0], z_dims_vec[1], z_dims_vec[2], &(z_vec[0]), algorithm, &d_min, &d_max, nullptr);

  std::ostringstream get_address;
  get_address << volume_context;
  element->setAttribute("_volume_context_address", get_address.str());

  auto parent_element = element->parentElement();
  getPlotParent(parent_element); // parent is plot in this case
  if (parent_element->hasAttribute("z_lim_min") && parent_element->hasAttribute("z_lim_max"))
    {
      dlim[0] = static_cast<double>(parent_element->getAttribute("z_lim_min"));
      dlim[1] = static_cast<double>(parent_element->getAttribute("z_lim_max"));
      dlim[0] = grm_min(dlim[0], d_min);
      dlim[1] = grm_max(dlim[1], d_max);
    }
  else
    {
      dlim[0] = d_min;
      dlim[1] = d_max;
    }

  parent_element->setAttribute("_c_lim_min", dlim[0]);
  parent_element->setAttribute("_c_lim_max", dlim[1]);

  if (redraw_ws)
    {
      GRM::PushDrawableToZQueue push_volume_to_z_queue(volume);
      push_volume_to_z_queue(element, context);
    }
}
