/* ######################### includes ############################################################################### */

#include <cstring>
#include <algorithm>
#include <map>
#include <sstream>
#include <vector>
#include <time.h>


#include "utilcpp_int.hxx"
#include "import_utils_int.hxx"
#include "grm/dom_render/graphics_tree/util.hxx"

/* ========================= static variables ======================================================================= */

/* ------------------------- key to types --------------------------------------------------------------------------- */

static std::map<std::string, const char *> key_to_types{
    {"algorithm", "s"},
    {"bar_color", "ddd"},
    {"bar_color", "i"},
    {"bar_width", "d"},
    {"bin_counts", "i"},
    {"bin_edges", "nD"},
    {"bin_width", "d"},
    {"c", "nD"},
    {"cell", "s"},
    {"clip_negative", "i"},
    {"colormap", "i"},
    {"connection_threshold", "d"},
    {"draw_edges", "i"},
    {"edge_color", "ddd"},
    {"edge_color", "i"},
    {"edge_width", "d"},
    {"equal_up_and_down_error", "i"},
    {"error", "a"},
    {"error_bar_style", "i"},
    {"error_type", "s"},
    {"ignore_blank_lines", "i"},
    {"int_limits_high", "nD"},
    {"int_limits_low", "nD"},
    {"isovalue", "d"},
    {"keep_aspect_ratio", "i"},
    {"keep_radii_axes", "i"},
    {"kind", "nS"},
    {"legend", "nS"},
    {"legend_line", "i"},
    {"levels", "i"},
    {"line_spec", "s"},
    {"location", "i"},
    {"major_h", "i"},
    {"marginal_heatmap_kind", "s"},
    {"marker_size", "d"},
    {"marker_type", "i"},
    {"molecule_file", "i"},
    {"num_bins", "i"},
    {"normalization", "s"},
    {"only_square_aspect_ratio", "i"},
    {"orientation", "s"},
    {"resample_method", "s"},
    {"r_lim", "dd"},
    {"r_log", "i"},
    {"r_range", "dd"},
    {"rotation", "d"},
    {"scale", "i"},
    {"scatter_z", "i"},
    {"spin_style", "i"},
    {"stairs", "i"},
    {"step_where", "s"},
    {"style", "s"},
    {"theta_flip", "i"},
    {"theta_lim", "dd"},
    {"theta_log", "i"},
    {"theta_range", "dd"},
    {"tilt", "d"},
    {"title", "s"},
    {"transformation", "i"},
    {"use_bins", "i"},
    {"use_gr3", "i"},
    {"use_grplot_changes", "i"},
    {"x_bins", "i"},
    {"x_flip", "i"},
    {"x_grid", "i"},
    {"x_label", "s"},
    {"x_lim", "dd"},
    {"x_log", "i"},
    {"x_range", "dd"},
    {"xye_file", "i"},
    {"xyz_file", "i"},
    {"y_bins", "i"},
    {"y_flip", "i"},
    {"y_grid", "i"},
    {"y_label", "s"},
    {"y_labels", "nS"},
    {"y_lim", "dd"},
    {"y_log", "i"},
    {"y_range", "dd"},
    {"z_grid", "i"},
    {"z_label", "s"},
    {"z_lim", "dd"},
    {"z_log", "i"},
    {"z_range", "dd"},
};

/* ------------------------- kind types ----------------------------------------------------------------------------- */

static std::list<std::string> kind_types = {"barplot",       "contour",    "contourf",      "heatmap",
                                            "hexbin",        "hist",       "histogram",     "imshow",
                                            "isosurface",    "line",       "line3",         "marginal_heatmap",
                                            "molecule",      "polar_line", "polar_heatmap", "polar_histogram",
                                            "polar_scatter", "pie",        "plot3",         "scatter",
                                            "scatter3",      "shade",      "surface",       "stem",
                                            "stairs",        "tricontour", "trisurface",    "quiver",
                                            "volume",        "wireframe"};

/* ------------------------- alias for keys ------------------------------------------------------------------------- */

static std::map<std::string, std::string> key_alias = {
    {"h_kind", "marginal_heatmap_kind"}, {"aspect", "keep_aspect_ratio"}, {"cmap", "colormap"}};

/* ------------------------- container parameter -------------------------------------------------------------------- */

static std::map<std::string, const char *> container_params{
    {"error", "a"},
};

static std::map<std::string, const char *> container_to_types{
    {"downwards_cap_color", "i"},
    {"error_bar_color", "i"},
    {"upwards_cap_color", "i"},
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ input stream parser ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

bool isValidKind(const std::string &kind)
{
  return std::find(kind_types.begin(), kind_types.end(), kind) != kind_types.end();
}

grm_error_t parseColumns(std::list<int> *columns, const char *colms)
{
  std::string token;
  std::stringstream scol(colms);
  while (std::getline(scol, token, ',') && token.length())
    {
      if (token.find(':') != std::string::npos)
        {
          std::stringstream stok(token);
          int start = 0, end = 0;
          if (startsWith(token, ":"))
            {
              try
                {
                  end = std::stoi(token.erase(0, 1));
                }
              catch (std::invalid_argument &e)
                {
                  fprintf(stderr, "Invalid argument for column parameter (%s)\n", token.c_str());
                  return GRM_ERROR_PARSE_INT;
                }
            }
          else
            {
              for (size_t coli = 0; std::getline(stok, token, ':') && token.length(); coli++)
                {
                  try
                    {
                      if (coli == 0)
                        {
                          start = std::stoi(token);
                        }
                      else
                        {
                          end = std::stoi(token);
                        }
                    }
                  catch (std::invalid_argument &e)
                    {
                      fprintf(stderr, "Invalid argument for column parameter (%s)\n", token.c_str());
                      return GRM_ERROR_PARSE_INT;
                    }
                }
            }
          for (int num = start; num <= end; num++)
            {
              (*columns).push_back(num);
            }
        }
      else
        {
          try
            {
              (*columns).push_back(std::stoi(token));
            }
          catch (std::invalid_argument &e)
            {
              fprintf(stderr, "Invalid argument for column parameter (%s)\n", token.c_str());
              return GRM_ERROR_PARSE_INT;
            }
        }
    }
  if (!(*columns).empty()) (*columns).sort();
  return GRM_ERROR_NONE;
}

void parseParameterDD(std::string *input, const std::string *key, std::string *x, std::string *y)
{
  size_t con_pos = 0;
  int k = 0;
  while ((con_pos = (*input).find(',')) != std::string::npos)
    {
      if (k == 0) *x = (*input).substr(0, con_pos);
      (*input).erase(0, con_pos + 1);
      k++;
    }
  if (k != 1 || (*input).length() == 0)
    {
      fprintf(stderr,
              "Given number doesn't fit the data for %s parameter. The "
              "parameter will be "
              "ignored\n",
              (*key).c_str());
    }
  *y = *input;
}

void parseParameterDDD(std::string *input, const std::string *key, std::string *r, std::string *g, std::string *b)
{
  size_t con_pos = 0;
  int k = 0;
  while ((con_pos = (*input).find(',')) != std::string::npos)
    {
      if (k == 0) *r = (*input).substr(0, con_pos);
      if (k == 1) *g = (*input).substr(0, con_pos);
      (*input).erase(0, con_pos + 1);
      k++;
    }
  if (k != 2 || (*input).length() == 0)
    {
      fprintf(stderr,
              "Given number doesn't fit the data for %s parameter. The "
              "parameter will be "
              "ignored\n",
              (*key).c_str());
    }
  *b = *input;
}

int parseParameterNI(std::string *input, const std::string *key, std::vector<int> *values)
{
  size_t con_pos = (*input).find(',');
  int k = 0;
  std::string param_num = (*input).substr(0, con_pos);
  (*input).erase(0, con_pos + 1);
  while ((con_pos = (*input).find(',')) != std::string::npos)
    {
      (*values)[k] = std::stoi((*input).substr(0, con_pos));
      (*input).erase(0, con_pos + 1);
      k++;
    }
  (*values)[k] = std::stoi((*input));
  if (k != std::stoi(param_num) - 1 || (*input).length() == 0)
    {
      fprintf(stderr,
              "Given number doesn't fit the data for %s parameter. The "
              "parameter will be "
              "ignored\n",
              (*key).c_str());
      return 0;
    }
  return 1;
}

int parseParameterNS(std::string *input, const std::string *key, std::vector<std::string> *values)
{
  size_t pos = (*input).find(',');
  int k = 0;
  std::string num = (*input).substr(0, pos);
  (*input).erase(0, pos + 1);
  while ((pos = (*input).find(',')) != std::string::npos)
    {
      (*values)[k] = (*input).substr(0, pos);
      (*input).erase(0, pos + 1);
      k++;
    }
  (*values)[k] = (*input);
  if (k != std::stoi(num) - 1 || (*input).length() == 0)
    {
      fprintf(stderr,
              "Given number doesn't fit the data for %s parameter. The parameter will be "
              "ignored\n",
              (*key).c_str());
      return 0;
    }
  return 1;
}

int parseParameterND(std::string *input, const std::string *key, std::vector<double> *values)
{
  size_t pos = (*input).find(',');
  int k = 0;
  std::string num = (*input).substr(0, pos);
  (*input).erase(0, pos + 1);
  while ((pos = (*input).find(',')) != std::string::npos)
    {
      (*values)[k] = std::stod((*input).substr(0, pos));
      (*input).erase(0, pos + 1);
      k++;
    }
  (*values)[k] = std::stod((*input));
  if (k != std::stoi(num) - 1 || (*input).length() == 0)
    {
      fprintf(stderr,
              "Given number doesn't fit the data for %s parameter. The parameter will be "
              "ignored\n",
              (*key).c_str());
      return 0;
    }
  return 1;
}

static std::string normalizeLine(const std::string &str)
{
  std::string s, item;
  std::istringstream ss(str);

  s = "";
  while (ss >> item)
    {
      if (item[0] == '#') break;
      if (!s.empty()) s += '\t';
      s += item;
    }
  return s;
}

static bool kindOfSameGroup(std::string new_kind, std::vector<std::string> kinds)
{
  std::vector<std::string> line_group = {"line", "scatter"};
  std::vector<std::string> barplot_group = {"barplot", "stem", "stairs"};
  std::vector<std::string> polar_line_group = {"polar_line", "polar_scatter"};

  // the other kind groups doesn't support series in grplot so they can be ignored for now
  if (kinds[kinds.size() - 1] == new_kind) return true;
  if (std::find(line_group.begin(), line_group.end(), new_kind) != line_group.end() &&
      std::find(line_group.begin(), line_group.end(), kinds[kinds.size() - 1]) != line_group.end())
    return true;
  if (std::find(barplot_group.begin(), barplot_group.end(), new_kind) != barplot_group.end() &&
      std::find(barplot_group.begin(), barplot_group.end(), kinds[kinds.size() - 1]) != barplot_group.end())
    return true;
  if (std::find(polar_line_group.begin(), polar_line_group.end(), new_kind) != polar_line_group.end() &&
      std::find(polar_line_group.begin(), polar_line_group.end(), kinds[kinds.size() - 1]) != polar_line_group.end())
    return true;
  return false;
}

std::vector<std::string> singleTokenConverter(std::string token, grm_args_t *args, PlotRange *ranges,
                                              grm_special_axis_series_t *special_axis_series, InputFlags &input_flags,
                                              int line_count)
{
  size_t found_key_size;
  std::string delim = ":", found_key;
  std::vector<std::string> kinds;
  size_t pos = token.find(delim);

  if (startsWith(token, "bottom:"))
    {
      if (special_axis_series->bottom.empty()) special_axis_series->bottom = token.substr(7, token.length() - 1);
    }
  else if (startsWith(token, "left:"))
    {
      if (special_axis_series->left.empty()) special_axis_series->left = token.substr(5, token.length() - 1);
    }
  else if (startsWith(token, "right:"))
    {
      if (special_axis_series->right.empty()) special_axis_series->right = token.substr(6, token.length() - 1);
    }
  else if (startsWith(token, "top:"))
    {
      if (special_axis_series->top.empty()) special_axis_series->top = token.substr(4, token.length() - 1);
    }
  else if (startsWith(token, "twin_x:"))
    {
      if (special_axis_series->twin_x.empty()) special_axis_series->twin_x = token.substr(7, token.length() - 1);
    }
  else if (startsWith(token, "twin_y:"))
    {
      if (special_axis_series->twin_y.empty()) special_axis_series->twin_y = token.substr(7, token.length() - 1);
    }
  else if (pos != std::string::npos)
    {
      found_key = token.substr(0, pos);
      found_key_size = found_key.size();
      /* replace the key if a known alias exists */
      if (auto search_alias = key_alias.find(found_key); search_alias != key_alias.end())
        {
          found_key = search_alias->second;
        }
      if (auto search = key_to_types.find(found_key); search != key_to_types.end())
        {
          std::string value = token.substr(found_key_size + 1, token.length() - 1);
          /* special case for kind, for following exception */
          if (strcmp(found_key.c_str(), "kind") == 0)
            {
              auto kind_string = token.substr(found_key_size + 1, token.length() - 1);
              std::string kind_token;
              std::istringstream kind_ss(normalizeLine(kind_string));

              for (size_t col = 0; std::getline(kind_ss, kind_token, ','); col++)
                {
                  if (kind_token == "hist")
                    kind_token = "histogram";
                  else if (kind_token == "plot3")
                    kind_token = "line3";
                  if (std::find(kind_types.begin(), kind_types.end(), kind_token) != kind_types.end())
                    {
                      // only allow different kinds of the same group for know since it result in less changes
                      if (kinds.empty() || kindOfSameGroup(kind_token, kinds))
                        {
                          kinds.push_back(kind_token);
                        }
                      else
                        {
                          fprintf(stderr,
                                  "Kind %s can't be combined with the previous types in the current GRPlot state\n",
                                  kind_token.c_str());
                        }
                    }
                  else if (kind_token.size() == 0)
                    {
                      if (kinds.size() > 0)
                        {
                          kinds.push_back(kinds[kinds.size() - 1]);
                        }
                      else
                        {
                          fprintf(stderr, "If multiple kinds should be used start with a valid kind\n");
                        }
                    }
                  else
                    {
                      fprintf(stderr, "Invalid plot kind (%s) -- ignore this value\n", kind_token.c_str());
                    }
                }
              return kinds;
            }

          if (value.length() == 0)
            {
              fprintf(stderr, "Parameter %s will be ignored. No data given\n", search->first.c_str());
              return kinds;
            }

          /* parameter is a container */
          if (auto container_search = container_params.find(found_key); container_search != container_params.end())
            {
              int num = 1;
              int num_of_parameter = 0;
              size_t pos_a, pos_b;
              int container_arr;
              int ind = 0;

              if ((container_arr = (pos_a = value.find(',')) < (pos_b = value.find('{'))) &&
                  strcmp(container_search->second, "nA") == 0)
                {
                  num = stoi(value.substr(0, pos_a));
                  value.erase(0, pos_a + 1);
                }
              else
                {
                  container_search->second = "a";
                }
              std::vector<grm_args_t *> new_args(num);
              for (num_of_parameter = 0; num_of_parameter < num; num_of_parameter++)
                {
                  new_args[num_of_parameter] = grm_args_new();
                }
              num_of_parameter = 0;

              /* fill the container */
              size_t pos_begin = value.find('{');
              while ((pos = value.find('}')) != std::string::npos)
                {
                  std::string arg = value.substr(pos_begin + 1, pos - 1);
                  if (startsWith(arg, "{"))
                    {
                      arg = arg.substr(1, arg.length());
                    }
                  else if (endsWith(arg, "}"))
                    {
                      arg = arg.substr(0, arg.length() - 1);
                    }
                  size_t key_pos = arg.find(delim);
                  if (key_pos != std::string::npos)
                    {
                      found_key = arg.substr(0, key_pos);
                      found_key_size = found_key.size();
                      if (auto con = container_to_types.find(found_key); con != container_to_types.end())
                        {
                          std::string container_value = arg.substr(found_key_size + 1, arg.length() - 1);

                          /* sometimes a parameter can be given by different types, the if makes sure the
                           * correct one is used */
                          if ((pos_a = value.find(',')) < (pos_b = value.find('}')) &&
                              (strEqualsAny(con->second, "i", "d")))
                            {
                              if (con.operator++()->second != NULL) con = con.operator++();
                            }
                          try
                            {
                              if (strcmp(con->second, "d") == 0)
                                {
                                  grm_args_push(new_args[ind], con->first.c_str(), con->second,
                                                std::stod(container_value));
                                }
                              else if (strcmp(con->second, "i") == 0)
                                {
                                  grm_args_push(new_args[ind], con->first.c_str(), con->second,
                                                std::stoi(container_value));
                                }
                              else if (strcmp(search->second, "dd") == 0)
                                {
                                  // not 100% correct since the dataset could be normal but it`s a rare case that u use
                                  // a timestamp as limit or range on a non timestamp dataset so it`s ignored for now
                                  std::string x, y;
                                  parseParameterDD(&value, &search->first, &x, &y);

                                  struct tm timestamp_x_tm, timestamp_y_tm;
                                  if ((search->first == "x_range" || search->first == "x_lim") &&
                                      parseIso8601WithoutTimezone(x, timestamp_x_tm) &&
                                      parseIso8601WithoutTimezone(y, timestamp_y_tm))
                                    grm_args_push(args, search->first.c_str(), search->second,
                                                  (int)mktime(&timestamp_x_tm), (int)mktime(&timestamp_y_tm));
                                  else
                                    grm_args_push(args, search->first.c_str(), search->second, std::stod(x),
                                                  std::stod(y));
                                  if (search->first == "x_range")
                                    {
                                      if (parseIso8601WithoutTimezone(x, timestamp_x_tm))
                                        ranges->xmin = (int)mktime(&timestamp_x_tm);
                                      else
                                        ranges->xmin = std::stod(x);
                                      if (parseIso8601WithoutTimezone(y, timestamp_y_tm))
                                        ranges->xmax = (int)mktime(&timestamp_y_tm);
                                      else
                                        ranges->xmax = std::stod(y);
                                    }
                                  else if (search->first == "y_range")
                                    {
                                      ranges->ymin = std::stod(x);
                                      ranges->ymax = std::stod(y);
                                    }
                                  else if (search->first == "z_range")
                                    {
                                      ranges->zmin = std::stod(x);
                                      ranges->zmax = std::stod(y);
                                    }
                                  else if (search->first == "theta_range")
                                    {
                                      ranges->xmin = std::stod(x);
                                      ranges->xmax = std::stod(y);
                                    }
                                  else if (search->first == "r_range")
                                    {
                                      ranges->ymin = std::stod(x);
                                      ranges->ymax = std::stod(y);
                                    }
                                }
                              else if (strcmp(con->second, "ddd") == 0)
                                {
                                  std::string r, g, b;
                                  parseParameterDDD(&container_value, &con->first, &r, &g, &b);
                                  grm_args_push(new_args[ind], con->first.c_str(), con->second, std::stod(r),
                                                std::stod(g), std::stod(b));
                                }
                              else if ((pos_a = value.find(',')) != std::string::npos && strcmp(con->second, "nI") == 0)
                                {
                                  size_t con_pos = container_value.find(',');
                                  std::string param_num = container_value.substr(0, con_pos);
                                  std::vector<int> values(std::stoi(param_num));
                                  int no_err = parseParameterNI(&container_value, &con->first, &values);
                                  if (no_err)
                                    {
                                      grm_args_push(new_args[ind], con->first.c_str(), con->second,
                                                    std::stoi(param_num), values.data());
                                    }
                                }
                              num_of_parameter += 1;
                              if (num_of_parameter % 2 == 0) ind += 1;
                            }
                          catch (std::invalid_argument &e)
                            {
                              fprintf(stderr, "Invalid argument for %s parameter (%s).\n", con->first.c_str(),
                                      container_value.c_str());
                            }
                        }
                    }
                  value.erase(0, pos + 2);
                }
              if (container_arr && strcmp(container_search->second, "nA") == 0 && num_of_parameter == num * 2)
                {
                  grm_args_t *tmp;
                  if (!grm_args_values(args, container_search->first.c_str(), container_search->second, &tmp))
                    grm_args_push(args, container_search->first.c_str(), container_search->second, num,
                                  new_args.data());
                }
              else if (num_of_parameter == 2 || strcmp(container_search->first.c_str(), "error") == 0)
                {
                  grm_args_t *tmp;
                  if (!grm_args_values(args, container_search->first.c_str(), container_search->second, &tmp))
                    grm_args_push(args, container_search->first.c_str(), container_search->second, new_args[0]);
                }
              else
                {
                  fprintf(stderr, "Not enough data for %s parameter\n", container_search->first.c_str());
                }
            }
          size_t pos_a;
          /* sometimes a parameter can be given by different types, the 'if' makes sure the correct one is used
           */
          if ((pos_a = value.find(',')) != std::string::npos && (strEqualsAny(search->second, "i", "d", "s")))
            {
              if (search.operator++()->second != NULL) search = search.operator++();
            }

          /* different types */
          if (strcmp(search->second, "s") == 0)
            {
              char *tmp;
              if (strcmp(search->first.c_str(), "error_type") == 0)
                {
                  if (value == "relative" || value == "absolute") input_flags.error_type = value;
                }
              else if (strcmp(search->first.c_str(), "cell") == 0)
                {
                  // special case for cell so it fits more the way it would be defined by the user
                  size_t value_pos;
                  std::vector<double> cell_vectors;
                  while ((value_pos = value.find(' ')) != std::string::npos)
                    {
                      cell_vectors.push_back(std::stod(value.substr(0, value_pos)));
                      value.erase(0, value_pos + 1);
                    }
                  cell_vectors.push_back(std::stod(value.substr(0, value.length())));
                  grm_args_push(args, "cell", "nD", cell_vectors.size(), cell_vectors.data());
                }
              else if (!grm_args_values(args, search->first.c_str(), search->second, &tmp))
                grm_args_push(args, search->first.c_str(), search->second, value.c_str());
            }
          else
            {
              try
                {
                  if (strcmp(search->second, "i") == 0)
                    {
                      /* special case for scatter plot, to decide how the read data gets interpreted */
                      if (strcmp(search->first.c_str(), "scatter_z") == 0)
                        {
                          input_flags.scatter_with_z = std::stoi(value);
                        }
                      else if (strcmp(search->first.c_str(), "use_bins") == 0)
                        {
                          input_flags.use_bins = std::stoi(value);
                          if (input_flags.use_bins) input_flags.xyz_file = 0;
                        }
                      else if (strcmp(search->first.c_str(), "equal_up_and_down_error") == 0)
                        {
                          input_flags.equal_up_and_down_error = std::stoi(value);
                        }
                      else if (strcmp(search->first.c_str(), "xye_file") == 0)
                        {
                          input_flags.xye_file = std::stoi(value);
                        }
                      else if (strcmp(search->first.c_str(), "xyz_file") == 0)
                        {
                          input_flags.xyz_file = std::stoi(value);
                          if (input_flags.xyz_file) input_flags.use_bins = 0;
                        }
                      else if (strcmp(search->first.c_str(), "ignore_blank_lines") == 0)
                        {
                          if (input_flags.ignore_blank_lines == 0 || input_flags.ignore_blank_lines == 1)
                            input_flags.ignore_blank_lines = std::stoi(value);
                        }
                      else if (strcmp(search->first.c_str(), "legend_line") == 0)
                        {
                          input_flags.force_legend_line = std::stoi(value);
                        }
                      else if (strcmp(search->first.c_str(), "molecule_file") == 0)
                        {
                          // needed since test.dat and a simple molecule dataset are the same parse wise
                          input_flags.xyz_molecule_file = std::stoi(value);
                        }
                      else
                        {
                          int tmp;
                          if (!grm_args_values(args, search->first.c_str(), search->second, &tmp))
                            grm_args_push(args, search->first.c_str(), search->second, std::stoi(value));
                        }
                    }
                  else if (strcmp(search->second, "d") == 0)
                    {
                      double tmp;
                      if (!grm_args_values(args, search->first.c_str(), search->second, &tmp))
                        grm_args_push(args, search->first.c_str(), search->second, std::stod(value));
                    }
                  else if (strcmp(search->second, "dd") == 0)
                    {
                      double tmp1, tmp2;
                      std::string x, y;
                      parseParameterDD(&value, &search->first, &x, &y);
                      if (!grm_args_values(args, search->first.c_str(), search->second, &tmp1, &tmp2))
                        {
                          // not 100% correct since the dataset could be normal but it`s a rare case that u use a
                          // timestamp as limit or range on a non timestamp dataset so it`s ignored for now
                          struct tm timestamp_x_tm, timestamp_y_tm;
                          if ((search->first == "x_range" || search->first == "x_lim") &&
                              parseIso8601WithoutTimezone(x, timestamp_x_tm) &&
                              parseIso8601WithoutTimezone(y, timestamp_y_tm))
                            grm_args_push(args, search->first.c_str(), search->second, (int)mktime(&timestamp_x_tm),
                                          (int)mktime(&timestamp_y_tm));
                          else
                            grm_args_push(args, search->first.c_str(), search->second, std::stod(x), std::stod(y));
                          if (search->first == "x_range")
                            {
                              if (parseIso8601WithoutTimezone(x, timestamp_x_tm))
                                ranges->xmin = (int)mktime(&timestamp_x_tm);
                              else
                                ranges->xmin = std::stod(x);
                              if (parseIso8601WithoutTimezone(y, timestamp_y_tm))
                                ranges->xmax = (int)mktime(&timestamp_y_tm);
                              else
                                ranges->xmax = std::stod(y);
                            }
                          else if (search->first == "y_range")
                            {
                              ranges->ymin = std::stod(x);
                              ranges->ymax = std::stod(y);
                            }
                          else if (search->first == "z_range")
                            {
                              ranges->zmin = std::stod(x);
                              ranges->zmax = std::stod(y);
                            }
                          else if (search->first == "theta_range")
                            {
                              ranges->xmin = std::stod(x);
                              ranges->xmax = std::stod(y);
                            }
                          else if (search->first == "r_range")
                            {
                              ranges->ymin = std::stod(x);
                              ranges->ymax = std::stod(y);
                            }
                        }
                    }
                  else if (strcmp(search->second, "ddd") == 0)
                    {
                      double tmp1, tmp2, tmp3;
                      std::string r, g, b;
                      parseParameterDDD(&value, &search->first, &r, &g, &b);
                      if (!grm_args_values(args, search->first.c_str(), search->second, &tmp1, &tmp2, &tmp3))
                        grm_args_push(args, search->first.c_str(), search->second, std::stod(r), std::stod(g),
                                      std::stod(b));
                    }
                  else if (strcmp(search->second, "nS") == 0)
                    {
                      size_t pos = value.find(',');
                      std::string num = value.substr(0, pos);
                      std::vector<std::string> values(std::stoi(num));
                      std::vector<const char *> c_values(std::stoi(num));
                      int no_err = parseParameterNS(&value, &search->first, &values);
                      if (no_err)
                        {
                          grm_args_t *tmp;
                          int ci;
                          for (ci = 0; ci < std::stoi(num); ci++)
                            {
                              c_values[ci] = values[ci].c_str();
                            }
                          if (!grm_args_values(args, search->first.c_str(), search->second, &tmp))
                            grm_args_push(args, search->first.c_str(), search->second, std::stoi(num), c_values.data());
                        }
                    }
                  else if (strcmp(search->second, "nD") == 0)
                    {
                      grm_args_t *tmp;
                      size_t pos = value.find(',');
                      std::string num = value.substr(0, pos);
                      std::vector<double> values(std::stoi(num));
                      int no_err = parseParameterND(&value, &search->first, &values);
                      if (no_err)
                        {
                          if (!grm_args_values(args, search->first.c_str(), search->second, &tmp))
                            grm_args_push(args, search->first.c_str(), search->second, std::stoi(num), values.data());
                        }
                    }
                }
              catch (std::invalid_argument &e)
                {
                  fprintf(stderr, "Invalid argument for %s parameter (%s).\n", search->first.c_str(), value.c_str());
                }
            }
        }
      else
        {
          if (line_count != -1)
            fprintf(stderr, "Unknown key:value pair (%s) in line %i\n", token.c_str(), line_count);
          else
            fprintf(stderr, "Unknown key:value pair in parameters (%s)\n", token.c_str());
        }
    }
  return kinds;
}
