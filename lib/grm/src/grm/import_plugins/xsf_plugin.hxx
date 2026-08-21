#ifndef XSF_PLUGIN_HXX_INCLUDED
#define XSF_PLUGIN_HXX_INCLUDED

#include "plugin_int.hxx"
#include "grm/util.h"

class XsfSource : public DataSource
{
public:
  XsfSource() = default;
  virtual ~XsfSource() = default;

  GRM_EXPORT grm_error_t readDataFile(const std::string &path, std::vector<std::vector<std::vector<double>>> &data,
                                      std::vector<int> &x_data, std::vector<int> &y_data, std::vector<int> &error_data,
                                      std::vector<std::string> &labels, grm_args_t *args, const char *colms,
                                      const char *x_colms, const char *y_colms, const char *e_colms, PlotRange *ranges,
                                      grm_special_axis_series_t *special_axis_series, InputFlags &input_flags,
                                      std::vector<int> &timestamps, double **special_data_grid) override;

private:
  std::string normalizeLine(const std::string &str);
};

class XsfPlugin : public Plugin
{
public:
  XsfPlugin() = default;
  virtual ~XsfPlugin() = default;

  DataSource *getDataSourceFromFile(const std::string &path) const override;

  const ApiVersion apiVersion() const override;
};

#ifdef __EMSCRIPTEN__
extern "C" GRM_EXPORT void *XsfAllocPlugin();
extern "C" GRM_EXPORT void XsfDeallocPlugin(void *p);
#else
extern "C" GRM_EXPORT void *allocPlugin();
extern "C" GRM_EXPORT void deallocPlugin(void *p);
#endif

#endif /* ifndef XSF_PLUGIN_HXX_INCLUDED */
