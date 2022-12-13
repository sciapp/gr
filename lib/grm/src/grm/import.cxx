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

/* ========================= static variables ======================================================================= */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ key to types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static std::map<std::string, const char *> key_to_types{{"algorithm", "s"}, {"colormap", "i"},
                                                        {"isovalue", "d"},  {"keep_aspect_ratio", "i"},
                                                        {"kind", "s"},      {"marginalheatmap_kind", "s"}};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ kind types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static std::list<std::string> kind_types = {"contour",         "heatmap", "imshow",  "isosurface", "line",
                                            "marginalheatmap", "plot3",   "surface", "volume",     "wireframe"};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ alias for keys ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static std::map<std::string, std::string> key_alias = {
    {"hkind", "marginalheatmap_kind"}, {"aspect", "keep_aspect_ratio"}, {"cmap", "colormap"}};


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

err_t read_data_file(const std::string &path, std::vector<std::vector<std::vector<double>>> &data,
                     std::vector<std::string> &labels, grm_args_t *args, const char *colms, PlotRange *ranges)
{
  std::string line;
  std::string token;
  std::ifstream file(path);
  std::list<int> columns;
  bool depth_change = true;
  int depth = 0;
  int linecount = 0;

  /* read the columns from the colms string also converts slicing into numbers */
  std::stringstream scol(colms);
  for (size_t col = 0; std::getline(scol, token, ',') && token.length(); col++)
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
              columns.push_back(num);
            }
        }
      else
        {
          try
            {
              columns.push_back(std::stoi(token));
            }
          catch (std::invalid_argument &e)
            {
              fprintf(stderr, "Invalid argument for column parameter (%s)\n", token.c_str());
              return ERROR_PARSE_INT;
            }
        }
    }
  if (!columns.empty())
    {
      columns.sort();
      ranges->ymin = *columns.begin();
    }

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
          if (str_equals_any(key.c_str(), 5, "title", "xlabel", "ylabel", "zlabel", "resample_method"))
            {
              grm_args_push(args, key.c_str(), "s", value.c_str());
            }
          else if (str_equals_any(key.c_str(), 7, "location", "xlog", "ylog", "zlog", "xgrid", "ygrid", "zgrid"))
            {
              try
                {
                  grm_args_push(args, key.c_str(), "i", std::stoi(value));
                }
              catch (std::invalid_argument &e)
                {
                  fprintf(stderr, "Invalid argument for plot parameter (%s:%s) in line %i\n", key.c_str(),
                          value.c_str(), linecount);
                  return ERROR_PARSE_INT;
                }
            }
          else if (str_equals_any(key.c_str(), 6, "xlim", "ylim", "zlim", "xrange", "yrange", "zrange"))
            {
              std::stringstream sv(value);
              std::string value1;
              std::string value2;
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
                  grm_args_push(args, key.c_str(), "dd", std::stod(value1), std::stod(value2));
                }
              catch (std::invalid_argument &e)
                {
                  fprintf(stderr, "Invalid argument for plot parameter (%s:%s,%s) in line %i\n", key.c_str(),
                          value1.c_str(), value2.c_str(), linecount);
                  return ERROR_PARSE_DOUBLE;
                }
              if (strcmp(key.c_str(), "xrange") == 0)
                {
                  ranges->xmin = std::stod(value1);
                  ranges->xmax = std::stod(value2);
                }
              else if (strcmp(key.c_str(), "yrange") == 0)
                {
                  ranges->ymin = std::stod(value1);
                  ranges->ymax = std::stod(value2);
                }
              else if (strcmp(key.c_str(), "zrange") == 0)
                {
                  ranges->zmin = std::stod(value1);
                  ranges->zmax = std::stod(value2);
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
          for (size_t col = 0; std::getline(line_ss, token, '\t') && token.length(); col++)
            {
              if (std::find(columns.begin(), columns.end(), col) != columns.end() || columns.empty())
                {
                  labels.push_back(token);
                }
            }
          break;
        }
    }

  /* read the numeric data for the plot */
  for (size_t row = 0; std::getline(file, line); row++)
    {
      std::istringstream line_ss(normalize_line(line));
      int cnt = 0;
      if (line.empty())
        {
          depth += 1;
          depth_change = true;
          continue;
        }
      for (size_t col = 0; std::getline(line_ss, token, '\t') && token.length(); col++)
        {
          if (std::find(columns.begin(), columns.end(), col) != columns.end() || (columns.empty() && labels.empty()) ||
              (columns.empty() && col < labels.size()))
            {
              if (row == 0 && col == 0 || depth_change && col == 0)
                {
                  data.emplace_back(std::vector<std::vector<double>>());
                }
              if (depth_change)
                {
                  data[depth].emplace_back(std::vector<double>());
                }
              try
                {
                  data[depth][cnt].push_back(std::stod(token));
                }
              catch (std::invalid_argument &e)
                {
                  fprintf(stderr, "Invalid number in line %zu, column %zu (%s)\n", row + linecount + 1, col + 1,
                          token.c_str());
                  return ERROR_PARSE_DOUBLE;
                }
              cnt += 1;
            }
        }
      depth_change = false;
    }
  return ERROR_NONE;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ argument container ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

grm_file_args_t *grm_file_args_new()
{
  auto *args = (grm_file_args_t *)malloc(sizeof(grm_file_args_t));
  if (args == nullptr)
    {
      debug_print_malloc_error();
      return nullptr;
    }
  args->file_path = "";
  args->file_columns = "";
  return args;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plot functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int grm_plot_from_file(int argc, char **argv)
/**
 * Allows to create a plot from a file. The file is holding the data and the container arguments for the plot.
 *
 * @param argc: number of elements inside argv.
 * @param argv: contains the parameter which specify the displayed plot. For example where the data is or which plot
 * should be drawn.
 * @return 1 when there was no error, 0 if there was an error.
 */
{
  grm_args_t *args;
  int error = 1;

  args = grm_args_new();
  error = grm_interactive_plot_from_file(args, argc, argv);
  grm_args_delete(args);
  return error;
}

int grm_interactive_plot_from_file(grm_args_t *args, int argc, char **argv)
/**
 * Allows to create an interactive plot from a file. The file is holding the data and the container arguments for the
 * plot.
 *
 * @param args: a grm container. Should be the container, which also defines the window.
 * @param argc: number of elements inside argv.
 * @param argv: contains the parameter which specify the displayed plot. For example where the data is or which plot
 * should be drawn.
 * @return 1 when there was no error, 0 if there was an error.
 */
{
  std::string s;
  size_t row, col, rows, cols, depth;
  std::vector<std::vector<std::vector<double>>> filedata;
  std::vector<std::string> labels;
  std::vector<const char *> labels_c;
  std::vector<grm_args_t *> series;
  char *env;
  void *handle = nullptr;
  const char *kind;
  grm_file_args_t *file_args;
  file_args = grm_file_args_new();
  PlotRange ranges = {0.0, -1.0, 0.0, -1.0, 0.0, -1.0};

  if (!convert_inputstream_into_args(args, file_args, argc, argv))
    {
      return 0;
    }

  if (!file_exists(file_args->file_path))
    {
      fprintf(stderr, "File not found (%s)\n", file_args->file_path.c_str());
      return 0;
    }
  if (read_data_file(file_args->file_path, filedata, labels, args, file_args->file_columns.c_str(), &ranges))
    {
      return 0;
    }
  if (!filedata.empty())
    {
      depth = filedata.size();
      cols = filedata[0].size();
      rows = filedata[0][0].size();
      depth = (depth == 1) ? 0 : depth;
    }
  else
    {
      fprintf(stderr, "File is empty\n");
      return 0;
    }

  series.resize(cols);

  if ((env = getenv("GR_DISPLAY")) != nullptr)
    {
      handle = grm_open(GRM_SENDER, env, 8002, nullptr, nullptr);
      if (handle == nullptr)
        {
          fprintf(stderr, "GRM connection to '%s' could not be established\n", env);
        }
    }

  grm_args_values(args, "kind", "s", &kind);
  if (strcmp("line", kind) == 0 && (rows >= 100 && cols >= 100))
    {
      fprintf(stderr, "Too much data for line plot - use heatmap instead\n");
      kind = "heatmap";
      grm_args_push(args, "kind", "s", kind);
    }
  if (strcmp("volume", kind) != 0 && strcmp("isosurface", kind) != 0 && depth >= 1)
    {
      fprintf(stderr, "Too much data for %s plot - use volume instead\n", kind);
      kind = "volume";
      grm_args_push(args, "kind", "s", kind);
    }
  if (strcmp(kind, "line") == 0 || cols != rows)
    {
      grm_args_push(args, "keep_aspect_ratio", "i", 0);
    }

  if (str_equals_any(kind, 6, "contour", "heatmap", "imshow", "marginalheatmap", "surface", "wireframe"))
    {
      std::vector<double> xi(rows), yi(cols), zi(rows * cols);

      if (cols <= 1)
        {
          fprintf(stderr, "Unsufficient data for %s plot\n", kind);
          return 0;
        }
      ranges.xmax = (ranges.xmax == -1.0) ? ((double)rows - 1.0 + ranges.xmin) : ranges.xmax;
      ranges.ymax = (ranges.ymax == -1.0) ? ((double)cols - 1.0 + ranges.ymin) : ranges.ymax;
      ranges.ymax = (ranges.ymax <= ranges.ymin) ? ranges.ymax + ranges.ymin : ranges.ymax;

      for (row = 0; row < rows; ++row)
        {
          xi[row] = ranges.xmin + (ranges.xmax - ranges.xmin) * ((double)row / ((double)rows - 1));
          for (col = 0; col < cols; ++col)
            {
              if (row == 0)
                {
                  yi[col] = ranges.ymin + (ranges.ymax - ranges.ymin) * ((double)col / ((double)cols - 1));
                }
              zi[((cols - 1) - col) * rows + row] = filedata[depth][col][row];
            }
        }

      if (ranges.zmax != -1)
        {
          int elem;
          double min_val = *std::min_element(zi.begin(), zi.end());
          double max_val = *std::max_element(zi.begin(), zi.end());

          for (elem = 0; elem < rows * cols; ++elem)
            {
              zi[elem] = ranges.zmin + (ranges.zmax - ranges.zmin) * (zi[elem] - min_val) / (max_val - min_val);
            }
        }

      // for imshow plot
      grm_args_push(args, "c", "nD", rows * cols, zi.data());
      grm_args_push(args, "c_dims", "ii", rows, cols);

      grm_args_push(args, "x", "nD", rows, xi.data());
      grm_args_push(args, "y", "nD", cols, yi.data());
      grm_args_push(args, "z", "nD", rows * cols, zi.data());
    }
  else if (strcmp(kind, "line") == 0)
    {
      std::vector<double> x(rows);
      for (row = 0; row < rows; row++)
        {
          x[row] = (double)row;
        }
      for (col = 0; col < cols; col++)
        {
          series[col] = grm_args_new();
          grm_args_push(series[col], "x", "nD", rows, x.data());
          grm_args_push(series[col], "y", "nD", rows, filedata[depth][col].data());
          if (!labels.empty())
            {
              labels_c.push_back(labels[col].c_str());
            }
        }
      grm_args_push(args, "series", "nA", cols, series.data());
      if (!labels_c.empty())
        {
          grm_args_push(args, "labels", "nS", cols, labels_c.data());
        }
    }
  else if (strcmp(kind, "volume") == 0 || strcmp(kind, "isosurface") == 0)
    {
      int i, j, k;
      std::vector<double> data(rows * cols * depth);
      int n = (int)rows * (int)cols * (int)depth;
      std::vector<int> dims = {(int)cols, (int)rows, (int)depth};
      double x, y, z;
      for (i = 0; i < rows; ++i)
        {
          for (j = 0; j < cols; ++j)
            {
              for (k = 0; k < depth; ++k)
                {
                  data[k * cols * rows + j * rows + i] = filedata[k][j][i];
                }
            }
        }

      grm_args_push(args, "c", "nD", n, data.data());
      grm_args_push(args, "c_dims", "nI", 3, dims.data());
    }
  else if (strcmp(kind, "plot3") == 0)
    {
      std::vector<double> x(rows);
      std::vector<double> y(rows);
      std::vector<double> z(rows);
      if (cols < 3)
        {
          fprintf(stderr, "Unsufficient data for 3d lineplot\n");
          return 0;
        }

      grm_args_push(args, "x", "nD", rows, filedata[depth][0].data());
      grm_args_push(args, "y", "nD", rows, filedata[depth][1].data());
      grm_args_push(args, "z", "nD", rows, filedata[depth][2].data());
    }
  grm_merge(args);

  if (handle != nullptr)
    {
      grm_send_args(handle, args);
      grm_close(handle);
    }

  free(file_args);
  return 1;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ input stream parser ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int convert_inputstream_into_args(grm_args_t *args, grm_file_args_t *file_args, int argc, char **argv)
{
  int i, j;
  std::string token, found_key;
  size_t found_key_size;
  std::string delim = ":";
  const char *kind = "line";
  std::string optional_file;

  for (i = 1; i < argc; i++)
    {
      token = argv[i];
      // parameter needed for import.cxx are treated different then grm-parameters
      if (starts_with(token, "file:"))
        {
          file_args->file_path = token.substr(5, token.length() - 1);
        }
      else if (i == 1 && token.find(delim) == std::string::npos)
        {
          optional_file = token; // its only getting used, when no "file:"-keyword was found
        }
      else if (starts_with(token, "columns:"))
        {
          file_args->file_columns = token.substr(8, token.length() - 1);
        }
      else
        {
          size_t pos = token.find(delim);
          if (pos != std::string::npos)
            {
              found_key = token.substr(0, pos);
              found_key_size = found_key.size();
              // check if there exist a know alias and in case of replace the key
              if (auto search_alias = key_alias.find(found_key); search_alias != key_alias.end())
                {
                  found_key = search_alias->second;
                }
              if (auto search = key_to_types.find(found_key); search != key_to_types.end())
                {
                  // special case for kind, for following exception
                  if (strcmp(found_key.c_str(), "kind") == 0)
                    {
                      kind = token.substr(found_key_size + 1, token.length() - 1).c_str();
                    }

                  // different types
                  if (strcmp(search->second, "s") == 0)
                    {
                      grm_args_push(args, search->first.c_str(), search->second,
                                    token.substr(found_key_size + 1, token.length() - 1).c_str());
                    }
                  else
                    {
                      std::string value = token.substr(found_key_size + 1, token.length() - 1);
                      try
                        {
                          if (strcmp(search->second, "i") == 0)
                            {
                              grm_args_push(args, search->first.c_str(), search->second, std::stoi(value));
                            }
                          else if (strcmp(search->second, "d") == 0)
                            {
                              grm_args_push(args, search->first.c_str(), search->second, std::stod(value));
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

  // errors than can be cached
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
      fprintf(stderr, "Invalid plot type (%s) - fallback to line plot\n", kind);
      kind = "line";
    }
  grm_args_push(args, "kind", "s", kind);
  return 1;
}
