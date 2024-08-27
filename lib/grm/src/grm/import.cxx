/* ######################### includes ############################################################################### */

#include "import_int.hxx"
#include "util_int.h"
#include "utilcpp_int.hxx"
#include <cmath>
#include <cstdio>
#include <fstream>
#include <list>
#include <sstream>
#include <algorithm>
#include <map>
#include <iostream>
#include <clocale>

/* ========================= static variables ======================================================================= */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ key to types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static std::map<std::string, const char *> key_to_types{
    {"accelerate", "i"},
    {"algorithm", "s"},
    {"bar_color", "ddd"},
    {"bar_color", "i"},
    {"bar_width", "d"},
    {"bin_counts", "i"},
    {"bin_edges", "nD"},
    {"bin_width", "d"},
    {"c", "nD"},
    {"clip_negative", "i"},
    {"colormap", "i"},
    {"draw_edges", "i"},
    {"edge_color", "ddd"},
    {"edge_color", "i"},
    {"edge_width", "d"},
    {"equal_up_and_down_error", "i"},
    {"error", "a"},
    {"error_bar_style", "i"},
    {"grplot", "i"},
    {"ind_bar_color", "nA"},
    {"ind_edge_color", "nA"},
    {"ind_edge_width", "nA"},
    {"int_limits_high", "nD"},
    {"int_limits_low", "nD"},
    {"isovalue", "d"},
    {"keep_aspect_ratio", "i"},
    {"keep_radii_axes", "i"},
    {"kind", "s"},
    {"levels", "i"},
    {"line_spec", "s"},
    {"location", "i"},
    {"major_h", "i"},
    {"marginal_heatmap_kind", "s"},
    {"marker_type", "i"},
    {"num_bins", "i"},
    {"normalization", "s"},
    {"only_quadratic_aspect_ratio", "i"},
    {"orientation", "s"},
    {"phi_flip", "i"},
    {"phi_lim", "dd"},
    {"resample_method", "s"},
    {"r_lim", "dd"},
    {"rotation", "d"},
    {"scale", "i"},
    {"scatter_z", "i"},
    {"stairs", "i"},
    {"step_where", "s"},
    {"style", "s"},
    {"tilt", "d"},
    {"title", "s"},
    {"transformation", "i"},
    {"use_bins", "i"},
    {"x_bins", "i"},
    {"x_colormap", "i"},
    {"x_flip", "i"},
    {"x_grid", "i"},
    {"x_label", "s"},
    {"x_lim", "dd"},
    {"x_log", "i"},
    {"x_range", "dd"},
    {"xye_file", "i"},
    {"xyz_file", "i"},
    {"y_bins", "i"},
    {"y_colormap", "i"},
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

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ kind types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static std::list<std::string> kind_types = {
    "barplot",    "contour",       "contourf",        "heatmap",       "hexbin",
    "hist",       "imshow",        "isosurface",      "line",          "marginal_heatmap",
    "polar_line", "polar_heatmap", "polar_histogram", "polar_scatter", "pie",
    "plot3",      "scatter",       "scatter3",        "shade",         "surface",
    "stem",       "stairs",        "tricontour",      "trisurface",    "quiver",
    "volume",     "wireframe"};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ alias for keys ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static std::map<std::string, std::string> key_alias = {
    {"h_kind", "marginal_heatmap_kind"}, {"aspect", "keep_aspect_ratio"}, {"cmap", "colormap"}};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ container parameter ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static std::map<std::string, const char *> container_params{
    {"error", "a"}, {"ind_bar_color", "nA"}, {"ind_edge_color", "nA"}, {"ind_edge_width", "nA"}};

static std::map<std::string, const char *> container_to_types{{"downwards_cap_color", "i"},
                                                              {"error_bar_color", "i"},
                                                              {"indices", "i"},
                                                              {"indices", "nI"},
                                                              {"rgb", "ddd"},
                                                              {"upwards_cap_color", "i"},
                                                              {"width", "d"}};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ global flags defined by the user input ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static int scatter_with_z = 0;
static int use_bins = 0;
static int equal_up_and_down_error = 0;
static int xye_file = 0;
static int xyz_file = 0;

/* ========================= functions ============================================================================== */

/* ------------------------- import --------------------------------------------------------------------------------- */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ filereader ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

std::string normalize_line(const std::string &str)
{
  std::string s;
  std::string item;
  std::istringstream ss(str);

  s = "";
  while (ss >> item)
    {
      if (item[0] == '#') break;
      if (!s.empty())
        {
          s += '\t';
        }
      s += item;
    }
  return s;
}

err_t parse_columns(std::list<int> *columns, const char *colms)
{
  std::string token;
  std::stringstream scol(colms);
  while (std::getline(scol, token, ',') && token.length())
    {
      if (token.find(':') != std::string::npos)
        {
          std::stringstream stok(token);
          int start = 0, end = 0;
          if (starts_with(token, ":"))
            {
              try
                {
                  end = std::stoi(token.erase(0, 1));
                }
              catch (std::invalid_argument &e)
                {
                  fprintf(stderr, "Invalid argument for column parameter (%s)\n", token.c_str());
                  return ERROR_PARSE_INT;
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
                      return ERROR_PARSE_INT;
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
              return ERROR_PARSE_INT;
            }
        }
    }
  if (!(*columns).empty()) (*columns).sort();
  return ERROR_NONE;
}

err_t read_data_file(const std::string &path, std::vector<std::vector<std::vector<double>>> &data,
                     std::vector<int> &x_data, std::vector<int> &error_data, std::vector<std::string> &labels,
                     grm_args_t *args, const char *colms, const char *x_colms, const char *y_colms, const char *e_colms,
                     PlotRange *ranges)
{
  std::string line;
  std::string token;
  std::ifstream file_path(path);
  std::istream &cin_path = std::cin;
  std::list<int> columns, x_columns, y_columns, e_columns;
  bool depth_change = true;
  int depth = 0, max_col = -1;
  int linecount = 0;
  err_t error = ERROR_NONE;

  /* read the columns from the colms string also converts slicing into numbers */
  if ((error = parse_columns(&columns, colms)) != ERROR_NONE) return error;
  if (!columns.empty()) ranges->ymin = *columns.begin();
  /* read the columns from the x_colms, y_colms and e_colms string also converts slicing into numbers */
  if ((error = parse_columns(&x_columns, x_colms)) != ERROR_NONE) return error;
  if ((error = parse_columns(&y_columns, y_colms)) != ERROR_NONE) return error;
  if ((error = parse_columns(&e_columns, e_colms)) != ERROR_NONE) return error;

  std::istream &file = (path == "-") ? cin_path : file_path;
  /* read the lines from the file */
  while (getline(file, line))
    {
      std::istringstream iss(line, std::istringstream::in);
      linecount += 1;
      /* the line defines a grm container parameter */
      if (line[0] == '#')
        {
          std::string key;
          std::string value;
          std::stringstream ss(line);

          /* read the key-value pairs from the file and redirect them to grm if possible */
          for (size_t col = 0; std::getline(ss, token, ':') && token.length(); col++)
            {
              if (col == 0)
                {
                  key = trim(token.substr(1, token.length() - 1));
                }
              else
                {
                  value = trim(token);
                }
            }
          if (str_equals_any(key, "title", "x_label", "y_label", "z_label", "resample_method"))
            {
              const char *tmp;
              if (!grm_args_values(args, key.c_str(), "s", &tmp)) grm_args_push(args, key.c_str(), "s", value.c_str());
            }
          else if (str_equals_any(key, "location", "x_log", "y_log", "z_log", "x_grid", "y_grid", "z_grid"))
            {
              try
                {
                  int tmp;
                  if (!grm_args_values(args, key.c_str(), "i", &tmp))
                    grm_args_push(args, key.c_str(), "i", std::stoi(value));
                }
              catch (std::invalid_argument &e)
                {
                  fprintf(stderr, "Invalid argument for plot parameter (%s:%s) in line %i\n", key.c_str(),
                          value.c_str(), linecount);
                  return ERROR_PARSE_INT;
                }
            }
          else if (str_equals_any(key, "phi_lim", "r_lim", "x_lim", "y_lim", "z_lim", "x_range", "y_range", "z_range"))
            {
              std::stringstream sv(value);
              std::string value1;
              std::string value2;
              double tmp1, tmp2;
              int ret_val;

              for (size_t col = 0; std::getline(sv, token, ',') && token.length(); col++)
                {
                  if (col == 0)
                    {
                      value1 = trim(token);
                    }
                  else
                    {
                      value2 = trim(token);
                    }
                }
              try
                {
                  ret_val = grm_args_values(args, key.c_str(), "dd", &tmp1, &tmp2);
                  if (!ret_val) grm_args_push(args, key.c_str(), "dd", std::stod(value1), std::stod(value2));
                }
              catch (std::invalid_argument &e)
                {
                  fprintf(stderr, "Invalid argument for plot parameter (%s:%s,%s) in line %i\n", key.c_str(),
                          value1.c_str(), value2.c_str(), linecount);
                  return ERROR_PARSE_DOUBLE;
                }
              if (!ret_val)
                {
                  if (strcmp(key.c_str(), "x_range") == 0)
                    {
                      ranges->xmin = std::stod(value1);
                      ranges->xmax = std::stod(value2);
                    }
                  else if (strcmp(key.c_str(), "y_range") == 0)
                    {
                      ranges->ymin = std::stod(value1);
                      ranges->ymax = std::stod(value2);
                    }
                  else if (strcmp(key.c_str(), "z_range") == 0)
                    {
                      ranges->zmin = std::stod(value1);
                      ranges->zmax = std::stod(value2);
                    }
                }
            }
          else
            {
              fprintf(stderr, "Unknown key:value pair (%s:%s) in line %i\n", key.c_str(), value.c_str(), linecount);
              /* TODO: extend these if more key values pairs are needed */
            }
          continue;
        }
      else /* the line contains the labels for the plot */
        {
          std::istringstream line_ss(normalize_line(line));
          std::string split_label;
          for (size_t col = 0; std::getline(line_ss, token, '\t') && token.length(); col++)
            {
              if (std::find(columns.begin(), columns.end(), col) != columns.end() || columns.empty())
                {
                  if (std::find(token.begin(), token.end(), '"') != token.end() && split_label.empty())
                    {
                      split_label = token.erase(0, 1);
                      continue;
                    }
                  if (!split_label.empty())
                    {
                      token.erase(std::remove(token.begin(), token.end(), '"'), token.end());
                      labels.push_back(split_label.append(" ").append(token));
                      split_label = "";
                      continue;
                    }
                  labels.push_back(token);
                }
            }
          break;
        }
    }

  // Save locale setting
  const std::string oldLocale = std::setlocale(LC_NUMERIC, nullptr);
  std::setlocale(LC_NUMERIC, "C");

  /* read the numeric data for the plot */
  for (size_t row = 0; std::getline(file, line); row++)
    {
      std::istringstream line_ss(normalize_line(line));
      int cnt = 0;
      char det = '\t';
      int start_with_nan = 0;
      size_t col;
      if (line.empty())
        {
          depth += 1;
          depth_change = true;
          continue;
        }
      if (std::find(line.begin(), line.end(), ',') != line.end())
        {
          det = ',';
          std::string tmp = ",";
          if (starts_with(trim(line), tmp)) start_with_nan = 1;
        }
      for (col = 0; std::getline(line_ss, token, det) && (token.length() || start_with_nan); col++)
        {
          if (std::find(columns.begin(), columns.end(), col) != columns.end() ||
              (columns.empty() && labels.empty() && (!use_bins || col > 0)) ||
              (columns.empty() && col < labels.size() && (!use_bins || col > 0)))
            {
              if ((row == 0 && (col == use_bins || col == columns.front())) ||
                  (depth_change && (col == use_bins || col == columns.front())) ||
                  (start_with_nan && (col == use_bins || col == columns.front())))
                {
                  data.emplace_back(std::vector<std::vector<double>>());
                }
              if (depth_change)
                {
                  data[depth].emplace_back(std::vector<double>());
                }
              if (max_col != -1 && max_col < (int)cnt + 1)
                {
                  fprintf(stderr, "Line %i has a different number of columns (%i) than previous lines (%i)\n",
                          (int)row + linecount + 1, cnt + 1, max_col);
                  return ERROR_PLOT_MISSING_DATA;
                }
              try
                {
                  trim(token);
                  token.erase(std::remove(token.begin(), token.end(), '\t'), token.end());
                  if (token.empty() && det == ',')
                    {
                      data[depth][cnt].push_back(NAN);
                    }
                  else
                    {
                      data[depth][cnt].push_back(std::stod(token));
                    }
                }
              catch (std::invalid_argument &e)
                {
                  fprintf(stderr, "Invalid number in line %zu, column %zu (%s)\n", row + linecount + 1, col + 1,
                          token.c_str());
                  return ERROR_PARSE_DOUBLE;
                }
              if (row == 0 && !x_columns.empty() &&
                  std::find(x_columns.begin(), x_columns.end(), col) != x_columns.end())
                x_data.emplace_back(cnt);
              if (row == 0 && !e_columns.empty() &&
                  std::find(e_columns.begin(), e_columns.end(), col) != e_columns.end())
                error_data.emplace_back(cnt);
              cnt += 1;
            }
          else if (use_bins && col == 0)
            {
              try
                {
                  if (row == 0)
                    {
                      ranges->ymin = std::stod(token);
                    }
                  else
                    {
                      ranges->ymax = std::stod(token); // not the best way to get ymax but the number of rows is unknown
                    }
                }
              catch (std::invalid_argument &e)
                {
                  fprintf(stderr,
                          "Invalid argument for y_range parameter (%s) while using option use_bins in line %i\n",
                          labels[0].c_str(), (int)row + linecount + 1);
                }
            }
        }
      depth_change = false;
      if (max_col == -1)
        {
          max_col = (int)col;
        }
      else if (max_col != (int)col)
        {
          fprintf(stderr, "Line %i has a different number of columns (%i) than previous lines (%i)\n",
                  (int)row + linecount + 1, (int)col, max_col);
          return ERROR_PLOT_MISSING_DATA;
        }
    }
  // Restore locale setting
  std::setlocale(LC_NUMERIC, oldLocale.c_str());
  return ERROR_NONE;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ argument container ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

grm_file_args_t *grm_file_args_new()
{
  auto *args = new grm_file_args_t;
  if (args == nullptr)
    {
      debug_print_malloc_error();
      return nullptr;
    }
  args->file_path = "";
  args->file_columns = "";
  args->file_x_columns = "";
  args->file_y_columns = "";
  args->file_error_columns = "";
  return args;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ utils ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void adjust_ranges(double *range_min, double *range_max, double default_value_min, double default_value_max)
{
  *range_min = (*range_min == INFINITY) ? default_value_min : *range_min;
  *range_max = (*range_max == INFINITY) ? default_value_max : *range_max;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plot functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int grm_plot_from_file(int argc, char **argv)
/**
 * Allows to create a plot from a file. The file is holding the data and the container arguments for the plot.
 *
 * \param argc: number of elements inside argv.
 * \param argv: contains the parameter which specify the displayed plot. For example where the data is or which plot
 * should be drawn.
 * \return 1 when there was no error, 0 if there was an error.
 */
{
  grm_args_t *args;
  int error = 1;

  args = grm_args_new();
  error = grm_interactive_plot_from_file(args, argc, argv);
  grm_plot(args);
  grm_args_delete(args);
  return error;
}

int grm_interactive_plot_from_file(grm_args_t *args, int argc, char **argv)
/**
 * Allows to create an interactive plot from a file. The file is holding the data and the container arguments for the
 * plot.
 *
 * \param args: a grm container. Should be the container, which also defines the window.
 * \param argc: number of elements inside argv.
 * \param argv: contains the parameter which specify the displayed plot. For example where the data is or which plot
 * should be drawn.
 * \return 1 when there was no error, 0 if there was an error.
 */
{
  std::string s;
  size_t row, col, rows, cols, depth;
  std::vector<std::vector<std::vector<double>>> file_data;
  std::vector<int> x_data, error_data;
  std::vector<std::string> labels;
  std::vector<const char *> labels_c;
  std::vector<grm_args_t *> series;
  char *env, *wstype;
  void *handle = nullptr;
  const char *kind;
  int grplot;
  grm_file_args_t *file_args;
  file_args = grm_file_args_new();
  PlotRange ranges = {INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY};

  if (!convert_inputstream_into_args(args, file_args, argc, argv, &ranges))
    {
      return 0;
    }

  if (file_args->file_path != "-" && !file_exists(file_args->file_path))
    {
      fprintf(stderr, "File not found (%s)\n", file_args->file_path.c_str());
      return 0;
    }

  grm_args_values(args, "kind", "s", &kind);
  if (!str_equals_any(kind, "barplot", "hist", "line", "scatter", "stairs", "stem"))
    {
      file_args->file_x_columns.clear();
      file_args->file_y_columns.clear();
      file_args->file_error_columns.clear();
      xye_file = 0;
    }
  if (xye_file)
    {
      file_args->file_x_columns.clear();
      file_args->file_x_columns = "0";
      file_args->file_y_columns.clear();
      file_args->file_y_columns = "1";
      file_args->file_error_columns.clear();
      file_args->file_error_columns = equal_up_and_down_error ? "2" : "2,3";
    }
  if (read_data_file(file_args->file_path, file_data, x_data, error_data, labels, args, file_args->file_columns.c_str(),
                     file_args->file_x_columns.c_str(), file_args->file_y_columns.c_str(),
                     file_args->file_error_columns.c_str(), &ranges))
    {
      return 0;
    }

  if (!file_data.empty())
    {
      depth = file_data.size();
      cols = file_data[0].size();
      if (str_equals_any(kind, "barplot", "hist", "line", "scatter", "stairs", "stem"))
        {
          cols -= x_data.size() + error_data.size(); // less y columns if x or error data given
        }
      rows = file_data[0][0].size();
      depth = (depth == 1) ? 0 : depth;
    }
  else
    {
      fprintf(stderr, "File is empty\n");
      return 0;
    }
  if (cols != labels.size())
    {
      fprintf(stderr, "The number of columns (%zu) doesn't fit the number of labels (%zu)\n", cols, labels.size());
    }

  series.resize(cols);

  wstype = getenv("GKS_WSTYPE");
  if (wstype != nullptr && strcmp(wstype, "381") == 0 && (env = getenv("GR_DISPLAY")) != nullptr)
    {
      handle = grm_open(GRM_SENDER, env, 8002, nullptr, nullptr);
      if (handle == nullptr)
        {
          fprintf(stderr, "GRM connection to '%s' could not be established\n", env);
        }
    }

  if ((strcmp(kind, "line") == 0 || (strcmp(kind, "scatter") == 0 && !scatter_with_z)) &&
      ((rows >= 50 && cols >= 50) || (use_bins && rows >= 49 && cols >= 49)))
    {
      fprintf(stderr, "Too much data for %s plot - use heatmap instead\n", kind);
      kind = "heatmap";
      grm_args_push(args, "kind", "s", kind);
    }
  if (!str_equals_any(kind, "isosurface", "quiver", "volume") && depth >= 1)
    {
      fprintf(stderr, "Too much data for %s plot - use volume instead\n", kind);
      kind = "volume";
      grm_args_push(args, "kind", "s", kind);
    }

  if (!str_equals_any(kind, "contour", "contourf", "heatmap", "imshow", "marginal_heatmap", "surface", "wireframe"))
    {
      // these parameters are only for surface and similar types
      use_bins = 0;
      xyz_file = 0;
    }

  if (str_equals_any(kind, "contour", "contourf", "heatmap", "imshow", "marginal_heatmap", "surface", "wireframe"))
    {
      int x_dim = cols, y_dim = rows, z_dim = rows * cols;
      if (cols <= 1 || (xyz_file && cols < 3))
        {
          fprintf(stderr, "Insufficient data for plot type (%s)\n", kind);
          return 0;
        }
      if (xyz_file)
        {
          x_dim = 1, z_dim = rows;
          for (int i = 1; i < rows; i++)
            {
              if (file_data[depth][1][i] == file_data[depth][1][i - 1])
                {
                  x_dim += 1;
                }
              else
                {
                  y_dim = rows / x_dim;
                  break;
                }
            }
          ranges.xmin = file_data[depth][0][0];
          ranges.xmax = file_data[depth][0][x_dim - 1];
          ranges.ymin = file_data[depth][1][0];
          ranges.ymax = file_data[depth][1][rows - 1];
        }

      std::vector<double> xi(x_dim), yi(y_dim), zi(z_dim);

      if (use_bins)
        {
          try
            {
              ranges.xmin = std::stod(labels[0]);
              ranges.xmax = std::stod(labels[cols - 1]);
            }
          catch (std::invalid_argument &e)
            {
              fprintf(stderr, "Invalid argument for x_range parameter (%s, %s) while using option use_bins\n",
                      labels[0].c_str(), labels[cols - 1].c_str());
            }
        }
      adjust_ranges(&ranges.xmin, &ranges.xmax, 0.0, (double)x_dim - 1.0);
      adjust_ranges(&ranges.ymin, &ranges.ymax, 0.0, (double)y_dim - 1.0);
      ranges.ymax = (ranges.ymax <= ranges.ymin) ? ranges.ymax + ranges.ymin : ranges.ymax;

      if (xyz_file)
        {
          for (row = 0; row < rows; ++row)
            {
              if (row < x_dim)
                {
                  xi[row] = ranges.xmin + (ranges.xmax - ranges.xmin) * ((double)row / ((double)x_dim - 1));
                }
              if (row % x_dim == 0)
                {
                  yi[row / x_dim] =
                      ranges.ymin + (ranges.ymax - ranges.ymin) * ((double)(row / x_dim) / ((double)y_dim - 1));
                }
              zi[row] = file_data[depth][2][row];
            }
        }
      else
        {
          for (col = 0; col < cols; ++col)
            {
              xi[col] = ranges.xmin + (ranges.xmax - ranges.xmin) * ((double)col / ((double)cols - 1));
              for (row = 0; row < rows; ++row)
                {
                  if (col == 0)
                    {
                      yi[row] = ranges.ymin + (ranges.ymax - ranges.ymin) * ((double)row / ((double)rows - 1));
                    }
                  zi[row * cols + col] = file_data[depth][col][row];
                }
            }
        }

      if (ranges.zmax != INFINITY)
        {
          int elem;
          double min_val = *std::min_element(std::begin(zi), std::end(zi));
          double max_val = *std::max_element(std::begin(zi), std::end(zi));

          for (elem = 0; elem < z_dim; ++elem)
            {
              zi[elem] = ranges.zmin + (ranges.zmax - ranges.zmin) * (zi[elem] - min_val) / (max_val - min_val);
            }
        }

      /* for imshow plot */
      grm_args_push(args, "c", "nD", z_dim, zi.data());
      grm_args_push(args, "c_dims", "ii", x_dim, y_dim);

      grm_args_push(args, "x", "nD", x_dim, xi.data());
      grm_args_push(args, "y", "nD", y_dim, yi.data());
      grm_args_push(args, "z", "nD", z_dim, zi.data());
    }
  else if (strcmp(kind, "line") == 0 || (strcmp(kind, "scatter") == 0 && !scatter_with_z))
    {
      grm_args_t *error = nullptr;
      std::vector<grm_args_t *> error_vec;
      std::vector<double> x(rows);
      std::vector<int> filtered_error_columns;
      int err = 0, col_group_elem = 3, down_err_off = 2;
      const char *spec;

      adjust_ranges(&ranges.xmin, &ranges.xmax, 0.0, (double)rows - 1.0);
      cols += x_data.size() + error_data.size();

      // calculate x-data if x_data is empty which means no x given
      if (x_data.empty())
        {
          for (row = 0; row < rows; row++)
            {
              x[row] = ranges.xmin + (ranges.xmax - ranges.xmin) * ((double)row / ((double)rows - 1));
            }
        }
      // precalculate the number of error columns, so they can be removed from the y-data in the following step
      if (grm_args_values(args, "error", "a", &error) || xye_file || equal_up_and_down_error)
        {
          if (error == nullptr)
            {
              error = grm_args_new();
              grm_args_push(args, "error", "a", error);
            }

          if (equal_up_and_down_error)
            {
              col_group_elem -= 1;
              down_err_off -= 1;
            }
          err = floor(cols / col_group_elem) * down_err_off;
        }
      // find min and max value of all y-data and make sure the all data points are inside that range
      if (ranges.ymax != INFINITY && ranges.ymin != INFINITY)
        {
          double min_val = INFINITY, max_val = -INFINITY;
          for (col = 0; col < cols - err; col++)
            {
              min_val = std::min<double>(
                  min_val,
                  *std::min_element(
                      std::begin(file_data[depth][col + ((col < err / down_err_off) ? col * down_err_off : err)]),
                      std::end(file_data[depth][col + ((col < err / down_err_off) ? col * down_err_off : err)])));
              max_val = std::max<double>(
                  max_val,
                  *std::max_element(
                      std::begin(file_data[depth][col + ((col < err / down_err_off) ? col * down_err_off : err)]),
                      std::end(file_data[depth][col + ((col < err / down_err_off) ? col * down_err_off : err)])));
            }

          for (col = 0; col < cols; ++col)
            {
              if (std::find(x_data.begin(), x_data.end(), col) != x_data.end()) continue;
              if (std::find(error_data.begin(), error_data.end(), col) != error_data.end()) continue;
              for (row = 0; row < rows; row++)
                {
                  file_data[depth][col][row] = ranges.ymin + (ranges.ymax - ranges.ymin) *
                                                                 (file_data[depth][col][row] - min_val) /
                                                                 (max_val - min_val);
                }
            }
        }

      // calculate the error data
      if (grm_args_values(args, "error", "a", &error))
        {
          int i;
          int color_up, color_down, color;
          std::vector<double> errors_up(rows);
          std::vector<double> errors_down(rows);

          if ((cols < col_group_elem && error_data.empty()) ||
              (!error_data.empty() && error_data.size() < down_err_off))
            {
              fprintf(stderr, "Not enough data for error parameter\n");
            }
          else
            {
              if (error_data.empty())
                {
                  err = floor(cols / col_group_elem);
                  error_vec.resize(err);
                  for (col = 0; col < err; col++)
                    {
                      error_vec[col] = grm_args_new();
                      for (i = 0; i < rows; i++)
                        {
                          errors_up[i] = file_data[depth][col + 1 + col * down_err_off][i];
                          errors_down[i] = file_data[depth][col + down_err_off + col * down_err_off][i];
                        }
                      grm_args_push(error_vec[col], "relative", "nDD", rows, errors_up.data(), errors_down.data());
                      if (grm_args_values(error, "error_bar_color", "i", &color))
                        grm_args_push(error_vec[col], "error_bar_color", "i", color);
                      if (grm_args_values(error, "downwards_cap_color", "i", &color_down))
                        grm_args_push(error_vec[col], "downwards_cap_color", "i", color_down);
                      if (grm_args_values(error, "upwards_cap_color", "i", &color_up))
                        grm_args_push(error_vec[col], "upwards_cap_color", "i", color_up);
                    }
                  err *= down_err_off;
                }
              else
                {
                  int cnt = 0;
                  err = 0;
                  error_vec.resize(equal_up_and_down_error ? error_data.size() : error_data.size() / 2);
                  for (i = 0; i < error_vec.size(); i++)
                    {
                      error_vec[i] = grm_args_new();
                      if (grm_args_values(error, "error_bar_color", "i", &color))
                        grm_args_push(error_vec[i], "error_bar_color", "i", color);
                      if (grm_args_values(error, "downwards_cap_color", "i", &color_down))
                        grm_args_push(error_vec[i], "downwards_cap_color", "i", color_down);
                      if (grm_args_values(error, "upwards_cap_color", "i", &color_up))
                        grm_args_push(error_vec[i], "upwards_cap_color", "i", color_up);
                    }
                  for (int error_col : error_data)
                    {
                      for (i = 0; i < rows; i++)
                        {
                          if (equal_up_and_down_error)
                            {
                              errors_up[i] = file_data[depth][error_col][i];
                              errors_down[i] = file_data[depth][error_col][i];
                            }
                          else if (cnt % 2 == 0)
                            {
                              errors_up[i] = file_data[depth][error_col][i];
                            }
                          else if (cnt % 2 != 0)
                            {
                              errors_down[i] = file_data[depth][error_col][i];
                            }
                        }
                      if (cnt % 2 != 0 || equal_up_and_down_error)
                        {
                          grm_args_push(error_vec[floor(cnt / (equal_up_and_down_error ? 1 : 2))], "relative", "nDD",
                                        rows, errors_up.data(), errors_down.data());
                        }
                      else
                        {
                          filtered_error_columns.push_back(error_col);
                        }
                      cnt += 1;
                    }
                }
            }
        }
      int y_cnt = 0, x_cnt = 0, err_cnt = 0;
      if (!x_data.empty() || !error_data.empty())
        {
          for (col = 0; col < cols - x_data.size() - error_data.size(); col++)
            {
              series[col] = grm_args_new();
            }
        }
      for (col = 0; col < cols - err; col++)
        {
          if (x_data.empty() && error_data.empty())
            {
              series[col] = grm_args_new();
              grm_args_push(series[col], "x", "nD", rows, x.data());
              grm_args_push(series[col], "y", "nD", rows,
                            file_data[depth][col + ((col < err / down_err_off) ? col * down_err_off : err)].data());
              if (col < err / down_err_off) grm_args_push(series[col], "error", "a", error_vec[col]);
              if (!labels.empty())
                {
                  labels_c.push_back(labels[col].c_str());
                }
              if (grm_args_values(args, "line_spec", "s", &spec)) grm_args_push(series[col], "line_spec", "s", spec);
            }
          else
            {
              if (std::find(x_data.begin(), x_data.end(), col) != x_data.end())
                {
                  int error_bar_style;
                  grm_args_push(series[x_cnt], "x", "nD", rows, file_data[depth][col].data());
                  if (grm_args_values(args, "error_bar_style", "i", &error_bar_style))
                    grm_args_push(series[x_cnt], "error_bar_style", "i", error_bar_style);
                  x_cnt += 1;
                }
              else if (std::find(error_data.begin(), error_data.end(), col) == error_data.end())
                {
                  grm_args_push(series[y_cnt], "y", "nD", rows, file_data[depth][col].data());
                  if (!labels.empty()) labels_c.push_back(labels[col].c_str());
                  if (grm_args_values(args, "line_spec", "s", &spec))
                    grm_args_push(series[y_cnt], "line_spec", "s", spec);
                  y_cnt += 1;
                }
              else if (std::find(filtered_error_columns.begin(), filtered_error_columns.end(), col) ==
                       filtered_error_columns.end())
                {
                  grm_args_push(series[err_cnt], "error", "a", error_vec[err_cnt]);
                  err_cnt += 1;
                }
            }
        }
      cols -= x_data.size() + error_data.size();
      grm_args_push(args, "series", "nA", cols - err, series.data());
      if (!labels_c.empty())
        {
          grm_args_push(args, "labels", "nS", cols - err, labels_c.data());
        }
    }
  else if (str_equals_any(kind, "isosurface", "volume"))
    {
      int i, j, k;
      std::vector<double> data(rows * cols * depth);
      int n = (int)rows * (int)cols * (int)depth;
      std::vector<int> dims = {(int)cols, (int)rows, (int)depth};
      for (i = 0; i < rows; ++i)
        {
          for (j = 0; j < cols; ++j)
            {
              for (k = 0; k < depth; ++k)
                {
                  data[k * cols * rows + j * rows + i] = file_data[k][j][i];
                }
            }
        }

      grm_args_push(args, "c", "nD", n, data.data());
      grm_args_push(args, "c_dims", "nI", 3, dims.data());
    }
  else if (str_equals_any(kind, "plot3", "scatter3", "tricontour", "trisurface") ||
           (strcmp(kind, "scatter") == 0 && scatter_with_z))
    {
      double min_x, max_x, min_y, max_y, min_z, max_z;
      if (cols < 3)
        {
          fprintf(stderr, "Insufficient data for plot type (%s)\n", kind);
          return 0;
        }
      if (cols > 3) fprintf(stderr, "Only the first 3 columns get displayed\n");

      /* apply the ranges to the data */
      if (ranges.xmax != INFINITY)
        {
          min_x = *std::min_element(std::begin(file_data[depth][0]), std::end(file_data[depth][0]));
          max_x = *std::max_element(std::begin(file_data[depth][0]), std::end(file_data[depth][0]));
          adjust_ranges(&ranges.xmin, &ranges.xmax, min_x, max_x);
        }
      if (ranges.ymax != INFINITY)
        {
          min_y = *std::min_element(std::begin(file_data[depth][1]), std::end(file_data[depth][1]));
          max_y = *std::max_element(std::begin(file_data[depth][1]), std::end(file_data[depth][1]));
          adjust_ranges(&ranges.ymin, &ranges.ymax, min_y, max_y);
          ranges.ymax = (ranges.ymax <= ranges.ymin) ? ranges.ymax + ranges.ymin : ranges.ymax;
        }
      if (ranges.zmax != INFINITY)
        {
          min_z = *std::min_element(std::begin(file_data[depth][2]), std::end(file_data[depth][2]));
          max_z = *std::max_element(std::begin(file_data[depth][2]), std::end(file_data[depth][2]));
          adjust_ranges(&ranges.zmin, &ranges.zmax, min_z, max_z);
          ranges.zmax = (ranges.zmax <= ranges.zmin) ? ranges.zmax + ranges.zmin : ranges.zmax;
        }
      for (row = 0; row < rows; ++row)
        {
          if (ranges.xmax != INFINITY)
            file_data[depth][0][row] = ranges.xmin + (ranges.xmax - ranges.xmin) *
                                                         (((double)file_data[depth][0][row]) - min_x) / (max_x - min_x);
          if (ranges.ymax != INFINITY)
            file_data[depth][1][row] = ranges.ymin + (ranges.ymax - ranges.ymin) *
                                                         (((double)file_data[depth][1][row]) - min_y) / (max_y - min_y);
          if (ranges.zmax != INFINITY)
            file_data[depth][2][row] = ranges.zmin + (ranges.zmax - ranges.zmin) *
                                                         (((double)file_data[depth][2][row]) - min_z) / (max_z - min_z);
        }

      grm_args_push(args, "x", "nD", rows, file_data[depth][0].data());
      grm_args_push(args, "y", "nD", rows, file_data[depth][1].data());
      grm_args_push(args, "z", "nD", rows, file_data[depth][2].data());
    }
  else if (str_equals_any(kind, "barplot", "hist", "stem", "stairs"))
    {
      std::vector<double> x(rows);
      double xmin, xmax, ymin, ymax;
      int y_ind = 0;
      grm_args_t *error = nullptr;

      if (strcmp(kind, "barplot") == 0)
        {
          adjust_ranges(&ranges.xmin, &ranges.xmax, 1.0, (double)rows);
        }
      else
        {
          adjust_ranges(&ranges.xmin, &ranges.xmax, 0.0, (double)rows - 1.0);
        }
      if (x_data.empty())
        {
          for (row = 0; row < rows; row++)
            {
              x[row] = ranges.xmin + (ranges.xmax - ranges.xmin) * ((double)row / ((double)rows - 1));
            }
        }
      else
        {
          x = file_data[depth][x_data[0]];
          if (!grm_args_values(args, "x_range", "dd", &xmin, &xmax))
            {
              xmin = *std::min_element(std::begin(file_data[depth][x_data[0]]), std::end(file_data[depth][x_data[0]]));
              xmax = *std::max_element(std::begin(file_data[depth][x_data[0]]), std::end(file_data[depth][x_data[0]]));
              adjust_ranges(&ranges.xmin, &ranges.xmax, xmin, xmax);
              grm_args_push(args, "x_range", "dd", ranges.xmin, ranges.xmax);
            }
          else
            {
              /* apply y_range to the data */
              xmin = *std::min_element(std::begin(file_data[depth][x_data[0]]), std::end(file_data[depth][x_data[0]]));
              xmax = *std::max_element(std::begin(file_data[depth][x_data[0]]), std::end(file_data[depth][x_data[0]]));
              for (row = 0; row < rows; ++row)
                {
                  x[row] = ranges.xmin + (ranges.xmax - ranges.xmin) / (xmax - xmin) *
                                             ((double)file_data[depth][x_data[0]][row] - xmin);
                }
            }
        }
      if (!x_data.empty() || !error_data.empty())
        {
          for (col = 0; col < cols + x_data.size() + error_data.size(); col++)
            {
              if (std::find(error_data.begin(), error_data.end(), col) == error_data.end() &&
                  std::find(x_data.begin(), x_data.end(), col) == x_data.end())
                {
                  y_ind = col;
                  break;
                }
            }
        }
      if (!grm_args_values(args, "y_range", "dd", &ymin, &ymax))
        {
          ymin = *std::min_element(std::begin(file_data[depth][y_ind]), std::end(file_data[depth][y_ind]));
          ymax = *std::max_element(std::begin(file_data[depth][y_ind]), std::end(file_data[depth][y_ind]));
          adjust_ranges(&ranges.ymin, &ranges.ymax, std::min<double>(0.0, ymin), ymax);
          grm_args_push(args, "y_range", "dd", ranges.ymin, ranges.ymax);
        }
      else
        {
          /* apply y_range to the data */
          ymin = *std::min_element(std::begin(file_data[depth][y_ind]), std::end(file_data[depth][y_ind]));
          ymax = *std::max_element(std::begin(file_data[depth][y_ind]), std::end(file_data[depth][y_ind]));
          for (row = 0; row < rows; ++row)
            {
              file_data[depth][y_ind][row] =
                  ranges.ymin + (ranges.ymax - ranges.ymin) / (ymax - 0) * ((double)file_data[depth][y_ind][row] - 0);
            }
        }
      if (!grm_args_values(args, "x_range", "dd", &xmin, &xmax))
        {
          grm_args_push(args, "x_range", "dd", ranges.xmin, ranges.xmax);
        }

      grm_args_push(args, "x", "nD", rows, x.data());
      /* for barplot */
      grm_args_push(args, "y", "nD", rows, file_data[depth][y_ind].data());
      /* for hist */
      grm_args_push(args, "weights", "nD", rows, file_data[depth][y_ind].data());
      /* for stairs */
      grm_args_push(args, "z", "nD", rows, file_data[depth][y_ind].data());

      /* the needed calculation to get the errorbars out of the data */
      if (grm_args_values(args, "error", "a", &error) || xye_file || equal_up_and_down_error)
        {
          int nbins, i;
          int col_group_elem = 3, down_err_off = 2;
          std::vector<double> errors_up(rows);
          std::vector<double> errors_down(rows);
          std::vector<double> bins;

          if (error == nullptr)
            {
              error = grm_args_new();
              grm_args_push(args, "error", "a", error);
            }

          if (equal_up_and_down_error)
            {
              col_group_elem -= 1;
              down_err_off -= 1;
            }
          if (cols < col_group_elem || (!error_data.empty() && error_data.size() < down_err_off))
            {
              fprintf(stderr, "Not enough data for error parameter\n");
            }
          else if (str_equals_any(kind, "barplot", "hist"))
            {
              if (!grm_args_values(args, "num_bins", "i", &nbins))
                {
                  if (!grm_args_values(args, "bins", "i", &nbins, &bins))
                    {
                      if (strcmp(kind, "hist") == 0) nbins = (int)(3.3 * log10((int)rows) + 0.5) + 1;
                    }
                }
              if (strcmp(kind, "barplot") == 0) nbins = (int)rows;
              if (nbins <= rows)
                {
                  for (i = 0; i < nbins; i++)
                    {
                      if (error_data.empty())
                        {
                          errors_up[i] = file_data[depth][1][i];
                          errors_down[i] = file_data[depth][down_err_off][i];
                        }
                      else
                        {
                          errors_up[i] = file_data[depth][error_data[0]][i];
                          errors_down[i] = file_data[depth][equal_up_and_down_error ? error_data[0] : error_data[1]][i];
                        }
                    }
                  grm_args_push(error, "relative", "nDD", nbins, errors_up.data(), errors_down.data());
                }
              else
                {
                  fprintf(stderr, "Not enough data for error parameter\n");
                }
            }
        }
    }
  else if (strcmp(kind, "pie") == 0)
    {
      std::vector<double> x(cols);
      std::vector<double> c(cols * 3);
      for (col = 0; col < cols; col++)
        {
          x[col] = file_data[depth][col][0];
          if (!labels.empty())
            {
              labels_c.push_back(labels[col].c_str());
            }
        }

      grm_args_push(args, "x", "nD", cols, x.data());
      if (!labels_c.empty())
        {
          grm_args_push(args, "labels", "nS", cols, labels_c.data());
        }
      if (rows >= 4)
        {
          for (col = 0; col < cols; col++)
            {
              for (row = 1; row < 4; row++)
                {
                  c[(row - 1) * cols + col] = (double)file_data[depth][col][row];
                }
            }
          grm_args_push(args, "c", "nD", c.size(), c.data());
        }
      else if (rows > 1)
        {
          fprintf(stderr, "Insufficient data for custom colors\n");
        }
    }
  else if (strcmp(kind, "polar_histogram") == 0)
    {
      if (cols > 1) fprintf(stderr, "Only the first column gets displayed\n");
      grm_args_push(args, "x", "nD", rows, file_data[depth][0].data());
    }
  else if (strcmp(kind, "polar_line") == 0 || strcmp(kind, "polar_scatter") == 0)
    {
      if (cols > 1) fprintf(stderr, "Only the first 2 columns get displayed\n");
      grm_args_push(args, "x", "nD", rows, file_data[depth][0].data());
      grm_args_push(args, "y", "nD", rows, file_data[depth][1].data());
    }
  else if (strcmp(kind, "polar_heatmap") == 0)
    {
      std::vector<double> xi(cols), yi(rows), zi(rows * cols);

      if (cols <= 1)
        {
          fprintf(stderr, "Insufficient data for plot type (%s)\n", kind);
          return 0;
        }
      adjust_ranges(&ranges.xmin, &ranges.xmax, 0.0, 360.0);
      adjust_ranges(&ranges.ymin, &ranges.ymax, 0.0, 3.0);
      ranges.ymax = (ranges.ymax <= ranges.ymin) ? ranges.ymax + ranges.ymin : ranges.ymax;

      for (col = 0; col < cols; ++col)
        {
          xi[col] = ranges.xmin + (ranges.xmax - ranges.xmin) * ((double)col / ((double)cols - 1));
          for (row = 0; row < rows; ++row)
            {
              if (col == 0)
                {
                  yi[row] = ranges.ymin + (ranges.ymax - ranges.ymin) * ((double)row / ((double)rows - 1));
                }
              zi[row * cols + col] = file_data[depth][col][row];
            }
        }
      grm_args_push(args, "x", "nD", cols, xi.data());
      grm_args_push(args, "y", "nD", rows, yi.data());
      grm_args_push(args, "z", "nD", cols * rows, zi.data());
    }
  else if (strcmp(kind, "quiver") == 0)
    {
      std::vector<double> x(cols);
      std::vector<double> y(rows);
      std::vector<double> u(cols * rows);
      std::vector<double> v(cols * rows);

      if (depth < 2)
        {
          fprintf(stderr, "Not enough data for quiver plot\n");
          return 0;
        }

      adjust_ranges(&ranges.xmin, &ranges.xmax, 0.0, (double)cols - 1.0);
      adjust_ranges(&ranges.ymin, &ranges.ymax, 0.0, (double)rows - 1.0);
      ranges.ymax = (ranges.ymax <= ranges.ymin) ? ranges.ymax + ranges.ymin : ranges.ymax;

      for (col = 0; col < cols; ++col)
        {
          x[col] = ranges.xmin + (ranges.xmax - ranges.xmin) * ((double)col / ((double)cols - 1));

          for (row = 0; row < rows; ++row)
            {
              if (col == 0) y[row] = ranges.ymin + (ranges.ymax - ranges.ymin) * ((double)row / ((double)rows - 1));
              u[row * cols + col] = file_data[0][col][row];
              v[row * cols + col] = file_data[1][col][row];
            }
        }

      grm_args_push(args, "x", "nD", cols, x.data());
      grm_args_push(args, "y", "nD", rows, y.data());
      grm_args_push(args, "u", "nD", cols * row, u.data());
      grm_args_push(args, "v", "nD", cols * row, v.data());
    }
  else if (str_equals_any(kind, "hexbin", "shade"))
    {
      double min_x, min_y, max_x, max_y;
      if (cols > 2) fprintf(stderr, "Only the first 2 columns get displayed\n");

      adjust_ranges(&ranges.xmin, &ranges.xmax, 0.0, (double)cols - 1.0);
      adjust_ranges(&ranges.ymin, &ranges.ymax, 0.0, (double)rows - 1.0);
      ranges.ymax = (ranges.ymax <= ranges.ymin) ? ranges.ymax + ranges.ymin : ranges.ymax;
      min_x = *std::min_element(std::begin(file_data[depth][0]), std::end(file_data[depth][0]));
      max_x = *std::max_element(std::begin(file_data[depth][0]), std::end(file_data[depth][0]));
      min_y = *std::min_element(std::begin(file_data[depth][1]), std::end(file_data[depth][1]));
      max_y = *std::max_element(std::begin(file_data[depth][1]), std::end(file_data[depth][1]));

      for (row = 0; row < rows; row++)
        {
          file_data[depth][0][row] =
              ranges.xmin + (ranges.xmax - ranges.xmin) * (file_data[depth][0][row] - min_x) / (max_x - min_x);
          file_data[depth][1][row] =
              ranges.ymin + (ranges.ymax - ranges.ymin) * (file_data[depth][1][row] - min_y) / (max_y - min_y);
        }
      grm_args_push(args, "x", "nD", rows, file_data[depth][0].data());
      grm_args_push(args, "y", "nD", rows, file_data[depth][1].data());
    }
  if (!grm_args_values(args, "grplot", "i", &grplot)) grm_args_push(args, "grplot", "i", 1);
  grm_merge(args);

  if (handle != nullptr)
    {
      grm_send_args(handle, args);
      grm_close(handle);
    }

  delete file_args;
  return 1;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ input stream parser ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int convert_inputstream_into_args(grm_args_t *args, grm_file_args_t *file_args, int argc, char **argv,
                                  PlotRange *ranges)
{
  int i;
  std::string token, found_key;
  size_t found_key_size;
  std::string delim = ":";
  std::string kind = "line";
  std::string optional_file;

  for (i = 1; i < argc; i++)
    {
      token = argv[i];
      /* parameter needed for import.cxx are handled differently than grm-parameters */
      if (starts_with(token, "file:"))
        {
          file_args->file_path = token.substr(5, token.length() - 1);
        }
      else if (i == 1 && (token.find(delim) == std::string::npos || (token.find(delim) == 1 && token.find('/') == 2)))
        {
          optional_file = token; /* it's only used, if no "file:" keyword was found */
        }
      else if (starts_with(token, "columns:"))
        {
          file_args->file_columns = token.substr(8, token.length() - 1);
        }
      else if (starts_with(token, "x_columns:"))
        {
          file_args->file_x_columns = token.substr(10, token.length() - 1);
        }
      else if (starts_with(token, "y_columns:"))
        {
          file_args->file_y_columns = token.substr(10, token.length() - 1);
        }
      else if (starts_with(token, "error_columns:"))
        {
          file_args->file_error_columns = token.substr(14, token.length() - 1);
        }
      else
        {
          size_t pos = token.find(delim);
          if (pos != std::string::npos)
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
                      kind = token.substr(found_key_size + 1, token.length() - 1);
                    }

                  if (value.length() == 0)
                    {
                      fprintf(stderr, "Parameter %s will be ignored. No data given\n", search->first.c_str());
                      continue;
                    }

                  /* parameter is a container */
                  if (auto container_search = container_params.find(found_key);
                      container_search != container_params.end())
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
                          if (starts_with(arg, "{"))
                            {
                              arg = arg.substr(1, arg.length());
                            }
                          else if (ends_with(arg, "}"))
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
                                      (str_equals_any(con->second, "i", "d")))
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
                                          std::string x, y;
                                          parse_parameter_dd(&value, &search->first, &x, &y);
                                          grm_args_push(args, search->first.c_str(), search->second, std::stod(x),
                                                        std::stod(y));
                                          if (search->first == "x_range")
                                            {
                                              ranges->xmin = std::stod(x);
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
                                        }
                                      else if (strcmp(con->second, "ddd") == 0)
                                        {
                                          std::string r, g, b;
                                          parse_parameter_ddd(&container_value, &con->first, &r, &g, &b);
                                          grm_args_push(new_args[ind], con->first.c_str(), con->second, std::stod(r),
                                                        std::stod(g), std::stod(b));
                                        }
                                      else if ((pos_a = value.find(',')) != std::string::npos &&
                                               strcmp(con->second, "nI") == 0)
                                        {
                                          size_t con_pos = container_value.find(',');
                                          std::string param_num = container_value.substr(0, con_pos);
                                          std::vector<int> values(std::stoi(param_num));
                                          int no_err = parse_parameter_nI(&container_value, &con->first, &values);
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
                          grm_args_push(args, container_search->first.c_str(), container_search->second, num,
                                        new_args.data());
                        }
                      else if (num_of_parameter == 2 || strcmp(container_search->first.c_str(), "error") == 0)
                        {
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
                  if ((pos_a = value.find(',')) != std::string::npos && (str_equals_any(search->second, "i", "d", "s")))
                    {
                      if (search.operator++()->second != NULL) search = search.operator++();
                    }

                  /* different types */
                  if (strcmp(search->second, "s") == 0)
                    {
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
                                  scatter_with_z = std::stoi(value);
                                }
                              else if (strcmp(search->first.c_str(), "use_bins") == 0)
                                {
                                  use_bins = std::stoi(value);
                                  if (use_bins) xyz_file = 0;
                                }
                              else if (strcmp(search->first.c_str(), "equal_up_and_down_error") == 0)
                                {
                                  equal_up_and_down_error = std::stoi(value);
                                }
                              else if (strcmp(search->first.c_str(), "xye_file") == 0)
                                {
                                  xye_file = std::stoi(value);
                                }
                              else if (strcmp(search->first.c_str(), "xyz_file") == 0)
                                {
                                  xyz_file = std::stoi(value);
                                  if (xyz_file) use_bins = 0;
                                }
                              else
                                {
                                  grm_args_push(args, search->first.c_str(), search->second, std::stoi(value));
                                }
                            }
                          else if (strcmp(search->second, "d") == 0)
                            {
                              grm_args_push(args, search->first.c_str(), search->second, std::stod(value));
                            }
                          else if (strcmp(search->second, "dd") == 0)
                            {
                              std::string x, y;
                              parse_parameter_dd(&value, &search->first, &x, &y);
                              grm_args_push(args, search->first.c_str(), search->second, std::stod(x), std::stod(y));
                              if (search->first == "x_range")
                                {
                                  ranges->xmin = std::stod(x);
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
                            }
                          else if (strcmp(search->second, "ddd") == 0)
                            {
                              std::string r, g, b;
                              parse_parameter_ddd(&value, &search->first, &r, &g, &b);
                              grm_args_push(args, search->first.c_str(), search->second, std::stod(r), std::stod(g),
                                            std::stod(b));
                            }
                          else if (strcmp(search->second, "nS") == 0)
                            {
                              size_t pos = value.find(',');
                              std::string num = value.substr(0, pos);
                              std::vector<std::string> values(std::stoi(num));
                              std::vector<const char *> c_values(std::stoi(num));
                              int no_err = parse_parameter_nS(&value, &search->first, &values);
                              if (no_err)
                                {
                                  int ci;
                                  for (ci = 0; ci < std::stoi(num); ci++)
                                    {
                                      c_values[ci] = values[ci].c_str();
                                    }
                                  grm_args_push(args, search->first.c_str(), search->second, std::stoi(num),
                                                c_values.data());
                                }
                            }
                          else if (strcmp(search->second, "nD") == 0)
                            {
                              size_t pos = value.find(',');
                              std::string num = value.substr(0, pos);
                              std::vector<double> values(std::stoi(num));
                              int no_err = parse_parameter_nD(&value, &search->first, &values);
                              if (no_err)
                                {
                                  grm_args_push(args, search->first.c_str(), search->second, std::stoi(num),
                                                values.data());
                                }
                            }
                        }
                      catch (std::invalid_argument &e)
                        {
                          fprintf(stderr, "Invalid argument for %s parameter (%s).\n", search->first.c_str(),
                                  value.c_str());
                        }
                    }
                }
              else
                {
                  fprintf(stderr, "Unknown key:value pair in parameters (%s)\n", token.c_str());
                }
            }
        }
    }

  /* errors that can be caught */
  if (file_args->file_path.empty())
    {
      if (!optional_file.empty())
        {
          file_args->file_path = optional_file;
        }
      else
        {
          fprintf(stderr, "Missing input file name\n");
          return 0;
        }
    }
  if (!(std::find(kind_types.begin(), kind_types.end(), kind) != kind_types.end()))
    {
      fprintf(stderr, "Invalid plot type (%s) - fallback to line plot\n", kind.c_str());
      kind = "line";
    }
  grm_args_push(args, "kind", "s", kind.c_str());
  return 1;
}

void parse_parameter_dd(std::string *input, const std::string *key, std::string *x, std::string *y)
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

void parse_parameter_ddd(std::string *input, const std::string *key, std::string *r, std::string *g, std::string *b)
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

int parse_parameter_nI(std::string *input, const std::string *key, std::vector<int> *values)
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

int parse_parameter_nS(std::string *input, const std::string *key, std::vector<std::string> *values)
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

int parse_parameter_nD(std::string *input, const std::string *key, std::vector<double> *values)
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
