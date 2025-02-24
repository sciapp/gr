#ifdef __unix__
#define _POSIX_C_SOURCE 200112L
#endif

/* ######################### includes ############################################################################### */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>

#include "error_int.h"
#include "bson_int.h"
#include "plot_int.h"


/* ######################### internal implementation ################################################################ */

/* ========================= macros ================================================================================= */

#define isLittleEndian() (*(char *)&(int){1})
#define OPT_ARRAYS 1

/* ------------------------- general -------------------------------------------------------------------------------- */

#ifndef DBL_DECIMAL_DIG
#define DBL_DECIMAL_DIG 17
#endif


/* ========================= static variables ======================================================================= */

/* ------------------------- bson deserializer ------------------------------------------------------------------------
 */

static grm_error_t (*from_bson_datatype_to_func[128])(FromBsonState *);
static int from_bson_static_variables_initialized = 0;

/* ------------------------- bson serializer ------------------------------------------------------------------------ */

static grm_error_t (*to_bson_datatype_to_func[128])(ToBsonState *);
static int to_bson_static_variables_initialized = 0;
static ToBsonPermanentState to_bson_permanent_state = {COMPLETE, 0, NULL};
static char to_bson_datatype_to_byte[128];
static char null = 0x00;


/* ========================= small helper functions ================================================================= */

void revMemCpy(void *dest, const void *src, size_t len)
{
  char *d = (char *)dest + len - 1;
  const char *s = src;
  while (len--)
    {
      *d-- = *s++;
    }
}

void memCpyRevChunks(void *dest, const void *src, size_t len, size_t chunk_size)
{
  char *d = dest;
  const char *s = src;
  int i, j;

  for (i = 0; i < len; i += chunk_size)
    {
      for (j = 0; j < chunk_size; j++)
        {
          d[i + chunk_size - j - 1] = s[i + j];
        }
    }
}

char byteToType(const char *byte)
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
    case (char)0x05:
      return 'x';
    case (char)0x08:
      return 'b';
    case (char)0x10:
      return 'i';
    default:
      return '\0';
    }
}

void intToBytes(int i, char **bytes)
{
  *bytes = (char *)malloc(sizeof(int) * sizeof(char));
  if (isLittleEndian())
    {
      memcpy(*bytes, &i, sizeof(i));
    }
  else
    {
      revMemCpy(*bytes, &i, sizeof(i));
    }
}

void bytesToInt(int *i, const char *bytes)
{
  if (isLittleEndian())
    {
      memcpy(i, bytes, sizeof(int));
    }
  else
    {
      revMemCpy(i, bytes, sizeof(int));
    }
}

void doubleToBytes(double d, char **bytes)
{
  *bytes = (char *)malloc(sizeof(double) * sizeof(char));
  if (isLittleEndian())
    {
      memcpy(*bytes, &d, sizeof(d));
    }
  else
    {
      revMemCpy(*bytes, &d, sizeof(d));
    }
}

void bytesToDouble(double *d, const char *bytes)
{
  if (isLittleEndian())
    {
      memcpy(d, bytes, sizeof(double));
    }
  else
    {
      revMemCpy(d, bytes, sizeof(double));
    }
}

/* ------------------------- bson deserializer ---------------------------------------------------------------------- */

grm_error_t fromBsonRead(grm_args_t *args, const char *bson_bytes)
{
  int document_size;
  FromBsonState state;
  FromBsonObjectInfos object_infos;
  grm_error_t error = GRM_ERROR_NONE;

  fromBsonInitStaticVariables();
  state.num_read_bytes = 0;
  state.cur_byte = bson_bytes;
  state.args = args;
  state.cur_value_buf = NULL;

  object_infos.num_bytes_read_before = 0;

  if ((error = fromBsonReadLength(&state, &document_size)) != GRM_ERROR_NONE) return error;

  object_infos.length = document_size;
  state.object_infos = &object_infos;

  if ((error = fromBsonReadObject(&state)) != GRM_ERROR_NONE) return error;

  return error;
}

grm_error_t fromBsonReadValueFormat(FromBsonState *state, char *value_format)
{
  *value_format = byteToType(state->cur_byte);
  state->num_read_bytes++;
  state->cur_byte++;

  return GRM_ERROR_NONE;
}

grm_error_t fromBsonReadKey(FromBsonState *state, const char **key)
{
  *key = state->cur_byte;

  while (*(state->cur_byte) != '\0')
    {
      ++(state->num_read_bytes);
      ++(state->cur_byte);
    }

  ++(state->num_read_bytes);
  ++(state->cur_byte);

  return GRM_ERROR_NONE;
}

grm_error_t fromBsonSkipKey(FromBsonState *state)
{
  while (*(state->cur_byte) != '\0')
    {
      ++(state->num_read_bytes);
      ++(state->cur_byte);
    }

  ++(state->num_read_bytes);
  ++(state->cur_byte);

  return GRM_ERROR_NONE;
}

grm_error_t fromBsonReadLength(FromBsonState *state, int *length)
{
  bytesToInt(length, state->cur_byte);
  state->cur_byte += sizeof(int);
  state->num_read_bytes += sizeof(int);

  return GRM_ERROR_NONE;
}

grm_error_t fromBsonReadDoubleValue(FromBsonState *state, double *d)
{
  bytesToDouble(d, state->cur_byte);

  state->num_read_bytes += sizeof(double);
  state->cur_byte += sizeof(double);

  return GRM_ERROR_NONE;
}

grm_error_t fromBsonReadIntValue(FromBsonState *state, int *i)
{
  bytesToInt(i, state->cur_byte);

  state->num_read_bytes += sizeof(int);
  state->cur_byte += sizeof(int);

  return GRM_ERROR_NONE;
}

grm_error_t fromBsonReadStringValue(FromBsonState *state, const char **s)
{
  *s = state->cur_byte;

  while (*(state->cur_byte) != '\0')
    {
      ++(state->num_read_bytes);
      ++(state->cur_byte);
    }

  ++(state->num_read_bytes);
  ++(state->cur_byte);

  return GRM_ERROR_NONE;
}

grm_error_t fromBsonReadBoolValue(FromBsonState *state, int *b)
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

  return GRM_ERROR_NONE;
}

grm_error_t fromBsonReadObject(FromBsonState *state)
{
  grm_error_t error = GRM_ERROR_NONE;
  int object_closed = 0;
  FromBsonObjectInfos *object_infos = state->object_infos;

  while (object_infos->length - (state->num_read_bytes - object_infos->num_bytes_read_before) > 0)
    {

      if ((error = fromBsonReadValueFormat(state, &(state->cur_value_format))) != GRM_ERROR_NONE) return error;
      if ((error = from_bson_datatype_to_func[state->cur_value_format](state)) != GRM_ERROR_NONE) return error;

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

  if (!object_closed) error = GRM_ERROR_PARSE_OBJECT;

  return error;
}

grm_error_t fromBsonParseDouble(FromBsonState *state)
{
  double d;
  int memory_allocated = 0;
  grm_error_t error;

  char cur_value_type[2] = "\0";
  cur_value_type[0] = state->cur_value_format;

  if ((error = fromBsonReadKey(state, &(state->cur_key))) != GRM_ERROR_NONE) goto cleanup;

  state->cur_value_buf = malloc(sizeof(double));
  if (state->cur_value_buf == NULL)
    {
      debugPrintMallocError();
      goto cleanup;
    }
  memory_allocated = 1;
  if ((error = fromBsonReadDoubleValue(state, &d)) != GRM_ERROR_NONE) goto cleanup;
  *((double *)state->cur_value_buf) = d;

  grm_args_push_buf(state->args, state->cur_key, cur_value_type, state->cur_value_buf, 0);

cleanup:
  if (memory_allocated) free(state->cur_value_buf);

  return error;
}

grm_error_t fromBsonParseInt(FromBsonState *state)
{
  int i;
  grm_error_t error;
  int memory_allocated = 0;

  char cur_value_type[2] = "\0";
  cur_value_type[0] = state->cur_value_format;

  if ((error = fromBsonReadKey(state, &(state->cur_key))) != GRM_ERROR_NONE) goto cleanup;

  state->cur_value_buf = malloc(sizeof(int));
  if (state->cur_value_buf == NULL)
    {
      debugPrintMallocError();
      goto cleanup;
    }
  memory_allocated = 1;
  if ((error = fromBsonReadIntValue(state, &i)) != GRM_ERROR_NONE) goto cleanup;
  *((int *)state->cur_value_buf) = i;

  grm_args_push_buf(state->args, state->cur_key, cur_value_type, state->cur_value_buf, 0);

cleanup:
  if (memory_allocated) free(state->cur_value_buf);

  return error;
}

grm_error_t fromBsonParseString(FromBsonState *state)
{
  int length;
  const char *s;
  grm_error_t error;
  int memory_allocated = 0;

  char cur_value_type[2] = "\0";
  cur_value_type[0] = state->cur_value_format;

  if ((error = fromBsonReadKey(state, &(state->cur_key))) != GRM_ERROR_NONE) goto cleanup;

  if ((error = fromBsonReadLength(state, &length)) != GRM_ERROR_NONE) return error;

  state->cur_value_buf = malloc(length * sizeof(char));
  if (state->cur_value_buf == NULL)
    {
      debugPrintMallocError();
      goto cleanup;
    }
  memory_allocated = 1;
  if ((error = fromBsonReadStringValue(state, &s)) != GRM_ERROR_NONE) goto cleanup;
  *((const char **)state->cur_value_buf) = s;

  grm_args_push_buf(state->args, state->cur_key, cur_value_type, state->cur_value_buf, 0);

cleanup:
  if (memory_allocated) free(state->cur_value_buf);

  return error;
}

grm_error_t fromBsonParseBool(FromBsonState *state)
{
  int b;
  grm_error_t error;
  int memory_allocated = 0;

  char cur_value_type[2] = "\0";
  cur_value_type[0] = state->cur_value_format;

  if ((error = fromBsonReadKey(state, &(state->cur_key))) != GRM_ERROR_NONE) goto cleanup;

  state->cur_value_buf = malloc(sizeof(int));
  if (state->cur_value_buf == NULL)
    {
      debugPrintMallocError();
      goto cleanup;
    }
  memory_allocated = 1;
  if ((error = fromBsonReadBoolValue(state, &b)) != GRM_ERROR_NONE) goto cleanup;
  *((int *)state->cur_value_buf) = b;

  grm_args_push_buf(state->args, state->cur_key, cur_value_type, state->cur_value_buf, 0);

cleanup:
  if (memory_allocated) free(state->cur_value_buf);

  return error;
}

grm_error_t fromBsonParseArray(FromBsonState *state)
{
  int length, num_bytes_read_before;
  char first_value_type;
  char final_value_type[3] = "\0";
  FromBsonArrayInfos array_infos;
  grm_error_t error;
  int memory_allocated = 0;

  final_value_type[0] = state->cur_value_format;
  if ((error = fromBsonReadKey(state, &(state->cur_key))) != GRM_ERROR_NONE) goto cleanup;

  num_bytes_read_before = state->num_read_bytes;
  if ((error = fromBsonReadLength(state, &length)) != GRM_ERROR_NONE) goto cleanup;

  /* read first value type without changing pointer */
  first_value_type = byteToType(state->cur_byte);
  final_value_type[1] = toupper(first_value_type);

  state->cur_value_format = first_value_type;
  array_infos.length = length;
  array_infos.num_bytes_read_before = num_bytes_read_before;
  state->array_infos = &array_infos;

  if ((error = from_bson_datatype_to_func[toupper(first_value_type)](state)) != GRM_ERROR_NONE) goto cleanup;
  memory_allocated = 1;

  grm_args_push(state->args, state->cur_key, final_value_type, state->array_infos->num_elements, state->cur_value_buf);

cleanup:
  if (memory_allocated) free(state->cur_value_buf);

  return error;
}

grm_error_t fromBsonParseOptimizedArray(FromBsonState *state)
{
  int length, num_elements, elem_size;
  char value_type;
  char final_value_type[3] = "\0";
  FromBsonArrayInfos array_infos;
  grm_error_t error;
  int memory_allocated = 0;
  int array_closed = 0;

  final_value_type[0] = 'n';
  if ((error = fromBsonReadKey(state, &(state->cur_key))) != GRM_ERROR_NONE) goto cleanup;

  if ((error = fromBsonReadLength(state, &length)) != GRM_ERROR_NONE) goto cleanup;

  /* checking subtype */
  if (*state->cur_byte != (char)0x80)
    {
      error = GRM_ERROR_UNSUPPORTED_DATATYPE;
      goto cleanup;
    }
  state->cur_byte++;
  state->num_read_bytes++;

  /* read value type */
  if ((error = fromBsonReadValueFormat(state, &value_type)) != GRM_ERROR_NONE) goto cleanup;
  final_value_type[1] = toupper(value_type);
  switch (value_type)
    {
    case 'i':
      elem_size = 4;
      break;
    case 'd':
      elem_size = 8;
      break;
    }

  state->cur_value_buf = malloc(length - 7); /* array length minus length, subtype, valuetype, null */
  if (state->cur_value_buf == NULL)
    {
      debugPrintMallocError();
      goto cleanup;
    }
  memory_allocated = 1;
  num_elements = (length - 7) / elem_size;

  /* copy values*/
  if (isLittleEndian())
    {
      memcpy(state->cur_value_buf, state->cur_byte, num_elements * elem_size);
    }
  else
    {
      memCpyRevChunks(state->cur_value_buf, state->cur_byte, num_elements * elem_size, elem_size);
    }
  state->cur_byte += num_elements * elem_size;
  state->num_read_bytes += num_elements * elem_size;

  /* check for the end */
  if (*(state->cur_byte) == '\0')
    {
      state->num_read_bytes++;
      state->cur_byte++;
      array_closed = 1;
    }

  if (!array_closed)
    {
      error = GRM_ERROR_PARSE_ARRAY;
      goto cleanup;
    }

  grm_args_push(state->args, state->cur_key, final_value_type, num_elements, state->cur_value_buf);

cleanup:
  if (memory_allocated) free(state->cur_value_buf);

  return error;
}

grm_error_t fromBsonParseObject(FromBsonState *state)
{
  int length;
  grm_error_t error;
  int num_bytes_read_before;
  grm_args_t *new_args = grm_args_new();
  FromBsonState inner_state;
  FromBsonObjectInfos object_infos;

  char cur_value_type[2] = "\0";
  cur_value_type[0] = state->cur_value_format;

  if ((error = fromBsonReadKey(state, &(state->cur_key))) != GRM_ERROR_NONE) return error;

  num_bytes_read_before = state->num_read_bytes;

  if ((error = fromBsonReadLength(state, &length)) != GRM_ERROR_NONE) return error;

  object_infos.length = length;
  object_infos.num_bytes_read_before = num_bytes_read_before;

  inner_state.num_read_bytes = state->num_read_bytes;
  inner_state.args = new_args;
  inner_state.cur_byte = state->cur_byte;
  inner_state.cur_value_buf = NULL;
  inner_state.object_infos = &object_infos;

  if ((error = fromBsonReadObject(&inner_state)) != GRM_ERROR_NONE) return error;

  state->num_read_bytes = inner_state.num_read_bytes;
  state->cur_byte = inner_state.cur_byte;
  grm_args_push(state->args, state->cur_key, cur_value_type, inner_state.args);

  return error;
}

grm_error_t fromBsonReadIntArray(FromBsonState *state)
{
  int i = 0;
  char next_value_type;
  int array_closed = 0;
  grm_error_t error = GRM_ERROR_NONE;
  int memory_allocated = 0;
  FromBsonArrayInfos *array_infos = state->array_infos;

  state->cur_value_buf = malloc(array_infos->length - 4); /* array length minus length bytes */
  if (state->cur_value_buf == NULL)
    {
      debugPrintMallocError();
      goto cleanup;
    }
  memory_allocated = 1;
  int current_value;

  while (array_infos->length - (state->num_read_bytes - array_infos->num_bytes_read_before) > 0)
    {
      if ((error = fromBsonReadValueFormat(state, &next_value_type)) != GRM_ERROR_NONE) goto cleanup;
      if (state->cur_value_format != next_value_type)
        {
          error = GRM_ERROR_PARSE_ARRAY;
          goto cleanup;
        }
      if ((error = fromBsonSkipKey(state)) != GRM_ERROR_NONE) goto cleanup;
      if ((error = fromBsonReadIntValue(state, &current_value)) != GRM_ERROR_NONE) goto cleanup;
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

  array_infos->num_elements = i;

  if (!array_closed) error = GRM_ERROR_PARSE_ARRAY;

cleanup:
  if (memory_allocated && error != GRM_ERROR_NONE) free(state->cur_value_buf);

  return error;
}

grm_error_t fromBsonReadDoubleArray(FromBsonState *state)
{
  int i = 0;
  char next_value_type;
  int array_closed = 0;
  grm_error_t error = GRM_ERROR_NONE;
  int memory_allocated = 0;
  FromBsonArrayInfos *array_infos = state->array_infos;

  state->cur_value_buf = malloc(array_infos->length - 4); /* array length minus length bytes */
  if (state->cur_value_buf == NULL)
    {
      debugPrintMallocError();
      goto cleanup;
    }
  memory_allocated = 1;
  double current_value;

  while (array_infos->length - (state->num_read_bytes - array_infos->num_bytes_read_before) > 0)
    {
      if ((error = fromBsonReadValueFormat(state, &next_value_type)) != GRM_ERROR_NONE) goto cleanup;
      if (state->cur_value_format != next_value_type)
        {
          error = GRM_ERROR_PARSE_ARRAY;
          goto cleanup;
        }
      if ((error = fromBsonSkipKey(state)) != GRM_ERROR_NONE) goto cleanup;
      if ((error = fromBsonReadDoubleValue(state, &current_value)) != GRM_ERROR_NONE) goto cleanup;
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

  array_infos->num_elements = i;

  if (!array_closed) error = GRM_ERROR_PARSE_ARRAY;

cleanup:
  if (memory_allocated && error != GRM_ERROR_NONE) free(state->cur_value_buf);

  return error;
}

grm_error_t fromBsonReadStringArray(FromBsonState *state)
{
  int i = 0;
  char next_value_type;
  int array_closed = 0;
  int string_length;
  grm_error_t error = GRM_ERROR_NONE;
  int memory_allocated = 0;
  FromBsonArrayInfos *array_infos = state->array_infos;

  state->cur_value_buf = malloc(array_infos->length - 4); /* array length minus length bytes */
  if (state->cur_value_buf == NULL)
    {
      debugPrintMallocError();
      goto cleanup;
    }
  memory_allocated = 1;
  const char *current_value;

  while (array_infos->length - (state->num_read_bytes - array_infos->num_bytes_read_before) > 0)
    {
      if ((error = fromBsonReadValueFormat(state, &next_value_type)) != GRM_ERROR_NONE) goto cleanup;
      if (state->cur_value_format != next_value_type)
        {
          error = GRM_ERROR_PARSE_ARRAY;
          goto cleanup;
        }
      if ((error = fromBsonSkipKey(state)) != GRM_ERROR_NONE) goto cleanup;
      if ((error = fromBsonReadLength(state, &string_length)) != GRM_ERROR_NONE) goto cleanup;
      if ((error = fromBsonReadStringValue(state, &current_value)) != GRM_ERROR_NONE) goto cleanup;
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

  if (!array_closed) error = GRM_ERROR_PARSE_ARRAY;

cleanup:
  if (memory_allocated && error != GRM_ERROR_NONE) free(state->cur_value_buf);

  return error;
}

grm_error_t fromBsonReadBoolArray(FromBsonState *state)
{
  int i = 0;
  char next_value_type;
  int array_closed = 0;
  grm_error_t error = GRM_ERROR_NONE;
  int memory_allocated = 0;
  FromBsonArrayInfos *array_infos = state->array_infos;

  state->cur_value_buf = malloc(array_infos->length - 4); /* array length minus length bytes */
  if (state->cur_value_buf == NULL)
    {
      debugPrintMallocError();
      goto cleanup;
    }
  memory_allocated = 1;
  int current_value;

  while (array_infos->length - (state->num_read_bytes - array_infos->num_bytes_read_before) > 0)
    {
      if ((error = fromBsonReadValueFormat(state, &next_value_type)) != GRM_ERROR_NONE) goto cleanup;
      if (state->cur_value_format != next_value_type)
        {
          error = GRM_ERROR_PARSE_ARRAY;
          goto cleanup;
        }
      if ((error = fromBsonSkipKey(state)) != GRM_ERROR_NONE) goto cleanup;
      if ((error = fromBsonReadBoolValue(state, &current_value)) != GRM_ERROR_NONE) goto cleanup;
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

  array_infos->num_elements = i;

  if (!array_closed) error = GRM_ERROR_PARSE_ARRAY;

cleanup:
  if (memory_allocated && error != GRM_ERROR_NONE) free(state->cur_value_buf);

  return error;
}

grm_error_t fromBsonReadObjectArray(FromBsonState *state)
{
  int i = 0;
  char next_value_type;
  int array_closed = 0;
  int cur_object_length;
  grm_error_t error = GRM_ERROR_NONE;
  int memory_allocated = 0;
  FromBsonArrayInfos *array_infos = state->array_infos;
  FromBsonState inner_state;
  FromBsonObjectInfos object_infos;
  int num_read_bytes_before;

  state->cur_value_buf = malloc(array_infos->length - 4); /* array length minus length bytes */
  if (state->cur_value_buf == NULL)
    {
      debugPrintMallocError();
      goto cleanup;
    }
  memory_allocated = 1;

  while (array_infos->length - (state->num_read_bytes - array_infos->num_bytes_read_before) > 0)
    {
      if ((error = fromBsonReadValueFormat(state, &next_value_type)) != GRM_ERROR_NONE) goto cleanup;
      if (state->cur_value_format != next_value_type)
        {
          error = GRM_ERROR_PARSE_ARRAY;
          goto cleanup;
        }
      if ((error = fromBsonSkipKey(state)) != GRM_ERROR_NONE) goto cleanup;
      num_read_bytes_before = state->num_read_bytes;
      if ((error = fromBsonReadLength(state, &cur_object_length)) != GRM_ERROR_NONE) goto cleanup;

      inner_state.args = grm_args_new();
      inner_state.cur_byte = state->cur_byte;
      inner_state.num_read_bytes = state->num_read_bytes;
      inner_state.cur_value_buf = NULL;

      object_infos.length = cur_object_length;
      object_infos.num_bytes_read_before = num_read_bytes_before;
      inner_state.object_infos = &object_infos;

      if ((error = fromBsonReadObject(&inner_state)) != GRM_ERROR_NONE) goto cleanup;

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

  if (!array_closed) error = GRM_ERROR_PARSE_ARRAY;

cleanup:
  if (memory_allocated && error != GRM_ERROR_NONE) free(state->cur_value_buf);

  return error;
}

void fromBsonInitStaticVariables(void)
{
  if (!from_bson_static_variables_initialized)
    {
      from_bson_datatype_to_func['n'] = fromBsonParseArray;
      from_bson_datatype_to_func['i'] = fromBsonParseInt;
      from_bson_datatype_to_func['I'] = fromBsonReadIntArray;
      from_bson_datatype_to_func['d'] = fromBsonParseDouble;
      from_bson_datatype_to_func['D'] = fromBsonReadDoubleArray;
      from_bson_datatype_to_func['s'] = fromBsonParseString;
      from_bson_datatype_to_func['S'] = fromBsonReadStringArray;
      from_bson_datatype_to_func['b'] = fromBsonParseBool;
      from_bson_datatype_to_func['B'] = fromBsonReadBoolArray;
      from_bson_datatype_to_func['a'] = fromBsonParseObject;
      from_bson_datatype_to_func['A'] = fromBsonReadObjectArray;
      from_bson_datatype_to_func['x'] = fromBsonParseOptimizedArray;

      from_bson_static_variables_initialized = 1;
    }
}

/* ------------------------- bson serializer ------------------------------------------------------------------------ */

grm_error_t toBsonIntValue(Memwriter *memwriter, int value)
{
  char *bytes;
  grm_error_t error;

  intToBytes(value, &bytes);

  error = memwriterPutsWithLen(memwriter, bytes, sizeof(int));
  free(bytes);

  return error;
}

grm_error_t toBsonDoubleValue(Memwriter *memwriter, double value)
{
  grm_error_t error;
  char *bytes;

  doubleToBytes(value, &bytes);
  error = memwriterPutsWithLen(memwriter, bytes, sizeof(double));
  free(bytes);

  return error;
}

grm_error_t toBsonStringValue(Memwriter *memwriter, char *value)
{
  grm_error_t error = GRM_ERROR_NONE;
  char *length_as_bytes;

  intToBytes(strlen(value) + 1, &length_as_bytes); /* plus one for the null byte */
  if ((error = memwriterPutsWithLen(memwriter, length_as_bytes, sizeof(int))) != GRM_ERROR_NONE) goto cleanup;
  if ((error = memwriterPrintf(memwriter, "%s", value)) != GRM_ERROR_NONE) goto cleanup;
  if ((error = memwriterPutc(memwriter, null)) != GRM_ERROR_NONE) goto cleanup;

cleanup:
  free(length_as_bytes);
  return error;
}

grm_error_t toBsonBoolValue(Memwriter *memwriter, int value)
{
  return memwriterPutc(memwriter, value ? (char)0x01 : (char)0x00);
}

grm_error_t toBsonCharValue(Memwriter *memwriter, char value)
{
  grm_error_t error;
  char length[4] = {(char)0x02, (char)0x00, (char)0x00, (char)0x00}; /*char is saved as string */
  if ((error = memwriterPutsWithLen(memwriter, length, 4)) != GRM_ERROR_NONE) return error;
  if ((error = memwriterPutc(memwriter, value)) != GRM_ERROR_NONE) return error;
  if ((error = memwriterPutc(memwriter, null)) != GRM_ERROR_NONE) return error;
  return GRM_ERROR_NONE;
}

grm_error_t toBsonArgsValue(Memwriter *memwriter, grm_args_t *args)
{
  grm_error_t error = GRM_ERROR_NONE;

  /* write object start */
  toBsonOpenObject(memwriter);

  to_bson_permanent_state.serial_result = INCOMPLETE_AT_STRUCT_BEGINNING;
  if ((error = toBsonWriteArgs(memwriter, args)) != GRM_ERROR_NONE) return error;

  return GRM_ERROR_NONE;
}

grm_error_t toBsonInt(ToBsonState *state)
{
  int value;
  grm_error_t error = GRM_ERROR_NONE;
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
  if ((error = toBsonIntValue(state->memwriter, value)) != GRM_ERROR_NONE) return error;
  state->shared->wrote_output = 1;
  return error;
}

grm_error_t toBsonDouble(ToBsonState *state)
{
  double value;
  grm_error_t error = GRM_ERROR_NONE;
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
  if ((error = toBsonDoubleValue(state->memwriter, value)) != GRM_ERROR_NONE) return error;
  state->shared->wrote_output = 1;
  return error;
}

grm_error_t toBsonChar(ToBsonState *state)
{
  char value;
  grm_error_t error = GRM_ERROR_NONE;
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
  if ((error = toBsonCharValue(state->memwriter, value)) != GRM_ERROR_NONE) return error;
  state->shared->wrote_output = 1;
  return error;
}

grm_error_t toBsonString(ToBsonState *state)
{
  char *value;
  grm_error_t error = GRM_ERROR_NONE;
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
  if ((error = toBsonStringValue(state->memwriter, value)) != GRM_ERROR_NONE) return error;
  state->shared->wrote_output = 1;
  return error;
}

grm_error_t toBsonBool(ToBsonState *state)
{
  int value;
  grm_error_t error = GRM_ERROR_NONE;
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
  if ((error = toBsonBoolValue(state->memwriter, value)) != GRM_ERROR_NONE) return error;
  state->shared->wrote_output = 1;
  return error;
}

grm_error_t toBsonArgs(ToBsonState *state)
{
  grm_args_t *value;
  grm_error_t error = GRM_ERROR_NONE;
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
  if ((error = toBsonArgsValue(state->memwriter, value)) != GRM_ERROR_NONE) return error;
  state->shared->wrote_output = 1;
  return error;
}

grm_error_t toBsonIntArray(ToBsonState *state)
{
  int *values;
  int current_value;
  unsigned int length;
  int remaining_elements;
  grm_error_t error = GRM_ERROR_NONE;
  char *length_as_bytes;
  char length_placeholder[4] = {0x01, 0x01, 0x01, 0x01};
  int size_before = state->memwriter->size;
  char *placeholder_pos;
  int key_num = 0;
  char *key;
  size_t num_digits;
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
      if (!strToUint(state->additional_type_info, &length))
        {
          debugPrintError(("The given array length \"%s\" is no valid number; the array contents will be ignored.",
                           state->additional_type_info));
          length = 0;
        }
    }
  else
    {
      length = state->shared->array_length;
    }
  remaining_elements = length;
  num_digits = log10(length) + 2; /* Max key length plus null byte */
  key = (char *)malloc(num_digits);
  /* write array start */
  if ((error = memwriterPutsWithLen(state->memwriter, length_placeholder, 4)) != GRM_ERROR_NONE) return error;
  /* write array content */
  while (remaining_elements)
    {
      current_value = *values++;
      /* Datatype */
      if ((error = memwriterPutc(state->memwriter, to_bson_datatype_to_byte['i'])) != GRM_ERROR_NONE) return error;
      /* Key */
      sprintf(key, "%d", key_num++);
      if ((error = memwriterPuts(state->memwriter, key)) != GRM_ERROR_NONE) return error;
      /* End Of Key */
      if ((error = memwriterPutc(state->memwriter, null)) != GRM_ERROR_NONE) return error;
      /* Value */
      if ((error = toBsonIntValue(state->memwriter, current_value)) != GRM_ERROR_NONE) return error;
      --remaining_elements;
    }
  /* write array end */
  if ((error = memwriterPutc(state->memwriter, null)) != GRM_ERROR_NONE) return error;
  /* Set length of object*/
  intToBytes(state->memwriter->size - size_before, &length_as_bytes);
  placeholder_pos = state->memwriter->buf + size_before;
  memcpy(placeholder_pos, length_as_bytes, 4);
  free(length_as_bytes);
  free(key);
  if (state->shared->data_ptr != NULL)
    {
      state->shared->data_ptr = ((int **)state->shared->data_ptr) + 1;
      state->shared->data_offset += sizeof(int *);
    }
  state->shared->wrote_output = 1;
  return error;
}

grm_error_t toBsonDoubleArray(ToBsonState *state)
{
  double *values;
  double current_value;
  unsigned int length;
  int remaining_elements;
  grm_error_t error = GRM_ERROR_NONE;
  char *length_as_bytes;
  char length_placeholder[4] = {0x01, 0x01, 0x01, 0x01};
  int size_before = state->memwriter->size;
  char *placeholder_pos;
  int key_num = 0;
  size_t num_digits;
  char *key;
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
      if (!strToUint(state->additional_type_info, &length))
        {
          debugPrintError(("The given array length \"%s\" is no valid number; the array contents will be ignored.",
                           state->additional_type_info));
          length = 0;
        }
    }
  else
    {
      length = state->shared->array_length;
    }
  remaining_elements = length;
  num_digits = log10(length) + 2; /* Max key length plus null byte */
  key = (char *)malloc(num_digits);
  /* write array start */
  /* Placeholder for length of array*/
  if ((error = memwriterPutsWithLen(state->memwriter, length_placeholder, 4)) != GRM_ERROR_NONE) return error;
  /* write array content */
  while (remaining_elements)
    {
      current_value = *values++;
      /* Datatype */
      if ((error = memwriterPutc(state->memwriter, to_bson_datatype_to_byte['d'])) != GRM_ERROR_NONE) return error;
      /* Key */
      sprintf(key, "%d", key_num++);
      if ((error = memwriterPutc(state->memwriter, *key)) != GRM_ERROR_NONE) return error;
      /* End Of Key */
      if ((error = memwriterPutc(state->memwriter, null)) != GRM_ERROR_NONE) return error;
      /* Value */
      if ((error = toBsonDoubleValue(state->memwriter, current_value)) != GRM_ERROR_NONE) return error;
      --remaining_elements;
    }
  /* write array end */
  if ((error = memwriterPutc(state->memwriter, null)) != GRM_ERROR_NONE) return error;
  /* Set length of object*/
  intToBytes(state->memwriter->size - size_before, &length_as_bytes);
  placeholder_pos = state->memwriter->buf + size_before;
  memcpy(placeholder_pos, length_as_bytes, 4);
  free(length_as_bytes);
  free(key);
  if (state->shared->data_ptr != NULL)
    {
      state->shared->data_ptr = ((double **)state->shared->data_ptr) + 1;
      state->shared->data_offset += sizeof(double *);
    }
  state->shared->wrote_output = 1;
  return error;
}

grm_error_t toBsonOptimizedArray(ToBsonState *state)
{
  double *values;
  unsigned int length;
  int total_length;
  char *total_length_as_bytes;
  grm_error_t error = GRM_ERROR_NONE;
  int elem_size;
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
      if (!strToUint(state->additional_type_info, &length))
        {
          debugPrintError(("The given array length \"%s\" is no valid number; the array contents will be ignored.",
                           state->additional_type_info));
          length = 0;
        }
    }
  else
    {
      length = state->shared->array_length;
    }
  /* length(4 bytes) + datatype user defined binary(1 byte) + datatype of values(1 byte) + nullbyte(1 byte) */
  switch (tolower(state->current_data_type))
    {
    case 'i':
      elem_size = 4;
      break;
    case 'd':
      elem_size = 8;
      break;
    }
  total_length = 7 + length * elem_size;
  intToBytes(total_length, &total_length_as_bytes);

  /* total Length */
  if ((error = memwriterPutsWithLen(state->memwriter, total_length_as_bytes, 4)) != GRM_ERROR_NONE) return error;

  /* datatype (user defined binary) */
  if ((error = memwriterPutc(state->memwriter, 0x80)) != GRM_ERROR_NONE) return error;

  /* datatype of values */
  if ((error = memwriterPutc(state->memwriter, to_bson_datatype_to_byte[tolower(state->current_data_type)])) !=
      GRM_ERROR_NONE)
    return error;

  /* values */
  if (isLittleEndian())
    {
      if ((error = memwriterMemcpy(state->memwriter, values, length * elem_size)) != GRM_ERROR_NONE) return error;
    }
  else
    {
      if ((error = memwriterMemcpyRevChunks(state->memwriter, values, length * elem_size, elem_size)) != GRM_ERROR_NONE)
        return error;
    }
  /* write array end */
  if ((error = memwriterPutc(state->memwriter, null)) != GRM_ERROR_NONE) return error;

  free(total_length_as_bytes);
  if (state->shared->data_ptr != NULL)
    {
      state->shared->data_ptr = ((double **)state->shared->data_ptr) + 1;
      state->shared->data_offset += sizeof(double *);
    }
  state->shared->wrote_output = 1;
  return error;
}

grm_error_t toBsonCharArray(ToBsonState *state)
{
  char *chars;
  unsigned int length;
  grm_error_t error = GRM_ERROR_NONE;

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
      if (!strToUint(state->additional_type_info, &length))
        {
          debugPrintError(("The given array length \"%s\" is no valid number; the array contents will be ignored.",
                           state->additional_type_info));
          return error;
        }
    }
  else
    {
      if (state->shared->read_length_from_string)
        {
          length = strlen(chars);
        }
      else
        {
          length = state->shared->array_length;
        }
    }
  /* TODO: Is it correct that `chars` is written twice here? */
  if ((error = memwriterPrintf(state->memwriter, "\"%.*s\"", length, chars)) != GRM_ERROR_NONE) return error;
  if ((error = toBsonStringValue(state->memwriter, chars)) != GRM_ERROR_NONE) return error;
  state->shared->wrote_output = 1;

  if (state->shared->data_ptr != NULL)
    {
      state->shared->data_ptr = ((char **)state->shared->data_ptr) + 1;
      state->shared->data_offset += sizeof(char *);
    }

  return error;
}

grm_error_t toBsonStringArray(ToBsonState *state)
{
  char **values;
  char *current_value;
  unsigned int length;
  int remaining_elements;
  grm_error_t error = GRM_ERROR_NONE;
  char *length_as_bytes;
  char length_placeholder[4] = {0x01, 0x01, 0x01, 0x01};
  int size_before = state->memwriter->size;
  char *placeholder_pos;
  int key_num = 0;
  size_t num_digits;
  char *key;
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
      if (!strToUint(state->additional_type_info, &length))
        {
          debugPrintError(("The given array length \"%s\" is no valid number; the array contents will be ignored.",
                           state->additional_type_info));
          length = 0;
        }
    }
  else
    {
      length = state->shared->array_length;
    }
  remaining_elements = length;
  num_digits = log10(length) + 2; /* Max key length plus null byte */
  key = (char *)malloc(num_digits);
  /* write array start */
  /* Placeholder for length of array*/
  if ((error = memwriterPutsWithLen(state->memwriter, length_placeholder, 4)) != GRM_ERROR_NONE) return error;
  /* write array content */
  while (remaining_elements)
    {
      current_value = *values++;
      /* Datatype */
      if ((error = memwriterPutc(state->memwriter, to_bson_datatype_to_byte['s'])) != GRM_ERROR_NONE) return error;
      /* Key */
      sprintf(key, "%d", key_num++);
      if ((error = memwriterPuts(state->memwriter, key)) != GRM_ERROR_NONE) return error;
      /* End Of Key */
      if ((error = memwriterPutc(state->memwriter, null)) != GRM_ERROR_NONE) return error;
      /* Value */
      if ((error = toBsonStringValue(state->memwriter, current_value)) != GRM_ERROR_NONE) return error;
      --remaining_elements;
    } /* write array end */
  if ((error = memwriterPutc(state->memwriter, null)) != GRM_ERROR_NONE) return error;
  /* Set length of object*/
  intToBytes(state->memwriter->size - size_before, &length_as_bytes);
  placeholder_pos = state->memwriter->buf + size_before;
  memcpy(placeholder_pos, length_as_bytes, 4);
  free(length_as_bytes);
  free(key);
  if (state->shared->data_ptr != NULL)
    {
      state->shared->data_ptr = ((char ***)state->shared->data_ptr) + 1;
      state->shared->data_offset += sizeof(char **);
    }
  state->shared->wrote_output = 1;
  return error;
}

grm_error_t toBsonBoolArray(ToBsonState *state)
{
  int *values;
  int current_value;
  unsigned int length;
  int remaining_elements;
  grm_error_t error = GRM_ERROR_NONE;
  char *length_as_bytes;
  char length_placeholder[4] = {0x01, 0x01, 0x01, 0x01};
  int size_before = state->memwriter->size;
  char *placeholder_pos;
  int key_num = 0;
  size_t num_digits;
  char *key;
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
      if (!strToUint(state->additional_type_info, &length))
        {
          debugPrintError(("The given array length \"%s\" is no valid number; the array contents will be ignored.",
                           state->additional_type_info));
          length = 0;
        }
    }
  else
    {
      length = state->shared->array_length;
    }
  remaining_elements = length;
  num_digits = log10(length) + 2; /* Max key length plus null byte */
  key = (char *)malloc(num_digits);
  /* write array start */
  /* Placeholder for length of array*/
  if ((error = memwriterPutsWithLen(state->memwriter, length_placeholder, 4)) != GRM_ERROR_NONE) return error;
  /* write array content */
  while (remaining_elements)
    {
      current_value = *values++;
      /* Datatype */
      if ((error = memwriterPutc(state->memwriter, to_bson_datatype_to_byte['b'])) != GRM_ERROR_NONE) return error;
      /* Key */
      sprintf(key, "%d", key_num++);
      if ((error = memwriterPuts(state->memwriter, key)) != GRM_ERROR_NONE) return error;
      /* End Of Key */
      if ((error = memwriterPutc(state->memwriter, null)) != GRM_ERROR_NONE) return error;
      /* Value */
      if ((error = toBsonBoolValue(state->memwriter, current_value)) != GRM_ERROR_NONE) return error;

      --remaining_elements;
    }
  /* write array end */
  if ((error = memwriterPutc(state->memwriter, null)) != GRM_ERROR_NONE) return error;
  /* Set length of object*/
  intToBytes(state->memwriter->size - size_before, &length_as_bytes);
  placeholder_pos = state->memwriter->buf + size_before;
  memcpy(placeholder_pos, length_as_bytes, 4);
  free(length_as_bytes);
  free(key);
  if (state->shared->data_ptr != NULL)
    {
      state->shared->data_ptr = ((int **)state->shared->data_ptr) + 1;
      state->shared->data_offset += sizeof(int *);
    }
  state->shared->wrote_output = 1;
  return error;
}

grm_error_t toBsonArgsArray(ToBsonState *state)
{
  grm_args_t **values;
  grm_args_t *current_value;
  unsigned int length;
  int remaining_elements;
  grm_error_t error = GRM_ERROR_NONE;
  char *length_as_bytes;
  char length_placeholder[4] = {0x01, 0x01, 0x01, 0x01};
  int size_before = state->memwriter->size;
  char *placeholder_pos;
  int key_num = 0;
  size_t num_digits;
  char *key;
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
      if (!strToUint(state->additional_type_info, &length))
        {
          debugPrintError(("The given array length \"%s\" is no valid number; the array contents will be ignored.",
                           state->additional_type_info));
          length = 0;
        }
    }
  else
    {
      length = state->shared->array_length;
    }
  remaining_elements = length;
  num_digits = log10(length) + 2; /* Max key length plus null byte */
  key = (char *)malloc(num_digits);
  /* write array start */
  /* Placeholder for length of array*/
  if ((error = memwriterPutsWithLen(state->memwriter, length_placeholder, 4)) != GRM_ERROR_NONE) return error;
  /* write array content */
  while (remaining_elements)
    {
      current_value = *values++;

      /* Datatype */
      if ((error = memwriterPutc(state->memwriter, to_bson_datatype_to_byte['a'])) != GRM_ERROR_NONE) return error;
      /* Key */
      sprintf(key, "%d", key_num++);
      if ((error = memwriterPuts(state->memwriter, key)) != GRM_ERROR_NONE) return error;
      /* End Of Key */
      if ((error = memwriterPutc(state->memwriter, null)) != GRM_ERROR_NONE) return error;
      /* Value */
      if ((error = toBsonArgsValue(state->memwriter, current_value)) != GRM_ERROR_NONE) return error;
      --remaining_elements;
    }
  /* write array end */
  if ((error = memwriterPutc(state->memwriter, null)) != GRM_ERROR_NONE) return error;
  /* Set length of object*/
  intToBytes(state->memwriter->size - size_before, &length_as_bytes);
  placeholder_pos = state->memwriter->buf + size_before;
  memcpy(placeholder_pos, length_as_bytes, 4);
  free(length_as_bytes);
  free(key);
  if (state->shared->data_ptr != NULL)
    {
      state->shared->data_ptr = ((grm_args_t ***)state->shared->data_ptr) + 1;
      state->shared->data_offset += sizeof(grm_args_t **);
    }
  state->shared->wrote_output = 1;
  return error;
}

grm_error_t toBsonObject(ToBsonState *state)
{
  char **member_names = NULL;
  char **data_types = NULL;
  char **member_name_ptr;
  char **data_type_ptr;
  int has_members;
  grm_error_t error = GRM_ERROR_NONE;

  /* IMPORTANT: additional_type_info is altered after the unzip call! */
  error = toBsonUnzipMemberNamesAndDatatypes(state->additional_type_info, &member_names, &data_types);
  cleanupIfError;

  member_name_ptr = member_names;
  data_type_ptr = data_types;

  has_members =
      (member_name_ptr != NULL && *member_name_ptr != NULL && data_type_ptr != NULL && *data_type_ptr != NULL);
  /* write object start */
  if (!state->add_data_without_separator)
    {
      if (!state->shared->add_data)
        {
          toBsonOpenObject(state->memwriter);
          ++(state->shared->struct_nested_level);
          cleanupIfError;
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
          /* check for optimization of arrays */
          if (OPT_ARRAYS)
            {
              if (**data_type_ptr == 'n')
                {
                  /* only optimize double and int */
                  if (strchr("DI", *((*data_type_ptr) + 1))) **data_type_ptr = 'x';
                }
            }
          /* Datatype */
          error = memwriterPutc(state->memwriter, to_bson_datatype_to_byte[**data_type_ptr]);
          cleanupIfError;
          /* Key */
          error = memwriterPrintf(state->memwriter, "%s", *member_name_ptr);
          cleanupIfError;
          /* End Of Key */
          error = memwriterPutc(state->memwriter, null);
          cleanupIfError;
          /* Values */
          error = toBsonSerialize(state->memwriter, *data_type_ptr, NULL, NULL, -1, -1, 0, NULL, NULL, state->shared);
          cleanupIfError;
          ++member_name_ptr;
          ++data_type_ptr;
          if (*member_name_ptr == NULL || *data_type_ptr == NULL) serialized_all_members = 1;
        }
    }
  /* write object end if the type info is complete */
  if (!state->is_type_info_incomplete)
    {
      error = toBsonCloseObject(state);
      cleanupIfError;
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

int toBsonGetMemberCount(const char *data_desc)
{
  int nested_level = 0;
  int member_count = 0;
  if (data_desc == NULL || *data_desc == '\0') return 0;
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

void toBsonReadDatatype(ToBsonState *state)
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

grm_error_t toBsonSkipBytes(ToBsonState *state)
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

grm_error_t toBsonOpenObject(Memwriter *memwriter)
{
  grm_error_t error = GRM_ERROR_NONE;

  const char length_placeholder[4] = {0x01, 0x01, 0x01, 0x01};
  if (to_bson_permanent_state.memwriter_object_start_offset_stack == NULL)
    {
      to_bson_permanent_state.memwriter_object_start_offset_stack = sizeTListNew();
      returnErrorIf(to_bson_permanent_state.memwriter_object_start_offset_stack == NULL, GRM_ERROR_MALLOC);
    }
  sizeTListPush(to_bson_permanent_state.memwriter_object_start_offset_stack, memwriterSize(memwriter));
  error = memwriterPutsWithLen(memwriter, (char *)length_placeholder, 4);

  return error;
}

grm_error_t toBsonCloseObject(ToBsonState *state)
{
  grm_error_t error;

  size_t size_before = sizeTListPop(to_bson_permanent_state.memwriter_object_start_offset_stack);
  char *length_as_bytes;
  char *placeholder_pos;

  /* Close object */
  if ((error = memwriterPutc(state->memwriter, null)) != GRM_ERROR_NONE) return error;

  /* Set length of object*/
  intToBytes(state->memwriter->size - size_before, &length_as_bytes);
  placeholder_pos = state->memwriter->buf + size_before;
  memcpy(placeholder_pos, length_as_bytes, 4);
  free(length_as_bytes);
  if (sizeTListEmpty(to_bson_permanent_state.memwriter_object_start_offset_stack))
    {
      sizeTListDelete(to_bson_permanent_state.memwriter_object_start_offset_stack);
      to_bson_permanent_state.memwriter_object_start_offset_stack = NULL;
    }

  --(state->shared->struct_nested_level);

  return GRM_ERROR_NONE;
}

grm_error_t toBsonReadArrayLength(ToBsonState *state)
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

  return GRM_ERROR_NONE;
}

grm_error_t toBsonUnzipMemberNamesAndDatatypes(char *mixed_ptr, char ***member_name_ptr, char ***data_type_ptr)
{
  int member_count;
  char **arrays[2];

  member_count = toBsonGetMemberCount(mixed_ptr);
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

grm_error_t toBsonSerialize(Memwriter *memwriter, char *data_desc, const void *data, va_list *vl, int apply_padding,
                            int add_data, int add_data_without_separator, unsigned int *struct_nested_level,
                            ToJsonSerializationResult *serial_result, ToBsonSharedState *shared_state)
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
  ToBsonState state;
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
      shared_state = malloc(sizeof(ToBsonSharedState));
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

  /* write list head if needed */
  while (*state.data_type_ptr != 0)
    {
      shared_state->wrote_output = 0;
      toBsonReadDatatype(&state);
      if (to_bson_datatype_to_func[(unsigned char)state.current_data_type])
        {
          error = to_bson_datatype_to_func[(unsigned char)state.current_data_type](&state);
        }
      else
        {
          debugPrintError(("WARNING: '%c' (ASCII code %d) is not a valid type identifier\n", state.current_data_type,
                           state.current_data_type));
          error = GRM_ERROR_UNSUPPORTED_DATATYPE;
        }
      if (error != GRM_ERROR_NONE) goto cleanup;
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

void toBsonInitStaticVariables(void)
{
  if (!to_bson_static_variables_initialized)
    {
      to_bson_datatype_to_func['n'] = toBsonReadArrayLength;
      to_bson_datatype_to_func['e'] = toBsonSkipBytes;
      to_bson_datatype_to_func['i'] = toBsonInt;
      if (OPT_ARRAYS)
        {
          to_bson_datatype_to_func['I'] = toBsonOptimizedArray;
        }
      else
        {
          to_bson_datatype_to_func['I'] = toBsonIntArray;
        }
      to_bson_datatype_to_func['d'] = toBsonDouble;
      if (OPT_ARRAYS)
        {
          to_bson_datatype_to_func['D'] = toBsonOptimizedArray;
        }
      else
        {
          to_bson_datatype_to_func['D'] = toBsonDoubleArray;
        }
      to_bson_datatype_to_func['c'] = toBsonChar;
      to_bson_datatype_to_func['C'] = toBsonCharArray;
      to_bson_datatype_to_func['s'] = toBsonString;
      to_bson_datatype_to_func['S'] = toBsonStringArray;
      to_bson_datatype_to_func['b'] = toBsonBool;
      to_bson_datatype_to_func['B'] = toBsonBoolArray;
      to_bson_datatype_to_func['o'] = toBsonObject;
      to_bson_datatype_to_func['a'] = toBsonArgs;
      to_bson_datatype_to_func['A'] = toBsonArgsArray;
      to_bson_datatype_to_func[')'] = toBsonCloseObject;
      /* user defined datatype (optimized array) */
      to_bson_datatype_to_func['x'] = toBsonReadArrayLength;

      to_bson_datatype_to_byte['d'] = 0x01;
      to_bson_datatype_to_byte['s'] = 0x02;
      to_bson_datatype_to_byte['c'] = 0x02;
      to_bson_datatype_to_byte['a'] = 0x03;
      to_bson_datatype_to_byte['n'] = 0x04;
      to_bson_datatype_to_byte['b'] = 0x08;
      to_bson_datatype_to_byte['i'] = 0x10;
      /* binary (optimized array) */
      to_bson_datatype_to_byte['x'] = 0x05;

      to_bson_static_variables_initialized = 1;
    }
}

grm_error_t toBsonInitVariables(int *add_data, int *add_data_without_separator, char **data_desc_priv,
                                const char *data_desc)
{
  toBsonInitStaticVariables();
  *add_data = (to_bson_permanent_state.serial_result != COMPLETE);
  *add_data_without_separator = (to_bson_permanent_state.serial_result == INCOMPLETE_AT_STRUCT_BEGINNING);
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

grm_error_t toBsonWrite(Memwriter *memwriter, const char *data_desc, ...)
{
  va_list vl;
  grm_error_t error;

  va_start(vl, data_desc);
  error = toBsonWriteVl(memwriter, data_desc, &vl);
  va_end(vl);

  return error;
}

grm_error_t toBsonWriteVl(Memwriter *memwriter, const char *data_desc, va_list *vl)
{
  int add_data, add_data_without_separator;
  char *data_desc_priv;
  grm_error_t error;

  error = toBsonInitVariables(&add_data, &add_data_without_separator, &data_desc_priv, data_desc);
  if (!error)
    {
      error =
          toBsonSerialize(memwriter, data_desc_priv, NULL, vl, 0, add_data, add_data_without_separator,
                          &to_bson_permanent_state.struct_nested_level, &to_bson_permanent_state.serial_result, NULL);
    }
  free(data_desc_priv);

  return error;
}

grm_error_t toBsonWriteBuf(Memwriter *memwriter, const char *data_desc, const void *buffer, int apply_padding)
{
  int add_data, add_data_without_separator;
  char *data_desc_priv;
  grm_error_t error;
  error = toBsonInitVariables(&add_data, &add_data_without_separator, &data_desc_priv, data_desc);
  if (!error)
    {
      error =
          toBsonSerialize(memwriter, data_desc_priv, buffer, NULL, apply_padding, add_data, add_data_without_separator,
                          &to_bson_permanent_state.struct_nested_level, &to_bson_permanent_state.serial_result, NULL);
    }
  free(data_desc_priv);

  return error;
}

grm_error_t toBsonWriteArg(Memwriter *memwriter, const grm_arg_t *arg)
{
  grm_error_t error = GRM_ERROR_NONE;

  if (arg->key == NULL)
    {
      if ((error = toBsonWriteBuf(memwriter, arg->value_format, arg->value_ptr, 1)) != GRM_ERROR_NONE) return error;
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
      if ((error = toBsonWriteBuf(memwriter, format, arg->value_ptr, 1)) != GRM_ERROR_NONE)
        {
          free(format);
          return error;
        }
      free(format);
    }

  return error;
}

grm_error_t toBsonWriteArgs(Memwriter *memwriter, const grm_args_t *args)
{
  grm_args_iterator_t *it;
  grm_arg_t *arg;

  it = grm_args_iter(args);
  if ((arg = it->next(it)))
    {
      toBsonWriteBuf(memwriter, "o(", NULL, 1);
      do
        {
          toBsonWriteArg(memwriter, arg);
        }
      while ((arg = it->next(it)));
      toBsonWriteBuf(memwriter, ")", NULL, 1);
    }
  argsIteratorDelete(it);

  return 0;
}

int toBsonIsComplete(void)
{
  return to_bson_permanent_state.serial_result == COMPLETE;
}

int toBsonStructNestedLevel(void)
{
  return to_bson_permanent_state.struct_nested_level;
}
