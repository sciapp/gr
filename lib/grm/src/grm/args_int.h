#ifndef GRM_ARGS_INT_H_INCLUDED
#define GRM_ARGS_INT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#include <stdarg.h>
#include <stddef.h>
#include <sys/types.h>

#include <grm/args.h>
#include "error_int.h"
#include "util_int.h"

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

/* ######################### internal interface ##################################################################### */

/* ========================= datatypes ============================================================================== */

/* ------------------------- argument ------------------------------------------------------------------------------- */

struct _grm_arg_private_t
{
  unsigned int reference_count;
};


/* ------------------------- argument parsing ----------------------------------------------------------------------- */

struct ArgparseState
{
  va_list *vl;
  const void *in_buffer;
  int apply_padding;
  ptrdiff_t data_offset;
  void *save_buffer;
  char current_format;
  int next_is_array;
  size_t default_array_length;
  ssize_t next_array_length;
  int dataslot_count;
};
typedef struct ArgparseState ArgparseState;

typedef void (*ReadParam)(ArgparseState *);
typedef void *(*CopyValue)(void *);
typedef void (*DeleteValue)(void *);


/* ------------------------- argument container --------------------------------------------------------------------- */

struct ArgsNode
{
  grm_arg_t *arg;
  struct ArgsNode *next;
};
typedef struct ArgsNode ArgsNode;

struct _grm_args_t
{
  ArgsNode *kwargs_head;
  ArgsNode *kwargs_tail;
  unsigned int count;
};

/* ------------------------- argument iterator ---------------------------------------------------------------------- */

struct _grm_args_iterator_private_t
{
  const ArgsNode *next_node;
  const ArgsNode *end;
};


/* ------------------------- value iterator ------------------------------------------------------------------------- */

struct _grm_args_value_iterator_private_t
{
  void *value_buffer;
  const char *value_format;
};


/* ========================= functions ============================================================================== */

/* ------------------------- argument parsing ----------------------------------------------------------------------- */

void *argparseReadParams(const char *format, const void *buffer, va_list *vl, int apply_padding, char **new_format);
void argparseReadInt(ArgparseState *state);
void argparseReadDouble(ArgparseState *state);
void argparseReadChar(ArgparseState *state);
void argparseReadString(ArgparseState *state);
void argparseReadDefaultArrayLength(ArgparseState *state);
void argparseReadCharArray(ArgparseState *state, int store_array_length);
void argparseInitStaticVariables(void);
size_t argparseCalculateNeededBufferSize(const char *format, int apply_padding);
size_t argparseCalculateNeededPadding(void *buffer, char current_format);
void argparseReadNextOption(ArgparseState *state, char **format);
const char *argparseSkipOption(const char *format);
char *argparseConvertToArray(ArgparseState *state);


/* ------------------------- argument container --------------------------------------------------------------------- */

grm_arg_t *argsCreateArgs(const char *key, const char *value_format, const void *buffer, va_list *vl,
                          int apply_padding);
int argsValidateFormatString(const char *format);
const char *argsSkipOption(const char *format);
void argsCopyFormatStringForArg(char *dst, const char *format);
void argsCopyFormatStringForParsing(char *dst, const char *format);
int argsCheckFormatCompatibility(const grm_arg_t *arg, const char *compatible_format);
void argsDecreaseArgReferenceCount(ArgsNode *args_node);


/* ------------------------- value copy ----------------------------------------------------------------------------- */

void *copyValue(char format, void *value_ptr);


/* ========================= methods ================================================================================ */

/* ------------------------- argument ------------------------------------------------------------------------------- */

grm_error_t argIncreaseArray(grm_arg_t *arg, size_t increment);

int argFirstValue(const grm_arg_t *arg, const char *first_value_format, void *first_value, unsigned int *array_length);
#define argFirstValue(arg, first_value_format, first_value, array_length) \
  argFirstValue(arg, first_value_format, (void *)first_value, array_length)
int argValues(const grm_arg_t *arg, const char *expected_format, ...);
int argValuesVl(const grm_arg_t *arg, const char *expected_format, va_list *vl);

/* ------------------------- argument container --------------------------------------------------------------------- */

void argsInit(grm_args_t *args);
void argsFinalize(grm_args_t *args);

grm_args_t *argsFlatCopy(const grm_args_t *args) UNUSED;
grm_args_t *argsCopy(const grm_args_t *copy_args);
grm_args_t *argsCopyExtended(const grm_args_t *copy_args, const char **keys_copy_as_array, const char **ignore_keys);

grm_error_t argsPushCommon(grm_args_t *args, const char *key, const char *value_format, const void *buffer, va_list *vl,
                           int apply_padding);
grm_error_t argsPushVl(grm_args_t *args, const char *key, const char *value_format, va_list *vl);
grm_error_t argsPushArg(grm_args_t *args, grm_arg_t *arg);
grm_error_t argsUpdateMany(grm_args_t *args, const grm_args_t *update_args) UNUSED;
grm_error_t argsMerge(grm_args_t *args, const grm_args_t *merge_args, const char *const *merge_keys);
grm_error_t argsSetDefaultCommon(grm_args_t *args, const char *key, const char *value_format, const void *buffer,
                                 va_list *vl, int apply_padding);
grm_error_t argsSetDefault(grm_args_t *args, const char *key, const char *value_format, ...);
grm_error_t argsSetDefaultBuf(grm_args_t *args, const char *key, const char *value_format, const void *buffer,
                              int apply_padding) UNUSED;
grm_error_t argsSetDefaultVl(grm_args_t *args, const char *key, const char *value_format, va_list *vl);

void argsClear(grm_args_t *args, const char **exclude_keys);

grm_error_t argsIncreaseArray(grm_args_t *args, const char *key, size_t increment) UNUSED;

unsigned int argsCount(const grm_args_t *args) UNUSED;

grm_arg_t *argsAt(const grm_args_t *args, const char *keyword);

ArgsNode *argsFindNode(const grm_args_t *args, const char *keyword);
int argsFindPreviousNode(const grm_args_t *args, const char *keyword, ArgsNode **previous_node);


/* ------------------------- argument iterator ---------------------------------------------------------------------- */

grm_args_iterator_t *argsIteratorNew(const ArgsNode *begin, const ArgsNode *end);
void argsIteratorInit(grm_args_iterator_t *args_iterator, const ArgsNode *begin, const ArgsNode *end);
void argsIteratorDelete(grm_args_iterator_t *args_iterator);
void argsIteratorFinalize(grm_args_iterator_t *args_iterator);
grm_arg_t *argsIteratorNext(grm_args_iterator_t *args_iterator);


/* ------------------------- value iterator ------------------------------------------------------------------------- */

grm_args_value_iterator_t *argsValueIteratorNew(const grm_arg_t *arg);
void argsValueIteratorInit(grm_args_value_iterator_t *args_value_iterator, const grm_arg_t *arg);
void argsValueIteratorDelete(grm_args_value_iterator_t *args_value_iterator);
void argsValueIteratorFinalize(grm_args_value_iterator_t *args_value_iterator);

void *argsValueIteratorNext(grm_args_value_iterator_t *args_value_iterator);


#ifdef __cplusplus
}
#endif
#endif /* ifndef GRM_ARGS_INT_H_INCLUDED */
