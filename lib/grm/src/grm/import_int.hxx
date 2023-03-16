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
};

/* ========================= functions ============================================================================== */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ import ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

std::string normalize_line(const std::string &str);
err_t read_data_file(const std::string &path, std::vector<std::vector<std::vector<double>>> &data,
                     std::vector<std::string> &labels, grm_args_t *args, const char *colms, PlotRange *ranges);
int convert_inputstream_into_args(grm_args_t *args, grm_file_args_t *file_args, int argc, char **argv);
grm_file_args_t *grm_file_args_new();
void parse_parameter_ddd(std::string *input, const std::string *key, std::string *r, std::string *g, std::string *b);
int parse_parameter_nI(std::string *input, const std::string *key, std::vector<int> values);
int parse_parameter_nS(std::string *input, const std::string *key, std::vector<std::string> *values);
int parse_parameter_nD(std::string *input, const std::string *key, std::vector<double> values);

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ utility ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void adjust_ranges(double *, double *, double, double);

#endif // GRM_IMPORT_INT_HXX_INCLUDED
