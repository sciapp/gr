#ifndef GRM_IMPORT_INT_HXX_INCLUDED
#define GRM_IMPORT_INT_HXX_INCLUDED

/* ######################### includes ############################################################################### */

#include <grm/import.h>
#include <string>
#include <cstring>
#include <vector>


/* ######################### internal interface ##################################################################### */

/* ========================= datatypes ============================================================================== */

/* ------------------------- plot ----------------------------------------------------------------------------------- */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plot ranges ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

struct PlotRange
{
  double xmin, xmax;
  double ymin, ymax;
  double zmin, zmax;
};

/* ------------------------- argument container --------------------------------------------------------------------- */

struct _grm_file_args_t
{
  std::string file_path;
  std::string file_columns;
  std::string file_x_columns;
  std::string file_y_columns;
  std::string file_error_columns;
};

struct _grm_special_axis_series_t
{
  std::string bottom;
  std::string left;
  std::string right;
  std::string top;
  std::string twin_x;
  std::string twin_y;
};

/* ========================= functions ============================================================================== */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ import ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

std::string normalizeLine(const std::string &str);
grm_error_t parseColumns(std::list<int> *columns, const char *colms);
grm_error_t readDataFile(const std::string &path, std::vector<std::vector<std::vector<double>>> &data,
                         std::vector<int> &x_data, std::vector<int> &y_data, std::vector<int> &error_data,
                         std::vector<std::string> &labels, grm_args_t *args, const char *colms, const char *x_colms,
                         const char *y_colms, const char *e_colms, PlotRange *ranges);
int convertInputstreamIntoArgs(grm_args_t *args, grm_file_args_t *file_args, int argc, char **argv, PlotRange *ranges,
                               grm_special_axis_series_t *special_axis_series);
grm_file_args_t *grm_file_args_new();
grm_special_axis_series_t *grm_special_axis_series_new();
void parseParameterDD(std::string *input, const std::string *key, std::string *x, std::string *y);
void parseParameterDDD(std::string *input, const std::string *key, std::string *r, std::string *g, std::string *b);
int parseParameterNI(std::string *input, const std::string *key, std::vector<int> *values);
int parseParameterNS(std::string *input, const std::string *key, std::vector<std::string> *values);
int parseParameterND(std::string *input, const std::string *key, std::vector<double> *values);

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ utility ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void adjustRanges(double *, double *, double, double);

#endif // GRM_IMPORT_INT_HXX_INCLUDED
