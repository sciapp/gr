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

  virtual grm_error_t readDataFile(const std::string &path, std::vector<std::vector<std::vector<double>>> &data,
                                   std::vector<int> &x_data, std::vector<int> &y_data, std::vector<int> &error_data,
                                   std::vector<std::string> &labels, grm_args_t *args, const char *colms,
                                   const char *x_colms, const char *y_colms, const char *e_colms, PlotRange *ranges,
                                   grm_special_axis_series_t *special_axis_series, InputFlags &input_flags,
                                   std::vector<int> &timestamps, double **special_data_grid) = 0;
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