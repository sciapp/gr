#ifndef GRM_DYNAMIC_ARGS_ARRAY_INT_H_INCLUDED
#define GRM_DYNAMIC_ARGS_ARRAY_INT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#include <grm/args.h>
#include "grm/error.h"


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
} dynamic_args_array_t;

/* ========================= methods ================================================================================ */

/* ------------------------- dynamic args array --------------------------------------------------------------------- */

dynamic_args_array_t *dynamic_args_array_new(void);
void dynamic_args_array_delete(dynamic_args_array_t *args_array);
void dynamic_args_array_delete_with_elements(dynamic_args_array_t *args_array);
err_t dynamic_args_array_push_back(dynamic_args_array_t *args_array, grm_args_t *args);


#ifdef __cplusplus
}
#endif
#endif /* ifndef GRM_DYNAMIC_ARGS_ARRAY_INT_H_INCLUDED */
