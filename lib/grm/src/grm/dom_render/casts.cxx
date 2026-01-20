#include "grm/dom_render/casts.hxx"
#include "grm/logging_int.h"
#include "grm/util_int.h"
#include "grm/dom_render/not_found_error.hxx"
#include <grm/dom_render/render.hxx>

/* ------------------------------- string to int ---------------------------------------------------------------------*/

int GRM::algorithmStringToInt(const std::string &algorithm_str)
{
  if (algorithm_string_to_int.count(algorithm_str)) return algorithm_string_to_int[algorithm_str];
  logger((stderr, "Got unknown volume algorithm \"%s\"\n", algorithm_str.c_str()));
  throw std::logic_error("For volume series the given algorithm is unknown.\n");
}

/* ------------------------------- int to string ---------------------------------------------------------------------*/

std::string GRM::algorithmIntToString(int algorithm)
{
  for (auto const &map_elem : algorithm_string_to_int)
    {
      if (map_elem.second == algorithm) return map_elem.first;
    }
  logger((stderr, "Got unknown volume algorithm \"%i\"\n", algorithm));
  throw std::logic_error("For volume series the given algorithm is unknown.\n");
}

int GRM::scientificFormatStringToInt(const std::string &scientific_format_str)
{
  if (scientific_format_string_to_int.count(scientific_format_str))
    return scientific_format_string_to_int[scientific_format_str];
  else
    {
      logger((stderr, "Got unknown scientific_format \"%s\"\n", scientific_format_str.c_str()));
      throw std::logic_error("Given scientific_format is unknown.\n");
    }
}

std::string GRM::scientificFormatIntToString(int scientific_format)
{
  for (auto const &map_elem : scientific_format_string_to_int)
    {
      if (map_elem.second == scientific_format) return map_elem.first;
    }
  logger((stderr, "Got unknown scientific_format \"%i\"\n", scientific_format));
  throw std::logic_error("Given scientific_format is unknown.\n");
}

int GRM::errorBarStyleStringToInt(const std::string &error_bar_style_str)
{
  if (error_bar_style_string_to_int.count(error_bar_style_str))
    return error_bar_style_string_to_int[error_bar_style_str];
  logger((stderr, "Got unknown error_bar_style \"%s\"\n", error_bar_style_str.c_str()));
  throw std::logic_error("Given error_bar_style is unknown.\n");
}

std::string GRM::errorBarStyleIntToString(int error_bar_style)
{
  for (auto const &map_elem : error_bar_style_string_to_int)
    {
      if (map_elem.second == error_bar_style) return map_elem.first;
    }
  logger((stderr, "Got unknown error_bar_style \"%i\"\n", error_bar_style));
  throw std::logic_error("Given error_bar_style is unknown.\n");
}

int GRM::clipRegionStringToInt(const std::string &clip_region_str)
{
  if (clip_region_string_to_int.count(clip_region_str)) return clip_region_string_to_int[clip_region_str];
  logger((stderr, "Got unknown clip_region \"%s\"\n", clip_region_str.c_str()));
  throw std::logic_error("Given clip_region is unknown.\n");
}

std::string GRM::clipRegionIntToString(int clip_region)
{
  for (auto const &map_elem : clip_region_string_to_int)
    {
      if (map_elem.second == clip_region) return map_elem.first;
    }
  logger((stderr, "Got unknown clip_region \"%i\"\n", clip_region));
  throw std::logic_error("Given clip_region is unknown.\n");
}

int GRM::resampleMethodStringToInt(const std::string &resample_method_str)
{
  if (resample_method_string_to_int.count(resample_method_str))
    return resample_method_string_to_int[resample_method_str];
  logger((stderr, "Got unknown resample_method \"%s\"\n", resample_method_str.c_str()));
  throw std::logic_error("Given resample_method is unknown.\n");
}

std::string GRM::resampleMethodIntToString(int resample_method)
{
  for (auto const &map_elem : resample_method_string_to_int)
    {
      if (map_elem.second == resample_method) return map_elem.first;
    }
  logger((stderr, "Got unknown resample_method \"%i\"\n", resample_method));
  throw std::logic_error("Given resample_method is unknown.\n");
}

int GRM::fillStyleStringToInt(const std::string &fill_style_str)
{
  if (fill_style_string_to_int.count(fill_style_str)) return fill_style_string_to_int[fill_style_str];
  logger((stderr, "Got unknown fill_style \"%s\"\n", fill_style_str.c_str()));
  throw std::logic_error("Given fill_style is unknown.\n");
}

std::string GRM::fillStyleIntToString(int fill_style)
{
  for (auto const &map_elem : fill_style_string_to_int)
    {
      if (map_elem.second == fill_style) return map_elem.first;
    }
  logger((stderr, "Got unknown fill_style \"%i\"\n", fill_style));
  throw std::logic_error("Given fill_style is unknown.\n");
}

int GRM::fillIntStyleStringToInt(const std::string &fill_int_style_str)
{
  if (fill_int_style_str == "hollow")
    return 0;
  else if (fill_int_style_str == "solid")
    return 1;
  else if (fill_int_style_str == "pattern")
    return 2;
  else if (fill_int_style_str == "hatch")
    return 3;
  else if (fill_int_style_str == "solid_with_border")
    return 4;
  else
    {
      logger((stderr, "Got unknown fill_int_style \"%s\"\n", fill_int_style_str.c_str()));
      throw std::logic_error("The given fill_int_style is unknown.\n");
    }
}

std::string GRM::fillIntStyleIntToString(int fill_int_style)
{
  if (fill_int_style == 0)
    return "hollow";
  else if (fill_int_style == 1)
    return "solid";
  else if (fill_int_style == 2)
    return "pattern";
  else if (fill_int_style == 3)
    return "hatch";
  else if (fill_int_style == 4)
    return "solid_with_border";
  else
    {
      logger((stderr, "Got unknown fill_int_style \"%i\"\n", fill_int_style));
      throw std::logic_error("The given fill_int_style is unknown.\n");
    }
}

int GRM::transformationStringToInt(const std::string &transformation_str)
{
  if (transformation_string_to_int.count(transformation_str)) return transformation_string_to_int[transformation_str];
  logger((stderr, "Got unknown transformation \"%s\"\n", transformation_str.c_str()));
  throw std::logic_error("Given transformation is unknown.\n");
}

std::string GRM::transformationIntToString(int transformation)
{
  for (auto const &map_elem : transformation_string_to_int)
    {
      if (map_elem.second == transformation) return map_elem.first;
    }
  logger((stderr, "Got unknown transformation \"%i\"\n", transformation));
  throw std::logic_error("Given transformation is unknown.\n");
}

int GRM::labelOrientationStringToInt(const std::string &label_orientation_str)
{
  if (label_orientation_str == "up")
    return 1;
  else if (label_orientation_str == "down")
    return -1;
  logger((stderr, "Got unknown label orientation \"%s\"\n", label_orientation_str.c_str()));
  throw std::logic_error("The given label orientation is unknown.\n");
}

std::string GRM::labelOrientationIntToString(int label_orientation)
{
  if (label_orientation > 0)
    return "up";
  else if (label_orientation < 0)
    return "down";
  logger((stderr, "Got unknown label orientation \"%i\"\n", label_orientation));
  throw std::logic_error("The given label orientation is unknown.\n");
}

int GRM::worldCoordinatesStringToInt(const std::string &world_coordinates_str)
{
  if (world_coordinates_str == "ndc")
    return 1;
  else if (world_coordinates_str == "wc")
    return 0;
  logger((stderr, "Got unknown world coordinates \"%s\"\n", world_coordinates_str.c_str()));
  throw std::logic_error("Given world coordinates is unknown.\n");
}

std::string GRM::worldCoordinatesIntToString(int world_coordinates)
{
  if (world_coordinates == 1)
    return "ndc";
  else if (world_coordinates == 0)
    return "wc";
  logger((stderr, "Got unknown world coordinates \"%i\"\n", world_coordinates));
  throw std::logic_error("Given world coordinates is unknown.\n");
}

int GRM::colormapStringToInt(const std::string &colormap_str)
{
  if (colormap_string_to_int.count(colormap_str)) return colormap_string_to_int[colormap_str];
  logger((stderr, "Got unknown colormap \"%s\"\n", colormap_str.c_str()));
  throw std::logic_error("Given colormap is unknown.\n");
}

std::string GRM::colormapIntToString(int colormap)
{
  for (auto const &map_elem : colormap_string_to_int)
    {
      if (map_elem.second == colormap) return map_elem.first;
    }
  logger((stderr, "Got unknown colormap \"%i\"\n", colormap));
  throw std::logic_error("The given colormap is unknown.\n");
}

int GRM::fontStringToInt(const std::string &font_str)
{
  if (font_string_to_int.count(font_str)) return font_string_to_int[font_str];
  logger((stderr, "Got unknown font \"%s\"\n", font_str.c_str()));
  throw std::logic_error("Given font is unknown.\n");
}

std::string GRM::fontIntToString(int font)
{
  for (auto const &map_elem : font_string_to_int)
    {
      if (map_elem.second == font) return map_elem.first;
    }
  logger((stderr, "Got unknown font \"%i\"\n", font));
  throw std::logic_error("The given font is unknown.\n");
}

int GRM::fontPrecisionStringToInt(const std::string &font_precision_str)
{
  if (font_precision_string_to_int.count(font_precision_str)) return font_precision_string_to_int[font_precision_str];
  logger((stderr, "Got unknown font_precision \"%s\"\n", font_precision_str.c_str()));
  throw std::logic_error("Given font_precision is unknown.\n");
}

std::string GRM::fontPrecisionIntToString(int font_precision)
{
  for (auto const &map_elem : font_precision_string_to_int)
    {
      if (map_elem.second == font_precision) return map_elem.first;
    }
  logger((stderr, "Got unknown font precision \"%i\"\n", font_precision));
  throw std::logic_error("The given font precision is unknown.\n");
}

int GRM::lineTypeStringToInt(const std::string &line_type_str)
{
  if (line_type_string_to_int.count(line_type_str)) return line_type_string_to_int[line_type_str];
  logger((stderr, "Got unknown line_type \"%s\"\n", line_type_str.c_str()));
  throw std::logic_error("Given line_type is unknown.\n");
}

std::string GRM::lineTypeIntToString(int line_type)
{
  for (auto const &map_elem : line_type_string_to_int)
    {
      if (map_elem.second == line_type) return map_elem.first;
    }
  logger((stderr, "Got unknown line type \"%i\"\n", line_type));
  throw std::logic_error("The given line type is unknown.\n");
}

int GRM::markerTypeStringToInt(const std::string &marker_type_str)
{
  if (marker_type_string_to_int.count(marker_type_str)) return marker_type_string_to_int[marker_type_str];
  logger((stderr, "Got unknown marker_type \"%s\"\n", marker_type_str.c_str()));
  throw std::logic_error("Given marker_type is unknown.\n");
}

std::string GRM::markerTypeIntToString(int marker_type)
{
  for (auto const &map_elem : marker_type_string_to_int)
    {
      if (map_elem.second == marker_type) return map_elem.first;
    }
  logger((stderr, "Got unknown marker type \"%i\"\n", marker_type));
  throw std::logic_error("The given marker type is unknown.\n");
}

int GRM::locationStringToInt(const std::string &location_str)
{
  if (location_string_to_int.count(location_str)) return location_string_to_int[location_str];
  logger((stderr, "Got unknown location \"%s\"\n", location_str.c_str()));
  throw std::logic_error("Given location is unknown.\n");
}

std::string GRM::locationIntToString(int location)
{
  for (auto const &map_elem : location_string_to_int)
    {
      if (map_elem.second == location) return map_elem.first;
    }
  logger((stderr, "Got unknown location \"%i\"\n", location));
  throw std::logic_error("The given location is unknown.\n");
}

int GRM::xAxisLocationStringToInt(const std::string &location_str)
{
  if (x_axis_location_string_to_int.count(location_str)) return x_axis_location_string_to_int[location_str];
  logger((stderr, "Got unknown location \"%s\"\n", location_str.c_str()));
  throw std::logic_error("Given location is unknown.\n");
}

std::string GRM::xAxisLocationIntToString(int location)
{
  for (auto const &map_elem : x_axis_location_string_to_int)
    {
      if (map_elem.second == location) return map_elem.first;
    }
  logger((stderr, "Got unknown location \"%i\"\n", location));
  throw std::logic_error("The given location is unknown.\n");
}

int GRM::yAxisLocationStringToInt(const std::string &location_str)
{
  if (y_axis_location_string_to_int.count(location_str)) return y_axis_location_string_to_int[location_str];
  logger((stderr, "Got unknown location \"%s\"\n", location_str.c_str()));
  throw std::logic_error("Given location is unknown.\n");
}

std::string GRM::yAxisLocationIntToString(int location)
{
  for (auto const &map_elem : y_axis_location_string_to_int)
    {
      if (map_elem.second == location) return map_elem.first;
    }
  logger((stderr, "Got unknown location \"%i\"\n", location));
  throw std::logic_error("The given location is unknown.\n");
}

int GRM::colorModelStringToInt(const std::string &color_model_str)
{
  if (color_model_string_to_int.count(color_model_str)) return color_model_string_to_int[color_model_str];
  logger((stderr, "Got unknown color model \"%s\"\n", color_model_str.c_str()));
  throw std::logic_error("Given color model is unknown.\n");
}

std::string GRM::colorModelIntToString(int color_model)
{
  for (auto const &map_elem : color_model_string_to_int)
    {
      if (map_elem.second == color_model) return map_elem.first;
    }
  logger((stderr, "Got unknown color model \"%i\"\n", color_model));
  throw std::logic_error("The given color model is unknown.\n");
}

int GRM::textAlignHorizontalStringToInt(const std::string &text_align_horizontal_str)
{
  if (text_align_horizontal_string_to_int.count(text_align_horizontal_str))
    return text_align_horizontal_string_to_int[text_align_horizontal_str];
  logger((stderr, "Got unknown text_align_horizontal \"%s\"\n", text_align_horizontal_str.c_str()));
  throw std::logic_error("Given text_align_horizontal is unknown.\n");
}

std::string GRM::textAlignHorizontalIntToString(int text_align_horizontal)
{
  for (auto const &map_elem : text_align_horizontal_string_to_int)
    {
      if (map_elem.second == text_align_horizontal) return map_elem.first;
    }
  logger((stderr, "Got unknown horizontal text aligment \"%i\"\n", text_align_horizontal));
  throw std::logic_error("The given horizontal text aligment is unknown.\n");
}

int GRM::textAlignVerticalStringToInt(const std::string &text_align_vertical_str)
{
  if (text_align_vertical_string_to_int.count(text_align_vertical_str))
    return text_align_vertical_string_to_int[text_align_vertical_str];
  logger((stderr, "Got unknown text_align_vertical \"%s\"\n", text_align_vertical_str.c_str()));
  throw std::logic_error("Given text_align_vertical is unknown.\n");
}

std::string GRM::textAlignVerticalIntToString(int text_align_vertical)
{
  for (auto const &map_elem : text_align_vertical_string_to_int)
    {
      if (map_elem.second == text_align_vertical) return map_elem.first;
    }
  logger((stderr, "Got unknown vertical text aligment \"%i\"\n", text_align_vertical));
  throw std::logic_error("The given vertical text aligment is unknown.\n");
}

int GRM::textEncodingStringToInt(const std::string &text_encoding_str)
{
  if (text_encoding_str == "latin1")
    return 300;
  else if (text_encoding_str == "utf8")
    return 301;
  logger((stderr, "Got unknown text encoding \"%s\"\n", text_encoding_str.c_str()));
  throw std::logic_error("The given text encoding is unknown.\n");
}

std::string GRM::textEncodingIntToString(int text_encoding)
{
  if (text_encoding == 300)
    return "latin1";
  else if (text_encoding == 301)
    return "utf8";
  logger((stderr, "Got unknown text encoding \"%i\"\n", text_encoding));
  throw std::logic_error("The given text encoding is unknown.\n");
}

int GRM::tickOrientationStringToInt(const std::string &tick_orientation_str)
{
  if (tick_orientation_str == "up")
    return 1;
  else if (tick_orientation_str == "down")
    return -1;
  logger((stderr, "Got unknown tick orientation \"%s\"\n", tick_orientation_str.c_str()));
  throw std::logic_error("The given tick orientation is unknown.\n");
}

std::string GRM::tickOrientationIntToString(int tick_orientation)
{
  if (tick_orientation > 0)
    return "up";
  else if (tick_orientation < 0)
    return "down";
  logger((stderr, "Got unknown tick orientation \"%i\"\n", tick_orientation));
  throw std::logic_error("The given tick orientation is unknown.\n");
}

/* ------------------------------- get functions ---------------------------------------------------------------------*/

std::vector<std::string> GRM::getSizeUnits()
{
  std::vector<std::string> size_units;
  size_units.reserve(symbol_to_meters_per_unit.size());
  for (auto const &imap : symbol_to_meters_per_unit) size_units.push_back(imap.first);
  return size_units;
}

std::vector<std::string> GRM::getColormaps()
{
  std::vector<std::string> colormaps;
  colormaps.reserve(colormap_string_to_int.size());
  for (auto const &imap : colormap_string_to_int)
    {
      if (imap.first != "default") colormaps.push_back(imap.first);
    }
  return colormaps;
}

std::vector<std::string> GRM::getFonts()
{
  std::vector<std::string> fonts;
  fonts.reserve(font_string_to_int.size());
  for (auto const &imap : font_string_to_int) fonts.push_back(imap.first);
  return fonts;
}

std::vector<std::string> GRM::getFontPrecisions()
{
  std::vector<std::string> font_precisions;
  font_precisions.reserve(font_precision_string_to_int.size());
  for (auto const &imap : font_precision_string_to_int) font_precisions.push_back(imap.first);
  return font_precisions;
}

std::vector<std::string> GRM::getLineTypes()
{
  std::vector<std::string> line_types;
  line_types.reserve(line_type_string_to_int.size());
  for (auto const &imap : line_type_string_to_int) line_types.push_back(imap.first);
  return line_types;
}

std::vector<std::string> GRM::getLocations()
{
  std::vector<std::string> locations;
  locations.reserve(location_string_to_int.size());
  for (auto const &imap : location_string_to_int) locations.push_back(imap.first);
  return locations;
}

std::vector<std::string> GRM::getXAxisLocations()
{
  std::vector<std::string> locations;
  locations.reserve(x_axis_location_string_to_int.size());
  for (auto const &imap : x_axis_location_string_to_int) locations.push_back(imap.first);
  return locations;
}

std::vector<std::string> GRM::getYAxisLocations()
{
  std::vector<std::string> locations;
  locations.reserve(y_axis_location_string_to_int.size());
  for (auto const &imap : y_axis_location_string_to_int) locations.push_back(imap.first);
  return locations;
}

std::vector<std::string> GRM::getMarkerTypes()
{
  std::vector<std::string> marker_types;
  marker_types.reserve(marker_type_string_to_int.size());
  for (auto const &imap : marker_type_string_to_int) marker_types.push_back(imap.first);
  return marker_types;
}

std::vector<std::string> GRM::getTextAlignHorizontal()
{
  std::vector<std::string> text_align_horizontal;
  text_align_horizontal.reserve(text_align_horizontal_string_to_int.size());
  for (auto const &imap : text_align_horizontal_string_to_int) text_align_horizontal.push_back(imap.first);
  return text_align_horizontal;
}

std::vector<std::string> GRM::getTextAlignVertical()
{
  std::vector<std::string> text_align_vertical;
  text_align_vertical.reserve(text_align_vertical_string_to_int.size());
  for (auto const &imap : text_align_vertical_string_to_int) text_align_vertical.push_back(imap.first);
  return text_align_vertical;
}

std::vector<std::string> GRM::getAlgorithm()
{
  std::vector<std::string> algorithm;
  algorithm.reserve(algorithm_string_to_int.size());
  for (auto const &imap : algorithm_string_to_int) algorithm.push_back(imap.first);
  return algorithm;
}

std::vector<std::string> GRM::getColorModel()
{
  std::vector<std::string> model;
  model.reserve(color_model_string_to_int.size());
  for (auto const &imap : color_model_string_to_int) model.push_back(imap.first);
  return model;
}

std::vector<std::string> GRM::getContextAttributes()
{
  std::vector<std::string> attributes;
  attributes.reserve(valid_context_attributes.size());
  for (auto const &attr : valid_context_attributes) attributes.push_back(attr);
  return attributes;
}

std::vector<std::string> GRM::getFillStyles()
{
  std::vector<std::string> fill_styles;
  fill_styles.reserve(fill_style_string_to_int.size());
  for (auto const &imap : fill_style_string_to_int) fill_styles.push_back(imap.first);
  return fill_styles;
}

std::vector<std::string> GRM::getFillIntStyles()
{
  std::vector<std::string> fill_styles = {"hollow", "solid", "pattern", "hatch", "solid_with_border"};
  return fill_styles;
}

std::vector<std::string> GRM::getTransformation()
{
  std::vector<std::string> transformations;
  transformations.reserve(transformation_string_to_int.size());
  for (auto const &imap : transformation_string_to_int) transformations.push_back(imap.first);
  return transformations;
}
