#define _USE_MATH_DEFINES

/* ######################### includes ############################################################################### */

#include "utilcpp_int.hxx"
#include <cmath>
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


/* ######################### internal implementation ################################################################ */

/* ========================= functions ============================================================================== */

/* ------------------------- util ----------------------------------------------------------------------------------- */

std::string_view ltrim(std::string_view s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

std::string_view rtrim(std::string_view s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

std::string_view trim(std::string_view s)
{
  return rtrim(ltrim(s));
}

bool starts_with(std::string_view str, std::string_view prefix)
{
  return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

bool ends_with(std::string_view str, std::string_view suffix)
{
  return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

/*!
 * \brief Check if a given string ends with any subprefix of a given prefix (excluding the prefix itself). For example,
 * the string "abcdefg" ends with any subprefix of "fghij" (subprefix "fg")."
 *
 * \param[in] str The input string which is checked at the end.
 * \param[in] prefix The prefix which will be shortened to any subprefix (removing one character at the end in each
 *            step).
 * \return The position of a found subprefix. If no subprefix is found, `std::string_view::npos` is returned.
 */
size_t ends_with_any_subprefix(std::string_view str, std::string_view prefix)
{
  for (auto i = prefix.size() - 1; i > 0; --i)
    {
      if (ends_with(str, prefix.substr(0, i)))
        {
          return str.size() - i;
        }
    }

  return std::string_view::npos;
}

/*!
 * \brief Check if a substring of a given input string consists of a certain character, ending with an another end
 *        character.
 *
 * \param[in] input The string to check.
 * \param[in] c The character to check for.
 * \param[in] ends_with The end character to check for.
 * \param[in] pos The starting position of the substring. All characters starting from `&input[pos]` will be checked.
 * \return The index of the ending character. If the substring does not consist of `c` and `ends_with`,
 *         `std::string_view::npos` is returned instead.
 */
size_t string_consists_of(std::string_view input, char c, char ends_with, size_t pos)
{
  auto it_not_c =
      std::find_if_not(std::begin(input) + pos, std::end(input), [&](char current_char) { return current_char == c; });
  if (it_not_c == std::end(input) || *it_not_c != ends_with)
    {
      return std::string_view::npos;
    }
  return it_not_c - std::begin(input);
}

namespace internal
{
/*!
 * \brief Escape or unescape two consecutive characters (like `--`) in a string.
 *
 * This function can be used to escape or unescape two consecutive characters in a string by putting a given escape
 * character in between them. To simplfy the explanation, assume the character which should be escaped is `-` and the
 * escape character is `\`. Occurences of `--` in the string will be replaced with `-\-` and occurrences of `-\-` will
 * be replaced with `-\\-` and so forth. Strings with even more consecutive characters (`---` or more) will also be
 * handled correctly (`---` will be replaced with `-\-\-`, `-\--` will be replaced by `-\\-\-` and so forth).
 *
 * \param[in] input The string to escape or unescape.
 * \param[in] escape_char The character which will be used as an escape character (`\` in the above example)
 * \param[in] to_escape_char The character which will be escaped if two consecutive characters are found (`-` in the
 *                           above example).
 * \param[in] unescape Can be set to `true` to unescape instead.
 */
std::string escape_or_unescape(std::string_view input, char escape_char, char to_escape_char, bool unescape)
{
  std::vector<std::string_view> output_parts;

  auto start_pos = input.find(to_escape_char);
  while (start_pos != std::string_view::npos)
    {
      auto end_pos = string_consists_of(input, escape_char, to_escape_char, start_pos + 1);
      if (end_pos != std::string_view::npos)
        {
          /* Explanation of `(unescape && end_pos - start_pos > 1 ? 1 : 0)`:
           *   In unescape mode, one escape character needs to be removed, so substract one from the found end position.
           *   However, if the routine found an already unescaped occurence (`--` instead of `-\-`), skip the
           *   substraction to not lose a non-escape character. */
          output_parts.push_back(input.substr(0, end_pos - (unescape && end_pos - start_pos > 1 ? 1 : 0)));
          // Leave the second minus of `--` in the input, so strings like `---` can be handled in the next correctly.
          input = input.substr(end_pos);
          start_pos = 0;
        }
      else
        {
          start_pos = input.find(to_escape_char, start_pos + 1);
        }
    }
  output_parts.push_back(input);

  return string_join(std::begin(output_parts), std::end(output_parts), unescape ? "" : std::string{escape_char});
}
} // namespace internal

/*!
 * \brief Escape double minus signs (`--`) in a string, by replacing them with `-\-`. See `escape_or_unescape` for more
 *        details.
 *
 * \param[in] input String to escape.
 * \return The escaped string.
 */
std::string escape_double_minus(std::string_view input)
{
  return internal::escape_or_unescape(input, '\\', '-', false);
}

/*!
 * \brief Unescape escaped double minus signs (`-\-`) in a string, by replacing them with `--`. See `escape_or_unescape`
 *        for more details.
 *
 * \param[in] input String to unescape.
 * \return The unescaped string.
 */
std::string unescape_double_minus(std::string_view input)
{
  return internal::escape_or_unescape(input, '\\', '-', true);
}

/*!
 * \brief Check if an attribute is a backup copy of another attribute of a graphics tree element.
 *
 * This function makes a decision based only on the attribute name itself, there is no graphcs tree check involved!
 *
 * \param[in] name The attribute name to check.
 * \return The attribute name of the original attribute if the input is a backup attribute, `std::nullopt` otherwise.
 */
std::optional<std::string_view> is_backup_attribute_for(std::string_view name)
{
  if (name.empty() || !(name[0] == '_' && ends_with(name, "_org") && name.size() > 5)) return std::nullopt;

  return name.substr(1, name.size() - 5);
}

bool file_exists(const std::string &name)
{
  return (access(name.c_str(), F_OK) != -1);
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
  const char minus[] = {(char)0xe2, (char)0x88, (char)0x92, '\0'}; // gr minus sign
  auto em_dash = std::string(minus);
  size_t start_pos = 0;
  if (starts_with(str, em_dash))
    {
      start_pos = em_dash.size();
    }
  auto pos = str.find_first_not_of(".-0123456789", start_pos);
  return pos == std::string::npos;
}

double round(double val, int digits)
{
  if (digits < 0) return (round((val * pow(0.1, digits)) + ((val < 0) ? -0.5 : 0.5))) / pow(0.1, digits);
  return (round((val * pow(0.1, digits)) + ((val < 0) ? -0.5 : 0.5) * pow(0.1, digits))) / pow(0.1, digits);
}

double ceil(double val, int digits)
{
  return ceil(val * pow(0.1, digits)) / pow(0.1, digits);
}

double floor(double val, int digits)
{
  return floor(round(val, -15) * pow(0.1, digits)) / pow(0.1, digits);
}


/* ========================= methods ================================================================================ */

/* ------------------------- IdPool --------------------------------------------------------------------------------- */

template <typename T> IdPool<T>::IdPool(T start_id) : start_id_(start_id) {}

template <typename T> T IdPool<T>::current()
{
  if (current_id_)
    {
      return current_id_.value();
    }
  else
    {
      throw NoCurrentIdError();
    }
}

template <typename T> T IdPool<T>::next()
{
  T next_id;
  if (used_id_ranges_.empty())
    {
      // There are no IDs in use, so the next ID is the start ID
      next_id = start_id_;
      used_id_ranges_.emplace_front(next_id, next_id);
    }
  else if (used_id_ranges_.front().first != start_id_)
    {
      // The `start_id_` was released in a previous operation, so use it as the next ID
      next_id = start_id_;
      if (used_id_ranges_.front().first == start_id_ + 1)
        {
          /* In this case, only the `start_id_` is missing from the first interval, so we can just add it, for
           * example:
           * ```
           * [1, 10] -> [0, 10]  (`start_id_` is `0`)
           * ``` */
          used_id_ranges_.front().first = next_id;
        }
      else
        {
          /* Otherwise, multiple IDs are missing from the first interval, for example:
           * ```
           * [5, 10]
           * ```
           * so create a new interval with the `start_id_` and add it to the front of the list:
           * ```
           * [0, 0], [5, 10]
           * ``` */
          used_id_ranges_.emplace_front(next_id, next_id);
        }
    }
  else
    {
      next_id = used_id_ranges_.front().second + 1;
      if (used_id_ranges_.size() > 1 && (++std::cbegin(used_id_ranges_))->first == next_id + 1)
        {
          /* In this case we have a gap of a single value between the first two intervals, for example:
           * ```
           * [0, 9], [11, 20]
           * ```
           * so `next_id` is `10` and both intervals can be merged to a single interval `[0, 20]`. */
          (++std::begin(used_id_ranges_))->first = start_id_;
          used_id_ranges_.pop_front();
        }
      else
        {
          used_id_ranges_.front().second = next_id;
        }
    }
  current_id_ = next_id;
  return next_id;
}

template <typename T> void IdPool<T>::release(T id)
{
  for (auto it = std::begin(used_id_ranges_); it != std::end(used_id_ranges_); ++it)
    {
      if (it->first <= id && id <= it->second)
        {
          if (it->first == it->second)
            {
              // In this case, the interval only consists of one value (`id`), so we can just remove it.
              used_id_ranges_.erase(it);
            }
          else if (it->first == id)
            {
              /* `id` is at the start of the interval and the interval consists of multiple values, for example
               * ```
               * [10, 12] with id = 10
               * ```
               * so increment the start value of the interval, so we get:
               * ```
               * [11, 12]
               * ``` */
              ++(it->first);
            }
          else if (it->second == id)
            {
              /* `id` is at the end of the interval and the interval consists of multiple values, for example
               * ```
               * [10, 12] with id = 12
               * ```
               * so decrement the end value of the interval, so we get:
               * ```
               * [10, 11]
               * ``` */
              --(it->second);
            }
          else
            {
              /* In this case, `id` is in the middle of the interval, so split it into two intervals, for example:
               * ```
               * [10, 12] with id = 11
               * ```
               * so we get:
               * ```
               * [10, 10], [12, 12]
               * ``` */
              auto second = it->second;
              it->second = id - 1;
              used_id_ranges_.emplace(++it, id + 1, second);
            }
          if (id == current_id_)
            {
              current_id_.reset();
            }
          return;
        }
    }
  throw IdNotFoundError(id);
}

template <typename T> void IdPool<T>::reset()
{
  used_id_ranges_.clear();
  current_id_.reset();
}

/* Generate code for int IDs since this is needed in the code */
template class IdPool<int>;
