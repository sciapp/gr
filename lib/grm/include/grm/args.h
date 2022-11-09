#ifndef GRM_ARGS_H_INCLUDED
#define GRM_ARGS_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#include <stdlib.h>

#include "util.h"


/* ######################### public interface ####################################################################### */

/* ========================= datatypes ============================================================================== */

/* ------------------------- argument ------------------------------------------------------------------------------- */

struct _arg_private_t;
typedef struct _arg_private_t arg_private_t;

typedef struct
{
  const char *key;
  void *value_ptr;
  const char *value_format;
  arg_private_t *priv;
} arg_t;


/* ------------------------- argument container --------------------------------------------------------------------- */

struct _args_node_t;
typedef struct _args_node_t args_node_t;

struct _grm_args_t;
typedef struct _grm_args_t grm_args_t;

typedef grm_args_t *grm_args_ptr_t;

/* ------------------------- argument iterator ---------------------------------------------------------------------- */

struct _args_iterator_private_t;
typedef struct _args_iterator_private_t args_iterator_private_t;

typedef struct _args_iterator_t
{
  arg_t *(*next)(struct _args_iterator_t *);
  arg_t *arg;
  args_iterator_private_t *priv;
} args_iterator_t;

/* ------------------------- value iterator ------------------------------------------------------------------------- */

struct _args_value_iterator_private_t;
typedef struct _args_value_iterator_private_t args_value_iterator_private_t;

typedef struct _grm_args_value_iterator_t
{
  void *(*next)(struct _grm_args_value_iterator_t *);
  void *value_ptr;
  char format;
  int is_array;
  size_t array_length;
  args_value_iterator_private_t *priv;
} args_value_iterator_t;


/* ========================= methods ================================================================================ */

/* ------------------------- argument container --------------------------------------------------------------------- */

EXPORT grm_args_t *grm_args_new(void);
EXPORT void grm_args_delete(grm_args_t *args);
EXPORT int grm_args_push(grm_args_t *args, const char *key, const char *value_format, ...);
EXPORT int grm_args_push_buf(grm_args_t *args, const char *key, const char *value_format, const void *buffer,
                             int apply_padding);
EXPORT int grm_args_contains(const grm_args_t *args, const char *keyword);
EXPORT void grm_args_clear(grm_args_t *args);
EXPORT void grm_args_remove(grm_args_t *args, const char *key);

/* ------------------------- utilities ------------------------------------------------------------------------------ */

EXPORT grm_args_ptr_t grm_length(double value, const char *unit);


#ifdef __cplusplus
}
#endif
#endif /* ifndef GRM_ARGS_H_INCLUDED */
