#ifndef GRM_UTIL_INT_HXX_INCLUDED
#define GRM_UTIL_INT_HXX_INCLUDED

/* ######################### includes ############################################################################### */

#include <grm/util.h>
#include <cassert>
#include <numeric>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <complex>


/* ######################### internal interface ##################################################################### */

/* ========================= macros ================================================================================= */

#define WHITESPACE " \n\r\t\f\v"

/* ========================= functions ============================================================================== */

/* ------------------------- util ----------------------------------------------------------------------------------- */

std::string_view ltrim(std::string_view s);
std::string_view rtrim(std::string_view s);
std::string_view trim(std::string_view s);
bool starts_with(std::string_view str, std::string_view prefix);
bool ends_with(std::string_view str, std::string_view suffix);
size_t ends_with_any_subprefix(std::string_view str, std::string_view prefix);
size_t string_consists_of(std::string_view input, char c, char ends_with, size_t pos = 0);

template <typename Iterator> std::string string_join(Iterator first, Iterator last, std::string_view delimiter = "")
{
  if (first == last)
    {
      return std::string{};
    }
  auto output_length = std::accumulate(first, last, 0, [](size_t sum, const auto &s) { return sum + s.size(); }) +
                       (last - first - 1) * delimiter.size();
  std::string output;
  output.reserve(output_length);
  for (; first != last - 1; ++first)
    {
      output += *first;
      output += delimiter;
    }
  output += *first;
  assert(output.size() == output_length);
  return output;
}

std::string escape_double_minus(std::string_view input);
std::string unescape_double_minus(std::string_view input);

std::optional<std::string_view> is_backup_attribute_for(std::string_view name);

bool file_exists(const std::string &name);

void linspace(double start, double end, int n, std::vector<double> &x);

void listcomprehension(double factor, double (*pFunction)(double), std::vector<double> &list, int num, int start,
                       std::vector<double> &result);

std::complex<double> moivre(double r, int x, int n);

template <typename... Args> constexpr bool str_equals_any(std::string_view target, const Args &...args)
{
  return ((target == args) || ...);
}

bool is_number(std::string_view str);

#endif // GRM_UTIL_INT_HXX_INCLUDED
