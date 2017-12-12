#ifdef __unix__
#define _XOPEN_SOURCE 500
#endif

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "gr.h"

#ifdef _WIN32
/* allow the use of posix functions on windows with msvc */
#define strdup _strdup
#endif

/* ######################### private interface ###################################################################### */

/* ========================= macros ================================================================================= */

/* ------------------------- error ---------------------------------------------------------------------------------- */

#ifndef NDEBUG
static void debug_printf(const char *format, ...) {
  va_list vl;
  va_start(vl, format);
  vfprintf(stderr, format, vl);
  va_end(vl);
}
#define debug_print_error(error_message_arguments) \
  do {                                             \
    debug_printf error_message_arguments;          \
  } while (0)
#else
#define debug_print_error(error_message_arguments)
#endif
#define debug_print_malloc_error() debug_print_error(("Memory allocation failed -> out of virtual memory.\n"))


/* ------------------------- memwriter ------------------------------------------------------------------------------ */

#define MEMWRITER_INITIAL_SIZE 32768
#define MEMWRITER_SIZE_INCREMENT 32768

#define ETB '\027'


/* ------------------------- sender --------------------------------------------------------------------------------- */

#define SENDARGS_FORMAT_MAX_LENGTH 100


/* ------------------------- util ----------------------------------------------------------------------------------- */

#define UNUSED(param) \
  do {                \
    (void)(param);    \
  } while (0)


/* ========================= datatypes ============================================================================== */

/* ------------------------- argument ------------------------------------------------------------------------------- */

typedef struct _gr_meta_arg_private_t {
  unsigned int reference_count;
} arg_private_t;

typedef struct {
  const char *key;
  void *value_ptr;
  const char *value_format;
  arg_private_t *priv;
} arg_t;


/* ------------------------- argument parsing ----------------------------------------------------------------------- */

struct _argparse_state_t {
  va_list *vl;
  const void *in_buffer;
  int apply_padding;
  ptrdiff_t data_offset;
  void *save_buffer;
  char current_format;
  unsigned int next_is_array;
  unsigned int default_array_length;
  unsigned int next_array_length;
};
typedef struct _argparse_state_t argparse_state_t;

typedef void (*read_param_t)(argparse_state_t *state);


/* ------------------------- argument container --------------------------------------------------------------------- */

typedef struct _args_node_t {
  arg_t *arg;
  struct _args_node_t *next;
} args_node_t;

struct _gr_meta_args_t {
  args_node_t *args_head;
  args_node_t *args_tail;
  args_node_t *kwargs_head;
  args_node_t *kwargs_tail;
  unsigned int args_count;
  unsigned int kwargs_count;
  unsigned int count;
};


/* ------------------------- argument iterator ---------------------------------------------------------------------- */

typedef struct {
  const args_node_t *next_node;
  const args_node_t *end;
} args_iterator_private_t;

typedef struct _gr_meta_args_iterator_t {
  arg_t *(*next)(struct _gr_meta_args_iterator_t *it);
  arg_t *arg;
  args_iterator_private_t *priv;
} args_iterator_t;


/* ------------------------- value iterator ------------------------------------------------------------------------- */

typedef struct {
  void *value_buffer;
  const char *value_format;
} args_value_iterator_private_t;

typedef struct _gr_meta_args_value_iterator_t {
  void *(*next)(struct _gr_meta_args_value_iterator_t *it);
  void *value_ptr;
  char format;
  int is_array;
  int array_length;
  args_value_iterator_private_t *priv;
} args_value_iterator_t;


/* ------------------------- json serializer ------------------------------------------------------------------------ */

enum { member_name, data_type };

typedef enum {
  /* 0 is unknown / not set */
  complete = 1,
  incomplete,
  incomplete_at_struct_beginning
} tojson_serialization_result_t;

typedef struct {
  int apply_padding;
  int array_length;
  const void *data_ptr;
  va_list *vl;
  int data_offset;
  int wrote_output;
  int add_data;
  int add_data_without_separator;
  tojson_serialization_result_t serial_result;
  unsigned int struct_nested_level;
} tojson_shared_state_t;

typedef struct {
  int next_is_ptr;
  char *data_type_ptr;
  char current_data_type;
  char *additional_type_info;
  int is_type_info_incomplete;
  tojson_shared_state_t *shared;
} tojson_state_t;

typedef struct {
  tojson_serialization_result_t serial_result;
  unsigned int struct_nested_level;
} tojson_permanent_state_t;


/* ------------------------- memwriter ------------------------------------------------------------------------------ */

struct _memwriter_t {
  char *buf;
  size_t size;
  size_t capacity;
};
typedef struct _memwriter_t memwriter_t;


/* ------------------------- sender --------------------------------------------------------------------------------- */

typedef int (*post_serialize_callback_t)(void *handle);
typedef int (*jupyter_send_callback_t)(const char *);

typedef struct {
  int target;
  memwriter_t *memwriter;
  post_serialize_callback_t post_serialize;
  union {
    struct {
      /* callback to a function that handles jupyter communication */
      jupyter_send_callback_t send;
    } jupyter;
    struct {
      int client_socket_fd;
      struct sockaddr_in server_address;
    } socket;
  } comm;
} metahandle_t;


/* ========================= static variables ======================================================================= */

/* ------------------------- argument parsing ----------------------------------------------------------------------- */

static int argparse_valid_format_specifier[128];
static read_param_t argparse_format_specifier_to_read_callback[128];
static size_t argparse_format_specifier_to_size[128];
static int argparse_static_variables_initialized = 0;


/* ------------------------- argument container --------------------------------------------------------------------- */

static const char *const ARGS_VALID_FORMAT_SPECIFIERS = "niIdDcCsS";
static const char *const ARGS_VALID_DATA_FORMAT_SPECIFIERS = "idcs"; /* Each specifier is also valid in upper case */


/* ------------------------- json serializer ------------------------------------------------------------------------ */

static tojson_permanent_state_t tojson_permanent_state = {complete, 0};


/* ========================= functions ============================================================================== */

/* ------------------------- argument parsing ----------------------------------------------------------------------- */

static void *argparse_read_params(const char *format, const void *buffer, va_list *vl, int apply_padding);
static void argparse_read_int(argparse_state_t *state);
static void argparse_read_double(argparse_state_t *state);
static void argparse_read_char(argparse_state_t *state);
static void argparse_read_string(argparse_state_t *state);
static void argparse_read_default_array_length(argparse_state_t *state);
static void argparse_read_char_array(argparse_state_t *state, int store_array_length);
static void argparse_init_static_variables();
static size_t argparse_calculate_needed_buffer_size(const char *format);
static size_t argparse_calculate_needed_padding(void *buffer, char current_format);
static void argparse_read_next_option(argparse_state_t *state, char **format);
static const char *argparse_skip_option(const char *format);


/* ------------------------- argument container --------------------------------------------------------------------- */

static arg_t *args_create_args(const char *key, const char *value_format, const void *buffer, va_list *vl,
                               int apply_padding);
static int args_validate_format_string(const char *format);
static const char *args_skip_option(const char *format);
static void args_copy_format_string_for_arg(char *dst, const char *format);
static void args_decrease_arg_reference_count(args_node_t *args_node);


/* ------------------------- sender --------------------------------------------------------------------------------- */

static int sender_post_serialize_socket(void *p);
static int sender_post_serialize_jupyter(void *p);


/* ------------------------- util ----------------------------------------------------------------------------------- */

static unsigned int str_to_uint(const char *str, int *was_successful);


/* ========================= methods ================================================================================ */

/* ------------------------- argument ------------------------------------------------------------------------------- */

static args_value_iterator_t *args_value_iter(const arg_t *arg);
static void *args_values_as_array(const arg_t *arg);


/* ------------------------- argument container --------------------------------------------------------------------- */

static void args_init(gr_meta_args_t *args);
static void args_finalize(gr_meta_args_t *args);

static void args_push_arg_common(gr_meta_args_t *args, const char *value_format, const void *buffer, va_list *vl,
                                 int apply_padding);
static void args_push_arg_vl(gr_meta_args_t *args, const char *value_format, va_list *vl);
static void args_push_kwarg_common(gr_meta_args_t *args, const char *key, const char *value_format, const void *buffer,
                                   va_list *vl, int apply_padding);
static void args_push_kwarg_vl(gr_meta_args_t *args, const char *key, const char *value_format, va_list *vl);
static void args_update_kwarg_common(gr_meta_args_t *args, const char *key, const char *value_format,
                                     const void *buffer, va_list *vl, int apply_padding);
static void args_update_kwarg(gr_meta_args_t *args, const char *key, const char *value_format, ...);
static void args_update_kwarg_buf(gr_meta_args_t *args, const char *key, const char *value_format, const void *buffer,
                                  int apply_padding);
static void args_update_kwarg_vl(gr_meta_args_t *args, const char *key, const char *value_format, va_list *vl);
static void args_push_args(gr_meta_args_t *args, const gr_meta_args_t *update_args);
static void args_update_kwargs(gr_meta_args_t *args, const gr_meta_args_t *update_args);

static void args_clear_args(gr_meta_args_t *args);
static void args_delete_kwarg(gr_meta_args_t *args, const char *key);

static unsigned int args_args_count(const gr_meta_args_t *args);
static unsigned int args_kwargs_count(const gr_meta_args_t *args);
static unsigned int args_count(const gr_meta_args_t *args);

static int args_has_keyword(const gr_meta_args_t *args, const char *keyword);
static arg_t *args_find_keyword(const gr_meta_args_t *args, const char *keyword);
static int args_get_first_value_by_keyword(const gr_meta_args_t *args, const char *keyword,
                                           const char *first_value_format, void *first_value);
#define args_get_first_value_by_keyword(args, keyword, first_value_format, first_value) \
  args_get_first_value_by_keyword(args, keyword, first_value_format, (void *)first_value)
static int args_values_by_keyword(const gr_meta_args_t *args, const char *keyword, const char *expected_format, ...);
static const void *args_values_as_array_by_keyword(const gr_meta_args_t *args, const char *keyword);

static args_node_t *args_find_node_by_keyword(const gr_meta_args_t *args, const char *keyword);
static int args_find_previous_node_by_keyword(const gr_meta_args_t *args, const char *keyword,
                                              args_node_t **previous_node);

static args_iterator_t *args_iter(const gr_meta_args_t *args);
static args_iterator_t *args_iter_args(const gr_meta_args_t *args);
static args_iterator_t *args_iter_kwargs(const gr_meta_args_t *args);


/* ------------------------- argument iterator ---------------------------------------------------------------------- */

static args_iterator_t *args_iterator_new(const args_node_t *begin, const args_node_t *end);
static void args_iterator_init(args_iterator_t *args_iterator, const args_node_t *begin, const args_node_t *end);
static void args_iterator_delete(args_iterator_t *args_iterator);
static void args_iterator_finalize(args_iterator_t *args_iterator);
static arg_t *args_iterator_next(args_iterator_t *args_iterator);


/* ------------------------- value iterator ------------------------------------------------------------------------- */

static args_value_iterator_t *args_value_iterator_new(const arg_t *arg);
static void args_value_iterator_init(args_value_iterator_t *args_value_iterator, const arg_t *arg);
static void args_value_iterator_delete(args_value_iterator_t *args_value_iterator);
static void args_value_iterator_finalize(args_value_iterator_t *args_value_iterator);

static void *args_value_iterator_next(args_value_iterator_t *args_value_iterator);


/* ------------------------- json serializer ------------------------------------------------------------------------ */

#define DECLARE_STRINGIFY_SINGLE(type, promoted_type, format_specifier) \
  static void tojson_stringify_##type(memwriter_t *memwriter, tojson_state_t *state);
#define DECLARE_STRINGIFY_MULTI(type, format_specifier) \
  static void tojson_stringify_##type##_array(memwriter_t *memwriter, tojson_state_t *state);

DECLARE_STRINGIFY_SINGLE(int, int, "%d")
DECLARE_STRINGIFY_MULTI(int, "%d")
DECLARE_STRINGIFY_SINGLE(double, double, "%f")
DECLARE_STRINGIFY_MULTI(double, "%f")
DECLARE_STRINGIFY_SINGLE(char, int, "%c")

#undef DECLARE_STRINGIFY_SINGLE
#undef DECLARE_STRINGIFY_MULTI

static void tojson_stringify_char_array(memwriter_t *memwriter, tojson_state_t *state);
static void tojson_stringify_bool(memwriter_t *memwriter, tojson_state_t *state);

static int tojson_get_member_count(const char *data_desc);
static int tojson_is_json_array_needed(const char *data_desc);
static int tojson_read_datatype(tojson_state_t *state);
static void tojson_read_next_is_ptr(tojson_state_t *state);
static void tojson_skip_bytes(tojson_state_t *state);
static void tojson_close_object(memwriter_t *memwriter, tojson_state_t *state);
static void tojson_read_array_length(tojson_state_t *state);
static void tojson_unzip_membernames_and_datatypes(char *mixed_ptr, char ***member_name_ptr, char ***data_type_ptr);
static void tojson_stringify_struct(memwriter_t *memwriter, tojson_state_t *state);
static int tojson_serialize(memwriter_t *memwriter, char *data_desc, const void *data, va_list *vl, int apply_padding,
                            int add_data, int add_data_without_separator, unsigned int *struct_nested_level,
                            tojson_serialization_result_t *serial_result, tojson_shared_state_t *shared_state);
static int tojson_init_variables(int *add_data, int *add_data_without_separator, char **_data_desc,
                                 const char *data_desc);
static int tojson_write_vl(memwriter_t *memwriter, const char *data_desc, va_list *vl);
static int tojson_write_buf(memwriter_t *memwriter, const char *data_desc, const void *buffer, int apply_padding);
static int tojson_is_complete();


/* ------------------------- memwriter ------------------------------------------------------------------------------ */

static memwriter_t *memwriter_new();
static void memwriter_delete(memwriter_t *memwriter);
static void memwriter_clear(memwriter_t *memwriter);
static int memwriter_enlarge_buf(memwriter_t *memwriter);
static int memwriter_printf(memwriter_t *memwriter, const char *format, ...);
static int memwriter_puts(memwriter_t *memwriter, const char *s);
static int memwriter_putc(memwriter_t *memwriter, char c);
static char *memwriter_buf(const memwriter_t *memwriter);
static size_t memwriter_size(const memwriter_t *memwriter);


/* ------------------------- sender --------------------------------------------------------------------------------- */

static int sender_init_for_socket(metahandle_t *handle, va_list *vl);
static int sender_init_for_jupyter(metahandle_t *handle, va_list *vl);
static int sender_finalize_for_socket(metahandle_t *handle);
static int sender_finalize_for_jupyter(metahandle_t *handle);


/* ######################### public implementation ################################################################## */

/* ========================= functions ============================================================================== */

/* ------------------------- sender --------------------------------------------------------------------------------- */

void *gr_openmeta(int target, ...) {
  va_list vl;
  metahandle_t *handle;
  int error = 0;

  handle = malloc(sizeof(metahandle_t));
  if (handle == NULL) {
    return NULL;
  }
  handle->target = target;
  va_start(vl, target);
  switch (target) {
  case GR_TARGET_JUPYTER:
    error = sender_init_for_jupyter(handle, &vl);
    break;
  case GR_TARGET_SOCKET:
    error = sender_init_for_socket(handle, &vl);
    break;
  }
  va_end(vl);

  if (error != 0) {
    free(handle);
    handle = NULL;
  }

  return (void *)handle;
}

void gr_closemeta(const void *p) {
  metahandle_t *handle = (metahandle_t *)p;

  switch (handle->target) {
  case GR_TARGET_JUPYTER:
    sender_finalize_for_jupyter(handle);
    break;
  case GR_TARGET_SOCKET:
    sender_finalize_for_socket(handle);
    break;
  }
  memwriter_delete(handle->memwriter);
  free(handle);
}

int gr_sendmeta(const void *p, const char *data_desc, ...) {
  metahandle_t *handle = (metahandle_t *)p;
  va_list vl;
  int was_successful;

  va_start(vl, data_desc);
  was_successful = !tojson_write_vl(handle->memwriter, data_desc, &vl);
  if (was_successful && tojson_is_complete() && handle->post_serialize != NULL) {
    int error = handle->post_serialize(handle);
    was_successful = was_successful && (error == 0);
  }
  va_end(vl);

  return was_successful;
}

int gr_sendmeta_buf(const void *p, const char *data_desc, const void *buffer, int apply_padding) {
  metahandle_t *handle = (metahandle_t *)p;
  int was_successful;

  was_successful = !tojson_write_buf(handle->memwriter, data_desc, buffer, apply_padding);
  if (was_successful && tojson_is_complete() && handle->post_serialize != NULL) {
    int error = handle->post_serialize(handle);
    was_successful = was_successful && (error == 0);
  }

  return was_successful;
}

int gr_sendmeta_args(const void *p, const gr_meta_args_t *args) {
  metahandle_t *handle = (metahandle_t *)p;
  args_iterator_t *it;
  arg_t *arg;
  char format[SENDARGS_FORMAT_MAX_LENGTH];
  int is_first_kwarg;

  it = args_iter_args(args);
  while ((arg = it->next(it))) {
    gr_sendmeta_buf(handle, arg->value_format, arg->value_ptr, 1);
  }
  args_iterator_delete(it);

  it = args_iter_kwargs(args);
  is_first_kwarg = 1;
  while ((arg = it->next(it))) {
    char *format_ptr = format;
    size_t format_remaining_size = SENDARGS_FORMAT_MAX_LENGTH;
    int had_enough_memory = 1;
    size_t length;
    if (is_first_kwarg) {
      memcpy(format_ptr, "s(", 2);
      format_ptr += 2;
      format_remaining_size -= 2;
      is_first_kwarg = 0;
    }
    length = strlen(arg->key);
    if (length + 2 < format_remaining_size) { /* strlen + ':' + 1 or more characters for format description */
      memcpy(format_ptr, arg->key, length);
      format_ptr += length;
      *format_ptr++ = ':';
      format_remaining_size -= length + 1;
      length = strlen(arg->value_format);
      if (length < format_remaining_size) {
        memcpy(format_ptr, arg->value_format, length);
        format_ptr += length;
        *format_ptr = '\0';
      } else {
        had_enough_memory = 0;
      }
    } else {
      had_enough_memory = 0;
    }
    if (had_enough_memory) {
      gr_sendmeta_buf(handle, format, arg->value_ptr, 1);
    } else {
      debug_print_error(("Out of local memory for creating a format string -> aborting"));
      args_iterator_delete(it);
      return -1;
    }
  }
  gr_sendmeta(handle, ")");
  args_iterator_delete(it);

  return 0;
}


/* ========================= methods ================================================================================ */

/* ------------------------- argument container --------------------------------------------------------------------- */

gr_meta_args_t *gr_meta_args_new() {
  gr_meta_args_t *args = malloc(sizeof(gr_meta_args_t));
  if (args == NULL) {
    debug_print_malloc_error();
    return NULL;
  }
  args_init(args);
  return args;
}

void gr_meta_args_delete(gr_meta_args_t *args) {
  args_finalize(args);
  free(args);
}

void gr_meta_args_push_arg(gr_meta_args_t *args, const char *value_format, ...) {
  va_list vl;
  va_start(vl, value_format);

  args_push_arg_vl(args, value_format, &vl);

  va_end(vl);
}

void gr_meta_args_push_arg_buf(gr_meta_args_t *args, const char *value_format, const void *buffer, int apply_padding) {
  args_push_arg_common(args, value_format, buffer, NULL, apply_padding);
}

void gr_meta_args_push_kwarg(gr_meta_args_t *args, const char *key, const char *value_format, ...) {
  /*
   * warning! this function does not check if a given key already exists in the container
   * -> use `args_update_kwarg` instead
   */
  va_list vl;
  va_start(vl, value_format);

  args_push_kwarg_vl(args, key, value_format, &vl);

  va_end(vl);
}

void gr_meta_args_push_kwarg_buf(gr_meta_args_t *args, const char *key, const char *value_format, const void *buffer,
                                 int apply_padding) {
  args_push_kwarg_common(args, key, value_format, buffer, NULL, apply_padding);
}


/* ######################### private implementation ################################################################# */

/* ========================= functions ============================================================================== */

/* ------------------------- argument parsing ----------------------------------------------------------------------- */

void *argparse_read_params(const char *format, const void *buffer, va_list *vl, int apply_padding) {
  char *fmt, *current_format;
  size_t needed_buffer_size;
  void *save_buffer;
  argparse_state_t state;

  argparse_init_static_variables();

  /* copy format string since it is modified during the parsing process */
  fmt = strdup(format);
  if (fmt == NULL) {
    debug_print_malloc_error();
    return NULL;
  }

  /* get needed save_buffer size to store all parameters and allocate memory */
  needed_buffer_size = argparse_calculate_needed_buffer_size(fmt);
  save_buffer = malloc(needed_buffer_size);
  if (save_buffer == NULL) {
    debug_print_malloc_error();
    free(fmt);
    return NULL;
  }

  /* initialize state object */
  state.vl = vl;
  state.in_buffer = buffer;
  state.apply_padding = apply_padding;
  state.data_offset = 0;
  state.save_buffer = save_buffer;
  state.next_is_array = 0;
  state.default_array_length = 1;
  state.next_array_length = 0;

  current_format = fmt;
  while (*current_format) {
    state.current_format = tolower(*current_format);
    if (state.current_format != *current_format) {
      state.next_is_array = 1;
    }
    argparse_read_next_option(&state, &current_format);
    state.save_buffer =
      ((char *)state.save_buffer) + argparse_calculate_needed_padding(state.save_buffer, state.current_format);
    argparse_format_specifier_to_read_callback[(unsigned char)state.current_format](&state);
    state.next_is_array = 0;
    state.next_array_length = 0;
    ++current_format;
  }

  /* cleanup */
  free(fmt);

  return save_buffer;
}

#define CHECK_PADDING(type)                                           \
  do {                                                                \
    if (state->in_buffer != NULL && state->apply_padding) {           \
      ptrdiff_t needed_padding = state->data_offset % sizeof(type);   \
      state->in_buffer = ((char *)state->in_buffer) + needed_padding; \
      state->data_offset += needed_padding;                           \
    }                                                                 \
  } while (0)

#define READ_TYPE(type)                                                                                         \
  void argparse_read_##type(argparse_state_t *state) {                                                          \
    size_t *size_t_typed_buffer;                                                                                \
    type *typed_buffer, **pointer_typed_buffer, *src_ptr;                                                       \
    size_t current_array_length;                                                                                \
                                                                                                                \
    if (state->next_is_array) {                                                                                 \
      current_array_length = state->next_array_length ? state->next_array_length : state->default_array_length; \
      if (state->in_buffer != NULL) {                                                                           \
        CHECK_PADDING(type **);                                                                                 \
        src_ptr = *(type **)state->in_buffer;                                                                   \
      } else {                                                                                                  \
        src_ptr = va_arg(*state->vl, type *);                                                                   \
      }                                                                                                         \
      size_t_typed_buffer = state->save_buffer;                                                                 \
      *size_t_typed_buffer = current_array_length;                                                              \
      pointer_typed_buffer = (type **)++size_t_typed_buffer;                                                    \
      *pointer_typed_buffer = malloc(current_array_length * sizeof(type));                                      \
      if (*pointer_typed_buffer != NULL) {                                                                      \
        memcpy(*pointer_typed_buffer, src_ptr, current_array_length * sizeof(type));                            \
      } else {                                                                                                  \
        debug_print_malloc_error();                                                                             \
      }                                                                                                         \
      if (state->in_buffer != NULL) {                                                                           \
        state->in_buffer = ((type **)state->in_buffer) + 1;                                                     \
        state->data_offset += sizeof(type *);                                                                   \
      }                                                                                                         \
      state->save_buffer = ++pointer_typed_buffer;                                                              \
    } else {                                                                                                    \
      typed_buffer = state->save_buffer;                                                                        \
      if (state->in_buffer != NULL) {                                                                           \
        CHECK_PADDING(type);                                                                                    \
        *typed_buffer = *((type *)state->in_buffer);                                                            \
        state->in_buffer = ((type *)state->in_buffer) + 1;                                                      \
        state->data_offset += sizeof(type);                                                                     \
      } else {                                                                                                  \
        *typed_buffer = va_arg(*state->vl, type);                                                               \
      }                                                                                                         \
      state->save_buffer = ++typed_buffer;                                                                      \
    }                                                                                                           \
  }

READ_TYPE(int)
READ_TYPE(double)

#undef READ_TYPE


void argparse_read_char(argparse_state_t *state) {
  if (state->next_is_array) {
    argparse_read_char_array(state, 1);
  } else {
    char *typed_buffer = state->save_buffer;
    if (state->in_buffer != NULL) {
      *typed_buffer = *((char *)state->in_buffer);
      state->in_buffer = (char *)state->in_buffer + 1;
      state->data_offset += sizeof(char);
    } else {
      *typed_buffer = va_arg(*state->vl, int); /* char is promoted to int */
    }
    state->save_buffer = ++typed_buffer;
  }
}


void argparse_read_string(argparse_state_t *state) {
  if (state->next_is_array) {
    size_t *size_t_typed_buffer;
    char ***pointer_typed_buffer;
    const char **src_ptr;
    size_t current_array_length;

    current_array_length = state->next_array_length ? state->next_array_length : state->default_array_length;
    if (state->in_buffer != NULL) {
      CHECK_PADDING(char **);
      src_ptr = (const char **)state->in_buffer;
    } else {
      src_ptr = va_arg(*state->vl, const char **);
    }
    size_t_typed_buffer = state->save_buffer;
    *size_t_typed_buffer = current_array_length;
    pointer_typed_buffer = (char ***)++size_t_typed_buffer;
    *pointer_typed_buffer = malloc((current_array_length + 1) * sizeof(char *));
    if (*pointer_typed_buffer != NULL) {
      int found_malloc_fail;
      unsigned int i;
      for (i = 0; i < current_array_length; i++) {
        (*pointer_typed_buffer)[i] = malloc(strlen(src_ptr[i]) + 1);
      }
      found_malloc_fail = 0;
      for (i = 0; i < current_array_length && !found_malloc_fail; i++) {
        if ((*pointer_typed_buffer)[i] == NULL) {
          found_malloc_fail = 1;
        }
      }
      if (!found_malloc_fail) {
        for (i = 0; i < current_array_length; i++) {
          size_t current_string_length;
          current_string_length = strlen(src_ptr[i]);
          memcpy((*pointer_typed_buffer)[i], src_ptr[i], current_string_length);
          (*pointer_typed_buffer)[i][current_string_length] = '\0';
        }
        (*pointer_typed_buffer)[current_array_length] = NULL; /* array terminator */
      } else {
        for (i = 0; i < current_array_length; i++) {
          free((*pointer_typed_buffer)[i]);
        }
        free(*pointer_typed_buffer);
        debug_print_malloc_error();
      }
    } else {
      debug_print_malloc_error();
    }
    if (state->in_buffer != NULL) {
      state->in_buffer = ((char ***)state->in_buffer) + 1;
      state->data_offset += sizeof(char **);
    }
    state->save_buffer = ++pointer_typed_buffer;
  } else {
    argparse_read_char_array(state, 0);
  }
}


void argparse_read_default_array_length(argparse_state_t *state) {
  if (state->in_buffer != NULL) {
    const int *typed_buffer = state->in_buffer;
    CHECK_PADDING(int);
    state->default_array_length = *typed_buffer;
    state->in_buffer = typed_buffer + 1;
    state->data_offset += sizeof(int);
  } else {
    state->default_array_length = va_arg(*state->vl, int);
  }
}


/* helper function */
void argparse_read_char_array(argparse_state_t *state, int store_array_length) {
  char **pointer_typed_buffer;
  const char *src_ptr;
  size_t current_array_length;

  if (state->in_buffer != NULL) {
    CHECK_PADDING(char *);
    src_ptr = state->in_buffer;
  } else {
    src_ptr = va_arg(*state->vl, char *);
  }
  current_array_length = state->next_array_length ? state->next_array_length : strlen(src_ptr);
  if (store_array_length) {
    size_t *size_t_typed_buffer = state->save_buffer;
    *size_t_typed_buffer = current_array_length;
    pointer_typed_buffer = (char **)++size_t_typed_buffer;
  } else {
    pointer_typed_buffer = (char **)state->save_buffer;
  }
  *pointer_typed_buffer = malloc(current_array_length + 1);
  if (*pointer_typed_buffer != NULL) {
    memcpy(*pointer_typed_buffer, src_ptr, current_array_length);
    (*pointer_typed_buffer)[current_array_length] = '\0';
  } else {
    debug_print_malloc_error();
  }
  if (state->in_buffer != NULL) {
    state->in_buffer = ((char **)state->in_buffer) + 1;
    state->data_offset += sizeof(char *);
  }
  state->save_buffer = ++pointer_typed_buffer;
}

#undef CHECK_PADDING

void argparse_init_static_variables() {
  if (!argparse_static_variables_initialized) {
    argparse_valid_format_specifier['n'] = 1;
    argparse_valid_format_specifier['i'] = 1;
    argparse_valid_format_specifier['I'] = 1;
    argparse_valid_format_specifier['d'] = 1;
    argparse_valid_format_specifier['D'] = 1;
    argparse_valid_format_specifier['c'] = 1;
    argparse_valid_format_specifier['C'] = 1;
    argparse_valid_format_specifier['s'] = 1;
    argparse_valid_format_specifier['S'] = 1;

    argparse_format_specifier_to_read_callback['i'] = argparse_read_int;
    argparse_format_specifier_to_read_callback['d'] = argparse_read_double;
    argparse_format_specifier_to_read_callback['c'] = argparse_read_char;
    argparse_format_specifier_to_read_callback['s'] = argparse_read_string;
    argparse_format_specifier_to_read_callback['n'] = argparse_read_default_array_length;

    argparse_format_specifier_to_size['i'] = sizeof(int);
    argparse_format_specifier_to_size['I'] = sizeof(int *);
    argparse_format_specifier_to_size['d'] = sizeof(double);
    argparse_format_specifier_to_size['D'] = sizeof(double *);
    argparse_format_specifier_to_size['c'] = sizeof(char);
    argparse_format_specifier_to_size['C'] = sizeof(char *);
    argparse_format_specifier_to_size['s'] = sizeof(char *);
    argparse_format_specifier_to_size['S'] = sizeof(char **);
    argparse_format_specifier_to_size['n'] = 0; /* size for array length is reserved by an array call itself */
    argparse_format_specifier_to_size['#'] = sizeof(size_t); /* only used internally */
  }
  argparse_static_variables_initialized = 1;
}

size_t argparse_calculate_needed_buffer_size(const char *format) {
  size_t needed_size;
  size_t size_for_current_specifier;
  int is_array;

  needed_size = 0;
  is_array = 0;
  while (*format) {
    char current_format;
    if (*format == '(') {
      format = argparse_skip_option(format);
      if (!*format) {
        break;
      }
    }
    current_format = tolower(*format);
    if (current_format != *format) {
      is_array = 1;
    }
    while (current_format) {
      size_for_current_specifier = argparse_format_specifier_to_size[(unsigned char)current_format];
      /* apply needed padding for memory alignment first */
      needed_size += argparse_calculate_needed_padding((void *)needed_size, current_format);
      /* then add the actual needed memory size */
      needed_size += size_for_current_specifier;
      if (is_array) {
        current_format = '#';
        is_array = 0;
      } else {
        current_format = '\0';
      }
    }
    ++format;
  }

  assert(needed_size > 0); /* needed_size must be > 0, otherwise there is an internal logical error */

  return needed_size;
}

size_t argparse_calculate_needed_padding(void *buffer, char current_format) {
  int size_for_current_specifier;
  int needed_padding;

  size_for_current_specifier = argparse_format_specifier_to_size[(unsigned char)current_format];
  if (size_for_current_specifier > 0) {
    needed_padding = size_for_current_specifier - ((char *)buffer - (char *)0) % size_for_current_specifier;
    if (needed_padding == size_for_current_specifier) {
      needed_padding = 0;
    }
  } else {
    needed_padding = 0;
  }

  return needed_padding;
}

void argparse_read_next_option(argparse_state_t *state, char **format) {
  char *fmt = *format;
  unsigned int next_array_length;
  char *current_char;
  int was_successful;

  ++fmt;
  if (*fmt != '(') {
    return; /* there is no option that could be read */
  }

  current_char = ++fmt;
  while (*current_char && *current_char != ')') {
    ++current_char;
  }
  if (!*current_char) {
    debug_print_error(("Option \"%s\" in format string \"%s\" is not terminated -> ignore it.\n", fmt, *format));
    return;
  }
  *current_char = '\0';

  next_array_length = str_to_uint(fmt, &was_successful);
  if (!was_successful) {
    debug_print_error(
      ("Option \"%s\" in format string \"%s\" could not be converted to a number -> ignore it.\n", fmt, *format));
    return;
  }

  state->next_array_length = next_array_length;
  *format = current_char;
}

const char *argparse_skip_option(const char *format) {
  if (*format != '(') {
    return format;
  }
  while (*format && *format != ')') {
    ++format;
  }
  if (*format) {
    ++format;
  }
  return format;
}


/* ------------------------- argument container --------------------------------------------------------------------- */

arg_t *args_create_args(const char *key, const char *value_format, const void *buffer, va_list *vl, int apply_padding) {
  arg_t *arg;

  if (!args_validate_format_string(value_format)) {
    return NULL;
  }

  arg = malloc(sizeof(arg_t));
  if (arg == NULL) {
    debug_print_malloc_error();
    return NULL;
  }
  if (key != NULL) {
    arg->key = strdup(key);
    if (arg->key == NULL) {
      debug_print_malloc_error();
      free(arg);
      return NULL;
    }
  } else {
    arg->key = NULL;
  }
  arg->value_format = malloc(strlen(value_format) + 1);
  if (arg->value_format == NULL) {
    debug_print_malloc_error();
    free((char *)arg->key);
    free(arg);
    return NULL;
  }
  args_copy_format_string_for_arg((char *)arg->value_format, value_format);
  arg->value_ptr = argparse_read_params(arg->value_format, buffer, vl, apply_padding);
  arg->priv = malloc(sizeof(arg_private_t));
  if (arg->priv == NULL) {
    debug_print_malloc_error();
    free((char *)arg->key);
    free((char *)arg->value_format);
    free(arg);
    return NULL;
  }
  arg->priv->reference_count = 1;

  return arg;
}

int args_validate_format_string(const char *format) {
  char *fmt;
  char *previous_char;
  char *current_char;
  char *option_start;
  int is_valid;

  if (format == NULL || *format == '\0') {
    return 0;
  }
  fmt = strdup(format);
  if (fmt == NULL) {
    debug_print_malloc_error();
    return 0;
  }

  previous_char = NULL;
  current_char = fmt;
  is_valid = 1;
  while (*current_char && is_valid) {
    if (*current_char == '(') {
      if (previous_char != NULL) {
        if (strchr(ARGS_VALID_DATA_FORMAT_SPECIFIERS, tolower(*previous_char)) != NULL) {
          previous_char = current_char;
          option_start = ++current_char;
          while (*current_char && *current_char != ')') {
            ++current_char;
          }
          if (*current_char) {
            *current_char = '\0';
            str_to_uint(option_start, &is_valid);
            if (!is_valid) {
              debug_print_error(
                ("The option \"%s\" in the format string \"%s\" in no valid number.", option_start, format));
            }
          } else {
            is_valid = 0;
            --current_char;
            debug_print_error(("Option \"%s\" in the format string \"%s\" is not terminated.", option_start, format));
          }
        } else {
          is_valid = 0;
          debug_print_error(
            ("Specifier '%c' in the format string \"%s\" cannot have any options.", *previous_char, format));
        }
      } else {
        is_valid = 0;
        debug_print_error(
          ("The format string \"%s\" is invalid: Format strings must not start with an option.", format));
      }
    } else {
      if (!(strchr(ARGS_VALID_FORMAT_SPECIFIERS, *current_char) != NULL)) {
        is_valid = 0;
        debug_print_error(("Invalid specifier '%c' in the format string \"%s\".", *current_char, format));
      }
      previous_char = current_char;
    }
    ++current_char;
  }

  free(fmt);

  return is_valid;
}

const char *args_skip_option(const char *format) {
  if (*format != '(') {
    return format;
  }
  while (*format && *format != ')') {
    ++format;
  }
  if (*format) {
    ++format;
  }
  return format;
}

void args_copy_format_string_for_arg(char *dst, const char *format) {
  while (*format) {
    if (*format == 'C') {
      /* char arrays and strings are the same -> store them as strings for unified data handling */
      *dst++ = 's';
      /* skip an optional array length since strings have no array length */
      ++format;
      format = args_skip_option(format);
    } else {
      *dst++ = *format++;
    }
  }
  *dst = '\0';
}

void args_decrease_arg_reference_count(args_node_t *args_node) {
  if (--(args_node->arg->priv->reference_count) == 0) {
    args_value_iterator_t *value_it = args_value_iter(args_node->arg);
    while (value_it->next(value_it) != NULL) {
      if (value_it->format == 's' && value_it->is_array) {
        char **current_string_ptr = *(char ***)value_it->value_ptr;
        while (*current_string_ptr != NULL) {
          free(*current_string_ptr);
          ++current_string_ptr;
        }
        free(*(char ***)value_it->value_ptr);
      } else if (value_it->format == 's' || value_it->is_array) {
        /* assume an char pointer since chars have no memory alignment restrictions */
        free(*(char **)value_it->value_ptr);
      }
    }
    args_value_iterator_delete(value_it);
    free((char *)args_node->arg->key);
    free((char *)args_node->arg->value_format);
    free(args_node->arg->priv);
    free(args_node->arg->value_ptr);
    free(args_node->arg);
  }
}


/* ------------------------- sender --------------------------------------------------------------------------------- */

int sender_post_serialize_socket(void *p) {
  metahandle_t *handle = (metahandle_t *)p;
  char *buf;
  size_t buf_size;

  memwriter_putc(handle->memwriter, ETB);

  buf = memwriter_buf(handle->memwriter);
  buf_size = memwriter_size(handle->memwriter);

  send(handle->comm.socket.client_socket_fd, buf, buf_size, 0);

  memwriter_clear(handle->memwriter);

  return 0;
}

int sender_post_serialize_jupyter(void *p) {
  metahandle_t *handle = (metahandle_t *)p;
  char *buf;

  buf = memwriter_buf(handle->memwriter);

  handle->comm.jupyter.send(buf);

  memwriter_clear(handle->memwriter);

  return 0;
}


/* ------------------------- util ----------------------------------------------------------------------------------- */

unsigned int str_to_uint(const char *str, int *was_successful) {
  char *conversion_end = NULL;
  unsigned long conversion_result;
  int success = 0;

  errno = 0;
  if (str != NULL) {
    conversion_result = strtoul(str, &conversion_end, 10);
  } else {
    conversion_result = 0;
  }
  if (conversion_end == NULL || *conversion_end != '\0') {
    debug_print_error(("The parameter \"%s\" is not a valid number!\n", str));
  } else if (errno == ERANGE || conversion_result > UINT_MAX) {
    debug_print_error(("The parameter \"%s\" is too big, the number has been clamped to \"%u\"\n", str, UINT_MAX));
    conversion_result = UINT_MAX;
  } else {
    success = 1;
  }
  if (was_successful != NULL) {
    *was_successful = success;
  }

  return (unsigned int)conversion_result;
}


/* ========================= methods ================================================================================ */

/* ------------------------- argument ------------------------------------------------------------------------------- */

args_value_iterator_t *args_value_iter(const arg_t *arg) {
  return args_value_iterator_new(arg);
}

void *args_values_as_array(const arg_t *arg) {
  const char *format_ptr;
  char format;

  /* First check if it is safe to return an pointer to the internal buffer -> all formats are equal */
  format_ptr = arg->value_format;
  format = *format_ptr;
  if (format == '\0') {
    return NULL;
  }
  ++format_ptr;
  while (*format_ptr == format) {
    ++format_ptr;
  }
  return (*format_ptr == '\0') ? arg->value_ptr : NULL;
}


/* ------------------------- argument container --------------------------------------------------------------------- */

void args_init(gr_meta_args_t *args) {
  args->args_head = NULL;
  args->args_tail = NULL;
  args->kwargs_head = NULL;
  args->kwargs_tail = NULL;
  args->args_count = 0;
  args->kwargs_count = 0;
  args->count = 0;
}

void args_finalize(gr_meta_args_t *args) {
  args_node_t *current_node, *next_node;

  current_node = (args->args_head != NULL) ? args->args_head : args->kwargs_head;
  while (current_node != NULL) {
    next_node = current_node->next;
    args_decrease_arg_reference_count(current_node);
    free(current_node);
    current_node = next_node;
  }
}

void args_push_arg_common(gr_meta_args_t *args, const char *value_format, const void *buffer, va_list *vl,
                          int apply_padding) {
  arg_t *arg;
  args_node_t *args_node;

  if ((arg = args_create_args(NULL, value_format, buffer, vl, apply_padding)) == NULL) {
    return;
  }

  args_node = malloc(sizeof(args_node_t));
  if (args_node == NULL) {
    debug_print_malloc_error();
    free((char *)arg->value_format);
    free(arg->priv);
    free(arg);
    return;
  }
  args_node->arg = arg;
  args_node->next = args->kwargs_head;

  if (args->args_head == NULL) {
    args->args_head = args_node;
    args->args_tail = args_node;
  } else {
    args->args_tail->next = args_node;
    args->args_tail = args_node;
  }

  ++(args->args_count);
  ++(args->count);
}

void args_push_arg_vl(gr_meta_args_t *args, const char *value_format, va_list *vl) {
  args_push_arg_common(args, value_format, NULL, vl, 0);
}

void args_push_kwarg_common(gr_meta_args_t *args, const char *key, const char *value_format, const void *buffer,
                            va_list *vl, int apply_padding) {
  /*
   * warning! this function does not check if a given key already exists in the container
   * -> use `args_update_kwarg` instead
   */
  arg_t *arg;
  args_node_t *args_node;

  if ((arg = args_create_args(key, value_format, buffer, vl, apply_padding)) == NULL) {
    return;
  }

  args_node = malloc(sizeof(args_node_t));
  if (args_node == NULL) {
    debug_print_malloc_error();
    free((char *)arg->key);
    free((char *)arg->value_format);
    free(arg->priv);
    free(arg);
    return;
  }
  args_node->arg = arg;
  args_node->next = NULL;

  if (args->kwargs_head == NULL) {
    args->kwargs_head = args_node;
    args->kwargs_tail = args_node;
    if (args->args_tail != NULL) {
      args->args_tail->next = args_node;
    }
  } else {
    args->kwargs_tail->next = args_node;
    args->kwargs_tail = args_node;
  }

  ++(args->kwargs_count);
  ++(args->count);
}

void args_push_kwarg_vl(gr_meta_args_t *args, const char *key, const char *value_format, va_list *vl) {
  args_push_kwarg_common(args, key, value_format, NULL, vl, 0);
}

void args_update_kwarg_common(gr_meta_args_t *args, const char *key, const char *value_format, const void *buffer,
                              va_list *vl, int apply_padding) {
  args_node_t *args_node;
  arg_t *arg;

  if ((args_node = args_find_node_by_keyword(args, key)) != NULL) {
    if ((arg = args_create_args(key, value_format, buffer, vl, apply_padding)) != NULL) {
      args_decrease_arg_reference_count(args_node);
      args_node->arg = arg;
    }
  } else {
    args_push_kwarg_vl(args, key, value_format, vl);
  }
}

void args_update_kwarg(gr_meta_args_t *args, const char *key, const char *value_format, ...) {
  va_list vl;
  va_start(vl, value_format);

  args_push_kwarg_vl(args, key, value_format, &vl);

  va_end(vl);
}

void args_update_kwarg_buf(gr_meta_args_t *args, const char *key, const char *value_format, const void *buffer,
                           int apply_padding) {
  args_update_kwarg_common(args, key, value_format, buffer, NULL, apply_padding);
}

void args_update_kwarg_vl(gr_meta_args_t *args, const char *key, const char *value_format, va_list *vl) {
  args_update_kwarg_common(args, key, value_format, NULL, vl, 0);
}

void args_push_args(gr_meta_args_t *args, const gr_meta_args_t *update_args) {
  args_iterator_t *it;
  args_node_t *args_node;
  arg_t *update_arg;

  it = args_iter_args(update_args);
  while ((update_arg = it->next(it)) != NULL) {
    ++(update_arg->priv->reference_count);
    args_node = malloc(sizeof(args_node_t));
    if (args_node == NULL) {
      debug_print_malloc_error();
      args_iterator_delete(it);
      return;
    }
    args_node->arg = update_arg;
    args_node->next = args->kwargs_head;

    if (args->args_head == NULL) {
      args->args_head = args_node;
      args->args_tail = args_node;
    } else {
      args->args_tail->next = args_node;
      args->args_tail = args_node;
    }

    ++(args->args_count);
    ++(args->count);
  }
  args_iterator_delete(it);
}

void args_update_kwargs(gr_meta_args_t *args, const gr_meta_args_t *update_args) {
  args_iterator_t *it;
  args_node_t *args_node, *previous_node_by_keyword;
  arg_t *update_arg;

  it = args_iter_kwargs(update_args);
  while ((update_arg = it->next(it)) != NULL) {
    ++(update_arg->priv->reference_count);
    args_node = malloc(sizeof(args_node_t));
    if (args_node == NULL) {
      debug_print_malloc_error();
      args_iterator_delete(it);
      return;
    }
    args_node->arg = update_arg;
    args_node->next = NULL;

    if (args->kwargs_head == NULL) {
      args->kwargs_head = args_node;
      args->kwargs_tail = args_node;
      if (args->args_tail != NULL) {
        args->args_tail->next = args_node;
      }
      ++(args->kwargs_count);
      ++(args->count);
    } else if (args_find_previous_node_by_keyword(args, update_arg->key, &previous_node_by_keyword)) {
      if (previous_node_by_keyword == NULL) {
        args_node->next = args->kwargs_head->next;
        if (args->kwargs_head == args->kwargs_tail) {
          args->kwargs_tail = args_node;
        }
        if (args->args_tail != NULL) {
          args->args_tail->next = args_node;
        }
        args_decrease_arg_reference_count(args->kwargs_head);
        free(args->kwargs_head);
        args->kwargs_head = args_node;
      } else {
        args_node->next = previous_node_by_keyword->next->next;
        args_decrease_arg_reference_count(previous_node_by_keyword->next);
        free(previous_node_by_keyword->next);
        previous_node_by_keyword->next = args_node;
        if (args_node->next == NULL) {
          args->kwargs_tail = args_node;
        }
      }
    } else {
      args->kwargs_tail->next = args_node;
      args->kwargs_tail = args_node;
      ++(args->kwargs_count);
      ++(args->count);
    }
  }
  args_iterator_delete(it);
}

void args_clear_args(gr_meta_args_t *args) {
  args_node_t *current_node, *next_node;

  current_node = args->args_head;
  while (current_node != args->kwargs_head) {
    next_node = current_node->next;
    args_decrease_arg_reference_count(current_node);
    free(current_node);
    current_node = next_node;
  }
  args->args_head = NULL;
  args->count -= args->args_count;
  args->args_count = 0;
}

void args_delete_kwarg(gr_meta_args_t *args, const char *key) {
  args_node_t *tmp_node, *previous_node_by_keyword;

  if (args_find_previous_node_by_keyword(args, key, &previous_node_by_keyword)) {
    if (previous_node_by_keyword == NULL) {
      tmp_node = args->kwargs_head->next;
      args_decrease_arg_reference_count(args->kwargs_head);
      free(args->kwargs_head);
      args->kwargs_head = tmp_node;
      if (tmp_node == NULL) {
        args->kwargs_tail = NULL;
      }
      if (args->args_tail != NULL) {
        args->args_tail->next = tmp_node;
      }
    } else {
      tmp_node = previous_node_by_keyword->next->next;
      args_decrease_arg_reference_count(previous_node_by_keyword->next);
      free(previous_node_by_keyword->next);
      previous_node_by_keyword->next = tmp_node;
      if (tmp_node == NULL) {
        args->kwargs_tail = previous_node_by_keyword;
      }
    }
    --(args->kwargs_count);
    --(args->count);
  }
}

unsigned int args_args_count(const gr_meta_args_t *args) {
  return args->args_count;
}

unsigned int args_kwargs_count(const gr_meta_args_t *args) {
  return args->kwargs_count;
}

unsigned int args_count(const gr_meta_args_t *args) {
  return args->count;
}

int args_has_keyword(const gr_meta_args_t *args, const char *keyword) {
  return args_find_keyword(args, keyword) != NULL;
}

arg_t *args_find_keyword(const gr_meta_args_t *args, const char *keyword) {
  args_node_t *current_node;

  current_node = args->kwargs_head;
  while (current_node != NULL && strcmp(current_node->arg->key, keyword) != 0) {
    current_node = current_node->next;
  }

  if (current_node != NULL) {
    return current_node->arg;
  }
  return NULL;
}

int(args_get_first_value_by_keyword)(const gr_meta_args_t *args, const char *keyword, const char *first_value_format,
                                     void *first_value) {
  arg_t *arg;
  char *transformed_first_value_format;

  arg = args_find_keyword(args, keyword);
  if (arg == NULL) {
    return 0;
  }
  transformed_first_value_format = malloc(strlen(first_value_format) + 1);
  if (transformed_first_value_format == NULL) {
    debug_print_malloc_error();
    return 0;
  }
  args_copy_format_string_for_arg(transformed_first_value_format, first_value_format);
  /* check if value_format does not start with the transformed first_value_format */
  if (strncmp(arg->value_format, transformed_first_value_format, strlen(transformed_first_value_format)) != 0) {
    free(transformed_first_value_format);
    return 0;
  }
  free(transformed_first_value_format);
  switch (*arg->value_format) {
  case 'i':
    *(int *)first_value = *(int *)arg->value_ptr;
    break;
  case 'I':
    *(int **)first_value = *(int **)arg->value_ptr;
    break;
  case 'd':
    *(double *)first_value = *(double *)arg->value_ptr;
    break;
  case 'D':
    *(double **)first_value = *(double **)arg->value_ptr;
    break;
  case 'c':
    *(char *)first_value = *(char *)arg->value_ptr;
    break;
  case 'C':
  case 's':
    *(char **)first_value = *(char **)arg->value_ptr;
    break;
  case 'S':
    *(char ***)first_value = *(char ***)arg->value_ptr;
    break;
  default:
    return 0;
  }
  return 1;
}

int args_values_by_keyword(const gr_meta_args_t *args, const char *keyword, const char *expected_format, ...) {
  va_list vl;
  arg_t *arg;
  args_value_iterator_t *value_it;

  arg = args_find_keyword(args, keyword);
  if (arg == NULL) {
    return 0;
  }
  if (strcmp(expected_format, arg->value_format) != 0) {
    return 0;
  }

  va_start(vl, expected_format);
  value_it = args_value_iter(arg);
  while (value_it->next(value_it) != NULL) {
    switch (value_it->format) {
      void *current_value_ptr;
    case 'i':
      current_value_ptr = va_arg(vl, int *);
      *(int *)current_value_ptr = *(int *)value_it->value_ptr;
      break;
    case 'I':
      current_value_ptr = va_arg(vl, int **);
      *(int **)current_value_ptr = *(int **)value_it->value_ptr;
      break;
    case 'd':
      current_value_ptr = va_arg(vl, double *);
      *(double *)current_value_ptr = *(double *)value_it->value_ptr;
      break;
    case 'D':
      current_value_ptr = va_arg(vl, double **);
      *(double **)current_value_ptr = *(double **)value_it->value_ptr;
      break;
    case 'c':
      current_value_ptr = va_arg(vl, char *);
      *(char *)current_value_ptr = *(char *)value_it->value_ptr;
      break;
    case 'C':
    case 's':
      current_value_ptr = va_arg(vl, char **);
      *(char **)current_value_ptr = *(char **)value_it->value_ptr;
      break;
    case 'S':
      current_value_ptr = va_arg(vl, char ***);
      *(char ***)current_value_ptr = *(char ***)value_it->value_ptr;
      break;
    default:
      break;
    }
  }
  args_value_iterator_delete(value_it);
  va_end(vl);

  return 1;
}

const void *args_values_as_array_by_keyword(const gr_meta_args_t *args, const char *keyword) {
  arg_t *arg;

  arg = args_find_keyword(args, keyword);
  if (arg == NULL) {
    return NULL;
  }
  return args_values_as_array(arg);
}

args_node_t *args_find_node_by_keyword(const gr_meta_args_t *args, const char *keyword) {
  args_node_t *current_node;

  current_node = args->kwargs_head;
  while (current_node != NULL && strcmp(current_node->arg->key, keyword) != 0) {
    current_node = current_node->next;
  }

  return current_node;
}

int args_find_previous_node_by_keyword(const gr_meta_args_t *args, const char *keyword, args_node_t **previous_node) {
  args_node_t *prev_node, *current_node;

  prev_node = NULL;
  current_node = args->kwargs_head;
  while (current_node != NULL && strcmp(current_node->arg->key, keyword) != 0) {
    prev_node = current_node;
    current_node = current_node->next;
  }

  if (current_node != NULL) {
    *previous_node = prev_node;
    return 1;
  }
  return 0;
}

args_iterator_t *args_iter(const gr_meta_args_t *args) {
  return args_iterator_new(args->args_head, NULL);
}

args_iterator_t *args_iter_args(const gr_meta_args_t *args) {
  return args_iterator_new(args->args_head, args->kwargs_head);
}

args_iterator_t *args_iter_kwargs(const gr_meta_args_t *args) {
  return args_iterator_new(args->kwargs_head, NULL);
}


/* ------------------------- argument iterator ---------------------------------------------------------------------- */

args_iterator_t *args_iterator_new(const args_node_t *begin, const args_node_t *end) {
  args_iterator_t *args_iterator;

  args_iterator = malloc(sizeof(args_iterator_t));
  if (args_iterator == NULL) {
    debug_print_malloc_error();
    return NULL;
  }
  args_iterator->priv = malloc(sizeof(args_iterator_private_t));
  if (args_iterator->priv == NULL) {
    debug_print_malloc_error();
    free(args_iterator);
    return NULL;
  }
  args_iterator_init(args_iterator, begin, end);
  return args_iterator;
}

void args_iterator_init(args_iterator_t *args_iterator, const args_node_t *begin, const args_node_t *end) {
  args_iterator->next = args_iterator_next;
  args_iterator->arg = NULL;
  args_iterator->priv->next_node = begin;
  args_iterator->priv->end = end;
}

void args_iterator_delete(args_iterator_t *args_iterator) {
  args_iterator_finalize(args_iterator);
  free(args_iterator->priv);
  free(args_iterator);
}

void args_iterator_finalize(args_iterator_t *args_iterator) {
  UNUSED(args_iterator);
}

arg_t *args_iterator_next(args_iterator_t *args_iterator) {
  arg_t *next_arg;

  if ((args_iterator->priv->next_node != NULL) && (args_iterator->priv->next_node != args_iterator->priv->end)) {
    next_arg = args_iterator->priv->next_node->arg;
    args_iterator->priv->next_node = args_iterator->priv->next_node->next;
  } else {
    next_arg = NULL;
  }
  args_iterator->arg = next_arg;
  return next_arg;
}


/* ------------------------- value iterator ------------------------------------------------------------------------- */

args_value_iterator_t *args_value_iterator_new(const arg_t *arg) {
  args_value_iterator_t *args_value_iterator;

  args_value_iterator = malloc(sizeof(args_value_iterator_t));
  if (args_value_iterator == NULL) {
    debug_print_malloc_error();
    return NULL;
  }
  args_value_iterator->priv = malloc(sizeof(args_value_iterator_private_t));
  if (args_value_iterator->priv == NULL) {
    debug_print_malloc_error();
    free(args_value_iterator);
    return NULL;
  }
  args_value_iterator_init(args_value_iterator, arg);
  return args_value_iterator;
}

void args_value_iterator_init(args_value_iterator_t *args_value_iterator, const arg_t *arg) {
  args_value_iterator->next = args_value_iterator_next;
  args_value_iterator->value_ptr = NULL;
  args_value_iterator->format = '\0';
  args_value_iterator->is_array = 0;
  args_value_iterator->array_length = 0;
  args_value_iterator->priv->value_buffer = arg->value_ptr;
  args_value_iterator->priv->value_format = arg->value_format;
}

void args_value_iterator_delete(args_value_iterator_t *args_value_iterator) {
  args_value_iterator_finalize(args_value_iterator);
  free(args_value_iterator->priv);
  free(args_value_iterator);
}

void args_value_iterator_finalize(args_value_iterator_t *args_value_iterator) {
  UNUSED(args_value_iterator);
}

void *args_value_iterator_next(args_value_iterator_t *args_value_iterator) {
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
  while (*format) {
    format = args_skip_option(format);
    if (!*format) {
      break;
    }
    current_format = tolower(*format);
    if (current_format != *format) {
      is_array = 1;
      array_length = *((size_t *)value_buffer);
      value_buffer = ((size_t *)value_buffer) + 1;
      value_ptr = value_buffer;
    }
#define STEP_VALUE_BUFFER_BY_TYPE(char, type)     \
  case char:                                      \
    if (is_array) {                               \
      value_buffer = ((type **)value_buffer) + 1; \
    } else {                                      \
      value_buffer = ((type *)value_buffer) + 1;  \
    }                                             \
    break;

    switch (current_format) {
      STEP_VALUE_BUFFER_BY_TYPE('i', int)
      STEP_VALUE_BUFFER_BY_TYPE('d', double)
      STEP_VALUE_BUFFER_BY_TYPE('c', char)
      STEP_VALUE_BUFFER_BY_TYPE('s', char *)
    case 'n':
      /* 'n' is not relevant for reading the values -> ignore it */
      break;
    default:
      break;
    }

#undef STEP_VALUE_BUFFER_BY_TYPE

    if (strchr(ARGS_VALID_DATA_FORMAT_SPECIFIERS, current_format) != NULL) {
      extracted_next_value = 1;
      break;
    }
    ++format;
  }

  if (extracted_next_value) {
    args_value_iterator->is_array = is_array;
    args_value_iterator->array_length = array_length;
    args_value_iterator->format = current_format;
    args_value_iterator->priv->value_format = ++format; /* FIXME: does not work for options! */
  } else {
    value_ptr = NULL;
    args_value_iterator->format = '\0';
  }

  args_value_iterator->priv->value_buffer = value_buffer;
  args_value_iterator->value_ptr = value_ptr;
  return value_ptr;
}


/* ------------------------- json serializer ------------------------------------------------------------------------ */

#define CHECK_PADDING(type)                                                         \
  do {                                                                              \
    if (state->shared->data_ptr != NULL && state->shared->apply_padding) {          \
      ptrdiff_t needed_padding = state->shared->data_offset % sizeof(type);         \
      state->shared->data_ptr = ((char *)state->shared->data_ptr) + needed_padding; \
      state->shared->data_offset += needed_padding;                                 \
    }                                                                               \
  } while (0)

#define RETRIEVE_SINGLE_VALUE(var, type, promoted_type)                \
  do {                                                                 \
    CHECK_PADDING(type);                                               \
    if (state->shared->data_ptr != NULL) {                             \
      var = *((type *)state->shared->data_ptr);                        \
      state->shared->data_ptr = ((type *)state->shared->data_ptr) + 1; \
      state->shared->data_offset += sizeof(type);                      \
    } else {                                                           \
      var = va_arg(*state->shared->vl, promoted_type);                 \
    }                                                                  \
  } while (0)

#define INIT_MULTI_VALUE(vars, type)               \
  do {                                             \
    if (state->shared->data_ptr != NULL) {         \
      if (state->next_is_ptr) {                    \
        CHECK_PADDING(type *);                     \
        vars = *(type **)state->shared->data_ptr;  \
      } else {                                     \
        CHECK_PADDING(type);                       \
        vars = (type *)state->shared->data_ptr;    \
      }                                            \
    } else {                                       \
      if (state->next_is_ptr) {                    \
        vars = va_arg(*state->shared->vl, type *); \
      }                                            \
    }                                              \
  } while (0)

#define RETRIEVE_NEXT_VALUE(var, vars, type)    \
  do {                                          \
    if (state->shared->data_ptr != NULL) {      \
      var = *vars++;                            \
    } else {                                    \
      if (state->next_is_ptr) {                 \
        var = *vars++;                          \
      } else {                                  \
        var = va_arg(*state->shared->vl, type); \
      }                                         \
    }                                           \
  } while (0)

#define FIN_MULTI_VALUE(type)                                                 \
  do {                                                                        \
    if (state->shared->data_ptr != NULL) {                                    \
      if (state->next_is_ptr) {                                               \
        state->shared->data_ptr = ((type **)state->shared->data_ptr) + 1;     \
        state->shared->data_offset += sizeof(type *);                         \
      } else {                                                                \
        state->shared->data_ptr = ((type *)state->shared->data_ptr) + length; \
        state->shared->data_offset += length * sizeof(type);                  \
      }                                                                       \
    }                                                                         \
  } while (0)

#define DEF_STRINGIFY_SINGLE(type, promoted_type, format_specifier)             \
  void tojson_stringify_##type(memwriter_t *memwriter, tojson_state_t *state) { \
    type value;                                                                 \
    RETRIEVE_SINGLE_VALUE(value, type, promoted_type);                          \
    memwriter_printf(memwriter, format_specifier, value);                       \
    state->shared->wrote_output = 1;                                            \
  }

#define DEF_STRINGIFY_MULTI(type, format_specifier)                                                                 \
  void tojson_stringify_##type##_array(memwriter_t *memwriter, tojson_state_t *state) {                             \
    type *values;                                                                                                   \
    type current_value;                                                                                             \
    int length;                                                                                                     \
    int remaining_elements;                                                                                         \
    INIT_MULTI_VALUE(values, type);                                                                                 \
    if (state->additional_type_info != NULL) {                                                                      \
      int was_successful;                                                                                           \
      length = str_to_uint(state->additional_type_info, &was_successful);                                           \
      if (!was_successful) {                                                                                        \
        debug_print_error(("The given array length \"%s\" is no valid number; the array contents will be ignored.", \
                           state->additional_type_info));                                                           \
        length = 0;                                                                                                 \
      }                                                                                                             \
    } else {                                                                                                        \
      length = state->shared->array_length;                                                                         \
    }                                                                                                               \
    remaining_elements = length;                                                                                    \
    /* write array start */                                                                                         \
    memwriter_putc(memwriter, '[');                                                                                 \
    /* write array content */                                                                                       \
    while (remaining_elements) {                                                                                    \
      RETRIEVE_NEXT_VALUE(current_value, values, type);                                                             \
      memwriter_printf(memwriter, format_specifier "%s", current_value, ((remaining_elements > 1) ? "," : ""));     \
      --remaining_elements;                                                                                         \
    }                                                                                                               \
    /* write array end */                                                                                           \
    memwriter_putc(memwriter, ']');                                                                                 \
    FIN_MULTI_VALUE(type);                                                                                          \
    state->shared->wrote_output = 1;                                                                                \
    state->next_is_ptr = 0;                                                                                         \
  }

DEF_STRINGIFY_SINGLE(int, int, "%d")
DEF_STRINGIFY_MULTI(int, "%d")
DEF_STRINGIFY_SINGLE(double, double, "%f")
DEF_STRINGIFY_MULTI(double, "%f")
DEF_STRINGIFY_SINGLE(char, int, "%c")

#undef DEF_STRINGIFY_SINGLE
#undef DEF_STRINGIFY_MULTI

void tojson_stringify_char_array(memwriter_t *memwriter, tojson_state_t *state) {
  char *chars;
  int length;

  if (state->shared->data_ptr == NULL && !state->next_is_ptr) {
    debug_print_error(("Serializing strings from single chars is not supported when using variable argument lists."));
  }

  if (state->next_is_ptr) {
    if (state->shared->data_ptr != NULL) {
      CHECK_PADDING(char *);
      chars = *(char **)state->shared->data_ptr;
    } else {
      chars = va_arg(*state->shared->vl, char *);
    }
  } else {
    CHECK_PADDING(char);
    chars = (char *)state->shared->data_ptr;
  }

  if (state->additional_type_info != NULL) {
    int was_successful;
    length = str_to_uint(state->additional_type_info, &was_successful);
    if (!was_successful) {
      debug_print_error(("The given array length \"%s\" is no valid number; the array contents will be ignored.",
                         state->additional_type_info));
      length = 0;
    }
  } else {
    length = state->shared->array_length;
  }
  memwriter_printf(memwriter, "\"%s\"", chars);

  FIN_MULTI_VALUE(char);
  state->shared->wrote_output = 1;
  state->next_is_ptr = 0;
}

void tojson_stringify_bool(memwriter_t *memwriter, tojson_state_t *state) {
  int value;
  RETRIEVE_SINGLE_VALUE(value, int, int);
  memwriter_puts(memwriter, value ? "true" : "false");
  state->shared->wrote_output = 1;
}

int tojson_get_member_count(const char *data_desc) {
  int nested_level = 0;
  int member_count = 0;
  if (data_desc == NULL || *data_desc == '\0') {
    return 0;
  }
  while (*data_desc != 0) {
    switch (*data_desc) {
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

int tojson_is_json_array_needed(const char *data_desc) {
  const char *relevant_data_types = "iIdDcCs";
  int nested_level = 0;
  int count_relevant_data_types = 0;

  while (*data_desc != 0 && count_relevant_data_types < 2) {
    switch (*data_desc) {
    case '(':
      ++nested_level;
      break;
    case ')':
      --nested_level;
      break;
    default:
      if (nested_level == 0) {
        if (strchr(relevant_data_types, *data_desc)) {
          ++count_relevant_data_types;
        }
      }
      break;
    }
    ++data_desc;
  }
  return count_relevant_data_types >= 2;
}

int tojson_read_datatype(tojson_state_t *state) {
  char *additional_type_info = NULL;
  state->current_data_type = *state->data_type_ptr;
  ++(state->data_type_ptr);
  if (*state->data_type_ptr == '(') {
    int nested_level = 1;
    additional_type_info = ++(state->data_type_ptr);
    while (*state->data_type_ptr != 0 && nested_level > 0) {
      switch (*state->data_type_ptr) {
      case '(':
        ++nested_level;
        break;
      case ')':
        --nested_level;
        break;
      }
      if (nested_level > 0) {
        ++(state->data_type_ptr);
      }
    }
    if (*state->data_type_ptr != 0) {
      *(state->data_type_ptr)++ = 0; /* termination character for additional_type_info */
      state->is_type_info_incomplete = 0;
    } else {
      state->is_type_info_incomplete = 1; /* character search hit '\0' and not ')' */
    }
  }
  state->additional_type_info = additional_type_info;
  return 0;
}

void tojson_read_next_is_ptr(tojson_state_t *state) {
  state->next_is_ptr = 1;
}

void tojson_skip_bytes(tojson_state_t *state) {
  int count;

  if (state->shared->data_ptr == NULL) {
    debug_print_error(("Skipping bytes is not supported when using the variable argument list and is ignored.\n"));
    return;
  }

  if (state->additional_type_info != NULL) {
    int was_successful;
    count = str_to_uint(state->additional_type_info, &was_successful);
    if (!was_successful) {
      debug_print_error(("Byte skipping with an invalid number -> ignoring.\n"));
      return;
    }
  } else {
    count = 1;
  }
  state->shared->data_ptr = ((char *)state->shared->data_ptr) + count;
  state->shared->data_offset += count;
}

void tojson_close_object(memwriter_t *memwriter, tojson_state_t *state) {
  --(state->shared->struct_nested_level);
  memwriter_putc(memwriter, '}');
}

void tojson_read_array_length(tojson_state_t *state) {
  int value;

  RETRIEVE_SINGLE_VALUE(value, int, int);
  state->shared->array_length = value;
}

void tojson_unzip_membernames_and_datatypes(char *mixed_ptr, char ***member_name_ptr, char ***data_type_ptr) {
  int member_count;
  char **arrays[2];

  member_count = tojson_get_member_count(mixed_ptr);
  /* add 1 to member count for a terminatory NULL pointer */
  *member_name_ptr = malloc((member_count + 1) * sizeof(char *));
  *data_type_ptr = malloc((member_count + 1) * sizeof(char *));
  if (*member_name_ptr == NULL || *data_type_ptr == NULL) {
    free(*member_name_ptr);
    free(*data_type_ptr);
    *member_name_ptr = *data_type_ptr = NULL;
    debug_print_malloc_error();
    return;
  }
  arrays[member_name] = *member_name_ptr;
  arrays[data_type] = *data_type_ptr;
  if (member_count > 0) {
    char separators[2] = {':', ','};
    int current_array_index = member_name;
    int nested_type_level = 0;
    *arrays[current_array_index]++ = mixed_ptr;

    /* iterate over the whole type list */
    assert(mixed_ptr != NULL); /* otherwise there is an internal logical error since member_count > 0 */
    while (nested_type_level >= 0 && *mixed_ptr != 0) {
      /* extract one name or one type */
      while (*mixed_ptr != 0 && (nested_type_level > 0 || *mixed_ptr != separators[current_array_index])) {
        if (current_array_index == data_type) {
          switch (*mixed_ptr) {
          case '(':
            ++nested_type_level;
            break;
          case ')':
            --nested_type_level;
            break;
          }
        }
        if (nested_type_level >= 0) {
          ++mixed_ptr;
        }
      }
      if (*mixed_ptr != 0) {
        *mixed_ptr++ = 0;                              /* terminate string in buffer */
        current_array_index = 1 - current_array_index; /* alternate between member_name (0) and data_type (1) */
        *arrays[current_array_index]++ = mixed_ptr;
      }
    }
  }
  *arrays[member_name] = NULL;
  *arrays[data_type] = NULL;
}

void tojson_stringify_struct(memwriter_t *memwriter, tojson_state_t *state) {
  char **member_names;
  char **data_types;
  char **member_name_ptr;
  char **data_type_ptr;
  int has_members;

  /* IMPORTANT: additional_type_info is altered after the unzip call! */
  tojson_unzip_membernames_and_datatypes(state->additional_type_info, &member_names, &data_types);
  member_name_ptr = member_names;
  data_type_ptr = data_types;

  has_members =
    (member_name_ptr != NULL && *member_name_ptr != NULL && data_type_ptr != NULL && *data_type_ptr != NULL);
  /* write object start */
  if (!state->shared->add_data_without_separator) {
    if (state->shared->add_data && has_members) {
      memwriter_putc(memwriter, ',');
      state->shared->add_data = 0;
    } else if (!state->shared->add_data) {
      memwriter_putc(memwriter, '{');
      ++(state->shared->struct_nested_level);
    }
  }
  if (has_members) {
    /* write object content */
    int serialized_all_members = 0;
    while (!serialized_all_members) {
      memwriter_printf(memwriter, "\"%s\":", *member_name_ptr);
      tojson_serialize(memwriter, *data_type_ptr, NULL, NULL, -1, -1, -1, NULL, NULL, state->shared);
      ++member_name_ptr;
      ++data_type_ptr;
      if (*member_name_ptr != NULL && *data_type_ptr != NULL) {
        /* write JSON separator */
        memwriter_putc(memwriter, ',');
      } else {
        serialized_all_members = 1;
      }
    }
  }
  /* write object end if the type info is complete */
  if (!state->is_type_info_incomplete) {
    --(state->shared->struct_nested_level);
    memwriter_putc(memwriter, '}');
  }
  /* Only set serial result if not set before */
  if (state->shared->serial_result == 0) {
    if ((state->is_type_info_incomplete)) {
      state->shared->serial_result = has_members ? incomplete : incomplete_at_struct_beginning;
    }
  }

  /* cleanup */
  free(member_names);
  free(data_types);

  state->shared->wrote_output = 1;
}

#undef CHECK_PADDING
#undef RETRIEVE_SINGLE_VALUE
#undef INIT_MULTI_VALUE
#undef RETRIEVE_NEXT_VALUE
#undef FIN_MULTI_VALUE

int tojson_serialize(memwriter_t *memwriter, char *data_desc, const void *data, va_list *vl, int apply_padding,
                     int add_data, int add_data_without_separator, unsigned int *struct_nested_level,
                     tojson_serialization_result_t *serial_result, tojson_shared_state_t *shared_state) {
  /**
   * memwriter: memwriter handle
   * data_desc: data description
   *      i: int
   *      I: int array -> I(count) or nI for variable length (see 'n' below); 'pI' indicates a pointer to an array
   *      d: double
   *      D: double array
   *      c: char
   *      C: string (char array)
   *      n: array length (for all following arrays)
   *      s: struct -> s(name:type, name:type, ...)
   *      e: empty byte (ignored memory) -> e(count) to specify multiple bytes
   * data: pointer to the buffer that shall be serialized
   * vl: if data is NULL the needed values are read from the va_list vl
   */

  tojson_state_t state;
  int json_array_needed = 0;
  int allocated_shared_state_mem = 0;

  state.next_is_ptr = 0;
  state.data_type_ptr = data_desc;
  state.current_data_type = 0;
  state.additional_type_info = NULL;
  state.is_type_info_incomplete = 0;
  if (shared_state == NULL) {
    shared_state = malloc(sizeof(tojson_shared_state_t));
    if (shared_state == NULL) {
      debug_print_malloc_error();
      return -1;
    }
    shared_state->apply_padding = apply_padding;
    shared_state->array_length = 0;
    shared_state->data_ptr = data;
    shared_state->vl = vl;
    shared_state->data_offset = 0;
    shared_state->wrote_output = 0;
    shared_state->add_data = add_data;
    shared_state->add_data_without_separator = add_data_without_separator;
    shared_state->serial_result = 0;
    shared_state->struct_nested_level = *struct_nested_level;
    allocated_shared_state_mem = 1;
  } else {
    if (data != NULL) {
      shared_state->data_ptr = data;
    }
    if (vl != NULL) {
      shared_state->vl = vl;
    }
    if (apply_padding >= 0) {
      shared_state->apply_padding = apply_padding;
    }
  }
  state.shared = shared_state;

  json_array_needed = tojson_is_json_array_needed(data_desc);
  /* write list head if needed */
  if (json_array_needed) {
    memwriter_putc(memwriter, '[');
  }
  while (*state.data_type_ptr != 0) {
    shared_state->wrote_output = 0;
    tojson_read_datatype(&state);
    switch (state.current_data_type) {
    case 'n':
      tojson_read_array_length(&state);
      break;
    case 'p':
      tojson_read_next_is_ptr(&state);
      break;
    case 'e':
      tojson_skip_bytes(&state);
      break;
    case 'i':
      tojson_stringify_int(memwriter, &state);
      break;
    case 'I':
      tojson_stringify_int_array(memwriter, &state);
      break;
    case 'd':
      tojson_stringify_double(memwriter, &state);
      break;
    case 'D':
      tojson_stringify_double_array(memwriter, &state);
      break;
    case 'c':
      tojson_stringify_char(memwriter, &state);
      break;
    case 'C':
      tojson_stringify_char_array(memwriter, &state);
      break;
    case 'b':
      tojson_stringify_bool(memwriter, &state);
      break;
    case 's':
      tojson_stringify_struct(memwriter, &state);
      break;
    case ')':
      tojson_close_object(memwriter, &state);
      break;
    default:
      debug_print_error(("WARNING: %c (ASCII code %d) is not a valid type identifier\n", state.current_data_type,
                         state.current_data_type));
      break;
    }
    if (*state.data_type_ptr != 0 && *state.data_type_ptr != ')' && shared_state->wrote_output) {
      /* write JSON separator, if data was written and the object is not closed in the next step */
      memwriter_putc(memwriter, ',');
    }
  }
  /* write list tail if needed */
  if (json_array_needed) {
    memwriter_putc(memwriter, ']');
  }

  if (serial_result != NULL) {
    /* check if shared_state->serial_result was set before */
    if (shared_state->serial_result) {
      *serial_result = shared_state->serial_result;
    } else {
      *serial_result = (shared_state->struct_nested_level > 0) ? incomplete : complete;
    }
  }
  if (struct_nested_level != NULL) {
    *struct_nested_level = shared_state->struct_nested_level;
  }

  /* cleanup */
  if (allocated_shared_state_mem) {
    free(shared_state);
  }

  return 0;
}

int tojson_init_variables(int *add_data, int *add_data_without_separator, char **_data_desc, const char *data_desc) {
  *add_data = (tojson_permanent_state.serial_result != complete);
  *add_data_without_separator = (tojson_permanent_state.serial_result == incomplete_at_struct_beginning);
  if (*add_data) {
    char *data_desc_ptr;
    int data_desc_len = strlen(data_desc);
    *_data_desc = malloc(data_desc_len + 3);
    if (*_data_desc == NULL) {
      debug_print_malloc_error();
      return -1;
    }
    data_desc_ptr = *_data_desc;
    if (strncmp(data_desc, "s(", 2) != 0) {
      memcpy(data_desc_ptr, "s(", 2);
      data_desc_ptr += 2;
    }
    memcpy(data_desc_ptr, data_desc, data_desc_len);
    data_desc_ptr += data_desc_len;
    *data_desc_ptr = '\0';
  } else {
    *_data_desc = strdup(data_desc);
    if (*_data_desc == NULL) {
      debug_print_malloc_error();
      return -1;
    }
  }

  return 0;
}

int tojson_write_vl(memwriter_t *memwriter, const char *data_desc, va_list *vl) {
  int add_data, add_data_without_separator;
  char *_data_desc;
  int error = 0;

  error = tojson_init_variables(&add_data, &add_data_without_separator, &_data_desc, data_desc);
  if (!error) {
    error = tojson_serialize(memwriter, _data_desc, NULL, vl, 0, add_data, add_data_without_separator,
                             &tojson_permanent_state.struct_nested_level, &tojson_permanent_state.serial_result, NULL);
  }
  free(_data_desc);

  return error;
}

int tojson_write_buf(memwriter_t *memwriter, const char *data_desc, const void *buffer, int apply_padding) {
  int add_data, add_data_without_separator;
  char *_data_desc;
  int error = 0;

  error = tojson_init_variables(&add_data, &add_data_without_separator, &_data_desc, data_desc);
  if (!error) {
    error = tojson_serialize(memwriter, _data_desc, buffer, NULL, apply_padding, add_data, add_data_without_separator,
                             &tojson_permanent_state.struct_nested_level, &tojson_permanent_state.serial_result, NULL);
  }
  free(_data_desc);

  return error;
}

int tojson_is_complete() {
  return tojson_permanent_state.serial_result == complete;
}


/* ------------------------- memwriter ------------------------------------------------------------------------------ */

memwriter_t *memwriter_new() {
  memwriter_t *memwriter;

  memwriter = malloc(sizeof(memwriter_t));
  if (memwriter == NULL) {
    debug_print_malloc_error();
    return NULL;
  }
  memwriter->buf = malloc(MEMWRITER_INITIAL_SIZE);
  if (memwriter->buf == NULL) {
    free(memwriter);
    debug_print_malloc_error();
    return NULL;
  }
  memwriter->size = 0;
  memwriter->capacity = MEMWRITER_INITIAL_SIZE;

  return memwriter;
}

void memwriter_delete(memwriter_t *memwriter) {
  if (memwriter != NULL) {
    free(memwriter->buf);
    free(memwriter);
  }
}

void memwriter_clear(memwriter_t *memwriter) {
  memwriter->size = 0;
  *memwriter->buf = '\0';
}

int memwriter_enlarge_buf(memwriter_t *memwriter) {
  void *new_buf;

  new_buf = realloc(memwriter->buf, memwriter->capacity + MEMWRITER_SIZE_INCREMENT);
  if (new_buf == NULL) {
    debug_print_malloc_error();
    return 0;
  }
  memwriter->buf = new_buf;
  memwriter->capacity += MEMWRITER_SIZE_INCREMENT;

  return 1;
}

int memwriter_printf(memwriter_t *memwriter, const char *format, ...) {
  va_list vl;
  int was_successful;

  was_successful = 0;
  while (1) {
    int chars_needed;
    va_start(vl, format);
    chars_needed = vsnprintf(&memwriter->buf[memwriter->size], memwriter->capacity - memwriter->size, format, vl);
    va_end(vl);
    if (chars_needed < (int)(memwriter->capacity - memwriter->size)) {
      memwriter->size += chars_needed;
      was_successful = 1;
      break;
    }
    if (!memwriter_enlarge_buf(memwriter)) {
      break;
    }
  }

  return was_successful;
}

int memwriter_puts(memwriter_t *memwriter, const char *s) {
  return memwriter_printf(memwriter, "%s", s);
}

int memwriter_putc(memwriter_t *memwriter, char c) {
  return memwriter_printf(memwriter, "%c", c);
}

char *memwriter_buf(const memwriter_t *memwriter) {
  return memwriter->buf;
}

size_t memwriter_size(const memwriter_t *memwriter) {
  return memwriter->size;
}


/* ------------------------- sender --------------------------------------------------------------------------------- */

int sender_init_for_socket(metahandle_t *handle, va_list *vl) {
  const char *hostname;
  unsigned int port;
  struct hostent *he;
#if defined(_WIN32) && !defined(__GNUC__)
  WSADATA wsa_data;
#endif

  hostname = va_arg(*vl, const char *);
  port = va_arg(*vl, unsigned int);

#if defined(_WIN32) && !defined(__GNUC__)
  /* Initialize Winsock */
  if (WSAStartup(MAKEWORD(2, 2), &wsa_data)) {
    debug_print_error(("Winsock initialization failed."));
    return -1;
  }
#endif

  he = gethostbyname(hostname);
  if (he == NULL || he->h_addr_list == NULL) {
    perror("gethostbyname");
    return -1;
  }
  handle->comm.socket.client_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  memcpy(&handle->comm.socket.server_address.sin_addr, he->h_addr_list[0], sizeof(struct in_addr));
  handle->comm.socket.server_address.sin_family = AF_INET;
  handle->comm.socket.server_address.sin_port = htons(port);
  if (connect(handle->comm.socket.client_socket_fd, (struct sockaddr *)&handle->comm.socket.server_address,
              sizeof(handle->comm.socket.server_address)) < 0) {
    perror("connect");
    return -1;
  }
  handle->memwriter = memwriter_new();
  if (handle->memwriter == NULL) {
    return -1;
  }
  handle->post_serialize = sender_post_serialize_socket;

  return 0;
}

int sender_init_for_jupyter(metahandle_t *handle, va_list *vl) {
  jupyter_send_callback_t jupyter_send_callback;

  jupyter_send_callback = va_arg(*vl, jupyter_send_callback_t);

  handle->comm.jupyter.send = jupyter_send_callback;
  handle->memwriter = memwriter_new();
  if (handle->memwriter == NULL) {
    return -1;
  }
  handle->post_serialize = sender_post_serialize_jupyter;
  return 0;
}

int sender_finalize_for_jupyter(metahandle_t *handle) {
  UNUSED(handle);
  return 0;
}

int sender_finalize_for_socket(metahandle_t *handle) {
  int result;
  int error = 0;

#if defined(_WIN32)
  result = closesocket(handle->comm.socket.client_socket_fd);
#else
  result = close(handle->comm.socket.client_socket_fd);
#endif
#if defined(_WIN32) && !defined(__GNUC__)
  result |= WSACleanup();
#endif
  if (result != 0) {
    debug_print_error(("Winsocket shutdown failed."));
    error = -1;
  }
  return error;
}
