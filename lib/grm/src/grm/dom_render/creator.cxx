#include "grm/plot.h"
#ifdef _WIN32
/*
 * Headers on Windows can define `min` and `max` as macros which causes
 * problem when using `std::min` and `std::max`
 * -> Define `NOMINMAX` to prevent the definition of these macros
 */
#define NOMINMAX
#endif

#include <cm.h>

#include <grm/dom_render/creator.hxx>
#include <grm/dom_render/context.hxx>
#include <grm/dom_render/render.hxx>
#include <grm/logging_int.h>
#include <grm/util_int.h>


std::shared_ptr<GRM::Creator> global_creator;
static int axis_id = 0;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ create functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

std::shared_ptr<GRM::Creator> GRM::Creator::createCreator(std::shared_ptr<Context> context)
{
  /*!
   * This function can be used to create a Creator object
   */
  global_creator = std::shared_ptr<Creator>(new Creator(context));
  return global_creator;
}

std::shared_ptr<GRM::Element> GRM::Creator::createPlot(int plot_id, const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> plot = (ext_element == nullptr) ? createElement("plot") : ext_element;

  plot->setAttribute("_plot_id", "plot" + std::to_string(plot_id));
  plot->setAttribute("plot_group", true);

  return plot;
}

std::shared_ptr<GRM::Element> GRM::Creator::createCentralRegion(const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> central_region =
      (ext_element == nullptr) ? createElement("central_region") : ext_element;

  return central_region;
}

std::shared_ptr<GRM::Element>
GRM::Creator::createPolymarker(const std::string &x_key, std::optional<std::vector<double>> x, const std::string &y_key,
                               std::optional<std::vector<double>> y, const std::shared_ptr<GRM::Context> &ext_context,
                               int marker_type, double marker_size, int marker_colorind,
                               const std::shared_ptr<GRM::Element> &ext_element)
{
  /*!
   * This function can be used to create a Polymarker GRM::Element
   *
   * \param[in] x_key A string used for storing the x coordinates in GRM::Context
   * \param[in] x A vector containing double values representing x coordinates
   * \param[in] y_key A string used for storing the y coordinates in GRM::Context
   * \param[in] y A vector containing double values representing y coordinates
   * \param[in] ext_context A GRM::Context that is used for storing the vectors. By default it uses GRM::Creator's
   * GRM::Context object but an external GRM::Context can be used
   * \param[in] marker_type An Integer setting the gr_markertype. By default it is 0
   * \param[in] marker_size A Double value setting the gr_markersize. By default it is 0.0
   * \param[in] marker_colorind An Integer setting the gr_markercolorind. By default it is 0
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("polymarker") : ext_element;

  if (x != std::nullopt) (*use_context)[x_key] = x.value();
  element->setAttribute("x", x_key);
  if (y != std::nullopt) (*use_context)[y_key] = y.value();
  element->setAttribute("y", y_key);

  if (marker_type != 0) element->setAttribute("marker_type", marker_type);
  if (marker_size != 0.0) element->setAttribute("marker_size", marker_size);
  if (marker_colorind != 0) element->setAttribute("marker_color_ind", marker_colorind);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createPolymarker(double x, double y, int marker_type, double marker_size,
                                                             int marker_colorind,
                                                             const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("polymarker") : ext_element;

  element->setAttribute("x", x);
  element->setAttribute("y", y);
  if (marker_type != 0) element->setAttribute("marker_type", marker_type);
  if (marker_size != 0.0) element->setAttribute("marker_size", marker_size);
  if (marker_colorind != 0) element->setAttribute("marker_color_ind", marker_colorind);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createPolyline(double x1, double x2, double y1, double y2, int line_type,
                                                           double line_width, int line_colorind,
                                                           const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("polyline") : ext_element;

  element->setAttribute("x1", x1);
  element->setAttribute("x2", x2);
  element->setAttribute("y1", y1);
  element->setAttribute("y2", y2);
  if (line_type != 0) element->setAttribute("line_type", line_type);
  if (line_width != 0.0) element->setAttribute("line_width", line_width);
  if (line_colorind != 0) element->setAttribute("line_color_ind", line_colorind);

  return element;
}

std::shared_ptr<GRM::Element>
GRM::Creator::createPolyline(const std::string &x_key, std::optional<std::vector<double>> x, const std::string &y_key,
                             std::optional<std::vector<double>> y, const std::shared_ptr<GRM::Context> &ext_context,
                             int line_type, double line_width, int line_colorind,
                             const std::shared_ptr<GRM::Element> &ext_element)
{
  /*!
   * This function can be used to create a Polyline GRM::Element
   *
   * \param[in] x_key A string used for storing the x coordinates in GRM::Context
   * \param[in] x A vector containing double values representing x coordinates
   * \param[in] y_key A string used for storing the y coordinates in GRM::Context
   * \param[in] y A vector containing double values representing y coordinates
   * \param[in] ext_context A GRM::Context that is used for storing the vectors. By default it uses GRM::Creator's
   * GRM::Context object but an external GRM::Context can be used
   * \param[in] line_type An Integer setting the gr_linetype. By default it is 0
   * \param[in] line_width A Double value setting the gr_linewidth. By default it is 0.0
   * \param[in] marker_colorind An Integer setting the gr_linecolorind. By default it is 0
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("polyline") : ext_element;

  if (x != std::nullopt) (*use_context)[x_key] = *x;
  element->setAttribute("x", x_key);
  if (y != std::nullopt) (*use_context)[y_key] = *y;
  element->setAttribute("y", y_key);

  if (line_type != 0) element->setAttribute("line_type", line_type);
  if (line_width != 0.0) element->setAttribute("line_width", line_width);
  if (line_colorind != 0) element->setAttribute("line_color_ind", line_colorind);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createText(double x, double y, const std::string &text,
                                                       CoordinateSpace world_coordinates,
                                                       const std::shared_ptr<GRM::Element> &ext_element)
{
  /*!
   * This function can be used to create a Text GRM::Element
   *
   * \param[in] x A double value representing the x coordinate
   * \param[in] y A double value representing the y coordinate
   * \param[in] text A string
   * \param[in] world_coordinates the coordinate space (WC or NDC) for x and y, default NDC
   */
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("text") : ext_element;

  element->setAttribute("x", x);
  element->setAttribute("y", y);
  element->setAttribute("text", text);
  element->setAttribute("world_coordinates", static_cast<int>(world_coordinates));

  return element;
}

std::shared_ptr<GRM::Element>
GRM::Creator::createFillArea(const std::string &x_key, std::optional<std::vector<double>> x, const std::string &y_key,
                             std::optional<std::vector<double>> y, const std::shared_ptr<GRM::Context> &ext_context,
                             int fill_int_style, int fill_style, int fill_color_ind,
                             const std::shared_ptr<GRM::Element> &ext_element)
{
  /*!
   * This function can be used to create a FillArea GRM::Element
   *
   * \param[in] n The number of data points
   * \param[in] x_key A string used for storing the x coordinates in GRM::Context
   * \param[in] x A vector containing double values representing x coordinates
   * \param[in] y_key A string used for storing the y coordinates in GRM::Context
   * \param[in] y A vector containing double values representing y coordinates
   * \param[in] ext_context A GRM::Context that is used for storing the vectors. By default it uses GRM::Creator's
   * GRM::Context object but an external GRM::Context can be used
   * \param[in] fillintstyle An Integer setting the gr_fillintstyle. By default it is 0
   * \param[in] fillstyle An Integer setting the gr_fillstyle. By default it is 0
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("fill_area") : ext_element;

  if (x != std::nullopt) (*use_context)[x_key] = *x;
  element->setAttribute("x", x_key);
  if (y != std::nullopt) (*use_context)[y_key] = *y;
  element->setAttribute("y", y_key);

  if (fill_int_style != 0) element->setAttribute("fill_int_style", fill_int_style);
  if (fill_style != 0) element->setAttribute("fill_style", fill_style);
  if (fill_color_ind != -1) element->setAttribute("fill_color_ind", fill_color_ind);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createCellArray(double xmin, double xmax, double ymin, double ymax,
                                                            int dimx, int dimy, int scol, int srow, int ncol, int nrow,
                                                            const std::string &color_key,
                                                            std::optional<std::vector<int>> color,
                                                            const std::shared_ptr<GRM::Context> &ext_context,
                                                            const std::shared_ptr<GRM::Element> &ext_element)
{
  /*!
   * This function can be used to create a cell_array GRM::Element
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
   * \param[in] ext_context A GRM::Context used for storing color. By default it uses GRM::Creator's GRM::Context object
   * but an external GRM::Context can be used
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("cell_array") : ext_element;

  element->setAttribute("x_min", xmin);
  element->setAttribute("x_max", xmax);
  element->setAttribute("y_min", ymin);
  element->setAttribute("y_max", ymax);
  element->setAttribute("x_dim", dimx);
  element->setAttribute("y_dim", dimy);
  element->setAttribute("start_col", scol);
  element->setAttribute("start_row", srow);
  element->setAttribute("num_col", ncol);
  element->setAttribute("num_row", nrow);
  element->setAttribute("color_ind_values", color_key);
  if (color != std::nullopt) (*use_context)[color_key] = *color;

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createEmptyAxis(const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("axis") : ext_element;

  if (!element->hasAttribute("_axis_id")) element->setAttribute("_axis_id", axis_id++);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createAxis(double min_val, double max_val, double tick, double org,
                                                       double pos, int major_count, int num_ticks, int num_tick_labels,
                                                       double tick_size, int tick_orientation, double label_pos,
                                                       const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("axis") : ext_element;

  element->setAttribute("min_value", min_val);
  element->setAttribute("max_value", max_val);
  element->setAttribute("tick", tick);
  element->setAttribute("origin", org);
  element->setAttribute("pos", pos);
  element->setAttribute("major_count", major_count);
  element->setAttribute("num_ticks", num_ticks);
  element->setAttribute("num_tick_labels", num_tick_labels);
  element->setAttribute("tick_size", tick_size);
  element->setAttribute("tick_orientation", tick_orientation);
  if (!element->hasAttribute("_label_pos_set_by_user")) element->setAttribute("label_pos", label_pos);
  if (!element->hasAttribute("_axis_id")) element->setAttribute("_axis_id", axis_id++);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createTickGroup(int is_major, const std::string &tick_label, double value,
                                                            double width,
                                                            const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("tick_group") : ext_element;

  element->setAttribute("is_major", is_major);
  element->setAttribute("tick_label", tick_label);
  element->setAttribute("value", value);
  element->setAttribute("width", width);
  element->setAttribute("z_index", -8);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createTick(int is_major, double value,
                                                       const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("tick") : ext_element;

  element->setAttribute("is_major", is_major);
  element->setAttribute("value", value);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createGridLine(int is_major, double value,
                                                           const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("grid_line") : ext_element;

  element->setAttribute("is_major", is_major);
  element->setAttribute("value", value);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createLegend(const std::shared_ptr<GRM::Element> &ext_element,
                                                         const std::shared_ptr<GRM::Context> &ext_context)
{
  /*!
   * This function can be used for creating a legend GRM::Element
   * This element is different compared to most of Creator's GRM::Element, the legend GRM::Element will incorporate
   * plot_draw_legend code from plot.cxx and will create new GRM::Elements as child nodes in the Creator document
   */
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("legend") : ext_element;
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;

  element->setAttribute("z_index", 4);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createPieSegment(const double start_angle, const double end_angle,
                                                             const std::string &text, const int color_index,
                                                             const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("pie_segment") : ext_element;

  element->setAttribute("start_angle", start_angle);
  element->setAttribute("end_angle", end_angle);
  element->setAttribute("text", text);
  element->setAttribute("fill_color_ind", color_index);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createBar(const double x1, const double x2, const double y1,
                                                      const double y2, const int bar_color_index,
                                                      const int edge_color_index, const std::string &bar_color_rgb,
                                                      const std::string &edge_color_rgb, const double linewidth,
                                                      const std::string &text,
                                                      const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("bar") : ext_element;

  element->setAttribute("x1", x1);
  element->setAttribute("x2", x2);
  element->setAttribute("y1", y1);
  element->setAttribute("y2", y2);
  element->setAttribute("line_color_ind", edge_color_index);
  element->setAttribute("fill_color_ind", bar_color_index);
  if (!bar_color_rgb.empty()) element->setAttribute("fill_color_rgb", bar_color_rgb);
  if (!edge_color_rgb.empty()) element->setAttribute("line_color_rgb", edge_color_rgb);
  if (linewidth != -1) element->setAttribute("line_width", linewidth);
  if (!text.empty()) element->setAttribute("text", text);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createSeries(const std::string &name)
{
  auto element = createElement("series_" + name);

  element->setAttribute("kind", name);
  element->setAttribute("_update_required", false);
  element->setAttribute("_delete_children", 0);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createDrawImage(double xmin, double ymin, double xmax, double ymax,
                                                            int width, int height, const std::string &data_key,
                                                            std::optional<std::vector<int>> data, int model,
                                                            const std::shared_ptr<GRM::Context> &ext_context,
                                                            const std::shared_ptr<GRM::Element> &ext_element)
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
   * \param[in] ext_context A GRM::Context used for storing data. By default it uses GRM::Creator's GRM::Context object
   * but an external GRM::Context can be used
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("draw_image") : ext_element;

  element->setAttribute("x_min", xmin);
  element->setAttribute("x_max", xmax);
  element->setAttribute("y_min", ymin);
  element->setAttribute("y_max", ymax);
  element->setAttribute("width", width);
  element->setAttribute("height", height);
  element->setAttribute("color_model", model);
  element->setAttribute("data", data_key);
  if (data != std::nullopt) (*use_context)[data_key] = *data;

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createDrawArc(double xmin, double xmax, double ymin, double ymax,
                                                          double start_angle, double end_angle,
                                                          const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("draw_arc") : ext_element;

  element->setAttribute("x_min", xmin);
  element->setAttribute("x_max", xmax);
  element->setAttribute("y_min", ymin);
  element->setAttribute("y_max", ymax);
  element->setAttribute("start_angle", start_angle);
  element->setAttribute("end_angle", end_angle);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createFillArc(double xmin, double xmax, double ymin, double ymax, double a1,
                                                          double a2, int fill_int_style, int fill_style,
                                                          int fill_color_ind,
                                                          const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("fill_arc") : ext_element;

  element->setAttribute("x_min", xmin);
  element->setAttribute("x_max", xmax);
  element->setAttribute("y_min", ymin);
  element->setAttribute("y_max", ymax);
  element->setAttribute("start_angle", a1);
  element->setAttribute("end_angle", a2);

  if (fill_int_style != 0) element->setAttribute("fill_int_style", fill_int_style);
  if (fill_style != 0) element->setAttribute("fill_style", fill_style);
  if (fill_color_ind != -1) element->setAttribute("fill_color_ind", fill_color_ind);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createDrawRect(double xmin, double xmax, double ymin, double ymax,
                                                           const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("draw_rect") : ext_element;

  element->setAttribute("x_min", xmin);
  element->setAttribute("x_max", xmax);
  element->setAttribute("y_min", ymin);
  element->setAttribute("y_max", ymax);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createFillRect(double xmin, double xmax, double ymin, double ymax,
                                                           int fill_int_style, int fill_style, int fill_color_ind,
                                                           const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("fill_rect") : ext_element;

  element->setAttribute("x_min", xmin);
  element->setAttribute("x_max", xmax);
  element->setAttribute("y_min", ymin);
  element->setAttribute("y_max", ymax);

  if (fill_int_style != 0) element->setAttribute("fill_int_style", fill_int_style);
  if (fill_style != 0) element->setAttribute("fill_style", fill_style);
  if (fill_color_ind != -1) element->setAttribute("fill_color_ind", fill_color_ind);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createQuiver(const std::string &x_key, std::optional<std::vector<double>> x,
                                                         const std::string &y_key, std::optional<std::vector<double>> y,
                                                         const std::string &u_key, std::optional<std::vector<double>> u,
                                                         const std::string &v_key, std::optional<std::vector<double>> v,
                                                         int colored, const std::shared_ptr<GRM::Context> &ext_context)
{
  /*
   * This function can be used to create a Quiver GRM::Element
   *
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  auto element = createSeries("quiver");

  element->setAttribute("x", x_key);
  element->setAttribute("y", y_key);
  element->setAttribute("u", u_key);
  element->setAttribute("v", v_key);
  element->setAttribute("colored", colored);

  if (x != std::nullopt) (*use_context)[x_key] = *x;
  if (y != std::nullopt) (*use_context)[y_key] = *y;
  if (u != std::nullopt) (*use_context)[u_key] = *u;
  if (v != std::nullopt) (*use_context)[v_key] = *v;

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createHexbin(const std::string &x_key, std::optional<std::vector<double>> x,
                                                         const std::string &y_key, std::optional<std::vector<double>> y,
                                                         const std::shared_ptr<GRM::Context> &ext_context)
{
  /*!
   * This function can be used to create a hexbin GRM::Element
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  auto element = createSeries("hexbin");

  element->setAttribute("x", x_key);
  element->setAttribute("y", y_key);

  if (x != std::nullopt) (*use_context)[x_key] = *x;
  if (y != std::nullopt) (*use_context)[y_key] = *y;

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createColorbar(unsigned int num_color_values,
                                                           const std::shared_ptr<GRM::Context> &ext_context,
                                                           const std::shared_ptr<GRM::Element> &ext_element)
{
  /*!
   * This function can be used to create a colorbar GRM::Element
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("colorbar") : ext_element;

  element->setAttribute("num_color_values", static_cast<int>(num_color_values));
  element->setAttribute("_update_required", false);
  element->setAttribute("_delete_children", 0);
  if (!element->hasAttribute("x_flip")) element->setAttribute("x_flip", false);
  if (!element->hasAttribute("y_flip")) element->setAttribute("y_flip", false);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createPolarCellArray(double theta_org, double r_org, double theta_min,
                                                                 double theta_max, double r_min, double r_max,
                                                                 int dim_theta, int dim_r, int s_col, int s_row,
                                                                 int n_col, int n_row, const std::string &color_key,
                                                                 std::optional<std::vector<int>> color,
                                                                 const std::shared_ptr<Context> &ext_context,
                                                                 const std::shared_ptr<GRM::Element> &ext_element)
{
  /*!
   * Display a two dimensional color index array mapped to a disk using polar
   * coordinates.
   *
   * \param[in] x_org X coordinate of the disk center in world coordinates
   * \param[in] y_org Y coordinate of the disk center in world coordinates
   * \param[in] theta_min start angle of the disk sector in degrees
   * \param[in] theta_max end angle of the disk sector in degrees
   * \param[in] r_min inner radius of the punctured disk in world coordinates
   * \param[in] r_max outer radius of the disk in world coordinates
   * \param[in] dim_theta Theta (X) dimension of the color index array
   * \param[in] dim_r R (Y) dimension of the color index array
   * \param[in] s_col number of leading columns in the color index array
   * \param[in] s_row number of leading rows in the color index array
   * \param[in] n_col total number of columns in the color index array
   * \param[in] n_row total number of rows in the color index array
   * \param[in] color color index array
   *
   * The two dimensional color index array is mapped to the resulting image by
   * interpreting the X-axis of the array as the angle and the Y-axis as the radius.
   * The center point of the resulting disk is located at `x_org`, `y_org` and the
   * radius of the disk is `rmax`.
   *
   * To draw a contiguous array as a complete disk use:
   *
   *     gr_polarcellarray(x_org, y_org, 0, 360, 0, r_max, dim_theta, dim_r, 1, 1, dim_theta, dim_r, color)
   *
   * The additional parameters to the function can be used to further control the
   * mapping from polar to cartesian coordinates.
   *
   * If `rmin` is greater than 0 the input data is mapped to a punctured disk (or
   * annulus) with an inner radius of `rmin` and an outer radius `rmax`. If `rmin`
   * is greater than `rmax` the Y-axis of the array is reversed.
   *
   * The parameter `theta_min` and `theta_max` can be used to map the data to a sector
   * of the (punctured) disk starting at `theta_min` and ending at `theta_max`. If
   * `theta_min` is greater than `theta_max` the X-axis is reversed. The visible sector
   * is the one starting in mathematically positive direction (counterclockwise)
   * at the smaller angle and ending at the larger angle. An example of the four
   * possible options can be found below:
   *
   * \verbatim embed:rst:leading-asterisk
   *
   * +--------------+--------------+---------------------------------------------------+
   * |**theta_min** |**theta_max** |**Result**                                         |
   * +--------------+--------------+---------------------------------------------------+
   * |90            |270           |Left half visible, mapped counterclockwise         |
   * +--------------+--------------+---------------------------------------------------+
   * |270           |90            |Left half visible, mapped clockwise                |
   * +--------------+--------------+---------------------------------------------------+
   * |-90           |90            |Right half visible, mapped counterclockwise        |
   * +--------------+--------------+---------------------------------------------------+
   * |90            |-90           |Right half visible, mapped clockwise               |
   * +--------------+--------------+---------------------------------------------------+
   *
   * \endverbatim
   *
   * `s_col` and `s_row` can be used to specify a (1-based) starting column and row
   * in the `color` array. `n_col` and `n_row` specify the actual dimension of the
   * array in the memory whereof `dim_theta` and `dimr` values are mapped to the disk.
   *
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("polar_cell_array") : ext_element;

  element->setAttribute("theta_org", theta_org);
  element->setAttribute("r_org", r_org);
  element->setAttribute("theta_min", theta_min);
  element->setAttribute("theta_max", theta_max);
  element->setAttribute("r_min", r_min);
  element->setAttribute("r_max", r_max);
  element->setAttribute("theta_dim", dim_theta);
  element->setAttribute("r_dim", dim_r);
  element->setAttribute("start_col", s_col);
  element->setAttribute("start_row", s_row);
  element->setAttribute("num_col", n_col);
  element->setAttribute("num_row", n_row);
  element->setAttribute("color_ind_values", color_key);
  if (color != std::nullopt) (*use_context)[color_key] = *color;

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createNonUniformPolarCellArray(
    double theta_org, double r_org, const std::string &theta_key, std::optional<std::vector<double>> theta,
    const std::string &r_key, std::optional<std::vector<double>> r, int dim_theta, int dim_r, int s_col, int s_row,
    int n_col, int n_row, const std::string &color_key, std::optional<std::vector<int>> color,
    const std::shared_ptr<GRM::Context> &ext_context, const std::shared_ptr<GRM::Element> &ext_element)
{
  /*!
   * Display a two dimensional color index array mapped to a disk using polar
   * coordinates.
   *
   * \param[in] x_org X coordinate of the disk center in world coordinates
   * \param[in] y_org Y coordinate of the disk center in world coordinates
   * \param[in] theta_min start angle of the disk sector in degrees
   * \param[in] theta_max end angle of the disk sector in degrees
   * \param[in] r_min inner radius of the punctured disk in world coordinates
   * \param[in] r_max outer radius of the disk in world coordinates
   * \param[in] dim_theta theta (X) dimension of the color index array
   * \param[in] dim_r R (Y) dimension of the color index array
   * \param[in] s_col number of leading columns in the color index array
   * \param[in] s_row number of leading rows in the color index array
   * \param[in] n_col total number of columns in the color index array
   * \param[in] n_row total number of rows in the color index array
   * \param[in] color color index array
   *
   * The two dimensional color index array is mapped to the resulting image by
   * interpreting the X-axis of the array as the angle and the Y-axis as the radius.
   * The center point of the resulting disk is located at `x_org`, `y_org` and the
   * radius of the disk is `rmax`.
   *
   * To draw a contiguous array as a complete disk use:
   *
   *     gr_polarcellarray(x_org, y_org, 0, 360, 0, r_max, dim_theta, dim_r, 1, 1, dim_theta, dim_r, color)
   *
   * The additional parameters to the function can be used to further control the
   * mapping from polar to cartesian coordinates.
   *
   * If `r_min` is greater than 0 the input data is mapped to a punctured disk (or
   * annulus) with an inner radius of `r_min` and an outer radius `r_max`. If `rmin`
   * is greater than `rmax` the Y-axis of the array is reversed.
   *
   * The parameter `theta_min` and `theta_max` can be used to map the data to a sector
   * of the (punctured) disk starting at `theta_min` and ending at `theta_max`. If
   * `theta_min` is greater than `theta_max` the X-axis is reversed. The visible sector
   * is the one starting in mathematically positive direction (counterclockwise)
   * at the smaller angle and ending at the larger angle. An example of the four
   * possible options can be found below:
   *
   * \verbatim embed:rst:leading-asterisk
   *
   * +--------------+--------------+---------------------------------------------------+
   * |**theta_min** |**theta_max** |**Result**                                         |
   * +--------------+--------------+---------------------------------------------------+
   * |90            |270           |Left half visible, mapped counterclockwise         |
   * +--------------+--------------+---------------------------------------------------+
   * |270           |90            |Left half visible, mapped clockwise                |
   * +--------------+--------------+---------------------------------------------------+
   * |-90           |90            |Right half visible, mapped counterclockwise        |
   * +--------------+--------------+---------------------------------------------------+
   * |90            |-90           |Right half visible, mapped clockwise               |
   * +--------------+--------------+---------------------------------------------------+
   *
   * \endverbatim
   *
   * `s_col` and `s_row` can be used to specify a (1-based) starting column and row
   * in the `color` array. `n_col` and `n_row` specify the actual dimension of the
   * array in the memory whereof `dim_theta` and `dim_r` values are mapped to the disk.
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  std::shared_ptr<GRM::Element> element =
      (ext_element == nullptr) ? createElement("nonuniform_polar_cell_array") : ext_element;

  element->setAttribute("theta_org", theta_org);
  element->setAttribute("r_org", r_org);
  element->setAttribute("r", r_key);
  element->setAttribute("theta", theta_key);
  element->setAttribute("theta_dim", dim_theta);
  element->setAttribute("r_dim", dim_r);
  element->setAttribute("start_col", s_col);
  element->setAttribute("start_row", s_row);
  element->setAttribute("num_col", n_col);
  element->setAttribute("num_row", n_row);
  element->setAttribute("color_ind_values", color_key);

  if (color != std::nullopt) (*use_context)[color_key] = *color;
  if (theta != std::nullopt) (*use_context)[theta_key] = *theta;
  if (r != std::nullopt) (*use_context)[r_key] = *r;

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createNonUniformCellArray(
    const std::string &x_key, std::optional<std::vector<double>> x, const std::string &y_key,
    std::optional<std::vector<double>> y, int dimx, int dimy, int scol, int srow, int ncol, int nrow,
    const std::string &color_key, std::optional<std::vector<int>> color,
    const std::shared_ptr<GRM::Context> &ext_context, const std::shared_ptr<GRM::Element> &ext_element)
{
  /*!
   * This function can be used to create a nonuniform cell array GRM::Element
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  std::shared_ptr<GRM::Element> element =
      (ext_element == nullptr) ? createElement("nonuniform_cell_array") : ext_element;

  element->setAttribute("x", x_key);
  element->setAttribute("y", y_key);
  element->setAttribute("color_ind_values", color_key);
  element->setAttribute("x_dim", dimx);
  element->setAttribute("y_dim", dimy);
  element->setAttribute("start_col", scol);
  element->setAttribute("start_row", srow);
  element->setAttribute("num_col", ncol);
  element->setAttribute("num_row", nrow);

  if (x != std::nullopt) (*use_context)[x_key] = *x;
  if (y != std::nullopt) (*use_context)[y_key] = *y;
  if (color != std::nullopt) (*use_context)[color_key] = *color;

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createGrid3d(double x_tick, double y_tick, double z_tick, double x_org,
                                                         double y_org, double z_org, int x_major, int y_major,
                                                         int z_major, const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("grid_3d") : ext_element;

  element->setAttribute("x_tick", x_tick);
  element->setAttribute("y_tick", y_tick);
  element->setAttribute("z_tick", z_tick);
  element->setAttribute("x_origin", x_org);
  element->setAttribute("y_origin", y_org);
  element->setAttribute("z_origin", z_org);
  element->setAttribute("x_major", x_major);
  element->setAttribute("y_major", y_major);
  element->setAttribute("z_major", z_major);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createEmptyGrid3d(bool x_grid, bool y_grid, bool z_grid,
                                                              const std::shared_ptr<GRM::Element> &ext_element)
{
  auto element = (ext_element == nullptr) ? createElement("grid_3d") : ext_element;

  if (!x_grid) element->setAttribute("x_tick", false);
  if (!y_grid) element->setAttribute("y_tick", false);
  if (!z_grid) element->setAttribute("z_tick", false);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createAxes3d(double x_tick, double y_tick, double z_tick, double x_org,
                                                         double y_org, double z_org, int major_x, int major_y,
                                                         int major_z, int tick_orientation,
                                                         const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("axes_3d") : ext_element;

  element->setAttribute("x_tick", x_tick);
  element->setAttribute("y_tick", y_tick);
  element->setAttribute("z_tick", z_tick);
  element->setAttribute("x_origin", x_org);
  element->setAttribute("y_origin", y_org);
  element->setAttribute("z_origin", z_org);
  element->setAttribute("major_x", major_x);
  element->setAttribute("major_y", major_y);
  element->setAttribute("major_z", major_z);
  element->setAttribute("tick_orientation", tick_orientation);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createEmptyAxes3d(int tick_orientation,
                                                              const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("axes_3d") : ext_element;

  element->setAttribute("tick_orientation", tick_orientation);

  return element;
}

std::shared_ptr<GRM::Element>
GRM::Creator::createPolyline3d(const std::string &x_key, std::optional<std::vector<double>> x, const std::string &y_key,
                               std::optional<std::vector<double>> y, const std::string &z_key,
                               std::optional<std::vector<double>> z, const std::shared_ptr<GRM::Context> &ext_context,
                               const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("polyline_3d") : ext_element;

  element->setAttribute("x", x_key);
  element->setAttribute("y", y_key);
  element->setAttribute("z", z_key);

  if (x != std::nullopt) (*use_context)[x_key] = *x;
  if (y != std::nullopt) (*use_context)[y_key] = *y;
  if (z != std::nullopt) (*use_context)[z_key] = *z;

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createPolymarker3d(
    const std::string &x_key, std::optional<std::vector<double>> x, const std::string &y_key,
    std::optional<std::vector<double>> y, const std::string &z_key, std::optional<std::vector<double>> z,
    const std::shared_ptr<GRM::Context> &ext_context, const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("polymarker_3d") : ext_element;

  element->setAttribute("x", x_key);
  element->setAttribute("y", y_key);
  element->setAttribute("z", z_key);

  if (x != std::nullopt) (*use_context)[x_key] = *x;
  if (y != std::nullopt) (*use_context)[y_key] = *y;
  if (z != std::nullopt) (*use_context)[z_key] = *z;

  return element;
}

std::shared_ptr<GRM::Element>
GRM::Creator::createTriSurface(const std::string &px_key, std::optional<std::vector<double>> px,
                               const std::string &py_key, std::optional<std::vector<double>> py,
                               const std::string &pz_key, std::optional<std::vector<double>> pz,
                               const std::shared_ptr<GRM::Context> &ext_context)
{
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  auto element = createSeries("trisurface");

  element->setAttribute("x", px_key);
  element->setAttribute("y", py_key);
  element->setAttribute("z", pz_key);

  if (px != std::nullopt) (*use_context)[px_key] = *px;
  if (py != std::nullopt) (*use_context)[py_key] = *py;
  if (pz != std::nullopt) (*use_context)[pz_key] = *pz;

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createTitles3d(const std::string &xlabel, const std::string &ylabel,
                                                           const std::string &zlabel,
                                                           const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("titles_3d") : ext_element;
  element->setAttribute("x_label_3d", xlabel);
  element->setAttribute("y_label_3d", ylabel);
  element->setAttribute("z_label_3d", zlabel);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createDrawGraphics(const std::string &data_key,
                                                               std::optional<std::vector<int>> data,
                                                               const std::shared_ptr<GRM::Context> &ext_context,
                                                               const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("draw_graphics") : ext_element;

  element->setAttribute("data", data_key);
  if (data != std::nullopt) (*use_context)[data_key] = *data;

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createLayoutGrid(const GRM::Grid &grid)
{
  auto element = createElement("layout_grid");

  if (grid.abs_height != -1) element->setAttribute("viewport_height_abs", grid.abs_height);
  if (grid.abs_width != -1) element->setAttribute("viewport_width_abs", grid.abs_width);
  if (grid.relative_height != -1) element->setAttribute("viewport_height_rel", grid.relative_height);
  if (grid.relative_width != -1) element->setAttribute("viewport_width_rel", grid.relative_width);
  if (grid.aspect_ratio != -1) element->setAttribute("aspect_ratio", grid.aspect_ratio);
  element->setAttribute("fit_parents_height", grid.fit_parents_height);
  element->setAttribute("fit_parents_width", grid.fit_parents_width);
  element->setAttribute("num_row", grid.getNRows());
  element->setAttribute("num_col", grid.getNCols());

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createLayoutGridElement(const GRM::GridElement &grid_element,
                                                                    const GRM::Slice &slice)
{
  auto element = createElement("layout_grid_element");

  if (grid_element.abs_height != -1) element->setAttribute("viewport_height_abs", grid_element.abs_height);
  if (grid_element.abs_width != -1) element->setAttribute("viewport_width_abs", grid_element.abs_width);
  element->setAttribute("fit_parents_height", grid_element.fit_parents_height);
  element->setAttribute("fit_parents_width", grid_element.fit_parents_width);
  if (grid_element.relative_height != -1) element->setAttribute("viewport_height_rel", grid_element.relative_height);
  if (grid_element.relative_width != -1) element->setAttribute("viewport_width_rel", grid_element.relative_width);
  if (grid_element.aspect_ratio != -1) element->setAttribute("aspect_ratio", grid_element.aspect_ratio);
  element->setAttribute("_start_row", slice.row_start);
  element->setAttribute("_stop_row", slice.row_stop);
  element->setAttribute("_start_col", slice.col_start);
  element->setAttribute("_stop_col", slice.col_stop);
  element->setAttribute("row_span", slice.row_stop - slice.row_start);
  element->setAttribute("col_span", slice.col_stop - slice.col_start);
  element->setAttribute("keep_size_if_swapped", true);
  element->setAttribute("position", std::to_string(slice.row_start) + " " + std::to_string(slice.col_start));

  double *plot = grid_element.plot;
  GRM::Render::setViewportNormalized(element, plot[0], plot[1], plot[2], plot[3]);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createPanzoom(double x, double y, double xzoom, double yzoom)
{
  auto element = createElement("panzoom");

  element->setAttribute("x", x);
  element->setAttribute("y", y);
  element->setAttribute("x_zoom", xzoom);
  element->setAttribute("y_zoom", yzoom);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createPolarBar(double count, int bin_nr,
                                                           const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("polar_bar") : ext_element;

  element->setAttribute("count", count);
  element->setAttribute("bin_nr", bin_nr);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createErrorBar(double error_bar_x, double error_bar_y_min,
                                                           double error_bar_y_max, int color_error_bar,
                                                           const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("error_bar") : ext_element;

  element->setAttribute("error_bar_x", error_bar_x);
  element->setAttribute("error_bar_y_min", error_bar_y_min);
  element->setAttribute("error_bar_y_max", error_bar_y_max);
  element->setAttribute("error_bar_color", color_error_bar);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createIntegral(double int_lim_low, double int_lim_high,
                                                           const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("integral") : ext_element;

  element->setAttribute("int_lim_low", int_lim_low);
  element->setAttribute("int_lim_high", int_lim_high);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createSideRegion(const std::string &location,
                                                             const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("side_region") : ext_element;

  element->setAttribute("location", location);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createTextRegion(const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("text_region") : ext_element;

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createSidePlotRegion(const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("side_plot_region") : ext_element;

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createRadialAxes(const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("radial_axes") : ext_element;

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createThetaAxes(const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("theta_axes") : ext_element;

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createAngleLine(double theta, double r, const std::string &angle_label,
                                                            const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("angle_line") : ext_element;

  element->setAttribute("angle_label", angle_label);
  element->setAttribute("theta", theta);
  element->setAttribute("r", r);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createArcGridLine(double value,
                                                              const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> element = (ext_element == nullptr) ? createElement("arc_grid_line") : ext_element;

  element->setAttribute("value", value);

  return element;
}

std::shared_ptr<GRM::Element> GRM::Creator::createOverlay(const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> overlay = (ext_element == nullptr) ? createElement("overlay") : ext_element;

  overlay->setAttribute("_viewport_normalized_x_min_org", 0.0);
  overlay->setAttribute("_viewport_normalized_x_max_org", 1.0);
  overlay->setAttribute("_viewport_normalized_y_min_org", 0.0);
  overlay->setAttribute("_viewport_normalized_y_max_org", 1.0);
  return overlay;
}

std::shared_ptr<GRM::Element> GRM::Creator::createOverlayElement(double x, double y, std::string type,
                                                                 const std::shared_ptr<GRM::Element> &ext_element)
{
  std::shared_ptr<GRM::Element> overlay_element =
      (ext_element == nullptr) ? createElement("overlay_element") : ext_element;

  overlay_element->setAttribute("x", x);
  overlay_element->setAttribute("y", y);
  overlay_element->setAttribute("element_type", type);

  return overlay_element;
}

/*
 * mixes gr color maps with size = size * size. If x and or y < 0
 */
void GRM::Creator::createColormap(int x, int y, int size, std::vector<int> &colormap)
{
  int r, g, b, a;
  int outer, inner;
  int r1, g1, b1;
  int r2, g2, b2;
  if (x > 47 || y > 47) logger((stderr, "values for the keyword \"colormap\" can not be greater than 47\n"));

  colormap.resize(size * size);
  if (x >= 0 && y < 0)
    {
      for (outer = 0; outer < size; outer++)
        {
          for (inner = 0; inner < size; inner++)
            {
              a = 255;
              r = ((cmap_h[x][static_cast<int>(inner * 255.0 / size)] >> 16) & 0xff);
              g = ((cmap_h[x][static_cast<int>(inner * 255.0 / size)] >> 8) & 0xff);
              b = (cmap_h[x][static_cast<int>(inner * 255.0 / size)] & 0xff);

              colormap[outer * size + inner] = (a << 24) + (b << 16) + (g << 8) + (r);
            }
        }
    }

  if (x < 0 && y >= 0)
    {
      gr_setcolormap(y);
      for (outer = 0; outer < size; outer++)
        {
          for (inner = 0; inner < size; inner++)
            {
              a = 255;
              r = ((cmap_h[y][static_cast<int>(inner * 255.0 / size)] >> 16) & 0xff);
              g = ((cmap_h[y][static_cast<int>(inner * 255.0 / size)] >> 8) & 0xff);
              b = (cmap_h[y][static_cast<int>(inner * 255.0 / size)] & 0xff);

              colormap[inner * size + outer] = (a << 24) + (b << 16) + (g << 8) + (r);
            }
        }
    }
  else if ((x >= 0 && y >= 0) || (x < 0 && y < 0))
    {
      if (x < 0 && y < 0) x = y = 0;
      gr_setcolormap(x);
      for (outer = 0; outer < size; outer++)
        {
          for (inner = 0; inner < size; inner++)
            {
              a = 255;
              r1 = ((cmap_h[x][static_cast<int>(inner * 255.0 / size)] >> 16) & 0xff);
              g1 = ((cmap_h[x][static_cast<int>(inner * 255.0 / size)] >> 8) & 0xff);
              b1 = (cmap_h[x][static_cast<int>(inner * 255.0 / size)] & 0xff);

              r2 = ((cmap_h[y][static_cast<int>(outer * 255.0 / size)] >> 16) & 0xff);
              g2 = ((cmap_h[y][static_cast<int>(outer * 255.0 / size)] >> 8) & 0xff);
              b2 = (cmap_h[y][static_cast<int>(outer * 255.0 / size)] & 0xff);

              colormap[outer * size + inner] =
                  (a << 24) + (((b1 + b2) / 2) << 16) + (((g1 + g2) / 2) << 8) + ((r1 + r2) / 2);
            }
        }
    }
}

GRM::Creator::Creator(std::shared_ptr<Context> context)
{
  /*!
   * This is the constructor for GRM::Creator
   */
  this->context = context;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ get functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int GRM::Creator::getAxisId()
{
  return axis_id;
}
