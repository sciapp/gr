#ifndef GRM_DYNAMIC_ARGS_ARRAY_INT_H_INCLUDED
#define GRM_DYNAMIC_ARGS_ARRAY_INT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#include <grm/args.h>
#include "error_int.h"


/* ######################### internal interface ##################################################################### */

/* ========================= macros ================================================================================= */

/* ------------------------- dynamic args array --------------------------------------------------------------------- */

#define DYNAMIC_ARGS_ARRAY_INITIAL_SIZE 10
#define DYNAMIC_ARGS_ARRAY_SIZE_INCREMENT 10


/* ========================= datatypes ============================================================================== */

/* ------------------------- dynamic args array --------------------------------------------------------------------- */

typedef struct
{
  grm_args_t **buf;
  size_t size;
  size_t capacity;
} DynamicArgsArray;

/* ========================= methods ================================================================================ */

/* ------------------------- dynamic args array --------------------------------------------------------------------- */

DynamicArgsArray *dynamicArgsArrayNew(void);
void dynamicArgsArrayDelete(DynamicArgsArray *args_array);
void dynamicArgsArrayDeleteWithElements(DynamicArgsArray *args_array);
grm_error_t dynamicArgsArrayPushBack(DynamicArgsArray *args_array, grm_args_t *args);


#ifdef __cplusplus
}
#endif
#endif /* ifndef GRM_DYNAMIC_ARGS_ARRAY_INT_H_INCLUDED */
