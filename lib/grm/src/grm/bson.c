#ifdef __unix__
#define _POSIX_C_SOURCE 200112L
#endif

/* ######################### includes ############################################################################### */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>

#include <grm/error.h>
#include "bson_int.h"
#include "plot_int.h"


/* ######################### internal implementation ################################################################ */

/* ========================= macros ================================================================================= */

const int i = 1;
#define is_bigendian() ((*(char *)&i) == 0)

/* ------------------------- general -------------------------------------------------------------------------------- */

#ifndef DBL_DECIMAL_DIG
#define DBL_DECIMAL_DIG 17
#endif


/* ========================= static variables ======================================================================= */

/* ------------------------- bson deserializer ------------------------------------------------------------------------
 */

static err_t (*frombson_datatype_to_func[128])(frombson_state_t *);
static int frombson_static_variables_initialized = 0;

/* ------------------------- bson serializer ------------------------------------------------------------------------ */

static err_t (*tobson_datatype_to_func[128])(tobson_state_t *);
static int tobson_static_variables_initialized = 0;
static tobson_permanent_state_t tobson_permanent_state = {complete, 0};
static char tobson_datatype_to_byte[128];
static char null = 0x00;


/* ========================= methods ================================================================================ */

void revmemcpy(void *dest, const void *src, size_t len)
{
  char *d = (char *)dest + len - 1;
  const char *s = src;
  while (len--)
    {
      *d-- = *s++;
    }
}

char byte_to_type(const char *byte)
{
  switch (*byte)
    {
    case (char)0x01:
      return 'd';
    case (char)0x02:
      return 's';
    case (char)0x03:
      return 'a';
    case (char)0x04:
      return 'n';
    case (char)0x08:
      return 'b';
    case (char)0x10:
      return 'i';
    default:
      return '\0';
    }
}

void int_to_bytes(int i, char **bytes)
{
  *bytes = (char *)malloc(sizeof(int) * sizeof(char));
  if (is_bigendian())
    {
      revmemcpy(*bytes, &i, sizeof(i));
    }
  else
    {
      memcpy(*bytes, &i, sizeof(i));
    }
}

void bytes_to_int(int *i, const char *bytes)
{
  if (is_bigendian())
    {
      revmemcpy(i, bytes, sizeof(int));
    }
  else
    {
      memcpy(i, bytes, sizeof(int));
    }
}

void double_to_bytes(double d, char **bytes)
{
  *bytes = (char *)malloc(sizeof(double) * sizeof(char));
  if (is_bigendian())
    {
      revmemcpy(*bytes, &d, sizeof(d));
    }
  else
    {
      memcpy(*bytes, &d, sizeof(d));
    }
}

void bytes_to_double(double *d, const char *bytes)
{
  if (is_bigendian())
    {
      revmemcpy(d, bytes, sizeof(double));
    }
  else
    {
      memcpy(d, bytes, sizeof(double));
    }
}


/* ------------------------- bson deserializer ---------------------------------------------------------------------- */

err_t frombson_read(grm_args_t *args, const char *bson_bytes)
{

  int document_size;
  frombson_state_t state;
  frombson_object_infos_t object_infos;
  err_t error = ERROR_NONE;

  frombson_init_static_variables();
  state.num_read_bytes = 0;
  state.cur_byte = bson_bytes;
  state.args = args;
  state.cur_value_buf = NULL;

  object_infos.num_bytes_read_before = 0;

  if ((error = frombson_read_length(&state, &document_size)) != ERROR_NONE)
    {
      return error;
    }

  object_infos.length = document_size;
  state.object_infos = &object_infos;

  if ((error = frombson_read_object(&state)) != ERROR_NONE)
    {
      return error;
    }

  return error;
}

err_t frombson_read_value_format(frombson_state_t *state, char *value_format)
{

  *value_format = byte_to_type(state->cur_byte);
  state->num_read_bytes++;
  state->cur_byte++;

  return ERROR_NONE;
}

err_t frombson_read_key(frombson_state_t *state, const char **key)
{

  *key = state->cur_byte;

  while (*(state->cur_byte) != '\0')
    {
      ++(state->num_read_bytes);
      ++(state->cur_byte);
    }

  ++(state->num_read_bytes);
  ++(state->cur_byte);

  return ERROR_NONE;
}

err_t frombson_skip_key(frombson_state_t *state)
{

  while (*(state->cur_byte) != '\0')
    {
      ++(state->num_read_bytes);
      ++(state->cur_byte);
    }

  ++(state->num_read_bytes);
  ++(state->cur_byte);

  return ERROR_NONE;
}

err_t frombson_read_length(frombson_state_t *state, int *length)
{

  bytes_to_int(length, state->cur_byte);
  state->cur_byte += sizeof(int);
  state->num_read_bytes += sizeof(int);

  return ERROR_NONE;
}

err_t frombson_read_double_value(frombson_state_t *state, double *d)
{

  bytes_to_double(d, state->cur_byte);

  state->num_read_bytes += sizeof(double);
  state->cur_byte += sizeof(double);

  return ERROR_NONE;
}

err_t frombson_read_int_value(frombson_state_t *state, int *i)
{

  bytes_to_int(i, state->cur_byte);

  state->num_read_bytes += sizeof(int);
  state->cur_byte += sizeof(int);

  return ERROR_NONE;
}

err_t frombson_read_string_value(frombson_state_t *state, const char **s)
{

  *s = state->cur_byte;

  while (*(state->cur_byte) != '\0')
    {
      ++(state->num_read_bytes);
      ++(state->cur_byte);
    }

  ++(state->num_read_bytes);
  ++(state->cur_byte);

  return ERROR_NONE;
}

err_t frombson_read_bool_value(frombson_state_t *state, int *b)
{

  if (*(state->cur_byte) == (char)0x00)
    {
      *b = 0;
    }
  else
    {
      *b = 1;
    }

  state->num_read_bytes++;
  state->cur_byte++;

  return ERROR_NONE;
}

err_t frombson_read_object(frombson_state_t *state)
{

  err_t error = ERROR_NONE;
  int object_closed = 0;
  frombson_object_infos_t *object_infos = state->object_infos;

  while (object_infos->length - (state->num_read_bytes - object_infos->num_bytes_read_before) > 0)
    {

      if ((error = frombson_read_value_format(state, &(state->cur_value_format))) != ERROR_NONE)
        {
          return error;
        }

      if ((error = frombson_datatype_to_func[state->cur_value_format](state)) != ERROR_NONE)
        {
          return error;
        }

      if (object_infos->length - (state->num_read_bytes - object_infos->num_bytes_read_before) == 1)
        {
          if (*(state->cur_byte) == '\0')
            {
              state->num_read_bytes++;
              state->cur_byte++;
              object_closed = 1;
            }
        }
    }

  if (!object_closed)
    {
      error = ERROR_PARSE_OBJECT;
    }

  return error;
}

err_t frombson_parse_double(frombson_state_t *state)
{

  double d;
  int memory_allocated = 0;
  err_t error;

  char cur_value_type[2] = "\0";
  cur_value_type[0] = state->cur_value_format;

  if ((error = frombson_read_key(state, &(state->cur_key))) != ERROR_NONE)
    {
      goto cleanup;
    }

  state->cur_value_buf = malloc(sizeof(double));
  if (state->cur_value_buf == NULL)
    {
      debug_print_malloc_error();
      goto cleanup;
    }
  memory_allocated = 1;
  if ((error = frombson_read_double_value(state, &d)) != ERROR_NONE)
    {
      goto cleanup;
    }
  *((double *)state->cur_value_buf) = d;

  grm_args_push_buf(state->args, state->cur_key, cur_value_type, state->cur_value_buf, 0);

cleanup:
  if (memory_allocated)
    {
      free(state->cur_value_buf);
    }

  return error;
}

err_t frombson_parse_int(frombson_state_t *state)
{

  int i;
  err_t error;
  int memory_allocated = 0;

  char cur_value_type[2] = "\0";
  cur_value_type[0] = state->cur_value_format;

  if ((error = frombson_read_key(state, &(state->cur_key))) != ERROR_NONE)
    {
      goto cleanup;
    }

  state->cur_value_buf = malloc(sizeof(int));
  if (state->cur_value_buf == NULL)
    {
      debug_print_malloc_error();
      goto cleanup;
    }
  memory_allocated = 1;
  if ((error = frombson_read_int_value(state, &i)) != ERROR_NONE)
    {
      goto cleanup;
    }
  *((int *)state->cur_value_buf) = i;

  grm_args_push_buf(state->args, state->cur_key, cur_value_type, state->cur_value_buf, 0);

cleanup:
  if (memory_allocated)
    {
      free(state->cur_value_buf);
    }

  return error;
}

err_t frombson_parse_string(frombson_state_t *state)
{

  int length;
  const char *s;
  err_t error;
  int memory_allocated = 0;

  char cur_value_type[2] = "\0";
  cur_value_type[0] = state->cur_value_format;


  if ((error = frombson_read_key(state, &(state->cur_key))) != ERROR_NONE)
    {
      goto cleanup;
    }

  if ((error = frombson_read_length(state, &length)) != ERROR_NONE)
    {
      return error;
    }

  state->cur_value_buf = malloc(length * sizeof(char));
  if (state->cur_value_buf == NULL)
    {
      debug_print_malloc_error();
      goto cleanup;
    }
  memory_allocated = 1;
  if ((error = frombson_read_string_value(state, &s)) != ERROR_NONE)
    {
      goto cleanup;
    }
  *((const char **)state->cur_value_buf) = s;

  grm_args_push_buf(state->args, state->cur_key, cur_value_type, state->cur_value_buf, 0);

cleanup:
  if (memory_allocated)
    {
      free(state->cur_value_buf);
    }

  return error;
}

err_t frombson_parse_bool(frombson_state_t *state)
{

  int b;
  err_t error;
  int memory_allocated = 0;

  char cur_value_type[2] = "\0";
  cur_value_type[0] = state->cur_value_format;

  if ((error = frombson_read_key(state, &(state->cur_key))) != ERROR_NONE)
    {
      goto cleanup;
    }

  state->cur_value_buf = malloc(sizeof(int));
  if (state->cur_value_buf == NULL)
    {
      debug_print_malloc_error();
      goto cleanup;
    }
  memory_allocated = 1;
  if ((error = frombson_read_bool_value(state, &b)) != ERROR_NONE)
    {
      goto cleanup;
    }
  *((int *)state->cur_value_buf) = b;

  grm_args_push_buf(state->args, state->cur_key, cur_value_type, state->cur_value_buf, 0);

cleanup:
  if (memory_allocated)
    {
      free(state->cur_value_buf);
    }

  return error;
}

err_t frombson_parse_array(frombson_state_t *state)
{

  int length, num_bytes_read_before;
  char first_value_type;
  char final_value_type[3] = "\0";
  frombson_array_infos_t array_infos;
  err_t error;
  int memory_allocated = 0;

  final_value_type[0] = state->cur_value_format;
  if ((error = frombson_read_key(state, &(state->cur_key))) != ERROR_NONE)
    {
      goto cleanup;
    }

  num_bytes_read_before = state->num_read_bytes;
  if ((error = frombson_read_length(state, &length)) != ERROR_NONE)
    {
      goto cleanup;
    }

  /* read first value type without changing pointer */
  first_value_type = byte_to_type(state->cur_byte);
  final_value_type[1] = toupper(first_value_type);

  state->cur_value_format = first_value_type;
  array_infos.length = length;
  array_infos.num_bytes_read_before = num_bytes_read_before;
  state->array_infos = &array_infos;

  if ((error = frombson_datatype_to_func[toupper(first_value_type)](state)) != ERROR_NONE)
    {
      goto cleanup;
    }
  memory_allocated = 1;

  grm_args_push(state->args, state->cur_key, final_value_type, state->array_infos->num_elements, state->cur_value_buf);

cleanup:
  if (memory_allocated)
    {
      free(state->cur_value_buf);
    }

  return error;
}

err_t frombson_parse_object(frombson_state_t *state)
{

  int length;
  err_t error;
  int num_bytes_read_before;
  grm_args_t *new_args = grm_args_new();
  frombson_state_t inner_state;
  frombson_object_infos_t object_infos;

  char cur_value_type[2] = "\0";
  cur_value_type[0] = state->cur_value_format;

  if ((error = frombson_read_key(state, &(state->cur_key))) != ERROR_NONE)
    {
      return error;
    }

  num_bytes_read_before = state->num_read_bytes;

  if ((error = frombson_read_length(state, &length)) != ERROR_NONE)
    {
      return error;
    }

  object_infos.length = length;
  object_infos.num_bytes_read_before = num_bytes_read_before;

  inner_state.num_read_bytes = state->num_read_bytes;
  inner_state.args = new_args;
  inner_state.cur_byte = state->cur_byte;
  inner_state.cur_value_buf = NULL;
  inner_state.object_infos = &object_infos;

  frombson_read_object(&inner_state);

  state->num_read_bytes = inner_state.num_read_bytes;
  state->cur_byte = inner_state.cur_byte;
  grm_args_push(state->args, state->cur_key, cur_value_type, inner_state.args);


  return error;
}

err_t frombson_read_int_array(frombson_state_t *state)
{

  int i = 0;
  char next_value_type;
  int array_closed = 0;
  err_t error;
  int memory_allocated = 0;
  frombson_array_infos_t *array_infos = state->array_infos;

  array_infos->num_elements = (array_infos->length - 5) / (sizeof(int) + 3);
  state->cur_value_buf = malloc(sizeof(int) * array_infos->num_elements);
  if (state->cur_value_buf == NULL)
    {
      debug_print_malloc_error();
      goto cleanup;
    }
  memory_allocated = 1;
  int current_value;

  while (array_infos->length - (state->num_read_bytes - array_infos->num_bytes_read_before) > 0)
    {
      if ((error = frombson_read_value_format(state, &next_value_type)) != ERROR_NONE)
        {
          goto cleanup;
        }
      if (state->cur_value_format != next_value_type)
        {
          error = ERROR_PARSE_ARRAY;
          goto cleanup;
        }
      if ((error = frombson_skip_key(state)) != ERROR_NONE)
        {
          goto cleanup;
        }
      if ((error = frombson_read_int_value(state, &current_value)) != ERROR_NONE)
        {
          goto cleanup;
        }
      ((int *)state->cur_value_buf)[i] = current_value;
      ++i;

      if (array_infos->length - (state->num_read_bytes - array_infos->num_bytes_read_before) == 1)
        {
          if (*(state->cur_byte) == '\0')
            {
              state->num_read_bytes++;
              state->cur_byte++;
              array_closed = 1;
            }
        }
    }

  if (!array_closed)
    {
      error = ERROR_PARSE_ARRAY;
    }

cleanup:
  if (memory_allocated && error != ERROR_NONE)
    {
      free(state->cur_value_buf);
    }

  return error;
}

err_t frombson_read_double_array(frombson_state_t *state)
{

  int i = 0;
  char next_value_type;
  int array_closed = 0;
  err_t error;
  int memory_allocated = 0;
  frombson_array_infos_t *array_infos = state->array_infos;

  array_infos->num_elements = (array_infos->length - 5) / (sizeof(double) + 3);
  state->cur_value_buf = malloc(sizeof(double) * array_infos->num_elements);
  if (state->cur_value_buf == NULL)
    {
      debug_print_malloc_error();
      goto cleanup;
    }
  memory_allocated = 1;
  double current_value;

  while (array_infos->length - (state->num_read_bytes - array_infos->num_bytes_read_before) > 0)
    {
      if ((error = frombson_read_value_format(state, &next_value_type)) != ERROR_NONE)
        {
          goto cleanup;
        }
      if (state->cur_value_format != next_value_type)
        {
          error = ERROR_PARSE_ARRAY;
          goto cleanup;
        }
      if ((error = frombson_skip_key(state)) != ERROR_NONE)
        {
          goto cleanup;
        }
      if ((error = frombson_read_double_value(state, &current_value)) != ERROR_NONE)
        {
          goto cleanup;
        }
      ((double *)state->cur_value_buf)[i] = current_value;
      ++i;

      if (array_infos->length - (state->num_read_bytes - array_infos->num_bytes_read_before) == 1)
        {
          if (*(state->cur_byte) == '\0')
            {
              state->num_read_bytes++;
              state->cur_byte++;
              array_closed = 1;
            }
        }
    }

  if (!array_closed)
    {
      error = ERROR_PARSE_ARRAY;
    }

cleanup:
  if (memory_allocated && error != ERROR_NONE)
    {
      free(state->cur_value_buf);
    }

  return error;
}

err_t frombson_read_string_array(frombson_state_t *state)
{

  int i = 0;
  char next_value_type;
  int array_closed = 0;
  int string_length;
  err_t error;
  int memory_allocated = 0;
  frombson_array_infos_t *array_infos = state->array_infos;

  state->cur_value_buf = malloc(array_infos->length - 4); /* array length minus length bytes */
  if (state->cur_value_buf == NULL)
    {
      debug_print_malloc_error();
      goto cleanup;
    }
  memory_allocated = 1;
  const char *current_value;

  while (array_infos->length - (state->num_read_bytes - array_infos->num_bytes_read_before) > 0)
    {
      if ((error = frombson_read_value_format(state, &next_value_type)) != ERROR_NONE)
        {
          goto cleanup;
        }
      if (state->cur_value_format != next_value_type)
        {
          error = ERROR_PARSE_ARRAY;
          goto cleanup;
        }
      if ((error = frombson_skip_key(state)) != ERROR_NONE)
        {
          goto cleanup;
        }
      if ((error = frombson_read_length(state, &string_length)) != ERROR_NONE)
        {
          goto cleanup;
        }
      if ((error = frombson_read_string_value(state, &current_value)) != ERROR_NONE)
        {
          goto cleanup;
        }
      ((const char **)state->cur_value_buf)[i] = current_value;
      ++i;

      if (array_infos->length - (state->num_read_bytes - array_infos->num_bytes_read_before) == 1)
        {
          if (*(state->cur_byte) == '\0')
            {
              state->num_read_bytes++;
              state->cur_byte++;
              array_closed = 1;
            }
        }
    }

  array_infos->num_elements = i;

  if (!array_closed)
    {
      error = ERROR_PARSE_ARRAY;
    }

cleanup:
  if (memory_allocated && error != ERROR_NONE)
    {
      free(state->cur_value_buf);
    }

  return error;
}

err_t frombson_read_bool_array(frombson_state_t *state)
{

  int i = 0;
  char next_value_type;
  int array_closed = 0;
  err_t error;
  int memory_allocated = 0;
  frombson_array_infos_t *array_infos = state->array_infos;

  array_infos->num_elements = (array_infos->length - 5) / (sizeof(char) + 3);
  state->cur_value_buf = malloc(sizeof(int) * array_infos->num_elements);
  if (state->cur_value_buf == NULL)
    {
      debug_print_malloc_error();
      goto cleanup;
    }
  memory_allocated = 1;
  int current_value;

  while (array_infos->length - (state->num_read_bytes - array_infos->num_bytes_read_before) > 0)
    {
      if ((error = frombson_read_value_format(state, &next_value_type)) != ERROR_NONE)
        {
          goto cleanup;
        }
      if (state->cur_value_format != next_value_type)
        {
          error = ERROR_PARSE_ARRAY;
          goto cleanup;
        }
      if ((error = frombson_skip_key(state)) != ERROR_NONE)
        {
          goto cleanup;
        }
      if ((error = frombson_read_bool_value(state, &current_value)) != ERROR_NONE)
        {
          goto cleanup;
        }
      ((int *)state->cur_value_buf)[i] = current_value;
      ++i;

      if (array_infos->length - (state->num_read_bytes - array_infos->num_bytes_read_before) == 1)
        {
          if (*(state->cur_byte) == '\0')
            {
              state->num_read_bytes++;
              state->cur_byte++;
              array_closed = 1;
            }
        }
    }

  if (!array_closed)
    {
      error = ERROR_PARSE_ARRAY;
    }

cleanup:
  if (memory_allocated && error != ERROR_NONE)
    {
      free(state->cur_value_buf);
    }

  return error;
}

err_t frombson_read_object_array(frombson_state_t *state)
{

  int i = 0;
  char next_value_type;
  int array_closed = 0;
  int cur_object_length;
  err_t error;
  int memory_allocated = 0;
  frombson_array_infos_t *array_infos = state->array_infos;
  frombson_state_t inner_state;
  frombson_object_infos_t object_infos;
  int num_read_bytes_before;

  state->cur_value_buf = malloc(array_infos->length - 4); /* array length minus length bytes */
  if (state->cur_value_buf == NULL)
    {
      debug_print_malloc_error();
      goto cleanup;
    }
  memory_allocated = 1;

  while (array_infos->length - (state->num_read_bytes - array_infos->num_bytes_read_before) > 0)
    {
      if ((error = frombson_read_value_format(state, &next_value_type)) != ERROR_NONE)
        {
          goto cleanup;
        }
      if (state->cur_value_format != next_value_type)
        {
          error = ERROR_PARSE_ARRAY;
          goto cleanup;
        }
      if ((error = frombson_skip_key(state)) != ERROR_NONE)
        {
          goto cleanup;
        }
      num_read_bytes_before = state->num_read_bytes;
      if ((error = frombson_read_length(state, &cur_object_length)) != ERROR_NONE)
        {
          goto cleanup;
        }

      inner_state.args = grm_args_new();
      inner_state.cur_byte = state->cur_byte;
      inner_state.num_read_bytes = state->num_read_bytes;
      inner_state.cur_value_buf = NULL;

      object_infos.length = cur_object_length;
      object_infos.num_bytes_read_before = num_read_bytes_before;
      inner_state.object_infos = &object_infos;

      if ((error = frombson_read_object(&inner_state)) != ERROR_NONE)
        {
          goto cleanup;
        }

      state->num_read_bytes = inner_state.num_read_bytes;
      state->cur_byte = inner_state.cur_byte;

      ((grm_args_t **)state->cur_value_buf)[i] = inner_state.args;
      ++i;

      if (array_infos->length - (state->num_read_bytes - array_infos->num_bytes_read_before) == 1)
        {
          if (*(state->cur_byte) == '\0')
            {
              state->num_read_bytes++;
              state->cur_byte++;
              array_closed = 1;
            }
        }
    }

  array_infos->num_elements = i;

  if (!array_closed)
    {
      error = ERROR_PARSE_ARRAY;
    }

cleanup:
  if (memory_allocated && error != ERROR_NONE)
    {
      free(state->cur_value_buf);
    }

  return error;
}

void frombson_init_static_variables(void)
{
  if (!frombson_static_variables_initialized)
    {
      frombson_datatype_to_func['n'] = frombson_parse_array;
      frombson_datatype_to_func['i'] = frombson_parse_int;
      frombson_datatype_to_func['I'] = frombson_read_int_array;
      frombson_datatype_to_func['d'] = frombson_parse_double;
      frombson_datatype_to_func['D'] = frombson_read_double_array;
      frombson_datatype_to_func['s'] = frombson_parse_string;
      frombson_datatype_to_func['S'] = frombson_read_string_array;
      frombson_datatype_to_func['b'] = frombson_parse_bool;
      frombson_datatype_to_func['B'] = frombson_read_bool_array;
      frombson_datatype_to_func['a'] = frombson_parse_object;
      frombson_datatype_to_func['A'] = frombson_read_object_array;

      frombson_static_variables_initialized = 1;
    }
}


/* ------------------------- bson serializer ------------------------------------------------------------------------ */

err_t tobson_stringify_int(tobson_state_t *state)
{
  int value;
  err_t error = ERROR_NONE;
  if (state->shared->data_ptr != NULL && state->shared->apply_padding)
    {
      ptrdiff_t needed_padding = state->shared->data_offset % sizeof(int);
      state->shared->data_ptr = ((char *)state->shared->data_ptr) + needed_padding;
      state->shared->data_offset += needed_padding;
    }
  if (state->shared->data_ptr != NULL)
    {
      value = *((int *)state->shared->data_ptr);
      state->shared->data_ptr = ((int *)state->shared->data_ptr) + 1;
      state->shared->data_offset += sizeof(int);
    }
  else
    {
      value = va_arg(*state->shared->vl, int);
    }
  if ((error = tobson_stringify_int_value(state->memwriter, value)) != ERROR_NONE)
    {
      return error;
    }
  state->shared->wrote_output = 1;
  return error;
}

err_t tobson_stringify_int_array(tobson_state_t *state)
{
  int *values;
  int current_value;
  unsigned int length;
  int remaining_elements;
  err_t error = ERROR_NONE;
  char *length_as_bytes;
  char length_placeholder[4] = {0x01, 0x01, 0x01, 0x01};
  int size_before = state->memwriter->size;
  char *placeholder_pos = (state->memwriter->buf) + (state->memwriter->size);
  char key = '0';
  if (state->shared->data_ptr != NULL)
    {
      if (state->shared->data_ptr != NULL && state->shared->apply_padding)
        {
          ptrdiff_t needed_padding = state->shared->data_offset % sizeof(int *);
          state->shared->data_ptr = ((char *)state->shared->data_ptr) + needed_padding;
          state->shared->data_offset += needed_padding;
        }
      values = *(int **)state->shared->data_ptr;
    }
  else
    {
      values = va_arg(*state->shared->vl, int *);
    }
  if (state->additional_type_info != NULL)
    {
      if (!str_to_uint(state->additional_type_info, &length))
        {
          debug_print_error(("The given array length \"%s\" is no valid number; the array contents will be ignored.",
                             state->additional_type_info));
          length = 0;
        }
    }
  else
    {
      length = state->shared->array_length;
    }
  remaining_elements = length;
  /* write array start */
  if ((error = memwriter_puts_with_len(state->memwriter, length_placeholder, 4)) != ERROR_NONE)
    {
      return error;
    }
  /* write array content */
  while (remaining_elements)
    {
      current_value = *values++;
      /* Datatype */
      if ((error = memwriter_putc(state->memwriter, tobson_datatype_to_byte['i'])) != ERROR_NONE)
        {
          return error;
        }
      /* Key */
      if ((error = memwriter_putc(state->memwriter, key++)) != ERROR_NONE)
        {
          return error;
        }
      /* End Of Key */
      if ((error = memwriter_putc(state->memwriter, null)) != ERROR_NONE)
        {
          return error;
        }
      /* Value */
      if ((error = tobson_stringify_int_value(state->memwriter, current_value)) != ERROR_NONE)
        {
          return error;
        }
      --remaining_elements;
    }
  /* write array end */
  if ((error = memwriter_putc(state->memwriter, null)) != ERROR_NONE)
    {
      return error;
    }
  /* Set length of object*/
  int_to_bytes(state->memwriter->size - size_before, &length_as_bytes);
  memcpy(placeholder_pos, length_as_bytes, 4);
  free(length_as_bytes);
  if (state->shared->data_ptr != NULL)
    {
      state->shared->data_ptr = ((int **)state->shared->data_ptr) + 1;
      state->shared->data_offset += sizeof(int *);
    }
  state->shared->wrote_output = 1;
  return error;
}

err_t tobson_stringify_int_value(memwriter_t *memwriter, int value)
{
  char *bytes;
  err_t error;

  int_to_bytes(value, &bytes);

  error = memwriter_puts_with_len(memwriter, bytes, sizeof(int));
  free(bytes);

  return error;
}


err_t tobson_stringify_double(tobson_state_t *state)
{
  double value;
  err_t error = ERROR_NONE;
  if (state->shared->data_ptr != NULL && state->shared->apply_padding)
    {
      ptrdiff_t needed_padding = state->shared->data_offset % sizeof(double);
      state->shared->data_ptr = ((char *)state->shared->data_ptr) + needed_padding;
      state->shared->data_offset += needed_padding;
    }
  if (state->shared->data_ptr != NULL)
    {
      value = *((double *)state->shared->data_ptr);
      state->shared->data_ptr = ((double *)state->shared->data_ptr) + 1;
      state->shared->data_offset += sizeof(double);
    }
  else
    {
      value = va_arg(*state->shared->vl, double);
    }
  if ((error = tobson_stringify_double_value(state->memwriter, value)) != ERROR_NONE)
    {
      return error;
    }
  state->shared->wrote_output = 1;
  return error;
}

err_t tobson_stringify_double_array(tobson_state_t *state)
{
  double *values;
  double current_value;
  unsigned int length;
  int remaining_elements;
  err_t error = ERROR_NONE;
  char *length_as_bytes;
  char length_placeholder[4] = {0x01, 0x01, 0x01, 0x01};
  int size_before = state->memwriter->size;
  char *placeholder_pos = (state->memwriter->buf) + (state->memwriter->size);
  char key = '0';
  if (state->shared->data_ptr != NULL)
    {
      if (state->shared->data_ptr != NULL && state->shared->apply_padding)
        {
          ptrdiff_t needed_padding = state->shared->data_offset % sizeof(double *);
          state->shared->data_ptr = ((char *)state->shared->data_ptr) + needed_padding;
          state->shared->data_offset += needed_padding;
        }
      values = *(double **)state->shared->data_ptr;
    }
  else
    {
      values = va_arg(*state->shared->vl, double *);
    }
  if (state->additional_type_info != NULL)
    {
      if (!str_to_uint(state->additional_type_info, &length))
        {
          debug_print_error(("The given array length \"%s\" is no valid number; the array contents will be ignored.",
                             state->additional_type_info));
          length = 0;
        }
    }
  else
    {
      length = state->shared->array_length;
    }
  remaining_elements = length;
  /* write array start */
  /* Placeholder for length of array*/
  if ((error = memwriter_puts_with_len(state->memwriter, length_placeholder, 4)) != ERROR_NONE)
    {
      return error;
    }
  /* write array content */
  while (remaining_elements)
    {
      current_value = *values++;
      /* Datatype */
      if ((error = memwriter_putc(state->memwriter, tobson_datatype_to_byte['d'])) != ERROR_NONE)
        {
          return error;
        }
      /* Key */
      if ((error = memwriter_putc(state->memwriter, key++)) != ERROR_NONE)
        {
          return error;
        }
      /* End Of Key */
      if ((error = memwriter_putc(state->memwriter, null)) != ERROR_NONE)
        {
          return error;
        }
      /* Value */
      if ((error = tobson_stringify_double_value(state->memwriter, current_value)) != ERROR_NONE)
        {
          return error;
        }
      --remaining_elements;
    }
  /* write array end */
  if ((error = memwriter_putc(state->memwriter, null)) != ERROR_NONE)
    {
      return error;
    }
  /* Set length of object*/
  int_to_bytes(state->memwriter->size - size_before, &length_as_bytes);
  memcpy(placeholder_pos, length_as_bytes, 4);
  free(length_as_bytes);
  if (state->shared->data_ptr != NULL)
    {
      state->shared->data_ptr = ((double **)state->shared->data_ptr) + 1;
      state->shared->data_offset += sizeof(double *);
    }
  state->shared->wrote_output = 1;
  return error;
}


err_t tobson_stringify_char(tobson_state_t *state)
{
  char value;
  err_t error = ERROR_NONE;
  if (state->shared->data_ptr != NULL && state->shared->apply_padding)
    {
      ptrdiff_t needed_padding = state->shared->data_offset % sizeof(char);
      state->shared->data_ptr = ((char *)state->shared->data_ptr) + needed_padding;
      state->shared->data_offset += needed_padding;
    }
  if (state->shared->data_ptr != NULL)
    {
      value = *((char *)state->shared->data_ptr);
      state->shared->data_ptr = ((char *)state->shared->data_ptr) + 1;
      state->shared->data_offset += sizeof(char);
    }
  else
    {
      value = va_arg(*state->shared->vl, int);
    }
  if ((error = tobson_stringify_char_value(state->memwriter, value)) != ERROR_NONE)
    {
      return error;
    }
  state->shared->wrote_output = 1;
  return error;
}

err_t tobson_stringify_char_value(memwriter_t *memwriter, char value)
{
  err_t error;
  char length[4] = {(char)0x02, (char)0x00, (char)0x00, (char)0x00}; /*char is saved as string */
  if ((error = memwriter_puts_with_len(memwriter, length, 4)) != ERROR_NONE)
    {
      return error;
    }
  if ((error = memwriter_putc(memwriter, value)) != ERROR_NONE)
    {
      return error;
    }
  if ((error = memwriter_putc(memwriter, null)) != ERROR_NONE)
    {
      return error;
    }
  return ERROR_NONE;
}


err_t tobson_stringify_string(tobson_state_t *state)
{
  char *value;
  err_t error = ERROR_NONE;
  if (state->shared->data_ptr != NULL && state->shared->apply_padding)
    {
      ptrdiff_t needed_padding = state->shared->data_offset % sizeof(char *);
      state->shared->data_ptr = ((char *)state->shared->data_ptr) + needed_padding;
      state->shared->data_offset += needed_padding;
    }
  if (state->shared->data_ptr != NULL)
    {
      value = *((char **)state->shared->data_ptr);
      state->shared->data_ptr = ((char **)state->shared->data_ptr) + 1;
      state->shared->data_offset += sizeof(char *);
    }
  else
    {
      value = va_arg(*state->shared->vl, char *);
    }
  if ((error = tobson_stringify_string_value(state->memwriter, value)) != ERROR_NONE)
    {
      return error;
    }
  state->shared->wrote_output = 1;
  return error;
}

err_t tobson_stringify_string_array(tobson_state_t *state)
{
  char **values;
  char *current_value;
  unsigned int length;
  int remaining_elements;
  err_t error = ERROR_NONE;
  char *length_as_bytes;
  char length_placeholder[4] = {0x01, 0x01, 0x01, 0x01};
  int size_before = state->memwriter->size;
  char *placeholder_pos = (state->memwriter->buf) + (state->memwriter->size);
  char key = '0';
  if (state->shared->data_ptr != NULL)
    {
      if (state->shared->data_ptr != NULL && state->shared->apply_padding)
        {
          ptrdiff_t needed_padding = state->shared->data_offset % sizeof(char **);
          state->shared->data_ptr = ((char *)state->shared->data_ptr) + needed_padding;
          state->shared->data_offset += needed_padding;
        }
      values = *(char ***)state->shared->data_ptr;
    }
  else
    {
      values = va_arg(*state->shared->vl, char **);
    }
  if (state->additional_type_info != NULL)
    {
      if (!str_to_uint(state->additional_type_info, &length))
        {
          debug_print_error(("The given array length \"%s\" is no valid number; the array contents will be ignored.",
                             state->additional_type_info));
          length = 0;
        }
    }
  else
    {
      length = state->shared->array_length;
    }
  remaining_elements = length;
  /* write array start */
  /* Placeholder for length of array*/
  if ((error = memwriter_puts_with_len(state->memwriter, length_placeholder, 4)) != ERROR_NONE)
    {
      return error;
    }
  /* write array content */
  while (remaining_elements)
    {
      current_value = *values++;
      /* Datatype */
      if ((error = memwriter_putc(state->memwriter, tobson_datatype_to_byte['s'])) != ERROR_NONE)
        {
          return error;
        }
      /* Key */
      if ((error = memwriter_putc(state->memwriter, key++)) != ERROR_NONE)
        {
          return error;
        }
      /* End Of Key */
      if ((error = memwriter_putc(state->memwriter, null)) != ERROR_NONE)
        {
          return error;
        }
      /* Value */
      if ((error = tobson_stringify_string_value(state->memwriter, current_value)) != ERROR_NONE)
        {
          return error;
        }
      --remaining_elements;
    } /* write array end */
  if ((error = memwriter_putc(state->memwriter, null)) != ERROR_NONE)
    {
      return error;
    }
  /* Set length of object*/
  int_to_bytes(state->memwriter->size - size_before, &length_as_bytes);
  memcpy(placeholder_pos, length_as_bytes, 4);
  free(length_as_bytes);
  if (state->shared->data_ptr != NULL)
    {
      state->shared->data_ptr = ((char ***)state->shared->data_ptr) + 1;
      state->shared->data_offset += sizeof(char **);
    }
  state->shared->wrote_output = 1;
  return error;
}

err_t tobson_stringify_bool(tobson_state_t *state)
{
  int value;
  err_t error = ERROR_NONE;
  if (state->shared->data_ptr != NULL && state->shared->apply_padding)
    {
      ptrdiff_t needed_padding = state->shared->data_offset % sizeof(int);
      state->shared->data_ptr = ((char *)state->shared->data_ptr) + needed_padding;
      state->shared->data_offset += needed_padding;
    }
  if (state->shared->data_ptr != NULL)
    {
      value = *((int *)state->shared->data_ptr);
      state->shared->data_ptr = ((int *)state->shared->data_ptr) + 1;
      state->shared->data_offset += sizeof(int);
    }
  else
    {
      value = va_arg(*state->shared->vl, int);
    }
  if ((error = tobson_stringify_bool_value(state->memwriter, value)) != ERROR_NONE)
    {
      return error;
    }
  state->shared->wrote_output = 1;
  return error;
}

err_t tobson_stringify_bool_array(tobson_state_t *state)
{
  int *values;
  int current_value;
  unsigned int length;
  int remaining_elements;
  err_t error = ERROR_NONE;
  char *length_as_bytes;
  char length_placeholder[4] = {0x01, 0x01, 0x01, 0x01};
  int size_before = state->memwriter->size;
  char *placeholder_pos = (state->memwriter->buf) + (state->memwriter->size);
  char key = '0';
  if (state->shared->data_ptr != NULL)
    {
      if (state->shared->data_ptr != NULL && state->shared->apply_padding)
        {
          ptrdiff_t needed_padding = state->shared->data_offset % sizeof(int *);
          state->shared->data_ptr = ((char *)state->shared->data_ptr) + needed_padding;
          state->shared->data_offset += needed_padding;
        }
      values = *(int **)state->shared->data_ptr;
    }
  else
    {
      values = va_arg(*state->shared->vl, int *);
    }
  if (state->additional_type_info != NULL)
    {
      if (!str_to_uint(state->additional_type_info, &length))
        {
          debug_print_error(("The given array length \"%s\" is no valid number; the array contents will be ignored.",
                             state->additional_type_info));
          length = 0;
        }
    }
  else
    {
      length = state->shared->array_length;
    }
  remaining_elements = length;
  /* write array start */
  /* Placeholder for length of array*/
  if ((error = memwriter_puts_with_len(state->memwriter, length_placeholder, 4)) != ERROR_NONE)
    {
      return error;
    }
  /* write array content */
  while (remaining_elements)
    {
      current_value = *values++;
      /* Datatype */
      if ((error = memwriter_putc(state->memwriter, tobson_datatype_to_byte['b'])) != ERROR_NONE)
        {
          return error;
        }
      /* Key */
      if ((error = memwriter_putc(state->memwriter, key++)) != ERROR_NONE)
        {
          return error;
        }
      /* End Of Key */
      if ((error = memwriter_putc(state->memwriter, null)) != ERROR_NONE)
        {
          return error;
        }
      /* Value */
      if ((error = tobson_stringify_bool_value(state->memwriter, current_value)) != ERROR_NONE)
        {
          return error;
        }

      --remaining_elements;
    }
  /* write array end */
  if ((error = memwriter_putc(state->memwriter, null)) != ERROR_NONE)
    {
      return error;
    }
  /* Set length of object*/
  int_to_bytes(state->memwriter->size - size_before, &length_as_bytes);
  memcpy(placeholder_pos, length_as_bytes, 4);
  free(length_as_bytes);
  if (state->shared->data_ptr != NULL)
    {
      state->shared->data_ptr = ((int **)state->shared->data_ptr) + 1;
      state->shared->data_offset += sizeof(int *);
    }
  state->shared->wrote_output = 1;
  return error;
}

err_t tobson_stringify_args(tobson_state_t *state)
{
  grm_args_t *value;
  err_t error = ERROR_NONE;
  if (state->shared->data_ptr != NULL && state->shared->apply_padding)
    {
      ptrdiff_t needed_padding = state->shared->data_offset % sizeof(grm_args_t *);
      state->shared->data_ptr = ((char *)state->shared->data_ptr) + needed_padding;
      state->shared->data_offset += needed_padding;
    }
  if (state->shared->data_ptr != NULL)
    {
      value = *((grm_args_t **)state->shared->data_ptr);
      state->shared->data_ptr = ((grm_args_t **)state->shared->data_ptr) + 1;
      state->shared->data_offset += sizeof(grm_args_t *);
    }
  else
    {
      value = va_arg(*state->shared->vl, grm_args_t *);
    }
  if ((error = tobson_stringify_args_value(state->memwriter, value)) != ERROR_NONE)
    {
      return error;
    }
  state->shared->wrote_output = 1;
  return error;
}

err_t tobson_stringify_args_array(tobson_state_t *state)
{
  grm_args_t **values;
  grm_args_t *current_value;
  unsigned int length;
  int remaining_elements;
  err_t error = ERROR_NONE;
  char *length_as_bytes;
  char length_placeholder[4] = {0x01, 0x01, 0x01, 0x01};
  int size_before = state->memwriter->size;
  char *placeholder_pos = (state->memwriter->buf) + (state->memwriter->size);
  char key = '0';
  if (state->shared->data_ptr != NULL)
    {
      if (state->shared->data_ptr != NULL && state->shared->apply_padding)
        {
          ptrdiff_t needed_padding = state->shared->data_offset % sizeof(grm_args_t **);
          state->shared->data_ptr = ((char *)state->shared->data_ptr) + needed_padding;
          state->shared->data_offset += needed_padding;
        }
      values = *(grm_args_t ***)state->shared->data_ptr;
    }
  else
    {
      values = va_arg(*state->shared->vl, grm_args_t **);
    }
  if (state->additional_type_info != NULL)
    {
      if (!str_to_uint(state->additional_type_info, &length))
        {
          debug_print_error(("The given array length \"%s\" is no valid number; the array contents will be ignored.",
                             state->additional_type_info));
          length = 0;
        }
    }
  else
    {
      length = state->shared->array_length;
    }
  remaining_elements = length;
  /* write array start */
  /* Placeholder for length of array*/
  if ((error = memwriter_puts_with_len(state->memwriter, length_placeholder, 4)) != ERROR_NONE)
    {
      return error;
    }
  /* write array content */
  while (remaining_elements)
    {
      current_value = *values++;

      /* Datatype */
      if ((error = memwriter_putc(state->memwriter, tobson_datatype_to_byte['a'])) != ERROR_NONE)
        {
          return error;
        }
      /* Key */
      if ((error = memwriter_putc(state->memwriter, key++)) != ERROR_NONE)
        {
          return error;
        }
      /* End Of Key */
      if ((error = memwriter_putc(state->memwriter, null)) != ERROR_NONE)
        {
          return error;
        }
      /* Value */
      if ((error = tobson_stringify_args_value(state->memwriter, current_value)) != ERROR_NONE)
        {
          return error;
        }
      --remaining_elements;
    }
  /* write array end */
  if ((error = memwriter_putc(state->memwriter, null)) != ERROR_NONE)
    {
      return error;
    }
  /* Set length of object*/
  int_to_bytes(state->memwriter->size - size_before, &length_as_bytes);
  memcpy(placeholder_pos, length_as_bytes, 4);
  free(length_as_bytes);

  if (state->shared->data_ptr != NULL)
    {
      state->shared->data_ptr = ((grm_args_t ***)state->shared->data_ptr) + 1;
      state->shared->data_offset += sizeof(grm_args_t **);
    }
  state->shared->wrote_output = 1;
  return error;
}

err_t tobson_stringify_double_value(memwriter_t *memwriter, double value)
{
  err_t error;
  char *bytes;

  double_to_bytes(value, &bytes);
  error = memwriter_puts_with_len(memwriter, bytes, sizeof(double));
  free(bytes);

  return error;
}

err_t tobson_stringify_char_array(tobson_state_t *state)
{
  char *chars;
  char *escaped_chars = NULL;
  unsigned int length;
  err_t error = ERROR_NONE;

  if (state->shared->data_ptr != NULL)
    {
      if (state->shared->data_ptr != NULL && state->shared->apply_padding)
        {
          ptrdiff_t needed_padding = state->shared->data_offset % sizeof(char *);
          state->shared->data_ptr = ((char *)state->shared->data_ptr) + needed_padding;
          state->shared->data_offset += needed_padding;
        }
      chars = *(char **)state->shared->data_ptr;
    }
  else
    {
      chars = va_arg(*state->shared->vl, char *);
    }

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
  if ((error = tobson_escape_special_chars(&escaped_chars, chars, &length)) != ERROR_NONE)
    {
      goto cleanup;
    }
  if ((error = memwriter_printf(state->memwriter, "\"%.*s\"", length, escaped_chars)) != ERROR_NONE)
    {
      goto cleanup;
    }
  if ((error = tobson_stringify_string_value(state->memwriter, escaped_chars)) != ERROR_NONE)
    {
      goto cleanup;
    }
  state->shared->wrote_output = 1;

  if (state->shared->data_ptr != NULL)
    {
      state->shared->data_ptr = ((char **)state->shared->data_ptr) + 1;
      state->shared->data_offset += sizeof(char *);
    }

cleanup:
  free(escaped_chars);
  return error;
}

err_t tobson_stringify_string_value(memwriter_t *memwriter, char *value)
{
  char *escaped_chars = NULL;
  unsigned int length = 0;
  err_t error = ERROR_NONE;
  char *length_as_bytes;

  if ((error = tobson_escape_special_chars(&escaped_chars, value, &length)))
    {
      goto cleanup;
    }
  int_to_bytes(length + 1, &length_as_bytes); /* plus one for the null byte */
  if ((error = memwriter_puts_with_len(memwriter, length_as_bytes, sizeof(int))) != ERROR_NONE)
    {
      goto cleanup;
    }
  if ((error = memwriter_printf(memwriter, "%s", escaped_chars)) != ERROR_NONE)
    {
      goto cleanup;
    }
  if ((error = memwriter_putc(memwriter, null)) != ERROR_NONE)
    {
      goto cleanup;
    }

cleanup:
  free(escaped_chars);
  free(length_as_bytes);
  return error;
}

err_t tobson_stringify_bool_value(memwriter_t *memwriter, int value)
{
  return memwriter_putc(memwriter, value ? (char)0x01 : (char)0x00);
}

err_t tobson_stringify_object(tobson_state_t *state)
{
  char **member_names = NULL;
  char **data_types = NULL;
  char **member_name_ptr;
  char **data_type_ptr;
  int has_members;
  err_t error = ERROR_NONE;

  /* IMPORTANT: additional_type_info is altered after the unzip call! */
  if ((error = tobson_unzip_membernames_and_datatypes(state->additional_type_info, &member_names, &data_types)) !=
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
      if (!state->shared->add_data)
        {
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
          /* Datatype */
          if ((error = memwriter_putc(state->memwriter, tobson_datatype_to_byte[**data_type_ptr])) != ERROR_NONE)
            {
              goto cleanup;
            }
          /* Key */
          if ((error = memwriter_printf(state->memwriter, "%s", *member_name_ptr)) != ERROR_NONE)
            {
              goto cleanup;
            }
          /* End Of Key */
          if ((error = memwriter_putc(state->memwriter, null)) != ERROR_NONE)
            {
              goto cleanup;
            }
          /* Values */
          if ((error = tobson_serialize(state->memwriter, *data_type_ptr, NULL, NULL, -1, -1, 0, NULL, NULL,
                                        state->shared)) != ERROR_NONE)
            {
              goto cleanup;
            }
          ++member_name_ptr;
          ++data_type_ptr;
          if (*member_name_ptr == NULL || *data_type_ptr == NULL)
            {
              serialized_all_members = 1;
            }
        }
    }
  /* write object end if the type info is complete */
  if (!state->is_type_info_incomplete)
    {
      --(state->shared->struct_nested_level);
      if ((error = memwriter_putc(state->memwriter, null)) != ERROR_NONE)
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

err_t tobson_stringify_args_value(memwriter_t *memwriter, grm_args_t *args)
{
  err_t error = ERROR_NONE;

  tobson_permanent_state.serial_result = incomplete_at_struct_beginning;
  if ((error = tobson_write_args(memwriter, args)) != ERROR_NONE)
    {
      return error;
    }

  return ERROR_NONE;
}

int tobson_get_member_count(const char *data_desc)
{
  int nested_level = 0;
  int member_count = 0;
  if (data_desc == NULL || *data_desc == '\0')
    {
      return 0;
    }
  while (*data_desc != 0)
    {
      switch (*data_desc)
        {
        case '(':
          ++nested_level;
          break;
        case ')':
          --nested_level;
          break;
        case ',':
          ++member_count;
          break;
        default:
          break;
        }
      ++data_desc;
    }
  ++member_count; /* add last member (because it is not terminated by a ',') */
  return member_count;
}

int tobson_is_bson_array_needed(const char *data_desc)
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

void tobson_read_datatype(tobson_state_t *state)
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

err_t tobson_skip_bytes(tobson_state_t *state)
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

err_t tobson_close_object(tobson_state_t *state)
{
  err_t error;
  --(state->shared->struct_nested_level);
  if ((error = memwriter_putc(state->memwriter, null)) != ERROR_NONE)
    {
      return error;
    }
  return ERROR_NONE;
}

err_t tobson_read_array_length(tobson_state_t *state)
{
  int value;

  if (state->shared->data_ptr != NULL && state->shared->apply_padding)
    {
      ptrdiff_t needed_padding = state->shared->data_offset % sizeof(size_t);
      state->shared->data_ptr = ((char *)state->shared->data_ptr) + needed_padding;
      state->shared->data_offset += needed_padding;
    }
  if (state->shared->data_ptr != NULL)
    {
      value = *((size_t *)state->shared->data_ptr);
      state->shared->data_ptr = ((size_t *)state->shared->data_ptr) + 1;
      state->shared->data_offset += sizeof(size_t);
    }
  else
    {
      value = va_arg(*state->shared->vl, size_t);
    }
  state->shared->array_length = value;

  return ERROR_NONE;
}

err_t tobson_unzip_membernames_and_datatypes(char *mixed_ptr, char ***member_name_ptr, char ***data_type_ptr)
{
  int member_count;
  char **arrays[2];

  member_count = tobson_get_member_count(mixed_ptr);
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

err_t tobson_escape_special_chars(char **escaped_string, const char *unescaped_string, unsigned int *length)
{
  /* characters '\' and '"' must be escaped before written to a bson string value */
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

err_t tobson_serialize(memwriter_t *memwriter, char *data_desc, const void *data, va_list *vl, int apply_padding,
                       int add_data, int add_data_without_separator, unsigned int *struct_nested_level,
                       tojson_serialization_result_t *serial_result, tobson_shared_state_t *shared_state)
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

  tobson_state_t state;
  int bson_array_needed = 0;
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
      shared_state = malloc(sizeof(tobson_shared_state_t));
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

  bson_array_needed = tobson_is_bson_array_needed(data_desc);
  /* write list head if needed */
  while (*state.data_type_ptr != 0)
    {
      shared_state->wrote_output = 0;
      tobson_read_datatype(&state);
      if (tobson_datatype_to_func[(unsigned char)state.current_data_type])
        {
          error = tobson_datatype_to_func[(unsigned char)state.current_data_type](&state);
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

void tobson_init_static_variables(void)
{
  if (!tobson_static_variables_initialized)
    {
      tobson_datatype_to_func['n'] = tobson_read_array_length;
      tobson_datatype_to_func['e'] = tobson_skip_bytes;
      tobson_datatype_to_func['i'] = tobson_stringify_int;
      tobson_datatype_to_func['I'] = tobson_stringify_int_array;
      tobson_datatype_to_func['d'] = tobson_stringify_double;
      tobson_datatype_to_func['D'] = tobson_stringify_double_array;
      tobson_datatype_to_func['c'] = tobson_stringify_char;
      tobson_datatype_to_func['C'] = tobson_stringify_char_array;
      tobson_datatype_to_func['s'] = tobson_stringify_string;
      tobson_datatype_to_func['S'] = tobson_stringify_string_array;
      tobson_datatype_to_func['b'] = tobson_stringify_bool;
      tobson_datatype_to_func['B'] = tobson_stringify_bool_array;
      tobson_datatype_to_func['o'] = tobson_stringify_object;
      tobson_datatype_to_func['a'] = tobson_stringify_args;
      tobson_datatype_to_func['A'] = tobson_stringify_args_array;
      tobson_datatype_to_func[')'] = tobson_close_object;

      tobson_datatype_to_byte['d'] = 0x01;
      tobson_datatype_to_byte['s'] = 0x02;
      tobson_datatype_to_byte['c'] = 0x02;
      tobson_datatype_to_byte['a'] = 0x03;
      tobson_datatype_to_byte['n'] = 0x04;
      tobson_datatype_to_byte['b'] = 0x08;
      tobson_datatype_to_byte['i'] = 0x10; /* 32 bit integer */

      tobson_static_variables_initialized = 1;
    }
}

err_t tobson_init_variables(int *add_data, int *add_data_without_separator, char **_data_desc, const char *data_desc)
{
  tobson_init_static_variables();
  *add_data = (tobson_permanent_state.serial_result != complete);
  *add_data_without_separator = (tobson_permanent_state.serial_result == incomplete_at_struct_beginning);
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

err_t tobson_write_vl(memwriter_t *memwriter, const char *data_desc, va_list *vl)
{
  int add_data, add_data_without_separator;
  char *_data_desc;
  err_t error;

  error = tobson_init_variables(&add_data, &add_data_without_separator, &_data_desc, data_desc);
  if (!error)
    {
      error =
          tobson_serialize(memwriter, _data_desc, NULL, vl, 0, add_data, add_data_without_separator,
                           &tobson_permanent_state.struct_nested_level, &tobson_permanent_state.serial_result, NULL);
    }
  free(_data_desc);

  return error;
}

err_t tobson_write_buf(memwriter_t *memwriter, const char *data_desc, const void *buffer, int apply_padding)
{
  int add_data, add_data_without_separator;
  char *_data_desc;
  err_t error;

  error = tobson_init_variables(&add_data, &add_data_without_separator, &_data_desc, data_desc);
  if (!error)
    {
      error =
          tobson_serialize(memwriter, _data_desc, buffer, NULL, apply_padding, add_data, add_data_without_separator,
                           &tobson_permanent_state.struct_nested_level, &tobson_permanent_state.serial_result, NULL);
    }
  free(_data_desc);

  return error;
}

err_t tobson_write_arg(memwriter_t *memwriter, const arg_t *arg)
{
  err_t error = ERROR_NONE;

  if (arg->key == NULL)
    {
      if ((error = tobson_write_buf(memwriter, arg->value_format, arg->value_ptr, 1)) != ERROR_NONE)
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
      if ((error = tobson_write_buf(memwriter, format, arg->value_ptr, 1)) != ERROR_NONE)
        {
          free(format);
          return error;
        }
      free(format);
    }

  return error;
}

err_t tobson_write_args(memwriter_t *memwriter, const grm_args_t *args)
{
  const char *key_hierarchy_name;
  grm_args_iterator_t *it;
  arg_t *arg;
  err_t error;
  char *length_as_bytes;
  char length_placeholder[4] = {0x01, 0x01, 0x01, 0x01};
  int size_before = memwriter->size;
  char *placeholder_pos = (memwriter->buf) + (memwriter->size);

  it = grm_args_iter(args);
  if ((arg = it->next(it)))
    {
      /* Placeholder for length of object */
      if ((error = memwriter_puts_with_len(memwriter, length_placeholder, 4)) != ERROR_NONE)
        {
          return error;
        }

      tobson_write_buf(memwriter, "o(", NULL, 1);
      do
        {
          tobson_write_arg(memwriter, arg);
        }
      while ((arg = it->next(it)));
      tobson_write_buf(memwriter, ")", NULL, 1);

      /* Set length of object */
      int_to_bytes(memwriter->size - size_before, &length_as_bytes);
      memcpy(placeholder_pos, length_as_bytes, 4);
      free(length_as_bytes);
    }
  args_iterator_delete(it);

  return 0;
}

int tobson_is_complete(void)
{
  return tobson_permanent_state.serial_result == complete;
}

int tobson_struct_nested_level(void)
{
  return tobson_permanent_state.struct_nested_level;
}
