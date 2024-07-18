#define _USE_MATH_DEFINES

#include "utilcpp_int.hxx"
#include <cmath>
#include <list>
#include <algorithm>
#include <string_view>

#ifdef _WIN64
#include <stdlib.h>
#include <io.h>
#include <process.h>
#include <direct.h>
#define F_OK 0
#define access _access
#else
#include <unistd.h>
#endif

std::string ltrim(const std::string &s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

std::string rtrim(const std::string &s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

std::string trim(const std::string &s)
{
  return rtrim(ltrim(s));
}

bool file_exists(const std::string &name)
{
  return (access(name.c_str(), F_OK) != -1);
}

bool starts_with(std::string_view str, std::string_view prefix)
{
  return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

bool ends_with(const std::string &str, const std::string &suffix)
{
  return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

void linspace(double start, double end, int n, std::vector<double> &x)
{
  int i;
  if (x.size() < n)
    {
      x.resize(n);
    }
  for (i = 0; i < n; i++)
    {
      x[i] = (start + i * (end - start) / (n - 1));
    }
}

/* like python list comprehension [factor * func(element) for element in list] saves values in result starting at start
 * index */
void listcomprehension(double factor, double (*pFunction)(double), std::vector<double> &list, int num, int start,
                       std::vector<double> &result)
{
  int i;
  if (result.size() < num)
    {
      result.resize(num);
    }

  for (i = 0; i < num; ++i)
    {
      // just in case if start + num + 1 exceeds the size of the vector
      if (i + start >= result.size())
        {
          break;
        }
      result[i + start] = factor * (*pFunction)(list[i]);
    }
}

std::complex<double> moivre(double r, int x, int n)
{
  if (n != 0)
    {
      return {pow(r, (1.0 / n)) * (cos(2.0 * x * M_PI / n)), pow(r, (1.0 / n)) * (sin(2.0 * x * M_PI / n))};
    }
  else
    {
      return {1.0, 0.0};
    }
}

bool is_number(std::string_view str)
{
  const char minus[3] = {(char)0xe2, (char)0x88, (char)0x92}; // gr minus sign
  auto em_dash = std::string(minus);
  size_t start_pos = 0;
  if (starts_with(str, em_dash))
    {
      start_pos = em_dash.size();
    }
  auto pos = str.find_first_not_of(".-0123456789", start_pos);
  return pos == std::string::npos;
}
