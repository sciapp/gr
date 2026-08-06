
#include <cmath>
#include <cstdio>
#include <fstream>
#include <list>
#include <sstream>
#include <algorithm>
#include <map>
#include <iostream>
#include <clocale>

#include <time.h>

#include "csv_plugin.hxx"
#include "grm/import_int.hxx"
#include "grm/error.h"
#include "grm/utilcpp_int.hxx"


static char detectDelimiter(const std::string &line)
{
  std::vector<char> candidates = {',', ';', '|', '\t', ' '};
  unsigned int max_occurences = 0;
  char best = candidates[0];

  for (char c : candidates)
    {
      auto occurences = std::count(line.begin(), line.end(), c);
      if (occurences > max_occurences)
        {
          max_occurences = occurences;
          best = c;
        }
    }
  return best;
}

std::string CsvSource::normalizeLine(const std::string &str)
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

grm_error_t CsvSource::readDataFile(const std::string &path, std::vector<std::vector<std::vector<double>>> &data,
                                    std::vector<int> &x_data, std::vector<int> &y_data, std::vector<int> &error_data,
                                    std::vector<std::string> &labels, grm_args_t *args, const char *colms,
                                    const char *x_colms, const char *y_colms, const char *e_colms, PlotRange *ranges,
                                    grm_special_axis_series_t *special_axis_series, InputFlags &input_flags,
                                    std::vector<int> &timestamps)
{
  std::string line;
  std::string token;
  std::ifstream file_path(path);
  std::istream &cin_path = std::cin;
  std::list<int> columns, x_columns, y_columns, e_columns;
  bool depth_change = true;
  bool legend_line = input_flags.force_legend_line;
  bool skip_legend_line = false;
  int depth = 0, max_col = -1;
  int linecount = 0;
  grm_error_t error = GRM_ERROR_NONE;

  /* read the columns from the colms string also converts slicing into numbers */
  if ((error = parseColumns(&columns, colms)) != GRM_ERROR_NONE) return error;
  if (!columns.empty()) ranges->ymin = *columns.begin();
  /* read the columns from the x_colms, y_colms and e_colms string also converts slicing into numbers */
  if ((error = parseColumns(&x_columns, x_colms)) != GRM_ERROR_NONE) return error;
  if ((error = parseColumns(&y_columns, y_colms)) != GRM_ERROR_NONE) return error;
  if ((error = parseColumns(&e_columns, e_colms)) != GRM_ERROR_NONE) return error;

  std::istream &file = (path == "-") ? cin_path : file_path;
  /* read the lines from the file */
  while (getline(file, line))
    {
      std::istringstream iss(line, std::istringstream::in);
      linecount += 1;
      /* the line defines a grm container parameter */
      if (line[0] == '#')
        {
          std::string key, value, tmp_value;
          std::stringstream ss(line);

          /* read the key-value pairs from the file and redirect them to grm if possible */
          std::getline(ss, token);
          size_t pos = token.find("#");
          token = token.substr(pos + 1, token.size() - 1);
          pos = token.find(":");
          if (pos != std::string::npos)
            {
              key = trim(token.substr(0, pos));
              tmp_value = token.substr(pos + 1, token.size() - 1);
              if (key != "legend") tmp_value = trim(tmp_value);
              if (key == "error")
                {
                  std::size_t found = tmp_value.find_first_of(":{}");
                  while (found != std::string::npos)
                    {
                      std::string tmp = std::string(trim(tmp_value.substr(0, found)));
                      tmp_value =
                          tmp + tmp_value[found] + std::string(trim(tmp_value.substr(found + 1, token.size() - 1)));
                      found = tmp_value.find_first_of(":{}", tmp.size() + 1);
                    }
                }
            }

          /* if there are multiple values seperated with a ',' read and trim those to create the commandline value */
          std::stringstream sv(tmp_value);
          for (size_t col = 0; std::getline(sv, token, ',') && token.length(); col++)
            {
              if (col == 0)
                {
                  value = trim(token);
                }
              else
                {
                  value += "," + std::string(trim(token));
                }
            }

          if (x_columns.empty() && key == "x_columns")
            {
              if ((error = parseColumns(&x_columns, value.c_str())) != GRM_ERROR_NONE) return error;
            }
          else if (y_columns.empty() && key == "y_columns")
            {
              if ((error = parseColumns(&y_columns, value.c_str())) != GRM_ERROR_NONE) return error;
            }
          else if (e_columns.empty() && key == "error_columns")
            {
              if ((error = parseColumns(&e_columns, value.c_str())) != GRM_ERROR_NONE) return error;
            }
          else if (columns.empty() && key == "columns")
            {
              if ((error = parseColumns(&columns, colms)) != GRM_ERROR_NONE) return error;
            }
          else if (key == "legend")
            {
              std::istringstream line_ss(tmp_value);
              std::string split_label;
              if (key == "legend" && !input_flags.force_legend_line) skip_legend_line = true;

              for (size_t col = 0; std::getline(line_ss, token, ','); col++)
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
          else if (args != nullptr)
            {
              /* use key + ":" + value to create a token which is similar to a commandline key:value pair */
              singleTokenConverter(key + ":" + value, args, ranges, special_axis_series, input_flags, linecount);
            }
        }
      else /* the line contains the labels for the plot */
        {
          auto normalized_line = normalizeLine(line);
          auto delim = detectDelimiter(normalized_line);
          std::istringstream line_ss(normalized_line);
          std::string split_label;
          if (skip_legend_line)
            break;
          else
            labels.clear();
          for (size_t col = 0; std::getline(line_ss, token, delim) && token.length(); col++)
            {
              if (!legend_line && isNumber(token) && !input_flags.use_bins) continue;
              if (std::find(columns.begin(), columns.end(), col + 1) != columns.end() || columns.empty())
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
                  legend_line = true;
                }
            }
          break;
        }
    }

  // Save locale setting
  const std::string old_locale = std::setlocale(LC_NUMERIC, nullptr);
  std::setlocale(LC_NUMERIC, "C");

  if (!legend_line) --linecount;

  /* read the numeric data for the plot */
  int skipped = 0;
  // Skip the first line read if the current `line` is not a legend line -> it already contains data
  bool skip_first_getline = !legend_line;
  char delim = '\0';
  for (size_t row = 0; skip_first_getline ? true : static_cast<bool>(std::getline(file, line)); row++)
    {
      skip_first_getline = false;
      auto normalized_line = normalizeLine(line);
      std::istringstream line_ss(normalized_line);
      int cnt = 0, start_with_nan = 0;
      size_t col;
      if (line.empty())
        {
          if (!input_flags.ignore_blank_lines && row != 0)
            {
              depth += 1;
              depth_change = true;
            }
          if (row == 0) skipped = 1;
          continue;
        }
      if (delim == '\0') delim = detectDelimiter(normalized_line);
      if (std::find(line.begin(), line.end(), ',') != line.end())
        {
          delim = ',';
          std::string tmp = ",";
          if (startsWith(trim(line), tmp)) start_with_nan = 1;
        }
      for (col = 0; std::getline(line_ss, token, delim) && (token.length() || start_with_nan); col++)
        {
          if ((!columns.empty() && std::find(columns.begin(), columns.end(), col + 1) != columns.end()) ||
              (columns.empty() && labels.empty() && (!input_flags.use_bins || col > 0)) ||
              (columns.empty() && (!input_flags.use_bins || col > 0)))
            {
              if ((row - skipped == 0 &&
                   (col == input_flags.use_bins || (!columns.empty() && col + 1 == columns.front()))) ||
                  (depth_change && (col == input_flags.use_bins || (!columns.empty() && col + 1 == columns.front()))) ||
                  (start_with_nan && (col == input_flags.use_bins || (!columns.empty() && col + 1 == columns.front()))))
                {
                  data.emplace_back(std::vector<std::vector<double>>());
                }
              if (depth_change) data[depth].emplace_back(std::vector<double>());
              if (max_col != -1 && max_col < (int)cnt + 1)
                {
                  fprintf(stderr, "Line %i has a different number of columns (%i) than previous lines (%i)\n",
                          (int)row + linecount + 1, cnt + 1, max_col);
                  return GRM_ERROR_PLOT_MISSING_DATA;
                }
              try
                {
                  trim(token);
                  token.erase(std::remove(token.begin(), token.end(), '\t'), token.end());
                  if (token.empty() && delim == ',')
                    {
                      data[depth][cnt].push_back(NAN);
                    }
                  else
                    {
                      struct tm timestamp_tm;
                      if (parseIso8601WithoutTimezone(token, timestamp_tm))
                        {
                          data[depth][cnt].push_back((int)mktime(&timestamp_tm));
                          timestamps.emplace_back(col);
                          x_columns.emplace_back(col + 1);
                        }
                      else
                        {
                          try
                            {
                              data[depth][cnt].push_back(std::stod(token));
                            }
                          catch (const std::out_of_range e)
                            {
                              fprintf(stderr,
                                      "Invalid entry found. Check ur data if there are more than 1 delimiter or if "
                                      "there is another mistake inside of the file. Row:%lu, detected value:\"%s\"\n",
                                      row + linecount + 1, token.c_str());
                              return GRM_ERROR_PLOT_MISSING_DATA;
                            }
                          if (!timestamps.empty()) y_columns.emplace_back(col + 1);
                        }
                    }
                }
              catch (std::invalid_argument &e)
                {
                  fprintf(stderr, "Invalid number in line %zu, column %zu (%s)\n", row + linecount + 1, col + 1,
                          token.c_str());
                  return GRM_ERROR_PARSE_DOUBLE;
                }
              if (row - skipped == 0 && !x_columns.empty() &&
                  std::find(x_columns.begin(), x_columns.end(), col + 1) != x_columns.end())
                x_data.emplace_back(cnt + 1);
              if (row - skipped == 0 && !y_columns.empty() &&
                  std::find(y_columns.begin(), y_columns.end(), col + 1) != y_columns.end())
                y_data.emplace_back(cnt + 1);
              if (row - skipped == 0 && !e_columns.empty() &&
                  std::find(e_columns.begin(), e_columns.end(), col + 1) != e_columns.end())
                error_data.emplace_back(cnt + 1);
              cnt += 1;
            }
          else if (input_flags.use_bins && col == 0)
            {
              try
                {
                  if (row - skipped == 0)
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
          return GRM_ERROR_PLOT_MISSING_DATA;
        }
    }
  // Restore locale setting
  std::setlocale(LC_NUMERIC, old_locale.c_str());
  return GRM_ERROR_NONE;
}

DataSource *CsvPlugin::getDataSourceFromFile(const std::string &path) const
{
  return new CsvSource;
}

const Plugin::ApiVersion CsvPlugin::apiVersion() const
{
  return ApiVersion{1, 0};
}

#ifdef __EMSCRIPTEN__
extern "C" void *CsvAllocPlugin()
#else
extern "C" void *allocPlugin()
#endif
{
  return new CsvPlugin;
}
#ifdef __EMSCRIPTEN__
extern "C" void CsvDeallocPlugin(void *p)
#else
extern "C" void deallocPlugin(void *p)
#endif
{
  auto d = static_cast<Plugin *>(p);
  delete d;
}
