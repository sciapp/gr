/* ######################### includes ############################################################################### */

#include "error_int.h"
#include "import_int.hxx"
#include "util_int.h"
#include "utilcpp_int.hxx"
#include "grm/dom_render/graphics_tree/util.hxx"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <list>

#include "import_plugins/vdr_int.hxx"

/* ========================= static variables ======================================================================= */

/* ------------------------- reader --------------------------------------------------------------------------------- */

/* Intentionally use a raw pointer instead of a smart pointer here.
 * Using a smart pointer would cause an automatic cleanup on program exit, however then we have no control over the
 * actual object destruction order, potentially causing segfaults. Use an explicit cleanup function instead, which is
 * called from `grm_finalize`.
 */
static Vdr *reader = nullptr;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ depending all plots ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static std::list<int> join_plot_numbers;
static int consecutive_colorbars = 0;

/* ========================= functions ============================================================================== */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ general ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void cleanupImportModule(void)
{
  delete reader;
}

/* ------------------------- import --------------------------------------------------------------------------------- */

grm_error_t readDataFile(const std::string &path, std::vector<std::vector<std::vector<double>>> &data,
                         std::vector<int> &x_data, std::vector<int> &y_data, std::vector<int> &error_data,
                         std::vector<std::string> &labels, grm_args_t *args, const char *colms, const char *x_colms,
                         const char *y_colms, const char *e_colms, PlotRange *ranges,
                         grm_special_axis_series_t *special_axis_series, InputFlags &input_flags,
                         std::vector<int> &timestamps)
{
  if (!reader)
    {
      reader = new Vdr;
    }
  DataSource *source = reader->loadSourceForFile(path);

  if (source)
    {
      return source->readDataFile(path, data, x_data, y_data, error_data, labels, args, colms, x_colms, y_colms,
                                  e_colms, ranges, special_axis_series, input_flags, timestamps);
    }
  return GRM_ERROR_DATAREADER_UNKNOWN_FILETYPE;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ argument container ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

grm_file_args_t *grm_file_args_new()
{
  auto *args = new grm_file_args_t;
  if (args == nullptr)
    {
      debugPrintMallocError();
      return nullptr;
    }
  args->file_path = "";
  args->file_columns = "";
  args->file_x_columns = "";
  args->file_y_columns = "";
  args->file_error_columns = "";
  return args;
}

grm_special_axis_series_t *grm_special_axis_series_new()
{
  auto *args = new grm_special_axis_series_t;
  if (args == nullptr)
    {
      debugPrintMallocError();
      return nullptr;
    }
  args->bottom = "";
  args->left = "";
  args->right = "";
  args->top = "";
  args->twin_x = "";
  args->twin_y = "";
  return args;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ utils ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void adjustRanges(double *range_min, double *range_max, double default_value_min, double default_value_max)
{
  *range_min = (*range_min == INFINITY) ? default_value_min : grm_min(*range_min, default_value_min);
  *range_max = (*range_max == INFINITY) ? default_value_max : grm_max(*range_max, default_value_max);
}

void setSeriesLocation(std::vector<grm_args_t *> series, int idx, std::list<int> bottom_series,
                       std::list<int> left_series, std::list<int> right_series, std::list<int> top_series,
                       std::list<int> twin_x_series, std::list<int> twin_y_series)
{
  std::string x_location, y_location;
  if (std::find(bottom_series.begin(), bottom_series.end(), idx) != bottom_series.end()) x_location = "bottom";
  if (std::find(left_series.begin(), left_series.end(), idx) != left_series.end()) y_location = "left";
  if (std::find(right_series.begin(), right_series.end(), idx) != right_series.end()) y_location = "right";
  if (std::find(top_series.begin(), top_series.end(), idx) != top_series.end()) x_location = "top";
  if (std::find(twin_x_series.begin(), twin_x_series.end(), idx) != twin_x_series.end()) x_location = "twin_x";
  if (std::find(twin_y_series.begin(), twin_y_series.end(), idx) != twin_y_series.end()) y_location = "twin_y";
  if (!x_location.empty()) grm_args_push(series[idx], "ref_x_axis_location", "s", x_location.c_str());
  if (!y_location.empty()) grm_args_push(series[idx], "ref_y_axis_location", "s", y_location.c_str());
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
  size_t row, col, rows, cols, depth, plot_num;
  std::vector<std::vector<std::vector<double>>> file_data;
  std::vector<int> x_data, y_data, error_data, plot_idx;
  std::list<int> bottom_series, left_series, right_series, top_series, twin_x_series, twin_y_series;
  std::vector<grm_args_t *> series;
  char *env, *wstype;
  void *handle = nullptr;
  const char *kind;
  int use_grplot_changes, divisor = 1;
  grm_file_args_t *file_args = grm_file_args_new();
  grm_special_axis_series_t *special_axis_series = grm_special_axis_series_new();
  InputFlags input_flags;
  std::vector<int> timestamps;

  for (int i = 1; i < argc; i++)
    {
      if (i == 1 && strcmp(argv[i], "--plot") != 0) plot_idx.push_back(i - 1);
      if (strcmp(argv[i], "--plot") == 0) plot_idx.push_back(i);
    }
  plot_num = plot_idx.size();
  for (int i = 2; i <= plot_num; i++)
    {
      if (plot_num % i == 0)
        {
          divisor = i;
          break;
        }
    }
  if (plot_num >= 4) divisor = 2;
  if (plot_num >= 9) divisor = 3;
  if (plot_num >= 16) divisor = 4;

  plot_idx.push_back(argc);
  std::vector<grm_args_t *> plot(plot_num);
  for (int plot_i = 0; plot_i < plot_num; plot_i++)
    {
      timestamps.clear();
      input_flags.reset();

      std::vector<std::string> labels;
      std::vector<const char *> labels_c;
      std::vector<char *> tmp;
      unsigned int kinds_length = 0;
      char **kinds;
      PlotRange ranges = {INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY};
      auto start = plot_idx[plot_i];
      auto end = plot_idx[plot_i + 1];
      for (int j = start; j < end; j++) tmp.push_back(argv[j]);

      file_data.clear();
      series.clear();
      y_data.clear();
      x_data.clear();
      error_data.clear();
      file_args = grm_file_args_new();
      plot[plot_i] = grm_args_new();
      input_flags.default_kind_used = false;
      if (!convertInputstreamIntoArgs(plot[plot_i], file_args, end - start, reinterpret_cast<char **>(tmp.data()),
                                      &ranges, special_axis_series, input_flags))
        return 0;

      if (file_args->file_path != "-" && !fileExists(file_args->file_path))
        {
          fprintf(stderr, "File not found or missing (%s)\n", file_args->file_path.c_str());
          return 0;
        }

      grm_args_first_value(plot[plot_i], "kind", "nS", &kinds, &kinds_length);
      kind = kinds[0]; // since all kinds are of the same group its okay to just use the first kind here
      if (plot_num == 1) grm_args_push(args, "kind", "s", kind);
      if (!strEqualsAny(kind, "barplot", "histogram", "line", "scatter", "stairs", "stem"))
        {
          file_args->file_x_columns.clear();
          file_args->file_y_columns.clear();
          file_args->file_error_columns.clear();
          input_flags.xye_file = 0;
        }
      if (input_flags.xye_file)
        {
          file_args->file_x_columns.clear();
          file_args->file_x_columns = "1";
          file_args->file_y_columns.clear();
          file_args->file_y_columns = "2";
          file_args->file_error_columns.clear();
          file_args->file_error_columns = input_flags.equal_up_and_down_error ? "3" : "3,4";
        }
      if (readDataFile(file_args->file_path, file_data, x_data, y_data, error_data, labels, plot[plot_i],
                       file_args->file_columns.c_str(), file_args->file_x_columns.c_str(),
                       file_args->file_y_columns.c_str(), file_args->file_error_columns.c_str(), &ranges,
                       special_axis_series, input_flags, timestamps))
        {
          return 0;
        }

      // convert grm_special_axis_series_t entries into int vector
      if (parseColumns(&bottom_series, special_axis_series->bottom.c_str()) != GRM_ERROR_NONE) return 0;
      if (parseColumns(&left_series, special_axis_series->left.c_str()) != GRM_ERROR_NONE) return 0;
      if (parseColumns(&right_series, special_axis_series->right.c_str()) != GRM_ERROR_NONE) return 0;
      if (parseColumns(&top_series, special_axis_series->top.c_str()) != GRM_ERROR_NONE) return 0;
      if (parseColumns(&twin_x_series, special_axis_series->twin_x.c_str()) != GRM_ERROR_NONE) return 0;
      if (parseColumns(&twin_y_series, special_axis_series->twin_y.c_str()) != GRM_ERROR_NONE) return 0;

      if (!file_data.empty())
        {
          depth = file_data.size();
          cols = file_data[0].size();
          if (strEqualsAny(kind, "barplot", "histogram", "line", "scatter", "stairs", "stem"))
            {
              cols -= x_data.size() + error_data.size(); // less y columns if x or error data given
            }
          else if (strEqualsAny(kind, "histogram"))
            {
              cols -= y_data.size() + error_data.size(); // less x columns if y or error data given
            }
          rows = file_data[0][0].size();
          depth = (depth == 1) ? 0 : depth;
        }
      else
        {
          fprintf(stderr, "File is empty\n");
          return 0;
        }
      if (cols + x_data.size() + error_data.size() != labels.size())
        {
          fprintf(stderr, "The number of columns (%zu) doesn't fit the number of labels (%zu)\n",
                  cols + x_data.size() + error_data.size(), labels.size());
        }
      if (kinds_length > 0 && !error_data.empty())
        {
          for (int i = 1; i < kinds_length; i++)
            {
              if (strcmp(kinds[i], kind) != 0)
                {
                  fprintf(stderr,
                          "Different kinds are only possible if no error data is given. Use kind %s for all series.\n",
                          kind);
                  grm_args_push(plot[plot_i], "kind", "s", kind);
                  kinds = const_cast<char **>(&kind);
                }
            }
        }

      series.resize(cols);

      wstype = getenv("GKS_WSTYPE");
      if (wstype != nullptr && strcmp(wstype, "381") == 0 && (env = getenv("GR_DISPLAY")) != nullptr)
        {
          handle = grm_open(GRM_SENDER, env, 8002, nullptr, nullptr);
          if (handle == nullptr) fprintf(stderr, "GRM connection to '%s' could not be established\n", env);
        }

      if ((strcmp(kind, "line") == 0 || (strcmp(kind, "scatter") == 0 && !input_flags.scatter_with_z)) &&
          ((rows >= 50 && cols >= 50) || (input_flags.use_bins && rows >= 49 && cols >= 49)))
        {
          fprintf(stderr, "Too much data for %s plot - use heatmap instead\n", kind);
          kind = "heatmap";
          grm_args_push(plot[plot_i], "kind", "s", kind);
          kinds = const_cast<char **>(&kind);
        }
      if (!strEqualsAny(kind, "isosurface", "quiver", "volume") && depth >= 1)
        {
          fprintf(stderr, "Too much data for %s plot - use volume instead\n", kind);
          kind = "volume";
          grm_args_push(plot[plot_i], "kind", "s", kind);
          kinds = const_cast<char **>(&kind);
        }

      if (!strEqualsAny(kind, "contour", "contourf", "heatmap", "imshow", "marginal_heatmap", "surface", "wireframe"))
        {
          // these parameters are only for surface and similar types
          input_flags.use_bins = 0;
          input_flags.xyz_file = 0;
        }

      if (strEqualsAny(kind, "contour", "contourf", "heatmap", "imshow", "marginal_heatmap", "surface", "wireframe"))
        {
          int x_dim = cols, y_dim = rows, z_dim = rows * cols;
          double xmin, xmax, ymin, ymax;
          if (cols <= 1 || (input_flags.xyz_file && cols < 3))
            {
              fprintf(stderr, "Insufficient data for plot type (%s)\n", kind);
              return 0;
            }
          if (input_flags.xyz_file)
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

          if (!grm_args_values(plot[plot_i], "x_range", "dd", &xmin, &xmax))
            {
              xmin = 0.0;
              xmax = x_dim - 1.0;
            }
          adjustRanges(&ranges.xmin, &ranges.xmax, xmin, xmax);
          if (!grm_args_values(plot[plot_i], "y_range", "dd", &ymin, &ymax))
            {
              ymin = 0.0;
              ymax = y_dim - 1.0;
            }
          if (!input_flags.use_bins)
            {
              adjustRanges(&ranges.ymin, &ranges.ymax, ymin, ymax);
            }

          if (input_flags.use_bins)
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
          ranges.ymax = (ranges.ymax <= ranges.ymin) ? ranges.ymax + ranges.ymin : ranges.ymax;

          if (input_flags.xyz_file)
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
                  xi[col] = ranges.xmin + (ranges.xmax - ranges.xmin) * ((double)col / ((double)x_dim - 1));
                  for (row = 0; row < rows; ++row)
                    {
                      if (col == 0)
                        {
                          yi[row] = ranges.ymin + (ranges.ymax - ranges.ymin) * ((double)row / ((double)y_dim - 1));
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
          grm_args_push(plot[plot_i], "c", "nD", z_dim, zi.data());
          grm_args_push(plot[plot_i], "c_dims", "ii", x_dim, y_dim);

          grm_args_push(plot[plot_i], "x", "nD", x_dim, xi.data());
          grm_args_push(plot[plot_i], "y", "nD", y_dim, yi.data());
          grm_args_push(plot[plot_i], "z", "nD", z_dim, zi.data());
        }
      else if (strcmp(kind, "line") == 0 || (strcmp(kind, "scatter") == 0 && !input_flags.scatter_with_z))
        {
          grm_args_t *error = nullptr;
          std::vector<grm_args_t *> error_vec;
          std::vector<double> x(rows);
          std::vector<int> filtered_error_columns;
          int err = 0, col_group_elem = 3, down_err_off = 2;
          const char *spec;
          int series_num;
          int y_cnt = 0, x_cnt = 0, err_cnt = 0;
          double xmin, xmax;
          double marker_size;
          int marker_type;

          if (!grm_args_values(plot[plot_i], "x_range", "dd", &xmin, &xmax))
            {
              xmin = 0.0;
              xmax = rows - 1.0;
            }
          adjustRanges(&ranges.xmin, &ranges.xmax, xmin, xmax);
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
          if (grm_args_values(plot[plot_i], "error", "a", &error) || input_flags.xye_file ||
              input_flags.equal_up_and_down_error)
            {
              if (error == nullptr)
                {
                  error = grm_args_new();
                  grm_args_push(plot[plot_i], "error", "a", error);
                }

              if (input_flags.equal_up_and_down_error)
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
                  if (std::find(x_data.begin(), x_data.end(), col + 1) != x_data.end()) continue;
                  if (std::find(error_data.begin(), error_data.end(), col + 1) != error_data.end()) continue;
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
                  if (std::find(x_data.begin(), x_data.end(), col + 1) != x_data.end()) continue;
                  if (std::find(error_data.begin(), error_data.end(), col + 1) != error_data.end()) continue;
                  for (row = 0; row < rows; row++)
                    {
                      file_data[depth][col][row] = ranges.ymin + (ranges.ymax - ranges.ymin) *
                                                                     (file_data[depth][col][row] - min_val) /
                                                                     (max_val - min_val);
                    }
                }
            }

          // calculate the error data
          if (grm_args_values(plot[plot_i], "error", "a", &error))
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
                          grm_args_push(error_vec[col], input_flags.error_type.c_str(), "nDD", rows, errors_up.data(),
                                        errors_down.data());
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
                      error_vec.resize(input_flags.equal_up_and_down_error ? error_data.size() : error_data.size() / 2);
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
                              if (input_flags.equal_up_and_down_error)
                                {
                                  errors_up[i] = file_data[depth][error_col - 1][i];
                                  errors_down[i] = file_data[depth][error_col - 1][i];
                                }
                              else if (cnt % 2 == 0)
                                {
                                  errors_up[i] = file_data[depth][error_col - 1][i];
                                }
                              else if (cnt % 2 != 0)
                                {
                                  errors_down[i] = file_data[depth][error_col - 1][i];
                                }
                            }
                          if (cnt % 2 != 0 || input_flags.equal_up_and_down_error)
                            {
                              grm_args_push(error_vec[floor(cnt / (input_flags.equal_up_and_down_error ? 1 : 2))],
                                            input_flags.error_type.c_str(), "nDD", rows, errors_up.data(),
                                            errors_down.data());
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

          if (!x_data.empty() || !error_data.empty())
            {
              series_num = y_data.size();
              for (col = 0; col < series_num; col++)
                {
                  series[col] = grm_args_new();
                  setSeriesLocation(series, col, bottom_series, left_series, right_series, top_series, twin_x_series,
                                    twin_y_series);
                  grm_args_push(series[col], "series_kind", "s",
                                col < kinds_length ? kinds[col] : kinds[kinds_length - 1]);
                }
            }
          else
            {
              series_num = cols - err;
            }
          for (col = 0; col < cols - err; col++)
            {
              if (x_data.empty() && y_data.empty() && error_data.empty())
                {
                  series[col] = grm_args_new();
                  setSeriesLocation(series, col, bottom_series, left_series, right_series, top_series, twin_x_series,
                                    twin_y_series);
                  grm_args_push(series[col], "series_kind", "s",
                                col < kinds_length ? kinds[col] : kinds[kinds_length - 1]);
                  grm_args_push(series[col], "x", "nD", rows, x.data());
                  grm_args_push(series[col], "y", "nD", rows,
                                file_data[depth][col + ((col < err / down_err_off) ? col * down_err_off : err)].data());
                  if (col < err / down_err_off)
                    {
                      int error_bar_style;
                      grm_args_push(series[col], "error", "a", error_vec[col]);
                      if (grm_args_values(plot[plot_i], "error_bar_style", "i", &error_bar_style))
                        grm_args_push(series[col], "error_bar_style", "i", error_bar_style);
                    }
                  if (!labels.empty() && labels.size() > col && !labels[col].empty())
                    grm_args_push(series[col], "label", "s", labels[col].c_str());
                  if (grm_args_values(plot[plot_i], "line_spec", "s", &spec))
                    grm_args_push(series[col], "line_spec", "s", spec);
                  if (grm_args_values(plot[plot_i], "marker_type", "i", &marker_type))
                    grm_args_push(series[col], "marker_type", "i", marker_type);
                  if (grm_args_values(plot[plot_i], "marker_size", "d", &marker_size))
                    grm_args_push(series[col], "marker_size", "d", marker_size);
                }
              else
                {
                  bool timestamp = true;
                  int keep_aspect_ratio = 1;
                  for (const auto x_col : x_data)
                    {
                      if (std::find(timestamps.begin(), timestamps.end(), x_col - 1) == timestamps.end())
                        timestamp = false;
                    }
                  grm_args_push(plot[plot_i], "timestamp", "i", timestamp);
                  if (timestamp && !grm_args_values(plot[plot_i], "keep_aspect_ratio", "i", &keep_aspect_ratio))
                    grm_args_push(plot[plot_i], "keep_aspect_ratio", "i", 0);
                  if (std::find(x_data.begin(), x_data.end(), col + 1) != x_data.end())
                    {
                      int error_bar_style;
                      grm_args_push(series[x_cnt], "x", "nD", rows, file_data[depth][col].data());
                      if (x_data.size() == 1)
                        {
                          // special case: if only one x-column is given use it for all y-columns
                          for (int k = 1; k < y_data.size(); k++)
                            {
                              grm_args_push(series[k], "x", "nD", rows, file_data[depth][col].data());
                            }
                        }
                      if (grm_args_values(plot[plot_i], "error_bar_style", "i", &error_bar_style))
                        grm_args_push(series[x_cnt], "error_bar_style", "i", error_bar_style);
                      x_cnt += 1;
                    }
                  else if (std::find(y_data.begin(), y_data.end(), col + 1) != y_data.end())
                    {
                      grm_args_push(series[y_cnt], "y", "nD", rows, file_data[depth][col].data());
                      if (!labels.empty() && labels.size() > col && !labels[col].empty())
                        grm_args_push(series[y_cnt], "label", "s", labels[col].c_str());
                      if (grm_args_values(plot[plot_i], "line_spec", "s", &spec))
                        grm_args_push(series[y_cnt], "line_spec", "s", spec);
                      else if (timestamp && strcmp(kind, "line") == 0)
                        grm_args_push(series[y_cnt], "line_spec", "s", "-+");
                      if (grm_args_values(plot[plot_i], "marker_type", "i", &marker_type))
                        grm_args_push(series[y_cnt], "marker_type", "i", marker_type);
                      if (grm_args_values(plot[plot_i], "marker_size", "d", &marker_size))
                        grm_args_push(series[y_cnt], "marker_size", "d", marker_size);
                      y_cnt += 1;
                    }
                  else if (!input_flags.equal_up_and_down_error && error != nullptr &&
                           std::find(filtered_error_columns.begin(), filtered_error_columns.end(), col + 1) ==
                               filtered_error_columns.end())
                    {
                      grm_args_push(series[err_cnt], "error", "a", error_vec[err_cnt]);
                      err_cnt += 1;
                    }
                  else if (input_flags.equal_up_and_down_error &&
                           std::find(error_data.begin(), error_data.end(), col + 1) != error_data.end())
                    {
                      grm_args_push(series[err_cnt], "error", "a", error_vec[err_cnt]);
                      err_cnt += 1;
                    }
                }
            }
          cols -= x_data.size() + error_data.size();
          grm_args_push(plot[plot_i], "series", "nA", series_num, series.data());
        }
      else if (strEqualsAny(kind, "isosurface", "volume"))
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

          grm_args_push(plot[plot_i], "c", "nD", n, data.data());
          grm_args_push(plot[plot_i], "c_dims", "nI", 3, dims.data());
        }
      else if (strEqualsAny(kind, "line3", "scatter3", "tricontour", "trisurface") ||
               (strcmp(kind, "scatter") == 0 && input_flags.scatter_with_z))
        {
          double min_x, max_x, min_y, max_y, min_z, max_z;
          double xmin, xmax, ymin, ymax, zmin, zmax;
          if (cols < 3)
            {
              fprintf(stderr, "Insufficient data for plot type (%s)\n", kind);
              return 0;
            }
          // Todo: add multiple column/data file support
          if (cols > 3) fprintf(stderr, "Only the first 3 columns get displayed\n");

          /* apply the ranges to the data */
          if (ranges.xmax != INFINITY)
            {
              min_x = *std::min_element(std::begin(file_data[depth][0]), std::end(file_data[depth][0]));
              max_x = *std::max_element(std::begin(file_data[depth][0]), std::end(file_data[depth][0]));
              if (!grm_args_values(plot[plot_i], "x_range", "dd", &xmin, &xmax))
                {
                  xmin = min_x;
                  xmax = max_x;
                }
              adjustRanges(&ranges.xmin, &ranges.xmax, xmin, xmax);
            }
          if (ranges.ymax != INFINITY)
            {
              min_y = *std::min_element(std::begin(file_data[depth][1]), std::end(file_data[depth][1]));
              max_y = *std::max_element(std::begin(file_data[depth][1]), std::end(file_data[depth][1]));
              if (!grm_args_values(plot[plot_i], "y_range", "dd", &ymin, &ymax))
                {
                  ymin = min_y;
                  ymax = max_y;
                }
              adjustRanges(&ranges.ymin, &ranges.ymax, ymin, ymax);
              ranges.ymax = (ranges.ymax <= ranges.ymin) ? ranges.ymax + ranges.ymin : ranges.ymax;
            }
          if (ranges.zmax != INFINITY)
            {
              min_z = *std::min_element(std::begin(file_data[depth][2]), std::end(file_data[depth][2]));
              max_z = *std::max_element(std::begin(file_data[depth][2]), std::end(file_data[depth][2]));
              if (!grm_args_values(plot[plot_i], "z_range", "dd", &zmin, &zmax))
                {
                  zmin = min_z;
                  zmax = max_z;
                }
              adjustRanges(&ranges.zmin, &ranges.zmax, zmin, zmax);
              ranges.zmax = (ranges.zmax <= ranges.zmin) ? ranges.zmax + ranges.zmin : ranges.zmax;
            }
          for (row = 0; row < rows; ++row)
            {
              if (ranges.xmax != INFINITY)
                file_data[depth][0][row] = ranges.xmin + (ranges.xmax - ranges.xmin) *
                                                             (((double)file_data[depth][0][row]) - min_x) /
                                                             (max_x - min_x);
              if (ranges.ymax != INFINITY)
                file_data[depth][1][row] = ranges.ymin + (ranges.ymax - ranges.ymin) *
                                                             (((double)file_data[depth][1][row]) - min_y) /
                                                             (max_y - min_y);
              if (ranges.zmax != INFINITY)
                file_data[depth][2][row] = ranges.zmin + (ranges.zmax - ranges.zmin) *
                                                             (((double)file_data[depth][2][row]) - min_z) /
                                                             (max_z - min_z);
            }

          grm_args_push(plot[plot_i], "x", "nD", rows, file_data[depth][0].data());
          grm_args_push(plot[plot_i], "y", "nD", rows, file_data[depth][1].data());
          grm_args_push(plot[plot_i], "z", "nD", rows, file_data[depth][2].data());
          if (!labels.empty() && !labels[0].empty()) grm_args_push(plot[plot_i], "label", "s", labels[0].c_str());
        }
      else if (strEqualsAny(kind, "barplot", "stem", "stairs"))
        {
          std::vector<double> x(rows);
          double xmin, xmax, ymin, ymax;
          grm_args_t *error = nullptr;
          const char *spec;
          std::vector<grm_args_t *> error_vec;
          std::vector<int> filtered_error_columns;
          int err = 0, col_group_elem = 3, down_err_off = 2;
          int series_num = 0;
          int err_cnt = 0, y_cnt = 0, x_cnt = 0;
          if (!grm_args_values(plot[plot_i], "x_range", "dd", &xmin, &xmax))
            {
              if (strcmp(kind, "barplot") == 0)
                {
                  xmin = 1;
                  xmax = rows;
                }
              else
                {
                  xmin = 0.0;
                  xmax = rows - 1.0;
                }
            }
          adjustRanges(&ranges.xmin, &ranges.xmax, xmin, xmax);
          cols += x_data.size() + error_data.size();

          // precalculate the number of error columns, so they can be removed from the y-data in the following step
          if (strEqualsAny(kind, "barplot") && (grm_args_values(plot[plot_i], "error", "a", &error) ||
                                                input_flags.xye_file || input_flags.equal_up_and_down_error))
            {
              if (error == nullptr)
                {
                  error = grm_args_new();
                  grm_args_push(plot[plot_i], "error", "a", error);
                }

              if (input_flags.equal_up_and_down_error)
                {
                  col_group_elem -= 1;
                  down_err_off -= 1;
                }
              err = floor(cols / col_group_elem) * down_err_off;
            }

          /* the needed calculation to get the errorbars out of the data */
          if (grm_args_values(plot[plot_i], "error", "a", &error) || input_flags.xye_file ||
              input_flags.equal_up_and_down_error)
            {
              int nbins, i;
              int color_up, color_down, color;
              std::vector<double> errors_up(rows);
              std::vector<double> errors_down(rows);
              std::vector<double> bins;

              if ((cols < col_group_elem && error_data.empty()) ||
                  (!error_data.empty() && error_data.size() < down_err_off))
                {
                  fprintf(stderr, "Not enough data for error parameter\n");
                }
              else if (strEqualsAny(kind, "barplot"))
                {
                  nbins = (int)rows;
                  if (nbins <= rows)
                    {
                      if (error_data.empty())
                        {
                          err = floor(cols / col_group_elem);
                          error_vec.resize(err);
                          for (col = 0; col < err; col++)
                            {
                              error_vec[col] = grm_args_new();
                              for (i = 0; i < nbins; i++)
                                {
                                  errors_up[i] = file_data[depth][col + 1 + col * down_err_off][i];
                                  errors_down[i] = file_data[depth][col + down_err_off + col * down_err_off][i];
                                }
                              grm_args_push(error_vec[col], input_flags.error_type.c_str(), "nDD", nbins,
                                            errors_up.data(), errors_down.data());
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
                          error_vec.resize(input_flags.equal_up_and_down_error ? error_data.size()
                                                                               : error_data.size() / 2);
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
                              for (i = 0; i < nbins; i++)
                                {
                                  if (input_flags.equal_up_and_down_error)
                                    {
                                      errors_up[i] = file_data[depth][error_col - 1][i];
                                      errors_down[i] = file_data[depth][error_col - 1][i];
                                    }
                                  else if (cnt % 2 == 0)
                                    {
                                      errors_up[i] = file_data[depth][error_col - 1][i];
                                    }
                                  else if (cnt % 2 != 0)
                                    {
                                      errors_down[i] = file_data[depth][error_col - 1][i];
                                    }
                                }
                              if (cnt % 2 != 0 || input_flags.equal_up_and_down_error)
                                {
                                  grm_args_push(error_vec[floor(cnt / (input_flags.equal_up_and_down_error ? 1 : 2))],
                                                input_flags.error_type.c_str(), "nDD", nbins, errors_up.data(),
                                                errors_down.data());
                                }
                              else
                                {
                                  filtered_error_columns.push_back(error_col);
                                }
                              cnt += 1;
                            }
                        }
                      grm_args_push(error, input_flags.error_type.c_str(), "nDD", nbins, errors_up.data(),
                                    errors_down.data());
                    }
                  else
                    {
                      fprintf(stderr, "Not enough data for error parameter\n");
                    }
                }
            }

          series_num = (!x_data.empty() || !error_data.empty()) ? y_data.size() : cols - err;
          for (col = 0; col < series_num; col++)
            {
              series[col] = grm_args_new();
              setSeriesLocation(series, col, bottom_series, left_series, right_series, top_series, twin_x_series,
                                twin_y_series);
              grm_args_push(series[col], "series_kind", "s", col < kinds_length ? kinds[col] : kinds[kinds_length - 1]);
            }

          // find min and max value of all x-data and make sure the all data points are inside that range
          if (x_data.empty())
            {
              for (row = 0; row < rows; row++)
                {
                  x[row] = ranges.xmin + (ranges.xmax - ranges.xmin) * ((double)row / ((double)rows - 1));
                }
              const char *style;
              if (strcmp(kind, "barplot") != 0 ||
                  (!grm_args_values(plot[plot_i], "style", "s", &style) || strcmp(style, "default") == 0))
                {
                  for (col = 0; col < series_num; col++)
                    {
                      grm_args_push(series[col], "x_range", "dd", ranges.xmin, ranges.xmax);
                    }
                }
            }
          else
            {
              const char *style;
              if (!grm_args_values(plot[plot_i], "x_range", "dd", &xmin, &xmax) &&
                  (strcmp(kind, "barplot") != 0 ||
                   (!grm_args_values(plot[plot_i], "style", "s", &style) || strcmp(style, "default") == 0)))
                {
                  double x_min = INFINITY, x_max = -INFINITY;
                  for (col = 0; col < x_data.size(); col++)
                    {
                      xmin = *std::min_element(std::begin(file_data[depth][x_data[col] - 1]),
                                               std::end(file_data[depth][x_data[col] - 1]));
                      xmax = *std::max_element(std::begin(file_data[depth][x_data[col] - 1]),
                                               std::end(file_data[depth][x_data[col] - 1]));
                      x_min = grm_min(x_min, xmin);
                      x_max = grm_max(x_max, xmax);
                      if (!x_data.empty() && col < series_num) grm_args_push(series[col], "x_range", "dd", xmin, xmax);
                    }
                  adjustRanges(&ranges.xmin, &ranges.xmax, x_min, x_max);
                }
              else
                {
                  for (col = 0; col < series_num; ++col)
                    {
                      grm_args_push(series[col], "x_range", "dd", xmin, xmax);
                    }
                  xmin = INFINITY;
                  xmax = -INFINITY;
                  for (col = 0; col < x_data.size(); ++col)
                    {
                      xmin = grm_min(xmin, *std::min_element(std::begin(file_data[depth][x_data[col] - 1]),
                                                             std::end(file_data[depth][x_data[col] - 1])));
                      xmax = grm_max(xmax, *std::max_element(std::begin(file_data[depth][x_data[col] - 1]),
                                                             std::end(file_data[depth][x_data[col] - 1])));
                    }
                  for (col = 0; col < x_data.size(); ++col)
                    {
                      for (row = 0; row < rows; row++)
                        {
                          file_data[depth][x_data[col] - 1][row] =
                              ranges.xmin + (ranges.xmax - ranges.xmin) / (xmax - xmin) *
                                                ((double)file_data[depth][x_data[col] - 1][row] - xmin);
                        }
                    }
                  // special case for barplot cause the x-values and bar_width gets calculated via
                  // x_range_min and max; without the following code block all series will always have the same x and
                  // same width
                  for (col = 0; col < x_data.size(); ++col)
                    {
                      if (strcmp(col < kinds_length ? kinds[col] : kinds[kinds_length - 1], "barplot") == 0)
                        {
                          xmin = *std::min_element(std::begin(file_data[depth][x_data[col] - 1]),
                                                   std::end(file_data[depth][x_data[col] - 1]));
                          xmax = *std::max_element(std::begin(file_data[depth][x_data[col] - 1]),
                                                   std::end(file_data[depth][x_data[col] - 1]));
                          if (col < series_num) grm_args_push(series[col], "x_range", "dd", xmin, xmax);
                        }
                    }
                }
            }

          // find min and max value of all y-data and make sure the all data points are inside that range
          if (!grm_args_values(plot[plot_i], "y_range", "dd", &ymin, &ymax))
            {
              double y_min = INFINITY, y_max = -INFINITY;
              int tmp_cnt = 0;
              for (col = 0; col < cols - err; col++)
                {
                  if (strcmp(kind, "stairs") == 0)
                    {
                      ymin = *std::min_element(
                          std::begin(file_data[depth][col + ((col < err / down_err_off) ? col * down_err_off : err)]),
                          std::end(file_data[depth][col + ((col < err / down_err_off) ? col * down_err_off : err)]));
                    }
                  else
                    {
                      ymin = grm_min(
                          0, *std::min_element(
                                 std::begin(
                                     file_data[depth][col + ((col < err / down_err_off) ? col * down_err_off : err)]),
                                 std::end(
                                     file_data[depth][col + ((col < err / down_err_off) ? col * down_err_off : err)])));
                    }
                  ymax = *std::max_element(
                      std::begin(file_data[depth][col + ((col < err / down_err_off) ? col * down_err_off : err)]),
                      std::end(file_data[depth][col + ((col < err / down_err_off) ? col * down_err_off : err)]));
                  y_min = grm_min(y_min, ymin);
                  y_max = grm_max(y_max, ymax);
                  if (cols - err == series_num)
                    grm_args_push(series[col], "y_range", "dd", ymin, ymax);
                  else if (std::find(y_data.begin(), y_data.end(), col + 1) != y_data.end())
                    grm_args_push(series[tmp_cnt++], "y_range", "dd", ymin, ymax);
                }
              adjustRanges(&ranges.ymin, &ranges.ymax, std::min<double>(0.0, y_min), y_max);
              grm_args_push(plot[plot_i], "y_line_pos", "d", 0.0);
            }
          else
            {
              /* apply y_range to the data */
              for (col = 0; col < series_num; ++col)
                {
                  grm_args_push(series[col], "y_range", "dd", ymin, ymax);
                }
              ymin = INFINITY;
              ymax = -INFINITY;
              for (col = 0; col < cols; ++col)
                {
                  if (std::find(x_data.begin(), x_data.end(), col + 1) != x_data.end()) continue;
                  if (std::find(error_data.begin(), error_data.end(), col + 1) != error_data.end()) continue;
                  ymin = grm_min(
                      ymin,
                      *std::min_element(
                          std::begin(file_data[depth][col + ((col < err / down_err_off) ? col * down_err_off : err)]),
                          std::end(file_data[depth][col + ((col < err / down_err_off) ? col * down_err_off : err)])));
                  ymax = grm_max(
                      ymax,
                      *std::max_element(
                          std::begin(file_data[depth][col + ((col < err / down_err_off) ? col * down_err_off : err)]),
                          std::end(file_data[depth][col + ((col < err / down_err_off) ? col * down_err_off : err)])));
                }
              int cnt = 0;
              for (col = 0; col < cols; ++col)
                {
                  if (std::find(x_data.begin(), x_data.end(), col + 1) != x_data.end()) continue;
                  if (std::find(error_data.begin(), error_data.end(), col + 1) != error_data.end()) continue;
                  if (strEqualsAny(cnt < kinds_length ? kinds[cnt] : kinds[kinds_length - 1], "barplot", "stem"))
                    ymin = grm_min(ymin, 0);
                  for (row = 0; row < rows; ++row)
                    {
                      file_data[depth][col][row] = ranges.ymin + (ranges.ymax - ranges.ymin) / (ymax - ymin) *
                                                                     ((double)file_data[depth][col][row] - ymin);
                    }
                  cnt += 1;
                }
              grm_args_push(plot[plot_i], "y_line_pos", "d",
                            ranges.ymin + (ranges.ymax - ranges.ymin) / (ymax - ymin) * (0.0 - ymin));
            }

          // push the data into the container structure
          for (col = 0; col < cols - err; col++)
            {
              if (x_data.empty() && y_data.empty() && error_data.empty())
                {
                  if (!labels.empty() && labels.size() > col && !labels[col].empty())
                    grm_args_push(series[col], "label", "s", labels[col].c_str());
                  grm_args_push(series[col], "x", "nD", rows, x.data());
                  /* for barplot */
                  grm_args_push(series[col], "y", "nD", rows,
                                file_data[depth][col + ((col < err / down_err_off) ? col * down_err_off : err)].data());
                  /* for stairs */
                  grm_args_push(series[col], "z", "nD", rows,
                                file_data[depth][col + ((col < err / down_err_off) ? col * down_err_off : err)].data());
                  if (grm_args_values(plot[plot_i], "line_spec", "s", &spec))
                    grm_args_push(series[col], "line_spec", "s", spec);
                  if (strEqualsAny(col < kinds_length ? kinds[col] : kinds[kinds_length - 1], "barplot") &&
                      series_num > 1)
                    {
                      const char *style;
                      if (!grm_args_values(plot[plot_i], "style", "s", &style) || strcmp(style, "default") == 0)
                        grm_args_push(series[col], "transparency", "d", 0.5);
                    }
                  if (col < err / down_err_off)
                    {
                      int error_bar_style;
                      grm_args_push(series[col], "error", "a", error_vec[col]);
                      if (grm_args_values(plot[plot_i], "error_bar_style", "i", &error_bar_style))
                        grm_args_push(series[err_cnt], "error_bar_style", "i", error_bar_style);
                    }
                }
              else
                {
                  bool timestamp = true;
                  int keep_aspect_ratio = 1;
                  for (const auto x_col : x_data)
                    {
                      if (std::find(timestamps.begin(), timestamps.end(), x_col - 1) == timestamps.end())
                        timestamp = false;
                    }
                  grm_args_push(plot[plot_i], "timestamp", "i", timestamp);
                  if (timestamp && !grm_args_values(plot[plot_i], "keep_aspect_ratio", "i", &keep_aspect_ratio))
                    grm_args_push(plot[plot_i], "keep_aspect_ratio", "i", 0);
                  if (std::find(x_data.begin(), x_data.end(), col + 1) != x_data.end())
                    {
                      int error_bar_style;
                      if (grm_args_values(plot[plot_i], "error_bar_style", "i", &error_bar_style))
                        grm_args_push(series[x_cnt], "error_bar_style", "i", error_bar_style);
                      grm_args_push(series[x_cnt], "x", "nD", rows, file_data[depth][col].data());
                      if (x_data.size() == 1)
                        {
                          // special case: if only one x-column is given use it for all y-columns
                          for (int k = 1; k < y_data.size(); k++)
                            {
                              grm_args_push(series[k], "x", "nD", rows, file_data[depth][col].data());
                            }
                        }
                      x_cnt += 1;
                    }
                  else if (std::find(y_data.begin(), y_data.end(), col + 1) != y_data.end())
                    {
                      if (!labels.empty() && labels.size() > col && !labels[col].empty())
                        grm_args_push(series[y_cnt], "label", "s", labels[col].c_str());
                      grm_args_push(series[y_cnt], "y", "nD", rows, file_data[depth][col].data());
                      /* for stairs */
                      grm_args_push(series[y_cnt], "z", "nD", rows, file_data[depth][col].data());
                      if (grm_args_values(plot[plot_i], "line_spec", "s", &spec))
                        grm_args_push(series[y_cnt], "line_spec", "s", spec);
                      if (strEqualsAny(y_cnt < kinds_length ? kinds[y_cnt] : kinds[kinds_length - 1], "barplot") &&
                          series_num > 1)
                        {
                          const char *style;
                          if (!grm_args_values(plot[plot_i], "style", "s", &style) || strcmp(style, "default") == 0)
                            grm_args_push(series[y_cnt], "transparency", "d", 0.5);
                        }
                      y_cnt += 1;
                    }
                  else if (!input_flags.equal_up_and_down_error &&
                           strEqualsAny(err_cnt < kinds_length ? kinds[err_cnt] : kinds[kinds_length - 1], "barplot") &&
                           error != nullptr &&
                           std::find(filtered_error_columns.begin(), filtered_error_columns.end(), col + 1) ==
                               filtered_error_columns.end())
                    {
                      grm_args_push(series[err_cnt], "error", "a", error_vec[err_cnt]);
                      err_cnt += 1;
                    }
                  else if (input_flags.equal_up_and_down_error &&
                           std::find(error_data.begin(), error_data.end(), col + 1) != error_data.end())
                    {
                      grm_args_push(series[err_cnt], "error", "a", error_vec[err_cnt]);
                      err_cnt += 1;
                    }
                }
            }
          cols -= x_data.size() + error_data.size();
          grm_args_push(plot[plot_i], "series", "nA", series_num, series.data());
        }
      else if (strEqualsAny(kind, "histogram"))
        {
          double xmin, xmax, ymin, ymax;
          grm_args_t *error = nullptr;
          const char *spec;
          std::vector<grm_args_t *> error_vec;
          std::vector<int> filtered_error_columns;
          int err = 0, col_group_elem = 3, down_err_off = 2;
          int series_num = 0;
          int err_cnt = 0, y_cnt = 0, x_cnt = 0;
          if (!grm_args_values(plot[plot_i], "x_range", "dd", &xmin, &xmax))
            {
              xmin = 0.0;
              xmax = rows - 1.0;
            }
          adjustRanges(&ranges.xmin, &ranges.xmax, xmin, xmax);
          cols += y_data.size() + error_data.size();

          // precalculate the number of error columns, so they can be removed from the y-data in the following step
          if ((grm_args_values(plot[plot_i], "error", "a", &error) || input_flags.xye_file ||
               input_flags.equal_up_and_down_error))
            {
              if (error == nullptr)
                {
                  error = grm_args_new();
                  grm_args_push(plot[plot_i], "error", "a", error);
                }

              if (input_flags.equal_up_and_down_error)
                {
                  col_group_elem -= 1;
                  down_err_off -= 1;
                }
              err = floor(cols / col_group_elem) * down_err_off;
            }

          /* the needed calculation to get the errorbars out of the data */
          if (grm_args_values(plot[plot_i], "error", "a", &error) || input_flags.xye_file ||
              input_flags.equal_up_and_down_error)
            {
              int nbins, i;
              int color_up, color_down, color;
              std::vector<double> errors_up(rows);
              std::vector<double> errors_down(rows);
              std::vector<double> bins;

              if ((cols < col_group_elem && error_data.empty()) ||
                  (!error_data.empty() && error_data.size() < down_err_off))
                {
                  fprintf(stderr, "Not enough data for error parameter\n");
                }
              else
                {
                  if (!grm_args_values(plot[plot_i], "num_bins", "i", &nbins))
                    {
                      if (!grm_args_values(plot[plot_i], "bins", "i", &nbins, &bins))
                        {
                          nbins = (int)(3.3 * log10((int)rows) + 0.5) + 1;
                        }
                    }
                  if (nbins <= rows)
                    {
                      if (error_data.empty())
                        {
                          err = floor(cols / col_group_elem);
                          error_vec.resize(err);
                          for (col = 0; col < err; col++)
                            {
                              error_vec[col] = grm_args_new();
                              for (i = 0; i < nbins; i++)
                                {
                                  errors_up[i] = file_data[depth][col + 1 + col * down_err_off][i];
                                  errors_down[i] = file_data[depth][col + down_err_off + col * down_err_off][i];
                                }
                              grm_args_push(error_vec[col], input_flags.error_type.c_str(), "nDD", nbins,
                                            errors_up.data(), errors_down.data());
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
                          error_vec.resize(input_flags.equal_up_and_down_error ? error_data.size()
                                                                               : error_data.size() / 2);
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
                              for (i = 0; i < nbins; i++)
                                {
                                  if (input_flags.equal_up_and_down_error)
                                    {
                                      errors_up[i] = file_data[depth][error_col - 1][i];
                                      errors_down[i] = file_data[depth][error_col - 1][i];
                                    }
                                  else if (cnt % 2 == 0)
                                    {
                                      errors_up[i] = file_data[depth][error_col - 1][i];
                                    }
                                  else if (cnt % 2 != 0)
                                    {
                                      errors_down[i] = file_data[depth][error_col - 1][i];
                                    }
                                }
                              if (cnt % 2 != 0 || input_flags.equal_up_and_down_error)
                                {
                                  grm_args_push(error_vec[floor(cnt / (input_flags.equal_up_and_down_error ? 1 : 2))],
                                                input_flags.error_type.c_str(), "nDD", nbins, errors_up.data(),
                                                errors_down.data());
                                }
                              else
                                {
                                  filtered_error_columns.push_back(error_col);
                                }
                              cnt += 1;
                            }
                        }
                      grm_args_push(error, input_flags.error_type.c_str(), "nDD", nbins, errors_up.data(),
                                    errors_down.data());
                    }
                  else
                    {
                      fprintf(stderr, "Not enough data for error parameter\n");
                    }
                }
            }

          series_num = (!y_data.empty() || !error_data.empty()) ? x_data.size() : cols - err;
          for (col = 0; col < series_num; col++)
            {
              series[col] = grm_args_new();
              setSeriesLocation(series, col, bottom_series, left_series, right_series, top_series, twin_x_series,
                                twin_y_series);
            }

          // find min and max value of all x-data and make sure the all data points are inside that range
          if (!grm_args_values(plot[plot_i], "x_range", "dd", &xmin, &xmax))
            {
              double x_min = INFINITY, x_max = -INFINITY;
              int tmp_cnt = 0;
              if (!x_data.empty())
                {
                  for (col = 0; col < x_data.size(); col++)
                    {
                      xmin = *std::min_element(std::begin(file_data[depth][x_data[col] - 1]),
                                               std::end(file_data[depth][x_data[col] - 1]));
                      xmax = *std::max_element(std::begin(file_data[depth][x_data[col] - 1]),
                                               std::end(file_data[depth][x_data[col] - 1]));
                      x_min = grm_min(x_min, xmin);
                      x_max = grm_max(x_max, xmax);
                      if (!x_data.empty() && col < series_num) grm_args_push(series[col], "x_range", "dd", xmin, xmax);
                    }
                }
              else
                {
                  for (col = 0; col < cols - err; col++)
                    {
                      xmin = grm_min(
                          0, *std::min_element(
                                 std::begin(
                                     file_data[depth][col + ((col < err / down_err_off) ? col * down_err_off : err)]),
                                 std::end(
                                     file_data[depth][col + ((col < err / down_err_off) ? col * down_err_off : err)])));
                      xmax = *std::max_element(
                          std::begin(file_data[depth][col + ((col < err / down_err_off) ? col * down_err_off : err)]),
                          std::end(file_data[depth][col + ((col < err / down_err_off) ? col * down_err_off : err)]));
                      x_min = grm_min(x_min, xmin);
                      x_max = grm_max(x_max, xmax);
                      if (cols - err == series_num)
                        grm_args_push(series[col], "x_range", "dd", xmin, xmax);
                      else if (std::find(x_data.begin(), x_data.end(), col + 1) != x_data.end())
                        grm_args_push(series[tmp_cnt++], "x_range", "dd", xmin, xmax);
                    }
                }
              adjustRanges(&ranges.xmin, &ranges.xmax, std::min<double>(0.0, x_min), x_max);
            }
          else
            {
              /* apply x_range to the data */
              for (col = 0; col < series_num; ++col)
                {
                  grm_args_push(series[col], "x_range", "dd", xmin, xmax);
                }
              xmin = INFINITY;
              xmax = -INFINITY;
              for (col = 0; col < cols; ++col)
                {
                  if (std::find(y_data.begin(), y_data.end(), col + 1) != y_data.end()) continue;
                  if (std::find(error_data.begin(), error_data.end(), col + 1) != error_data.end()) continue;
                  xmin = grm_min(
                      xmin,
                      *std::min_element(
                          std::begin(file_data[depth][col + ((col < err / down_err_off) ? col * down_err_off : err)]),
                          std::end(file_data[depth][col + ((col < err / down_err_off) ? col * down_err_off : err)])));
                  xmax = grm_max(
                      xmax,
                      *std::max_element(
                          std::begin(file_data[depth][col + ((col < err / down_err_off) ? col * down_err_off : err)]),
                          std::end(file_data[depth][col + ((col < err / down_err_off) ? col * down_err_off : err)])));
                }
              xmin = grm_min(xmin, 0);
              for (col = 0; col < cols; ++col)
                {
                  if (std::find(y_data.begin(), y_data.end(), col + 1) != y_data.end()) continue;
                  if (std::find(error_data.begin(), error_data.end(), col + 1) != error_data.end()) continue;
                  for (row = 0; row < rows; ++row)
                    {
                      file_data[depth][col][row] = ranges.xmin + (ranges.xmax - ranges.xmin) / (xmax - xmin) *
                                                                     ((double)file_data[depth][col][row] - xmin);
                    }
                }
              // special case for histogram cause the x-values and bar_width gets calculated via
              // x_range_min and max; without the following code block all series will always have the same x and
              // same width
              for (col = 0; col < x_data.size(); ++col)
                {
                  xmin = *std::min_element(std::begin(file_data[depth][x_data[col] - 1]),
                                           std::end(file_data[depth][x_data[col] - 1]));
                  xmax = *std::max_element(std::begin(file_data[depth][x_data[col] - 1]),
                                           std::end(file_data[depth][x_data[col] - 1]));
                  if (col < series_num) grm_args_push(series[col], "x_range", "dd", xmin, xmax);
                }
            }

          // find min and max value of all y-data and make sure the all data points are inside that range
          if (!grm_args_values(plot[plot_i], "y_range", "dd", &ymin, &ymax))
            {
              double y_min = INFINITY, y_max = -INFINITY;
              for (col = 0; col < y_data.size(); col++)
                {
                  ymin = *std::min_element(std::begin(file_data[depth][y_data[col] - 1]),
                                           std::end(file_data[depth][y_data[col] - 1]));
                  ymax = *std::max_element(std::begin(file_data[depth][y_data[col] - 1]),
                                           std::end(file_data[depth][y_data[col] - 1]));
                  y_min = grm_min(y_min, ymin);
                  y_max = grm_max(y_max, ymax);
                  if (!y_data.empty() && col < series_num) grm_args_push(series[col], "y_range", "dd", ymin, ymax);
                }
              adjustRanges(&ranges.ymin, &ranges.ymax, y_min, y_max);
              grm_args_push(plot[plot_i], "y_line_pos", "d", 0.0);
            }
          else
            {
              for (col = 0; col < series_num; ++col)
                {
                  grm_args_push(series[col], "y_range", "dd", ymin, ymax);
                }
              ymin = INFINITY;
              ymax = -INFINITY;
              for (col = 0; col < y_data.size(); ++col)
                {
                  ymin = grm_min(ymin, *std::min_element(std::begin(file_data[depth][y_data[col] - 1]),
                                                         std::end(file_data[depth][y_data[col] - 1])));
                  ymax = grm_max(ymax, *std::max_element(std::begin(file_data[depth][y_data[col] - 1]),
                                                         std::end(file_data[depth][y_data[col] - 1])));
                }
              for (col = 0; col < y_data.size(); ++col)
                {
                  for (row = 0; row < rows; row++)
                    {
                      file_data[depth][y_data[col] - 1][row] =
                          ranges.ymin + (ranges.ymax - ranges.ymin) / (ymax - ymin) *
                                            ((double)file_data[depth][y_data[col] - 1][row] - ymin);
                    }
                }
              if (!y_data.empty())
                {
                  grm_args_push(plot[plot_i], "y_line_pos", "d",
                                ranges.ymin + (ranges.ymax - ranges.ymin) / (ymax - ymin) * (0.0 - ymin));
                }
              else
                {
                  grm_args_push(plot[plot_i], "y_line_pos", "d", ranges.ymin);
                }
            }

          // push the data into the container structure
          for (col = 0; col < cols - err; col++)
            {
              if (x_data.empty() && y_data.empty() && error_data.empty())
                {
                  if (!labels.empty() && labels.size() > col && !labels[col].empty())
                    grm_args_push(series[col], "label", "s", labels[col].c_str());
                  grm_args_push(series[col], "x", "nD", rows,
                                file_data[depth][col + ((col < err / down_err_off) ? col * down_err_off : err)].data());
                  if (grm_args_values(plot[plot_i], "line_spec", "s", &spec))
                    grm_args_push(series[col], "line_spec", "s", spec);
                  if (series_num > 1) grm_args_push(series[col], "transparency", "d", 0.5);
                  if (col < err / down_err_off)
                    {
                      int error_bar_style;
                      grm_args_push(series[col], "error", "a", error_vec[col]);
                      if (grm_args_values(plot[plot_i], "error_bar_style", "i", &error_bar_style))
                        grm_args_push(series[err_cnt], "error_bar_style", "i", error_bar_style);
                    }
                }
              else
                {
                  bool timestamp = true;
                  int keep_aspect_ratio = 1;
                  for (const auto x_col : x_data)
                    {
                      if (std::find(timestamps.begin(), timestamps.end(), x_col - 1) == timestamps.end())
                        timestamp = false;
                    }
                  grm_args_push(plot[plot_i], "timestamp", "i", timestamp);
                  if (timestamp && !grm_args_values(plot[plot_i], "keep_aspect_ratio", "i", &keep_aspect_ratio))
                    grm_args_push(plot[plot_i], "keep_aspect_ratio", "i", 0);
                  if (std::find(x_data.begin(), x_data.end(), col + 1) != x_data.end())
                    {
                      int error_bar_style;
                      if (grm_args_values(plot[plot_i], "error_bar_style", "i", &error_bar_style))
                        grm_args_push(series[x_cnt], "error_bar_style", "i", error_bar_style);
                      grm_args_push(series[x_cnt], "x", "nD", rows, file_data[depth][col].data());
                      if (x_data.size() == 1)
                        {
                          // special case: if only one x-column is given use it for all y-columns
                          for (int k = 1; k < y_data.size(); k++)
                            {
                              grm_args_push(series[k], "x", "nD", rows, file_data[depth][col].data());
                            }
                        }
                      x_cnt += 1;
                    }
                  else if (std::find(y_data.begin(), y_data.end(), col + 1) != y_data.end())
                    {
                      if (!labels.empty() && labels.size() > col && !labels[col].empty())
                        grm_args_push(series[y_cnt], "label", "s", labels[col].c_str());
                      grm_args_push(series[y_cnt], "weights", "nD", rows, file_data[depth][col].data());
                      if (grm_args_values(plot[plot_i], "line_spec", "s", &spec))
                        grm_args_push(series[y_cnt], "line_spec", "s", spec);
                      if (series_num > 1) grm_args_push(series[y_cnt], "transparency", "d", 0.5);
                      y_cnt += 1;
                    }
                  else if (!input_flags.equal_up_and_down_error && error != nullptr &&
                           std::find(filtered_error_columns.begin(), filtered_error_columns.end(), col + 1) ==
                               filtered_error_columns.end())
                    {
                      grm_args_push(series[err_cnt], "error", "a", error_vec[err_cnt]);
                      err_cnt += 1;
                    }
                  else if (input_flags.equal_up_and_down_error &&
                           std::find(error_data.begin(), error_data.end(), col + 1) != error_data.end())
                    {
                      grm_args_push(series[err_cnt], "error", "a", error_vec[err_cnt]);
                      err_cnt += 1;
                    }
                }
            }
          cols -= x_data.size() + error_data.size();
          grm_args_push(plot[plot_i], "series", "nA", series_num, series.data());
        }
      else if (strcmp(kind, "pie") == 0)
        {
          std::vector<double> x(cols);
          std::vector<double> c(cols * 3);
          for (col = 0; col < cols; col++)
            {
              x[col] = file_data[depth][col][0];
              if (!labels.empty() && !labels[col].empty()) labels_c.push_back(labels[col].c_str());
            }

          grm_args_push(plot[plot_i], "x", "nD", cols, x.data());
          grm_args_push(plot[plot_i], "labels", "nS", labels_c.size(), labels_c.data());
          if (rows >= 4)
            {
              for (col = 0; col < cols; col++)
                {
                  for (row = 1; row < 4; row++)
                    {
                      c[(row - 1) * cols + col] = (double)file_data[depth][col][row];
                    }
                }
              grm_args_push(plot[plot_i], "c", "nD", c.size(), c.data());
            }
          else if (rows > 1)
            {
              fprintf(stderr, "Insufficient data for custom colors\n");
            }
        }
      else if (strcmp(kind, "polar_histogram") == 0)
        {
          if (cols > 1) fprintf(stderr, "Only the first column gets displayed\n");
          grm_args_push(plot[plot_i], "theta", "nD", rows, file_data[depth][0].data());
        }
      else if (strcmp(kind, "polar_line") == 0 || strcmp(kind, "polar_scatter") == 0)
        {
          int marker_type;
          double marker_size;

          if (cols % 2 == 1)
            {
              fprintf(stderr, "For polar_line and polar_scatter plots x and y must always be given, but in this case "
                              "the number of columns is odd -> 1 column is missing.\n");
              cols -= 1;
            }

          for (col = 0; col <= cols / 2; col += 2)
            {
              series[col / 2] = grm_args_new();
              grm_args_push(series[col / 2], "theta", "nD", rows, file_data[depth][col].data());
              grm_args_push(series[col / 2], "r", "nD", rows, file_data[depth][col + 1].data());
              grm_args_push(series[col / 2], "series_kind", "s",
                            col / 2 < kinds_length ? kinds[col / 2] : kinds[kinds_length - 1]);
              if (!labels.empty() && col / 2 < labels.size() && !labels[col / 2].empty())
                grm_args_push(series[col / 2], "label", "s", labels[col / 2].c_str());
              if (grm_args_values(plot[plot_i], "marker_type", "i", &marker_type))
                grm_args_push(series[col / 2], "marker_type", "i", marker_type);
              if (grm_args_values(plot[plot_i], "marker_size", "d", &marker_size))
                grm_args_push(series[col / 2], "marker_size", "d", marker_size);
            }
          grm_args_push(plot[plot_i], "series", "nA", cols / 2, series.data());
        }
      else if (strcmp(kind, "polar_heatmap") == 0)
        {
          std::vector<double> xi(cols), yi(rows), zi(rows * cols);
          double theta_min, theta_max, r_min, r_max;

          if (cols <= 1)
            {
              fprintf(stderr, "Insufficient data for plot type (%s)\n", kind);
              return 0;
            }
          if (!grm_args_values(plot[plot_i], "theta_range", "dd", &theta_min, &theta_max))
            {
              theta_min = 0.0;
              theta_max = 360.0;
            }
          if (!grm_args_values(plot[plot_i], "r_range", "dd", &r_min, &r_max))
            {
              r_min = 0.0;
              r_max = 3.0;
            }
          adjustRanges(&ranges.xmin, &ranges.xmax, theta_min, theta_max);
          adjustRanges(&ranges.ymin, &ranges.ymax, r_min, r_max);
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
          grm_args_push(plot[plot_i], "theta", "nD", cols, xi.data());
          grm_args_push(plot[plot_i], "r", "nD", rows, yi.data());
          grm_args_push(plot[plot_i], "z", "nD", cols * rows, zi.data());
        }
      else if (strcmp(kind, "quiver") == 0)
        {
          std::vector<double> x(cols);
          std::vector<double> y(rows);
          std::vector<double> u(cols * rows);
          std::vector<double> v(cols * rows);
          double xmin, xmax, ymin, ymax;

          if (depth < 2)
            {
              fprintf(stderr, "Not enough data for quiver plot\n");
              return 0;
            }

          if (!grm_args_values(plot[plot_i], "x_range", "dd", &xmin, &xmax))
            {
              xmin = 0.0;
              xmax = cols - 1.0;
            }
          if (!grm_args_values(plot[plot_i], "y_range", "dd", &ymin, &ymax))
            {
              ymin = 0.0;
              ymax = rows - 1.0;
            }
          adjustRanges(&ranges.xmin, &ranges.xmax, xmin, xmax);
          adjustRanges(&ranges.ymin, &ranges.ymax, ymin, ymax);
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

          grm_args_push(plot[plot_i], "x", "nD", cols, x.data());
          grm_args_push(plot[plot_i], "y", "nD", rows, y.data());
          grm_args_push(plot[plot_i], "u", "nD", cols * row, u.data());
          grm_args_push(plot[plot_i], "v", "nD", cols * row, v.data());
        }
      else if (strEqualsAny(kind, "hexbin", "shade"))
        {
          double min_x, min_y, max_x, max_y;
          double xmin, xmax, ymin, ymax;
          if (cols > 2) fprintf(stderr, "Only the first 2 columns get displayed\n");

          min_x = *std::min_element(std::begin(file_data[depth][0]), std::end(file_data[depth][0]));
          max_x = *std::max_element(std::begin(file_data[depth][0]), std::end(file_data[depth][0]));
          min_y = *std::min_element(std::begin(file_data[depth][1]), std::end(file_data[depth][1]));
          max_y = *std::max_element(std::begin(file_data[depth][1]), std::end(file_data[depth][1]));

          if (!grm_args_values(plot[plot_i], "x_range", "dd", &xmin, &xmax))
            {
              xmin = min_x;
              xmax = max_x;
            }
          if (!grm_args_values(plot[plot_i], "y_range", "dd", &ymin, &ymax))
            {
              ymin = min_y;
              ymax = max_y;
            }
          adjustRanges(&ranges.xmin, &ranges.xmax, xmin, xmax);
          adjustRanges(&ranges.ymin, &ranges.ymax, ymin, ymax);
          ranges.ymax = (ranges.ymax <= ranges.ymin) ? ranges.ymax + ranges.ymin : ranges.ymax;

          for (row = 0; row < rows; row++)
            {
              file_data[depth][0][row] =
                  ranges.xmin + (ranges.xmax - ranges.xmin) * (file_data[depth][0][row] - min_x) / (max_x - min_x);
              file_data[depth][1][row] =
                  ranges.ymin + (ranges.ymax - ranges.ymin) * (file_data[depth][1][row] - min_y) / (max_y - min_y);
            }
          grm_args_push(plot[plot_i], "x", "nD", rows, file_data[depth][0].data());
          grm_args_push(plot[plot_i], "y", "nD", rows, file_data[depth][1].data());
        }
      if (plot_num > 1)
        {
          grm_args_push(plot[plot_i], "row", "i", plot_i % divisor);
          grm_args_push(plot[plot_i], "col", "i", plot_i / divisor);
        }
      if (!grm_args_values(plot[plot_i], "use_grplot_changes", "i", &use_grplot_changes))
        grm_args_push(plot[plot_i], "use_grplot_changes", "i", 1);
    }
  if (consecutive_colorbars)
    {
      grm_args_push(args, "consecutive_colorbars", "i", consecutive_colorbars);
    }
  grm_args_push(args, "subplots", "nA", plot_num, plot.data());
  if (!join_plot_numbers.empty())
    {
      std::vector<int> join_plot_numbers_vec;
      for (const auto &plot_number : join_plot_numbers)
        {
          if (plot_number <= plot.size()) join_plot_numbers_vec.push_back(plot_number);
        }
      grm_args_push(args, "join_plots", "nI", join_plot_numbers_vec.size(), join_plot_numbers_vec.data());
    }
  grm_merge(args);

  if (handle != nullptr)
    {
      grm_send_args(handle, args);
      grm_close(handle);
    }

  delete file_args;
  return 1;
}

int grm_context_data_from_file(const std::shared_ptr<GRM::Context> &context, const std::string &path,
                               bool interpret_matrix_as_one_column)
{
  size_t cols, rows, depth;
  std::vector<std::vector<std::vector<double>>> file_data;
  std::vector<int> x_data, y_data, error_data;
  std::vector<std::string> labels;
  PlotRange ranges = {INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY};
  std::shared_ptr<GRM::Element> root = grm_get_document_root();
  grm_special_axis_series_t *special_axis_series = grm_special_axis_series_new();
  InputFlags input_flags;
  std::vector<int> timestamps;

  if (readDataFile(path, file_data, x_data, y_data, error_data, labels, nullptr, "", "", "", "", &ranges,
                   special_axis_series, input_flags, timestamps))
    return 0;

  if (!file_data.empty())
    {
      depth = file_data.size();
      cols = file_data[0].size();
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
      fprintf(stderr,
              "The number of columns (%zu) doesn't fit the number of context names (%zu). Dummy names will be used "
              "instead\n",
              cols, labels.size());
    }

  if (!interpret_matrix_as_one_column)
    {
      for (int col = 0; col < cols; col++)
        {
          std::string name;
          if (!labels.empty() && !labels[col].empty())
            {
              name = labels[col];
            }
          else
            {
              auto id = static_cast<int>(root->getAttribute("_id"));
              root->setAttribute("_id", ++id);
              name = "tmp" + std::to_string(id);
            }
          (*context)[name] = file_data[depth][col];
        }
    }
  else
    {
      std::string name;
      std::vector<double> z(rows * cols);
      for (int col = 0; col < cols; ++col)
        {
          for (int row = 0; row < rows; ++row)
            {
              z[row * cols + col] = file_data[depth][col][row];
            }
        }

      if (!labels.empty() && !labels[0].empty())
        {
          name = labels[0];
        }
      else
        {
          auto id = static_cast<int>(root->getAttribute("_id"));
          root->setAttribute("_id", ++id);
          name = "tmp" + std::to_string(id);
        }
      (*context)[name] = z;
    }
  return 1;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ input stream parser ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int convertInputstreamIntoArgs(grm_args_t *args, grm_file_args_t *file_args, int argc, char **argv, PlotRange *ranges,
                               grm_special_axis_series_t *special_axis_series, InputFlags &input_flags)
{
  std::string token, delim = ":", optional_file;
  std::vector<std::string> kinds;
  std::vector<const char *> kinds_c;

  for (int i = 1; i < argc; i++)
    {
      token = argv[i];

      /* parameter needed for import.cxx are handled differently than grm-parameters */
      if (startsWith(token, "file:"))
        {
          file_args->file_path = token.substr(5, token.length() - 1);
        }
      else if (i == 1 &&
               (token.find(":") == std::string::npos ||
                // Check if is an absolute Windows path, like `C:\Users\...`
                (token.length() > 2 && isalpha(token[0]) && token[1] == ':' && (token[2] == '/' || token[2] == '\\'))))
        {
          optional_file = token; /* it's only used, if no "file:" keyword was found */
        }
      else if (startsWith(token, "columns:"))
        {
          file_args->file_columns = token.substr(8, token.length() - 1);
        }
      else if (startsWith(token, "x_columns:"))
        {
          file_args->file_x_columns = token.substr(10, token.length() - 1);
        }
      else if (startsWith(token, "y_columns:"))
        {
          file_args->file_y_columns = token.substr(10, token.length() - 1);
        }
      else if (startsWith(token, "error_columns:"))
        {
          file_args->file_error_columns = token.substr(14, token.length() - 1);
        }
      else if (startsWith(token, "join_plots:"))
        {
          auto tmp = token.substr(11, token.length() - 1);
          parseColumns(&join_plot_numbers, tmp.c_str());
        }
      else if (startsWith(token, "consecutive_colorbars:"))
        {
          auto tmp = token.substr(22, token.length() - 1);
          if (tmp != "") consecutive_colorbars = std::stoi(tmp);
        }
      else
        {
          auto tmp_kinds = singleTokenConverter(token, args, ranges, special_axis_series, input_flags);
          if (tmp_kinds.size() > 0 && isValidKind(tmp_kinds[0]))
            {
              kinds = tmp_kinds;
              input_flags.default_kind_used = false;
            }
          if (!kinds.empty())
            {
              for (int col = 0; col < kinds.size(); col++)
                {
                  kinds_c.push_back(kinds[col].c_str());
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
  if (kinds.empty())
    {
      kinds_c.push_back("line");
      fprintf(stderr, "No valid plot kind given- fallback to line plot\n");
    }
  grm_args_push(args, "kind", "nS", kinds_c.size(), kinds_c.data());
  return 1;
}
