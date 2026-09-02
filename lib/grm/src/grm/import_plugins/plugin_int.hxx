#ifndef PLUGIN_INT_HXX_INCLUDED
#define PLUGIN_INT_HXX_INCLUDED

#include <string>

#include "grm.h"
#include "grm/import_int.hxx"

class DataSource
{
public:
  DataSource() = default;
  virtual ~DataSource() = default;

  /*
   * Output parameters are passed to the plugin as vector, which already contains data for previously parsed plots. The
   * plugin is expected to append a new entry for every plot the current file (passed as `path`) defines to each vector
   * (output vectors must all have the same size after reading the file).
   * Parameters, which are both input and output may already contain data for a plot in the given file (e.g.
   * specified on the command line). Therefore these parameters already contain a new element for the first plot upon
   * calling `readDataFile`.
   *
   * In parameters:
   * `path`, `colms`, `x_colms`, `y_colms`, `e_colms`, `input_flags`
   * `args`, `ranges`, `special_axis_series`
   *
   * Out parameters:
   * `data`, `x_data`, `y_data`, `error_data`, `labels`, `timestamps`, `special_data_grids`
   * `args`, `ranges`, `special_axis_series`
   */
  virtual grm_error_t
  readDataFile(const std::string &path, std::vector<std::vector<std::vector<std::vector<double>>>> &data,
               std::vector<std::vector<int>> &x_data, std::vector<std::vector<int>> &y_data,
               std::vector<std::vector<int>> &error_data, std::vector<std::vector<std::string>> &labels,
               std::vector<grm_args_t *> &args, const char *colms, const char *x_colms, const char *y_colms,
               const char *e_colms, std::vector<PlotRange> &ranges,
               std::vector<grm_special_axis_series_t *> &special_axis_series, InputFlags &input_flags,
               std::vector<std::vector<int>> &timestamps, std::vector<double *> special_data_grids) = 0;
};

class Plugin
{
public:
  Plugin() = default;
  virtual ~Plugin() = default;

  virtual DataSource *getDataSourceFromFile(const std::string &path) const = 0;

  struct ApiVersion
  {
    int major;
    int minor;
  };
  virtual const ApiVersion apiVersion() const = 0;
};

#endif /* ifndef PLUGIN_INT_HXX_INCLUDED */