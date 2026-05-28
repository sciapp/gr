#include "grm/dom_render/dom_render_c.h"
#include "grm/dom_render/render.hxx"
#include "grm/dom_render/casts.hxx"
#include "grm/dom_render/context.hxx"
#include "grm/dom_render/creator.hxx"
#include "grm/dom_render/process_attributes.hxx"
#include "grm/dom_render/process_elements.hxx"
#include "grm/dom_render/render_util.hxx"
#include "grm/dom_render/updater.hxx"

int grm_algorithm_string_to_int(const char *algorithm_str)
{
  return GRM::algorithmStringToInt(algorithm_str);
}

int grm_colormap_string_to_int(const char *colormap_str)
{
  return GRM::colormapStringToInt(colormap_str);
}

int grm_font_string_to_int(const char *font_str)
{
  return GRM::fontStringToInt(font_str);
}

int grm_font_precision_string_to_int(const char *ont_precision_str)
{
  return GRM::fontStringToInt(ont_precision_str);
}

int grm_line_type_string_to_int(const char *line_type_str)
{
  return GRM::lineTypeStringToInt(line_type_str);
}

int grm_location_string_to_int(const char *location_str)
{
  return GRM::locationStringToInt(location_str);
}

int grm_x_axis_location_string_to_int(const char *location_str)
{
  return GRM::locationStringToInt(location_str);
}

int grm_y_axis_location_string_to_int(const char *location_str)
{
  return GRM::locationStringToInt(location_str);
}

int grm_marker_type_string_to_int(const char *marker_type_str)
{
  return GRM::markerTypeStringToInt(marker_type_str);
}

int grm_color_model_string_to_int(const char *color_model_str)
{
  return GRM::colorModelStringToInt(color_model_str);
}

int grm_scientific_format_string_to_int(const char *scientific_format_str)
{
  return GRM::scientificFormatStringToInt(scientific_format_str);
}

int grm_text_align_horizontal_string_to_int(const char *text_align_horizontal_str)
{
  return GRM::textAlignHorizontalStringToInt(text_align_horizontal_str);
}

int grm_text_align_vertical_string_to_int(const char *text_align_vertical_str)
{
  return GRM::textAlignVerticalStringToInt(text_align_vertical_str);
}

int grm_text_encoding_string_to_int(const char *text_encoding_str)
{
  return GRM::textEncodingStringToInt(text_encoding_str);
}

int grm_tick_orientation_string_to_int(const char *tick_orientation_str)
{
  return GRM::tickOrientationStringToInt(tick_orientation_str);
}

int grm_error_bar_style_string_to_int(const char *error_bar_stylr_str)
{
  return GRM::errorBarStyleStringToInt(error_bar_stylr_str);
}

int grm_clip_region_string_to_int(const char *error_bar_stylr_str)
{
  return GRM::clipRegionStringToInt(error_bar_stylr_str);
}

int grm_resample_method_string_to_int(const char *error_bar_stylr_str)
{
  return GRM::resampleMethodStringToInt(error_bar_stylr_str);
}

int grm_fill_style_string_to_int(const char *fill_style_str)
{
  return GRM::fillStyleStringToInt(fill_style_str);
}

int grm_fill_int_style_string_to_int(const char *fill_int_style_str)
{
  return GRM::fillIntStyleStringToInt(fill_int_style_str);
}

int grm_transformation_string_to_int(const char *transformation_str)
{
  return GRM::transformationStringToInt(transformation_str);
}

int grm_label_orientation_string_to_int(const char *label_orientation_str)
{
  return GRM::labelOrientationStringToInt(label_orientation_str);
}

int grm_world_coordinates_string_to_int(const char *world_coordinates_str)
{
  return GRM::worldCoordinatesStringToInt(world_coordinates_str);
}

/* ------------------------------- int to string ---------------------------------------------------------------------*/

const char *grm_algorithm_int_to_string(int algorithm)
{
  return strdup(GRM::algorithmIntToString(algorithm).c_str());
}

const char *grm_colormap_int_to_string(int colormap)
{
  return strdup(GRM::colormapIntToString(colormap).c_str());
}

const char *grm_font_int_to_string(int font)
{
  return strdup(GRM::fontIntToString(font).c_str());
}

const char *grm_font_precision_int_to_string(int font_precision)
{
  return strdup(GRM::fontPrecisionIntToString(font_precision).c_str());
}

const char *grm_line_type_int_to_string(int line_type)
{
  return strdup(GRM::lineTypeIntToString(line_type).c_str());
}

const char *grm_location_int_to_string(int location)
{
  return strdup(GRM::locationIntToString(location).c_str());
}

const char *grm_x_axis_location_int_to_string(int location)
{
  return strdup(GRM::locationIntToString(location).c_str());
}

const char *grm_y_axis_location_int_to_string(int location)
{
  return strdup(GRM::locationIntToString(location).c_str());
}

const char *grm_marker_type_int_to_string(int marker_type)
{
  return strdup(GRM::markerTypeIntToString(marker_type).c_str());
}

const char *grm_color_model_int_to_string(int color_model)
{
  return strdup(GRM::colorModelIntToString(color_model).c_str());
}

const char *grm_scientific_format_int_to_string(int scientific_format)
{
  return strdup(GRM::scientificFormatIntToString(scientific_format).c_str());
}

const char *grm_text_align_horizontal_int_to_string(int text_align_horizontal)
{
  return strdup(GRM::textAlignHorizontalIntToString(text_align_horizontal).c_str());
}

const char *grm_text_align_vertical_int_to_string(int text_align_vertical)
{
  return strdup(GRM::textAlignVerticalIntToString(text_align_vertical).c_str());
}

const char *grm_text_encoding_int_to_string(int text_encoding)
{
  return strdup(GRM::textEncodingIntToString(text_encoding).c_str());
}

const char *grm_tick_orientation_int_to_string(int tick_orientation)
{
  return strdup(GRM::tickOrientationIntToString(tick_orientation).c_str());
}

const char *grm_error_bar_style_int_to_string(int error_bar_style)
{
  return strdup(GRM::errorBarStyleIntToString(error_bar_style).c_str());
}

const char *grm_clip_region_int_to_string(int error_bar_style)
{
  return strdup(GRM::clipRegionIntToString(error_bar_style).c_str());
}

const char *grm_resample_method_int_to_string(int error_bar_style)
{
  return strdup(GRM::resampleMethodIntToString(error_bar_style).c_str());
}

const char *grm_fill_style_int_to_string(int fill_style)
{
  return strdup(GRM::fillStyleIntToString(fill_style).c_str());
}

const char *grm_fill_int_style_int_to_string(int fill_int_style)
{
  return strdup(GRM::fillIntStyleIntToString(fill_int_style).c_str());
}

const char *grm_transformation_int_to_string(int transformation)
{
  return strdup(GRM::transformationIntToString(transformation).c_str());
}

const char *grm_label_orientation_int_to_string(int label_orientation)
{
  return strdup(GRM::labelOrientationIntToString(label_orientation).c_str());
}

const char *grm_world_coordinates_int_to_string(int world_coordinates)
{
  return strdup(GRM::worldCoordinatesIntToString(world_coordinates).c_str());
}

/* ------------------------------- get functions ---------------------------------------------------------------------*/

void grm_get_size_units(const char **return_value)
{
  int i = 0;
  auto string_vec = GRM::getSizeUnits();
  return_value = static_cast<const char **>(realloc(return_value, string_vec.size() * sizeof(const char *)));
  while (return_value != nullptr)
    {
      *return_value = string_vec[i++].c_str();
      return_value++;
    }
}

void grm_get_colormaps(const char **return_value)
{
  int i = 0;
  auto string_vec = GRM::getColormaps();
  return_value = static_cast<const char **>(realloc(return_value, string_vec.size() * sizeof(const char *)));
  while (return_value != nullptr)
    {
      *return_value = string_vec[i++].c_str();
      return_value++;
    }
}

void grm_get_fonts(const char **return_value)
{
  int i = 0;
  auto string_vec = GRM::getFonts();
  return_value = static_cast<const char **>(realloc(return_value, string_vec.size() * sizeof(const char *)));
  while (return_value != nullptr)
    {
      *return_value = string_vec[i++].c_str();
      return_value++;
    }
}

void grm_get_font_precisions(const char **return_value)
{
  int i = 0;
  auto string_vec = GRM::getFontPrecisions();
  return_value = static_cast<const char **>(realloc(return_value, string_vec.size() * sizeof(const char *)));
  while (return_value != nullptr)
    {
      *return_value = string_vec[i++].c_str();
      return_value++;
    }
}

void grm_get_line_types(const char **return_value)
{
  int i = 0;
  auto string_vec = GRM::getLineTypes();
  return_value = static_cast<const char **>(realloc(return_value, string_vec.size() * sizeof(const char *)));
  while (return_value != nullptr)
    {
      *return_value = string_vec[i++].c_str();
      return_value++;
    }
}

void grm_get_locations(const char **return_value)
{
  int i = 0;
  auto string_vec = GRM::getLocations();
  return_value = static_cast<const char **>(realloc(return_value, string_vec.size() * sizeof(const char *)));
  while (return_value != nullptr)
    {
      *return_value = string_vec[i++].c_str();
      return_value++;
    }
}

void grm_get_x_axis_locations(const char **return_value)
{
  int i = 0;
  auto string_vec = GRM::getXAxisLocations();
  return_value = static_cast<const char **>(realloc(return_value, string_vec.size() * sizeof(const char *)));
  while (return_value != nullptr)
    {
      *return_value = string_vec[i++].c_str();
      return_value++;
    }
}

void grm_get_y_axis_locations(const char **return_value)
{
  int i = 0;
  auto string_vec = GRM::getYAxisLocations();
  return_value = static_cast<const char **>(realloc(return_value, string_vec.size() * sizeof(const char *)));
  while (return_value != nullptr)
    {
      *return_value = string_vec[i++].c_str();
      return_value++;
    }
}

void grm_get_marker_types(const char **return_value)
{
  int i = 0;
  auto string_vec = GRM::getMarkerTypes();
  return_value = static_cast<const char **>(realloc(return_value, string_vec.size() * sizeof(const char *)));
  while (return_value != nullptr)
    {
      *return_value = string_vec[i++].c_str();
      return_value++;
    }
}

void grm_get_text_align_horizontal(const char **return_value)
{
  int i = 0;
  auto string_vec = GRM::getTextAlignHorizontal();
  return_value = static_cast<const char **>(realloc(return_value, string_vec.size() * sizeof(const char *)));
  while (return_value != nullptr)
    {
      *return_value = string_vec[i++].c_str();
      return_value++;
    }
}

void grm_get_text_align_vertical(const char **return_value)
{
  int i = 0;
  auto string_vec = GRM::getTextAlignVertical();
  return_value = static_cast<const char **>(realloc(return_value, string_vec.size() * sizeof(const char *)));
  while (return_value != nullptr)
    {
      *return_value = string_vec[i++].c_str();
      return_value++;
    }
}

void grm_get_algorithm(const char **return_value)
{
  int i = 0;
  auto string_vec = GRM::getAlgorithm();
  return_value = static_cast<const char **>(realloc(return_value, string_vec.size() * sizeof(const char *)));
  while (return_value != nullptr)
    {
      *return_value = string_vec[i++].c_str();
      return_value++;
    }
}

void grm_get_color_model(const char **return_value)
{
  int i = 0;
  auto string_vec = GRM::getColorModel();
  return_value = static_cast<const char **>(realloc(return_value, string_vec.size() * sizeof(const char *)));
  while (return_value != nullptr)
    {
      *return_value = string_vec[i++].c_str();
      return_value++;
    }
}

void grm_get_context_attributes(const char **return_value)
{
  int i = 0;
  auto string_vec = GRM::getContextAttributes();
  return_value = static_cast<const char **>(realloc(return_value, string_vec.size() * sizeof(const char *)));
  while (return_value != nullptr)
    {
      *return_value = string_vec[i++].c_str();
      return_value++;
    }
}

void grm_get_fill_styles(const char **return_value)
{
  int i = 0;
  auto string_vec = GRM::getFillStyles();
  return_value = static_cast<const char **>(realloc(return_value, string_vec.size() * sizeof(const char *)));
  while (return_value != nullptr)
    {
      *return_value = string_vec[i++].c_str();
      return_value++;
    }
}

void grm_get_fill_int_styles(const char **return_value)
{
  int i = 0;
  auto string_vec = GRM::getFillIntStyles();
  return_value = static_cast<const char **>(realloc(return_value, string_vec.size() * sizeof(const char *)));
  while (return_value != nullptr)
    {
      *return_value = string_vec[i++].c_str();
      return_value++;
    }
}

void grm_get_transformation(const char **return_value)
{
  int i = 0;
  auto string_vec = GRM::getTransformation();
  return_value = static_cast<const char **>(realloc(return_value, string_vec.size() * sizeof(const char *)));
  while (return_value != nullptr)
    {
      *return_value = string_vec[i++].c_str();
      return_value++;
    }
}

/* =============================== context ========================================================================== */

void grm_new_inner_context(grm_context_t *context, const char *key, grm_inner_context_t **a_inner_context)
{
  *a_inner_context =
      reinterpret_cast<grm_inner_context_t *>(new GRM::Context::Inner(*reinterpret_cast<GRM::Context *>(context), key));
}

void grm_new_inner_context_const(const grm_context_t *context, const char *key, grm_inner_context_t **a_inner_context)
{
  *a_inner_context = reinterpret_cast<grm_inner_context_t *>(
      new GRM::Context::Inner(*reinterpret_cast<const GRM::Context *>(context), key));
}

void grm_inner_context_set_int(grm_inner_context_t *inner_context, int *value, unsigned int value_length)
{
  GRM::Context::Inner *inner = reinterpret_cast<GRM::Context::Inner *>(inner_context);
  std::vector<int> vec;
  for (int i = 0; i < value_length; i++)
    {
      vec.push_back(value[i]);
    }
  *inner = vec;
}

void grm_inner_context_set_double(grm_inner_context_t *inner_context, double *value, unsigned int value_length)
{
  GRM::Context::Inner *inner = reinterpret_cast<GRM::Context::Inner *>(inner_context);
  std::vector<double> vec;
  for (int i = 0; i < value_length; i++)
    {
      vec.push_back(value[i]);
    }
  *inner = vec;
}

void grm_inner_context_set_string(grm_inner_context_t *inner_context, const char **value, unsigned int value_length)
{
  GRM::Context::Inner *inner = reinterpret_cast<GRM::Context::Inner *>(inner_context);
  std::vector<std::string> vec;
  for (int i = 0; i < value_length; i++)
    {
      vec.push_back(value[i]);
    }
  *inner = vec;
}

int grm_inner_context_int_used(grm_inner_context_t *inner_context)
{
  GRM::Context::Inner *inner = reinterpret_cast<GRM::Context::Inner *>(inner_context);
  return inner->intUsed();
}

int grm_inner_context_double_used(grm_inner_context_t *inner_context)
{
  GRM::Context::Inner *inner = reinterpret_cast<GRM::Context::Inner *>(inner_context);
  return inner->doubleUsed();
}

int grm_inner_context_string_used(grm_inner_context_t *inner_context)
{
  GRM::Context::Inner *inner = reinterpret_cast<GRM::Context::Inner *>(inner_context);
  return inner->stringUsed();
}

void grm_inner_context_delete_key(grm_inner_context_t *inner_context, const char *key)
{
  GRM::Context::Inner *inner = reinterpret_cast<GRM::Context::Inner *>(inner_context);
  inner->deleteKey(key);
}

void grm_inner_context_use_context_key(grm_inner_context_t *inner_context, const char *key, const char *old_key)
{
  GRM::Context::Inner *inner = reinterpret_cast<GRM::Context::Inner *>(inner_context);
  inner->useContextKey(key, old_key);
}

void grm_inner_context_decrement_key(grm_inner_context_t *inner_context, const char *key)
{
  GRM::Context::Inner *inner = reinterpret_cast<GRM::Context::Inner *>(inner_context);
  inner->decrementKey(key);
}

void grm_new_context(grm_context_t **a_context)
{
  GRM::Context *context = new GRM::Context();
  *a_context = reinterpret_cast<grm_context *>(context);
}

/* =============================== creator ========================================================================== */
/* ------------------------------- create functions ----------------------------------------------------------------- */

grm_creator_t *grm_create_creator(grm_context_t *context)
{
  auto context_ptr = reinterpret_cast<GRM::Context *>(context);
  auto creator_ptr = GRM::Creator::createCreator(std::shared_ptr<GRM::Context>(context_ptr));

  return reinterpret_cast<grm_creator_t *>(creator_ptr.get());
}

grm_element_t *grm_create_plot(int plot_id, grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> plot_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createPlot(plot_id, element_ptr);

  return reinterpret_cast<grm_element_t *>(plot_ptr.get());
}

grm_element_t *grm_create_central_region(grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> central_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createCentralRegion(element_ptr);

  return reinterpret_cast<grm_element_t *>(central_ptr.get());
}

grm_element_t *grm_create_polymarkers(const char *x_key, double *x, int x_length, const char *y_key, double *y,
                                      int y_length, grm_context_t *ext_context, int marker_type, double marker_size,
                                      int marker_color_ind, grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  const std::shared_ptr<GRM::Context> context_ptr =
      ext_context == nullptr ? std::shared_ptr<GRM::Context>(reinterpret_cast<GRM::Context *>(ext_context)) : nullptr;
  std::vector<double> x_vec, y_vec;
  for (int i = 0; i < x_length; i++)
    {
      x_vec.push_back(x[i]);
    }
  for (int i = 0; i < y_length; i++)
    {
      y_vec.push_back(y[i]);
    }
  std::shared_ptr<GRM::Element> polymarker_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createPolymarker(
          x_key, x_vec, y_key, y_vec, context_ptr, marker_type, marker_size, marker_color_ind, element_ptr);

  return reinterpret_cast<grm_element_t *>(polymarker_ptr.get());
}

grm_element_t *grm_create_polymarker(double x, double y, int marker_type, double marker_size, int marker_colorind,
                                     grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> polymarker_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createPolymarker(x, y, marker_type, marker_size,
                                                                                marker_colorind, element_ptr);

  return reinterpret_cast<grm_element_t *>(polymarker_ptr.get());
}

grm_element_t *grm_create_polylines(const char *x_key, double *x, int x_length, const char *y_key, double *y,
                                    int y_length, grm_context_t *ext_context, int line_type, double line_width,
                                    int line_colorind, grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  const std::shared_ptr<GRM::Context> context_ptr =
      ext_context == nullptr ? std::shared_ptr<GRM::Context>(reinterpret_cast<GRM::Context *>(ext_context)) : nullptr;
  std::vector<double> x_vec, y_vec;
  for (int i = 0; i < x_length; i++)
    {
      x_vec.push_back(x[i]);
    }
  for (int i = 0; i < y_length; i++)
    {
      y_vec.push_back(y[i]);
    }
  std::shared_ptr<GRM::Element> polyline_ptr = reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createPolyline(
      x_key, x_vec, y_key, y_vec, context_ptr, line_type, line_width, line_colorind, element_ptr);

  return reinterpret_cast<grm_element_t *>(polyline_ptr.get());
}

grm_element_t *grm_create_polyline(double x1, double x2, double y1, double y2, int line_type, double line_width,
                                   int line_colorind, grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> polyline_ptr = reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createPolyline(
      x1, x2, y1, y2, line_type, line_width, line_colorind, element_ptr);

  return reinterpret_cast<grm_element_t *>(polyline_ptr.get());
}

grm_element_t *grm_create_text(double x, double y, const char *text, grm_coordinate_space_t *space,
                               grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> text_ptr = reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createText(
      x, y, text, *reinterpret_cast<CoordinateSpace *>(space), element_ptr);

  return reinterpret_cast<grm_element_t *>(text_ptr.get());
}

grm_element_t *grm_create_fill_area(const char *x_key, double *x, int x_length, const char *y_key, double *y,
                                    int y_length, grm_context_t *ext_context, int fill_int_style, int fill_style,
                                    int fill_color_ind, grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  const std::shared_ptr<GRM::Context> context_ptr =
      ext_context == nullptr ? std::shared_ptr<GRM::Context>(reinterpret_cast<GRM::Context *>(ext_context)) : nullptr;
  std::vector<double> x_vec, y_vec;
  for (int i = 0; i < x_length; i++)
    {
      x_vec.push_back(x[i]);
    }
  for (int i = 0; i < y_length; i++)
    {
      y_vec.push_back(y[i]);
    }
  std::shared_ptr<GRM::Element> fill_area_ptr = reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createFillArea(
      x_key, x_vec, y_key, y_vec, context_ptr, fill_int_style, fill_style, fill_color_ind, element_ptr);

  return reinterpret_cast<grm_element_t *>(fill_area_ptr.get());
}

grm_element_t *grm_create_cell_array(double xmin, double xmax, double ymin, double ymax, int dimx, int dimy, int scol,
                                     int srow, int ncol, int nrow, const char *color_key, int *color, int color_length,
                                     grm_context_t *ext_context, grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  const std::shared_ptr<GRM::Context> context_ptr =
      ext_context == nullptr ? std::shared_ptr<GRM::Context>(reinterpret_cast<GRM::Context *>(ext_context)) : nullptr;
  std::vector<int> color_vec;
  for (int i = 0; i < color_length; i++)
    {
      color_vec.push_back(color[i]);
    }
  std::shared_ptr<GRM::Element> cell_array_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createCellArray(
          xmin, xmax, ymin, ymax, dimx, dimy, scol, srow, ncol, nrow, color_key, color_vec, context_ptr, element_ptr);

  return reinterpret_cast<grm_element_t *>(cell_array_ptr.get());
}

grm_element_t *grm_create_non_uniform_polar_cell_array(double x_org, double y_org, const char *theta_key, double *theta,
                                                       int theta_length, const char *r_key, double *r, int r_length,
                                                       int dim_theta, int dim_r, int s_col, int s_row, int n_col,
                                                       int n_row, const char *color_key, int *color, int color_length,
                                                       grm_context_t *ext_context, grm_element_t *ext_element,
                                                       grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  const std::shared_ptr<GRM::Context> context_ptr =
      ext_context == nullptr ? std::shared_ptr<GRM::Context>(reinterpret_cast<GRM::Context *>(ext_context)) : nullptr;
  std::vector<int> color_vec;
  std::vector<double> theta_vec, r_vec;
  for (int i = 0; i < color_length; i++)
    {
      color_vec.push_back(color[i]);
    }
  for (int i = 0; i < theta_length; i++)
    {
      theta_vec.push_back(theta[i]);
    }
  for (int i = 0; i < r_length; i++)
    {
      r_vec.push_back(r[i]);
    }
  std::shared_ptr<GRM::Element> non_uniform_polar_cell_array_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createNonUniformPolarCellArray(
          x_org, y_org, theta_key, theta_vec, r_key, r_vec, dim_theta, dim_r, s_col, s_row, n_col, n_row, color_key,
          color_vec, context_ptr, element_ptr);

  return reinterpret_cast<grm_element_t *>(non_uniform_polar_cell_array_ptr.get());
}

grm_element_t *grm_create_polar_cell_array(double x_org, double y_org, double theta_min, double theta_max, double r_min,
                                           double r_max, int dim_theta, int dim_r, int s_col, int s_row, int n_col,
                                           int n_row, const char *color_key, int *color, int color_length,
                                           grm_context_t *ext_context, grm_element_t *ext_element,
                                           grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  const std::shared_ptr<GRM::Context> context_ptr =
      ext_context == nullptr ? std::shared_ptr<GRM::Context>(reinterpret_cast<GRM::Context *>(ext_context)) : nullptr;
  std::vector<int> color_vec;
  for (int i = 0; i < color_length; i++)
    {
      color_vec.push_back(color[i]);
    }
  std::shared_ptr<GRM::Element> polar_cell_array_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createPolarCellArray(
          x_org, y_org, theta_min, theta_max, r_min, r_max, dim_theta, dim_r, s_col, s_row, n_col, n_row, color_key,
          color_vec, context_ptr, element_ptr);

  return reinterpret_cast<grm_element_t *>(polar_cell_array_ptr.get());
}

grm_element_t *grm_create_empty_axis(grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> empty_axes_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createEmptyAxis(element_ptr);

  return reinterpret_cast<grm_element_t *>(empty_axes_ptr.get());
}

grm_element_t *grm_create_axis(double min_val, double max_val, double tick, double org, double pos, int major_count,
                               int num_ticks, int num_tick_labels, double tick_size, int tick_orientation,
                               double label_pos, grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> axis_ptr = reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createAxis(
      min_val, max_val, tick, org, pos, major_count, num_ticks, num_tick_labels, tick_size, tick_orientation, label_pos,
      element_ptr);

  return reinterpret_cast<grm_element_t *>(axis_ptr.get());
}

grm_element_t *grm_create_tick_group(int is_major, const char *tick_label, double value, double width,
                                     grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> tick_group_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createTickGroup(is_major, tick_label, value, width,
                                                                               element_ptr);

  return reinterpret_cast<grm_element_t *>(tick_group_ptr.get());
}

grm_element_t *grm_create_tick(int is_major, double value, grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> tick_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createTick(is_major, value, element_ptr);

  return reinterpret_cast<grm_element_t *>(tick_ptr.get());
}

grm_element_t *grm_create_grid_line(int is_major, double value, grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> grid_line_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createGridLine(is_major, value, element_ptr);

  return reinterpret_cast<grm_element_t *>(grid_line_ptr.get());
}

grm_element_t *grm_create_legend(grm_element_t *ext_element, grm_context_t *ext_context, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  const std::shared_ptr<GRM::Context> context_ptr =
      ext_context == nullptr ? std::shared_ptr<GRM::Context>(reinterpret_cast<GRM::Context *>(ext_context)) : nullptr;
  std::shared_ptr<GRM::Element> legend_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createLegend(element_ptr, context_ptr);

  return reinterpret_cast<grm_element_t *>(legend_ptr.get());
}

grm_element_t *grm_create_pie_segment(const double start_angle, const double end_angle, const char *text,
                                      const int color_index, grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> pie_segment_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createPieSegment(start_angle, end_angle, text,
                                                                                color_index, element_ptr);

  return reinterpret_cast<grm_element_t *>(pie_segment_ptr.get());
}

grm_element_t *grm_create_bar(const double x1, const double x2, const double y1, const double y2,
                              const int bar_color_index, const int edge_color_index, const char *bar_color_rgb,
                              const char *edge_color_rgb, const double linewidth, const char *text,
                              grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> bar_ptr = reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createBar(
      x1, x2, y1, y2, bar_color_index, edge_color_index, bar_color_rgb, edge_color_rgb, linewidth, text, element_ptr);

  return reinterpret_cast<grm_element_t *>(bar_ptr.get());
}

grm_element_t *grm_create_series(const char *name, grm_creator_t *creator)
{
  std::shared_ptr<GRM::Element> series_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createSeries(name);

  return reinterpret_cast<grm_element_t *>(series_ptr.get());
}

grm_element_t *grm_create_draw_image(double xmin, double ymin, double xmax, double ymax, int width, int height,
                                     const char *data_key, int *data, int data_length, int model,
                                     grm_context_t *ext_context, grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  const std::shared_ptr<GRM::Context> context_ptr =
      ext_context == nullptr ? std::shared_ptr<GRM::Context>(reinterpret_cast<GRM::Context *>(ext_context)) : nullptr;
  std::vector<int> data_vec;
  for (int i = 0; i < data_length; i++)
    {
      data_vec.push_back(data[i]);
    }
  std::shared_ptr<GRM::Element> draw_image_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createDrawImage(
          xmin, ymin, xmax, ymax, width, height, data_key, data_vec, model, context_ptr, element_ptr);

  return reinterpret_cast<grm_element_t *>(draw_image_ptr.get());
}

grm_element_t *grm_create_draw_arc(double xmin, double xmax, double ymin, double ymax, double start_angle,
                                   double end_angle, grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> draw_arc_ptr = reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createDrawArc(
      xmin, xmax, ymin, ymax, start_angle, end_angle, element_ptr);

  return reinterpret_cast<grm_element_t *>(draw_arc_ptr.get());
}

grm_element_t *grm_create_fill_arc(double xmin, double xmax, double ymin, double ymax, double a1, double a2,
                                   int fill_int_style, int fill_style, int fill_color_ind, grm_element_t *ext_element,
                                   grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> fill_arc_ptr = reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createFillArc(
      xmin, xmax, ymin, ymax, a1, a2, fill_int_style, fill_style, fill_color_ind, element_ptr);

  return reinterpret_cast<grm_element_t *>(fill_arc_ptr.get());
}

grm_element_t *grm_create_draw_rect(double xmin, double xmax, double ymin, double ymax, grm_element_t *ext_element,
                                    grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> draw_rect_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createDrawRect(xmin, xmax, ymin, ymax, element_ptr);

  return reinterpret_cast<grm_element_t *>(draw_rect_ptr.get());
}

grm_element_t *grm_create_fill_rect(double xmin, double xmax, double ymin, double ymax, int fill_int_style,
                                    int fill_style, int fill_color_ind, grm_element_t *ext_element,
                                    grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> fill_rect_ptr = reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createFillRect(
      xmin, xmax, ymin, ymax, fill_int_style, fill_style, fill_color_ind, element_ptr);

  return reinterpret_cast<grm_element_t *>(fill_rect_ptr.get());
}

grm_element_t *grm_create_quiver(const char *x_key, double *x, int x_length, const char *y_key, double *y, int y_length,
                                 const char *u_key, double *u, int u_length, const char *v_key, double *v, int v_length,
                                 int colored, grm_context_t *ext_context, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Context> context_ptr =
      ext_context == nullptr ? std::shared_ptr<GRM::Context>(reinterpret_cast<GRM::Context *>(ext_context)) : nullptr;
  std::vector<double> x_vec, y_vec, u_vec, v_vec;
  for (int i = 0; i < x_length; i++)
    {
      x_vec.push_back(x[i]);
    }
  for (int i = 0; i < y_length; i++)
    {
      y_vec.push_back(y[i]);
    }
  for (int i = 0; i < u_length; i++)
    {
      u_vec.push_back(u[i]);
    }
  for (int i = 0; i < v_length; i++)
    {
      v_vec.push_back(v[i]);
    }
  std::shared_ptr<GRM::Element> quiver_ptr = reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createQuiver(
      x_key, x_vec, y_key, y_vec, u_key, u_vec, v_key, v_vec, colored, context_ptr);

  return reinterpret_cast<grm_element_t *>(quiver_ptr.get());
}

grm_element_t *grm_create_hexbin(const char *x_key, double *x, int x_length, const char *y_key, double *y, int y_length,
                                 grm_context_t *ext_context, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Context> context_ptr =
      ext_context == nullptr ? std::shared_ptr<GRM::Context>(reinterpret_cast<GRM::Context *>(ext_context)) : nullptr;
  std::vector<double> x_vec, y_vec;
  for (int i = 0; i < x_length; i++)
    {
      x_vec.push_back(x[i]);
    }
  for (int i = 0; i < y_length; i++)
    {
      y_vec.push_back(y[i]);
    }
  std::shared_ptr<GRM::Element> hexbin_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createHexbin(x_key, x_vec, y_key, y_vec, context_ptr);

  return reinterpret_cast<grm_element_t *>(hexbin_ptr.get());
}

grm_element_t *grm_create_colorbar(unsigned int num_color_values, grm_context_t *ext_context,
                                   grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  const std::shared_ptr<GRM::Context> context_ptr =
      ext_context == nullptr ? std::shared_ptr<GRM::Context>(reinterpret_cast<GRM::Context *>(ext_context)) : nullptr;
  std::shared_ptr<GRM::Element> colorbar_ptr = reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createColorbar(
      num_color_values, context_ptr, element_ptr);

  return reinterpret_cast<grm_element_t *>(colorbar_ptr.get());
}

grm_element_t *grm_create_non_uniform_cell_array(const char *x_key, double *x, int x_length, const char *y_key,
                                                 double *y, int y_length, int dimx, int dimy, int scol, int srow,
                                                 int ncol, int nrow, const char *color_key, int *color,
                                                 int color_length, grm_context_t *ext_context,
                                                 grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  const std::shared_ptr<GRM::Context> context_ptr =
      ext_context == nullptr ? std::shared_ptr<GRM::Context>(reinterpret_cast<GRM::Context *>(ext_context)) : nullptr;
  std::vector<double> x_vec, y_vec;
  std::vector<int> color_vec;
  for (int i = 0; i < x_length; i++)
    {
      x_vec.push_back(x[i]);
    }
  for (int i = 0; i < y_length; i++)
    {
      y_vec.push_back(y[i]);
    }
  for (int i = 0; i < color_length; i++)
    {
      color_vec.push_back(color[i]);
    }
  std::shared_ptr<GRM::Element> non_uniform_cell_array_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createNonUniformCellArray(
          x_key, x_vec, y_key, y_vec, dimx, dimy, scol, srow, ncol, nrow, color_key, color_vec, context_ptr);

  return reinterpret_cast<grm_element_t *>(non_uniform_cell_array_ptr.get());
}

grm_element_t *grm_create_grid_3d(double x_tick, double y_tick, double z_tick, double x_org, double y_org, double z_org,
                                  int major_x, int major_y, int major_z, grm_element_t *ext_element,
                                  grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> grid_3d_ptr = reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createGrid3d(
      x_tick, y_tick, z_tick, x_org, y_org, z_org, major_x, major_y, major_z, element_ptr);

  return reinterpret_cast<grm_element_t *>(grid_3d_ptr.get());
}

grm_element_t *grm_create_empty_grid_3d(int x_grid, int y_grid, int z_grid, grm_element_t *ext_element,
                                        grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> empty_grid_3d_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createEmptyGrid3d(x_grid, y_grid, z_grid, element_ptr);

  return reinterpret_cast<grm_element_t *>(empty_grid_3d_ptr.get());
}

grm_element_t *grm_create_axes_3d(double x_tick, double y_tick, double z_tick, double x_org, double y_org, double z_org,
                                  int major_x, int major_y, int major_z, int tick_orientation,
                                  grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> axes_3d_ptr = reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createAxes3d(
      x_tick, y_tick, z_tick, x_org, y_org, z_org, major_x, major_y, major_z, tick_orientation, element_ptr);

  return reinterpret_cast<grm_element_t *>(axes_3d_ptr.get());
}

grm_element_t *grm_create_empty_axes_3d(int tick_orientation, grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> empty_axes_3d_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createEmptyAxes3d(tick_orientation, element_ptr);

  return reinterpret_cast<grm_element_t *>(empty_axes_3d_ptr.get());
}

grm_element_t *grm_create_polyline_3d(const char *x_key, double *x, int x_length, const char *y_key, double *y,
                                      int y_length, const char *z_key, double *z, int z_length,
                                      grm_context_t *ext_context, grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  const std::shared_ptr<GRM::Context> context_ptr =
      ext_context == nullptr ? std::shared_ptr<GRM::Context>(reinterpret_cast<GRM::Context *>(ext_context)) : nullptr;
  std::vector<double> x_vec, y_vec, z_vec;
  for (int i = 0; i < x_length; i++)
    {
      x_vec.push_back(x[i]);
    }
  for (int i = 0; i < y_length; i++)
    {
      y_vec.push_back(y[i]);
    }
  for (int i = 0; i < z_length; i++)
    {
      z_vec.push_back(z[i]);
    }
  std::shared_ptr<GRM::Element> polyline_3d_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createPolyline3d(x_key, x_vec, y_key, y_vec, z_key,
                                                                                z_vec, context_ptr, element_ptr);

  return reinterpret_cast<grm_element_t *>(polyline_3d_ptr.get());
}

grm_element_t *grm_create_polymarker_3d(const char *x_key, double *x, int x_length, const char *y_key, double *y,
                                        int y_length, const char *z_key, double *z, int z_length,
                                        grm_context_t *ext_context, grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  const std::shared_ptr<GRM::Context> context_ptr =
      ext_context == nullptr ? std::shared_ptr<GRM::Context>(reinterpret_cast<GRM::Context *>(ext_context)) : nullptr;
  std::vector<double> x_vec, y_vec, z_vec;
  for (int i = 0; i < x_length; i++)
    {
      x_vec.push_back(x[i]);
    }
  for (int i = 0; i < y_length; i++)
    {
      y_vec.push_back(y[i]);
    }
  for (int i = 0; i < z_length; i++)
    {
      z_vec.push_back(z[i]);
    }
  std::shared_ptr<GRM::Element> polymarker_3d_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createPolyline3d(x_key, x_vec, y_key, y_vec, z_key,
                                                                                z_vec, context_ptr, element_ptr);

  return reinterpret_cast<grm_element_t *>(polymarker_3d_ptr.get());
}

grm_element_t *grm_create_draw_graphics(const char *data_key, int *data, int data_length, grm_context_t *ext_context,
                                        grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  const std::shared_ptr<GRM::Context> context_ptr =
      ext_context == nullptr ? std::shared_ptr<GRM::Context>(reinterpret_cast<GRM::Context *>(ext_context)) : nullptr;
  std::vector<int> data_vec;
  for (int i = 0; i < data_length; i++)
    {
      data_vec.push_back(data[i]);
    }
  std::shared_ptr<GRM::Element> draw_graphics_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createDrawGraphics(data_key, data_vec, context_ptr,
                                                                                  element_ptr);

  return reinterpret_cast<grm_element_t *>(draw_graphics_ptr.get());
}

grm_element_t *grm_create_tri_surface(const char *px_key, double *px, int px_length, const char *py_key, double *py,
                                      int py_length, const char *pz_key, double *pz, int pz_length,
                                      grm_context_t *ext_context, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Context> context_ptr =
      ext_context == nullptr ? std::shared_ptr<GRM::Context>(reinterpret_cast<GRM::Context *>(ext_context)) : nullptr;
  std::vector<double> px_vec, py_vec, pz_vec;
  for (int i = 0; i < px_length; i++)
    {
      px_vec.push_back(px[i]);
    }
  for (int i = 0; i < py_length; i++)
    {
      py_vec.push_back(py[i]);
    }
  for (int i = 0; i < pz_length; i++)
    {
      pz_vec.push_back(pz[i]);
    }
  std::shared_ptr<GRM::Element> tri_surface_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createTriSurface(px_key, px_vec, py_key, py_vec, pz_key,
                                                                                pz_vec, context_ptr);

  return reinterpret_cast<grm_element_t *>(tri_surface_ptr.get());
}

grm_element_t *grm_create_titles_3d(const char *x_label, const char *y_label, const char *z_label,
                                    grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> titles_3d_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createTitles3d(x_label, y_label, z_label, element_ptr);

  return reinterpret_cast<grm_element_t *>(titles_3d_ptr.get());
}

grm_element_t *grm_create_layout_grid(grm_grid_t *grid, grm_creator_t *creator)
{
  GRM::Grid grm_grid = *reinterpret_cast<GRM::Grid *>(grid);
  std::shared_ptr<GRM::Element> layout_grid_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createLayoutGrid(grm_grid);

  return reinterpret_cast<grm_element_t *>(layout_grid_ptr.get());
}

grm_element_t *grm_create_layout_grid_element(grm_element_t *grid_element, grm_slice_t *slice, grm_creator_t *creator)
{
  GRM::GridElement grm_grid_element = *reinterpret_cast<GRM::GridElement *>(grid_element);
  GRM::Slice grm_slice = *reinterpret_cast<GRM::Slice *>(slice);
  std::shared_ptr<GRM::Element> layout_grid_element_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createLayoutGridElement(grm_grid_element, grm_slice);

  return reinterpret_cast<grm_element_t *>(layout_grid_element_ptr.get());
}

grm_element_t *grm_create_panzoom(double x, double y, double xzoom, double yzoom, grm_creator_t *creator)
{
  std::shared_ptr<GRM::Element> panzoom_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createPanzoom(x, y, xzoom, yzoom);

  return reinterpret_cast<grm_element_t *>(panzoom_ptr.get());
}

grm_element_t *grm_create_polar_bar(double count, int class_nr, grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> polar_bar_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createPolarBar(count, class_nr, element_ptr);

  return reinterpret_cast<grm_element_t *>(polar_bar_ptr.get());
}

grm_element_t *grm_create_error_bar(double error_bar_x, double error_bar_y_min, double error_bar_y_max,
                                    int color_error_bar, grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> error_bar_ptr = reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createErrorBar(
      error_bar_x, error_bar_y_min, error_bar_y_max, color_error_bar, element_ptr);

  return reinterpret_cast<grm_element_t *>(error_bar_ptr.get());
}

grm_element_t *grm_create_integral(double int_lim_low, double int_lim_high, grm_element_t *ext_element,
                                   grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> integral_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createIntegral(int_lim_low, int_lim_high, element_ptr);

  return reinterpret_cast<grm_element_t *>(integral_ptr.get());
}

grm_element_t *grm_create_side_region(const char *location, grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> side_region_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createSideRegion(location, element_ptr);

  return reinterpret_cast<grm_element_t *>(side_region_ptr.get());
}

grm_element_t *grm_create_text_region(grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> text_region_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createTextRegion(element_ptr);

  return reinterpret_cast<grm_element_t *>(text_region_ptr.get());
}

grm_element_t *grm_create_side_plot_region(grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> side_plot_region_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createSidePlotRegion(element_ptr);

  return reinterpret_cast<grm_element_t *>(side_plot_region_ptr.get());
}

grm_element_t *grm_create_radial_axes(grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> radial_axes_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createRadialAxes(element_ptr);

  return reinterpret_cast<grm_element_t *>(radial_axes_ptr.get());
}

grm_element_t *grm_create_theta_axes(grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> theta_axes_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createThetaAxes(element_ptr);

  return reinterpret_cast<grm_element_t *>(theta_axes_ptr.get());
}

grm_element_t *grm_create_angle_line(double x, double y, const char *angle_label, grm_element_t *ext_element,
                                     grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> angle_line_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createAngleLine(x, y, angle_label, element_ptr);

  return reinterpret_cast<grm_element_t *>(angle_line_ptr.get());
}

grm_element_t *grm_create_arc_grid_line(double value, grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> arc_grid_line_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createArcGridLine(value, element_ptr);

  return reinterpret_cast<grm_element_t *>(arc_grid_line_ptr.get());
}

grm_element_t *grm_create_overlay(grm_element_t *ext_element, grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> overlay_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createOverlay(element_ptr);

  return reinterpret_cast<grm_element_t *>(overlay_ptr.get());
}

grm_element_t *grm_create_overlay_element(double x, double y, const char *type, grm_element_t *ext_element,
                                          grm_creator_t *creator)
{
  const std::shared_ptr<GRM::Element> element_ptr =
      ext_element == nullptr ? std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(ext_element)) : nullptr;
  std::shared_ptr<GRM::Element> overlay_element_ptr =
      reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::createOverlayElement(x, y, type, element_ptr);

  return reinterpret_cast<grm_element_t *>(overlay_element_ptr.get());
}

/* ---------------------------------- get functions ------------------------------------------------------------------*/

int grm_creator_get_axis_id(grm_creator_t *creator)
{
  return reinterpret_cast<GRM::Creator *>(creator)->GRM::Creator::getAxisId();
}

/* =============================== group mask ======================================================================= */

grm_element_t *grm_get_document_root_c(void)
{
  return reinterpret_cast<grm_element_t *>(grm_get_document_root().get());
}

/* =============================== process attributes =============================================================== */

void grm_process_limits(grm_element_t *element)
{
  GRM::processLimits(std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element)));
}

void grm_process_window(grm_element_t *element)
{
  GRM::processLimits(std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element)));
}

/* =============================== render =========================================================================== */

void grm_render_add_valid_context_key(const char *key)
{
  GRM::addValidContextKey(key);
}

const grm_group_mask_t *grm_render_get_group_mask()
{
  return reinterpret_cast<const grm_group_mask_t *>(GRM::getGroupMask());
}

void grm_render_caller()
{
  GRM::renderCaller();
}

void grm_render_update_context_attribute(grm_element_t *element, const char *attr, grm_value_t *old_value)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  GRM::updateContextAttribute(element_ptr, attr, *reinterpret_cast<GRM::Value *>(old_value));
}

void grm_delete_context_attribute(grm_element_t *element)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  GRM::deleteContextAttribute(element_ptr);
}

void grm_cleanup_element(grm_element_t *element)
{
  auto element_ptr = reinterpret_cast<GRM::Element *>(element);
  GRM::cleanupElement(*element_ptr);
}

grm_render_t *grm_create_render()
{
  return reinterpret_cast<grm_render_t *>(GRM::Render::createRender().get());
}

/* ------------------------------- setter functions ----------------------------------------------------------------*/

void grm_render_set_clip_region(grm_element_t *element, int region, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setClipRegion(element_ptr, region);
}

void grm_render_set_viewport(grm_element_t *element, double xmin, double xmax, double ymin, double ymax,
                             grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setViewport(element_ptr, xmin, xmax, ymin, ymax);
}

void grm_render_set_ws_viewport(grm_element_t *element, double xmin, double xmax, double ymin, double ymax,
                                grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setWSViewport(element_ptr, xmin, xmax, ymin, ymax);
}

void grm_render_set_window(grm_element_t *element, double xmin, double xmax, double ymin, double ymax,
                           grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setWindow(element_ptr, xmin, xmax, ymin, ymax);
}

void grm_render_set_ws_window(grm_element_t *element, double xmin, double xmax, double ymin, double ymax,
                              grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setWSWindow(element_ptr, xmin, xmax, ymin, ymax);
}

void grm_render_set_marker_type(grm_element_t *element, int type, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setMarkerType(element_ptr, type);
}

void grm_render_set_marker_types(grm_element_t *element, const char *types_key, int *types, int types_length,
                                 grm_context_t *ext_context, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  auto context_ptr = std::shared_ptr<GRM::Context>(reinterpret_cast<GRM::Context *>(ext_context));
  std::vector<int> types_vec;
  for (int i = 0; i < types_length; i++)
    {
      types_vec.push_back(types[i]);
    }
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setMarkerType(element_ptr, types_key, types_vec, context_ptr);
}

void grm_render_set_marker_sizes(grm_element_t *element, const char *sizes_key, double *sizes, int sizes_length,
                                 grm_context_t *ext_context, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  auto context_ptr = std::shared_ptr<GRM::Context>(reinterpret_cast<GRM::Context *>(ext_context));
  std::vector<double> sizes_vec;
  for (int i = 0; i < sizes_length; i++)
    {
      sizes_vec.push_back(sizes[i]);
    }
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setMarkerSize(element_ptr, sizes_key, sizes_vec, context_ptr);
}

void grm_render_set_marker_size(grm_element_t *element, double size, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setMarkerType(element_ptr, size);
}

void grm_render_set_marker_color_inds(grm_element_t *element, const char *colorinds_key, int *colorinds,
                                      int colorinds_length, grm_context_t *ext_context, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  auto context_ptr = std::shared_ptr<GRM::Context>(reinterpret_cast<GRM::Context *>(ext_context));
  std::vector<int> colorinds_vec;
  for (int i = 0; i < colorinds_length; i++)
    {
      colorinds_vec.push_back(colorinds[i]);
    }
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setMarkerColorInd(element_ptr, colorinds_key, colorinds_vec,
                                                                          context_ptr);
}

void grm_render_set_marker_color_ind(grm_element_t *element, int color, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setMarkerColorInd(element_ptr, color);
}

void grm_render_set_line_types(grm_element_t *element, const char *types_key, int *types, int types_length,
                               grm_context_t *ext_context, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  auto context_ptr = std::shared_ptr<GRM::Context>(reinterpret_cast<GRM::Context *>(ext_context));
  std::vector<int> types_vec;
  for (int i = 0; i < types_length; i++)
    {
      types_vec.push_back(types[i]);
    }
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setLineType(element_ptr, types_key, types_vec, context_ptr);
}

void grm_render_set_line_type(grm_element_t *element, int type, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setLineType(element_ptr, type);
}

void grm_render_set_line_widths(grm_element_t *element, const char *widths_key, double *widths, int widths_length,
                                grm_context_t *ext_context, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  auto context_ptr = std::shared_ptr<GRM::Context>(reinterpret_cast<GRM::Context *>(ext_context));
  std::vector<double> widths_vec;
  for (int i = 0; i < widths_length; i++)
    {
      widths_vec.push_back(widths[i]);
    }
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setLineWidth(element_ptr, widths_key, widths_vec, context_ptr);
}

void grm_render_set_line_width(grm_element_t *element, double width, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setLineWidth(element_ptr, width);
}

void grm_render_set_line_color_ind(grm_element_t *element, const char *colorinds_key, int *colorinds,
                                   int colorinds_length, grm_context_t *ext_context, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  auto context_ptr = std::shared_ptr<GRM::Context>(reinterpret_cast<GRM::Context *>(ext_context));
  std::vector<int> colorinds_vec;
  for (int i = 0; i < colorinds_length; i++)
    {
      colorinds_vec.push_back(colorinds[i]);
    }
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setLineColorInd(element_ptr, colorinds_key, colorinds_vec,
                                                                        context_ptr);
}

void grm_render_set_line_color_inds(grm_element_t *element, int color, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setLineColorInd(element_ptr, color);
}

void grm_render_set_char_up(grm_element_t *element, double ux, double uy, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setCharUp(element_ptr, ux, uy);
}

void grm_render_set_text_align(grm_element_t *element, int horizontal, int vertical, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setTextAlign(element_ptr, horizontal, vertical);
}

void grm_render_set_text_width_and_height(grm_element_t *element, double width, double height, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setTextWidthAndHeight(element_ptr, width, height);
}

void grm_render_set_color_rep(grm_element_t *element, int index, double red, double green, double blue,
                              grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setColorRep(element_ptr, index, red, green, blue);
}

void grm_render_set_line_spec(grm_element_t *element, const char *spec, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setLineSpec(element_ptr, spec);
}

void grm_render_set_fill_int_style(grm_element_t *element, int index, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setFillIntStyle(element_ptr, index);
}

void grm_render_set_fill_color_ind(grm_element_t *element, int color, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setFillColorInd(element_ptr, color);
}

void grm_render_set_fill_style(grm_element_t *element, int index, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setFillStyle(element_ptr, index);
}

void grm_render_set_scale(grm_element_t *element, int scale, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setScale(element_ptr, scale);
}

void grm_render_set_window_3d(grm_element_t *element, double xmin, double xmax, double ymin, double ymax, double zmin,
                              double zmax, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setWindow3d(element_ptr, xmin, xmax, ymin, ymax, zmin, zmax);
}

void grm_render_set_space_3d(grm_element_t *element, double fov, double camera_distance, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setSpace3d(element_ptr, fov, camera_distance);
}

void grm_render_set_space(grm_element_t *element, double zmin, double zmax, int rotation, int tilt,
                          grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setSpace(element_ptr, zmin, zmax, rotation, tilt);
}

void grm_render_set_select_specific_xform(grm_element_t *element, int transform, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setSelectSpecificXform(element_ptr, transform);
}

void grm_render_set_text_color_ind(grm_element_t *element, int index, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setTextColorInd(element_ptr, index);
}

void grm_render_set_border_color_ind(grm_element_t *element, int index, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setBorderColorInd(element_ptr, index);
}

void grm_render_set_border_width(grm_element_t *element, double width, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setBorderWidth(element_ptr, width);
}

void grm_render_set_char_height(grm_element_t *element, double height, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setCharHeight(element_ptr, height);
}

void grm_render_set_transparency(grm_element_t *element, double transparency, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setTransparency(element_ptr, transparency);
}

void grm_render_set_resample_method(grm_element_t *element, int resample, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setResampleMethod(element_ptr, resample);
}

void grm_render_set_text_encoding(grm_element_t *element, int encoding, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setTextEncoding(element_ptr, encoding);
}

void grm_render_set_viewport_normalized(grm_element_t *element, double xmin, double xmax, double ymin, double ymax,
                                        grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setViewportNormalized(element_ptr, xmin, xmax, ymin, ymax);
}

void grm_render_set_origin_position(grm_element_t *element, const char *x_org_pos, const char *y_org_pos,
                                    grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setOriginPosition(element_ptr, x_org_pos, y_org_pos);
}

void grm_render_set_origin_position_3d(grm_element_t *element, const char *x_org_pos, const char *y_org_pos,
                                       const char *z_org_pos, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setOriginPosition3d(element_ptr, x_org_pos, y_org_pos,
                                                                            z_org_pos);
}

void grm_render_set_gr3_light_parameters(grm_element_t *element, double ambient, double diffuse, double specular,
                                         double specular_power, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setGR3LightParameters(element_ptr, ambient, diffuse, specular,
                                                                              specular_power);
}

void grm_render_set_auto_update(int update, grm_render_t *render)
{
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setAutoUpdate(update);
}

void grm_render_set_enable_editor(int update, grm_render_t *render)
{
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setEnableEditor(update);
}

void grm_render_set_active_figure(grm_element_t *element, grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setActiveFigure(element_ptr);
}

void grm_render_set_first_call(int call, grm_render_t *render)
{
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setFirstCall(call);
}

void grm_render_set_previous_scatter_marker_type(int *previous_scatter_marker_type, grm_render_t *render)
{
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setPreviousScatterMarkerType(previous_scatter_marker_type);
}

void grm_render_set_previous_line_marker_type(int *previous_line_marker_type, grm_render_t *render)
{
  reinterpret_cast<GRM::Render *>(render)->GRM::Render::setPreviousLineMarkerType(previous_line_marker_type);
}

/* ------------------------------- getter functions ----------------------------------------------------------------*/

grm_element_t *grm_render_get_active_figure(grm_render_t *render)
{
  return reinterpret_cast<grm_element_t *>(reinterpret_cast<GRM::Render *>(render)->getActiveFigure().get());
}

void grm_render_get_auto_update(int *update, grm_render_t *render)
{
  bool u;
  reinterpret_cast<GRM::Render *>(render)->getAutoUpdate(&u);
  *update = static_cast<int>(u);
}

void grm_render_get_enable_editor(int *update, grm_render_t *render)
{
  bool u;
  reinterpret_cast<GRM::Render *>(render)->getEnableEditor(&u);
  *update = static_cast<int>(u);
}

int grm_render_get_first_call(grm_render_t *render)
{
  return reinterpret_cast<GRM::Render *>(render)->getFirstCall();
}

int grm_render_get_z_queue_is_being_rendered(grm_render_t *render)
{
  return reinterpret_cast<GRM::Render *>(render)->getZQueueIsBeingRendered();
}

grm_manage_z_index_t *grm_render_get_z_index_manager(grm_render_t *render)
{
  return reinterpret_cast<grm_manage_z_index_t *>(reinterpret_cast<GRM::Render *>(render)->getZIndexManager());
}

int grm_render_get_redraw_ws(grm_render_t *render)
{
  return reinterpret_cast<GRM::Render *>(render)->getRedrawWs();
}

int grm_render_get_highlighted_attr_exist(grm_render_t *render)
{
  return reinterpret_cast<GRM::Render *>(render)->getHighlightedAttrExist();
}

int *grm_render_get_previous_scatter_marker_type(grm_render_t *render)
{
  return reinterpret_cast<GRM::Render *>(render)->getPreviousScatterMarkerType();
}

int *grm_render_get_previous_line_marker_type(grm_render_t *render)
{
  return reinterpret_cast<GRM::Render *>(render)->getPreviousLineMarkerType();
}

int grm_render_get_viewport(grm_element_t *element, double *xmin, double *xmax, double *ymin, double *ymax,
                            grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  return reinterpret_cast<GRM::Render *>(render)->getViewport(element_ptr, xmin, xmax, ymin, ymax);
}

grm_context_t *grm_render_get_render_context(grm_render_t *render)
{
  return reinterpret_cast<grm_context_t *>(reinterpret_cast<GRM::Render *>(render)->getRenderContext().get());
}

void grm_render_c(grm_render_t *render)
{
  reinterpret_cast<GRM::Render *>(render)->render();
}

void grm_render_context(grm_context_t *ext_context, grm_render_t *render)
{
  auto context_ptr = std::shared_ptr<GRM::Context>(reinterpret_cast<GRM::Context *>(ext_context));
  reinterpret_cast<GRM::Render *>(render)->render(context_ptr);
}

void grm_render_document(grm_document_t *document, grm_render_t *render)
{
  auto document_ptr = std::shared_ptr<GRM::Document>(reinterpret_cast<GRM::Document *>(document));
  reinterpret_cast<GRM::Render *>(render)->render(document_ptr);
}

void grm_render_both(grm_document_t *document, grm_context_t *ext_context, grm_render_t *render)
{
  auto context_ptr = std::shared_ptr<GRM::Context>(reinterpret_cast<GRM::Context *>(ext_context));
  auto document_ptr = std::shared_ptr<GRM::Document>(reinterpret_cast<GRM::Document *>(document));
  reinterpret_cast<GRM::Render *>(render)->render(document_ptr, context_ptr);
}

void grm_render_process_tree(grm_render_t *render)
{
  reinterpret_cast<GRM::Render *>(render)->processTree();
}

void grm_render_mask_highlight(grm_element_t *highlighted_element,
                               void (*image_callback)(int, unsigned int, unsigned int, unsigned int, unsigned int,
                                                      unsigned int *),
                               grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(highlighted_element));
  reinterpret_cast<GRM::Render *>(render)->renderMaskHighlight(element_ptr, image_callback);
}

void grm_render_finalize(grm_render_t *render)
{
  reinterpret_cast<GRM::Render *>(render)->finalize();
}

grm_context_t *grm_render_get_context(grm_render_t *render)
{
  return reinterpret_cast<grm_context_t *>(reinterpret_cast<GRM::Render *>(render)->getContext().get());
}

const char **grm_render_get_default_and_tooltip(grm_element_t *element, const char *attribute_name,
                                                grm_render_t *render)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  auto tooltips = reinterpret_cast<GRM::Render *>(render)->getDefaultAndTooltip(element_ptr, attribute_name);
  static auto tooltips_string = tooltips.data();

  return reinterpret_cast<const char **>(tooltips_string);
}

/* ----------------------- history -------------------------------------------------------------------------------- */

const char *grm_render_initialize_history(grm_render_t *render)
{
  return reinterpret_cast<GRM::Render *>(render)->initializeHistory();
}

/* =============================== render util ====================================================================== */

void grm_render_get_figure_size(int *pixel_width, int *pixel_height, double *metric_width, double *metric_height)
{
  GRM::getFigureSize(pixel_width, pixel_height, metric_width, metric_height);
}

void grm_render_calculate_char_height(grm_element_t *element)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  GRM::calculateCharHeight(element_ptr);
}

/* =============================== updater ========================================================================== */

void grm_update_filter(grm_element_t *element, const char *attr, const char *value)
{
  auto element_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(element));
  GRM::updateFilter(element_ptr, attr, value);
}
