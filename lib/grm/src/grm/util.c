#ifdef __unix__
#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 500
#endif

/* ######################### includes ############################################################################### */

#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#if defined __unix__ || defined __APPLE__
#include <ftw.h>
#include <sys/stat.h>
#elif defined _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "gkscore.h"

#include "grm/error.h"
#include "json_int.h"
#include "util_int.h"


/* ######################### internal implementation ################################################################ */

#ifdef isnan
#define is_nan(a) isnan(a)
#else
#define is_nan(x) ((x) != (x))
#endif

/* ========================= macros ================================================================================= */

#define PRIVATE_NAME_BUFFER_LEN 80


/* ========================= static variables ======================================================================= */

static const char *tmp_dir_ = NULL;


/* ========================= functions ============================================================================== */

/* ------------------------- util ----------------------------------------------------------------------------------- */

void bin_data(unsigned int n, double *x, unsigned int num_bins, double *bins, double *weights)
{
  double x_min = DBL_MAX, x_max = -DBL_MAX;
  unsigned int i;

  for (i = 0; i < n; ++i)
    {
      if (is_nan(x[i])) continue;
      x_min = grm_min(x[i], x_min);
      x_max = grm_max(x[i], x_max);
    }
  memset(bins, 0, num_bins * sizeof(double));
  for (i = 0; i < n; ++i)
    {
      if (is_nan(x[i])) continue;
      unsigned int current_bin = (int)((x[i] - x_min) / (x_max - x_min) * num_bins);
      if (current_bin == num_bins) --current_bin;
      bins[current_bin] += (weights != NULL) ? weights[i] : 1;
    }
}

/*!
 * \brief Create an exclusive temporary directory. Consecutive calls return the same directory, unless deleted with
 *        delete_tmp_dir().
 *
 * \return The path to the directory or `NULL` if an error occurred.
 */
const char *create_tmp_dir(void)
{
  char *system_tmp_dir = NULL, *tmp_dir = NULL;
#ifdef _WIN32
  LPWSTR tmp_dir_wide = NULL;
#endif

  if (tmp_dir_ == NULL)
    {
      const char *dirname_template = "grm.XXXXXX";
      size_t tmp_dir_len;

      system_tmp_dir = get_tmp_directory();
      tmp_dir_len = strlen(system_tmp_dir) + strlen(dirname_template) + 1;
      tmp_dir = malloc(tmp_dir_len + 1);
      error_cleanup_if(tmp_dir == NULL);
      sprintf(tmp_dir, "%s%c%s", system_tmp_dir, PATH_SEPARATOR, dirname_template);
#ifdef _WIN32
      {
        char *tmp_dir_utf8;

        tmp_dir_wide = convert_utf8_to_wstring(tmp_dir);
        error_cleanup_if(_wmktemp_s(tmp_dir_wide, tmp_dir_len + 1) != ERROR_SUCCESS ||
                         !CreateDirectoryW(tmp_dir_wide, NULL));
        tmp_dir_utf8 = convert_wstring_to_utf8(tmp_dir_wide);
        error_cleanup_if(tmp_dir_utf8 == NULL);
        free(tmp_dir);
        tmp_dir = tmp_dir_utf8;
      }
#else
      error_cleanup_if(mkdtemp(tmp_dir) == NULL);
#endif

      tmp_dir_ = tmp_dir;
    }

  goto cleanup;

error_cleanup:
  free(tmp_dir);
  tmp_dir = NULL;

cleanup:
  free(system_tmp_dir);
#ifdef _WIN32
  free(tmp_dir_wide);
#endif

  return tmp_dir_;
}

#ifdef _WIN32
BOOL remove_directory_recursively(LPCWSTR dir_path)
{
  WIN32_FIND_DATAW find_file_data;
  wchar_t search_pattern[MAX_PATH];
  HANDLE find_handle = INVALID_HANDLE_VALUE;
  BOOL successful = FALSE;

  cleanup_if(_snwprintf_s(search_pattern, MAX_PATH, MAX_PATH, L"%s\\*", dir_path) < 0);
  find_handle = FindFirstFileW(search_pattern, &find_file_data);
  cleanup_if(find_handle == INVALID_HANDLE_VALUE);

  do
    {
      wchar_t current_path[MAX_PATH];
      if (wcscmp(find_file_data.cFileName, L".") == 0 || wcscmp(find_file_data.cFileName, L"..") == 0)
        {
          continue;
        }
      cleanup_if(_snwprintf_s(current_path, MAX_PATH, MAX_PATH, L"%s\\%s", dir_path, find_file_data.cFileName) < 0);
      if (find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
          cleanup_if(!remove_directory_recursively(current_path));
        }
      else
        {
          cleanup_if(!DeleteFileW(current_path));
        }
    }
  while (FindNextFileW(find_handle, &find_file_data));

  cleanup_if(!RemoveDirectoryW(dir_path));

  successful = TRUE;

cleanup:
  if (find_handle != INVALID_HANDLE_VALUE)
    {
      FindClose(find_handle);
    }

  return successful;
}
#else
static int remove_callback(const char *fpath, const struct stat *sb UNUSED, int typeflag UNUSED,
                           struct FTW *ftwbuf UNUSED)
{
  int rv = remove(fpath);
  if (rv) perror(fpath);
  return rv;
}
#endif

/*!
 * \brief Delete the temporary directory and all its contents created by create_tmp_dir().
 */
/*!
 * \brief Delete the temporary directory and all its contents created by create_tmp_dir().
 */
int delete_tmp_dir(void)
{
  int successful = 0;
  if (tmp_dir_ == NULL) return 0;
#ifdef _WIN32
  {
    LPWSTR tmp_dir_wide = convert_utf8_to_wstring(tmp_dir_);
    if (tmp_dir_wide == NULL) return 0;
    successful = remove_directory_recursively(tmp_dir_wide);
    free(tmp_dir_wide);
  }
#else
  successful = (nftw(tmp_dir_, remove_callback, 64, FTW_DEPTH | FTW_PHYS) == 0);
#endif
  if (successful)
    {
      free((void *)tmp_dir_);
      tmp_dir_ = NULL;
    }

  return successful;
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

int str_equals_any(const char *str, ...)
{
  va_list vl;
  char *current_str;
  int any_is_equal;
  unsigned int i;

  va_start(vl, str);
  any_is_equal = 0;
  while (1)
    {
      current_str = va_arg(vl, char *);
      if (current_str == NULL) break;
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

int is_env_variable_enabled(const char *env_variable_name)
{
  return getenv(env_variable_name) != NULL &&
         str_equals_any(getenv(env_variable_name), "1", "on", "ON", "true", "TRUE", "yes", "YES", NULL);
}

int file_exists(const char *file_path)
{
#ifdef _WIN32
  LPWSTR file_path_wide = convert_utf8_to_wstring(file_path);
  if (file_path_wide == NULL)
    {
      return 0;
    }
  DWORD fileAttributes = GetFileAttributesW(file_path_wide);
  free(file_path_wide);
  return (fileAttributes != INVALID_FILE_ATTRIBUTES && !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY));
#else
  struct stat file_stat;
  return stat(file_path, &file_stat) == 0 && S_ISREG(file_stat.st_mode);
#endif
}

char *get_env_variable(const char *name)
{
#ifdef _WIN32
  DWORD char_count;
  LPWSTR name_wide = NULL, value_wide = NULL;
  LPSTR value_utf8 = NULL;

  name_wide = convert_utf8_to_wstring(name);
  error_cleanup_if(name_wide == NULL);
  char_count = GetEnvironmentVariableW(name_wide, NULL, 0) + 1;
  error_cleanup_if(char_count == 0);
  value_wide = malloc(sizeof(wchar_t) * char_count);
  error_cleanup_if(value_wide == NULL);
  char_count = GetEnvironmentVariableW(name_wide, value_wide, char_count);
  error_cleanup_if(char_count == 0);

  value_utf8 = convert_wstring_to_utf8(value_wide);
  error_cleanup_if(value_utf8 == NULL);

  goto cleanup;

error_cleanup:
  free(value_utf8);
  value_utf8 = NULL;

cleanup:
  free(name_wide);
  free(value_wide);

  return value_utf8;

#else
  const char *value;
  if ((value = getenv(name)) != NULL)
    {
      return strdup(value);
    }

  return NULL;
#endif
}

char *get_gr_dir(void)
{
  char *env_variable_value;
  if ((env_variable_value = get_env_variable("GRDIR")) != NULL)
    {
      return env_variable_value;
    }
  else
    {
      return strdup(GRDIR);
    }
}

char *get_tmp_directory(void)
{
  char *tmp_dir;
  const char *env_vars[] = {
      "TMPDIR",
      "TMP",
      "TEMP",
      "TEMPDIR",
  };
  unsigned int i;

  for (i = 0; i < array_size(env_vars); ++i)
    {
      if ((tmp_dir = get_env_variable(env_vars[i])) != NULL)
        {
          break;
        }
    }
  if (tmp_dir == NULL)
    {
      tmp_dir = strdup("/tmp");
    }

  return tmp_dir;
}

#ifdef _WIN32
char *convert_wstring_to_utf8(const wchar_t *wstring)
{
  int utf8_byte_count;
  char *utf8_bytes;

  if ((utf8_byte_count = WideCharToMultiByte(CP_UTF8, 0, wstring, -1, NULL, 0, NULL, NULL)) == 0)
    {
      return NULL;
    }
  if ((utf8_bytes = malloc(utf8_byte_count)) == NULL)
    {
      return NULL;
    }

  if (WideCharToMultiByte(CP_UTF8, 0, wstring, -1, utf8_bytes, utf8_byte_count, NULL, NULL) == 0)
    {
      free(utf8_bytes);
      return NULL;
    }

  return utf8_bytes;
}

wchar_t *convert_utf8_to_wstring(const char *utf8_bytes)
{
  int wide_char_count;
  wchar_t *wide_chars;

  if ((wide_char_count = MultiByteToWideChar(CP_UTF8, 0, utf8_bytes, -1, NULL, 0)) == 0)
    {
      return NULL;
    }
  if ((wide_chars = malloc(sizeof(wchar_t) * wide_char_count)) == NULL)
    {
      return NULL;
    }
  if (MultiByteToWideChar(CP_UTF8, 0, utf8_bytes, -1, wide_chars, wide_char_count) == 0)
    {
      free(wide_chars);
      return NULL;
    }

  return wide_chars;
}
#endif


/* ######################### public implementation ################################################################## */

/* ========================= functions ============================================================================== */

/* ------------------------- util ----------------------------------------------------------------------------------- */

#if !defined(NDEBUG) && defined(EMSCRIPTEN)
FILE *grm_get_stdout(void)
{
  return stdout;
}
#endif
