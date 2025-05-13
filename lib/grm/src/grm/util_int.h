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

#define isStringDelimiter(char_ptr, str) ((*(char_ptr) == '"') && (((char_ptr) == (str)) || *((char_ptr)-1) != '\\'))

#ifndef arraySize
#define arraySize(a) ((sizeof(a) / sizeof(*(a))))
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
#define returnErrorIf(condition, error_value)                                                          \
  do                                                                                                   \
    {                                                                                                  \
      if (condition)                                                                                   \
        {                                                                                              \
          logger((stderr, "Got error \"%d\" (\"%s\")!\n", error_value, grm_error_names[error_value])); \
          return (error_value);                                                                        \
        }                                                                                              \
    }                                                                                                  \
  while (0)
#define returnIfError returnErrorIf((error) != GRM_ERROR_NONE, (error))
#define gotoIf(condition, goto_label) \
  do                                  \
    {                                 \
      if (condition)                  \
        {                             \
          goto goto_label;            \
        }                             \
    }                                 \
  while (0)
#define cleanupIf(condition) gotoIf((condition), cleanup)
#define errorCleanupIf(condition) gotoIf((condition), error_cleanup)
#define gotoIfError(goto_label)                                                                \
  do                                                                                           \
    {                                                                                          \
      if ((error) != GRM_ERROR_NONE)                                                           \
        {                                                                                      \
          logger((stderr, "Got error \"%d\" (\"%s\")!\n", (error), grm_error_names[(error)])); \
          goto goto_label;                                                                     \
        }                                                                                      \
    }                                                                                          \
  while (0)
#define cleanupIfError gotoIfError(cleanup)
#define errorCleanupIfError gotoIfError(error_cleanup)
#define gotoAndSetErrorIf(condition, error_value, goto_label)                                              \
  do                                                                                                       \
    {                                                                                                      \
      if (condition)                                                                                       \
        {                                                                                                  \
          error = (error_value);                                                                           \
          if (error == GRM_ERROR_MALLOC)                                                                   \
            {                                                                                              \
              debugPrintMallocError();                                                                     \
            }                                                                                              \
          else                                                                                             \
            {                                                                                              \
              logger((stderr, "Got error \"%d\" (\"%s\")!\n", error_value, grm_error_names[error_value])); \
            }                                                                                              \
          goto goto_label;                                                                                 \
        }                                                                                                  \
    }                                                                                                      \
  while (0)
#define cleanupAndSetErrorIf(condition, error_value) gotoAndSetErrorIf((condition), (error_value), cleanup)
#define errorCleanupAndSetErrorIf(condition, error_value) gotoAndSetErrorIf((condition), (error_value), error_cleanup)
#define cleanupAndSetError(error_value) gotoAndSetErrorIf(1, (error_value), cleanup)
#define errorCleanupAndSetError(error_value) gotoAndSetErrorIf(1, (error_value), error_cleanup)

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

void binData(unsigned int num_points, double *points, unsigned int num_bins, double *bins, double *weights,
             double ymin);
const char *createTmpDir(void);
int deleteTmpDir(void);
void linSpace(double start, double end, unsigned int n, double *x);
size_t djb2Hash(const char *str);
int isEquidistantArray(unsigned int length, const double *x);
int isIntNumber(const char *str);
int strToUint(const char *str, unsigned int *value_ptr);
int intEqualsAny(int number, unsigned int n, ...);
#ifndef __cplusplus
/* C++ code shall use the C++ version in `utilcpp_int.hxx` */
int strEqualsAny(const char *str, ...);
#endif
int strEqualsAnyInArray(const char *str, const char **str_array);
int upperCaseCount(const char *str);
char *strFilter(const char *str, const char *filter_chars);
int isHomogenousStringOfChar(const char *str, char c);
const char *privateName(const char *public_name);
unsigned long nextOrEqualPower2(unsigned long num);
int isEnvVariableEnabled(const char *env_variable_name);
int fileExists(const char *file_path);
char *getEnvVariable(const char *name);
char *getGrDir(void);
char *getTmpDirectory(void);
#ifdef _WIN32
char *convertWstringToUtf8(const wchar_t *wstring);
wchar_t *convertUtf8ToWstring(const char *utf8_bytes);
#endif

#ifdef __cplusplus
}
#endif
#endif /* ifndef GRM_UTIL_INT_H_INCLUDED */
