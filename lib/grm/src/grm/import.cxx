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

err_t read_data_file(const std::string &path, std::vector<std::vector<double>> &data, std::vector<std::string> &labels,
                     grm_args_t *args, const char *colms, PlotRange *ranges)
{
  std::string line;
  std::string token;
  std::ifstream file(path);
  std::list<int> columns;
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
                  fprintf(stderr,
                          "Value error for column parameter. Use numbers in the "
                          "specified format. Entry '%s'\n",
                          token.c_str());
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
                      fprintf(stderr,
                              "Value error for column parameter. Use numbers in the "
                              "specified format. Entry '%s'\n",
                              token.c_str());
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
              fprintf(stderr,
                      "Value error for column parameter. Use numbers in the "
                      "specified format. Entry '%s'\n",
                      token.c_str());
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
          if (str_equals_any(key.c_str(), 4, "title", "xlabel", "ylabel", "resample_method"))
            {
              grm_args_push(args, key.c_str(), "s", value.c_str());
            }
          else if (str_equals_any(key.c_str(), 5, "location", "xlog", "ylog", "xgrid", "ygrid"))
            {
              try
                {
                  grm_args_push(args, key.c_str(), "i", std::stoi(value));
                }
              catch (std::invalid_argument &e)
                {
                  fprintf(stderr,
                          "Value error for plot parameter in argument '%s : %s'. Problem "
                          "appeared in line %i.\n",
                          key.c_str(), value.c_str(), linecount);
                  return ERROR_PARSE_INT;
                }
            }
          else if (str_equals_any(key.c_str(), 4, "xlim", "ylim", "xrange", "yrange"))
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
                  fprintf(stderr,
                          "Value error for plot parameter in argument '%s : %s, %s'. "
                          "Problem appeared in line %i.\n",
                          key.c_str(), value1.c_str(), value2.c_str(), linecount);
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
            }
          else
            {
              fprintf(stderr,
                      "Key-value pair '%s : %s' in line %i unknown. Check if the key-value pair "
                      "exists in grm.\n",
                      key.c_str(), value.c_str(),
                      linecount); /* TODO: extend these ifs when more key values pairs are needed */
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
  for (size_t row = 0; std::getline(file, line) && line.length(); row++)
    {
      std::istringstream line_ss(normalize_line(line));
      int cnt = 0;
      for (size_t col = 0; std::getline(line_ss, token, '\t') && token.length(); col++)
        {
          if (std::find(columns.begin(), columns.end(), col) != columns.end() || (columns.empty() && labels.empty()) ||
              (columns.empty() && col < labels.size()))
            {
              if (row == 0)
                {
                  data.emplace_back(std::vector<double>());
                }
              try
                {
                  data[cnt].push_back(std::stod(token));
                }
              catch (std::invalid_argument &e)
                {
                  fprintf(stderr,
                          "Value error inside data. Check if '%s' in row %zu and column "
                          "%zu is correct.\n",
                          token.c_str(), row + linecount + 1, col + 1);
                  return ERROR_PARSE_DOUBLE;
                }
              cnt += 1;
            }
        }
    }
  return ERROR_NONE;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plot functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int grm_plot_from_file(const char *data_file, const char **plot_type, const char *colms)
/**
 * Allows to create a plot from a file. The file is holding the data and the container arguments for the plot.
 *
 * @param data_file: Valid path the file which contains the data for the plot.
 * @param plot_type: One of the arguments inside the brackets (line, heatmap, marginalheatmap). The default is line.
 * @param colms: Defines which columns from the file should be used for the plot. The default is all.
 * @return 1 when there was no error, 0 if there was an error.
 */
{
  grm_args_t *args;
  int error = 1;

  args = grm_args_new();
  error = grm_interactive_plot_from_file(args, data_file, plot_type, colms, "all", "sum");
  grm_args_delete(args);
  return error;
}

int grm_interactive_plot_from_file(grm_args_t *args, const char *data_file, const char **plot_type, const char *colms,
                                   const char *heatmap_type, const char *heatmap_algo)
/**
 * Allows to create an interactive plot from a file. The file is holding the data and the container arguments for the
 * plot.
 *
 * @param args: A grm container. Should be the container, which also defines the window.
 * @param data_file: Valid path the file which contains the data for the plot.
 * @param plot_type: One of the arguments inside the brackets (line, heatmap, marginalheatmap). The default is line.
 * @param colms: Defines which columns from the file should be used for the plot. The default is all.
 * @param heatmap_type: Valid types are 'all' and 'line'. The porameter defines if all or only one line and column is
 * used for the sideplots of the marginalheatmap;
 * @param heatmap_algo: Valid algorithms are 'sum' and 'max'. The parameter defines the way how the histograms are
 * calculated.
 * @return 1 when there was no error, 0 if there was an error.
 */
{
  std::string s;
  size_t row, col, rows, cols;
  std::vector<std::vector<double>> filedata;
  std::vector<std::string> labels;
  std::vector<const char *> labels_c;
  std::vector<grm_args_t *> series;
  char *env;
  void *handle = nullptr;
  PlotRange ranges = {0.0, -1.0, 0.0, -1.0};

  if (!file_exists(data_file))
    {
      fprintf(stderr,
              "Error! No file with the name %s was found. Please use a correct "
              "filename and filepath.\n",
              data_file);
      return 0;
    }
  if (read_data_file(data_file, filedata, labels, args, colms, &ranges))
    {
      return 0;
    }
  if (!filedata.empty())
    {
      cols = filedata.size();
      rows = filedata[0].size();
    }
  else
    {
      fprintf(stderr, "Error! No data or valid columns are specified.\n");
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

  if (!str_equals_any(*plot_type, 3, "line", "heatmap", "marginalheatmap"))
    {
      *plot_type = "line";
      fprintf(stderr, "No correct plot type was specified. A normal line plot is "
                      "getting used.\n");
    }
  if (strcmp("line", *plot_type) == 0 && (rows >= 100 && cols >= 100))
    {
      *plot_type = "heatmap";
      fprintf(stderr, "File data is to big for line plot. A heatmap is being tried instead.\n");
    }
  grm_args_push(args, "kind", "s", *plot_type);

  if (str_equals_any(*plot_type, 2, "heatmap", "marginalheatmap"))
    {
      std::vector<double> xi(rows), yi(cols), zi(rows * cols);

      if (cols <= 1)
        {
          fprintf(stderr, "Error! For heatmap and marginalheatmap there must be "
                          "atleast two specified columns.\n");
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
              zi[((cols - 1) - col) * rows + row] = filedata[col][row];
            }
        }

      grm_args_push(args, "x", "nD", rows, xi.data());
      grm_args_push(args, "y", "nD", cols, yi.data());
      grm_args_push(args, "z", "nD", rows * cols, zi.data());
      grm_args_push(args, "type", "s", heatmap_type);
      grm_args_push(args, "algorithm", "s", heatmap_algo);
    }
  else if (strcmp(*plot_type, "line") == 0)
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
          grm_args_push(series[col], "y", "nD", rows, filedata[col].data());
          if (!labels.empty())
            {
              labels_c.push_back(labels[col].c_str());
            }
        }
      grm_args_push(args, "series", "nA", cols, series.data());
      if (labels_c.empty())
        {
          fprintf(stderr, "No labels specified. Continue with no labels for the lines.\n");
        }
      else
        {
          grm_args_push(args, "labels", "nS", cols, labels_c.data());
        }
    }
  grm_merge(args);

  if (handle != nullptr)
    {
      grm_send_args(handle, args);
      grm_close(handle);
    }

  return 1;
}
