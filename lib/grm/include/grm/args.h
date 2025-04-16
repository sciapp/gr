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

struct _grm_arg_private_t;
typedef struct _grm_arg_private_t grm_arg_private_t;

typedef struct
{
  const char *key;
  void *value_ptr;
  const char *value_format;
  grm_arg_private_t *priv;
} grm_arg_t;


/* ------------------------- argument container --------------------------------------------------------------------- */

struct _grm_args_t;
typedef struct _grm_args_t grm_args_t;

typedef grm_args_t *grm_args_ptr_t;

/* ------------------------- argument iterator ---------------------------------------------------------------------- */

struct _grm_args_iterator_private_t;
typedef struct _grm_args_iterator_private_t grm_args_iterator_private_t;

typedef struct _grm_args_iterator_t
{
  grm_arg_t *(*next)(struct _grm_args_iterator_t *);
  grm_arg_t *arg;
  grm_args_iterator_private_t *priv;
} grm_args_iterator_t;

/* ------------------------- value iterator ------------------------------------------------------------------------- */

struct _grm_args_value_iterator_private_t;
typedef struct _grm_args_value_iterator_private_t grm_args_value_iterator_private_t;

typedef struct _grm_args_value_iterator_t
{
  void *(*next)(struct _grm_args_value_iterator_t *);
  void *value_ptr;
  char format;
  int is_array;
  size_t array_length;
  grm_args_value_iterator_private_t *priv;
} grm_args_value_iterator_t;


/* ========================= methods ================================================================================ */

/* ------------------------- argument ------------------------------------------------------------------------------- */

GRM_EXPORT grm_args_value_iterator_t *grm_arg_value_iter(const grm_arg_t *arg);

/* ------------------------- argument container --------------------------------------------------------------------- */

GRM_EXPORT grm_args_t *grm_args_new(void);
GRM_EXPORT void grm_args_delete(grm_args_t *args);

GRM_EXPORT int grm_args_push(grm_args_t *args, const char *key, const char *value_format, ...);
GRM_EXPORT int grm_args_push_buf(grm_args_t *args, const char *key, const char *value_format, const void *buffer,
                                 int apply_padding);

GRM_EXPORT int grm_args_contains(const grm_args_t *args, const char *keyword);

GRM_EXPORT int grm_args_first_value(const grm_args_t *args, const char *keyword, const char *first_value_format,
                                    void *first_value, unsigned int *array_length);
#define grm_args_first_value(args, keyword, first_value_format, first_value, array_length) \
  grm_args_first_value(args, keyword, first_value_format, (void *)first_value, array_length)
GRM_EXPORT int grm_args_values(const grm_args_t *args, const char *keyword, const char *expected_format, ...);

GRM_EXPORT void grm_args_clear(grm_args_t *args);
GRM_EXPORT void grm_args_remove(grm_args_t *args, const char *key);

GRM_EXPORT grm_args_iterator_t *grm_args_iter(const grm_args_t *args);


/* ------------------------- utilities ------------------------------------------------------------------------------ */

GRM_EXPORT grm_args_ptr_t grm_length(double value, const char *unit);


#ifdef __cplusplus
}
#endif
#endif /* ifndef GRM_ARGS_H_INCLUDED */
