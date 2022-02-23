/* ######################### includes ############################################################################### */

#include "dynamic_args_array_int.h"


/* ######################### internal implementation ################################################################ */

/* ========================= methods ================================================================================ */

/* ------------------------- dynamic args array --------------------------------------------------------------------- */

dynamic_args_array_t *dynamic_args_array_new(void)
{
  dynamic_args_array_t *args_array;

  args_array = malloc(sizeof(dynamic_args_array_t));
  if (args_array == NULL)
    {
      return NULL;
    }
  args_array->buf = malloc(DYNAMIC_ARGS_ARRAY_INITIAL_SIZE * sizeof(grm_args_t *));
  if (args_array->buf == NULL)
    {
      free(args_array);
      return NULL;
    }
  args_array->capacity = DYNAMIC_ARGS_ARRAY_INITIAL_SIZE;
  args_array->size = 0;

  return args_array;
}

void dynamic_args_array_delete(dynamic_args_array_t *args_array)
{
  free(args_array->buf);
  free(args_array);
}

void dynamic_args_array_delete_with_elements(dynamic_args_array_t *args_array)
{
  size_t i;
  for (i = 0; i < args_array->size; ++i)
    {
      grm_args_delete(args_array->buf[i]);
    }
  dynamic_args_array_delete(args_array);
}

err_t dynamic_args_array_push_back(dynamic_args_array_t *args_array, grm_args_t *args)
{
  if (args_array->size == args_array->capacity)
    {
      grm_args_t **enlarged_buf =
          realloc(args_array->buf, (args_array->capacity + DYNAMIC_ARGS_ARRAY_SIZE_INCREMENT) * sizeof(grm_args_t *));
      if (enlarged_buf == NULL)
        {
          return ERROR_MALLOC;
        }
      args_array->buf = enlarged_buf;
      args_array->capacity += DYNAMIC_ARGS_ARRAY_SIZE_INCREMENT;
    }
  args_array->buf[args_array->size] = args;
  ++args_array->size;

  return NO_ERROR;
}
