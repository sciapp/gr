#ifdef __unix__
#define _POSIX_C_SOURCE 200112L
#endif

/* ######################### includes ############################################################################### */

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "args_int.h"
#include "gkscore.h"
#include "logging_int.h"
#include "plot_int.h"


/* ######################### internal implementation ################################################################ */

/* ========================= static variables ======================================================================= */

/* ------------------------- argument parsing ----------------------------------------------------------------------- */

static int argparse_valid_format[128];
static ReadParam argparse_format_to_read_callback[128];
static CopyValue argparse_format_to_copy_callback[128];
static DeleteValue argparse_format_to_delete_callback[128];
static size_t argparse_format_to_size[128];
static int argparse_format_has_array_terminator[128];
static int argparse_static_variables_initialized = 0;


/* ------------------------- argument container --------------------------------------------------------------------- */

static const char *const ARGS_VALID_FORMAT_SPECIFIERS = "niIdDcCsSaA";
static const char *const ARGS_VALID_DATA_FORMAT_SPECIFIERS = "idcsa"; /* Each specifier is also valid in upper case */


/* ========================= functions ============================================================================== */

/* ------------------------- argument parsing ----------------------------------------------------------------------- */

void *argparseReadParams(const char *format, const void *buffer, va_list *vl, int apply_padding, char **new_format)
{
  char *fmt, *current_format, first_format_char;
  size_t needed_buffer_size;
  void *save_buffer;
  ArgparseState state;

  argparseInitStaticVariables();

  /* copy format string since it is modified during the parsing process */
  fmt = gks_strdup(format);
  if (fmt == NULL)
    {
      debugPrintMallocError();
      return NULL;
    }

  /* get needed save_buffer size to store all parameters and allocate memory */
  needed_buffer_size = argparseCalculateNeededBufferSize(fmt, apply_padding);
  if (needed_buffer_size > 0)
    {
      save_buffer = malloc(needed_buffer_size);
      if (save_buffer == NULL)
        {
          debugPrintMallocError();
          free(fmt);
          return NULL;
        }
    }
  else
    {
      save_buffer = NULL;
    }

  /* initialize state object */
  state.vl = vl;
  state.in_buffer = buffer;
  state.apply_padding = apply_padding;
  state.data_offset = 0;
  state.save_buffer = save_buffer;
  state.next_is_array = 0;
  state.default_array_length = 1;
  state.next_array_length = -1;
  state.dataslot_count = 0;

  current_format = fmt;
  while (*current_format)
    {
      state.current_format = tolower(*current_format);
      if (state.current_format != *current_format)
        {
          state.next_is_array = 1;
        }
      argparseReadNextOption(&state, &current_format);
      state.save_buffer =
          ((char *)state.save_buffer) + argparseCalculateNeededPadding(state.save_buffer, state.current_format);
      argparse_format_to_read_callback[(unsigned char)state.current_format](&state);
      state.next_is_array = 0;
      state.next_array_length = -1;
      if (strchr(ARGS_VALID_DATA_FORMAT_SPECIFIERS, tolower(*current_format)) != NULL)
        {
          ++state.dataslot_count;
          if (state.dataslot_count == 1)
            {
              first_format_char = *current_format;
            }
        }
      ++current_format;
    }

  /* Reset the save buffer since it was shifted during the parsing process */
  state.save_buffer = save_buffer;
  if (islower(first_format_char))
    {
      if (state.dataslot_count > 1 && new_format != NULL)
        {
          *new_format = argparseConvertToArray(&state);
        }
      else if (argparse_format_has_array_terminator[(unsigned char)state.current_format])
        {
          ((void **)state.save_buffer)[state.dataslot_count] = NULL;
        }
    }

  /* cleanup */
  free(fmt);

  return state.save_buffer;
}

#define CHECK_PADDING(type)                                               \
  do                                                                      \
    {                                                                     \
      if (state->in_buffer != NULL && state->apply_padding)               \
        {                                                                 \
          ptrdiff_t needed_padding = state->data_offset % sizeof(type);   \
          state->in_buffer = ((char *)state->in_buffer) + needed_padding; \
          state->data_offset += needed_padding;                           \
        }                                                                 \
    }                                                                     \
  while (0)

#define READ_TYPE(name, type, terminate_array)                                                                 \
  void argparseRead##name(ArgparseState *state)                                                                \
  {                                                                                                            \
    size_t *size_t_typed_buffer;                                                                               \
    type *typed_buffer, **pointer_typed_buffer, *src_ptr;                                                      \
    size_t current_array_length;                                                                               \
                                                                                                               \
    if (state->next_is_array)                                                                                  \
      {                                                                                                        \
        current_array_length =                                                                                 \
            (state->next_array_length >= 0) ? state->next_array_length : state->default_array_length;          \
        size_t_typed_buffer = state->save_buffer;                                                              \
        *size_t_typed_buffer = current_array_length;                                                           \
        pointer_typed_buffer = (type **)++size_t_typed_buffer;                                                 \
        if (current_array_length + (terminate_array ? 1 : 0) > 0)                                              \
          {                                                                                                    \
            *pointer_typed_buffer = malloc((current_array_length + (terminate_array ? 1 : 0)) * sizeof(type)); \
          }                                                                                                    \
        else                                                                                                   \
          {                                                                                                    \
            *pointer_typed_buffer = NULL;                                                                      \
          }                                                                                                    \
        if (current_array_length > 0)                                                                          \
          {                                                                                                    \
            if (state->in_buffer != NULL)                                                                      \
              {                                                                                                \
                CHECK_PADDING(type **);                                                                        \
                src_ptr = *(type **)state->in_buffer;                                                          \
              }                                                                                                \
            else                                                                                               \
              {                                                                                                \
                src_ptr = va_arg(*state->vl, type *);                                                          \
              }                                                                                                \
            if (*pointer_typed_buffer != NULL)                                                                 \
              {                                                                                                \
                memcpy(*pointer_typed_buffer, src_ptr, current_array_length * sizeof(type));                   \
                if (terminate_array)                                                                           \
                  {                                                                                            \
                    /* cast to `type ***` instead of `type **` to ensure that `= NULL` is always a syntactical \
                     * valid statement (it wouldn't for primary types like `int` and `double`);                \
                     * for non-pointer types the following statement is never executed, so it should be fine   \
                     */                                                                                        \
                    (*(type ***)pointer_typed_buffer)[current_array_length] = NULL; /* array terminator */     \
                  }                                                                                            \
              }                                                                                                \
            else                                                                                               \
              {                                                                                                \
                debugPrintMallocError();                                                                       \
              }                                                                                                \
            if (state->in_buffer != NULL)                                                                      \
              {                                                                                                \
                state->in_buffer = ((type **)state->in_buffer) + 1;                                            \
                state->data_offset += sizeof(type *);                                                          \
              }                                                                                                \
            state->save_buffer = ++pointer_typed_buffer;                                                       \
          }                                                                                                    \
      }                                                                                                        \
    else                                                                                                       \
      {                                                                                                        \
        typed_buffer = state->save_buffer;                                                                     \
        if (state->in_buffer != NULL)                                                                          \
          {                                                                                                    \
            CHECK_PADDING(type);                                                                               \
            *typed_buffer = *((type *)state->in_buffer);                                                       \
            state->in_buffer = ((type *)state->in_buffer) + 1;                                                 \
            state->data_offset += sizeof(type);                                                                \
          }                                                                                                    \
        else                                                                                                   \
          {                                                                                                    \
            *typed_buffer = va_arg(*state->vl, type);                                                          \
          }                                                                                                    \
        state->save_buffer = ++typed_buffer;                                                                   \
      }                                                                                                        \
  }

READ_TYPE(Int, int, 0)
READ_TYPE(Double, double, 0)
READ_TYPE(GrmArgsPtrT, grm_args_ptr_t, 1)

#undef READ_TYPE


void argparseReadChar(ArgparseState *state)
{
  if (state->next_is_array)
    {
      argparseReadCharArray(state, 1);
    }
  else
    {
      char *typed_buffer = state->save_buffer;
      if (state->in_buffer != NULL)
        {
          *typed_buffer = *((char *)state->in_buffer);
          state->in_buffer = (char *)state->in_buffer + 1;
          state->data_offset += sizeof(char);
        }
      else
        {
          *typed_buffer = va_arg(*state->vl, int); /* char is promoted to int */
        }
      state->save_buffer = ++typed_buffer;
    }
}

void argparseReadString(ArgparseState *state)
{
  if (state->next_is_array)
    {
      size_t *size_t_typed_buffer;
      char ***pointer_typed_buffer;
      const char **src_ptr;
      size_t current_array_length;

      current_array_length = (state->next_array_length >= 0) ? state->next_array_length : state->default_array_length;
      if (state->in_buffer != NULL)
        {
          CHECK_PADDING(char **);
          src_ptr = *(const char ***)state->in_buffer;
        }
      else
        {
          src_ptr = va_arg(*state->vl, const char **);
        }
      size_t_typed_buffer = state->save_buffer;
      *size_t_typed_buffer = current_array_length;
      pointer_typed_buffer = (char ***)++size_t_typed_buffer;
      *pointer_typed_buffer = malloc((current_array_length + 1) * sizeof(char *));
      if (*pointer_typed_buffer != NULL)
        {
          int found_malloc_fail;
          unsigned int i;
          for (i = 0; i < current_array_length; i++)
            {
              (*pointer_typed_buffer)[i] = malloc(strlen(src_ptr[i]) + 1);
            }
          found_malloc_fail = 0;
          for (i = 0; i < current_array_length && !found_malloc_fail; i++)
            {
              if ((*pointer_typed_buffer)[i] == NULL)
                {
                  found_malloc_fail = 1;
                }
            }
          if (!found_malloc_fail)
            {
              for (i = 0; i < current_array_length; i++)
                {
                  size_t current_string_length;
                  current_string_length = strlen(src_ptr[i]);
                  memcpy((*pointer_typed_buffer)[i], src_ptr[i], current_string_length);
                  (*pointer_typed_buffer)[i][current_string_length] = '\0';
                }
              (*pointer_typed_buffer)[current_array_length] = NULL; /* array terminator */
            }
          else
            {
              for (i = 0; i < current_array_length; i++)
                {
                  free((*pointer_typed_buffer)[i]);
                }
              free(*pointer_typed_buffer);
              debugPrintMallocError();
            }
        }
      else
        {
          debugPrintMallocError();
        }
      if (state->in_buffer != NULL)
        {
          state->in_buffer = ((char ***)state->in_buffer) + 1;
          state->data_offset += sizeof(char **);
        }
      state->save_buffer = ++pointer_typed_buffer;
    }
  else
    {
      argparseReadCharArray(state, 0);
    }
}

void argparseReadDefaultArrayLength(ArgparseState *state)
{
  if (state->in_buffer != NULL)
    {
      const size_t *typed_buffer = state->in_buffer;
      CHECK_PADDING(size_t);
      state->default_array_length = *typed_buffer;
      state->in_buffer = typed_buffer + 1;
      state->data_offset += sizeof(size_t);
    }
  else
    {
      state->default_array_length = va_arg(*state->vl, int);
    }
}

/* helper function */
void argparseReadCharArray(ArgparseState *state, int store_array_length)
{
  char **pointer_typed_buffer;
  const char *src_ptr;
  size_t current_array_length;

  if (state->in_buffer != NULL)
    {
      CHECK_PADDING(char *);
      src_ptr = *(char **)state->in_buffer;
    }
  else
    {
      src_ptr = va_arg(*state->vl, char *);
    }
  current_array_length = (state->next_array_length >= 0) ? state->next_array_length : (int)strlen(src_ptr);
  if (store_array_length)
    {
      size_t *size_t_typed_buffer = state->save_buffer;
      *size_t_typed_buffer = current_array_length;
      pointer_typed_buffer = (char **)++size_t_typed_buffer;
    }
  else
    {
      pointer_typed_buffer = (char **)state->save_buffer;
    }
  *pointer_typed_buffer = malloc(current_array_length + 1);
  if (*pointer_typed_buffer != NULL)
    {
      memcpy(*pointer_typed_buffer, src_ptr, current_array_length);
      (*pointer_typed_buffer)[current_array_length] = '\0';
    }
  else
    {
      debugPrintMallocError();
    }
  if (state->in_buffer != NULL)
    {
      state->in_buffer = ((char **)state->in_buffer) + 1;
      state->data_offset += sizeof(char *);
    }
  state->save_buffer = ++pointer_typed_buffer;
}

#undef CHECK_PADDING

void argparseInitStaticVariables()
{
  if (!argparse_static_variables_initialized)
    {
      argparse_valid_format['n'] = 1;
      argparse_valid_format['i'] = 1;
      argparse_valid_format['I'] = 1;
      argparse_valid_format['d'] = 1;
      argparse_valid_format['D'] = 1;
      argparse_valid_format['c'] = 1;
      argparse_valid_format['C'] = 1;
      argparse_valid_format['s'] = 1;
      argparse_valid_format['S'] = 1;
      argparse_valid_format['a'] = 1;
      argparse_valid_format['A'] = 1;

      argparse_format_to_read_callback['i'] = argparseReadInt;
      argparse_format_to_read_callback['d'] = argparseReadDouble;
      argparse_format_to_read_callback['c'] = argparseReadChar;
      argparse_format_to_read_callback['s'] = argparseReadString;
      argparse_format_to_read_callback['a'] = argparseReadGrmArgsPtrT;
      argparse_format_to_read_callback['n'] = argparseReadDefaultArrayLength;

      argparse_format_to_copy_callback['s'] = (CopyValue)gks_strdup;
      argparse_format_to_copy_callback['a'] = (CopyValue)argsCopy;

      argparse_format_to_delete_callback['s'] = free;
      argparse_format_to_delete_callback['a'] = (DeleteValue)grm_args_delete;

      argparse_format_to_size['i'] = sizeof(int);
      argparse_format_to_size['I'] = sizeof(int *);
      argparse_format_to_size['d'] = sizeof(double);
      argparse_format_to_size['D'] = sizeof(double *);
      argparse_format_to_size['c'] = sizeof(char);
      argparse_format_to_size['C'] = sizeof(char *);
      argparse_format_to_size['s'] = sizeof(char *);
      argparse_format_to_size['S'] = sizeof(char **);
      argparse_format_to_size['a'] = sizeof(grm_args_t *);
      argparse_format_to_size['A'] = sizeof(grm_args_t **);
      argparse_format_to_size['n'] = 0;              /* size for array length is reserved by an array call itself */
      argparse_format_to_size['#'] = sizeof(size_t); /* only used internally */

      argparse_format_has_array_terminator['s'] = 1;
      argparse_format_has_array_terminator['a'] = 1;

      argparse_static_variables_initialized = 1;
    }
}

size_t argparseCalculateNeededBufferSize(const char *format, int apply_padding)
{
  size_t needed_size;
  size_t size_for_current_specifier;
  int is_array;

  needed_size = 0;
  is_array = 0;
  if (argparse_format_has_array_terminator[(unsigned char)*format])
    {
      /* Add size for a NULL pointer terminator in the buffer itself because it will be converted to an array buffer
       * later (-> see `argparse_convert_to_array`) */
      size_for_current_specifier = argparse_format_to_size[(unsigned char)*format];
      needed_size = size_for_current_specifier;
    }
  while (*format)
    {
      char current_format;
      if (*format == '(')
        {
          format = argparseSkipOption(format);
          if (!*format)
            {
              break;
            }
        }
      if (tolower(*format) != *format)
        {
          is_array = 1;
        }
      current_format = *format;
      while (current_format)
        {
          size_for_current_specifier = argparse_format_to_size[(unsigned char)current_format];
          if (apply_padding)
            {
              /* apply needed padding for memory alignment first */
              needed_size += argparseCalculateNeededPadding((void *)needed_size, current_format);
            }
          /* then add the actual needed memory size */
          needed_size += size_for_current_specifier;
          if (is_array)
            {
              current_format = '#';
              is_array = 0;
            }
          else
            {
              current_format = '\0';
            }
        }
      ++format;
    }

  return needed_size;
}

size_t argparseCalculateNeededPadding(void *buffer, char current_format)
{
  int size_for_current_specifier;
  int needed_padding;

  size_for_current_specifier = argparse_format_to_size[(unsigned char)current_format];
  if (size_for_current_specifier > 0)
    {
      needed_padding = size_for_current_specifier - ((char *)buffer - (char *)0) % size_for_current_specifier;
      if (needed_padding == size_for_current_specifier)
        {
          needed_padding = 0;
        }
    }
  else
    {
      needed_padding = 0;
    }

  return needed_padding;
}

void argparseReadNextOption(ArgparseState *state, char **format)
{
  char *fmt = *format;
  unsigned int next_array_length;
  char *current_char;

  ++fmt;
  if (*fmt != '(')
    {
      return; /* there is no option that could be read */
    }

  current_char = ++fmt;
  while (*current_char && *current_char != ')')
    {
      ++current_char;
    }
  if (!*current_char)
    {
      debugPrintError(("Option \"%s\" in format string \"%s\" is not terminated -> ignore it.\n", fmt, *format));
      return;
    }
  *current_char = '\0';

  if (!strToUint(fmt, &next_array_length))
    {
      debugPrintError(
          ("Option \"%s\" in format string \"%s\" could not be converted to a number -> ignore it.\n", fmt, *format));
      return;
    }

  state->next_array_length = next_array_length;
  *format = current_char;
}

const char *argparseSkipOption(const char *format)
{
  if (*format != '(')
    {
      return format;
    }
  while (*format && *format != ')')
    {
      ++format;
    }
  if (*format)
    {
      ++format;
    }
  return format;
}

char *argparseConvertToArray(ArgparseState *state)
{
  void *new_save_buffer = NULL;
  size_t *size_t_typed_buffer;
  void ***general_typed_buffer;
  char *new_format = NULL;

  new_save_buffer = malloc(sizeof(size_t) + sizeof(void *));
  if (new_save_buffer == NULL)
    {
      goto cleanup;
    }
  size_t_typed_buffer = new_save_buffer;
  *size_t_typed_buffer = state->dataslot_count;
  general_typed_buffer = (void ***)(size_t_typed_buffer + 1);
  *general_typed_buffer = state->save_buffer;
  if (argparse_format_has_array_terminator[(unsigned char)state->current_format])
    {
      (*general_typed_buffer)[*size_t_typed_buffer] = NULL;
    }
  state->save_buffer = new_save_buffer;
  new_format = malloc(2 * sizeof(char));
  new_format[0] = toupper(state->current_format);
  new_format[1] = '\0';
  if (new_format == NULL)
    {
      goto cleanup;
    }
  return new_format;

cleanup:
  free(new_save_buffer);
  free(new_format);
  debugPrintMallocError();
  return NULL;
}


/* ------------------------- argument container --------------------------------------------------------------------- */

grm_arg_t *argsCreateArgs(const char *key, const char *value_format, const void *buffer, va_list *vl, int apply_padding)
{
  grm_arg_t *arg;
  char *parsing_format;
  char *new_format = NULL;

  if (!argsValidateFormatString(value_format))
    {
      return NULL;
    }

  arg = malloc(sizeof(grm_arg_t));
  if (arg == NULL)
    {
      debugPrintMallocError();
      return NULL;
    }
  if (key != NULL)
    {
      arg->key = gks_strdup(key);
      if (arg->key == NULL)
        {
          debugPrintMallocError();
          free(arg);
          return NULL;
        }
    }
  else
    {
      arg->key = NULL;
    }
  arg->value_format = malloc(2 * strlen(value_format) + 1);
  if (arg->value_format == NULL)
    {
      debugPrintMallocError();
      free((char *)arg->key);
      free(arg);
      return NULL;
    }
  parsing_format = malloc(strlen(value_format) + 1);
  if (parsing_format == NULL)
    {
      debugPrintMallocError();
      free((char *)arg->key);
      free((char *)arg->value_format);
      free(arg);
      return NULL;
    }
  argsCopyFormatStringForParsing(parsing_format, value_format);
  arg->value_ptr = argparseReadParams(parsing_format, buffer, vl, apply_padding, &new_format);
  if (new_format == NULL)
    {
      argsCopyFormatStringForArg((char *)arg->value_format, value_format);
    }
  else
    {
      argsCopyFormatStringForArg((char *)arg->value_format, new_format);
      free(new_format);
    }
  free(parsing_format);
  arg->priv = malloc(sizeof(grm_arg_private_t));
  if (arg->priv == NULL)
    {
      debugPrintMallocError();
      free((char *)arg->key);
      free((char *)arg->value_format);
      free(arg);
      return NULL;
    }
  arg->priv->reference_count = 1;

  return arg;
}

int argsValidateFormatString(const char *format)
{
  char *fmt;
  char *first_format_char;
  char *previous_char;
  char *current_char;
  char *option_start;
  int is_valid;

  if (format == NULL)
    {
      return 0;
    }
  fmt = gks_strdup(format);
  if (fmt == NULL)
    {
      debugPrintMallocError();
      return 0;
    }

  first_format_char = NULL;
  previous_char = NULL;
  current_char = fmt;
  is_valid = 1;
  while (*current_char && is_valid)
    {
      if (*current_char == '(')
        {
          if (previous_char != NULL)
            {
              if (strchr(ARGS_VALID_DATA_FORMAT_SPECIFIERS, tolower(*previous_char)) != NULL)
                {
                  previous_char = current_char;
                  option_start = ++current_char;
                  while (*current_char && *current_char != ')')
                    {
                      ++current_char;
                    }
                  if (*current_char)
                    {
                      *current_char = '\0';
                      is_valid = strToUint(option_start, NULL);
                      if (!is_valid)
                        {
                          debugPrintError(("The option \"%s\" in the format string \"%s\" in no valid number.\n",
                                           option_start, format));
                        }
                    }
                  else
                    {
                      is_valid = 0;
                      --current_char;
                      debugPrintError(
                          ("Option \"%s\" in the format string \"%s\" is not terminated.\n", option_start, format));
                    }
                }
              else
                {
                  is_valid = 0;
                  debugPrintError(("Specifier '%c' in the format string \"%s\" cannot have any options.\n",
                                   *previous_char, format));
                }
            }
          else
            {
              is_valid = 0;
              debugPrintError(
                  ("The format string \"%s\" is invalid: Format strings must not start with an option.\n", format));
            }
        }
      else
        {
          if (strchr(ARGS_VALID_FORMAT_SPECIFIERS, *current_char) == NULL)
            {
              is_valid = 0;
              debugPrintError(("Invalid specifier '%c' in the format string \"%s\".\n", *current_char, format));
            }
          else if (strchr(ARGS_VALID_DATA_FORMAT_SPECIFIERS, *current_char) != NULL)
            {
              if (first_format_char != NULL && *current_char != *first_format_char)
                {
                  is_valid = 0;
                  debugPrintError(
                      ("The format string \"%s\" consists of different types which is not allowed.\n", format));
                }
              if (first_format_char == NULL)
                {
                  first_format_char = current_char;
                }
            }
          previous_char = current_char;
        }
      ++current_char;
    }

  free(fmt);

  return is_valid;
}

const char *argsSkipOption(const char *format)
{
  if (*format != '(')
    {
      return format;
    }
  while (*format && *format != ')')
    {
      ++format;
    }
  if (*format)
    {
      ++format;
    }
  return format;
}

void argsCopyFormatStringForParsing(char *dst, const char *format)
{
  while (*format)
    {
      if (*format == 'C')
        {
          /* char arrays and strings are the same -> store them as strings for unified data handling */
          *dst++ = 's';
          /* skip an optional array length since strings have no array length */
          ++format;
          format = argsSkipOption(format);
        }
      else
        {
          *dst++ = *format++;
        }
    }
  *dst = '\0';
}

void argsCopyFormatStringForArg(char *dst, const char *format)
{
  /* `dst` should have twice as much memory as `format` to ensure that no buffer overun can occur */
  while (*format)
    {
      if (*format == 'n')
        {
          /* Skip `n` since it is added for arrays anyway when needed */
          ++format;
          continue;
        }
      if (*format == 'C')
        {
          /* char arrays and strings are the same -> store them as strings for unified data handling */
          *dst++ = 's';
          ++format;
        }
      else
        {
          if (isupper(*format))
            {
              /* all array formats get an internal size value */
              *dst++ = 'n';
            }
          *dst++ = *format++;
        }
      /* Skip an optional array length since it already saved in the argument buffer itself (-> `n` format) */
      format = argsSkipOption(format);
    }
  *dst = '\0';
}

int argsCheckFormatCompatibility(const grm_arg_t *arg, const char *compatible_format)
{
  char first_compatible_format_char, first_value_format_char = 0;
  const char *current_format_ptr;
  char *compatible_format_for_arg;
  size_t dataslot_count, len_compatible_format;

  /* First, check if the compatible format itself is valid (-> known format, homogeneous, no options) */
  first_compatible_format_char = *compatible_format;
  if (strchr(ARGS_VALID_DATA_FORMAT_SPECIFIERS, tolower(first_compatible_format_char)) == NULL) return 0;
  current_format_ptr = compatible_format;
  while (*current_format_ptr != '\0')
    {
      if (*current_format_ptr != first_compatible_format_char) return 0;
      ++current_format_ptr;
    }
  len_compatible_format = current_format_ptr - compatible_format;

  /* Second, check if original and compatible format are identical */
  /* within an argument, formats are stored **with** array length slots, so we need `nD` instead of `D` for example
   * -> convert the format before comparison */
  compatible_format_for_arg = malloc(2 * strlen(compatible_format) + 1);
  if (compatible_format_for_arg == NULL)
    {
      debugPrintMallocError();
      return 0;
    }
  argsCopyFormatStringForArg(compatible_format_for_arg, compatible_format);
  if (strcmp(arg->value_format, compatible_format_for_arg) == 0)
    {
      free(compatible_format_for_arg);
      return 2;
    }
  free(compatible_format_for_arg);

  /* Otherwise, check if the format is compatible */
  /* Compatibility is only possible if a single scalar or a single array is stored in the argument
   * -> the original format string (ignoring `n`!) must not be longer than 1 */
  dataslot_count = 0;
  current_format_ptr = arg->value_format;
  while (*current_format_ptr != '\0' && dataslot_count < 2)
    {
      if (strchr(ARGS_VALID_DATA_FORMAT_SPECIFIERS, tolower(*current_format_ptr)) != NULL)
        {
          ++dataslot_count;
          if (dataslot_count == 1) first_value_format_char = *current_format_ptr;
        }
      ++current_format_ptr;
    }
  if (dataslot_count > 1) return 0;
  /* Check if the first format types are identical (scalar and arrays of same type are considered compatible) */
  if (tolower(first_value_format_char) != tolower(first_compatible_format_char)) return 0;
  /* Check if the stored value is a scalar */
  if (first_value_format_char == tolower(first_value_format_char))
    {
      /* Check if the compatible format has the length 1 since single scalar values can only be converted to one array
       */
      if (len_compatible_format != 1) return 0;
    }
  /* Otherwise, it must be an array */
  else
    {
      /* Check if the compatible format is not longer than the stored array */
      if (len_compatible_format > *(size_t *)arg->value_ptr) return 0;
    }

  return 1;
}

void argsDecreaseArgReferenceCount(ArgsNode *args_node)
{
  if (--(args_node->arg->priv->reference_count) == 0)
    {
      grm_args_value_iterator_t *value_it = grm_arg_value_iter(args_node->arg);
      while (value_it->next(value_it) != NULL)
        {
          /* use a char pointer since chars have no memory alignment restrictions */
          if (value_it->is_array)
            {
              if (argparse_format_to_delete_callback[(int)value_it->format] != NULL)
                {
                  char **current_value_ptr = *(char ***)value_it->value_ptr;
                  while (*current_value_ptr != NULL)
                    {
                      argparse_format_to_delete_callback[(int)value_it->format](*current_value_ptr);
                      /* cast to (char *) to allow pointer increment by bytes */
                      current_value_ptr =
                          (char **)((char *)current_value_ptr + argparse_format_to_size[(int)value_it->format]);
                    }
                }
              free(*(char ***)value_it->value_ptr);
            }
          else if (argparse_format_to_delete_callback[(int)value_it->format] != NULL)
            {
              argparse_format_to_delete_callback[(int)value_it->format](*(char **)value_it->value_ptr);
            }
        }
      argsValueIteratorDelete(value_it);
      free((char *)args_node->arg->key);
      free((char *)args_node->arg->value_format);
      free(args_node->arg->priv);
      free(args_node->arg->value_ptr);
      free(args_node->arg);
    }
}


/* ------------------------- value copy ----------------------------------------------------------------------------- */

void *copyValue(char format, void *value_ptr)
{
  void *copy;

  if (!argparse_valid_format[(int)format] || !argparse_format_to_size[(int)format])
    {
      debugPrintError(("The format '%c' is unsupported.\n", format));
      return NULL;
    }
  if (tolower(format) != format)
    {
      debugPrintError(("Array formats are not supported in the function `copy_value`.\n"));
      return NULL;
    }

  copy = malloc(argparse_format_to_size[(int)format]);
  if (copy == NULL)
    {
      debugPrintMallocError();
      return NULL;
    }

  /* check if it is a complex or plain datatype */
  if (argparse_format_to_copy_callback[(int)format])
    {
      /* complex datatypes like argument containers or strings need a copy routine */
      *(void **)copy = argparse_format_to_copy_callback[(int)format](*(void **)value_ptr);
    }
  else
    {
      /* plain datatypes like int, double, char can be copied directly */
      memcpy(copy, value_ptr, argparse_format_to_size[(int)format]);
    }

  return copy;
}


/* ========================= methods ================================================================================ */

/* ------------------------- argument ------------------------------------------------------------------------------- */

grm_args_value_iterator_t *grm_arg_value_iter(const grm_arg_t *arg)
{
  return argsValueIteratorNew(arg);
}

grm_error_t argIncreaseArray(grm_arg_t *arg, size_t increment)
{
  size_t *current_size_ptr, new_size;
  void ***current_buffer_ptr, **new_buffer;
  int has_array_terminator;

  returnErrorIf(arg->value_format[0] != 'n', GRM_ERROR_ARGS_INCREASING_NON_ARRAY_VALUE);
  /* Currently, only one dimensional arrays can be increased */
  returnErrorIf(strlen(arg->value_format) != 2, GRM_ERROR_ARGS_INCREASING_MULTI_DIMENSIONAL_ARRAY);

  has_array_terminator = argparse_format_has_array_terminator[tolower(arg->value_format[1])];

  current_size_ptr = (size_t *)arg->value_ptr;
  current_buffer_ptr = (void ***)(current_size_ptr + 1);

  new_size = *current_size_ptr + increment;
  new_buffer = realloc(*current_buffer_ptr, sizeof(void *) * (new_size + (has_array_terminator ? 1 : 0)));
  returnErrorIf(new_buffer == NULL, GRM_ERROR_MALLOC);

  if (has_array_terminator)
    {
      unsigned int i;
      for (i = *current_size_ptr + 1; i < new_size + 1; ++i)
        {
          new_buffer[i] = NULL;
        }
    }

  *current_size_ptr = new_size;
  *current_buffer_ptr = new_buffer;

  return GRM_ERROR_NONE;
}

int(argFirstValue)(const grm_arg_t *arg, const char *first_value_format, void *first_value, unsigned int *array_length)
{
  char *transformed_first_value_format = NULL;
  size_t transformed_first_value_format_length;
  int array_requested;
  char first_value_type;
  void *value_ptr;
  size_t *size_t_typed_value_ptr;
  int was_successful = 0;

  transformed_first_value_format = malloc(2 * strlen(first_value_format) + 1);
  if (transformed_first_value_format == NULL)
    {
      debugPrintMallocError();
      goto cleanup;
    }
  argsCopyFormatStringForArg(transformed_first_value_format, first_value_format);
  transformed_first_value_format_length = strlen(transformed_first_value_format);
  array_requested = (transformed_first_value_format_length == 2 && transformed_first_value_format[0] == 'n');
  /* if value_format does not start with the transformed first_value_format, the value cannot be read, so return here */
  if (strncmp(arg->value_format, transformed_first_value_format, transformed_first_value_format_length) != 0)
    {
      /* One exception: If the stored value format is a scalar value (e.g. `i`) and an array of same type shall be read
       * (in this case `nI`), then allow this (will return a pointer to the internal buffer to emulate an array) */
      cleanupIf(!(array_requested && strlen(arg->value_format) == 1 &&
                  arg->value_format[0] == tolower(transformed_first_value_format[1])));
    }
  first_value_type = (arg->value_format[0] != 'n') ? arg->value_format[0] : arg->value_format[1];
  if (islower(first_value_type))
    {
      value_ptr = arg->value_ptr;
      if (array_length != NULL)
        {
          *array_length = 1;
        }
    }
  else
    {
      size_t_typed_value_ptr = arg->value_ptr;
      if (array_length != NULL)
        {
          *array_length = *size_t_typed_value_ptr;
        }
      value_ptr = (size_t_typed_value_ptr + 1);
    }
  if (first_value != NULL)
    {
      if (isupper(first_value_type))
        {
          /* if the first value is an array simply store the pointer; the type the pointer is pointing to is unimportant
           * in this case so use a void pointer conversion to shorten the code */
          *(void **)first_value = *(void **)value_ptr;
        }
      else if (array_requested)
        {
          /* if the first value is a scalar but an array is requested simply store the pointer to the internal buffer */
          *(void **)first_value = value_ptr;
        }
      else
        {
          switch (first_value_type)
            {
            case 'i':
              *(int *)first_value = *(int *)value_ptr;
              break;
            case 'd':
              *(double *)first_value = *(double *)value_ptr;
              break;
            case 'c':
              *(char *)first_value = *(char *)value_ptr;
              break;
            case 's':
              *(char **)first_value = *(char **)value_ptr;
              break;
            case 'a':
              *(grm_args_t **)first_value = *(grm_args_t **)value_ptr;
              break;
            default:
              goto cleanup;
            }
        }
    }
  was_successful = 1;

cleanup:
  free(transformed_first_value_format);

  return was_successful;
}

int argValues(const grm_arg_t *arg, const char *expected_format, ...)
{
  va_list vl;
  int was_successful;

  va_start(vl, expected_format);

  was_successful = argValuesVl(arg, expected_format, &vl);

  va_end(vl);

  return was_successful;
}

int argValuesVl(const grm_arg_t *arg, const char *expected_format, va_list *vl)
{
  grm_args_value_iterator_t *value_it = NULL;
  const char *current_va_format;
  int formats_are_equal = 0;
  int data_offset = 0;
  int was_successful = 0;

  if (!(formats_are_equal = argsCheckFormatCompatibility(arg, expected_format)))
    {
      goto cleanup;
    }
  formats_are_equal = (formats_are_equal == 2);

  current_va_format = expected_format;
  value_it = grm_arg_value_iter(arg);
  if (value_it->next(value_it) == NULL)
    {
      goto cleanup;
    }
  while (*current_va_format != '\0')
    {
      void *current_value_ptr;
      current_value_ptr = va_arg(*vl, void *);
      if (isupper(*current_va_format))
        {
          if (value_it->is_array)
            {
              /* If an array is stored and an array format is given by the user, simply assign a pointer to the data.
               * The datatype itself is unimportant in this case. */
              *(void **)current_value_ptr = *(void **)value_it->value_ptr;
            }
          else
            {
              /* Otherwise, a scalar is stored but an array format given. Reuse the internal buffer as an array in this
               * case. */
              *(void **)current_value_ptr = value_it->value_ptr;
            }
        }
      else
        {
          switch (value_it->format)
            {
            case 'i':
              if (value_it->is_array)
                {
                  /* The data is stored as an array but the user wants to assign the values to single variables (the
                   * array case was handled before by the void pointer). -> Assign value by value by incrementing a data
                   * offset in each step. */
                  *(int *)current_value_ptr = (*(int **)value_it->value_ptr)[data_offset++];
                }
              else
                {
                  /* The data is stored as a single value. Assign that value to the variable given by the user. */
                  *(int *)current_value_ptr = *(int *)value_it->value_ptr;
                }
              break;
            case 'd':
              if (value_it->is_array)
                {
                  *(double *)current_value_ptr = (*(double **)value_it->value_ptr)[data_offset++];
                }
              else
                {
                  *(double *)current_value_ptr = *(double *)value_it->value_ptr;
                }
              break;
            case 'c':
              if (value_it->is_array)
                {
                  *(char *)current_value_ptr = (*(char **)value_it->value_ptr)[data_offset++];
                }
              else
                {
                  *(char *)current_value_ptr = *(char *)value_it->value_ptr;
                }
              break;
            case 's':
              if (value_it->is_array)
                {
                  *(char **)current_value_ptr = (*(char ***)value_it->value_ptr)[data_offset++];
                }
              else
                {
                  *(char **)current_value_ptr = *(char **)value_it->value_ptr;
                }
              break;
            case 'a':
              if (value_it->is_array)
                {
                  *(grm_args_t **)current_value_ptr = (*(grm_args_t ***)value_it->value_ptr)[data_offset++];
                }
              else
                {
                  *(grm_args_t **)current_value_ptr = *(grm_args_t **)value_it->value_ptr;
                }
              break;
            default:
              goto cleanup;
            }
        }
      if (formats_are_equal)
        {
          /* Only iterate if the format given by the user is equal to the stored internal format. In the other case, the
           * user requests to store **one** array to single variables -> values are read from one pointer, no iteration
           * is needed. */
          value_it->next(value_it);
          data_offset = 0;
        }
      ++current_va_format;
    }
  was_successful = 1;

cleanup:
  if (value_it != NULL)
    {
      argsValueIteratorDelete(value_it);
    }

  return was_successful;
}


/* ------------------------- argument container --------------------------------------------------------------------- */

void argsInit(grm_args_t *args)
{
  args->kwargs_head = NULL;
  args->kwargs_tail = NULL;
  args->count = 0;
}

void argsFinalize(grm_args_t *args)
{
  argsClear(args, NULL);
}

grm_args_t *argsFlatCopy(const grm_args_t *copy_args)
{
  /* Clone the linked list but share the referenced values */
  grm_args_t *args = NULL;
  grm_args_iterator_t *it = NULL;
  ArgsNode *args_node;
  grm_arg_t *copy_arg;

  args = grm_args_new();
  if (args == NULL)
    {
      debugPrintMallocError();
      goto error_cleanup;
    }
  it = grm_args_iter(copy_args);
  while ((copy_arg = it->next(it)) != NULL)
    {
      ++(copy_arg->priv->reference_count);
      args_node = malloc(sizeof(ArgsNode));
      if (args_node == NULL)
        {
          debugPrintMallocError();
          goto error_cleanup;
        }
      args_node->arg = copy_arg;
      args_node->next = NULL;

      if (args->kwargs_head == NULL)
        {
          args->kwargs_head = args_node;
        }
      else
        {
          args->kwargs_tail->next = args_node;
        }
      args->kwargs_tail = args_node;
      ++(args->count);
    }
  argsIteratorDelete(it);

  return args;

error_cleanup:
  if (args != NULL)
    {
      grm_args_delete(args);
    }
  if (it != NULL)
    {
      argsIteratorDelete(it);
    }

  return NULL;
}

grm_args_t *argsCopy(const grm_args_t *copy_args)
{
  return argsCopyExtended(copy_args, NULL, NULL);
}

grm_args_t *argsCopyExtended(const grm_args_t *copy_args, const char **keys_copy_as_array, const char **ignore_keys)
{
  /* Clone the linked list and all values that are argument containers as well. Share all other values (-> **no deep
   * copy!**).
   * `keys_copy_as_array` can be used to always copy values of the specified keys as an array. It is only read for
   * values which are argument containers. The array must be terminated with a NULL pointer.
   * `ignore_keys` is an array of keys which will not be copied. The array must be terminated with a NULL pointer. */
  grm_args_t *args = NULL, **args_array = NULL, *copied_args = NULL, **copied_args_array = NULL,
             **current_args_copy = NULL;
  grm_args_iterator_t *it = NULL;
  grm_args_value_iterator_t *value_it = NULL;
  ArgsNode *args_node;
  grm_arg_t *copy_arg;

  args = grm_args_new();
  if (args == NULL)
    {
      debugPrintMallocError();
      goto error_cleanup;
    }
  it = grm_args_iter(copy_args);
  errorCleanupIf(it == NULL);
  while ((copy_arg = it->next(it)) != NULL)
    {
      if (ignore_keys != NULL && strEqualsAnyInArray(copy_arg->key, ignore_keys))
        {
          continue;
        }
      if (strncmp(copy_arg->value_format, "a", 1) == 0 || strncmp(copy_arg->value_format, "nA", 2) == 0)
        {
          value_it = grm_arg_value_iter(copy_arg);
          errorCleanupIf(value_it == NULL);
          /* Do not support two dimensional argument arrays like `nAnA`) -> a loop would be needed with more memory
           * management */
          errorCleanupIf(value_it->next(value_it) == NULL);
          if (value_it->is_array)
            {
              args_array = *(grm_args_t ***)value_it->value_ptr;
              copied_args_array = malloc(value_it->array_length * sizeof(grm_args_t *));
              errorCleanupIf(copied_args_array == NULL);
              current_args_copy = copied_args_array;
              while (*args_array != NULL)
                {
                  *current_args_copy = argsCopyExtended(*args_array, keys_copy_as_array, ignore_keys);
                  errorCleanupIf(*current_args_copy == NULL);
                  ++args_array;
                  ++current_args_copy;
                }
              current_args_copy = NULL;
              grm_args_push(args, it->arg->key, "nA", value_it->array_length, copied_args_array);
            }
          else
            {
              copied_args = argsCopyExtended(*(grm_args_t **)value_it->value_ptr, keys_copy_as_array, ignore_keys);
              errorCleanupIf(copied_args == NULL);
              if (keys_copy_as_array != NULL && strEqualsAnyInArray(it->arg->key, keys_copy_as_array))
                {
                  grm_args_push(args, it->arg->key, "A(1)", &copied_args);
                }
              else
                {
                  grm_args_push(args, it->arg->key, "a", copied_args);
                }
              copied_args = NULL;
            }
        }
      else
        {
          ++(copy_arg->priv->reference_count);
          args_node = malloc(sizeof(ArgsNode));
          if (args_node == NULL)
            {
              debugPrintMallocError();
              goto error_cleanup;
            }
          args_node->arg = copy_arg;
          args_node->next = NULL;

          if (args->kwargs_head == NULL)
            {
              args->kwargs_head = args_node;
            }
          else
            {
              args->kwargs_tail->next = args_node;
            }
          args->kwargs_tail = args_node;
          ++(args->count);
        }
    }
  goto cleanup;

error_cleanup:
  if (args != NULL)
    {
      grm_args_delete(args);
      args = NULL;
    }
cleanup:
  if (current_args_copy != NULL)
    {
      while (current_args_copy != copied_args_array)
        {
          if (*current_args_copy != NULL)
            {
              grm_args_delete(*current_args_copy);
            }
          --current_args_copy;
        }
    }
  if (copied_args_array != NULL)
    {
      free(copied_args_array);
    }
  if (it != NULL)
    {
      argsIteratorDelete(it);
    }
  if (value_it != NULL)
    {
      argsValueIteratorDelete(value_it);
    }

  return args;
}

grm_error_t argsPushCommon(grm_args_t *args, const char *key, const char *value_format, const void *buffer, va_list *vl,
                           int apply_padding)
{
  grm_arg_t *arg;
  ArgsNode *args_node;

  if ((arg = argsCreateArgs(key, value_format, buffer, vl, apply_padding)) == NULL) return GRM_ERROR_MALLOC;

  if ((args_node = argsFindNode(args, key)) != NULL)
    {
      argsDecreaseArgReferenceCount(args_node);
      args_node->arg = arg;
    }
  else
    {
      args_node = malloc(sizeof(ArgsNode));
      if (args_node == NULL)
        {
          debugPrintMallocError();
          free((char *)arg->key);
          free((char *)arg->value_format);
          free(arg->priv);
          free(arg);
          return GRM_ERROR_MALLOC;
        }
      args_node->arg = arg;
      args_node->next = NULL;
      if (args->kwargs_head == NULL)
        {
          args->kwargs_head = args_node;
          args->kwargs_tail = args_node;
        }
      else
        {
          args->kwargs_tail->next = args_node;
          args->kwargs_tail = args_node;
        }
      ++(args->count);
    }

  return GRM_ERROR_NONE;
}

grm_error_t argsPushVl(grm_args_t *args, const char *key, const char *value_format, va_list *vl)
{
  return argsPushCommon(args, key, value_format, NULL, vl, 0);
}

grm_error_t argsPushArg(grm_args_t *args, grm_arg_t *arg)
{
  ArgsNode *args_node = NULL, *previous_node_by_keyword = NULL;
  grm_error_t error = GRM_ERROR_NONE;

  ++(arg->priv->reference_count);
  args_node = malloc(sizeof(ArgsNode));
  errorCleanupAndSetErrorIf(args_node == NULL, GRM_ERROR_MALLOC);
  args_node->arg = arg;
  args_node->next = NULL;

  if (args->kwargs_head == NULL)
    {
      args->kwargs_head = args_node;
      args->kwargs_tail = args_node;
      ++(args->count);
    }
  else if (argsFindPreviousNode(args, arg->key, &previous_node_by_keyword))
    {
      if (previous_node_by_keyword == NULL)
        {
          args_node->next = args->kwargs_head->next;
          if (args->kwargs_head == args->kwargs_tail) args->kwargs_tail = args_node;
          argsDecreaseArgReferenceCount(args->kwargs_head);
          free(args->kwargs_head);
          args->kwargs_head = args_node;
        }
      else
        {
          args_node->next = previous_node_by_keyword->next->next;
          argsDecreaseArgReferenceCount(previous_node_by_keyword->next);
          free(previous_node_by_keyword->next);
          previous_node_by_keyword->next = args_node;
          if (args_node->next == NULL) args->kwargs_tail = args_node;
        }
    }
  else
    {
      args->kwargs_tail->next = args_node;
      args->kwargs_tail = args_node;
      ++(args->count);
    }

  return GRM_ERROR_NONE;

error_cleanup:
  if (args_node != NULL) free(args_node);
  return error;
}

grm_error_t argsUpdateMany(grm_args_t *args, const grm_args_t *update_args)
{
  return argsMerge(args, update_args, NULL);
}

grm_error_t argsMerge(grm_args_t *args, const grm_args_t *merge_args, const char *const *merge_keys)
{
  grm_args_iterator_t *it = NULL;
  grm_args_value_iterator_t *value_it = NULL, *merge_value_it = NULL;
  grm_arg_t *update_arg, *current_arg;
  grm_args_t **args_array, **merge_args_array;
  const char *const *current_key_ptr;
  int merge;
  unsigned int i;
  grm_error_t error = GRM_ERROR_NONE;

  it = grm_args_iter(merge_args);
  cleanupAndSetErrorIf(it == NULL, GRM_ERROR_MALLOC);
  while ((update_arg = it->next(it)) != NULL)
    {
      merge = 0;
      if (merge_keys != NULL)
        {
          current_key_ptr = merge_keys;
          while (*current_key_ptr != NULL)
            {
              if (strcmp(it->arg->key, *current_key_ptr) == 0)
                {
                  merge = 1;
                  break;
                }
              ++current_key_ptr;
            }
        }
      if (merge && (current_arg = argsAt(args, update_arg->key)) != NULL)
        {
          value_it = grm_arg_value_iter(current_arg);
          merge_value_it = grm_arg_value_iter(update_arg);
          cleanupAndSetErrorIf(value_it == NULL, GRM_ERROR_MALLOC);
          cleanupAndSetErrorIf(merge_value_it == NULL, GRM_ERROR_MALLOC);
          /* Do not support two dimensional argument arrays like `nAnA`) -> a loop would be needed with more memory
           * management */
          cleanupAndSetErrorIf(value_it->next(value_it) == NULL, GRM_ERROR_MALLOC);
          cleanupAndSetErrorIf(merge_value_it->next(merge_value_it) == NULL, GRM_ERROR_MALLOC);
          if (value_it->is_array)
            {
              args_array = *(grm_args_t ***)value_it->value_ptr;
            }
          else
            {
              args_array = (grm_args_t **)value_it->value_ptr;
            }
          if (merge_value_it->is_array)
            {
              merge_args_array = *(grm_args_t ***)merge_value_it->value_ptr;
            }
          else
            {
              merge_args_array = (grm_args_t **)merge_value_it->value_ptr;
            }
          for (i = 0; i < value_it->array_length && i < merge_value_it->array_length; ++i)
            {
              error = argsMerge(args_array[i], merge_args_array[i], merge_keys);
              cleanupIfError;
            }
        }
      else
        {
          error = argsPushArg(args, update_arg);
          cleanupIfError;
        }
    }

cleanup:
  if (it != NULL) argsIteratorDelete(it);
  if (value_it != NULL) argsValueIteratorDelete(value_it);
  if (merge_value_it != NULL) argsValueIteratorDelete(merge_value_it);

  return error;
}

grm_error_t argsSetDefaultCommon(grm_args_t *args, const char *key, const char *value_format, const void *buffer,
                                 va_list *vl, int apply_padding)
{
  if (!grm_args_contains(args, key)) return argsPushCommon(args, key, value_format, buffer, vl, apply_padding);
  return GRM_ERROR_NONE;
}

grm_error_t argsSetDefault(grm_args_t *args, const char *key, const char *value_format, ...)
{
  grm_error_t error;
  va_list vl;
  va_start(vl, value_format);

  error = argsSetDefaultVl(args, key, value_format, &vl);

  va_end(vl);

  return error;
}

grm_error_t argsSetDefaultBuf(grm_args_t *args, const char *key, const char *value_format, const void *buffer,
                              int apply_padding)
{
  return argsSetDefaultCommon(args, key, value_format, buffer, NULL, apply_padding);
}

grm_error_t argsSetDefaultVl(grm_args_t *args, const char *key, const char *value_format, va_list *vl)
{
  return argsSetDefaultCommon(args, key, value_format, NULL, vl, 0);
}

void argsClear(grm_args_t *args, const char **exclude_keys)
{
  ArgsNode *current_node, *next_node, *last_excluded_node;

  current_node = args->kwargs_head;
  last_excluded_node = NULL;
  while (current_node != NULL)
    {
      next_node = current_node->next;
      if (exclude_keys != NULL && strEqualsAnyInArray(current_node->arg->key, exclude_keys))
        {
          if (last_excluded_node == NULL)
            {
              args->kwargs_head = current_node;
            }
          else
            {
              last_excluded_node->next = current_node;
            }
          last_excluded_node = current_node;
        }
      else
        {
          argsDecreaseArgReferenceCount(current_node);
          free(current_node);
          --(args->count);
        }
      current_node = next_node;
    }
  args->kwargs_tail = last_excluded_node;
  if (args->kwargs_tail == NULL)
    {
      args->kwargs_head = NULL;
    }
  else
    {
      args->kwargs_tail->next = NULL;
    }
}

grm_error_t argsIncreaseArray(grm_args_t *args, const char *key, size_t increment)
{
  grm_arg_t *arg;

  arg = argsAt(args, key);
  returnErrorIf(arg == NULL, GRM_ERROR_ARGS_INVALID_KEY);
  return argIncreaseArray(arg, increment);
}

unsigned int argsCount(const grm_args_t *args)
{
  return args->count;
}

grm_arg_t *argsAt(const grm_args_t *args, const char *keyword)
{
  ArgsNode *current_node;

  current_node = argsFindNode(args, keyword);

  if (current_node != NULL) return current_node->arg;
  return NULL;
}

int(grm_args_first_value)(const grm_args_t *args, const char *keyword, const char *first_value_format,
                          void *first_value, unsigned int *array_length)
{
  grm_arg_t *arg;

  arg = argsAt(args, keyword);
  if (arg == NULL) return 0;

  return argFirstValue(arg, first_value_format, first_value, array_length);
}

int grm_args_values(const grm_args_t *args, const char *keyword, const char *expected_format, ...)
{
  va_list vl;
  grm_arg_t *arg;
  int was_successful = 0;

  va_start(vl, expected_format);

  arg = argsAt(args, keyword);
  cleanupIf(arg == NULL);

  was_successful = argValuesVl(arg, expected_format, &vl);

cleanup:
  va_end(vl);

  return was_successful;
}

ArgsNode *argsFindNode(const grm_args_t *args, const char *keyword)
{
  ArgsNode *current_node;

  current_node = args->kwargs_head;
  while (current_node != NULL && strcmp(current_node->arg->key, keyword) != 0)
    {
      current_node = current_node->next;
    }

  return current_node;
}

int argsFindPreviousNode(const grm_args_t *args, const char *keyword, ArgsNode **previous_node)
{
  ArgsNode *prev_node, *current_node;

  prev_node = NULL;
  current_node = args->kwargs_head;
  while (current_node != NULL && strcmp(current_node->arg->key, keyword) != 0)
    {
      prev_node = current_node;
      current_node = current_node->next;
    }

  if (current_node != NULL)
    {
      *previous_node = prev_node;
      return 1;
    }
  return 0;
}

grm_args_iterator_t *grm_args_iter(const grm_args_t *args)
{
  return argsIteratorNew(args->kwargs_head, NULL);
}


/* ------------------------- argument iterator ---------------------------------------------------------------------- */

grm_args_iterator_t *argsIteratorNew(const ArgsNode *begin, const ArgsNode *end)
{
  grm_args_iterator_t *args_iterator;

  args_iterator = malloc(sizeof(grm_args_iterator_t));
  if (args_iterator == NULL)
    {
      debugPrintMallocError();
      return NULL;
    }
  args_iterator->priv = malloc(sizeof(grm_args_iterator_private_t));
  if (args_iterator->priv == NULL)
    {
      debugPrintMallocError();
      free(args_iterator);
      return NULL;
    }
  argsIteratorInit(args_iterator, begin, end);
  return args_iterator;
}

void argsIteratorInit(grm_args_iterator_t *args_iterator, const ArgsNode *begin, const ArgsNode *end)
{
  args_iterator->next = argsIteratorNext;
  args_iterator->arg = NULL;
  args_iterator->priv->next_node = begin;
  args_iterator->priv->end = end;
}

void argsIteratorDelete(grm_args_iterator_t *args_iterator)
{
  argsIteratorFinalize(args_iterator);
  free(args_iterator->priv);
  free(args_iterator);
}

void argsIteratorFinalize(grm_args_iterator_t *args_iterator UNUSED) {}

grm_arg_t *argsIteratorNext(grm_args_iterator_t *args_iterator)
{
  grm_arg_t *next_arg;

  if ((args_iterator->priv->next_node != NULL) && (args_iterator->priv->next_node != args_iterator->priv->end))
    {
      next_arg = args_iterator->priv->next_node->arg;
      args_iterator->priv->next_node = args_iterator->priv->next_node->next;
    }
  else
    {
      next_arg = NULL;
    }
  args_iterator->arg = next_arg;
  return next_arg;
}


/* ------------------------- value iterator ------------------------------------------------------------------------- */

grm_args_value_iterator_t *argsValueIteratorNew(const grm_arg_t *arg)
{
  grm_args_value_iterator_t *args_value_iterator;

  args_value_iterator = malloc(sizeof(grm_args_value_iterator_t));
  if (args_value_iterator == NULL)
    {
      debugPrintMallocError();
      return NULL;
    }
  args_value_iterator->priv = malloc(sizeof(grm_args_value_iterator_private_t));
  if (args_value_iterator->priv == NULL)
    {
      debugPrintMallocError();
      free(args_value_iterator);
      return NULL;
    }
  argsValueIteratorInit(args_value_iterator, arg);
  return args_value_iterator;
}

void argsValueIteratorInit(grm_args_value_iterator_t *args_value_iterator, const grm_arg_t *arg)
{
  args_value_iterator->next = argsValueIteratorNext;
  args_value_iterator->value_ptr = NULL;
  args_value_iterator->format = '\0';
  args_value_iterator->is_array = 0;
  args_value_iterator->array_length = 0;
  args_value_iterator->priv->value_buffer = arg->value_ptr;
  args_value_iterator->priv->value_format = arg->value_format;
}

void argsValueIteratorDelete(grm_args_value_iterator_t *args_value_iterator)
{
  argsValueIteratorFinalize(args_value_iterator);
  free(args_value_iterator->priv);
  free(args_value_iterator);
}

void argsValueIteratorFinalize(grm_args_value_iterator_t *args_value_iterator UNUSED) {}

void *argsValueIteratorNext(grm_args_value_iterator_t *args_value_iterator)
{
  const char *format;
  char current_format;
  void *value_buffer;
  void *value_ptr;
  int is_array;
  size_t array_length;
  int extracted_next_value;

  format = args_value_iterator->priv->value_format;
  value_buffer = args_value_iterator->priv->value_buffer;
  value_ptr = value_buffer;
  is_array = 0;
  array_length = 1;
  extracted_next_value = 0;
  while (*format)
    {
      format = argsSkipOption(format);
      if (!*format) break;
      current_format = tolower(*format);
      if (current_format != *format)
        {
          is_array = 1;
          array_length = *((size_t *)value_buffer);
          value_buffer = ((size_t *)value_buffer) + 1;
          value_ptr = value_buffer;
        }
#define STEP_VALUE_BUFFER_BY_TYPE(char, type)       \
  case char:                                        \
    if (is_array)                                   \
      {                                             \
        value_buffer = ((type **)value_buffer) + 1; \
      }                                             \
    else                                            \
      {                                             \
        value_buffer = ((type *)value_buffer) + 1;  \
      }                                             \
    break;

      switch (current_format)
        {
          STEP_VALUE_BUFFER_BY_TYPE('i', int)
          STEP_VALUE_BUFFER_BY_TYPE('d', double)
          STEP_VALUE_BUFFER_BY_TYPE('c', char)
          STEP_VALUE_BUFFER_BY_TYPE('s', char *)
          STEP_VALUE_BUFFER_BY_TYPE('a', grm_args_t *)
        case 'n':
          /* 'n' is not relevant for reading the values -> ignore it */
          break;
        default:
          break;
        }

#undef STEP_VALUE_BUFFER_BY_TYPE

      if (strchr(ARGS_VALID_DATA_FORMAT_SPECIFIERS, current_format) != NULL)
        {
          extracted_next_value = 1;
          break;
        }
      ++format;
    }

  if (extracted_next_value)
    {
      args_value_iterator->is_array = is_array;
      args_value_iterator->array_length = array_length;
      args_value_iterator->format = current_format;
      args_value_iterator->priv->value_format = ++format; /* FIXME: does not work for options! */
    }
  else
    {
      value_ptr = NULL;
      args_value_iterator->format = '\0';
    }

  args_value_iterator->priv->value_buffer = value_buffer;
  args_value_iterator->value_ptr = value_ptr;
  return value_ptr;
}


/* ######################### public implementation ################################################################## */

/* ========================= methods ================================================================================ */

/* ------------------------- argument container --------------------------------------------------------------------- */

grm_args_t *grm_args_new()
{
  grm_args_t *args = malloc(sizeof(grm_args_t));
  if (args == NULL)
    {
      debugPrintMallocError();
      return NULL;
    }
  argsInit(args);
  return args;
}

void grm_args_delete(grm_args_t *args)
{
  argsFinalize(args);
  free(args);
}

int grm_args_push(grm_args_t *args, const char *key, const char *value_format, ...)
{
  va_list vl;
  grm_error_t error;

  va_start(vl, value_format);

  error = argsPushVl(args, key, value_format, &vl);

  va_end(vl);

  return error == GRM_ERROR_NONE;
}

int grm_args_push_buf(grm_args_t *args, const char *key, const char *value_format, const void *buffer,
                      int apply_padding)
{
  grm_error_t error;

  error = argsPushCommon(args, key, value_format, buffer, NULL, apply_padding);

  return error == GRM_ERROR_NONE;
}

int grm_args_contains(const grm_args_t *args, const char *keyword)
{
  return argsAt(args, keyword) != NULL;
}

void grm_args_clear(grm_args_t *args)
{
  argsClear(args, plot_clear_exclude_keys);
}

void grm_args_remove(grm_args_t *args, const char *key)
{
  ArgsNode *tmp_node, *previous_node_by_keyword;

  if (argsFindPreviousNode(args, key, &previous_node_by_keyword))
    {
      if (previous_node_by_keyword == NULL)
        {
          tmp_node = args->kwargs_head->next;
          argsDecreaseArgReferenceCount(args->kwargs_head);
          free(args->kwargs_head);
          args->kwargs_head = tmp_node;
          if (tmp_node == NULL) args->kwargs_tail = NULL;
        }
      else
        {
          tmp_node = previous_node_by_keyword->next->next;
          argsDecreaseArgReferenceCount(previous_node_by_keyword->next);
          free(previous_node_by_keyword->next);
          previous_node_by_keyword->next = tmp_node;
          if (tmp_node == NULL) args->kwargs_tail = previous_node_by_keyword;
        }
      --(args->count);
    }
}

/* ------------------------- utilities ------------------------------------------------------------------------------ */

grm_args_ptr_t grm_length(double value, const char *unit)
{
  grm_args_ptr_t length = grm_args_new();
  grm_args_push(length, "value", "d", value);
  grm_args_push(length, "unit", "s", unit);
  return length;
}
