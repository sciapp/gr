#ifndef GRM_UTIL_INT_H_INCLUDED
#define GRM_UTIL_INT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#include <stdlib.h>

#include <grm/args.h>
#include "logging_int.h"
#include <grm/util.h>


/* ######################### internal interface ##################################################################### */

/* ========================= macros ================================================================================= */

/* ------------------------- util ----------------------------------------------------------------------------------- */

#ifndef PATH_SEPARATOR
#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif
#endif

#define is_string_delimiter(char_ptr, str) ((*(char_ptr) == '"') && (((char_ptr) == (str)) || *((char_ptr)-1) != '\\'))

#ifndef array_size
#define array_size(a) ((sizeof(a) / sizeof(*(a))))
#endif

#ifndef INFINITY
#define INFINITY (1.0 / 0.0)
#endif

#define grm_isnan(x) ((x) != (x))
#define grm_min(x, y) (((x) < (y)) ? (x) : (y))
#define grm_max(x, y) (((x) > (y)) ? (x) : (y))
#define grm_round(x) (((x) < 0) ? ceil((x)-.5) : floor((x) + .5))
#define grm_static_assert(cond, message) ((void)sizeof(char[(cond) ? 1 : -1]))

/* test macros which can be used like `assert` */
#define return_error_if(condition, error_value)                                                    \
  do                                                                                               \
    {                                                                                              \
      if (condition)                                                                               \
        {                                                                                          \
          logger((stderr, "Got error \"%d\" (\"%s\")!\n", error_value, error_names[error_value])); \
          return (error_value);                                                                    \
        }                                                                                          \
    }                                                                                              \
  while (0)
#define return_if_error return_error_if((error) != ERROR_NONE, (error))
#define goto_if(condition, goto_label) \
  do                                   \
    {                                  \
      if (condition)                   \
        {                              \
          goto goto_label;             \
        }                              \
    }                                  \
  while (0)
#define cleanup_if(condition) goto_if((condition), cleanup)
#define error_cleanup_if(condition) goto_if((condition), error_cleanup)
#define goto_if_error(goto_label)                                                          \
  do                                                                                       \
    {                                                                                      \
      if ((error) != ERROR_NONE)                                                           \
        {                                                                                  \
          logger((stderr, "Got error \"%d\" (\"%s\")!\n", (error), error_names[(error)])); \
          goto goto_label;                                                                 \
        }                                                                                  \
    }                                                                                      \
  while (0)
#define cleanup_if_error goto_if_error(cleanup)
#define error_cleanup_if_error goto_if_error(error_cleanup)
#define goto_and_set_error_if(condition, error_value, goto_label)                                      \
  do                                                                                                   \
    {                                                                                                  \
      if (condition)                                                                                   \
        {                                                                                              \
          error = (error_value);                                                                       \
          if (error == ERROR_MALLOC)                                                                   \
            {                                                                                          \
              debug_print_malloc_error();                                                              \
            }                                                                                          \
          else                                                                                         \
            {                                                                                          \
              logger((stderr, "Got error \"%d\" (\"%s\")!\n", error_value, error_names[error_value])); \
            }                                                                                          \
          goto goto_label;                                                                             \
        }                                                                                              \
    }                                                                                                  \
  while (0)
#define cleanup_and_set_error_if(condition, error_value) goto_and_set_error_if((condition), (error_value), cleanup)
#define error_cleanup_and_set_error_if(condition, error_value) \
  goto_and_set_error_if((condition), (error_value), error_cleanup)
#define cleanup_and_set_error(error_value) goto_and_set_error_if(1, (error_value), cleanup)
#define error_cleanup_and_set_error(error_value) goto_and_set_error_if(1, (error_value), error_cleanup)

#if (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5))) || defined(__clang__)
#define MAYBE_UNUSED __attribute__((unused))
#define UNUSED \
  __attribute__((unused, deprecated("Marked as \"UNUSED\" but used. Please remove the \"UNUSED\" marker.")))
#else
#define MAYBE_UNUSED
#define UNUSED
#endif

#if defined(__GNUC__) || defined(__clang__)
#define CURRENT_FUNCTION __func__
#elif defined(__FUNCTION__)
#define CURRENT_FUNCTION __FUNCTION__
#else
#define CURRENT_FUNCTION "<unknown>"
#endif


/* ========================= functions ============================================================================== */

/* ------------------------- util ----------------------------------------------------------------------------------- */

void bin_data(unsigned int num_points, double *points, unsigned int num_bins, double *bins, double *weights);
void linspace(double start, double end, unsigned int n, double *x);
size_t djb2_hash(const char *str);
int is_equidistant_array(unsigned int length, const double *x);
int is_int_number(const char *str);
int str_to_uint(const char *str, unsigned int *value_ptr);
int int_equals_any(int number, unsigned int n, ...);
#ifndef __cplusplus
/* C++ code shall use the C++ version in `utilcpp_int.hxx` */
int str_equals_any(const char *str, ...);
#endif
int str_equals_any_in_array(const char *str, const char **str_array);
int uppercase_count(const char *str);
char *str_filter(const char *str, const char *filter_chars);
int is_homogenous_string_of_char(const char *str, char c);
const char *private_name(const char *public_name);
unsigned long next_or_equal_power2(unsigned long num);
int is_env_variable_enabled(const char *env_variable_name);
int file_exists(const char *file_path);
char *get_gr_dir(void);
const char *get_tmp_directory(void);
#ifdef _WIN32
char *convert_wstring_to_utf8(const wchar_t *wstring);
wchar_t *convert_utf8_to_wstring(const char *utf8_bytes);
#endif

#ifdef __cplusplus
}
#endif
#endif /* ifndef GRM_UTIL_INT_H_INCLUDED */
