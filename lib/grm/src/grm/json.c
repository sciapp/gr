#ifdef __unix__
#define _POSIX_C_SOURCE 200112L
#endif

/* ######################### includes ############################################################################### */

#include <ctype.h>
#include <locale.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>

#include "grm/error.h"
#include "json_int.h"
#include "plot_int.h"


/* ######################### internal implementation ################################################################ */

/* ========================= macros ================================================================================= */

/* ------------------------- general -------------------------------------------------------------------------------- */

#ifndef DBL_DECIMAL_DIG
#define DBL_DECIMAL_DIG 17
#endif

/* ========================= static variables ======================================================================= */

/* ------------------------- json deserializer ---------------------------------------------------------------------- */

const char *FROMJSON_VALID_DELIMITERS = ",]}";
const char FROMJSON_STRING_DELIMITER = '"';
const char FROMJSON_ESCAPE_CHARACTER = '\\';

static err_t (*fromjson_datatype_to_func[])(fromjson_state_t *) = {NULL,
                                                                   fromjson_parse_null,
                                                                   fromjson_parse_bool,
                                                                   fromjson_parse_number,
                                                                   fromjson_parse_string,
                                                                   fromjson_parse_array,
                                                                   fromjson_parse_object};

static const char *const fromjson_datatype_to_string[] = {"unknown", "null",  "bool",  "number",
                                                          "string",  "array", "object"};


/* ------------------------- json serializer ------------------------------------------------------------------------ */

static err_t (*tojson_datatype_to_func[128])(tojson_state_t *);
static int tojson_static_variables_initialized = 0;
static tojson_permanent_state_t tojson_permanent_state = {complete, 0};


/* ========================= methods ================================================================================ */

/* ------------------------- json deserializer ---------------------------------------------------------------------- */

int grm_read(grm_args_t *args, const char *json_string)
{
  return (fromjson_read(args, json_string) == ERROR_NONE);
}

err_t fromjson_read(grm_args_t *args, const char *json_string)
{
  return fromjson_parse(args, json_string, NULL);
}

int grm_load_from_str(const char *json_string)
{
  return (fromjson_read(active_plot_args, json_string) == ERROR_NONE);
}

err_t fromjson_parse(grm_args_t *args, const char *json_string, fromjson_shared_state_t *shared_state)
{
  char *filtered_json_string = NULL;
  fromjson_state_t state;
  int allocated_shared_state_mem = 0;
  err_t error = ERROR_NONE;
  char *saved_locale;

  state.datatype = JSON_DATATYPE_UNKNOWN;
  state.value_buffer = NULL;
  state.value_buffer_pointer_level = 0;
  state.next_value_memory = NULL;
  state.next_value_type = malloc(NEXT_VALUE_TYPE_SIZE);
  if (state.next_value_type == NULL)
    {
      debug_print_malloc_error();
      return ERROR_MALLOC;
    }
  state.args = args;
  if (shared_state == NULL)
    {
      shared_state = malloc(sizeof(fromjson_shared_state_t));
      if (shared_state == NULL)
        {
          free(state.next_value_type);
          debug_print_malloc_error();
          return ERROR_MALLOC;
        }
      if ((error = fromjson_copy_and_filter_json_string(&filtered_json_string, json_string)) != ERROR_NONE)
        {
          free(state.next_value_type);
          free(shared_state);
          return error;
        }
      shared_state->json_ptr = filtered_json_string;
      shared_state->parsed_any_value_before = 0;
      allocated_shared_state_mem = 1;
    }
  state.shared_state = shared_state;
  state.parsing_object = (*state.shared_state->json_ptr == '{');
  if (state.parsing_object)
    {
      ++state.shared_state->json_ptr;
    }

  saved_locale = setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "C");

  while (strchr("}", *state.shared_state->json_ptr) == NULL)
    {
      const char *current_key = NULL;

      if (state.parsing_object)
        {
          fromjson_parse_string(&state);
          current_key = *(const char **)state.value_buffer;
          free(state.value_buffer);
          state.value_buffer = NULL;
          ++(state.shared_state->json_ptr);
        }
      state.datatype = fromjson_check_type(&state);
      if (state.datatype)
        {
          if ((error = fromjson_datatype_to_func[state.datatype](&state)) != ERROR_NONE)
            {
              break;
            }
          if (state.parsing_object)
            {
              grm_args_push_buf(args, current_key, state.next_value_type, state.value_buffer, 0);
            }
          else
            {
              /* parsing values without an outer object (-> missing key) is not supported by the argument container */
              error = ERROR_PARSE_MISSING_OBJECT_CONTAINER;
              break;
            }
          if (strchr(FROMJSON_VALID_DELIMITERS, *state.shared_state->json_ptr) != NULL)
            {
              if (*state.shared_state->json_ptr == ',')
                {
                  ++state.shared_state->json_ptr;
                }
            }
          else
            {
              error = ERROR_PARSE_INVALID_DELIMITER;
              break;
            }
        }
      else
        {
          error = ERROR_PARSE_UNKNOWN_DATATYPE;
          break;
        }
      if (state.value_buffer_pointer_level > 1)
        {
          int i, outer_array_length = uppercase_count(state.next_value_type);
          for (i = 0; i < outer_array_length; ++i)
            {
              free(((char **)state.value_buffer)[i]);
            }
        }
      free(state.value_buffer);
      state.value_buffer = NULL;
      state.value_buffer_pointer_level = 0;
    }
  if (state.parsing_object && *state.shared_state->json_ptr == '\0')
    {
      error = ERROR_PARSE_INCOMPLETE_STRING;
    }
  if (*state.shared_state->json_ptr)
    {
      ++state.shared_state->json_ptr;
    }

  free(state.value_buffer);
  free(filtered_json_string);
  free(state.next_value_type);

  if (allocated_shared_state_mem)
    {
      free(shared_state);
    }

  if (saved_locale)
    {
      setlocale(LC_NUMERIC, saved_locale);
    }

  return error;
}

#define CHECK_AND_ALLOCATE_MEMORY(type, length)                \
  do                                                           \
    {                                                          \
      if (state->value_buffer == NULL)                         \
        {                                                      \
          state->value_buffer = malloc(length * sizeof(type)); \
          if (state->value_buffer == NULL)                     \
            {                                                  \
              debug_print_malloc_error();                      \
              return 0;                                        \
            }                                                  \
          state->value_buffer_pointer_level = 1;               \
          state->next_value_memory = state->value_buffer;      \
        }                                                      \
    }                                                          \
  while (0)

err_t fromjson_parse_null(fromjson_state_t *state)
{
  if (strncmp(state->shared_state->json_ptr, "null", 4) != 0)
    {
      return ERROR_PARSE_NULL;
    }
  strcpy(state->next_value_type, "");
  state->shared_state->json_ptr += 4;
  return ERROR_NONE;
}

err_t fromjson_parse_bool(fromjson_state_t *state)
{
  int bool_value;

  if (strncmp(state->shared_state->json_ptr, "true", 4) == 0)
    {
      bool_value = 1;
    }
  else if (strncmp(state->shared_state->json_ptr, "false", 5) == 0)
    {
      bool_value = 0;
    }
  else
    {
      return ERROR_PARSE_BOOL;
    }
  CHECK_AND_ALLOCATE_MEMORY(int, 1);
  *((int *)state->next_value_memory) = bool_value;
  strcpy(state->next_value_type, "i");
  state->shared_state->json_ptr += bool_value ? 4 : 5;
  return ERROR_NONE;
}

err_t fromjson_parse_number(fromjson_state_t *state)
{
  err_t error;

  if (is_int_number(state->shared_state->json_ptr))
    {
      error = fromjson_parse_int(state);
    }
  else
    {
      error = fromjson_parse_double(state);
    }
  return error;
}

err_t fromjson_parse_int(fromjson_state_t *state)
{
  int was_successful;
  int int_value;

  int_value = fromjson_str_to_int((const char **)&state->shared_state->json_ptr, &was_successful);
  if (!was_successful)
    {
      return ERROR_PARSE_INT;
    }
  CHECK_AND_ALLOCATE_MEMORY(int, 1);
  *((int *)state->next_value_memory) = int_value;
  strcpy(state->next_value_type, "i");
  return ERROR_NONE;
}

err_t fromjson_parse_double(fromjson_state_t *state)
{
  int was_successful;
  double double_value;

  double_value = fromjson_str_to_double((const char **)&state->shared_state->json_ptr, &was_successful);
  if (!was_successful)
    {
      return ERROR_PARSE_DOUBLE;
    }
  CHECK_AND_ALLOCATE_MEMORY(double, 1);
  *((double *)state->next_value_memory) = double_value;
  strcpy(state->next_value_type, "d");
  return ERROR_NONE;
}

err_t fromjson_parse_string(fromjson_state_t *state)
{
  char *string_value;
  char *json_ptr;
  const char *src_ptr;
  char *dest_ptr;
  int string_is_complete;
  int skipped_char;

  CHECK_AND_ALLOCATE_MEMORY(char *, 1);
  json_ptr = state->shared_state->json_ptr;
  string_value = ++json_ptr;
  while (*json_ptr && !is_string_delimiter(json_ptr, string_value))
    {
      ++json_ptr;
    }
  string_is_complete = (*json_ptr != '\0');
  *json_ptr = '\0';
  /* Unescape '"' and '\' (since '\' is the escape character) */
  src_ptr = dest_ptr = string_value;
  skipped_char = 0;
  while (*src_ptr)
    {
      if (*src_ptr == '\\' && !skipped_char)
        {
          ++src_ptr;
          skipped_char = 1;
        }
      else
        {
          *dest_ptr = *src_ptr;
          ++src_ptr;
          ++dest_ptr;
          skipped_char = 0;
        }
    }
  *dest_ptr = '\0';

  *((const char **)state->next_value_memory) = string_value;
  strcpy(state->next_value_type, "s");
  ++json_ptr;
  state->shared_state->json_ptr = json_ptr;

  return string_is_complete ? ERROR_NONE : ERROR_PARSE_STRING;
}

err_t fromjson_parse_array(fromjson_state_t *state)
{
  fromjson_datatype_t json_datatype;
  const char *next_delim_ptr;

#define PARSE_VALUES(parse_suffix, c_type)                                                                         \
  do                                                                                                               \
    {                                                                                                              \
      c_type *values;                                                                                              \
      c_type *current_value_ptr;                                                                                   \
      CHECK_AND_ALLOCATE_MEMORY(c_type *, 1);                                                                      \
      values = malloc(array_length * sizeof(c_type));                                                              \
      if (values == NULL)                                                                                          \
        {                                                                                                          \
          debug_print_malloc_error();                                                                              \
          return ERROR_MALLOC;                                                                                     \
        }                                                                                                          \
      current_value_ptr = values;                                                                                  \
      *(c_type **)state->next_value_memory = values;                                                               \
      state->value_buffer_pointer_level = 2;                                                                       \
      state->next_value_memory = values;                                                                           \
      --state->shared_state->json_ptr;                                                                             \
      while (!error && strchr("]", *state->shared_state->json_ptr) == NULL)                                        \
        {                                                                                                          \
          ++state->shared_state->json_ptr;                                                                         \
          error = fromjson_parse_##parse_suffix(state);                                                            \
          ++current_value_ptr;                                                                                     \
          state->next_value_memory = current_value_ptr;                                                            \
        }                                                                                                          \
      snprintf(array_type + strlen(array_type), NEXT_VALUE_TYPE_SIZE, "%c(%zu)", toupper(*state->next_value_type), \
               array_length);                                                                                      \
    }                                                                                                              \
  while (0)

  if (strchr("]", *(state->shared_state->json_ptr + 1)) == NULL)
    {
      char array_type[NEXT_VALUE_TYPE_SIZE];
      size_t outer_array_length = 0;
      int is_nested_array = (*(state->shared_state->json_ptr + 1) == '[');
      unsigned int current_outer_array_index = 0;
      if (is_nested_array)
        {
          outer_array_length = fromjson_get_outer_array_length(state->shared_state->json_ptr);
          /* `char *` is only used as a generic type since all pointers to values have the same storage size */
          CHECK_AND_ALLOCATE_MEMORY(char *, outer_array_length);
        }
      array_type[0] = '\0';
      do
        {
          err_t error = ERROR_NONE;
          size_t array_length = 0;
          state->shared_state->json_ptr += is_nested_array ? 2 : 1;
          next_delim_ptr = state->shared_state->json_ptr;
          while (*next_delim_ptr != ']' &&
                 fromjson_find_next_delimiter(&next_delim_ptr, next_delim_ptr, array_length == 0, 1))
            {
              ++array_length;
            }
          if (*next_delim_ptr != ']')
            {
              state->shared_state->json_ptr -= is_nested_array ? 2 : 1;
              return ERROR_PARSE_INCOMPLETE_STRING;
            }
          assert(array_length > 0);
          json_datatype = fromjson_check_type(state);
          switch (json_datatype)
            {
            case JSON_DATATYPE_NUMBER:
              if (is_int_number(state->shared_state->json_ptr))
                {
                  PARSE_VALUES(int, int);
                }
              else
                {
                  PARSE_VALUES(double, double);
                }
              break;
            case JSON_DATATYPE_STRING:
              PARSE_VALUES(string, char *);
              break;
            case JSON_DATATYPE_OBJECT:
              PARSE_VALUES(object, grm_args_t *);
              break;
            case JSON_DATATYPE_ARRAY:
              debug_print_error(("Arrays only support one level of nesting!\n"));
              state->shared_state->json_ptr -= is_nested_array ? 2 : 1;
              return ERROR_PARSE_ARRAY;
              break;
            default:
              debug_print_error(("The datatype \"%s\" is currently not supported in arrays!\n",
                                 fromjson_datatype_to_string[json_datatype]));
              state->shared_state->json_ptr -= is_nested_array ? 2 : 1;
              return ERROR_PARSE_ARRAY;
              break;
            }
          ++(state->shared_state->json_ptr);
          if (is_nested_array)
            {
              ++current_outer_array_index;
              /* `char **` is only used as a generic type since all pointers to values have the same storage size */
              state->next_value_memory = ((char **)state->value_buffer) + current_outer_array_index;
            }
        }
      while (is_nested_array && current_outer_array_index < outer_array_length &&
             strchr("]", *state->shared_state->json_ptr) == NULL);
      if (is_nested_array && *state->shared_state->json_ptr == ']')
        {
          ++state->shared_state->json_ptr;
        }
      strcpy(state->next_value_type, array_type);
    }
  else
    {
      strcpy(state->next_value_type, "I(0)");
    }

  return ERROR_NONE;

#undef PARSE_VALUES
}

err_t fromjson_parse_object(fromjson_state_t *state)
{
  grm_args_t *args;
  err_t error;

  CHECK_AND_ALLOCATE_MEMORY(grm_args_t *, 1);
  args = grm_args_new();
  error = fromjson_parse(args, state->shared_state->json_ptr, state->shared_state);
  *((grm_args_t **)state->next_value_memory) = args;
  strcpy(state->next_value_type, "a");
  return error;
}

#undef CHECK_AND_ALLOCATE_MEMORY

fromjson_datatype_t fromjson_check_type(const fromjson_state_t *state)
{
  fromjson_datatype_t datatype;

  datatype = JSON_DATATYPE_UNKNOWN;
  switch (*state->shared_state->json_ptr)
    {
    case '"':
      datatype = JSON_DATATYPE_STRING;
      break;
    case '[':
      datatype = JSON_DATATYPE_ARRAY;
      break;
    case '{':
      datatype = JSON_DATATYPE_OBJECT;
      break;
    default:
      break;
    }
  if (datatype == JSON_DATATYPE_UNKNOWN)
    {
      if (*state->shared_state->json_ptr == 'n')
        {
          datatype = JSON_DATATYPE_NULL;
        }
      else if (strchr("ft", *state->shared_state->json_ptr) != NULL)
        {
          datatype = JSON_DATATYPE_BOOL;
        }
      else
        {
          datatype = JSON_DATATYPE_NUMBER;
        }
    }
  return datatype;
}

err_t fromjson_copy_and_filter_json_string(char **dest, const char *src)
{
  const char *src_ptr;
  char *dest_buffer, *dest_ptr;
  int in_string;

  src_ptr = src;
  dest_buffer = malloc(strlen(src) + 1);
  if (dest_buffer == NULL)
    {
      debug_print_malloc_error();
      return ERROR_MALLOC;
    }
  dest_ptr = dest_buffer;

  in_string = 0;
  while (*src_ptr)
    {
      if (is_string_delimiter(src_ptr, src))
        {
          in_string = !in_string;
        }
      if (in_string || !isspace(*src_ptr))
        {
          *dest_ptr = *src_ptr;
          ++dest_ptr;
        }
      ++src_ptr;
    }
  *dest_ptr = '\0';

  *dest = dest_buffer;

  return ERROR_NONE;
}

int fromjson_is_escaped_delimiter(const char *delim_ptr, const char *str)
{
  const char *first_non_escape_char_ptr;

  first_non_escape_char_ptr = delim_ptr - 1;
  while (first_non_escape_char_ptr != str - 1 && *first_non_escape_char_ptr == FROMJSON_ESCAPE_CHARACTER)
    {
      --first_non_escape_char_ptr;
    }
  return (delim_ptr - first_non_escape_char_ptr) % 2 == 0;
}

int fromjson_find_next_delimiter(const char **delim_ptr, const char *src, int include_start,
                                 int exclude_nested_structures)
{
  int is_in_string = 0;

  if (*src == '\0')
    {
      return 0;
    }
  if (!include_start)
    {
      ++src;
    }
  if (exclude_nested_structures)
    {
      const char *src_ptr = src;
      int nested_level = 0;

      while (*src_ptr != '\0')
        {
          if (*src_ptr == FROMJSON_STRING_DELIMITER && !fromjson_is_escaped_delimiter(src_ptr, src))
            {
              is_in_string = !is_in_string;
            }
          if (!is_in_string)
            {
              if (strchr("[{", *src_ptr))
                {
                  ++nested_level;
                }
              else if (strchr("]}", *src_ptr))
                {
                  if (nested_level > 0)
                    {
                      --nested_level;
                    }
                  else
                    {
                      break;
                    }
                }
              else if (*src_ptr == ',' && nested_level == 0)
                {
                  break;
                }
            }
          ++src_ptr;
        }
      if (*src_ptr != '\0')
        {
          *delim_ptr = src_ptr;
          return 1;
        }
    }
  else
    {
      const char *src_ptr = src;
      while (*src_ptr != '\0' && (is_in_string || strchr(FROMJSON_VALID_DELIMITERS, *src_ptr) == NULL))
        {
          if (*src_ptr == FROMJSON_STRING_DELIMITER && !fromjson_is_escaped_delimiter(src_ptr, src))
            {
              is_in_string = !is_in_string;
            }
          ++src_ptr;
        }
      if (*src_ptr != '\0')
        {
          *delim_ptr = src_ptr;
          return 1;
        }
    }
  return 0;
}

size_t fromjson_get_outer_array_length(const char *str)
{
  size_t outer_array_length = 0;
  int current_array_level = 1;

  if (*str != '[')
    {
      return outer_array_length;
    }
  ++str;
  while (current_array_level > 0 && *str)
    {
      switch (*str)
        {
        case '[':
          ++current_array_level;
          break;
        case ']':
          --current_array_level;
          break;
        case ',':
          if (current_array_level == 1)
            {
              ++outer_array_length;
            }
          break;
        default:
          break;
        }
      ++str;
    }
  ++outer_array_length;
  return outer_array_length;
}

double fromjson_str_to_double(const char **str, int *was_successful)
{
  char *conversion_end = NULL;
  double conversion_result;
  int success = 0;
  const char *next_delim_ptr = NULL;

  errno = 0;
  if (*str != NULL)
    {
      conversion_result = strtod(*str, &conversion_end);
    }
  else
    {
      conversion_result = 0.0;
    }
  if (conversion_end == NULL)
    {
      debug_print_error(("No number conversion was executed (the string is NULL)!\n"));
    }
  else if (*str == conversion_end || strchr(FROMJSON_VALID_DELIMITERS, *conversion_end) == NULL)
    {
      fromjson_find_next_delimiter(&next_delim_ptr, *str, 1, 0);
      debug_print_error(("The parameter \"%.*s\" is not a valid number!\n", next_delim_ptr - *str, *str));
    }
  else if (errno == ERANGE)
    {
      fromjson_find_next_delimiter(&next_delim_ptr, *str, 1, 0);
      if (conversion_result == HUGE_VAL || conversion_result == -HUGE_VAL)
        {
          debug_print_error(("The parameter \"%.*s\" caused an overflow, the number has been clamped to \"%lf\"\n",
                             next_delim_ptr - *str, *str, conversion_result));
        }
      else
        {
          debug_print_error(("The parameter \"%.*s\" caused an underflow, the number has been clamped to \"%lf\"\n",
                             next_delim_ptr - *str, *str, conversion_result));
        }
    }
  else
    {
      success = 1;
      *str = conversion_end;
    }
  if (was_successful != NULL)
    {
      *was_successful = success;
    }

  return conversion_result;
}

int fromjson_str_to_int(const char **str, int *was_successful)
{
  char *conversion_end = NULL;
  long conversion_result;
  int success = 0;
  const char *next_delim_ptr = NULL;

  errno = 0;
  if (*str != NULL)
    {
      conversion_result = strtol(*str, &conversion_end, 10);
    }
  else
    {
      conversion_result = 0;
    }
  if (conversion_end == NULL)
    {
      debug_print_error(("No number conversion was executed (the string is NULL)!\n"));
    }
  else if (*str == conversion_end || strchr(FROMJSON_VALID_DELIMITERS, *conversion_end) == NULL)
    {
      fromjson_find_next_delimiter(&next_delim_ptr, *str, 1, 0);
      debug_print_error(("The parameter \"%.*s\" is not a valid number!\n", next_delim_ptr - *str, *str));
    }
  else if (errno == ERANGE || conversion_result > INT_MAX || conversion_result < INT_MIN)
    {
      fromjson_find_next_delimiter(&next_delim_ptr, *str, 1, 0);
      if (conversion_result > INT_MAX)
        {
          debug_print_error(("The parameter \"%.*s\" is too big, the number has been clamped to \"%d\"\n",
                             next_delim_ptr - *str, *str, INT_MAX));
          conversion_result = INT_MAX;
        }
      else
        {
          debug_print_error(("The parameter \"%.*s\" is too small, the number has been clamped to \"%d\"\n",
                             next_delim_ptr - *str, *str, INT_MIN));
          conversion_result = INT_MIN;
        }
    }
  else
    {
      success = 1;
      *str = conversion_end;
    }
  if (was_successful != NULL)
    {
      *was_successful = success;
    }

  return (int)conversion_result;
}


/* ------------------------- json serializer ------------------------------------------------------------------------ */

#define CHECK_PADDING(type)                                                             \
  do                                                                                    \
    {                                                                                   \
      if (state->shared->data_ptr != NULL && state->shared->apply_padding)              \
        {                                                                               \
          ptrdiff_t needed_padding = state->shared->data_offset % sizeof(type);         \
          state->shared->data_ptr = ((char *)state->shared->data_ptr) + needed_padding; \
          state->shared->data_offset += needed_padding;                                 \
        }                                                                               \
    }                                                                                   \
  while (0)

#define RETRIEVE_SINGLE_VALUE(var, type, promoted_type)                    \
  do                                                                       \
    {                                                                      \
      CHECK_PADDING(type);                                                 \
      if (state->shared->data_ptr != NULL)                                 \
        {                                                                  \
          var = *((type *)state->shared->data_ptr);                        \
          state->shared->data_ptr = ((type *)state->shared->data_ptr) + 1; \
          state->shared->data_offset += sizeof(type);                      \
        }                                                                  \
      else                                                                 \
        {                                                                  \
          var = va_arg(*state->shared->vl, promoted_type);                 \
        }                                                                  \
    }                                                                      \
  while (0)

#define INIT_MULTI_VALUE(vars, type)                 \
  do                                                 \
    {                                                \
      if (state->shared->data_ptr != NULL)           \
        {                                            \
          CHECK_PADDING(type *);                     \
          vars = *(type **)state->shared->data_ptr;  \
        }                                            \
      else                                           \
        {                                            \
          vars = va_arg(*state->shared->vl, type *); \
        }                                            \
    }                                                \
  while (0)

#define FIN_MULTI_VALUE(type)                                               \
  do                                                                        \
    {                                                                       \
      if (state->shared->data_ptr != NULL)                                  \
        {                                                                   \
          state->shared->data_ptr = ((type **)state->shared->data_ptr) + 1; \
          state->shared->data_offset += sizeof(type *);                     \
        }                                                                   \
    }                                                                       \
  while (0)

#define DEFINE_STRINGIFY_VALUE(name, type, format_specifier)                \
  err_t tojson_stringify_##name##_value(memwriter_t *memwriter, type value) \
  {                                                                         \
    return memwriter_printf(memwriter, format_specifier, value);            \
  }

#define DEFINE_STRINGIFY_SINGLE(name, type, promoted_type)                                \
  err_t tojson_stringify_##name(tojson_state_t *state)                                    \
  {                                                                                       \
    type value;                                                                           \
    err_t error = ERROR_NONE;                                                             \
    RETRIEVE_SINGLE_VALUE(value, type, promoted_type);                                    \
    if ((error = tojson_stringify_##name##_value(state->memwriter, value)) != ERROR_NONE) \
      {                                                                                   \
        return error;                                                                     \
      }                                                                                   \
    state->shared->wrote_output = 1;                                                      \
    return error;                                                                         \
  }

#define DEFINE_STRINGIFY_MULTI(name, type)                                                                \
  err_t tojson_stringify_##name##_array(tojson_state_t *state)                                            \
  {                                                                                                       \
    type *values;                                                                                         \
    type current_value;                                                                                   \
    unsigned int length;                                                                                  \
    int remaining_elements;                                                                               \
    err_t error = ERROR_NONE;                                                                             \
    INIT_MULTI_VALUE(values, type);                                                                       \
    if (state->additional_type_info != NULL)                                                              \
      {                                                                                                   \
        if (!str_to_uint(state->additional_type_info, &length))                                           \
          {                                                                                               \
            debug_print_error(                                                                            \
                ("The given array length \"%s\" is no valid number; the array contents will be ignored.", \
                 state->additional_type_info));                                                           \
            length = 0;                                                                                   \
          }                                                                                               \
      }                                                                                                   \
    else                                                                                                  \
      {                                                                                                   \
        length = state->shared->array_length;                                                             \
      }                                                                                                   \
    remaining_elements = length;                                                                          \
    /* write array start */                                                                               \
    if ((error = memwriter_putc(state->memwriter, '[')) != ERROR_NONE)                                    \
      {                                                                                                   \
        return error;                                                                                     \
      }                                                                                                   \
    /* write array content */                                                                             \
    while (remaining_elements)                                                                            \
      {                                                                                                   \
        current_value = *values++;                                                                        \
        if ((error = tojson_stringify_##name##_value(state->memwriter, current_value)) != ERROR_NONE)     \
          {                                                                                               \
            return error;                                                                                 \
          }                                                                                               \
        if (remaining_elements > 1)                                                                       \
          {                                                                                               \
            if ((error = memwriter_putc(state->memwriter, ',')) != ERROR_NONE)                            \
              {                                                                                           \
                return error;                                                                             \
              }                                                                                           \
          }                                                                                               \
        --remaining_elements;                                                                             \
      }                                                                                                   \
    /* write array end */                                                                                 \
    if ((error = memwriter_putc(state->memwriter, ']')) != ERROR_NONE)                                    \
      {                                                                                                   \
        return error;                                                                                     \
      }                                                                                                   \
    FIN_MULTI_VALUE(type);                                                                                \
    state->shared->wrote_output = 1;                                                                      \
    return error;                                                                                         \
  }

DEFINE_STRINGIFY_SINGLE(int, int, int)
DEFINE_STRINGIFY_MULTI(int, int)
DEFINE_STRINGIFY_VALUE(int, int, "%d")
DEFINE_STRINGIFY_SINGLE(double, double, double)
DEFINE_STRINGIFY_MULTI(double, double)
DEFINE_STRINGIFY_SINGLE(char, char, int)
DEFINE_STRINGIFY_VALUE(char, char, "%c")
DEFINE_STRINGIFY_SINGLE(string, char *, char *)
DEFINE_STRINGIFY_MULTI(string, char *)
DEFINE_STRINGIFY_SINGLE(bool, int, int)
DEFINE_STRINGIFY_MULTI(bool, int)
DEFINE_STRINGIFY_SINGLE(args, grm_args_t *, grm_args_t *)
DEFINE_STRINGIFY_MULTI(args, grm_args_t *)

#undef DEFINE_STRINGIFY_SINGLE
#undef DEFINE_STRINGIFY_MULTI
#undef DEFINE_STRINGIFY_VALUE

#define STR(x) #x
#define XSTR(x) STR(x)

err_t tojson_stringify_double_value(memwriter_t *memwriter, double value)
{
  err_t error;
  size_t string_start_index;
  const char *unprocessed_string;

  string_start_index = memwriter_size(memwriter);
  /*
     The %G format specifier is important here, as it returns "NAN" for missing values -
     %g would return "nan" and be handled as JSON_DATATYPE_NULL in fromjson_check_type()
   */
  if ((error = memwriter_printf(memwriter, "%." XSTR(DBL_DECIMAL_DIG) "G", value)) != ERROR_NONE)
    {
      return error;
    }
  unprocessed_string = memwriter_buf(memwriter) + string_start_index;
  if (strspn(unprocessed_string, "0123456789-") == memwriter_size(memwriter) - string_start_index)
    {
      if ((error = memwriter_putc(memwriter, '.')) != ERROR_NONE)
        {
          return error;
        }
    }
  return ERROR_NONE;
}

#undef XSTR
#undef STR

err_t tojson_stringify_char_array(tojson_state_t *state)
{
  char *chars;
  char *escaped_chars = NULL;
  unsigned int length;
  err_t error = ERROR_NONE;

  INIT_MULTI_VALUE(chars, char);

  if (state->additional_type_info != NULL)
    {
      if (!str_to_uint(state->additional_type_info, &length))
        {
          debug_print_error(("The given array length \"%s\" is no valid number; the array contents will be ignored.",
                             state->additional_type_info));
          goto cleanup;
        }
    }
  else
    {
      if (state->shared->read_length_from_string)
        {
          length = 0;
        }
      else
        {
          length = state->shared->array_length;
        }
    }
  if ((error = tojson_escape_special_chars(&escaped_chars, chars, &length)) != ERROR_NONE)
    {
      goto cleanup;
    }
  if ((error = memwriter_printf(state->memwriter, "\"%.*s\"", length, escaped_chars)) != ERROR_NONE)
    {
      goto cleanup;
    }
  state->shared->wrote_output = 1;

  FIN_MULTI_VALUE(char);

cleanup:
  free(escaped_chars);
  return error;
}

err_t tojson_stringify_string_value(memwriter_t *memwriter, char *value)
{
  char *escaped_chars = NULL;
  unsigned int length = 0;
  err_t error = ERROR_NONE;

  if ((error = tojson_escape_special_chars(&escaped_chars, value, &length)))
    {
      goto cleanup;
    }
  if ((error = memwriter_printf(memwriter, "\"%s\"", escaped_chars)) != ERROR_NONE)
    {
      goto cleanup;
    }

cleanup:
  free(escaped_chars);
  return error;
}

err_t tojson_stringify_bool_value(memwriter_t *memwriter, int value)
{
  return memwriter_puts(memwriter, value ? "true" : "false");
}

err_t tojson_stringify_object(tojson_state_t *state)
{
  char **member_names = NULL;
  char **data_types = NULL;
  char **member_name_ptr;
  char **data_type_ptr;
  int has_members;
  err_t error = ERROR_NONE;

  /* IMPORTANT: additional_type_info is altered after the unzip call! */
  if ((error = tojson_unzip_membernames_and_datatypes(state->additional_type_info, &member_names, &data_types)) !=
      ERROR_NONE)
    {
      goto cleanup;
    }
  member_name_ptr = member_names;
  data_type_ptr = data_types;

  has_members =
      (member_name_ptr != NULL && *member_name_ptr != NULL && data_type_ptr != NULL && *data_type_ptr != NULL);
  /* write object start */
  if (!state->add_data_without_separator)
    {
      if (state->shared->add_data && has_members)
        {
          if ((error = memwriter_putc(state->memwriter, ',')) != ERROR_NONE)
            {
              goto cleanup;
            }
        }
      else if (!state->shared->add_data)
        {
          if ((error = memwriter_putc(state->memwriter, '{')) != ERROR_NONE)
            {
              goto cleanup;
            }
          ++(state->shared->struct_nested_level);
        }
    }
  /* `add_data` is only relevant for the first object start; reset it to default afterwards since nested objects can
   * follow*/
  state->shared->add_data = 0;
  if (has_members)
    {
      /* write object content */
      int serialized_all_members = 0;
      while (!serialized_all_members)
        {
          if ((error = memwriter_printf(state->memwriter, "\"%s\":", *member_name_ptr)) != ERROR_NONE)
            {
              goto cleanup;
            }
          if ((error = tojson_serialize(state->memwriter, *data_type_ptr, NULL, NULL, -1, -1, 0, NULL, NULL,
                                        state->shared)) != ERROR_NONE)
            {
              goto cleanup;
            }
          ++member_name_ptr;
          ++data_type_ptr;
          if (*member_name_ptr != NULL && *data_type_ptr != NULL)
            {
              /* write JSON separator */
              if ((error = memwriter_putc(state->memwriter, ',')) != ERROR_NONE)
                {
                  goto cleanup;
                }
            }
          else
            {
              serialized_all_members = 1;
            }
        }
    }
  /* write object end if the type info is complete */
  if (!state->is_type_info_incomplete)
    {
      --(state->shared->struct_nested_level);
      if ((error = memwriter_putc(state->memwriter, '}')) != ERROR_NONE)
        {
          goto cleanup;
        }
    }
  /* Only set serial result if not set before */
  if (state->shared->serial_result == 0 && state->is_type_info_incomplete)
    {
      state->shared->serial_result = has_members ? incomplete : incomplete_at_struct_beginning;
    }

cleanup:
  free(member_names);
  free(data_types);
  if (error != ERROR_NONE)
    {
      return error;
    }

  state->shared->wrote_output = 1;

  return ERROR_NONE;
}

err_t tojson_stringify_args_value(memwriter_t *memwriter, grm_args_t *args)
{
  err_t error = ERROR_NONE;

  if ((error = memwriter_putc(memwriter, '{')) != ERROR_NONE)
    {
      return error;
    }
  tojson_permanent_state.serial_result = incomplete_at_struct_beginning;
  if ((error = tojson_write_args(memwriter, args)) != ERROR_NONE)
    {
      return error;
    }

  return ERROR_NONE;
}

int tojson_get_member_count(const char *data_desc)
{
  int member_count = 0;
  if (data_desc == NULL || *data_desc == '\0')
    {
      return 0;
    }
  while (*data_desc != 0)
    {
      if (*data_desc == ',')
        {
          ++member_count;
        }
      ++data_desc;
    }
  ++member_count; /* add last member (because it is not terminated by a ',') */
  return member_count;
}

int tojson_is_json_array_needed(const char *data_desc)
{
  const char *relevant_data_types = "iIdDcCs";
  int nested_level = 0;
  int count_relevant_data_types = 0;

  while (*data_desc != 0 && count_relevant_data_types < 2)
    {
      if (*data_desc == '(')
        {
          ++nested_level;
        }
      else if (*data_desc == ')')
        {
          --nested_level;
        }
      else if (nested_level == 0 && strchr(relevant_data_types, *data_desc))
        {
          ++count_relevant_data_types;
        }
      ++data_desc;
    }
  return count_relevant_data_types >= 2;
}

void tojson_read_datatype(tojson_state_t *state)
{
  char *additional_type_info = NULL;
  state->current_data_type = *state->data_type_ptr;
  ++(state->data_type_ptr);
  if (*state->data_type_ptr == '(')
    {
      int nested_level = 1;
      additional_type_info = ++(state->data_type_ptr);
      while (*state->data_type_ptr != 0 && nested_level > 0)
        {
          if (*state->data_type_ptr == '(')
            {
              ++nested_level;
            }
          else if (*state->data_type_ptr == ')')
            {
              --nested_level;
            }
          if (nested_level > 0)
            {
              ++(state->data_type_ptr);
            }
        }
      if (*state->data_type_ptr != 0)
        {
          *(state->data_type_ptr)++ = 0; /* termination character for additional_type_info */
          state->is_type_info_incomplete = 0;
        }
      else
        {
          state->is_type_info_incomplete = 1; /* character search hit '\0' and not ')' */
        }
    }
  state->additional_type_info = additional_type_info;
}

err_t tojson_skip_bytes(tojson_state_t *state)
{
  unsigned int count;

  if (state->shared->data_ptr == NULL)
    {
      debug_print_error(("Skipping bytes is not supported when using the variable argument list and is ignored.\n"));
      return ERROR_NONE;
    }

  if (state->additional_type_info != NULL)
    {
      if (!str_to_uint(state->additional_type_info, &count))
        {
          debug_print_error(("Byte skipping with an invalid number -> ignoring.\n"));
          return ERROR_NONE;
        }
    }
  else
    {
      count = 1;
    }
  state->shared->data_ptr = ((char *)state->shared->data_ptr) + count;
  state->shared->data_offset += count;

  return ERROR_NONE;
}

err_t tojson_close_object(tojson_state_t *state)
{
  err_t error;
  --(state->shared->struct_nested_level);
  if ((error = memwriter_putc(state->memwriter, '}')) != ERROR_NONE)
    {
      return error;
    }
  return ERROR_NONE;
}

err_t tojson_read_array_length(tojson_state_t *state)
{
  int value;

  RETRIEVE_SINGLE_VALUE(value, size_t, size_t);
  state->shared->array_length = value;

  return ERROR_NONE;
}

err_t tojson_unzip_membernames_and_datatypes(char *mixed_ptr, char ***member_name_ptr, char ***data_type_ptr)
{
  int member_count;
  char **arrays[2];

  member_count = tojson_get_member_count(mixed_ptr);
  /* add 1 to member count for a terminatory NULL pointer */
  *member_name_ptr = malloc((member_count + 1) * sizeof(char *));
  *data_type_ptr = malloc((member_count + 1) * sizeof(char *));
  if (*member_name_ptr == NULL || *data_type_ptr == NULL)
    {
      free(*member_name_ptr);
      free(*data_type_ptr);
      *member_name_ptr = *data_type_ptr = NULL;
      debug_print_malloc_error();
      return ERROR_MALLOC;
    }
  arrays[member_name] = *member_name_ptr;
  arrays[data_type] = *data_type_ptr;
  if (member_count > 0)
    {
      char separators[2] = {':', ','};
      int current_array_index = member_name;
      int nested_type_level = 0;
      *arrays[current_array_index]++ = mixed_ptr;

      /* iterate over the whole type list */
      assert(mixed_ptr != NULL); /* otherwise there is an internal logical error since member_count > 0 */
      while (nested_type_level >= 0 && *mixed_ptr != 0)
        {
          /* extract one name or one type */
          while (*mixed_ptr != 0 && (nested_type_level > 0 || *mixed_ptr != separators[current_array_index]))
            {
              if (current_array_index == data_type)
                {
                  if (*mixed_ptr == '(')
                    {
                      ++nested_type_level;
                    }
                  else if (*mixed_ptr == ')')
                    {
                      --nested_type_level;
                    }
                }
              if (nested_type_level >= 0)
                {
                  ++mixed_ptr;
                }
            }
          if (*mixed_ptr != 0)
            {
              *mixed_ptr++ = 0;                              /* terminate string in buffer */
              current_array_index = 1 - current_array_index; /* alternate between member_name (0) and data_type (1) */
              *arrays[current_array_index]++ = mixed_ptr;
            }
        }
    }
  *arrays[member_name] = NULL;
  *arrays[data_type] = NULL;
  return ERROR_NONE;
}

err_t tojson_escape_special_chars(char **escaped_string, const char *unescaped_string, unsigned int *length)
{
  /* characters '\' and '"' must be escaped before written to a json string value */
  /* length can be `0` -> use `strlen(unescaped_string)` instead */
  const char *src_ptr;
  char *dest_ptr;
  size_t needed_memory;
  unsigned int remaining_chars;
  unsigned int len;
  const char *chars_to_escape = "\\\"";

  len = (length != NULL && *length != 0) ? *length : strlen(unescaped_string);
  needed_memory = len + 1; /* reserve memory for the terminating `\0` character */
  src_ptr = unescaped_string;
  remaining_chars = len;
  while (remaining_chars)
    {
      if (strchr(chars_to_escape, *src_ptr) != NULL)
        {
          ++needed_memory;
        }
      ++src_ptr;
      --remaining_chars;
    }
  dest_ptr = malloc(needed_memory);
  if (dest_ptr == NULL)
    {
      return ERROR_MALLOC;
    }
  *escaped_string = dest_ptr;
  src_ptr = unescaped_string;
  remaining_chars = len;
  while (remaining_chars)
    {
      if (strchr(chars_to_escape, *src_ptr) != NULL)
        {
          *dest_ptr++ = '\\';
        }
      *dest_ptr++ = *src_ptr++;
      --remaining_chars;
    }
  *dest_ptr = '\0';
  if (length != NULL)
    {
      *length = needed_memory - 1;
    }

  return ERROR_NONE;
}

#undef CHECK_PADDING
#undef RETRIEVE_SINGLE_VALUE
#undef INIT_MULTI_VALUE
#undef FIN_MULTI_VALUE

err_t tojson_serialize(memwriter_t *memwriter, char *data_desc, const void *data, va_list *vl, int apply_padding,
                       int add_data, int add_data_without_separator, unsigned int *struct_nested_level,
                       tojson_serialization_result_t *serial_result, tojson_shared_state_t *shared_state)
{
  /**
   * memwriter: memwriter handle
   * data_desc: data description
   *      i: int
   *      I: int array -> I(count) or nI for variable length (see 'n' below)
   *      d: double
   *      D: double array
   *      c: char
   *      C: char array (fixed-width string)
   *      s: string ('\0'-terminated)
   *      n: array length (for all following arrays)
   *      o: object -> o(name:type, name:type, ...)
   *      e: empty byte (ignored memory) -> e(count) to specify multiple bytes
   * data: pointer to the buffer that shall be serialized
   * vl: if data is NULL the needed values are read from the va_list vl
   */

  tojson_state_t state;
  int json_array_needed = 0;
  int allocated_shared_state_mem = 0;
  err_t error = ERROR_NONE;

  state.memwriter = memwriter;
  state.data_type_ptr = data_desc;
  state.current_data_type = 0;
  state.additional_type_info = NULL;
  state.add_data_without_separator = add_data_without_separator;
  state.is_type_info_incomplete = 0;
  if (shared_state == NULL)
    {
      shared_state = malloc(sizeof(tojson_shared_state_t));
      if (shared_state == NULL)
        {
          debug_print_malloc_error();
          goto cleanup;
        }
      shared_state->apply_padding = apply_padding;
      shared_state->array_length = 0;
      shared_state->read_length_from_string = 0;
      shared_state->data_ptr = data;
      shared_state->vl = vl;
      shared_state->data_offset = 0;
      shared_state->wrote_output = 0;
      shared_state->add_data = add_data;
      shared_state->serial_result = 0;
      shared_state->struct_nested_level = *struct_nested_level;
      allocated_shared_state_mem = 1;
    }
  else
    {
      if (data != NULL)
        {
          shared_state->data_ptr = data;
        }
      if (vl != NULL)
        {
          shared_state->vl = vl;
        }
      if (apply_padding >= 0)
        {
          shared_state->apply_padding = apply_padding;
        }
    }
  state.shared = shared_state;

  json_array_needed = tojson_is_json_array_needed(data_desc);
  /* write list head if needed */
  if (json_array_needed)
    {
      if ((error = memwriter_putc(memwriter, '[')) != ERROR_NONE)
        {
          goto cleanup;
        }
    }
  while (*state.data_type_ptr != 0)
    {
      shared_state->wrote_output = 0;
      tojson_read_datatype(&state);
      if (tojson_datatype_to_func[(unsigned char)state.current_data_type])
        {
          error = tojson_datatype_to_func[(unsigned char)state.current_data_type](&state);
        }
      else
        {
          debug_print_error(("WARNING: '%c' (ASCII code %d) is not a valid type identifier\n", state.current_data_type,
                             state.current_data_type));
          error = ERROR_UNSUPPORTED_DATATYPE;
        }
      if (error != ERROR_NONE)
        {
          goto cleanup;
        }
      if (*state.data_type_ptr != 0 && *state.data_type_ptr != ')' && shared_state->wrote_output)
        {
          /* write JSON separator, if data was written and the object is not closed in the next step */
          if ((error = memwriter_putc(memwriter, ',')) != ERROR_NONE)
            {
              goto cleanup;
            }
        }
    }
  /* write list tail if needed */
  if (json_array_needed)
    {
      if ((error = memwriter_putc(memwriter, ']')) != ERROR_NONE)
        {
          goto cleanup;
        }
    }

  if (serial_result != NULL)
    {
      /* check if shared_state->serial_result was set before */
      if (shared_state->serial_result)
        {
          *serial_result = shared_state->serial_result;
        }
      else
        {
          *serial_result = (shared_state->struct_nested_level > 0) ? incomplete : complete;
        }
    }
  if (struct_nested_level != NULL)
    {
      *struct_nested_level = shared_state->struct_nested_level;
    }

cleanup:
  if (allocated_shared_state_mem)
    {
      free(shared_state);
    }

  return error;
}

void tojson_init_static_variables(void)
{
  if (!tojson_static_variables_initialized)
    {
      tojson_datatype_to_func['n'] = tojson_read_array_length;
      tojson_datatype_to_func['e'] = tojson_skip_bytes;
      tojson_datatype_to_func['i'] = tojson_stringify_int;
      tojson_datatype_to_func['I'] = tojson_stringify_int_array;
      tojson_datatype_to_func['d'] = tojson_stringify_double;
      tojson_datatype_to_func['D'] = tojson_stringify_double_array;
      tojson_datatype_to_func['c'] = tojson_stringify_char;
      tojson_datatype_to_func['C'] = tojson_stringify_char_array;
      tojson_datatype_to_func['s'] = tojson_stringify_string;
      tojson_datatype_to_func['S'] = tojson_stringify_string_array;
      tojson_datatype_to_func['b'] = tojson_stringify_bool;
      tojson_datatype_to_func['B'] = tojson_stringify_bool_array;
      tojson_datatype_to_func['o'] = tojson_stringify_object;
      tojson_datatype_to_func['a'] = tojson_stringify_args;
      tojson_datatype_to_func['A'] = tojson_stringify_args_array;
      tojson_datatype_to_func[')'] = tojson_close_object;

      tojson_static_variables_initialized = 1;
    }
}

err_t tojson_init_variables(int *add_data, int *add_data_without_separator, char **_data_desc, const char *data_desc)
{
  tojson_init_static_variables();
  *add_data = (tojson_permanent_state.serial_result != complete);
  *add_data_without_separator = (tojson_permanent_state.serial_result == incomplete_at_struct_beginning);
  if (*add_data)
    {
      char *data_desc_ptr;
      int data_desc_len = strlen(data_desc);
      *_data_desc = malloc(data_desc_len + 3);
      if (*_data_desc == NULL)
        {
          debug_print_malloc_error();
          return ERROR_MALLOC;
        }
      data_desc_ptr = *_data_desc;
      if (strncmp(data_desc, "o(", 2) != 0)
        {
          memcpy(data_desc_ptr, "o(", 2);
          data_desc_ptr += 2;
        }
      memcpy(data_desc_ptr, data_desc, data_desc_len);
      data_desc_ptr += data_desc_len;
      *data_desc_ptr = '\0';
    }
  else
    {
      *_data_desc = gks_strdup(data_desc);
      if (*_data_desc == NULL)
        {
          debug_print_malloc_error();
          return ERROR_MALLOC;
        }
    }

  return ERROR_NONE;
}

err_t tojson_write(memwriter_t *memwriter, const char *data_desc, ...)
{
  va_list vl;
  err_t error;

  va_start(vl, data_desc);
  error = tojson_write_vl(memwriter, data_desc, &vl);
  va_end(vl);

  return error;
}

err_t tojson_write_vl(memwriter_t *memwriter, const char *data_desc, va_list *vl)
{
  int add_data, add_data_without_separator;
  char *_data_desc;
  err_t error;

  error = tojson_init_variables(&add_data, &add_data_without_separator, &_data_desc, data_desc);
  if (!error)
    {
      error =
          tojson_serialize(memwriter, _data_desc, NULL, vl, 0, add_data, add_data_without_separator,
                           &tojson_permanent_state.struct_nested_level, &tojson_permanent_state.serial_result, NULL);
    }
  free(_data_desc);

  return error;
}

err_t tojson_write_buf(memwriter_t *memwriter, const char *data_desc, const void *buffer, int apply_padding)
{
  int add_data, add_data_without_separator;
  char *_data_desc;
  err_t error;

  error = tojson_init_variables(&add_data, &add_data_without_separator, &_data_desc, data_desc);
  if (!error)
    {
      error =
          tojson_serialize(memwriter, _data_desc, buffer, NULL, apply_padding, add_data, add_data_without_separator,
                           &tojson_permanent_state.struct_nested_level, &tojson_permanent_state.serial_result, NULL);
    }
  free(_data_desc);

  return error;
}

err_t tojson_write_arg(memwriter_t *memwriter, const arg_t *arg)
{
  err_t error = ERROR_NONE;

  if (arg->key == NULL)
    {
      if ((error = tojson_write_buf(memwriter, arg->value_format, arg->value_ptr, 1)) != ERROR_NONE)
        {
          return error;
        }
    }
  else
    {
      char *format, *format_ptr;
      size_t key_length, value_format_length;
      key_length = strlen(arg->key);
      value_format_length = strlen(arg->value_format);
      format = malloc(key_length + value_format_length + 2); /* 2 = 1 ':' + 1 '\0' */
      if (format == NULL)
        {
          debug_print_malloc_error();
          return ERROR_MALLOC;
        }
      format_ptr = format;
      memcpy(format_ptr, arg->key, key_length);
      format_ptr += key_length;
      *format_ptr++ = ':';
      memcpy(format_ptr, arg->value_format, value_format_length);
      format_ptr += value_format_length;
      *format_ptr = '\0';
      if ((error = tojson_write_buf(memwriter, format, arg->value_ptr, 1)) != ERROR_NONE)
        {
          free(format);
          return error;
        }
      free(format);
    }

  return error;
}

err_t tojson_write_args(memwriter_t *memwriter, const grm_args_t *args)
{
  grm_args_iterator_t *it;
  arg_t *arg;

  it = grm_args_iter(args);
  if ((arg = it->next(it)))
    {
      tojson_write_buf(memwriter, "o(", NULL, 1);
      do
        {
          tojson_write_arg(memwriter, arg);
        }
      while ((arg = it->next(it)));
      tojson_write_buf(memwriter, ")", NULL, 1);
    }
  args_iterator_delete(it);

  return 0;
}

int tojson_is_complete(void)
{
  return tojson_permanent_state.serial_result == complete;
}

int tojson_struct_nested_level(void)
{
  return tojson_permanent_state.struct_nested_level;
}
