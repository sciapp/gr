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

#include "error_int.h"
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

const char *const FROM_JSON_VALID_DELIMITERS = ",]}";
const char FROM_JSON_STRING_DELIMITER = '"';
const char FROM_JSON_ESCAPE_CHARACTER = '\\';

static grm_error_t (*from_json_datatype_func[])(FromJsonState *) = {NULL,
                                                                    fromJsonParseNull,
                                                                    fromJsonParseBool,
                                                                    fromJsonParseNumber,
                                                                    fromJsonParseString,
                                                                    fromJsonParseArray,
                                                                    fromJsonParseObject};

static const char *const FROM_JSON_DATATYPE_STRING[] = {"unknown", "null",  "bool",  "number",
                                                        "string",  "array", "object"};


/* ------------------------- json serializer ------------------------------------------------------------------------ */

static grm_error_t (*to_json_datatype_to_func[128])(ToJsonState *);
static int to_json_static_variables_initialized = 0;
static ToJsonPermanentState to_json_permanent_state = {COMPLETE, 0};


/* ========================= methods ================================================================================ */

/* ------------------------- json deserializer ---------------------------------------------------------------------- */

int grm_read(grm_args_t *args, const char *json_string)
{
  return (fromJsonRead(args, json_string) == GRM_ERROR_NONE);
}

grm_error_t fromJsonRead(grm_args_t *args, const char *json_string)
{
  return fromJsonParse(args, json_string, NULL);
}

int grm_load_from_str(const char *json_string)
{
  return (fromJsonRead(active_plot_args, json_string) == GRM_ERROR_NONE);
}

grm_error_t fromJsonParse(grm_args_t *args, const char *json_string, FromJsonSharedState *shared_state)
{
  char *filtered_json_string = NULL;
  FromJsonState state;
  int allocated_shared_state_mem = 0;
  grm_error_t error = GRM_ERROR_NONE;
  char *saved_locale;

  state.datatype = JSON_DATATYPE_UNKNOWN;
  state.value_buffer = NULL;
  state.value_buffer_pointer_level = 0;
  state.next_value_memory = NULL;
  state.next_value_type = malloc(NEXT_VALUE_TYPE_SIZE);
  if (state.next_value_type == NULL)
    {
      debugPrintMallocError();
      return GRM_ERROR_MALLOC;
    }
  state.args = args;
  if (shared_state == NULL)
    {
      shared_state = malloc(sizeof(FromJsonSharedState));
      if (shared_state == NULL)
        {
          free(state.next_value_type);
          debugPrintMallocError();
          return GRM_ERROR_MALLOC;
        }
      if ((error = fromJsonCopyAndFilterJsonString(&filtered_json_string, json_string)) != GRM_ERROR_NONE)
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
  if (state.parsing_object) ++state.shared_state->json_ptr;

  saved_locale = setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "C");

  while (strchr("}", *state.shared_state->json_ptr) == NULL)
    {
      const char *current_key = NULL;

      if (state.parsing_object)
        {
          fromJsonParseString(&state);
          current_key = *(const char **)state.value_buffer;
          free(state.value_buffer);
          state.value_buffer = NULL;
          ++(state.shared_state->json_ptr);
        }
      state.datatype = fromJsonCheckType(&state);
      if (state.datatype)
        {
          if ((error = from_json_datatype_func[state.datatype](&state)) != GRM_ERROR_NONE) break;
          if (state.parsing_object)
            {
              grm_args_push_buf(args, current_key, state.next_value_type, state.value_buffer, 0);
            }
          else
            {
              /* parsing values without an outer object (-> missing key) is not supported by the argument container */
              error = GRM_ERROR_PARSE_MISSING_OBJECT_CONTAINER;
              break;
            }
          if (strchr(FROM_JSON_VALID_DELIMITERS, *state.shared_state->json_ptr) != NULL)
            {
              if (*state.shared_state->json_ptr == ',') ++state.shared_state->json_ptr;
            }
          else
            {
              error = GRM_ERROR_PARSE_INVALID_DELIMITER;
              break;
            }
        }
      else
        {
          error = GRM_ERROR_PARSE_UNKNOWN_DATATYPE;
          break;
        }
      if (state.value_buffer_pointer_level > 1)
        {
          int i, outer_array_length = upperCaseCount(state.next_value_type);
          for (i = 0; i < outer_array_length; ++i)
            {
              free(((char **)state.value_buffer)[i]);
            }
        }
      free(state.value_buffer);
      state.value_buffer = NULL;
      state.value_buffer_pointer_level = 0;
    }
  if (state.parsing_object && *state.shared_state->json_ptr == '\0') error = GRM_ERROR_PARSE_INCOMPLETE_STRING;
  if (*state.shared_state->json_ptr) ++state.shared_state->json_ptr;

  free(state.value_buffer);
  free(filtered_json_string);
  free(state.next_value_type);

  if (allocated_shared_state_mem) free(shared_state);

  if (saved_locale) setlocale(LC_NUMERIC, saved_locale);

  return error;
}

#define checkAndAllocateMemory(type, length)                   \
  do                                                           \
    {                                                          \
      if (state->value_buffer == NULL)                         \
        {                                                      \
          state->value_buffer = malloc(length * sizeof(type)); \
          if (state->value_buffer == NULL)                     \
            {                                                  \
              debugPrintMallocError();                         \
              return 0;                                        \
            }                                                  \
          state->value_buffer_pointer_level = 1;               \
          state->next_value_memory = state->value_buffer;      \
        }                                                      \
    }                                                          \
  while (0)

grm_error_t fromJsonParseNull(FromJsonState *state)
{
  if (strncmp(state->shared_state->json_ptr, "null", 4) != 0) return GRM_ERROR_PARSE_NULL;
  strcpy(state->next_value_type, "");
  state->shared_state->json_ptr += 4;
  return GRM_ERROR_NONE;
}

grm_error_t fromJsonParseBool(FromJsonState *state)
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
      return GRM_ERROR_PARSE_BOOL;
    }
  checkAndAllocateMemory(int, 1);
  *((int *)state->next_value_memory) = bool_value;
  strcpy(state->next_value_type, "i");
  state->shared_state->json_ptr += bool_value ? 4 : 5;
  return GRM_ERROR_NONE;
}

grm_error_t fromJsonParseNumber(FromJsonState *state)
{
  grm_error_t error;

  if (isIntNumber(state->shared_state->json_ptr))
    {
      error = fromJsonParseInt(state);
    }
  else
    {
      error = fromJsonParseDouble(state);
    }
  return error;
}

grm_error_t fromJsonParseInt(FromJsonState *state)
{
  int was_successful;
  int int_value;

  int_value = fromJsonStrToInt((const char **)&state->shared_state->json_ptr, &was_successful);
  if (!was_successful) return GRM_ERROR_PARSE_INT;
  checkAndAllocateMemory(int, 1);
  *((int *)state->next_value_memory) = int_value;
  strcpy(state->next_value_type, "i");
  return GRM_ERROR_NONE;
}

grm_error_t fromJsonParseDouble(FromJsonState *state)
{
  int was_successful;
  double double_value;

  double_value = fromJsonStrToDouble((const char **)&state->shared_state->json_ptr, &was_successful);
  if (!was_successful) return GRM_ERROR_PARSE_DOUBLE;
  checkAndAllocateMemory(double, 1);
  *((double *)state->next_value_memory) = double_value;
  strcpy(state->next_value_type, "d");
  return GRM_ERROR_NONE;
}

grm_error_t fromJsonParseString(FromJsonState *state)
{
  char *string_value;
  char *json_ptr;
  const char *src_ptr;
  char *dest_ptr;
  int string_is_complete;
  int skipped_char;

  checkAndAllocateMemory(char *, 1);
  json_ptr = state->shared_state->json_ptr;
  string_value = ++json_ptr;
  while (*json_ptr && !isStringDelimiter(json_ptr, string_value))
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

  return string_is_complete ? GRM_ERROR_NONE : GRM_ERROR_PARSE_STRING;
}

grm_error_t fromJsonParseArray(FromJsonState *state)
{
  FromJsonDatatype json_datatype;
  const char *next_delim_ptr;

#define parseValues(parse_suffix, c_type)                                                                          \
  do                                                                                                               \
    {                                                                                                              \
      c_type *values;                                                                                              \
      c_type *current_value_ptr;                                                                                   \
      checkAndAllocateMemory(c_type *, 1);                                                                         \
      values = malloc(array_length * sizeof(c_type));                                                              \
      if (values == NULL)                                                                                          \
        {                                                                                                          \
          debugPrintMallocError();                                                                                 \
          return GRM_ERROR_MALLOC;                                                                                 \
        }                                                                                                          \
      current_value_ptr = values;                                                                                  \
      *(c_type **)state->next_value_memory = values;                                                               \
      state->value_buffer_pointer_level = 2;                                                                       \
      state->next_value_memory = values;                                                                           \
      --state->shared_state->json_ptr;                                                                             \
      while (!error && strchr("]", *state->shared_state->json_ptr) == NULL)                                        \
        {                                                                                                          \
          ++state->shared_state->json_ptr;                                                                         \
          error = fromJsonParse##parse_suffix(state);                                                              \
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
          outer_array_length = fromJsonGetOuterArrayLength(state->shared_state->json_ptr);
          /* `char *` is only used as a generic type since all pointers to values have the same storage size */
          checkAndAllocateMemory(char *, outer_array_length);
        }
      array_type[0] = '\0';
      do
        {
          grm_error_t error = GRM_ERROR_NONE;
          size_t array_length = 0;
          state->shared_state->json_ptr += is_nested_array ? 2 : 1;
          next_delim_ptr = state->shared_state->json_ptr;
          while (*next_delim_ptr != ']' &&
                 fromJsonFindNextDelimiter(&next_delim_ptr, next_delim_ptr, array_length == 0, 1))
            {
              ++array_length;
            }
          if (*next_delim_ptr != ']')
            {
              state->shared_state->json_ptr -= is_nested_array ? 2 : 1;
              return GRM_ERROR_PARSE_INCOMPLETE_STRING;
            }
          assert(array_length > 0);
          json_datatype = fromJsonCheckType(state);
          switch (json_datatype)
            {
            case JSON_DATATYPE_NUMBER:
              if (isIntNumber(state->shared_state->json_ptr))
                {
                  parseValues(Int, int);
                }
              else
                {
                  parseValues(Double, double);
                }
              break;
            case JSON_DATATYPE_STRING:
              parseValues(String, char *);
              break;
            case JSON_DATATYPE_OBJECT:
              parseValues(Object, grm_args_t *);
              break;
            case JSON_DATATYPE_ARRAY:
              debugPrintError(("Arrays only support one level of nesting!\n"));
              state->shared_state->json_ptr -= is_nested_array ? 2 : 1;
              return GRM_ERROR_PARSE_ARRAY;
              break;
            default:
              debugPrintError(("The datatype \"%s\" is currently not supported in arrays!\n",
                               FROM_JSON_DATATYPE_STRING[json_datatype]));
              state->shared_state->json_ptr -= is_nested_array ? 2 : 1;
              return GRM_ERROR_PARSE_ARRAY;
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

  return GRM_ERROR_NONE;

#undef PARSE_VALUES
}

grm_error_t fromJsonParseObject(FromJsonState *state)
{
  grm_args_t *args;
  grm_error_t error;

  checkAndAllocateMemory(grm_args_t *, 1);
  args = grm_args_new();
  error = fromJsonParse(args, state->shared_state->json_ptr, state->shared_state);
  *((grm_args_t **)state->next_value_memory) = args;
  strcpy(state->next_value_type, "a");
  return error;
}

#undef CHECK_AND_ALLOCATE_MEMORY

FromJsonDatatype fromJsonCheckType(const FromJsonState *state)
{
  FromJsonDatatype datatype;

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

grm_error_t fromJsonCopyAndFilterJsonString(char **dest, const char *src)
{
  const char *src_ptr;
  char *dest_buffer, *dest_ptr;
  int in_string;

  src_ptr = src;
  dest_buffer = malloc(strlen(src) + 1);
  if (dest_buffer == NULL)
    {
      debugPrintMallocError();
      return GRM_ERROR_MALLOC;
    }
  dest_ptr = dest_buffer;

  in_string = 0;
  while (*src_ptr)
    {
      if (isStringDelimiter(src_ptr, src)) in_string = !in_string;
      if (in_string || !isspace(*src_ptr))
        {
          *dest_ptr = *src_ptr;
          ++dest_ptr;
        }
      ++src_ptr;
    }
  *dest_ptr = '\0';

  *dest = dest_buffer;

  return GRM_ERROR_NONE;
}

int fromJsonIsEscapedDelimiter(const char *delim_ptr, const char *str)
{
  const char *first_non_escape_char_ptr;

  first_non_escape_char_ptr = delim_ptr - 1;
  while (first_non_escape_char_ptr != str - 1 && *first_non_escape_char_ptr == FROM_JSON_ESCAPE_CHARACTER)
    {
      --first_non_escape_char_ptr;
    }
  return (delim_ptr - first_non_escape_char_ptr) % 2 == 0;
}

int fromJsonFindNextDelimiter(const char **delim_ptr, const char *src, int include_start, int exclude_nested_structures)
{
  int is_in_string = 0;

  if (*src == '\0') return 0;
  if (!include_start) ++src;
  if (exclude_nested_structures)
    {
      const char *src_ptr = src;
      int nested_level = 0;

      while (*src_ptr != '\0')
        {
          if (*src_ptr == FROM_JSON_STRING_DELIMITER && !fromJsonIsEscapedDelimiter(src_ptr, src))
            is_in_string = !is_in_string;
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
      while (*src_ptr != '\0' && (is_in_string || strchr(FROM_JSON_VALID_DELIMITERS, *src_ptr) == NULL))
        {
          if (*src_ptr == FROM_JSON_STRING_DELIMITER && !fromJsonIsEscapedDelimiter(src_ptr, src))
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

size_t fromJsonGetOuterArrayLength(const char *str)
{
  size_t outer_array_length = 0;
  int current_array_level = 1;

  if (*str != '[') return outer_array_length;
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

double fromJsonStrToDouble(const char **str, int *was_successful)
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
      debugPrintError(("No number conversion was executed (the string is NULL)!\n"));
    }
  else if (*str == conversion_end || strchr(FROM_JSON_VALID_DELIMITERS, *conversion_end) == NULL)
    {
      fromJsonFindNextDelimiter(&next_delim_ptr, *str, 1, 0);
      debugPrintError(("The parameter \"%.*s\" is not a valid number!\n", next_delim_ptr - *str, *str));
    }
  else if (errno == ERANGE)
    {
      fromJsonFindNextDelimiter(&next_delim_ptr, *str, 1, 0);
      if (conversion_result == HUGE_VAL || conversion_result == -HUGE_VAL)
        {
          debugPrintError(("The parameter \"%.*s\" caused an overflow, the number has been clamped to \"%lf\"\n",
                           next_delim_ptr - *str, *str, conversion_result));
        }
      else
        {
          debugPrintError(("The parameter \"%.*s\" caused an underflow, the number has been clamped to \"%lf\"\n",
                           next_delim_ptr - *str, *str, conversion_result));
        }
    }
  else
    {
      success = 1;
      *str = conversion_end;
    }
  if (was_successful != NULL) *was_successful = success;

  return conversion_result;
}

int fromJsonStrToInt(const char **str, int *was_successful)
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
      debugPrintError(("No number conversion was executed (the string is NULL)!\n"));
    }
  else if (*str == conversion_end || strchr(FROM_JSON_VALID_DELIMITERS, *conversion_end) == NULL)
    {
      fromJsonFindNextDelimiter(&next_delim_ptr, *str, 1, 0);
      debugPrintError(("The parameter \"%.*s\" is not a valid number!\n", next_delim_ptr - *str, *str));
    }
  else if (errno == ERANGE || conversion_result > INT_MAX || conversion_result < INT_MIN)
    {
      fromJsonFindNextDelimiter(&next_delim_ptr, *str, 1, 0);
      if (conversion_result > INT_MAX)
        {
          debugPrintError(("The parameter \"%.*s\" is too big, the number has been clamped to \"%d\"\n",
                           next_delim_ptr - *str, *str, INT_MAX));
          conversion_result = INT_MAX;
        }
      else
        {
          debugPrintError(("The parameter \"%.*s\" is too small, the number has been clamped to \"%d\"\n",
                           next_delim_ptr - *str, *str, INT_MIN));
          conversion_result = INT_MIN;
        }
    }
  else
    {
      success = 1;
      *str = conversion_end;
    }
  if (was_successful != NULL) *was_successful = success;

  return (int)conversion_result;
}


/* ------------------------- json serializer ------------------------------------------------------------------------ */

#define checkPadding(type)                                                              \
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

#define retrieveSingleValue(var, type, promoted_type)                      \
  do                                                                       \
    {                                                                      \
      checkPadding(type);                                                  \
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

#define initMultiValue(vars, type)                   \
  do                                                 \
    {                                                \
      if (state->shared->data_ptr != NULL)           \
        {                                            \
          checkPadding(type *);                      \
          vars = *(type **)state->shared->data_ptr;  \
        }                                            \
      else                                           \
        {                                            \
          vars = va_arg(*state->shared->vl, type *); \
        }                                            \
    }                                                \
  while (0)

#define finMultiValue(type)                                                 \
  do                                                                        \
    {                                                                       \
      if (state->shared->data_ptr != NULL)                                  \
        {                                                                   \
          state->shared->data_ptr = ((type **)state->shared->data_ptr) + 1; \
          state->shared->data_offset += sizeof(type *);                     \
        }                                                                   \
    }                                                                       \
  while (0)

#define defineStringifyValue(name, type, format_specifier)                   \
  grm_error_t toJsonStringify##name##Value(Memwriter *memwriter, type value) \
  {                                                                          \
    return memwriterPrintf(memwriter, format_specifier, value);              \
  }

#define defineStringifySingle(name, type, promoted_type)                                   \
  grm_error_t toJsonStringify##name(ToJsonState *state)                                    \
  {                                                                                        \
    type value;                                                                            \
    grm_error_t error = GRM_ERROR_NONE;                                                    \
    retrieveSingleValue(value, type, promoted_type);                                       \
    if ((error = toJsonStringify##name##Value(state->memwriter, value)) != GRM_ERROR_NONE) \
      {                                                                                    \
        return error;                                                                      \
      }                                                                                    \
    state->shared->wrote_output = 1;                                                       \
    return error;                                                                          \
  }

#define defineStringifyMulti(name, type)                                                                              \
  grm_error_t toJsonStringify##name##Array(ToJsonState *state)                                                        \
  {                                                                                                                   \
    type *values;                                                                                                     \
    type current_value;                                                                                               \
    unsigned int length;                                                                                              \
    int remaining_elements;                                                                                           \
    grm_error_t error = GRM_ERROR_NONE;                                                                               \
    initMultiValue(values, type);                                                                                     \
    if (state->additional_type_info != NULL)                                                                          \
      {                                                                                                               \
        if (!strToUint(state->additional_type_info, &length))                                                         \
          {                                                                                                           \
            debugPrintError(("The given array length \"%s\" is no valid number; the array contents will be ignored.", \
                             state->additional_type_info));                                                           \
            length = 0;                                                                                               \
          }                                                                                                           \
      }                                                                                                               \
    else                                                                                                              \
      {                                                                                                               \
        length = state->shared->array_length;                                                                         \
      }                                                                                                               \
    remaining_elements = length;                                                                                      \
    /* write array start */                                                                                           \
    if ((error = memwriterPutc(state->memwriter, '[')) != GRM_ERROR_NONE) return error;                               \
    /* write array content */                                                                                         \
    while (remaining_elements)                                                                                        \
      {                                                                                                               \
        current_value = *values++;                                                                                    \
        if ((error = toJsonStringify##name##Value(state->memwriter, current_value)) != GRM_ERROR_NONE) return error;  \
        if (remaining_elements > 1)                                                                                   \
          {                                                                                                           \
            if ((error = memwriterPutc(state->memwriter, ',')) != GRM_ERROR_NONE) return error;                       \
          }                                                                                                           \
        --remaining_elements;                                                                                         \
      }                                                                                                               \
    /* write array end */                                                                                             \
    if ((error = memwriterPutc(state->memwriter, ']')) != GRM_ERROR_NONE) return error;                               \
    finMultiValue(type);                                                                                              \
    state->shared->wrote_output = 1;                                                                                  \
    return error;                                                                                                     \
  }

defineStringifySingle(Int, int, int) defineStringifyMulti(Int, int) defineStringifyValue(Int, int, "%d")
    defineStringifySingle(Double, double, double) defineStringifyMulti(Double, double)
        defineStringifySingle(Char, char, int) defineStringifyValue(Char, char, "%c")
            defineStringifySingle(String, char *, char *) defineStringifyMulti(String, char *)
                defineStringifySingle(Bool, int, int) defineStringifyMulti(Bool, int)
                    defineStringifySingle(Args, grm_args_t *, grm_args_t *) defineStringifyMulti(Args, grm_args_t *)

#undef defineStringifySingle
#undef defineStringifyMulti
#undef defineStringifyValue

#define str(x) #x
#define xStr(x) str(x)

                        grm_error_t toJsonStringifyDoubleValue(Memwriter *memwriter, double value)
{
  grm_error_t error;
  size_t string_start_index;
  const char *unprocessed_string;

  string_start_index = memwriterSize(memwriter);
  /*
     The %G format specifier is important here, as it returns "NAN" for missing values -
     %g would return "nan" and be handled as JSON_DATATYPE_NULL in from_json_check_type()
   */
  if ((error = memwriterPrintf(memwriter, "%." xStr(DBL_DECIMAL_DIG) "G", value)) != GRM_ERROR_NONE) return error;
  unprocessed_string = memwriterBuf(memwriter) + string_start_index;
  if (strspn(unprocessed_string, "0123456789-") == memwriterSize(memwriter) - string_start_index)
    {
      if ((error = memwriterPutc(memwriter, '.')) != GRM_ERROR_NONE) return error;
    }
  return GRM_ERROR_NONE;
}

#undef xStr
#undef str

grm_error_t toJsonStringifyCharArray(ToJsonState *state)
{
  char *chars;
  char *escaped_chars = NULL;
  unsigned int length;
  grm_error_t error = GRM_ERROR_NONE;

  initMultiValue(chars, char);

  if (state->additional_type_info != NULL)
    {
      if (!strToUint(state->additional_type_info, &length))
        {
          debugPrintError(("The given array length \"%s\" is no valid number; the array contents will be ignored.",
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
  if ((error = toJsonEscapeSpecialChars(&escaped_chars, chars, &length)) != GRM_ERROR_NONE) goto cleanup;
  if ((error = memwriterPrintf(state->memwriter, "\"%.*s\"", length, escaped_chars)) != GRM_ERROR_NONE) goto cleanup;
  state->shared->wrote_output = 1;

  finMultiValue(char);

cleanup:
  free(escaped_chars);
  return error;
}

grm_error_t toJsonStringifyStringValue(Memwriter *memwriter, char *value)
{
  char *escaped_chars = NULL;
  unsigned int length = 0;
  grm_error_t error = GRM_ERROR_NONE;

  if ((error = toJsonEscapeSpecialChars(&escaped_chars, value, &length))) goto cleanup;
  if ((error = memwriterPrintf(memwriter, "\"%s\"", escaped_chars)) != GRM_ERROR_NONE) goto cleanup;

cleanup:
  free(escaped_chars);
  return error;
}

grm_error_t toJsonStringifyBoolValue(Memwriter *memwriter, int value)
{
  return memwriterPuts(memwriter, value ? "true" : "false");
}

grm_error_t tojsonStringifyObject(ToJsonState *state)
{
  char **member_names = NULL;
  char **data_types = NULL;
  char **member_name_ptr;
  char **data_type_ptr;
  int has_members;
  grm_error_t error = GRM_ERROR_NONE;

  /* IMPORTANT: additional_type_info is altered after the unzip call! */
  if ((error = toJsonUnzipMemberNamesAndDatatypes(state->additional_type_info, &member_names, &data_types)) !=
      GRM_ERROR_NONE)
    goto cleanup;
  member_name_ptr = member_names;
  data_type_ptr = data_types;

  has_members =
      (member_name_ptr != NULL && *member_name_ptr != NULL && data_type_ptr != NULL && *data_type_ptr != NULL);
  /* write object start */
  if (!state->add_data_without_separator)
    {
      if (state->shared->add_data && has_members)
        {
          if ((error = memwriterPutc(state->memwriter, ',')) != GRM_ERROR_NONE) goto cleanup;
        }
      else if (!state->shared->add_data)
        {
          if ((error = memwriterPutc(state->memwriter, '{')) != GRM_ERROR_NONE) goto cleanup;
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
          if ((error = memwriterPrintf(state->memwriter, "\"%s\":", *member_name_ptr)) != GRM_ERROR_NONE) goto cleanup;
          if ((error = toJsonSerialize(state->memwriter, *data_type_ptr, NULL, NULL, -1, -1, 0, NULL, NULL,
                                       state->shared)) != GRM_ERROR_NONE)
            goto cleanup;
          ++member_name_ptr;
          ++data_type_ptr;
          if (*member_name_ptr != NULL && *data_type_ptr != NULL)
            {
              /* write JSON separator */
              if ((error = memwriterPutc(state->memwriter, ',')) != GRM_ERROR_NONE) goto cleanup;
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
      if ((error = memwriterPutc(state->memwriter, '}')) != GRM_ERROR_NONE) goto cleanup;
    }
  /* Only set serial result if not set before */
  if (state->shared->serial_result == 0 && state->is_type_info_incomplete)
    {
      state->shared->serial_result = has_members ? INCOMPLETE : INCOMPLETE_AT_STRUCT_BEGINNING;
    }

cleanup:
  free(member_names);
  free(data_types);
  if (error != GRM_ERROR_NONE) return error;

  state->shared->wrote_output = 1;

  return GRM_ERROR_NONE;
}

grm_error_t toJsonStringifyArgsValue(Memwriter *memwriter, grm_args_t *args)
{
  grm_error_t error = GRM_ERROR_NONE;

  if ((error = memwriterPutc(memwriter, '{')) != GRM_ERROR_NONE) return error;
  to_json_permanent_state.serial_result = INCOMPLETE_AT_STRUCT_BEGINNING;
  if ((error = toJsonWriteArgs(memwriter, args)) != GRM_ERROR_NONE) return error;

  return GRM_ERROR_NONE;
}

int toJsonGetMemberCount(const char *data_desc)
{
  int member_count = 0;
  if (data_desc == NULL || *data_desc == '\0') return 0;
  while (*data_desc != 0)
    {
      if (*data_desc == ',') ++member_count;
      ++data_desc;
    }
  ++member_count; /* add last member (because it is not terminated by a ',') */
  return member_count;
}

int toJsonIsJsonArrayNeeded(const char *data_desc)
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

void toJsonReadDatatype(ToJsonState *state)
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

grm_error_t toJsonSkipBytes(ToJsonState *state)
{
  unsigned int count;

  if (state->shared->data_ptr == NULL)
    {
      debugPrintError(("Skipping bytes is not supported when using the variable argument list and is ignored.\n"));
      return GRM_ERROR_NONE;
    }

  if (state->additional_type_info != NULL)
    {
      if (!strToUint(state->additional_type_info, &count))
        {
          debugPrintError(("Byte skipping with an invalid number -> ignoring.\n"));
          return GRM_ERROR_NONE;
        }
    }
  else
    {
      count = 1;
    }
  state->shared->data_ptr = ((char *)state->shared->data_ptr) + count;
  state->shared->data_offset += count;

  return GRM_ERROR_NONE;
}

grm_error_t toJsonCloseObject(ToJsonState *state)
{
  grm_error_t error;
  --(state->shared->struct_nested_level);
  if ((error = memwriterPutc(state->memwriter, '}')) != GRM_ERROR_NONE) return error;
  return GRM_ERROR_NONE;
}

grm_error_t toJsonReadArrayLength(ToJsonState *state)
{
  int value;

  retrieveSingleValue(value, size_t, size_t);
  state->shared->array_length = value;

  return GRM_ERROR_NONE;
}

grm_error_t toJsonUnzipMemberNamesAndDatatypes(char *mixed_ptr, char ***member_name_ptr, char ***data_type_ptr)
{
  int member_count;
  char **arrays[2];

  member_count = toJsonGetMemberCount(mixed_ptr);
  /* add 1 to member count for a terminatory NULL pointer */
  *member_name_ptr = malloc((member_count + 1) * sizeof(char *));
  *data_type_ptr = malloc((member_count + 1) * sizeof(char *));
  if (*member_name_ptr == NULL || *data_type_ptr == NULL)
    {
      free(*member_name_ptr);
      free(*data_type_ptr);
      *member_name_ptr = *data_type_ptr = NULL;
      debugPrintMallocError();
      return GRM_ERROR_MALLOC;
    }
  arrays[MEMBER_NAME] = *member_name_ptr;
  arrays[DATA_TYPE] = *data_type_ptr;
  if (member_count > 0)
    {
      char separators[2] = {':', ','};
      int current_array_index = MEMBER_NAME;
      int nested_type_level = 0;
      *arrays[current_array_index]++ = mixed_ptr;

      /* iterate over the whole type list */
      assert(mixed_ptr != NULL); /* otherwise there is an internal logical error since member_count > 0 */
      while (nested_type_level >= 0 && *mixed_ptr != 0)
        {
          /* extract one name or one type */
          while (*mixed_ptr != 0 && (nested_type_level > 0 || *mixed_ptr != separators[current_array_index]))
            {
              if (current_array_index == DATA_TYPE)
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
              if (nested_type_level >= 0) ++mixed_ptr;
            }
          if (*mixed_ptr != 0)
            {
              *mixed_ptr++ = 0;                              /* terminate string in buffer */
              current_array_index = 1 - current_array_index; /* alternate between member_name (0) and data_type (1) */
              *arrays[current_array_index]++ = mixed_ptr;
            }
        }
    }
  *arrays[MEMBER_NAME] = NULL;
  *arrays[DATA_TYPE] = NULL;
  return GRM_ERROR_NONE;
}

grm_error_t toJsonEscapeSpecialChars(char **escaped_string, const char *unescaped_string, unsigned int *length)
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
      if (strchr(chars_to_escape, *src_ptr) != NULL) ++needed_memory;
      ++src_ptr;
      --remaining_chars;
    }
  dest_ptr = malloc(needed_memory);
  if (dest_ptr == NULL) return GRM_ERROR_MALLOC;
  *escaped_string = dest_ptr;
  src_ptr = unescaped_string;
  remaining_chars = len;
  while (remaining_chars)
    {
      if (strchr(chars_to_escape, *src_ptr) != NULL) *dest_ptr++ = '\\';
      *dest_ptr++ = *src_ptr++;
      --remaining_chars;
    }
  *dest_ptr = '\0';
  if (length != NULL) *length = needed_memory - 1;

  return GRM_ERROR_NONE;
}

#undef CHECK_PADDING
#undef RETRIEVE_SINGLE_VALUE
#undef INIT_MULTI_VALUE
#undef FIN_MULTI_VALUE

grm_error_t toJsonSerialize(Memwriter *memwriter, char *data_desc, const void *data, va_list *vl, int apply_padding,
                            int add_data, int add_data_without_separator, unsigned int *struct_nested_level,
                            ToJsonSerializationResult *serial_result, ToJsonSharedState *shared_state)
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

  ToJsonState state;
  int json_array_needed = 0;
  int allocated_shared_state_mem = 0;
  grm_error_t error = GRM_ERROR_NONE;

  state.memwriter = memwriter;
  state.data_type_ptr = data_desc;
  state.current_data_type = 0;
  state.additional_type_info = NULL;
  state.add_data_without_separator = add_data_without_separator;
  state.is_type_info_incomplete = 0;
  if (shared_state == NULL)
    {
      shared_state = malloc(sizeof(ToJsonSharedState));
      if (shared_state == NULL)
        {
          debugPrintMallocError();
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
      if (data != NULL) shared_state->data_ptr = data;
      if (vl != NULL) shared_state->vl = vl;
      if (apply_padding >= 0) shared_state->apply_padding = apply_padding;
    }
  state.shared = shared_state;

  json_array_needed = toJsonIsJsonArrayNeeded(data_desc);
  /* write list head if needed */
  if (json_array_needed)
    {
      if ((error = memwriterPutc(memwriter, '[')) != GRM_ERROR_NONE) goto cleanup;
    }
  while (*state.data_type_ptr != 0)
    {
      shared_state->wrote_output = 0;
      toJsonReadDatatype(&state);
      if (to_json_datatype_to_func[(unsigned char)state.current_data_type])
        {
          error = to_json_datatype_to_func[(unsigned char)state.current_data_type](&state);
        }
      else
        {
          debugPrintError(("WARNING: '%c' (ASCII code %d) is not a valid type identifier\n", state.current_data_type,
                           state.current_data_type));
          error = GRM_ERROR_UNSUPPORTED_DATATYPE;
        }
      if (error != GRM_ERROR_NONE) goto cleanup;
      if (*state.data_type_ptr != 0 && *state.data_type_ptr != ')' && shared_state->wrote_output)
        {
          /* write JSON separator, if data was written and the object is not closed in the next step */
          if ((error = memwriterPutc(memwriter, ',')) != GRM_ERROR_NONE) goto cleanup;
        }
    }
  /* write list tail if needed */
  if (json_array_needed)
    {
      if ((error = memwriterPutc(memwriter, ']')) != GRM_ERROR_NONE) goto cleanup;
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
          *serial_result = (shared_state->struct_nested_level > 0) ? INCOMPLETE : COMPLETE;
        }
    }
  if (struct_nested_level != NULL) *struct_nested_level = shared_state->struct_nested_level;

cleanup:
  if (allocated_shared_state_mem) free(shared_state);

  return error;
}

void toJsonInitStaticVariables(void)
{
  if (!to_json_static_variables_initialized)
    {
      to_json_datatype_to_func['n'] = toJsonReadArrayLength;
      to_json_datatype_to_func['e'] = toJsonSkipBytes;
      to_json_datatype_to_func['i'] = toJsonStringifyInt;
      to_json_datatype_to_func['I'] = toJsonStringifyIntArray;
      to_json_datatype_to_func['d'] = toJsonStringifyDouble;
      to_json_datatype_to_func['D'] = toJsonStringifyDoubleArray;
      to_json_datatype_to_func['c'] = toJsonStringifyChar;
      to_json_datatype_to_func['C'] = toJsonStringifyCharArray;
      to_json_datatype_to_func['s'] = toJsonStringifyString;
      to_json_datatype_to_func['S'] = toJsonStringifyStringArray;
      to_json_datatype_to_func['b'] = toJsonStringifyBool;
      to_json_datatype_to_func['B'] = toJsonStringifyBoolArray;
      to_json_datatype_to_func['o'] = tojsonStringifyObject;
      to_json_datatype_to_func['a'] = toJsonStringifyArgs;
      to_json_datatype_to_func['A'] = toJsonStringifyArgsArray;
      to_json_datatype_to_func[')'] = toJsonCloseObject;

      to_json_static_variables_initialized = 1;
    }
}

grm_error_t toJsonInitVariables(int *add_data, int *add_data_without_separator, char **data_desc_priv,
                                const char *data_desc)
{
  toJsonInitStaticVariables();
  *add_data = (to_json_permanent_state.serial_result != COMPLETE);
  *add_data_without_separator = (to_json_permanent_state.serial_result == INCOMPLETE_AT_STRUCT_BEGINNING);
  if (*add_data)
    {
      char *data_desc_ptr;
      int data_desc_len = strlen(data_desc);
      *data_desc_priv = malloc(data_desc_len + 3);
      if (*data_desc_priv == NULL)
        {
          debugPrintMallocError();
          return GRM_ERROR_MALLOC;
        }
      data_desc_ptr = *data_desc_priv;
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
      *data_desc_priv = gks_strdup(data_desc);
      if (*data_desc_priv == NULL)
        {
          debugPrintMallocError();
          return GRM_ERROR_MALLOC;
        }
    }

  return GRM_ERROR_NONE;
}

grm_error_t toJsonWrite(Memwriter *memwriter, const char *data_desc, ...)
{
  va_list vl;
  grm_error_t error;

  va_start(vl, data_desc);
  error = toJsonWriteVl(memwriter, data_desc, &vl);
  va_end(vl);

  return error;
}

grm_error_t toJsonWriteVl(Memwriter *memwriter, const char *data_desc, va_list *vl)
{
  int add_data, add_data_without_separator;
  char *data_desc_priv;
  grm_error_t error;

  error = toJsonInitVariables(&add_data, &add_data_without_separator, &data_desc_priv, data_desc);
  if (!error)
    {
      error =
          toJsonSerialize(memwriter, data_desc_priv, NULL, vl, 0, add_data, add_data_without_separator,
                          &to_json_permanent_state.struct_nested_level, &to_json_permanent_state.serial_result, NULL);
    }
  free(data_desc_priv);

  return error;
}

grm_error_t toJsonWriteBuf(Memwriter *memwriter, const char *data_desc, const void *buffer, int apply_padding)
{
  int add_data, add_data_without_separator;
  char *data_desc_priv;
  grm_error_t error;

  error = toJsonInitVariables(&add_data, &add_data_without_separator, &data_desc_priv, data_desc);
  if (!error)
    {
      error =
          toJsonSerialize(memwriter, data_desc_priv, buffer, NULL, apply_padding, add_data, add_data_without_separator,
                          &to_json_permanent_state.struct_nested_level, &to_json_permanent_state.serial_result, NULL);
    }
  free(data_desc_priv);

  return error;
}

grm_error_t toJsonWriteArg(Memwriter *memwriter, const grm_arg_t *arg)
{
  grm_error_t error;

  if (arg->key == NULL)
    {
      if ((error = toJsonWriteBuf(memwriter, arg->value_format, arg->value_ptr, 1)) != GRM_ERROR_NONE) return error;
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
          debugPrintMallocError();
          return GRM_ERROR_MALLOC;
        }
      format_ptr = format;
      memcpy(format_ptr, arg->key, key_length);
      format_ptr += key_length;
      *format_ptr++ = ':';
      memcpy(format_ptr, arg->value_format, value_format_length);
      format_ptr += value_format_length;
      *format_ptr = '\0';
      if ((error = toJsonWriteBuf(memwriter, format, arg->value_ptr, 1)) != GRM_ERROR_NONE)
        {
          free(format);
          return error;
        }
      free(format);
    }

  return error;
}

grm_error_t toJsonWriteArgs(Memwriter *memwriter, const grm_args_t *args)
{
  grm_args_iterator_t *it;
  grm_arg_t *arg;

  it = grm_args_iter(args);
  if ((arg = it->next(it)))
    {
      toJsonWriteBuf(memwriter, "o(", NULL, 1);
      do
        {
          toJsonWriteArg(memwriter, arg);
        }
      while ((arg = it->next(it)));
      toJsonWriteBuf(memwriter, ")", NULL, 1);
    }
  argsIteratorDelete(it);

  return 0;
}

int toJsonIsComplete(void)
{
  return to_json_permanent_state.serial_result == COMPLETE;
}

int toJsonStructNestedLevel(void)
{
  return to_json_permanent_state.struct_nested_level;
}
