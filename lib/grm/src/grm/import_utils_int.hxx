#ifndef IMPORT_UTILS_INT_HXX_INCLUDED
#define IMPORT_UTILS_INT_HXX_INCLUDED

/* ######################### includes ############################################################################### */

#include <list>
#include <string>
#include <vector>
#include <grm/args.h>
#include <grm/error.h>

/* ######################### internal interface ##################################################################### */

#define MAX_LEN 4096

/* ========================= datatypes ============================================================================== */

/* ------------------------- argument container --------------------------------------------------------------------- */

struct _grm_file_args_t
{
  std::string file_path;
  std::string file_columns;
  std::string file_x_columns;
  std::string file_y_columns;
  std::string file_error_columns;
};
typedef struct _grm_file_args_t grm_file_args_t;

struct _grm_special_axis_series_t
{
  std::string bottom;
  std::string left;
  std::string right;
  std::string top;
  std::string twin_x;
  std::string twin_y;
};
typedef struct _grm_special_axis_series_t grm_special_axis_series_t;

/* ------------------------- plot ----------------------------------------------------------------------------------- */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plot ranges ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

struct PlotRange
{
  double xmin, xmax;
  double ymin, ymax;
  double zmin, zmax;
};

/* ------------------------- user input flags ----------------------------------------------------------------------- */

struct InputFlags
{
  int scatter_with_z;
  int use_bins;
  int equal_up_and_down_error;
  int xye_file;
  int xyz_file;
  bool default_kind_used;
  std::string error_type;
  bool ignore_blank_lines;
  bool force_legend_line;

  InputFlags() { reset(); }

  void reset()
  {
    scatter_with_z = 0;
    use_bins = 0;
    equal_up_and_down_error = 0;
    xye_file = 0;
    xyz_file = 0;
    default_kind_used = false;
    error_type = "relative";
    ignore_blank_lines = false;
    force_legend_line = false;
  }
};

/* ========================= functions ============================================================================== */

/* ------------------------- import --------------------------------------------------------------------------------- */

bool isValidKind(const std::string &kind);
grm_error_t parseColumns(std::list<int> *columns, const char *colms);
void parseParameterDD(std::string *input, const std::string *key, std::string *x, std::string *y);
void parseParameterDDD(std::string *input, const std::string *key, std::string *r, std::string *g, std::string *b);
int parseParameterNI(std::string *input, const std::string *key, std::vector<int> *values);
int parseParameterNS(std::string *input, const std::string *key, std::vector<std::string> *values);
int parseParameterND(std::string *input, const std::string *key, std::vector<double> *values);
std::vector<std::string> singleTokenConverter(std::string token, grm_args_t *args, PlotRange *ranges,
                                              grm_special_axis_series_t *special_axis_series, InputFlags &input_flags,
                                              int line_count = -1);

#endif /* ifndef IMPORT_UTILS_INT_HXX_INCLUDED */
