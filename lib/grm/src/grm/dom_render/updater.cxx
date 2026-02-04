#ifdef _WIN32
/*
 * Headers on Windows can define `min` and `max` as macros which causes
 * problem when using `std::min` and `std::max`
 * -> Define `NOMINMAX` to prevent the definition of these macros
 */
#define NOMINMAX
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <grm/dom_render/updater.hxx>
#include <grm/dom_render/render.hxx>
#include <grm/dom_render/render_util.hxx>
#include <grm/dom_render/graphics_tree/util.hxx>
#include <grm/utilcpp_int.hxx>
#include <grm/util_int.h>
#include <grm/dom_render/process_attributes.hxx>
#include <grm/plot.h>

void GRM::updateFilter(const std::shared_ptr<GRM::Element> &element, const std::string &attr,
                       const std::string &value = "")
{
  std::vector<std::string> bar{
      "fill_color_rgb",
      "fill_color_ind",
      "fill_int_style",
      "fill_style",
      "line_color_rgb",
      "line_color_ind",
      "line_width",
      "text",
      "x1",
      "x2",
      "y1",
      "y2",
  };
  std::vector<std::string> error_bar{
      "cap_x_max", "cap_x_min", "downwards_e", "error_bar_x", "error_bar_y_max", "error_bar_y_min", "upwards_e",
  };
  std::vector<std::string> error_bars{
      "error_bar_style",
  };
  std::vector<std::string> marginal_heatmap_plot{
      "algorithm", "marginal_heatmap_kind", "x", "x_flip", "y", "y_flip", "z",
  };
  std::vector<std::string> overlay_element{
      "height_abs",
      "width_abs",
      "x",
      "y",
  };
  std::vector<std::string> polar_bar{
      "bin_width",      "bin_widths",     "bin_edges",  "bin_nr",         "count", "draw_edges",
      "fill_color_ind", "fill_int_style", "fill_style", "line_color_ind", "norm",  "theta_flip",
  };
  std::vector<std::string> series_barplot{
      "bar_width",
      "color_ind_values",
      "color_rgb_values",
      "edge_width",
      "fill_color_ind",
      "fill_int_style",
      "fill_style",
      "font",
      "font_precision",
      "indices",
      "inner_series",
      "line_color_ind",
      "line_spec",
      "line_width",
      "rgb",
      "style",
      "text_align_horizontal",
      "text_align_vertical",
      "text_color_ind",
      "width",
      "y",
      "y_labels",
  };
  std::vector<std::string> series_contour{
      "levels", "px", "py", "pz", "x", "y", "z", "z_max", "z_min",
  };
  std::vector<std::string> series_contourf = series_contour;
  std::vector<std::string> series_heatmap{
      "x", "y", "z", "z_range_max", "z_range_min",
  };
  std::vector<std::string> series_hexbin{
      "num_bins",
      "x",
      "y",
  };
  std::vector<std::string> series_histogram{
      "bins",           "fill_color_ind", "fill_color_rgb", "fill_int_style", "fill_style",
      "line_color_ind", "line_color_rgb", "line_spec",      "weights",
  };
  std::vector<std::string> series_imshow{
      "data", "x", "y", "z", "z_dims",
  };
  std::vector<std::string> series_isosurface{
      "ambient", "color_rgb", "diffuse", "isovalue", "specular", "specular_power", "z", "z_dims",
  };
  std::vector<std::string> series_line{
      "line_color_ind", "line_spec", "line_type", "line_width", "x", "y",
  };
  std::vector<std::string> series_nonuniform_heatmap = series_heatmap;
  std::vector<std::string> series_nonuniform_polar_heatmap{
      "r", "r_range_max", "r_range_min", "theta", "theta_range_max", "theta_range_min",
      "z", "z_range_max", "z_range_min",
  };
  std::vector<std::string> series_pie{
      "color_ind_values",
      "x",
  };
  std::vector<std::string> series_line3{
      "x",
      "y",
      "z",
  };
  std::vector<std::string> series_polar_heatmap = series_nonuniform_polar_heatmap;
  std::vector<std::string> series_polar_histogram{
      "bin_counts",      "bin_edges",       "bin_width",   "bin_widths",   "counts",
      "draw_edges",      "keep_radii_axes", "num_bins",    "norm",         "r_max",
      "r_min",           "r_range_max",     "r_range_min", "theta",        "theta_range_max",
      "theta_range_min", "stairs",          "tick",        "transparency",

  };
  std::vector<std::string> series_polar_line{
      "clip_negative",   "line_color_ind",  "line_spec", "line_type",   "marker_color_ind", "marker_type",
      "r_max",           "r_min",           "r",         "r_range_max", "r_range_min",      "theta",
      "theta_range_max", "theta_range_min",
  };
  std::vector<std::string> series_polar_scatter{
      "clip_negative",   "line_color_ind",  "line_spec", "line_type",   "marker_color_ind", "marker_type",
      "r_max",           "r_min",           "r",         "r_range_max", "r_range_min",      "theta",
      "theta_range_max", "theta_range_min",
  };
  std::vector<std::string> series_quiver{
      "colored", "u", "v", "x", "y",
  };
  std::vector<std::string> series_scatter{
      "c", "marker_color_ind", "x", "y", "z",
  };
  std::vector<std::string> series_scatter3{
      "c",
      "x",
      "y",
      "z",
  };
  std::vector<std::string> series_shade{
      "transformation", "x", "x_bins", "y", "y_bins",
  };
  std::vector<std::string> series_stairs{
      "line_color_ind", "line_spec", "step_where", "x", "y", "z",
  };
  std::vector<std::string> series_stem{
      "line_color_ind", "line_spec", "x", "y", "y_range_min",
  };
  std::vector<std::string> series_surface{
      "use_gr3",
      "x",
      "y",
      "z",
  };
  std::vector<std::string> series_tricontour{
      "levels",
      "x",
      "y",
      "z",
  };
  std::vector<std::string> series_trisurface{
      "x",
      "y",
      "z",
  };
  std::vector<std::string> series_volume{
      "d_max", "d_min", "x", "y", "z", "z_dims",
  };
  std::vector<std::string> series_wireframe{
      "x",
      "y",
      "z",
  };
  std::vector<std::string> coordinate_system_element{"theta_flip", "x_grid", "y_grid", "z_grid", "plot_type"};
  std::vector<std::string> tick_group{
      "line_color_ind",
      "line_color_rgb",
      "line_spec",
      "line_type",
  };
  static std::map<std::string, std::vector<std::string>> element_names{
      {std::string("bar"), bar},
      {std::string("error_bar"), error_bar},
      {std::string("error_bars"), error_bars},
      {std::string("polar_bar"), polar_bar},
      {std::string("coordinate_system"), coordinate_system_element},
      {std::string("marginal_heatmap_plot"), marginal_heatmap_plot},
      {std::string("overlay_element"), overlay_element},
      {std::string("series_barplot"), series_barplot},
      {std::string("series_contour"), series_contour},
      {std::string("series_contourf"), series_contourf},
      {std::string("series_heatmap"), series_heatmap},
      {std::string("series_hexbin"), series_hexbin},
      {std::string("series_histogram"), series_histogram},
      {std::string("series_imshow"), series_imshow},
      {std::string("series_isosurface"), series_isosurface},
      {std::string("series_line"), series_line},
      {std::string("series_nonuniform_heatmap"), series_nonuniform_heatmap},
      {std::string("series_nonuniform_polar_heatmap"), series_nonuniform_polar_heatmap},
      {std::string("series_pie"), series_pie},
      {std::string("series_line3"), series_line3},
      {std::string("series_polar_heatmap"), series_polar_heatmap},
      {std::string("series_polar_histogram"), series_polar_histogram},
      {std::string("series_polar_line"), series_polar_line},
      {std::string("series_polar_scatter"), series_polar_scatter},
      {std::string("series_quiver"), series_quiver},
      {std::string("series_scatter"), series_scatter},
      {std::string("series_scatter3"), series_scatter3},
      {std::string("series_shade"), series_shade},
      {std::string("series_stairs"), series_stairs},
      {std::string("series_stem"), series_stem},
      {std::string("series_surface"), series_surface},
      {std::string("series_tricontour"), series_tricontour},
      {std::string("series_trisurface"), series_trisurface},
      {std::string("series_volume"), series_volume},
      {std::string("series_wireframe"), series_wireframe},
      {std::string("tick_group"), tick_group},
  };
  // plot attributes which needs a bounding box redraw
  // TODO: critical update in plot means critical update inside children, extend the following lists
  std::vector<std::string> plot_bbox_attributes{
      "keep_aspect_ratio",
      "only_square_aspect_ratio",
      "reset_ranges",
  };
  std::vector<std::string> plot_critical_attributes{
      "colormap", "colormap_inverted", "r_log",     "theta_flip", "x_flip", "x_lim_max", "x_lim_min", "x_log",
      "y_flip",   "y_lim_max",         "y_lim_min", "y_log",      "z_flip", "z_lim_max", "z_lim_min", "z_log",
  };
  std::vector<std::string> integral_critical_attributes{
      "int_lim_high",
      "int_lim_low",
      "x_shift_wc",
  };
  bool automatic_update;
  auto global_creator = grm_get_creator();
  auto global_render = grm_get_render();
  auto global_root = grm_get_document_root();
  auto active_figure = global_render->getActiveFigure();
  auto tick_modification_map = *getTickModificationMap();

  GRM::Render::getAutoUpdate(&automatic_update);
  // only do updates when there is a change made
  if (automatic_update && !startsWith(attr, "_"))
    {
      GRM::Render::setAutoUpdate(false);
      if (attr == "kind")
        {
          // special case for kind attributes to support switching the kind of a plot
          std::vector<std::string> line_group = {"line", "scatter"};
          std::vector<std::string> heatmap_group = {"contour",          "contourf", "heatmap",  "imshow",
                                                    "marginal_heatmap", "surface",  "wireframe"};
          std::vector<std::string> isosurface_group = {"isosurface", "volume"};
          std::vector<std::string> line3_group = {"line3", "scatter", "scatter3", "tricontour", "trisurface"};
          std::vector<std::string> barplot_group = {"barplot", "stem", "stairs"};
          std::vector<std::string> hexbin_group = {"hexbin", "shade"};
          std::vector<std::string> polar_line_group = {"polar_line", "polar_scatter"};
          std::shared_ptr<GRM::Element> new_element = nullptr;
          std::shared_ptr<GRM::Element> central_region, central_region_parent;

          auto plot_parent = element->parentElement();
          getPlotParent(plot_parent);
          central_region_parent = plot_parent;
          if (plot_parent->children()[0]->localName() == "marginal_heatmap_plot")
            central_region_parent = plot_parent->children()[0];
          for (const auto &child : central_region_parent->children())
            {
              if (child->localName() == "central_region")
                {
                  central_region = child;
                  break;
                }
            }

          auto kind = static_cast<std::string>(element->getAttribute("kind"));
          if (kind == "hist")
            kind = "histogram";
          else if (kind == "plot3")
            kind = "line3";
          if (std::find(line_group.begin(), line_group.end(), value) != line_group.end() &&
              std::find(line_group.begin(), line_group.end(), kind) != line_group.end())
            {
              auto new_series = global_creator->createSeries(kind);
              new_element = new_series;
              element->parentElement()->insertBefore(new_series, element);
              plot_parent->setAttribute("_kind", kind);
              new_series->setAttribute("x", element->getAttribute("x"));
              new_series->setAttribute("y", element->getAttribute("y"));
              if (element->hasAttribute("label")) new_series->setAttribute("label", element->getAttribute("label"));
              applyBoundingBoxId(*new_series, *element, true);
              if (element->hasAttribute("ref_x_axis_location"))
                new_series->setAttribute("ref_x_axis_location",
                                         static_cast<std::string>(element->getAttribute("ref_x_axis_location")));
              if (element->hasAttribute("ref_y_axis_location"))
                new_series->setAttribute("ref_y_axis_location",
                                         static_cast<std::string>(element->getAttribute("ref_y_axis_location")));
              if (static_cast<int>(central_region->getAttribute("keep_window"))) setRanges(element, new_series);
              for (const auto &child : element->children())
                {
                  if (child->localName() == "error_bars") new_series->append(child);
                  child->remove();
                }
              element->remove();
              new_series->setAttribute("_update_required", true);
              new_series->setAttribute("_delete_children", 2);
              if (auto legend = plot_parent->querySelectors("legend"); legend != nullptr)
                {
                  legend->setAttribute("_delete_children", 2);
                  legend->removeAttribute("_legend_elems");
                }
            }
          else if (std::find(heatmap_group.begin(), heatmap_group.end(), value) != heatmap_group.end() &&
                   std::find(heatmap_group.begin(), heatmap_group.end(), kind) != heatmap_group.end())
            {
              std::shared_ptr<GRM::Element> new_series;
              if (kind == "marginal_heatmap")
                {
                  new_series = global_render->createElement("marginal_heatmap_plot");
                  new_series->setAttribute("kind", "marginal_heatmap");
                }
              else
                {
                  new_series = global_creator->createSeries(kind);
                }
              new_element = new_series;
              if (value == "marginal_heatmap")
                {
                  std::shared_ptr<GRM::Element> first_side_region = nullptr;
                  // kind was 'marginal_heatmap' so the marginal_heatmap_plot must be removed and a new series created
                  for (const auto &child : element->children())
                    {
                      if (child->localName() == "side_region")
                        {
                          element->parentElement()->append(child);
                          if (first_side_region == nullptr) first_side_region = child;
                          for (const auto &side_region_child : child->children())
                            {
                              if (side_region_child->localName() == "side_plot_region" &&
                                  child->hasAttribute("marginal_heatmap_side_plot"))
                                {
                                  side_region_child->remove();
                                  break;
                                }
                            }
                          if (child->hasAttribute("marginal_heatmap_side_plot"))
                            child->removeAttribute("marginal_heatmap_side_plot");
                          continue;
                        }
                      if (child->localName() == "central_region")
                        {
                          for (const auto &central_region_child : child->children())
                            {
                              if (central_region_child->hasAttribute("_child_id") &&
                                  static_cast<int>(central_region_child->getAttribute("_child_id")) == 0 &&
                                  central_region_child->localName() == "series_heatmap")
                                central_region_child->remove();
                            }
                        }
                    }
                  element->parentElement()->insertBefore(central_region, first_side_region);
                  central_region->append(new_series);
                }
              else if (kind == "marginal_heatmap")
                {
                  std::shared_ptr<GRM::Element> first_side_region = nullptr;
                  // move the side_regions into the marginal_heatmap_plot
                  for (const auto &side_region_child : central_region->parentElement()->children())
                    {
                      if (side_region_child->localName() == "side_region")
                        {
                          if (first_side_region == nullptr) first_side_region = side_region_child;
                          new_series->append(side_region_child);
                          if (side_region_child->querySelectors("colorbar"))
                            {
                              side_region_child->querySelectors("colorbar")->parentElement()->remove();
                              side_region_child->setAttribute("viewport_offset", PLOT_DEFAULT_SIDEREGION_OFFSET);
                              side_region_child->setAttribute("width", PLOT_DEFAULT_SIDEREGION_WIDTH);
                            }
                        }
                    }
                  // create marginal_heatmap_plot as central_region father
                  central_region->parentElement()->insertBefore(new_series, central_region);
                  new_series->insertBefore(central_region, first_side_region);
                  // declare which side_region contains the marginal_heatmap side_plot
                  new_series->querySelectors("side_region[location=\"top\"]")
                      ->setAttribute("marginal_heatmap_side_plot", 1);
                  new_series->querySelectors("side_region[location=\"right\"]")
                      ->setAttribute("marginal_heatmap_side_plot", 1);
                }
              else
                {
                  element->parentElement()->insertBefore(new_series, element);
                }
              plot_parent->setAttribute("_kind", kind);
              new_series->setAttribute("x", element->getAttribute("x"));
              new_series->setAttribute("y", element->getAttribute("y"));
              new_series->setAttribute("z", element->getAttribute("z"));
              applyBoundingBoxId(*new_series, *element, true);
              if (static_cast<int>(central_region->getAttribute("keep_window"))) setRanges(element, new_series);
              if (kind == "imshow")
                {
                  auto context = global_render->getContext();
                  auto id = static_cast<int>(global_root->getAttribute("_id"));
                  auto str = std::to_string(id);
                  auto x = static_cast<std::string>(element->getAttribute("x"));
                  auto x_vec = GRM::get<std::vector<double>>((*context)[x]);
                  int x_length = static_cast<int>(x_vec.size());
                  auto y = static_cast<std::string>(element->getAttribute("y"));
                  auto y_vec = GRM::get<std::vector<double>>((*context)[y]);
                  int y_length = static_cast<int>(y_vec.size());
                  std::vector<int> z_dims_vec = {static_cast<int>(x_length), static_cast<int>(y_length)};
                  (*context)["z_dims" + str] = z_dims_vec;
                  new_series->setAttribute("z_dims", "z_dims" + str);
                  global_root->setAttribute("_id", id++);
                }
              if (value == "imshow")
                {
                  if (element->hasAttribute("_x_org"))
                    {
                      auto x_key = static_cast<std::string>(element->getAttribute("_x_org"));
                      new_series->setAttribute("x", x_key);
                    }
                  if (element->hasAttribute("_y_org"))
                    {
                      auto y_key = static_cast<std::string>(element->getAttribute("_y_org"));
                      new_series->setAttribute("y", y_key);
                    }
                }

              for (const auto &child : element->children())
                {
                  child->remove();
                }
              element->remove();
              new_series->setAttribute("_update_required", true);
              new_series->setAttribute("_delete_children", 2);
            }
          else if (std::find(isosurface_group.begin(), isosurface_group.end(), value) != isosurface_group.end() &&
                   std::find(isosurface_group.begin(), isosurface_group.end(), kind) != isosurface_group.end())
            {
              auto new_series = global_creator->createSeries(kind);
              new_element = new_series;
              element->parentElement()->insertBefore(new_series, element);
              plot_parent->setAttribute("_kind", kind);
              new_series->setAttribute("z", element->getAttribute("z"));
              new_series->setAttribute("z_dims", element->getAttribute("z_dims"));
              if (element->hasAttribute("d_min")) new_series->setAttribute("d_min", element->getAttribute("d_min"));
              if (element->hasAttribute("d_max")) new_series->setAttribute("d_max", element->getAttribute("d_max"));
              applyBoundingBoxId(*new_series, *element, true);
              if (static_cast<int>(central_region->getAttribute("keep_window"))) setRanges(element, new_series);
              for (const auto &child : element->children())
                {
                  child->remove();
                }
              element->remove();
              new_series->setAttribute("_update_required", true);
              new_series->setAttribute("_delete_children", 2);
            }
          else if (std::find(line3_group.begin(), line3_group.end(), value) != line3_group.end() &&
                   std::find(line3_group.begin(), line3_group.end(), kind) != line3_group.end())
            {
              auto new_series = global_creator->createSeries(kind);
              new_element = new_series;
              element->parentElement()->insertBefore(new_series, element);
              plot_parent->setAttribute("_kind", kind);
              new_series->setAttribute("x", element->getAttribute("x"));
              new_series->setAttribute("y", element->getAttribute("y"));
              new_series->setAttribute("z", element->getAttribute("z"));
              if (element->hasAttribute("label")) new_series->setAttribute("label", element->getAttribute("label"));
              applyBoundingBoxId(*new_series, *element, true);
              if (static_cast<int>(central_region->getAttribute("keep_window"))) setRanges(element, new_series);
              for (const auto &child : element->children())
                {
                  child->remove();
                }
              element->remove();
              new_series->setAttribute("_update_required", true);
              new_series->setAttribute("_delete_children", 2);

              if (auto legend = plot_parent->querySelectors("legend"); legend != nullptr)
                {
                  legend->setAttribute("_delete_children", 2);
                  legend->removeAttribute("_legend_elems");
                }
            }
          else if (std::find(barplot_group.begin(), barplot_group.end(), value) != barplot_group.end() &&
                   std::find(barplot_group.begin(), barplot_group.end(), kind) != barplot_group.end())
            {
              auto new_series = global_creator->createSeries(kind);
              new_element = new_series;
              element->parentElement()->insertBefore(new_series, element);
              plot_parent->setAttribute("_kind", kind);
              new_series->setAttribute("x", element->getAttribute("x"));
              if (element->hasAttribute("label")) new_series->setAttribute("label", element->getAttribute("label"));
              applyBoundingBoxId(*new_series, *element, true);
              if (element->hasAttribute("ref_x_axis_location"))
                new_series->setAttribute("ref_x_axis_location",
                                         static_cast<std::string>(element->getAttribute("ref_x_axis_location")));
              if (element->hasAttribute("ref_y_axis_location"))
                new_series->setAttribute("ref_y_axis_location",
                                         static_cast<std::string>(element->getAttribute("ref_y_axis_location")));
              if (static_cast<int>(central_region->getAttribute("keep_window"))) setRanges(element, new_series);

              new_series->setAttribute("y", element->getAttribute("y"));

              for (const auto &child : element->children())
                {
                  if (child->localName() == "error_bars") new_series->append(child);
                  child->remove();
                }
              element->remove();
              new_series->setAttribute("_update_required", true);
              new_series->setAttribute("_delete_children", 2);

              if (kind == "barplot")
                {
                  int series_count = new_series->parentElement()->querySelectorsAll("series_" + kind).size();
                  // if more than 1 barplot series is displayed they could overlap each other -> problem
                  // to make everything visible again transparency must be used
                  for (const auto &series : new_series->parentElement()->querySelectorsAll("series_" + kind))
                    {
                      if (series_count > 1 && !series->hasAttribute("transparency"))
                        {
                          series->setAttribute("transparency", 0.5);
                        }
                    }
                }
            }
          else if (std::find(hexbin_group.begin(), hexbin_group.end(), value) != hexbin_group.end() &&
                   std::find(hexbin_group.begin(), hexbin_group.end(), kind) != hexbin_group.end())
            {
              auto new_series = global_creator->createSeries(kind);
              new_element = new_series;
              element->parentElement()->insertBefore(new_series, element);
              plot_parent->setAttribute("_kind", kind);
              new_series->setAttribute("x", element->getAttribute("x"));
              new_series->setAttribute("y", element->getAttribute("y"));
              applyBoundingBoxId(*new_series, *element, true);
              if (static_cast<int>(central_region->getAttribute("keep_window"))) setRanges(element, new_series);
              for (const auto &child : element->children())
                {
                  child->remove();
                }
              element->remove();
              new_series->setAttribute("_update_required", true);
              new_series->setAttribute("_delete_children", 2);
            }
          else if (std::find(polar_line_group.begin(), polar_line_group.end(), value) != polar_line_group.end() &&
                   std::find(polar_line_group.begin(), polar_line_group.end(), kind) != polar_line_group.end())
            {
              auto new_series = global_creator->createSeries(kind);
              new_element = new_series;
              element->parentElement()->insertBefore(new_series, element);
              plot_parent->setAttribute("_kind", kind);
              new_series->setAttribute("theta", element->getAttribute("theta"));
              new_series->setAttribute("r", element->getAttribute("r"));
              if (element->hasAttribute("label")) new_series->setAttribute("label", element->getAttribute("label"));
              applyBoundingBoxId(*new_series, *element, true);
              if (element->hasAttribute("clip_negative"))
                new_series->setAttribute("clip_negative", element->getAttribute("clip_negative"));
              for (const auto &child : element->children())
                {
                  child->remove();
                }
              element->remove();
              new_series->setAttribute("_update_required", true);
              new_series->setAttribute("_delete_children", 2);
            }
          else
            {
              fprintf(stderr, "Update kind %s to %s is not possible\n", kind.c_str(), value.c_str());
              std::cerr << toXML(element->getRootNode(), GRM::SerializerOptions{std::string(2, ' ')}) << "\n";
            }

          if (new_element)
            {
              auto plot = new_element->parentElement();
              getPlotParent(plot);

              /* update the limits since they depend on the kind */
              plot->setAttribute("_update_limits", 1);

              /* overwrite attributes for which a kind dependent default exists */
              bool use_grplot_changes = plot->hasAttribute("use_grplot_changes")
                                            ? static_cast<int>(plot->getAttribute("use_grplot_changes"))
                                            : false;
              if (use_grplot_changes)
                {
                  plot->setAttribute("_overwrite_kind_dependent_defaults", 1);
                  applyPlotDefaults(plot);
                  plot->removeAttribute("_overwrite_kind_dependent_defaults");
                }

              /* update coordinate system if needed */
              std::vector<std::string> colorbar_group = {
                  "quiver",  "contour", "contourf", "hexbin",     "polar_heatmap", "nonuniform_polar_heatmap",
                  "heatmap", "surface", "volume",   "trisurface", "tricontour"};

              auto coordinate_system = plot->querySelectors("coordinate_system");
              auto new_kind = static_cast<std::string>(new_element->getAttribute("kind"));
              std::string new_type = "2d";
              const std::string &old_kind = value;
              if (polar_kinds.count(new_kind) != 0) new_type = "polar";
              if (kinds_3d.count(new_kind) != 0) new_type = "3d";

              // the default diag_factor must be recalculated cause the default plot size can diverge
              // f.e. surface plots are smaller than heatmap plots so the diag_factor isn't the same
              if (old_kind != new_kind)
                {
                  for (const auto &elem : plot_parent->querySelectorsAll("[_default_diag_factor]"))
                    {
                      elem->removeAttribute("_default_diag_factor");
                    }
                  if (plot_parent->parentElement()->localName() == "layout_grid_element")
                    plot_parent->removeAttribute("_default_diag_factor");
                  if (new_type == "3d" && !central_region->hasAttribute("_diag_factor_set_by_user"))
                    central_region->removeAttribute("_diag_factor");
                  active_figure->setAttribute("_kind_changed", true);
                  plot_parent->removeAttribute("_initial_factor");
                }

              if (coordinate_system)
                {
                  auto left_side_region = plot->querySelectors("side_region[location=\"left\"]");
                  auto bottom_side_region = plot->querySelectors("side_region[location=\"bottom\"]");
                  auto old_type = static_cast<std::string>(coordinate_system->getAttribute("plot_type"));
                  if (use_grplot_changes && old_type == "2d" &&
                      new_type == "2d") // special case which will reset the tick_orientation when kind is changed
                    {
                      coordinate_system->setAttribute("_update_required", true);
                      coordinate_system->setAttribute("_delete_children",
                                                      static_cast<int>(DelValues::UPDATE_WITH_DEFAULT));
                    }
                  if (new_type != old_type)
                    {
                      coordinate_system->setAttribute("plot_type", new_type);
                      coordinate_system->setAttribute("_update_required", true);
                      coordinate_system->setAttribute("_delete_children",
                                                      static_cast<int>(DelValues::RECREATE_ALL_CHILDREN));

                      if (old_type == "2d")
                        {
                          if (bottom_side_region->hasAttribute("text_content"))
                            {
                              coordinate_system->setAttribute(
                                  "x_label",
                                  static_cast<std::string>(bottom_side_region->getAttribute("text_content")));
                              bottom_side_region->removeAttribute("text_content");
                              bottom_side_region->setAttribute("_update_required", true);

                              if (auto text_child = bottom_side_region->querySelectors("text_region");
                                  text_child != nullptr)
                                bottom_side_region->removeChild(text_child);
                            }
                          if (left_side_region->hasAttribute("text_content"))
                            {
                              coordinate_system->setAttribute(
                                  "y_label", static_cast<std::string>(left_side_region->getAttribute("text_content")));
                              left_side_region->removeAttribute("text_content");
                              left_side_region->setAttribute("_update_required", true);

                              if (auto text_child = left_side_region->querySelectors("text_region");
                                  text_child != nullptr)
                                left_side_region->removeChild(text_child);
                            }
                        }
                      else if (old_type == "3d" && new_type == "2d")
                        {
                          if (coordinate_system->hasAttribute("x_label"))
                            {
                              bottom_side_region->setAttribute(
                                  "text_content", static_cast<std::string>(coordinate_system->getAttribute("x_label")));
                              bottom_side_region->setAttribute("_update_required", true);
                              coordinate_system->removeAttribute("x_label");
                            }
                          if (coordinate_system->hasAttribute("y_label"))
                            {
                              left_side_region->setAttribute(
                                  "text_content", static_cast<std::string>(coordinate_system->getAttribute("y_label")));
                              left_side_region->setAttribute("_update_required", true);
                              coordinate_system->removeAttribute("y_label");
                            }
                        }
                    }
                  if (new_kind == "imshow" || new_kind == "isosurface")
                    {
                      auto top_side_region = plot->querySelectors("side_region[location=\"top\"]");

                      left_side_region->setAttribute("_update_required", true);
                      left_side_region->setAttribute("_delete_children", 2);
                      bottom_side_region->setAttribute("_update_required", true);
                      bottom_side_region->setAttribute("_delete_children", 2);
                      top_side_region->setAttribute("_update_required", true);
                      top_side_region->setAttribute("_delete_children", 2);
                      coordinate_system->setAttribute("hide", true);
                    }
                  else if (old_kind == "imshow" || old_kind == "isosurface")
                    {
                      coordinate_system->removeAttribute("hide");
                    }
                  else if ((new_kind == "shade" && old_kind == "hexbin") ||
                           (new_kind == "hexbin" && old_kind == "shade"))
                    {
                      /* special case cause shade doesn't have a grid element while hexbin has */
                      coordinate_system->setAttribute("_update_required", true);
                      coordinate_system->setAttribute("_delete_children",
                                                      static_cast<int>(DelValues::RECREATE_ALL_CHILDREN));

                      for (const auto &child : coordinate_system->children())
                        {
                          if (child->localName() == "axis")
                            {
                              if (new_kind == "shade") child->setAttribute("draw_grid", false);
                              if (new_kind == "hexbin") child->setAttribute("draw_grid", true);
                            }
                        }
                    }
                  if (use_grplot_changes && (new_kind == "barplot" || old_kind == "barplot"))
                    {
                      // barplot has different x_major value
                      int major_count, x_major = 1;
                      for (const auto &child : coordinate_system->children())
                        {
                          if (child->localName() == "axis")
                            {
                              auto axis_type = static_cast<std::string>(child->getAttribute("axis_type"));
                              std::string orientation = PLOT_DEFAULT_ORIENTATION;
                              if (new_element != nullptr)
                                orientation =
                                    static_cast<std::string>(new_element->parentElement()->getAttribute("orientation"));

                              getMajorCount(child, new_kind, major_count);
                              if (new_kind == "barplot")
                                {
                                  bool problematic_bar_num = false, only_barplot = true;
                                  auto context = global_render->getContext();
                                  auto barplots = central_region->querySelectorsAll("series_barplot");
                                  for (const auto &barplot : barplots)
                                    {
                                      if (!barplot->hasAttribute("style") ||
                                          static_cast<std::string>(barplot->getAttribute("style")) == "default")
                                        {
                                          auto y_key = static_cast<std::string>(barplot->getAttribute("y"));
                                          auto y_vec = GRM::get<std::vector<double>>((*context)[y_key]);
                                          if (size(y_vec) > 20)
                                            {
                                              problematic_bar_num = true;
                                              break;
                                            }
                                        }
                                    }
                                  x_major = problematic_bar_num ? major_count : 1;

                                  // barplot has some special default which can only be applied if no other kind is
                                  // included inside the central_region
                                  for (const auto &series : central_region->children())
                                    {
                                      if (startsWith(series->localName(), "series_") &&
                                          series->localName() != "series_barplot")
                                        {
                                          only_barplot = false;
                                          break;
                                        }
                                    }
                                  if (only_barplot)
                                    {
                                      plot_parent->setAttribute("adjust_x_lim", false);
                                      if (axis_type == "x" && orientation == "horizontal")
                                        child->setAttribute("draw_grid", false);
                                      if (axis_type == "y" && orientation == "vertical")
                                        child->setAttribute("draw_grid", false);
                                      child->setAttribute("_delete_children", 2);
                                    }
                                }
                              else
                                {
                                  x_major = major_count;
                                  plot_parent->setAttribute("adjust_x_lim", true);
                                  if (axis_type == "x" && orientation == "horizontal")
                                    child->setAttribute("draw_grid", true);
                                  if (axis_type == "y" && orientation == "vertical")
                                    child->setAttribute("draw_grid", true);
                                }
                              // reset attributes on axis so the axis gets recreated which is needed cause the barplot
                              // has a different window
                              clearAxisAttributes(child);
                              child->setAttribute("x_major", x_major);
                            }
                        }
                    }
                  if (use_grplot_changes && strEqualsAny(new_kind, "barplot", "stem"))
                    {
                      if (coordinate_system->hasAttribute("y_line")) coordinate_system->setAttribute("y_line", true);
                    }
                  else if (use_grplot_changes && strEqualsAny(old_kind, "barplot", "stem"))
                    {
                      if (coordinate_system->hasAttribute("y_line")) coordinate_system->setAttribute("y_line", false);
                    }
                  // heatmap and marginal_heatmap have a different default behaviour when it comes to adjust lims
                  if (use_grplot_changes && strEqualsAny(new_kind, "heatmap", "marginal_heatmap", "barplot"))
                    {
                      bool no_other_kind = true;
                      for (const auto &series : central_region->children())
                        {
                          if (startsWith(series->localName(), "series_") &&
                              (!strEqualsAny(series->localName(), "series_barplot", "series_heatmap")))
                            {
                              no_other_kind = false;
                              break;
                            }
                        }

                      if (no_other_kind)
                        {
                          for (const auto &child : coordinate_system->children())
                            {
                              if (child->localName() == "axis") clearAxisAttributes(child);
                            }
                          plot_parent->setAttribute("adjust_x_lim", false);
                          plot_parent->setAttribute("adjust_y_lim", false);
                          plot_parent->setAttribute("adjust_z_lim", false);
                        }
                    }
                  else if (use_grplot_changes && strEqualsAny(new_kind, "heatmap", "marginal_heatmap", "barplot"))
                    {
                      for (const auto &child : coordinate_system->children())
                        {
                          if (child->localName() == "axis") clearAxisAttributes(child);
                        }
                      plot_parent->setAttribute("adjust_x_lim", true);
                      plot_parent->setAttribute("adjust_y_lim", true);
                      plot_parent->setAttribute("adjust_z_lim", true);
                    }
                  // need to change tick_orientation if old_kind was heatmap, marginal_heatmap or shade and no other
                  // series of these kinds is left
                  if (use_grplot_changes && strEqualsAny(old_kind, "contourf", "heatmap", "marginal_heatmap", "shade"))
                    {
                      bool one_of_these_kinds_left = true;
                      for (const auto &series : central_region->children())
                        {
                          if (startsWith(series->localName(), "series_") &&
                              !strEqualsAny(series->localName(), "series_contourf", "series_heatmap", "series_shade"))
                            {
                              one_of_these_kinds_left = false;
                              break;
                            }
                        }
                      if (!one_of_these_kinds_left)
                        {
                          for (const auto &child : coordinate_system->children())
                            {
                              if (child->localName() == "axis")
                                {
                                  if (child->hasAttribute("tick_orientation"))
                                    child->setAttribute("tick_orientation", 1);
                                }
                            }
                        }
                    }
                }
              else if (new_kind != "imshow" && new_kind != "isosurface")
                {
                  coordinate_system = global_render->createElement("coordinate_system");
                  coordinate_system->setAttribute("plot_type", new_type);
                  plot->prepend(coordinate_system);
                }
              if (std::find(colorbar_group.begin(), colorbar_group.end(), new_kind) == colorbar_group.end())
                {
                  std::shared_ptr<GRM::Element> side_region, side_plot;
                  if (std::shared_ptr<GRM::Element> colorbar = plot->querySelectors("colorbar"))
                    {
                      side_plot = colorbar->parentElement();
                      side_region = side_plot->parentElement();
                      for (const auto &child : colorbar->children())
                        {
                          child->remove();
                        }
                      colorbar->remove();
                      side_region->remove();
                      side_plot->remove();
                    }
                }
              else
                {
                  double offset;
                  int colors;
                  std::shared_ptr<GRM::Element> side_region = plot->querySelectors("side_region[location=\"right\"]"),
                                                colorbar = plot->querySelectors("colorbar"), side_plot = nullptr;
                  std::tie(offset, colors) = getColorbarAttributes(new_kind, plot);

                  if (side_region == nullptr)
                    side_region = global_creator->createSideRegion(PLOT_DEFAULT_SIDEREGION_LOCATION);
                  else
                    side_plot = side_region->querySelectors("side_plot_region");
                  side_region->setAttribute("viewport_offset", offset + PLOT_DEFAULT_COLORBAR_OFFSET);
                  side_region->setAttribute("width", PLOT_DEFAULT_COLORBAR_WIDTH);
                  side_region->setAttribute("_update_required", true);

                  if (side_plot == nullptr) side_plot = global_creator->createSidePlotRegion();

                  colorbar = global_creator->createColorbar(colors, nullptr, colorbar);
                  if (!colorbar->hasAttribute("_char_height_set_by_user"))
                    colorbar->setAttribute("char_height", PLOT_DEFAULT_COLORBAR_CHAR_HEIGHT);

                  colorbar->setAttribute("_update_required", true);
                  colorbar->setAttribute("_delete_children", static_cast<int>(DelValues::RECREATE_ALL_CHILDREN));

                  if (!plot->querySelectors("side_region[location=\"right\"]")) plot->append(side_region);
                  if (!side_region->querySelectors("side_plot_region")) side_region->append(side_plot);
                  if (!plot->querySelectors("colorbar")) side_plot->append(colorbar);
                }
            }
          if (plot_parent != nullptr && plot_parent->parentElement()->localName() != "figure")
            {
              plot_parent->setAttribute("_active_through_update", true);
            }
        }
      else
        {
          if (auto search = element_names.find(element->localName()); search != element_names.end())
            {
              // attributes which causes a critical update
              if (std::find(search->second.begin(), search->second.end(), attr) != search->second.end())
                {
                  element->setAttribute("_update_required", true);
                  element->setAttribute("_delete_children", 1);
                  if (strEqualsAny(attr, "clip_negative")) element->setAttribute("_delete_children", 0);
                  if (strEqualsAny(attr, "bin_edges", "bin_widths", "bins", "c", "draw_edges", "error_bar_style",
                                   "inner_series", "levels", "line_spec", "marginal_heatmap_kind", "num_bins", "px",
                                   "py", "pz", "r", "stairs", "theta", "u", "v", "x", "y", "z") &&
                      element->localName() != "overlay_element")
                    element->setAttribute("_delete_children", 2);
                  if (strEqualsAny(attr, "text") && element->localName() == "bar")
                    element->setAttribute("_delete_children", 2);
                  if (element->localName() == "series_polar_histogram" && (attr == "num_bins" || attr == "bin_widths"))
                    {
                      for (const auto &child : element->children())
                        {
                          if (child->localName() == "polar_bar") child->setAttribute("_update_required", true);
                        }
                    }
                  if (element->localName() == "coordinate_system")
                    {
                      element->setAttribute("_delete_children", 0);
                    }
                  if (strEqualsAny(attr, "r_range_max", "r_range_min", "theta_range_max", "theta_range_min"))
                    {
                      auto plot_parent = element;
                      getPlotParent(plot_parent);

                      if (auto coordinate_system = plot_parent->querySelectors("coordinate_system");
                          coordinate_system != nullptr)
                        {
                          coordinate_system->setAttribute("_update_required", true);
                          coordinate_system->setAttribute("_delete_children", int(DelValues::RECREATE_OWN_CHILDREN));
                        }
                    }
                }
              if (attr == "ref_y_axis_location" || attr == "ref_x_axis_location")
                {
                  auto plot_parent = element->parentElement()->parentElement();
                  auto coordinate_system = plot_parent->querySelectors("coordinate_system");

                  if (auto plot_type = static_cast<std::string>(coordinate_system->getAttribute("plot_type"));
                      plot_type == "2d")
                    {
                      plot_parent->setAttribute("_update_limits", true);
                      plot_parent->setAttribute("_update_required", true);

                      if (strEqualsAny(static_cast<std::string>(element->getAttribute(attr)), "twin_x", "twin_y"))
                        {
                          coordinate_system->setAttribute("_update_required", true);
                          coordinate_system->setAttribute("_delete_children", 2);
                        }
                    }
                }
            }
          else if (startsWith(element->localName(), "series"))
            {
              // conditional critical attributes
              auto kind = static_cast<std::string>(element->getAttribute("kind"));
              if (kind == "heatmap" || kind == "polar_heatmap" || kind == "nonuniform_polar_heatmap")
                {
                  if (element->hasAttribute("c_range_max") && element->hasAttribute("c_range_min"))
                    {
                      if (attr == "c_range_max" || attr == "c_range_min")
                        {
                          element->setAttribute("_update_required", true);
                          element->setAttribute("_delete_children", 2);
                        }
                    }
                }
              if (kind == "heatmap" || kind == "surface" || kind == "polar_heatmap" ||
                  kind == "nonuniform_polar_heatmap")
                {
                  if (!element->hasAttribute("x") && !element->hasAttribute("y") && !element->hasAttribute("theta") &&
                      !element->hasAttribute("r"))
                    {
                      if (attr == "z_dims")
                        {
                          element->setAttribute("_update_required", true);
                          element->setAttribute("_delete_children", 2);
                        }
                    }
                  if (!element->hasAttribute("x"))
                    {
                      if (attr == "x_range_max" || attr == "x_range_min")
                        {
                          element->setAttribute("_update_required", true);
                          element->setAttribute("_delete_children", 2);
                        }
                    }
                  if (!element->hasAttribute("y"))
                    {
                      if (attr == "y_range_max" || attr == "y_range_min")
                        {
                          element->setAttribute("_update_required", true);
                          element->setAttribute("_delete_children", 2);
                        }
                    }
                  if (!element->hasAttribute("theta"))
                    {
                      if (attr == "theta_range_max" || attr == "theta_range_min")
                        {
                          element->setAttribute("_update_required", true);
                          element->setAttribute("_delete_children", 2);
                        }
                    }
                  if (!element->hasAttribute("r"))
                    {
                      if (attr == "r_range_max" || attr == "r_range_min")
                        {
                          element->setAttribute("_update_required", true);
                          element->setAttribute("_delete_children", 2);
                        }
                    }
                }
              else if (kind == "marginal_heatmap" && (attr == "x_ind" || attr == "y_ind"))
                {
                  auto x_ind = static_cast<int>(element->getAttribute("x_ind"));
                  auto y_ind = static_cast<int>(element->getAttribute("y_ind"));
                  if ((attr == "x_ind" && y_ind != -1) || (attr == "y_ind" && x_ind != -1))
                    element->setAttribute("_update_required", true);
                }
            }
          else if ((attr == "size_x" || attr == "size_y") && element->localName() == "figure" &&
                   static_cast<int>(element->getAttribute("active")))
            {
              // imshow needs vp which gets inflicted by the size attributes in figure, this means when the size is
              // changed the imshow_series must be recalculated
              auto imshow = global_render->querySelectorsAll("series_imshow");
              for (const auto &imshow_elem : imshow)
                {
                  imshow_elem->setAttribute("_update_required", true);
                }

              // reset the bounding boxes for all series
              for (const auto &child : element->children()) // plot level
                {
                  for (const auto &childchild : child->children())
                    {
                      if (startsWith(childchild->localName(), "series_")) resetOldBoundingBoxes(childchild);
                      if (childchild->localName() == "series_marginal_heatmap")
                        {
                          for (const auto &childchildchild : childchild->children())
                            {
                              if (startsWith(childchildchild->localName(), "series_"))
                                resetOldBoundingBoxes(childchildchild);
                            }
                        }
                    }
                }

              // reset the bounding boxes for elements in list
              for (const std::string name :
                   {"bar", "pie_segment", "colorbar", "polar_bar", "coordinate_system", "plot", "error_bar", "integral",
                    "integral_group", "tick_group", "radial_axes", "theta_axes"})
                {
                  for (const auto &elem : element->querySelectorsAll(name))
                    {
                      resetOldBoundingBoxes(elem);
                      // plot gets calculated to quit so this special case is needed to get the right bboxes
                      if (name == "plot") removeBoundingBoxId(*elem);
                    }
                }

              // reset the bounding boxes for figure
              resetOldBoundingBoxes(element);
              removeBoundingBoxId(*element);
            }
          else if (element->localName() == "plot" && std::find(plot_bbox_attributes.begin(), plot_bbox_attributes.end(),
                                                               attr) != plot_bbox_attributes.end())
            {
              // when the ranges gets reseted the bounding boxes of the series can be wrong, to solve this problem
              // they get calculated again out of their children
              if (strEqualsAny(attr, "keep_aspect_ratio", "only_square_aspect_ratio")) gr_clearws();
              if (attr == "reset_ranges")
                {
                  for (const auto &child : element->children())
                    {
                      if (startsWith(child->localName(), "series_")) resetOldBoundingBoxes(child);
                    }
                  if (auto radial_axes = element->querySelectors("radial_axes");
                      radial_axes != nullptr && radial_axes->hasAttribute("_r_max"))
                    radial_axes->removeAttribute("_r_max");

                  auto coordinate_system = element->querySelectors("coordinate_system");
                  if (auto plot_type = static_cast<std::string>(coordinate_system->getAttribute("plot_type"));
                      plot_type == "polar")
                    {
                      coordinate_system->setAttribute("_update_required", true);
                      coordinate_system->setAttribute("_delete_children", 2);
                    }

                  // reset the bounding boxes for elements in list
                  for (const std::string name : {"bar", "pie_segment", "colorbar", "polar_bar", "axes_text_group",
                                                 "error_bar", "integral", "integral_group"})
                    {
                      for (const auto &elem : element->querySelectorsAll(name))
                        {
                          resetOldBoundingBoxes(elem);
                        }
                    }
                }
              else
                {
                  resetOldBoundingBoxes(element);
                }
            }
          else if (element->localName() == "plot" &&
                   std::find(plot_critical_attributes.begin(), plot_critical_attributes.end(), attr) !=
                       plot_critical_attributes.end())
            {
              auto central_region_parent = element;
              if (auto kind = static_cast<std::string>(element->getAttribute("_kind"));
                  kind == "marginal_heatmap") // needed so that f.e. colormap changes gets applied
                central_region_parent = element->querySelectors("marginal_heatmap_plot");

              for (const auto &plot_child : central_region_parent->children())
                {
                  for (const auto &child : plot_child->children())
                    {
                      if (startsWith(child->localName(), "series_"))
                        {
                          child->setAttribute("_update_required", true);

                          // reset data when log is set to false
                          if (attr == "x_log" || attr == "x_flip" || attr == "x_lim_max" || attr == "x_lim_min")
                            {
                              element->setAttribute("_update_limits", true);
                              element->setAttribute("_no_x_reset_ranges", true);

                              if (auto coordinate_system = element->querySelectors("coordinate_system");
                                  coordinate_system != nullptr)
                                {
                                  if (auto axes = coordinate_system->querySelectorsAll("axis[location=\"x\"]");
                                      !axes.empty())
                                    {
                                      for (const auto &axis : axes)
                                        {
                                          axis->setAttribute("_update_required", true);
                                          axis->setAttribute("_delete_children",
                                                             static_cast<int>(DelValues::RECREATE_OWN_CHILDREN));
                                        }
                                    }
                                  else
                                    {
                                      // no axis-elements in 3d plots so the complete coordinate_system must be reseted
                                      auto type =
                                          static_cast<std::string>(coordinate_system->getAttribute("plot_type"));
                                      if (type == "3d")
                                        {
                                          coordinate_system->setAttribute("_update_required", true);
                                          coordinate_system->setAttribute(
                                              "_delete_children", static_cast<int>(DelValues::RECREATE_OWN_CHILDREN));
                                        }
                                    }
                                }
                            }
                          if (attr == "y_log" || attr == "y_flip" || attr == "y_lim_max" || attr == "y_lim_min")
                            {
                              element->setAttribute("_update_limits", true);
                              element->setAttribute("_no_y_reset_ranges", true);

                              if (auto coordinate_system = element->querySelectors("coordinate_system");
                                  coordinate_system != nullptr)
                                {
                                  if (auto axes = coordinate_system->querySelectorsAll("axis[location=\"y\"]");
                                      !axes.empty())
                                    {
                                      for (const auto &axis : axes)
                                        {
                                          axis->setAttribute("_update_required", true);
                                          axis->setAttribute("_delete_children",
                                                             static_cast<int>(DelValues::RECREATE_OWN_CHILDREN));
                                        }
                                    }
                                }
                              else
                                {
                                  // no axis-elements in 3d plots so the complete coordinate_system must be reseted
                                  if (auto type =
                                          static_cast<std::string>(coordinate_system->getAttribute("plot_type"));
                                      type == "3d")
                                    {
                                      coordinate_system->setAttribute("_update_required", true);
                                      coordinate_system->setAttribute(
                                          "_delete_children", static_cast<int>(DelValues::RECREATE_OWN_CHILDREN));
                                    }
                                }
                            }
                          if (attr == "z_log" || attr == "z_flip" || attr == "z_lim_max" || attr == "z_lim_min")
                            {
                              element->setAttribute("_update_limits", true);

                              // no axis-elements in 3d plots so the complete coordinate_system must be reseted
                              if (auto coordinate_system = element->querySelectors("coordinate_system");
                                  coordinate_system != nullptr)
                                {
                                  if (auto type =
                                          static_cast<std::string>(coordinate_system->getAttribute("plot_type"));
                                      type == "3d")
                                    {
                                      coordinate_system->setAttribute("_update_required", true);
                                      coordinate_system->setAttribute(
                                          "_delete_children", static_cast<int>(DelValues::RECREATE_OWN_CHILDREN));
                                    }
                                }
                            }
                          if (attr == "r_log")
                            {
                              if (child->hasAttribute("r") && child->hasAttribute("_r_org"))
                                {
                                  auto r_org = static_cast<std::string>(child->getAttribute("_r_org"));
                                  child->setAttribute("r", r_org);

                                  if (child->hasAttribute("r_range_min") && child->hasAttribute("_r_range_min_org"))
                                    child->setAttribute("r_range_min",
                                                        static_cast<double>(child->getAttribute("_r_range_min_org")));
                                  if (child->hasAttribute("r_range_max") && child->hasAttribute("_r_range_max_org"))
                                    child->setAttribute("r_range_max",
                                                        static_cast<double>(child->getAttribute("_r_range_max_org")));
                                  child->setAttribute("_update_required", true);
                                  child->setAttribute("_delete_children",
                                                      static_cast<int>(DelValues::RECREATE_OWN_CHILDREN));
                                }
                              element->setAttribute("_update_limits", true);

                              if (auto coordinate_system = element->querySelectors("coordinate_system");
                                  coordinate_system != nullptr)
                                {
                                  coordinate_system->setAttribute("_update_required", true);
                                  coordinate_system->setAttribute("_delete_children",
                                                                  static_cast<int>(DelValues::RECREATE_OWN_CHILDREN));
                                }
                            }
                        }
                    }
                }
            }
          else if (attr == "location")
            {
              if (element->hasAttribute("x_max_shift_ndc")) element->removeAttribute("x_max_shift_ndc");
              if (element->hasAttribute("x_min_shift_ndc")) element->removeAttribute("x_min_shift_ndc");
              if (element->hasAttribute("x_shift_ndc")) element->removeAttribute("x_shift_ndc");
              if (element->hasAttribute("y_max_shift_ndc")) element->removeAttribute("y_max_shift_ndc");
              if (element->hasAttribute("y_min_shift_ndc")) element->removeAttribute("y_min_shift_ndc");
              if (element->hasAttribute("y_shift_ndc")) element->removeAttribute("y_shift_ndc");
              resetOldBoundingBoxes(element);

              if (strEqualsAny(element->localName(), "side_plot_region", "text_region"))
                {
                  auto new_location = static_cast<std::string>(element->getAttribute("location"));
                  auto parent = element->parentElement();
                  auto other_parent =
                      parent->parentElement()->querySelectors("side_region[location=" + new_location + "]");
                  auto parent_text = static_cast<std::string>(parent->getAttribute("text_content"));
                  auto parent_width = static_cast<double>(parent->getAttribute("width"));
                  auto parent_offset = static_cast<double>(parent->getAttribute("viewport_offset"));
                  auto other_parent_text = static_cast<std::string>(other_parent->getAttribute("text_content"));
                  auto other_parent_width = static_cast<double>(other_parent->getAttribute("width"));
                  auto other_parent_offset = static_cast<double>(other_parent->getAttribute("viewport_offset"));

                  for (const auto &child : other_parent->querySelectorsAll(element->localName()))
                    {
                      // figure out if an element needs to be swapped back
                      if (child != element)
                        {
                          parent->appendChild(child);
                          child->setAttribute("_update_required", 1);
                          child->setAttribute("location", value);
                          resetOldBoundingBoxes(child);
                        }
                    }
                  // move element between both regions
                  other_parent->appendChild(element);
                  if (element->localName() == "text_region")
                    {
                      other_parent->setAttribute("text_content", parent_text);
                      if (!other_parent_text.empty())
                        parent->setAttribute("text_content", other_parent_text);
                      else
                        parent->removeAttribute("text_content");
                    }

                  if (element->localName() == "side_plot_region")
                    {
                      // reset width and offset cause with the side_plot_region change they could be different
                      other_parent->setAttribute("width", parent_width);
                      parent->setAttribute("width", other_parent_width);
                      other_parent->setAttribute("viewport_offset", parent_offset);
                      parent->setAttribute("viewport_offset", other_parent_offset);
                    }
                  element->setAttribute("_update_required", 1);
                  resetOldBoundingBoxes(element);
                  resetOldBoundingBoxes(other_parent);
                  resetOldBoundingBoxes(parent);
                }
              else if (element->localName() == "side_region")
                {
                  // switch the complete side_region if its location gets changed
                  auto new_location = static_cast<std::string>(element->getAttribute("location"));
                  for (const auto &side_region :
                       element->parentElement()->querySelectorsAll("side_region[location=" + new_location + "]"))
                    {
                      if (side_region != element)
                        {
                          side_region->setAttribute("location", value);
                          for (const auto &child : side_region->children())
                            {
                              if (child->hasAttribute("location")) child->setAttribute("location", value);
                            }
                          for (const auto &child : element->children())
                            {
                              if (child->hasAttribute("location")) child->setAttribute("location", new_location);
                            }
                          resetOldBoundingBoxes(side_region);
                        }
                    }
                }
              else if (element->localName() == "axis")
                {
                  std::shared_ptr<GRM::Element> plot_parent = element, new_parent;
                  getPlotParent(plot_parent);

                  auto new_location = static_cast<std::string>(element->getAttribute(attr));
                  auto parent = element->parentElement();

                  if (!strEqualsAny(std::string(value), "x", "y") ||
                      plot_parent->querySelectors("axis[location=\"" + std::string(value) + "\"]") ||
                      plot_parent->querySelectors("axis[location=\"" + new_location + "\"]"))
                    {
                      if ((strEqualsAny(std::string(value), "left", "right", "twin_y", "y") &&
                           strEqualsAny(new_location, "left", "right", "twin_y", "y")) ||
                          (strEqualsAny(std::string(value), "bottom", "top", "twin_x", "x") &&
                           strEqualsAny(new_location, "bottom", "top", "twin_x", "x")))
                        {
                          auto a = plot_parent->getAttribute("_" + std::string(value) + "_window_xform_a_org");
                          auto b = plot_parent->getAttribute("_" + std::string(value) + "_window_xform_b_org");

                          plot_parent->setAttribute("_" + new_location + "_window_xform_a_org", a);
                          plot_parent->setAttribute("_" + new_location + "_window_xform_a", a);
                          plot_parent->setAttribute("_" + new_location + "_window_xform_b_org", b);
                          plot_parent->setAttribute("_" + new_location + "_window_xform_b", b);

                          plot_parent->removeAttribute("_" + std::string(value) + "_window_xform_a");
                          plot_parent->removeAttribute("_" + std::string(value) + "_window_xform_b");
                          plot_parent->removeAttribute("_" + std::string(value) + "_window_xform_a_org");
                          plot_parent->removeAttribute("_" + std::string(value) + "_window_xform_b_org");

                          // change the reference on the series elements
                          for (const auto &series : plot_parent->querySelectors("central_region")->children())
                            {
                              if (!startsWith(series->localName(), "series_")) continue;

                              if (strEqualsAny(new_location, "bottom", "top", "twin_x", "x"))
                                {
                                  if ((series->hasAttribute("ref_x_axis_location") &&
                                       static_cast<std::string>(series->getAttribute("ref_x_axis_location")) ==
                                           std::string(value)) ||
                                      (std::string(value) == "x" && !series->hasAttribute("ref_x_axis_location")))
                                    {
                                      series->setAttribute("ref_x_axis_location", new_location);
                                    }
                                  else if ((series->hasAttribute("ref_x_axis_location") &&
                                            static_cast<std::string>(series->getAttribute("ref_x_axis_location")) ==
                                                new_location) ||
                                           (new_location == "x" && !series->hasAttribute("ref_x_axis_location")))
                                    {
                                      series->setAttribute("ref_x_axis_location", std::string(value));
                                    }
                                }
                              else if (strEqualsAny(new_location, "left", "right", "twin_y", "y"))
                                {
                                  if ((series->hasAttribute("ref_y_axis_location") &&
                                       static_cast<std::string>(series->getAttribute("ref_y_axis_location")) ==
                                           std::string(value)) ||
                                      (std::string(value) == "y" && !series->hasAttribute("ref_y_axis_location")))
                                    {
                                      series->setAttribute("ref_y_axis_location", new_location);
                                    }
                                  else if ((series->hasAttribute("ref_y_axis_location") &&
                                            static_cast<std::string>(series->getAttribute("ref_y_axis_location")) ==
                                                new_location) ||
                                           (new_location == "y" && !series->hasAttribute("ref_y_axis_location")))
                                    {
                                      series->setAttribute("ref_y_axis_location", std::string(value));
                                    }
                                }
                            }

                          // special cases to reset special twin-axis xform which allows a grid
                          if (element->hasAttribute("_twin_x_window_xform_a"))
                            {
                              element->removeAttribute("_twin_x_window_xform_a");
                              element->removeAttribute("_twin_x_window_xform_b");
                            }
                          if (element->hasAttribute("_twin_y_window_xform_a"))
                            {
                              element->removeAttribute("_twin_y_window_xform_a");
                              element->removeAttribute("_twin_y_window_xform_b");
                            }
                          if (strEqualsAny(new_location, "x", "y"))
                            {
                              auto axis = plot_parent->querySelectors("axis[location=\"twin_x\"");
                              if (axis != nullptr && axis->hasAttribute("_twin_x_window_xform_a"))
                                {
                                  axis->removeAttribute("_twin_x_window_xform_a");
                                  axis->removeAttribute("_twin_x_window_xform_b");
                                }
                              axis = plot_parent->querySelectors("axis[location=\"twin_y\"");
                              if (axis != nullptr && axis->hasAttribute("_twin_y_window_xform_a"))
                                {
                                  axis->removeAttribute("_twin_y_window_xform_a");
                                  axis->removeAttribute("_twin_y_window_xform_b");
                                }
                            }

                          // get the parent element for the new axis
                          if (strEqualsAny(new_location, "twin_x", "twin_y", "x", "y"))
                            new_parent = plot_parent->querySelectors("coordinate_system");
                          else if (strEqualsAny(new_location, "bottom", "left", "right", "top"))
                            new_parent = plot_parent->querySelectors("side_region[location=" + new_location + "]");

                          // figure out if an element needs to be swapped back
                          for (const auto &child : new_parent->querySelectorsAll(element->localName()))
                            {
                              if (child != element && child->localName() == "side_plot_region")
                                {
                                  parent->appendChild(child);
                                  child->setAttribute("_update_required", 1);
                                  child->setAttribute("location", value);
                                  resetOldBoundingBoxes(child);
                                }
                              else if (child != element &&
                                       static_cast<std::string>(child->getAttribute("location")) == new_location)
                                {
                                  child->setAttribute("_update_required", 1);
                                  child->setAttribute("location", value);
                                }
                            }

                          // move the old axis to the new parent
                          if (strEqualsAny(new_location, "twin_x", "twin_y", "x", "y"))
                            {
                              if (!strEqualsAny(std::string(value), "twin_x", "twin_y", "x", "y"))
                                new_parent->appendChild(element);
                              if (strEqualsAny(std::string(value), "x", "y"))
                                plot_parent->setAttribute("_update_required", true);
                              if (new_location == "twin_x")
                                {
                                  auto x_axis = plot_parent->querySelectors("axis[location=\"x\"]");
                                  x_axis->setAttribute("_update_required", true);
                                  x_axis->setAttribute("_delete_children", 2);
                                  x_axis->setAttribute("draw_grid", false);
                                  if (strEqualsAny(std::string(value), "x", "y"))
                                    plot_parent->setAttribute("_update_limits", true);
                                  if (!strEqualsAny(std::string(value), "twin_x", "twin_y", "x", "y")) parent->remove();
                                }
                              else if (new_location == "twin_y")
                                {
                                  auto y_axis = plot_parent->querySelectors("axis[location=\"y\"]");
                                  y_axis->setAttribute("_update_required", true);
                                  y_axis->setAttribute("_delete_children", 2);
                                  y_axis->setAttribute("draw_grid", false);
                                  if (strEqualsAny(std::string(value), "x", "y"))
                                    plot_parent->setAttribute("_update_limits", true);
                                  if (!strEqualsAny(std::string(value), "twin_x", "twin_y", "x", "y")) parent->remove();
                                }
                              else if (strEqualsAny(new_location, "x", "y"))
                                {
                                  auto axis = plot_parent->querySelectors("axis[location=\"" + value + "\"]");
                                  for (const auto &child : axis->children())
                                    {
                                      child->remove();
                                    }
                                  clearAxisAttributes(axis);
                                  axis->setAttribute("_update_required", true);
                                  plot_parent->setAttribute("_update_limits", true);
                                  if (!strEqualsAny(std::string(value), "twin_x", "twin_y"))
                                    {
                                      new_parent->appendChild(element);
                                      new_parent->setAttribute("viewport_offset", 0.02);
                                      if (new_location == "right") new_parent->setAttribute("viewport_offset", 0.05);
                                      new_parent->setAttribute("width", PLOT_DEFAULT_ADDITIONAL_AXIS_WIDTH);
                                      element->setAttribute("mirrored_axis", true);
                                      element->setAttribute("draw_grid", true);
                                      parent->appendChild(axis);
                                      axis->setAttribute("mirrored_axis", false);
                                      axis->setAttribute("line_color_ind", 1);
                                    }
                                }
                              for (const auto &child : element->children())
                                {
                                  child->remove();
                                }

                              clearAxisAttributes(element);
                              element->setAttribute("_update_required", true);
                            }
                          else if (strEqualsAny(new_location, "bottom", "left", "right", "top"))
                            {
                              if (strEqualsAny(std::string(value), "twin_x", "twin_y", "x", "y"))
                                {
                                  auto axis = new_parent->querySelectors("axis[location=\"" + value + "\"]");
                                  if (new_parent->querySelectors("side_plot_region") != nullptr)
                                    {
                                      new_parent->querySelectors("side_plot_region")->appendChild(element);
                                    }
                                  else
                                    {
                                      auto side_plot_region = global_creator->createSidePlotRegion();
                                      new_parent->appendChild(side_plot_region);
                                      side_plot_region->appendChild(element);
                                    }

                                  element->setAttribute("mirrored_axis", false);
                                  element->setAttribute("line_color_ind", 1);
                                  if (axis != nullptr)
                                    {
                                      parent->appendChild(axis);
                                      axis->setAttribute("mirrored_ticks", true);
                                      axis->setAttribute("draw_grid", true);
                                    }
                                  else if (std::string(value) == "twin_x")
                                    {
                                      auto x_axis = plot_parent->querySelectors("axis[location=\"x\"]");
                                      x_axis->setAttribute("mirrored_axis", true);
                                      x_axis->setAttribute("_delete_children", 2);
                                      x_axis->setAttribute("_update_required", true);
                                    }
                                  else if (std::string(value) == "twin_y")
                                    {
                                      auto y_axis = plot_parent->querySelectors("axis[location=\"y\"]");
                                      y_axis->setAttribute("mirrored_axis", true);
                                      y_axis->setAttribute("_delete_children", 2);
                                      y_axis->setAttribute("_update_required", true);
                                    }
                                }
                              else
                                {
                                  new_parent->appendChild(parent);
                                }
                              new_parent->setAttribute("viewport_offset", 0.02);
                              if (new_location == "right") new_parent->setAttribute("viewport_offset", 0.05);
                              new_parent->setAttribute("width", PLOT_DEFAULT_ADDITIONAL_AXIS_WIDTH);
                              for (const auto &child : element->children())
                                {
                                  child->remove();
                                }
                              clearAxisAttributes(element);
                              element->setAttribute("_update_required", true);
                              if (strEqualsAny(std::string(value), "x", "y"))
                                plot_parent->setAttribute("_update_limits", true);
                            }
                          for (const auto &elem : plot_parent->querySelectorsAll("[_default_diag_factor]"))
                            {
                              elem->removeAttribute("_default_diag_factor");
                            }
                          resetOldBoundingBoxes(new_parent);
                          resetOldBoundingBoxes(parent);
                        }
                      else
                        {
                          fprintf(stderr, "The location can only be swapped if the old and new location match the same "
                                          "axis_type.\n");
                        }
                    }
                  else
                    {
                      fprintf(stderr,
                              "The location swap is invalid. If the selected axis has the location x or y the new "
                              "location must match an existing axis. Therefore no location change was made.\n");
                      element->setAttribute("location", value);
                    }
                }
            }
          else if (element->localName() == "integral" &&
                   std::find(integral_critical_attributes.begin(), integral_critical_attributes.end(), attr) !=
                       integral_critical_attributes.end())
            {
              element->setAttribute("_update_required", true);
            }
          else if (element->parentElement() != nullptr && element->parentElement()->localName() == "integral" &&
                   element->localName() == "fill_area" && attr == "x_shift_wc")
            {
              element->parentElement()->setAttribute("_update_required", true);
            }
          else if (element->localName() == "tick_group")
            {
              auto val = static_cast<double>(element->getAttribute("value"));
              if (auto map_idx = static_cast<int>(element->parentElement()->getAttribute("_axis_id"));
                  tick_modification_map[map_idx][val].count(attr) > 0)
                {
                  tick_modification_map[map_idx][val][attr] = element->getAttribute(attr);
                }
              else
                {
                  tick_modification_map[map_idx][val].emplace(attr, element->getAttribute(attr));
                }
            }
          else if (element->localName() == "text" && element->parentElement()->localName() == "tick_group")
            {
              auto val = static_cast<double>(element->parentElement()->getAttribute("value"));
              if (auto map_idx = static_cast<int>(element->parentElement()->parentElement()->getAttribute("_axis_id"));
                  tick_modification_map[map_idx][val].count(attr) > 0)
                {
                  tick_modification_map[map_idx][val][attr] = element->getAttribute(attr);
                }
              else
                {
                  tick_modification_map[map_idx][val].emplace(attr, element->getAttribute(attr));
                }
            }
          else if (element->localName() == "tick" && element->parentElement()->localName() == "tick_group")
            {
              auto val = static_cast<double>(element->getAttribute("value"));
              if (auto map_idx = static_cast<int>(element->parentElement()->parentElement()->getAttribute("_axis_id"));
                  tick_modification_map[map_idx][val].count(attr) > 0)
                {
                  tick_modification_map[map_idx][val][attr] = element->getAttribute(attr);
                }
              else
                {
                  tick_modification_map[map_idx][val].emplace(attr, element->getAttribute(attr));
                }
            }
          else if (attr == "tick_size")
            {
              element->setAttribute("_tick_size_set_by_user", element->getAttribute(attr));
            }
          else if (attr == "x_lim_min" || attr == "x_lim_max")
            {
              if (auto coordinate_system = element->querySelectors("coordinate_system"))
                {
                  if (auto plot_type = static_cast<std::string>(coordinate_system->getAttribute("plot_type"));
                      plot_type == "polar")
                    {
                      for (const auto &child : coordinate_system->children())
                        {
                          if (child->localName() == "angle_axes")
                            {
                              child->setAttribute("_update_required", true);
                              child->setAttribute("_delete_children", 2);
                            }
                        }
                    }
                }
              else
                {
                  element->setAttribute("_update_required", true);
                  element->setAttribute("_delete_children", 2);
                }
            }
          else if (element->localName() == "plot" && attr == "polar_with_pan")
            {
              auto central_region = element->querySelectors("central_region");
              auto coordinate_system = central_region->querySelectors("coordinate_system");
              if (auto plot_type = static_cast<std::string>(coordinate_system->getAttribute("plot_type"));
                  plot_type == "polar")
                {
                  global_render->setClipRegion(central_region, !static_cast<int>(element->getAttribute(attr)));
                  global_render->setSelectSpecificXform(central_region, 1);
                  processClipRegion(central_region);
                  element->setAttribute("reset_ranges", 1);

                  if (std::stod(value) != static_cast<int>(element->getAttribute("polar_with_pan")))
                    {
                      coordinate_system->setAttribute("_update_required", true);
                      coordinate_system->setAttribute("_delete_children",
                                                      static_cast<int>(DelValues::RECREATE_OWN_CHILDREN));
                    }
                }
            }
          else if (element->localName() == "axis" && attr == "tick_orientation")
            {
              if (element->hasAttribute("_ignore_next_tick_orientation"))
                {
                  element->removeAttribute("_ignore_next_tick_orientation");
                  element->setAttribute("tick_orientation", value);
                }
            }
          else if (element->localName() == "axis" &&
                   strEqualsAny(attr, "adjust_x_lim", "adjust_y_lim", "x_lim_max", "x_lim_min", "y_lim_max",
                                "y_lim_min", "window_x_max", "window_x_min", "window_y_max", "window_y_min"))
            {
              auto plot_parent = element;
              getPlotParent(plot_parent);
              auto location = static_cast<std::string>(element->getAttribute("location"));
              if (strEqualsAny(location, "x", "y"))
                {
                  plot_parent->setAttribute("_update_limits", true);
                  clearAxisAttributes(element);
                }

              if (strEqualsAny(location, "twin_x", "twin_y", "x", "y"))
                {
                  auto axis = plot_parent->querySelectors("axis[location=\"twin_x\"");
                  if (axis != nullptr && axis->hasAttribute("_twin_x_window_xform_a"))
                    {
                      axis->removeAttribute("_twin_x_window_xform_a");
                      axis->removeAttribute("_twin_x_window_xform_b");
                    }
                  axis = plot_parent->querySelectors("axis[location=\"twin_y\"");
                  if (axis != nullptr && axis->hasAttribute("_twin_y_window_xform_a"))
                    {
                      axis->removeAttribute("_twin_y_window_xform_a");
                      axis->removeAttribute("_twin_y_window_xform_b");
                    }
                }
            }
          else if (element->localName() == "axis" && strEqualsAny(attr, "draw_grid", "mirrored_axis"))
            {
              element->setAttribute("_update_required", true);
              element->setAttribute("_delete_children", 2);
            }
          else if (attr == "orientation")
            {
              element->setAttribute("_update_required", true);
              for (const auto &child : element->children())
                {
                  if (startsWith(child->localName(), "series_")) child->setAttribute("_update_required", true);
                }
              resetOldBoundingBoxes(element->parentElement());
              resetOldBoundingBoxes(element);
              removeBoundingBoxId(*element);
            }
          else if (element->localName() == "layout_grid_element")
            {
              if (attr == "col_span")
                {
                  auto new_num = static_cast<int>(element->getAttribute(attr));
                  auto diff = new_num - std::stoi(value);
                  auto stop_col = static_cast<int>(element->getAttribute("_stop_col"));
                  auto start_col = static_cast<int>(element->getAttribute("_start_col"));
                  auto start_row = static_cast<int>(element->getAttribute("_start_row"));
                  element->setAttribute("_stop_col", stop_col + diff);

                  auto num_col = static_cast<int>(element->parentElement()->getAttribute("num_col"));
                  auto max = stop_col + diff;

                  for (const auto &grid_element : element->parentElement()->children())
                    {
                      if (grid_element != element)
                        {
                          auto grid_stop_col = static_cast<int>(grid_element->getAttribute("_stop_col"));
                          auto grid_start_col = static_cast<int>(grid_element->getAttribute("_start_col"));
                          auto grid_start_row = static_cast<int>(grid_element->getAttribute("_start_row"));
                          if (grid_start_col > start_col && grid_start_row >= start_row)
                            {
                              grid_element->setAttribute("_stop_col", grid_stop_col + diff);
                              grid_element->setAttribute("_start_col", grid_start_col + diff);
                              grid_element->setAttribute("position", std::to_string(grid_start_row) + " " +
                                                                         std::to_string(grid_start_col + diff));
                              max = grm_max(max, grid_stop_col + diff);
                            }
                        }
                    }
                  element->parentElement()->setAttribute("num_col", grm_max(num_col, max));
                }
              else if (attr == "row_span")
                {
                  auto new_num = static_cast<int>(element->getAttribute(attr));
                  auto diff = new_num - std::stoi(value);
                  auto stop_row = static_cast<int>(element->getAttribute("_stop_row"));
                  auto start_row = static_cast<int>(element->getAttribute("_start_row"));
                  auto start_col = static_cast<int>(element->getAttribute("_start_col"));
                  element->setAttribute("_stop_row", stop_row + diff);

                  auto num_row = static_cast<int>(element->parentElement()->getAttribute("num_row"));
                  auto max = stop_row + diff;

                  for (const auto &grid_element : element->parentElement()->children())
                    {
                      if (grid_element != element)
                        {
                          auto grid_stop_row = static_cast<int>(grid_element->getAttribute("_stop_row"));
                          auto grid_start_row = static_cast<int>(grid_element->getAttribute("_start_row"));
                          if (auto grid_start_col = static_cast<int>(grid_element->getAttribute("_start_col"));
                              grid_start_row > start_row && grid_start_col >= start_col)
                            {
                              grid_element->setAttribute("_start_row", grid_start_row + diff);
                              grid_element->setAttribute("_stop_row", grid_stop_row + diff);
                              grid_element->setAttribute("position", std::to_string(grid_start_row + diff) + " " +
                                                                         std::to_string(grid_start_col));
                              max = grm_max(max, grid_stop_row + diff);
                            }
                        }
                    }
                  element->parentElement()->setAttribute("num_row", grm_max(num_row, stop_row + diff));
                }
              else if (attr == "position")
                {
                  if (auto new_pos = static_cast<std::string>(element->getAttribute("position"));
                      element->parentElement()
                          ->querySelectorsAll("layout_grid_element[position=\"" + new_pos + "\"]")
                          .size() > 1)
                    {
                      for (const auto &grid_element : element->parentElement()->querySelectorsAll(
                               "layout_grid_element[position=\"" + new_pos + "\"]"))
                        {
                          if (grid_element != element)
                            {
                              auto elem_start_row = static_cast<int>(element->getAttribute("_start_row"));
                              auto elem_stop_row = static_cast<int>(element->getAttribute("_stop_row"));
                              auto elem_start_col = static_cast<int>(element->getAttribute("_start_col"));
                              auto elem_stop_col = static_cast<int>(element->getAttribute("_stop_col"));
                              auto elem_col_span = static_cast<int>(element->getAttribute("col_span"));
                              auto elem_row_span = static_cast<int>(element->getAttribute("row_span"));
                              auto grid_start_row = static_cast<int>(grid_element->getAttribute("_start_row"));
                              auto grid_stop_row = static_cast<int>(grid_element->getAttribute("_stop_row"));
                              auto grid_start_col = static_cast<int>(grid_element->getAttribute("_start_col"));
                              auto grid_stop_col = static_cast<int>(grid_element->getAttribute("_stop_col"));
                              auto grid_col_span = static_cast<int>(grid_element->getAttribute("col_span"));
                              auto grid_row_span = static_cast<int>(grid_element->getAttribute("row_span"));

                              element->setAttribute("_start_row", grid_start_row);
                              element->setAttribute("_start_col", grid_start_col);
                              grid_element->setAttribute("_start_row", elem_start_row);
                              grid_element->setAttribute("_start_col", elem_start_col);

                              if (element->hasAttribute("keep_size_if_swapped") &&
                                  static_cast<int>(element->getAttribute("keep_size_if_swapped")))
                                {
                                  element->setAttribute("_stop_row", grid_start_row + elem_row_span);
                                  element->setAttribute("_stop_col", grid_start_col + elem_col_span);
                                  grid_element->setAttribute("_stop_row", elem_start_row + grid_row_span);
                                  grid_element->setAttribute("_stop_col", elem_start_col + grid_col_span);

                                  auto max_col =
                                      grm_max(grid_start_col + elem_col_span, elem_start_col + grid_col_span);
                                  auto max_row =
                                      grm_max(grid_start_row + elem_row_span, elem_start_row + grid_row_span);
                                  for (const auto &elem : element->parentElement()->children())
                                    {
                                      if (elem == grid_element || elem == element) continue;
                                      auto stop_row = static_cast<int>(elem->getAttribute("_stop_row"));
                                      auto start_row = static_cast<int>(elem->getAttribute("_start_row"));
                                      auto start_col = static_cast<int>(elem->getAttribute("_start_col"));
                                      auto stop_col = static_cast<int>(elem->getAttribute("_stop_col"));
                                      if (start_row > elem_start_row && start_col >= elem_start_col)
                                        {
                                          elem->setAttribute("_start_row", start_row + (start_row - elem_start_row));
                                          elem->setAttribute("_stop_row", stop_row + (start_row - elem_start_row));
                                          elem->setAttribute("position",
                                                             std::to_string(start_row + (start_row - elem_start_row)) +
                                                                 " " + std::to_string(start_col));
                                          max_row = grm_max(max_row, stop_row + (start_row - elem_start_row));
                                        }
                                      if (start_col > elem_start_col && start_row >= elem_start_row)
                                        {
                                          elem->setAttribute("_stop_col", stop_col + (start_col - elem_start_col));
                                          elem->setAttribute("_start_col", start_col + (start_col - elem_start_col));
                                          elem->setAttribute(
                                              "position", std::to_string(start_row) + " " +
                                                              std::to_string(start_col + (start_col - elem_start_col)));
                                          max_col = grm_max(max_col, stop_col + (start_col - elem_start_col));
                                        }
                                      if (start_row > grid_start_row && start_col >= grid_start_col)
                                        {
                                          elem->setAttribute("_start_row", start_row + (start_row - grid_start_row));
                                          elem->setAttribute("_stop_row", stop_row + (start_row - grid_start_row));
                                          elem->setAttribute("position",
                                                             std::to_string(start_row + (start_row - grid_start_row)) +
                                                                 " " + std::to_string(start_col));
                                          max_row = grm_max(max_row, stop_row + (start_row - grid_start_row));
                                        }
                                      if (start_col > grid_start_col && start_row >= grid_start_row)
                                        {
                                          elem->setAttribute("_stop_col", stop_col + (start_col - grid_start_col));
                                          elem->setAttribute("_start_col", start_col + (start_col - grid_start_col));
                                          elem->setAttribute(
                                              "position", std::to_string(start_row) + " " +
                                                              std::to_string(start_col + (start_col - grid_start_col)));
                                          max_col = grm_max(max_col, stop_col + (start_col - grid_start_col));
                                        }
                                    }

                                  auto num_col = static_cast<int>(element->parentElement()->getAttribute("num_col"));
                                  auto num_row = static_cast<int>(element->parentElement()->getAttribute("num_row"));
                                  element->parentElement()->setAttribute("num_col", grm_max(num_col, max_col));
                                  element->parentElement()->setAttribute("num_row", grm_max(num_row, max_row));
                                }
                              else
                                {
                                  element->setAttribute("_stop_row", grid_stop_row);
                                  element->setAttribute("_stop_col", grid_stop_col);
                                  element->setAttribute("col_span", grid_col_span);
                                  element->setAttribute("row_span", grid_row_span);
                                  grid_element->setAttribute("_stop_row", elem_stop_row);
                                  grid_element->setAttribute("_stop_col", elem_stop_col);
                                  grid_element->setAttribute("col_span", elem_col_span);
                                  grid_element->setAttribute("row_span", elem_row_span);
                                }

                              grid_element->setAttribute("position", value);
                              break;
                            }
                        }
                    }
                  else
                    {
                      auto new_row = std::stoi(GRM::split(new_pos, " ")[0]);
                      auto new_col = std::stoi(GRM::split(new_pos, " ")[1]);
                      auto num_col = static_cast<int>(element->parentElement()->getAttribute("num_col"));
                      auto num_row = static_cast<int>(element->parentElement()->getAttribute("num_row"));
                      auto col_span = static_cast<int>(element->getAttribute("col_span"));
                      auto row_span = static_cast<int>(element->getAttribute("row_span"));

                      // check if number of rows or cols must be adjusted
                      if (new_row + row_span > num_row)
                        element->parentElement()->setAttribute("num_row", new_row + row_span);
                      if (new_col + col_span > num_col)
                        element->parentElement()->setAttribute("num_col", new_col + col_span);

                      element->setAttribute("_start_row", new_row);
                      element->setAttribute("_start_col", new_col);
                      element->setAttribute("_stop_row", new_row + row_span);
                      element->setAttribute("_stop_col", new_col + col_span);
                    }
                }
              if (element->parentElement()->hasAttribute("trim_col") &&
                  static_cast<int>(element->parentElement()->getAttribute("trim_col")))
                {
                  const auto num_col = static_cast<int>(element->parentElement()->getAttribute("num_col"));
                  const auto num_row = static_cast<int>(element->parentElement()->getAttribute("num_row"));
                  std::vector<std::vector<int>> grid_matrix;
                  // min num of empty space which all cols/rows contains
                  int min_empty_col_num = num_col;

                  // initialize grid_matrix with layout grid
                  for (int i = 0; i < num_row; i++)
                    {
                      std::vector<int> row;
                      for (int j = 0; j < num_col; j++) row.push_back(-1);
                      grid_matrix.push_back(row);
                    }
                  for (const auto &grid_elem : element->parentElement()->children())
                    {
                      auto start_col = static_cast<int>(grid_elem->getAttribute("_start_col"));
                      auto stop_col = static_cast<int>(grid_elem->getAttribute("_stop_col"));
                      auto start_row = static_cast<int>(grid_elem->getAttribute("_start_row"));
                      auto stop_row = static_cast<int>(grid_elem->getAttribute("_stop_row"));
                      auto pos = static_cast<std::string>(grid_elem->getAttribute("position"));

                      for (int i = start_row; i < stop_row; i++)
                        {
                          for (int j = start_col; j < stop_col; j++)
                            {
                              grid_matrix[i][j] = 1;
                            }
                        }
                    }

                  // calculate amount of columns which can be removed in each row
                  for (int i = 0; i < num_row; i++)
                    {
                      int cnt = 0;
                      for (int j = 0; j < num_col; j++)
                        {
                          if (grid_matrix[i][j] == -1) cnt += 1;
                        }
                      min_empty_col_num = grm_min(min_empty_col_num, cnt);
                    }

                  // remove min num of empty grid elements in each row
                  /* a bit tricky is "a x x b c" with min_empty_col = 1, cause b and c must be moved 1 to the
                  left and then num_col on the parent element has to be changed to remove to useless col */
                  if (min_empty_col_num > 0)
                    {
                      for (int i = 0; i < num_row; i++)
                        {
                          int removed_spaces = 0;
                          for (int j = 0; j < num_col; j++)
                            {
                              if (grid_matrix[i][j] == -1 && removed_spaces < min_empty_col_num)
                                {
                                  removed_spaces += 1;
                                }
                              else if (removed_spaces != 0 && grid_matrix[i][j] != -1)
                                {
                                  auto grid_elem = element->parentElement()->querySelectors(
                                      "layout_grid_element[position=\"" + std::to_string(i) + " " + std::to_string(j) +
                                      "\"]");
                                  if (grid_elem != nullptr)
                                    {
                                      auto start_col = static_cast<int>(grid_elem->getAttribute("_start_col"));
                                      auto stop_col = static_cast<int>(grid_elem->getAttribute("_stop_col"));
                                      auto start_row = static_cast<int>(grid_elem->getAttribute("_start_row"));

                                      grid_elem->setAttribute("_start_col", start_col - removed_spaces);
                                      grid_elem->setAttribute("_stop_col", stop_col - removed_spaces);
                                      grid_elem->setAttribute("position",
                                                              std::to_string(start_row) + " " +
                                                                  std::to_string(start_col - removed_spaces));
                                    }
                                }
                            }
                        }
                      GRM::Render::setAutoUpdate(false);
                      element->parentElement()->setAttribute("num_col", num_col - min_empty_col_num);
                      GRM::Render::setAutoUpdate(true);
                    }
                }
              if (element->parentElement()->hasAttribute("trim_row") &&
                  static_cast<int>(element->parentElement()->getAttribute("trim_row")))
                {
                  const auto num_col = static_cast<int>(element->parentElement()->getAttribute("num_col"));
                  const auto num_row = static_cast<int>(element->parentElement()->getAttribute("num_row"));
                  std::vector<std::vector<int>> grid_matrix;
                  // min num of empty space which all cols/rows contains
                  int min_empty_row_num = num_row;

                  // initialize grid_matrix with layout grid
                  for (int i = 0; i < num_row; i++)
                    {
                      std::vector<int> row;
                      for (int j = 0; j < num_col; j++)
                        {
                          row.push_back(-1);
                        }
                      grid_matrix.push_back(row);
                    }
                  for (const auto &grid_elem : element->parentElement()->children())
                    {
                      auto start_col = static_cast<int>(grid_elem->getAttribute("_start_col"));
                      auto stop_col = static_cast<int>(grid_elem->getAttribute("_stop_col"));
                      auto start_row = static_cast<int>(grid_elem->getAttribute("_start_row"));
                      auto stop_row = static_cast<int>(grid_elem->getAttribute("_stop_row"));
                      auto pos = static_cast<std::string>(grid_elem->getAttribute("position"));

                      for (int i = start_row; i < stop_row; i++)
                        {
                          for (int j = start_col; j < stop_col; j++)
                            {
                              grid_matrix[i][j] = 1;
                            }
                        }
                    }

                  // calculate amount of rows which can be removed in each col
                  for (int j = 0; j < num_col; j++)
                    {
                      int cnt = 0;
                      for (int i = 0; i < num_row; i++)
                        {
                          if (grid_matrix[i][j] == -1) cnt += 1;
                        }
                      min_empty_row_num = grm_min(min_empty_row_num, cnt);
                    }

                  // remove min num of empty grid elements in each col
                  if (min_empty_row_num > 0)
                    {
                      for (int j = 0; j < num_col; j++)
                        {
                          int removed_spaces = 0;
                          for (int i = 0; i < num_row; i++)
                            {
                              if (grid_matrix[i][j] == -1 && removed_spaces < min_empty_row_num)
                                {
                                  removed_spaces += 1;
                                }
                              else if (removed_spaces != 0 && grid_matrix[i][j] != -1)
                                {
                                  auto grid_elem = element->parentElement()->querySelectors(
                                      "layout_grid_element[position=\"" + std::to_string(i) + " " + std::to_string(j) +
                                      "\"]");
                                  if (grid_elem != nullptr)
                                    {
                                      auto start_row = static_cast<int>(grid_elem->getAttribute("_start_row"));
                                      auto stop_row = static_cast<int>(grid_elem->getAttribute("_stop_row"));
                                      auto start_col = static_cast<int>(grid_elem->getAttribute("_start_col"));

                                      grid_elem->setAttribute("_start_row", start_row - removed_spaces);
                                      grid_elem->setAttribute("_stop_row", stop_row - removed_spaces);
                                      grid_elem->setAttribute("position", std::to_string(start_row - removed_spaces) +
                                                                              " " + std::to_string(start_col));
                                    }
                                }
                            }
                        }
                      GRM::Render::setAutoUpdate(false);
                      element->parentElement()->setAttribute("num_row", num_row - min_empty_row_num);
                      GRM::Render::setAutoUpdate(true);
                    }
                }
              if (strEqualsAny(attr, "fit_parents_height", "fit_parents_width", "col_span", "row_span", "position"))
                {
                  for (const auto &grid_element : element->parentElement()->children())
                    {
                      grid_element->querySelectors("plot")->removeAttribute("_start_aspect_ratio");
                      for (const auto &elem : grid_element->querySelectorsAll("[_default_diag_factor]"))
                        {
                          elem->removeAttribute("_default_diag_factor");
                          resetOldBoundingBoxes(elem);
                        }
                      resetOldBoundingBoxes(grid_element);
                    }

                  // imshow needs vp which gets inflicted by the size attributes in figure, this means when the size is
                  // changed the imshow_series must be recalculated
                  auto imshow = global_render->querySelectorsAll("series_imshow");
                  for (const auto &imshow_elem : imshow)
                    {
                      imshow_elem->setAttribute("_update_required", true);
                    }
                }
            }
          else if (element->localName() == "layout_grid")
            {
              if (attr == "flip_col_and_row" && static_cast<int>(element->getAttribute(attr)))
                {
                  auto num_col = static_cast<int>(element->getAttribute("num_col"));
                  auto num_row = static_cast<int>(element->getAttribute("num_row"));
                  auto fit_parents_height = static_cast<int>(element->getAttribute("fit_parents_height"));
                  auto fit_parents_width = static_cast<int>(element->getAttribute("fit_parents_width"));
                  element->setAttribute("num_col", num_row);
                  element->setAttribute("num_row", num_col);
                  element->setAttribute("fit_parents_height", fit_parents_width);
                  element->setAttribute("fit_parents_width", fit_parents_height);

                  for (const auto &grid_element : element->children())
                    {
                      auto legend = grid_element->querySelectors("legend");
                      auto start_col = static_cast<int>(grid_element->getAttribute("_start_col"));
                      auto stop_col = static_cast<int>(grid_element->getAttribute("_stop_col"));
                      auto start_row = static_cast<int>(grid_element->getAttribute("_start_row"));
                      auto stop_row = static_cast<int>(grid_element->getAttribute("_stop_row"));
                      auto viewport_normalized_x_min =
                          static_cast<double>(grid_element->getAttribute("_viewport_normalized_x_min_org"));
                      auto viewport_normalized_x_max =
                          static_cast<double>(grid_element->getAttribute("_viewport_normalized_x_max_org"));
                      auto viewport_normalized_y_min =
                          static_cast<double>(grid_element->getAttribute("_viewport_normalized_y_min_org"));
                      auto viewport_normalized_y_max =
                          static_cast<double>(grid_element->getAttribute("_viewport_normalized_y_max_org"));
                      fit_parents_height = static_cast<int>(grid_element->getAttribute("fit_parents_height"));
                      fit_parents_width = static_cast<int>(grid_element->getAttribute("fit_parents_width"));

                      grid_element->setAttribute("_start_col", start_row);
                      grid_element->setAttribute("_stop_col", stop_row);
                      grid_element->setAttribute("_start_row", start_col);
                      grid_element->setAttribute("_stop_row", stop_col);
                      grid_element->setAttribute("viewport_normalized_x_min", viewport_normalized_y_min);
                      grid_element->setAttribute("viewport_normalized_x_max", viewport_normalized_y_max);
                      grid_element->setAttribute("viewport_normalized_y_min", viewport_normalized_x_min);
                      grid_element->setAttribute("viewport_normalized_y_max", viewport_normalized_x_max);
                      grid_element->setAttribute("_viewport_normalized_x_min_org", viewport_normalized_y_min);
                      grid_element->setAttribute("_viewport_normalized_x_max_org", viewport_normalized_y_max);
                      grid_element->setAttribute("_viewport_normalized_y_min_org", viewport_normalized_x_min);
                      grid_element->setAttribute("_viewport_normalized_y_max_org", viewport_normalized_x_max);
                      grid_element->setAttribute("fit_parents_height", fit_parents_width);
                      grid_element->setAttribute("fit_parents_width", fit_parents_height);

                      grid_element->querySelectors("plot")->removeAttribute("_start_aspect_ratio");
                      for (const auto &elem : grid_element->querySelectorsAll("[_default_diag_factor]"))
                        {
                          elem->removeAttribute("_default_diag_factor");
                        }
                      if (legend != nullptr)
                        {
                          legend->removeAttribute("_initial_scale_factor");
                          legend->removeAttribute("_scale_factor");
                          legend->removeAttribute("_start_w");
                          legend->removeAttribute("_start_h");
                        }
                      resetOldBoundingBoxes(grid_element);
                    }

                  element->setAttribute("flip_col_and_row", false);
                }
              else if (strEqualsAny(attr, "num_col", "num_row"))
                {
                  for (const auto &grid_element : element->children())
                    {
                      auto legend = grid_element->querySelectors("legend");
                      grid_element->querySelectors("plot")->removeAttribute("_start_aspect_ratio");
                      for (const auto &elem : grid_element->querySelectorsAll("[_default_diag_factor]"))
                        {
                          elem->removeAttribute("_default_diag_factor");
                        }
                      if (legend != nullptr)
                        {
                          legend->removeAttribute("_initial_scale_factor");
                          legend->removeAttribute("_scale_factor");
                          legend->removeAttribute("_start_w");
                          legend->removeAttribute("_start_h");
                        }
                      if (std::stoi(value) > static_cast<int>(element->getAttribute("num_col")))
                        element->setAttribute("num_col", stoi(value));
                      if (std::stoi(value) > static_cast<int>(element->getAttribute("num_row")))
                        element->setAttribute("num_row", stoi(value));
                      resetOldBoundingBoxes(grid_element);
                    }
                }
              else if (strEqualsAny(attr, "start_col", "start_col", "stop_col", "stop_row"))
                {
                  if (strEqualsAny(attr, "stop_col", "stop_row"))
                    {
                      auto num_col = static_cast<int>(element->getAttribute("num_col"));
                      auto num_row = static_cast<int>(element->getAttribute("num_row"));
                      auto end_val = static_cast<int>(element->getAttribute(attr));
                      if (attr == "stop_col")
                        element->setAttribute("num_col", grm_max(num_col, end_val));
                      else
                        element->setAttribute("num_row", grm_max(num_row, end_val));
                    }
                  else if (strEqualsAny(attr, "start_col", "start_row"))
                    {
                      auto start_val = static_cast<int>(element->getAttribute(attr));
                      if (attr == "start_col")
                        element->setAttribute("col_span",
                                              static_cast<int>(element->getAttribute("stop_col")) - start_val);
                      else
                        element->setAttribute("row_span",
                                              static_cast<int>(element->getAttribute("stop_row")) - start_val);
                    }

                  for (const auto &grid_element : element->parentElement()->children())
                    {
                      grid_element->querySelectors("plot")->removeAttribute("_start_aspect_ratio");
                      for (const auto &elem : grid_element->querySelectorsAll("[_default_diag_factor]"))
                        {
                          elem->removeAttribute("_default_diag_factor");
                          resetOldBoundingBoxes(elem);
                        }
                      resetOldBoundingBoxes(grid_element);
                    }
                }
              else if (attr == "trim_row" || attr == "trim_col")
                {
                  if (static_cast<int>(element->getAttribute(attr)))
                    {
                      const auto num_col = static_cast<int>(element->getAttribute("num_col"));
                      const auto num_row = static_cast<int>(element->getAttribute("num_row"));
                      std::vector<std::vector<int>> grid_matrix;
                      // min num of empty space which all cols/rows contains
                      int min_empty_num = attr == "trim_row" ? num_row : num_col;

                      // initialize grid_matrix with layout grid
                      for (int i = 0; i < num_row; i++)
                        {
                          std::vector<int> row;
                          for (int j = 0; j < num_col; j++) row.push_back(-1);
                          grid_matrix.push_back(row);
                        }
                      for (const auto &grid_elem : element->children())
                        {
                          auto start_col = static_cast<int>(grid_elem->getAttribute("_start_col"));
                          auto stop_col = static_cast<int>(grid_elem->getAttribute("_stop_col"));
                          auto start_row = static_cast<int>(grid_elem->getAttribute("_start_row"));
                          auto stop_row = static_cast<int>(grid_elem->getAttribute("_stop_row"));

                          for (int i = start_row; i < stop_row; i++)
                            {
                              for (int j = start_col; j < stop_col; j++)
                                {
                                  grid_matrix[i][j] = 1;
                                }
                            }
                        }

                      // calculate amount of rows/cols which can be removed in each col/row
                      for (int j = 0; j < (attr == "trim_row" ? num_col : num_row); j++)
                        {
                          int cnt = 0;
                          for (int i = 0; i < (attr == "trim_row" ? num_row : num_col); i++)
                            {
                              if ((attr == "trim_row" && grid_matrix[i][j] == -1) ||
                                  (attr == "trim_col" && grid_matrix[j][i] == -1))
                                cnt += 1;
                            }
                          min_empty_num = grm_min(min_empty_num, cnt);
                        }

                      // remove min num of empty grid elements in each col/row
                      if (min_empty_num > 0)
                        {
                          for (int j = 0; j < (attr == "trim_row" ? num_col : num_row); j++)
                            {
                              int removed_spaces = 0;
                              for (int i = 0; i < (attr == "trim_row" ? num_row : num_col); i++)
                                {
                                  int matrix_entry;
                                  if (attr == "trim_row")
                                    matrix_entry = grid_matrix[i][j];
                                  else
                                    matrix_entry = grid_matrix[j][i];
                                  if (matrix_entry == -1 && removed_spaces < min_empty_num)
                                    {
                                      removed_spaces += 1;
                                    }
                                  else if (removed_spaces != 0 && matrix_entry != -1)
                                    {
                                      std::shared_ptr<GRM::Element> grid_elem;
                                      if (attr == "trim_row")
                                        {
                                          grid_elem = element->querySelectors("layout_grid_element[position=\"" +
                                                                              std::to_string(i) + " " +
                                                                              std::to_string(j) + "\"]");
                                        }
                                      else
                                        {
                                          grid_elem = element->querySelectors("layout_grid_element[position=\"" +
                                                                              std::to_string(j) + " " +
                                                                              std::to_string(i) + "\"]");
                                        }
                                      if (grid_elem != nullptr)
                                        {
                                          auto start_row = static_cast<int>(grid_elem->getAttribute("_start_row"));
                                          auto stop_row = static_cast<int>(grid_elem->getAttribute("_stop_row"));
                                          auto start_col = static_cast<int>(grid_elem->getAttribute("_start_col"));
                                          auto stop_col = static_cast<int>(grid_elem->getAttribute("_stop_col"));

                                          if (attr == "trim_row")
                                            {
                                              grid_elem->setAttribute("_start_row", start_row - removed_spaces);
                                              grid_elem->setAttribute("_stop_row", stop_row - removed_spaces);
                                              grid_elem->setAttribute("position",
                                                                      std::to_string(start_row - removed_spaces) + " " +
                                                                          std::to_string(start_col));
                                            }
                                          else
                                            {
                                              grid_elem->setAttribute("_start_col", start_col - removed_spaces);
                                              grid_elem->setAttribute("_stop_col", stop_col - removed_spaces);
                                              grid_elem->setAttribute("position",
                                                                      std::to_string(start_row) + " " +
                                                                          std::to_string(start_col - removed_spaces));
                                            }
                                        }
                                    }
                                }
                            }
                          GRM::Render::setAutoUpdate(false);
                          if (attr == "trim_row")
                            element->setAttribute("num_row", num_row - min_empty_num);
                          else
                            element->setAttribute("num_col", num_col - min_empty_num);
                          GRM::Render::setAutoUpdate(true);

                          for (const auto &grid_element : element->children())
                            {
                              grid_element->querySelectors("plot")->removeAttribute("_start_aspect_ratio");
                              for (const auto &elem : grid_element->querySelectorsAll("[_default_diag_factor]"))
                                {
                                  elem->removeAttribute("_default_diag_factor");
                                  resetOldBoundingBoxes(elem);
                                }
                              resetOldBoundingBoxes(grid_element);
                            }
                        }
                    }
                }

              // imshow needs vp which gets inflicted by the size attributes in figure, this means when the size is
              // changed the imshow_series must be recalculated
              auto imshow = global_render->querySelectorsAll("series_imshow");
              for (const auto &imshow_elem : imshow)
                {
                  imshow_elem->setAttribute("_update_required", true);
                }
            }
          else if (attr == "text_content" && element->localName() == "side_region")
            {
              auto plot_parent = element;
              getPlotParent(plot_parent);

              if (!element->hasChildNodes())
                {
                  for (const auto &elem : plot_parent->querySelectorsAll("[_default_diag_factor]"))
                    {
                      elem->removeAttribute("_default_diag_factor");
                    }
                }
            }

          if (strEqualsAny(attr, "ws_viewport_x_min", "ws_viewport_x_max", "ws_viewport_y_min", "ws_viewport_y_max"))
            {
              element->setAttribute("_ws_viewport_set_by_user", true);
            }
          else if (strEqualsAny(attr, "ws_window_x_min", "ws_window_x_max", "ws_window_y_min", "ws_window_y_max"))
            {
              element->setAttribute("_ws_window_set_by_user", true);
            }
          else if (attr == "char_height")
            {
              element->setAttribute("_char_height_set_by_user", element->getAttribute(attr));
            }
          else if (strEqualsAny(attr, "char_up_x", "char_up_y"))
            {
              element->setAttribute("_char_up_set_by_user", true);
            }
          else if (strEqualsAny(attr, "window_x_min", "window_x_max", "window_y_min", "window_y_max"))
            {
              element->setAttribute("_window_set_by_user", true);
            }
          else if (strEqualsAny(attr, "viewport_normalized_x_min", "viewport_normalized_x_max",
                                "viewport_normalized_y_min", "viewport_normalized_y_max"))
            {
              if (attr == "viewport_normalized_x_min" && element->localName() == "layout_grid_element")
                element->setAttribute("_viewport_normalized_x_min_set_by_user", true);
              else if (attr == "viewport_normalized_x_max" && element->localName() == "layout_grid_element")
                element->setAttribute("_viewport_normalized_x_max_set_by_user", true);
              else if (attr == "viewport_normalized_y_min" && element->localName() == "layout_grid_element")
                element->setAttribute("_viewport_normalized_y_min_set_by_user", true);
              else if (attr == "viewport_normalized_y_max" && element->localName() == "layout_grid_element")
                element->setAttribute("_viewport_normalized_y_max_set_by_user", true);
              element->setAttribute("_viewport_normalized_x_min_org",
                                    static_cast<double>(element->getAttribute("viewport_normalized_x_min")));
              element->setAttribute("_viewport_normalized_x_max_org",
                                    static_cast<double>(element->getAttribute("viewport_normalized_x_max")));
              element->setAttribute("_viewport_normalized_y_min_org",
                                    static_cast<double>(element->getAttribute("viewport_normalized_y_min")));
              element->setAttribute("_viewport_normalized_y_max_org",
                                    static_cast<double>(element->getAttribute("viewport_normalized_y_max")));
            }
          else if (strEqualsAny(attr, "viewport_offset"))
            {
              element->setAttribute("_viewport_offset_set_by_user", true);
            }
          else if (strEqualsAny(attr, "width"))
            {
              element->setAttribute("_width_set_by_user", true);
            }
          else if (strEqualsAny(attr, "label_orientation"))
            {
              element->setAttribute("_label_orientation_set_by_user", true);
            }
          else if (strEqualsAny(attr, "label_pos"))
            {
              element->setAttribute("_label_pos_set_by_user", true);
            }
          else if (strEqualsAny(attr, "min_value"))
            {
              auto min_value = static_cast<double>(element->getAttribute("min_value"));
              element->setAttribute("_min_value_set_by_user", min_value);
            }
          else if (strEqualsAny(attr, "max_value"))
            {
              auto max_value = static_cast<double>(element->getAttribute("max_value"));
              element->setAttribute("_max_value_set_by_user", max_value);
            }
          else if (strEqualsAny(attr, "tick"))
            {
              auto tick = static_cast<double>(element->getAttribute("tick"));
              element->setAttribute("_tick_set_by_user", tick);
              element->setAttribute("_delete_children", 2);
              element->setAttribute("_update_required", true);
            }
          else if (strEqualsAny(attr, "origin"))
            {
              auto org = static_cast<double>(element->getAttribute("origin"));
              element->setAttribute("_origin_set_by_user", org);
            }
          else if (strEqualsAny(attr, "pos"))
            {
              auto pos = static_cast<double>(element->getAttribute("pos"));
              element->setAttribute("_pos_set_by_user", pos);
            }
          else if (strEqualsAny(attr, "major_count"))
            {
              auto major_count = static_cast<int>(element->getAttribute("major_count"));
              element->setAttribute("_major_count_set_by_user", major_count);
              element->setAttribute("_delete_children", 2);
              element->setAttribute("_update_required", true);
            }
          else if (strEqualsAny(attr, "num_ticks"))
            {
              auto num_ticks = static_cast<int>(element->getAttribute("num_ticks"));
              element->setAttribute("_num_ticks_set_by_user", num_ticks);
              element->setAttribute("_delete_children", 2);
              element->setAttribute("_update_required", true);
            }
          else if (strEqualsAny(attr, "num_tick_labels"))
            {
              auto num_tick_labels = static_cast<int>(element->getAttribute("num_tick_labels"));
              element->setAttribute("_num_tick_labels_set_by_user", num_tick_labels);
              element->setAttribute("_delete_children", 2);
              element->setAttribute("_update_required", true);
            }
          else if (strEqualsAny(attr, "tick_size"))
            {
              auto tick_size = static_cast<double>(element->getAttribute("tick_size"));
              element->setAttribute("_tick_size_set_by_user", tick_size);
            }
          else if (strEqualsAny(attr, "tick_orientation"))
            {
              auto tick_orientation = static_cast<int>(element->getAttribute("tick_orientation"));
              element->setAttribute("_tick_orientation_set_by_user", tick_orientation);
              element->setAttribute("_delete_children", 2);
              element->setAttribute("_update_required", true);
            }
          else if (strEqualsAny(attr, "value") && strEqualsAny(element->localName(), "grid_line", "tick"))
            {
              auto val = static_cast<double>(element->getAttribute("value"));
              element->setAttribute("_value_set_by_user", val);
            }
          else if (strEqualsAny(attr, "is_major") && element->localName() == "grid_line")
            {
              auto is_major = static_cast<int>(element->getAttribute("is_major"));
              element->setAttribute("_is_major_set_by_user", is_major);
            }
          else if (strEqualsAny(attr, "world_coordinates"))
            {
              auto world_coordinates = static_cast<int>(element->getAttribute("world_coordinates"));
              element->setAttribute("_world_coordinates_set_by_user", world_coordinates);
            }
          else if (strEqualsAny(attr, "fill_color_ind") && element->localName() == "legend")
            {
              element->setAttribute("_fill_color_ind_set_by_user", true);
            }
          else if (strEqualsAny(attr, "fill_int_style") && element->localName() == "legend")
            {
              element->setAttribute("_fill_int_style_set_by_user", true);
            }
          else if (strEqualsAny(attr, "scale") && element->localName() == "legend")
            {
              element->setAttribute("_scale_set_by_user", true);
            }
          else if (strEqualsAny(attr, "select_specific_xform") && element->localName() == "legend")
            {
              element->setAttribute("_select_specific_xform_set_by_user", 1);
            }
          else if (strEqualsAny(attr, "text_color_ind") && element->localName() == "bar")
            {
              element->setAttribute("_text_color_ind_set_by_user", true);
            }
          else if (strEqualsAny(attr, "num_col") &&
                   strEqualsAny(element->localName(), "cell_array", "nonuniform_cell_array"))
            {
              auto num_col = static_cast<int>(element->getAttribute("num_col"));
              element->setAttribute("_num_col_set_by_user", num_col);
            }
          else if (strEqualsAny(attr, "num_row") &&
                   strEqualsAny(element->localName(), "cell_array", "nonuniform_cell_array"))
            {
              auto num_row = static_cast<int>(element->getAttribute("num_row"));
              element->setAttribute("_num_row_set_by_user", num_row);
            }
          else if (strEqualsAny(attr, "start_col") &&
                   strEqualsAny(element->localName(), "cell_array", "nonuniform_cell_array"))
            {
              auto start_col = static_cast<int>(element->getAttribute("start_col"));
              element->setAttribute("_start_col_set_by_user", start_col);
            }
          else if (strEqualsAny(attr, "start_row") &&
                   strEqualsAny(element->localName(), "cell_array", "nonuniform_cell_array"))
            {
              auto start_row = static_cast<int>(element->getAttribute("start_row"));
              element->setAttribute("_start_row_set_by_user", start_row);
            }
          else if (strEqualsAny(attr, "x_dim") &&
                   strEqualsAny(element->localName(), "cell_array", "nonuniform_cell_array"))
            {
              auto x_dim = static_cast<int>(element->getAttribute("x_dim"));
              element->setAttribute("_x_dim_set_by_user", x_dim);
            }
          else if (strEqualsAny(attr, "x_min") &&
                   strEqualsAny(element->localName(), "cell_array", "nonuniform_cell_array"))
            {
              auto x_min = static_cast<double>(element->getAttribute("x_min"));
              element->setAttribute("_x_min_set_by_user", x_min);
            }
          else if (strEqualsAny(attr, "x_max") &&
                   strEqualsAny(element->localName(), "cell_array", "nonuniform_cell_array"))
            {
              auto x_max = static_cast<double>(element->getAttribute("x_max"));
              element->setAttribute("_x_max_set_by_user", x_max);
            }
          else if (strEqualsAny(attr, "y_dim") &&
                   strEqualsAny(element->localName(), "cell_array", "nonuniform_cell_array"))
            {
              auto y_dim = static_cast<int>(element->getAttribute("y_dim"));
              element->setAttribute("_y_dim_set_by_user", y_dim);
            }
          else if (strEqualsAny(attr, "y_min") &&
                   strEqualsAny(element->localName(), "cell_array", "nonuniform_cell_array"))
            {
              auto y_min = static_cast<double>(element->getAttribute("y_min"));
              element->setAttribute("_y_min_set_by_user", y_min);
            }
          else if (strEqualsAny(attr, "y_max") &&
                   strEqualsAny(element->localName(), "cell_array", "nonuniform_cell_array"))
            {
              auto y_max = static_cast<double>(element->getAttribute("y_max"));
              element->setAttribute("_y_max_set_by_user", y_max);
            }
          else if (strEqualsAny(attr, "theta_org") &&
                   strEqualsAny(element->localName(), "polar_cell_array", "nonuniform_polar_cell_array"))
            {
              auto theta_org = static_cast<double>(element->getAttribute("theta_org"));
              element->setAttribute("_theta_org_set_by_user", theta_org);
            }
          else if (strEqualsAny(attr, "r_org") &&
                   strEqualsAny(element->localName(), "polar_cell_array", "nonuniform_polar_cell_array"))
            {
              auto r_org = static_cast<double>(element->getAttribute("r_org"));
              element->setAttribute("_r_org_set_by_user", r_org);
            }
          else if (strEqualsAny(attr, "num_col") &&
                   strEqualsAny(element->localName(), "polar_cell_array", "nonuniform_polar_cell_array"))
            {
              auto num_col = static_cast<int>(element->getAttribute("num_col"));
              element->setAttribute("_num_col_set_by_user", num_col);
            }
          else if (strEqualsAny(attr, "num_row") &&
                   strEqualsAny(element->localName(), "polar_cell_array", "nonuniform_polar_cell_array"))
            {
              auto num_row = static_cast<int>(element->getAttribute("num_row"));
              element->setAttribute("_num_row_set_by_user", num_row);
            }
          else if (strEqualsAny(attr, "r_dim") &&
                   strEqualsAny(element->localName(), "polar_cell_array", "nonuniform_polar_cell_array"))
            {
              auto r_dim = static_cast<int>(element->getAttribute("r_dim"));
              element->setAttribute("_r_dim_set_by_user", r_dim);
            }
          else if (strEqualsAny(attr, "r_min") &&
                   strEqualsAny(element->localName(), "polar_cell_array", "nonuniform_polar_cell_array"))
            {
              auto r_min = static_cast<double>(element->getAttribute("r_min"));
              element->setAttribute("_r_min_set_by_user", r_min);
            }
          else if (strEqualsAny(attr, "r_max") &&
                   strEqualsAny(element->localName(), "polar_cell_array", "nonuniform_polar_cell_array"))
            {
              auto r_max = static_cast<double>(element->getAttribute("r_max"));
              element->setAttribute("_r_max_set_by_user", r_max);
            }
          else if (strEqualsAny(attr, "theta_dim") &&
                   strEqualsAny(element->localName(), "polar_cell_array", "nonuniform_polar_cell_array"))
            {
              auto theta_dim = static_cast<int>(element->getAttribute("theta_dim"));
              element->setAttribute("_theta_dim_set_by_user", theta_dim);
            }
          else if (strEqualsAny(attr, "theta_min") &&
                   strEqualsAny(element->localName(), "polar_cell_array", "nonuniform_polar_cell_array"))
            {
              auto theta_min = static_cast<double>(element->getAttribute("theta_min"));
              element->setAttribute("_theta_min_set_by_user", theta_min);
            }
          else if (strEqualsAny(attr, "theta_max") &&
                   strEqualsAny(element->localName(), "polar_cell_array", "nonuniform_polar_cell_array"))
            {
              auto theta_max = static_cast<double>(element->getAttribute("theta_max"));
              element->setAttribute("_theta_max_set_by_user", theta_max);
            }
          else if (strEqualsAny(attr, "start_col") &&
                   strEqualsAny(element->localName(), "polar_cell_array", "nonuniform_polar_cell_array"))
            {
              auto start_col = static_cast<int>(element->getAttribute("start_col"));
              element->setAttribute("_start_col_set_by_user", start_col);
            }
          else if (strEqualsAny(attr, "start_row") &&
                   strEqualsAny(element->localName(), "polar_cell_array", "nonuniform_polar_cell_array"))
            {
              auto start_row = static_cast<int>(element->getAttribute("start_row"));
              element->setAttribute("_start_row_set_by_user", start_row);
            }
          else if (strEqualsAny(attr, "line_color_ind"))
            {
              auto line_color_ind = static_cast<int>(element->getAttribute("line_color_ind"));
              element->setAttribute("_line_color_ind_set_by_user", line_color_ind);
            }
          else if (strEqualsAny(attr, "line_width"))
            {
              auto line_width = static_cast<double>(element->getAttribute("line_width"));
              element->setAttribute("_line_width_set_by_user", line_width);
            }
          else if (strEqualsAny(attr, "line_color_rgb"))
            {
              auto line_color_rgb = static_cast<std::string>(element->getAttribute("line_color_rgb"));
              element->setAttribute("_line_color_rgb_set_by_user", line_color_rgb);
            }
          else if (strEqualsAny(attr, "line_spec"))
            {
              auto line_spec = static_cast<std::string>(element->getAttribute("line_spec"));
              element->setAttribute("_line_spec_set_by_user", line_spec);
            }
          else if (strEqualsAny(attr, "line_type"))
            {
              auto line_type = static_cast<int>(element->getAttribute("line_type"));
              element->setAttribute("_line_type_set_by_user", line_type);
            }
          else if (strEqualsAny(attr, "fill_color_ind"))
            {
              auto fill_color_ind = static_cast<int>(element->getAttribute("fill_color_ind"));
              element->setAttribute("_fill_color_ind_set_by_user", fill_color_ind);
            }
          else if (strEqualsAny(attr, "fill_color_rgb"))
            {
              auto fill_color_rgb = static_cast<std::string>(element->getAttribute("fill_color_rgb"));
              element->setAttribute("_fill_color_rgb_set_by_user", fill_color_rgb);
            }
          else if (strEqualsAny(attr, "fill_int_style"))
            {
              auto fill_int_style = static_cast<int>(element->getAttribute("fill_int_style"));
              element->setAttribute("_fill_int_style_set_by_user", fill_int_style);
            }
          else if (strEqualsAny(attr, "fill_style"))
            {
              auto fill_style = static_cast<int>(element->getAttribute("fill_style"));
              element->setAttribute("_fill_style_set_by_user", fill_style);
            }
          else if (strEqualsAny(attr, "font"))
            {
              auto font = static_cast<int>(element->getAttribute("font"));
              element->setAttribute("_font_set_by_user", font);
            }
          else if (strEqualsAny(attr, "font_precision"))
            {
              auto font_precision = static_cast<int>(element->getAttribute("font_precision"));
              element->setAttribute("_font_precision_set_by_user", font_precision);
            }
          else if (strEqualsAny(attr, "text_align_vertical"))
            {
              auto text_align_vertical = static_cast<int>(element->getAttribute("text_align_vertical"));
              element->setAttribute("_text_align_vertical_set_by_user", text_align_vertical);
            }
          else if (strEqualsAny(attr, "text_align_horizontal"))
            {
              auto text_align_horizontal = static_cast<int>(element->getAttribute("text_align_horizontal"));
              element->setAttribute("_text_align_horizontal_set_by_user", text_align_horizontal);
            }
          else if (strEqualsAny(attr, "text_color_ind"))
            {
              auto text_color_ind = static_cast<int>(element->getAttribute("text_color_ind"));
              element->setAttribute("_text_color_ind_set_by_user", text_color_ind);
            }
          else if (strEqualsAny(attr, "marker_color_ind"))
            {
              auto marker_color_ind = static_cast<int>(element->getAttribute("marker_color_ind"));
              element->setAttribute("_marker_color_ind_set_by_user", marker_color_ind);
            }
          else if (strEqualsAny(attr, "marker_color_indices"))
            {
              auto marker_color_indices = static_cast<std::string>(element->getAttribute("marker_color_indices"));
              element->setAttribute("_marker_color_indices_set_by_user", marker_color_indices);
            }
          else if (strEqualsAny(attr, "marker_size"))
            {
              auto marker_size = static_cast<double>(element->getAttribute("marker_size"));
              element->setAttribute("_marker_size_set_by_user", marker_size);
            }
          else if (strEqualsAny(attr, "marker_sizes"))
            {
              auto marker_sizes = static_cast<std::string>(element->getAttribute("marker_sizes"));
              element->setAttribute("_marker_sizes_set_by_user", marker_sizes);
            }
          else if (strEqualsAny(attr, "marker_type"))
            {
              auto marker_type = static_cast<int>(element->getAttribute("marker_type"));
              element->setAttribute("_marker_type_set_by_user", marker_type);
            }
          else if (strEqualsAny(attr, "border_color_ind"))
            {
              auto border_color_ind = static_cast<int>(element->getAttribute("border_color_ind"));
              element->setAttribute("_border_color_ind_set_by_user", border_color_ind);
            }
          else if (strEqualsAny(attr, "border_width"))
            {
              auto border_width = static_cast<double>(element->getAttribute("border_width"));
              element->setAttribute("_border_width_set_by_user", border_width);
            }

          if (attr == "viewport_x_min")
            {
              double old_shift = (element->hasAttribute("_x_min_shift"))
                                     ? static_cast<double>(element->getAttribute("_x_min_shift"))
                                     : 0;
              element->setAttribute("_x_min_shift",
                                    old_shift + static_cast<double>(element->getAttribute(attr)) - std::stod(value));
              element->setAttribute("_update_required", true);
            }
          else if (attr == "viewport_x_max")
            {
              double old_shift = (element->hasAttribute("_x_max_shift"))
                                     ? static_cast<double>(element->getAttribute("_x_max_shift"))
                                     : 0;
              element->setAttribute("_x_max_shift",
                                    old_shift + static_cast<double>(element->getAttribute(attr)) - std::stod(value));
              element->setAttribute("_update_required", true);
            }
          else if (attr == "viewport_y_min")
            {
              double old_shift = (element->hasAttribute("_y_min_shift"))
                                     ? static_cast<double>(element->getAttribute("_y_min_shift"))
                                     : 0;
              element->setAttribute("_y_min_shift",
                                    old_shift + static_cast<double>(element->getAttribute(attr)) - std::stod(value));
              element->setAttribute("_update_required", true);
            }
          else if (attr == "viewport_y_max")
            {
              double old_shift = (element->hasAttribute("_y_max_shift"))
                                     ? static_cast<double>(element->getAttribute("_y_max_shift"))
                                     : 0;
              element->setAttribute("_y_max_shift",
                                    old_shift + static_cast<double>(element->getAttribute(attr)) - std::stod(value));
              element->setAttribute("_update_required", true);
            }
          else if (attr == "labels" && element->localName() == "series_pie")
            {
              auto plot_parent = element;
              getPlotParent(plot_parent);

              if (auto legend = plot_parent->querySelectors("legend"); legend != nullptr)
                {
                  legend->setAttribute("_update_required", true);
                  legend->setAttribute("_delete_children", static_cast<int>(DelValues::RECREATE_OWN_CHILDREN));
                  legend->removeAttribute("_start_w");
                  legend->removeAttribute("_start_h");
                }
              else
                {
                  global_creator->createLegend();
                }
            }
          else if (attr == "label")
            {
              auto plot_parent = element;
              getPlotParent(plot_parent);

              if (auto legend = plot_parent->querySelectors("legend"); legend != nullptr)
                {
                  legend->setAttribute("_update_required", true);
                  legend->setAttribute("_delete_children", static_cast<int>(DelValues::RECREATE_OWN_CHILDREN));
                  legend->removeAttribute("_start_w");
                  legend->removeAttribute("_start_h");
                }
              else
                {
                  legend = global_creator->createLegend();
                  plot_parent->append(legend);
                }
            }
          else if (attr == "text_angle")
            {
              auto val = static_cast<int>(element->getAttribute("text_angle"));
              element->setAttribute("char_up_x", std::cos(val * M_PI / 180.0));
              element->setAttribute("char_up_y", std::sin(val * M_PI / 180.0));
              element->removeAttribute("text_angle");
            }

          auto plot_parent = element;
          getPlotParent(plot_parent);
          if (plot_parent != nullptr && plot_parent->parentElement() != nullptr &&
              plot_parent->parentElement()->localName() != "figure")
            {
              plot_parent->setAttribute("_active_through_update", true);
            }
          else if (plot_parent == nullptr)
            {
              if (active_figure != nullptr)
                {
                  GRM::Render::setAutoUpdate(false);
                  for (const auto &plot : active_figure->querySelectorsAll("plot[_active=\"1\"]"))
                    {
                      plot->removeAttribute("_active");
                    }
                  GRM::Render::setAutoUpdate(true);
                }
            }
        }
      global_root->setAttribute("_modified", true);

      global_render->setFirstCall(true);
      GRM::Render::setAutoUpdate(true);
    }
}
