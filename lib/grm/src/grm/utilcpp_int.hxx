#ifndef GRM_UTIL_INT_HXX_INCLUDED
#define GRM_UTIL_INT_HXX_INCLUDED

/* ######################### includes ############################################################################### */

#include <grm/util.h>
#include <string>
#include <vector>


/* ######################### internal interface ##################################################################### */

/* ========================= macros ================================================================================= */

#define WHITESPACE " \n\r\t\f\v"

/* ========================= functions ============================================================================== */

/* ------------------------- util ----------------------------------------------------------------------------------- */

std::string ltrim(const std::string &s);
std::string rtrim(const std::string &s);
std::string trim(const std::string &s);
bool file_exists(const std::string &name);
bool starts_with(const std::string &str, const std::string &prefix);
bool ends_with(const std::string &str, const std::string &suffix);

void linspace(double start, double end, int n, std::vector<double> &x);

void listcomprehension(double factor, double (*pFunction)(double), std::vector<double> &list, int num, int start,
                       std::vector<double> &result);

#endif // GRM_UTIL_INT_HXX_INCLUDED
