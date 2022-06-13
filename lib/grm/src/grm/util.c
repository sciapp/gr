#ifdef __unix__
#define _POSIX_C_SOURCE 200112L
#endif

/* ######################### includes ############################################################################### */

#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <string.h>

#include "gkscore.h"

#include "grm/error.h"
#include "json_int.h"
#include "util_int.h"


/* ######################### internal implementation ################################################################ */

/* ========================= macros ================================================================================= */

#define PRIVATE_NAME_BUFFER_LEN 80


/* ========================= functions ============================================================================== */

/* ------------------------- util ----------------------------------------------------------------------------------- */

void bin_data(unsigned int n, double *x, unsigned int num_bins, double *bins, double *weights)
{
  double x_min = DBL_MAX, x_max = -DBL_MAX;
  unsigned int i;

  for (i = 0; i < n; ++i)
    {
      x_min = grm_min(x[i], x_min);
      x_max = grm_max(x[i], x_max);
    }
  memset(bins, 0, num_bins * sizeof(double));
  for (i = 0; i < n; ++i)
    {
      unsigned int current_bin = (int)((x[i] - x_min) / (x_max - x_min) * num_bins);
      if (current_bin == num_bins) --current_bin;
      bins[current_bin] += (weights != NULL) ? weights[i] : 1;
    }
}

void linspace(double start, double end, unsigned int n, double *x)
{
  unsigned int i;
  for (i = 0; i < n; i++)
    {
      x[i] = start + i * (end - start) / (n - 1);
    }
}

size_t djb2_hash(const char *str)
{
  /* String hash function by Dan Bernstein, see http://www.cse.yorku.ca/~oz/hash.html */
  size_t hash = 5381;
  char c;

  while ((c = *str++) != '\0')
    {
      hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

  return hash;
}

int is_equidistant_array(unsigned int length, const double *x)
{
  if (x != NULL && length > 2)
    {
      double distance = x[1] - x[0];
      unsigned int i;
      for (i = 2; i < length; ++i)
        {
          if (fabs((x[i] - x[i - 1]) - distance) > FEPS)
            {
              return 0;
            }
        }
    }
  return 1;
}

int is_int_number(const char *str)
{
  return strchr(FROMJSON_VALID_DELIMITERS, str[strspn(str, "0123456789-+")]) != NULL;
}

int str_to_uint(const char *str, unsigned int *value_ptr)
{
  char *conversion_end = NULL;
  unsigned long conversion_result;
  int success = 0;

  errno = 0;
  if (str != NULL && *str != '\0')
    {
      conversion_result = strtoul(str, &conversion_end, 10);
    }
  else
    {
      conversion_result = 0;
    }
  if (conversion_end == NULL || *conversion_end != '\0')
    {
      debug_print_error(("The parameter \"%s\" is not a valid number!\n", str));
    }
  else if (errno == ERANGE || conversion_result > UINT_MAX)
    {
      debug_print_error(("The parameter \"%s\" is too big, the number has been clamped to \"%u\"\n", str, UINT_MAX));
      conversion_result = UINT_MAX;
    }
  else
    {
      success = 1;
    }
  if (value_ptr != NULL)
    {
      *value_ptr = (unsigned int)conversion_result;
    }

  return success;
}

int int_equals_any(int number, unsigned int n, ...)
{
  va_list vl;
  int current_number;
  int any_is_equal;
  unsigned int i;

  va_start(vl, n);
  any_is_equal = 0;
  for (i = 0; i < n; i++)
    {
      current_number = va_arg(vl, int);
      if (number == current_number)
        {
          any_is_equal = 1;
          break;
        }
    }
  va_end(vl);

  return any_is_equal;
}

int str_equals_any(const char *str, unsigned int n, ...)
{
  va_list vl;
  char *current_str;
  int any_is_equal;
  unsigned int i;

  va_start(vl, n);
  any_is_equal = 0;
  for (i = 0; i < n; i++)
    {
      current_str = va_arg(vl, char *);
      if (strcmp(str, current_str) == 0)
        {
          any_is_equal = 1;
          break;
        }
    }
  va_end(vl);

  return any_is_equal;
}

int str_equals_any_in_array(const char *str, const char **str_array)
{
  /* `str_array` must be NULL terminated */
  const char **current_str_ptr;
  int any_is_equal;

  any_is_equal = 0;
  current_str_ptr = str_array;
  while (*current_str_ptr != NULL)
    {
      if (strcmp(str, *current_str_ptr) == 0)
        {
          any_is_equal = 1;
          break;
        }
      ++current_str_ptr;
    }

  return any_is_equal;
}

int uppercase_count(const char *str)
{
  int uppercase_count = 0;

  while (*str)
    {
      if (isupper(*str))
        {
          ++uppercase_count;
        }
      ++str;
    }
  return uppercase_count;
}

char *str_filter(const char *str, const char *filter_chars)
{
  char *reduced_str;
  const char *src_ptr;
  char *dst_ptr;

  reduced_str = malloc(strlen(str) + 1);
  if (reduced_str == NULL)
    {
      return NULL;
    }
  src_ptr = str;
  dst_ptr = reduced_str;
  while (*src_ptr != '\0')
    {
      if (strchr(filter_chars, *src_ptr) == NULL)
        {
          *dst_ptr = *src_ptr;
          ++dst_ptr;
        }
      ++src_ptr;
    }
  *dst_ptr = '\0';

  return reduced_str;
}

int is_homogenous_string_of_char(const char *str, char c)
{
  const char *current_format_char_ptr = str;
  while (*current_format_char_ptr != '\0')
    {
      if (*current_format_char_ptr != c)
        {
          break;
        }
      ++current_format_char_ptr;
    }
  return *current_format_char_ptr == '\0';
}

const char *private_name(const char *public_name)
{
  static char private_name_buffer[PRIVATE_NAME_BUFFER_LEN];

  snprintf(private_name_buffer, PRIVATE_NAME_BUFFER_LEN, "_%s", public_name);

  return private_name_buffer;
}
unsigned long next_or_equal_power2(unsigned long num)
{
#if defined(__GNUC__) || defined(__clang__)
  /* Subtract the count of leading bit zeros from the count of all bits to get `floor(log2(num)) + 1`. If `num` is a
   * power of 2 (only one bit is set), subtract 1 more. The result is `ceil(log2(num))`. Calculate
   * `1 << ceil(log2(num))` to get `exp2(ceil(log2(num)))` which is the power of 2 which is greater or equal than `num`.
   */
  return 1ul << ((CHAR_BIT * sizeof(unsigned long)) - __builtin_clzl(num) - (__builtin_popcountl(num) == 1 ? 1 : 0));
#elif defined(_MSC_VER)
  /* Calculate the index of the highest set bit (bit scan reverse) to get `floor(log2(num)) + 1`. If `num` is a power
   * of 2 (only one bit is set), subtract 1 more. The result is `ceil(log2(num))`. Calculate `1 << ceil(log2(num))`
   * to get `exp2(ceil(log2(num)))` which is the power of 2 which is greater or equal than `num`.
   */
  unsigned long index;
  _BitScanReverse(&index, num);
  return 1ul << (index + 1 - (__popcnt(num) == 1 ? 1 : 0));
#else
  /* Fallback algorithm in software: Shift one bit till the resulting number is greater or equal than `num` */
  unsigned long power = 1;
  while (power < num)
    {
      power <<= 1;
    }
  return power;
#endif
}


/* ######################### public implementation ################################################################## */

/* ========================= functions ============================================================================== */

/* ------------------------- util ----------------------------------------------------------------------------------- */

#if !defined(NDEBUG) && defined(EMSCRIPTEN)
FILE *grm_get_stdout(void)
{
  return stdout;
}
#endif
