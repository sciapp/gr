#ifndef GRM_UTIL_INT_HXX_INCLUDED
#define GRM_UTIL_INT_HXX_INCLUDED

/* ######################### includes ############################################################################### */

#include <grm/util.h>
#include <string>


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

#endif // GRM_UTIL_INT_HXX_INCLUDED
