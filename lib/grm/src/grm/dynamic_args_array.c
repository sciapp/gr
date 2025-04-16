/* ######################### includes ############################################################################### */

#include "dynamic_args_array_int.h"


/* ######################### internal implementation ################################################################ */

/* ========================= methods ================================================================================ */

/* ------------------------- dynamic args array --------------------------------------------------------------------- */

DynamicArgsArray *dynamicArgsArrayNew(void)
{
  DynamicArgsArray *args_array;

  args_array = malloc(sizeof(DynamicArgsArray));
  if (args_array == NULL) return NULL;
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

void dynamicArgsArrayDelete(DynamicArgsArray *args_array)
{
  free(args_array->buf);
  free(args_array);
}

void dynamicArgsArrayDeleteWithElements(DynamicArgsArray *args_array)
{
  size_t i;
  for (i = 0; i < args_array->size; ++i)
    {
      grm_args_delete(args_array->buf[i]);
    }
  dynamicArgsArrayDelete(args_array);
}

grm_error_t dynamicArgsArrayPushBack(DynamicArgsArray *args_array, grm_args_t *args)
{
  if (args_array->size == args_array->capacity)
    {
      grm_args_t **enlarged_buf =
          realloc(args_array->buf, (args_array->capacity + DYNAMIC_ARGS_ARRAY_SIZE_INCREMENT) * sizeof(grm_args_t *));
      if (enlarged_buf == NULL) return GRM_ERROR_MALLOC;
      args_array->buf = enlarged_buf;
      args_array->capacity += DYNAMIC_ARGS_ARRAY_SIZE_INCREMENT;
    }
  args_array->buf[args_array->size] = args;
  ++args_array->size;

  return GRM_ERROR_NONE;
}
